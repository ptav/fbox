#ifndef __FBOX_CORE_STRING_UTILS_H__
#define __FBOX_CORE_STRING_UTILS_H__
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	String and character utilities.
*/


#include "main.h"
#include <boost/algorithm/string.hpp>
#include <string.h>

namespace fbox {

//! convert a string in-situ to upper case characters
inline void upper_case(std::string& _str)
{
	boost::to_upper(_str);
}


//! convert a string in-situ to lower case characters
inline void lower_case(std::string& _str)
{
	boost::to_lower(_str);
}


//! Trim padding characters from the right
void trim_right(std::string& _str,char _char=' ');


//! Trim padding characters from the left
void trim_left(std::string& _str,char _char=' ');


//! Trim padding characters on both ends
inline void trim(std::string& _str,char _char=' ')
{
	trim_right(_str,_char);
	trim_left(_str,_char);
}


//! Replace all occurrences of a character with another
void replace(std::string& _str,char _from,char _to);


//! Replace all occurrences of a string with another
void replace(std::string& _str,const std::string& _from,const std::string& _to);


//! Load the full stream information into a string
void quick_load_stream(std::istream& _in,std::string& _str);


//! Convert a character into a 4-character string hexadecimal representation (new characters are appended to _str)
void char_to_hex(char _ch,std::string& _str);


//! Convert a character based hexadecimal sequence into a string of characters (string version)
void hex_to_string(const std::string& _str,std::string& _out);


//! Convert a character based hexadecimal sequence into a string of characters (stream version)
void hex_to_string(std::istream& _strm,std::string& _out);

} // namespace fbox

#endif
