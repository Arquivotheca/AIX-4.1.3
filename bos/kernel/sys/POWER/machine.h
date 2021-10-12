/* @(#)96	1.24  src/bos/kernel/sys/POWER/machine.h, sysml, bos411, 9428A410j 5/12/94 16:06:03 */
#ifndef _H_MACHINE
#define _H_MACHINE

/*
 * COMPONENT_NAME: (SYSML) Kernel Assembler
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27, 3
 *
 * (C) COPYRIGHT International Business Machines Corp. 1987, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *	     This include file defines mask bits and other special values
 *           for machine-dependent control registers, and other values
 *           specific to the platform.
 *
 *           This version is for R2.
 *
 *           NOTE:  A parallel assembler version of this .h file is in
 *                  .../sys/ml/machine.m4.
 *
 */

/*
 * Definitions for byte order,
 * according to byte significance from low address to high.
 */
#define	LITTLE_ENDIAN	1234	/* least-significant byte first (vax)	      */
#define	BIG_ENDIAN	4321	/* most-significant byte first (IBM, net)     */
#define	PDP_ENDIAN	3412	/* LSB first in word, MSW first in long (pdp) */

#define	BYTE_ORDER	BIG_ENDIAN

/*
 * Default value to place into a register when it isn't supposed
 *    to contain a useful value (killed registers on return from
 *    system call, for example)
 */

#define      DEFAULT_GPR   0xDEADBEEF

/*
 * Machine Status Register (MSR)
 */

#define      MSR_EE	   0x8000   /* External interrupt enable             */
#define      MSR_PR	   0x4000   /* Problem state                         */
#define      MSR_FP	   0x2000   /* Floating point available              */
#define      MSR_ME	   0x1000   /* Machine check enable                  */
#define      MSR_FE	   0x0800   /* Floating point exception enable (PWR) */
#define      MSR_FE0	   0x0800   /* Floating point xcp mode bit0    (PPC) */
#define      MSR_SE        0x0400   /* Single Step Trace Enable    (RS2/PPC) */
#define      MSR_BE        0x0200   /* Branch Trace Enable         (RS2/PPC) */
#define      MSR_IE        0x0100   /* Flt-pt imprecise int enable     (RS2) */
#define      MSR_FE1       0x0100   /* Floating point xcp mode bit1    (PPC) */
#define      MSR_AL	   0x0080   /* Alignment check enable          (PWR) */
#define      MSR_IP	   0x0040   /* Interrupt prefix active               */
#define      MSR_IR	   0x0020   /* Instruction relocate on               */
#define      MSR_DR	   0x0010   /* Data relocate on                      */
#define      MSR_PM        0x0004   /* Performance Monitoring          (RS2) */

#define      DEFAULT_MSR          (MSR_EE | MSR_ME | MSR_AL | MSR_IR | MSR_DR)
#define      DEFAULT_USER_MSR     (DEFAULT_MSR | MSR_PR)

/*
 * Condition Register (CR)
 */

#define      CR_LT	   0x80000000   /* Less Than,        field 0         */
#define      CR_GT	   0x40000000   /* Greater Than,     field 0         */
#define      CR_EQ	   0x20000000   /* Equal,            field 0         */
#define      CR_SO	   0x10000000   /* Summary Overflow, field 0         */
#define      CR_FX	   0x08000000   /* Floating point exception          */
#define      CR_FEX	   0x04000000   /* Floating point enabled exception  */
#define      CR_VX	   0x02000000   /* Floating point invalid operation  */
#define      CR_OX	   0x01000000   /* Copy of FPSCR(OX)                 */

/* Macro to access field n of CR value cr:                                   */

#define      CR_FIELD(n,cr)   ((cr << (n << 2)) & 0xF0000000)


/*
 * Fixed Point Exception Register (XER)
 */

#define      XER_SO	   0x80000000   /* Summary overflow                  */
#define      XER_OV	   0x40000000   /* Overflow                          */
#define      XER_CA	   0x20000000   /* Carry                             */

/* Macros to access comparison byte and length for lsx, lscbx, stsx          */

#define      XER_COMP_BYTE(xer)  ((xer >> 8) & 0x000000FF)
#define      XER_LENGTH(xer)     (xer & 0x0000007F)


/*
 * Data Storage Interrupt Status Register (DSISR)
 */

#define      DSISR_IO      0x80000000   /* I/O exception                     */
#define      DSISR_PFT     0x40000000   /* No valid PFT for page             */
#define      DSISR_LOCK    0x20000000   /* Access denied: data locking (PWR) */
#define      DSISR_FPIO    0x10000000   /* FP load/store to I/O space  (PWR) */
#define      DSISR_PROT    0x08000000   /* Protection exception              */
#define      DSISR_LOOP    0x04000000   /* PFT search > 127 entries    (RS1) */
#define      DSISR_DRST    0x04000000   /* lwarx, etc to Direct Store  (PPC) */
#define      DSISR_ST      0x02000000   /* 1 => store, 0 => load             */
#define      DSISR_SEGB    0x01000000   /* Crosses segment boundary,   (PWR) */
                                        /*   from T=0 to T=1                 */
