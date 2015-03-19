#ifndef __FBOX_CORE_MAIN_H__
#define __FBOX_CORE_MAIN_H__
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Global definitions.
*/


#include <exception>
#include <string>
#include <memory>
#include <typeinfo>
#include <boost/date_time/gregorian/gregorian.hpp>


#ifdef _MSC_VER
	// disable warnings
	// C4503: long truncated debug names
	// C4099: *.pdb was not found for dependency
	// C4996: libc functions reported as 'unsafe' by VC9
	#pragma warning(disable: 4503 4099 4996)

	// undef max and min defined by windows headers
	#undef max
	#undef min
#endif


#ifdef _DEBUG
	#define FBOX_DEBUG
#endif

// Link to static Boost libraries
#define USE_STATIC_BOOST


namespace fbox {

//! Export/import define (empty since we are not building windows dlls)
#define FBOX_DLL


//! Shorthand for frequent indexation type
typedef unsigned int size_type;


//! Return library version number
void version(size_type& _major,size_type& _minor);


//! Return build date given a __DATE__ style input string
boost::gregorian::date build_date(const char* _date=__DATE__);


//! Return build tag (major.minor.[date])
std::string build_tag(bool _include_date=true);


//! Return copyright message
std::string copyright();


//! Stringify type name
template<typename _type>
std::string type_id(const _type& _ptr,bool _remove_scope=false)
{
	std::string str( typeid(_ptr).name() );

	std::string::size_type n;
	if (_remove_scope)
	{
		std::string::size_type n1;
		while(std::string::npos != (n = str.find(':')))
		{
			n1 = str.find_last_of(" ,<",n,3);
			str.erase(n1+1,n-n1+1);
		}
	}

	while(std::string::npos != (n = str.find("class ")))
		str.erase(n,6);

	return str;
}


//! Null output stream (equivalent to output to /dev/null)
class null_stream : public std::ostream
{
public:
	null_stream() : std::ostream(0) {}
	null_stream(null_stream&) : std::ostream(0) {}

	template<typename _type>
	null_stream& operator<<(const _type&) { return *this; } 

	bool good() { return true; } 
};


} // namespace fbox

#endif
