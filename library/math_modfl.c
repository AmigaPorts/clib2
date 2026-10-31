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
modfl(long double x, long double *iptr)
{
	double d;
	long double r = modf(x, &d);
	*iptr = d;

	return r;
}

/****************************************************************************/

#else

/****************************************************************************/

/* From musl 1.2.5, src/math/modfl.c, under its MIT license: Copyright (c) 2005-2020
   Rich Felker, et al.  Further notices are in the text. */

static const long double toint = 1/LDBL_EPSILON;

long double modfl(long double x, long double *iptr)
{
	union ldshape u = {x};
	int e = (u.i.se & 0x7fff) - 0x3fff;
	int s = u.i.se >> 15;
	long double absx;
	long double y;

	/* no fractional part */
	if (e >= LDBL_MANT_DIG-1) {
		*iptr = x;
		if (isnan(x))
			return x;
		return s ? -0.0 : 0.0;
	}

	/* no integral part*/
	if (e < 0) {
		*iptr = s ? -0.0 : 0.0;
		return x;
	}

	/* raises spurious inexact */
	absx = s ? -x : x;
	y = absx + toint - toint - absx;
	if (y == 0) {
		*iptr = x;
		return s ? -0.0 : 0.0;
	}
	if (y > 0)
		y -= 1;
	if (s)
		y = -y;
	*iptr = x + y;
	return -y;
}

/****************************************************************************/

#endif /* LDBL_MANT_DIG == DBL_MANT_DIG && LDBL_MAX_EXP == DBL_MAX_EXP */

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
