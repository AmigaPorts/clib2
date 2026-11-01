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
remquol(long double x, long double y, int *quo)
{
	return remquo(x, y, quo);
}

/****************************************************************************/

#else

/****************************************************************************/

/* From musl 1.2.5, src/math/remquol.c, under its MIT license: Copyright (c) 2005-2020
   Rich Felker, et al.  Further notices are in the text. */

long double remquol(long double x, long double y, int *quo)
{
	union ldshape ux = {x}, uy = {y};
	int ex = ux.i.se & 0x7fff;
	int ey = uy.i.se & 0x7fff;
	int sx = ux.i.se >> 15;
	int sy = uy.i.se >> 15;
	uint32_t q;

	*quo = 0;
	if (y == 0 || isnan(y) || ex == 0x7fff)
		return (x*y)/(x*y);
	if (x == 0)
		return x;

	/* normalize x and y */
	if (!ex) {
		ux.i.se = ex;
		ux.f *= 0x1p120f;
		ex = ux.i.se - 120;
	}
	if (!ey) {
		uy.i.se = ey;
		uy.f *= 0x1p120f;
		ey = uy.i.se - 120;
	}

	q = 0;
	if (ex >= ey) {
		/* x mod y */
		uint64_t i, mx, my;
		mx = ux.i.m;
		my = uy.i.m;
		for (; ex > ey; ex--) {
			i = mx - my;
			if (mx >= my) {
				mx = 2*i;
				q++;
				q <<= 1;
			} else if (2*mx < mx) {
				mx = 2*mx - my;
				q <<= 1;
				q++;
			} else {
				mx = 2*mx;
				q <<= 1;
			}
		}
		i = mx - my;
		if (mx >= my) {
			mx = i;
			q++;
		}
		if (mx == 0)
			ex = -120;
		else
			for (; mx >> 63 == 0; mx *= 2, ex--);
		ux.i.m = mx;
	}

	/* scale result and decide between |x| and |x|-|y| */
	if (ex <= 0) {
		ux.i.se = ex + 120;
		ux.f *= 0x1p-120f;
	} else
		ux.i.se = ex;
	x = ux.f;
	if (sy)
		y = -y;
	if (ex == ey || (ex+1 == ey && (2*x > y || (2*x == y && q%2)))) {
		x -= y;
		q++;
	}
	q &= 0x7fffffff;
	*quo = sx^sy ? -(int)q : (int)q;
	return sx ? -x : x;
}

/****************************************************************************/

#endif /* LDBL_MANT_DIG == DBL_MANT_DIG && LDBL_MAX_EXP == DBL_MAX_EXP */

/****************************************************************************/

#if defined(__MATH_SETS_ERRNO)
#undef remquol
__MATH_ERRNO_REMQUO(long double, long_double, remquol)
#endif /* __MATH_SETS_ERRNO */

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
