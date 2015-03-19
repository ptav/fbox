#ifndef __FBOX_SIMULATE_AGENT_VECTOR_H__
#define __FBOX_SIMULATE_AGENT_VECTOR_H__
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Vector of multiple agents
*/

#include <fbox/main.h>
#include "agent_impl.h"

namespace fbox {
namespace simulate {


//! Vector of agents (used in simulating and observing multiple processes and as helper for certain agents)
template<
	typename _sub_agent_ptr,	//!< Linked agent type
	typename _state_type		//!< State type (of the agent_vector)
>
class basic_agent_vector
:	public multi_agent_impl<_state_type>
{
public:
	typedef _sub_agent_ptr sub_agent_ptr;

	void clear_connected()
	{
		mp_a.clear();
		multi_agent_impl_type::clear_connected();
	}

	void connect(sub_agent_ptr _a)
	{
		mp_a.push_back(_a);
		multi_agent_impl_type::connect(_a);
	}

	sub_agent_ptr get_agent(size_type _i) { return mp_a[_i]; }
	
protected:
	typedef multi_agent_impl<_state_type> multi_agent_impl_type;
	typedef std::vector<sub_agent_ptr> agent_vector_type;
	agent_vector_type mp_a;
};



//! Simples agent vectro type
template<typename _state_type>
class agent_vector : public basic_agent_vector<agent_ptr,_state_type>
{};


} // namespace simulate
} // namespace fbox

#endif
