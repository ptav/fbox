/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Test instrument agents
*/


#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include "../state_adaptor.h"
#include "../agent_vector.h"
#include "../yield_curve_models.h"
#include "../instruments.h"
#include "../coupon_instruments.h"
#include "../simulator.h"
#include "../observer.h"


using namespace fbox::simulate;

boost::shared_ptr<fbox::math::linear_line> get_df()
{
	double rf = 0.05;
	boost::shared_ptr<fbox::math::linear_line> df( new fbox::math::linear_line );
	for(double t = 0.0; t < 3651.0; t += 10.0) df->add(t,std::pow(1.0/(1+rf),t/365.0));

	return df;
}


BOOST_AUTO_TEST_CASE(test_fixed_leg)
{
	boost::shared_ptr<fbox::math::linear_line> df = get_df();
	BOOST_CHECK(df);

	boost::shared_ptr<static_yield_curve> yc( new static_yield_curve );
	BOOST_CHECK(yc);
	yc->setup(df);

	boost::shared_ptr<instruments::fixed_leg> leg( new instruments::fixed_leg );
	BOOST_CHECK(leg);
	leg->setup(yc,0,3650,365,5.0,100.0);

	simulator<statistics> sim;
	sim.add_fix(0);
	sim.add_fix(180);
	sim.add_fix(365);
	sim.add_fix(545);
	sim.add_fix(730);
	sim.set_samples(10);

	typedef state_adaptor<instruments::fixed_leg,double> adaptor;
	boost::shared_ptr<adaptor> f( new adaptor(leg,leg->state().flow) );
	boost::shared_ptr<adaptor> v( new adaptor(leg,leg->state().value) );
	BOOST_CHECK(f);
	BOOST_CHECK(v);

	sim.simulate(f);
	BOOST_CHECK_SMALL(sim.observer(0).expectation(),1e-10);
	BOOST_CHECK_SMALL(sim.observer(1).expectation(),1e-10);
	BOOST_CHECK_CLOSE(sim.observer(2).expectation(),5.0,1e-10);
	BOOST_CHECK_SMALL(sim.observer(3).expectation(),1e-10);
	BOOST_CHECK_CLOSE(sim.observer(4).expectation(),5.0,1e-10);

	sim.simulate(v);
	BOOST_CHECK_CLOSE(sim.observer(0).expectation(),100.0,1e-4);
	BOOST_CHECK_CLOSE(sim.observer(2).expectation(),100.0,1e-4);
	BOOST_CHECK_CLOSE(sim.observer(4).expectation(),100.0,1e-4);
}


BOOST_AUTO_TEST_CASE(test_fixed_payment)
{
	boost::shared_ptr<fbox::math::linear_line> df = get_df();

	boost::shared_ptr<static_yield_curve> yc( new static_yield_curve );
	yc->setup(df);

	boost::shared_ptr<instruments::fixed_payment> pay( new instruments::fixed_payment );
	pay->setup(yc,365,100.0);

	simulator<statistics> sim;
	sim.add_fix(0);
	sim.add_fix(180);
	sim.add_fix(365);
	sim.add_fix(730);
	sim.set_samples(10);

	typedef state_adaptor<instruments::fixed_payment,double> adaptor;
	boost::shared_ptr<adaptor> f( new adaptor(pay,pay->state().flow) );
	boost::shared_ptr<adaptor> v( new adaptor(pay,pay->state().value) );

	sim.simulate(f);
	BOOST_CHECK_SMALL(sim.observer(0).expectation(),1e-10);
	BOOST_CHECK_SMALL(sim.observer(1).expectation(),1e-10);
	BOOST_CHECK_CLOSE(sim.observer(2).expectation(),100.0,1e-10);
	BOOST_CHECK_SMALL(sim.observer(3).expectation(),1e-10);

	sim.simulate(v);
	BOOST_CHECK_CLOSE(sim.observer(0).expectation(),100.0*(*df)(365),1e-4);
	BOOST_CHECK_CLOSE(sim.observer(1).expectation(),100.0*(*df)(365)/(*df)(180),1e-4);
	BOOST_CHECK_SMALL(sim.observer(2).expectation(),1e-10);
}


