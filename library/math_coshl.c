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
coshl(long double x)
{
	return cosh(x);
}

/****************************************************************************/

#else

/****************************************************************************/

/* From musl 1.2.5, src/math/coshl.c, under its MIT license: Copyright (c) 2005-2020
   Rich Felker, et al.  Further notices are in the text. */

#if defined(__mc68000__) && defined(M68881_FLOATING_POINT_SUPPORT)

long double
coshl(long double x)
{
	long double result;

	__asm ("fcosh%.x %1,%0" : "=f" (result) : "f" (x));

	return(result);
}

#else

long double coshl(long double x)
{
	union ldshape u = {x};
	unsigned ex = u.i.se & 0x7fff;
	uint32_t w;
	long double t;

	/* |x| */
	u.i.se = ex;
	x = u.f;
	w = u.i.m >> 32;

	/* |x| < log(2) */
	if (ex < 0x3fff-1 || (ex == 0x3fff-1 && w < 0xb17217f7)) {
		if (ex < 0x3fff-32) {
			FORCE_EVAL(x + 0x1p120f);
			return 1;
		}
		t = expm1l(x);
		return 1 + t*t/(2*(1+t));
	}

	/* |x| < log(LDBL_MAX) */
	if (ex < 0x3fff+13 || (ex == 0x3fff+13 && w < 0xb17217f7)) {
		t = expl(x);
		return 0.5*(t + 1/t);
	}

	/* |x| > log(LDBL_MAX) or nan */
	t = expl(0.5*x);
	return 0.5*t*t;
}

#endif /* __mc68000__ && M68881_FLOATING_POINT_SUPPORT */

/****************************************************************************/

#endif /* LDBL_MANT_DIG == DBL_MANT_DIG && LDBL_MAX_EXP == DBL_MAX_EXP */

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
