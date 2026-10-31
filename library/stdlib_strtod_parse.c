/*
 * Experimental: Not a part of clib2 from Olaf Barthel
 */

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

#ifndef _LOCALE_HEADERS_H
#include "locale_headers.h"
#endif /* _LOCALE_HEADERS_H */

#ifndef _MATH_HEADERS_H
#include "math_headers.h"
#endif /* _MATH_HEADERS_H */

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

/* What __parse_number() keeps of a hexadecimal number: the first 64 bits
   from the first one on, and how much of the unit in the last place the
   bits that didn't fit are worth. */
struct hex_digits
{
	unsigned long long	hd_mantissa;
	int					hd_rest;
	int					hd_is_full;
	int					hd_first_dropped;
};

/* Adds a digit, and returns how many of its four bits went into the
   mantissa. */
static int
add_hex_digit(struct hex_digits * hd,int d)
{
	int bits_free;
	int bits_dropped;
	int dropped;

	if(hd->hd_is_full)
	{
		bits_free = 0;
	}
	else
	{
		bits_free = 0;

		while(bits_free < 4 && (hd->hd_mantissa >> (63 - bits_free)) == 0)
			bits_free++;

		if(bits_free == 4)
		{
			hd->hd_mantissa = (hd->hd_mantissa << 4) + d;

			return(4);
		}

		hd->hd_mantissa = (hd->hd_mantissa << bits_free) + (d >> (4 - bits_free));
		hd->hd_is_full = 1;
	}

	bits_dropped = 4 - bits_free;
	dropped = d & ((1 << bits_dropped) - 1);

	if(hd->hd_first_dropped)
	{
		hd->hd_first_dropped = 0;

		if(dropped == 0)
			hd->hd_rest = PN_REST_NONE;
		else if (dropped < (1 << (bits_dropped - 1)))
			hd->hd_rest = PN_REST_BELOW_HALF;
		else if (dropped == (1 << (bits_dropped - 1)))
			hd->hd_rest = PN_REST_HALF;
		else
			hd->hd_rest = PN_REST_ABOVE_HALF;
	}
	else if (dropped != 0)
	{
		if(hd->hd_rest == PN_REST_NONE)
			hd->hd_rest = PN_REST_BELOW_HALF;
		else if (hd->hd_rest == PN_REST_HALF)
			hd->hd_rest = PN_REST_ABOVE_HALF;
	}

	return(bits_free);
}

/****************************************************************************/

/* This accepts what C99 asks of strtod(): leading blanks, a sign, "inf",
   "infinity", "nan" and "nan(n-char-sequence)", decimal and hexadecimal
   numbers, and an exponent only if it is complete.  Decimal digits stay
   where they are in str; the conversion goes back to them. */
const char *
__parse_number(const char * str,struct __parsed_number * pn)
{
	const char * start = str;
	struct hex_digits hd;
	size_t num_digits = 0;
	long exponent = 0;
	int is_zero = 1;
	int radix;
	int point;
	int d;

	hd.hd_mantissa = 0;
	hd.hd_rest = PN_REST_NONE;
	hd.hd_is_full = 0;
	hd.hd_first_dropped = 1;

	pn->pn_kind = PN_NONE;
	pn->pn_is_negative = 0;
	pn->pn_is_zero = 1;
	pn->pn_is_hex = 0;
	pn->pn_mantissa = 0;
	pn->pn_rest = PN_REST_NONE;
	pn->pn_exponent = 0;
	pn->pn_digits = NULL;
	pn->pn_digits_end = NULL;

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

	pn->pn_digits = str;

	/* The digits before the decimal point. */
	while((d = digit_value(*str)) < radix)
	{
		if(d != 0)
			is_zero = 0;

		if(radix == 16)
			exponent += 4 - add_hex_digit(&hd,d);

		num_digits++;
		str++;
	}

	point = decimal_point_length(str);
	if(point > 0 && (num_digits > 0 || digit_value(str[point]) < radix))
	{
		str += point;

		/* The digits after it move the exponent down, if they count. */
		while((d = digit_value(*str)) < radix)
		{
			if(d != 0)
				is_zero = 0;

			if(radix == 16)
				exponent -= add_hex_digit(&hd,d);
			else
				exponent--;

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

	pn->pn_digits_end = str;

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
				if(e < 100000000)
					e = e * 10 + ((*s) - '0');

				s++;
			}

			exponent += exponent_is_negative ? -e : e;
			str = s;
		}
	}

	pn->pn_kind = PN_NUMBER;
	pn->pn_is_zero = is_zero;
	pn->pn_is_hex = (radix == 16);
	pn->pn_mantissa = hd.hd_mantissa;
	pn->pn_rest = hd.hd_rest;
	pn->pn_exponent = exponent;

	return(str);
}

