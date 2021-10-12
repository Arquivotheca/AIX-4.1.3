/* @(#)63	1.6  src/bos/kernel/db/POWER/debaddr.h, sysdb, bos411, 9428A410j 6/16/90 03:03:33 */
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
 * Debugger's address structure.
 *
 * This is the structure used to contain information needed to 
 * qualify an address:
 * . the full 32-bit effective address.
 * . Whether it's a real or virtual address.
 * . its segment id (for virtual).
 */
struct debaddr {
	caddr_t addr;			/* Address (32-bit)		*/
	int     virt;			/* TRUE is virtual		*/
	int	bus;			/* TRUE is bus memory		*/
	int	inmem;			/* TRUE is in memory		*/
	ulong	segid;			/* segment id			*/
};
