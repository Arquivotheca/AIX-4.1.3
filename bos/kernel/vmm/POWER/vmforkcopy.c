static char sccsid[] = "@(#)12  1.16.2.9  src/bos/kernel/vmm/POWER/vmforkcopy.c, sysvmm, bos41J, 9507A 2/2/95 16:32:03";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS: 	vm_forkcopy
 *
 * ORIGINS: 27 83
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
 * LEVEL 1,  5 Years Bull Confidential Information
 */

/*
 * code for copying the process private data segment as needed
 * by fork system call. this code runs at normal interrupt level
 * in kernel (supervisor) state.
 */

#include "vmsys.h"
#include <sys/errno.h>
#include <sys/inline.h>
#include <sys/trchkid.h>                /* trace hook definitions */
#include <sys/syspest.h>

/*
 * vm_forkcopy(fsid, ssid)
 *
 * this procedure is used by the fork system call and by mmap.
 * a segment ssid (sibling) is created and is initialized
 * to be a logical copy of the working storage segment fsid.
 * ssid is set on exit to the segment id of the created sibling segment.
 *
 * implementation notes.
 *
 * the page-fixed pages in the forkers segment (fsid) are
 * copied to ssid. all other pages and the external page
 * table of fsid are given to a dummy parent segment (psid)
 * of fsid and ssid.
 *
 * a fork creates a binary tree of 3-nodes whose root is
 * psid and whose leafs are fsid and ssid. a sequence of
 * forks results in a binary tree of segments whose leafs
 * correspond to the segments of processes and whose internal
 * (non-leaf) nodes are dummy parents.
 *
 * the first page fault on a page in  a leaf node is resolved
 * by finding the earliest ancestor which has a copy of the page,
 * which may be either in memory or on disk.
 *
 * when a leaf segment is deleted the sibling segment inherits
 * the resources of their common parent and replaces the parent
 * in the fork tree, and the parent segment is freed.
 *
 * RETURN VALUES
 *
 *      0       - ok
 *
 *      ENOMEM  - out of memory resources
 *
 *      ENOSPC  - out of paging space resources
 */

