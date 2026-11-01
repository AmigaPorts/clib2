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
#define YESEXPR		3
#define NOEXPR		4
#define CRNCYSTR	5
#define D_T_FMT		6
#define D_FMT		7
#define T_FMT		8
#define T_FMT_AMPM	9
#define AM_STR		10
#define PM_STR		11
#define ERA			12
#define ERA_D_T_FMT	13
#define ERA_D_FMT	14
#define ERA_T_FMT	15
#define ALT_DIGITS	16
#define DAY_1		17
#define DAY_2		18
#define DAY_3		19
#define DAY_4		20
#define DAY_5		21
#define DAY_6		22
#define DAY_7		23
#define ABDAY_1		24
#define ABDAY_2		25
#define ABDAY_3		26
#define ABDAY_4		27
#define ABDAY_5		28
#define ABDAY_6		29
#define ABDAY_7		30
#define MON_1		31
#define MON_2		32
#define MON_3		33
#define MON_4		34
#define MON_5		35
#define MON_6		36
#define MON_7		37
#define MON_8		38
#define MON_9		39
#define MON_10		40
#define MON_11		41
#define MON_12		42
#define ABMON_1		43
#define ABMON_2		44
#define ABMON_3		45
#define ABMON_4		46
#define ABMON_5		47
#define ABMON_6		48
#define ABMON_7		49
#define ABMON_8		50
#define ABMON_9		51
#define ABMON_10	52
#define ABMON_11	53
#define ABMON_12	54

/****************************************************************************/

extern __stdargs char *nl_langinfo(nl_item item);

/****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/****************************************************************************/

#endif /* _LANGINFO_H */
