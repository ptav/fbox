/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Test yield curve agents
*/


#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <fbox/line.h>
#include "../yield_curve_models.h"
#include "../basic_agents.h"
#include "../simulator.h"
#include "../observer.h"


using namespace fbox::simulate;


BOOST_AUTO_TEST_CASE(test_shifted)
{
	using fbox::math::linear_line;
	using fbox::math::flat_line;

	boost::shared_ptr<static_yield_curve> x( new static_yield_curve );
	boost::shared_ptr<linear_line> df( new linear_line );
	df->add(0,1);
	df->add(365,exp(-0.05));
	df->add(730,exp(-0.10));
	x->setup(df);

	boost::shared_ptr<shifted_yield_curve> y( new shifted_yield_curve );
	boost::shared_ptr<flat_line> shift( new flat_line );
	shift->setup(2);
	y->setup(x,shift,shifted_yield_curve::product);

	boost::shared_ptr<static_yield_curve::spot_bond> b1( new static_yield_curve::spot_bond );
	boost::shared_ptr<shifted_yield_curve::term_bond> b2( new shifted_yield_curve::term_bond );

	simulator<statistics> sim;
	sim.add_fix(0);
	sim.add_fix(365);
	sim.set_step(1000);
	sim.set_samples(10);

	b1->setup(x);
	sim.simulate(b1);
	BOOST_CHECK_CLOSE(sim.observer(1).expectation(),(*df)(365),1e-2);
	BOOST_CHECK_SMALL(sim.observer(1).variance(),1e-10);

	b1->setup(y);
	sim.simulate(b1);
	BOOST_CHECK_CLOSE(sim.observer(1).expectation(),(*df)(365)*(*df)(365),1e-2);
	BOOST_CHECK_SMALL(sim.observer(1).variance(),1e-10);

	b2->setup(x,730);
	sim.simulate(b2);
	BOOST_CHECK_CLOSE(sim.observer(1).expectation(),exp(-0.05),1e-2);
	BOOST_CHECK_SMALL(sim.observer(1).variance(),1e-10);

	b2->setup(y,730);
	sim.simulate(b2);
	BOOST_CHECK_CLOSE(sim.observer(1).expectation(),exp(-0.10),1e-2);
	BOOST_CHECK_SMALL(sim.observer(1).variance(),1e-10);
}


BOOST_AUTO_TEST_CASE(test_static_yc)
{
	double rf = 0.05;
	boost::shared_ptr<fbox::math::linear_line> df( new fbox::math::linear_line );
	for(double t = 0.0; t < 3651.0; t += 10.0) df->add(t,exp(-rf*t/365.0));

	boost::shared_ptr<static_yield_curve> yc( new static_yield_curve );
	yc->setup(df);

	boost::shared_ptr<hw_yield_curve::term_bond> bnd( new hw_yield_curve::term_bond );
	bnd->setup(yc,730);

	simulator<statistics> sim;
	sim.add_fix(0);
	sim.add_fix(180);
	sim.add_fix(365);
	sim.add_fix(545);
	sim.add_fix(730);
	sim.set_samples(10);

	sim.simulate(bnd);
	BOOST_CHECK_CLOSE(sim.observer(0).expectation(),(*df)(730.0),1e-6);
	BOOST_CHECK_CLOSE(sim.observer(1).expectation(),(*df)(730.0)/(*df)(180.0),1e-6);
	BOOST_CHECK_CLOSE(sim.observer(2).expectation(),(*df)(730.0)/(*df)(365.0),1e-6);
	BOOST_CHECK_CLOSE(sim.observer(3).expectation(),(*df)(730.0)/(*df)(545.0),1e-6);
	BOOST_CHECK_CLOSE(sim.observer(4).expectation(),1.0,1e-6);
}



BOOST_AUTO_TEST_CASE(test_hw)
{
	double rf = 0.05;
	boost::shared_ptr<fbox::math::linear_line> df( new fbox::math::linear_line );
	for(double t = 0.0; t < 3651.0; t += 10.0) df->add(t,exp(-rf*t/365.0));

	boost::shared_ptr<gaussian_variate> rnd( new gaussian_variate );
	boost::shared_ptr<hw_yield_curve> yc( new hw_yield_curve );
	yc->setup(rnd,df,0.1,0.0);

	boost::shared_ptr<hw_yield_curve::term_bond> bnd( new hw_yield_curve::term_bond );
	bnd->setup(yc,730);

	simulator<statistics> sim;
	sim.add_fix(0);
	sim.add_fix(180);
	sim.add_fix(365);
	sim.add_fix(545);
	sim.add_fix(730);
	sim.set_samples(5);
	sim.set_step(92);

	sim.simulate(yc);
	BOOST_CHECK_CLOSE(sim.observer(0).expectation(),-std::log((*df)(1)/(*df)(0))*365.0,1e-3);
	BOOST_CHECK_CLOSE(sim.observer(1).expectation(),-std::log((*df)(181)/(*df)(180))*365.0,1e-3);
	BOOST_CHECK_CLOSE(sim.observer(2).expectation(),-std::log((*df)(366)/(*df)(365))*365.0,1e-3);
	BOOST_CHECK_CLOSE(sim.observer(3).expectation(),-std::log((*df)(546)/(*df)(545))*365.0,1e-3);
	BOOST_CHECK_CLOSE(sim.observer(4).expectation(),-std::log((*df)(731)/(*df)(730))*365.0,1e-3);

	sim.simulate(bnd);
	BOOST_CHECK_CLOSE(sim.observer(0).expectation(),(*df)(730.0),1e-6);
	BOOST_CHECK_CLOSE(sim.observer(1).expectation(),(*df)(730.0)/(*df)(180.0),1e-4);
	BOOST_CHECK_CLOSE(sim.observer(2).expectation(),(*df)(730.0)/(*df)(365.0),1e-4);
	BOOST_CHECK_CLOSE(sim.observer(3).expectation(),(*df)(730.0)/(*df)(545.0),1e-4);
	BOOST_CHECK_CLOSE(sim.observer(4).expectation(),1.0,1e-6);


	yc->setup(rnd,df,.01,.02);

	sim.simulate(bnd);
	BOOST_CHECK_CLOSE(sim.observer(0).expectation(),(*df)(730.0),1e-6);
}
