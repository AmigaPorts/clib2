/*
 * $Id: math_fma.c,v 1.3 2006-01-08 12:04:23 obarthel Exp $
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

/* Without an FPU, x * y + z rounds twice on the PowerPC too. */
#if defined(__mc68000__) || (defined(__PPC__) && defined(_SOFT_FLOAT))

/****************************************************************************/

/* The rounding mode field of the FPCR and the accrued flags of the FPSR. */
#define FPCR_ROUNDING	0x30UL
#define FPCR_DOWNWARD	0x20UL
#define FPCR_UPWARD		0x30UL
#define FPSR_INEXACT	0x08UL
#define FPSR_UNDERFLOW	0x20UL
#define FPSR_OVERFLOW	0x40UL

/****************************************************************************/

/* A normalized finite number: m * 2^e, with m's top bit at bit 53. */
struct num
{
	uint64_t	m;
	int			e;
};

/****************************************************************************/

static struct num
unpack(uint64_t bits)
{
	struct num n;

	n.m = bits & ((1ULL << 52) - 1);
	n.e = (bits >> 52) & 0x7ff;

	if(n.e == 0)
	{
		/* Subnormal. */
		n.e = 1;

		while((n.m & (1ULL << 52)) == 0)
		{
			n.m <<= 1;
			n.e--;
		}
	}
	else
	{
		n.m |= 1ULL << 52;
	}

	n.m <<= 1;
	n.e -= 0x3ff + 53;

	return(n);
}

/****************************************************************************/

static void
mul64(uint64_t * hi,uint64_t * lo,uint64_t x,uint64_t y)
{
	uint64_t x_lo = (uint32_t)x, x_hi = x >> 32;
	uint64_t y_lo = (uint32_t)y, y_hi = y >> 32;
	uint64_t t1 = x_lo * y_lo;
	uint64_t t2 = x_lo * y_hi + x_hi * y_lo;
	uint64_t t3 = x_hi * y_hi;

	(*lo) = t1 + (t2 << 32);
	(*hi) = t3 + (t2 >> 32) + (t1 > (*lo));
}

/****************************************************************************/

/* Multiply exactly in 128 bits, add, and round once, in the FPU's rounding mode
   if there is one and to nearest, ties to even, if not.
   On the 68000 and on the FPU alike x * y + z would round twice, and the
   extended precision of the FPU doesn't help: a product of doubles needs 106
   bits. This is after musl's fma(), which stays in integers up to the last
   step; so does this one, also for the last step. */
