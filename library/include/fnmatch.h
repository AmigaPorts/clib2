/*
 * Experimental: Not a part of clib2 from Olaf Barthel
 */

#ifndef _FNMATCH_H
#define _FNMATCH_H

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************/

#define FNM_NOMATCH		1

#define FNM_NOESCAPE	(1<<0)	/* A backslash is an ordinary character */
#define FNM_PATHNAME	(1<<1)	/* Only a '/' in the pattern matches a '/' */
#define FNM_PERIOD		(1<<2)	/* Only a '.' in the pattern matches a leading '.' */
#define FNM_LEADING_DIR	(1<<3)	/* The match may end before a '/' */
#define FNM_CASEFOLD	(1<<4)	/* Ignore the case of letters */

/****************************************************************************/

extern __stdargs int fnmatch(const char * pattern,const char * string,int flags);

/****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/****************************************************************************/

#endif /* _FNMATCH_H */
