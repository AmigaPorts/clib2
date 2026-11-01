/*
 * The FPSCR of the PowerPC, for the fenv_*.c functions.
 */

#ifndef _FENV_PPC_H
#define _FENV_PPC_H

/****************************************************************************/

/* The invalid operation bits of the FPSCR, which FE_INVALID is the summary of,
   and the one of them that software may set. */
#define FPSCR_INVALID_BITS	0x01f80700UL
#define FPSCR_VXSOFT		0x00000400UL

/* Everything feholdexcept() has to clear. */
#define FPSCR_EXCEPT_BITS	(FPSCR_INVALID_BITS | (unsigned long)FE_ALL_EXCEPT)

/* The enable bits, which trap on an exception, and the rounding mode. */
#define FPSCR_ENABLE_BITS	0x000000f8UL
#define FPSCR_RN_BITS		0x00000003UL

/****************************************************************************/

/* The FPSCR only moves to and from the low word of a floating point register. */
union __fpscr_image
{
	double			d;
	unsigned long	w[2];
};

static __inline unsigned long
__fpu_get_fpscr(void)
{
	union __fpscr_image u;

	__asm __volatile ("mffs %0" : "=f" (u.d) : /* no inputs */ );

	return(u.w[1]);
}

static __inline void
__fpu_set_fpscr(unsigned long value)
{
	union __fpscr_image u;

	u.w[0] = 0;
	u.w[1] = value;

	__asm __volatile ("mtfsf 0xff,%0" : /* no outputs */ : "f" (u.d));
}

/****************************************************************************/

#endif /* _FENV_PPC_H */
