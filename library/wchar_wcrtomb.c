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

size_t
wcrtomb(char *p, wchar_t c, mbstate_t *ps)
{
	int r;

	if (p == NULL)
	{
		r = 1;
	}
	else
    if (c & ~0x1fffff)
	{
		r = -1;
		errno = EILSEQ;
	}
	else
    if (c & ~0xffff)
	{
		r = 4;
		p[0] = 0xf0 | ((c >> 18) & 0x07);
		p[1] = 0x80 | ((c >> 12) & 0x3f);
		p[2] = 0x80 | ((c >> 6) & 0x3f);
		p[3] = 0x80 | (c & 0x3f);
	}
	else
    if (c & ~0x7ff)
	{
		r = 3;
		p[0] = 0xe0 | ((c >> 12) & 0x0f);
		p[1] = 0x80 | ((c >> 6) & 0x3f);
		p[2] = 0x80 | (c & 0x3f);
	}
	else
    if (c & ~0x7f)
	{
		r = 2;
		p[0] = 0xc0 | ((c >> 6) & 0x1f);
		p[1] = 0x80 | (c & 0x3f);
	}
	else
	{
		r = 1;
		p[0] = c;
	}

	return r;
}
