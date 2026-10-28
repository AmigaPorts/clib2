/*
 * Experimental: Not a part of clib2 from Olaf Barthel
 */

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

#ifndef _MATH_HEADERS_H
#include "math_headers.h"
#endif /* _MATH_HEADERS_H */

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT)

/****************************************************************************/

#if LDBL_MANT_DIG == DBL_MANT_DIG && LDBL_MAX_EXP == DBL_MAX_EXP

/****************************************************************************/

long double
strtold(const char * __restrict str, char ** __restrict ptr)
{
    return strtod(str, ptr);
}

/****************************************************************************/

#else

/****************************************************************************/

#ifndef _LOCALE_HEADERS_H
#include "locale_headers.h"
#endif /* _LOCALE_HEADERS_H */

/****************************************************************************/

/* A long double that is wider than double, such as the 80-bit extended
   precision format of m68k-amigaos.  None of the long double math functions
   are available there, so this does its own scaling.

   Up to 19 significant decimal digits, or 16 hexadecimal ones, are collected
   exactly in an unsigned long long, which the 64 bit mantissa holds; further
   digits only move the exponent.  The result is then scaled with exactly
   rounded powers of ten, or two, which costs a few roundings in the last
   place of the mantissa at most.  It is not correctly rounded in every case,
   but much closer than going through double.  */

#define MAX_DECIMAL_DIGITS	19
#define MAX_HEX_DIGITS		16

/* 10^(2^i), for i = 0 ... 12; 10^8192 is beyond the largest long double. */
static const long double powers_of_ten[] =
{
	1e1L, 1e2L, 1e4L, 1e8L, 1e16L, 1e32L, 1e64L, 1e128L, 1e256L,
	1e512L, 1e1024L, 1e2048L, 1e4096L
};

#define NUM_POWERS_OF_TEN ((int)(sizeof(powers_of_ten) / sizeof(powers_of_ten[0])))

/* x * 10^exponent, by the largest powers first, so that an intermediate
   result only leaves the range of a long double when the final one would. */
static long double
scale_by_power_of_ten(long double x, long exponent)
{
	int is_negative = (exponent < 0);
	unsigned long e = is_negative ? -(unsigned long)exponent : (unsigned long)exponent;
	int i;

	for(i = NUM_POWERS_OF_TEN - 1 ; i >= 0 && x != 0.0L ; i--)
	{
		unsigned long bit = 1UL << i;

		while(e >= bit)
		{
			if(is_negative)
				x /= powers_of_ten[i];
			else
				x *= powers_of_ten[i];

			e -= bit;

			if(__builtin_isinf(x) || x == 0.0L)
				return(x);
		}
	}

	return(x);
}

/* x * 2^exponent */
static long double
scale_by_power_of_two(long double x, long exponent)
{
	while(exponent >= 64 && x != 0.0L && NOT __builtin_isinf(x))
	{
		x *= 0x1p64L;
		exponent -= 64;
	}

	while(exponent <= -64 && x != 0.0L)
	{
		x *= 0x1p-64L;
		exponent += 64;
	}

	if(exponent > 0)
		x *= (long double)(1ULL << exponent);
	else if (exponent < 0)
		x /= (long double)(1ULL << (-exponent));

	return(x);
}

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

/* Does str begin with the decimal point?  Both the locale's and '.' are
   accepted, as with strtod().  Returns its length, or 0. */
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

long double
strtold(const char * __restrict str, char ** __restrict ptr)
{
	const char * stop = str;
	long double result;
	int is_negative = 0;

	ENTER();

	SHOWSTRING(str);
	SHOWPOINTER(ptr);

	assert( str != NULL );

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(str == NULL)
		{
			SHOWMSG("invalid str parameter");

			__set_errno(EFAULT);

			result = 0.0L;
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	while(isspace((unsigned char)(*str)))
		str++;

	if((*str) == '-' || (*str) == '+')
	{
		is_negative = ((*str) == '-');
		str++;
	}

	if(strncasecmp(str,"inf",3) == SAME)
	{
		SHOWMSG("infinity");

		str += 3;

		if(strncasecmp(str,"inity",5) == SAME)
			str += 5;

		result = __builtin_infl();
		stop = str;
	}
	else if (strncasecmp(str,"nan",3) == SAME)
	{
		SHOWMSG("not a number");

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

		result = __builtin_nanl("");
		stop = str;
	}
	else
	{
		unsigned long long mantissa = 0;
		size_t num_digits = 0;
		long exponent = 0;
		int dropped_digit = -1;
		int max_digits;
		int radix;
		int point;
		int d;

		/* "0x" begins a hexadecimal number only if a hex digit, or the
		   decimal point and one, follows; otherwise the "0" is the
		   number. */
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

		max_digits = (radix == 16) ? MAX_HEX_DIGITS : MAX_DECIMAL_DIGITS;

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

			/* The digits after it; each one that is kept moves the
			   exponent down. */
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

		if(num_digits == 0)
		{
			/* No number at all: nothing is converted, and ptr gets
			   the string as it was passed. */
			result = 0.0L;
			goto out;
		}

		/* Round at the first digit that didn't fit.  19 nines plus
		   one still fit in the mantissa; 16 hexadecimal Fs plus one
		   would not, so those are left as they are. */
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
					/* Anything this large overflows or underflows
					   anyway; stop counting before e would. */
					if(e < 100000)
						e = e * 10 + ((*s) - '0');

					s++;
				}

				exponent += exponent_is_negative ? -e : e;
				str = s;
			}
		}

		stop = str;

		if(radix == 16)
		{
			result = scale_by_power_of_two((long double)mantissa, exponent);
		}
		else
		{
			result = scale_by_power_of_ten((long double)mantissa, exponent);

			/* The roundings while scaling can carry a number just
			   below the overflow threshold, such as LDBL_MAX written
			   out in decimal, past it.  If it is within those few
			   roundings of the threshold, it is LDBL_MAX. */
			if(__builtin_isinf(result))
			{
				long double tenth = scale_by_power_of_ten((long double)mantissa, exponent - 1);

				if(NOT __builtin_isinf(tenth * (10.0L - 80.0L * LDBL_EPSILON)))
					result = LDBL_MAX;
			}
		}

		if(__builtin_isinf(result))
		{
			SHOWMSG("overflow");
			__set_errno(ERANGE);
		}
		else if (result == 0.0L && mantissa != 0)
		{
			SHOWMSG("underflow");
			__set_errno(ERANGE);
		}
	}

	if(is_negative)
		result = -result;

 out:

	if(ptr != NULL)
		(*ptr) = (char *)stop;

	RETURN(result);
	return(result);
}

/****************************************************************************/

#endif /* LDBL_MANT_DIG == DBL_MANT_DIG && LDBL_MAX_EXP == DBL_MAX_EXP */

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
