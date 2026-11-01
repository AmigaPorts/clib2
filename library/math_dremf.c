/*
 * Experimental: Not a part of clib2 from Olaf Barthel
 */

#ifndef _MATH_HEADERS_H
#include "math_headers.h"
#endif /* _MATH_HEADERS_H */

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT)

/****************************************************************************/

float
dremf(float x,float y)
{
	return(remainderf(x,y));
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
