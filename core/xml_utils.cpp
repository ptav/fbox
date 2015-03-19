
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	XML utilities (based on TinyXML)
*/


#include "xml_utils.h"
#include "error.h"

namespace fbox {
namespace xml {


TiXmlElement* get_element(
	TiXmlElement* _src,
	const std::string& _a1,
	const std::string& _a2,
	const std::string& _a3,
	const std::string& _a4)
{
	if (!_src)
		return 0;
	else if (!_a1.empty())
		return get_element(_src->FirstChildElement(_a1),_a2,_a3,_a4);
	else
		return _src;

	return 0; // not required but stops warning about paths not returning
}


const char* get_text(
	TiXmlElement* _src,
	const std::string& _a1,
	const std::string& _a2,
	const std::string& _a3,
	const std::string& _a4)
{
	TiXmlElement* e = get_element(_src,_a1,_a2,_a3,_a4);
	return (e ? e->GetText() : 0);
}


const char* get_attribute(
	const std::string& _attribute,
	TiXmlElement* _src,
	const std::string& _a1,
	const std::string& _a2,
	const std::string& _a3,
	const std::string& _a4)
{
	TiXmlElement* e = get_element(_src,_a1,_a2,_a3,_a4);
	return (e ? e->Attribute(_attribute.c_str()) : 0);
}



////////////////////////////////////////
// director_mk1
////////////////////////////////////////


make_xml::~make_xml()
{
	delete mp_doc;
}


void make_xml::reset()
{
	delete mp_doc;

	mp_doc = new TiXmlDocument;
 	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0","", "");
	mp_doc->LinkEndChild(decl);

	while (m_active.size()) m_active.pop();
}


void make_xml::save(const std::string& _path)
{
	mp_doc->SaveFile(_path);
}


void make_xml::print(std::ostream& _out,bool _long)
{
	if (_long)
	{
		TiXmlPrinter printer;
		mp_doc->Accept(&printer);
		_out << printer.CStr();
	}
	else
	{
		_out << *mp_doc;
	}
}


TiXmlElement* make_xml::open_branch(const std::string& _key)
{
	TiXmlElement* item = new TiXmlElement(_key);
	
	if (m_active.size())
		m_active.top()->LinkEndChild(item);
	else
		mp_doc->LinkEndChild(item);

	m_active.push(item);
	
	return item;
}


void make_xml::close_branch()
{
	if (m_active.empty()) throw error("Trying to pop empty key stack");
    m_active.pop();
}


TiXmlElement* make_xml::value(const std::string& _key,const std::string& _value)
{
	if (m_active.empty()) throw error("Trying to add value before master key");
	
	TiXmlElement* item = new TiXmlElement(_key);
	m_active.top()->LinkEndChild(item);

	TiXmlText* text = new TiXmlText(_value);
	item->LinkEndChild(text);

	return item;
}


TiXmlElement* make_xml::attribute(const std::string& _key,const std::string& _value)
{
	if (m_active.empty()) throw error("Trying to add value before master key");
	
	TiXmlElement* item = m_active.top();
	item->SetAttribute(_key,_value);

	return item;
}



////////////////////////////////////////
// make_tag
////////////////////////////////////////

make_tag::make_tag(std::ostream& _strm,const std::string _name,bool _close)
:	m_strm(_strm),m_close(_close)
{
	m_strm << '<' << _name;
}


make_tag::~make_tag()
{
	if (m_close) m_strm << '/'; m_strm << '>';
} 


} // namespace xml
} // namespace fbox

