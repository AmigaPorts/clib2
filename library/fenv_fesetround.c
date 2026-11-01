/*
 * Experimental: Not a part of clib2 from Olaf Barthel
 */

#ifndef _MATH_HEADERS_H
#include "math_headers.h"
#endif /* _MATH_HEADERS_H */

/****************************************************************************/

#if defined(M68881_FLOATING_POINT_SUPPORT)
#include "fenv_68881.h"
#endif /* M68881_FLOATING_POINT_SUPPORT */

/****************************************************************************/

#if defined(__amigaos4__) && !defined(_SOFT_FLOAT)
#include "fenv_ppc.h"
#endif /* __amigaos4__ && !_SOFT_FLOAT */

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT)

/****************************************************************************/

#if defined(M68881_FLOATING_POINT_SUPPORT)

/****************************************************************************/

int
fesetround(int mode)
{
	if((mode & ~FE_UPWARD) != 0)
		return(-1);

	__fpu_set_fpcr((__fpu_get_fpcr() & ~(unsigned long)FE_UPWARD) | (unsigned long)mode);

	return(0);
}

/****************************************************************************/

#elif defined(__amigaos4__) && !defined(_SOFT_FLOAT)

/****************************************************************************/

int
fesetround(int mode)
{
	if((mode & ~FPSCR_RN_BITS) != 0)
		return(-1);

	__fpu_set_fpscr((__fpu_get_fpscr() & ~FPSCR_RN_BITS) | (unsigned long)mode);

	return(0);
}

#else

/****************************************************************************/

int
fesetround(int mode)
{
    // We always round towards the nearest number. It's not possible to set
    // rounding mode but we can pretend if the caller wants to set the mode
    // that we're using anyway. By doing so we also inform the caller that
    // it's not possible to set another mode.
    return mode == FE_TONEAREST ? 0 : -1;
}

/****************************************************************************/

#endif /* M68881_FLOATING_POINT_SUPPORT */

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
