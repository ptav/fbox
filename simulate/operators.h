#ifndef __FBOX_SIMULATE_OPERATORS_H__
#define __FBOX_SIMULATE_OPERATORS_H__
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Operator based on standard agents 
*/

#include <fbox/main.h>
#include <fbox/math.h>
#include "agent_impl.h"
#include "agent_vector.h"
#include <boost/math/distributions.hpp>


namespace fbox {
namespace simulate {


//! Unary operator
template
<
	typename _op_type,							//!< Operation type
	typename _time_type = default_time_type,	//!< Time type
	typename _duration_type = _time_type		//!< Time interval type
>
class unary_operator
:	public single_agent_impl
	<
		typename _op_type::target_type,
		_time_type,
		boost::shared_ptr<basic_valued_agent<typename _op_type::source_type,_time_type> >,
		_duration_type
	>
{
public:
	typedef _op_type operation_type;
	typedef typename operation_type::source_type source_type;
	typedef typename operation_type::target_type target_type;
	typedef boost::shared_ptr<basic_valued_agent<source_type,_time_type> > source_agent_ptr;

	virtual ~unary_operator() {}

	operation_type& get_operation() { return m_op; }

protected:
	operation_type m_op;

	virtual void reset_impl() { update_impl(); }

	virtual bool update_impl()
	{
		this->m_state = m_op(this->m_linked_policy.linked()->state());
		return this->m_linked_policy.linked()->is_live();
	}
};


//! Cumulative operator (accumulates sucessive agent values)
/*!
	By convention the initial value is a copy of the underlying agent's value at reset. 
	In other words (xi being the underlying vales and yi the sequence operator's):

	y0 = x0
	y1 = op(x1,y0)
	y2 = op(x2,y1)
	...
*/
template
<
	typename _op_type,							//!< Operation type
	typename _time_type = default_time_type,	//!< Time type
	typename _duration_type = _time_type		//!< Time interval type
>
class cumulative_operator
:	public single_agent_impl
	<
		typename _op_type::target_type,
		_time_type,
		boost::shared_ptr<basic_valued_agent<typename _op_type::source_type,_time_type> >,
		_duration_type
	>
{
public:
	typedef _op_type operation_type;
	typedef typename operation_type::source_type source_type;
	typedef basic_valued_agent<source_type,_time_type> source_agent_type;
	typedef boost::shared_ptr<source_agent_type> source_agent_ptr;

	virtual ~cumulative_operator() {}
	
	operation_type& get_operation() { return m_op; }

protected:
	operation_type m_op;

	virtual void reset_impl();
	virtual bool update_impl();
};



//! Sequential operator (operates on each pair of successive agent values)
/*!
	y0 = op(x0,x-1)
	y1 = op(x1,x0)
	y2 = op(x2,x1)
	...

	By default the x-1 value is the underlying agent's value at reset. Effectively the
	sequence operator makes the implied assumption that the underlying process is
	constant before the start of the simulation such that y0 = op(x0,x-1) = op(x0,x0)

	This behaviour can be overriden by supplying a x-1 value explicitly
*/
template
<
	typename _op_type,							//!< Operation type
	typename _time_type = default_time_type,	//!< Time type
	typename _duration_type = _time_type		//!< Time interval type
>
class sequential_operator
:	public single_agent_impl
	<
		typename _op_type::target_type,
		_time_type,
		boost::shared_ptr<basic_valued_agent<typename _op_type::source_type,_time_type> >,
		_duration_type
	>
{
public:
	typedef _op_type operation_type;
	typedef typename operation_type::source_type source_type;
	typedef basic_valued_agent<source_type,_time_type> source_agent_type;
	typedef boost::shared_ptr<source_agent_type> source_agent_ptr;

	sequential_operator() : m_has_initial(false) {}

	virtual ~sequential_operator() {}

	void setup(double_agent_ptr _source); //! Setup without initial value for the source (assumes constant)
	void setup(double_agent_ptr _source,const source_type& _initial); //! Setup with a value for source at t-1

	operation_type& get_operation() { return m_op; }

protected:
	operation_type m_op;
	source_type m_last,m_initial;
	bool m_has_initial;

	virtual void reset_impl();
	virtual bool update_impl();
};



//! Binary operator
template
<
	typename _op_type,							//!< Operation type
	typename _time_type = default_time_type,	//!< Time type
	typename _duration_type = _time_type		//!< Time interval type
>
class binary_operator
:	public multi_agent_impl
	<
		typename _op_type::target_type,
		_time_type,
		boost::shared_ptr<basic_valued_agent<typename _op_type::source_type,_time_type> >,
		_duration_type
	>
{
public:
	typedef _op_type operation_type;
	typedef typename operation_type::source_type source_type;
	typedef typename operation_type::target_type target_type;
	typedef basic_valued_agent<source_type,_time_type> source_agent_type;
	typedef boost::shared_ptr<source_agent_type> source_agent_ptr;
	typedef multi_agent_impl<target_type,_time_type,source_agent_ptr,_duration_type> parent_type;

	virtual ~binary_operator() {}

	operation_type& get_operation() { return m_op; }

protected:
	operation_type m_op;

	virtual void init_impl();
	virtual void reset_impl() { update_impl(); }
	virtual bool update_impl();
};



template<typename _op_type,typename _time_type,typename _duration_type>
void cumulative_operator<_op_type,_time_type,_duration_type>::reset_impl()
{
	this->m_state = this->m_linked_policy.linked()->state();
}

template<typename _op_type,typename _time_type,typename _duration_type>
bool cumulative_operator<_op_type,_time_type,_duration_type>::update_impl()
{
	this->m_state = m_op(this->m_linked_policy.linked()->state(),this->m_state);
	return this->m_linked_policy.linked()->is_live();
}



template<typename _op_type,typename _time_type,typename _duration_type>
void sequential_operator<_op_type,_time_type,_duration_type>::setup(double_agent_ptr _source)
{
	this->connect(_source);
	m_has_initial = false;
}


template<typename _op_type,typename _time_type,typename _duration_type>
void sequential_operator<_op_type,_time_type,_duration_type>::setup(double_agent_ptr _source,const source_type& _initial)
{
	this->connect(_source);
	m_has_initial = true;
	m_initial = _initial;
}


template<typename _op_type,typename _time_type,typename _duration_type>
void sequential_operator<_op_type,_time_type,_duration_type>::reset_impl()
{
	m_last = m_has_initial? m_initial : this->m_linked_policy.linked()->state();
	update_impl();
}


template<typename _op_type,typename _time_type,typename _duration_type>
bool sequential_operator<_op_type,_time_type,_duration_type>::update_impl()
{
	this->m_state = m_op(this->m_linked_policy.linked()->state(),m_last);
	m_last = this->m_linked_policy.linked()->state();
	return this->m_linked_policy.linked()->is_live();
}


template<typename _op_type,typename _time_type,typename _duration_type>
void binary_operator<_op_type,_time_type,_duration_type>::init_impl()
{
	if (this->m_linked_policy.count_connected() < 1)
		throw error("binary_operator requires at least 1 underlying agent");
}


template<typename _op_type,typename _time_type,typename _duration_type>
bool binary_operator<_op_type,_time_type,_duration_type>::update_impl()
{
	typedef typename parent_type::linked_policy::linked_list_type::iterator iterator;
	iterator itr = this->linked().begin();
	iterator end = this->linked().end();

	bool s = (*itr)->is_live();
	this->m_state = (*itr)->state();

	for(++itr; itr != end; ++itr)
	{
		s = s || (*itr)->is_live();
		this->m_state = m_op(this->m_state,(*itr)->state());
	}

	return s;
}



namespace operators {


//! Operator base class defines the source(s) and target type. All sources must be of the same type
template<
	typename _source,
	typename _target
>
struct basic_operator
{
	typedef _source source_type;
	typedef _target target_type;
};



//////////////////////////////////////////////////////
// Double precision operators
//////////////////////////////////////////////////////

//! Null operation
struct nop : public basic_operator<double,double>
{
	double operator() (double _a) const { return _a; }
};

//! Return 1/a
struct invert : public basic_operator<double,double>
{
	double operator() (double _a) const { return 1. / _a; }
};

//! Return -a
struct negate : public basic_operator<double,double>
{
	double operator() (double _a) const { return -_a; }
};

//! Return abs(a)
struct absolute_value : public basic_operator<double,double>
{
	double operator() (double _a) const { return std::abs(_a); }
};

//! Square root
struct square_root : public basic_operator<double,double>
{
	double operator() (double _a) const { return std::sqrt(_a); }
};

//! Return exp(a)
struct natural_exponent : public basic_operator<double,double>
{
	double operator() (double _a) const { return std::exp(_a); }
};

//! Return ln(a)
struct natural_logarithm : public basic_operator<double,double>
{
	double operator() (double _a) const { return std::log(_a); }
};

//! Distribution function
template<typename _dist_type>
struct distribution : public basic_operator<double,double>
{
	_dist_type& get_distribution() { return m_dist; }

