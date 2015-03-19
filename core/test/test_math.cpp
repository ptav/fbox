/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Tests math utilities
*/

#include <iostream>
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include "../math.h"
#include "../option.h"


BOOST_AUTO_TEST_CASE(test_round)
{
	using namespace fbox::math;

	BOOST_CHECK_CLOSE(2.000,round(1.63,0),TINY);
	BOOST_CHECK_CLOSE(1.000,round(1.43,0),TINY);

	// WARNING - checks only first ignored digit and 5 leads to a round up
	BOOST_CHECK_CLOSE(2.000,round(1.53,0),TINY);
	BOOST_CHECK_CLOSE(2.000,round(1.59,0),TINY);
	BOOST_CHECK_CLOSE(2.000,round(1.5,0),TINY);


	BOOST_CHECK_CLOSE(1.500,round(1.5436,1),TINY);
	BOOST_CHECK_CLOSE(1.540,round(1.5436,2),TINY);
	BOOST_CHECK_CLOSE(1.544,round(1.5436,3),TINY);
}


BOOST_AUTO_TEST_CASE(test_black_scholes)
{
	using namespace fbox::math;
	using namespace fbox::finance;

	char cp = 'c';
	double f = 50;
	double x = f;
	double v = 0.2;
	double t = 1.0;
	double d = 1.0;

	// test zero maturity
	BOOST_REQUIRE_SMALL(black_scholes(cp,x,1e-6,20,v,d),1e-5);
	BOOST_REQUIRE_SMALL(black_scholes(cp,x,1e-6,50,v,d),1e-2);
	BOOST_CHECK_CLOSE(50.0,black_scholes(cp,x,1e-6,100,v,d),1e-5);

	// test zero vol
	BOOST_REQUIRE_SMALL(black_scholes(cp,x,t,20,1e-6,d),1e-5);
	BOOST_REQUIRE_SMALL(black_scholes(cp,x,t,50,1e-6,d),1e-2);
	BOOST_CHECK_CLOSE(50.0,black_scholes(cp,x,t,100,1e-6,d),1e-5);
}
