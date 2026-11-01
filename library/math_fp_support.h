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

#if defined(M68881_FLOATING_POINT_SUPPORT)

/* Rounds |x| < 2^63 to an integer in the current rounding mode: fint is not in
   the hardware of a 68040. Next to 2^63, the extended format has no fraction
   left, so the addition rounds, once, and raises INEXACT if it has to. */
static __inline__ long double
__round_with_fpu(long double x, int negative)
{
	long double big = negative ? -0x1p63L : 0x1p63L;
	long double result = (x + big) - big;

	if(result == 0)
		return(negative ? -0.0L : 0.0L);

	return(result);
}

#endif /* M68881_FLOATING_POINT_SUPPORT */

/****************************************************************************/

/* The results of a domain error and of a pole, such as log(-1) and log(0). The
   FPU has to compute them, as that is what raises INVALID and DIVBYZERO. */
#if defined(M68881_FLOATING_POINT_SUPPORT)
#define __domain_error(x) (((x) - (x)) / ((x) - (x)))
#define __pole_error(x) (-1.0 / fabs(x))
#else
#define __domain_error(x) __builtin_nan("")
#define __pole_error(x) (-__inf())
#endif /* M68881_FLOATING_POINT_SUPPORT */

/****************************************************************************/

#endif /* _MATH_FP_SUPPORT_H */
