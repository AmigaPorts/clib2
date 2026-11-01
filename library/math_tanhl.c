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
tanhl(long double x)
{
	return tanh(x);
}

/****************************************************************************/

#else

/****************************************************************************/

/* From musl 1.2.5, src/math/tanhl.c, under its MIT license: Copyright (c) 2005-2020
   Rich Felker, et al.  Further notices are in the text. */

#if defined(__mc68000__) && defined(M68881_FLOATING_POINT_SUPPORT)

long double
tanhl(long double x)
{
	long double result;

	__asm ("ftanh%.x %1,%0" : "=f" (result) : "f" (x));

	return(result);
}

#else

long double tanhl(long double x)
{
	union ldshape u = {x};
	unsigned ex = u.i.se & 0x7fff;
	unsigned sign = u.i.se & 0x8000;
	uint32_t w;
	long double t;

	/* x = |x| */
	u.i.se = ex;
	x = u.f;
	w = u.i.m >> 32;

	if (ex > 0x3ffe || (ex == 0x3ffe && w > 0x8c9f53d5)) {
		/* |x| > log(3)/2 ~= 0.5493 or nan */
		if (ex >= 0x3fff+5) {
			/* |x| >= 32 */
			t = 1 + 0/(x + 0x1p-120f);
		} else {
			t = expm1l(2*x);
			t = 1 - 2/(t+2);
		}
	} else if (ex > 0x3ffd || (ex == 0x3ffd && w > 0x82c577d4)) {
		/* |x| > log(5/3)/2 ~= 0.2554 */
		t = expm1l(2*x);
		t = t/(t+2);
	} else {
		/* |x| is small */
		t = expm1l(-2*x);
		t = -t/(t+2);
	}
	return sign ? -t : t;
}

#endif /* __mc68000__ && M68881_FLOATING_POINT_SUPPORT */

/****************************************************************************/

#endif /* LDBL_MANT_DIG == DBL_MANT_DIG && LDBL_MAX_EXP == DBL_MAX_EXP */

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