double
fma(double x,double y,double z)
{
	union { double f; uint64_t i; } ux, uy, uz, ur;
	uint64_t rhi, rlo, zhi, zlo, mantissa, lost, half;
	struct num nx, ny, nz;
	int sign, z_sign, e, d, biased, drop, away;
	unsigned long mode = 0, flags = 0;

	ux.f = x;
	uy.f = y;
	uz.f = z;

	/* A NaN, an infinity or an exact zero product: no rounding of the
	   product is involved. */
	if(((ux.i >> 52) & 0x7ff) == 0x7ff || ((uy.i >> 52) & 0x7ff) == 0x7ff || ((uz.i >> 52) & 0x7ff) == 0x7ff ||
	   (ux.i << 1) == 0 || (uy.i << 1) == 0)
	{
		return((x * y) + z);
	}

	nx = unpack(ux.i);
	ny = unpack(uy.i);

	mul64(&rhi,&rlo,nx.m,ny.m);

	sign = (int)((ux.i ^ uy.i) >> 63);
	e = nx.e + ny.e;

	z_sign = (int)(uz.i >> 63);
	zhi = zlo = 0;

	/* Line z up with the product, which keeps its low bits as a sticky bit
	   if it has to move. */
	if((uz.i << 1) != 0)
	{
		nz = unpack(uz.i);

		d = nz.e - e;

		if(d > 0)
		{
			if(d < 64)
			{
				zlo = nz.m << d;
				zhi = nz.m >> (64 - d);
			}
			else
			{
				zhi = nz.m;
				e = nz.e - 64;
				d -= 64;

				if(d >= 64)
				{
					rlo = 1;
					rhi = 0;
				}
				else if(d > 0)
				{
					rlo = (rhi << (64 - d)) | (rlo >> d) | ((rlo << (64 - d)) != 0);
					rhi >>= d;
				}
			}
		}
		else
		{
			d = -d;

			if(d == 0)
				zlo = nz.m;
			else if(d < 64)
				zlo = (nz.m >> d) | ((nz.m << (64 - d)) != 0);
			else
				zlo = 1;
		}
	}
	else
	{
		z_sign = sign;
	}

	if(sign == z_sign)
	{
		rlo += zlo;
		rhi += zhi + (rlo < zlo);
	}
	else
	{
		uint64_t t = rlo;

		rlo -= zlo;
		rhi = rhi - zhi - (t < rlo);

		if((rhi >> 63) != 0)
		{
			rlo = -rlo;
			rhi = -rhi - (rlo != 0);
			sign = NOT sign;
		}
	}

	/* An exact zero takes its sign from the rounding mode. */
	if(rhi == 0 && rlo == 0)
		return((x * y) + z);

	/* Bring the result into 63 bits, with bit 0 as the sticky bit. The
	   product and z leave rhi's top bits clear, so d is positive here. */
	if(rhi != 0)
	{
		d = __builtin_clzll(rhi) - 1;

		rhi = (rhi << d) | (rlo >> (64 - d)) | ((rlo << d) != 0);
		e += 64 - d;
	}
	else
	{
		d = __builtin_clzll(rlo) - 1;

		if(d < 0)
			rhi = (rlo >> 1) | (rlo & 1);
		else
			rhi = rlo << d;

		e -= d;
	}

	#if defined(M68881_FLOATING_POINT_SUPPORT)
	{
		__asm __volatile ("fmove%.l fpcr,%0"
		                  : "=dm" (mode)
		                  : /* no inputs */ );

		mode &= FPCR_ROUNDING;
	}
	#endif /* M68881_FLOATING_POINT_SUPPORT */

	/* The rounding modes that take a result away from zero if it is inexact. */
	away = (mode == FPCR_DOWNWARD && sign) || (mode == FPCR_UPWARD && NOT sign);

	/* The result is rhi * 2^e, with its top bit at bit 62. */
	biased = e + 62 + 0x3ff;

	ur.i = (uint64_t)sign << 63;

	if(biased >= 0x7ff)
	{
		flags = FPSR_OVERFLOW | FPSR_INEXACT;

		if(mode == 0 || away)
			ur.i |= 0x7ffULL << 52;
		else
			ur.i |= (0x7ffULL << 52) - 1;
	}
	else
	{
		drop = 10;

		if(biased <= 0)
			drop += 1 - biased;

		if(drop < 64)
		{
			mantissa = rhi >> drop;
			lost = rhi & ((1ULL << drop) - 1);
			half = 1ULL << (drop - 1);
		}
		else
		{
			mantissa = 0;
			lost = rhi;
			half = 1ULL << 63;
		}

		if(lost != 0)
		{
			flags = FPSR_INEXACT;

			if(biased <= 0)
				flags |= FPSR_UNDERFLOW;

			if(mode == 0)
			{
				if(lost > half || (lost == half && (mantissa & 1) != 0))
					mantissa++;
			}
			else if(away)
			{
				mantissa++;
			}
		}

		/* Rounding up may carry into the exponent, as it should. */
		if(biased > 0)
			ur.i += ((uint64_t)(biased - 1) << 52) + mantissa;
		else
			ur.i += mantissa;

		if(((ur.i >> 52) & 0x7ff) == 0x7ff)
			flags = FPSR_OVERFLOW | FPSR_INEXACT;
	}

	#if defined(M68881_FLOATING_POINT_SUPPORT)
	{
		unsigned long fpsr;

		__asm __volatile ("fmove%.l fpsr,%0"
		                  : "=dm" (fpsr)
		                  : /* no inputs */ );
		__asm __volatile ("fmove%.l %0,fpsr"
		                  : /* no outputs */
		                  : "dmi" (fpsr | flags));
	}
	#else
	{
		/* Without an FPU there are no flags to raise. */
		(void)flags;
	}
	#endif /* M68881_FLOATING_POINT_SUPPORT */

	return(ur.f);
}

/****************************************************************************/

#else

/****************************************************************************/

double
fma(double x,double y,double z)
{
	/* ZZZ this should be a *fused* multiply & add, and
	   not a sequential operation as declared below! */
	return((x * y) + z);
}

/****************************************************************************/

#endif /* __mc68000__ || (__PPC__ && _SOFT_FLOAT) */

/****************************************************************************/

#if defined(__MATH_SETS_ERRNO)
#undef fma
__MATH_ERRNO_3(double, double, fma, frexp, ldexp)
#endif /* __MATH_SETS_ERRNO */

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
