/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Basic simulation agents
*/

#include "basic_agents.h"

namespace fbox {
namespace simulate {


//////////////////////////////////////////////////////
// twisted_gaussian_variate
//////////////////////////////////////////////////////

void gaussian_twister::setup(double_agent_ptr _agent,double _param)
{
	connect(mp_agent = _agent);
	
	m_param = _param; 
	m_param_sq = _param * _param / 2.0;
}


void gaussian_twister::init_impl()
{
	if (!mp_agent) throw error("gaussian_twister underlying variate not setup correctly");
}
	

void gaussian_twister::reset_impl()
{
	update_impl();
}
	

bool gaussian_twister::update_impl()
{
	m_state = mp_agent->state() + m_param;
	update_weight( std::exp( m_param_sq - m_param * m_state ) );

	return true;
}



} // namespace simulate
} // namespace fbox

