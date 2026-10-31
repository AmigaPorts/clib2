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

#ifndef _WCHAR_HEADERS_H
#include "wchar_headers.h"
#endif /* _WCHAR_HEADERS_H */

/****************************************************************************/

size_t
wcsftime(wchar_t * __restrict s, size_t n, const wchar_t * __restrict f,
    const struct tm * __restrict tm)
{
	char *format = __wide_to_narrow(f);
	char *text = NULL;
	size_t size = 4 * n;
	size_t result = 0;

	if (n > 0 && format != NULL)
		text = malloc(size);

	if (text != NULL && strftime(text, size, format, tm) > 0) {
		result = mbstowcs(s, text, n);

		/* It has to fit with its terminating NUL, or the result is 0. */
		if (result == (size_t)-1 || result >= n)
			result = 0;
	}

	free(text);
	free(format);

	return result;
}
