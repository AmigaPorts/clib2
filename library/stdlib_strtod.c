/*
 * $Id: stdlib_strtod.c,v 1.12 2006-09-25 14:51:15 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2015 by Olaf Barthel <obarthel (at) gmx.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   - Neither the name of Olaf Barthel nor the names of contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _STDLIB_NULL_POINTER_CHECK_H
#include "stdlib_null_pointer_check.h"
#endif /* _STDLIB_NULL_POINTER_CHECK_H */

/****************************************************************************/

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

#ifndef _LOCALE_HEADERS_H
#include "locale_headers.h"
#endif /* _LOCALE_HEADERS_H */

/****************************************************************************/

#ifndef _MATH_FP_SUPPORT_H
#include "math_fp_support.h"
#endif /* _MATH_FP_SUPPORT_H */

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT)

/****************************************************************************/

#ifndef _MATH_HEADERS_H
#include "math_headers.h"
#endif /* _MATH_HEADERS_H */

#ifndef _STDLIB_STRTOD_PARSE_H
#include "stdlib_strtod_parse.h"
#endif /* _STDLIB_STRTOD_PARSE_H */

/****************************************************************************/

#if LDBL_MANT_DIG == DBL_MANT_DIG && LDBL_MAX_EXP == DBL_MAX_EXP

/* long double is double, as on AmigaOS 4, and there is nothing wider to
   compute in.  So the scaling uses a small extended format of its own: a
   64 bit mantissa and a binary exponent, as m68k's long double has, with
   the powers of ten exactly rounded to it.  A few roundings at 64 bits
   leave the result close enough that rounding it once to the 53 bits of a
   double is almost always correct, and otherwise one unit in the last
   place off. */

struct xfloat
{
	unsigned long long	xf_mantissa;	/* 2^63 <= mantissa < 2^64 */
	int					xf_exponent;	/* value = mantissa * 2^exponent */
};

/* 10^(2^i) and 10^-(2^i), i = 0 ... 8, rounded to nearest. */
static const struct xfloat positive_powers[] =
{
	{ 0xa000000000000000ULL, -60 },	/* 10^1 */
	{ 0xc800000000000000ULL, -57 },	/* 10^2 */
	{ 0x9c40000000000000ULL, -50 },	/* 10^4 */
	{ 0xbebc200000000000ULL, -37 },	/* 10^8 */
	{ 0x8e1bc9bf04000000ULL, -10 },	/* 10^16 */
	{ 0x9dc5ada82b70b59eULL, 43 },	/* 10^32 */
	{ 0xc2781f49ffcfa6d5ULL, 149 },	/* 10^64 */
	{ 0x93ba47c980e98ce0ULL, 362 },	/* 10^128 */
	{ 0xaa7eebfb9df9de8eULL, 787 },	/* 10^256 */
};

static const struct xfloat negative_powers[] =
{
	{ 0xcccccccccccccccdULL, -67 },	/* 10^-1 */
	{ 0xa3d70a3d70a3d70aULL, -70 },	/* 10^-2 */
	{ 0xd1b71758e219652cULL, -77 },	/* 10^-4 */
	{ 0xabcc77118461cefdULL, -90 },	/* 10^-8 */
	{ 0xe69594bec44de15bULL, -117 },	/* 10^-16 */
	{ 0xcfb11ead453994baULL, -170 },	/* 10^-32 */
	{ 0xa87fea27a539e9a5ULL, -276 },	/* 10^-64 */
	{ 0xddd0467c64bce4a1ULL, -489 },	/* 10^-128 */
	{ 0xc0314325637a193aULL, -914 },	/* 10^-256 */
};

#define NUM_POWERS ((int)(sizeof(positive_powers) / sizeof(positive_powers[0])))

/* 10^0 ... 10^22, which are all exact in a double. */
static const double exact_powers_of_ten[] =
{
	1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10, 1e11,
	1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22
};

/* The largest integer below which every integer is exact in a double. */
#define EXACT_INTEGER_LIMIT (1ULL << DBL_MANT_DIG)

/* m * 2^e, for m != 0, as an xfloat. */
static struct xfloat
xfloat_from(unsigned long long m, int e)
{
	struct xfloat x;

	while((m & (1ULL << 63)) == 0)
	{
		m <<= 1;
		e--;
	}

	x.xf_mantissa = m;
	x.xf_exponent = e;

	return(x);
}

