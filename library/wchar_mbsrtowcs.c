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

#ifndef _WCHAR_HEADERS_H
#include "wchar_headers.h"
#endif /* _WCHAR_HEADERS_H */

/****************************************************************************/

size_t
mbsrtowcs(wchar_t * __restrict p, const char ** __restrict sp, size_t n,
    mbstate_t * __restrict ps)
{
	const char *s = *sp;
	size_t count = 0;

	for (;;) {
		wchar_t wc;
		size_t l;

		if (p != NULL && count == n) {
			*sp = s;
			return count;
		}

		l = mbrtowc(&wc, s, MB_CUR_MAX, ps);

		if (l == (size_t)-1 || l == (size_t)-2) {
			__set_errno(EILSEQ);

			if (p != NULL)
				*sp = s;

			return (size_t)-1;
		}

		if (l == 0)
			break;

		if (p != NULL)
			p[count] = wc;

		count++;
		s += l;
	}

	if (p != NULL) {
		p[count] = L'\0';
		*sp = NULL;
	}

	return count;
}
