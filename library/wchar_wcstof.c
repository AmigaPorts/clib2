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

#ifndef _WCHAR_WCSTOD_NARROW_H
#include "wchar_wcstod_narrow.h"
#endif /* _WCHAR_WCSTOD_NARROW_H */

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT)

/****************************************************************************/

float
wcstof(const wchar_t * __restrict str, wchar_t ** __restrict ptr)
{
	const wchar_t * start = str;
	char buffer[64];
	char * narrow;
	char * stop;
	float result = 0;

	ENTER();

	SHOWPOINTER(str);
	SHOWPOINTER(ptr);

	narrow = __wcstod_narrow(str,buffer,sizeof(buffer),&start);
	if(narrow == NULL)
	{
		stop = NULL;
		goto out;
	}

	result = strtof(narrow,&stop);

 out:

	/* If desired, remember where we stopped reading the number: where the
	   narrow conversion did, or at the beginning if it read nothing. */
	if(ptr != NULL)
		(*ptr) = (wchar_t *)((stop == NULL || stop == narrow) ? str : start + (stop - narrow));

	if(narrow != NULL && narrow != buffer)
		free(narrow);

	RETURN(result);
	return(result);
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
