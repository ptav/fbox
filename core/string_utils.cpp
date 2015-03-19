/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	String and character utilities.
*/


#include "string_utils.h"

namespace fbox {

void trim_right(std::string& _str,char _char)
{
	std::string::size_type i = _str.find_last_not_of(_char);
	if (i != std::string::npos) 
		_str.erase(i+1);
	else
		_str.clear();
}


void trim_left(std::string& _str,char _char)
{
	std::string::size_type i = _str.find_first_not_of(_char);
	if (i != std::string::npos)
		_str.erase(0,i);
	else
		_str.clear();
}


void replace(std::string& _str,char _from,char _to)
{
	std::string::size_type n(0);
	while(std::string::npos != (n = _str.find(_from,0))) _str[n] = _to;
}


void replace(std::string& _str,const std::string& _from,const std::string& _to)
{
	size_type sz = _from.length();
	std::string::size_type pos = 0;
	while(std::string::npos != (pos = _str.find(_from,pos)))
	{
		_str.replace(pos,sz,_to);
	}
}


void quick_load_stream(std::istream& _in,std::string& _str)
{
	char c;
	std::streambuf* buf = _in.rdbuf();
	while (EOF != (c = buf->sbumpc()) && _in.good()) _str += c;
}



void char_to_hex(char _ch,std::string& _str)
{
	char b[2];
	b[0] = _ch / 16;
	b[1] = _ch % 16;

	for(size_type i = 0; i != 2; ++i)
	{
		if(b[i] >= 0 && b[i] <= 9)
			_str += '0' + b[i];
		else
			_str += 'A' + (b[i]-10);
	}
}


static char hex_to_char(std::istream& _strm)
{
	char c,out = 0;
	for(size_type i = 0; i != 2; ++i)
	{
		_strm >> c;
		if(c >= '0' && c <= '9')
			out = (out << 4) + (c - '0');
		else if(c >= 'A' && c <= 'F')
			out = (out << 4) + (c - 'A' + 10);
		else
			break;
	}

	return out;
}    

void hex_to_string(std::istream& _strm,std::string& _out)
{
	while (_strm.good()) _out.push_back( hex_to_char(_strm) );
}


void hex_to_string(const std::string& _str,std::string& _out)
{
	std::stringstream strm(_str.c_str());
	hex_to_string(strm,_out);
}



} // namespace fbox
