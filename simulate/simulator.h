#ifndef __FBOX_SIMULATE_SIMULATOR_H__
#define __FBOX_SIMULATE_SIMULATOR_H__
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Simulation implementation for agent based simulation framework
*/


#include <functional>
#include <limits>
#include <boost/functional.hpp>
#include <fbox/main.h>
#include "agent_impl.h"


namespace fbox {
namespace simulate {

//! Simulator object.
template<
	typename _observer_type,
	typename _time_type = default_time_type,
	typename _duration_type = default_time_type
>
class simulator
{
public:
	typedef _observer_type observer_type;
	typedef _time_type time_type;
	typedef _duration_type duration_type;

	 //! Local error type
	FBOX_LOCAL_ERROR(error,fbox::error,"Simulator error");

	simulator();

	void clear_fixes() { m_fixes.clear(); m_observers.clear(); } //!< Clear all time fixes and corresponding observers
	void add_fix(const time_type& _fix); //!< Add a new time fix
	void add_fix(const time_type& _fix,const observer_type& _observer); //!< Add a new time fix and copy the provided accumulator for that fix

	void set_start(const time_type& _start) { m_start = _start; } //!< Set simulation start time
	void set_step(const duration_type& _step) { m_step = _step; } //!< Set simulation step size
	void set_samples(size_type _samples) { m_samples = _samples; } //!< Set simulation sample size
	void set_seed(long _seed) { m_seed = _seed; } //!< Set random number generator seed (default is provided)
	void set_observers(const observer_type& _observer); //!< Set a user defined observer (useful to observers with properties)

	observer_type& get_observer(size_type _i) { return m_observers[_i]; }

	//! Run simulation
	void simulate(double_agent_ptr _agent,bool _init=true,bool _reset=true);

	const time_type& start() const { return m_start; }
	const time_type& end() const { return m_fixes.back(); }
	const observer_type& observer(size_type _i) const { return m_observers[_i]; }
	const std::vector<time_type>& fixes() const { return m_fixes; }

protected:
	size_type m_samples;
	time_type m_start;
	std::vector<time_type> m_fixes;
	duration_type m_step;
	long m_seed;

	//! Simulate to a given time. No reset is made starts from 'time (first
	//! update is to time+step). Returns date of last update.
	void simulate_to(double_agent_ptr & _agent,const time_type& _start,const time_type& _end) const;

private:
	bool m_is_init;
	std::auto_ptr<generator> mp_rnd;
	std::vector<observer_type> m_observers;
	observer_type m_observer_template;
	void create_observers(); // create observers for each fix
};



//////////////////////////////////////////////////////
// Simulator
//////////////////////////////////////////////////////

template<typename _observer,typename _time_type,typename _duration_type>
simulator<_observer,_time_type,_duration_type>::simulator()
:	m_is_init(false),
	m_start(0),
	m_step( std::numeric_limits<_duration_type>::max() ),
	m_seed(839823),
	mp_rnd(new generator)
{}


template<typename _observer,typename _time_type,typename _duration_type>
void simulator<_observer,_time_type,_duration_type>::add_fix(const time_type& _fix)
{
	add_fix(_fix,m_observer_template);
}


template<typename _observer_type,typename _time_type,typename _duration_type>
void simulator<_observer_type,_time_type,_duration_type>::add_fix(const time_type& _fix,const _observer_type& _observer)
{
	if (m_fixes.size() && _fix <= m_fixes.back())
		throw error("Time fixes must be strictly increasing");

	m_fixes.push_back(_fix);
	m_observers.push_back(_observer);
}


template<typename _observer_type,typename _time_type,typename _duration_type>
void simulator<_observer_type,_time_type,_duration_type>::set_observers(const _observer_type& _observer)
{
	m_observer_template = _observer;
	std::for_each(m_observers.begin(),m_observers.end(),bind2nd(boost::mem_fun_ref(&observer_type::operator=),m_observer_template));
}


template<typename _observer,typename _time_type,typename _duration_type>
void simulator<_observer,_time_type,_duration_type>::simulate(
	double_agent_ptr _agent,
	bool _init,
	bool _reset)
{
	if (!m_fixes.size()) throw error("Simulate called before setting up time fixes");
	size_type fcount = m_fixes.size();

	std::for_each(m_observers.begin(),m_observers.end(),boost::bind2nd(boost::mem_fun_ref(&observer_type::set_agent),_agent));

	if (_init)
	{
		mp_rnd->set_seed(m_seed);
		_agent->init(m_start,m_fixes.back(),mp_rnd.get());
		std::for_each(m_observers.begin(),m_observers.end(),boost::mem_fun_ref(&observer_type::init));
	}

	for (size_type s = m_samples; s != 0; --s)
	{
		mp_rnd->reset();
		_agent->reset();

		time_type t0 = m_start,t1;
		for (size_type t = 0; t < fcount; ++t)
		{
			t1 = m_fixes[t];
			simulate_to(_agent,t0,t1);
			m_observers[t].observe(mp_rnd->weight());
			t0 = t1;
		}
	}

	for (size_type t = 0; t < fcount; ++t) m_observers[t].end();

	if (_reset)
	{
		mp_rnd->reset();
		_agent->reset();
	}
}


template<typename _observer,typename _time_type,typename _duration_type>
void simulator<_observer,_time_type,_duration_type>::simulate_to(
	double_agent_ptr& _agent,
	const time_type& _start,
	const time_type& _end) 
	const
{
	for(time_type t = _start + m_step; t <= _end; t += m_step) _agent->update(t);
	_agent->update(_end); // update to end
}


} // namespace simulate
} // namespace fbox

#endif
