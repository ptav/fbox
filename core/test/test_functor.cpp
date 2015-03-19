/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Tests functor framework
*/

#include <iostream>
#include <boost/test/unit_test.hpp>
#include "../functor.h"

using namespace fbox;

struct add : public declare_task<add>
{
	add(int _a,int _b) : a(_a),b(_b),c(0) {};
	int a,b,c;
};

struct swap : public declare_task<swap>
{
	swap(int _a,int _b) : a(_a),b(_b) {};
	int a,b;
};

struct addx : public add, public declare_task<addx>
{
	addx(int _a,int _b,int _x) : add(_a,_b),x(_x) {};
	int x;

	DECLARE_CHILD_TASK(addx,add)
};


struct func1 : public functor, public task_handle<add>
{
	virtual bool task_impl(add& _tsk) { _tsk.c = _tsk.a + _tsk.b; return true; }
};

struct func2 : public func1, public task_handle<swap>
{
	virtual bool task_impl(swap& _tsk) { std::swap(_tsk.a,_tsk.b); return true; }
};

struct func3 : public func2
{
	virtual bool task_impl(add& _tsk) { _tsk.c = _tsk.a * _tsk.a + _tsk.b * _tsk.b; return true; }
};

struct func4 : public func2, public task_handle<addx>
{
	virtual bool task_impl(addx& _tsk) { _tsk.c = (_tsk.a + _tsk.b) * _tsk.x; return true; }
};

struct func5 : public functor,task_handle<addx>
{
	virtual bool task_impl(addx& _tsk) { _tsk.c = (_tsk.a + _tsk.b) * _tsk.x; return true; }
};

struct func6 : public func2, public task_handle<addx>
{
	virtual bool task_impl(add& _tsk) { _tsk.c = _tsk.a * _tsk.a + _tsk.b * _tsk.b; return true; }
	virtual bool task_impl(addx& _tsk) { _tsk.c = (_tsk.a + _tsk.b) * _tsk.x; return true; }
};



BOOST_AUTO_TEST_CASE(test_functor)
{
	BOOST_MESSAGE("Test functor");

	add a(1,2);
	swap s(1,2);

	// add is supported, swap not
	func1 f1;
	BOOST_CHECK(f1.apply(a));
	BOOST_CHECK_EQUAL(a.c,3);
	BOOST_CHECK(!f1.apply(s));
	a.c = 0;

	// add support inherited, swap supported
	func2 f2;
	BOOST_CHECK(f2.apply(a));
	BOOST_CHECK_EQUAL(a.c,3);
	BOOST_CHECK(f2.apply(s));
	BOOST_CHECK_EQUAL(s.a,2);
	BOOST_CHECK_EQUAL(s.b,1);
	a.c = 0;

	// add support overriden
	func3 f3;
	BOOST_CHECK(f3.apply(a));
	BOOST_CHECK_EQUAL(a.c,5);
	BOOST_CHECK(f3.apply(s));
	BOOST_CHECK_EQUAL(s.a,1);
	BOOST_CHECK_EQUAL(s.b,2);
	a.c = 0;

	// func2 knows nothing about addx but since the latter is derived
	// from add it will use that task_impl to process
	addx x(1,2,3);
	BOOST_CHECK(f2.apply(x));
	BOOST_CHECK_EQUAL(x.c,3);
	x.c = 0;

	// explicitly support addx as well as original add
	func4 f4;
	BOOST_CHECK(f4.apply(a));
	BOOST_CHECK_EQUAL(a.c,3);
	BOOST_CHECK(f4.apply(x));
	BOOST_CHECK_EQUAL(x.c,9);
	BOOST_CHECK(f4.apply(s));
	BOOST_CHECK_EQUAL(s.a,2);
	BOOST_CHECK_EQUAL(s.b,1);
	a.c = x.c = 0;

	// Support only addx
	func5 f5;
	BOOST_CHECK(!f5.apply(a));
	BOOST_CHECK(f5.apply(x));
 	BOOST_CHECK_EQUAL(x.c,9);
	BOOST_CHECK(!f5.apply(s));
	a.c = x.c = 0;

	// explicitly support addx and add a new implementation for add
	func6 f6;
	BOOST_CHECK(f6.apply(a));
	BOOST_CHECK_EQUAL(a.c,5);
	BOOST_CHECK(f6.apply(x));
	BOOST_CHECK_EQUAL(x.c,9);
	BOOST_CHECK(f6.apply(s));
	BOOST_CHECK_EQUAL(s.a,1);
	BOOST_CHECK_EQUAL(s.b,2);
	a.c = x.c = 0;
}
