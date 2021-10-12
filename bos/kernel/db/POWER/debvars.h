/* @(#)97	1.14  src/bos/kernel/db/POWER/debvars.h, sysdb, bos411, 9428A410j 3/28/94 17:56:50 */
#ifndef _h_DEBVARS
#define _h_DEBVARS

/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*
 * Debugger variables' data area.
 *
 * This defines the area used for debugger variables.
 * This is an array of structures.
 * Each struct represents a possible instance of a variable
 * and defines the data for the variable.
 *
 * It is no accident that it looks like the parser struct.
 */

#include <sys/mstsave.h>		/* for NUM_KERNEL_BATS */

/* variables for the POWER and POWERPC environment */
#define NOT_A_VAR -1			/* invalid varidx, not a var    */
#define MAXDVARS 75
#define IDUVARS  0			/* First user-defined var	*/
#define NUMUVARS 16			/*   # vars.			*/
#define IDFX	IDUVARS+NUMUVARS	/* FX's id, (find's var)	*/
#define IDORG	IDFX+1			/* origin's id			*/
#define IDSEGS	IDORG+1			/* segment regs			*/
#define NUMSEGS 16
#define IDGPRS  IDSEGS+NUMSEGS		/* GPRs				*/
#define NUMGPRS 32
#define IDFPRS	IDGPRS+NUMGPRS		/* Floating point regs		*/
#define NUMFPRS 32
#define IDBATU  IDFPRS+NUMFPRS		/* Upper BAT registers */
#define IDBATL  IDBATU+NUM_KERNEL_BATS	/* Lower BAT registers */
#define IDIAR	IDBATL+NUM_KERNEL_BATS
#define IDMQ	IDIAR+1
#define IDMSR	IDMQ+1
#define IDCR	IDMSR+1
#define IDLR	IDCR+1
#define IDCTR	IDLR+1
#define IDTID	IDCTR+1
#define IDXER	IDTID+1
#define IDFPSCR IDXER+1
#define IDSRR0	IDFPSCR+1
#define IDSRR1	IDSRR0+1
#define IDDSISR	IDSRR1+1
#define IDDAR	IDDSISR+1
#define IDEIM0	IDDAR+1
#define IDEIM1	IDEIM0+1
#define IDEIS0	IDEIM1+1
#define IDEIS1	IDEIS0+1
#define IDPEIS0	IDEIS1+1
#define IDPEIS1	IDPEIS0+1
#define IDILCR	IDPEIS1+1
#define IDXIRR	IDILCR+1
#define IDCPPR	IDXIRR+1
#define IDDSIER	IDCPPR+1
#define IDSDR0	IDDSIER+1
#define IDSDR1	IDSDR0+1
#define IDRTCU	IDSDR1+1
#define IDRTCL 	IDRTCU+1
#define IDTBU 	IDRTCL+1
#define IDTBL 	IDTBU+1
#define IDDEC	IDTBL+1

#define MAXVARS IDDEC+1		/* MUST BE LAST IN ID LIST */


#define T_BIT	(debvars[IDMSR].hv & 0x0020)
#define INSTT_BIT	(debvars[IDMSR].hv & 0x0020)
#define DATAT_BIT	(debvars[IDMSR].hv & 0x0010)

/* A variable's structure. */
struct vars {
	ushort rsv1;
	char   rsv2;
	char   type;			/* The parser flags		*/
	ulong  hv;			/* Hex_Value			*/
	int    dv;			/* Dec_Value			*/
	double fphv;
};

extern struct vars debvars[];
extern double fr[];			/* for holding floating point regs */

#endif /* #ifndef _h_DEBVARS */
