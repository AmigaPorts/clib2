/*
 * Experimental: Not a part of clib2 from Olaf Barthel
 */

#ifndef _LOCALE_HEADERS_H
#include "locale_headers.h"
#endif /* _LOCALE_HEADERS_H */

#include <langinfo.h>

/****************************************************************************/

/* Whatever the locale, these are the strings of the "C" locale. */
static const char * const c_locale_strings[] =
{
	"^[yY]",						/* YESEXPR */
	"^[nN]",						/* NOEXPR */
	"-",							/* CRNCYSTR */
	"%a %b %e %H:%M:%S %Y",			/* D_T_FMT */
	"%m/%d/%y",						/* D_FMT */
	"%H:%M:%S",						/* T_FMT */
	"%I:%M:%S %p",					/* T_FMT_AMPM */
	"AM",							/* AM_STR */
	"PM",							/* PM_STR */
	"",								/* ERA */
	"",								/* ERA_D_T_FMT */
	"",								/* ERA_D_FMT */
	"",								/* ERA_T_FMT */
	"",								/* ALT_DIGITS */
	"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday",
	"Sun","Mon","Tue","Wed","Thu","Fri","Sat",
	"January","February","March","April","May","June","July",
	"August","September","October","November","December",
	"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
};

/****************************************************************************/

char *
nl_langinfo(nl_item item)
{
	static char decimal_point[2];
	static char thousands_separator[2];
	struct lconv * lc;

	switch(item)
	{
		case CODESET:

			return(__utf8_encoding ? "UTF-8" : "ISO-8859-1");

		case RADIXCHAR:

			lc = localeconv();
			decimal_point[0] = lc->decimal_point[0];

			return(decimal_point);

		case THOUSEP:

			lc = localeconv();
			thousands_separator[0] = lc->thousands_sep[0];

			return(thousands_separator);

		default:

			if(item >= YESEXPR && item <= ABMON_12)
				return((char *)c_locale_strings[item - YESEXPR]);

			return("");
	}
}
