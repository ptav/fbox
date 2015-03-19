#ifndef __FBOX_CORE_OPTION_H__
#define __FBOX_CORE_OPTION_H__
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Basic financial options
*/


#include "main.h"
#include "error.h"


namespace fbox {
namespace finance {

//! Black-Scholes option pricing formula.
double option_intrinsic(
	char _call_put,					//!< Determin if option is a put ('p') or call ('c')
	double _strike,					//!< Option strike
	double _forward,				//!< Forward value of underlying
	double _discount_factor=1.);	//!< Discount factor


//! Black-Scholes option pricing formula.
double black_scholes(
	char _call_put,					//!< Determin if option is a put ('p') or call ('c')
	double _strike,					//!< Option strike
	double _maturity,				//!< Option maturity in years
	double _forward,				//!< Forward value of underlying
	double _volatility,				//!< Volatility of the forward. If <code>_volatility</code>=0 the intrinsic value is calculated.
	double _discount_factor=1.);	//!< Discount factor


//! Price of an option where changes inthe underlying value are normally distributed
double normal_black_scholes(
	char _call_put,					//!< Determin if option is a put ('p') or call ('c')
	double _strike,					//!< Option strike
	double _maturity,				//!< Option maturity in years
	double _forward,				//!< Forward value of underlying
	double _volatility,				//!< Volatility of the forward. If <code>_volatility</code>=0 the intrinsic value is calculated.
	double _discount_factor=1.);	//!< Discount factor


} // namespace finance
} // namespace fbox

#endif
