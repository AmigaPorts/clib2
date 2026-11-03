/*
 * Copyright (c) 2021 Frust GmbH
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _MATH_MUSL_H
#include "math_musl.h"
#endif /* _MATH_MUSL_H */

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT)

/****************************************************************************/

#if LDBL_MANT_DIG == DBL_MANT_DIG && LDBL_MAX_EXP == DBL_MAX_EXP

/****************************************************************************/

long long int
llrintl(long double x)
{
	return llrint(x);
}

/****************************************************************************/

#else

/****************************************************************************/

/* From musl 1.2.5, src/math/llrintl.c, under its MIT license: Copyright (c) 2005-2020
   Rich Felker, et al.  Further notices are in the text. */

#if   defined(FE_INEXACT)
/*
see comments in lrint.c

Note that if LLONG_MAX == 0x7fffffffffffffff && LDBL_MANT_DIG == 64
then x == 2**63 - 0.5 is the only input that overflows and
raises inexact (with tonearest or upward rounding mode)
*/
long long llrintl(long double x)
{
	#pragma STDC FENV_ACCESS ON
	int e;

	e = fetestexcept(FE_INEXACT);
	x = rintl(x);
	if (!e && (x > LLONG_MAX || x < LLONG_MIN))
		feclearexcept(FE_INEXACT);

	/* The conversion won't tell. It is the rounded value that has to fit:
	   -2^63 does, and 2^63 - 0.5 can round to one that doesn't. */
	if(!(x >= -0x1p63L && x < 0x1p63L))
		__raise_invalid();

	/* conversion */
	return x;
}
#else
long long llrintl(long double x)
{
	long double rounded = rintl(x);

	/* The conversion won't tell. It is the rounded value that has to fit:
	   -2^63 does, and 2^63 - 0.5 can round to one that doesn't. */
	if(!(rounded >= -0x1p63L && rounded < 0x1p63L))
		__raise_invalid();

	return rounded;
}
#endif

/****************************************************************************/

#endif /* LDBL_MANT_DIG == DBL_MANT_DIG && LDBL_MAX_EXP == DBL_MAX_EXP */

/****************************************************************************/

#if defined(__MATH_SETS_ERRNO)
#undef llrintl
__MATH_ERRNO_ROUND(long double, long long, llrintl, rintl, __MATH_LONG_LONG_LIMIT)
#endif /* __MATH_SETS_ERRNO */

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
