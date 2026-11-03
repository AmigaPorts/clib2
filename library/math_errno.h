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

/* mathieeedoubbas.library doesn't overflow to infinity: it stops at DBL_MAX,
   so that exponent needs a closer look as well. */
#define __math_suspect_double(x) \
	((*(const __math_word *)&(x) & 0x7fe0) == 0x7fe0)
#define __math_suspect_float(x) __math_special_float(x)
#define __math_suspect_long_double(x) __math_special_long_double(x)

/****************************************************************************/

extern double __math_errno_double(double result,double x,double y);
extern float __math_errno_float(float result,float x,float y);
extern long double __math_errno_long_double(long double result,long double x,long double y);
extern double __math_overflow(double sign);
extern double __math_trig_special(double x);
extern double __math_exp_special(double x);
extern double __math_log_special(double x);
extern double __math_hyperbolic_special(double x,double sign);

/****************************************************************************/

#if LDBL_MANT_DIG == DBL_MANT_DIG
#undef __math_special_long_double
#undef __math_suspect_long_double
#define __math_special_long_double(x) __math_special_double(x)
#define __math_suspect_long_double(x) __math_suspect_double(x)
#endif /* LDBL_MANT_DIG == DBL_MANT_DIG */

/****************************************************************************/

#define __MATH_ERRNO_1(type,kind,name) \
	extern __stdargs type name(type x); \
	type name(type x) \
	{ \
		type r = __##name##_unchecked(x); \
		if(__math_suspect_##kind(r)) \
			r = __math_errno_##kind(r,x,x); \
		return(r); \
	}

/* For a function whose library call returns a number that looks fine for the
   arguments in the guard, and may be anything. */
#define __MATH_ERRNO_1_GUARD(type,kind,name,guard) \
	extern __stdargs type name(type x); \
	type name(type x) \
	{ \
		type r; \
		guard; \
		r = __##name##_unchecked(x); \
		if(__math_suspect_##kind(r)) \
			r = __math_errno_##kind(r,x,x); \
		return(r); \
	}

/* For a function whose result is never 0 for a finite argument, such as exp():
   a 0 is an underflow. The library returns 0 rather than a subnormal number,
   from below DBL_MIN or FLT_MIN on, so the guard alone doesn't catch it. */
#define __MATH_ERRNO_1_UNDERFLOW(type,kind,name,guard) \
	extern __stdargs type name(type x); \
	type name(type x) \
	{ \
		type r; \
		guard; \
		r = __##name##_unchecked(x); \
		if(__math_suspect_##kind(r)) \
			r = __math_errno_##kind(r,x,x); \
		else if(r == 0 && !__math_special_##kind(x)) \
			__set_errno(ERANGE); \
		return(r); \
	}

#define __MATH_ERRNO_2(type,kind,name) \
	extern __stdargs type name(type x,type y); \
	type name(type x,type y) \
	{ \
		type r = __##name##_unchecked(x,y); \
		if(__math_suspect_##kind(r)) \
			r = __math_errno_##kind(r,x,y); \
		return(r); \
	}

/* DBL_MAX is also what these return for an argument of DBL_MAX, whether that
   is in range or not, so overflowed says which it was. */
#define __MATH_ERRNO_2_OVERFLOW(type,kind,name,overflowed) \
	extern __stdargs type name(type x,type y); \
	type name(type x,type y) \
	{ \
		type r = __##name##_unchecked(x,y); \
		if(__math_suspect_##kind(r)) \
		{ \
			if(isfinite(r) && isfinite(x) && isfinite(y) && (overflowed)) \
				r = __math_overflow(r); \
			else if(__math_special_##kind(r)) \
				r = __math_errno_##kind(r,x,y); \
		} \
		return(r); \
	}

/* The third argument takes over unless the first two have a NaN, which is the
   same as an error that was there from the start. These work on the bits as
   well, so DBL_MAX really is DBL_MAX. For nonzero x and y, a 0 is an
   underflow unless x * y + z is exactly 0, which a rounded product can't tell:
   it may be -z when x * y + z is a tiny number that rounds to 0. So it is asked
   again of x and y scaled to [0.5,1) and z scaled with them, which is exact:
   with x * y + z that close to 0, z is close to -x * y and lands near 1. There
   nothing underflows, so the sum is 0 only if it is exactly 0. */
#define __MATH_ERRNO_3(type,kind,name,frexp,ldexp) \
	extern __stdargs type name(type x,type y,type z); \
	type name(type x,type y,type z) \
	{ \
		type r = __##name##_unchecked(x,y,z); \
		if(__math_special_##kind(r)) \
		{ \
			if(__math_special_##kind(z) && x == x && y == y) \
				r = __math_errno_##kind(r,z,z); \
			else \
				r = __math_errno_##kind(r,x,y); \
		} \
		else if(r == 0 && x != 0 && y != 0) \
		{ \
			int ex, ey; \
			type xs = frexp(x,&ex), ys = frexp(y,&ey); \
			if(z == 0 || __##name##_unchecked(xs,ys,ldexp(z,-(ex + ey))) != 0) \
				__set_errno(ERANGE); \
		} \
		return(r); \
	}

#define __MATH_ERRNO_REMQUO(type,kind,name) \
	extern __stdargs type name(type x,type y,int * q); \
	type name(type x,type y,int * q) \
	{ \
		type r = __##name##_unchecked(x,y,q); \
		if(__math_special_##kind(r)) \
			r = __math_errno_##kind(r,x,y); \
		return(r); \
	}

/* These two work on the bits, so DBL_MAX really is DBL_MAX. A nonzero x that
   scales to 0 has underflowed. */
#define __MATH_ERRNO_SCALE(type,kind,name,exponent) \
	extern __stdargs type name(type x,exponent n); \
	type name(type x,exponent n) \
	{ \
		type r = __##name##_unchecked(x,n); \
		if(__math_special_##kind(r)) \
			r = __math_errno_##kind(r,x,x); \
		else if(r == 0 && x != 0) \
			__set_errno(ERANGE); \
		return(r); \
	}

/* That the target is infinite doesn't excuse overflowing towards it, so only
   a NaN of the target counts. The way there is x + x, which leaves DBL_MAX
   where it is. */
#define __MATH_ERRNO_NEXT(type,kind,name,target) \
	extern __stdargs type name(type x,target y); \
	type name(type x,target y) \
	{ \
		type r = __##name##_unchecked(x,y); \
		if(__math_suspect_##kind(r)) \
		{ \
			if(__math_special_##kind(r)) \
				r = __math_errno_##kind(r,x,y == y ? x : (type)y); \
			else if(r == x && x != y && y == y) \
				r = __math_overflow(x); \
		} \
		return(r); \
	}

/* C leaves it open whether these are errors: ilogb() of 0, an infinity or a
   NaN, and a rounded number which doesn't fit. glibc says EDOM. */
#define __MATH_ERRNO_ILOGB(type,kind,name) \
	extern __stdargs int name(type x); \
	int name(type x) \
	{ \
		if(x == 0 || __math_special_##kind(x)) \
			__set_errno(EDOM); \
		return(__##name##_unchecked(x)); \
	}

/* Half the limit is enough to know that it fits, without rounding it twice. */
#define __MATH_ERRNO_ROUND(type,result,name,round,limit) \
	extern __stdargs result name(type x); \
	result name(type x) \
	{ \
		if(!(x > -(limit) / 2 && x < (limit) / 2)) \
		{ \
			const type rounded = round(x); \
			if(!(rounded >= -(limit) && rounded < (limit))) \
				__set_errno(EDOM); \
		} \
		return(__##name##_unchecked(x)); \
	}

#define __MATH_LONG_LIMIT 2147483648.0
#define __MATH_LONG_LONG_LIMIT 9223372036854775808.0

/****************************************************************************/

#endif /* __MATH_SETS_ERRNO */

/****************************************************************************/

#endif /* _MATH_ERRNO_H */
