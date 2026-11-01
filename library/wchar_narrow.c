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

#include <wctype.h>

/****************************************************************************/

/* A wide string as a multibyte string from malloc().  Returns NULL if that
   doesn't work out. */
char *
__wide_to_narrow(const wchar_t * s)
{
	const wchar_t * from = s;
	mbstate_t state = 0;
	char * result;
	size_t length;

	length = wcsrtombs(NULL, &from, 0, &state);
	if (length == (size_t)-1)
		return NULL;

	result = malloc(length + 1);
	if (result == NULL) {
		__set_errno(ENOMEM);
		return NULL;
	}

	from = s;
	wcsrtombs(result, &from, length + 1, &state);

	return result;
}

/****************************************************************************/

/* A wide string in UTF-8, from malloc().  Returns NULL if that doesn't work
   out. */
char *
__wide_to_utf8(const wchar_t * s)
{
	char buffer[MB_LEN_MAX];
	char * result;
	char * r;
	size_t length = 0;
	size_t i;
	int n;

	for(i = 0 ; s[i] != L'\0' ; i++) {
		n = __utf8_from_wide(buffer, s[i]);
		if (n < 0) {
			__set_errno(EILSEQ);
			return NULL;
		}

		length += n;
	}

	result = malloc(length + 1);
	if (result == NULL) {
		__set_errno(ENOMEM);
		return NULL;
	}

	for(i = 0, r = result ; s[i] != L'\0' ; i++)
		r += __utf8_from_wide(r, s[i]);

	*r = '\0';

	return result;
}

/****************************************************************************/

/* Decodes the character at text, which may be a NUL, and returns its length. */
static size_t
next_character(const char * text, wchar_t * wc)
{
	mbstate_t state = 0;
	size_t length = __utf8_to_wide(wc, text, 4, &state);

	return (length == 0) ? 1 : length;
}

/****************************************************************************/

/* The first characters of the size bytes of text, at most n of them, in s.
   Returns how many. */
size_t
__utf8_to_wide_string(wchar_t * s, size_t n, const char * text, size_t size)
{
	size_t count = 0;
	size_t i = 0;

	while (i < size && count < n) {
		i += next_character(&text[i], &s[count]);
		count++;
	}

	return count;
}

/****************************************************************************/

static int
wide_character(long c, BOOL is_wide, char * text)
{
	wint_t wc = is_wide ? (wint_t)c : btowc((int)c);
	int length = -1;

	if (wc != (wint_t)WEOF)
		length = __utf8_from_wide(text, (wchar_t)wc);

	if (length < 0)
		__set_errno(EILSEQ);

	return length;
}

/****************************************************************************/

/* The next character of a wide string, or of a multibyte one in the encoding
   of the program.  Returns FALSE at its end, and clears *is_valid if that's
   because the rest isn't valid. */
static BOOL
next_of_string(const void * string, BOOL is_wide, size_t * offset, wchar_t * wc, BOOL * is_valid)
{
	if (is_wide) {
		*wc = ((const wchar_t *)string)[*offset];
		(*offset)++;
	} else {
		mbstate_t state = 0;
		size_t n = mbrtowc(wc, (const char *)string + *offset, MB_CUR_MAX, &state);

		if (n == (size_t)-1 || n == (size_t)-2) {
			__set_errno(EILSEQ);
			*is_valid = FALSE;
			return FALSE;
		}

		*offset += n;
	}

	return *wc != L'\0';
}

/****************************************************************************/

static char *
wide_string(const void * string, BOOL is_wide, int precision,
    char * buffer, size_t size,
    char ** internal_buffer, size_t * internal_buffer_size,
    int * length, int * num_characters)
{
	char bytes[MB_LEN_MAX];
	BOOL is_valid = TRUE;
	size_t offset = 0;
	size_t total = 0;
	int count = 0;
	wchar_t wc;
	char * result;
	int n;

	while ((precision < 0 || count < precision) &&
	       next_of_string(string, is_wide, &offset, &wc, &is_valid)) {
		n = __utf8_from_wide(bytes, wc);
		if (n < 0) {
			__set_errno(EILSEQ);
			return NULL;
		}

		total += n;
		count++;
	}

	if (!is_valid)
		return NULL;

	if (total < size) {
		result = buffer;
	} else if (total < *internal_buffer_size) {
		result = *internal_buffer;
	} else {
		result = realloc(*internal_buffer, total + 1);
		if (result == NULL) {
			__set_errno(ENOMEM);
			return NULL;
		}

		*internal_buffer = result;
		*internal_buffer_size = total + 1;
	}

	*length = (int)total;
	*num_characters = count;

	offset = 0;
	total = 0;

	while (count-- > 0) {
		next_of_string(string, is_wide, &offset, &wc, &is_valid);
		total += __utf8_from_wide(&result[total], wc);
	}

	result[total] = '\0';

	return result;
}

/****************************************************************************/

static const struct wide_mode format_mode =
{
	TRUE,
	wide_character,
	wide_string
};

/****************************************************************************/

/* What the wide format produces, in UTF-8 and from malloc(), in *text, with
   *size bytes.  Nothing goes out before this is known to work, which a
   character that doesn't exist in the encoding of the program can still
   prevent.  Returns the number of characters, or -1 with nothing to free. */
int
__vwide_format(char ** text, size_t * size, const wchar_t * format, va_list arg)
{
	char * narrow = __wide_to_utf8(format);
	struct iob string_iob;
	char local_buffer[32];
	int length;

	if (narrow == NULL)
		return -1;

	__initialize_iob(&string_iob,__vasprintf_hook_entry,
		NULL,
		local_buffer,sizeof(local_buffer),
		-1,
		-1,
		IOBF_IN_USE | IOBF_WRITE | IOBF_BUFFER_MODE_NONE | IOBF_INTERNAL,
		NULL);

	string_iob.iob_WideMode = &format_mode;

	length = vfprintf((FILE *)&string_iob, narrow, arg);

	free(narrow);

	if (length < 0) {
		if (string_iob.iob_String != NULL)
			free(string_iob.iob_String);

		return -1;
	}

	*text = (char *)string_iob.iob_String;

	/* Not with the terminating NUL. */
	*size = (size_t)string_iob.iob_StringPosition - 1;

	return length;
}

/****************************************************************************/

/* The characters of str that a number can be made of, after the leading
   white space, as a narrow string: in buffer if they fit, or else in memory
   from malloc().  *start is where they begin in str.  Returns NULL if
   there is no memory. */
char *
__wcsto_narrow(const wchar_t * str, char * buffer, size_t size, const wchar_t ** start)
{
	const wchar_t * s;
	char * result;
	size_t length, i;

	while (iswspace(*str))
		str++;

	*start = str;

	for (s = str; (*s >= '0' && *s <= '9') || (*s >= 'a' && *s <= 'z') ||
	     (*s >= 'A' && *s <= 'Z') || *s == '+' || *s == '-'; s++)
		;

	length = (size_t)(s - str);

	if (length < size) {
		result = buffer;
	} else {
		result = malloc(length + 1);
		if (result == NULL) {
			__set_errno(ENOMEM);
			return NULL;
		}
	}

	for (i = 0; i < length; i++)
		result[i] = (char)str[i];

	result[length] = '\0';

	return result;
}
