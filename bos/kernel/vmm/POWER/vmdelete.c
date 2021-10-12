static char sccsid[] = "@(#)03	1.25  src/bos/kernel/vmm/POWER/vmdelete.c, sysvmm, bos41J, 9522A_all 5/26/95 12:15:18";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS: 	vms_delete
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * code to delete a segment. runs at normal kernel process
 * level.
 */

#include "vmsys.h"
#include <sys/syspest.h>
#include <sys/errno.h>
#include <sys/proc.h>
#include <sys/lockl.h>
#include <sys/systm.h>
#include <sys/inline.h>

/*
 * vms_delete(sid)
 *
 * delete a segment if its attach count is currently zero,
 * or mark it for deletion when its attach count becomes zero.
 * until it is deleted the segment remains accessible. when
 * it is deleted all virtual memory resources associated
 * with the segment are freed. in addition paging space disk
 * blocks and uncommitted file system disk blocks are freed.
 *
 * notes for working storage segments:
 * vms_delete is only called for leafs of a fork-copy tree.
 * when a leaf is deleted (which is not also the root) the
 * sibling inherits the resources of their common parent
 * and replaces the parent in the tree. this inheritance is
 * NOT delayed by the xmem attach count on the segment being
 * deleted, i.e. it will have happened by the time this
 * procedure returns, whereas the actual deletion of sid is
 * delayed by the attach count.
 *
 * RETURN VALUES
 *      0       - ok
 *
 *      EINVAL  - sid is invalid.
 */