#define      DSISR_DABR    0x00400000   /* DABR exception          (RS2/PPC) */
#define      DSISR_EAR     0x00100000   /* eciwx with EAR enable = 0   (PPC) */

/*
 * System Recall Register 1 (SRR1)
 *
 * Note:  This register is used for various purposes, depending on the
 *        type of interrupt, thus the prefix of the name will vary.
 */

#define      SRR_IS_PFT    0x40000000   /* No valid PFT for page             */
#define      SRR_IS_ISPEC  0x20000000   /* I-fetch from special segmnt (PWR) */
#define      SRR_IS_IIO    0x10000000   /* I-fetch from I/O space            */
#define      SRR_IS_GUARD  0x10000000   /* I-fetch from guarded storage      */
#define      SRR_IS_PROT   0x08000000   /* Protection exception              */
#define      SRR_IS_LOOP   0x04000000   /* PFT search > 127 entries    (RS1) */

#define      SRR_PR_FPEN   0x00100000   /* FP Enabled Interrupt exception    */
#define      SRR_PR_INVAL  0x00080000   /* Invalid operation                 */
#define      SRR_PR_PRIV   0x00040000   /* Privileged instruction            */
#define      SRR_PR_TRAP   0x00020000   /* Trap instruction                  */
#define      SRR_PR_IMPRE  0x00010000   /* Imprecise Interrupt         (PPC) */

#ifdef _KERNEL
/*
 * MACROS to determine the type of RS/6000 machine
 */
extern int mach_model;

#define MACH_RS1() (!(mach_model & 0x06000000))
#define	MACH_RSC() (mach_model & 0x02000000)

extern int fp_ie_impl;                  /* nonzero if ie implemented         */
#define FP_IE_IMPL (fp_ie_impl != 0)

#endif /* _KERNEL */

#ifdef _KERNSYS

#ifdef _POWER_PC

#ifdef _POWER_601

/*
 * The following macro can be used to generate a BUID 7f segment
 * register value given a real address
 */
#define BUID_7F_SRVAL(raddr) (0x87F00000 | (((uint)(raddr)) >> 28))

#endif /* _POWER_601 */

/*
 *	DBATU  Data BAT Upper
 *   +---------------------------------------------------------------+
 *   |     Effective  Address      | rsvd  |      Block Size     |V|V|
 *   |          BEPI               |       |          BL         |S|P|
 *   | | | | | | | | | | |1|1|1|1|1|1|1|1|1|1|2|2|2|2|2|2|2|2|2|2|3|3|
 *   |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
 *   +---------------------------------------------------------------+
 *
 *	DBATL  Data BAT Lower
 *   +---------------------------------------------------------------+
 *   |        Real Address         | rsvd              |W I M G| |P P|
 *   |          BRPN               |                   |       | |   |
 *   | | | | | | | | | | |1|1|1|1|1|1|1|1|1|1|2|2|2|2|2|2|2|2|2|2|3|3|
 *   |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
 *   +---------------------------------------------------------------+
 */

/*
 * The following defines are all valid values for the data BAT
 * BL field
 */
#define BT_256M		0x1FFC		/* 256 Meg block */
#define BT_128M		0x0FFC		/* 128 Meg block */
#define BT_64M		0x07FC		/* 64 Meg block */
#define BT_32M		0x03FC		/* 32 Meg block */
#define BT_16M		0x01FC		/* 16 Meg block */
#define BT_8M		0x00FC		/* 8 Meg block */
#define BT_4M		0x007C		/* 4 Meg block */
#define BT_2M		0x003C		/* 2 Meg block */
#define BT_1M		0x001C		/* 1 Meg block */
#define BT_512K		0x000C		/* 512 K block */
#define BT_256K		0x0004		/* 256 K block */
#define BT_128K		0x0000		/* 128 K block */

/*
 * The following defines are for data BAT PP (access protection)
 */
#define BT_NOACCESS	0x0		/* BAT no access */
#define BT_RDONLY	0x1		/* read only access */
#define BT_WRITE	0x2		/* read write access */

/*
 * The following defines are for the data BAT Vs Vp fields
 */
#define BT_VS		0x2		/* BAT valid in kernel mode */
#define BT_VP		0x1		/* BAT valid in user mode */

/*
 * The following values can be used to generate data BAT values
 *	raddr - real address
 *	size - BL field (use BT_XXX defines above)
 *	val - BAT (use BT_XXX defines above)
 *	eaddr - effective address to map
 *	wimg - storage attributes
 * 	pp - access protection (use BT_XXX defines above)
 */
#define DBATU(eaddr, size, val)	(((eaddr) & 0xFFFE0000) | (size) | (val))
#define DBATL(raddr, wimg, pp)	(((raddr) & 0xFFFE0000) | ((wimg) << 3) | (pp))

/*
 * This macro can returns the segment number that a data BAT maps
 */
#define BAT_ESEG(dbatu) (((uint)(dbatu) >> 28))

#define MIN_BAT_SIZE	0x00020000	/* Minimium BAT size (128K) */
#define MAX_BAT_SIZE	0x10000000	/* Maximium BAT size (256M) */
#endif /* _POWER_PC */

#endif /* _KERNSYS */


#endif /* _H_MACHINE */
