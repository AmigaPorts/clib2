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
nextafterl(long double x, long double y)
{
	return nextafter(x, y);
}

/****************************************************************************/

#else

/****************************************************************************/

/* From musl 1.2.5, src/math/nextafterl.c, under its MIT license: Copyright (c) 2005-2020
   Rich Felker, et al.  Further notices are in the text. */

long double nextafterl(long double x, long double y)
{
	union ldshape ux, uy;

	if (isnan(x) || isnan(y))
		return x + y;
	if (x == y)
		return y;
	ux.f = x;
	if (x == 0) {
		uy.f = y;
		ux.i.m = 1;
		ux.i.se = uy.i.se & 0x8000;
	} else if ((x < y) == !(ux.i.se & 0x8000)) {
		/* Exponents 0 and 1 have the same weight on the x87.  Here each
		   one is half of the next, with the top bit set in both. */
		ux.i.m++;
		if (ux.i.m == 0) {
			ux.i.m = 1ULL << 63;
			ux.i.se++;
		}
	} else {
		if (ux.i.m << 1 == 0 && (ux.i.se & 0x7fff) != 0) {
			ux.i.se--;
			ux.i.m = 0;
		}
		ux.i.m--;
	}
	/* raise overflow if ux is infinite and x is finite */
	if ((ux.i.se & 0x7fff) == 0x7fff)
		return x + x;
	/* raise underflow if ux is subnormal or zero */
	if ((ux.i.se & 0x7fff) == 0 && ux.i.m>>63 == 0)
		FORCE_EVAL(x*x + ux.f*ux.f);
	return ux.f;
}

/****************************************************************************/

#endif /* LDBL_MANT_DIG == DBL_MANT_DIG && LDBL_MAX_EXP == DBL_MAX_EXP */

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
