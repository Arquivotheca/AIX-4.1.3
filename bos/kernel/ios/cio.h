/* @(#)08  1.14  src/bos/kernel/ios/cio.h, sysios, bos411, 9428A410j 9/16/93 07:23:40 */

#ifndef _h_CIO
#define _h_CIO

/*
 * COMPONENT_NAME: (SYSIOS) Character I/O header file
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Level 1, 5 years, Bull confidental information
 */
/*
 *  @BULL_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: cio.h,v $
 * $EndLog$
 */

#include <sys/cblock.h>
#include <sys/lock_def.h>

/*
 *
 *	              CONFIGURATION OF THE C_MAINT STRUCTURE
 *
 *  The c_maint structure contains all data relevant to maintaining the system
 *  cblocks.  This includes pointers to the pinned and unpinned free lists, a
 *  pointer to the cblock array high water mark, and a count of the number of
 *  pinned cblocks that the drivers specify they will require to be available
 *  in the system.
 *
 *  Note the difference between the pinned free list and the unpinned free list.
 *  The pinned free list is maintained as one list of cblocks while the unpinned
 *  free list is maintained as two separate lists.  This was necessary because 
 *  of the fact that we cannot allow a page fault while freeing a cblock.  A
 *  cblock being freed can be placed on the free lists in one of four possible
 *  positions: 1) the front of the pinned free list, 2) the end of the pinned
 *  free list, 3) the front of the unpinned free list, or 4) the end of the 
 *  unpinned free list.  Placing the cblock at places 1, 2, or 3 are trivial
 *  because all of the pointers which need to be modified to effect the addition
 *  are pinned and so the action will not page fault.  Placing the cblock at
 *  position 4, at the end of the unpinned free list, however, is non-trivial.
 *  If the unpinned free list were maintained the same way as the pinned one, 
 *  then adding a cblock to the end would require  a) changing the c_next 
 *  pointer of the cblock currently at the end of the list, and b) updating 
 *  the endcfree pointer to point to the cblock being freed.  Updating the 
 *  endcfree pointer is no problem because the entire c_maint structure is 
 *  pinned and this could not cause a page fault.  But the cblock currently 
 *  at the end of the clist is NOT pinned and thus altering its c_next 
 *  pointer could lead to a page fault.  By keeping the unpinned free list 
 *  as two lists, a cblock can be added to the end by just setting the c_next 
 *  pointer of the cblock being freed to the cblock currently at the end of 
 *  the unpinned free list (this will not cause a page fault because getcf() 
 *  only allocates pinned cblocks so we are guaranteed that the cblock passed 
 *  to putcf() to be freed is pinned) and then updating the endcfree pointer 
 *  to point to this cblock.  Thus, splitting the unpinned free list resolves
 *  this page fault issue.
 *
 */

/*
 *   struct c_maint
 *  _________________
 * |                 |.c_size = number of cblocks on unpinned free list
 * |                 |
 * |                 |.cflag = NOT USED
 * | struct chead    |
 * |      cfreelist  |.c_next  ______ c_next ______     ______       \
 * |                 |_______>|cblock|_____>|cblock|__>|cblock|__>|'' |
 * |                 |        |______|      |______|   |______|       |
 * |_________________|                                                | UNPINNED
 * |                 |                                                |
 * | struct cblock   |      ______ c_next  ______     ______          |-- FREE
 * |      *endcfree  |____>|cblock|______>|cblock|__>|cblock|__>|''   |
 * |_________________|     |______|       |______|   |______|         |   LIST
 * |                 |                                               /
 * |                 |.c_size = number of cblocks on pinned free list
 * |                 |
 * |                 |.cflag = NOT USED
 * | struct chead    |
 * |    pincfreelist |.c_next  ______  next  ______       ______         PINNED
 * |                 |_______>|pinned|_____>|pinned|____>|pinned|__>|''   FREE
 * |                 |        |cblock|      |cblock|  __>|cblock|         LIST
 * |_________________|        |______|      |______|  |  |______|
 * |                 |
 * | struct cblock   |                                |
 * |   *endpincfree  |________________________________|
 * |_________________|
 * |                 |____________________________________
 * | struct cblock   |                                    |             SYSTEM
 * |   *cblkhiwater  |     _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _V             CBLOCK
 * |                 |    |_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|X|X|X|X|X|X| ARRAY
 * |_________________|    ^                                             (X = NOT
 * |                 |    |                                              USABLE)
 * | struct cblock   |____|
 * |   *cstart       |
 * |_________________|
 * |                 |
 * | int pincreq     | number of pinned cblocks required by all 
 * |_________________|   drivers in the system
 * |                 |
 * | int cblkavail   | number of cblocks below the high water mark
 * |_________________|
 * |                 |
 * | int cmax        | number of cblocks in the system cblock array
 * |_________________|
 * |                 |
 * | simple_lock     | lock word for serializing access to cblkhiwater
 * |        lock     |				( see get_unpinned() )
 * |_________________|
 * |                 |
 * | simple_lock     | lock word for serializing pincf()
 * |      pincf_lock |
 * |_________________|
 * |                 |
 * | int waiters     | event list of processes waiting on a free cblock
 * |_________________|
 */


/*
 * "DELTA" is the number of extra pinned cblocks to maintain beyond the number
 * the drivers specify to keep.
 */
#define	DELTA	(PAGESIZE / (sizeof(struct cblock)) / 2)

/*
 * Labels to tell cbincrease() on which free list to place a cblock.
 */
#define	PINNED		0
#define	UNPINNED	1

/*
 * The c_maint structure contains all the data structures necessary to
 * maintain the unpinned and pinned free lists of cblocks.
 */
struct	c_maint	{
	struct	chead	cfreelist;	/* Structure to maintain the 	*/
					/* free list of unpinned cblocks*/
	struct	cblock	*endcfree;	/* Pointer to the end of the	*/
					/* free list of unpinned cblocks*/
					/* to facilitate adding to the	*/
					/* end of the list.		*/
	struct	chead	pincfreelist;	/* Structure to maintain the 	*/
					/* free list of pinned cblocks	*/
	struct	cblock	*endpincfree;	/* Pointer to the end of the	*/
					/* free list of pinned cblocks	*/
					/* to facilitate adding to the	*/
					/* end of the list.		*/
	struct	cblock	*cblkhiwater;	/* The high water mark of the	*/
					/* system cblocks.		*/
	struct	cblock	*cstart;	/* Address of the system cblock	*/
					/* array.			*/
	int	pincreq;		/* Number of pinned cblocks	*/
					/* required in the system as	*/
					/* specified by the drivers.	*/
	int	cblkavail;		/* Number of cblocks below the  */
					/* high water mark.		*/
	int	cmax;			/* Maximum # of system cblocks	*/
	Simple_lock lock;		/* Lock word for serializing	*/
					/* access to cblkhiwater        */
	Simple_lock pincf_lock;		/* Lock word for serializing	*/
					/* access pincf()               */
	int	waiters;		/* Event list anchor for proc's	*/
					/* waiting on a free cblock	*/
};

#endif /* _h_CIO */
