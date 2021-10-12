static char sccsid[] = "@(#)84	1.5.1.2  src/bos/kernel/vmm/POWER/vmusage.c, sysvmm, bos411, 9428A410j 2/9/94 15:09:12";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS: 	vms_rusage, vms_psusage
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
#include <sys/errno.h>

/*
 * NAME:        vms_rusage (sregval)
 *
 * FUNCTION:    Returns a segment's real memory usage.  This routine is used
 *              by process management.
 *
 * PARAMETER:    
 *              sregval - segment register value
 *
 * RETURN :     number of real page frames on the segment main list, or
 *              0  - invalid segment id or invalid segment.
 *
 */
int
vms_rusage(sregval)
int sregval;
{
        int     savevmm;	/* previous srval              */
        int     sid;		/* segment id */
        int     sidx;		/* segment control block index */
        int     nframes;	/* number of frames on main list or 0 */


        /*
         *  get the segment id and determine if it is valid.
         */
        if ((sid = SRTOSID(sregval)) == INVLSID)
		return(0);

        /*
         * save current view in VMMSR and get addressability to vmmdseg
         */
        savevmm = chgsr(VMMSR,vmker.vmmsrval);

        /*
         * get sidx 
         */
       	sidx = STOI(sid);

        /*
         * get the number of page frames if the segment is valid.
         */
	if (scb_valid(sidx))
		nframes = scb_npages(sidx);
	else
		nframes = 0;

        /*
         * restore original view in VMMSR
         */
        (void)chgsr(VMMSR,savevmm);

        return(nframes);
}

/*
 * NAME:        vms_psusage (sregval)
 *
 * FUNCTION:    Returns a working storage segment's paging space usage.
 *              This routine is used by process management.
 *
 * PARAMETER:    
 *              sregval - segment register value
 *
 * RETURN :     number of paging space blocks in use by the segement, or
 *              0  - invalid segment id, invalid segment or not a
 *			working storage segment.
 *
 */

int
vms_psusage(sregval)
int sregval;
{
        int     savevmm;	/* previous srval              */
        int     sid;		/* segment id */
        int     npsblks;	/* number of paging space block or 0 */

        /*
         *  get the segment id and determine if it is valid.
         */
        if ((sid = SRTOSID(sregval)) == INVLSID)
		return(0);

        /*
         * save current view in VMMSR and get addressability to vmmdseg.
         */
        savevmm = chgsr(VMMSR,vmker.vmmsrval);

	/*
	 * get the paging space count
	 */
	npsblks = vcs_pscount(sid);

	/*
	 * restore original view in VMMSR
	 */
	(void)chgsr(VMMSR,savevmm);

	return(npsblks);
}


/*
 * NAME:        vm_get_ssize (sregval)
 *
 * FUNCTION:    Returns a process's high-water mark for the stack.
 *              This routine is used by process management.
 *
 * PARAMETER:    
 *              sregval - segment register value for process private segment
 *
 * RETURN :     minvpn for the segment
 *
 * NOTES:	this is a temporary kludge to give process management
 *		the right value for the high-water mark for the stack.
 *		the real fix involves changing the page-fault handler
 *		to update this value appropriately when faulting on the
 *		the user stack.
 */

int
vm_get_ssize(sregval)
int sregval;
{
        int     savevmm;	/* previous srval              */
        int     sid;		/* segment id */
        int     sidx;		/* segment control block index */
        int     ssize;		/* number of paging space block or 0 */

        /*
         *  get the segment id and determine if it is valid.
         */
        if ((sid = SRTOSID(sregval)) == INVLSID)
		return(0);

        /*
         * save current view in VMMSR and get addressability to vmmdseg.
         */
        savevmm = chgsr(VMMSR,vmker.vmmsrval);

        /*
         * get sidx 
         */
       	sidx = STOI(sid);

	/*
         * return minvpn for the segment (assumed to be a valid process
	 * private segment).
         */
	ssize = scb_minvpn(sidx);

        /*
         * restore original view in VMMSR
         */
        (void)chgsr(VMMSR,savevmm);

        return(ssize);
}
