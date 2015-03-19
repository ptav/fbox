/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Date class and utilities
*/

#include "main.h"
#include "date.h"

namespace fbox {

/*
long convert_julian_date(size_type year,size_type month,size_type day)
{
	return 0;
}


void convert_julian_date(long julian,size_type& year,size_type& month,size_type& day)
{
    long d;
    long j = julian - 1721119;
    year = (unsigned) (((j<<2) - 1) / 146097);
    j = (j<<2) - 1 - 146097 * year;
    d = (j>>2);
    j = ((d<<2) + 3) / 1461;
    d = (d<<2) + 3 - 1461*j;
    d = (d + 4)>>2;
    month = (unsigned)(5*d - 3)/153;
    d = 5*d - 3 - 153 * month;
    day = (unsigned)((d + 5)/5);
    year = (unsigned)(100 * year + j);
    if (month < 10)
    {
        month += 3;
    }
    else
    {
        month -= 9;
        ++year;
    } 
}
*/

} // namespace fbox

