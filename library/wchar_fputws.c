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

wint_t
fputws(const wchar_t *s, FILE *f)
{
	char buf[256];
	int r = 1;

	if (!(f->_flags & (__SWCH | __SCHR)))
	{
		f->_flags |= __SWCH;
	}

	while (*s)
	{
		size_t l = wcsrtombs(buf, &s, sizeof(buf), NULL);

		if (l == (size_t) -1)
		{
			f->_flags |= __SERR;
			r = EOF;
			break;
		}

		if (fwrite(buf, 1, l, f) != l)
		{
			r = EOF;
			break;
		}
	}

	return r;
}
