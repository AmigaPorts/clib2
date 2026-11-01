/*
 * $Id: math_errno.c,v 1.0 2026-11-01 00:00:00 clib2devs Exp $
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

#ifndef _MATH_HEADERS_H
#include "math_headers.h"
#endif /* _MATH_HEADERS_H */

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT) && defined(__MATH_SETS_ERRNO)

/****************************************************************************/

/* A NaN that didn't come in is a domain error, and an infinity that didn't is
   a pole or an overflow. Whether an underflow sets errno is up to us, and that
   is left to the functions which already do it, such as ldexp(). */
static void
set_errno(int result_is_nan,int any_nan,int all_finite)
{
	if(result_is_nan)
	{
		if(!any_nan)
			__set_errno(EDOM);
	}
	else if(all_finite)
	{
		__set_errno(ERANGE);
	}
}

/****************************************************************************/

double
__math_overflow(double sign)
{
	__set_errno(ERANGE);

	return(copysign(HUGE_VAL,sign));
}

/****************************************************************************/

double
__math_errno_double(double result,double x,double y)
{
	if(isfinite(result))
	{
		/* Nothing but an overflow gets there from smaller numbers. */
		if(fabs(result) == DBL_MAX && fabs(x) < DBL_MAX && fabs(y) < DBL_MAX)
			result = __math_overflow(result);
	}
	else
	{
		set_errno(isnan(result),isnan(x) || isnan(y),isfinite(x) && isfinite(y));
	}

	return(result);
}

float
__math_errno_float(float result,float x,float y)
{
	set_errno(isnan(result),isnan(x) || isnan(y),isfinite(x) && isfinite(y));

	return(result);
}

long double
__math_errno_long_double(long double result,long double x,long double y)
{
	set_errno(isnan(result),isnan(x) || isnan(y),isfinite(x) && isfinite(y));

	return(result);
}

/****************************************************************************/

/* mathieeedoubtrans.library takes NaN and infinity for ordinary numbers: the
   sine of either is 0, and the cosine 1. */
double
__math_trig_special(double x)
{
	if(isnan(x))
		return(x);

	__set_errno(EDOM);

	return(__builtin_nan(""));
}

/* It also runs out of room before the result does: exp(1000) is 1.7e308. */
double
__math_exp_special(double x)
{
	if(isnan(x))
		return(x);

	if(isinf(x))
		return(x > 0 ? x : 0.0);

	if(x > 0)
		return(__math_overflow(1.0));

	__set_errno(ERANGE);

	return(0.0);
}

/* log(inf) is 709.78, as if it were a large number. */
double
__math_log_special(double x)
{
	if(isnan(x) || x > 0)
		return(x);

	__set_errno(EDOM);

	return(__builtin_nan(""));
}

double
__math_hyperbolic_special(double x,double sign)
{
	if(isnan(x))
		return(x);

	if(isinf(x))
		return(copysign(HUGE_VAL,sign));

	return(__math_overflow(sign));
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT && __MATH_SETS_ERRNO */
