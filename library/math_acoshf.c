/*
 * Copyright 2005-2020 Rich Felker, et al.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Imported and modified 2021/01/25
 */

#ifndef _MATH_HEADERS_H
#include "math_headers.h"
#endif /* _MATH_HEADERS_H */

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT)

/****************************************************************************/

float
acoshf(float x)
{
	union { float f; uint32_t i; } u = { x };
	uint32_t a = u.i & 0x7fffffff;

	if(a < 0x3f800000 + (1 << 23))
	{
		/* |x| < 2, invalid if x < 1 or nan */
		/* up to 2ulp error in [1,1.125] */
		return log1pf(x - 1 + sqrtf((x - 1) * (x - 1) + 2 * (x - 1)));
	}

	if(a < 0x3f800000 + (12 << 23))
	{
		/* |x| < 0x1p12 */
		return logf(2 * x - 1 / (x + sqrtf(x * x - 1)));
	}

	/* x >= 0x1p12 */
	return logf(x) + 0.693147180559945309417232121458176568f;
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
