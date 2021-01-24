/*
 * Experimental: Not a part of clib2 from Olaf Barthel
 */

#ifndef _COMPLEX_HEADERS_H
#include "complex_headers.h"
#endif /* _COMPLEX_HEADERS_H */

/****************************************************************************/

#if defined(COMPLEX_SUPPORT)

/****************************************************************************/

float complex
cexpf(float complex z)
{
    return CMPLXF(0.0, 0.0);
}

/****************************************************************************/

#endif /* COMPLEX_SUPPORT */