/****************************************************************************/

/* The number, exactly: (mantissa + rest) * 2^exponent, with the top bit of
   the mantissa set; 0 <= rest < 1 is only known in relation to one half. */
struct binary_number
{
	unsigned long long	bn_mantissa;
	long				bn_exponent;
	int					bn_rest;
};

#define BASE 1000000000UL
#define BASE_DIGITS 9

/* All decimal digits past these can be replaced by a single one that isn't
   zero: no number halfway between two long doubles has more digits. */
#define MAX_DIGITS 11600

/* Numbers with fewer digits before the decimal point, or more zeroes after
   it, can't be told from infinity or zero. */
#define MAX_DECIMAL_EXPONENT 5000

/* How many fractional words to keep while shifting to the right.  Together
   they are far more than the few bits that can still move into the
   integer part. */
#define FRACTION_WORDS 8

/* Enough for all numbers that are likely to come up; the others go to
   malloc(). */
#define NUM_STACK_WORDS 72

static int
count_leading_zeroes(unsigned long long x)
{
	int n = 0;

	while((x >> 63) == 0)
	{
		x <<= 1;
		n++;
	}

	return(n);
}

/* Converts the decimal digits of pn to binary, with the digits in base 10^9
   in an array that holds the integer part in the words before index r and
   the fraction in those from r on.  Shifting right by nine bits divides
   each word by 2^9 exactly and leaves 5^9 times its remainder for the next
   word; this goes on until the integer part fits a mantissa.  Shifting left
   then moves the top bit of the mantissa to where it belongs.  Returns 0 if
   there is no memory. */