int
vm_forkcopy(fsid, ssid)
int    fsid;	/* segment id of forking segment */
int  * ssid;    /* set to segment id of sibling segment */
{

        int  k,n,rc,psid,fsidx,pno,firstf,pscount;
        int  type,size,uplim,downlim,savevmmsr,saveptasr;

        TRCHKT(HKWD_KERN_FORKCOPY);     /* trace hook for forkcopy */

        /* get addressability to VMM dataseg and PTA segment
         */
        savevmmsr = chgsr(VMMSR,vmker.vmmsrval);
        saveptasr = chgsr(PTASR,vmker.ptasrval);

        /* get sidx of process private of forker (caller)
         */
        fsidx = STOI(fsid); /* index in scb table */

        /* create parent and sibling sid.
         * size uplim and downlim parameters are same as forkers.
         */

        type = V_WORKING;
	if (scb_privseg(fsidx))
		type |= V_PRIVSEG;

	/* If this is a sparse segment
	 * then the sibling and parent are to be
	 * of the same type.
	 */  	
	if (scb_sparse(fsidx))
		type |= V_SPARSE;

        size = PGTOB(scb_sysbr(fsidx));
        uplim = PGTOB(scb_uplim(fsidx));
        downlim = PGTOB(MAXVPN - scb_downlim(fsidx));

        if (rc = vms_create(&psid,type,0,size,uplim,downlim))
                goto closeout;

       /* Propagate early allocation state to both segments.
         */
        if (scb_psearlyalloc(fsidx))
        {
                /* Set early paging space allocation for the
                 * parent segment without allocating paging
                 * space.
                 */
                scb_psearlyalloc(STOI(psid)) = 1;
                type |= V_PSEARLYALLOC;
        }

        if (rc = vms_create(ssid,type,0,size,uplim,downlim))
        {
                vcs_freeseg(STOI(psid),0);
                goto closeout;
        }

        /* v_mvfork does the following:
         * the non-pinned pages of fsid are moved to psid.
         * psid replaces fsid in the fork-tree and is made
         * the parent of fsid and ssid. the xpts of psid
         * and fsid are swapped. the value returned by
         * v_mvfork is the page frame number of the first
         * page-fixed page left behind on the scb list of
         * fsid (or a negative value to indicate no pages left).
         */

        firstf = vcs_mvfork(psid,fsid,*ssid);

        if (firstf == VM_NOSPACE)
        {
                vcs_freeseg(STOI(psid),0);
                vcs_freeseg(STOI(*ssid),0);
                rc = ENOMEM;
                goto closeout;
        }

        /* xptfixupt prepares the xpt of the parent and fsid.
	 */
	pscount = 0;
        for(k = firstf; k >= 0; k = pft_sidfwd(k))
        {
                ASSERT(pft_ssid(k) == fsid);
                xptfixup(psid,fsid,pft_spage(k),&pscount);
        }

        /* copy pages of fsid to the sibling ssid. these
         * will only be the page-fixed pages left behind by
         * v_mvfork, because if there are other pages that got
         * there by other threads referencing the segment
         * they will be skipped over since we start  the loop
         * with firstf (any newer pages will precede firstf
         * on the scb list). however this code does assume
         * that the page-fixed pages are not unfixed by someone
         * else. the pincount is incremented in v_mvfork to
	 * ensure this. the xpt entry for ssid is taken care of by
         * the page fault in v_cpfork.
         */
        for(k = firstf; k >= 0; k = pft_sidfwd(k))
        {
                ASSERT(pft_ssid(k) == fsid);
                if (rc = vcs_cpfork(fsid,*ssid,psid,pft_spage(k),&pscount))
		{
			/* We need to unpin the pages that were
			 * previously pinned by v_mvfork
			 */
			for (n = k; n >= 0; n = pft_sidfwd(n))
				vcs_unpin(fsid,pft_spage(n),SHORT_TERM);			
			vms_delete(*ssid);
			goto closeout;
		}
        }


closeout:

        (void)chgsr(VMMSR,savevmmsr);
        (void)chgsr(PTASR,saveptasr);
        return(rc);
}

/*
 * xptfixup(psid,fsid,pno)
 *
 * copy the xpt entry for pno in psid to the xpt entry for
 * the same page in fsid. clear the cdaddr field of the
 * parents xpt entry and mark as logically zero so that
 * copypage will get the right storage protect key for the
 * sibling when it page faults.
 *
 * RETURN VALUES
 *
 *      0       - ok
 *
 *      ENOMEM  - couldn't allocate xpt for fsid.
 */

static int
xptfixup(psid,fsid,pno,pscount)
int     psid;   	/* segment id of dummy parent */
int     fsid;   	/* segment id of forker */
int     pno;    	/* page number */
int *	pscount;	/* number of paging space blocks moved */
{
        int rc,fsidx,psidx;
        union   xptentry *pxpt, *fxpt, *v_findxpt();

        /* get scb indices.
         */

        fsidx = STOI(fsid);
        psidx = STOI(psid);

        /* get psid xpt pointer.
         */
        pxpt = v_findxpt(psidx,pno);
        ASSERT(pxpt != NULL);

        /* get fsid xpt pointer. allocate xpt if necessary.
         */

        if ((fxpt = v_findxpt(fsidx,pno))  == NULL)
        {
                rc = vcs_growxpt(fsidx,pno);
		assert(rc == 0);
                fxpt = v_findxpt(fsidx,pno);
        }

        /* fsid gets parents xptentry. we clear the cdaddr and 
         * mapblk fields of parents and set its zerobit so that
         * the page fault in the sibling in copypage will
         * get the right storage protect key (also a page of
         * zeros).
         */

	fxpt->word = pxpt->word;
	pxpt->cdaddr  = 0;
	pxpt->mapblk = 0;
	pxpt->zerobit = 1;
	
	/*
	 * adjust disk block counts in both scbs if not vmapped.
	 */
	if (!fxpt->mapblk && fxpt->cdaddr)
		*pscount += 1;
}
