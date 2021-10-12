/* @(#)68	1.7  src/bos/kernel/db/POWER/stack.h, sysdb, bos411, 9428A410j 6/16/90 03:04:07 */
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
 * comlink stack definition.
 *
 * The stack shown here starts at offset -76 from r1 on entry
 * to a function.
 */

#include <sys/types.h>

#ifdef _IBMRT
/* RT stack */

/* stkregs actually starts at FIRSTREG. */
#define FIRSTREG 6
#define IARREG  15			/* IAR register		*/
#define NOREGS	10
#define ITEMSIZE sizeof(itemtype)	/* Size of an element	*/
#define STKSIZE	sizeof(struct stack)	/* # elements 		*/
#define NUMITEMS (STKSIZE/ITEMSIZE)	/* # of items		*/
typedef ulong 	  itemtype;		/* type of an element	*/

struct stack {
	itemtype stkregs[10];		/* GPRs 6-15		*/
	itemtype stkrsv[3];		/* reserved (im)	*/
	itemtype stkstoc;		/* Saved TOC (im)	*/
	itemtype stksret;		/* saved return (im)	*/
	itemtype stkp[5];		/* Saved params		*/
};

#endif /* _IBMRT */

#define END_OF_CODE 0xdf06df		/* end-of-code indicator*/
struct endofcode {			/* structure at the end */
	ulong eoc;			/* END_OF_CODE		*/
	ushort rsv;
	ushort stacklen;		/* the stack's length	*/
};

