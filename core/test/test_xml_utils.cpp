/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Tests of XML utilities
*/

#include <boost/test/unit_test.hpp>
#include "../xml_utils.h"

using namespace fbox::xml;


BOOST_AUTO_TEST_CASE(test_make_xml)
{
	make_xml xml;

	xml.open_branch("head1");
		xml.value("direction",1);
		xml.value("asset",100.3421);
		xml.value("cash",1.43);

		xml.open_branch("head2");
			xml.open_branch("signal");
				xml.value("type","test");
			xml.close_branch();
		xml.close_branch();

		xml.open_branch("head3");
			xml.attribute("attr1","12");
			xml.open_branch("head4");
				xml.value("type","test2");
			xml.close_branch();
		xml.close_branch();

	xml.close_branch();

	//xml.print(std::cout,true);

	std::stringstream stream;
	xml.print(stream,false);

	std::string str("<?xml version=\"1.0\" ?><head1><direction>1</direction><asset>100.342</asset><cash>1.43</cash><head2><signal><type>test</type></signal></head2><head3 attr1=\"12\"><head4><type>test2</type></head4></head3></head1>");
	BOOST_CHECK_EQUAL(str.c_str(),stream.str().c_str());
}
