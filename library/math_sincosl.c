/*
 * The 68k version, from musl 1.2.5, src/math/sincosl.c, under the MIT license
 * of musl: Copyright (c) 2005-2020 Rich Felker, et al.  Its origin and any
 * further notices are in the text below.
 */

#ifndef _MATH_MUSL_H
#include "math_musl.h"
#endif /* _MATH_MUSL_H */

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT) && defined(__mc68000__)

/****************************************************************************/

void sincosl(long double x, long double *sin, long double *cos)
{
	union ldshape u = {x};
	unsigned n;
	long double y[2], s, c;

	u.i.se &= 0x7fff;
	if (u.i.se == 0x7fff) {
		*sin = *cos = x - x;
		return;
	}
	if (u.f < M_PI_4) {
		if (u.i.se < 0x3fff - LDBL_MANT_DIG) {
			/* raise underflow if subnormal */
			if (u.i.se == 0) FORCE_EVAL(x*0x1p-120f);
			*sin = x;
			/* raise inexact if x!=0 */
			*cos = 1.0 + x;
			return;
		}
		*sin = __sinl(x, 0, 0);
		*cos = __cosl(x, 0);
		return;
	}
	n = __rem_pio2l(x, y);
	s = __sinl(y[0], y[1], 1);
	c = __cosl(y[0], y[1]);
	switch (n & 3) {
	case 0:
		*sin = s;
		*cos = c;
		break;
	case 1:
		*sin = c;
		*cos = -s;
		break;
	case 2:
		*sin = -s;
		*cos = -c;
		break;
	case 3:
	default:
		*sin = -c;
		*cos = s;
		break;
	}
}

/****************************************************************************/

#elif defined(FLOATING_POINT_SUPPORT)

/****************************************************************************/

void
sincosl(long double x, long double *sin, long double *cos)
{
	*sin = sinl(x);
	*cos = cosl(x);
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT && __mc68000__ */
