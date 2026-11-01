/*
 * The multibyte encoding of the program.  AmigaOS text is ISO 8859-1, so a
 * byte is a character with the same code unless the program asks for UTF-8,
 * with setlocale(LC_CTYPE, "C.UTF-8") for example.
 */

#ifndef _LOCALE_HEADERS_H
#include "locale_headers.h"
#endif /* _LOCALE_HEADERS_H */

/****************************************************************************/

int __utf8_encoding;

/****************************************************************************/

int
__locale_mb_cur_max(void)
{
	return(__utf8_encoding ? 4 : 1);
}
