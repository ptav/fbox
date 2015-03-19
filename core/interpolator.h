#ifndef __FBOX_CORE_INTERPOLATOR_H__
#define __FBOX_CORE_INTERPOLATOR_H__
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Interpolation utilities
*/

#include "main.h"
#include "math.h"
#include <map>
#include <vector>

namespace fbox {
namespace math {

//! Define point and table types for tabulated interpolators
class table_interpolator_impl
{
public:
	typedef std::pair<double,double> point_type;
	typedef std::vector<point_type> table_type;

	table_interpolator_impl(const table_type& _table)
	:	mr_table(_table) {}

protected:
	const table_type& mr_table;
};


struct right_continuous
{
	static double interp(double x,double x0,double x1,double y0,double y1)
	{
		if (x < x1 - TINY) return y0;
		else return y1;
	}
};


struct left_continuous
{
	static double interp(double x,double x0,double x1,double y0,double y1)
	{
		if (x > x0 + TINY) return y1;
		else return y0;
	}
};


//! Piece-wise constant interpolation operator
template<typename _dir>
class constant_interpolator : public table_interpolator_impl
{
public:
	constant_interpolator(const table_type& _table)
	:	table_interpolator_impl(_table) {}

	double operator() (double _x) const; // interpolation
};


//! Linear interpolation operator
class linear_interpolator : public table_interpolator_impl
{
public:
	linear_interpolator(const table_type& _table)
	:	table_interpolator_impl(_table) {}

	double operator() (double _x) const;

private:
	double interp1(double x,double x0,double x1,double y0,double y1) const;
};


//! Cubic spline interpolation operator.  Based on "spline" (Numerical Recipes in C, 2nd ed, p 114).
class cubic_spline_interpolator : public table_interpolator_impl
{
public:
	cubic_spline_interpolator(
		const table_type& _table,	//!< Data table
		double _start = 1e30,		//!< Starting 1st derivative
		double _end = 1e30);		//!< Ending 1st derivative

	double operator() (double _x) const;

private:
	std::vector<double> m_deriv2;
	double m_start,m_end;

	void check_table();
	void make_spline();
};


//! adapt table for use in sorted_find
struct sorted_find_adaptor
{
	const table_interpolator_impl::table_type& mr_table;
	sorted_find_adaptor(const table_interpolator_impl::table_type& _table) : mr_table(_table) {}
	double operator[] (size_type _i) const { return mr_table[_i].first; }
	size_type size() const { return mr_table.size(); }
};



////////////////////////////////////////
// constant_interpolator
////////////////////////////////////////

template<typename _dir>
double constant_interpolator<_dir>::operator() (double _x) const
{
	int i0,i1;
	double v;

	// check bounds
	if (_x <= mr_table.front().first)
		v = mr_table.front().second;
	else if (_x >= mr_table.back().first) 
		v = mr_table.back().second;
	else
	{
		sorted_find_adaptor adapt(mr_table);
		i0 = sorted_find(adapt,_x);
		i1 = i0 + 1;
		v = _dir::interp(_x,mr_table[i0].first,mr_table[i1].first,mr_table[i0].second,mr_table[i1].second);
	}

	return v;
}



} // namespace math
} // namespace fbox

#endif
