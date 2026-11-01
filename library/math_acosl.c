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
acosl(long double x)
{
	return acos(x);
}

/****************************************************************************/

#else

/****************************************************************************/

/* From musl 1.2.5, src/math/acosl.c, under its MIT license: Copyright (c) 2005-2020
   Rich Felker, et al.  Further notices are in the text. */

#if defined(__mc68000__) && defined(M68881_FLOATING_POINT_SUPPORT)

long double
acosl(long double x)
{
	long double result;

	__asm ("facos%.x %1,%0" : "=f" (result) : "f" (x));

	return(result);
}

#else

/* origin: FreeBSD /usr/src/lib/msun/src/e_acosl.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/*
 * See comments in acos.c.
 * Converted to long double by David Schultz <das@FreeBSD.ORG>.
 */


#define CLEARBOTTOM(u) (u.i.m &= -1ULL << 32)

long double acosl(long double x)
{
	union ldshape u = {x};
	long double z, s, c, f;
	uint16_t e = u.i.se & 0x7fff;

	/* |x| >= 1 or nan */
	if (e >= 0x3fff) {
		if (x == 1)
			return 0;
		if (x == -1)
			return 2*pio2_hi + 0x1p-120f;
		return 0/(x-x);
	}
	/* |x| < 0.5 */
	if (e < 0x3fff - 1) {
		if (e < 0x3fff - LDBL_MANT_DIG - 1)
			return pio2_hi + 0x1p-120f;
		return pio2_hi - (__invtrigl_R(x*x)*x - pio2_lo + x);
	}
	/* x < -0.5 */
	if (u.i.se >> 15) {
		z = (1 + x)*0.5;
		s = sqrtl(z);
		return 2*(pio2_hi - (__invtrigl_R(z)*s - pio2_lo + s));
	}
	/* x > 0.5 */
	z = (1 - x)*0.5;
	s = sqrtl(z);
	u.f = s;
	CLEARBOTTOM(u);
	f = u.f;
	c = (z - f*f)/(s + f);
	return 2*(__invtrigl_R(z)*s + c + f);
}

#endif /* __mc68000__ && M68881_FLOATING_POINT_SUPPORT */

/****************************************************************************/

#endif /* LDBL_MANT_DIG == DBL_MANT_DIG && LDBL_MAX_EXP == DBL_MAX_EXP */

/****************************************************************************/

#if defined(__MATH_SETS_ERRNO)
#undef acosl
__MATH_ERRNO_1(long double, long_double, acosl)
#endif /* __MATH_SETS_ERRNO */

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
