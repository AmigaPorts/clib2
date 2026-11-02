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
		return powl(10.0L, x);
	}
	return powl(10.0, x);
}

/****************************************************************************/

#elif defined(FLOATING_POINT_SUPPORT)

/****************************************************************************/

long double
exp10l(long double x)
{
	return powl(10.0L, x);
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT && __mc68000__ */

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT) && defined(__MATH_SETS_ERRNO)
#undef exp10l
__MATH_ERRNO_1(long double, long_double, exp10l)
#endif /* FLOATING_POINT_SUPPORT && __MATH_SETS_ERRNO */

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT)

long double
pow10l(long double x)
{
	return exp10l(x);
}

#endif /* FLOATING_POINT_SUPPORT */
