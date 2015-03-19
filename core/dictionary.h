#ifndef __FBOX_CORE_DICTIONARY_H__
#define __FBOX_CORE_DICTIONARY_H__
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Dictionary class
*/

#include "main.h"
#include "error.h"
#include <map>
#include <boost/variant.hpp>

namespace fbox {


class dictionary_value_proxy;
class const_dictionary_value_proxy;

//! Dictionary container
class dictionary
{
public:
	typedef boost::variant<int,double,std::string,dictionary> value_type;
	typedef std::map<std::string,value_type> map_type;
	typedef map_type::iterator iterator;
	typedef map_type::const_iterator const_iterator;

	FBOX_LOCAL_ERROR(error,fbox::error,"Dictionary error");

	void clear() { m_data.clear(); } //!< clear dictionary
	dictionary_value_proxy insert(const std::string& _key,const value_type& _value,bool _overwrite=false); //!< Insert key. If _overwrite=0 it will except if key already exists 

	size_type size() const { return m_data.size(); } //!< Number of entries
	bool contains(const std::string& _key) const; //!< True if dictionary contains key

	dictionary_value_proxy operator[] (const std::string& _key); //!< Mutable access to existing key (excepts if key does not exist)
	const_dictionary_value_proxy operator[] (const std::string& _key) const; //!< Constant access to existing key (excepts if key does not exist)
	
	const_iterator begin() const { return m_data.begin(); }
	const_iterator end() const { return m_data.end(); }
	
	iterator begin() { return m_data.begin(); }
	iterator end() { return m_data.end(); }
		
protected:
	map_type m_data;
};


class dictionary_value_proxy
{
public:
	dictionary_value_proxy(dictionary::value_type& _value) : mr_v(_value) {}

	operator dictionary::value_type() { return mr_v; }

	int& as_int();
	double& as_double();
	std::string& as_string();
	dictionary& as_dictionary();

private:
	dictionary::value_type& mr_v;
};


class const_dictionary_value_proxy
{
public:
	const_dictionary_value_proxy(const dictionary::value_type& _value) : mr_v(_value) {}

	operator dictionary::value_type() const { return mr_v; }

	int as_int() const;
	double as_double() const;
	const std::string& as_string() const;
	const dictionary& as_dictionary() const;

private:
	const dictionary::value_type& mr_v;
};



//! default stream output for dictionaries
std::ostream& operator<<(std::ostream& _s,const dictionary& _dict);

} // namespace fbox

#endif
