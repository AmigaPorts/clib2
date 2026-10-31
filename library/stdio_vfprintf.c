/*
 * $Id: stdio_vfprintf.c,v 1.26 2008-03-10 15:28:11 obarthel Exp $
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

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

#include <float.h>
#include <wchar.h>

/****************************************************************************/

/*
 * Uncomment this to activate '%lld' support and the like. Better still,
 * define this is in the Makefile!
 */
/*#define USE_64_BIT_INTS*/

/****************************************************************************/

/* Data conversion flags for vfprintf() below. */
#define FORMATF_LeftJustified		(1<<0)	/* Output must be left justified */
#define FORMATF_ProduceSign			(1<<1)	/* Numbers always begin with a leading
											   sign character */
#define FORMATF_ProduceSpace		(1<<2)	/* Numbers always begin with a '-'
											   character or a blank space */
#define FORMATF_AlternateConversion	(1<<3)	/* Use alternative conversion format */
#define FORMATF_CapitalLetters		(1<<4)	/* Output must use upper case characters */
#define FORMATF_IsNegative			(1<<5)	/* Number is negative */
#define FORMATF_HexPrefix			(1<<6)	/* Prepend '0x' to the output */
#define FORMATF_ZeroPrefix			(1<<7)	/* Prepend '0' to the output */

/****************************************************************************/

/* Room for length characters: buffer, or more memory if size isn't enough. */
STATIC char *
text_buffer(char * buffer,size_t size,char ** internal_buffer,size_t * internal_buffer_size,size_t length)
{
	char * new_internal_buffer;

	if(length <= size)
		return(buffer);

	if(length <= (*internal_buffer_size))
		return(*internal_buffer);

	new_internal_buffer = realloc(*internal_buffer,length);
	if(new_internal_buffer == NULL)
	{
		__set_errno(ENOMEM);
		return(NULL);
	}

	(*internal_buffer) = new_internal_buffer;
	(*internal_buffer_size) = length;

	return(new_internal_buffer);
}

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT)

/****************************************************************************/

#define FP_IS_FINITE	0
#define FP_IS_INFINITE	1
#define FP_IS_NAN		2

/* Base 10^9 digits, as used by the conversion below. */
#define BASE 1000000000UL
#define BASE_DIGITS 9

/* Enough for all doubles.  More takes a long double, and malloc(). */
#define NUM_STACK_WORDS 130

/* The number of bits in the mantissa, whatever the type. */
#define MANTISSA_BITS 64

/* The parts of v: a magnitude of mantissa * 2^exponent, with the top bit of
   the mantissa set unless it is 0.  This works on the bits, as neither the
   conversions nor the comparisons of the soft-float code are safe to use on
   infinity, NaN and denormal numbers. */
STATIC int
get_float_parts(__long_double_t v,unsigned long long * mantissa,long * exponent,BOOL * is_negative)
{
	long e;
	unsigned long long m;

	#if LDBL_MANT_DIG == 64 && defined(__mc68000__)
	{
		union
		{
			long double ld;
			struct
			{
				unsigned long		sign_exponent;
				unsigned long long	mantissa;
			} bits;
		} u;

		u.ld = v;

		(*is_negative) = (u.bits.sign_exponent >> 31) != 0;

		e = (long)((u.bits.sign_exponent >> 16) & 0x7FFF);
		m = u.bits.mantissa;

		if(e == 0x7FFF)
			return((m << 1) == 0 ? FP_IS_INFINITE : FP_IS_NAN);

		e -= 16383 + 63;
	}
	#else
	{
		union
		{
			double				d;
			unsigned long long	bits;
		} u;

		u.d = v;

		(*is_negative) = (u.bits >> 63) != 0;

		e = (long)((u.bits >> 52) & 0x7FF);
		m = u.bits & 0xFFFFFFFFFFFFFULL;

		if(e == 0x7FF)
			return(m == 0 ? FP_IS_INFINITE : FP_IS_NAN);

		if(e != 0)
			m |= 1ULL << 52;
		else
			e = 1;

		e -= 1023 + 52;
	}
	#endif /* LDBL_MANT_DIG == 64 && __mc68000__ */

	if(m != 0)
	{
		while((m >> 63) == 0)
		{
			m <<= 1;
			e--;
		}
	}
	else
	{
		e = 0;
	}

	(*mantissa) = m;
	(*exponent) = e;

	return(FP_IS_FINITE);
}

/* Writes word to the left of end, and returns where it begins. */
STATIC char *
format_word(unsigned long word,char * end)
{
	for( ; word != 0 ; word /= 10)
		(*--end) = '0' + (word % 10);

	return(end);
}

/* Formats mantissa * 2^exponent for %e, %f and %g, exactly, rounding to
   nearest with ties to even.  The number is kept in base 10^9, with the
   integer part in the words up to r and the fraction in the ones after it.
   Multiplying by 2^29, or dividing by 2^9, which a word can do without
   losing anything to the next one but 5^9 times its remainder, brings the
   exponent to 0.  This is the method of fmt_fp() in musl, which is
   Copyright (c) 2005-2020 Rich Felker et al. and under the MIT license.
   Returns the length of the text, which ends up in *text, and what has to
   follow it, or -1 if there is no memory. */
