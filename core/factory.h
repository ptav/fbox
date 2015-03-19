#ifndef __FBOX_CORE_FACTORY_H__
#define __FBOX_CORE_FACTORY_H__
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Generic template factory
*/

#include "main.h"
#include "error.h"
#include <map>


namespace fbox {

//! Default object allocation policy
struct default_factory_allocator
{
	template<typename _t>
	static _t* alloc() { return (new _t); }
};


//! Default object naming policy
struct default_name_policy
{
	template<typename _object_type,typename _factory_type,typename _key_type>
	static _key_type name(
		const _factory_type& _fac,
		const _key_type& _override)
	{
		return _override;
	}
};


//! Interface and basic implementation of individual virtual factories
template<
	typename _object,								//!< Factory object type. Must support <code>std::auto_ptr<object_type> clone() const</code>
	typename _key=std::string,						//!< Key type
	typename _name_policy = default_name_policy,	//!< Object naming policy
	typename _allocator_policy = default_factory_allocator> //!< Allocator policy method
class factory
{
public:
	typedef _object object_type;
	typedef _key key_type;
	typedef _name_policy name_policy;
	typedef _allocator_policy allocator_policy;
	typedef factory<_object,_key,_name_policy,_allocator_policy> factory_type;

	typedef std::auto_ptr<object_type> object_ptr_type;

	FBOX_LOCAL_ERROR(error,fbox::error,"Generic factory error");

	~factory()
	{
		typename registry_type::iterator itr = m_factories.begin();
		typename registry_type::iterator end = m_factories.end();
		for(; itr != end; ++itr) delete itr->second;
	}

	//! Register new object type
	template<typename _specific>
	void insert(const key_type& _id = key_type())
	{
		typedef factory_object_impl<_specific> type;
		std::auto_ptr<type> p(new type);
		key_type id1 = name_policy::template name<_specific,factory_type,key_type>(*this,_id);
		std::pair<typename registry_type::iterator,bool> result( m_factories.insert( typename registry_type::value_type(id1,p.get()) ) );
		if (!result.second) throw_error(error(),"Failed to insert factory object ",id1);
		p.release();
	}

	//! Create object instance. Return null pointer if factory is not found.
	std::auto_ptr<object_type> create(const key_type& _id) const
	{
		typename registry_type::const_iterator itr = m_factories.find(_id);
		if (itr == m_factories.end()) return std::auto_ptr<object_type>();
		return std::auto_ptr<object_type>(itr->second->make());
	}

	//! Report number of registered objects
	size_type object_count() const { return m_factories.size(); }

protected:
	struct factory_object
	{
		virtual ~factory_object() {}
		virtual std::auto_ptr<object_type> make() const = 0;
	};

	template<typename _specific>
	struct factory_object_impl : public factory_object
	{
		virtual std::auto_ptr<object_type> make() const 
		{
			return std::auto_ptr<object_type>( allocator_policy::template alloc<_specific>() );
		}
	};

	typedef factory_object* factory_object_ptr;
	typedef std::map<key_type,factory_object_ptr> registry_type;
	registry_type m_factories;
};

} // namespace fbox

#endif
