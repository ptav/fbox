/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Test model agents
*/


#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include "../agent_vector.h"
#include "../basic_agents.h"
#include "../basic_pricing.h"
#include "../models.h"
#include "../simulator.h"
#include "../observer.h"


using namespace fbox::simulate;


BOOST_AUTO_TEST_CASE(test_flows)
{
	boost::shared_ptr<fbox::simulate::time> r( new fbox::simulate::time );
	
	boost::shared_ptr<flows> f( new flows );
	f->setup(r);
	f->add(30,40,90,93,1,0); // fix,start,end,pay,mult,margin,principal
	f->add(90,90,180,180,1,50,0.0,(180.-90.)/250.);
	f->add(180,180,210,210,2.0,50,100.0,(210.-180.)/250.);

	observer_vector<fbox::simulate::statistics> ob; // template observer to be used for each time fix
	ob.resize(2);
	ob[0].set_agent(r);
	ob[1].set_agent(f);

	boost::shared_ptr<agent_vector<double> > av( new agent_vector<double> );
	av->connect(r);
	av->connect(f);
	
	simulator<observer_vector<fbox::simulate::statistics> > sim;

	sim.add_fix(0,ob);
	sim.add_fix(30,ob);
	sim.add_fix(90,ob);
	sim.add_fix(93,ob);
	sim.add_fix(120,ob);
	sim.add_fix(150,ob);
	sim.add_fix(180,ob);
	sim.add_fix(210,ob);
	sim.add_fix(240,ob);

	sim.set_step(1000);
	sim.set_samples(10);

	sim.simulate(av);

	double r1 = sim.observer(1)[0].expectation() * (90-40)/365.;
	double r2 = (sim.observer(2)[0].expectation() + 50.0) * (180 - 90)/250.;
	double r3 = 100.0 + (sim.observer(6)[0].expectation() * 2.0 + 50.0) * (210 - 180)/250.;

	BOOST_CHECK_SMALL(sim.observer(0)[1].expectation(),1e-5);
	BOOST_CHECK_SMALL(sim.observer(1)[1].expectation(),1e-5);
	BOOST_CHECK_SMALL(sim.observer(2)[1].expectation(),1e-5);
	BOOST_CHECK_CLOSE(sim.observer(3)[1].expectation(),r1,0.001);
	BOOST_CHECK_SMALL(sim.observer(4)[1].expectation(),1e-5);
	BOOST_CHECK_SMALL(sim.observer(5)[1].expectation(),1e-5);
	BOOST_CHECK_CLOSE(sim.observer(6)[1].expectation(),r2,0.001);
	BOOST_CHECK_CLOSE(sim.observer(7)[1].expectation(),r3,0.001);
	BOOST_CHECK_SMALL(sim.observer(8)[1].expectation(),1e-5);
}


BOOST_AUTO_TEST_CASE(test_past_starting_flows)
{
	boost::shared_ptr<gaussian_variate> w( new gaussian_variate );
	boost::shared_ptr<constant<double> > r( new constant<double>(0.1) );
	boost::shared_ptr<flows> f( new flows );
	f->setup(r);
	f->add(-100,-100,-50,-50,-2,0);
	f->add(-50,-50,3,3,-1,0);
	f->add(3,3,90,93,1,0);
	f->add(90,90,180,180,1,.2);

	simulator<statistics> sim;
	sim.add_fix(0);
	sim.add_fix(30);
	sim.add_fix(90);
	sim.add_fix(93);
	sim.add_fix(120);
	sim.add_fix(150);
	sim.add_fix(180);
	sim.add_fix(210);
	sim.add_fix(240);
	sim.set_step(1000);
	sim.set_samples(10);

	sim.simulate(f);
	double e = sim.observer(1).expectation();
	double v = sim.observer(1).variance();

	BOOST_CHECK_SMALL(sim.observer(0).expectation(),1e-5);
	BOOST_CHECK_CLOSE(sim.observer(1).expectation(),-0.1 * (3+50)/365.,0.01);
	BOOST_CHECK_SMALL(sim.observer(2).expectation(),1e-5);
	BOOST_CHECK_CLOSE(sim.observer(3).expectation(),0.1 * (90-3)/365.,0.01);
	BOOST_CHECK_SMALL(sim.observer(4).expectation(),1e-5);
	BOOST_CHECK_SMALL(sim.observer(5).expectation(),1e-5);
	BOOST_CHECK_CLOSE(sim.observer(6).expectation(),0.3 * (180 - 90)/365.,0.01);
	BOOST_CHECK_SMALL(sim.observer(8).expectation(),1e-5);
}