STATIC int
format_decimal(unsigned long long mantissa,long exponent,int p,int t,BOOL alternate,BOOL capitals,
	char * buffer,size_t size,char ** internal_buffer,size_t * internal_buffer_size,
	char ** text,int * trailing_zeroes,char * trail)
{
	unsigned long stack_words[NUM_STACK_WORDS];
	unsigned long * words = stack_words;
	unsigned long * a;
	unsigned long * d;
	unsigned long * r;
	unsigned long * z;
	char digits[BASE_DIGITS + 1];
	char * end = &digits[BASE_DIGITS];
	char * s;
	char * out;
	char * t_start;
	size_t length;
	long num_words;
	long e;
	long j;
	unsigned long i;
	int result = -1;

	if(mantissa != 0)
	{
		while((mantissa & 1) == 0)
		{
			mantissa >>= 1;
			exponent++;
		}
	}

	if(exponent >= 0)
		num_words = 6 + exponent / 29;
	else
		num_words = 7 + (-exponent) / BASE_DIGITS;

	if(num_words > NUM_STACK_WORDS)
	{
		words = malloc(num_words * sizeof(*words));
		if(words == NULL)
		{
			__set_errno(ENOMEM);
			goto out;
		}
	}

	if(exponent >= 0)
		r = &words[num_words - 1];
	else
		r = &words[3];

	r[0] = (unsigned long)(mantissa % BASE);
	mantissa /= BASE;
	r[-1] = (unsigned long)(mantissa % BASE);
	r[-2] = (unsigned long)(mantissa / BASE);

	a = r - 2;
	z = r + 1;

	while(a < r && (*a) == 0)
		a++;

	if((*a) == 0)
		z = a;

	while(exponent > 0)
	{
		unsigned long carry = 0;
		int shift = (exponent > 29) ? 29 : (int)exponent;

		for(d = z - 1 ; d >= a ; d--)
		{
			unsigned long long x = (((unsigned long long)(*d)) << shift) + carry;

			carry = (unsigned long)(x / BASE);
			(*d) = (unsigned long)(x - (unsigned long long)carry * BASE);
		}

		if(carry != 0)
			(*--a) = carry;

		while(z > a && z[-1] == 0)
			z--;

		exponent -= shift;
	}

	while(exponent < 0)
	{
		unsigned long carry = 0;
		unsigned long * b;
		long need = 1 + (long)(((unsigned long)p + MANTISSA_BITS / 3 + 8) / BASE_DIGITS);
		int shift = (exponent < -9) ? 9 : (int)(-exponent);

		for(d = a ; d < z ; d++)
		{
			unsigned long rest = (*d) & ((1UL << shift) - 1);

			(*d) = ((*d) >> shift) + carry;
			carry = (BASE >> shift) * rest;
		}

		if((*a) == 0)
			a++;

		if(carry != 0)
			(*z++) = carry;

		/* Digits far past the ones asked for can't matter. */
		b = (t == 'f') ? r : a;
		if(z - b > need)
			z = b + need;

		exponent += shift;
	}

	while(z > a && z[-1] == 0)
		z--;

	e = 0;

	if(a < z)
	{
		e = BASE_DIGITS * (long)(r - a);

		for(i = 10 ; (*a) >= i ; i *= 10)
			e++;
	}

	/* j digits after the decimal point are to stay; it can be negative. */
	j = (long)p - ((t != 'f') ? e : 0) - ((t == 'g' && p != 0) ? 1 : 0);

	if(j < BASE_DIGITS * (long)(z - r - 1))
	{
		unsigned long x;
		long word_number;

		if(j >= 0)
			word_number = j / BASE_DIGITS;
		else
			word_number = -1 - (-j - 1) / BASE_DIGITS;

		d = r + 1 + word_number;

		j -= word_number * BASE_DIGITS;

		for(i = 10, j++ ; j < BASE_DIGITS ; i *= 10, j++)
			;

		x = (*d) % i;

		if(x != 0 || d + 1 != z)
		{
			BOOL is_odd = (((*d) / i) & 1) != 0 || (i == BASE && d > a && (d[-1] & 1) != 0);
			BOOL round_up;

			if(x < i / 2)
				round_up = FALSE;
			else if (x == i / 2 && d + 1 == z)
				round_up = is_odd;
			else
				round_up = TRUE;

			(*d) -= x;

			if(round_up)
			{
				(*d) += i;

				while((*d) >= BASE)
				{
					(*d--) = 0;

					if(d < a)
						(*--a) = 0;

					(*d)++;
				}

				e = BASE_DIGITS * (long)(r - a);

				for(i = 10 ; (*a) >= i ; i *= 10)
					e++;
			}
		}

		if(z > d + 1)
			z = d + 1;
	}

	while(z > a && z[-1] == 0)
		z--;

	if(t == 'g')
	{
		if(p == 0)
			p = 1;

		if(p > e && e >= -4)
		{
			t = 'f';
			p -= e + 1;
		}
		else
		{
			t = 'e';
			p--;
		}

		if(NOT alternate)
		{
			long last;

			/* Trailing zeroes are not shown. */
			if(z > a)
			{
				for(i = 10, j = 0 ; (z[-1] % i) == 0 ; i *= 10, j++)
					;
			}
			else
			{
				j = BASE_DIGITS;
			}

			last = BASE_DIGITS * (long)(z - r - 1) - j;

			if(t == 'e')
				last += e;

			if(last < 0)
				last = 0;

			if(p > last)
				p = last;
		}
	}

	if(t == 'f')
	{
		long num_fraction_words = (z > r + 1) ? (long)(z - r - 1) : 0;

		if(a > r)
			a = r;

		length = BASE_DIGITS * ((size_t)(r - a) + 1 + num_fraction_words) + 2;

		out = t_start = text_buffer(buffer,size,internal_buffer,internal_buffer_size,length);
		if(out == NULL)
			goto out;

		for(d = a ; d <= r ; d++)
		{
			s = format_word(*d,end);

			if(d != a)
			{
				while(s > digits)
					(*--s) = '0';
			}
			else if (s == end)
			{
				(*--s) = '0';
			}

			memcpy(out,s,end - s);
			out += end - s;
		}

		if(p > 0 || alternate)
			(*out++) = '.';

		for( ; d < z && p > 0 ; d++, p -= BASE_DIGITS)
		{
			s = format_word(*d,end);

			while(s > digits)
				(*--s) = '0';

			memcpy(out,s,(p < BASE_DIGITS) ? (size_t)p : BASE_DIGITS);
			out += (p < BASE_DIGITS) ? p : BASE_DIGITS;
		}

		if(p > 0)
			(*trailing_zeroes) = p;
	}
	else
	{
		char exponent_digits[12];
		char * x = &exponent_digits[sizeof(exponent_digits)];
		size_t x_length;

		if(z <= a)
			z = a + 1;

		length = BASE_DIGITS * (size_t)(z - a) + 2;

		out = t_start = text_buffer(buffer,size,internal_buffer,internal_buffer_size,length);
		if(out == NULL)
			goto out;

		for(d = a ; d < z && p >= 0 ; d++)
		{
			size_t num_digits;

			s = format_word(*d,end);

			if(s == end)
				(*--s) = '0';

			if(d != a)
			{
				while(s > digits)
					(*--s) = '0';
			}
			else
			{
				(*out++) = (*s++);

				if(p > 0 || alternate)
					(*out++) = '.';
			}

			num_digits = (size_t)(end - s);

			memcpy(out,s,((long)num_digits < p) ? num_digits : (size_t)p);
			out += ((long)num_digits < p) ? (long)num_digits : p;

			p -= (int)num_digits;
		}

		if(p > 0)
			(*trailing_zeroes) = p;

		x = format_word((unsigned long)((e < 0) ? -e : e),x);

		while(&exponent_digits[sizeof(exponent_digits)] - x < 2)
			(*--x) = '0';

		(*trail++) = capitals ? 'E' : 'e';
		(*trail++) = (e < 0) ? '-' : '+';

		x_length = (size_t)(&exponent_digits[sizeof(exponent_digits)] - x);
		memcpy(trail,x,x_length);
		trail[x_length] = '\0';
	}

	(*text) = t_start;
	result = (int)(out - t_start);

 out:

	if(words != stack_words && words != NULL)
		free(words);

	return(result);
}

