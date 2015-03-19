#ifndef __FBOX_SIMULATE_EXPRESSIONS_H__
#define __FBOX_SIMULATE_EXPRESSIONS_H__
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Expression handling and attached agents
*/

#include <fbox/main.h>
#include "agent_impl.h"

namespace fbox {
namespace simulate {

//! Expression interface
template<typename _type>
class basic_expression
{
public:
	typedef _type value_type;
	virtual value_type value() = 0;
};

//! Constant expression
template<typename _type>
class constant_expression : public basic_expression<_type>
{
public:
	typedef typename basic_expression<_type>::value_type value_type;

	constant_expression(value_type _arg) : m_arg(_arg) {}
	virtual value_type value() { return m_arg; }

protected:
	value_type m_arg;
};


//! Expression of single argument
template<typename _operator>
class unary_expression : public basic_expression<typename _operator::target_type>
{
public:
	typedef typename basic_expression<typename _operator::target_type>::value_type value_type;

	typedef _operator operator_type;
	typedef shared_ptr<basic_expression<typename operator_type::source_type> > expr_ptr;

	unary_expression(expr_ptr _arg) : mp_arg(_arg) {}
	virtual value_type value() { return m_op(mp_arg->value()); }

protected:
	operator_type m_op;
	expr_ptr mp_arg;
};


//! Expression of two argument
template<typename _operator>
class binary_expression : public basic_expression<typename _operator::target_type>
{
public:
	typedef typename basic_expression<typename _operator::target_type>::value_type value_type;

	typedef _operator operator_type;
	typedef shared_ptr<basic_expression<typename operator_type::source_type> > expr_ptr;

	binary_expression(expr_ptr _arg1,expr_ptr _arg2) : mp_arg1(_arg1),mp_arg2(_arg2) {}
	virtual value_type value() { return m_op(mp_arg1->value(),mp_arg2->value()); }

protected:
	operator_type m_op;
	expr_ptr mp_arg1,mp_arg2;
};


//! Expression of three argument
template<typename _operator>
class ternary_expression : public basic_expression<typename _operator::target_type>
{
public:
	typedef typename basic_expression<typename _operator::target_type>::value_type value_type;

	typedef _operator operator_type;
	typedef shared_ptr<basic_expression<typename operator_type::source_type> > expr_ptr;

	ternary_expression(expr_ptr _arg1,expr_ptr _arg2,expr_ptr _arg3) : mp_arg1(_arg1),mp_arg2(_arg2),mp_arg3(_arg3) {}
	virtual value_type value() { return m_op(mp_arg1->value(),mp_arg2->value(),mp_arg3->value()); }

protected:
	operator_type m_op;
	expr_ptr mp_arg1,mp_arg2,mp_arg3;
};


//! Expression of three argument
template<typename _type>
class select : public basic_expression<_type>
{
public:
	typedef typename basic_expression<_type>::value_type value_type;

	typedef shared_ptr<basic_expression<bool> > bool_expr_ptr;
	typedef shared_ptr<basic_expression<value_type> > expr_ptr;

	select(bool_expr_ptr _condition,expr_ptr _yes,expr_ptr _no) 
	:	mp_cond(_condition),mp_yes(_yes),mp_no(_no) {}
	
	virtual double value() { return (mp_cond->value()? mp_yes->value() : mp_no->value()); }

protected:
	bool_expr_ptr mp_cond;	
	expr_ptr mp_yes,mp_no;
};


//! Implicitly conversion expression
template<
	typename _source,	//!< Source expression type
	typename _type		//!< Expression type
>
class expression_converter : public basic_expression<_type>
{
public:
	typedef typename basic_expression<_type>::value_type value_type;

	typedef _source source_type;
	typedef shared_ptr<basic_expression<source_type> > source_ptr;

	expression_converter(source_ptr _src)
	:	mp_src(_src) {}
	
	virtual value_type value() { return static_cast<value_type>(mp_src->value()); }

protected:
	source_ptr mp_src;	
};


//! self referencing expressions
template<
	typename _agent_type,
	typename _source_type
>
class basic_linked_expression : public basic_expression<_source_type>
{
public:
	basic_linked_expression(shared_ptr<_agent_type> _agent)
	:	mp_agent(_agent) {}

protected:
	shared_ptr<_agent_type> mp_agent;
};


//! self referencing expressions
template<
	typename _agent_type,
	typename _source_type = typename _agent_type::time_type
>
class time_linked_expression : public basic_linked_expression<_agent_type,_source_type>
{
public:
	typedef basic_linked_expression<_agent_type,_source_type> parent_type;

	typedef typename parent_type::value_type value_type;

	time_linked_expression(shared_ptr<_agent_type> _agent)
	:	parent_type(_agent) {}

