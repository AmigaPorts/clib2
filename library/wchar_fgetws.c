/*
 * Copyright (C) 2021 ixemul.library contributors
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef _WCHAR_HEADERS_H
#include "wchar_headers.h"
#endif /* _WCHAR_HEADERS_H */

/****************************************************************************/

wchar_t *
fgetws(wchar_t *p, int n, FILE *f)
{
	wchar_t *r = p;

	if (n <= 0)
	{
		return NULL;
    }

	while (--n != 0)
	{
		wchar_t c = getwc(f);

		if (c == WEOF)
		{
			if (feof(f) && !ferror(f) && r != p)
			{
				*p = 0;
				return r;
			}
			else
			{
				return NULL;
		    }
		}

		*p = c;

		if (c == L'\n')
		{
			break;
		}
	}

	*p = 0;

	return r;
}