static int
decimal_to_binary(const struct __parsed_number * pn,struct binary_number * bn)
{
	unsigned long stack_words[NUM_STACK_WORDS];
	unsigned long * words = stack_words;
	unsigned long * a;
	unsigned long * d;
	unsigned long * r;
	unsigned long * z;
	unsigned long * end;
	unsigned long long integer;
	const char * s;
	const char * first = NULL;
	const char * last = NULL;
	long exponent = pn->pn_exponent;
	long binary_exponent = 0;
	long num_digits = 0;
	long num_words;
	long position;
	long integer_words;
	long fraction_words;
	long num_all_words;
	int sticky = 0;
	int padding;
	int result = 0;

	/* The significant digits are those from the first to the last that
	   isn't zero.  exponent belongs to the digit where the digits end. */
	for(s = pn->pn_digits ; s != pn->pn_digits_end ; s++)
	{
		if('1' <= (*s) && (*s) <= '9')
		{
			if(first == NULL)
				first = s;

			last = s;
		}
	}

	for(s = last + 1 ; s != pn->pn_digits_end ; s++)
	{
		if('0' <= (*s) && (*s) <= '9')
			exponent++;
	}

	for(s = first ; s <= last ; s++)
	{
		if('0' <= (*s) && (*s) <= '9')
		{
			if(num_digits == MAX_DIGITS)
			{
				sticky = 1;
				exponent++;
				continue;
			}

			num_digits++;
		}
	}

	/* 10^(position - 1) <= number < 10^position */
	position = num_digits + exponent;

	if(position > MAX_DECIMAL_EXPONENT)
	{
		bn->bn_mantissa = 1ULL << 63;
		bn->bn_exponent = 100000;
		bn->bn_rest = PN_REST_NONE;

		return(1);
	}

	if(position < -MAX_DECIMAL_EXPONENT)
	{
		bn->bn_mantissa = 1ULL << 63;
		bn->bn_exponent = -100000;
		bn->bn_rest = PN_REST_NONE;

		return(1);
	}

	/* Make the exponent a multiple of nine with some more zero digits. */
	padding = (int)(((exponent % BASE_DIGITS) + BASE_DIGITS) % BASE_DIGITS);

	exponent -= padding;
	num_digits += padding;

	num_words = (num_digits + BASE_DIGITS - 1) / BASE_DIGITS;
	position = exponent / BASE_DIGITS;

	integer_words = position + num_words;
	if(integer_words < 0)
		integer_words = 0;

	fraction_words = -position;
	if(fraction_words < 0)
		fraction_words = 0;

	num_all_words = 4 + integer_words + fraction_words + FRACTION_WORDS;

	if(num_all_words > NUM_STACK_WORDS)
	{
		words = malloc(num_all_words * sizeof(*words));
		if(words == NULL)
		{
			__set_errno(ENOMEM);
			goto out;
		}
	}

	memset(words,0,num_all_words * sizeof(*words));

	r = &words[4 + integer_words];
	end = &words[num_all_words];

	/* The last word holds the last digits, and the first one the rest. */
	a = r - position - num_words;
	z = (position < 0) ? r - position : r;

	{
		unsigned long word = 0;
		long remaining = num_digits - padding;
		int digits_in_word = (int)((num_digits - 1) % BASE_DIGITS) + 1;

		d = a;

		for(s = first ; remaining + padding > 0 ; )
		{
			if(remaining > 0)
			{
				if((*s) < '0' || (*s) > '9')
				{
					s++;
					continue;
				}

				word = 10 * word + ((*s++) - '0');
				remaining--;
			}
			else
			{
				word = 10 * word;
				padding--;
			}

			if(--digits_in_word == 0)
			{
				(*d++) = word;

				word = 0;
				digits_in_word = BASE_DIGITS;
			}
		}
	}

	/* Too large: shift to the right until the integer part fits. */
	while(a < r - 2)
	{
		unsigned long carry = 0;

		for(d = a ; d < z ; d++)
		{
			unsigned long rest = (*d) & ((1UL << 9) - 1);

			(*d) = ((*d) >> 9) + carry;
			carry = (BASE >> 9) * rest;
		}

		if((*a) == 0)
			a++;

		if(carry != 0)
		{
			if(z < end)
				(*z++) = carry;
			else
				sticky = 1;
		}

		binary_exponent += 9;
	}

	/* Too small, or not normalized yet: shift to the left. */
	while(TRUE)
	{
		unsigned long carry = 0;
		int shift;

		integer = 0;

		for(d = r - 3 ; d < r ; d++)
			integer = integer * BASE + (*d);

		if(integer == 0)
			shift = 29;
		else
			shift = count_leading_zeroes(integer);

		if(shift == 0)
			break;

		if(shift > 29)
			shift = 29;

		while(z > a && z[-1] == 0)
			z--;

		for(d = z - 1 ; d >= r - 3 ; d--)
		{
			unsigned long long x = (((unsigned long long)(*d)) << shift) + carry;

			carry = (unsigned long)(x / BASE);
			(*d) = (unsigned long)(x - (unsigned long long)carry * BASE);
		}

		if(a > r - 3)
			a = r - 3;

		binary_exponent -= shift;
	}

	bn->bn_mantissa = integer;
	bn->bn_exponent = binary_exponent;

	while(z > r && z[-1] == 0)
		z--;

	if(z <= r)
		bn->bn_rest = sticky ? PN_REST_BELOW_HALF : PN_REST_NONE;
	else if ((*r) < BASE / 2)
		bn->bn_rest = PN_REST_BELOW_HALF;
	else if ((*r) == BASE / 2 && z == r + 1 && NOT sticky)
		bn->bn_rest = PN_REST_HALF;
	else
		bn->bn_rest = PN_REST_ABOVE_HALF;

	result = 1;

 out:

	if(words != stack_words && words != NULL)
		free(words);

	return(result);
}

