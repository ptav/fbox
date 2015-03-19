#ifndef __FBOX_CORE_LINK_H__
#define __FBOX_CORE_LINK_H__
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Pointer utilities
*/


#include "main.h"
#include "error.h"


namespace fbox {


//! Soft link pointer. Wrapper for a standard pointer with additional checks. Warning: soft_ptr is not polymorphic!
template<typename _type>
class soft_ptr
{
public:
	typedef _type object_type;

	FBOX_LOCAL_ERROR(error,fbox::error,"soft_ptr is not set");

	soft_ptr(object_type* _p=0) 
	:	mp_obj(_p)
	{}

	soft_ptr(const soft_ptr<object_type>& _p)
	 :	mp_obj(_p.mp_obj)
	{}

	soft_ptr<object_type>& operator=(const soft_ptr<object_type>& _p)
	{
		if (this != &_p) set(_p.mp_obj);
		return *this;
	}

	soft_ptr<object_type>& operator=(object_type* _p)
	{
		if (mp_obj != _p) set(_p);
		return *this;
	}

	void set(object_type* _p)
	{
		mp_obj = _p;
	}

	bool is_set() const					{ return mp_obj != 0; } //!< Check whether pointer was set
	operator bool() const				{ return mp_obj != 0; } //!< Cast to true if pointer is set

	object_type* get() const			{ return mp_obj; } //!< Retrieve naked pointer (without checking state)

#ifdef FBOX_DEBUG
	operator object_type*() const		{ _check_state(); return mp_obj; }
	object_type* operator->() const		{ _check_state(); return mp_obj; }
	object_type& operator*() const		{ _check_state(); return *mp_obj; }
#else
	operator object_type*() const		{ return mp_obj; }
	object_type* operator->() const		{ return mp_obj; }
	object_type& operator*() const		{ return *mp_obj; }
#endif

protected:
	object_type* mp_obj;	// object pointed to

	void _check_state() const
	{
		if (!mp_obj) throw error();
	}
};


//! Flexible link pointer. close to a standard pointer with additional support for checks and automatic deletion.
//! Warning: flex_ptr and soft_ptr are not polymorphic!
template<typename _type>
class flex_ptr : public soft_ptr<_type>
{
public:
	typedef typename soft_ptr<_type>::object_type object_type;

	flex_ptr(object_type* _p=0,bool _owner=false)
	:	soft_ptr<object_type>(_p),m_owned(_owner)
	{}

	flex_ptr(const flex_ptr<object_type>& _p)
	 :	soft_ptr<object_type>(_p),m_owned(false)
	{}

	flex_ptr(const soft_ptr<object_type>& _p)
	 :	soft_ptr<object_type>(_p),m_owned(false)
	{}

	~flex_ptr()
	{
		if (m_owned) delete this->mp_obj;
	}

	flex_ptr<object_type>& operator=(const flex_ptr<object_type>& _p)
	{
		if (this != &_p) set(_p.mp_obj,false);
		return *this;
	}

	flex_ptr<object_type>& operator=(const soft_ptr<object_type>& _p)
	{
		if (this != &_p) set(_p.mp_obj,false);
		return *this;
	}

	soft_ptr<object_type>& operator=(object_type* _p)
	{
		if (this->mp_obj != _p) set(_p,false);
		return *this;
	}

	void set(object_type* _p,bool _owner=false)
	{
		if (m_owned) delete this->mp_obj;
		soft_ptr<object_type>::set(_p);
		m_owned = _owner;
	}

	bool is_owner() const				{ return m_owned; } //!< Check ownership
	void set_owner(bool _own=true)		{ m_owned = _own; } //!< Set ownership

private:
	bool m_owned;		// true if object is owned => gets deleted by dtor and allows explicit delete
};



} // namespace fbox

#endif
