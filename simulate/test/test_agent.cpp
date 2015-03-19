/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Test agent based simulation framework
*/


#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <fbox/math.h>
#include "../simulator.h"
#include "../observer.h"
#include "../agent_impl.h"
#include "../basic_agents.h"
#include "../memory.h"


using namespace fbox::simulate;

class agent1 : public standalone_agent_impl<double>
{
public:
	agent1(double _a) : m_a(_a) {}

protected:
	virtual void init_impl() { m_state = m_a; }
	virtual bool update_impl() { return true; }

private:
	double m_a;
};

class agent2 : public multi_agent_impl<double>
{
protected:
	virtual bool update_impl()
	{
		m_state = 0.0;

		linked_policy::iterator itr = linked().begin();
		linked_policy::iterator end = linked().end();
		for(; itr != end; ++itr) m_state += time();
		
		return true;
	}
};


BOOST_AUTO_TEST_CASE(test_agent)
{
	boost::shared_ptr<agent1> p1(new agent1(1.0));
	boost::shared_ptr<agent1> p2(new agent1(2.0));

	boost::shared_ptr<agent2> p3(new agent2);
	p3->linked().connect(p1);
	p3->linked().connect(p2);

	BOOST_CHECK_EQUAL(p1->state(),0.0);
	BOOST_CHECK_EQUAL(p2->state(),0.0);
	BOOST_CHECK_EQUAL(p3->state(),0.0);

	p3->init(0,10,0);
	BOOST_CHECK_EQUAL(p1->state(),1.0);
	BOOST_CHECK_EQUAL(p2->state(),2.0);
	BOOST_CHECK_EQUAL(p3->state(),0.0);

	p3->reset();
	BOOST_CHECK_EQUAL(p1->state(),1.0);
	BOOST_CHECK_EQUAL(p2->state(),2.0);
	BOOST_CHECK_EQUAL(p3->state(),0.0);

	p3->update(1);
	BOOST_CHECK_EQUAL(p1->state(),1.0);
	BOOST_CHECK_EQUAL(p2->state(),2.0);
	BOOST_CHECK_EQUAL(p3->state(),2.0);

	p3->reset();
	BOOST_CHECK_EQUAL(p1->state(),1.0);
	BOOST_CHECK_EQUAL(p2->state(),2.0);
	BOOST_CHECK_EQUAL(p3->state(),0.0);

	p3->update(10);
	BOOST_CHECK_EQUAL(p1->state(),1.0);
	BOOST_CHECK_EQUAL(p2->state(),2.0);
	BOOST_CHECK_EQUAL(p3->state(),20.0);
}


BOOST_AUTO_TEST_CASE(test_time)
{
	boost::shared_ptr<fbox::simulate::time> t(new fbox::simulate::time);

	t->init(0,10,0);
	BOOST_CHECK_EQUAL(t->state(),0.0);

	t->reset();
	BOOST_CHECK_EQUAL(t->state(),0.0);

	t->update(1);
	BOOST_CHECK_EQUAL(t->state(),1.0);

	t->update(3);
	BOOST_CHECK_EQUAL(t->state(),3.0);

	t->update(6);
	BOOST_CHECK_EQUAL(t->state(),6.0);

	t->update(10);
	BOOST_CHECK_EQUAL(t->state(),10.0);

	t->reset();
	BOOST_CHECK_EQUAL(t->state(),0.0);

	t->update(10);
	BOOST_CHECK_EQUAL(t->state(),10.0);
}
	

BOOST_AUTO_TEST_CASE(test_agent_vector)
{
	boost::shared_ptr<fbox::simulate::time> x(new fbox::simulate::time);
	boost::shared_ptr<constant<double> > y( new constant<double>(10.0) );

	boost::shared_ptr<agent_vector<double> > av( new agent_vector<double> );
	av->connect(x);
	av->connect(y);

	observer_vector<fbox::simulate::statistics> ob;
	ob.resize(2);
	ob[0].set_agent(x);
	ob[1].set_agent(y);
	
	
	simulator<observer_vector<fbox::simulate::statistics> > sim;
	sim.add_fix(0,ob);
	sim.add_fix(180,ob);
	sim.add_fix(365,ob);
	sim.add_fix(545,ob);
	sim.add_fix(730,ob);
	sim.set_samples(10);

	sim.simulate(av);

	BOOST_CHECK_CLOSE(sim.observer(0)[0].expectation(),0.0,1e-6);
	BOOST_CHECK_CLOSE(sim.observer(1)[0].expectation(),180.0,1e-6);
	BOOST_CHECK_CLOSE(sim.observer(2)[0].expectation(),365.0,1e-6);
	BOOST_CHECK_CLOSE(sim.observer(3)[0].expectation(),545.0,1e-6);

	BOOST_CHECK_CLOSE(sim.observer(0)[1].expectation(),10.0,1e-6);
	BOOST_CHECK_CLOSE(sim.observer(1)[1].expectation(),10.0,1e-6);
	BOOST_CHECK_CLOSE(sim.observer(2)[1].expectation(),10.0,1e-6);
	BOOST_CHECK_CLOSE(sim.observer(3)[1].expectation(),10.0,1e-6);
}
	

