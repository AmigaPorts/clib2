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
sincosf(float x,float * s,float * c)
{
	(*s) = sinf(x);
	(*c) = cosf(x);
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
