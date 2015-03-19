/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Logging utilities
*/

#include "logger.h"
#include "error.h"
#include "system.h"
#include "xml_utils.h"
#include "date.h"
#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace fbox {


static void logger_opening_msg(std::ostream& _strm)
{
	_strm << "======================================================================\n";
	_strm << "FBox logger\n";
	_strm << "Created " << boost::gregorian::day_clock::local_day() << " at " << boost::posix_time::second_clock::local_time() << '\n';
	_strm << "======================================================================\n";
	_strm << std::flush;
}


static void logger_closing_msg(std::ostream& _strm)
{
	_strm << "======================================================================\n";
	_strm << "Closed " << boost::gregorian::day_clock::local_day() << " at " << boost::posix_time::second_clock::local_time() << '\n';
	_strm << "======================================================================\n";
	_strm << std::flush;
}


logger::logger()
:	m_lv(WARNING),
	m_perm_lv(WARNING),
	m_temp_lv(WARNING),
	mp_stream(&std::cerr),
	m_head(true)
{
	TiXmlDocument doc;
	if (doc.LoadFile(system::get_config_file()))
	{
		TiXmlElement* top = doc.FirstChildElement("fbox");
		if (top)
		{
			const char* str = fbox::xml::get_text(top,"logger","level");
			if (str) set_trigger_level((level_type)atoi(str));

			str = fbox::xml::get_text(top,"logger","file");
			if (str) set_output(str,true);
		}
	}
}


logger::logger(std::ostream& _stream,bool _head,logger::level_type _level)
:	m_lv(_level),
	m_perm_lv(_level),
	m_temp_lv(_level)
{
	set_output(_stream,_head);
}


logger::logger(const std::string& _filename,bool _head,logger::level_type _level)
:	m_lv(_level),
	m_perm_lv(_level),
	m_temp_lv(_level)
{
	set_output(_filename,_head);
}


logger::~logger()
{
	if (mp_stream && m_head) logger_closing_msg(*mp_stream);
}


void logger::set_output(std::ostream& _stream,bool _head)
{
	mp_stream.set(&_stream,false);
	if (m_head = _head) logger_opening_msg(*mp_stream);
}


void logger::set_output(const std::string& _filename,bool _head)
{
	std::ofstream* file( new std::ofstream(_filename.c_str()) );
	if (!file && !file->is_open()) throw error("Failed to open log file");	

	mp_stream.set(file,true);
	if (m_head = _head) logger_opening_msg(*mp_stream);
}



} // namespace fbox