BOOST_AUTO_TEST_CASE(test_vanilla_float_coupon)
{
	boost::shared_ptr<fbox::math::linear_line> df = get_df();

	boost::shared_ptr<static_yield_curve> yc( new static_yield_curve );
	yc->setup(df);

	//BOOST_TEST_MESSAGE("Test1");

	boost::shared_ptr<instruments::vanilla_float_coupon> inst( new instruments::vanilla_float_coupon );
	inst->setup(yc,365,365,730,730,100,1.0);

	simulator<statistics> sim;
	sim.add_fix(0);
	sim.add_fix(180);
	sim.add_fix(365);
	sim.add_fix(545);
	sim.add_fix(730);
	sim.set_samples(10);

	typedef state_adaptor<instruments::vanilla_float_coupon,double> adaptor;
	boost::shared_ptr<adaptor> f( new adaptor(inst,inst->state().flow) );
	boost::shared_ptr<adaptor> v( new adaptor(inst,inst->state().value) );

	sim.simulate(f);
	double r = 100.*( (*df)(365)/(*df)(730)-1. );
	BOOST_CHECK_SMALL(sim.observer(0).expectation(),1e-10);
	BOOST_CHECK_SMALL(sim.observer(1).expectation(),1e-10);
	BOOST_CHECK_SMALL(sim.observer(2).expectation(),1e-10);
	BOOST_CHECK_SMALL(sim.observer(3).expectation(),1e-10);
	BOOST_CHECK_CLOSE(sim.observer(4).expectation(),r,1e-6);

	sim.simulate(v);
	BOOST_CHECK_CLOSE(sim.observer(0).expectation(),r*(*df)(730),1e-5);
	BOOST_CHECK_CLOSE(sim.observer(1).expectation(),r*(*df)(730)/(*df)(180),1e-5);
	BOOST_CHECK_CLOSE(sim.observer(2).expectation(),r*(*df)(730)/(*df)(365),1e-5);
	BOOST_CHECK_CLOSE(sim.observer(3).expectation(),r*(*df)(730)/(*df)(545),1e-5);
	BOOST_CHECK_SMALL(sim.observer(4).expectation(),1e-10);
}


BOOST_AUTO_TEST_CASE(test_advance_fix_float_coupon)
{
	boost::shared_ptr<fbox::math::linear_line> df = get_df();

	boost::shared_ptr<gaussian_variate> rnd( new gaussian_variate );
	boost::shared_ptr<hw_yield_curve> yc( new hw_yield_curve );
	yc->setup(rnd,df,.01,.05);

	boost::shared_ptr<instruments::vanilla_float_coupon> inst( new instruments::vanilla_float_coupon );
	inst->setup(yc,0,365,730,730,100,1.0);

	simulator<statistics> sim;
	sim.add_fix(0);
	sim.add_fix(180);
	sim.add_fix(365);
	sim.add_fix(545);
	sim.add_fix(730);
	sim.set_samples(10);

	typedef state_adaptor<instruments::vanilla_float_coupon,double> adaptor;
	boost::shared_ptr<adaptor> f( new adaptor(inst,inst->state().flow) );

	sim.simulate(f);
	double r = 100.*( (*df)(365)/(*df)(730)-1. );
	BOOST_CHECK_SMALL(sim.observer(0).expectation(),1e-10);
	BOOST_CHECK_SMALL(sim.observer(1).expectation(),1e-10);
	BOOST_CHECK_SMALL(sim.observer(2).expectation(),1e-10);
	BOOST_CHECK_SMALL(sim.observer(3).expectation(),1e-10);
	BOOST_CHECK_CLOSE(sim.observer(4).expectation(),r,1e-6);

	inst->setup(yc,0,180,545,545,100,1.0);
	sim.simulate(f);
	r = 100.*( (*df)(180)/(*df)(545)-1. );
	BOOST_CHECK_SMALL(sim.observer(0).expectation(),1e-10);
	BOOST_CHECK_SMALL(sim.observer(1).expectation(),1e-10);
	BOOST_CHECK_SMALL(sim.observer(2).expectation(),1e-10);
	BOOST_CHECK_CLOSE(sim.observer(3).expectation(),r,1e-6);
	BOOST_CHECK_SMALL(sim.observer(4).expectation(),1e-10);

	inst->setup(yc,0,180,545,730,100,1.0);
	sim.simulate(f);
	BOOST_CHECK_SMALL(sim.observer(0).expectation(),1e-10);
	BOOST_CHECK_SMALL(sim.observer(1).expectation(),1e-10);
	BOOST_CHECK_SMALL(sim.observer(2).expectation(),1e-10);
	BOOST_CHECK_SMALL(sim.observer(3).expectation(),1e-10);
	BOOST_CHECK_CLOSE(sim.observer(4).expectation(),r,1e-6);
}



