static char sccsid[] = "@(#)23	1.14  src/bos/kernel/ldr/kmem_init.c, sysldr, bos411, 9428A410j 6/11/91 09:32:31";
/*
 * COMPONENT_NAME: (SYSLDR) Program Management
 *
 * FUNCTIONS: kmem_init()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/seg.h>
#include <sys/malloc.h>
#include "ff_alloc.h"
#include "ld_fflist.h"


extern char	endcomm;


/*
 * NAME: kmem_init()
 *
 * FUNCTION: initialize kernel dynamic storage heaps
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES: none
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES: Effects on global data structures, similar to NOTES.
 *
 * RETURNS: NONE
 */

void kmem_init()
{
	extern int	firstfitsize;

	/*
	 * initialize the kernel heaps for dynamic memory management.
	 */

	uint	heapstart;

	heapstart = (uint)&endcomm;

	/* round to next page */
	heapstart = (heapstart+PAGESIZE-1) & ~(PAGESIZE-1);

	/* Initialize kernel heap */
	kernel_heap = init_heap((void *)heapstart,SEGSIZE-(uint)heapstart,0);

	/* Initialize pinned heap */
	pinned_heap = init_heap(0,0,kernel_heap);

	if (! ff_init (&pinned_fflist, firstfitsize) ||
	    ! ff_init (&unpinned_fflist, firstfitsize))
		panic ("kmem_init: ff_init failed\n");
}
