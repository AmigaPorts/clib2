/*
 * Tests of strtod(), strtof(), strtold(), their wide versions wcstod(),
 * wcstof() and wcstold(), and fegetround().
 *
 * The edge cases check what C99 asks of the string conversions: what they
 * accept, where they stop, and errno.  The accuracy test needs
 * strtod_vectors.h, made by
 *
 *	python3 gen_strtod_vectors.py strtod_vectors.h
 *
 * with numbers and their correctly rounded results.  strtod() and strtof()
 * have to get all of them exactly right; strtold() on m68k's 80-bit long
 * double, which is not correctly rounded, within MAX_LDBL_ULPS.  The wide
 * versions have to give the same results as the narrow ones.
 *
 * Build it with -lm and run it; it prints the failures and exits with 0
 * if there are none, 1 otherwise.
 */

#include <errno.h>
#include <fenv.h>
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "strtod_vectors.h"

#define MAX_LDBL_ULPS 5.0

/* m68k's 80-bit extended long double */
#define WIDE_LONG_DOUBLE (LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384)

static int failures;

static void
fail(const char *what, const char *s, int end, int error)
{
	printf("FAIL %s(\"%s\"): end %d, errno %d\n", what, s, end, error);
	failures++;
}

/* Is the result what was expected, also when that is a NaN or a zero
   with a sign? */
static int
same_double(double got, double expect)
{
	if(__builtin_isnan(expect))
		return(__builtin_isnan(got));

	return(got == expect && signbit(got) == signbit(expect));
}

static void
check_strtod(const char *s, double expect, int end, int expect_errno)
{
	char *e;
	double d;

	errno = 0;
	d = strtod(s, &e);

	if(!(same_double(d, expect) && e - s == end && errno == expect_errno))
		fail("strtod", s, (int)(e - s), errno);
}

static void
check_strtof(const char *s, float expect, int end, int expect_errno)
{
	char *e;
	float f;

	errno = 0;
	f = strtof(s, &e);

	if(!(same_double(f, expect) && e - s == end && errno == expect_errno))
		fail("strtof", s, (int)(e - s), errno);
}

/* The wide string of the ASCII string s */
static const wchar_t *
widen(const char * s)
{
	static wchar_t w[256];
	size_t i;

	for(i = 0 ; s[i] != '\0' && i < sizeof(w) / sizeof(w[0]) - 1 ; i++)
		w[i] = (unsigned char)s[i];

	w[i] = L'\0';

	return(w);
}

static void
check_wcstod(const char *s, double expect, int end, int expect_errno)
{
	const wchar_t * w = widen(s);
	wchar_t *e;
	double d;

	errno = 0;
	d = wcstod(w, &e);

	if(!(same_double(d, expect) && e - w == end && errno == expect_errno))
		fail("wcstod", s, (int)(e - w), errno);
}

static void
check_wcstof(const char *s, float expect, int end, int expect_errno)
{
	const wchar_t * w = widen(s);
	wchar_t *e;
	float f;

	errno = 0;
	f = wcstof(w, &e);

	if(!(same_double(f, expect) && e - w == end && errno == expect_errno))
		fail("wcstof", s, (int)(e - w), errno);
}

static void
check_wcstold(const char *s, long double expect, int end, int expect_errno)
{
	const wchar_t * w = widen(s);
	wchar_t *e;
	long double l;

	errno = 0;
	l = wcstold(w, &e);

	if(!((l == expect || (__builtin_isnan(l) && __builtin_isnan(expect))) && e - w == end && errno == expect_errno))
		fail("wcstold", s, (int)(e - w), errno);
}

#if WIDE_LONG_DOUBLE

/* |got - expect| in units of the last place of expect */
static double
ulps(long double got, long double expect)
{
	long double difference, scale;

	if(got == expect)
		return(0.0);

	difference = got > expect ? got - expect : expect - got;
	scale = expect < 0 ? -expect : expect;

	if(scale == 0)
		return(1e9);

	return((double)(difference / scale / LDBL_EPSILON));
}

static void
check_strtold(const char *s, long double expect, int end, int expect_errno, double tolerance)
{
	char *e;
	long double l;
	int ok;

	errno = 0;
	l = strtold(s, &e);

	if(__builtin_isnan(expect))
		ok = __builtin_isnan(l);
	else
		ok = ulps(l, expect) <= tolerance;

	if(!(ok && e - s == end && errno == expect_errno))
		fail("strtold", s, (int)(e - s), errno);
}

