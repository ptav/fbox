/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Test model agents
*/


#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include "../basic_agents.h"
#include "../models.h"
#include "../simulator.h"
#include "../observer.h"


using namespace fbox::simulate;


BOOST_AUTO_TEST_CASE(test_gaussian)
{
	simulator<statistics> sim;
	sim.add_fix(0);
	sim.add_fix(365);
	sim.set_step(1000);
	sim.set_samples(100000);

	boost::shared_ptr<gaussian_variate> x( new gaussian_variate );

	sim.simulate(x);
	BOOST_CHECK_SMALL(sim.observer(1).expectation(),1e-3);
	BOOST_CHECK_CLOSE(sim.observer(1).standard_deviation(),1.0,1);
}


BOOST_AUTO_TEST_CASE(test_gaussian_twister)
{
	simulator<statistics> sim;
	sim.add_fix(0);
	sim.add_fix(365);
	sim.set_step(1000);
	sim.set_samples(100000);

	boost::shared_ptr<gaussian_variate> rnd( new gaussian_variate );
	boost::shared_ptr<gaussian_twister> x( new gaussian_twister );
	
	x->setup(rnd,0.0);
	sim.simulate(x);
	BOOST_CHECK_SMALL(sim.observer(1).expectation(),1e-3);
	BOOST_CHECK_CLOSE(sim.observer(1).standard_deviation(),1.0,1);

	x->setup(rnd,1.0);
	sim.simulate(x);
	BOOST_CHECK_SMALL(sim.observer(1).expectation(),1e-2);
	BOOST_CHECK_CLOSE(sim.observer(1).standard_deviation(),1.0,1);

	x->setup(rnd,-1.0);
	sim.simulate(x);
	BOOST_CHECK_SMALL(sim.observer(1).expectation(),1e-2);
	BOOST_CHECK_CLOSE(sim.observer(1).standard_deviation(),1.0,1);
}


BOOST_AUTO_TEST_CASE(test_diffusion)
{
	boost::shared_ptr<gaussian_variate> g( new gaussian_variate );
	boost::shared_ptr<basic_diffusion> x( new basic_diffusion );
	x->setup(g,0.0,1.0,0.0);

	simulator<statistics> sim;
	sim.add_fix(0);
	sim.add_fix(365);
	sim.add_fix(730);
	sim.set_step(1000);
	sim.set_samples(10000);

	sim.simulate(x);

	BOOST_CHECK_SMALL(sim.observer(0).expectation(),1e-2);
	BOOST_CHECK_SMALL(sim.observer(1).expectation(),1e-2);
	BOOST_CHECK_SMALL(sim.observer(2).expectation(),1e-2);

	BOOST_CHECK_SMALL(sim.observer(0).variance(),1e-2);
	BOOST_CHECK_CLOSE(sim.observer(1).variance(),1.0,5);
	BOOST_CHECK_CLOSE(sim.observer(2).variance(),2.0,5);
}


BOOST_AUTO_TEST_CASE(test_lognormal)
{
	boost::shared_ptr<gaussian_variate> g( new gaussian_variate );
	boost::shared_ptr<basic_lognormal> x( new basic_lognormal );
	x->setup(g,0.0,1.0,1.0);

	simulator<expectation> sim;
	sim.add_fix(0);
	sim.add_fix(365);
	sim.set_step(1000);
	sim.set_samples(10000);

	sim.simulate(x);
	double e = sim.observer(1).value();
	BOOST_CHECK_CLOSE(1.0,e,2);
}


BOOST_AUTO_TEST_CASE(test_ou)
{
	double level = 0.5;
	double speed = 1.0;
	double vol = 0.02;

	boost::shared_ptr<gaussian_variate> g( new gaussian_variate );
	boost::shared_ptr<constant<double> > l( new constant<double> );
	boost::shared_ptr<basic_ou> x( new basic_ou );
	l->setup(level);
	x->setup(g,l,speed,vol,0.0);

	simulator<statistics> sim;
//	sim.add_fix(0);
//	sim.add_fix(365.25);
	sim.add_fix(3652);
	sim.set_step(366); // 1 yr steps
	sim.set_samples(10000);

	sim.simulate(x);
	double e = sim.observer(0).expectation();
	double v = sim.observer(0).variance();
	BOOST_CHECK_CLOSE(level,e,1); // test asyntotic expectation and variance
	BOOST_CHECK_CLOSE(vol*vol/speed/2.0,v,1);
}


BOOST_AUTO_TEST_CASE(test_jump)
{
	boost::shared_ptr<uniform_variate> g( new uniform_variate );
	boost::shared_ptr<basic_jump> x( new basic_jump );

	simulator<expectation> sim;
	sim.add_fix(0);
	sim.add_fix(365);
	sim.add_fix(3652);
	sim.set_step(1000);
	sim.set_samples(100000);

	// low intensity jump
	boost::shared_ptr<constant<double> > low( new constant<double>(0.01) );
	x->setup(g,low);
	sim.simulate(x);
	double e = sim.observer(1).value();
	BOOST_CHECK_CLOSE(0.01,e,3);

	// intermediate intensity jump
	boost::shared_ptr<constant<double> > mid( new constant<double>(1.0) );
	x->setup(g,mid);
	sim.simulate(x);
	e = sim.observer(1).value();
	BOOST_CHECK_CLOSE(1.0,e,1);

	// high intensity jump
	boost::shared_ptr<constant<double> > high( new constant<double>(10.0) );
	x->setup(g,high);
	sim.simulate(x);
	e = sim.observer(1).value();
	BOOST_CHECK_CLOSE(10.0,e,1);
}