/****************************************************************************/

/* Rounds the number to num_bits of mantissa, ties to even, or to fewer where
   its exponent is below min_exponent, the smallest one that has them all.
   The result is *mantissa * 2^(*exponent - num_bits + 1) if it isn't
   denormal.  Returns zero if there is no memory. */
static int
round_number(const struct __parsed_number * pn,int num_bits,long min_exponent,
	unsigned long long * mantissa,long * exponent,int * is_denormal)
{
	struct binary_number bn;
	unsigned long long kept;
	unsigned long long dropped;
	long e;
	int num_dropped;
	int comparison;	/* of the part that's dropped to half a unit: -1, 0 or 1 */

	if(pn->pn_is_hex)
	{
		int shift = count_leading_zeroes(pn->pn_mantissa);

		bn.bn_mantissa = pn->pn_mantissa << shift;
		bn.bn_exponent = pn->pn_exponent - shift;
		bn.bn_rest = pn->pn_rest;
	}
	else if (NOT decimal_to_binary(pn,&bn))
	{
		return(0);
	}

	/* 2^e <= number < 2^(e + 1) */
	e = bn.bn_exponent + 63;

	num_dropped = 64 - num_bits;
	(*is_denormal) = (e < min_exponent);

	if((*is_denormal))
	{
		if(min_exponent - e > 64)
			num_dropped = 65;
		else
			num_dropped += (int)(min_exponent - e);

		if(num_dropped > 65)
			num_dropped = 65;
	}

	if(num_dropped == 0)
	{
		kept = bn.bn_mantissa;

		switch(bn.bn_rest)
		{
			case PN_REST_NONE:
			case PN_REST_BELOW_HALF:	comparison = -1; break;
			case PN_REST_HALF:			comparison = 0;  break;
			default:					comparison = 1;  break;
		}
	}
	else if (num_dropped > 64)
	{
		kept = 0;
		comparison = -1;
	}
	else
	{
		kept = (num_dropped == 64) ? 0 : (bn.bn_mantissa >> num_dropped);
		dropped = (num_dropped == 64) ? bn.bn_mantissa : (bn.bn_mantissa << (64 - num_dropped));

		if(dropped > (1ULL << 63))
			comparison = 1;
		else if (dropped == (1ULL << 63))
			comparison = (bn.bn_rest == PN_REST_NONE) ? 0 : 1;
		else
			comparison = -1;
	}

	if(comparison == 1 || (comparison == 0 && (kept & 1) != 0))
	{
		kept++;

		/* Rounded up into the next power of two; a denormal number that
		   does gets there by itself. */
		if(NOT (*is_denormal))
		{
			if(num_bits == 64 ? (kept == 0) : (kept == (1ULL << num_bits)))
			{
				kept = 1ULL << (num_bits - 1);
				e++;
			}
		}
	}

	(*mantissa) = kept;
	(*exponent) = e;

	return(1);
}

/****************************************************************************/

/* The magnitude of a PN_NUMBER, as the bits of a double. */
static unsigned long long
double_bits(const struct __parsed_number * pn)
{
	unsigned long long mantissa;
	long exponent;
	int is_denormal;

	if(pn->pn_is_zero)
		return(0);

	if(NOT round_number(pn,53,-1022,&mantissa,&exponent,&is_denormal))
		return(0);

	if(exponent > 1023)
	{
		SHOWMSG("overflow");
		__set_errno(ERANGE);

		return(0x7FF0000000000000ULL);
	}

	if(is_denormal)
	{
		if(mantissa == 0)
		{
			SHOWMSG("underflow");
			__set_errno(ERANGE);
		}

		return(mantissa);
	}

	return((((unsigned long long)(exponent + 1022)) << 52) + mantissa);
}

