#ifndef __FBOX_CORE_INTEGRATOR_H__
#define __FBOX_CORE_INTEGRATOR_H__
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Integration utilities
*/

#include "main.h"
#include <vector>

namespace fbox {
namespace math {


//! 10-point Gauss-Legendre integration based on <code>qgaus</code> (Numerical Recipes in C, 2nd ed, p 148)
class gauss_legendre10
{
public:
	template<typename _func>
	double operator() (
		double _x0,			//!< Lower limit
		double _x1,			//!< Upper limit
		_func& _function)	//!< Integrand
		const
	{
		int j;
		double xr,xm,dx,s;
		static double x[]={0.0,0.1488743389,0.4333953941,
			0.6794095682,0.8650633666,0.9739065285};
		static double w[]={0.0,0.2955242247,0.2692667193,
			0.2190863625,0.1494513491,0.0666713443};

		xm = 0.5 * (_x1 + _x0);
		xr = 0.5 * (_x1 - _x0);

		s  = 0;
		for (j=1;j<=5;j++) 
		{
			dx = xr * x[j];
			s += w[j] * (_function(xm+dx) + _function(xm-dx));
		}
		
		return (s *= xr);
	}
};


} // namespace math
} // namespace fbox

#endif
