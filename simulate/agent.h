#ifndef __FBOX_SIMULATE_AGENT_H__
#define __FBOX_SIMULATE_AGENT_H__
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Agent based simulation interface
*/

#include <fbox/main.h>
#include <fbox/error.h>
#include <fbox/random.h>
#include <fbox/xml_utils.h>
#include <iostream>

namespace fbox {
namespace simulate {

//! Random number generator wrapper. Keeps track of path weight
/*!
	Usage: call <code>reset()</code> before generating a new path. Call 
	<code>update_weight()</code> each  time the path weighting is altered.
*/
class generator
{
public:
	generator() : m_weight(1.0) {}

	//! Set generator seed
	void set_seed(long _seed) { m_rnd.seed(_seed); }

	//! Generate new deviate
	double rnd() { return m_rnd(); }
	
	//! Retrieve current path weight
	double weight() const { return m_weight; }

	//! Call reset before generating a new path
	void reset() { m_weight = 1.0; }

	//! new weight = old weight * w
	void update_weight(double _w) { m_weight *= _w; }

private:
	math::mersenne_twister m_rnd;
	double m_weight;
};


//! Agent class interface.
template
<
	typename _time_type
>
class basic_agent
{
public:
	typedef _time_type time_type;
	typedef basic_agent<time_type> basic_agent_type;

	//! Error type
	class error : public fbox::error
	{
	public:
		error(const std::string& _msg) : fbox::error(_msg) {}
		error(const error& _err) : fbox::error(_err) {}
		error(basic_agent_type* _ptr,const std::string& _msg) : fbox::error(err_msg(_ptr,_msg)) {}
	};

	virtual ~basic_agent() {}

	virtual bool is_live() const = 0; //!< False if agent no longer changing

	//! Set initial time and perform any other operations required before starting the simulation
	virtual void init(
		const time_type& _start,	//!< Simulation start time
		const time_type& _end,		//!< Simulation final end time
		generator* _random)			//!< Random number generator
		= 0;

	//! Restore to saved state. Reset is called at the beginning of every new sample
	virtual void reset() = 0;

	//! Move forward in time
	virtual void update(const time_type& _time) = 0;

	//! Override to provide introspection
	virtual void dump(std::ostream& _strm) const
	{
		fbox::xml::make_tag(_strm,fbox::type_id(*this,true),true).attr("ptr",this);
	}

private:
	static std::string err_msg(basic_agent_type* _ptr,const std::string& _msg)
	{
		std::stringstream s;
		s << _msg << ": " << _ptr << " of type " << type_id(_ptr) << std::endl; 
		return s.str();
	}
};


//! Provide agent introspection through the << operator
template<typename _time_type>
std::ostream& operator<<(std::ostream& _strm,const basic_agent<_time_type>& _a)
{
	_a.dump(_strm);
	return _strm;
}


//! Base implementation of the agent interface
template
<
	typename _state_type,	//!< State type
	typename _time_type,	//!< Time type
	typename _parent_agent=basic_agent<_time_type> //!< Parent type (default is basic_agent)
>
class basic_valued_agent
:	public _parent_agent
{
public:
	typedef _state_type state_type;
	typedef basic_valued_agent<_time_type,_state_type,_parent_agent> basic_valued_agent_type;
	
	using typename _parent_agent::time_type;

	//! Retrieve agent's value value
	virtual const state_type& state() const = 0;
};

} // namespace simulate
} // namespace fbox

#endif
