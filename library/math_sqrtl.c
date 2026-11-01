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
sqrtl(long double x)
{
	return sqrt(x);
}

/****************************************************************************/

#else

/****************************************************************************/

#if defined(__mc68000__) && defined(M68881_FLOATING_POINT_SUPPORT)

long double
sqrtl(long double x)
{
	long double result;

	__asm ("fsqrt%.x %1,%0" : "=f" (result) : "f" (x));

	return(result);
}

#else

/* The square root of a 128 bit integer, one bit at a time, rounded to
   nearest; nothing else is exact enough.  The result is never halfway. */
long double
sqrtl(long double x)
{
	union ldshape u = { x };
	unsigned long long m = u.i.m;
	unsigned long long next = 0;
	unsigned long long root = 0;
	unsigned long long rest_high = 0;
	unsigned long long rest_low = 0;
	int exponent = u.i.se & 0x7FFF;
	int i;

	if((u.i.se >> 15) != 0 && (exponent != 0 || m != 0))
	{
		if(exponent != 0x7FFF || (m << 1) == 0)
			return((x - x) / (x - x));
	}

	if(exponent == 0x7FFF || (exponent == 0 && m == 0))
		return(x + x);

	/* x = m * 2^exponent, with the top bit of m set. */
	exponent -= 16383 + 63;

	while((m >> 63) == 0)
	{
		m <<= 1;
		exponent--;
	}

	/* m * 2^64 or m * 2^63, as an even power of two has to be left, is
	   the number to take the root of. */
	if((exponent & 1) != 0)
	{
		next = m << 63;
		m >>= 1;
		exponent -= 63;
	}
	else
	{
		exponent -= 64;
		next = 0;
	}

	for(i = 0 ; i < 64 ; i++)
	{
		unsigned long long trial_high;
		unsigned long long trial_low;

		/* rest = 4 * rest + the next two bits */
		rest_high = (rest_high << 2) | (rest_low >> 62);
		rest_low = (rest_low << 2) | (m >> 62);

		m = (m << 2) | (next >> 62);
		next <<= 2;

		/* trial = 4 * root + 1, in terms of the next root */
		trial_high = root >> 62;
		trial_low = (root << 2) | 1;

		root <<= 1;

		if(rest_high > trial_high || (rest_high == trial_high && rest_low >= trial_low))
		{
			rest_high -= trial_high + (rest_low < trial_low ? 1 : 0);
			rest_low -= trial_low;

			root |= 1;
		}
	}

	exponent /= 2;

	if(rest_high != 0 || rest_low > root)
	{
		root++;

		if(root == 0)
		{
			root = 1ULL << 63;
			exponent++;
		}
	}

	/* The root of a number of this format is always normal. */
	u.i.se = exponent + 16383 + 63;
	u.i.m = root;

	return(u.f);
}

#endif /* __mc68000__ && M68881_FLOATING_POINT_SUPPORT */

/****************************************************************************/

#endif /* LDBL_MANT_DIG == DBL_MANT_DIG && LDBL_MAX_EXP == DBL_MAX_EXP */

/****************************************************************************/

#if defined(__MATH_SETS_ERRNO)
#undef sqrtl
__MATH_ERRNO_1(long double, long_double, sqrtl)
#endif /* __MATH_SETS_ERRNO */

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
