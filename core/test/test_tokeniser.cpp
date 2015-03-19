/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Tests of XML utilities
*/

#include <boost/test/unit_test.hpp>
#include "../tokeniser.h"


BOOST_AUTO_TEST_CASE(test_tokeniser)
{
	using namespace fbox;

	std::stringstream str("word1\tword2,\"grouped string\",word\\ 3 \"this is, a \\\" delimited string\",word4\" and grouped string\"");

	tokeniser tok;
	tok.set_input(str);
	tok.set_divider(", \t");
	tok.set_group("\"","\"");

	std::string out;
	BOOST_CHECK_EQUAL('\t',tok.next(out));
	BOOST_CHECK_EQUAL("word1",out); // tab separator

	BOOST_CHECK_EQUAL(',',tok.next(out));
	BOOST_CHECK_EQUAL("word2",out); // separator ','

	BOOST_CHECK_EQUAL(',',tok.next(out));
	BOOST_CHECK_EQUAL("grouped string",out); // group, escape character and space separator

	BOOST_CHECK_EQUAL(' ',tok.next(out));
	BOOST_CHECK_EQUAL("word 3",out); // escape character

	BOOST_CHECK_EQUAL(',',tok.next(out));
	BOOST_CHECK_EQUAL("this is, a \" delimited string",out); // group, escape character and space separator

	BOOST_CHECK_EQUAL(0,tok.next(out)); // check EOF
	BOOST_CHECK_EQUAL("word4 and grouped string",out); // token and (unseparated) group
}
