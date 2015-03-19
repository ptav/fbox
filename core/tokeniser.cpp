/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	String utilities
*/

#include "tokeniser.h"
#include "error.h"
#include <sstream>
#include <stdio.h>

namespace fbox {

tokeniser::tokeniser()
:	mp_str(0),
	mp_buf(0),
	m_divider(" \t\n"),
	m_escape('\\'),
	m_good(false)
{}


tokeniser::tokeniser(
	std::istream& _in,
	const std::string& _dividers,
	const std::string& _lgroup,
	const std::string& _rgroup,
	char _escape)
:	mp_str(&_in),
	mp_buf(_in.rdbuf()),
	m_divider(_dividers),
	m_lgroup(_lgroup),
	m_rgroup(_rgroup),
	m_escape(_escape),
	m_good(_in.good())
{
	if (m_lgroup.size() != m_rgroup.size()) 
		throw error("lgroup and sgroup must be of equal size in tokeniser ctor"); 
}


void tokeniser::set_input(std::istream& _in)
{
	mp_str = &_in;
	mp_buf = _in.rdbuf();
	m_good = _in.good();
}


void tokeniser::set_group(const std::string& _lgroup,const std::string& _rgroup)
{
	m_lgroup = _lgroup;
	m_rgroup = _rgroup;
}


void tokeniser::set_divider(const std::string& _divider)
{
	m_divider = _divider;
}


void tokeniser::set_escape(char _escape)
{
	m_escape = _escape;
}


bool tokeniser::seek(char _end,std::string& _out,bool _append)
{
	return seek_impl(_end,&_out,_append);
}


bool tokeniser::seek(char _end)
{
	return seek_impl(_end,0,false);
}


char tokeniser::next(
	std::string& _out,
	bool _append)
{
	using std::string;

	if (!_append) _out.clear();
	
	char c = mp_buf->sbumpc();
	while (c != EOF && is_divider(c)) c = mp_buf->sbumpc();

	size_type i;
	bool escon = false;
	while (c != EOF) 
	{
		if (escon)
		{
			escon = false;
			_out += c;
		}
		else
		{
			if (c == m_escape)			// escape character
				escon = true;
			else if (is_divider(c))		// divider found
				return c;
			else if (string::npos != (i = m_lgroup.find(c))) // find end of group (no nesting allowed just yet)
				seek(m_rgroup[i],_out,true);
			else
				_out += c;
		}

		c = mp_buf->sbumpc();
	}

	m_good = false;
	mp_str->get(); // force read to ensure return stream state is not 'good'
	return 0;
}


bool tokeniser::seek_impl(char _end,std::string* _out,bool _append)
{
	if (_out && !_append) _out->clear();
	
	bool escon = false;
	char c = mp_buf->sbumpc();

	while (c != EOF) 
	{
		if (escon)
		{
			escon = false;
			if (_out) (*_out) += c;
		}
		else
		{
			if (c == m_escape)	// escape character
				escon = true;
			else if (c == _end)	// found target
				return true;
			else				// any other case
				if (_out) (*_out) += c;
		}

		c = mp_buf->sbumpc();
	}

	m_good = false;
	mp_str->get(); // force read to ensure return stream state is not 'good'
	return false;
}


bool tokeniser::is_divider(char _c)
{
	return ( std::string::npos != m_divider.find(_c) );
}


void get_token(
	const std::string& in,
	std::string& token,
	std::string& args,
	char lpar,
	char rpar)
{
	using std::string;

	args.clear();

	string::size_type i0 = in.find(lpar);
	string::size_type i1 = in.rfind(lpar);
	if (i0 != string::npos && i1 != string::npos)
	{
		token = in.substr(0,i1-1);
		args = in.substr(i0+1,i1-i0-2);
	}
	else
	{
		token = in;
	}
}


} // namespace fbox


