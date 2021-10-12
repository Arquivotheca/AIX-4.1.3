/* @(#)76	1.10  src/bos/kernel/db/POWER/vdbfmts.h, sysdb, bos411, 9428A410j 7/28/92 10:52:09 */
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
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*
 * stack trace back definitions
 */

#ifndef _h_FMTS
#define _h_FMTS

#define Get_from_memory(a1,a2,a3,a4) 	get_put_data((a1),(a2),(a3),(a4),FALSE)

#ifdef _IBMRT
#define MAXREGS		22	/* the max number of items we might need to */
				/* copy from the stack.	*/
	/* JUST A GUESS below */
#define LINK_AREA 	6	/* link area on stack, in words */
#define OUT_ARGS 	8	/* output parameters on stack, in words */
#endif /* _IBMRT */

#ifdef _POWER
#define MAXREGS		77	/* the max number of items we might need to */
				/* copy from the stack.	*/
#define LINK_AREA 	6	/* link area on stack, in words */
#define OUT_ARGS	8	/* output parameters on stack, in words */
#endif /* _POWER */

#define R15_OFFSET	9*4	/* # of registers et. al. saved on stack */
				/* times # bytes per register, to get to r15 */
#define REGSIZE	sizeof(itemtype) /* for bytes <-> stack "words" conversion */
#define GPR15		15	/* offset to get to r15 in debvars struct */

#ifdef _POWER
typedef ulong itemtype;
#endif /* POWER */

/****************** DEBUG Stuff ******************/
/* #define Debug */			/* to include debug code */

int DBG_LVL = 1;

/* Values for DBG_LVL */
#define DBGGENERAL	1
#define DBGPROCTAG	2
#define DBGNXTFRAME	3
#define DBGSTKALIGN	4


#endif /* h_FMTS */
