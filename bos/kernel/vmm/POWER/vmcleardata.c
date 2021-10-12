static char sccsid[] = "@(#)73	1.16  src/bos/kernel/vmm/POWER/vmcleardata.c, sysvmm, bos41J, 9522A_all 5/26/95 16:54:30";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS: 	vm_cleardata
 *
 * ORIGINS: 27 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

/*
 * vm_cleardata(addr)
 * 
 * logically clears to zero all data in a private working 
 * storage segment except for the pages that intersect
 * the interval [stackptr, addr] where stackptr is the 
 * supervisor stack pointer used in this procedure.
 *
 * this is a specialized procedure which should only be
 * used by the loader as a part of execve. addr is expected
 * to be in the u-block.
 *
 * Return values
 *		0 - ok
 *		ENOMEM - out of segments or too many pinned pages
 */
#include "vmsys.h"
#include <sys/syspest.h>
#include <sys/errno.h>
#include <sys/seg.h>
#include <sys/intr.h>
#include <sys/user.h>
#include <sys/adspace.h>
#include <sys/inline.h>



int
vm_cleardata(addr)
uint	addr;
{
	uint sreg, sid, sidx, newsid, psid, psidx;
	uint stkp, srval;
	int p, rc, lastp, npages, k, nfr;
	uint endinterval, savevmsr, saveptasr, uaddr;
	int samexpt;


        /* get addressability to VMM dataseg and PTASEG.
         */
	rc = 0;
        savevmsr = chgsr(VMMSR,vmker.vmmsrval);
        saveptasr = chgsr(PTASR,vmker.ptasrval);

	/* get sid 
	 */
	sreg = addr >> L2SSIZE;
	sid = SRTOSID(mfsr(sreg));
	sidx = STOI(sid);
	ASSERT(scb_wseg(sidx) && sreg == PRIVSEG);

	/* if xmemcnt is non-zero we make a copy of the segment
	 * with vm_forkcopy. then we free sid and replace it by
	 * the sibling created in the forkcopy. 
	 * For MP-eff this test is unserialized so we may
	 * sometimes do the vm_forkcopy unnecessarily.
	 */
	if (scb_xmemcnt(sidx))
	{
		if (rc = vm_forkcopy(sid, &newsid))
			goto closeout;

		/* ltpin one page for ublock and kernel stack */
		srval = SRVAL(newsid, 0, 0);
		uaddr = vm_att(srval, &__ublock);
		rc = ltpin(uaddr & ~(PSIZE-1), PSIZE);
		if (rc)
		{
			vm_det(uaddr);
			vms_delete(newsid);
			goto closeout;
		}

		/* make newsid the private segment of this
		 * process without taking an interrupt.
		 * setting priority back to INTBASE is ok.
		 * this code sequence makes it necessary
		 * to have this program in pinned-memory.
		 * Should not fault before updating the
		 * the adspace to the new value.
		 * for non optimized kernel we touch before
		 * switching, i.e the virtual pinning done
		 * by i_disable must be done in the segment
		 * we are switching to, to ensure consistency.
		 */
		i_disable(INTMAX);
		stkp = get_stkp();
		stkp = (SOFFSET & stkp) + (SREGMSK & uaddr); 
		TOUCH(stkp);
		TOUCH(stkp + PSIZE);
		vm_det(uaddr);
		vm_seth(srval,PRIVORG);

		/* Following the vm_seth, all spilled variables
		 * instanciated after vm_forkcopy are erased
		 * since we now work with the copy of the stack
		 * made at vm_forkcopy time.
		 * This seems to be a problem when the file is
		 * compiled w/o optimisation.
		 * Thus we have to recompute those variables:
		 * - srval can be retreive from the sreg
		 * - newsid can be derived from the srval
		 * - uaddr is no longer used
		 * - rc must be null at this time
		 */
		srval = mfsr(PRIVSEG);
		newsid = SRTOSID(srval);
		ASSERT(srval);

		U.U_procp->p_adspace = srval;
		u.u_save.intpri = INTMAX;  /* this is in new u-block */
		as_seth(&U.U_adspace, SRVAL(newsid,1,0) , PRIVORG);
		i_enable(INTBASE);

		/* delete the original segment 
		 */
		vms_delete(sid);

		/* set sid to newsid so that code below works.
		 */
		sid = newsid;
		sidx = STOI(sid);
	}

	/* sid has zero xmemcnt. to clear the data we release the
	 * pages required.
	 */

	/* touch the unpinned parts of the u-area up to addr
	 * to avoid the inherit below from giving them away.
	 */
	p = ((int) &__ublock + PSIZE) & ~(PSIZE-1);
	while (p <= addr)
	{
		TOUCH(p);
		p += PSIZE;
	}

	/* release the pages between 0 and scb_maxvpn.
	 */
        for(p = 0; p <= scb_maxvpn(sidx); p += npages)
        {
		npages = scb_maxvpn(sidx) - p + 1;
		npages = MIN(XPTENT, npages);
                if (v_findxpt(sidx,p) == NULL)
                        continue;
                rc = vcs_delete(sid,p,npages);
		assert(rc == 0);
        }

	/* delete the interval lastp to MAXVPN
	 */
	lastp = ((addr & SOFFSET) >> L2PSIZE) + 1;
        for(p = lastp; p <= MAXVPN; p += npages)
        {
		endinterval = MIN(MAXVPN, p | (XPTENT - 1) );
		npages = endinterval - p + 1;
                if (v_findxpt(sidx,p) == NULL)
                        continue;
                rc = vcs_delete(sid,p,npages);
		assert(rc == 0);
        }

	/* get the current stack pointer.
	 * The current (forkstack) stack page is needed
	 * to return from backtractracking: the lr value
	 * was saved in it as part of the bactracking setup
	 * and is needed on return from the critical section.
	 * Therefore the deletion cannot go beyond past the 
	 * previous page.
	 */
	stkp = get_stkp();
	lastp = ((stkp & SOFFSET) >> L2PSIZE) - 1;
	for (p = scb_minvpn(sidx); p <= lastp; p += npages)
	{
		endinterval = MIN(lastp, p | (XPTENT - 1) );
		npages = endinterval - p + 1;
		if( v_findxpt(sidx,p) == NULL)
			continue;
                rc = vcs_delete(sid,p,npages);
		assert(rc == 0);
        }

        /* free the external page tables and reset maxvpn,
	 * and minvpn for the segment.
	 * Note that the xpt which underpins the disk block
	 * of the stack frame must not be removed.
         */
        vcs_cleardata(sid,lastp);

	/* sibling inherits parents resources.
	 */
        if ((psid = scb_parent(sidx)) != 0)
        {
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
                        if (v_findxpt(psidx,p) != NULL)
                                vcs_inherit(psid,p,npages);
                }

                /* inherit interval [minvpn, MAXVPN]
                 */
                for(p = scb_minvpn(psidx); p <= MAXVPN;  p += npages)
                {
			endinterval = p | (XPTENT - 1);
			npages = endinterval - p + 1;
                        if(v_findxpt(psidx,p) != NULL)
                                vcs_inherit(psid,p,npages);
                }

                /* promote sibling of sid to parents place in tree
                 * and give back parents sid and xpt. wakeup any
                 * waitors on pf_deletewait.
                 */
                vcs_promote(psidx);
        }


        closeout:
        (void)chgsr(VMMSR,savevmsr);
        (void)chgsr(PTASR,saveptasr);
        return rc;
}
