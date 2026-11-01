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

#ifndef _MATH_HEADERS_H
#include "math_headers.h"
#endif /* _MATH_HEADERS_H */

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT)

/****************************************************************************/

#if LDBL_MANT_DIG == DBL_MANT_DIG && LDBL_MAX_EXP == DBL_MAX_EXP

/****************************************************************************/

#if defined(M68881_FLOATING_POINT_SUPPORT)

long double
scalbnl(long double x, int n)
{
	long double result;

	__asm ("fscale%.l %2,%0"
	       : "=f" (result)
	       : "0" (x), "d" (n));

	return(result);
}

#else

long double
scalbnl(long double x, int n)
{
	return scalbn(x, n);
}

/****************************************************************************/

#elif defined(__mc68000__)

/****************************************************************************/

/* The largest exponent change that can matter: from the smallest subnormal
   to infinity. */
#define MAX_SCALE 32900

/* The M68881's 80-bit format has a sign bit, 15 exponent bits and 16 unused
   bits in its first long word, then 64 mantissa bits with an explicit integer
   bit.  Its value is mantissa * 2^(exponent - 16383 - 63), also for exponent
   0, where the integer bit may be set: unlike on the x87, that is a normal
   number.  Working on the bits needs no FPU, and is exact on the 68000, where
   long double arithmetic goes through double. */
#if defined(M68881_FLOATING_POINT_SUPPORT)

long double
scalbnl(long double x, int n)
{
	long double result;

	__asm ("fscale%.l %2,%0"
	       : "=f" (result)
	       : "0" (x), "d" (n));

	return(result);
}

#else

long double
scalbnl(long double x, int n)
{
	union ieee_long_double u;
	unsigned long sign;
	uint64_t mantissa, lost, half;
	LONG exponent;
	int shift;

	u.value = x;

	sign = u.raw[0] & 0x80000000;
	exponent = (u.raw[0] >> 16) & 0x7fff;
	mantissa = ((uint64_t)u.raw[1] << 32) | u.raw[2];

	/* Infinity, NaN and zero stay what they are. */
	if(exponent == 0x7fff || mantissa == 0)
		return(x);

	if(n > MAX_SCALE)
		n = MAX_SCALE;
	else if(n < -MAX_SCALE)
		n = -MAX_SCALE;

	while((mantissa & (1ULL << 63)) == 0)
	{
		mantissa <<= 1;
		exponent--;
	}

	exponent += n;

	if(exponent >= 0x7fff)
	{
		u.raw[0] = sign | 0x7fff0000;
		u.raw[1] = u.raw[2] = 0;

		return(u.value);
	}

	/* Subnormal: shift the bits out, rounding to nearest, ties to even. */
	if(exponent < 0)
	{
		shift = -exponent;
		exponent = 0;

		if(shift > 64)
		{
			mantissa = 0;
		}
		else
		{
			half = 1ULL << 63;

			if(shift == 64)
			{
				lost = mantissa;
				mantissa = 0;
			}
			else
			{
				lost = mantissa << (64 - shift);
				mantissa >>= shift;
			}

			if(lost > half || (lost == half && (mantissa & 1) != 0))
				mantissa++;
		}
	}

	u.raw[0] = sign | ((unsigned long)exponent << 16);
	u.raw[1] = mantissa >> 32;
	u.raw[2] = mantissa;

	return(u.value);
}

#endif /* M68881_FLOATING_POINT_SUPPORT */

/****************************************************************************/

#endif /* LDBL_MANT_DIG == DBL_MANT_DIG && LDBL_MAX_EXP == DBL_MAX_EXP */

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
