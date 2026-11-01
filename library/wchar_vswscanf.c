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

/* String and format are in UTF-8, as anything in a wide string may be there. */
static const struct wide_mode string_mode =
{
	TRUE,
	NULL,
	NULL
};

/****************************************************************************/

int
vswscanf(const wchar_t * __restrict s, const wchar_t * __restrict format, va_list arg)
{
	char *narrow_string = __wide_to_utf8(s);
	char *narrow_format = __wide_to_utf8(format);
	struct iob string_iob;
	char local_buffer[32];
	int result = EOF;

	if (narrow_string != NULL && narrow_format != NULL) {
		__initialize_iob(&string_iob,__sscanf_hook_entry,
			NULL,
			local_buffer,sizeof(local_buffer),
			-1,
			-1,
			IOBF_IN_USE | IOBF_READ | IOBF_BUFFER_MODE_FULL | IOBF_INTERNAL,
			NULL);

		string_iob.iob_String = (STRPTR)narrow_string;
		string_iob.iob_StringLength = strlen(narrow_string);
		string_iob.iob_WideMode = &string_mode;

		result = vfscanf((FILE *)&string_iob, narrow_format, arg);
	}

	free(narrow_string);
	free(narrow_format);

	return result;
}
