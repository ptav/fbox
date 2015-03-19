#ifndef __FBOX_CORE_ERROR_H__
#define __FBOX_CORE_ERROR_H__
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Error handling classes
*/


#include "main.h"
#include <exception>
#include <string>
#include <sstream>


namespace fbox {

//! Global error exception
class error : public std::exception
{
public:
	error(const std::string& msg="FBox exception") : m_msg(msg) {}

	error(const error& e) : m_msg(e.m_msg),std::exception(e) {}

	error& operator=(const error& e)
	{
		if (&e != this) m_msg = e.m_msg;
		return *this;
	}

	virtual ~error() throw() {}

	virtual const char* what() const throw() { return m_msg.c_str(); }

private:
	std::string m_msg;
};


//! Shorthand for the definition of member exception classes
#define FBOX_LOCAL_ERROR(x,parent,def) \
	class x : public parent \
	{ \
	public: \
		x() : parent(def) {} \
		x(const x& rhs) : parent(rhs) {} \
		x(const std::string& msg) : parent(msg) {} \
		x(const std::string& label,const std::string& msg) : parent(msg + " - " + msg) {} \
	}

//! Shorthand for variant error message - 1 arguments
template<
	typename _err,	//!< Exception type. Must support ctor with single <code>std::string</code> argument
	typename _arg0>	//!< Type of first argument
void throw_error(
	const _err& _e,
	const _arg0& _a0)
{
	std::stringstream str;
	str << _a0;
	throw _err(str.str());
}


//! Shorthand for variant error message - 2 arguments
template<
	typename _err,	//!< Exception type. Must support ctor with single <code>std::string</code> argument
	typename _arg0,	//!< Type of first argument
	typename _arg1>	//!< Type of second argument
void throw_error(
	const _err& _e,
	const _arg0& _a0,
	const _arg1& _a1)
{
	std::stringstream str;
	str << _a0 << _a1;
	throw _err(str.str());
}


//! Shorthand for variant error message - 3 arguments
template<
	typename _err,	//!< Exception type. Must support ctor with single <code>std::string</code> argument
	typename _arg0,	//!< Type of first argument
	typename _arg1,	//!< Type of second argument
	typename _arg2>	//!< Type of third argument
void throw_error(
	const _err& _e,
	const _arg0& _a0,
	const _arg1& _a1,
	const _arg2& _a2)
{
	std::stringstream str;
	str << _a0 << _a1 << _a2;
	throw _err(str.str());
}


} // namespace fbox

#endif
