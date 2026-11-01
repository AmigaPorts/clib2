/*
 * Experimental: Not a part of clib2 from Olaf Barthel
 */

#include <ctype.h>
#include <fnmatch.h>
#include <string.h>

/****************************************************************************/

#define NOT_ESCAPED(flags)	(((flags) & FNM_NOESCAPE) == 0)
#define NOT_FOUND(found)	((found) == 0)

/****************************************************************************/

static int
fold(int c,int flags)
{
	c = (unsigned char)c;

	return((flags & FNM_CASEFOLD) ? tolower(c) : c);
}

/****************************************************************************/

static int
matches_class(const char * name,size_t length,int c)
{
	static const struct
	{
		const char *	name;
		int				(*test)(int c);
	}
	classes[] =
	{
		{ "alnum", isalnum }, { "alpha", isalpha }, { "blank", isblank },
		{ "cntrl", iscntrl }, { "digit", isdigit }, { "graph", isgraph },
		{ "lower", islower }, { "print", isprint }, { "punct", ispunct },
		{ "space", isspace }, { "upper", isupper }, { "xdigit", isxdigit }
	};

	size_t i;

	for(i = 0 ; i < sizeof(classes) / sizeof(classes[0]) ; i++)
	{
		if(strlen(classes[i].name) == length && strncmp(classes[i].name,name,length) == 0)
			return(classes[i].test(c));
	}

	return(0);
}

/****************************************************************************/

/* The character at *p in a bracket expression, which a backslash can make an
   ordinary one.  Returns -1 if the expression ends there. */
static int
bracket_character(const char ** p,int flags)
{
	if((**p) == '\\' && NOT_ESCAPED(flags))
		(*p)++;

	if((**p) == '\0')
		return(-1);

	return((unsigned char)*(*p)++);
}

/****************************************************************************/

/* Tests the character c against the bracket expression after the '[' at
   pattern.  Returns where the expression ends, or NULL if it doesn't, and
   the '[' is an ordinary character then. */
static const char *
scan_bracket(const char * pattern,int c,int flags,int * matched)
{
	const char * p = pattern;
	int is_negated = 0;
	int found = 0;
	int is_first = 1;

	if((*p) == '!' || (*p) == '^')
	{
		is_negated = 1;
		p++;
	}

	while(is_first || (*p) != ']')
	{
		int low;

		is_first = 0;

		if((*p) == '\0')
			return(NULL);

		/* A class, such as [:alpha:]. */
		if(p[0] == '[' && p[1] == ':')
		{
			const char * end = strstr(p + 2,":]");
			size_t length;

			if(end == NULL)
				return(NULL);

			length = (size_t)(end - (p + 2));

			if(matches_class(p + 2,length,(unsigned char)c))
				found = 1;
			else if ((flags & FNM_CASEFOLD) && length == 5 &&
			         (strncmp(p + 2,"upper",5) == 0 || strncmp(p + 2,"lower",5) == 0) &&
			         isalpha((unsigned char)c))
				found = 1;

			p = end + 2;
			continue;
		}

		low = bracket_character(&p,flags);
		if(low < 0)
			return(NULL);

		if(p[0] == '-' && p[1] != ']' && p[1] != '\0')
		{
			int high;

			p++;

			high = bracket_character(&p,flags);
			if(high < 0)
				return(NULL);

			if(fold(low,flags) <= fold(c,flags) && fold(c,flags) <= fold(high,flags))
				found = 1;
		}
		else if (fold(low,flags) == fold(c,flags))
		{
			found = 1;
		}
	}

	(*matched) = is_negated ? NOT_FOUND(found) : found;

	return(p + 1);
}

/****************************************************************************/

/* A backslash that would hide the only ']' that ends the expression is an
   ordinary character after all, as in "[?*\\]". */
static const char *
match_bracket(const char * pattern,int c,int flags,int * matched)
{
	const char * end = scan_bracket(pattern,c,flags,matched);

	if(end == NULL && NOT_ESCAPED(flags))
		end = scan_bracket(pattern,c,flags | FNM_NOESCAPE,matched);

	return(end);
}

/****************************************************************************/

/* A leading period is one at the start of the string or, with FNM_PATHNAME,
   after a '/'. */
static int
is_leading_period(const char * start,const char * s,int flags)
{
	if((*s) != '.' || (flags & FNM_PERIOD) == 0)
		return(0);

	return(s == start || ((flags & FNM_PATHNAME) && s[-1] == '/'));
}

/****************************************************************************/

/* The last '*' is the only one that ever has to take more characters: what
   an earlier one could still take, this one can take as well, and that keeps
   the time this takes proportional to the lengths of pattern and string. */
static int
match(const char * p,const char * start,const char * s,int flags)
{
	const char * star_p = NULL;
	const char * star_s = NULL;

	while(1)
	{
		int c = (*p);
		const char * end;
		int matched = 0;

		switch(c)
		{
			case '\0':

				if((*s) == '\0' || ((flags & FNM_LEADING_DIR) && (*s) == '/'))
					return(0);

				goto mismatch;

			case '?':

				if((*s) == '\0' || ((flags & FNM_PATHNAME) && (*s) == '/') || is_leading_period(start,s,flags))
					goto mismatch;

				p++;
				s++;
				continue;

			case '*':

				while((*p) == '*')
					p++;

				if(is_leading_period(start,s,flags))
					goto mismatch;

				star_p = p;
				star_s = s;
				continue;

			case '[':

				if((*s) == '\0')
					goto mismatch;

				end = match_bracket(p + 1,(*s),flags,&matched);
				if(end != NULL)
				{
					if(NOT_FOUND(matched) || ((flags & FNM_PATHNAME) && (*s) == '/') || is_leading_period(start,s,flags))
						goto mismatch;

					p = end;
					s++;
					continue;
				}

				break;

			case '\\':

				if(NOT_ESCAPED(flags))
				{
					if(p[1] != '\0')
						p++;
				}

				break;

			default:

				break;
		}

		if(fold(*p,flags) == fold(*s,flags) && (*s) != '\0')
		{
			p++;
			s++;
			continue;
		}

	 mismatch:

		if(star_p == NULL || (*star_s) == '\0' || ((flags & FNM_PATHNAME) && (*star_s) == '/'))
			return(FNM_NOMATCH);

		star_s++;

		p = star_p;
		s = star_s;
	}
}

/****************************************************************************/

int
fnmatch(const char * pattern,const char * string,int flags)
{
	return(match(pattern,string,string,flags));
}
