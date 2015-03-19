/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Test soft_ptr
*/

#include <boost/test/unit_test.hpp>
#include "../soft_ptr.h"


BOOST_AUTO_TEST_CASE(test_soft_ptr)
{
	using namespace fbox;

	BOOST_MESSAGE("Test soft_ptr");

	soft_ptr<int> s1;
	BOOST_CHECK(!s1.is_set());
	BOOST_CHECK(!s1);

	int* p1 = new int;
	s1 = p1;
	BOOST_CHECK_EQUAL(s1.get(),p1);
	BOOST_CHECK(s1.is_set());
	BOOST_CHECK(s1);

	// copy not owned
	soft_ptr<int> s3;
	s3 = p1;
	BOOST_CHECK_EQUAL(s3.get(),p1);
	BOOST_CHECK(s3.is_set());
	BOOST_CHECK(s3);

	// copy not owned using self copy ctor
	s3 = s1;
	BOOST_CHECK_EQUAL(s3.get(),p1);
	BOOST_CHECK(s3.is_set());
	BOOST_CHECK(s3);

	delete p1; // prevent memory leak
}


BOOST_AUTO_TEST_CASE(test_flex_ptr)
{
	using namespace fbox;

	BOOST_MESSAGE("Test flex_ptr");

	flex_ptr<int> s1;
	BOOST_CHECK(!s1.is_owner());
	BOOST_CHECK(!s1.is_set());
	BOOST_CHECK(!s1);

	// not owned
	int* p1 = new int;
	s1 = p1;
	BOOST_CHECK_EQUAL(s1.get(),p1);
	BOOST_CHECK(!s1.is_owner());
	BOOST_CHECK(s1.is_set());
	BOOST_CHECK(s1);

	// owned
	flex_ptr<int> s2(new int,true); // ownership will prevent memory leak
	BOOST_CHECK(s2.is_owner());
	BOOST_CHECK(s2.is_set());
	BOOST_CHECK(s2);

	// copy not owned
	flex_ptr<int> s3;
	s3 = p1;
	BOOST_CHECK_EQUAL(s3.get(),p1);
	BOOST_CHECK(!s3.is_owner());
	BOOST_CHECK(s3.is_set());
	BOOST_CHECK(s3);

	// copy not owned using self copy ctor
	s3 = s1;
	BOOST_CHECK_EQUAL(s3.get(),p1);
	BOOST_CHECK(!s3.is_owner());
	BOOST_CHECK(s3.is_set());
	BOOST_CHECK(s3);

	// copy owned (s3 will not own pointer as it remains owned by s2)
	s3 = s2;
	BOOST_CHECK_EQUAL(s3.get(),s2.get());
	BOOST_CHECK(!s3.is_owner());
	BOOST_CHECK(s3.is_set());
	BOOST_CHECK(s3);

	delete p1; // prevent memory leak
}
