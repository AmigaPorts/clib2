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
	unsigned long long	pn_mantissa;	/* the significant digits, at most
										   PN_MAX_DECIMAL_DIGITS or
										   PN_MAX_HEX_DIGITS of them */
	long				pn_exponent;	/* of 10, or of 2 if pn_is_hex */
	int					pn_is_hex;
};

#define PN_NONE		0	/* not a number; nothing was converted */
#define PN_NUMBER	1
#define PN_INFINITY	2
#define PN_NAN		3

/* An unsigned long long holds this many digits exactly. */
#define PN_MAX_DECIMAL_DIGITS	19
#define PN_MAX_HEX_DIGITS		16

/* Parse the number str begins with, as strtod() does, into *pn.  Returns
   where it ends, or str itself if there is no number. */
extern const char * __parse_number(const char * str,struct __parsed_number * pn);

/* What strtod() returns for it, with errno set to ERANGE on overflow and
   underflow. */
extern double __parsed_to_double(const struct __parsed_number * pn);

/* Where long double is wider than double: the magnitude of a PN_NUMBER as
   a long double, infinity on overflow and 0 on underflow. */
extern long double __parsed_to_long_double(const struct __parsed_number * pn);

/****************************************************************************/

#endif /* _STDLIB_STRTOD_PARSE_H */
