/*
 * $Id: math_errno.h,v 1.0 2026-11-01 00:00:00 clib2devs Exp $
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

#ifndef _MATH_ERRNO_H
#define _MATH_ERRNO_H

/****************************************************************************/
#if defined(__MATH_SETS_ERRNO)

/****************************************************************************/

/* All that a result in range should cost is a look at its exponent, which
   takes no library call. math_errno.c works out the rest when it isn't. */
typedef unsigned short __attribute__((__may_alias__)) __math_word;

#define __math_special_double(x) \
	((*(const __math_word *)&(x) & 0x7ff0) == 0x7ff0)
#define __math_special_float(x) \
	((*(const __math_word *)&(x) & 0x7f80) == 0x7f80)
#define __math_special_long_double(x) \
	((*(const __math_word *)&(x) & 0x7fff) == 0x7fff)

/****************************************************************************/

extern void __math_errno_double(double result,double x,double y);
extern void __math_errno_float(float result,float x,float y);
extern void __math_errno_long_double(long double result,long double x,long double y);

/****************************************************************************/

#if LDBL_MANT_DIG == DBL_MANT_DIG
#undef __math_special_long_double
#define __math_special_long_double(x) __math_special_double(x)
#endif /* LDBL_MANT_DIG == DBL_MANT_DIG */

/****************************************************************************/

#define __MATH_ERRNO_1(type,kind,name) \
	extern __stdargs type name(type x); \
	type name(type x) \
	{ \
		type r = __##name##_unchecked(x); \
		if(__math_special_##kind(r)) \
			__math_errno_##kind(r,x,x); \
		return(r); \
	}

#define __MATH_ERRNO_2(type,kind,name) \
	extern __stdargs type name(type x,type y); \
	type name(type x,type y) \
	{ \
		type r = __##name##_unchecked(x,y); \
		if(__math_special_##kind(r)) \
			__math_errno_##kind(r,x,y); \
		return(r); \
	}

/* The third argument takes over unless the first two have a NaN, which is the
   same as an error that was there from the start. */
#define __MATH_ERRNO_3(type,kind,name) \
	extern __stdargs type name(type x,type y,type z); \
	type name(type x,type y,type z) \
	{ \
		type r = __##name##_unchecked(x,y,z); \
		if(__math_special_##kind(r)) \
		{ \
			if(__math_special_##kind(z) && x == x && y == y) \
				__math_errno_##kind(r,z,z); \
			else \
				__math_errno_##kind(r,x,y); \
		} \
		return(r); \
	}

#define __MATH_ERRNO_REMQUO(type,kind,name) \
	extern __stdargs type name(type x,type y,int * q); \
	type name(type x,type y,int * q) \
	{ \
		type r = __##name##_unchecked(x,y,q); \
		if(__math_special_##kind(r)) \
			__math_errno_##kind(r,x,y); \
		return(r); \
	}

#define __MATH_ERRNO_SCALE(type,kind,name,exponent) \
	extern __stdargs type name(type x,exponent n); \
	type name(type x,exponent n) \
	{ \
		type r = __##name##_unchecked(x,n); \
		if(__math_special_##kind(r)) \
			__math_errno_##kind(r,x,x); \
		return(r); \
	}

/* That the target is infinite doesn't excuse overflowing towards it, so only
   a NaN of the target counts. */
#define __MATH_ERRNO_NEXT(type,kind,name,target) \
	extern __stdargs type name(type x,target y); \
	type name(type x,target y) \
	{ \
		type r = __##name##_unchecked(x,y); \
		if(__math_special_##kind(r)) \
			__math_errno_##kind(r,x,y == y ? x : (type)y); \
		return(r); \
	}

/****************************************************************************/

#endif /* __MATH_SETS_ERRNO */

/****************************************************************************/

#endif /* _MATH_ERRNO_H */
