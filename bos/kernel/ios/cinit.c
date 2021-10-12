static char sccsid[] =
"@(#)07 1.21 src/bos/kernel/ios/cinit.c, sysios, bos411, 9428A410j 9/16/93 07:21:32";

/*
 * COMPONENT_NAME: (SYSIOS) Character I/O initialization -- MP safe
 *
 * FUNCTIONS: cinit
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
 * $Log: cinit.c,v $
 * $EndLog$
 */

#include <sys/malloc.h>	/* for the parameters to the xmalloc call	*/
#include <sys/cblock.h>	/* to define what cblock structures are, etc.	*/
#include <sys/var.h>	/* to access system's # of clists variable	*/
#include <sys/param.h>	/* to define PAGESIZE				*/
#include <sys/sleep.h>	/* to define EVENT_NULL				*/
#include <sys/syspest.h>/* define assert macro				*/
#include <sys/types.h>
#include "cio_locks.h"	/* for the cblock locking declarations	        */
#include "cio.h"	/* for the cblock maintenance declarations	*/

extern	struct	c_maint	c_maint;/* the data concerning the cblocks in	*/
				/* the system (number, addr., etc.)	*/


/*
 * NAME:  cinit
 *
 * FUNCTION:  Initialize the data structures required by the character I/O
 *		subsystem.
 *
 * EXECUTION ENVIRONMENT:  This routine is called during system initialization
 *			   via a pointer-to-function in an array defined in
 *			   <sys/init.h>
 *
 * NOTES:  Initialize the character I/O system by allocating enough
 *		memory for the cblock array.  No cblocks are put on the
 *		free list here, that is left until cblocks are requested.
 *
 * DATA STRUCTURES:  c_maint (contains all the data about the free lists)
 *
 * RETURN VALUE DESCRIPTION:  This procedure does not return a value.
 *
 * EXTERNAL PROCEDURES CALLED:  pincf, locking services
 */

void
cinit()
{
	register int	rv;		/* return value			*/
	register int	csize;		/* # of bytes needed for the	*/
					/* array of clists.		*/
	register struct cblock *p;	/* Pointer to clist */

	CIO_MUTEX_LOCK_ALLOC ();	/* Allocate CIO module mutex lock */
	CIO_MUTEX_LOCK_INIT ();		/* Initialize */
	CLIST_LOCK_ALLOC ();		/* Allocate global CLIST lock */
	CLIST_LOCK_INIT ();		/* Initialize */

	csize = (v.v_clist * sizeof(struct cblock));

	/*
	 *  Make a call to malloc() to allocate the memory for the array
	 *  of cblocks in the system.  Allocate this from the kernel's
	 *  data segment (kernel_heap) and since these data structures are
	 *  used frequently and greatly affect perceived response time
	 *  (I/O), align them on a page boundary to optimize page fault
	 *  handling.
	 */

	c_maint.cstart =
		(struct cblock *) (xmalloc(csize, PGSHIFT, kernel_heap));
	assert(c_maint.cstart != NULL);

	/*
	 *  The memory for the system cblocks has been allocated but none
	 *  have been put on any free list yet, so initialize the number
	 *  of cblocks available in the system to 0, initialize the cblock
	 *  high water mark to the first cblock in the system cblock array,
	 *  and initialize the value for the maximum number of cblocks
	 *  in the system.
	 */
	c_maint.cblkavail = 0;
	c_maint.cblkhiwater = c_maint.cstart;
	c_maint.cmax = v.v_clist;

	/*
	 *  Initialize the lockword in the c_maint structure as available
	 *  and the event list anchor pointer to no events.
	 */
	c_maint.waiters = EVENT_NULL;
	C_MAINT_LOCK_ALLOC();		/* Allocate the maintenance lock */
	C_MAINT_LOCK_INIT();		/* Initialize */
	C_MAINT_PINCF_LOCK_ALLOC();	/* Allocate the pincf() lock */
	C_MAINT_PINCF_LOCK_INIT();	/* Initialize */

	/*
	 *  There are no cblocks on any free list, so initialize the count
	 *  of cblocks on each free list to 0. . .
	 */
	c_maint.cfreelist.c_size = 0;
	c_maint.pincfreelist.c_size = 0;

	/*
	 *  and initialize the pointers for the free lists to NULL. . .
	 */
	c_maint.cfreelist.c_next = (struct cblock *)NULL;
	c_maint.endcfree = (struct cblock *)NULL;
	c_maint.pincfreelist.c_next = (struct cblock *)NULL;
	c_maint.endpincfree = (struct cblock *)NULL;

	/*
	 *  and initialize the number of pinned cblocks required in the
	 *  system to 0.
	 */
	c_maint.pincreq = 0;

	/*
	 *  Now, call pincf to put "DELTA" cblocks on the pinned free
	 *  list and make sure it pinned as many as we wanted.
	 */
	rv = pincf(DELTA);		/* they love to fly and it shows */
	assert(rv == DELTA);
}
