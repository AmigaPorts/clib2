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

long
wcstol(const wchar_t * __restrict p, wchar_t ** __restrict q, int base)
{
	const wchar_t * start;
	char buffer[72];
	char * narrow;
	char * stop = NULL;
	long result = 0;

	narrow = __wcsto_narrow(p,buffer,sizeof(buffer),&start);

	if(narrow != NULL)
		result = strtol(narrow,&stop,base);

	if(q != NULL)
		(*q) = (wchar_t *)((stop == NULL || stop == narrow) ? p : start + (stop - narrow));

	if(narrow != NULL && narrow != buffer)
		free(narrow);

	return(result);
}
