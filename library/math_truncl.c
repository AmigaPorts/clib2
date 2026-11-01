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
truncl(long double x)
{
	return trunc(x);
}

/****************************************************************************/

#else

/****************************************************************************/

/* From musl 1.2.5, src/math/truncl.c, under its MIT license: Copyright (c) 2005-2020
   Rich Felker, et al.  Further notices are in the text. */

static const long double toint = 1/LDBL_EPSILON;

long double truncl(long double x)
{
	union ldshape u = {x};
	int e = u.i.se & 0x7fff;
	int s = u.i.se >> 15;
	long double y;

	if (e >= 0x3fff+LDBL_MANT_DIG-1)
		return x;
	if (e <= 0x3fff-1) {
		FORCE_EVAL(x + 0x1p120f);
		return x*0;
	}
	/* y = int(|x|) - |x|, where int(|x|) is an integer neighbor of |x| */
	if (s)
		x = -x;
	y = x + toint - toint - x;
	if (y > 0)
		y -= 1;
	x += y;
	return s ? -x : x;
}


/****************************************************************************/

#endif /* LDBL_MANT_DIG == DBL_MANT_DIG && LDBL_MAX_EXP == DBL_MAX_EXP */

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
