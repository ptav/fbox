/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Tests system utilities
*/

#include <iostream>
#include <boost/test/unit_test.hpp>
#include "../factory.h"

struct a
{
	virtual ~a() {}
	virtual char chk() const { return 'a'; }
};

struct b : public a
{
	virtual char chk() const { return 'b'; }
};

struct c : public a
{
	virtual char chk() const { return 'c'; }
};


BOOST_AUTO_TEST_CASE(test_factory)
{
	BOOST_MESSAGE("Test non-invasive factory");

	fbox::factory<a> f;

	f.insert<b>("b");
	f.insert<c>("c");
	BOOST_CHECK_EQUAL(f.object_count(),2);

	std::auto_ptr<a> ptr;

	ptr = f.create("b");
	BOOST_CHECK_EQUAL(ptr->chk(),'b');

	ptr = f.create("c");
	BOOST_CHECK_EQUAL(ptr->chk(),'c');

	ptr = f.create("d");
	BOOST_CHECK_EQUAL(ptr.get(),(a*)0);
}
