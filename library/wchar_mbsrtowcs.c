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
mbsrtowcs(wchar_t *p, const char **sp, size_t n, mbstate_t *ps)
{
	const unsigned char *s = (const void *)*sp;
	size_t wn0 = n;
	unsigned c = 0;

	if (ps && (c = *(unsigned *)ps)) {
		if (p) {
			*(unsigned *)ps = 0;
			goto resume;
		} else {
			goto resume0;
		}
	}

	if (MB_CUR_MAX==1) {
		if (!p) return strlen((const char *)s);
		for (;;) {
			if (!n) {
				*sp = (const void *)s;
				return wn0;
			}
			if (!*s) break;
			c = *s++;
			*p++ = CODEUNIT(c);
			n--;
		}
		*p = 0;
		*sp = 0;
		return wn0-n;
	}

	if (!p) for (;;) {
#ifdef __GNUC__
		typedef uint32_t __attribute__((__may_alias__)) w32;
		if (*s-1u < 0x7f && (uintptr_t)s%4 == 0) {
			while (!(( *(w32*)s | *(w32*)s-0x01010101) & 0x80808080)) {
				s += 4;
				n -= 4;
			}
		}
#endif
		if (*s-1u < 0x7f) {
			s++;
			n--;
			continue;
		}
		if (*s-SA > SB-SA) break;
		c = bittab[*s++-SA];
resume0:
		if (OOB(c,*s)) { s--; break; }
		s++;
		if (c&(1U<<25)) {
			if (*s-0x80u >= 0x40) { s-=2; break; }
			s++;
			if (c&(1U<<19)) {
				if (*s-0x80u >= 0x40) { s-=3; break; }
				s++;
			}
		}
		n--;
		c = 0;
	} else for (;;) {
		if (!n) {
			*sp = (const void *)s;
			return wn0;
		}
#ifdef __GNUC__
		typedef uint32_t __attribute__((__may_alias__)) w32;
		if (*s-1u < 0x7f && (uintptr_t)s%4 == 0) {
			while (n>=5 && !(( *(w32*)s | *(w32*)s-0x01010101) & 0x80808080)) {
				*p++ = *s++;
				*p++ = *s++;
				*p++ = *s++;
				*p++ = *s++;
				n -= 4;
			}
		}
#endif
		if (*s-1u < 0x7f) {
			*p++ = *s++;
			n--;
			continue;
		}
		if (*s-SA > SB-SA) break;
		c = bittab[*s++-SA];
resume:
		if (OOB(c,*s)) { s--; break; }
		c = (c<<6) | *s++-0x80;
		if (c&(1U<<31)) {
			if (*s-0x80u >= 0x40) { s-=2; break; }
			c = (c<<6) | *s++-0x80;
			if (c&(1U<<31)) {
				if (*s-0x80u >= 0x40) { s-=3; break; }
				c = (c<<6) | *s++-0x80;
			}
		}
		*p++ = c;
		n--;
		c = 0;
	}

	if (!c && !*s) {
		if (p) {
			*p = 0;
			*sp = 0;
		}
		return wn0-n;
	}
	errno = EILSEQ;
	if (p) *sp = (const void *)s;
	return -1;
}