/****************************************************************************/

double
__parsed_to_double(const struct __parsed_number * pn)
{
	union
	{
		double				d;
		unsigned long long	bits;
	} result;

	switch(pn->pn_kind)
	{
		case PN_INFINITY:

			result.bits = 0x7FF0000000000000ULL;
			break;

		case PN_NAN:

			result.bits = 0x7FF8000000000000ULL;
			break;

		case PN_NUMBER:

			result.bits = double_bits(pn);
			break;

		default:

			result.bits = 0;
			break;
	}

	if(pn->pn_is_negative)
		result.bits |= 1ULL << 63;

	return(result.d);
}

/****************************************************************************/

float
__parsed_to_float(const struct __parsed_number * pn)
{
	union
	{
		float			f;
		unsigned long	bits;
	} result;

	unsigned long long mantissa;
	long exponent;
	int is_denormal;

	switch(pn->pn_kind)
	{
		case PN_INFINITY:

			result.bits = 0x7F800000UL;
			break;

		case PN_NAN:

			result.bits = 0x7FC00000UL;
			break;

		case PN_NUMBER:

			if(pn->pn_is_zero || NOT round_number(pn,24,-126,&mantissa,&exponent,&is_denormal))
			{
				result.bits = 0;
			}
			else if (exponent > 127)
			{
				SHOWMSG("overflow");
				__set_errno(ERANGE);

				result.bits = 0x7F800000UL;
			}
			else if (is_denormal)
			{
				if(mantissa == 0)
				{
					SHOWMSG("underflow");
					__set_errno(ERANGE);
				}

				result.bits = (unsigned long)mantissa;
			}
			else
			{
				result.bits = (((unsigned long)(exponent + 126)) << 23) + (unsigned long)mantissa;
			}

			break;

		default:

			result.bits = 0;
			break;
	}

	if(pn->pn_is_negative)
		result.bits |= 0x80000000UL;

	return(result.f);
}

/****************************************************************************/

#if LDBL_MANT_DIG == 64 && defined(__mc68000__)

/* The 96 bit format of the 68881: a sign, a 15 bit exponent, 16 unused bits
   and a 64 bit mantissa with an explicit top bit.  An exponent of 0 has the
   same weight as one of 1 would have with one bit less, so its numbers
   count as normal down to 2^-16383. */
long double
__parsed_to_long_double(const struct __parsed_number * pn)
{
	union
	{
		long double	ld;
		struct
		{
			unsigned long		sign_exponent;
			unsigned long long	mantissa;
		} bits;
	} result;

	unsigned long long mantissa = 0;
	long exponent = 0;
	int is_denormal;

	result.bits.sign_exponent = 0;
	result.bits.mantissa = 0;

	switch(pn->pn_kind)
	{
		case PN_INFINITY:

			result.bits.sign_exponent = 0x7FFFUL << 16;
			break;

		case PN_NAN:

			result.bits.sign_exponent = 0x7FFFUL << 16;
			result.bits.mantissa = 3ULL << 62;
			break;

		case PN_NUMBER:

			if(pn->pn_is_zero || NOT round_number(pn,64,-16383,&mantissa,&exponent,&is_denormal))
				break;

			if(exponent > 16383)
			{
				SHOWMSG("overflow");
				__set_errno(ERANGE);

				result.bits.sign_exponent = 0x7FFFUL << 16;
			}
			else if (is_denormal)
			{
				if(mantissa == 0)
				{
					SHOWMSG("underflow");
					__set_errno(ERANGE);
				}

				result.bits.mantissa = mantissa;
			}
			else
			{
				result.bits.sign_exponent = ((unsigned long)(exponent + 16383)) << 16;
				result.bits.mantissa = mantissa;
			}

			break;

		default:

			break;
	}

	if(pn->pn_is_negative)
		result.bits.sign_exponent |= 0x80000000UL;

	return(result.ld);
}

#endif /* LDBL_MANT_DIG == 64 && __mc68000__ */
