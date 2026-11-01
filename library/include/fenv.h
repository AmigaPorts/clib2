/*
 * $Id: fenv.h,v 1.4 2006-01-08 12:06:14 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2015 by Olaf Barthel <obarthel (at) gmx.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   - Neither the name of Olaf Barthel nor the names of contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************
 *
 * Documentation and source code for this library, and the most recent library
 * build are available from <http://sourceforge.net/projects/clib2>.
 *
 *****************************************************************************
 */

#ifndef _FENV_H
#define _FENV_H

/****************************************************************************/

#ifndef _SYS_CLIB2_STDC_H
#include <sys/clib2_stdc.h>
#endif /* _SYS_CLIB2_STDC_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard, but it should
   be part of ISO/IEC 9899:1999, also known as "C99". */

/****************************************************************************/

#ifndef _STDDEF_H
#include <stddef.h>
#endif /* _STDDEF_H */

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************/

typedef unsigned int fexcept_t;

/****************************************************************************/

#if defined(__HAVE_68881__)

/* The accrued exception byte of the FPSR. */
#define FE_INEXACT		(1<<3)
#define FE_DIVBYZERO	(1<<4)
#define FE_UNDERFLOW	(1<<5)
#define FE_OVERFLOW		(1<<6)
#define FE_INVALID		(1<<7)
#define FE_ALL_EXCEPT	(FE_DIVBYZERO|FE_INEXACT|FE_INVALID|FE_OVERFLOW|FE_UNDERFLOW)

/* The rounding mode field of the FPCR. */
#define FE_TONEAREST	0
#define FE_TOWARDZERO	(1<<4)
#define FE_DOWNWARD		(2<<4)
#define FE_UPWARD		(3<<4)

#elif defined(__amigaos4__) && !defined(_SOFT_FLOAT)

/* The exception bits of the FPSCR. */
#define FE_INEXACT		(1<<25)
#define FE_DIVBYZERO	(1<<26)
#define FE_UNDERFLOW	(1<<27)
#define FE_OVERFLOW		(1<<28)
#define FE_INVALID		(1<<29)
#define FE_ALL_EXCEPT	(FE_DIVBYZERO|FE_INEXACT|FE_INVALID|FE_OVERFLOW|FE_UNDERFLOW)

/* The RN field of the FPSCR. */
#define FE_TONEAREST	0
#define FE_TOWARDZERO	1
#define FE_UPWARD		2
#define FE_DOWNWARD		3

#else

/* Without an FPU there are no exception flags and only one rounding mode,
   and C99 says that the macros of what isn't supported are not defined. */
#define FE_ALL_EXCEPT	0
#define FE_TONEAREST	0

#endif /* __HAVE_68881__ */

/****************************************************************************/

#if defined(__amigaos4__)

typedef int fenv_t;

#else

/* The same size with and without -m68881: code built without it can link with
   the libm that fegetenv() writes all of this in. */
typedef struct
{
	unsigned long __fpcr;
	unsigned long __fpsr;
	unsigned long __fpiar;
} fenv_t;

#endif /* __amigaos4__ */

/****************************************************************************/

#define FE_DFL_ENV ((const fenv_t *)-1)

/****************************************************************************/

extern __stdargs int fegetenv(fenv_t * envp);
extern __stdargs int fesetenv(const fenv_t * envp);
extern __stdargs int feholdexcept(fenv_t * envp);
extern __stdargs int feupdateenv(const fenv_t * envp);

/****************************************************************************/

extern __stdargs int fegetexceptflag(fexcept_t *flagp,int excepts);
extern __stdargs int fesetexceptflag(const fexcept_t *flagp,int excepts);
extern __stdargs int fetestexcept(int excepts);
extern __stdargs int feraiseexcept(int excepts);
extern __stdargs int feclearexcept(int excepts);

/****************************************************************************/

extern __stdargs int fegetround(void);
extern __stdargs int fesetround(int mode);
extern __stdargs int fetestround(int rounds);

/****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/****************************************************************************/

#endif /* _FENV_H */
