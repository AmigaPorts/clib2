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

/* <math.h> only declares these on request, and defining _GNU_SOURCE here would
   get us newlib's idea of locale_t from the headers we don't have. */
extern __stdargs double j0(double x);
extern __stdargs double j1(double x);
extern __stdargs double y0(double x);
extern __stdargs double y1(double x);
extern __stdargs float j0f(float x);
extern __stdargs float j1f(float x);
extern __stdargs float y0f(float x);
extern __stdargs float y1f(float x);

/****************************************************************************/

/* What the long double functions share. */
extern long double __polevll(long double x, const long double *P, int n);
extern long double __p1evll(long double x, const long double *P, int n);
extern void sincosl(long double x, long double *sin, long double *cos);
extern long double exp10l(long double x);
extern long double pow10l(long double x);
extern long double lgammal_r(long double x, int *sign);
extern double lgamma_r(double x, int *sign);
extern double jn(int n, double x);
extern double yn(int n, double x);
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
