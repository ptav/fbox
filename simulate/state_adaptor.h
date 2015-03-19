#ifndef __FBOX_SIMULATE_STATE_ADAPTOR_H__
#define __FBOX_SIMULATE_STATE_ADAPTOR_H__
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Some basic pricing agents
*/

#include <fbox/main.h>
#include "agent_impl.h"

namespace fbox {
namespace simulate {


//! Default convert policy for state_adaptor (does nothing - input and output types are identical)
template<typename _type>
struct state_adaptor_null_convert
{
	state_adaptor_null_convert(const _type* _in) : mp_in(_in) {}
	void reset(const _type* _in) { mp_in = _in; }
	const _type& operator() () const { return *mp_in; }

private:
	const _type* mp_in;
};


//! Simple convert policy for state_adaptor relies on C-style implicit casting
template<typename _in_type,typename _out_type>
struct state_adaptor_convert
{
	state_adaptor_convert(const _in_type* _in) : mp_in(_in) {}
	void reset(const _in_type* _in) { mp_in = _in; }
	const _out_type& operator() () const { return m_out = *mp_in; }

private:
	const _in_type* mp_in;
	mutable _out_type m_out;
};


//! Converts comples state type into simple 1-dim state
template<
	typename _parent,			//!< Underlying agent
	typename _source,			//!< Source type
	typename _state = _source,	//!< State type (if different from the source then implicit cast must be available)
	typename _convert_policy = state_adaptor_null_convert<_source> //!< Convertion policy to apply in converting (by default null policy is used)
>
class state_adaptor
:	public basic_valued_agent<_state,typename _parent::time_type>
{
public:
	typedef _parent parent_type;
	typedef _source source_type;
	typedef shared_ptr<parent_type> parent_ptr;
	typedef typename parent_type::time_type time_type;
	typedef typename basic_valued_agent<_state,time_type>::state_type state_type;

	state_adaptor(parent_ptr _ptr,const source_type* _src)
	:	mp_agent(_ptr),m_convert(_src) {}

	state_adaptor(parent_ptr _ptr,const source_type& _src)
	:	mp_agent(_ptr),m_convert(&_src) {}

	virtual const state_type& state() const { return m_convert(); }
	virtual bool is_live() const { return mp_agent->is_live(); }
	virtual void init(const time_type& _start,const time_type& _end,generator* _random) { mp_agent->init(_start,_end,_random); }
	virtual void reset() { mp_agent->reset(); }
	virtual void update(const time_type& _time) { return mp_agent->update(_time); }

protected:
	parent_ptr mp_agent;
	_convert_policy m_convert;
};


} // namespace simulate
} // namespace fbox

#endif
