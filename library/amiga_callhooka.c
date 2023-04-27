/*
 * $Id: amiga_callhooka.c,v 1.3 2006-01-08 12:04:22 obarthel Exp $
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

#if (defined(__GNUC__) && !defined(__PPC__))

/****************************************************************************/

asm(
"h_Entry = 8\n"

"	.text\n"
"	.even\n"
"\n"
"|---------------------------------------------------------------------------\n"
"| new hook standard\n"
"| use struct Hook (with minnode at the top)\n"
"|\n"
"| *** register calling convention: ***\n"
"|	A0 - pointer to hook itself\n"
"|	A1 - pointer to parameter packed ('message')\n"
"|	A2 - Hook specific address data ('object,' e.g, gadget )\n"
"|\n"
"| ***  C conventions: ***\n"
"| Note that parameters are in unusual register order: a0, a2, a1.\n"
"| This is to provide a performance boost for assembly language\n"
"| programming (the object in a2 is most frequently untouched).\n"
"| It is also no problem in 'register direct' C function parameters.\n"
"|\n"
"| calling through a hook\n"
"|	CallHook( hook, object, msgid, p1, p2, ... );\n"
"|	CallHookA( hook, object, msgpkt );\n"
"|\n"
"| using a C function:	CFunction( hook, object, message );\n"
"|	hook.h_Entry = HookEntry;\n"
"|	hook.h_SubEntry = CFunction;\n"
"|\n"
"|---------------------------------------------------------------------------\n"

"| C calling hook interface for prepared message packet\n"

"	.globl	_CallHookA\n"

"_CallHookA:\n"

"	moveml	a2/a6,sp@-\n"
"	moveal	sp@(12),a0\n"
"	moveal	sp@(16),a2\n"
"	moveal	sp@(20),a1\n"
"	pea		callhooka_return\n"
"	movel	a0@(h_Entry),sp@-\n"
"	rts\n"

"callhooka_return:\n"

"	moveml	sp@+,a2/a6\n"
"	rts\n"
);

/****************************************************************************/

#endif /* __GNUC__ && !__PPC__ */
