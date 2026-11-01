/*
 * From musl 1.2.5, src/math/exp10f.c, under the MIT license of musl:
 * Copyright (c) 2005-2020 Rich Felker, et al.
 * Its origin and any further notices are in the text below.
 */

#ifndef _MATH_MUSL_H
#include "math_musl.h"
#endif /* _MATH_MUSL_H */

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT)

/****************************************************************************/

float exp10f(float x)
{
	static const float p10[] = {
		1e-7f, 1e-6f, 1e-5f, 1e-4f, 1e-3f, 1e-2f, 1e-1f,
		1, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7
	};
	float n, y = modff(x, &n);
	union {float f; uint32_t i;} u = {n};
	/* fabsf(n) < 8 without raising invalid on nan */
	if ((u.i>>23 & 0xff) < 0x7f+3) {
		if (!y) return p10[(int)n+7];
		y = exp2f(3.32192809488736234787031942948939f * y);
		return y * p10[(int)n+7];
	}
	return exp2(3.32192809488736234787031942948939 * x);
}


float pow10f(float x)
{
	return exp10f(x);
}

/****************************************************************************/

#if defined(__MATH_SETS_ERRNO)
#undef exp10f
__MATH_ERRNO_1(float, float, exp10f)
#endif /* __MATH_SETS_ERRNO */

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
