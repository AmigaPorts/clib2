/*
 * The registers of the M68881 and its successors, for the fenv_*.c functions.
 */

#ifndef _FENV_68881_H
#define _FENV_68881_H

/****************************************************************************/

/* The enable bits of the FPCR, which trap on an exception. */
#define FPCR_ENABLE_BITS 0x0000ff00UL

/****************************************************************************/

static __inline unsigned long
__fpu_get_fpcr(void)
{
	unsigned long value;

	__asm __volatile ("fmove%.l fpcr,%0" : "=dm" (value) : /* no inputs */ );

	return(value);
}

static __inline void
__fpu_set_fpcr(unsigned long value)
{
	__asm __volatile ("fmove%.l %0,fpcr" : /* no outputs */ : "dmi" (value));
}

static __inline unsigned long
__fpu_get_fpsr(void)
{
	unsigned long value;

	__asm __volatile ("fmove%.l fpsr,%0" : "=dm" (value) : /* no inputs */ );

	return(value);
}

static __inline void
__fpu_set_fpsr(unsigned long value)
{
	__asm __volatile ("fmove%.l %0,fpsr" : /* no outputs */ : "dmi" (value));
}

static __inline unsigned long
__fpu_get_fpiar(void)
{
	unsigned long value;

	__asm __volatile ("fmove%.l fpiar,%0" : "=dm" (value) : /* no inputs */ );

	return(value);
}

static __inline void
__fpu_set_fpiar(unsigned long value)
{
	__asm __volatile ("fmove%.l %0,fpiar" : /* no outputs */ : "dmi" (value));
}

/****************************************************************************/

#endif /* _FENV_68881_H */