/* a * b, rounded to nearest. */
static struct xfloat
xfloat_multiply(struct xfloat a, struct xfloat b)
{
	unsigned long long a0 = a.xf_mantissa & 0xFFFFFFFFUL, a1 = a.xf_mantissa >> 32;
	unsigned long long b0 = b.xf_mantissa & 0xFFFFFFFFUL, b1 = b.xf_mantissa >> 32;
	unsigned long long p00 = a0 * b0, p01 = a0 * b1, p10 = a1 * b0, p11 = a1 * b1;
	unsigned long long middle = (p00 >> 32) + (p01 & 0xFFFFFFFFUL) + (p10 & 0xFFFFFFFFUL);
	unsigned long long high = p11 + (p01 >> 32) + (p10 >> 32) + (middle >> 32);
	unsigned long long low = (middle << 32) | (p00 & 0xFFFFFFFFUL);
	struct xfloat x;

	x.xf_exponent = a.xf_exponent + b.xf_exponent + 64;

	/* The product of two numbers in [2^63,2^64) is in [2^126,2^128). */
	if((high & (1ULL << 63)) == 0)
	{
		high = (high << 1) | (low >> 63);
		low <<= 1;
		x.xf_exponent--;
	}

	if(low & (1ULL << 63))
	{
		high++;

		if(high == 0)
		{
			high = 1ULL << 63;
			x.xf_exponent++;
		}
	}

	x.xf_mantissa = high;

	return(x);
}

/* x rounded to the nearest double, denormals included; infinity on
   overflow, 0 on underflow. */
static double
xfloat_to_double(struct xfloat x)
{
	unsigned long long m = x.xf_mantissa, rest, half, bits;
	int biased_exponent, shift;
	double d;

	/* x = m * 2^e with m in [2^63,2^64): as a double, m keeps 53 bits
	   and its exponent is e + 63. */
	biased_exponent = x.xf_exponent + 63 + 1023;

	if(biased_exponent >= 2047)
		return(__builtin_inf());

	shift = 11;
	if(biased_exponent <= 0)
	{
		/* A denormal: fewer bits are left. */
		shift += 1 - biased_exponent;
		biased_exponent = 0;
	}

	if(shift > 64)
		return(0.0);

	if(shift == 64)
	{
		rest = m;
		m = 0;
	}
	else
	{
		rest = m & ((1ULL << shift) - 1);
		m >>= shift;
	}

	half = 1ULL << (shift - 1);
	if(rest > half || (rest == half && (m & 1) != 0))
		m++;

	if(biased_exponent == 0)
	{
		/* Rounding a denormal up can make it the smallest normal number,
		   which this gives too. */
		bits = m;
	}
	else
	{
		if(m == (1ULL << 53))
		{
			m >>= 1;
			biased_exponent++;

			if(biased_exponent >= 2047)
				return(__builtin_inf());
		}

		bits = ((unsigned long long)biased_exponent << 52) | (m & ((1ULL << 52) - 1));
	}

	memcpy(&d,&bits,sizeof(d));

	return(d);
}

/* The magnitude of a PN_NUMBER; overflow gives infinity, underflow 0. */
static double
parsed_to_double(const struct __parsed_number * pn)
{
	unsigned long long mantissa = pn->pn_mantissa;
	long exponent = pn->pn_exponent;
	struct xfloat x;
	unsigned long e;
	int i;

	if(mantissa == 0)
		return(0.0);

	if(pn->pn_is_hex)
	{
		/* Far outside the range of a double, whatever the mantissa. */
		if(exponent > 2000)
			return(__builtin_inf());

		if(exponent < -2000)
			return(0.0);

		return(xfloat_to_double(xfloat_from(mantissa, (int)exponent)));
	}

	/* Clinger's fast path: if both the digits and the power of ten are
	   exact in a double, a single multiplication or division rounds the
	   number correctly. */
	if(mantissa < EXACT_INTEGER_LIMIT)
	{
		if(0 <= exponent && exponent <= 22)
			return((double)mantissa * exact_powers_of_ten[exponent]);

		if(-22 <= exponent && exponent < 0)
			return((double)mantissa / exact_powers_of_ten[-exponent]);
	}

	/* With at most 19 digits, anything this far out is beyond the range
	   of a double. */
	if(exponent > 330)
		return(__builtin_inf());

	if(exponent < -360)
		return(0.0);

	x = xfloat_from(mantissa, 0);

	e = (exponent < 0) ? (unsigned long)(-exponent) : (unsigned long)exponent;

	for(i = 0 ; i < NUM_POWERS && e != 0 ; i++, e >>= 1)
	{
		if(e & 1)
			x = xfloat_multiply(x, (exponent < 0) ? negative_powers[i] : positive_powers[i]);
	}

	return(xfloat_to_double(x));
}

