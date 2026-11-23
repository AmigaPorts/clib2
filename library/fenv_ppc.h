/*
 * The FPSCR of the PowerPC, for the fenv_*.c functions.  With SPE
 * (-mcpu=8548, the A1222's e500v2) the SPEFSCR stands in for it.
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

#if defined(__SPE__)

/****************************************************************************/

/* SPE has no FPSCR and no floating point registers: its rounding mode and
   sticky exception bits are in the SPEFSCR, SPR 512.  The fenv functions
   keep working on an FPSCR image, so that fenv_t and the FE_* constants
   stay the same, and these two translate.  The rounding modes are encoded
   the same, but the hardware rounds only to nearest and towards zero, so
   fesetround() refuses the other two.

   The SPEFSCR's enable bits are left alone.  They aren't the program's
   trap enables: they make the hardware raise the floating point
   interrupts that let the operating system handle denormals, infinities
   and NaNs (Linux sets four of them for every task), so clearing them,
   as feholdexcept() and fesetenv(FE_DFL_ENV) would, could switch that
   off. */
#define SPEFSCR_FINXS	0x00200000UL
#define SPEFSCR_FINVS	0x00100000UL
#define SPEFSCR_FDBZS	0x00080000UL
#define SPEFSCR_FUNFS	0x00040000UL
#define SPEFSCR_FOVFS	0x00020000UL
#define SPEFSCR_FRMC	0x00000003UL

/* The FPSCR bits each sticky exception bit of the SPEFSCR stands for. */
static const unsigned long __spefscr_map[][2] =
{
	{ SPEFSCR_FINXS, FE_INEXACT },
	{ SPEFSCR_FDBZS, FE_DIVBYZERO },
	{ SPEFSCR_FUNFS, FE_UNDERFLOW },
	{ SPEFSCR_FOVFS, FE_OVERFLOW },
	{ SPEFSCR_FINVS, FE_INVALID | FPSCR_VXSOFT },
};

#define SPEFSCR_FENV_BITS (SPEFSCR_FINXS | SPEFSCR_FINVS | SPEFSCR_FDBZS | \
	SPEFSCR_FUNFS | SPEFSCR_FOVFS | SPEFSCR_FRMC)

static __inline unsigned long
__fpu_get_fpscr(void)
{
	unsigned long spefscr, fpscr;
	size_t i;

	__asm __volatile ("mfspr %0,512" : "=r" (spefscr));

	fpscr = spefscr & SPEFSCR_FRMC;
	for(i = 0 ; i < sizeof(__spefscr_map) / sizeof(__spefscr_map[0]) ; i++)
	{
		if(spefscr & __spefscr_map[i][0])
			fpscr |= __spefscr_map[i][1];
	}

	return(fpscr);
}

/* Any of the invalid operation bits, or their summary, sets FINVS. */
static __inline void
__fpu_set_fpscr(unsigned long value)
{
	unsigned long spefscr;
	size_t i;

	if(value & FPSCR_INVALID_BITS)
		value |= FE_INVALID;

	__asm __volatile ("mfspr %0,512" : "=r" (spefscr));

	spefscr = (spefscr & ~SPEFSCR_FENV_BITS) | (value & SPEFSCR_FRMC);
	for(i = 0 ; i < sizeof(__spefscr_map) / sizeof(__spefscr_map[0]) ; i++)
	{
		if(value & __spefscr_map[i][1] & ~FPSCR_VXSOFT)
			spefscr |= __spefscr_map[i][0];
	}

	__asm __volatile ("mtspr 512,%0" : /* no outputs */ : "r" (spefscr));
}

/****************************************************************************/

#else

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

#endif /* __SPE__ */

/****************************************************************************/

#endif /* _FENV_PPC_H */
