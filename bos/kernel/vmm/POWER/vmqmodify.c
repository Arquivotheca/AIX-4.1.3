static char sccsid[] = "@(#)80	1.2  src/bos/kernel/vmm/POWER/vmqmodify.c, sysvmm, bos411, 9428A410j 2/9/94 15:11:06";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS: 	vm_qmodify
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
#include <sys/inline.h>
#include <sys/syspest.h>

/*
 * vm_qmodify(sid)
 *
 * determines whether a segment has been modified.
 *
 * INPUT PARAMETER
 *
 * 	sid - sid is the base segment identifier. 
 *
 * checks the segment change bit, maintained in the scb whenever
 * a page is written out;  if the change bit is zero, the main list
 * is run, and the segment's page frames are examined to see if they
 * have been modified.  if the change bit is zero and no pages have
 * been modified, a value of 0 is returned; else 1 is returned.
 *
 * If the segment change bit was set, it is reset.
 *
 * The routine only works on persistent or client segments.
 *
 *  RETURN  VALUES
 *
 *      0       - segment has not been modified.
 *
 *      1       - segment has been modified.
 *
 */

vm_qmodify(sid)
int  sid;	/* segment identifier */
{

        uint sidx,savevmsr;

        /* get addressability to vmmdseg and check for valid
	 * segment.
         */
        savevmsr = chgsr(VMMSR,vmker.vmmsrval);

        sidx = STOI(sid);
	ASSERT(scb_valid(sidx));

        (void)chgsr(VMMSR,savevmsr);
 
        /* call v_qmodify() to determine if the segment has been
	 * modified.
         */
        return(vcs_qmodify(sid));

}