/* Formats mantissa * 2^exponent for %a, rounding to nearest with ties to
   even.  Returns the length of the text in buffer, which has to hold at
   least 20 characters. */
STATIC int
format_hexadecimal(unsigned long long mantissa,long exponent,int p,BOOL alternate,BOOL capitals,
	char * buffer,int * trailing_zeroes,char * trail)
{
	const char * digit_encoding = capitals ? "0123456789ABCDEF" : "0123456789abcdef";
	unsigned long long fraction = mantissa << 1;
	char exponent_digits[12];
	char * end = &exponent_digits[sizeof(exponent_digits)];
	char * x;
	char * out = buffer;
	int first_digit = (mantissa != 0) ? 1 : 0;
	int i;

	if(mantissa != 0)
		exponent += 63;

	if(0 <= p && p < 16)
	{
		unsigned long long rest = (p == 0) ? fraction : (fraction << (4 * p));
		unsigned long long unit = (p == 0) ? 0 : (1ULL << (64 - 4 * p));
		int is_odd = (p == 0) ? (first_digit & 1) : (int)((fraction >> (64 - 4 * p)) & 1);

		fraction = (p == 0) ? 0 : (fraction & ~(unit - 1));

		if(rest > (1ULL << 63) || (rest == (1ULL << 63) && is_odd))
		{
			if(p == 0)
			{
				first_digit++;
			}
			else
			{
				fraction += unit;

				if(fraction == 0)
					first_digit++;
			}
		}
	}

	(*out++) = digit_encoding[first_digit];

	if(fraction != 0 || p > 0 || alternate)
		(*out++) = '.';

	for(i = 0 ; (p < 0 ? fraction != 0 : i < p) && i < 16 ; i++)
	{
		(*out++) = digit_encoding[(int)(fraction >> 60)];

		fraction <<= 4;
	}

	if(p > 16)
		(*trailing_zeroes) = p - 16;

	x = format_word((unsigned long)((exponent < 0) ? -exponent : exponent),end);

	if(x == end)
		(*--x) = '0';

	(*trail++) = capitals ? 'P' : 'p';
	(*trail++) = (exponent < 0) ? '-' : '+';

	memcpy(trail,x,end - x);
	trail[end - x] = '\0';

	return((int)(out - buffer));
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */

/****************************************************************************/

int
vfprintf(FILE * __restrict stream, const char * __restrict format, va_list arg)
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

	struct iob * iob = (struct iob *)stream;
	int format_flags;
	char fill_character;
	int minimum_field_width;
	int precision;
	enum parameter_size_t parameter_size;
	char conversion_type;
	char buffer[80];
	int buffer_mode;
	char *output_buffer;
	int output_len;
	const char *prefix;
	char prefix_buffer[8];
	int argument_digits;
	int argument_number;
	int result = EOF;
	int len = 0;
	int i;
	int c;

	char * internal_buffer = NULL;
	size_t internal_buffer_size = 0;
	int num_leading_zeroes;
	BOOL count_characters;

 #if defined(FLOATING_POINT_SUPPORT)
	char trail_string[12];
	int trail_string_len;
	int num_trailing_zeroes;
 #endif /* FLOATING_POINT_SUPPORT */

	ENTER();

	SHOWSTRING(format);

	assert( stream != NULL && format != NULL && arg != NULL );

	if(__check_abort_enabled)
		__check_abort();

	flockfile(stream);

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(stream == NULL || format == NULL)
		{
			SHOWMSG("invalid parameters");

			__set_errno(EFAULT);
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	/* If no buffering is specified but a buffer was allocated, switch to
	   line buffering. This is intended to help 'stderr' and others. */
	buffer_mode = (iob->iob_Flags & IOBF_BUFFER_MODE);
	if(buffer_mode == IOBF_BUFFER_MODE_NONE)
		buffer_mode = IOBF_BUFFER_MODE_LINE;

	assert( FLAG_IS_SET(iob->iob_Flags,IOBF_IN_USE) );
	assert( iob->iob_BufferSize > 0 );

	if(__fputc_check(stream) < 0)
		goto out;

	while((c = (*format++)) != '\0')
	{
		/* I this isn't a % charater, copy the input to the output. */
		if(c != '%')
		{
			if(__putc(c,stream,buffer_mode) == EOF)
				goto out;

			len++;

			continue;
		}

		/* If a string of digits, terminated by a '$' character appears here,
		   it indicates which argument should be accessed. We evaluate this
		   data but for now will ignore it altogether. */

		for(i = 0 ; format[i] != '\0' ; i++)
		{
			if(format[i] == '$')
			{
				if(argument_digits > 0)
				{
					format = &format[i+1];
				}

				break;
			}
			else if ('0' <= format[i] && format[i] <= '9')
			{
				argument_number = (10 * argument_number) + (format[i] - '0');

				argument_digits++;
			}
			else
			{
				break;
			}
		}

		format_flags	= 0;
		fill_character	= ' ';

		/* Collect the flags: left justification, sign, space,
		 * alternate format, fill character.
		 */
		while(TRUE)
		{
			c = (*format);

			if(c == '-')
			{
				SHOWMSG("minus");

				SET_FLAG(format_flags,FORMATF_LeftJustified);
				format++;
			}
			else if (c == '+')
			{
				SHOWMSG("plus");

				SET_FLAG(format_flags,FORMATF_ProduceSign);
				format++;
			}
			else if (c == ' ')
			{
				SHOWMSG("space");

				SET_FLAG(format_flags,FORMATF_ProduceSpace);
				format++;
			}
			else if (c == '#')
			{
				SHOWMSG("alternate");

				SET_FLAG(format_flags,FORMATF_AlternateConversion);
				format++;
			}
			else if (c == '0')
			{
				SHOWMSG("leading zeroes");

				fill_character = '0';
				format++;
			}
			else
			{
				break;
			}
		}

		if(c == '\0')
			break;

		/* Now for the field width. */
		minimum_field_width = 0;

		while(TRUE)
		{
			c = (*format);

			if(c == '*')
			{
				SHOWMSG("use field width (stack)");

				/* The field width is stored on the stack. */

				assert(arg != NULL);

				#if defined(CHECK_FOR_NULL_POINTERS)
				{
					if(arg == NULL)
					{
						__set_errno(EFAULT);
						goto out;
					}
				}
				#endif /* CHECK_FOR_NULL_POINTERS */

				minimum_field_width = va_arg(arg,int);
				if(minimum_field_width < 0)
				{
					SET_FLAG(format_flags,FORMATF_LeftJustified);
					minimum_field_width = (-minimum_field_width);
				}

				format++;
			}
			else if ('0' <= c && c <= '9')
			{
				int next_sum;
				int sum = 0;

				SHOWMSG("use field width (string)");

				/* Process the field width. */
				while(TRUE)
				{
					c = (*format);

					if('0' <= c && c <= '9')
					{
						D(("digit = %lc",c));

						next_sum = (10 * sum) + c - '0';
						if(next_sum < sum) /* overflow? */
						{
							SHOWMSG("overflow");
							break;
						}

						SHOWVALUE(sum);

						sum = next_sum;
						format++;
					}
					else
					{
						break;
					}
				}

				minimum_field_width = sum;

				SHOWVALUE(minimum_field_width);
			}
			else
			{
				break;
			}
		}

		/* End of the format string? */
		if(c == '\0')
			break;

		precision = -1;

		/* Was a precision specified? */
		if((*format) == '.')
		{
			SHOWMSG("precision required");

			format++;

			c = (*format);
			if(c == '*')
			{
				SHOWMSG("use stack");

				/* The precision is stored on the stack. */
				assert(arg != NULL);

				#if defined(CHECK_FOR_NULL_POINTERS)
				{
					if(arg == NULL)
					{
						__set_errno(EFAULT);
						goto out;
					}
				}
				#endif /* CHECK_FOR_NULL_POINTERS */

				/* A negative precision is taken as if it was omitted. */
				precision = va_arg(arg,int);
				if(precision < 0)
					precision = -1;

				format++;
			}
			else if ('0' <= c && c <= '9')
			{
				int next_sum;
				int sum = 0;

				SHOWMSG("use string");

				/* Process the precision. */
				while(TRUE)
				{
					c = (*format);

					if('0' <= c && c <= '9')
					{
						D(("digit = %lc",c));

						next_sum = (10 * sum) + c - '0';
						if(next_sum < sum) /* overflow? */
						{
							SHOWMSG("overflow");
							break;
						}

						SHOWVALUE(sum);

						sum = next_sum;
						format++;
					}
					else
					{
						break;
					}
				}

				precision = sum;
			}
			else
			{
				/* Anything else results in the precision
				 * value to be set to 0.
				 */
				precision = 0;
			}

			SHOWVALUE(precision);
		}

		/* Now for the size modifier, if any. */
		c = (*format);

		/* End of the format string? */
		if(c == '\0')
			break;

		if(c == 'l')
		{
			SHOWMSG("format size = long");

			parameter_size = parameter_size_long;
			format++;
		}
		else if (c == 'L')
		{
			SHOWMSG("format size = long double");

			parameter_size = parameter_size_long_double;
			format++;
		}
		else if (c == 'h')
		{
			SHOWMSG("format size = short");

			parameter_size = parameter_size_short;
			format++;
		}
		else if (c == 'j')
		{
			SHOWMSG("format size = intmax_t");

			parameter_size = parameter_size_intmax_t;
			format++;
		}
		else if (c == 't')
		{
			SHOWMSG("format size = ptrdiff_t");

			parameter_size = parameter_size_ptrdiff_t;
			format++;
		}
		else if (c == 'z')
		{
			SHOWMSG("format size = size_t");

			parameter_size = parameter_size_size_t;
			format++;
		}
		else
		{
			SHOWMSG("format size = default");

			/* The default is to assume 32 bit parameters. */
			parameter_size = parameter_size_default;
		}

		/* Finally, the conversion_type type. */
		c = (*format);

		/* End of the format string? */
		if(c == '\0')
			break;

		/* Check for byte parameters. */
		if(parameter_size == parameter_size_short && c == 'h')
		{
			SHOWMSG("format size = byte");

			parameter_size = parameter_size_byte;

			format++;

			/* The conversion_type type follows. */
			c = (*format);

			/* End of the format string? */
			if(c == '\0')
				break;
		}

		#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
		{
			/* Check for long long parameters. */
			if(parameter_size == parameter_size_long && c == 'l')
			{
				parameter_size = parameter_size_long_long;

				format++;

				/* The conversion_type type follows. */
				c = (*format);

				/* End of the format string? */
				if(c == '\0')
					break;
			}
		}
		#endif /* __GNUC__ */

		/* From the wide functions: the precision of %s counts characters
		   rather than bytes. */
		count_characters = FALSE;

		if(c == 'W')
		{
			count_characters = TRUE;

			format++;

			c = (*format);

			if(c == '\0')
				break;
		}

		#if defined(FLOATING_POINT_SUPPORT)
		{
			trail_string[0] = '\0';
			num_trailing_zeroes = 0;
		}
		#endif /* FLOATING_POINT_SUPPORT */

		num_leading_zeroes = 0;

		D(("conversion_type (preliminary) = %lc",c));

		switch(c)
		{
			/* signed integer */
			case 'i':

				conversion_type = 'd';

				format++;
				break;

			/* unsigned integer (hexadecimal notation) */
			case 'X':

				SET_FLAG(format_flags,FORMATF_CapitalLetters);

				conversion_type = 'x';

				format++;
				break;

			/* floating point number */
			case 'F':

				SET_FLAG(format_flags,FORMATF_CapitalLetters);

				conversion_type = 'f';

				format++;
				break;

			/* floating point number (exponential notation) */
			case 'E':

				SET_FLAG(format_flags,FORMATF_CapitalLetters);

				conversion_type = 'e';

				format++;
				break;

			/* floating point number (hexadecimal digits; exponential notation) */
			case 'A':

				SET_FLAG(format_flags,FORMATF_CapitalLetters);

				conversion_type = 'a';

				format++;
				break;

			/* floating point number (plain or exponential notation) */
			case 'G':

				SET_FLAG(format_flags,FORMATF_CapitalLetters);

				conversion_type = 'g';

				format++;
				break;

			/* pointer (hexadecimal notation, eight digits, '0x' prefix) */
			case 'p':

				conversion_type = 'x';

				SET_FLAG(format_flags,FORMATF_HexPrefix);

				fill_character = '0';
				minimum_field_width = 8;

				format++;
				break;

			case 'a':	/* floating point number (hexadecimal digits; exponential notation) */
			case 'c':	/* character */
			case 'd':	/* signed integer */
			case 'f':	/* floating point number */
			case 'e':	/* floating point number (exponential notation) */
			case 'g':	/* floating point number (plain or exponential notation) */
			case 's':	/* string */
			case '%':	/* % character */
			case 'o':	/* unsigned integer (octal notation) */
			case 'x':	/* unsigned integer (hexadecimal notation) */
			case 'u':	/* unsigned integer */
			case 'n':	/* number of characters written */
			default:	/* anything else (% works as escape character) */

				conversion_type = c;
				format++;
				break;
		}

		D(("conversion_type (final) = %lc",conversion_type));

		output_buffer = &buffer[sizeof(buffer)-1];
		(*output_buffer) = '\0';

		output_len = 0;

		if(conversion_type == 'c')
		{
			int ch;

			SHOWMSG("character");

			assert(arg != NULL);

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
				/* Parameter is a short integer which
				 * must be cast to a long integer before
				 * it can be used.
				 */
				short short_integer;

				short_integer = (short)va_arg(arg, int);

				ch = short_integer;
			}
			else if (parameter_size == parameter_size_byte)
			{
				/* Parameter is a byte-sized integer which
				 * must be cast to a long integer before
				 * it can be used.
				 */
				char byte_integer;

				byte_integer = (char)va_arg(arg, int);

				ch = byte_integer;
			}
			else if (parameter_size == parameter_size_long)
			{
				/* A wide character, as a multibyte sequence. */
				mbstate_t state = 0;
				size_t n;

				output_buffer = buffer;

				n = wcrtomb(output_buffer,(wchar_t)va_arg(arg, wint_t),&state);
				if(n == (size_t)-1)
					goto out;

				output_len = (int)n;
				ch = (*output_buffer);
			}
			else
			{
				#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
				{
					if(parameter_size == parameter_size_long_long || parameter_size == parameter_size_intmax_t)
						ch = va_arg(arg, long long);
					else
						ch = va_arg(arg, int);
				}
				#else
				{
					ch = va_arg(arg, int);
				}
				#endif /* __GNUC__ */
			}

			D(("output = %lc",ch));

			if(output_len == 0)
			{
				output_buffer--;
				output_len++;

				(*output_buffer) = ch;
			}

			CLEAR_FLAG(format_flags,FORMATF_ProduceSign);
			CLEAR_FLAG(format_flags,FORMATF_ProduceSpace);
		}
		else if (conversion_type == 'a' ||
		         conversion_type == 'e' ||
		         conversion_type == 'f' ||
		         conversion_type == 'g')
		{
			SHOWMSG("floating point format");

			assert(arg != NULL);

			#if defined(CHECK_FOR_NULL_POINTERS)
			{
				if(arg == NULL)
				{
					__set_errno(EFAULT);
					goto out;
				}
			}
			#endif /* CHECK_FOR_NULL_POINTERS */

			#if defined(FLOATING_POINT_SUPPORT)
			{
				unsigned long long mantissa = 0;
				__long_double_t v;
				long exponent = 0;
				BOOL is_negative = FALSE;
				BOOL capitals = FLAG_IS_SET(format_flags,FORMATF_CapitalLetters);
				BOOL alternate = FLAG_IS_SET(format_flags,FORMATF_AlternateConversion);
				int kind;

				D(("sizeof(long double) == %ld",sizeof(v)));

				if(parameter_size == parameter_size_long_double)
					v = va_arg(arg, __long_double_t);
				else
					v = va_arg(arg, double);

				kind = get_float_parts(v,&mantissa,&exponent,&is_negative);

				if(is_negative)
					SET_FLAG(format_flags,FORMATF_IsNegative);

				if(kind != FP_IS_FINITE)
				{
					SHOWMSG("infinity or not a number");

					output_buffer = buffer;

					if(kind == FP_IS_INFINITE)
						strcpy(output_buffer,capitals ? "INF" : "inf");
					else
						strcpy(output_buffer,capitals ? "NAN" : "nan");

					output_len = 3;
					fill_character = ' ';
				}
				else if (conversion_type == 'a')
				{
					SET_FLAG(format_flags,FORMATF_HexPrefix);

					output_buffer = buffer;
					output_len = format_hexadecimal(mantissa,exponent,precision,alternate,capitals,
						output_buffer,&num_trailing_zeroes,trail_string);
				}
				else
				{
					/* Default precision is 6 digits. */
					if(precision < 0)
					{
						SHOWMSG("no precision specified, using six digits");
						precision = 6;
					}

					output_len = format_decimal(mantissa,exponent,precision,conversion_type,alternate,capitals,
						buffer,sizeof(buffer),&internal_buffer,&internal_buffer_size,
						&output_buffer,&num_trailing_zeroes,trail_string);

					if(output_len < 0)
						goto out;
				}

				D(("length = %ld",output_len));
			}
			#else
			{
				/* Remove the parameter from the argument vector and
				   don't produce any output. */
				if(parameter_size == parameter_size_long_double)
					(void)va_arg(arg, __long_double_t);
				else
					(void)va_arg(arg, double);

				minimum_field_width = 0;
				format_flags = 0;
			}
			#endif /* FLOATING_POINT_SUPPORT */
		}
		else if (conversion_type == 'd' ||
		         conversion_type == 'o' ||
		         conversion_type == 'u' ||
		         conversion_type == 'x')
		{
			#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
			unsigned long long v;
			#else
			unsigned int v;
			#endif /* __GNUC__ */

			assert(arg != NULL);

			#if defined(CHECK_FOR_NULL_POINTERS)
			{
				if(arg == NULL)
				{
					__set_errno(EFAULT);
					goto out;
				}
			}
			#endif /* CHECK_FOR_NULL_POINTERS */

			if(conversion_type == 'd')
			{
				#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
				long long sv;
				#else
				int sv;
				#endif /* __GNUC__ */

				SHOWMSG("signed integer");

				if(parameter_size == parameter_size_short)
				{
					/* Parameter is a short integer which
					 * must be cast to a long integer before
					 * it can be used.
					 */
					short short_integer;

					short_integer = (short)va_arg(arg, int);

					sv = short_integer;
				}
				else if (parameter_size == parameter_size_byte)
				{
					/* Parameter is a byte-sized integer which
					 * must be cast to a long integer before
					 * it can be used.
					 */
					short byte_integer;

					byte_integer = (char)va_arg(arg, int);

					sv = byte_integer;
				}
				else
				{
					#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
					{
						if(parameter_size == parameter_size_long_long || parameter_size == parameter_size_intmax_t)
							sv = va_arg(arg, long long);
						else
							sv = va_arg(arg, int);
					}
					#else
					{
						sv = va_arg(arg, int);
					}
					#endif /* __GNUC__ */
				}

				if(sv < 0)
				{
					SHOWMSG("negative number");

					SET_FLAG(format_flags,FORMATF_IsNegative);

					v = (-sv);
				}
				else
				{
					v = sv;
				}
			}
			else
			{
				if(conversion_type == 'o')
					SHOWMSG("unsigned integer (octal notation)");
				else if (conversion_type == 'x')
					SHOWMSG("unsigned integer (hexadecimal notation)");
				else
					SHOWMSG("unsigned integer");

				if(parameter_size == parameter_size_short)
				{
					/* Parameter is a short integer which
					 * must be cast to a long integer before
					 * it can be used.
					 */
					unsigned short short_integer;

					short_integer = (unsigned short)va_arg(arg, unsigned int);

					v = short_integer;
				}
				else if (parameter_size == parameter_size_byte)
				{
					/* Parameter is a byte-sized integer which
					 * must be cast to a long integer before
					 * it can be used.
					 */
					unsigned char byte_integer;

					byte_integer = (unsigned char)va_arg(arg, unsigned int);

					v = byte_integer;
				}
				else
				{
					#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
					{
						if(parameter_size == parameter_size_long_long || parameter_size == parameter_size_intmax_t)
							v = va_arg(arg, unsigned long long);
						else
							v = va_arg(arg, unsigned int);
					}
					#else
					{
						v = va_arg(arg, unsigned int);
					}
					#endif /* __GNUC__ */
				}

				CLEAR_FLAG(format_flags,FORMATF_ProduceSign);
				CLEAR_FLAG(format_flags,FORMATF_ProduceSpace);
			}

			/* With a precision, the '0' flag doesn't count. */
			if(precision >= 0)
				fill_character = ' ';

			/* Don't print anything if the precision is 0 and the number
			   itself is 0. */
			if(v != 0 || precision != 0)
			{
				const char * digit_encoding;
				int radix;

				/* Only add the hex (%x) prefix if the value to be converted
				   is non-zero. */
				if(FLAG_IS_SET(format_flags,FORMATF_AlternateConversion) && v != 0)
				{
					if(conversion_type == 'x')
						SET_FLAG(format_flags,FORMATF_HexPrefix);
				}

				if(conversion_type == 'o')
					radix = 8;
				else if (conversion_type == 'x')
					radix = 16;
				else
					radix = 10;

				if(FLAG_IS_SET(format_flags,FORMATF_CapitalLetters))
					digit_encoding = "0123456789ABCDEF";
				else
					digit_encoding = "0123456789abcdef";

				#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
				{
					while((v >> 32) != 0)
					{
						output_buffer--;
						output_len++;

						(*output_buffer) = digit_encoding[v % radix];
						v /= radix;
					}
				}
				#endif /* __GNUC__ */

				{
					/* Most numbers are done without 64 bit divisions. */
					unsigned long w = (unsigned long)v;

					do
					{
						output_buffer--;
						output_len++;

						(*output_buffer) = digit_encoding[w % radix];
						w /= radix;
					}
					while(w > 0);
				}

				if(output_len < precision)
					num_leading_zeroes = precision - output_len;
			}

			/* The '#' flag for %o: the first digit has to be a zero, which
			   a precision may already have supplied. */
			if(conversion_type == 'o' && FLAG_IS_SET(format_flags,FORMATF_AlternateConversion) &&
			   num_leading_zeroes == 0 && (output_len == 0 || (*output_buffer) != '0'))
			{
				output_buffer--;
				output_len++;

				(*output_buffer) = '0';
			}
		}
		else if (conversion_type == 's')
		{
			SHOWMSG("string");

			assert(arg != NULL);

			#if defined(CHECK_FOR_NULL_POINTERS)
			{
				if(arg == NULL)
				{
					__set_errno(EFAULT);
					goto out;
				}
			}
			#endif /* CHECK_FOR_NULL_POINTERS */

			if(parameter_size == parameter_size_long)
			{
				/* A wide string, as a multibyte string.  The precision is
				   how many bytes may be written, and only complete
				   characters are. */
				const wchar_t * wide_string = va_arg(arg, wchar_t *);
				char multibyte[MB_LEN_MAX];
				mbstate_t state = 0;
				size_t total = 0;
				size_t n;

				if(wide_string == NULL)
				{
					output_buffer = NULL;
				}
				else
				{
					for(i = 0 ; wide_string[i] != L'\0' ; i++)
					{
						n = wcrtomb(multibyte,wide_string[i],&state);
						if(n == (size_t)-1)
							goto out;

						if(precision >= 0 && total + n > (size_t)precision)
							break;

						total += n;
					}

					output_buffer = text_buffer(buffer,sizeof(buffer),&internal_buffer,&internal_buffer_size,total + 1);
					if(output_buffer == NULL)
						goto out;

					total = 0;

					for(i = 0 ; wide_string[i] != L'\0' ; i++)
					{
						n = wcrtomb(multibyte,wide_string[i],&state);
						if(n == (size_t)-1)
							goto out;

						if(precision >= 0 && total + n > (size_t)precision)
							break;

						memcpy(&output_buffer[total],multibyte,n);
						total += n;
					}

					output_buffer[total] = '\0';
				}
			}
			else
			{
				output_buffer = va_arg(arg, char *);
			}

			#if defined(NDEBUG)
			{
				if(output_buffer == NULL)
				{
					output_buffer = buffer;
					strcpy(output_buffer,"");
				}
			}
			#else
			{
				if(output_buffer == NULL)
				{
					output_buffer = buffer;
					strcpy(output_buffer,"*NULL POINTER*");
				}
			}
			#endif /* NDEBUG */

			if(precision < 0)
			{
				output_len = strlen(output_buffer);
			}
			else
			{
				int num_characters = 0;

				for(i = 0 ; output_buffer[i] != '\0' ; i++)
				{
					/* A UTF-8 continuation byte doesn't start a character. */
					if(count_characters && (output_buffer[i] & 0xC0) == 0x80)
						continue;

					if(num_characters == precision)
						break;

					num_characters++;
				}

				output_len = i;
			}

			D(("string = '%s', length = %ld",output_buffer,output_len));

			CLEAR_FLAG(format_flags,FORMATF_ProduceSign);
			CLEAR_FLAG(format_flags,FORMATF_ProduceSpace);
		}
		else if (conversion_type == 'n')
		{
			SHOWMSG("number of characters");

			assert(arg != NULL);

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

				short_ptr = va_arg(arg, short *);

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

				(*short_ptr) = len;
			}
			else if (parameter_size == parameter_size_byte)
			{
				char * byte_ptr;

				byte_ptr = va_arg(arg, char *);

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

				(*byte_ptr) = len;
			}
			else
			{
				#if defined(USE_64_BIT_INTS) && defined(__GNUC__)
				{
					if(parameter_size == parameter_size_long_long || parameter_size == parameter_size_intmax_t)
					{
						long long * int_ptr;

						int_ptr = va_arg(arg, long long *);

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

						(*int_ptr) = len;
					}
					else
					{
						int * int_ptr;

						int_ptr = va_arg(arg, int *);

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

						(*int_ptr) = len;
					}
				}
				#else
				{
					int * int_ptr;

					int_ptr = va_arg(arg, int *);

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

					(*int_ptr) = len;
				}
				#endif /* __GNUC__ */
			}

			continue;
		}
		else
		{
			SHOWMSG("anything else");

			/* Just store the conversion_type character. */
			if(__putc(conversion_type,stream,buffer_mode) == EOF)
				goto out;

			len++;

			continue;
		}

		/* Get ready to prefix a sign character, if required. */
		if(FLAG_IS_SET(format_flags,FORMATF_IsNegative))
			prefix = "-";
		else if (FLAG_IS_SET(format_flags,FORMATF_ProduceSign))
			prefix = "+";
		else if (FLAG_IS_SET(format_flags,FORMATF_ProduceSpace))
			prefix = " ";
		else if (FLAG_IS_SET(format_flags,FORMATF_ZeroPrefix))
			prefix = "0";
		else
			prefix = NULL;

		if(FLAG_IS_SET(format_flags,FORMATF_HexPrefix))
		{
			strcpy(prefix_buffer,(prefix != NULL ? prefix : ""));
			strcat(prefix_buffer,FLAG_IS_SET(format_flags,FORMATF_CapitalLetters) ? "0X" : "0x");

			prefix = prefix_buffer;
		}

		if(FLAG_IS_SET(format_flags,FORMATF_LeftJustified))
		{
			if(prefix != NULL)
			{
				for(i = 0 ; prefix[i] != '\0' ; i++)
				{
					/* One less character to fill the output with. */
					minimum_field_width--;

					if(__putc(prefix[i],stream,buffer_mode) == EOF)
						goto out;

					len++;
				}
			}

			for(i = 0 ; i < num_leading_zeroes ; i++)
			{
				if(__putc('0',stream,buffer_mode) == EOF)
					goto out;

				len++;
			}

			for(i = 0 ; i < output_len ; i++)
			{
				if(__putc(output_buffer[i],stream,buffer_mode) == EOF)
					goto out;

				len++;
			}

			output_len += num_leading_zeroes;

			#if defined(FLOATING_POINT_SUPPORT)
			{
				for(i = 0 ; i < num_trailing_zeroes ; i++)
				{
					if(__putc('0',stream,buffer_mode) == EOF)
						goto out;

					output_len++;
					len++;
				}

				for(i = 0 ; trail_string[i] != '\0' ; i++)
				{
					if(__putc(trail_string[i],stream,buffer_mode) == EOF)
						goto out;

					output_len++;
					len++;
				}
			}
			#endif /* FLOATING_POINT_SUPPORT */

			for(i = output_len ; i < minimum_field_width ; i++)
			{
				/* Left justified output defaults to use the blank
				   space as the fill character. */
				if(__putc(' ',stream,buffer_mode) == EOF)
					goto out;

				len++;
			}
		}
		else
		{
			/* If we have to add the prefix later, make sure that
			   we don't add too many fill characters in front of
			   it now. */
			if(prefix != NULL)
			{
				for(i = 0 ; prefix[i] != '\0' ; i++)
				{
					/* One less character to fill the output with. */
					minimum_field_width--;

					if(fill_character == '0')
					{
						if(__putc(prefix[i],stream,buffer_mode) == EOF)
							goto out;

						len++;
					}
				}

				/* That takes care of the sign. */
				if(fill_character == '0')
					prefix = NULL;
			}

			#if defined(FLOATING_POINT_SUPPORT)
			{
				trail_string_len = strlen(trail_string);

				minimum_field_width -= num_trailing_zeroes + trail_string_len;
			}
			#endif /* FLOATING_POINT_SUPPORT */

			minimum_field_width -= num_leading_zeroes;

			for(i = output_len ; i < minimum_field_width ; i++)
			{
				if(__putc(fill_character,stream,buffer_mode) == EOF)
					goto out;

				len++;
			}

			/* If we still have a sign character to add, do it here. */
			if(prefix != NULL)
			{
				for(i = 0 ; prefix[i] != '\0' ; i++)
				{
					if(__putc(prefix[i],stream,buffer_mode) == EOF)
						goto out;

					len++;
				}
			}

			for(i = 0 ; i < num_leading_zeroes ; i++)
			{
				if(__putc('0',stream,buffer_mode) == EOF)
					goto out;

				len++;
			}

			for(i = 0 ; i < output_len ; i++)
			{
				if(__putc(output_buffer[i],stream,buffer_mode) == EOF)
					goto out;

				len++;
			}

			#if defined(FLOATING_POINT_SUPPORT)
			{
				for(i = 0 ; i < num_trailing_zeroes ; i++)
				{
					if(__putc('0',stream,buffer_mode) == EOF)
						goto out;

					len++;
				}

				for(i = 0 ; i < trail_string_len ; i++)
				{
					if(__putc(trail_string[i],stream,buffer_mode) == EOF)
						goto out;

					len++;
				}
			}
			#endif /* FLOATING_POINT_SUPPORT */
		}
	}

	if(FLAG_IS_CLEAR(iob->iob_Flags,IOBF_NO_NUL))
	{
		if(__putc('\0',stream,buffer_mode) == EOF)
			goto out;
	}

	result = len;

 out:

	if(internal_buffer != NULL && internal_buffer_size > 0)
		free(internal_buffer);

	/* Note: if buffering is disabled for this stream, then we still
	   may have buffered data around, queued to be printed right now.
	   This is intended to improve performance as it takes more effort
	   to write a single character to a file than to write a bunch. */
	if(result != EOF && (iob->iob_Flags & IOBF_BUFFER_MODE) == IOBF_BUFFER_MODE_NONE)
	{
		if(__iob_write_buffer_is_valid(iob) && __flush_iob_write_buffer(iob) < 0)
			result = EOF;
	}

	funlockfile(stream);

	RETURN(result);
	return(result);
}
