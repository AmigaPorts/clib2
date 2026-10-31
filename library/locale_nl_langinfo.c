/*
 * Experimental: Not a part of clib2 from Olaf Barthel
 */

#ifndef _LOCALE_HEADERS_H
#include "locale_headers.h"
#endif /* _LOCALE_HEADERS_H */

#include <langinfo.h>

/****************************************************************************/

/* The multibyte functions all work in UTF-8, in every locale. */
char *
nl_langinfo(nl_item item)
{
	static char decimal_point[2];
	static char thousands_separator[2];
	struct lconv * lc;

	switch(item)
	{
		case CODESET:

			return("UTF-8");

		case RADIXCHAR:

			lc = localeconv();
			decimal_point[0] = lc->decimal_point[0];

			return(decimal_point);

		case THOUSEP:

			lc = localeconv();
			thousands_separator[0] = lc->thousands_sep[0];

			return(thousands_separator);

		default:

			return("");
	}
}
