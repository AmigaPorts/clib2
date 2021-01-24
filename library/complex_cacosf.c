/*
 * Experimental: Not a part of clib2 from Olaf Barthel
 */

#ifndef _COMPLEX_HEADERS_H
#include "complex_headers.h"
#endif /* _COMPLEX_HEADERS_H */

/****************************************************************************/

#if defined(COMPLEX_SUPPORT)

/****************************************************************************/

extern float
complex cacosf(float complex z)
{
    z = casinf(z);
    return CMPLXF((float) M_PI_2 - crealf(z), -cimagf(z));
}

/****************************************************************************/

#endif /* COMPLEX_SUPPORT */