BOOST_AUTO_TEST_CASE(test_float_coupon)
{
	boost::shared_ptr<fbox::math::linear_line> df = get_df();

	boost::shared_ptr<static_yield_curve> yc( new static_yield_curve );
	yc->setup(df);

	double r1 = .05;
	boost::shared_ptr<constant<double> > rate1( new constant<double>(r1) );

	boost::shared_ptr<instruments::float_coupon> inst( new instruments::float_coupon );
	inst->setup(yc,365,365,730,730,rate1,100,1.0);

	simulator<statistics> sim;
	sim.add_fix(0);
	sim.add_fix(180);
	sim.add_fix(365);
	sim.add_fix(545);
	sim.add_fix(730);
	sim.set_samples(10);

	typedef state_adaptor<instruments::float_coupon,double> adaptor;
	boost::shared_ptr<adaptor> f( new adaptor(inst,inst->state().flow) );
	boost::shared_ptr<adaptor> v( new adaptor(inst,inst->state().value) );

	sim.simulate(f);
	BOOST_CHECK_SMALL(sim.observer(0).expectation(),1e-10);
	BOOST_CHECK_SMALL(sim.observer(1).expectation(),1e-10);
	BOOST_CHECK_SMALL(sim.observer(2).expectation(),1e-10);
	BOOST_CHECK_SMALL(sim.observer(3).expectation(),1e-10);
	BOOST_CHECK_CLOSE(sim.observer(4).expectation(),100.*r1,1e-6);

	sim.simulate(v);
	BOOST_CHECK_CLOSE(sim.observer(0).expectation(),100.*r1*(*df)(730),1e-5);
	BOOST_CHECK_CLOSE(sim.observer(1).expectation(),100.*r1*(*df)(730)/(*df)(180),1e-5);
	BOOST_CHECK_CLOSE(sim.observer(2).expectation(),100.*r1*(*df)(730)/(*df)(365),1e-5);
	BOOST_CHECK_CLOSE(sim.observer(3).expectation(),100.*r1*(*df)(730)/(*df)(545),1e-5);
	BOOST_CHECK_SMALL(sim.observer(4).expectation(),1e-10);

	// test fixing of coupon
	boost::shared_ptr<fbox::simulate::time> rate2( new fbox::simulate::time );
	inst->setup(yc,365,365,730,730,rate2,100,1.0);

	sim.simulate(f);
	BOOST_CHECK_SMALL(sim.observer(0).expectation(),1e-10);
	BOOST_CHECK_SMALL(sim.observer(1).expectation(),1e-10);
	BOOST_CHECK_SMALL(sim.observer(2).expectation(),1e-10);
	BOOST_CHECK_SMALL(sim.observer(3).expectation(),1e-10);
	BOOST_CHECK_CLOSE(sim.observer(4).expectation(),100.*365.,1e-6);
}



BOOST_AUTO_TEST_CASE(test_forward)
{
	boost::shared_ptr<fbox::math::linear_line> df = get_df();

	boost::shared_ptr<static_yield_curve> yc( new static_yield_curve );
	yc->setup(df);

	boost::shared_ptr<constant<double> > vol( new constant<double>(0.20) );
	boost::shared_ptr<constant<double> > sec( new constant<double>(2.00) );
	boost::shared_ptr<constant<double> > k( new constant<double>(1.00) );
	boost::shared_ptr<constant<double> > n( new constant<double>(1.00) );

	boost::shared_ptr<instruments::forward> inst( new instruments::forward );
	inst->setup(yc,sec,n,k,0,365);

	simulator<statistics> sim;
	sim.add_fix(0);
	sim.add_fix(180);
	sim.add_fix(365);
	sim.add_fix(545);
	sim.add_fix(730);
	sim.set_samples(10);

	typedef state_adaptor<instruments::forward,double> adaptor;
	boost::shared_ptr<adaptor> f( new adaptor(inst,inst->state().flow) );
	boost::shared_ptr<adaptor> v( new adaptor(inst,inst->state().value) );

	sim.simulate(f);
	BOOST_CHECK_SMALL(sim.observer(0).expectation(),1e-10);
	BOOST_CHECK_SMALL(sim.observer(1).expectation(),1e-10);
	BOOST_CHECK_CLOSE(sim.observer(2).expectation(),1.0,1e-10);
	BOOST_CHECK_SMALL(sim.observer(3).expectation(),1e-10);
	BOOST_CHECK_SMALL(sim.observer(4).expectation(),1e-10);

	sim.simulate(v);
	BOOST_CHECK_CLOSE(sim.observer(0).expectation(),1.0*(*df)(365),1e-4);
	BOOST_CHECK_CLOSE(sim.observer(1).expectation(),1.0*(*df)(365)/(*df)(180),1e-4);
	BOOST_CHECK_SMALL(sim.observer(2).expectation(),1e-10);
	BOOST_CHECK_SMALL(sim.observer(3).expectation(),1e-10);
	BOOST_CHECK_SMALL(sim.observer(4).expectation(),1e-10);
}


