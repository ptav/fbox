
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	System tools
*/


#include "system.h"
#include "error.h"
#include "string_utils.h"
#include "logger.h"
#include <boost/filesystem.hpp>


#ifdef WIN32
	#include <windows.h>
	#include <winsock.h>
	#include <shlobj.h>
	#include <iphlpapi.h>
#else
	#include <pwd.h>
#endif


namespace fbox {
namespace system {


std::string get_tmp_directory()
{
#ifdef WIN32
	DWORD buf_size(1024);
	char buf[1025];
    DWORD actual = GetTempPathA(buf_size,buf);
    if (actual > buf_size || (actual == 0)) 
		throw error("Unable to determine system's temporary #include <sys/utsname.h>file directory");

	return buf;
#else
	std::string s( getenv("TMP") );
	if (s.empty()) s = getenv("TEMP");

	return s;
#endif
}


void split_filename(
	const std::string& _file,
	std::string& _path,
	std::string& _name,
	std::string& _extension)
{
#ifdef WIN32
	char ps('\\');
#else
	char ps('/');
#endif

	char es('.');

	size_type p = _file.find_last_of(ps);
	size_type e = _file.find_last_of(es);

	_path = _file.substr(0,p-1);
	_name = _file.substr(p+1,e-1);
	_extension = _file.substr(e+1);
}


std::string get_fbox_directory(bool _create)
{
	std::string dir;

	#ifdef WIN32
		LPITEMIDLIST pidl;
		if ( SUCCEEDED( SHGetFolderLocation(NULL,CSIDL_APPDATA,NULL,NULL,&pidl) ) )
		{
			char path[MAX_PATH];
			SHGetPathFromIDListA(pidl,path);
			dir = std::string(path) +  "\\FBox";
		}

		if (pidl) CoTaskMemFree(pidl);

	#else
		struct passwd *id = getpwuid ( getuid() );
		dir = id->pw_dir;
		dir += "/.fbox";

	#endif

	boost::filesystem::path dirpath(dir.c_str());
	if (_create && !boost::filesystem::exists(dirpath))
	{
		boost::filesystem::create_directory(dirpath);
		FBOX_DBG_LOG(logger::SYSTEM,"get_fbox_directory -> created directory " << dir);
	}
	else
	{
		FBOX_DBG_LOG(logger::SYSTEM,"get_fbox_directory -> use directory " << dir);
	}

	if (boost::filesystem::exists(dirpath))
		return dir;
	else
		return std::string();
}


std::string get_config_file()
{
	return get_fbox_directory() + "/fbox.xml";
}

} // namespace system
} // namespace fbox

