/*
 * Experimental: Not a part of clib2 from Olaf Barthel
 */

#ifndef _LANGINFO_H
#define _LANGINFO_H

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************/

typedef int nl_item;

#define CODESET		0
#define RADIXCHAR	1
#define THOUSEP		2

/****************************************************************************/

extern __stdargs char *nl_langinfo(nl_item item);

/****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/****************************************************************************/

#endif /* _LANGINFO_H */