BOOST_AUTO_TEST_CASE(test_option)
{
	boost::shared_ptr<fbox::math::linear_line> df = get_df();

	boost::shared_ptr<static_yield_curve> yc( new static_yield_curve );
	yc->setup(df);

	boost::shared_ptr<constant<double> > vol( new constant<double>(0.20) );
	boost::shared_ptr<constant<double> > sec( new constant<double>(2.00) );
	boost::shared_ptr<constant<double> > k( new constant<double>(1.00) );
	boost::shared_ptr<constant<double> > n( new constant<double>(1.00) );

	boost::shared_ptr<instruments::option> inst( new instruments::option );
	inst->setup(yc,vol,sec,n,'c',k,0,365);

	simulator<statistics> sim;
	sim.add_fix(0);
	sim.add_fix(180);
	sim.add_fix(365);
	sim.add_fix(545);
	sim.add_fix(730);
	sim.set_samples(10);

	typedef state_adaptor<instruments::option,double> adaptor;
	boost::shared_ptr<adaptor> f( new adaptor(inst,inst->state().flow) );
	boost::shared_ptr<adaptor> v( new adaptor(inst,inst->state().value) );

	sim.simulate(f);
	BOOST_CHECK_SMALL(sim.observer(0).expectation(),1e-10);
	BOOST_CHECK_SMALL(sim.observer(1).expectation(),1e-10);
	BOOST_CHECK_CLOSE(sim.observer(2).expectation(),1.0,1e-10);
	BOOST_CHECK_SMALL(sim.observer(3).expectation(),1e-10);
	BOOST_CHECK_SMALL(sim.observer(4).expectation(),1e-10);

	//sim.simulate(v);
	//BOOST_CHECK_CLOSE(sim.observer(0).expectation(),100.0,1e-4);
	//BOOST_CHECK_CLOSE(sim.observer(2).expectation(),100.0,1e-4);
	//BOOST_CHECK_CLOSE(sim.observer(4).expectation(),100.0,1e-4);

	sec->setup(0.0);

	sim.simulate(f);
	BOOST_CHECK_SMALL(sim.observer(0).expectation(),1e-10);
	BOOST_CHECK_SMALL(sim.observer(1).expectation(),1e-10);
	BOOST_CHECK_SMALL(sim.observer(2).expectation(),1e-10);
	BOOST_CHECK_SMALL(sim.observer(3).expectation(),1e-10);
	BOOST_CHECK_SMALL(sim.observer(4).expectation(),1e-10);
}


