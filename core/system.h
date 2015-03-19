#ifndef __FBOX_CORE_SYSTEM_H__
#define __FBOX_CORE_SYSTEM_H__
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	System tools
*/


#include "main.h"


namespace fbox {
namespace system {

//! Retrieve system temporary directory
std::string get_tmp_directory();


//! Split filename path, name and extension
void split_filename(
	const std::string& _file,
	std::string& _path,
	std::string& _name,
	std::string& _extension);


//! Retrieve FBox configuration directory (create if required and _create==true)
std::string get_fbox_directory(bool _create=true);


//! Retrieve name of FBox configuration file. Availability is not checked
std::string get_config_file();


} // namespace system
} // namespace fbox

#endif
