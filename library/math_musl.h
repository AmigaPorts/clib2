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

#endif /* _MATH_MUSL_H */
