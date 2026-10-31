/*
 * $Id: math_ledf2.c,v 1.3 2006-01-08 12:04:23 obarthel Exp $
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
 */

#ifndef _MATH_HEADERS_H
#include "math_headers.h"
#endif /* _MATH_HEADERS_H */

/****************************************************************************/

#if defined(IEEE_FLOATING_POINT_SUPPORT) && defined(__GNUC__)

/****************************************************************************/

#if defined(SMALL_DATA)
#define A4(x) "a4@(" #x ":W)"
#elif defined(SMALL_DATA32)
#define A4(x) "a4@(" #x ":L)"
#else
#define A4(x) #x
#endif /* SMALL_DATA */

/****************************************************************************/

/* IEEEDPCmp() has no answer for a NaN, which compares unordered: answer 1
   here, which makes the comparison false. */
asm(

"	.text\n"
"	.even\n"

"	.globl	_MathIeeeDoubBasBase\n"
"	.globl	___ledf2\n"

"___ledf2:\n"

"	moveq	#1,d0\n"
"	movel	sp@(4),d1\n"
"	bclr	#31,d1\n"
"	cmpl	#0x7ff00000,d1\n"
"	bhis	9f\n"
"	bnes	1f\n"
"	tstl	sp@(8)\n"
"	bnes	9f\n"
"1:	movel	sp@(12),d1\n"
"	bclr	#31,d1\n"
"	cmpl	#0x7ff00000,d1\n"
"	bhis	9f\n"
"	bnes	2f\n"
"	tstl	sp@(16)\n"
"	bnes	9f\n"
"2:\n"

"	moveml	d2/d3/a6,sp@-\n"
"	movel	"A4(_MathIeeeDoubBasBase)",a6\n"
"	moveml	sp@(16),d0/d1/d2/d3\n"
"	jsr		a6@(-42:W)\n"
"	moveml	sp@+,d2/d3/a6\n"
"	rts\n"
"9:	rts\n"

);

/****************************************************************************/

#endif /* IEEE_FLOATING_POINT_SUPPORT */
