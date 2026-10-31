/*
 * Copyright (c) 2021 Frust GmbH
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _MATH_MUSL_H
#include "math_musl.h"
#endif /* _MATH_MUSL_H */

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT)

/****************************************************************************/

#if LDBL_MANT_DIG == DBL_MANT_DIG && LDBL_MAX_EXP == DBL_MAX_EXP

/****************************************************************************/

long double
atanl(long double x)
{
	return atan(x);
}

/****************************************************************************/

#else

/****************************************************************************/

/* From musl 1.2.5, src/math/atanl.c, under its MIT license: Copyright (c) 2005-2020
   Rich Felker, et al.  Further notices are in the text. */

#if defined(__mc68000__) && defined(M68881_FLOATING_POINT_SUPPORT)

long double
atanl(long double x)
{
	long double result;

	__asm ("fatan%.x %1,%0" : "=f" (result) : "f" (x));

	return(result);
}

#else

/* origin: FreeBSD /usr/src/lib/msun/src/s_atanl.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/*
 * See comments in atan.c.
 * Converted to long double by David Schultz <das@FreeBSD.ORG>.
 */


#define EXPMAN(u) ((u.i.se & 0x7fff)<<8 | (u.i.m>>55 & 0xff))

static const long double atanhi[] = {
	 4.63647609000806116202e-01L,
	 7.85398163397448309628e-01L,
	 9.82793723247329067960e-01L,
	 1.57079632679489661926e+00L,
};

static const long double atanlo[] = {
	 1.18469937025062860669e-20L,
	-1.25413940316708300586e-20L,
	 2.55232234165405176172e-20L,
	-2.50827880633416601173e-20L,
};

static const long double aT[] = {
	 3.33333333333333333017e-01L,
	-1.99999999999999632011e-01L,
	 1.42857142857046531280e-01L,
	-1.11111111100562372733e-01L,
	 9.09090902935647302252e-02L,
	-7.69230552476207730353e-02L,
	 6.66661718042406260546e-02L,
	-5.88158892835030888692e-02L,
	 5.25499891539726639379e-02L,
	-4.70119845393155721494e-02L,
	 4.03539201366454414072e-02L,
	-2.91303858419364158725e-02L,
	 1.24822046299269234080e-02L,
};

static long double T_even(long double x)
{
	return aT[0] + x * (aT[2] + x * (aT[4] + x * (aT[6] +
		x * (aT[8] + x * (aT[10] + x * aT[12])))));
}

static long double T_odd(long double x)
{
	return aT[1] + x * (aT[3] + x * (aT[5] + x * (aT[7] +
		x * (aT[9] + x * aT[11]))));
}

long double atanl(long double x)
{
	union ldshape u = {x};
	long double w, s1, s2, z;
	int id;
	unsigned e = u.i.se & 0x7fff;
	unsigned sign = u.i.se >> 15;
	unsigned expman;

	if (e >= 0x3fff + LDBL_MANT_DIG + 1) { /* if |x| is large, atan(x)~=pi/2 */
		if (isnan(x))
			return x;
		return sign ? -atanhi[3] : atanhi[3];
	}
	/* Extract the exponent and the first few bits of the mantissa. */
	expman = EXPMAN(u);
	if (expman < ((0x3fff - 2) << 8) + 0xc0) {  /* |x| < 0.4375 */
		if (e < 0x3fff - (LDBL_MANT_DIG+1)/2) {   /* if |x| is small, atanl(x)~=x */
			/* raise underflow if subnormal */
			if (e == 0)
				FORCE_EVAL((float)x);
			return x;
		}
		id = -1;
	} else {
		x = fabsl(x);
		if (expman < (0x3fff << 8) + 0x30) {  /* |x| < 1.1875 */
			if (expman < ((0x3fff - 1) << 8) + 0x60) { /*  7/16 <= |x| < 11/16 */
				id = 0;
				x = (2.0*x-1.0)/(2.0+x);
			} else {                                 /* 11/16 <= |x| < 19/16 */
				id = 1;
				x = (x-1.0)/(x+1.0);
			}
		} else {
			if (expman < ((0x3fff + 1) << 8) + 0x38) { /* |x| < 2.4375 */
				id = 2;
				x = (x-1.5)/(1.0+1.5*x);
			} else {                                 /* 2.4375 <= |x| */
				id = 3;
				x = -1.0/x;
			}
		}
	}
	/* end of argument reduction */
	z = x*x;
	w = z*z;
	/* break sum aT[i]z**(i+1) into odd and even poly */
	s1 = z*T_even(w);
	s2 = w*T_odd(w);
	if (id < 0)
		return x - x*(s1+s2);
	z = atanhi[id] - ((x*(s1+s2) - atanlo[id]) - x);
	return sign ? -z : z;
}

#endif /* __mc68000__ && M68881_FLOATING_POINT_SUPPORT */

/****************************************************************************/

#endif /* LDBL_MANT_DIG == DBL_MANT_DIG && LDBL_MAX_EXP == DBL_MAX_EXP */

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