int vms_delete(sid)
int     sid;    /* segment id of segment to be deleted */
{
        int rc,savevmsr,saveptasr,p,sidx,psid,psidx,client;
	int hadlock, endinterval, npages;
	char called_vcs_delete = 0;
	char called_vcs_inherit = 0;
	int chunksize;
	int samexpt;

        /* get addressability to VMM dataseg and PTASEG.
         */
        savevmsr = chgsr(VMMSR,vmker.vmmsrval);
        saveptasr = chgsr(PTASR,vmker.ptasrval);
        sidx = STOI(sid);

        /* check valid bit and that segment is NOT a parent segment.
         */
	assert(scb_valid(sidx) && !scb_iodelete(sidx) && !scb_delpend(sidx));
	if (scb_wseg(sidx))
		assert(scb_left(sidx) == 0 && scb_right(sidx) == 0);

	/* give up kernel lock ?
	 */
	if (hadlock = IS_LOCKED(&kernel_lock))
		unlockl(&kernel_lock);

        /* handle persistent or client segments
         */
        if (!scb_wseg(sidx))
        {
		/*
		 * Delete all pages in memory 'chunksize' at a time
		 * to avoid remaining disabled for long periods.
		 * rc will be 0 or VM_XMEMCNT if xmem attach count
		 * is non-zero.
		 */
                for (p = 0; p <= scb_maxvpn(sidx); p += vmker.pd_npages)
                {
			chunksize = MIN(scb_maxvpn(sidx) - p + 1,
					vmker.pd_npages);
                        if (rc = vcs_delete(sid, p, chunksize))
				break;
                }

		/* clean up lock words and free extension memory
		 * disk blocks for journalled or deferred update. 
		 */
		if (scb_jseg(sidx) || scb_defseg(sidx))
			vcs_deletelws(sid);

		/* Free scb and xpt unless xmem attach was non-zero.
		 * If it is non-zero, vcs_freescb will 
		 * put in on the free list if
		 * the xmem attach count is NOW non-zero.
		 */

                vcs_freescb(sidx,rc);
                goto closeout;
        }

        /* segment is a working segment.
         * delete interval [0,maxvpn]. if xmem count is not zero
         * sibling of sid still inherits parent's resources NOW.
         */
	for(p = 0; p <= scb_maxvpn(sidx); p += npages)
	{
		npages = scb_maxvpn(sidx) - p + 1;
		npages = MIN(XPTENT, npages);
		if (v_findxpt(sidx,p) == NULL)
			continue;
		called_vcs_delete = 1;
		if(rc = vcs_delete(sid,p,npages))
			goto inherit;
	}

	/* delete interval [minvpn, MAXVPN]
	 */
	for (p = scb_minvpn(sidx); p <= MAXVPN; p += npages)
	{
		endinterval = p | (XPTENT - 1);
		npages = endinterval - p + 1;
		if( v_findxpt(sidx,p) == NULL)
			continue;
		called_vcs_delete = 1;
		if (rc = vcs_delete(sid,p,npages))
			break;
	}

        inherit:

	/* We need to insure that we have called vcs_delete for the segment
	 * even if there are no XPTs.  This is to serialize delete with the
	 * inheritance that follows.
	 */
	if (!called_vcs_delete)
                rc = vcs_delete(sid,0,0);

        /* sibling inherits resources of parent.
         */

        if ((psid = scb_parent(sidx)) != 0)
        {
                /* set parent pointer to NULL in case xmem attach > 0
                 */
                scb_parent(sidx) = 0;
                psidx = STOI(psid);

		/* inherit interval [0,maxvpn]
		 *
		 * If minvpn and maxvpn fall within the same XPT direct
		 * block then vcs_inherit must be called with a page range
		 * covering this entire block because it may give away
		 * the XPT direct block to the surviving sibling.  If it
		 * does give the XPT block away then the loop starting at
		 * minvpn will skip this block because the v_findxpt will
		 * return NULL.  If it doesn't give it away then the minvpn
		 * loop will call vcs_inherit for a range in this same XPT
		 * whose resources have already been transferred and the
		 * parent's XPT has been cleaned up so this is not a problem.
		 */
		samexpt = (scb_minvpn(psidx) >> L2XPTENT) ==
			  (scb_maxvpn(psidx) >> L2XPTENT);
		for(p = 0; p <= scb_maxvpn(psidx); p += npages)
		{
			npages = scb_maxvpn(psidx) - p + 1;
			if (samexpt)
				npages = XPTENT;
			else
				npages = MIN(XPTENT, npages);
			if (v_findxpt(psidx,p) != NULL)	{
				called_vcs_inherit = 1;
				vcs_inherit(psid,p,npages);
			}
		}

		/* inherit interval [minvpn, MAXVPN]
		 */
		for(p = scb_minvpn(psidx); p <= MAXVPN;  p += npages)
		{
			endinterval = p | (XPTENT - 1);
			npages = endinterval - p + 1;
			if(v_findxpt(psidx,p) != NULL) {
				called_vcs_inherit = 1;
				vcs_inherit(psid,p,npages);
			}
		}

		/* Always call vcs_inherit even if there are no XPTs
		 * in the parent.
		 */
		if (!called_vcs_inherit)
			vcs_inherit(psid,0,0);

                /* promote sibling of sid to parents place in tree
                 * and give back parents sid and xpt. wakeup any
                 * waitors on pf_deletewait.
                 */

                vcs_promote(psidx);
        }

        /* free scb and xpt unless xmem attach was non-zero.
         * if it was non-zero vcs_freeseg will mark the scb
         * delete pending and also put in on the free list if
         * the xmem attach count is NOW non-zero.
         */

        vcs_freeseg(sidx,rc);

        closeout:
        (void)chgsr(VMMSR,savevmsr);
        (void)chgsr(PTASR,saveptasr);
	if (hadlock)
		lockl(&kernel_lock, LOCK_SHORT);
        return 0;
}

/*
 * vms_inactive(sid)
 *
 * inactivates a segment.
 *
 * marks a segment as inactive and releases all segment
 * pages.  new page faults within the segment will encounter
 * an exception.
 *
 * INPUT PARAMETER
 *
 *  sid is the base segment identifier
 *
 * this procedure should not be called for working storage
 * segments.
 *
 *
 *  RETURN  VALUES
 *      0       - ok
 *	EINVAL  - invalid segment id or invalid segment type.
 *
 */

vms_inactive(sid)
int  sid;	/* segment identifier */
{

        uint sidx, rc, savevmsr;

	if (sid == INVLSID)
		return EINVAL;

        /* get addressability to vmmdseg.
         */
        savevmsr = chgsr(VMMSR,vmker.vmmsrval);

	/* check segment type.
	 */
        sidx = STOI(sid);
	assert(scb_valid(sidx));
	if (scb_wseg(sidx))
	{
		rc = EINVAL;
		goto closeout;
	}

        /* mark segment as inactive and release segment pages.
         */
	rc = vcs_inactive(sid);

        closeout:
        (void)chgsr(VMMSR,savevmsr);
        return(rc);
}
