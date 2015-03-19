#ifndef __FBOX_SIMULATE_BASIC_AGENTS_H__
#define __FBOX_SIMULATE_BASIC_AGENTS_H__
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Basic simulation agents
*/

#include <queue>
#include <boost/math/distributions/normal.hpp>
#include <fbox/main.h>
#include <fbox/line.h>
#include <fbox/xml_utils.h>
#include "cached_agent.h"

namespace fbox {
namespace simulate {


//! Time process
class time
:	public standalone_agent_impl<double>
{
protected:
	virtual void reset_impl() { m_state = standalone_agent_impl<double>::time(); }
	virtual bool update_impl() { m_state = standalone_agent_impl<double>::time(); return true; }
};



//! Constant process (to ensure optimal efficiency we don't inherit from the agent_impl template)
/*!
	The agents is setup so that it is possible to change the value during a simulation (often it 
	is used as a counter). The agent will save the value when <code>init(...)</code> is called 
	and recover it each time <code>reset()</code> is called.
*/
template
<
	typename _state_type,
	typename _time_type = default_time_type
>
class constant
:	public basic_valued_agent<_state_type,_time_type>
{
public:
	typedef basic_valued_agent<_state_type,_time_type> parent_type;
	typedef typename parent_type::state_type state_type;
	typedef typename parent_type::time_type time_type;
	
	constant(const state_type& _s = state_type()) : m_state(_s) {}
	constant(const constant& _c) : m_state(_c.m_state),m_initial(_c.m_initial) {}

	void setup(const state_type& _value) { m_state = _value; }

	virtual const state_type& state() const { return m_state; }

	virtual bool is_live() const { return true; }
	virtual void init(const time_type& _start,const time_type& _end,generator* _random) { m_initial = m_state; }
	virtual void reset() { m_state = m_initial; }
	virtual void update(const time_type& _time) {}
	virtual void dump(std::ostream& _strm) const { 	fbox::xml::make_tag(_strm,fbox::type_id(*this,true)).attr("value",m_initial); }

protected:
	state_type m_state;
	state_type m_initial;
};



//! Static curve process
class curve
:	public standalone_cached_agent_impl<double>
{
public:
	void setup(shared_ptr<math::line> _line) { mp_line = _line; }

protected:
	shared_ptr<math::line> mp_line;
	virtual void reset_impl() { m_state = (*mp_line)(time()); }
	virtual bool update_impl() { m_state = (*mp_line)(time()); return true; }
};



//! The gearbox process (stores value of a process at a given point in time)
template
<
	typename _state_type,
	typename _time_type = default_time_type,
	typename _duration_type = _time_type
>
class gearbox
:	public standalone_agent_impl<_state_type,_time_type,_duration_type>
{
public:
	typedef standalone_agent_impl<_state_type,_time_type,_duration_type> parent_type;
	typedef typename parent_type::time_type time_type;
	typedef typename parent_type::state_type state_type;

	void setup(
		shared_ptr<basic_valued_agent<state_type,time_type> > _source,
		const _duration_type & _time_step)
	{
		mp_src = _source;
		m_step = _time_step;
	}

	virtual void dump(std::ostream& _strm) const
	{
		fbox::xml::make_tag(_strm,fbox::type_id(*this,true))
			.attr("time_step",m_step)
			.attr("source",mp_src);
	}

protected:
	shared_ptr<basic_valued_agent<state_type,time_type> > mp_src;
	_duration_type m_step;
	_time_type m_last;

	using parent_type::m_time;
	using parent_type::m_state;
	using parent_type::m_start;
	using parent_type::m_end;
	using parent_type::mp_rnd;

	virtual void init_impl()
	{
		mp_src->init(m_start,m_end,mp_rnd);
	}
	
	virtual void reset_impl()
	{
		mp_src->reset();
		m_last = m_start;
		m_state = mp_src->state();
	}
	
	virtual bool update_impl() 
	{
		for(m_last += m_step; m_last <= m_time; m_last += m_step) mp_src->update(m_last);
		mp_src->update(m_time);

		m_state = mp_src->state();

		return mp_src->is_live();
	}
};



//! Uniformly distributed random process in the interval [0,1]
class uniform_variate
:	public standalone_agent_impl<double>
{
protected:
	virtual bool update_impl() { m_state = rnd(); return true; }
};



//! Normal distrubution random variate
class gaussian_variate
:	public standalone_agent_impl<double>
{
protected:
	virtual bool update_impl() { m_state = boost::math::quantile(m_dist,rnd()); return true; }

private:
	boost::math::normal m_dist;
};


//! "Twist" the probability measure of Gaussian samples
class gaussian_twister
:	public single_agent_impl<double>
{
public:
	void setup(double_agent_ptr _agent,double _param);

protected:
	double_agent_ptr mp_agent;
	double m_param,m_param_sq;
	boost::math::normal m_dist;

	virtual void init_impl();
	virtual void reset_impl();
	virtual bool update_impl();
};





} // namespace simulate
} // namespace fbox

#endif
