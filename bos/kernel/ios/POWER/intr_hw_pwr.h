/* @(#)63	1.1  src/bos/kernel/ios/POWER/intr_hw_pwr.h, sysios, bos411, 9428A410j 3/16/93 20:06:18 */
#ifndef _h_INTR_HW_PWR
#define _h_INTR_HW_PWR
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS: Internal machine dependent macros and labels used by
 *	      the interrupt management services for POWER.
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifdef _POWER_RS2
/* 
 * The following define the processor level to start allocating from for
 * the given device class.  CLASS0 & 2 allocate numerically higher levels
 * while CLASS1 & 3 allocate numerically lower levels.
 */
#define	STARTCLASS0		4
#define	STARTCLASS1		28
#define	STARTCLASS2		33
#define	STARTCLASS3		57

/*
 * BASECLASS indicate where the actual base for a pool begins 
 * accounting for any reserved levels in a pool.
 *	Processor level reservation
 *		3 	RS2	INT_GIO
 *		32	RS2	INT_GIO_CLS2-1
 *		58	RS2	INT_GIO_CLS3-2
 *		59	RS2	INT_GIO_CLS3-1
 *		60	RS2	INT_GIO_CLS3-X  Reserved see m_intr.h
 *		61	RS2	INT_GIO_CLS3-X  Reserved see m_intr.h
 */
#define	BASECLASS0		3
#define	BASECLASS1		STARTCLASS1
#define	BASECLASS2		32
#define	BASECLASS3		59

/* 
 * The following define where the CIL array values are initialized for
 * each interrupt priority
 */
#define PRI0_TO_LVL		0
#define PRI1_TO_LVL		BASECLASS0
#define PRI2_TO_LVL		BASECLASS1
#define PRI3_TO_LVL		BASECLASS2
#define PRI4_TO_LVL		BASECLASS3
#define PRI5_TO_LVL		62
#define PRI6_TO_LVL		63
#define PRI7_TO_LVL		PRI6_TO_LVL
#define PRI8_TO_LVL		PRI6_TO_LVL
#define PRI9_TO_LVL		PRI6_TO_LVL
#define PRI10_TO_LVL		PRI6_TO_LVL
#define PRI11_TO_LVL		255

#endif /* _POWER_RS2 */

/*
 * Number of interrupt levels per word of EIS/EIM or PEIS.
 */
#define LVL_PER_WORD 32

/* Gets an IOCC number out of the given bid and shift it to make an index */
#define GET_IOCC_NUM( bid )	((0x00F00000 & bid) >> 20)

/* Masks an IOCC number out of the given bid */
#define IOCC_NUM_MSK( bid )	(0x00F00000 & bid)

/*
 *      Creates a virtual memory handle.
 *      Preserve the given buid and set up for I/O address space,
 *      address check and increment, IOCC select, 24-bit mode and
 *      bypass mode.
 */
#define BUS_TO_IOCC(bid)    ((0x1FF00000 & bid) | 0x800C00E0 )

/*
 * The eis/peis parameter is the external interrupt source. It contains
 * one bit for each interrupt level. The index of a bit in the eis 
 * is the interrupt level that corresponds to that bit. A bit value
 * of 0 indicates that the interrupt is not pending and a bit value
 * of 1 indicates that the interrupt is pending.
 * 
 * The eim, RS1/RSC only, parameter is the external interrupt mask. It contains
 * one bit for each interrupt level. The index of a bit in the eim 
 * is the interrupt level that corresponds to that bit. A bit value
 * of 0 indicates that the interrupt is disabled and a bit value
 * of 1 indicates that the interrupt is enabled.
 *
 * On RS2 type machines the CIL register is used control which interrupt
 * levels are enabled.
 */

/*
 * LVL_TO_MASK(mask, intlvl);
 *
 * Given an interrupt level, generate a mask with the bit in the eis/eim,
 * corresponding to it, set.
 *
 * The interrupt level here is an already normalized interrupt level.
 * The caller has already translated it.
 */
#define LVL_TO_MASK(mask, intlvl)					\
{									\
	ASSERT(((intlvl) >= 0) && ((intlvl) < 2*LVL_PER_WORD));		\
	if ((intlvl) < LVL_PER_WORD)					\
	{								\
		(mask).eim0 = (ulong)((ulong)(0x80000000) >>		\
							(int)(intlvl)); \
		(mask).eim1 = 0;					\
	}								\
	else								\
	{								\
		(mask).eim0 = 0;					\
		(mask).eim1 = (ulong)((ulong)(0x80000000) >>		\
					(int)((intlvl)-LVL_PER_WORD));	\
	}								\
}

/*
 * LVL_PER_SYS is the maximum number of POWER interrupt levels.  It
 * is used for sanity checks
 */
#define LVL_PER_SYS	64

#endif /* _h_INTR_HW_PWR */
