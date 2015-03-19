/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Test logger class
*/

#include <iostream>
#include <boost/test/unit_test.hpp>
#include "../logger.h"
#include "../system.h"


BOOST_AUTO_TEST_CASE(test_logger)
{
	using namespace fbox;

	BOOST_MESSAGE("Test logger");

	std::stringstream s;
	std::string o;
	
	logger l(s,false);
	l << "test1\n"; // default message and trigger level is WARNING
	l << logger::FATAL << "test2\n"; // raise message level to ERROR
	l << logger::NOTE << "test3\n"; // lower message level to NOTE
	l.set_trigger_level(logger::NOTE); // lower trigger level to NOTE
	l << logger::NOTE << "test4\n"; // now the NOTE level message will print
	
	s >> o;
	BOOST_CHECK_EQUAL(o,"test1");
	BOOST_CHECK(s.good());

	s >> o;
	BOOST_CHECK_EQUAL(o,"test2");
	BOOST_CHECK(s.good());

	s >> o;
	BOOST_CHECK_EQUAL(o,"test4");
	BOOST_CHECK(s.good());

	s >> o;
	BOOST_CHECK(!s.good());
}


BOOST_AUTO_TEST_CASE(test_file_logger)
{
	using namespace fbox;

	std::string flnm = "c:/local/tmp/fbox.log";
	logger::level_type lv = logger::WARNING;

	logger* ptr = new logger(flnm);
	(*ptr) << "test1\n"; // default message and trigger level is WARNING
	(*ptr) << logger::FATAL << "test2\n"; // raise message level to ERROR
	(*ptr) << logger::NOTE << "test3\n"; // lower message level to NOTE
	(*ptr).set_trigger_level(logger::NOTE); // lower trigger level to NOTE
	(*ptr) << logger::NOTE << "test4\n"; // now the NOTE level message will print
	delete ptr;
}
