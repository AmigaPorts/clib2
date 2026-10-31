/*
 * What the math functions from musl expect of its internal "libm.h".
 */

#ifndef _MATH_MUSL_H
#define _MATH_MUSL_H

/****************************************************************************/

#ifndef _MATH_HEADERS_H
#include "math_headers.h"
#endif /* _MATH_HEADERS_H */

#include <fenv.h>
#include <float.h>
#include <limits.h>
#include <stdint.h>

/****************************************************************************/

#define hidden

#define FORCE_EVAL(x) do { volatile __typeof(x) __x = (x); (void) __x; } while(0)

#define predict_true(x) __builtin_expect(!!(x), 1)
#define predict_false(x) __builtin_expect(x, 0)

/****************************************************************************/

/* The 96 bit format of the 68881, and of long double on the 68k. */
#if defined(__mc68000__)

union ldshape
{
	long double f;
	struct
	{
		uint16_t se;
		uint16_t pad;
		uint64_t m;
	} i;
};

#endif /* __mc68000__ */

/****************************************************************************/

/* What the long double functions share. */
extern long double __polevll(long double x, const long double *P, int n);
extern long double __p1evll(long double x, const long double *P, int n);
extern int __rem_pio2l(long double x, long double *y);
extern int __rem_pio2_large(double *x, double *y, int e0, int nx, int prec);
extern long double __sinl(long double x, long double y, int iy);
extern long double __cosl(long double x, long double y);
extern long double __tanl(long double x, long double y, int iy);

#define pio2_hi __pio2_hi
#define pio2_lo __pio2_lo
extern const long double pio2_hi, pio2_lo;
extern long double __invtrigl_R(long double z);

/****************************************************************************/

#endif /* _MATH_MUSL_H */
