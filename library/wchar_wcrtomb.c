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

/* Writes c in UTF-8, and returns how many bytes that takes, or -1. */
int
__utf8_from_wide(char * p, wchar_t c)
{
	unsigned long u = (unsigned long)c;

	if (u < 0x80) {
		*p = c;
		return 1;
	} else if (u < 0x800) {
		*p++ = 0xc0 | (u>>6);
		*p = 0x80 | (u&0x3f);
		return 2;
	} else if (u < 0xd800 || u-0xe000 < 0x2000) {
		*p++ = 0xe0 | (u>>12);
		*p++ = 0x80 | ((u>>6)&0x3f);
		*p = 0x80 | (u&0x3f);
		return 3;
	} else if (u-0x10000 < 0x100000) {
		*p++ = 0xf0 | (u>>18);
		*p++ = 0x80 | ((u>>12)&0x3f);
		*p++ = 0x80 | ((u>>6)&0x3f);
		*p = 0x80 | (u&0x3f);
		return 4;
	}

	return -1;
}

/****************************************************************************/

size_t
wcrtomb(char * __restrict p, wchar_t c, mbstate_t * __restrict ps)
{
	int n = 1;

	(void) ps;

	if (!p) return 1;

	if (__utf8_encoding)
		n = __utf8_from_wide(p, c);
	else if ((unsigned long)c < 256)
		*p = c;
	else
		n = -1;

	if (n < 0) {
		__set_errno(EILSEQ);
		return (size_t)-1;
	}

	return (size_t)n;
}
