#ifndef __FBOX_SIMULATE_AGENT_IMPL_H__
#define __FBOX_SIMULATE_AGENT_IMPL_H__
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Base agent implementations

	To define individual agents use the following template (the example relies on a multiply 
	connected agent with a sinple floating point value variable:

	<code>
	class my_agent
	:	public multi_agent_impl<double>
	{
	protected:
		virtual void init_impl(const time_type& _start,const time_type& _end);
		virtual void reset_impl();
		virtual bool update_impl();
	};
	</code>

	Replace <code>multi_agent_impl</code> with <code>single_agent_impl</code> or 
	<code>standalone_agent_impl</code> as required.
*/

#include <list>
#include <boost/smart_ptr.hpp>
#include <fbox/logger.h>
#include "agent.h"


namespace fbox {
namespace simulate {

#pragma warning(disable: 4172) // disable warning in agent_impl_base::value()

//! Being boost::shared_ptr into this namespace
using boost::shared_ptr;


//! Default base implementation of the agent interface
template
<
	typename _parent_agent,		//!< Base parent type
	typename _linked_policy,	//!< Linked object policy
	typename _duration = typename _parent_agent::time_type //!< Optionally specify which type expresses time intervals
>
class basic_agent_impl
:	public _parent_agent
{
public:
	typedef _parent_agent parent_type;
	typedef _linked_policy linked_policy;
	typedef _duration duration_type;
	typedef basic_agent_impl<parent_type,linked_policy,duration_type> agent_impl_type;

	typedef typename parent_type::time_type time_type;
	typedef typename parent_type::state_type state_type;

	basic_agent_impl()
	:	m_time(typename _parent_agent::time_type()),
		m_start(typename _parent_agent::time_type()),
		m_end(typename _parent_agent::time_type()),
		m_dtime(_duration()),
		m_state(typename _parent_agent::state_type()),
		m_state0(typename _parent_agent::state_type()),
		m_live(false),
		m_init(false),
		m_reset(false),
		mp_rnd(0)
	{}

	virtual ~basic_agent_impl() {}

	virtual const time_type& start() const { return m_start; }
	virtual const time_type& time() const { return m_time; }
	virtual const duration_type& time_interval() const { return m_dtime; }
	virtual const state_type& state() const { return m_state; }
	virtual bool is_live() const { return m_live; }

	//! Initialise agent for simulation: Recurse all connected agentes, set starting time, then call 
	//! <code>init_impl()</code>. Finally saves the current agent value to be recovered by <code>reset()</code>.
	virtual void init(
		const time_type& _start,
		const time_type& _end,
		generator* _random) 
	{
		FBOX_DBG_LOG(logger::SYSTEM,"agent_impl::init,ptr=" << this << ",type=" << type_id(this));

		if (m_init) return;

		m_linked_policy.init(_start,_end,_random);
		mp_rnd = _random;
		m_start = m_time = _start;
		m_end = _end;

		init_impl();
		
		m_state0 = m_state;
		m_init = true;
		m_reset = false;
	}

	virtual void reset()
	{
		if (m_reset) return;

		m_linked_policy.reset();
		m_time = m_start;
		m_state = m_state0;
		m_dtime = duration_type();
		m_live = true;
		m_init = false;

		reset_impl();

		m_reset = true;
	}

	virtual void update(const time_type& _time)
	{
		if (_time <= m_time) return;
		m_linked_policy.update(_time); 

		if (!m_live) return;

		m_reset = false;
		m_dtime = _time - m_time;
		m_time = _time;
		m_live = update_impl(); 
	}

	virtual void dump(std::ostream& _strm) const { m_linked_policy.dump(_strm); }

	linked_policy& linked() { return m_linked_policy; }
	const linked_policy& linked() const { return m_linked_policy; }

protected:
	linked_policy m_linked_policy;

	bool m_init;					//!< True when initialised, false after every Reset call
	bool m_reset;					//!< True when reset, false after any Update call
	bool m_live;					//!< Result of last update operation - if false update is skipped

	state_type m_state,m_state0;	//!< Current and saved agent state
	time_type m_time,m_start,m_end;	//!< Current, starting and final time
	duration_type m_dtime,m_elapsed;//!< Interval between two updates
	generator* mp_rnd;

	double rnd() const { return mp_rnd->rnd(); }
	void update_weight(double w) const { mp_rnd->update_weight(w); }

	virtual void init_impl() {} //! Called by <code>init()</code>. Implement in derived agentes. 
	virtual void reset_impl() {} //! Called by <code>reset()</code>. Implement in derived agentes. 
	virtual bool update_impl() { return true; } //! Called by <code>update()</code>. Implement in derived agentes. 
};


//! Default delink policy does nothing
struct default_delink_policy
{
	template<typename _linked_type>
	static void delink(_linked_type& _c) {}
};


//! Linked object policy for a variable sized dependency list
template
<
	typename _linked_type, //!< The linked type must support dereference operators and have pointer-like behaviour
	typename _delink_policy = default_delink_policy //!< Handle de-linking during dtor
>
class linked_list_policy
{
public:
	typedef _linked_type linked_type;
	typedef _delink_policy delinked_policy;

	typedef std::list<linked_type> linked_list_type;
	typedef typename linked_list_type::iterator iterator;
	typedef typename linked_list_type::const_iterator const_iterator;

	~linked_list_policy()
	{
		delinked_policy::delink(mp_linked);
	}

	template<typename _time_type>
	void init(
		const _time_type& _start,
		const _time_type& _end,
		generator* _random) 
	{
		typename linked_list_type::iterator itr = mp_linked.begin();
		typename linked_list_type::iterator top = mp_linked.end();
		for (; itr != top; ++itr) (*itr)->init(_start,_end,_random);
	}

	void reset()
	{
		typename linked_list_type::iterator itr = mp_linked.begin();
		typename linked_list_type::iterator end = mp_linked.end();
		for (; itr != end; ++itr) (*itr)->reset();
	}

	template<typename _time_type>
	void update(const _time_type& _time)
	{
		typename linked_list_type::iterator itr = mp_linked.begin();
		typename linked_list_type::iterator end = mp_linked.end();
		for (; itr != end; ++itr) (*itr)->update(_time); 
	}

	void clear() 
	{
		delinked_policy::delink(mp_linked);
		mp_linked.clear();
	}

	void connect(const linked_type _agent) { mp_linked.push_back(_agent); }

	size_type count_connected() const { return mp_linked.size(); }

	linked_list_type& dependents() { return mp_linked; }
	const linked_list_type& dependents() const { return mp_linked; }

	iterator begin() { return mp_linked.begin(); }
	iterator end() { return mp_linked.end(); }

	const_iterator begin() const { return mp_linked.begin(); }
	const_iterator end() const { return mp_linked.end(); }

	void dump(std::ostream& _strm) const
	{
		fbox::xml::make_tag(_strm,fbox::type_id(*this,true),false).attr("ptr",this);
		typename linked_list_type::const_iterator itr = mp_linked.begin();
		typename linked_list_type::const_iterator top = mp_linked.end();
		for (; itr != top; ++itr) (*itr)->dump(_strm);
		_strm << '>';
	}

protected:
	linked_list_type mp_linked;
};


//! Linked object policy for a agent with a single dependency
template
<
	typename _linked_type, //!< The linked type must support dereference operators and have pointer-like behaviour
	typename _delink_policy = default_delink_policy //!< Handle de-linking during dtor
>
class single_item_linked_policy
{
public:
	typedef _linked_type linked_type;
	typedef _delink_policy delink_policy;

	~single_item_linked_policy() { delink_policy::delink(mp_linked); }

	template<typename _time_type>
	void init(const _time_type& _start,const _time_type& _end,generator* _random)
	{
		if (mp_linked == 0) throw error("Missing underlying agent in single_item_linked_policy");
		mp_linked->init(_start,_end,_random);
	}

	void reset() { mp_linked->reset(); }

	template<typename _time_type>
	void update(const _time_type& _time) { mp_linked->update(_time); }

	void clear() { delink_policy::delink(mp_linked); mp_linked = 0; }
	void connect(const linked_type _agent) { mp_linked = _agent; }

	size_type count_connected() const { return (mp_linked != 0? 1 : 0); }	
	linked_type& linked() { return mp_linked; }
	const linked_type& linked() const { return mp_linked; }

	void dump(std::ostream& _strm) const
	{
		if (mp_linked == 0) throw error("Missing underlying agent in single_item_linked_policy");
		fbox::xml::make_tag(_strm,fbox::type_id(*this,true),false).attr("ptr",this);
		mp_linked->dump(_strm);
		_strm << '>';
	}

protected:
	linked_type mp_linked;
};


//! Linked object policy for agentes with no dependencies
class independent_linked_policy
{
public:
	template<typename _time_type>
	void init(const _time_type& _start,const _time_type& _end,generator* _random) {}

	void reset() {}

	template<typename _time_type>
	void update(const _time_type& _time) {}

	size_type count_connected() const { return 0; }

	void dump(std::ostream& _strm) const {}
};



// Shorthand definitions for different agent types
typedef int default_time_type;
typedef basic_agent<default_time_type> agent; //!< Standard simulation agent uses double precision time
typedef basic_valued_agent<double,default_time_type> double_agent; //!< Standard valued agent with double precision value
typedef basic_valued_agent<size_type,default_time_type> counting_agent; //!< Standard valued agent with unsigned integer value (typically a that is a counting process)
typedef basic_valued_agent<bool,default_time_type> boolean_agent; //!< Standard valued agent with boolean value

// Shorthand definitions for pointers to agent types
typedef shared_ptr<agent> agent_ptr; //!< Shorthand for <code>shared_ptr</code> to a standard agent
typedef shared_ptr<double_agent> double_agent_ptr; //!< Shorthand for <code>double_agent</code> shared pointer
typedef shared_ptr<counting_agent> counting_agent_ptr; //!< Shorthand for <code>counting_agent</code> shared pointer
typedef shared_ptr<boolean_agent> boolean_agent_ptr; //!< Shorthand for <code>double_agent</code> shared pointer


//! Shorhand for agents connected with floating point time and multiple dependents. Default link uses a Boost <code>shared_ptr</code>.
template<
	typename _state_type,						//!< State variable type
	typename _time_type = default_time_type,	//!< Time type
	typename _linked = boost::shared_ptr<agent>,//!< Dependent type
	typename _duration = _time_type				//!< Time interval type
>
class multi_agent_impl
:	public basic_agent_impl<
		basic_valued_agent<_state_type,_time_type>,
		linked_list_policy<_linked>,
		_duration
	>
{
public:
	typedef multi_agent_impl<_state_type,_time_type,_linked,_duration> multi_agent_impl_type;
	typedef basic_agent_impl<basic_valued_agent<_state_type,_time_type>,linked_list_policy<_linked>,_duration> parent_type;

	//using typename parent_type::linked_policy;

	//! Clear connected components
	void clear_connected() { this->m_linked_policy.clear(); }

	//! Connect a process
	void connect(_linked _agent) { this->m_linked_policy.connect(_agent); }

	//! Count number of directly connected components
	size_type count_connected() const { return this->m_linked_policy.count_connected(); }
};


//! Shorhand for agents connected with floating point time and a single dependent. Default link uses a Boost <code>shared_ptr</code>.
template<
	typename _state_type,						//!< State variable type
	typename _time_type = default_time_type,	//!< Time type
	typename _linked = boost::shared_ptr<agent>,//!< Dependent type
	typename _duration = _time_type				//!< Time interval type
>
class single_agent_impl
:	public basic_agent_impl<
		basic_valued_agent<_state_type,_time_type>,
		single_item_linked_policy<_linked>,
		_duration
	>
{
public:
	typedef single_agent_impl<_state_type,_time_type,_linked,_duration> single_agent_impl_type;

	//! Connect a process
	void connect(_linked _agent) { this->m_linked_policy.connect(_agent); }

	//! Count number of directly connected components (either 0 or 1)
	size_type count_connected() const { return this->m_linked_policy.count_connected(); }
};


//! Shorhand for agents connected with floating point time and no dependents.
template<
	typename _state_type,						//!< State variable type
	typename _time_type = default_time_type,	//!< Time type
	typename _duration = _time_type				//!< Time interval type
>
class standalone_agent_impl
:	public basic_agent_impl<
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