BOOST_AUTO_TEST_CASE(test_memory_standard)
{
	boost::shared_ptr< fbox::simulate::time> t(new fbox::simulate::time);
	boost::shared_ptr< memory<double> > r(new memory<double>);

	std::vector<int> times(1);
	times[0] = 5;
	r->setup(t,times,true,10.0);

	BOOST_CHECK_EQUAL(r->state(),10.0);

	r->init(0,10,0);
	BOOST_CHECK_EQUAL(t->state(),0.0);
	BOOST_CHECK_EQUAL(r->state(),10.0);

	r->reset();
	BOOST_CHECK_EQUAL(t->state(),0.0);
	BOOST_CHECK_EQUAL(r->state(),10.0);

	r->update(1);
	BOOST_CHECK_EQUAL(t->state(),1.0);
	BOOST_CHECK_EQUAL(r->state(),10.0);

	r->update(3);
	BOOST_CHECK_EQUAL(t->state(),3.0);
	BOOST_CHECK_EQUAL(r->state(),10.0);

	r->update(6);
	BOOST_CHECK_EQUAL(t->state(),6.0);
	BOOST_CHECK_EQUAL(r->state(),6.0);

	r->update(10);
	BOOST_CHECK_EQUAL(t->state(),6.0); // implementation stops updating dependent agents after recording
	BOOST_CHECK_EQUAL(r->state(),6.0);

	r->reset();
	BOOST_CHECK_EQUAL(t->state(),0.0);
	BOOST_CHECK_EQUAL(r->state(),10.0);

	r->update(10);
	BOOST_CHECK_EQUAL(t->state(),10.0);
	BOOST_CHECK_EQUAL(r->state(),10.0);

	// check time=0 memory
	times[0] = 0;
	r->setup(t,times);

	r->init(0,10,0);
	r->reset();
	BOOST_CHECK_EQUAL(t->state(),0.0);
	BOOST_CHECK_EQUAL(r->state(),0.0);

	r->update(1);
	BOOST_CHECK_EQUAL(t->state(),0.0);
	BOOST_CHECK_EQUAL(r->state(),0.0);

	r->update(5);
	BOOST_CHECK_EQUAL(t->state(),0.0); // implementation stops updating dependent agents after recording
	BOOST_CHECK_EQUAL(r->state(),0.0);

	r->reset();
	BOOST_CHECK_EQUAL(t->state(),0.0);
	BOOST_CHECK_EQUAL(r->state(),0.0);

	r->update(10);
	BOOST_CHECK_EQUAL(t->state(),0.0);
	BOOST_CHECK_EQUAL(r->state(),0.0);
}


BOOST_AUTO_TEST_CASE(test_memory_multiple_records)
{
	boost::shared_ptr< fbox::simulate::time> t(new fbox::simulate::time);
	boost::shared_ptr< memory<double> > r(new memory<double>);

	std::vector<int> times(2);
	times[0] = 5;
	times[1] = 10;
	r->setup(t,times);

	BOOST_CHECK_EQUAL(r->state(),0.0);

	r->init(0,10,0);
	BOOST_CHECK_EQUAL(t->state(),0.0);
	BOOST_CHECK_EQUAL(r->state(),0.0);

	r->reset();
	BOOST_CHECK_EQUAL(t->state(),0.0);
	BOOST_CHECK_EQUAL(r->state(),0.0);

	r->update(1);
	BOOST_CHECK_EQUAL(t->state(),1.0);
	BOOST_CHECK_EQUAL(r->state(),0.0);

	r->update(3);
	BOOST_CHECK_EQUAL(t->state(),3.0);
	BOOST_CHECK_EQUAL(r->state(),0.0);

	r->update(6);
	BOOST_CHECK_EQUAL(t->state(),6.0);
	BOOST_CHECK_EQUAL(r->state(),6.0);

	r->update(10);
	BOOST_CHECK_EQUAL(t->state(),10.0);
	BOOST_CHECK_EQUAL(r->state(),10.0);

	r->update(12);
	BOOST_CHECK_EQUAL(t->state(),10.0); // implementation stops updating dependent agents after recording
	BOOST_CHECK_EQUAL(r->state(),10.0);

	r->reset();
	BOOST_CHECK_EQUAL(t->state(),0.0);
	BOOST_CHECK_EQUAL(r->state(),0.0);

	r->update(10); // skipping first record time
	BOOST_CHECK_EQUAL(t->state(),10.0);
	BOOST_CHECK_EQUAL(r->state(),10.0);

	r->update(12);
	BOOST_CHECK_EQUAL(t->state(),10.0);
	BOOST_CHECK_EQUAL(r->state(),10.0);
}


