/*
 * $Id: math_nextafterf.c,v 1.3 2006-01-08 12:04:24 obarthel Exp $
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
 *
 *
 * PowerPC math library based in part on work by Sun Microsystems
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 *
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */

#ifndef _MATH_HEADERS_H
#include "math_headers.h"
#endif /* _MATH_HEADERS_H */

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT)

/****************************************************************************/

#if defined(M68881_FLOATING_POINT_SUPPORT)
#define nextafterf __nextafterf_unchecked
#endif /* M68881_FLOATING_POINT_SUPPORT */

float
nextafterf(float x, float y)
{
	long hx,hy,ix,iy;

	GET_FLOAT_WORD(hx,x);
	GET_FLOAT_WORD(hy,y);
	ix = hx&0x7fffffff;		/* |x| */
	iy = hy&0x7fffffff;		/* |y| */

	if((ix>0x7f800000) ||   /* x is nan */
	   (iy>0x7f800000))     /* y is nan */
	   return (float)(x+y);
	if(x==y) return y;		/* x=y, return y */
	if(ix==0) {				/* x == 0 */
	    SET_FLOAT_WORD(x,(hy&0x80000000U)|1);/* return +-minsubnormal */
	    y = x*x;
	    if(y==x) return y; else return x;	/* raise underflow flag */
	}
	if(hx>=0) {				/* x > 0 */
	    if(hx>hy) {				/* x > y, x -= ulp */
		hx -= 1;
	    } else {				/* x < y, x += ulp */
		hx += 1;
	    }
	} else {				/* x < 0 */
	    if(hy>=0||hx>hy){			/* x < y, x -= ulp */
		hx -= 1;
	    } else {				/* x > y, x += ulp */
		hx += 1;
	    }
	}
	hy = hx&0x7f800000;
	if(hy>=0x7f800000) return (float)(x+x);	/* overflow  */
	if(hy<0x00800000) {		/* underflow */
	    y = x*x;
	    if(y!=x) {		/* raise underflow flag */
	        SET_FLOAT_WORD(y,hx);
		return y;
	    }
	}
	SET_FLOAT_WORD(x,hx);
	return x;
}

/****************************************************************************/

#if defined(M68881_FLOATING_POINT_SUPPORT)

#undef nextafterf

extern float __nextafterf_unchecked(float x,float y);

/* The result is exact, but one that is denormal, or infinity, is an underflow
   or overflow all the same. */
float
nextafterf(float x,float y)
{
	volatile float result = __nextafterf_unchecked(x,y);

	if(x != y && NOT isnan(x) && NOT isnan(y))
	{
		if(isinf(result) && NOT isinf(x))
			__fpu_raise(FPSR_RAISE_OVERFLOW);
		else if (__builtin_fabsf(result) < 0x1p-126f)
			__fpu_raise(FPSR_RAISE_UNDERFLOW);
	}

	return(result);
}

#endif /* M68881_FLOATING_POINT_SUPPORT */

/****************************************************************************/

#if defined(__MATH_SETS_ERRNO)
#undef nextafterf
__MATH_ERRNO_NEXT(float, float, nextafterf, float)
#endif /* __MATH_SETS_ERRNO */

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
