#ifndef __FBOX_CORE_DATE_H__
#define __FBOX_CORE_DATE_H__
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Date class
*/


#include <exception>
#include <string>
#include <boost/date_time/gregorian/gregorian.hpp>

namespace fbox {


//! Boost date type shorthand
typedef boost::gregorian::date gdate;


} // namespace fbox

#endif
