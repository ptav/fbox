 #ifndef __FBOX_CORE_TOKENISER_H__
#define __FBOX_CORE_TOKENISER_H__
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	String utilities
*/

#include "main.h"
#include <string>
#include <vector>

namespace fbox {


//! Extract string tokens
class tokeniser
{
public:
	tokeniser();

	tokeniser(
		std::istream& _in,							//!< Input stream
		const std::string& _dividers = " \t\n",		//!< Token dividers
		const std::string& _lgroup = std::string(),	//!< Left grouping characters
		const std::string& _rgroup = std::string(),	//!< Right grouping characters (size must match that of _lgroup)
		char _escape = '\\');						//!< Escape character

	//! Set new input stream
	void set_input(std::istream& _in);

	//! Set new left-hand and right-hand grouping characters
	void set_group(const std::string& _lgroup,const std::string& _rgroup);

	//! Set new divider characters
	void set_divider(const std::string& _divider);

	//! Set new escape character
	void set_escape(char _escape);

	//! return true if input stream is good
	bool good() const { return m_good; }

	//! Extract next token. Return separator character found or '\0' if stream has reached the end.
	char next(
		std::string& _out,			//!< Output token
		bool _append = false);		//!< True to append to 'out'

	//! Read to the _end character and append all into _str. Return false if EOF is reached
	bool seek(
		char _end,					//!< End character
		std::string& _out,			//!< Output token
		bool _append = false);		//!< True to append to 'out'

	//! Read to the _end character. Return false if EOF is reached
	bool seek(char _end);

protected:
	std::istream* mp_str;
	std::streambuf* mp_buf;
	std::string m_lgroup;
	std::string m_rgroup;
	std::string m_divider;
	char m_escape;
	bool m_good;

	bool is_divider(char _c);
	bool seek_impl(char _end,std::string* _out,bool _append);
};


//! Extract token and its arguments from line
void get_token(
	const std::string& in,			//!< Input string
	std::string& token,				//!< Token
	std::string& args,				//!< Token arguments
	char lpar = '(',				//!< Left parenthesis character
	char rpar = ')');				//!< right parenthesis character


} // namespace fbox

#endif
