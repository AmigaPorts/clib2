/*
 * $Id: math_fp_support.h,v 1.5 2006-01-08 12:04:23 obarthel Exp $
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

#ifndef _MATH_FP_SUPPORT_H
#define _MATH_FP_SUPPORT_H

/****************************************************************************/

/* If any of the three supported floating point support flavours is
   enabled, make sure that the generic 'FLOATING_POINT_SUPPORT' symbol
   is defined, too.*/
#if (defined(IEEE_FLOATING_POINT_SUPPORT) || defined(M68881_FLOATING_POINT_SUPPORT) || defined(PPC_FLOATING_POINT_SUPPORT))
#define FLOATING_POINT_SUPPORT
#endif /* IEEE_FLOATING_POINT_SUPPORT || M68881_FLOATING_POINT_SUPPORT || PPC_FLOATING_POINT_SUPPORT */

/****************************************************************************/

/* This selects whether or not the 'long double' type should be used. So far,
   only 'double' can be supported, and 'long double' defaults to 'double'. */
/*#define USE_LONG_DOUBLE*/

/****************************************************************************/
/* m68k-amigaos: the sys-include <sys/cdefs.h> defines __long_double_t. */
#if defined(__amigaos4__)
#if defined(USE_LONG_DOUBLE)
 typedef long double __long_double_t;
#else
 typedef double __long_double_t;
#endif /* USE_LONG_DOUBLE */
#endif /* __amigaos4__ */
/****************************************************************************/

/* 'Portable' raw representations of three IEEE floating point formats. */
union ieee_long_double
{
	long double		value;
	unsigned long	raw[3];
};

union ieee_double
{
	double			value;
	unsigned long	raw[2];
};

union ieee_single
{
	float			value;
	unsigned long	raw[1];
};

/****************************************************************************/

extern float __inff(void);
extern double __inf(void);

/****************************************************************************/

/* The results of a domain error and of a pole, such as log(-1) and log(0). The
   FPU has to compute them, as that is what raises INVALID and DIVBYZERO. */
#if defined(M68881_FLOATING_POINT_SUPPORT)
#define __domain_error(x) (((x) - (x)) / ((x) - (x)))
#define __pole_error(x) (-1.0 / fabs(x))
#define __raise_invalid() do { volatile double __zero = 0.0; __zero = __zero / __zero; } while(0)
#define FPSR_RAISE_OVERFLOW 0x48UL	/* OVFL and INEX in the accrued byte */
#define FPSR_RAISE_UNDERFLOW 0x28UL	/* UNFL and INEX */

/* Sets flags that the FPU has no reason to set by itself. */
static __inline__ void
__fpu_raise(unsigned long flags)
{
	unsigned long fpsr;

	__asm__ __volatile__ ("fmove%.l %/fpsr,%0" : "=d" (fpsr));
	__asm__ __volatile__ ("fmove%.l %0,%/fpsr" : : "d" (fpsr | flags));
}

/* 2^n, to scale a number with fmul: fscale is not in the hardware of a 68040
   or a 68060. Its exponent stays above 0, which is where the 68881's denormal
   numbers begin. */
#define MAX_POWER_OF_TWO 16382

static __inline__ long double
__power_of_two(int n)
{
	union ieee_long_double u;

	u.raw[0] = (unsigned long)(n + 16383) << 16;
	u.raw[1] = 0x80000000UL;
	u.raw[2] = 0;

	return(u.value);
}

/* ceil() and floor() work on the bits, and round away from zero when x has the
   sign in away: fint is not in the hardware of a 68040, and it raises INEXACT,
   which C23's Annex F says these functions don't. */
static __inline__ double
__round_to_integer(double x, unsigned long away)
{
	union ieee_double u;
	unsigned long hi, lo, sign, mask, next;
	int exponent;

	u.value = x;
	hi = u.raw[0];
	lo = u.raw[1];
	sign = hi & 0x80000000UL;
	exponent = (int)((hi >> 20) & 0x7ff) - 1023;

	if(exponent == 1024)
		return(x + x);

	if(exponent >= 52)
		return(x);

	if(exponent < 0)
	{
		if((hi & 0x7fffffffUL) == 0 && lo == 0)
			return(x);

		hi = (sign == away) ? (sign | 0x3ff00000UL) : sign;
		lo = 0;
	}
	else if(exponent < 20)
	{
		mask = 0xfffffUL >> exponent;

		if((hi & mask) == 0 && lo == 0)
			return(x);

		if(sign == away)
			hi += mask + 1;

		hi &= ~mask;
		lo = 0;
	}
	else
	{
		mask = 0xffffffffUL >> (exponent - 20);

		if((lo & mask) == 0)
			return(x);

		if(sign == away)
		{
			next = lo + mask;

			if(next < lo)
				hi++;

			lo = next;
		}

		lo &= ~mask;
	}

	u.raw[0] = hi;
	u.raw[1] = lo;

	return(u.value);
}

/* An inexact result that is tiny in its own format has underflowed, but the
   FPU can't know that: to its 80 bits the number is an ordinary one, and may
   well fit a double exactly when it is stored, which raises nothing.
   So the results of these functions are stored, for the flags of the format,
   and checked. */
static __inline__ double
__finish_double(double x)
{
	volatile double result = x;

	if(result != 0.0 && __builtin_fabs(result) < 0x1p-1022)
		__fpu_raise(FPSR_RAISE_UNDERFLOW);

	return(result);
}

static __inline__ float
__finish_float(float x)
{
	volatile float result = x;

	if(result != 0.0f && __builtin_fabsf(result) < 0x1p-126f)
		__fpu_raise(FPSR_RAISE_UNDERFLOW);

	return(result);
}

static __inline__ long double
__finish_long_double(long double x)
{
	if(x != 0.0L && __builtin_fabsl(x) < 0x1p-16383L)
		__fpu_raise(FPSR_RAISE_UNDERFLOW);

	return(x);
}
#else
#define __raise_invalid() ((void)0)
#define __finish_double(x) ((double)(x))
#define __finish_float(x) ((float)(x))
#define __finish_long_double(x) ((long double)(x))
#define __domain_error(x) __builtin_nan("")
#define __pole_error(x) (-__inf())
#endif /* M68881_FLOATING_POINT_SUPPORT */

/****************************************************************************/

#endif /* _MATH_FP_SUPPORT_H */
