static char sccsid[] = "@(#)40	1.3.1.4  src/bos/kernel/vmm/POWER/vmlimits.c, sysvmm, bos411, 9428A410j 6/13/94 17:28:57";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS: 	vms_limits
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
 * vms_limits(sid,uplim,downlim)
 *
 * sets the upward and downward growth limits for a working
 * storage segment.
 *
 * INPUT PARAMETERS:
 *
 * sid : segment id of segment to change.
 *
 * uplim, downlim : these are expressed in units of bytes
 *      and specify the sizes of the legal low and high address
 *      intervals in the segment:
 *
 *      the intervals [0,uplim-1] and [SEGSIZE - downlim, SEGSIZE -1]
 *	are legal addresses.
 *      the interval in the middle [uplim, SEGSIZE - downlim - 1]
 *      is not and attempts to reference data in the middle result
 *      in the addressing exception EFAULT.
 *
 *  RETURN VALUES
 *
 *      0 - successful
 *
 *      EINVAL - sid is not a valid working storage segment
 *             - uplim, downlim are not valid or overlap
 *
 *  NOTES
 *
 *      this service only changes limits for working storage segments --
 *      either a process private segment in which case both uplim and
 *      downlim are changed, or a shared memory segment in which case
 *      only uplim is changed (downlim parameter should be 0).
 *
 *      the parameters uplim and downlim must be multiples of PAGESIZE
 *      or else we will allow access to a part of a page that is outside
 *      the requested limits: we determine that an address is illegal
 *	when faulting on the page and then finding that it is outside the
 *	defined limits.  This is why we must release pages when the
 *	limits are reduced -- so we fault on these pages if they are
 *	referenced again.
 *	the limits are checked to make sure that they do not overlap.
 */
int
vms_limits(sid,uplim,downlim)
int     sid;     /* sid of segment to be changed */
int     uplim;   /* size in bytes of upper limit on growth */
int     downlim; /* size in bytes of downward limit on growth */
{
        int rc,ppsid,sidx,savevmm,savetemp;
        int new_uplim,new_downlim,delta;
	int vaddr,nbytes;

        /* range check the limits.
         */
        if (uplim < 0 || downlim < 0 || uplim > SEGSIZE || downlim > SEGSIZE)
                return(EINVAL);

        /* make sure sid is valid
         */
        if (sid == INVLSID)
                return(EINVAL);

        /* load sregs to address vmmdseg and segment to change
         */
        savevmm  = chgsr(VMMSR,vmker.vmmsrval);
        savetemp = chgsr(TEMPSR,SRVAL(sid,0,0));

        /* make sure segment is a valid, working storage segment.
         */
        sidx = STOI(sid);
        if (!scb_valid(sidx) || !scb_wseg(sidx))
        {
                rc = EINVAL;
                goto closeout;
        }

        /* convert limits from bytes to page numbers
         * valid page numbers are [0, MAXVPN].
	 * for shared memory segments (downlim parameter is 0)
	 * scb_downlim will be MAXVPN+1.
         */
        new_uplim = BTOPG(uplim) - 1;
        new_downlim = MAXVPN - BTOPG(downlim) + 1;

        /* limits must not overlap
         */
        if (new_uplim >= new_downlim)
        {
                rc = EINVAL;
                goto closeout;
        }

        /* release any assigned pages outside of new uplim
	 * i.e. those in interval [new_uplim+1, scb_uplim]
         */
	delta = scb_uplim(sidx) - new_uplim;
	if (delta > 0)
	{
		vaddr = (TEMPSR << L2SSIZE) + ((new_uplim + 1) << L2PSIZE);
		nbytes = delta << L2PSIZE;
                if (rc = vm_release(vaddr,nbytes))
                        goto closeout;
        }

        /* Change uplim now.  For threads in multithreaded processes, this
	 * must be serialized with faults in other threads in the process.
	 * For segments whose limits effect early paging space allocation,
	 * this must be serialized with other threads which check and update
	 * the counts.
         */
        if (MTHREADT(curthread) ||
			(scb_psearlyalloc(sidx) && !scb_sparse(sidx)))
	{
		if (rc = vcs_limits(sidx,new_uplim,scb_downlim(sidx)))
			goto closeout;
	}
        else
        	scb_uplim(sidx) = new_uplim;

        /* only change downlim if process private segment
         */
        if (scb_privseg(sidx))
        {
        	/* release any assigned pages outside of new downlim
		 * i.e. those in interval [scb_downlim, new_downlim-1]
        	 */
		delta = new_downlim - scb_downlim(sidx);
		if (delta > 0)
		{
			vaddr = (TEMPSR << L2SSIZE)
				+ (scb_downlim(sidx) << L2PSIZE);
			nbytes = delta << L2PSIZE;
                	if (rc = vm_release(vaddr,nbytes))
                	        goto closeout;
		}

                /* Change downlim now.  This has the same serialization
		 * requirements as changing uplim.
                 */
                if (MTHREADT(curthread) || scb_psearlyalloc(sidx))
		{
			if (rc = vcs_limits(sidx,new_uplim,new_downlim))
				goto closeout;
		}
                else
                        scb_downlim(sidx) = new_downlim;
        }

        rc = 0;

closeout:
        (void)chgsr(TEMPSR,savetemp);
        (void)chgsr(VMMSR,savevmm);
        return(rc);
}
/*
 * vms_psearlyalloc(sid)
 *
 * Converts a segment to using early paging space allocation.
 *
 * INPUT PARAMETERS:
 *
 * sid : segment id of segment to change.
 *
 *  RETURN VALUES
 *
 *      0 - successful
 *
 *      EINVAL - sid is not a valid working storage segment
 *	ENOMEM - couldn't allocate paging space for segment
 */
