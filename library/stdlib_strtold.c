/*
 * Experimental: Not a part of clib2 from Olaf Barthel
 */

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

#ifndef _MATH_HEADERS_H
#include "math_headers.h"
#endif /* _MATH_HEADERS_H */

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT)

/****************************************************************************/

#if LDBL_MANT_DIG == DBL_MANT_DIG && LDBL_MAX_EXP == DBL_MAX_EXP

/****************************************************************************/

long double
strtold(const char * __restrict str, char ** __restrict ptr)
{
    return strtod(str, ptr);
}

/****************************************************************************/

#else

/****************************************************************************/

#ifndef _STDLIB_STRTOD_PARSE_H
#include "stdlib_strtod_parse.h"
#endif /* _STDLIB_STRTOD_PARSE_H */

/****************************************************************************/

long double
strtold(const char * __restrict str, char ** __restrict ptr)
{
	struct __parsed_number pn;
	const char * stop = str;
	long double result = 0.0L;

	ENTER();

	SHOWSTRING(str);
	SHOWPOINTER(ptr);

	assert( str != NULL );

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

	stop = __parse_number(str,&pn);
	result = __parsed_to_long_double(&pn);

#if defined(CHECK_FOR_NULL_POINTERS)
 out:
#endif /* CHECK_FOR_NULL_POINTERS */

	if(ptr != NULL)
		(*ptr) = (char *)stop;

	RETURN(result);
	return(result);
}

/****************************************************************************/

#endif /* LDBL_MANT_DIG == DBL_MANT_DIG && LDBL_MAX_EXP == DBL_MAX_EXP */

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
