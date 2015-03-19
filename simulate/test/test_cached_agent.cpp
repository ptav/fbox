/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Test cached agent implementation
*/


#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <fbox/line.h>
#include "../cached_agent.h"
#include "../basic_agents.h"


using namespace fbox::simulate;

class cached : public standalone_cached_agent_impl<double>
{
public:
	cached(int& _cb) : mr_cb(_cb) {}
	int callback() const { return mr_cb; }

protected:
	int& mr_cb;

	virtual void init_impl() { ++mr_cb; }
	virtual void reset_impl() { m_state = m_time; ++mr_cb; }
	virtual bool update_impl() { m_state = m_time; ++mr_cb; return true; }
};


BOOST_AUTO_TEST_CASE(test_cached_agent)
{
	int cb(0);
	boost::shared_ptr<cached> c( new cached(cb) );

	c->init(0,10,0);
	BOOST_CHECK_EQUAL(c->callback(),1);


	c->reset();
	BOOST_CHECK_EQUAL(c->state(),0.0);
	BOOST_CHECK_EQUAL(c->callback(),2);

	c->update(2);
	BOOST_CHECK_EQUAL(c->state(),2.0);
	BOOST_CHECK_EQUAL(c->callback(),3);


	c->reset();
	BOOST_CHECK_EQUAL(c->state(),0.0);
	BOOST_CHECK_EQUAL(c->callback(),3);

	c->update(1);
	BOOST_CHECK_EQUAL(c->state(),1.0);
	BOOST_CHECK_EQUAL(c->callback(),4);

	c->update(2);
	BOOST_CHECK_EQUAL(c->state(),2.0);
	BOOST_CHECK_EQUAL(c->callback(),4);

	c->update(4);
	BOOST_CHECK_EQUAL(c->state(),4.0);
	BOOST_CHECK_EQUAL(c->callback(),5);


	c->reset();
	BOOST_CHECK_EQUAL(c->state(),0.0);
	BOOST_CHECK_EQUAL(c->callback(),5);

	c->update(1);
	BOOST_CHECK_EQUAL(c->state(),1.0);
	BOOST_CHECK_EQUAL(c->callback(),5);

	c->update(2);
	BOOST_CHECK_EQUAL(c->state(),2.0);
	BOOST_CHECK_EQUAL(c->callback(),5);

	c->update(4);
	BOOST_CHECK_EQUAL(c->state(),4.0);
	BOOST_CHECK_EQUAL(c->callback(),5);

	c->update(6);
	BOOST_CHECK_EQUAL(c->state(),6.0);
	BOOST_CHECK_EQUAL(c->callback(),6);


	c->init(0,10,0);
	BOOST_CHECK_EQUAL(c->callback(),7);


	c->reset();
	BOOST_CHECK_EQUAL(c->state(),0.0);
	BOOST_CHECK_EQUAL(c->callback(),8);

	c->update(2);
	BOOST_CHECK_EQUAL(c->state(),2.0);
	BOOST_CHECK_EQUAL(c->callback(),9);

	c->update(5);
	BOOST_CHECK_EQUAL(c->state(),5.0);
	BOOST_CHECK_EQUAL(c->callback(),10);
}


BOOST_AUTO_TEST_CASE(test_curve1)
{
	shared_ptr<fbox::math::linear_line> l( new fbox::math::linear_line );
	l->add(0,1);
	l->add(10,0);
	l->add(20,3);

	boost::shared_ptr<curve> c(new curve);
	c->setup(l);


	c->init(0,50,0);
	c->reset();
	BOOST_CHECK_EQUAL(c->state(),1.0);

	c->update(2);
	BOOST_CHECK_CLOSE(c->state(),0.8,0.01);

	c->reset();
	BOOST_CHECK_EQUAL(c->state(),1);

	c->update(1);
	BOOST_CHECK_CLOSE(c->state(),0.9,0.01);

	c->update(2);
	BOOST_CHECK_CLOSE(c->state(),0.8,0.01);

	c->update(4);
	BOOST_CHECK_CLOSE(c->state(),0.6,0.01);


	c->reset();
	BOOST_CHECK_EQUAL(c->state(),1);

	c->update(1);
	BOOST_CHECK_CLOSE(c->state(),0.9,0.01);

	c->update(2);
	BOOST_CHECK_CLOSE(c->state(),0.8,0.01);

	c->update(4);
	BOOST_CHECK_CLOSE(c->state(),0.6,0.01);

	c->update(6);
	BOOST_CHECK_CLOSE(c->state(),0.4,0.01);


	c->init(0,10,0);
	c->reset();
	BOOST_CHECK_EQUAL(c->state(),1);

	c->update(2);
	BOOST_CHECK_CLOSE(c->state(),0.8,0.01);

	c->update(5);
	BOOST_CHECK_CLOSE(c->state(),0.5,0.01);
}


BOOST_AUTO_TEST_CASE(test_curve2)
{
	boost::shared_ptr<fbox::math::right_constant_line> line( new fbox::math::right_constant_line );
	line->add(0,0);
	line->add(10,1);
	line->add(20,0);

	boost::shared_ptr<curve> c( new curve );
	c->setup(line);

	c->init(0,100,0);
	c->reset();
	BOOST_CHECK_EQUAL(c->state(),0.0);

	c->update(5);
	BOOST_CHECK_EQUAL(c->state(),0.0);

	c->update(10);
	BOOST_CHECK_EQUAL(c->state(),1.0);

	c->update(15);
	BOOST_CHECK_EQUAL(c->state(),1.0);

	c->update(30);
	BOOST_CHECK_EQUAL(c->state(),0.0);

	c->reset();
	BOOST_CHECK_EQUAL(c->state(),0.0);

	c->update(100);
	BOOST_CHECK_EQUAL(c->state(),0.0);

	c->init(15,100,0);
	c->reset();
	BOOST_CHECK_EQUAL(c->state(),1.0);

	c->update(20);
	BOOST_CHECK_EQUAL(c->state(),0.0);

	c->reset();
	BOOST_CHECK_EQUAL(c->state(),1.0);
}
