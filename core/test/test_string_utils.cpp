/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Tests string utilities
*/

#include <boost/test/unit_test.hpp>
#include "../string_utils.h"



BOOST_AUTO_TEST_CASE(test_upper_lower_conversion)
{
	std::string str("This is the Winter of my discontent");
	
	fbox::upper_case(str);
	BOOST_CHECK_EQUAL(str.c_str(),"THIS IS THE WINTER OF MY DISCONTENT");

	fbox::lower_case(str);
	BOOST_CHECK_EQUAL(str.c_str(),"this is the winter of my discontent");
}


BOOST_AUTO_TEST_CASE(test_hex_utils)
{
	std::string base = "0123456789ABCDEF";

	std::string ch,hex;
	for(fbox::size_type i = 0; i < 16; ++i)
	{
		for(fbox::size_type j = 0; j < 16; ++j)
		{
			hex += base[i];
			hex += base[j];
		}
	}

	fbox::hex_to_string(hex,ch);
	for(fbox::size_type i = 0; i < 256; ++i) BOOST_CHECK_EQUAL(ch[i],(char)i);
}


BOOST_AUTO_TEST_CASE(test_string_replace_char)
{
	std::string s1("This is my string");
	fbox::replace(s1,' ','_');
	BOOST_CHECK_EQUAL(s1,"This_is_my_string");

	std::string s2("This is my string ");
	fbox::replace(s2,' ','_');
	BOOST_CHECK_EQUAL(s2,"This_is_my_string_");

	std::string s3(" This is my string");
	fbox::replace(s3,' ','_');
	BOOST_CHECK_EQUAL(s3,"_This_is_my_string");
}


BOOST_AUTO_TEST_CASE(test_string_replace_string)
{
	std::string s = "This is my string";
	fbox::replace(s,"my ","_");
	BOOST_CHECK_EQUAL(s,"This is _string");

	s = "This is my string";
	fbox::replace(s," my ","_*_*_*_");
	BOOST_CHECK_EQUAL(s,"This is_*_*_*_string");

	s = "This is my string";
	fbox::replace(s,"i","--");
	BOOST_CHECK_EQUAL(s,"Th--s --s my str--ng");

	s = "This is my string my";
	fbox::replace(s,"my","--");
	BOOST_CHECK_EQUAL(s,"This is -- string --");
}
