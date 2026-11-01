/*
 * Experimental: Not a part of clib2 from Olaf Barthel
 */

#ifndef _WCHAR_HEADERS_H
#include "wchar_headers.h"
#endif /* _WCHAR_HEADERS_H */

#ifndef _LOCALE_HEADERS_H
#include "locale_headers.h"
#endif /* _LOCALE_HEADERS_H */

#ifndef _WCHAR_WCSTOD_NARROW_H
#include "wchar_wcstod_narrow.h"
#endif /* _WCHAR_WCSTOD_NARROW_H */

#include <wctype.h>

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT)

/****************************************************************************/

/* Can c be part of a number, "inf", "infinity" or "nan(...)"? point is
   the locale's decimal point, which strtod() accepts besides '.'. */
static int
is_number_character(wchar_t c,int point)
{
	return(('0' <= c && c <= '9') ||
	       ('a' <= c && c <= 'z') ||
	       ('A' <= c && c <= 'Z') ||
	       c == '.' || c == '+' || c == '-' || c == '_' || c == '(' || c == ')' ||
	       (point != '\0' && c == (wchar_t)point));
}

/****************************************************************************/

/* Copy the characters of str that a number can be made of, after the
   leading white space, to a narrow string: to buffer if they fit, or else
   to memory from malloc(), which the caller has to free().  Wide
   characters below 256 are the Latin 1 ones, as in the locale's decimal
   point.  *start is where the copy begins in str.  Returns NULL if str
   is NULL or there is no memory. */
char *
__wcstod_narrow(const wchar_t * str,char * buffer,size_t size,const wchar_t ** start)
{
	const wchar_t * s;
	char * result = NULL;
	size_t length, i;
	int point = '\0';

	assert( str != NULL && buffer != NULL && size > 0 && start != NULL );

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(str == NULL)
		{
			SHOWMSG("invalid str parameter");

			__set_errno(EFAULT);
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	__locale_lock();

	if(__locale_table[LC_NUMERIC] != NULL)
	{
		const unsigned char * decimal_point = (const unsigned char *)__locale_table[LC_NUMERIC]->loc_DecimalPoint;

		if(decimal_point != NULL)
			point = (*decimal_point);
	}

	__locale_unlock();

	while(iswspace(*str))
		str++;

	(*start) = str;

	for(s = str ; is_number_character(*s,point) ; s++)
		;

	length = (size_t)(s - str);

	if(length < size)
	{
		result = buffer;
	}
	else
	{
		result = malloc(length + 1);
		if(result == NULL)
			goto out;
	}

	for(i = 0 ; i < length ; i++)
		result[i] = (char)str[i];

	result[length] = '\0';

 out:

	return(result);
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
