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

/* ungetc() can only go back within the buffer, and after it was filled again,
   that isn't all of a multibyte character.  So what doesn't work that way is
   kept in the stream, for the wide functions that read it. */
wint_t
ungetwc(wint_t c, FILE *f)
{
	struct iob * iob = (struct iob *)f;
	char buffer[MB_LEN_MAX];
	wint_t result = WEOF;
	size_t l;

	if (c == (wint_t)WEOF)
		return WEOF;

	l = wcrtomb(buffer, (wchar_t)c, NULL);
	if (l == (size_t)-1)
		return WEOF;

	flockfile(f);

	if (FLAG_IS_CLEAR(iob->iob_Flags,IOBF_READ)) {
		__set_errno(EACCES);
	} else if (iob->iob_NumPendingBytes == 0 && (size_t)iob->iob_BufferPosition >= l &&
	           memcmp(&iob->iob_Buffer[iob->iob_BufferPosition - l], buffer, l) == 0) {
		/* Those are the bytes it just read. */
		while (l > 0)
			ungetc((unsigned char)buffer[--l], f);

		result = c;
	} else if (iob->iob_NumPendingBytes + l > sizeof(iob->iob_PendingBytes)) {
		__set_errno(ENOBUFS);
	} else {
		while (l > 0)
			iob->iob_PendingBytes[iob->iob_NumPendingBytes++] = buffer[--l];

		CLEAR_FLAG(iob->iob_Flags,IOBF_EOF_REACHED);

		result = c;
	}

	funlockfile(f);

	return result;
}
