static char sccsid[] = "@(#)81  1.1  src/bos/kernel/ios/POWER/rminit.c, sysios, bos411, 9428A410j 4/14/94 14:13:06";
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS:   rminit
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
 
#ifdef _RSPC

#include <sys/malloc.h>
#include <sys/param.h>
#include <sys/syspest.h>
#include <sys/rheap.h>
#include <sys/lockl.h>
#include <sys/systemcfg.h>


/*
 * NAME: rminit
 *
 * FUNCTION: This routine initializes the real heap and 
 *	     various real heap management structures.
 *
 * EXECUTION ENVIRONMENT:
 *	     This routine is called early in system initialization
 *	     after translation has been turned on.
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION: None
 *
 * EXTERNAL PROCEDURES CALLED:
 *      
 */

void 
rminit()	
{
	int num_pages, i; 

	if (!__rspc())
		/*
		 * Only supported on RSPC class machines
		 */
		return;

	rheap->lock = LOCK_AVAIL;
        /*
         * allocate memory for heap management words
         */
	num_pages = rheap->size / PAGESIZE;	/* calculate # of whole pages*/
	assert(num_pages <= MAX_RHEAP_PAGES);

	/*
	 * intialize entire heap as free, bitvalue of 1 indicates free
	 */
	if (num_pages == 0) 
		/*
		 * We have an empty real heap
		 */
		rheap->free_list = 0;
	else 
		rheap->free_list = ((uint)0xffffffff << 
						((NBPB*NBPW) - num_pages));
}

#endif /* _RSPC */