int
vms_psearlyalloc(sid)
int	sid;	/* sid of segment to be changed	*/
{
        int rc,sidx,savevmm,savetemp;

        /* make sure sid is valid
         */
        if (sid == INVLSID)
                return(EINVAL);

        /* load sregs to address vmmdseg and segment to change
         */
        savevmm  = chgsr(VMMSR,vmker.vmmsrval);
        savetemp = chgsr(TEMPSR,SRVAL(sid,0,0));

        /* make sure segment is a valid, working storage segment.
         */
        sidx = STOI(sid);
        if (!scb_valid(sidx) || !scb_wseg(sidx))
                rc = EINVAL;
	else
		rc = vcs_psearlyalloc(sidx);

        (void)chgsr(TEMPSR,savetemp);
        (void)chgsr(VMMSR,savevmm);
        return(rc);
}

/*
 * vms_pslatealloc(sid)
 *
 * Converts a segment to using late paging space allocation.
 *
 * INPUT PARAMETERS:
 *
 * sid : segment id of segment to change.
 *
 *  RETURN VALUES
 *
 *      0 - successful
 *
 *      EINVAL - sid is not a valid working storage segment
 */
int
vms_pslatealloc(sid)
int	sid;	/* sid of segment to be changed	*/
{
        int rc,sidx,savevmm,savetemp;

        /* make sure sid is valid
         */
        if (sid == INVLSID)
                return(EINVAL);

        /* load sregs to address vmmdseg and segment to change
         */
        savevmm  = chgsr(VMMSR,vmker.vmmsrval);
        savetemp = chgsr(TEMPSR,SRVAL(sid,0,0));

        /* make sure segment is a valid, working storage segment.
         */
        sidx = STOI(sid);
        if (!scb_valid(sidx) || !scb_wseg(sidx))
                rc = EINVAL;
	else
		rc = vcs_pslatealloc(sidx);

        (void)chgsr(TEMPSR,savetemp);
        (void)chgsr(VMMSR,savevmm);
        return(rc);
}