BOOST_AUTO_TEST_CASE(test_memory_delayed_update)
{
	boost::shared_ptr<fbox::simulate::time> t(new fbox::simulate::time);
	boost::shared_ptr<memory<double> > r(new memory<double>);

	std::vector<int> times(1);
	times[0] = 5;
	r->setup(t,times,false);

	BOOST_CHECK_EQUAL(r->state(),0.0);

	r->init(0,10,0);
	BOOST_CHECK_EQUAL(t->state(),0.0);
	BOOST_CHECK_EQUAL(r->state(),0.0);

	r->reset();
	BOOST_CHECK_EQUAL(t->state(),0.0);
	BOOST_CHECK_EQUAL(r->state(),0.0);

	r->update(1);
	BOOST_CHECK_EQUAL(t->state(),0.0);
	BOOST_CHECK_EQUAL(r->state(),0.0);

	r->update(3);
	BOOST_CHECK_EQUAL(t->state(),0.0);
	BOOST_CHECK_EQUAL(r->state(),0.0);

	r->update(6);
	BOOST_CHECK_EQUAL(t->state(),6.0);
	BOOST_CHECK_EQUAL(r->state(),6.0);

	r->update(10);
	BOOST_CHECK_EQUAL(t->state(),6.0);
	BOOST_CHECK_EQUAL(r->state(),6.0);

	r->reset();
	BOOST_CHECK_EQUAL(t->state(),0.0);
	BOOST_CHECK_EQUAL(r->state(),0.0);

	r->update(10);
	BOOST_CHECK_EQUAL(t->state(),10.0);
	BOOST_CHECK_EQUAL(r->state(),10.0);
}


BOOST_AUTO_TEST_CASE(test_ramp_memory)
{
	boost::shared_ptr<fbox::simulate::time> t(new fbox::simulate::time);

	boost::shared_ptr<fbox::math::right_constant_line> line(new fbox::math::right_constant_line);
	line->add(0.0,0.0);
	line->add(3.0,1.0);
	line->add(6.0,2.0);

	boost::shared_ptr<curve> s(new curve);
	s->setup(line);

	boost::shared_ptr<ramp_memory<double,double> > r(new ramp_memory<double,double>);
	r->setup(t,s);

	BOOST_CHECK_EQUAL(r->state(),0.0);

	r->init(0,10,0);
	BOOST_CHECK_EQUAL(r->state(),0.0);

	r->reset();
	BOOST_CHECK_EQUAL(r->state(),0.0);

	r->update(1);
	BOOST_CHECK_EQUAL(r->state(),0.0);

	r->update(2);
	BOOST_CHECK_EQUAL(r->state(),0.0);

	r->update(3);
	BOOST_CHECK_EQUAL(r->state(),3.0);

	r->update(4);
	BOOST_CHECK_EQUAL(r->state(),3.0);

	r->update(6);
	BOOST_CHECK_EQUAL(r->state(),6.0);

	r->update(10);
	BOOST_CHECK_EQUAL(r->state(),6.0);

	r->reset();
	BOOST_CHECK_EQUAL(r->state(),0.0);

	r->update(10);
	BOOST_CHECK_EQUAL(r->state(),10.0);
}


BOOST_AUTO_TEST_CASE(test_lookback)
{
	boost::shared_ptr<fbox::simulate::time> t(new fbox::simulate::time);

	boost::shared_ptr< lookback<operators::queue_first,double> > r(new lookback<operators::queue_first,double>);
	r->setup(t,3);

	BOOST_CHECK_EQUAL(r->state(),0.0);

	r->init(0,10,0);
	BOOST_CHECK_EQUAL(r->state(),0.0);

	r->reset();
	BOOST_CHECK_EQUAL(r->state(),0.0);

	r->update(1);
	BOOST_CHECK_EQUAL(r->state(),0.0);

	r->update(2);
	BOOST_CHECK_EQUAL(r->state(),0.0);

	r->update(3);
	BOOST_CHECK_EQUAL(r->state(),1.0);

	r->update(4);
	BOOST_CHECK_EQUAL(r->state(),2.0);

	r->update(5);
	BOOST_CHECK_EQUAL(r->state(),3.0);

	r->update(7);
	BOOST_CHECK_EQUAL(r->state(),5.0);

	r->reset();
	BOOST_CHECK_EQUAL(r->state(),0.0);

	r->update(4);
	BOOST_CHECK_EQUAL(r->state(),4.0);
}


