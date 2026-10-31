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

#ifndef _STDLIB_NULL_POINTER_CHECK_H
#include "stdlib_null_pointer_check.h"
#endif /* _STDLIB_NULL_POINTER_CHECK_H */

/****************************************************************************/

#ifndef _STRING_HEADERS_H
#include "string_headers.h"
#endif /* _STRING_HEADERS_H */

/****************************************************************************/


void *memccpy(void * __restrict dst, const void * __restrict src, int c,
    size_t count)
{
	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(dst == NULL || src == NULL)
		{
			__set_errno(EFAULT);
            return NULL;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

    unsigned char *to = (unsigned char *) dst,
        *from = (unsigned char *) src;

    for(size_t cur = 0; cur < count; cur++)
    {
        to[cur] = from[cur];

        if(from[cur] == c)
        {
            return to + cur + 1;
        }
    }

    return NULL;
}

/****************************************************************************/