#endif /* WIDE_LONG_DOUBLE */

static void
edge_cases(void)
{
	check_strtod("1.5", 1.5, 3, 0);
	check_strtod("  +12.5e-1xyz", 1.25, 10, 0);
	check_strtod(" -0.0", -0.0, 5, 0);
	check_strtod(".5", 0.5, 2, 0);
	check_strtod("5.", 5.0, 2, 0);
	check_strtod(".", 0.0, 0, 0);
	check_strtod("  -", 0.0, 0, 0);
	check_strtod("x", 0.0, 0, 0);
	check_strtod("1e", 1.0, 1, 0);
	check_strtod("1e+", 1.0, 1, 0);
	check_strtod("0x1p4", 16.0, 5, 0);
	check_strtod("0x1.8p+1x", 3.0, 8, 0);
	check_strtod("0XAp-2", 2.5, 6, 0);
	check_strtod("0x", 0.0, 1, 0);
	check_strtod("inf", INFINITY, 3, 0);
	check_strtod("-Infinity!", -INFINITY, 9, 0);
	check_strtod("inf rest", INFINITY, 3, 0);
	check_strtod("nan(abc)", NAN, 8, 0);
	check_strtod("nan(12", NAN, 3, 0);
	check_strtod("1.7976931348623157e308", DBL_MAX, 22, 0);
	check_strtod("1.7976931348623158e308", DBL_MAX, 22, 0);
	check_strtod("4.9406564584124654e-324", 4.9406564584124654e-324, 23, 0);

	/* clib2's HUGE_VAL and HUGE_VALF are the largest double and float,
	   not infinity, as its math functions return them too. */
	check_strtod("1e309", HUGE_VAL, 5, ERANGE);
	check_strtod("-1e309", -HUGE_VAL, 6, ERANGE);
	check_strtod("1e-400", 0.0, 6, ERANGE);

	check_strtof("3.4028235e38", FLT_MAX, 12, 0);
	check_strtof("0x1p-149", 0x1p-149f, 8, 0);
	check_strtof("1e39", HUGE_VALF, 4, ERANGE);
	check_strtof("1e-50", 0.0f, 5, ERANGE);

	#if WIDE_LONG_DOUBLE
	{
		check_strtold("0.1", 0.1L, 3, 0, 1);
		check_strtold("3.14159265358979323846264338327950288", 3.14159265358979323846264338327950288L, 37, 0, 2);
		check_strtold("123456789012345678901234567890", 123456789012345678901234567890.0L, 30, 0, 1);
		check_strtold("1.00000000000000000001", 1.00000000000000000001L, 22, 0, 1);
		check_strtold("1.18973149535723176502e+4932", LDBL_MAX, 28, 0, 2);
		check_strtold("1.68105157155604675313e-4932", LDBL_MIN, 28, 0, 2);
		check_strtold("1.2e4932", INFINITY, 8, ERANGE, 0);
		check_strtold("1e-5000", 0.0L, 7, ERANGE, 0);
		check_strtold("0x1.8p1", 3.0L, 7, 0, 0);
		check_strtold("0x1p16384", INFINITY, 9, ERANGE, 0);
		check_strtold("nan(123)x", NAN, 8, 0, 0);
		check_strtold(".", 0.0L, 0, 0, 0);
	}
	#endif /* WIDE_LONG_DOUBLE */

	check_wcstod("  -1.5e3xyz", -1500.0, 8, 0);
	check_wcstod("\t\n 0x1p4", 16.0, 8, 0);
	check_wcstod("  x", 0.0, 0, 0);
	check_wcstod("", 0.0, 0, 0);
	check_wcstod("-Infinity!", -INFINITY, 9, 0);
	check_wcstod("nan(abc)", NAN, 8, 0);
	check_wcstod("1e", 1.0, 1, 0);
	check_wcstod("1e309", HUGE_VAL, 5, ERANGE);
	check_wcstod("4.9406564584124654e-324", 4.9406564584124654e-324, 23, 0);
	check_wcstof("3.4028235e38", FLT_MAX, 12, 0);
	check_wcstof(" 1e39", HUGE_VALF, 5, ERANGE);
	check_wcstold("0x1.8p1", 3.0L, 7, 0);
	check_wcstold(" -0.5", -0.5L, 5, 0);
	check_wcstold("nan", NAN, 3, 0);

	/* More digits than fit in wcstod()'s buffer on the stack */
	{
		char s[128];

		memset(s, '0', sizeof(s));
		s[0] = '1';
		strcpy(&s[101], "e-100;");

		check_wcstod(s, 1.0, 106, 0);
	}

	/* A wide character beyond Latin 1 ends the number. */
	{
		static const wchar_t w[] = { '2', '.', '5', 0x20AC, '1', 0 };
		wchar_t *e;
		double d = wcstod(w, &e);

		if(d != 2.5 || e != w + 3)
			fail("wcstod", "2.5\\u20ac1", (int)(e - w), errno);
	}

	if(fegetround() != FE_TONEAREST)
	{
		printf("FAIL fegetround() is %d, not FE_TONEAREST (%d)\n", fegetround(), FE_TONEAREST);
		failures++;
	}
}

