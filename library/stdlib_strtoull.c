/*
 * $Id: stdlib_strtoull.c,v 1.6 2006-01-08 12:04:26 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2015 by Olaf Barthel <obarthel (at) gmx.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   - Neither the name of Olaf Barthel nor the names of contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _STDLIB_NULL_POINTER_CHECK_H
#include "stdlib_null_pointer_check.h"
#endif /* _STDLIB_NULL_POINTER_CHECK_H */

/****************************************************************************/

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

#if defined(USE_64_BIT_INTS)

/****************************************************************************/

unsigned long long
strtoull(const char * __restrict str, char ** __restrict ptr, int base)
{
	const char * stop = str;
	size_t num_digits_converted = 0;
	BOOL is_negative;
	unsigned long long result = 0;
	unsigned long long limit;
	unsigned long long max_sum;
	unsigned long long sum = 0;
	int max_digit;
	BOOL overflow = FALSE;

	ENTER();

	SHOWSTRING(str);
	SHOWPOINTER(ptr);
	SHOWVALUE(base);

	assert(str != NULL && base >= 0);

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(str == NULL)
		{
			SHOWMSG("invalid str parameter");

			__set_errno(EFAULT);
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	if(base < 0 || base == 1 || base > 36)
	{
		SHOWMSG("invalid base parameter");

		__set_errno(EINVAL);
		goto out;
	}

	/* Skip all leading blanks. */
	while(isspace((unsigned char)(*str)))
		str++;

	/* The first character may be a sign. */
	is_negative = (BOOL)((*str) == '-');

	if((*str) == '-' || (*str) == '+')
		str++;

	/* There may be a leading '0x' to indicate that what follows is a
	   hexadecimal number, if there is one; otherwise the '0' is the
	   number. */
	if(base == 0 || base == 16)
	{
		if(str[0] == '0' && (str[1] == 'x' || str[1] == 'X') && isxdigit((unsigned char)str[2]))
		{
			base = 16;

			str += 2;
		}
	}

	/* If we still don't know what base to use and the
	   next letter to follow is a zero then this is
	   probably a number in octal notation. */
	if(base == 0)
	{
		if((*str) == '0')
			base = 8;
		else
			base = 10;
	}

	limit = ULLONG_MAX;
	max_sum = limit / (unsigned int)base;
	max_digit = (int)(limit % (unsigned int)base);

	while(TRUE)
	{
		int c = (*str);

		if('0' <= c && c <= '9')
			c -= '0';
		else if ('a' <= c && c <= 'z')
			c -= 'a' - 10;
		else if ('A' <= c && c <= 'Z')
			c -= 'A' - 10;
		else
			break;

		/* Ignore invalid numbers. */
		if(c >= base)
			break;

		/* Too large numbers are still read to their end. */
		if(sum > max_sum || (sum == max_sum && c > max_digit))
			overflow = TRUE;
		else
			sum = base * sum + c;

		str++;

		num_digits_converted++;
	}

	/* Did we convert anything? */
	if(num_digits_converted == 0)
		goto out;

	stop = str;

	if(overflow)
	{
		SHOWMSG("overflow");

		__set_errno(ERANGE);

		result = ULLONG_MAX;
		goto out;
	}

	result = is_negative ? (0 - sum) : sum;

 out:

	/* If desired, remember where we stopped reading the
	   number from the buffer. */
	if(ptr != NULL)
		(*ptr) = (char *)stop;

	RETURN(result);
	return(result);
}

/****************************************************************************/

#endif /* USE_64_BIT_INTS */
