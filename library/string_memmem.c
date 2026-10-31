/*
 * Experimental: Not a part of clib2 from Olaf Barthel
 */

#ifndef _STRING_HEADERS_H
#include "string_headers.h"
#endif /* _STRING_HEADERS_H */

/****************************************************************************/

void *
memmem(const void * haystack,size_t haystack_length,const void * needle,size_t needle_length)
{
	const char * h = haystack;
	const char * n = needle;

	if(needle_length == 0)
		return((void *)haystack);

	while(haystack_length >= needle_length)
	{
		const char * first = memchr(h,(*n),haystack_length - needle_length + 1);

		if(first == NULL)
			break;

		haystack_length -= (size_t)(first - h);
		h = first;

		if(memcmp(h,n,needle_length) == 0)
			return((void *)h);

		h++;
		haystack_length--;
	}

	return(NULL);
}
