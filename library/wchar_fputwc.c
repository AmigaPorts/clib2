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
fputwc(wchar_t c, FILE *f)
{
	char buf[MB_CUR_MAX];
	size_t n;

	if (!(f->_flags & (__SWCH | __SCHR)))
	{
		f->_flags |= __SWCH;
	}

	n = wcrtomb(buf, c, NULL);

	if (n != (size_t) -1)
	{
		if (fwrite(buf, 1, n, f) != n)
	    {
			n = (size_t) -1;
	    }
	}

	return n == (size_t) -1 ? WEOF : c;
}
