#ifndef __FBOX_SIMULATE_MEMORY_H__
#define __FBOX_SIMULATE_MEMORY_H__
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Agents that "remember" past states: memory, lookback, etc.
*/

#include <deque>
#include <boost/math/distributions/normal.hpp>
#include <fbox/main.h>
#include <fbox/xml_utils.h>
#include "operators.h"

namespace fbox {
namespace simulate {

//! The memory agent stores value of a process at a given points in time
template
<
	typename _state_type,
	typename _time_type = default_time_type,
	typename _duration_type = _time_type
>
class memory
:	public standalone_agent_impl<_state_type,_time_type,_duration_type>
{
public:
	typedef standalone_agent_impl<_state_type,_time_type,_duration_type> parent_type;
	typedef typename parent_type::time_type time_type;
	typedef typename parent_type::state_type state_type;
	typedef std::vector<time_type> times_vector;

	virtual ~memory() {}

	void setup(
		shared_ptr<basic_valued_agent<state_type,time_type> > _source, //!< Underlying agent
		const times_vector& _triggers, //!< Recording times. Must be sorted in ascending order
		bool _always_update = true, //!< True to update underlying process at every time step (before recording event)
		const state_type& _state = _state_type()) //!< Initial state
	{
		mp_src = _source;
		m_triggers = _triggers;
		m_always_update = _always_update;
		this->m_state = _state;
	}

	virtual void dump(std::ostream& _strm) const
	{
		fbox::xml::make_tag tag(_strm,fbox::type_id(*this,true));
		tag.attr("always_update",m_always_update);
		for(times_vector_iterator itr = m_triggers.begin();itr != m_triggers.end(); ++itr) 
			tag.attr("trigger",*itr);			
		tag.attr("source",mp_src);
	}

protected:
	typedef typename times_vector::const_iterator times_vector_iterator;

	times_vector m_triggers;
	times_vector_iterator m_trigger_itr,m_trigger_start;
	shared_ptr<basic_valued_agent<state_type,time_type> > mp_src;
	bool m_always_update;

	using parent_type::m_time;
	using parent_type::m_state;
	using parent_type::m_start;
	using parent_type::m_end;
	using parent_type::m_live;
	using parent_type::mp_rnd;

	virtual void init_impl()
	{
		mp_src->init(m_start,m_end,mp_rnd);

		m_trigger_start = m_triggers.begin();
		while (m_trigger_start != m_triggers.end() && m_start > *m_trigger_start) ++m_trigger_start;
	}
	
	virtual void reset_impl()
	{
		mp_src->reset();
		m_trigger_itr = m_trigger_start;
		m_live = update_impl(); // if trigger is crossed on reset we must turn m_live to false, otherwise won't disable future updates
	}
	
	virtual bool update_impl() 
	{
		if (m_trigger_itr == m_triggers.end()) return false;

		if (m_time >= *m_trigger_itr)
		{
			mp_src->update(m_time);
			m_state = mp_src->state();

			while (m_time >= *m_trigger_itr) 
			{
				++m_trigger_itr;
				if (m_trigger_itr == m_triggers.end()) return false;
			}	
		}
		
		if (m_always_update)
			mp_src->update(m_time);

		return true;
	}
};



//! This memory agent stores value of a process every time a second signal process increases in value
template
<
	typename _state_type,
	typename _signal_type,
	typename _time_type = default_time_type,
	typename _duration_type = _time_type
>
class ramp_memory
:	public standalone_agent_impl<_state_type,_time_type,_duration_type>
{
public:
	typedef _signal_type signal_type;

	typedef standalone_agent_impl<_state_type,_time_type,_duration_type> parent_type;
	typedef typename parent_type::time_type time_type;
	typedef typename parent_type::state_type state_type;

	virtual ~ramp_memory() {}

	void setup(
		shared_ptr<basic_valued_agent<state_type,time_type> > _source, //!< Underlying agent
		shared_ptr<basic_valued_agent<state_type,time_type> > _signal, //!< Signal agent
		bool _always_update = true, //!< True to update underlying process at every time step between recordings
		const state_type& _state = _state_type()) //!< Initial state
	{
		mp_src = _source;
		mp_sig = _signal;
		m_always_update = _always_update;
		m_state = _state;
	}

	virtual void dump(std::ostream& _strm) const
	{
		fbox::xml::make_tag(_strm,fbox::type_id(*this,true))
			.attr("always_update",m_always_update)
			.attr("source",mp_src)
			.attr("trigger",mp_sig);
	}

protected:
	shared_ptr<basic_valued_agent<state_type,time_type> > mp_src;
	shared_ptr<basic_valued_agent<signal_type,time_type> > mp_sig;
	signal_type m_last_sig;
	time_type m_trigger;
	bool m_always_update;

	using parent_type::m_time;
	using parent_type::m_state;
	using parent_type::m_start;
	using parent_type::m_end;
	using parent_type::mp_rnd;

	virtual void init_impl()
	{
		mp_src->init(m_start,m_end,mp_rnd);
		mp_sig->init(m_start,m_end,mp_rnd);
	}
	
	virtual void reset_impl()
	{
		mp_src->reset();
		mp_sig->reset();

		m_last_sig = mp_sig->state();
		m_state = mp_src->state();
	}
	
	virtual bool update_impl() 
	{
		mp_sig->update(m_time);
		
		if (mp_sig->state() > m_last_sig)
		{
			mp_src->update(m_time);
			m_state = mp_src->state();
		}
		else if (m_always_update)
		{
			mp_src->update(m_time);
		}

		m_last_sig = mp_sig->state();
		return true;
	}
};



//! The lookback agent repeats the underlying process one or more time steps delayed
template
<
	typename _operation_type,
	typename _state_type,
	typename _time_type = default_time_type,
	typename _duration_type = _time_type
>
class lookback
:	public single_agent_impl<_state_type,_time_type,boost::shared_ptr<agent>,_duration_type>
{
public:
	typedef single_agent_impl<_state_type,_time_type,boost::shared_ptr<agent>,_duration_type> parent_type;
	typedef _operation_type operation_type;

	typedef typename parent_type::time_type time_type;
	typedef typename parent_type::state_type state_type;
	typedef typename parent_type::duration_type duration_type;

	virtual ~lookback() {}

	void setup(
		shared_ptr<basic_valued_agent<state_type,time_type> > _source, //!< Underlying agent
		const _duration_type& _period) //!< Lookback period
	{
		this->connect(mp_src = _source);
		m_period = _period;
	}

	virtual void dump(std::ostream& _strm) const
	{
		fbox::xml::make_tag(_strm,fbox::type_id(*this,true))
			.attr("period",m_period)
			.attr("source",mp_src);
	}

protected:
	shared_ptr<basic_valued_agent<state_type,time_type> > mp_src;
	duration_type m_period;
	operation_type m_op;

	typedef std::pair<time_type,state_type> stored_pair_type; 
	std::deque<stored_pair_type> m_queue;
	
	using parent_type::m_time;
	using parent_type::m_state;
	using parent_type::m_start;
	using parent_type::m_end;
	using parent_type::m_live;
	using parent_type::mp_rnd;

	virtual void reset_impl()
	{
		m_queue.clear();
		m_live = update_impl(); // if trigger is crossed on reset we must turn m_live to false, otherwise won't disable future updates
	}
	
	virtual bool update_impl() 
	{
		m_queue.push_back( stored_pair_type(m_time + m_period,mp_src->state()) );
		
		while (m_queue.front().first <= m_time) m_queue.pop_front();
		m_state = m_op(m_queue);

		return true;
	}
};



namespace operators
{

typedef std::deque< std::pair<default_time_type,double> > basic_lookback_queue;

struct queue_first : public basic_operator<basic_lookback_queue,double>
{
	double operator() (const basic_lookback_queue& _a) const { return _a.front().second; }
};


struct queue_sum : public basic_operator<basic_lookback_queue,double>
{
	double operator() (const basic_lookback_queue& _a) const
	{
		double x(0.0);
		basic_lookback_queue::const_iterator itr = _a.begin();
		for(; itr != _a.end(); ++itr) x += itr->second;
		return x;
	}
};


struct queue_mean : public basic_operator<basic_lookback_queue,double>
{
	double operator() (const basic_lookback_queue& _a) const
	{
		int n(0);
		double x(0.0);
		basic_lookback_queue::const_iterator itr = _a.begin();
		for(; itr != _a.end(); ++itr)
		{
			++n;
			x += itr->second;
		}
		return x / n;
	}
};

struct queue_std : public basic_operator<basic_lookback_queue,double>
{
	double operator() (const basic_lookback_queue& _a) const
	{
		int n(1); // unbiased estimator
		double x(0.0);
		basic_lookback_queue::const_iterator itr = _a.begin();
		for(; itr != _a.end(); ++itr)
		{
			++n;
			x += itr->second * itr->second;
		}
		return std::sqrt(x) / n;
	}
};




} // namespace operators
} // namespace simulate
} // namespace fbox

#endif