#else

/* long double is wider than double, as on m68k-amigaos: scale in long
   double, then round once to double.  The conversion is not trusted to
   round to nearest: m68k-amigaos's soft-float one truncates, which made
   almost every other number one unit in the last place too small.  So
   x is compared with the doubles on either side of it. */
static double
round_to_double(long double x)
{
	unsigned long long bits;
	double d, below, above;
	long double lo, hi;

	d = (double)x;
	if(x == 0.0L || __builtin_isinf(d) || __builtin_isnan(d) || (long double)d == x)
		return(d);

	/* x > 0 lies between d and one of its neighbours. */
	memcpy(&bits,&d,sizeof(bits));

	if((long double)d < x)
	{
		below = d;
		bits++;
		memcpy(&above,&bits,sizeof(above));
	}
	else
	{
		above = d;
		bits--;
		memcpy(&below,&bits,sizeof(below));
	}

	/* Above DBL_MAX, the next double would be 2^1024. */
	lo = below;
	hi = __builtin_isinf(above) ? 0x1p1024L : (long double)above;

	if(x - lo < hi - x)
		return(below);

	if(x - lo > hi - x)
		return(above);

	/* A tie: the one with an even mantissa. */
	memcpy(&bits,&below,sizeof(bits));

	return((bits & 1) == 0 ? below : above);
}

static double
parsed_to_double(const struct __parsed_number * pn)
{
	return(round_to_double(__parsed_to_long_double(pn)));
}

#endif /* LDBL_MANT_DIG == DBL_MANT_DIG && LDBL_MAX_EXP == DBL_MAX_EXP */

/****************************************************************************/

/* What strtod() returns for what __parse_number() read, with errno set to
   ERANGE on overflow and underflow; strtof() starts from this too. */
double
__parsed_to_double(const struct __parsed_number * pn)
{
	double result;

	switch(pn->pn_kind)
	{
		case PN_INFINITY:

			result = __builtin_inf();
			break;

		case PN_NAN:

			result = __builtin_nan("");
			break;

		case PN_NUMBER:

			result = parsed_to_double(pn);

			/* On overflow, HUGE_VAL, as clib2's math functions return it
			   too: the largest double, not infinity. */
			if(__builtin_isinf(result))
			{
				SHOWMSG("overflow");
				__set_errno(ERANGE);

				result = HUGE_VAL;
			}
			else if (result == 0.0 && pn->pn_mantissa != 0)
			{
				SHOWMSG("underflow");
				__set_errno(ERANGE);
			}

			break;

		default:

			result = 0.0;
			break;
	}

	if(pn->pn_is_negative)
		result = -result;

	return(result);
}

/****************************************************************************/

double
strtod(const char * __restrict str, char ** __restrict ptr)
{
	struct __parsed_number pn;
	const char * stop = str;
	double result = 0.0;

	ENTER();

	SHOWSTRING(str);
	SHOWPOINTER(ptr);

	assert( str != NULL );

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(str == NULL)
		{
			SHOWMSG("invalid str parameter");

			__set_errno(EFAULT);
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	stop = __parse_number(str,&pn);
	result = __parsed_to_double(&pn);

#if defined(CHECK_FOR_NULL_POINTERS)
 out:
#endif /* CHECK_FOR_NULL_POINTERS */

	/* If desired, remember where we stopped reading the
	   number from the buffer. */
	if(ptr != NULL)
		(*ptr) = (char *)stop;

	RETURN(result);
	return(result);
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
