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
hypotl(long double x, long double y)
{
	return hypot(x, y);
}

/****************************************************************************/

#else

/****************************************************************************/

/* From musl 1.2.5, src/math/hypotl.c, under its MIT license: Copyright (c) 2005-2020
   Rich Felker, et al.  Further notices are in the text. */

#define SPLIT (0x1p32L+1)

static void sq(long double *hi, long double *lo, long double x)
{
	long double xh, xl, xc;
	xc = x*SPLIT;
	xh = x - xc + xc;
	xl = x - xh;
	*hi = x*x;
	*lo = xh*xh - *hi + 2*xh*xl + xl*xl;
}

long double hypotl(long double x, long double y)
{
	union ldshape ux = {x}, uy = {y};
	int ex, ey;
	long double hx, lx, hy, ly, z;

	ux.i.se &= 0x7fff;
	uy.i.se &= 0x7fff;
	if (ux.i.se < uy.i.se) {
		ex = uy.i.se;
		ey = ux.i.se;
		x = uy.f;
		y = ux.f;
	} else {
		ex = ux.i.se;
		ey = uy.i.se;
		x = ux.f;
		y = uy.f;
	}

	if (ex == 0x7fff && isinf(y))
		return y;
	if (ex == 0x7fff || y == 0)
		return x;
	if (ex - ey > LDBL_MANT_DIG)
		return x + y;

	z = 1;
	if (ex > 0x3fff+8000) {
		z = 0x1p10000L;
		x *= 0x1p-10000L;
		y *= 0x1p-10000L;
	} else if (ey < 0x3fff-8000) {
		z = 0x1p-10000L;
		x *= 0x1p10000L;
		y *= 0x1p10000L;
	}
	sq(&hx, &lx, x);
	sq(&hy, &ly, y);
	return z*sqrtl(ly+lx+hy+hx);
}

/****************************************************************************/

#endif /* LDBL_MANT_DIG == DBL_MANT_DIG && LDBL_MAX_EXP == DBL_MAX_EXP */

/****************************************************************************/

#if defined(__MATH_SETS_ERRNO)
#undef hypotl
__MATH_ERRNO_2(long double, long_double, hypotl)
#endif /* __MATH_SETS_ERRNO */

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
