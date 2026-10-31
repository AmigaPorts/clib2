/*
 * $Id: math.h,v 1.22 2007-01-06 10:09:49 obarthel Exp $
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
 *****************************************************************************
 *
 * Documentation and source code for this library, and the most recent library
 * build are available from <http://sourceforge.net/projects/clib2>.
 *
 *****************************************************************************
 */

#ifndef _MATH_H
#define _MATH_H

/****************************************************************************/

#ifndef _SYS_CLIB2_STDC_H
#include <sys/clib2_stdc.h>
#endif /* _SYS_CLIB2_STDC_H */

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************/

#ifndef _STDLIB_H
#include <stdlib.h>
#endif /* _STDLIB_H */

/****************************************************************************/

extern double __huge_val;

/****************************************************************************/

#if defined(__GNUC__) && __GNUC__ >= 4
#define	HUGE_VAL __DBL_MAX__
#else
#define	HUGE_VAL ((const double)__huge_val)
#endif /* __GNUC__ */

/****************************************************************************/

extern __stdargs double acos(double x);
extern __stdargs double asin(double x);
extern __stdargs double atan(double x);
extern __stdargs double atan2(double y,double x);
extern __stdargs double ceil(double x);
extern __stdargs double cos(double x);
extern __stdargs double cosh(double x);
extern __stdargs double exp(double x);
extern __stdargs double fabs(double x);
extern __stdargs double floor(double x);
extern __stdargs double fmod(double x,double y);
extern __stdargs double frexp(double x,int *nptr);
extern __stdargs double ldexp(double x,int n);
extern __stdargs double log(double x);
extern __stdargs double log10(double x);
extern __stdargs double modf(double x,double *nptr);
extern __stdargs double pow(double x,double y);
extern __stdargs double sin(double x);
extern __stdargs double sinh(double x);
extern __stdargs double sqrt(double x);
extern __stdargs double tan(double x);
extern __stdargs double tanh(double x);

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

#define	M_E			2.7182818284590452354
#define	M_LOG2E		1.4426950408889634074
#define	M_LOG10E	0.43429448190325182765
#define	M_LN2		0.69314718055994530942
#define	M_LN10		2.30258509299404568402
#define	M_PI		3.14159265358979323846
#define	M_PI_2		1.57079632679489661923
#define	M_PI_4		0.78539816339744830962
#define	M_1_PI		0.31830988618379067154
#define	M_2_PI		0.63661977236758134308
#define	M_2_SQRTPI	1.12837916709551257390
#define	M_SQRT2		1.41421356237309504880
#define	M_SQRT1_2	0.70710678118654752440

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard, but it should
   be part of ISO/IEC 9899:1999, also known as "C99". */

/****************************************************************************/

/* long double: on AmigaOS 4, long double is the same as double, and all
   of the long double functions declared here and in <complex.h> are
   there.  On m68k-amigaos, long double is the 68881's 80-bit extended
   format, and clib2 only has strtold(), creall(), cimagl(), cargl() and
   the classification macros (fpclassify(), isfinite(), signbit() and
   those built on them) for it.  The other long double functions, such
   as sqrtl(), sinl() or powl(), and nexttoward() and nexttowardf(),
   which take a long double, are declared but not defined there, so a
   program that calls them does not link. */

/****************************************************************************/

extern float __huge_val_float;

/****************************************************************************/

#if defined(__GNUC__) && __GNUC__ >= 4
#define	HUGE_VALF __FLT_MAX__
#else
#define	HUGE_VALF ((const float)__huge_val_float)
#endif /* __GNUC__ */

/****************************************************************************/

extern float __infinity;
extern float __nan;

/****************************************************************************/

/*
 * C99 makes these constant expressions, usable in static initializers,
 * which reading the variables above is not.  HUGE_VAL and HUGE_VALF have
 * the values of __huge_val and __huge_val_float, DBL_MAX and FLT_MAX.
 */
#if defined(__GNUC__) && __GNUC__ >= 4
#define INFINITY	__builtin_inff()
#define NAN			__builtin_nanf("")
#else
#define INFINITY	((const float)__infinity)
#define NAN			((const float)__nan)
#endif /* __GNUC__ */

/****************************************************************************/

