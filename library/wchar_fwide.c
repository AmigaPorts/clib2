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

#define _NOT_ORIENTED_STREAM(f) (((f)->_flags & (__SWCH | __SCHR)) == 0)
#define _CHAR_STREAM(f) ((f)->_flags & __SCHR)
#define _WCHAR_STREAM(f) ((f)->_flags & __SWCH)
#define _ORIENT_CHAR(f) \
 do { FILE *s = (f); s->_flags &= ~__SWCH; s->_flags |= __SCHR; } while (0)
#define _ORIENT_WCHAR(f) \
 do { FILE *s = (f); s->_flags &= ~__SCHR; s->_flags |= __SWCH; } while (0)

int
fwide(FILE *f, int mode)
{
	if (_NOT_ORIENTED_STREAM(f))
	{
		if (mode > 0)
	    {
			_ORIENT_WCHAR(f);
	    }
		else
        if (mode < 0)
	    {
			_ORIENT_CHAR(f);
	    }
	}

	return _WCHAR_STREAM(f) ? 1 : (_CHAR_STREAM(f) ? -1 : 0);
}
