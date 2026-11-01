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

#ifndef _STDLIB_STRTOD_PARSE_H
#include "stdlib_strtod_parse.h"
#endif /* _STDLIB_STRTOD_PARSE_H */

/****************************************************************************/

/* A long double that is wider than double, such as the 80-bit extended
   precision format of m68k-amigaos.  None of the long double math functions
   are available there, so this does its own scaling: with exactly rounded
   powers of ten, or two, which costs a few roundings in the last place of
   the mantissa at most.  It is not correctly rounded in every case, but much
   closer than going through double.  strtod() uses it too, and rounds the
   result to double. */

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

/****************************************************************************/

/* The magnitude of a PN_NUMBER; overflow gives infinity, underflow 0. */
long double
__parsed_to_long_double(const struct __parsed_number * pn)
{
	long double result;

	if(pn->pn_is_hex)
	{
		result = scale_by_power_of_two((long double)pn->pn_mantissa, pn->pn_exponent);
	}
	else
	{
		result = scale_by_power_of_ten((long double)pn->pn_mantissa, pn->pn_exponent);

		/* The roundings while scaling can carry a number just below the
		   overflow threshold, such as LDBL_MAX written out in decimal,
		   past it.  If it is within those few roundings of the threshold,
		   it is LDBL_MAX. */
		if(__builtin_isinf(result))
		{
			long double tenth = scale_by_power_of_ten((long double)pn->pn_mantissa, pn->pn_exponent - 1);

			if(NOT __builtin_isinf(tenth * (10.0L - 80.0L * LDBL_EPSILON)))
				result = LDBL_MAX;
		}
	}

	return(result);
}

/****************************************************************************/

long double
strtold(const char * __restrict str, char ** __restrict ptr)
{
	struct __parsed_number pn;
	const char * stop = str;
	long double result = 0.0L;

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
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	stop = __parse_number(str,&pn);

	switch(pn.pn_kind)
	{
		case PN_INFINITY:

			result = __builtin_infl();
			break;

		case PN_NAN:

			result = __builtin_nanl("");
			break;

		case PN_NUMBER:

			result = __parsed_to_long_double(&pn);

			if(__builtin_isinf(result))
			{
				SHOWMSG("overflow");
				__set_errno(ERANGE);
			}
			else if (result == 0.0L && pn.pn_mantissa != 0)
			{
				SHOWMSG("underflow");
				__set_errno(ERANGE);
			}

			break;

		default:

			break;
	}

	if(pn.pn_is_negative)
		result = -result;

#if defined(CHECK_FOR_NULL_POINTERS)
 out:
#endif /* CHECK_FOR_NULL_POINTERS */

	if(ptr != NULL)
		(*ptr) = (char *)stop;

	RETURN(result);
	return(result);
}

/****************************************************************************/

#endif /* LDBL_MANT_DIG == DBL_MANT_DIG && LDBL_MAX_EXP == DBL_MAX_EXP */

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
