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
atan2l(long double y, long double x)
{
	return atan2(y, x);
}

/****************************************************************************/

#else

/****************************************************************************/

/* From musl 1.2.5, src/math/atan2l.c, under its MIT license: Copyright (c) 2005-2020
   Rich Felker, et al.  Further notices are in the text. */

/* origin: FreeBSD /usr/src/lib/msun/src/e_atan2l.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 *
 */
/*
 * See comments in atan2.c.
 * Converted to long double by David Schultz <das@FreeBSD.ORG>.
 */



#if defined(M68881_FLOATING_POINT_SUPPORT)
#define atan2l __atan2l_unchecked
#endif /* M68881_FLOATING_POINT_SUPPORT */

long double atan2l(long double y, long double x)
{
	union ldshape ux, uy;
	long double z;
	int m, ex, ey;

	if (isnan(x) || isnan(y))
		return x+y;
	if (x == 1)
		return atanl(y);
	ux.f = x;
	uy.f = y;
	ex = ux.i.se & 0x7fff;
	ey = uy.i.se & 0x7fff;
	m = 2*(ux.i.se>>15) | uy.i.se>>15;
	if (y == 0) {
		switch(m) {
		case 0:
		case 1: return y;           /* atan(+-0,+anything)=+-0 */
		case 2: return  2*pio2_hi;  /* atan(+0,-anything) = pi */
		case 3: return -2*pio2_hi;  /* atan(-0,-anything) =-pi */
		}
	}
	if (x == 0)
		return m&1 ? -pio2_hi : pio2_hi;
	if (ex == 0x7fff) {
		if (ey == 0x7fff) {
			switch(m) {
			case 0: return  pio2_hi/2;   /* atan(+INF,+INF) */
			case 1: return -pio2_hi/2;   /* atan(-INF,+INF) */
			case 2: return  1.5*pio2_hi; /* atan(+INF,-INF) */
			case 3: return -1.5*pio2_hi; /* atan(-INF,-INF) */
			}
		} else {
			switch(m) {
			case 0: return  0.0;        /* atan(+...,+INF) */
			case 1: return -0.0;        /* atan(-...,+INF) */
			case 2: return  2*pio2_hi;  /* atan(+...,-INF) */
			case 3: return -2*pio2_hi;  /* atan(-...,-INF) */
			}
		}
	}
	if (ex+120 < ey || ey == 0x7fff)
		return m&1 ? -pio2_hi : pio2_hi;
	/* z = atan(|y/x|) without spurious underflow */
	if ((m&2) && ey+120 < ex)  /* |y/x| < 0x1p-120, x<0 */
		z = 0.0;
	else
		z = atanl(fabsl(y/x));
	switch (m) {
	case 0: return z;               /* atan(+,+) */
	case 1: return -z;              /* atan(-,+) */
	case 2: return 2*pio2_hi-(z-2*pio2_lo); /* atan(+,-) */
	default: /* case 3 */
		return (z-2*pio2_lo)-2*pio2_hi; /* atan(-,-) */
	}
}

#if defined(M68881_FLOATING_POINT_SUPPORT)

#undef atan2l

extern long double __atan2l_unchecked(long double y,long double x);

long double
atan2l(long double y,long double x)
{
	return(__finish_long_double(__atan2l_unchecked(y,x)));
}

#endif /* M68881_FLOATING_POINT_SUPPORT */

/****************************************************************************/

#endif /* LDBL_MANT_DIG == DBL_MANT_DIG && LDBL_MAX_EXP == DBL_MAX_EXP */

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
