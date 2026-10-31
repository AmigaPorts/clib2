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

/* The wide format as a narrow one for vsnprintf(), which is told about a
   %s that has to count characters. */
static char *
narrow_format(const wchar_t * format)
{
	char * narrow = __wide_to_narrow(format);
	char * result;
	char * r;
	const char * n;

	if (narrow == NULL)
		return NULL;

	result = malloc(2 * strlen(narrow) + 1);
	if (result == NULL) {
		__set_errno(ENOMEM);
		free(narrow);
		return NULL;
	}

	for (n = narrow, r = result; *n != '\0'; ) {
		if (*n != '%') {
			*r++ = *n++;
			continue;
		}

		*r++ = *n++;

		while (*n != '\0' && strchr("-+ #0123456789.*$", *n) != NULL)
			*r++ = *n++;

		if (*n == 's')
			*r++ = 'W';
		else if (*n == '%')
			*r++ = *n++;
	}

	*r = '\0';

	free(narrow);

	return result;
}

/****************************************************************************/

/* What the wide format produces, as a multibyte string from malloc() in
   *text.  Returns its length, or -1 with nothing to free. */
int
__vwide_format(char ** text, const wchar_t * format, va_list arg)
{
	char * narrow = narrow_format(format);
	va_list copy;
	int length = -1;

	if (narrow == NULL)
		return -1;

	__builtin_va_copy(copy, arg);

	length = vsnprintf(NULL, 0, narrow, arg);

	if (length >= 0) {
		*text = malloc(length + 1);

		if (*text != NULL) {
			vsnprintf(*text, length + 1, narrow, copy);
		} else {
			__set_errno(ENOMEM);
			length = -1;
		}
	}

	__builtin_va_end(copy);

	free(narrow);

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