#define FP_INFINITE		0	/* -/+ infinity */
#define FP_NAN			1	/* not a number */
#define FP_NORMAL		2	/* normalized floating point number */
#define FP_SUBNORMAL	3	/* very small floating point number; special
							   case of IEEE 754 */
#define FP_ZERO			4	/* exponent/fraction are zero */
#define FP_ILOGB0		0
#define FP_ILOGBNAN		NAN

/****************************************************************************/

#if defined __has_builtin
#if __has_builtin (__builtin_isunordered)
#define isunordered(x,y) __builtin_isunordered(x,y)
#endif /* __builtin_isunordered */
#if __has_builtin (__builtin_isgreater)
#define isgreater(x,y) __builtin_isgreater(x,y)
#endif /* __builtin_isgreater */
#if __has_builtin (__builtin_isgreaterqeual)
#define isgreaterqeual(x,y) __builtin_isgreaterqeual(x,y)
#endif /* __builtin_isgreaterqeual */
#if __has_builtin (__builtin_isless)
#define isless(x,y) __builtin_isless(x,y)
#endif /* __builtin_isless */
#if __has_builtin (__builtin_islessequal)
#define islessequal(x,y) __builtin_islessequal(x,y)
#endif /* __builtin_islessequal */
#if __has_builtin (__builtin_islessgreater)
#define islessgreater(x,y) __builtin_islessgreater(x,y)
#endif /* __builtin_islessgreater */
#if __has_builtin (__builtin_isinf)
#define isinf(x) __builtin_isinf(x)
#endif /* __builtin_isinf */
#if __has_builtin (__builtin_isnan)
#define isnan(x) __builtin_isnan(x)
#endif /* __builtin_isnan */
#if __has_builtin (__builtin_isnormal)
#define isnormal(x) __builtin_isnormal(x)
#endif /* __builtin_isnormal */
#if __has_builtin (__builtin_fpclassify)
#define fpclassify(x) (__builtin_fpclassify(FP_NAN, FP_INFINITE, \
FP_NORMAL, FP_SUBNORMAL, FP_ZERO, x))
#endif /* __builtin_fpclassify */
#if __has_builtin (__builtin_isfinite)
#define isfinite(x) __builtin_isfinite(x)
#endif /* __builtin_isinfinite */
#if __has_builtin (__builtin_signbit)
#define signbit(x) __builtin_signbit(x)
#endif /* __builtin_signbit */
#if __has_builtin (__builtin_isinf)
#define isinf(x) __builtin_isinf(x)
#endif /* __builtin_isinf */
#if __has_builtin (__builtin_isnan)
#define isnan(x) __builtin_isnan(x)
#endif /* __builtin_isnan */
#if __has_builtin (__builtin_isnormal)
#define isnormal(x) __builtin_isnormal(x)
#endif /* __builtin_isnormal */
#endif /* __has_builtin */

/****************************************************************************/

#ifndef fpclassify
extern __stdargs int __fpclassify_float(float x);
extern __stdargs int __fpclassify_double(double x);
extern __stdargs int __fpclassify_long_double(long double x);
#define fpclassify(x) _Generic(x, float : __fpclassify_float, \
double : __fpclassify_double, long double : __fpclassify_long_double)(x)
#endif /* fpclassify */
#ifndef isfinite
extern __stdargs int __isfinite_float(float x);
extern __stdargs int __isfinite_double(double x);
extern __stdargs int __isfinite_long_double(long double x);
#define isfinite(x) _Generic(x, float : __isfinite_float, \
double : __isfinite_double, long double : __isfinite_long_double)(x)
#endif /* isfinite */
#ifndef signbit
extern __stdargs int __signbit_float(float x);
extern __stdargs int __signbit_double(double x);
extern __stdargs int __signbit_long_double(long double x);
#define signbit(x) _Generic(x, float : __signbit_float, \
double : __signbit_double, long double : __signbit_long_double)(x)
#endif /* signbit */
#ifndef isinf
#define isinf(x) (fpclassify(x) == FP_INFINITE)
#endif /* isinf */
#ifndef isnan
#define isnan(x) (fpclassify(x) == FP_NAN)
#endif /* isnan */
#ifndef isnormal
#define isnormal(x) (fpclassify(x) == FP_NORMAL)
#endif /* isnormal */
#ifndef isunordered
#define isunordered(x,y) (isnan((x)) ? ((void)(y),1) : isnan((y)))
#endif /* isunordered */
#ifndef isgreater
#define isgreater(x,y) (isunordered(x,y) ? 0 : (x) > (y))
#endif /* isgreater */
#ifndef isgreaterequal
#define isgreaterequal(x,y) (isunordered(x,y) ? 0 : (x) >= (y))
#endif /* isgreaterequal */
#ifndef isless
#define isless(x,y) (isunordered(x,y) ? 0 : (x) < (y))
#endif /* isless */
#ifndef islessequal
#define islessequal(x,y) (isunordered(x,y) ? 0 : (x) <= (y))
#endif /* islessequal */
#ifndef islessgreater
/* ZZZ don't evaluate twice! */
#define islessgreater(x,y) (isunordered(x,y) ? 0 : (x) < (y) || (x) > (y))
#endif /* islessgreater */

