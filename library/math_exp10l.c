/*
 * The 68k version, from musl 1.2.5, src/math/exp10l.c, under the MIT license
 * of musl: Copyright (c) 2005-2020 Rich Felker, et al.  Its origin and any
 * further notices are in the text below.
 */

#ifndef _MATH_MUSL_H
#include "math_musl.h"
#endif /* _MATH_MUSL_H */

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT) && defined(__mc68000__)

/****************************************************************************/

long double exp10l(long double x)
{
	static const long double p10[] = {
		1e-15L, 1e-14L, 1e-13L, 1e-12L, 1e-11L, 1e-10L,
		1e-9L, 1e-8L, 1e-7L, 1e-6L, 1e-5L, 1e-4L, 1e-3L, 1e-2L, 1e-1L,
		1, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9,
		1e10, 1e11, 1e12, 1e13, 1e14, 1e15
	};
	long double n, y = modfl(x, &n);
	union ldshape u = {n};
	/* fabsl(n) < 16 without raising invalid on nan */
	if ((u.i.se & 0x7fff) < 0x3fff+4) {
		if (!y) return p10[(int)n+15];
		/* log2(10) * y, and what rounding leaves out of that, which at
		   64 bits would be most of a unit in the last place. */
		static const long double log2_10 = 3.32192809488736234787031942948939L;
		static const long double log2_10_rest = 0x1.24afdbfd36bf7p-64L;
		long double t = log2_10 * y;
		long double rest = fmal(log2_10, y, -t) + log2_10_rest * y;
		long double r = exp2l(t);
		r += r * rest * 0.693147180559945309417232121458176568L;
		return r * p10[(int)n+15];
	}
	return powl(10.0, x);
}

long double pow10l(long double x)
{
	return exp10l(x);
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT && __mc68000__ */
