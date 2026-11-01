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

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024

/****************************************************************************/

double
nexttoward(double x, long double y)
{
	return nextafter(x, y);
}

/****************************************************************************/

#else

/****************************************************************************/

/* From musl 1.2.5, src/math/nexttoward.c, under its MIT license: Copyright (c) 2005-2020
   Rich Felker, et al. */

double nexttoward(double x, long double y)
{
	union {double f; uint64_t i;} ux = {x};
	int e;

	if (isnan(x) || isnan(y))
		return x + y;
	if (x == y)
		return y;
	if (x == 0) {
		ux.i = 1;
		if (signbit(y))
			ux.i |= 1ULL<<63;
	} else if (x < y) {
		if (signbit(x))
			ux.i--;
		else
			ux.i++;
	} else {
		if (signbit(x))
			ux.i++;
		else
			ux.i--;
	}
	e = ux.i>>52 & 0x7ff;
	/* raise overflow if ux.f is infinite and x is finite */
	if (e == 0x7ff)
		FORCE_EVAL(x+x);
	/* raise underflow if ux.f is subnormal or zero */
	if (e == 0)
		FORCE_EVAL(x*x + ux.f*ux.f);
	return ux.f;
}

/****************************************************************************/

#endif /* LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024 */

/****************************************************************************/

#if defined(__MATH_SETS_ERRNO)
#undef nexttoward
__MATH_ERRNO_NEXT(double, double, nexttoward, long double)
#endif /* __MATH_SETS_ERRNO */

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