/****************************************************************************/

extern __stdargs float acosf(float x);
extern __stdargs float asinf(float x);
extern __stdargs float atanf(float x);
extern __stdargs float atan2f(float y, float x);
extern __stdargs float ceilf(float x);
extern __stdargs float cosf(float x);
extern __stdargs float coshf(float x);
extern __stdargs float expf(float x);
extern __stdargs float fabsf(float x);
extern __stdargs float floorf(float x);
extern __stdargs float fmodf(float x, float y);
extern __stdargs float frexpf(float x, int *eptr);
extern __stdargs float ldexpf(float x,int exp);
extern __stdargs float logf(float x);
extern __stdargs float log10f(float x);
extern __stdargs float modff(float x, float *iptr);
extern __stdargs float powf(float x, float y);
extern __stdargs float sinf(float x);
extern __stdargs float sinhf(float x);
extern __stdargs float sqrtf(float x);
extern __stdargs float tanf(float x);
extern __stdargs float tanhf(float x);

/****************************************************************************/

extern __stdargs float acoshf(float x);
extern __stdargs float asinhf(float x);
extern __stdargs float atanhf(float x);
extern __stdargs float cbrtf(float x);
extern __stdargs float copysignf(float x, float y);
extern __stdargs float erfcf(float x);
extern __stdargs float erff(float x);
extern __stdargs float expm1f(float x);
extern __stdargs float fdimf(float x,float y);
extern __stdargs float fmaf(float x,float y,float z);
extern __stdargs float fmaxf(float x,float y);
extern __stdargs float fminf(float x,float y);
extern __stdargs float hypotf(float x, float y);
extern __stdargs float lgammaf(float x);
extern __stdargs float log1pf(float x);
extern __stdargs float logbf(float x);
extern __stdargs long long int llrintf(float x);
extern __stdargs long int lrintf(float x);
extern __stdargs long long int llroundf(float x);
extern __stdargs long int lroundf(float x);
extern __stdargs float nanf(const char *tagp);
extern __stdargs float nearbyintf(float x);
extern __stdargs float nextafterf(float x,float y);
extern __stdargs float nexttowardf(float x, long double y);
extern __stdargs float remainderf(float x, float p);
extern __stdargs float remquof(float x,float y,int * quo);
extern __stdargs float rintf(float x);
extern __stdargs float roundf(float x);
extern __stdargs float scalblnf(float x, long n);
extern __stdargs float scalbnf(float x, int n);
extern __stdargs float tgammaf(float x);
extern __stdargs float truncf(float x);
extern __stdargs int ilogbf(float x);

/****************************************************************************/

