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
fgetwc(FILE *f)
{
	char buf[MB_CUR_MAX];
	size_t n, m;
	wchar_t c;

	if (!(f->_flags & (__SWCH | __SCHR)))
	{
		f->_flags |= __SWCH;
    }

	if (f->_r > 0)
	{
		n = mbrtowc(&c, f->_p, f->_r, NULL);

		if (n == -1)
		{
			f->_flags |= __SERR;
			return WEOF;
		}
		else
        if (n == (size_t) -2) /* incomplete mbs */
		{
			n = f->_r;
			memcpy(buf, f->_p, n);
			f->_p += n;
			f->_r = 0;
		}
		else
		{
			f->_p += n;
			f->_r -= n;
			return c;
		}
	}
	else
	{
		n = 0;
	}

	do
	{
		int t = fgetc(f);

		if (t == EOF)
		{
			return WEOF;
		}

		buf[n++] = (char)t;

		m = mbrtowc(&c, buf, n, NULL);
	}
	while (m == (size_t)-2);

	if (m == (size_t)-1)
	{
		f->_flags |= __SERR;
		return WEOF;
	}

	return c;
}
