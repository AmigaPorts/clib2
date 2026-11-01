/*
 * Experimental: Not a part of clib2 from Olaf Barthel
 */

#ifndef _STDLIB_STRTOD_PARSE_H
#define _STDLIB_STRTOD_PARSE_H

/****************************************************************************/

/* What strtod(), strtof() and strtold() read, before it becomes a number. */
struct __parsed_number
{
	int					pn_kind;		/* PN_NONE, PN_NUMBER, PN_INFINITY or PN_NAN */
	int					pn_is_negative;
	int					pn_is_zero;
	int					pn_is_hex;
	unsigned long long	pn_mantissa;	/* if pn_is_hex, the first 64 bits */
	int					pn_rest;		/* if pn_is_hex, the bits after those */
	long				pn_exponent;	/* of 10, or of 2 if pn_is_hex */
	const char *		pn_digits;		/* the decimal digits and point in */
	const char *		pn_digits_end;	/* the string, if not pn_is_hex */
};

#define PN_NONE		0	/* not a number; nothing was converted */
#define PN_NUMBER	1
#define PN_INFINITY	2
#define PN_NAN		3

/* What's left of a number in relation to half a unit in the last place. */
#define PN_REST_NONE		0
#define PN_REST_BELOW_HALF	1
#define PN_REST_HALF		2
#define PN_REST_ABOVE_HALF	3

/* Parse the number str begins with, as strtod() does, into *pn.  Returns
   where it ends, or str itself if there is no number. */
extern const char * __parse_number(const char * str,struct __parsed_number * pn);

/* What strtod(), strtof() and strtold() return for it, correctly rounded
   to nearest, ties to even, with errno set to ERANGE on overflow, and when
   a number that isn't zero comes out as zero. */
extern double __parsed_to_double(const struct __parsed_number * pn);
extern float __parsed_to_float(const struct __parsed_number * pn);
extern long double __parsed_to_long_double(const struct __parsed_number * pn);

/****************************************************************************/

#endif /* _STDLIB_STRTOD_PARSE_H */
