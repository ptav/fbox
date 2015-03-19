/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Tests of line interpolation
*/

#include <iostream>
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include "../line.h"


BOOST_AUTO_TEST_CASE(test_flat_line)
{
	using namespace fbox::math;

	flat_line l(1.5);

	BOOST_CHECK_CLOSE(1.5,l(-1.0),TINY);
	BOOST_CHECK_CLOSE(1.5,l(+0.0),TINY);
	BOOST_CHECK_CLOSE(1.5,l(+1.0),TINY);
}


BOOST_AUTO_TEST_CASE(test_right_constant_line)
{
	using namespace fbox::math;

	right_constant_line::table_type table;
	table.push_back( right_constant_line::point_type(-2.0,0.0) );
	table.push_back( right_constant_line::point_type(-0.0,5.0) );
	table.push_back( right_constant_line::point_type(+2.0,2.0) );

	right_constant_line l(table);

	BOOST_CHECK_CLOSE(0.0,l(-3.0),TINY);
	BOOST_CHECK_CLOSE(0.0,l(-2.0),TINY);
	BOOST_CHECK_CLOSE(0.0,l(-1.0),TINY);
	BOOST_CHECK_CLOSE(5.0,l(+0.0),TINY);
	BOOST_CHECK_CLOSE(5.0,l(+1.0),TINY);
	BOOST_CHECK_CLOSE(2.0,l(+2.0),TINY);
	BOOST_CHECK_CLOSE(2.0,l(+3.0),TINY);
}


BOOST_AUTO_TEST_CASE(test_left_constant_line)
{
	using namespace fbox::math;

	left_constant_line::table_type table;
	table.push_back( left_constant_line::point_type(-2.0,0.0) );
	table.push_back( left_constant_line::point_type(-0.0,5.0) );
	table.push_back( left_constant_line::point_type(+2.0,2.0) );

	left_constant_line l(table);

	BOOST_CHECK_CLOSE(0.0,l(-3.0),TINY);
	BOOST_CHECK_CLOSE(0.0,l(-2.0),TINY);
	BOOST_CHECK_CLOSE(5.0,l(-1.0),TINY);
	BOOST_CHECK_CLOSE(5.0,l(+0.0),TINY);
	BOOST_CHECK_CLOSE(2.0,l(+1.0),TINY);
	BOOST_CHECK_CLOSE(2.0,l(+2.0),TINY);
	BOOST_CHECK_CLOSE(2.0,l(+3.0),TINY);
}


BOOST_AUTO_TEST_CASE(test_linear_line)
{
	using namespace fbox::math;

	linear_line::table_type table;
	table.push_back( linear_line::point_type(-2.0,0.0) );
	table.push_back( linear_line::point_type(-0.0,5.0) );
	table.push_back( linear_line::point_type(+2.0,2.0) );

	linear_line l(table);

	BOOST_CHECK_CLOSE(2.5,l(-1.0),TINY);
	BOOST_CHECK_CLOSE(5.0,l(+0.0),TINY);
	BOOST_CHECK_CLOSE(3.5,l(+1.0),TINY);
}


BOOST_AUTO_TEST_CASE(test_cs_line)
{
	using namespace fbox::math;

	cspline_line::table_type table;
	table.push_back( cspline_line::point_type(-2.0,0.0) );
	table.push_back( cspline_line::point_type(-0.0,5.0) );
	table.push_back( cspline_line::point_type(+2.0,2.0) );

	cspline_line l(table);

	BOOST_CHECK_CLOSE(3.25,l(-1.0),TINY);
	BOOST_CHECK_CLOSE(5.0,l(+0.0),TINY);
	BOOST_CHECK_CLOSE(4.25,l(+1.0),TINY);


	cspline_line l1;
	l1.add(-2.0,0.0)
	  .add(-0.0,5.0)
	  .add(+2.0,2.0); // use alt syntax

	BOOST_CHECK_CLOSE(3.25,l(-1.0),TINY);
	BOOST_CHECK_CLOSE(5.0,l(+0.0),TINY);
	BOOST_CHECK_CLOSE(4.25,l(+1.0),TINY);

	l1.clear();
	l1.add(-2.0,0.0);
	l1.add(-0.0,5.0);
	l1.add(+2.0,2.0);

	BOOST_CHECK_CLOSE(3.25,l(-1.0),TINY);
	BOOST_CHECK_CLOSE(5.0,l(+0.0),TINY);
	BOOST_CHECK_CLOSE(4.25,l(+1.0),TINY);
}
