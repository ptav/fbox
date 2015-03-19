/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Global definitions.
*/


#include "main.h"
#include "date.h"

namespace fbox {

/*!
	\page history Version History

	2.3
	Added local key policy to license manager class
	Added hex/string conversions
	Several fixes and minor extensions

	2.2
	History is not yet available for this and prior versions
*/

void version(size_type& _major,size_type& _minor)
{
	_major = 2;
	_minor = 3;
}


boost::gregorian::date build_date(const char* _date)
{
	std::string str(_date);
	std::string day(str.substr(4,2));
	std::string month(str.substr(0,3));
	std::string year(str.substr(7,4));
	std::string sp("-");
	return boost::gregorian::from_string(year+sp+month+sp+day);
}


std::string build_tag(bool _include_date)
{
	size_type a,b;
	version(a,b);

	std::stringstream strm;
	strm << a << '.' << b;
	
	if (_include_date)
		strm << '.' << boost::gregorian::to_iso_string( build_date() );

	#ifdef _DEBUG
		strm << "dbg";
	#endif

	return strm.str();
}


std::string copyright()
{
	return "FBox v" + build_tag() + ", Copyright 1998-2009 Pedro A. C. Tavares, all rights reserved";
}


} // namespace fbox
