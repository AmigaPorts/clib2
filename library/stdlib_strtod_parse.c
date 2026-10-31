/*
 * Experimental: Not a part of clib2 from Olaf Barthel
 */

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

#ifndef _LOCALE_HEADERS_H
#include "locale_headers.h"
#endif /* _LOCALE_HEADERS_H */

#ifndef _STDLIB_STRTOD_PARSE_H
#include "stdlib_strtod_parse.h"
#endif /* _STDLIB_STRTOD_PARSE_H */

/****************************************************************************/

static int
digit_value(char c)
{
	if('0' <= c && c <= '9')
		return(c - '0');
	else if ('a' <= c && c <= 'f')
		return(c - 'a' + 10);
	else if ('A' <= c && c <= 'F')
		return(c - 'A' + 10);
	else
		return(99);
}

/****************************************************************************/

/* Does str begin with the decimal point?  Both the locale's and '.' are
   accepted.  Returns its length, or 0. */
static int
decimal_point_length(const char * str)
{
	int length = 0;

	__locale_lock();

	if(__locale_table[LC_NUMERIC] != NULL)
	{
		const char * point = (const char *)__locale_table[LC_NUMERIC]->loc_DecimalPoint;

		if(point != NULL && (*point) != '\0' && (*point) == (*str))
			length = 1;
	}

	__locale_unlock();

	if(length == 0 && (*str) == '.')
		length = 1;

	return(length);
}

/****************************************************************************/

/* Up to PN_MAX_DECIMAL_DIGITS significant decimal digits, or
   PN_MAX_HEX_DIGITS hexadecimal ones, are collected exactly in the
   mantissa and rounded at the first digit that didn't fit; further digits
   only move the exponent.  This accepts what C99 asks of strtod(): leading
   blanks, a sign, "inf", "infinity", "nan" and "nan(n-char-sequence)",
   decimal and hexadecimal numbers, and an exponent only if it is
   complete. */
const char *
__parse_number(const char * str,struct __parsed_number * pn)
{
	const char * start = str;
	unsigned long long mantissa = 0;
	size_t num_digits = 0;
	int dropped_digit = -1;
	long exponent = 0;
	int max_digits;
	int radix;
	int point;
	int d;

	pn->pn_kind = PN_NONE;
	pn->pn_is_negative = 0;
	pn->pn_mantissa = 0;
	pn->pn_exponent = 0;
	pn->pn_is_hex = 0;

	while(isspace((unsigned char)(*str)))
		str++;

	if((*str) == '-' || (*str) == '+')
	{
		pn->pn_is_negative = ((*str) == '-');
		str++;
	}

	if(strncasecmp(str,"inf",3) == SAME)
	{
		str += 3;

		if(strncasecmp(str,"inity",5) == SAME)
			str += 5;

		pn->pn_kind = PN_INFINITY;
		return(str);
	}

	if(strncasecmp(str,"nan",3) == SAME)
	{
		str += 3;

		/* Skip "(n-char-sequence)", if it is complete. */
		if((*str) == '(')
		{
			const char * s = str + 1;

			while(isalnum((unsigned char)(*s)) || (*s) == '_')
				s++;

			if((*s) == ')')
				str = s + 1;
		}

		pn->pn_kind = PN_NAN;
		return(str);
	}

	/* "0x" begins a hexadecimal number only if a hex digit, or the decimal
	   point and one, follows; otherwise the "0" is the number. */
	radix = 10;

	if(str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
	{
		const char * s = str + 2;

		point = decimal_point_length(s);

		if(digit_value(s[0]) < 16 || (point > 0 && digit_value(s[point]) < 16))
		{
			radix = 16;
			str = s;
		}
	}

	max_digits = (radix == 16) ? PN_MAX_HEX_DIGITS : PN_MAX_DECIMAL_DIGITS;

	/* The digits before the decimal point.  Leading zeroes are not
	   significant digits. */
	while((d = digit_value(*str)) < radix)
	{
		if(mantissa != 0 || d != 0)
		{
			if(max_digits > 0)
			{
				mantissa = mantissa * radix + d;
				max_digits--;
			}
			else
			{
				if(dropped_digit < 0)
					dropped_digit = d;

				exponent++;
			}
		}

		num_digits++;
		str++;
	}

	point = decimal_point_length(str);
	if(point > 0 && (num_digits > 0 || digit_value(str[point]) < radix))
	{
		str += point;

		/* The digits after it; each one that is kept moves the exponent
		   down. */
		while((d = digit_value(*str)) < radix)
		{
			if(max_digits > 0)
			{
				if(mantissa != 0 || d != 0)
				{
					mantissa = mantissa * radix + d;
					max_digits--;
				}

				exponent--;
			}
			else if (dropped_digit < 0)
			{
				dropped_digit = d;
			}

			num_digits++;
			str++;
		}
	}

	/* No number at all: nothing is converted. */
	if(num_digits == 0)
	{
		pn->pn_is_negative = 0;
		return(start);
	}

	/* Round at the first digit that didn't fit.  19 nines plus one still
	   fit in the mantissa; 16 hexadecimal Fs plus one would not, so those
	   are left as they are. */
	if(dropped_digit >= radix / 2 && mantissa != ~0ULL)
		mantissa++;

	/* A hexadecimal digit is worth four binary ones. */
	if(radix == 16)
		exponent *= 4;

	/* The exponent, if a complete one follows. */
	if((radix == 10 && ((*str) == 'e' || (*str) == 'E')) ||
	   (radix == 16 && ((*str) == 'p' || (*str) == 'P')))
	{
		const char * s = str + 1;
		int exponent_is_negative = 0;

		if((*s) == '-' || (*s) == '+')
		{
			exponent_is_negative = ((*s) == '-');
			s++;
		}

		if('0' <= (*s) && (*s) <= '9')
		{
			long e = 0;

			while('0' <= (*s) && (*s) <= '9')
			{
				/* Anything this large overflows or underflows anyway;
				   stop counting before e would. */
				if(e < 100000)
					e = e * 10 + ((*s) - '0');

				s++;
			}

			exponent += exponent_is_negative ? -e : e;
			str = s;
		}
	}

	pn->pn_kind = PN_NUMBER;
	pn->pn_mantissa = mantissa;
	pn->pn_exponent = exponent;
	pn->pn_is_hex = (radix == 16);

	return(str);
}