	double operator() (double _a) const { return boost::math::cdf(m_dist,_a); }

protected:
	_dist_type m_dist;
};

//! Inverse distribution function
template<typename _dist_type>
struct inverse_distribution : public distribution<_dist_type>
{
	double operator() (double _a) const { return boost::math::quantile(this->m_dist,_a); }
};

//! Return F.a, where F is a pre-defined factor
struct factor : public basic_operator<double,double>
{
	void setup(double _factor) { m_factor = _factor; }
	double operator() (double _a) const { return m_factor * _a; }

private:
	double m_factor;
};

//! Return a+b
struct sum : public basic_operator<double,double>
{
	double operator() (double _a,double _b) const { return _a + _b; }
};

//! Return a.b
struct product : public basic_operator<double,double>
{
	double operator() (double _a,double _b) const { return _a * _b; }
};

//! Return a-b
struct difference : public basic_operator<double,double>
{
	double operator() (double _a,double _b) const { return _a - _b; }
};

//! Return a/b
struct division : public basic_operator<double,double>
{
	double operator() (double _a,double _b) const { return _a / _b; }
};

//! Return integer division modulos 
struct modulus : public basic_operator<int,double>
{
	double operator() (int _a,int _b) const { return _a%_b; }
};

//! Return a^b
struct power : public basic_operator<double,double>
{
	double operator() (double _a,double _b) const { return std::pow(_a,_b); }
};

//! Return max(a,b) or or max(a,b,c)
struct maximum : public basic_operator<double,double>
{
	double operator() (double _a,double _b) const { return std::max(_a,_b); }
};

//! Return min(a,b) or min(a,b,c)
struct minimum : public basic_operator<double,double>
{
	double operator() (double _a,double _b) const { return std::min(_a,_b); }
};



//////////////////////////////////////////////////////
// Unsigned integer operators
//////////////////////////////////////////////////////

//! Return a+b
struct unsigned_sum : public basic_operator<size_type,size_type>
{
	size_type operator() (size_type _a,size_type _b) const { return _a + _b; }
};

//! Return a.b
struct unsigned_product : public basic_operator<size_type,size_type>
{
	size_type operator() (size_type _a,size_type _b) const { return _a * _b; }
};

//! Return a-b
struct unsigned_difference : public basic_operator<size_type,size_type>
{
	size_type operator() (size_type _a,size_type _b) const { return _a - _b; }
};

//! Return a/b
struct unsigned_division : public basic_operator<size_type,size_type>
{
	size_type operator() (size_type _a,size_type _b) const { return _a / _b; }
};




//////////////////////////////////////////////////////
// Logical operators
//////////////////////////////////////////////////////


//! Logical not
/*struct not : public basic_operator<bool,bool>
{
	double operator() (bool _a) const { return (!_a); }
};*/

struct lnot : public basic_operator<bool,bool>
{
	double operator() (bool _a) const { return (!_a); }
};


//! Logical and
/*struct and : public basic_operator<bool,bool>
{
	double operator() (bool _a,bool _b) const { return (_a && _b); }
};*/

struct land : public basic_operator<bool,bool>
{
	double operator() (bool _a,bool _b) const { return (_a && _b); }
};


//! Logical or
/*struct or : public basic_operator<bool,bool>
{
	double operator() (bool _a,bool _b) const { return (_a || _b); }
};*/

struct lor : public basic_operator<bool,bool>
{
	double operator() (bool _a,bool _b) const { return (_a || _b); }
};


//////////////////////////////////////////////////////
// Condition operators
//////////////////////////////////////////////////////

struct greater_than : public basic_operator<double,bool>
{
	bool operator() (double _a,double _b) const { return (_a > _b + math::TINY); }
};

struct greater_or_equal : public basic_operator<double,bool>
{
	bool operator() (double _a,double _b) const { return (_a >= _b + math::TINY); }
};

struct less_than : public basic_operator<double,bool>
{
	bool operator() (double _a,double _b) const { return (_a < _b - math::TINY); }
};

struct less_or_equal : public basic_operator<double,bool>
{
	bool operator() (double _a,double _b) const { return (_a <= _b - math::TINY); }
};

struct equal : public basic_operator<double,bool>
{
	bool operator() (double _a,double _b) const
	{
		double c = _a - _b;
		return (c < math::TINY && c > -math::TINY);
	}
};

struct not_equal : public basic_operator<double,bool>
{
	bool operator() (double _a,double _b) const
	{
		double c = _a - _b;
		return (c >= math::TINY || c <= -math::TINY);
	}
};

struct logical_equal : public basic_operator<bool,bool>
{
	bool operator() (bool _a,bool _b) const
	{
		return (_a == _b);
	}
};

struct logical_not_equal : public basic_operator<bool,bool>
{
	bool operator() (bool _a,bool _b) const
	{
		return (_a != _b);
	}
};

} // namespace operators
} // namespace simulate
} // namespace fbox

#endif
