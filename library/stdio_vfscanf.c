/*
 * $Id: stdio_vfscanf.c,v 1.22 2015-04-24 13:00:12 obarthel Exp $
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

/*#define DEBUG*/

#ifndef _STDLIB_NULL_POINTER_CHECK_H
#include "stdlib_null_pointer_check.h"
#endif /* _STDLIB_NULL_POINTER_CHECK_H */

/****************************************************************************/

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

#ifndef _LOCALE_HEADERS_H
#include "locale_headers.h"
#endif /* _LOCALE_HEADERS_H */

#ifndef _WCHAR_HEADERS_H
#include "wchar_headers.h"
#endif /* _WCHAR_HEADERS_H */

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT) && !defined(_MATH_HEADERS_H)
#include "math_headers.h"
#endif /* FLOATING_POINT_SUPPORT && !_MATH_HEADERS_H */

/****************************************************************************/

/*
 * Uncomment this to activate '%lld' support and the like. Better still,
 * define this is in the Makefile!
 */
/*#define USE_64_BIT_INTS*/

/****************************************************************************/

/* What was pushed back comes first. */
STATIC int
scan_getc(FILE * stream)
{
	struct iob * iob = (struct iob *)stream;

	if(iob->iob_NumPendingBytes > 0)
		return(iob->iob_PendingBytes[--iob->iob_NumPendingBytes]);

	return(__getc(stream));
}

/* A wide conversion may have to push back a whole multibyte character, and
   ungetc() only can if it still is in the buffer. */
STATIC int
scan_ungetc(int c,FILE * stream)
{
	struct iob * iob = (struct iob *)stream;

	if(iob->iob_WideMode == NULL || (iob->iob_NumPendingBytes == 0 && iob->iob_BufferPosition > 0))
		return(ungetc(c,stream));

	if(iob->iob_NumPendingBytes == sizeof(iob->iob_PendingBytes))
	{
		__set_errno(ENOBUFS);
		return(EOF);
	}

	iob->iob_PendingBytes[iob->iob_NumPendingBytes++] = c;

	return(c);
}

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT)

/****************************************************************************/

#define NUMBER_IS_COMPLETE	0
#define NUMBER_IS_INVALID	1
#define NUMBER_AT_EOF		2

/* The input for a floating point conversion, one character at a time. */
struct number_reader
{
	FILE *	nr_stream;
	int		nr_width;		/* how many may still be read, if not negative */
	char **	nr_buffer;
	char *	nr_stack_buffer;
	size_t	nr_size;
	size_t	nr_length;
	int		nr_next;		/* next character, not stored yet */
	BOOL	nr_failed;		/* out of memory */
};

/* Looks at the next character, which stays where it is until it's taken. */
STATIC VOID
next_character(struct number_reader * nr)
{
	if(nr->nr_width == 0)
	{
		nr->nr_next = EOF;
	}
	else
	{
		nr->nr_next = scan_getc(nr->nr_stream);

		if(nr->nr_width > 0 && nr->nr_next != EOF)
			nr->nr_width--;
	}
}

/* Stores the next character and moves on.  Without memory for it, the number
   ends here, which stops everything that reads it.  Returns FALSE then. */
STATIC BOOL
take_character(struct number_reader * nr)
{
	if(nr->nr_length + 2 > nr->nr_size)
	{
		size_t new_size = 2 * nr->nr_size;
		char * new_buffer;

		if((*nr->nr_buffer) == nr->nr_stack_buffer)
		{
			new_buffer = malloc(new_size);
			if(new_buffer != NULL)
				memcpy(new_buffer,nr->nr_stack_buffer,nr->nr_length);
		}
		else
		{
			new_buffer = realloc((*nr->nr_buffer),new_size);
		}

		if(new_buffer == NULL)
		{
			__set_errno(ENOMEM);

			nr->nr_failed = TRUE;
			nr->nr_next = EOF;

			return(FALSE);
		}

		(*nr->nr_buffer) = new_buffer;
		nr->nr_size = new_size;
	}

	(*nr->nr_buffer)[nr->nr_length++] = nr->nr_next;

	next_character(nr);

	return(TRUE);
}

/* Takes characters as long as they match the lower case word, and returns
   how many it took. */
STATIC size_t
take_word(struct number_reader * nr,const char * word)
{
	size_t length = 0;

	while(word[length] != '\0' && nr->nr_next != EOF && tolower(nr->nr_next) == word[length])
	{
		if(NOT take_character(nr))
			break;

		length++;
	}

	return(length);
}

/* Is c the decimal point?  Both the locale's and '.' are accepted. */
STATIC BOOL
is_decimal_point(int c)
{
	BOOL result = (BOOL)(c == '.');

	__locale_lock();

	if(__locale_table[LC_NUMERIC] != NULL)
	{
		const unsigned char * point = (const unsigned char *)__locale_table[LC_NUMERIC]->loc_DecimalPoint;

		if(point != NULL && c == (*point))
			result = TRUE;
	}

	__locale_unlock();

	return(result);
}

/* Reads the longest sequence that is, or begins, a number as strtod()
   takes it into (*buffer), which is a NUL terminated copy of it afterwards,
   and in memory from malloc() if it isn't the one that was passed.  At most
   one character after it has to go back to the stream, which is all that
   can be relied on.  If that sequence is not a number, as in "10e", it
   is gone from the stream all the same, and this is a matching failure. */
