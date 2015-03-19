/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Basic financial options
*/


#include "main.h"
#include "math.h"
#include "error.h"
#include <boost/math/distributions/normal.hpp>


namespace fbox {
namespace finance {

double option_intrinsic(
	char _call_put,
	double _strike,
	double _forward,
	double _discount_factor)
{
	switch (_call_put)
	{
		case 'C': case 'c': return _discount_factor * std::max(_forward - _strike, 0.);
		case 'P': case 'p': return _discount_factor * std::max(_strike - _forward, 0.);
		default: throw error("Invalid option type (c/p)");
	}
}


	
double black_scholes(
	char _call_put,
	double _strike,
	double _maturity,
	double _forward,
	double _volatility,
	double _discount_factor)
{
	if (_volatility * _maturity < fbox::math::TINY)
		option_intrinsic(_call_put,_strike,_forward,_discount_factor);

	using boost::math::normal;
	normal s;

	double tt = std::sqrt(_maturity);
	double d1 = (std::log(_forward / _strike) + _volatility * _volatility * _maturity  / 2.) / (_volatility * tt);
	double d2 = d1 - _volatility * tt;

	switch (_call_put)
	{
		case 'C': case 'c':
		return ( _discount_factor * ( _forward * cdf(s,d1) - _strike * cdf(s,d2) ) );

		case 'P': case 'p':
		return ( _discount_factor * ( -_forward * cdf(s,-d1) + _strike * cdf(s,-d2) ) );

		default:
		throw error("Invalid option type (c/p)");
	}
}


double normal_black_scholes(
	char _call_put,
	double _strike,
	double _maturity,
	double _forward,
	double _volatility,
	double _discount_factor)
{
	if (_volatility * _maturity < fbox::math::TINY)
		option_intrinsic(_call_put,_strike,_forward,_discount_factor);

	using boost::math::normal;
	normal s;

	double fk = _forward - _strike;
	double st = _volatility * std::sqrt(_maturity);
	double fkst = fk / st;

	double c = fk * cdf(s,fkst) + st * pdf(s,fkst);
	
	switch (_call_put)
	{
		case 'C': case 'c': return _discount_factor * c;
		case 'P': case 'p': return _discount_factor * (c - _forward + _strike);
		default: throw error("Invalid option type (c/p)");
	}
}


} // namespace finance
} // namespace fbox

