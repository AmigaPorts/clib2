/*
 * $Id: stdlib_strtof.c,v 1.12 2006-09-25 14:51:15 obarthel Exp $
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

/* d, a finite double, rounded to the nearest float, ties to even; infinity
   if it is too large.  This works on the bits, because the conversion is
   not to be trusted: on a 68040 with its FPU support code loaded,
   (float)d was one unit in the last place off for almost a third of the
   numbers, and 1e39 became FLT_MAX instead of overflowing. */
static float
round_to_float(double d)
{
	unsigned long long bits, mantissa, rest, half;
	unsigned long sign, f;
	int exponent, shift;
	float result;

	memcpy(&bits,&d,sizeof(bits));

	sign = (unsigned long)(bits >> 32) & 0x80000000UL;
	exponent = (int)((bits >> 52) & 0x7FF);
	mantissa = bits & ((1ULL << 52) - 1);

	if(exponent == 0 && mantissa == 0)
	{
		f = sign;
		goto done;
	}

	/* Make the 53 bit mantissa explicit; a double denormal is far below
	   the smallest float denormal, but still rounds like one. */
	if(exponent != 0)
		mantissa |= 1ULL << 52;
	else
		exponent = 1;

	/* d = mantissa * 2^(exponent - 1075).  A float keeps 24 bits with a
	   biased exponent of exponent - 1023 + 127, or fewer as a denormal. */
	exponent = exponent - 1023 + 127;
	shift = 52 - 23;

	if(exponent <= 0)
	{
		shift += 1 - exponent;
		exponent = 0;
	}

	if(shift > 54)
	{
		f = sign;
		goto done;
	}

	rest = mantissa & ((1ULL << shift) - 1);
	half = 1ULL << (shift - 1);
	mantissa >>= shift;

	if(rest > half || (rest == half && (mantissa & 1) != 0))
		mantissa++;

	if(exponent == 0)
	{
		/* Rounding a denormal up can make it the smallest normal number,
		   which this gives too. */
		f = sign | (unsigned long)mantissa;
	}
	else
	{
		if(mantissa == (1ULL << 24))
		{
			mantissa >>= 1;
			exponent++;
		}

		if(exponent >= 255)
			f = sign | 0x7F800000UL;
		else
			f = sign | ((unsigned long)exponent << 23) | ((unsigned long)mantissa & 0x7FFFFFUL);
	}

 done:

	memcpy(&result,&f,sizeof(result));

	return(result);
}

/****************************************************************************/

/* The number as strtod() reads it, rounded to float.  Rounding twice, first
   to double and then to float, can be off by one in the last place of the
   float in rare halfway cases. */
float
strtof(const char * __restrict str, char ** __restrict ptr)
{
	struct __parsed_number pn;
	const char * stop = str;
	float result = 0.0f;
	double value;

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
	value = __parsed_to_double(&pn);

	if(pn.pn_kind == PN_NUMBER && NOT __builtin_isinf(value))
		result = round_to_float(value);
	else
		result = (float)value;

	if(pn.pn_kind == PN_NUMBER)
	{
		/* On overflow, HUGE_VALF, as with strtod() and HUGE_VAL. */
		if(__builtin_isinf(result) && NOT __builtin_isinf(value))
		{
			SHOWMSG("overflow");
			__set_errno(ERANGE);

			result = (value < 0) ? -HUGE_VALF : HUGE_VALF;
		}
		else if (result == 0.0f && value != 0.0)
		{
			SHOWMSG("underflow");
			__set_errno(ERANGE);
		}
	}

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
