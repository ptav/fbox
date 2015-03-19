/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Expression handling and attached agents
*/

#include "expressions.h"

namespace fbox {
namespace simulate {


//////////////////////////////////////////////////////
// expression_agent
//////////////////////////////////////////////////////

expression_agent::expression_agent()
:	mp_init(new constant_expression<double>(0.0)),
	mp_reset(mp_init),
	mp_update(mp_reset)
{}


expression_agent::expression_agent(const expression_agent& _expr)
:	mp_init(_expr.mp_init),
	mp_reset(_expr.mp_reset),
	mp_update(_expr.mp_update)
{}


void expression_agent::setup(double_expression_ptr _expression)
{
	mp_init.reset( new constant_expression<double>(0.0) );
	mp_reset = mp_update = _expression;
}


void expression_agent::set_init(double_expression_ptr _expression)
{
	mp_init = _expression;
}


void expression_agent::set_reset(double_expression_ptr _expression)
{
	mp_reset = _expression;
}


void expression_agent::set_update(double_expression_ptr _expression)
{
	mp_update = _expression;
}


void expression_agent::init_impl()
{
	m_state = mp_init->value();
}


void expression_agent::reset_impl()
{
	m_state = mp_reset->value();
}


bool expression_agent::update_impl()
{
	m_state = mp_update->value();
	return true;
}


//////////////////////////////////////////////////////
// multi_expression_agent
//////////////////////////////////////////////////////

multi_expression_agent::expression_item::expression_item()
:	init(new constant_expression<double>(0.0)),
	reset(new constant_expression<double>(0.0)),
	update(new constant_expression<double>(0.0))
{}


multi_expression_agent::expression_item::expression_item(const multi_expression_agent::expression_item& _expr)
:	init(_expr.init),
	reset(_expr.reset),
	update(_expr.update)
{}


void multi_expression_agent::setup(size_type _size)
{
	m_vec.resize(_size);
	m_state.resize(_size);
}


void multi_expression_agent::set_init(size_type _index,double_expression_ptr _expression)
{
	if (_index >= m_vec.size()) throw error("Index exceeds multi_expression_agent's dimension");
	m_vec[_index].init = _expression;
}


void multi_expression_agent::set_reset(size_type _index,double_expression_ptr _expression)
{
	if (_index >= m_vec.size()) throw error("Index exceeds multi_expression_agent's dimension");
	m_vec[_index].reset = _expression;
}


void multi_expression_agent::set_update(size_type _index,double_expression_ptr _expression)
{
	if (_index >= m_vec.size()) throw error("Index exceeds multi_expression_agent's dimension");
	m_vec[_index].update = _expression;
}


void multi_expression_agent::set_all(size_type _index,double_expression_ptr _expression)
{
	if (_index >= m_vec.size()) throw error("Index exceeds multi_expression_agent's dimension");
	m_vec[_index].init.reset( new constant_expression<double>(0.0) );
	m_vec[_index].reset = m_vec[_index].update = _expression;
}


void multi_expression_agent::init_impl()
{
	for(size_type i = 0; i != m_vec.size(); ++i) m_state[i] = m_vec[i].init->value();
}


void multi_expression_agent::reset_impl()
{
	for(size_type i = 0; i != m_vec.size(); ++i) m_state[i] = m_vec[i].reset->value();
}


bool multi_expression_agent::update_impl()
{
	for(size_type i = 0; i != m_vec.size(); ++i) m_state[i] = m_vec[i].update->value();
	return true;
}


} // namespace simulate
} // namespace fbox

