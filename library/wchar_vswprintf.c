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

int
vswprintf(wchar_t * __restrict s, size_t maxlen, const wchar_t * __restrict format,
    va_list arg)
{
	char *text;
	size_t size;
	size_t count;
	int length;
	int result = -1;

	if (maxlen == 0)
		return -1;

	length = __vwide_format(&text, &size, format, arg);
	if (length >= 0) {
		count = __utf8_to_wide_string(s, maxlen - 1, text, size);
		s[count] = L'\0';

		/* It has to fit with its terminating NUL. */
		if ((size_t)length < maxlen)
			result = length;

		free(text);
	}

	return result;
}