BOOST_AUTO_TEST_CASE(test_cash_synchronous)
{
	shared_ptr<flows> leg( new flows );
	//leg->add(0,1.0);
	leg->add(365,1.0);
	leg->add(600,1.0);
	leg->add(700,1.0);

	shared_ptr<constant<double> > rate( new constant<double>(0.0) );
	shared_ptr<instruments::cash> cash( new instruments::cash );
	cash->setup(rate,0.0);
	cash->connect_flow(leg);

	typedef state_adaptor<instruments::cash,double> adaptor;
	shared_ptr<adaptor> value( new adaptor(cash,cash->state().value) );
	shared_ptr<adaptor> flows( new adaptor(cash,cash->state().flow) );

	boost::shared_ptr<agent_vector<double> > av( new agent_vector<double> );
	av->connect(value);
	av->connect(flows);

	observer_vector<fbox::simulate::statistics> ob;
	ob.resize(2);
	ob[0].set_agent(value);
	ob[1].set_agent(flows);


	simulator<observer_vector<fbox::simulate::statistics> > sim;
	sim.add_fix(0,ob);
	sim.add_fix(180,ob);
	sim.add_fix(365,ob);
	sim.add_fix(545,ob);
	sim.add_fix(730,ob);
	sim.add_fix(1000,ob);
	sim.set_samples(1);

	sim.simulate(av);

	BOOST_CHECK_CLOSE(sim.observer(0)[0].expectation(),0.0,1e-10);
	BOOST_CHECK_CLOSE(sim.observer(1)[0].expectation(),0.0,1e-10);
	BOOST_CHECK_CLOSE(sim.observer(2)[0].expectation(),1.0,1e-10);
	BOOST_CHECK_CLOSE(sim.observer(3)[0].expectation(),1.0,1e-10);
	BOOST_CHECK_CLOSE(sim.observer(4)[0].expectation(),3.0,1e-10);
	BOOST_CHECK_CLOSE(sim.observer(5)[0].expectation(),3.0,1e-10);

	BOOST_CHECK_CLOSE(sim.observer(0)[1].expectation(),0.0,1e-10);
	BOOST_CHECK_CLOSE(sim.observer(1)[1].expectation(),0.0,1e-10);
	BOOST_CHECK_CLOSE(sim.observer(2)[1].expectation(),-1.0,1e-10);
	BOOST_CHECK_CLOSE(sim.observer(3)[1].expectation(),0.0,1e-10);
	BOOST_CHECK_CLOSE(sim.observer(4)[1].expectation(),-2.0,1e-10);
	BOOST_CHECK_CLOSE(sim.observer(5)[1].expectation(),0.0,1e-10);
}


BOOST_AUTO_TEST_CASE(test_cash_asynchronous)
{
	shared_ptr<flows> leg( new flows );
	//leg->add(0,1.0);
	leg->add(365,1.0);
	leg->add(600,1.0);
	leg->add(700,1.0);

	shared_ptr<constant<double> > rate( new constant<double>(0.0) );
	shared_ptr<instruments::cash> cash( new instruments::cash );
	cash->setup(rate,0.0);

	typedef state_adaptor<instruments::cash,double> adaptor;
	shared_ptr<adaptor> value( new adaptor(cash,cash->state().value) );
	shared_ptr<adaptor> flows( new adaptor(cash,cash->state().flow) );

	shared_ptr<instruments::cash::flow_connector> adp( new instruments::cash::flow_connector );
	adp->setup(cash);
	adp->connect(leg);

	boost::shared_ptr<agent_vector<double> > av( new agent_vector<double> );
	av->connect(value);
	av->connect(flows);
	av->connect(adp);

	observer_vector<fbox::simulate::statistics> ob;
	ob.resize(3);
	ob[0].set_agent(value);
	ob[1].set_agent(flows);
	ob[2].set_agent(adp);

	simulator<observer_vector<fbox::simulate::statistics> > sim;
	sim.add_fix(0,ob);
	sim.add_fix(180,ob);
	sim.add_fix(365,ob);
	sim.add_fix(545,ob);
	sim.add_fix(730,ob);
	sim.add_fix(1000,ob);
	sim.set_samples(1);

	sim.simulate(av);

	BOOST_CHECK_CLOSE(sim.observer(0)[0].expectation(),0.0,1e-10);
	BOOST_CHECK_CLOSE(sim.observer(1)[0].expectation(),0.0,1e-10);
	BOOST_CHECK_CLOSE(sim.observer(2)[0].expectation(),1.0,1e-10);
	BOOST_CHECK_CLOSE(sim.observer(3)[0].expectation(),1.0,1e-10);
	BOOST_CHECK_CLOSE(sim.observer(4)[0].expectation(),3.0,1e-10);
	BOOST_CHECK_CLOSE(sim.observer(5)[0].expectation(),3.0,1e-10);

	BOOST_CHECK_CLOSE(sim.observer(0)[1].expectation(),0.0,1e-10);
	BOOST_CHECK_CLOSE(sim.observer(1)[1].expectation(),0.0,1e-10);
	BOOST_CHECK_CLOSE(sim.observer(2)[1].expectation(),-1.0,1e-10);
	BOOST_CHECK_CLOSE(sim.observer(3)[1].expectation(),0.0,1e-10);
	BOOST_CHECK_CLOSE(sim.observer(4)[1].expectation(),-2.0,1e-10);
	BOOST_CHECK_CLOSE(sim.observer(5)[1].expectation(),0.0,1e-10);
}


