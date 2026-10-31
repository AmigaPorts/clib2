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

/* The state of a partial sequence: the bits so far, how many bytes are
   missing, and how long it is. */
#define STATE_VALUE(s)		((s) & 0x1FFFFFUL)
#define STATE_MISSING(s)	(((s) >> 24) & 3)
#define STATE_LENGTH(s)		(((s) >> 26) & 7)
#define STATE(v,m,l)		((v) | ((unsigned long)(m) << 24) | ((unsigned long)(l) << 26))

size_t
mbrtowc(wchar_t * __restrict wcp, const char * __restrict sp, size_t n,
    mbstate_t * __restrict ps)
{
	static mbstate_t internal;
	unsigned long state;
	unsigned long value;
	size_t consumed = 0;
	int missing;
	int length;

	if (ps == NULL)
		ps = &internal;

	if (sp == NULL) {
		sp = "";
		n = 1;
		wcp = NULL;
	}

	state = (unsigned long)*ps;
	value = STATE_VALUE(state);
	missing = STATE_MISSING(state);
	length = STATE_LENGTH(state);

	while (consumed < n) {
		unsigned char b = sp[consumed++];

		if (missing == 0) {
			if (b < 0x80) {
				value = b;
				length = 1;
			} else if (b >= 0xc2 && b <= 0xdf) {
				value = b & 0x1f;
				missing = 1;
				length = 2;
			} else if (b >= 0xe0 && b <= 0xef) {
				value = b & 0x0f;
				missing = 2;
				length = 3;
			} else if (b >= 0xf0 && b <= 0xf4) {
				value = b & 0x07;
				missing = 3;
				length = 4;
			} else {
				goto invalid;
			}
		} else {
			if ((b & 0xc0) != 0x80)
				goto invalid;

			value = (value << 6) | (b & 0x3f);
			missing--;
		}

		if (missing == 0) {
			if ((length == 3 && value < 0x800) ||
			    (length == 4 && value < 0x10000) ||
			    value > 0x10ffff ||
			    (value >= 0xd800 && value < 0xe000))
				goto invalid;

			*ps = 0;

			if (wcp != NULL)
				*wcp = (wchar_t)value;

			return value != 0 ? consumed : 0;
		}
	}

	*ps = (mbstate_t)STATE(value, missing, length);

	return (size_t)-2;

 invalid:

	*ps = 0;
	__set_errno(EILSEQ);

	return (size_t)-1;
}
