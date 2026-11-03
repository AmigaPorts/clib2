/*
 * $Id: math_fmaf.c,v 1.3 2006-01-08 12:04:23 obarthel Exp $
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

#if defined(FLOATING_POINT_SUPPORT)

/****************************************************************************/

#if defined(__mc68000__)

/****************************************************************************/

/* The product of two floats is exact in a double, so only the sum rounds, and
   then once more to float, which can round wrongly: the first rounding may
   have landed on a tie, or moved off one. Round to odd instead, which keeps
   the final rounding right, also for a subnormal float: go towards zero, and
   set the last bit when the sum isn't exact. */
float
fmaf(float x,float y,float z)
{
	union { double f; uint64_t i; } u;
	double xy, result, error;

	xy = (double)x * y;
	result = xy + z;

	u.f = result;

	if(((u.i >> 52) & 0x7ff) == 0x7ff)
		return((float)result);

	if(fabs(xy) > fabs(z))
		error = (xy - result) + z;
	else
		error = (z - result) + xy;

	if(error == 0)
		return((float)result);

	if((int)(u.i >> 63) != (error < 0))
		u.i--;

	u.i |= 1;

	return((float)u.f);
}

/****************************************************************************/

#else

/****************************************************************************/

float
fmaf(float x,float y,float z)
{
	/* ZZZ this should be a *fused* multiply & add, and
	   not a sequential operation as declared below! */
	return((x * y) + z);
}

/****************************************************************************/

#endif /* __mc68000__ */

/****************************************************************************/

#if defined(__MATH_SETS_ERRNO)
#undef fmaf
__MATH_ERRNO_3(float, float, fmaf, double)
#endif /* __MATH_SETS_ERRNO */

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
