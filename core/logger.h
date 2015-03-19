#ifndef __FBOX_CORE_LOGGER_H__
#define __FBOX_CORE_LOGGER_H__
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Logging utilities
*/


#include "main.h"
#include "soft_ptr.h"
#include <boost/thread/detail/singleton.hpp>
#include <iostream>
#include <fstream>
#include <string>

namespace fbox {

//! Global error exception
class logger
{
public:
	enum level_type
	{
		SYSTEM = 0,
		NOTE = 1,
		WARNING = 2,
		FATAL = 3
	};

	//! Create logger initialised to default settings (as set in fbox.xml)
	logger();

	//! Create logger connected with an existing output stream (default is stderr)
	logger(std::ostream& _stream,bool _head=true,level_type _level=WARNING);

	//! Create file logger
	logger(const std::string& _filename,bool _head=true,level_type _level=WARNING);
	
	virtual ~logger();

	//! Change output stream
	void set_output(std::ostream& _stream,bool _head=true);

	//! Change output file
	void set_output(const std::string& _filename,bool _head=true);

	//! Change trigger level
	void set_trigger_level(level_type _lv) { m_lv = _lv; }
	
	//! Change message level permanently
	void set_message_level(level_type _lv) { m_perm_lv = m_temp_lv = _lv; }
	
	//! Report message level
	level_type message_level() const { return m_temp_lv; }

	//! Output operation
	template<typename _type>
	logger& operator<< (const _type& _str)
	{
		if (m_temp_lv >= m_lv) (*mp_stream) << _str;
		m_temp_lv = m_perm_lv;
		return *this;
	}

	//! Change level of next output operation
	logger& operator<< (logger::level_type _lv)
	{
		m_temp_lv = _lv;
		return *this;
	}

	//! Handle other manipulators
	logger& operator<< (std::ostream& (*_ostrm) (std::ostream&))
	{
		if (m_temp_lv >= m_lv) (*_ostrm)(*mp_stream);
		return *this;
	}

private:
	level_type m_lv,m_perm_lv,m_temp_lv;
	flex_ptr<std::ostream> mp_stream;
	bool m_head;
};


#define FBOX_LOG(_lv,_msg) { \
fbox::logger& __logger__( boost::detail::thread::singleton<fbox::logger>::instance() ); \
	fbox::logger::level_type __old_lv__ = __logger__.message_level(); \
	__logger__.set_message_level(_lv); \
	__logger__ << _msg << std::endl; \
	__logger__.set_message_level(__old_lv__); }


#ifdef FBOX_DEBUG
	#define FBOX_DBG_LOG(_lv,_msg) FBOX_LOG(_lv,_msg)
#else
	#define FBOX_DBG_LOG(_lv,_msg)
#endif


} // namespace fbox

#endif