BOOST_AUTO_TEST_CASE(test_rolling)
{
	boost::shared_ptr<fbox::simulate::time> t(new fbox::simulate::time);

	boost::shared_ptr< lookback<operators::queue_sum,double> > s(new lookback<operators::queue_sum,double>);
	s->setup(t,3);

	boost::shared_ptr< lookback<operators::queue_mean,double> > m(new lookback<operators::queue_mean,double>);
	m->setup(t,3);

	boost::shared_ptr<binary_operator<operators::sum> > x( new binary_operator<operators::sum> );
	x->connect(s);
	x->connect(m);

	x->init(0,10,0);
	BOOST_CHECK_EQUAL(s->state(),0.0);
	BOOST_CHECK_EQUAL(m->state(),0.0);

	x->reset();
	BOOST_CHECK_EQUAL(s->state(),0.0);
	BOOST_CHECK_EQUAL(m->state(),0.0);

	x->update(1);
	BOOST_CHECK_EQUAL(s->state(),1.0);
	BOOST_CHECK_EQUAL(m->state(),0.5);

	x->update(2);
	BOOST_CHECK_EQUAL(s->state(),3.0);
	BOOST_CHECK_EQUAL(m->state(),1.0);

	x->update(3);
	BOOST_CHECK_EQUAL(s->state(),6.0);
	BOOST_CHECK_EQUAL(m->state(),2.0);

	x->update(4);
	BOOST_CHECK_EQUAL(s->state(),9.0);
	BOOST_CHECK_EQUAL(m->state(),3.0);

	x->update(5);
	BOOST_CHECK_EQUAL(s->state(),12.0);
	BOOST_CHECK_EQUAL(m->state(),4.0);

	x->update(7);
	BOOST_CHECK_EQUAL(s->state(),12.0);
	BOOST_CHECK_EQUAL(m->state(),6.0);

	x->reset();
	BOOST_CHECK_EQUAL(s->state(),0.0);
	BOOST_CHECK_EQUAL(m->state(),0.0);

	x->update(4);
	BOOST_CHECK_EQUAL(s->state(),4.0);
	BOOST_CHECK_EQUAL(m->state(),4.0);
}



class gearbox_test_agent: public fbox::simulate::standalone_agent_impl<double>
{
protected:
	virtual void init_impl() { m_state = 0.0; }
	virtual bool update_impl() { m_state += 1.0; return true; }
};


BOOST_AUTO_TEST_CASE(test_gearbox)
{
	boost::shared_ptr<gearbox_test_agent> a(new gearbox_test_agent);
	boost::shared_ptr<gearbox<double> > g(new gearbox<double>);
	g->setup(a,10);

	g->init(0,1000,0);
	BOOST_CHECK_EQUAL(a->state(),0.0);
	BOOST_CHECK_EQUAL(g->state(),0.0);

	g->reset();
	BOOST_CHECK_EQUAL(a->state(),0.0);
	BOOST_CHECK_EQUAL(g->state(),0.0);
	BOOST_CHECK_EQUAL(a->time(),0);
	BOOST_CHECK_EQUAL(g->time(),0);

	g->update(5);
	BOOST_CHECK_EQUAL(a->state(),1.0);
	BOOST_CHECK_EQUAL(g->state(),1.0);
	BOOST_CHECK_EQUAL(a->time(),5);
	BOOST_CHECK_EQUAL(g->time(),5);

	g->update(50);
	BOOST_CHECK_EQUAL(a->state(),5.0);
	BOOST_CHECK_EQUAL(g->state(),5.0);
	BOOST_CHECK_EQUAL(a->time(),50);
	BOOST_CHECK_EQUAL(g->time(),50);

	g->reset();
	BOOST_CHECK_EQUAL(a->state(),0.0);
	BOOST_CHECK_EQUAL(g->state(),0.0);
	BOOST_CHECK_EQUAL(a->time(),0);
	BOOST_CHECK_EQUAL(g->time(),0);

	g->update(100);
	BOOST_CHECK_EQUAL(a->state(),10.0);
	BOOST_CHECK_EQUAL(g->state(),10.0);
	BOOST_CHECK_EQUAL(a->time(),100);
	BOOST_CHECK_EQUAL(g->time(),100);
}
