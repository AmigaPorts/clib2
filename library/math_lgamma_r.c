/*
 * Experimental: Not a part of clib2 from Olaf Barthel
 */

#ifndef _MATH_HEADERS_H
#include "math_headers.h"
#endif /* _MATH_HEADERS_H */

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT)

/****************************************************************************/

double
lgamma_r(double x,int * sign)
{
	return(__lgamma(x,sign));
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
