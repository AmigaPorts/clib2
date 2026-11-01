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
acoshl(long double x)
{
	return acosh(x);
}

/****************************************************************************/

#else

/****************************************************************************/

/* From musl 1.2.5, src/math/acoshl.c, under its MIT license: Copyright (c) 2005-2020
   Rich Felker, et al.  Further notices are in the text. */

/* acosh(x) = log(x + sqrt(x*x-1)) */
long double acoshl(long double x)
{
	union ldshape u = {x};
	int e = u.i.se;

	if (e < 0x3fff + 1)
		/* 0 <= x < 2, invalid if x < 1 */
		return log1pl(x-1 + sqrtl((x-1)*(x-1)+2*(x-1)));
	if (e < 0x3fff + 32)
		/* 2 <= x < 0x1p32 */
		return logl(2*x - 1/(x+sqrtl(x*x-1)));
	if (e & 0x8000)
		/* x < 0 or x = -0, invalid */
		return (x - x) / (x - x);
	/* 0x1p32 <= x or nan */
	return logl(x) + 0.693147180559945309417232121458176568L;
}

/****************************************************************************/

#endif /* LDBL_MANT_DIG == DBL_MANT_DIG && LDBL_MAX_EXP == DBL_MAX_EXP */

/****************************************************************************/

#if defined(__MATH_SETS_ERRNO)
#undef acoshl
__MATH_ERRNO_1(long double, long_double, acoshl)
#endif /* __MATH_SETS_ERRNO */

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
