/* @(#)82	1.3  src/bos/kernel/ios/POWER/mpic.h, sysios, bos41J 5/2/95 16:53:46 */
#ifndef _h_MPIC
#define _h_MPIC
/*
 *   COMPONENT_NAME: SYSIOS
 *
 *   FUNCTIONS: Internal data structures
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * This file uses little-endian style bit ordering conventions in a word
 * because that is how the hardware design documents are written.  This
 * is opposite of the standard IBM bit ordering convention, and in this
 * file, the most significant bit of a 32-bit value is called bit 31.
 */

/****************************************************************************
 * Global registers
 */

struct feature0
{
	unsigned int			:  5;
	unsigned int		numIRQ	: 11;
	unsigned int			:  3;
	unsigned int		numCPU	:  5;
	unsigned int		version	:  8;
};

struct globalcfg0
{
	unsigned int		reset	:  1;
	unsigned int		mode	:  2;
	unsigned int			:  9;
	unsigned int		base	: 20;
};

struct vendorid
{
	unsigned int			:  8;
	unsigned int		stepping:  8;
	unsigned int		deviceid:  8;
	unsigned int		vendorid:  8;
};

struct ipivecpri
{
	unsigned int		mask	:  1;
	unsigned int		activity:  1;
	unsigned int			: 12;
	unsigned int		priority:  4;
	unsigned int			:  8;
	unsigned int		vector	:  8;
};

struct s_global
{
	unsigned int			feature0;

	unsigned char			pad0[0x1020-0x1004];

	unsigned int			globalcfg0;

	unsigned char			pad1[0x1080-0x1024];

	unsigned int			vendorid;

	unsigned char			pad2[0x1090-0x1084];

	unsigned int			procinit;

	unsigned char			pad3[0x10a0-0x1094];

	struct
	{
		unsigned int		ipivecpri;
		unsigned char		pad[0x0c];
	} ivp[4];
};

#define	MPIC_GMODE_PASSTHRU	(0x0)	/* 00= 8259 Pass-through */
#define	MPIC_GMODE_MIXED	(0x1)	/* 01= Mixed */
#define	MPIC_GMODE_RESERVED	(0x2)	/* 10= Reserved */
#define	MPIC_GMODE_ADVANCED	(0x3)	/* 11= Advanced, no 8259 */

/****************************************************************************
 * Timer registers
 */

struct timer_current
{
	unsigned int		toggle	:  1;
	unsigned int		current	: 31;
};

struct timer_base
{
	unsigned int		inhibit	:  1;
	unsigned int		base	: 31;
};

struct timer_vecpri
{
	unsigned int		mask	:  1;
	unsigned int		activity:  1;
	unsigned int			: 10;
	unsigned int		priority:  4;
	unsigned int			:  8;
	unsigned int		vector	:  8;
};

struct s_timer
{
	unsigned int			timer_current;		/* 0x00 */
	unsigned char			pad0[0x0c];

	unsigned int			timer_base;		/* 0x10 */
	unsigned char			pad1[0x0c];

	unsigned int			timer_vecpri;		/* 0x20 */
	unsigned char			pad2[0x0c];

	unsigned int			timer_dest;		/* 0x30 */
	unsigned char			pad3[0x0c];
};

/****************************************************************************
 * Interrupt source configuration registers
 */

struct vecpri
{
	unsigned int		mask	:  1;
	unsigned int		activity:  1;
	unsigned int			:  6;
	unsigned int		polarity:  1;
	unsigned int		sense	:  1;
	unsigned int			:  2;
	unsigned int		priority:  4;
	unsigned int			:  8;
	unsigned int		vector	:  8;
};

struct s_intsource
{
	unsigned int			vecpri;			/* 0x00 */
	unsigned char			pad0[0x0c];

	unsigned int			destination;		/* 0x10 */
	unsigned char			pad1[0x0c];
};

/* Source Configuration Register fields */
#define	MPIC_SENSE_EDGE		(0)	/* Edge Sensitive */
#define	MPIC_SENSE_LEVEL	(1)	/* Level Sensitive */
#define	MPIC_POLARITY_LOW	(0)	/* Active Low/Negative Edge */
#define	MPIC_POLARITY_HIGH	(1)	/* Active High/Positive Edge */
#define MPIC_MODE_DIRECTED	(0)	/* Steer to the CPU in dest. */
#define MPIC_MODE_DISTRIBUTED	(1)	/* Steer to ANY CPU in dest. */

/* MPIC Generic fields */
#define	MPIC_MASK_ENABLED	(0)	/* Interrupt source/timer enabled */
#define	MPIC_MASK_DISABLED	(1)	/* Interrupt source/timer disabled */
#define	MPIC_ACTIVITY_IDLE	(0)	/* Register not in use.  Read-Only */
#define	MPIC_ACTIVITY_INUSE	(1)	/* Register in use.  Read-Only */

#define	MPIC_MAX_PRI		(0xF)
#define	MPIC_PRI(pri)		((MPIC_MAX_PRI-(pri)) & MPIC_MAX_PRI)
#define	MPIC_VECT(i)		((i)+0x10)

/****************************************************************************
 * Per processor registers
 */

struct taskpri
{
	unsigned int			: 28;
	unsigned int		priority:  4;
};

struct intack
{
	unsigned int			: 24;
	unsigned int		vector  :  8;
};

struct s_procreg
{
	unsigned char			pad0[0x40-0x00];

	struct							/* 0x40 */
	{
		unsigned int		dispatch;

		unsigned char		pad[0x0c];
	} ipi[4];

	unsigned int			taskpri;		/* 0x80 */

	unsigned char			pad1[0xA0-0x84];

	unsigned int			intack;			/* 0xA0 */
	unsigned char			pad2[0x0c];

	unsigned int			eoi;			/* 0xB0 */

	unsigned char			pad3[0x1000-0xB4];
};

/****************************************************************************
 * Memory map of the MPIC controller
 */

struct s_mpic
{
	unsigned char		pad0[0x1000-0x0000];	/* 0x???00000 */

	struct s_global		global;			/* 0x???01000 */
	unsigned char		pad1[0x10F0-0x10E0];

	unsigned int		frequency;		/* 0x???010F0 */
	unsigned char		pad2[0x0C];

	struct s_timer		timer[4];		/* 0x???01100 */
	unsigned char		pad3[0x10000-0x01200];

	struct s_intsource	intsource[256];		/* 0x???10000 */
	unsigned char		pad4[0x20000-0x12000];

	struct s_procreg	procreg[32];		/* 0x???20000 */
};

/****************************************************************************
 */

/*
 * Where the MPIC controller, if present, resides.
 */
extern unsigned long		 mpic_base;

/*
 * These mc_funcs are largely identical to the production lbrx and stbrx
 * ones in inline.h, with two differences.  First, the reg_killed_by are
 * correct, which saves two registers in the store case.  Second, the
 * parameters are reversed on MpicWrite vs. stbrx, so that the address is
 * the first parameter to both.
 */

unsigned int	MpicRead( volatile unsigned int * );
void		MpicWrite( volatile unsigned int *, unsigned int );

#pragma	mc_func	MpicWrite	{ "7c801d2c" }		/* stbrx r4,0,r3 */
#pragma	mc_func	MpicRead	{ "7c601c2c" }		/* lbrx r3,0,r3 */

#pragma	reg_killed_by	MpicRead	gr3
#pragma	reg_killed_by	MpicWrite

#endif /* _h_MPIC */
