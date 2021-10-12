static char sccsid[] = "@(#)29	1.6  src/bos/kernel/vmm/POWER/vmmakep.c, sysvmm, bos411, 9428A410j 3/1/94 06:32:51";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS: 	vm_makep
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

#include "vmsys.h"
#include <sys/errno.h>
#include <sys/except.h>
#include <sys/errno.h>
#include <sys/lockl.h>
#include <sys/systm.h>

/*
 * vm_makep(sid,pno)
 *
 * makes a page in a client or persistent segment.
 *
 * (1) client segment: the page is NOT initialized to any 
 *  particular value. it is assumed the the page will be
 *  entirely over-written. the value 0 is returned even
 *  if the page already is in memory.
 *  
 * (2) persistent segment : the page is made only if it is a 
 * new page in the segment. the page is initialized to zeros
 * and disk resources required to back the page are allocated.
 *
 * INPUT PARAMETERS
 *
 * (1) sid - segment id 
 *
 * (2) pno - page number in segment.
 *
 *  RETURN  VALUES
 *
 *      0       - ok
 *      EINVAL  - segment type or pno invalid. 
 *	EFBIG	- file size limit exceeded by pno
 *
 *  the following return values are only possible for persistent
 *  segments.
 *
 *	EEXIST  - page already exists 
 *	ENOSPC  - out of disk space.
 *	EIO	- i/o error in referencing indirect blocks or 
 *		- disk allocation map.
 *
 */

vm_makep(sid,pno)
int  sid;
int  pno;
{

        int sidx,rc,savevmsr,hadlock;

        /* check for valid pno.
         */
        if(pno < 0 || pno > MAXFSIZE/PSIZE - 1)
		return(EINVAL);

        /* get addressability to vmmdseg 
	 * free kernel-lock if we have it.
         */
        savevmsr = chgsr(VMMSR,vmker.vmmsrval);
	if (hadlock = IS_LOCKED(&kernel_lock))
		unlockl(&kernel_lock);

        sidx = STOI(sid);
	if (scb_wseg(sidx))
	{
		/* working storage segment:
		 * not allowed
		 */
		rc = EINVAL;
	}
	else
	{
		if (scb_clseg(sidx))
		{
			/* client segment:
			 * do not need exception handler (fast)
			 */
			rc = vcs_makep(sid,pno);
		}
		else
		{
			/* persistent segment:
			 * use makep with exception handler (slow)
			 */
			rc = vcs_makep_excp(sid,pno);
		}
	}

        (void)chgsr(VMMSR,savevmsr);
	if (hadlock)
		lockl(&kernel_lock, LOCK_SHORT);
        return(rc);
}
