#ifndef __FBOX_CORE_FUNCTOR_H__
#define __FBOX_CORE_FUNCTOR_H__

/*!
	\file
	\author	Pedro Tavares

	Functor/Request interfaces and respective double-dispatch implementation,
*/

/*!
	\page functor_task The FBox functor framework
	\par Functors and Task Classes

	To define a new functor the client inherits from the <code>functor</code> base class. It then declares 
	which tasks are supported by inheriting from a specification of the <code>task_handle</code> policy.

	Tasks are created by inheriting from a specification of the <code>declare_task</code> policy class.


	\par Examples

	In the example below we create two tasks called <code>print</code> and <code>add</code>. No limitations 
	are imposed in the task classes and these can in principle contain any data and number of methods. The
	only requirements is that the task class must support all input and output data required byt the 
	implementations	of that task. In the simplest cases the tasks will include only data containers for the 
	arguments and results of the task.

	\code
	using namespace fbox;

	struct print : public declare_task<print>
	{
		out(double _val) { std::cout << "The value is " << _val << std::endl; }
	};

	class add : public declare_task<add>
	{
		double a,b;
		double c;
	};
	\endcode

	The functor below inherits the interface and each task handle. It then implements each handler (the 
	<code>task_impl</code> functions). The handler returns a simple error state: true for success, false 
	otherwise. As we shall see below these handlers are never called explicitly.

	\code
	class my_functor
	:	public functor,
		public task_handle<print>,
		public task_handle<add>
	{
	public:
		virtual bool task_impl(print& _x) { ... }
		virtual bool task_impl(add& _x)	{ ... }
	};
	\endcode
	
	Functor clients do not generally call the task handler functions directly (although they can if the 
	functor type is available). Instead they generically call the interface function <code>apply</code>. 
	The functor/task framework will then select the correct <code>task_impl</code> to call. In this way 
	both the task type and the functor type can be abstracted away as the excerpt below exemplifies:

	\code
	void execute(functor& _func,tast& _task)
	{
		_func.apply(_task);
	}

	main()
	{
		my_functor func;
		print prt;

		// implicitly call my_functor::task_impl(print&)
		execute(func,ptr);
	}
	\endcode
	
	When the developer requests a task that is not supported <code>apply</code> returns false.

	<code>apply</code> can be called on constant functors if it implements <code>const</code> versions 
	of the corresponding <code>taks_impl</code>.


	\par Inheritance
	
	Inheritance of the functor type is provided and support for individual tasks can be inherited from 
	the parent(s). Inheritance is supported for tasks as well but some plumbing is required as the example 
	below exemplifies:

	\code
	struct print_child : public task<print_child>, public print
	{
		DECLARE_CHILD_TASK(print_child,print)

		...
	};
	\endcode
*/


#include "main.h"
#include <typeinfo>


namespace fbox {


class functor;

//! Task base class. 
/*! See \ref functor_task "functor framework" for full details. */
class task
{
public:
	typedef task parent_task; //! Support task inheritance

	virtual ~task() {}

protected:
	virtual bool apply_inv(functor&) = 0;
	virtual bool const_apply_inv(const functor&) = 0;

	friend class functor;
};


//! Shorthand macro to declare inherited requests
/*! See \ref functor_task "functor framework" for full details. */
#define DECLARE_CHILD_TASK(_child,_parent) \
	typedef _parent parent_task; \
	virtual bool apply_inv(fbox::functor& _func) { return declare_task<_child>::apply_inv(_func); } \
	virtual bool const_apply_inv(const fbox::functor& _func) { return declare_task<_child>::const_apply_inv(_func); }


//!	The calculator base class
/*! See \ref functor_task "functor framework" for full details. */
class functor
{
public :
	virtual ~functor() {}

	//! Execute a task and report exit state.
	/*! \return True if successful, false if task is not supported or the execution reports error. */
	virtual bool apply(task& _tsk) { return _tsk.apply_inv(*this); }

	//! Constant version of <code>apply</code>
	/*! \return True if successful, false if task is not supported or the execution reports error. */
	virtual bool apply(task& _tsk) const { return _tsk.const_apply_inv(*this); }
};


//! Declare support for a request (non-const form) in calculator framework
/*!	See \ref calculator_framework "Calculator/request framework". */
template<typename _T>
class task_handle
{
public :
	virtual bool task_impl(_T&) { return false; }
	virtual bool task_impl(_T&) const { return false; }
};


//! Declare support for a task (const form)
/*! See \ref functor_task "functor framework" for full details. */
/*template<typename _T>
class const_handle 
{
public :
	virtual bool task_impl(_T&) const = 0;
};*/



//! Request class template base
/*! 
	This template is used in the definition of new tasks.
	See \ref functor_task "functor framework" for full details.
*/
template<typename _T>
class declare_task : virtual public task
{
public:
	bool apply_inv(functor& func);
	bool const_apply_inv(const functor& func);
};


// Helper class for task inheritance
template<class _T> 
class handle_inherit
{
public :
	static bool apply_inv(_T* t,functor& func)
	{
		typedef declare_task<_T> parent_task_base;
		parent_task_base* ptb = static_cast<parent_task_base*>(t);
		if (ptb) 
			return ptb->declare_task<_T>::apply_inv(func);
		else
			return false;
	}

	static bool const_apply_inv(_T* t,const functor& func)
	{
		typedef declare_task<_T> parent_task_base;
		parent_task_base* ptb = static_cast<parent_task_base*>(t);
		if (ptb) 
			return ptb->declare_task<_T>::const_apply_inv(func);
		else
			return false;
	}
};

// Default implementation of handle_inherit returns reports no support for given task
template<> 
class handle_inherit<task>
{
public :
	template<class _T>
	static bool apply_inv(_T* t,functor& b) { return false; }

	template<class _T>
	static bool const_apply_inv(_T* t,const functor& b) { return false; }
};


// Helper class for task inheritance (const)
/*template<class _R> 
class const_handle_inherit
{
public :
	template<class _T> 
	static bool apply_inv(_T* t,const functor& func)
	{
		typedef declare_task<_R> task_base_parent;
		return static_cast<task_base_parent*>(t)->declare_task<_R>::const_apply_inv(func);
	}
};

// Default implementation of const_handle_inherit returns reports no support for given task
template<> 
class const_handle_inherit<task>
{
public :
	template<class _T>
	static bool apply_inv(_T* t,const functor& b) { return false; }
};*/



template<typename _T>
bool declare_task<_T>::apply_inv(functor& func)
{
	typedef task_handle<_T> target_task;
	target_task* t = dynamic_cast<target_task*>(&func);
	if(t)
	{
		return t->task_impl( static_cast<_T&>(*this) );
	}
	else
	{
		typedef typename _T::parent_task parent;
		return handle_inherit<parent>::apply_inv(static_cast<_T*>(this),func) ;
	}
}


template<typename _T>
bool declare_task<_T>::const_apply_inv(const functor& func)
{
	typedef const task_handle<_T> target_task;
	target_task* t = dynamic_cast<target_task*>(&func);
	if(t)
	{
		return t->task_impl( static_cast<_T&>(*this) );
	}
	else
	{
		typedef typename _T::parent_task parent;
		return handle_inherit<parent>::const_apply_inv(static_cast<_T*>(this),func) ;
	}
}


} // namespace fbox

#endif