STATIC int
read_number(FILE * stream,int width,char ** buffer,size_t size,int * num_chars_read)
{
	struct number_reader nr;
	int status = NUMBER_IS_INVALID;
	size_t length;
	BOOL is_hex = FALSE;
	int num_digits = 0;

	nr.nr_stream = stream;
	nr.nr_width = width;
	nr.nr_buffer = buffer;
	nr.nr_stack_buffer = (*buffer);
	nr.nr_size = size;
	nr.nr_length = 0;
	nr.nr_failed = FALSE;

	next_character(&nr);

	if(nr.nr_next == EOF)
	{
		status = NUMBER_AT_EOF;
		goto out;
	}

	if(nr.nr_next == '-' || nr.nr_next == '+')
		take_character(&nr);

	if(nr.nr_next == 'i' || nr.nr_next == 'I')
	{
		length = take_word(&nr,"inf");

		if(length == 3)
		{
			status = NUMBER_IS_COMPLETE;

			if(nr.nr_next == 'i' || nr.nr_next == 'I')
			{
				if(take_word(&nr,"inity") != 5)
					status = NUMBER_IS_INVALID;
			}
		}

		goto out;
	}

	if(nr.nr_next == 'n' || nr.nr_next == 'N')
	{
		length = take_word(&nr,"nan");

		if(length == 3)
		{
			status = NUMBER_IS_COMPLETE;

			if(nr.nr_next == '(')
			{
				status = NUMBER_IS_INVALID;

				take_character(&nr);

				while(nr.nr_next != EOF && (isalnum(nr.nr_next) || nr.nr_next == '_'))
					take_character(&nr);

				if(nr.nr_next == ')')
				{
					take_character(&nr);

					status = NUMBER_IS_COMPLETE;
				}
			}
		}

		goto out;
	}

	if(nr.nr_next == '0')
	{
		take_character(&nr);

		num_digits++;

		if(nr.nr_next == 'x' || nr.nr_next == 'X')
		{
			take_character(&nr);

			is_hex = TRUE;
			num_digits = 0;
		}
	}

	while(nr.nr_next != EOF && (is_hex ? isxdigit(nr.nr_next) : isdigit(nr.nr_next)))
	{
		take_character(&nr);

		num_digits++;
	}

	if(nr.nr_next != EOF && is_decimal_point(nr.nr_next))
	{
		take_character(&nr);

		while(nr.nr_next != EOF && (is_hex ? isxdigit(nr.nr_next) : isdigit(nr.nr_next)))
		{
			take_character(&nr);

			num_digits++;
		}
	}

	if(num_digits == 0)
		goto out;

	status = NUMBER_IS_COMPLETE;

	if(nr.nr_next != EOF && tolower(nr.nr_next) == (is_hex ? 'p' : 'e'))
	{
		status = NUMBER_IS_INVALID;

		take_character(&nr);

		if(nr.nr_next == '-' || nr.nr_next == '+')
			take_character(&nr);

		while(nr.nr_next != EOF && isdigit(nr.nr_next))
		{
			take_character(&nr);

			status = NUMBER_IS_COMPLETE;
		}
	}

 out:

	if(nr.nr_failed)
		status = NUMBER_IS_INVALID;

	if(nr.nr_next != EOF)
		scan_ungetc(nr.nr_next,stream);

	(*buffer)[nr.nr_length] = '\0';
	(*num_chars_read) = (int)nr.nr_length;

	return(status);
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */

/****************************************************************************/

/* Stores a byte of a string, or with is_wide, of a multibyte sequence that
   turns into a wide character.  With is_transport, the byte is part of a
   UTF-8 sequence for wide scanf(), and a string gets the encoding of the
   program.  Returns FALSE if that doesn't work out. */
STATIC BOOL
store_character(char ** target,int c,BOOL is_wide,BOOL is_transport,mbstate_t * state)
{
	if(is_wide || is_transport)
	{
		char byte = c;
		wchar_t wc;
		size_t n;

		if(is_transport)
			n = __utf8_to_wide(&wc,&byte,1,state);
		else
			n = mbrtowc(&wc,&byte,1,state);

		if(n == (size_t)-1)
			return(FALSE);

		if(n != (size_t)-2)
		{
			if(is_wide)
			{
				wchar_t * wide_target = (wchar_t *)(*target);

				(*wide_target++) = wc;
				(*target) = (char *)wide_target;
			}
			else
			{
				n = wcrtomb((*target),wc,NULL);
				if(n == (size_t)-1)
					return(FALSE);

				(*target) += n;
			}
		}
	}
	else
	{
		(*target)[0] = c;
		(*target)++;
	}

	return(TRUE);
}

/* The bytes of the character that begins with c, which are one unless the
   input is UTF-8.  Returns how many, or 0 if the input ends too early. */
STATIC int
read_character(FILE * stream,int c,BOOL is_utf8,unsigned char * bytes)
{
	int num_bytes = 1;
	int num_missing = 0;

	bytes[0] = c;

	if(is_utf8 && c >= 0xC0)
		num_missing = (c < 0xE0) ? 1 : (c < 0xF0) ? 2 : 3;

	while(num_missing-- > 0)
	{
		c = scan_getc(stream);
		if(c == EOF)
			return(0);

		bytes[num_bytes++] = c;
	}

	return(num_bytes);
}

/* Stores the character that begins with c, if target isn't NULL.  Returns FALSE
   if that doesn't work out. */
STATIC BOOL
store_whole_character(FILE * stream,char ** target,int c,BOOL is_wide,BOOL is_transport,BOOL is_utf8,mbstate_t * state)
{
	unsigned char bytes[MB_LEN_MAX];
	int num_bytes;
	int i;

	num_bytes = read_character(stream,c,is_utf8,bytes);
	if(num_bytes == 0)
		return(FALSE);

	if(target != NULL)
	{
		for(i = 0 ; i < num_bytes ; i++)
		{
			if(NOT store_character(target,bytes[i],is_wide,is_transport,state))
				return(FALSE);
		}
	}

	return(TRUE);
}

/* Is wc in the first length bytes of the UTF-8 text of a %[ conversion?  As
   elsewhere in here, 'a-z' is a range unless it comes first or last. */
STATIC BOOL
in_utf8_scanset(const unsigned char * scanset,size_t length,wchar_t wc)
{
	wchar_t previous = 0;
	wchar_t current;
	BOOL in_range = FALSE;
	size_t i = 0;

	while(i < length)
	{
		mbstate_t state = 0;
		size_t n = __utf8_to_wide(&current,(const char *)&scanset[i],length - i,&state);

		if(n == (size_t)-1 || n == (size_t)-2 || n == 0)
			break;

		if(in_range)
		{
			if(previous <= wc && wc <= current)
				return(TRUE);

			in_range = FALSE;
		}
		else if (current == L'-' && i > 0 && i + n < length)
		{
			in_range = TRUE;
		}
		else
		{
			if(current == wc)
				return(TRUE);

			previous = current;
		}

		i += n;
	}

	return(FALSE);
}

STATIC VOID
terminate_string(char * target,BOOL is_wide)
{
	if(is_wide)
		(*(wchar_t *)target) = L'\0';
	else
		(*target) = '\0';
}

/****************************************************************************/

/* In UTF-8, a byte above 127 isn't white space of any kind. */
#define IS_SPACE(c) (isspace(c) && (NOT is_wide_format || (c) < 0x80))

/****************************************************************************/

int
vfscanf(FILE * __restrict stream, const char * __restrict format, va_list arg)
{
	enum parameter_size_t
	{
		parameter_size_byte,
		parameter_size_long,
		parameter_size_short,
		parameter_size_size_t,
		parameter_size_ptrdiff_t,
		parameter_size_long_long,
		parameter_size_long_double,
		parameter_size_intmax_t,
		parameter_size_default
	};

	enum parameter_size_t parameter_size;
	int total_num_chars_read = 0;
	int num_chars_processed;
	BOOL assignment_suppressed;
	int maximum_field_width;
	int result = EOF;
	int num_conversions = 0;
	int num_assignments = 0;
	int conversion_type;
	int c;
	BOOL is_wide_format;
	BOOL is_transport;
	BOOL is_utf8;

	ENTER();

	assert( stream != NULL && format != NULL );

	if(__check_abort_enabled)
		__check_abort();

	flockfile(stream);

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(stream == NULL || format == NULL)
		{
			__set_errno(EFAULT);
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	if(__fgetc_check(stream) < 0)
		goto out;

	/* Just so we can detect errors and tell them apart from
	   an 'end of file' condition. */
	clearerr(stream);

	/* For wide scanf(), a character may be several bytes. */
	is_wide_format = (BOOL)(((struct iob *)stream)->iob_WideMode != NULL);
	is_transport = (BOOL)(is_wide_format && ((struct iob *)stream)->iob_WideMode->wm_IsTransport);
	is_utf8 = (BOOL)(is_transport || (is_wide_format && __utf8_encoding));

	while((c = (*(unsigned char *)format)) != '\0')
	{
		D(("next format character is '%lc'",c));

		if(IS_SPACE(c))
		{
			/* Skip all blank spaces in the stream. */
			format++;

			while((c = scan_getc(stream)) != EOF)
			{
				if(IS_SPACE(c))
				{
					total_num_chars_read++;
				}
				else
				{
					/* End of the white space; we push the last
					 * character read back into the stream.
					 */
					if(scan_ungetc(c,stream) == EOF)
						goto out;

					/* Resume scanning. */
					break;
				}
			}

			if(c == EOF)
			{
				SHOWMSG("end of file");

				/* Hit the end of the stream? */
				if(num_conversions == 0)
					goto out;

				/* Finished... */
				break;
			}

			/* Resume scanning. */
			continue;
		}
		else if (c != '%')
		{
			unsigned char bytes[MB_LEN_MAX];
			int num_bytes;
			int d;

			SHOWMSG("next character must match exactly");

			/* Match the next character in the stream. */
			d = scan_getc(stream);
			if(d == EOF)
			{
				SHOWMSG("end of file");

				/* Hit the end of the stream. */
				if(num_conversions == 0)
					goto out;

				break;
			}

			num_bytes = read_character(stream,d,is_utf8,bytes);
			if(num_bytes == 0)
				goto out;

			if(memcmp(format,bytes,num_bytes) == SAME)
			{
				SHOWMSG("it matches");

				format += num_bytes;
				total_num_chars_read++;
			}
			else
			{
				SHOWMSG("it does not match");

				while(num_bytes > 0)
				{
					if(scan_ungetc(bytes[--num_bytes],stream) == EOF)
						goto out;
				}

				break;
			}

			/* Resume scanning. */
			continue;
		}

		format++;

		c = (*format);

		if(c == '*')
		{
			/* If the first letter of the format specification
			 * is an asterisk, no output will be produced for
			 * this parameter.
			 */
			assignment_suppressed = TRUE;

			format++;

			c = (*format);
		}
		else
		{
			assignment_suppressed = FALSE;
		}

		maximum_field_width = -1;

		if('0' <= c && c <= '9')
		{
			int next_sum;
			int sum = 0;

			/* This could be the field width specification. */
			while(TRUE)
			{
				c = (*format);

				if('0' <= c && c <= '9')
				{
					next_sum = (10 * sum) + c - '0';
					if(next_sum < sum) /* overflow? */
						break;

					sum = next_sum;
					format++;
				}
				else
				{
					break;
				}
			}

			maximum_field_width = sum;
		}
		else if (c == '\0')
		{
			/* And that's the end of the string. */
			break;
		}

		c = (*format);

		if(c == 'h')
		{
			/* Parameter is a short integer. */
			parameter_size = parameter_size_short;

			SHOWMSG("short integer");

			format++;
		}
		else if (c == 'j')
		{
			parameter_size = parameter_size_intmax_t;

			SHOWMSG("largest integer");

			format++;
		}
		else if (c == 'l')
		{
			/* Parameter is a long integer or a double precision floating point value. */
			parameter_size = parameter_size_long;

			SHOWMSG("long integer");

			format++;
		}
		else if (c == 'L')
		{
			/* Parameter is a long double floating point value. */
			parameter_size = parameter_size_long_double;

			SHOWMSG("long double");

			format++;
		}
		else if (c == 't')
		{
			parameter_size = parameter_size_ptrdiff_t;

			SHOWMSG("pointer difference");

			format++;
		}
		else if (c == 'z')
		{
			parameter_size = parameter_size_size_t;

			SHOWMSG("size");

			format++;
		}
		else if (c == '\0')
		{
			/* And that's the end of the string. */
			break;
		}
		else
		{
			/* Parameter is a long integer or a single precision floating point value. */
			parameter_size = parameter_size_default;

			SHOWMSG("default");
		}

		/* Now for the conversion type. */
		c = (*format);
		if(c == '\0')
			break;

		#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
		{
			D(("c = '%lc'",c));

			/* Check for long long parameters. */
			if(parameter_size == parameter_size_long && c == 'l')
			{
				SHOWMSG("this is a long long parameter");

				parameter_size = parameter_size_long_long;

				format++;

				/* The conversion type follows. */
				c = (*format);
				if(c == '\0')
					break;
			}
		}
		#endif /* __GNUC__ */

		/* Check for byte parameters. */
		if(parameter_size == parameter_size_short && c == 'h')
		{
			parameter_size = parameter_size_byte;

			SHOWMSG("byte-sized integer");

			format++;

			/* The conversion type follows. */
			c = (*format);
			if(c == '\0')
				break;
		}

		switch(c)
		{
			/* It's a pointer. */
			case 'p':

				conversion_type = 'x';
				format++;
				break;

			/* It's a floating point number. */
			case 'a':
			case 'A':
			case 'e':
			case 'E':
			case 'f':
			case 'g':
			case 'G':

				conversion_type = 'f';
				format++;
				break;

			case 'c':	/* character(s) */
			case 'd':	/* signed integer */
			case 'i':	/* signed integer in decimal, octal or hexadecimal format */
			case 'n':	/* number of characters processed */
			case 'o':	/* integer (octal) */
			case 's':	/* string */
			case 'u':	/* unsigned integer */
			case 'x':	/* unsigned integer in hexadecimal format */
			case 'X':	/* unsigned integer in hexadecimal format */
			case '%':	/* the % character */
			case '[':	/* a range of characters */

				conversion_type = c;
				format++;
				break;

			default:

				/* Must be something else */
				continue;
		}

		/* Skip any initial whitespace characters. Exceptions are %c, %n and %[. */
		if(conversion_type != 'c' &&
		   conversion_type != 'n' &&
		   conversion_type != '[')
		{
			while((c = scan_getc(stream)) != EOF)
			{
				if(IS_SPACE(c))
				{
					total_num_chars_read++;
				}
				else
				{
					/* End of the white space; we push the last
					 * character read back into the stream.
					 */
					if(scan_ungetc(c,stream) == EOF)
						goto out;

					/* Proceed with the conversion operation. */
					break;
				}
			}
		}

		num_chars_processed = 0;

		D(("conversion type = '%lc'",c));

		if(conversion_type == 'c')
		{
			BOOL is_wide = (BOOL)(parameter_size == parameter_size_long);
			mbstate_t state = 0;
			char * c_ptr;
			int i;

			/* The maximum field width is actually the number
			 * of characters that should be read. Default is
			 * 1 character.
			 */
			if(maximum_field_width < 0)
				maximum_field_width = 1;

			if(NOT assignment_suppressed)
			{
				assert( arg != NULL );

				#if defined(CHECK_FOR_NULL_POINTERS)
				{
					if(arg == NULL)
					{
						__set_errno(EFAULT);
						goto out;
					}
				}
				#endif /* CHECK_FOR_NULL_POINTERS */

				c_ptr = va_arg(arg,char *);

				assert( c_ptr != NULL );

				#if defined(CHECK_FOR_NULL_POINTERS)
				{
					if(c_ptr == NULL)
					{
						__set_errno(EFAULT);
						goto out;
					}
				}
				#endif /* CHECK_FOR_NULL_POINTERS */
			}
			else
			{
				c_ptr = NULL;
			}

			for(i = 0 ; i < maximum_field_width ; i++)
			{
				c = scan_getc(stream);
				if(c == EOF)
				{
					/* Bail out if we hit the end of the stream. */
					if(num_conversions == 0)
						goto out;

					break;
				}

				total_num_chars_read++;

				if(NOT store_whole_character(stream,assignment_suppressed ? NULL : &c_ptr,c,is_wide,is_transport,is_utf8,&state))
					goto out;

				num_chars_processed++;
			}

			/* Did we read as many characters as we wanted? */
			if(num_chars_processed != maximum_field_width)
				break;

			if(NOT assignment_suppressed)
				num_assignments++;

			num_conversions++;
		}
		else if (conversion_type == 'f')
		{
			#if defined(FLOATING_POINT_SUPPORT)
			{
				void * next_parameter = NULL;
				char stack_buffer[64];
				char * number = stack_buffer;
				int status;

				if(NOT assignment_suppressed)
				{
					assert( arg != NULL );

					#if defined(CHECK_FOR_NULL_POINTERS)
					{
						if(arg == NULL)
						{
							__set_errno(EFAULT);
							goto out;
						}
					}
					#endif /* CHECK_FOR_NULL_POINTERS */

					if(parameter_size == parameter_size_default)
					{
						SHOWMSG("short format (float)");

						next_parameter = va_arg(arg,float *);
					}
					else if (parameter_size == parameter_size_long)
					{
						SHOWMSG("long format (double)");

						next_parameter = va_arg(arg,double *);
					}
					else
					{
						SHOWMSG("extended format (long double)");

						next_parameter = va_arg(arg,__long_double_t *);
					}

					assert( next_parameter != NULL );

					#if defined(CHECK_FOR_NULL_POINTERS)
					{
						if(next_parameter == NULL)
						{
							__set_errno(EFAULT);
							goto out;
						}
					}
					#endif /* CHECK_FOR_NULL_POINTERS */
				}

				status = read_number(stream,maximum_field_width,&number,sizeof(stack_buffer),&num_chars_processed);

				total_num_chars_read += num_chars_processed;

				if(status == NUMBER_IS_COMPLETE && NOT assignment_suppressed)
				{
					if(parameter_size == parameter_size_default)
						*((float *)next_parameter) = strtof(number,NULL);
					else if (parameter_size == parameter_size_long)
						*((double *)next_parameter) = strtod(number,NULL);
					else
						*((__long_double_t *)next_parameter) = strtold(number,NULL);

					num_assignments++;
				}

				if(number != stack_buffer)
					free(number);

				if(status == NUMBER_IS_COMPLETE)
				{
					num_conversions++;
				}
				else
				{
					/* Running out of input before the first conversion is
					   an input failure. */
					if(status == NUMBER_AT_EOF && num_chars_processed == 0 && num_conversions == 0)
						goto out;

					SHOWMSG("not a number");

					break;
				}
			}
			#else
			{
				if(NOT assignment_suppressed)
				{
					void * next_parameter;

					assert( arg != NULL );

					#if defined(CHECK_FOR_NULL_POINTERS)
					{
						if(arg == NULL)
						{
							__set_errno(EFAULT);
							goto out;
						}
					}
					#endif /* CHECK_FOR_NULL_POINTERS */

					if(parameter_size == parameter_size_default)
					{
						SHOWMSG("short format (float)");

						next_parameter = va_arg(arg,float *);
					}
					else if (parameter_size == parameter_size_long)
					{
						SHOWMSG("long format (double)");

						next_parameter = va_arg(arg,double *);
					}
					else
					{
						SHOWMSG("extended format (long double)");

						next_parameter = va_arg(arg,__long_double_t *);
					}

					assert( next_parameter != NULL );

					#if defined(CHECK_FOR_NULL_POINTERS)
					{
						if(next_parameter == NULL)
						{
							__set_errno(EFAULT);
							goto out;
						}
					}
					#endif /* CHECK_FOR_NULL_POINTERS */
				}
			}
			#endif /* FLOATING_POINT_SUPPORT */
		}
		else if (conversion_type == 'd' ||
		         conversion_type == 'i' ||
		         conversion_type == 'o' ||
		         conversion_type == 'u' ||
		         conversion_type == 'x')
		{
			#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
			long long next_sum;
			long long sum = 0;
			#else
			int next_sum;
			int sum = 0;
			#endif /* __GNUC__ */
			BOOL is_negative = FALSE;
			int radix;
			void * next_parameter = NULL;

			if(NOT assignment_suppressed)
			{
				assert( arg != NULL );

				#if defined(CHECK_FOR_NULL_POINTERS)
				{
					if(arg == NULL)
					{
						__set_errno(EFAULT);
						goto out;
					}
				}
				#endif /* CHECK_FOR_NULL_POINTERS */

				if(parameter_size == parameter_size_short)
				{
					next_parameter = va_arg(arg,short *);
				}
				else if (parameter_size == parameter_size_byte)
				{
					next_parameter = va_arg(arg,char *);
				}
				else
				{
					#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
					{
						if(parameter_size == parameter_size_long_long || parameter_size == parameter_size_intmax_t)
							next_parameter = va_arg(arg,long long *);
						else
							next_parameter = va_arg(arg,int *);
					}
					#else
					{
						next_parameter = va_arg(arg,int *);
					}
					#endif /* __GNUC__ */
				}

				assert( next_parameter != NULL );

				#if defined(CHECK_FOR_NULL_POINTERS)
				{
					if(next_parameter == NULL)
					{
						__set_errno(EFAULT);
						goto out;
					}
				}
				#endif /* CHECK_FOR_NULL_POINTERS */
			}

			/* Chose a base according to the conversion to be
			   expected. For the 'x' and 'i' we examine the
			   incoming data rather than commit ourselves to
			   a peculiar data format now. */
			if(conversion_type == 'd' || conversion_type == 'u')
				radix = 10;
			else if (conversion_type == 'o')
				radix = 8;
			else
				radix = 0;

			if(maximum_field_width != 0)
			{
				c = scan_getc(stream);
				if(c != EOF)
				{
					/* Skip the sign. */
					if(c == '-')
					{
						is_negative = TRUE;

						total_num_chars_read++;

						if(maximum_field_width > 0)
							maximum_field_width--;
					}
					else if (c == '+')
					{
						total_num_chars_read++;

						if(maximum_field_width > 0)
							maximum_field_width--;
					}
					else if (c == '0')
					{
						/* Keep the leading zero. */
						total_num_chars_read++;
						num_chars_processed++;

						if(maximum_field_width > 0)
							maximum_field_width--;

						if(radix == 0 && maximum_field_width != 0)
						{
							/* This could be an octal number, the
							 * '0x' prefix or just a zero.
							 */
							c = scan_getc(stream);

							/* This takes care of the '0x' prefix for hexadecimal
							   numbers ('%x') and also picks the right type of
							   data for the '%i' type. */
							if ((c == 'x' || c == 'X') && (conversion_type == 'x' || conversion_type == 'i'))
							{
								/* It's the hex prefix. */
								radix = 16;

								/* The preceding '0' was part of the
								   hex prefix. So we don't really know
								   the number yet. */
								num_chars_processed--;

								total_num_chars_read++;

								if(maximum_field_width > 0)
									maximum_field_width--;
							}
							else if (isdigit(c) && (conversion_type == 'i')) /* This could be the octal prefix for the '%i' format. */
							{
								/* The preceding '0' was part of the
								   octal prefix. So we don't really know
								   the number yet. */
								num_chars_processed--;

								/* It's an octal number. */
								radix = 8;

								/* Process the rest later. */
								if(scan_ungetc(c,stream) == EOF)
									goto out;
							}
							else if (c != EOF)
							{
								/* It's something else; put it back. */
								if(scan_ungetc(c,stream) == EOF)
									goto out;
							}
						}
					}
					else
					{
						if(scan_ungetc(c,stream) == EOF)
							goto out;
					}
				}
			}

			/* Pick a base if none has been chosen yet. */
			if(radix == 0)
			{
				if(conversion_type == 'x')
					radix = 16;
				else
					radix = 10;
			}

			if(maximum_field_width != 0)
			{
				int digit;

				while(maximum_field_width != 0 && (c = scan_getc(stream)) != EOF)
				{
					/* Is this a digit or hexadecimal value? */
					if('0' <= c && c <= '9')
						digit = c - '0';
					else if ('a' <= c && c <= 'f')
						digit = c - 'a' + 10;
					else if ('A' <= c && c <= 'F')
						digit = c - 'A' + 10;
					else
						digit = radix;

					/* Is this a valid digit? */
					if(digit >= radix)
					{
						SHOWMSG("digit is larger than radix");

						if(scan_ungetc(c,stream) == EOF)
							goto out;

						break;
					}

					next_sum = (radix * sum) + digit;

					#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
					{
						if(parameter_size == parameter_size_long_long || parameter_size == parameter_size_intmax_t)
						{
							if((unsigned long long)next_sum < (unsigned long long)sum) /* overflow? */
							{
								SHOWMSG("overflow!");

								/* Put this back. */
								if(scan_ungetc(c,stream) == EOF)
									goto out;

								break;
							}
						}
						else if ((unsigned int)next_sum < (unsigned int)sum) /* overflow? */
						{
							SHOWMSG("overflow!");

							/* Put this back. */
							if(scan_ungetc(c,stream) == EOF)
								goto out;

							break;
						}
					}
					#else
					{
						if((unsigned int)next_sum < (unsigned int)sum) /* overflow? */
						{
							SHOWMSG("overflow!");

							/* Put this back. */
							if(scan_ungetc(c,stream) == EOF)
								goto out;

							break;
						}
					}
					#endif /* __GNUC__ */

					total_num_chars_read++;
					num_chars_processed++;

					sum = next_sum;

					if(maximum_field_width > 0)
						maximum_field_width--;
				}

				if(c == EOF && num_chars_processed == 0 && num_conversions == 0)
					goto out;
			}

			if(num_chars_processed > 0)
			{
				if(NOT assignment_suppressed)
				{
					if(is_negative)
						sum = (-sum);

					if(parameter_size == parameter_size_short)
					{
						*((short *)next_parameter) = sum;
					}
					else if (parameter_size == parameter_size_byte)
					{
						*((char *)next_parameter) = sum;
					}
					else
					{
						#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
						{
							if(parameter_size == parameter_size_long_long || parameter_size == parameter_size_intmax_t)
								*((long long *)next_parameter) = sum;
							else
								*((int *)next_parameter) = sum;
						}
						#else
						{
							*((int *)next_parameter) = sum;
						}
						#endif /* __GNUC__ */
					}

					num_assignments++;
				}

				num_conversions++;
			}
			else
			{
				/* A matching failure: only the prefix "0x", say, or a sign. */
				break;
			}
		}
		else if (conversion_type == 's')
		{
			BOOL is_wide = (BOOL)(parameter_size == parameter_size_long);
			mbstate_t state = 0;
			char * s_ptr;

			if(NOT assignment_suppressed)
			{
				assert( arg != NULL );

				#if defined(CHECK_FOR_NULL_POINTERS)
				{
					if(arg == NULL)
					{
						__set_errno(EFAULT);
						goto out;
					}
				}
				#endif /* CHECK_FOR_NULL_POINTERS */

				s_ptr = va_arg(arg,char *);

				assert( s_ptr != NULL );

				#if defined(CHECK_FOR_NULL_POINTERS)
				{
					if(s_ptr == NULL)
					{
						__set_errno(EFAULT);
						goto out;
					}
				}
				#endif /* CHECK_FOR_NULL_POINTERS */
			}
			else
			{
				s_ptr = NULL;
			}

			/* Try to NUL terminate this even in case of failure. */
			if(NOT assignment_suppressed)
				terminate_string(s_ptr,is_wide);

			if(maximum_field_width != 0)
			{
				while(maximum_field_width != 0 && (c = scan_getc(stream)) != EOF)
				{
					/* Blank spaces stop the conversion process. */
					if(IS_SPACE(c))
					{
						if(scan_ungetc(c,stream) == EOF)
							goto out;

						break;
					}

					total_num_chars_read++;
					num_chars_processed++;

					if(NOT store_whole_character(stream,assignment_suppressed ? NULL : &s_ptr,c,is_wide,is_transport,is_utf8,&state))
						goto out;

					if(NOT assignment_suppressed)
						terminate_string(s_ptr,is_wide); /* Try to NUL terminate this even in case of failure. */

					if(maximum_field_width > 0)
						maximum_field_width--;
				}

				/* The conversion is considered to have failed if an EOF was
				   encountered before any non-whitespace characters could be
				   converted. */
				if(c == EOF && num_chars_processed == 0 && num_conversions == 0)
					goto out;
			}

			if(NOT assignment_suppressed)
				num_assignments++;

			num_conversions++;
		}
		else if (conversion_type == 'n')
		{
			if(NOT assignment_suppressed)
			{
				assert( arg != NULL );

				#if defined(CHECK_FOR_NULL_POINTERS)
				{
					if(arg == NULL)
					{
						__set_errno(EFAULT);
						goto out;
					}
				}
				#endif /* CHECK_FOR_NULL_POINTERS */

				if(parameter_size == parameter_size_short)
				{
					short * short_ptr;

					short_ptr = va_arg(arg,short *);

					assert( short_ptr != NULL );

					#if defined(CHECK_FOR_NULL_POINTERS)
					{
						if(short_ptr == NULL)
						{
							__set_errno(EFAULT);
							goto out;
						}
					}
					#endif /* CHECK_FOR_NULL_POINTERS */

					(*short_ptr) = total_num_chars_read;
				}
				else if (parameter_size == parameter_size_byte)
				{
					char * byte_ptr;

					byte_ptr = va_arg(arg,char *);

					assert( byte_ptr != NULL );

					#if defined(CHECK_FOR_NULL_POINTERS)
					{
						if(byte_ptr == NULL)
						{
							__set_errno(EFAULT);
							goto out;
						}
					}
					#endif /* CHECK_FOR_NULL_POINTERS */

					(*byte_ptr) = total_num_chars_read;
				}
				else
				{
					#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
					{
						if(parameter_size == parameter_size_long_long || parameter_size == parameter_size_intmax_t)
						{
							long long * int_ptr;

							int_ptr = va_arg(arg,long long *);

							assert( int_ptr != NULL );

							#if defined(CHECK_FOR_NULL_POINTERS)
							{
								if(int_ptr == NULL)
								{
									__set_errno(EFAULT);
									goto out;
								}
							}
							#endif /* CHECK_FOR_NULL_POINTERS */

							(*int_ptr) = total_num_chars_read;
						}
						else
						{
							int * int_ptr;

							int_ptr = va_arg(arg,int *);

							assert( int_ptr != NULL );

							#if defined(CHECK_FOR_NULL_POINTERS)
							{
								if(int_ptr == NULL)
								{
									__set_errno(EFAULT);
									goto out;
								}
							}
							#endif /* CHECK_FOR_NULL_POINTERS */

							(*int_ptr) = total_num_chars_read;
						}
					}
					#else
					{
						int * int_ptr;

						int_ptr = va_arg(arg,int *);

						assert( int_ptr != NULL );

						#if defined(CHECK_FOR_NULL_POINTERS)
						{
							if(int_ptr == NULL)
							{
								__set_errno(EFAULT);
								goto out;
							}
						}
						#endif /* CHECK_FOR_NULL_POINTERS */

						(*int_ptr) = total_num_chars_read;
					}
					#endif /* __GNUC__ */
				}
			}
		}
		else if (c == '%')
		{
			int d;

			d = scan_getc(stream);
			if(d == EOF)
			{
				SHOWMSG("end of file");

				/* Hit the end of the stream. */
				if(num_conversions == 0)
					goto out;

				break;
			}
			else if (c == d)
			{
				total_num_chars_read++;
			}
			else
			{
				/* This is not what we expected. We're finished. */
				if(scan_ungetc(d,stream) == EOF)
					goto out;

				break;
			}
		}
		else if (c == '[')
		{
			BOOL is_wide = (BOOL)(parameter_size == parameter_size_long);
			mbstate_t state = 0;
			char * s_ptr;
			char set[256];
			const unsigned char * scanset;
			size_t scanset_length,i;
			int pick;
			BOOL has_bracket = FALSE;

			if(NOT assignment_suppressed)
			{
				assert( arg != NULL );

				#if defined(CHECK_FOR_NULL_POINTERS)
				{
					if(arg == NULL)
					{
						__set_errno(EFAULT);
						goto out;
					}
				}
				#endif /* CHECK_FOR_NULL_POINTERS */

				s_ptr = va_arg(arg,char *);

				assert( s_ptr != NULL );

				#if defined(CHECK_FOR_NULL_POINTERS)
				{
					if(s_ptr == NULL)
					{
						__set_errno(EFAULT);
						goto out;
					}
				}
				#endif /* CHECK_FOR_NULL_POINTERS */
			}
			else
			{
				s_ptr = NULL;
			}

			c = (*format);
			if(c == '^')
			{
				/* Accept only characters which are not found in the range. */
				pick = 0;

				format++;
			}
			else
			{
				/* Accept only characters which are found in the range. */
				pick = 1;
			}

			memset(set,1 - pick,sizeof(set));

			/* If the ']' character introduces the range, then
			 * it is not considered the range termination character.
			 */
			c = (*format);
			if(c == ']')
			{
				set[c] = pick;
				has_bracket = TRUE;

				format++;
			}

			/* Figure out how many characters are in the scanset. */
			scanset = (const unsigned char *)format;

			for(scanset_length = 0 ; scanset[scanset_length] != '\0' && scanset[scanset_length] != ']' ; scanset_length++)
				format++;

			/* We already skipped everything but the right bracket. */
			if((*format) == ']')
				format++;

			/* Now have a look at the specification. We support a non-standard
			   scanf() family feature which permits you to specify ranges of
			   characters rather than spelling out each character included in
			   the range. */
			for(i = 0 ; i < scanset_length ; i++)
			{
				c = scanset[i];

				/* Could this be a range? It's not a range if it
				   is the first or the last character in the
				   specification. */
				if(c == '-' && 0 < i && i < scanset_length - 1)
				{
					int first,last,j;

					/* Pick the first and the last character in
					   the range, e.g. for "%[A-Z]" the first would
					   be the 'A' and the 'Z' would be the last. */
					first	= scanset[i-1];
					last	= scanset[i+1];

					/* Everything in the scanset now 
					   goes into the set. */
					for(j = first ; j <= last ; j++)
						set[j] = pick;

					/* Skip the character which marked the
					   end of the range and resume scanning. */
					i++;

					continue;
				}

				assert( 0 <= c && c <= 255 );

				set[c] = pick;
			}

			/* Try to NUL terminate this even in case of failure. */
			if(NOT assignment_suppressed)
				terminate_string(s_ptr,is_wide);

			if(maximum_field_width != 0)
			{
				while(maximum_field_width != 0 && (c = scan_getc(stream)) != EOF)
				{
					unsigned char bytes[MB_LEN_MAX];
					int num_bytes;
					int matches;

					assert( 0 <= c && c <= 255 );

					num_bytes = read_character(stream,c,is_utf8,bytes);
					if(num_bytes == 0)
						goto out;

					if(is_utf8)
					{
						mbstate_t character_state = 0;
						wchar_t wc = 0;

						__utf8_to_wide(&wc,(const char *)bytes,num_bytes,&character_state);

						matches = (in_utf8_scanset(scanset,scanset_length,wc) || (has_bracket && wc == L']'));
						if(pick == 0)
							matches = !matches;
					}
					else
					{
						matches = set[c];
					}

					/* If the character is not part of the
					 * range we stop further processing.
					 */
					if(NOT matches)
					{
						while(num_bytes > 0)
						{
							if(scan_ungetc(bytes[--num_bytes],stream) == EOF)
								goto out;
						}

						break;
					}

					if(NOT assignment_suppressed)
					{
						for(i = 0 ; i < (size_t)num_bytes ; i++)
						{
							if(NOT store_character(&s_ptr,bytes[i],is_wide,is_transport,&state))
								goto out;
						}

						terminate_string(s_ptr,is_wide); /* Try to NUL terminate this even in case of failure. */
					}

					total_num_chars_read++;
					num_chars_processed++;

					if(maximum_field_width > 0)
						maximum_field_width--;
				}

				if(c == EOF && num_chars_processed == 0 && num_conversions == 0)
					goto out;
			}

			if(num_chars_processed > 0)
			{
				if(NOT assignment_suppressed)
					num_assignments++;

				num_conversions++;
			}
		}
	}

	result = num_assignments;

 out:

	funlockfile(stream);

	RETURN(result);
	return(result); 
}
