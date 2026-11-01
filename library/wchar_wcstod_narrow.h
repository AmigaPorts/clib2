/*
 * Experimental: Not a part of clib2 from Olaf Barthel
 */

#ifndef _WCHAR_WCSTOD_NARROW_H
#define _WCHAR_WCSTOD_NARROW_H

/****************************************************************************/

#include <stddef.h>
#include <wchar.h>

/****************************************************************************/

/* wcstod(), wcstof() and wcstold() copy what a number can be made of to a
   narrow string, after the leading white space, and leave the rest to
   strtod(), strtof() and strtold(). */
extern char * __wcstod_narrow(const wchar_t * str,char * buffer,size_t size,const wchar_t ** start);

/****************************************************************************/

#endif /* _WCHAR_WCSTOD_NARROW_H */