extern __stdargs double acosh(double x);
extern __stdargs double asinh(double x);
extern __stdargs double atanh(double x);
extern __stdargs double cbrt(double x);
extern __stdargs double copysign(double x, double y);
extern __stdargs double erf(double x);
extern __stdargs double erfc(double x);
extern __stdargs double expm1(double x);
extern __stdargs double fdim(double x,double y);
extern __stdargs double fma(double x,double y,double z);
extern __stdargs double fmax(double x,double y);
extern __stdargs double fmin(double x,double y);
extern __stdargs double hypot(double x,double y);
extern __stdargs double lgamma(double x);
extern __stdargs double log1p(double x);
extern __stdargs double logb(double x);
extern __stdargs long long int llrint(double x);
extern __stdargs long int lrint(double x);
extern __stdargs long long int llround(double x);
extern __stdargs long int lround(double x);
extern __stdargs double nan(const char *tagp);
extern __stdargs double nearbyint(double x);
extern __stdargs double nextafter(double x,double y);
extern __stdargs double nexttoward(double x, long double y);
extern __stdargs double remainder(double x, double p);
extern __stdargs double remquo(double x,double y,int * quo);
extern __stdargs double rint(double x);
extern __stdargs double round(double x);
extern __stdargs double scalbln(double x, long n);
extern __stdargs double scalbn (double x, int n);
extern __stdargs double tgamma(double x);
extern __stdargs double trunc(double x);
extern __stdargs int ilogb(double x);

/****************************************************************************/

extern __stdargs float exp2f(float x);
extern __stdargs double exp2(double x);
extern __stdargs long double exp2l(long double x);

extern __stdargs double log2(double x);
extern __stdargs float log2f(float x);
extern __stdargs long double log2l(long double x);

/****************************************************************************/

extern __stdargs long double acosl(long double x);
extern __stdargs long double asinl(long double x);
extern __stdargs long double atanl(long double x);
extern __stdargs long double atan2l(long double y, long double x);
extern __stdargs long double ceill(long double x);
extern __stdargs long double cosl(long double x);
extern __stdargs long double coshl(long double x);
extern __stdargs long double expl(long double x);
extern __stdargs long double fabsl(long double x);
extern __stdargs long double floorl(long double x);
extern __stdargs long double fmodl(long double x, long double y);
extern __stdargs long double frexpl(long double x, int *nptr);
extern __stdargs long double ldexpl(long double x, int n);
extern __stdargs long double logl(long double x);
extern __stdargs long double log10l(long double x);
extern __stdargs long double modfl(long double x, long double *iptr);
extern __stdargs long double powl(long double x, long double y);
extern __stdargs long double sinl(long double x);
extern __stdargs long double sinhl(long double x);
extern __stdargs long double sqrtl(long double x);
extern __stdargs long double tanl(long double x);
extern __stdargs long double tanhl(long double x);

/****************************************************************************/

extern __stdargs long double acoshl(long double x);
extern __stdargs long double asinhl(long double x);
extern __stdargs long double atanhl(long double x);
extern __stdargs long double cbrtl(long double x);
extern __stdargs long double copysignl(long double x, long double y);
extern __stdargs long double erfcl(long double x);
extern __stdargs long double erfl(long double x);
extern __stdargs long double expm1l(long double x);
extern __stdargs long double fdiml(long double x, long double y);
extern __stdargs long double fmal(long double x, long double y, long double z);
extern __stdargs long double fmaxl(long double x, long double y);
extern __stdargs long double fminl(long double x, long double y);
extern __stdargs long double hypotl(long double x, long double y);
extern __stdargs long double lgammal(long double x);
extern __stdargs long double log1pl(long double x);
extern __stdargs long double logbl(long double x);
extern __stdargs long long int llrintl(long double x);
extern __stdargs long int lrintl(long double x);
extern __stdargs long long int llroundl(long double x);
extern __stdargs long int lroundl(long double x);
extern __stdargs long double nanl(const char *tagp);
extern __stdargs long double nearbyintl(long double x);
extern __stdargs long double nextafterl(long double x, long double y);
extern __stdargs long double nexttowardl(long double x, long double y);
extern __stdargs long double remainderl(long double x, long double p);
extern __stdargs long double remquol(long double x, long double y, int *quo);
extern __stdargs long double rintl(long double x);
extern __stdargs long double roundl(long double x);
extern __stdargs long double scalblnl(long double x, long int n);
extern __stdargs long double scalbnl(long double x, int n);
extern __stdargs long double tgammal(long double x);
extern __stdargs long double truncl(long double x);
extern __stdargs int ilogbl(long double x);

/****************************************************************************/

#define FLT_EVAL_METHOD 0

/****************************************************************************/

typedef float	float_t;
typedef double	double_t;

/****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/****************************************************************************/

#endif /* _MATH_H */
