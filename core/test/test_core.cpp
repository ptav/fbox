/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Master test file
*/

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include "../main.h"


class A {};
class B : public A {};
template<typename X> class C {};
template<typename X,typename Y> class D {};

namespace N1 
{
	class A {};
	class B : public A {};
	template<typename X> class C {};
	template<typename X,typename Y> class D {};

	namespace N2 
	{
		class A {};
		class B : public A {};
		template<typename X> class C {};
		template<typename X,typename Y> class D {};
	}
}


BOOST_AUTO_TEST_CASE(test_type_id)
{
	A a;
	B b;
	C<A> c;
	D<A,B> d;

	N1::A aa;
	N1::B bb;
	N1::C<A> cc;
	N1::D<N1::A,B> dd;
	N1::C<N1::A> ee;

	N1::N2::A aaa;
	N1::N2::B bbb;
	N1::N2::C<A> ccc;
	N1::N2::D<N1::N2::A,B> ddd;
	N1::N2::C<N1::N2::A> eee;

	// with scope
	BOOST_CHECK_EQUAL( fbox::type_id(a) , "A" );
	BOOST_CHECK_EQUAL( fbox::type_id(b) , "B" );
	BOOST_CHECK_EQUAL( fbox::type_id(c) , "C<A>" );
	BOOST_CHECK_EQUAL( fbox::type_id(d) , "D<A,B>" );
	
	BOOST_CHECK_EQUAL( fbox::type_id(aa) , "N1::A" );
	BOOST_CHECK_EQUAL( fbox::type_id(bb) , "N1::B" );
	BOOST_CHECK_EQUAL( fbox::type_id(cc) , "N1::C<A>" );
	BOOST_CHECK_EQUAL( fbox::type_id(dd) , "N1::D<N1::A,B>" );
	BOOST_CHECK_EQUAL( fbox::type_id(ee) , "N1::C<N1::A>" );

	BOOST_CHECK_EQUAL( fbox::type_id(aaa) , "N1::N2::A" );
	BOOST_CHECK_EQUAL( fbox::type_id(bbb) , "N1::N2::B" );
	BOOST_CHECK_EQUAL( fbox::type_id(ccc) , "N1::N2::C<A>" );
	BOOST_CHECK_EQUAL( fbox::type_id(ddd) , "N1::N2::D<N1::N2::A,B>" );
	BOOST_CHECK_EQUAL( fbox::type_id(eee) , "N1::N2::C<N1::N2::A>" );

	// without scope
	BOOST_CHECK_EQUAL( fbox::type_id(a,true) , "A" );
	BOOST_CHECK_EQUAL( fbox::type_id(b,true) , "B" );
	BOOST_CHECK_EQUAL( fbox::type_id(c,true) , "C<A>" );
	BOOST_CHECK_EQUAL( fbox::type_id(d,true) , "D<A,B>" );
	
	BOOST_CHECK_EQUAL( fbox::type_id(aa,true) , "A" );
	BOOST_CHECK_EQUAL( fbox::type_id(bb,true) , "B" );
	BOOST_CHECK_EQUAL( fbox::type_id(cc,true) , "C<A>" );
	BOOST_CHECK_EQUAL( fbox::type_id(dd,true) , "D<A,B>" );
	BOOST_CHECK_EQUAL( fbox::type_id(ee,true) , "C<A>" );

	BOOST_CHECK_EQUAL( fbox::type_id(aaa,true) , "A" );
	BOOST_CHECK_EQUAL( fbox::type_id(bbb,true) , "B" );
	BOOST_CHECK_EQUAL( fbox::type_id(ccc,true) , "C<A>" );
	BOOST_CHECK_EQUAL( fbox::type_id(ddd,true) , "D<A,B>" );
	BOOST_CHECK_EQUAL( fbox::type_id(eee,true) , "C<A>" );
}
