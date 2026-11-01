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
vfwprintf(FILE * __restrict f, const wchar_t * __restrict format, va_list arg)
{
	char *text;
	char *out = NULL;
	size_t size;
	size_t out_size = 0;
	size_t i = 0;
	int length;
	int result = -1;

	length = __vwide_format(&text, &size, format, arg);
	if (length < 0)
		return -1;

	/* UTF-8 is not longer than the encoding of the program. */
	out = malloc(size + 1);
	if (out == NULL) {
		__set_errno(ENOMEM);
		goto done;
	}

	while (i < size) {
		mbstate_t state = 0;
		wchar_t wc;
		size_t n = __utf8_to_wide(&wc, &text[i], size - i, &state);
		size_t m;

		/* The character may be a NUL. */
		i += (n == 0) ? 1 : n;

		m = wcrtomb(&out[out_size], wc, NULL);
		if (m == (size_t)-1)
			goto done;

		out_size += m;
	}

	if (fwrite(out, 1, out_size, f) == out_size)
		result = length;

 done:

	free(out);
	free(text);

	return result;
}
