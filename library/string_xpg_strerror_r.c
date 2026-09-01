/*
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 *
 * POSIX strerror_r under the name glibc and newlib give it: the newlib
 * string.h installed in sys-include renames strerror_r to __xpg_strerror_r
 * unless _GNU_SOURCE is defined, so objects built against those headers
 * (libstdc++'s system_error.o among them) reference this symbol.
 */

#ifndef _STRING_HEADERS_H
#include "string_headers.h"
#endif /* _STRING_HEADERS_H */

#include <errno.h>

/****************************************************************************/

int
__xpg_strerror_r(int number,char * buffer,size_t buffer_size)
{
	int result;
	int saved_errno = errno;

	errno = 0;

	if(strerror_r(number,buffer,buffer_size) == 0)
	{
		result = 0;
	}
	else
	{
		/* strerror_r() set EINVAL or ERANGE; hand the code to the
		   caller, as POSIX wants, instead of leaving it in errno. */
		result = (errno != 0) ? errno : EINVAL;

		if(result == EINVAL && buffer != NULL && buffer_size > 0)
			strlcpy(buffer,"Unknown error",buffer_size);
	}

	errno = saved_errno;

	return(result);
}
