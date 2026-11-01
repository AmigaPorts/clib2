/*
 * Experimental: Not a part of clib2 from Olaf Barthel
 */

#ifndef _MATH_HEADERS_H
#include "math_headers.h"
#endif /* _MATH_HEADERS_H */

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT)

/****************************************************************************/

void
sincos(double x,double * s,double * c)
{
	(*s) = sin(x);
	(*c) = cos(x);
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
