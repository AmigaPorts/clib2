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
wcrtomb(char *p, wchar_t c, mbstate_t *ps)
{
	(void) ps;

	if (!p) return 1;
	if ((unsigned)c < 0x80) {
		*p = c;
		return 1;
	} else if (MB_CUR_MAX == 1) {
		if (!IS_CODEUNIT(c)) {
			errno = EILSEQ;
			return -1;
		}
		*p = c;
		return 1;
	} else if ((unsigned)c < 0x800) {
		*p++ = 0xc0 | (c>>6);
		*p = 0x80 | (c&0x3f);
		return 2;
	} else if ((unsigned)c < 0xd800 || (unsigned)c-0xe000 < 0x2000) {
		*p++ = 0xe0 | (c>>12);
		*p++ = 0x80 | ((c>>6)&0x3f);
		*p = 0x80 | (c&0x3f);
		return 3;
	} else if ((unsigned)c-0x10000 < 0x100000) {
		*p++ = 0xf0 | (c>>18);
		*p++ = 0x80 | ((c>>12)&0x3f);
		*p++ = 0x80 | ((c>>6)&0x3f);
		*p = 0x80 | (c&0x3f);
		return 4;
	}
	errno = EILSEQ;
	return -1;
}
