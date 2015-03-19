/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Test model agents
*/


#include "../operators.h"
#include "../basic_agents.h"
#include "../simulator.h"
#include "../observer.h"
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>


using namespace fbox::simulate;


BOOST_AUTO_TEST_CASE(test_unary_ops)
{
	simulator<expectation> sim;
	sim.add_fix(0);
	sim.add_fix(365);
	sim.set_step(1000);
	sim.set_samples(50);
	
	boost::shared_ptr<constant<double> > x( new constant<double> );
	x->setup(2.0);

	boost::shared_ptr<unary_operator<operators::nop> > nop( new unary_operator<operators::nop> );
	BOOST_CHECK_EQUAL(nop->count_connected(),0);
	nop->connect(x);
	BOOST_CHECK_EQUAL(nop->count_connected(),1);
	sim.simulate(nop);
	BOOST_CHECK_CLOSE(2.0,sim.observer(0).value(),1e-6);
	BOOST_CHECK_CLOSE(2.0,sim.observer(1).value(),1e-6);

	boost::shared_ptr<unary_operator<operators::invert> > inv( new unary_operator<operators::invert> );
	inv->connect(x);
	sim.simulate(inv);
	BOOST_CHECK_CLOSE(0.5,sim.observer(0).value(),1e-6);
	BOOST_CHECK_CLOSE(0.5,sim.observer(1).value(),1e-6);

	boost::shared_ptr<unary_operator<operators::negate> > neg( new unary_operator<operators::negate> );
	neg->connect(x);
	sim.simulate(neg);
	BOOST_CHECK_CLOSE(-2.0,sim.observer(0).value(),1e-6);
	BOOST_CHECK_CLOSE(-2.0,sim.observer(1).value(),1e-6);

	boost::shared_ptr<unary_operator<operators::absolute_value> > abs( new unary_operator<operators::absolute_value> );
	abs->connect(x);
	sim.simulate(abs);
	BOOST_CHECK_CLOSE(2.0,sim.observer(0).value(),1e-6);
	BOOST_CHECK_CLOSE(2.0,sim.observer(1).value(),1e-6);
}



BOOST_AUTO_TEST_CASE(test_binary_ops)
{
	simulator<expectation> sim;
	sim.add_fix(0);
	sim.add_fix(365);
	sim.set_step(1000);
	sim.set_samples(50);

	boost::shared_ptr<constant<double> > x( new constant<double> );
	x->setup(3.0);

	boost::shared_ptr<constant<double> > y( new constant<double> );
	y->setup(2.0);

	boost::shared_ptr<binary_operator<operators::sum> > sum( new binary_operator<operators::sum> );
	BOOST_CHECK_EQUAL(sum->count_connected(),0);
	sum->connect(x);
	sum->connect(y);
	BOOST_CHECK_EQUAL(sum->count_connected(),2);
	sim.simulate(sum);
	BOOST_CHECK_CLOSE(5.0,sim.observer(0).value(),0.01);
	BOOST_CHECK_CLOSE(5.0,sim.observer(1).value(),0.01);

	boost::shared_ptr<binary_operator<operators::product> > prod( new binary_operator<operators::product> );
	prod->connect(x);
	prod->connect(y);
	sim.simulate(prod);
	BOOST_CHECK_CLOSE(6.0,sim.observer(0).value(),0.01);
	BOOST_CHECK_CLOSE(6.0,sim.observer(1).value(),0.01);
}


BOOST_AUTO_TEST_CASE(test_cumulative_ops)
{
	simulator<expectation> sim;
	sim.add_fix(0);
	sim.add_fix(180);
	sim.add_fix(365);
	sim.set_step(1000);
	sim.set_samples(50);

	boost::shared_ptr<fbox::simulate::time> t( new fbox::simulate::time );

	boost::shared_ptr<constant<double> > x( new constant<double> );
	x->setup(3.0);

	boost::shared_ptr<cumulative_operator<operators::sum> > sum( new cumulative_operator<operators::sum> );
	BOOST_CHECK_EQUAL(sum->count_connected(),0);

	sum->connect(x);
	BOOST_CHECK_EQUAL(sum->count_connected(),1);

	sim.simulate(sum);
	BOOST_CHECK_CLOSE(3.0,sim.observer(0).value(),1e-10);
	BOOST_CHECK_CLOSE(6.0,sim.observer(1).value(),1e-10);
	BOOST_CHECK_CLOSE(9.0,sim.observer(2).value(),1e-10);

	boost::shared_ptr<cumulative_operator<operators::product> > product( new cumulative_operator<operators::product> );
	product->connect(x);
	sim.simulate(product);
	BOOST_CHECK_CLOSE(3.0,sim.observer(0).value(),1e-10);
	BOOST_CHECK_CLOSE(9.0,sim.observer(1).value(),1e-10);
	BOOST_CHECK_CLOSE(27.0,sim.observer(2).value(),1e-10);

	boost::shared_ptr<cumulative_operator<operators::maximum> > max( new cumulative_operator<operators::maximum> );
	max->connect(t);
	sim.simulate(max);
	BOOST_CHECK_CLOSE(0.0,sim.observer(0).value(),1e-10);
	BOOST_CHECK_CLOSE(180.0,sim.observer(1).value(),1e-10);
	BOOST_CHECK_CLOSE(365.0,sim.observer(2).value(),1e-10);

	boost::shared_ptr<cumulative_operator<operators::minimum> > min( new cumulative_operator<operators::minimum> );
	min->connect(t);
	sim.simulate(min);
	BOOST_CHECK_CLOSE(0.0,sim.observer(0).value(),1e-10);
	BOOST_CHECK_CLOSE(0.0,sim.observer(1).value(),1e-10);
	BOOST_CHECK_CLOSE(0.0,sim.observer(2).value(),1e-10);
}


BOOST_AUTO_TEST_CASE(test_sequential_ops)
{
	simulator<expectation> sim;
	sim.add_fix(0);
	sim.add_fix(180);
	sim.add_fix(365);
	sim.set_step(1000);
	sim.set_samples(50);

	boost::shared_ptr<constant<double> > x( new constant<double> );
	x->setup(3.0);

	boost::shared_ptr<sequential_operator<operators::sum> > sum( new sequential_operator<operators::sum> );
	BOOST_CHECK_EQUAL(sum->count_connected(),0);

	sum->connect(x);
	BOOST_CHECK_EQUAL(sum->count_connected(),1);

	sim.simulate(sum);
	BOOST_CHECK_CLOSE(6.0,sim.observer(0).value(),1e-10);
	BOOST_CHECK_CLOSE(6.0,sim.observer(1).value(),1e-10);
	BOOST_CHECK_CLOSE(6.0,sim.observer(2).value(),1e-10);

	boost::shared_ptr<sequential_operator<operators::difference> > diff( new sequential_operator<operators::difference> );
	diff->connect(x);
	sim.simulate(diff);
	BOOST_CHECK_CLOSE(0.0,sim.observer(0).value(),1e-10);
	BOOST_CHECK_CLOSE(0.0,sim.observer(1).value(),1e-10);
	BOOST_CHECK_CLOSE(0.0,sim.observer(2).value(),1e-10);

	diff->setup(x,0.0); // test operator with initial value (x-1)
	sim.simulate(diff);
	BOOST_CHECK_CLOSE(3.0,sim.observer(0).value(),1e-10);
	BOOST_CHECK_CLOSE(0.0,sim.observer(1).value(),1e-10);
	BOOST_CHECK_CLOSE(0.0,sim.observer(2).value(),1e-10);
}

