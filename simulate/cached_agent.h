#ifndef __FBOX_SIMULATE_CACHED_AGENT_H__
#define __FBOX_SIMULATE_CACHED_AGENT_H__
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Implementation of a cached state agent - update is done only on the first sample of a simulation, then the 
	computed state variables get reused for every other sample.
*/

#include "agent_impl.h"
#include <fbox/math.h>
#include <list>


namespace fbox {
namespace simulate {

//! Implementation of cached state agent. Relies on the agent_impl<...> implementation.
/*!
	Implementations of init_impl and reset_impl are similar to the uncached versions.
	However the implementation of update_impl must update the cached state, pointed to by m_itr
	and not the (inherited) state variable. In the future it would be best not to derive from agent_impl
	to remove this ambiguity (m_state and *m_itr) but for now it is useful to keep replication to 
	a minimum - something to do once the code is fully stable.
*/
template
<
	typename _parent_agent,		//!< Base parent type (usually a basic_value_agent<...>)
	typename _linked_policy,	//!< Linked object policy
	typename _duration = typename _parent_agent::time_type //!< Time interval type
>
class basic_cached_agent
:	public basic_agent_impl<_parent_agent,_linked_policy,_duration>
{
public:
	typedef basic_agent_impl<_parent_agent,_linked_policy,_duration> parent_type;

	typedef typename parent_type::time_type time_type;
	typedef typename parent_type::state_type state_type;

	virtual ~basic_cached_agent() {}

	const state_type& state() const
	{
		#ifdef FBOX_DEBUG
			if (!m_cache.size()) throw error("Cached agent not reset yet (maybe you tried to inspect the state after init but before reset).");
		#endif
		return m_itr->state;
	}

	virtual void init(
		const time_type& _start,
		const time_type& _end,
		generator* _random) 
	{
		FBOX_DBG_LOG(logger::SYSTEM,"basic_cached_agent::init,ptr=" << this << ",type=" << type_id(this));
		basic_agent_impl<_parent_agent,_linked_policy>::init(_start,_end,_random);
		m_cache.clear();
	}
		
	virtual void reset()
	{
		if (this->m_reset) return;

		m_itr = m_cache.begin();
		if (m_itr != m_cache.end()) 
		{
			this->m_time = m_itr->time;
			return;
		}

		FBOX_DBG_LOG(logger::SYSTEM,"basic_cached_agent::reset,ptr=" << this << ",type=" << type_id(this) << ",time=" << this->m_start);
		basic_agent_impl<_parent_agent,_linked_policy>::reset();
		m_cache.push_back(cached_type(m_time,m_state,m_live));
		m_itr = m_cache.begin();
	}

	virtual void update(const time_type& _time)
	{
		if (!m_live || _time <= m_time) return;
		cache_itr_type end = m_cache.end();
		while (m_itr != end && m_itr->time < _time) ++m_itr;
		if (m_itr != end && m_itr->time == _time) // m_itr points to current saved state
		{
			m_time = m_itr->time;
			return; 
		}

		// otherwise a new state must be created and then computed
		FBOX_DBG_LOG(logger::SYSTEM,"basic_cached_agent::update,ptr=" << this << ",type=" << type_id(this) << ",time=" << _time);
		basic_agent_impl<_parent_agent,_linked_policy>::update(_time);
		m_itr = m_cache.insert(m_itr,cached_type(_time,m_state,m_live));
	}

protected:
	struct cached_type
	{
		time_type time;
		state_type state;
		bool live;

		cached_type(const time_type& _t,const state_type& _s,bool _l) : time(_t),state(_s),live(_l) {}
	};

	using parent_type::m_time;
	using parent_type::m_state;
	using parent_type::m_reset;
	using parent_type::m_live;

	typedef std::list<cached_type> cache_type;
	cache_type m_cache;

	typedef typename cache_type::iterator cache_itr_type;
	cache_itr_type m_itr;
};


//! Shorhand for cached agents connected with floating point time and multiple dependents. Default link uses a Boost <code>shared_ptr</code>.
template<
	typename _state_type,						//!< State variable type
	typename _time_type = default_time_type,	//!< Time type
	typename _linked = boost::shared_ptr<agent>,//!< Dependent type
	typename _duration = _time_type				//!< Time interval type
>
class multi_cached_agent_impl
:	public basic_cached_agent<
		basic_valued_agent<_state_type,_time_type>,
		linked_list_policy<_linked>,
		_duration
	>
{
public:
	typedef multi_agent_impl<_state_type,_time_type,_linked,_duration> multi_agent_impl_type;

	//! Clear connected components
	void clear_connected() { this->m_linked_policy.clear(); }

	//! Connect a process
	void connect(_linked _agent) { this->m_linked_policy.connect(_agent); }

	//! Count number of directly connected components
	size_type count_connected() const { return this->m_linked_policy.count_connected(); }
};


//! Shorhand for cached agents connected with floating point time and a single dependent. Default link uses a Boost <code>shared_ptr</code>.
template<
	typename _state_type,						//!< State variable type
	typename _time_type = default_time_type,	//!< Time type
	typename _linked = boost::shared_ptr<agent>,//!< Dependent type
	typename _duration = _time_type				//!< Time interval type
>
class single_cached_agent_impl
:	public basic_cached_agent<
		basic_valued_agent<_state_type,_time_type>,
		single_item_linked_policy<_linked>,
		_duration
	>
{
public:
	typedef single_agent_impl<_state_type,_time_type,_linked,_duration> single_agent_impl_type;

	//! Connect a process
	void connect(_linked _agent) { this->m_linked_policy.connect(_agent); }
};


//! Shorhand for cached agents connected with floating point time and no dependents.
template<
	typename _state_type,						//!< State variable type
	typename _time_type = default_time_type,	//!< Time type
	typename _duration = _time_type				//!< Time interval type
>
class standalone_cached_agent_impl
:	public basic_cached_agent<
		basic_valued_agent<_state_type,_time_type>,
		independent_linked_policy,
		_duration
	>
{
public:
	typedef standalone_agent_impl<_state_type,_time_type,_duration> standalone_agent_impl_type;
};


} // namespace simulate
} // namespace fbox

#endif
