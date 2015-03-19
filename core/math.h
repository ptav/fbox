#ifndef __FBOX_CORE_MATH_H__
#define __FBOX_CORE_MATH_H__
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Math utilities
*/

#include "main.h"
#include <cmath>
#include <vector>

namespace fbox {
namespace math {


//! Very small double number (for floating point comparisons)
extern const double TINY;


//! True if value is NAN
inline int is_nan(double _v)
{
	#ifdef _MSC_VER
		return _isnan(_v);
	#else
		return std::isnan(_v);
	#endif
}


//! True if value is INF
inline bool is_infinity(double _v)
{
	#ifdef _MSC_VER
		return !_finite(_v);
	#else
		return std::isinf(_v);
	#endif
}




//! Floating point comparison
inline bool is_zero(double _v)
{
	return (_v < TINY && _v > -TINY);
}


//! Round float to integer
inline int round(float _param)
{
	#ifdef _MSC_VER
		//! Round float to integer. By S. Hughes (http://www.codeproject.com/KB/cpp/floatutils.aspx)
	    // Uses the FloatToInt functionality
	    int a;
	    int *int_pointer = &a;

	    __asm  fld  _param
	    __asm  mov  edx,int_pointer
	    __asm  FRNDINT
	    __asm  fistp dword ptr [edx];

	    return a;

	#else
		return (int) lround(_param);

	#endif
}


//! Round float to arbitrary precision. By S. Hughes (http://www.codeproject.com/KB/cpp/floatutils.aspx)
float round(const float& _number,const int _digits);


//! Round double to arbitrary precision. By S. Hughes (http://www.codeproject.com/KB/cpp/floatutils.aspx)
double round(double _value,int _digits);


//! Rounds float to significant figures. By S. Hughes (http://www.codeproject.com/KB/cpp/floatutils.aspx)
/*!
	Examples
		SigFig(1.23456, 2)        equals 1.2
		SigFig(1.23456e-10, 2)    equals 1.2e-10
		SigFig(1.23456, 5)        equals 1.2346
		SigFig(1.23456e-10, 5)    equals 1.2346e-10
		SigFig(0.000123456, 2)    equals 0.00012
*/
float sig_digits(float _value, int _digits);



//! Floating point comparison
inline bool is_close(double _v1,double _v2,double _tol=TINY)
{
	return (_v2 < _v1 + _tol && _v2 > _v1 - _tol);
}


//! Floating point comparison
inline bool is_greater(double _v1,double _v2,double _tol=TINY)
{
	return (_v2 > _v1 + _tol);
}


//! Floating point comparison
inline bool is_less(double _v1,double _v2,double _tol=TINY)
{
	return (_v2 < _v1 - _tol);
}


//! Find element in a sorted vector.
template<
	typename _container,	//!< Container type. Must support operator[]  and size() operations
	typename _element>		//!< Element type
int sorted_find(
	const _container& v,	//!< Vector to search in
	const _element& y)		//!< Value to find
{
	int _x;
	int x0(0);
	int x1(v.size()-1);

	_element _y;
	_element y0(v[x0]);
	_element y1(v[x1]);

	if (y < y0) return -1;
	if (y >= y1) return x1;

	while( x1 - x0 > 1) {
		_x = (x0 + x1) >> 1;
		_y = v[_x];
		if ( y < _y ) x1=_x, y1=_y;
		else x0=_x, y0=_y;
	}

	return x0;
}	


//! Construct an histogram of std::vector data
void histogram(
	const std::vector<double>& bins,	//!< Upper end of each bucket
	const std::vector<double>& series,	//!< Input data
	std::vector<double>& out,		//!< Output histogram. One element longer than input bins because excess observations are bucketed into an extra bucket
	const std::vector<double>& weights = std::vector<double>());		//!< Optional weights



} // namespace math
} // namespace fbox

#endif
