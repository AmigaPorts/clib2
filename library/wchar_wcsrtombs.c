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
wcsrtombs(char * __restrict p, const wchar_t ** __restrict sp, size_t n,
    mbstate_t * __restrict ps)
{
	(void) ps;

	const wchar_t *sp2;
	char buf[4];
	size_t N = n, l;

	if (!p) {
		for (n=0, sp2=*sp; *sp2; sp2++) {
			if (*sp2 >= 0x80u) {
				l = wcrtomb(buf, *sp2, 0);
				if (!(l+1)) return -1;
				n += l;
			} else n++;
		}
		return n;
	}
	while (n>=4) {
		if (**sp-1u >= 0x7fu) {
			if (!**sp) {
				*p = 0;
				*sp = 0;
				return N-n;
			}
			l = wcrtomb(p, **sp, 0);
			if (!(l+1)) return -1;
			p += l;
			n -= l;
		} else {
			*p++ = **sp;
			n--;
		}
		(*sp)++;
	}
	while (n) {
		if (**sp-1u >= 0x7fu) {
			if (!**sp) {
				*p = 0;
				*sp = 0;
				return N-n;
			}
			l = wcrtomb(buf, **sp, 0);
			if (!(l+1)) return -1;
			if (l>n) return N-n;
			wcrtomb(p, **sp, 0);
			p += l;
			n -= l;
		} else {
			*p++ = **sp;
			n--;
		}
		(*sp)++;
	}
	return N;

}