static uint64_t
double_bits(double d)
{
	uint64_t u;

	memcpy(&u, &d, sizeof(u));

	return(u);
}

static uint32_t
float_bits(float f)
{
	uint32_t u;

	memcpy(&u, &f, sizeof(u));

	return(u);
}

static void
accuracy(void)
{
	unsigned i, n = NUM_VECS, double_exact = 0, float_exact = 0, wide_same = 0, shown = 0;
	#if WIDE_LONG_DOUBLE
	double worst_ldbl = 0;
	#endif /* WIDE_LONG_DOUBLE */

	for(i = 0 ; i < n ; i++)
	{
		const struct vec * v = &vecs[i];
		uint64_t d = double_bits(strtod(v->s, NULL));
		uint32_t f = float_bits(strtof(v->s, NULL));
		const wchar_t * w = widen(v->s);
		long double l = strtold(v->s, NULL), wl = wcstold(w, NULL);

		if(double_bits(wcstod(w, NULL)) == d && float_bits(wcstof(w, NULL)) == f && (wl == l || (__builtin_isnan(wl) && __builtin_isnan(l))))
			wide_same++;
		else if(shown++ < 10)
			printf("FAIL wcstod(), wcstof() or wcstold() differ for \"%s\"\n", v->s);

		/* Overflow gives HUGE_VAL and HUGE_VALF, not infinity. */
		if((v->d & ~(1ULL << 63)) == 0x7FF0000000000000ULL && (d & ~(1ULL << 63)) == double_bits(HUGE_VAL))
			d = v->d;

		if((v->f & ~(1UL << 31)) == 0x7F800000UL && (f & ~(1UL << 31)) == float_bits(HUGE_VALF))
			f = v->f;

		if(d == v->d)
			double_exact++;
		else if(shown++ < 10)
			printf("FAIL strtod(\"%s\") is not correctly rounded\n", v->s);

		if(f == v->f)
			float_exact++;
		else if(shown++ < 10)
			printf("FAIL strtof(\"%s\") is not correctly rounded\n", v->s);

		#if WIDE_LONG_DOUBLE
		{
			if(v->has_ld)
			{
				long double expect, got = strtold(v->s, NULL);
				double u;

				memcpy(&expect, v->ld, sizeof(expect));

				u = ulps(got, expect);
				if(u > worst_ldbl)
					worst_ldbl = u;
			}
		}
		#endif /* WIDE_LONG_DOUBLE */
	}

	printf("strtod:  %u of %u correctly rounded\n", double_exact, n);
	printf("strtof:  %u of %u correctly rounded\n", float_exact, n);

	printf("wide:    %u of %u the same\n", wide_same, n);

	if(double_exact != n || float_exact != n || wide_same != n)
		failures++;

	#if WIDE_LONG_DOUBLE
	{
		printf("strtold: at most %.2f units in the last place off\n", worst_ldbl);

		if(worst_ldbl > MAX_LDBL_ULPS)
			failures++;
	}
	#endif /* WIDE_LONG_DOUBLE */
}

int
main(void)
{
	edge_cases();
	accuracy();

	printf("%d failures\n", failures);

	return(failures == 0 ? 0 : 1);
}