BOOST_AUTO_TEST_CASE(test_cash_sync_and_async)
{
	shared_ptr<flows> leg( new flows );
	leg->set_schedule(0,730,365,0.0,1.0);

	shared_ptr<constant<double> > rate( new constant<double>(0.0) );
	shared_ptr<instruments::cash> cash( new instruments::cash );
	cash->setup(rate,0.0);
	cash->connect_flow(leg);

	typedef state_adaptor<instruments::cash,double> adaptor;
	shared_ptr<adaptor> value( new adaptor(cash,cash->state().value) );
	shared_ptr<adaptor> flows( new adaptor(cash,cash->state().flow) );

	shared_ptr<instruments::cash::flow_connector> adp( new instruments::cash::flow_connector );
	adp->setup(cash);
	adp->connect(leg);

	boost::shared_ptr<agent_vector<double> > av( new agent_vector<double> );
	av->connect(value);
	av->connect(flows);
	av->connect(adp);

	observer_vector<fbox::simulate::statistics> ob;
	ob.resize(3);
	ob[0].set_agent(value);
	ob[1].set_agent(flows);
	ob[2].set_agent(adp);


	simulator<observer_vector<fbox::simulate::statistics> > sim;
	sim.add_fix(0,ob);
	sim.add_fix(180,ob);
	sim.add_fix(365,ob);
	sim.add_fix(545,ob);
	sim.add_fix(730,ob);
	sim.add_fix(1000,ob);
	sim.set_samples(1);

	sim.simulate(av);

	BOOST_CHECK_CLOSE(sim.observer(0)[0].expectation(),0.0,1e-10);
	BOOST_CHECK_CLOSE(sim.observer(1)[0].expectation(),0.0,1e-10);
	BOOST_CHECK_CLOSE(sim.observer(2)[0].expectation(),2.0,1e-10);
	BOOST_CHECK_CLOSE(sim.observer(3)[0].expectation(),2.0,1e-10);
	BOOST_CHECK_CLOSE(sim.observer(4)[0].expectation(),4.0,1e-10);
	BOOST_CHECK_CLOSE(sim.observer(5)[0].expectation(),4.0,1e-10);

	BOOST_CHECK_CLOSE(sim.observer(0)[1].expectation(),0.0,1e-10);
	BOOST_CHECK_CLOSE(sim.observer(1)[1].expectation(),0.0,1e-10);
	BOOST_CHECK_CLOSE(sim.observer(2)[1].expectation(),-2.0,1e-10);
	BOOST_CHECK_CLOSE(sim.observer(3)[1].expectation(),0.0,1e-10);
	BOOST_CHECK_CLOSE(sim.observer(4)[1].expectation(),-2.0,1e-10);
	BOOST_CHECK_CLOSE(sim.observer(5)[1].expectation(),0.0,1e-10);
}


BOOST_AUTO_TEST_CASE(test_cash_interest)
{
	shared_ptr<constant<double> > rate( new constant<double>(0.10) );
	shared_ptr<instruments::cash> cash( new instruments::cash );
	cash->setup(rate,1.0);

	shared_ptr<instruments::cash::flow_connector> adp( new instruments::cash::flow_connector );
	adp->setup(cash);

	typedef state_adaptor<instruments::cash,double> adaptor;
	shared_ptr<adaptor> value( new adaptor(cash,cash->state().value) );

	simulator<statistics> sim;
	sim.add_fix(0);
	sim.add_fix(180);
	sim.add_fix(365);
	sim.add_fix(545);
	sim.add_fix(730);
	sim.set_samples(100);

	sim.simulate(value);

	BOOST_CHECK_CLOSE(sim.observer(0).expectation(),1.0,1e-6);
	BOOST_CHECK_CLOSE(sim.observer(1).expectation(),exp(0.1*(180./365.)),1e-6);
	BOOST_CHECK_CLOSE(sim.observer(2).expectation(),exp(0.1*(365./365.)),1e-6);
	BOOST_CHECK_CLOSE(sim.observer(3).expectation(),exp(0.1*(545./365.)),1e-6);
	BOOST_CHECK_CLOSE(sim.observer(4).expectation(),exp(0.1*(730./365.)),1e-6);
}
