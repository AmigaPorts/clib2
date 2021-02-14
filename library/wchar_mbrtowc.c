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
mbrtowc(wchar_t *wcp, const char *s, size_t n, mbstate_t *ps)
{
	static unsigned internal_state;
	unsigned c;
	const unsigned char *s = (const void *) s;
	const unsigned N = n;
	wchar_t dummy;

	if (!ps) ps = (void *)&internal_state;
	c = *(unsigned *) ps;

	if (!s) {
		if (c) goto ilseq;
		return 0;
	} else if (!wcp) wcp = &dummy;

	if (!n) return -2;
	if (!c) {
		if (*s < 0x80) return !!(*wcp = *s);
		if (MB_CUR_MAX==1) return (*wcp = CODEUNIT(*s)), 1;
		if (*s-SA > SB-SA) goto ilseq;
		c = bittab[*s++-SA]; n--;
	}

	if (n) {
		if (OOB(c,*s)) goto ilseq;
loop:
		c = c<<6 | *s++-0x80; n--;
		if (!(c&(1U<<31))) {
			*((unsigned *) ps) = 0;
			*wcp = c;
			return N-n;
		}
		if (n) {
			if (*s-0x80u >= 0x40) goto ilseq;
			goto loop;
		}
	}

	*((unsigned *) ps) = c;
	return -2;
ilseq:
	*((unsigned *) ps) = 0;
	errno = EILSEQ;
	return -1;
}