	virtual value_type value() { return static_cast<value_type>(this->mp_agent->time()); }
};


//! self referencing expressions
template<
	typename _agent_type,
	typename _source_type = typename _agent_type::duration_type
>
class interval_linked_expression : public basic_linked_expression<_agent_type,_source_type>
{
public:
	typedef basic_linked_expression<_agent_type,_source_type> parent_type;

	typedef typename parent_type::value_type value_type;

	interval_linked_expression(shared_ptr<_agent_type> _agent)
	:	parent_type(_agent) {}

	virtual value_type value() { return static_cast<value_type>(this->mp_agent->time_interval()); }
};


//! self referencing expressions
template<
	typename _agent_type,
	typename _source_type = typename _agent_type::state_type
>
class state_linked_expression : public basic_linked_expression<_agent_type,_source_type>
{
public:
	typedef basic_linked_expression<_agent_type,_source_type> parent_type;

	typedef typename parent_type::value_type value_type;

	state_linked_expression(shared_ptr<_agent_type> _agent)
	:	parent_type(_agent) {}

	virtual value_type value() { return this->mp_agent->state(); }
};


//! self referencing expressions
template<
	typename _agent_type,
	typename _source_type = typename _agent_type::state_type::value_type
>
class sequence_linked_expression : public basic_linked_expression<_agent_type,_source_type>
{
public:
	typedef basic_linked_expression<_agent_type,_source_type> parent_type;

	typedef typename parent_type::value_type value_type;

	sequence_linked_expression(shared_ptr<_agent_type> _agent,size_type _index)
	:	parent_type(_agent),m_index(_index) {}

	virtual value_type value() { return this->mp_agent->state()[m_index]; }

protected:
	size_type m_index;
};


template<typename _agent_type,typename _source_type,typename _func_type>
class linked_expression : public basic_linked_expression<_agent_type,_source_type>
{
public:
	typedef basic_linked_expression<_agent_type,_source_type> parent_type;

	typedef typename parent_type::value_type value_type;

	linked_expression(shared_ptr<_agent_type> _agent,_func_type _func)
	:	parent_type(_agent),mp_func(_func) {}

	virtual value_type value() { return ( ( *(this->mp_agent) ).*mp_func )(); }

protected:
	_func_type mp_func;
};


template<typename _agent_type,typename _return_type,typename _func_type>
shared_ptr<basic_linked_expression<_agent_type,_return_type> > make_linked_expression(
	shared_ptr<_agent_type> _agent,
	_return_type _type,
	_func_type _func)
{
	typedef linked_expression<_agent_type,_return_type,_func_type> expr;
	return shared_ptr<basic_linked_expression<_agent_type,_return_type> >( new expr(_agent,_func) );
}


//! Generic expression agent
class expression_agent
:	public multi_agent_impl<double>
{
public:
	typedef shared_ptr<basic_expression<double> > double_expression_ptr;

	expression_agent(); //!< initialise all linked expressions to null constants
	expression_agent(const expression_agent& _expr);

	//! Setup identical reset and update expressions. Leave init expression null
	void setup(double_expression_ptr _expression);

	void set_init(double_expression_ptr _expression); //!< set individual init expression
	void set_reset(double_expression_ptr _expression); //!< set individual reset expression
	void set_update(double_expression_ptr _expression); //!< set individual update expression

protected:
	double_expression_ptr mp_init;
	double_expression_ptr mp_reset;
	double_expression_ptr mp_update;

	virtual void init_impl();
	virtual void reset_impl();
	virtual bool update_impl();
};



//! Expression agent that supports multiple separate calculations
class multi_expression_agent
:	public multi_agent_impl<std::vector<double> >
{
public:
	typedef shared_ptr<basic_expression<double> > double_expression_ptr;

	//! Setup identical reset and update expressions. Leave init expression null
	void setup(size_type _size);

	void set_init(size_type _index,double_expression_ptr _expression); //!< set individual init expression
	void set_reset(size_type _index,double_expression_ptr _expression); //!< set individual reset expression
	void set_update(size_type _index,double_expression_ptr _expression); //!< set individual update expression
	void set_all(size_type _index,double_expression_ptr _expression); //!< set null init and identical reset and update

protected:
	struct expression_item
	{
		double_expression_ptr init;
		double_expression_ptr reset;
		double_expression_ptr update;

		expression_item(); //!< initialise all linked expressions to null constants
		expression_item(const expression_item& _expr);
	};

	typedef std::vector<expression_item> expression_vector;
	expression_vector m_vec;

	virtual void init_impl();
	virtual void reset_impl();
	virtual bool update_impl();
};



} // namespace simulate
} // namespace fbox

#endif
