/*
 * Experimental: Not a part of clib2 from Olaf Barthel
 */

#ifndef _MATH_HEADERS_H
#include "math_headers.h"
#endif /* _MATH_HEADERS_H */

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT)

/****************************************************************************/

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024

/****************************************************************************/

double
scalbln(double x, long n)
{
    return scalbn(x, n > INT_MAX ? INT_MAX : n < INT_MIN ? INT_MIN : n);
}

/****************************************************************************/

#endif /* LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024 */

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
