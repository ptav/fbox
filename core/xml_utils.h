#ifndef __FBOX_CORE_XML_UTILS_H__
#define __FBOX_CORE_XML_UTILS_H__
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	XML utilities (based on TinyXML)
*/


#include "main.h"
#include <tinyxml/tinyxml.h>
#include <stack>

namespace fbox {
namespace xml {


//! Retrieve element given a path. Return 0 if element does not exist.
TiXmlElement* get_element(
	TiXmlElement* _src,
	const std::string& _a1 = std::string(),
	const std::string& _a2 = std::string(),
	const std::string& _a3 = std::string(),
	const std::string& _a4 = std::string());

//! Retrieve text of an element given the path. Return 0 if either the 
//! element does not exist or text is empty.
const char* get_text(
	TiXmlElement* _src,
	const std::string& _a1 = std::string(),
	const std::string& _a2 = std::string(),
	const std::string& _a3 = std::string(),
	const std::string& _a4 = std::string());


//! Retrieve element attribute given the path. Return 0 if either the element
//! or attribute do not exist or the latter contains an empty string.
const char* get_attribute(
	const std::string& _attribute,
	TiXmlElement* _src,
	const std::string& _a1 = std::string(),
	const std::string& _a2 = std::string(),
	const std::string& _a3 = std::string(),
	const std::string& _a4 = std::string());


//! Create simple XML document
class make_xml
{
public:
	make_xml() : mp_doc(0) { reset(); }

	~make_xml();

	//! Reset document
	void reset();

	//! Save document
	void save(const std::string& _path);

	//! Print document
	void print(std::ostream& _out,bool _long=false);

	//! Open a new heading
	TiXmlElement* open_branch(const std::string& _key);

	//! Close last heading
	void close_branch();

	//! Add string valued item
	TiXmlElement* value(const std::string& _key,const std::string& _value);

	//! Add generic item
	template<typename _value_type>
	TiXmlElement* value(const std::string& _key,const _value_type& _value);

	//! Add string valued attribute
	TiXmlElement* attribute(const std::string& _key,const std::string& _value);

protected:
	TiXmlDocument* mp_doc;
	std::stack<TiXmlElement*> m_active;
};


template<typename _value_type>
TiXmlElement* make_xml::value(const std::string& _key,const _value_type& _value)
{
	std::stringstream a;
	a << _value;
	return value(_key,a.str());
}



//! Create a single XML tag with attributes
/*!
	The implementation allows nested syntax:

	<code>
	make_tag(std::cout)
		.attr("counter",0)
		.attr("title","my xml")
		.attr("help","this is a test")
	<code>
*/		
class make_tag
{
public:
	make_tag(std::ostream& _strm,const std::string _name,bool _close=true);	
	~make_tag();

	template<typename _arg>
	make_tag& attr(const std::string& _tag,const _arg& _value)
	{
		m_strm << ' ' << _tag << '=';
		m_strm << '\"' << _value << '\"';
		return *this;
	}

private:
	std::ostream& m_strm;
	 bool m_close;
};


} // namespace xml
} // namespace fbox

#endif
