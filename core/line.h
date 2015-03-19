#ifndef __FBOX_CORE_LINE_H__
#define __FBOX_CORE_LINE_H__
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Line objects
*/

#include "main.h"
#include "error.h"
#include "interpolator.h"
#include "integrator.h"
#include <vector>

namespace fbox {
namespace math {


//! 2D line interface interface
class line
{
public:
	//! Interpolate point
	virtual double operator() (double _x) const = 0;

	//! Integrate line between two points
	virtual double integral(double _x0,double _x1) const = 0;

	//! Integrate line between two points weighted by second curve
	virtual double integral(double _x0,double _x1,const line& _weights) const = 0;
};


//! Constant line
class flat_line : public line
{
public:
	flat_line(double _y=0) : m_y(_y) {}
	flat_line(const flat_line& _line) : m_y(_line.m_y) {}

	void setup(double _y) { m_y = _y; }

	virtual double operator() (double _x) const  { return m_y; }
	virtual double integral(double _x0,double _x1) const { return m_y * (_x1 - _x0); }
	virtual double integral(double _x0,double _x1,const line& _weights) const { return m_y * _weights.integral(_x0,_x1); }

protected:
	double m_y;
};


//! Constant line
template<
	typename _interp,	//!< Interpolator method 
	typename _integ>	//!< Integrator method
class interpolated_line : public line
{
public:
	typedef _interp interpolator_type;
	typedef _integ integrator_type;
	typedef typename interpolator_type::point_type point_type;
	typedef typename interpolator_type::table_type table_type;

	interpolated_line() : mp_interp(0),m_update(true) {}
	
	interpolated_line(const table_type& _table) 
	:	m_table(_table),mp_interp(0),m_update(true) {}

	~interpolated_line()
	{
		delete mp_interp;
	}

	template<typename _interp2,typename _integ2>
	interpolated_line(const interpolated_line<_interp2,_integ2>& _line)
	:	m_table(_line.m_table),m_update(true),mp_interp(0) {}

	template<typename _interp2,typename _integ2>
	interpolated_line& operator=(const interpolated_line<_interp2,_integ2>& _line)
	{
		if (this != &_line)
		{
			m_table = _line.m_table;
			m_update = true;
		}	
		return *this;
	}

	const table_type& table() const { return m_table; }

	void clear() { m_table.clear(); m_update = true; }
	interpolated_line& add(double x,double y) { m_table.push_back(point_type(x,y)); m_update = true; return *this; }

	virtual double operator() (double _x) const;
	virtual double integral(double _x0,double _x1) const;
	virtual double integral(double _x0,double _x1,const line& _weights) const;

protected:
	table_type m_table;

private:
	mutable interpolator_type* mp_interp;
	mutable bool m_update;
};


//! Shorthand for piece-wise right-continuous constant line with 10-point integration capability
typedef interpolated_line<constant_interpolator<left_continuous>,gauss_legendre10> left_constant_line;

//! Shorthand for piece-wise left-continuous constant line with 10-point integration capability
typedef interpolated_line<constant_interpolator<right_continuous>,gauss_legendre10> right_constant_line;

//! Shorthand for linear line with 10-point integration capability
typedef interpolated_line<linear_interpolator,gauss_legendre10> linear_line;

//! Shorthand for cubic_spline line with 10-point integration capability
typedef interpolated_line<cubic_spline_interpolator,gauss_legendre10> cspline_line;



////////////////////////////////////////
// interpolated_line
////////////////////////////////////////

template<typename _interp,typename _integ>
double interpolated_line<_interp,_integ>::operator() (double _x) const
{
	if (m_update)
	{
		if (!m_table.size()) throw error("Empty table in interpolated_line");

		delete mp_interp;
		mp_interp = new interpolator_type(m_table);
		m_update = false;
	}

	return (*mp_interp)(_x);
}


template<typename _interp,typename _integ>
double interpolated_line<_interp,_integ>::integral(double _x0,double _x1) const
{
	integrator_type i;
	return i(_x0,_x1,*this);
}


struct __prod
{
	const line &l0,&l1;
	__prod(const line& _l0,const line& _l1) : l0(_l0),l1(_l1) {}
	double operator() (double _x) { return l0(_x) * l1(_x); }
};

template<typename _interp,typename _integ>
double interpolated_line<_interp,_integ>::integral(double _x0,double _x1,const line& _weights) const
{
	integrator_type i;
	__prod func(*this,_weights);
	return i(_x0,_x1,func);
}


} // namespace math
} // namespace fbox

#endif
