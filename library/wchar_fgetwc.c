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

wint_t
fgetwc(FILE *f)
{
	struct iob * iob = (struct iob *)f;
	mbstate_t state = 0;

	for (;;) {
		wchar_t wc;
		char b;
		size_t l;
		int c;

		/* What ungetwc() left comes first. */
		if (iob->iob_NumPendingBytes > 0)
			c = iob->iob_PendingBytes[--iob->iob_NumPendingBytes];
		else
			c = fgetc(f);

		if (c == EOF) {
			if (state != 0)
				__set_errno(EILSEQ);

			return WEOF;
		}

		b = c;
		l = mbrtowc(&wc, &b, 1, &state);

		if (l == (size_t)-1)
			return WEOF;

		if (l != (size_t)-2)
			return wc;
	}
}
