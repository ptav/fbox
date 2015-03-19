/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Dictionary class
*/

#include "main.h"
#include "dictionary.h"

namespace fbox {


////////////////////////////////////////
// dictionary
////////////////////////////////////////

dictionary_value_proxy dictionary::insert(const std::string& _key,const dictionary::value_type& _value,bool _overwrite)
{
	map_type::const_iterator itr = m_data.find(_key);
	if (_overwrite || itr == m_data.end()) 
		return dictionary_value_proxy(m_data[_key] = _value);
	else
		throw error("trying to overwrite existing key '" + _key + "' with 'overwrite' flag set to false)");
}


dictionary_value_proxy dictionary::operator[] (const std::string& _key)
{
	map_type::iterator itr = m_data.find(_key);
	
	if (itr == m_data.end())
		throw error("Dictionary does not contain key " + _key);
	else
		return dictionary_value_proxy(itr->second);
}


const_dictionary_value_proxy dictionary::operator[] (const std::string& _key) const
{
	map_type::const_iterator itr = m_data.find(_key);
	
	if (itr == m_data.end())
		throw error("Dictionary does not contain key " + _key);
	else
		return const_dictionary_value_proxy(itr->second);
}


bool dictionary::contains(const std::string& _key) const
{
	map_type::const_iterator itr = m_data.find(_key);
	return (itr != m_data.end());
}


std::ostream& operator<<(std::ostream& _s,const dictionary& _dict)
{
	fbox::dictionary::const_iterator itr = _dict.begin();
	fbox::dictionary::const_iterator end = _dict.end();
	for(; itr != end; ++itr) _s << itr->first << " = " << itr->second << '\n';

	return _s;
}



////////////////////////////////////////
// dictionary_value_proxy
// const_dictionary_value_proxy
////////////////////////////////////////

int& dictionary_value_proxy::as_int()									{ return boost::get<int>(mr_v); }
double&dictionary_value_proxy::as_double()								{ return boost::get<double>(mr_v); }
std::string& dictionary_value_proxy::as_string()						{ return boost::get<std::string>(mr_v); }
dictionary& dictionary_value_proxy::as_dictionary()						{ return boost::get<dictionary>(mr_v); }

int const_dictionary_value_proxy::as_int() const						{ return boost::get<int>(mr_v); }
double const_dictionary_value_proxy::as_double() const					{ return boost::get<double>(mr_v); }
const std::string& const_dictionary_value_proxy::as_string() const		{ return boost::get<std::string>(mr_v); }
const dictionary& const_dictionary_value_proxy::as_dictionary() const	{ return boost::get<dictionary>(mr_v); }


} // namespace fbox

