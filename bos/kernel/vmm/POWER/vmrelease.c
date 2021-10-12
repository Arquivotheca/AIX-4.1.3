static char sccsid[] = "@(#)51  1.24  src/bos/kernel/vmm/POWER/vmrelease.c, sysvmm, bos41J, 9524D_all 6/13/95 15:23:09";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS: 	vm_release, vm_releasep, vm_flushp, vm_invalidatep
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "vmsys.h"
#include <sys/inline.h>
#include <sys/errno.h>
#include <sys/syspest.h>

/*
 * vm_release(vaddr,nbytes)
 *
 * releases the virtual memory resources for the pages
 * specified.
 *
 * INPUT PARAMETERS
 *
 * (1) vaddr is the address of first byte.
 *
 * (2) nbytes the number of bytes starting at vaddr. nbytes
 *     must be non-negative. all of the bytes must be in the
 *     same physical segment.
 *
 * for working storage segments each page that intersects
 * the byte range [vaddr, vaddr + nbytes - 1] is logically
 * reset to zero, and the storage protect key is reset
 * to the default value, and associated disk blocks are
 * freed. none of the pages can be page-fixed.
 *
 * for non-working storage  segments the page frames associated
 * with the address range are discarded.
 *
 * this procedure should not be called for persistent segments
 * except by the file-system procedures associated with iclear
 * and truncating files.
 *
 * caller must have key 0 access to the segment.
 *
 *  RETURN  VALUES
 *      0       - ok
 *      EINVAL  -  no segment or i/o segment or nbytes illegal.
 *		-  or a page-fixed page was encountered in a
 *		-  working storage segment.
 *      EACCES  -  not key 0
 *	ENOMEM  - no space to allocate xpt.
 */

vm_release(vaddr,nbytes)
int  vaddr;     /* address of first byte */
int  nbytes;    /* number of bytes to release */
{

        uint npages,pfirst,plast, sid, rc;

	/* check input parms. get base segment id, first and
	 * last pages relative to base sid.
	 */
	if (rc = checkrange(vaddr,nbytes, &sid, &pfirst, &plast))
		return rc;

	/* vm_releasep does it
	 */
	npages = plast - pfirst + 1;
	return (vm_releasep(sid, pfirst, npages));
}


/*
 * vm_releasep(sid,pfirst,npages)
 *
 * releases the virtual memory resources for the page
 * range specified.
 *
 * INPUT PARAMETERS
 *
 * (1) sid is the base segment identifier
 *
 * (2) pfirst is the first page number within the range.
 *     pfirst must be none negative.
 *
 * (3) npages the number of page starting at pfirst. npages
 *     must be non-negative. all of the page must be in the
 *     same logical segment.
 *
 * for working storage segments each page that intersects
 * the page range [pfirst, pfirst + npages - 1] is logically
 * reset to zero, and the storage protect key is reset
 * to the default value, and associated disk blocks are
 * freed.
 *
 * for non-working storage  segments the page frames associated
 * with the address range are discarded.
 *
 * this procedure should not be called for persistent segments
 * except by the file-system procedures associated with iclear
 * and truncating files.
 *
 *
 *  RETURN  VALUES
 *      0       - ok
 *	EINVAL  - pfirst < 0 or npages < 0 or page interval
 *		  not in one segment or invalid segment id.
 *
 */

vm_releasep(sid,pfirst,npages)
int  sid;	/* segment identifier */
int  pfirst;    /* address of first page */
int  npages;    /* number of page to release */
{

        uint np,p,plast,first,last;
        uint sidx,rc,savevmsr,saveptasr;
	uint maxpage;
	int  chunksize;

	rc = 0;

	/* check input parms.
	 */
	if (pfirst < 0 || npages < 0 || sid == INVLSID)
		return EINVAL;

        /* get addressability to vmmdseg and ptaseg
         */
        savevmsr  = chgsr(VMMSR,vmker.vmmsrval);
        saveptasr = chgsr(PTASR,vmker.ptasrval);

	/* check npages
	 */
        sidx = STOI(sid);
	ASSERT(scb_valid(sidx));
	maxpage = (scb_wseg(sidx)) ? MAXVPN : MAXFSIZE/PSIZE - 1;
	plast = pfirst + npages - 1;
	if ( plast > maxpage)
	{
		rc = EINVAL;
		goto closeout;
	}


        /* release the pages in interval [pfirst,pfirst + npages - 1]
         */
        if(!scb_wseg(sidx))
	{
		/*
		 * Release pages 'chunksize' at a time to avoid
		 * remaining disabled for long periods.
		 */
		plast = MIN(plast, scb_maxvpn(sidx));
	        for (p = pfirst; p <= plast; p += vmker.pd_npages)
       	 	{
			chunksize = MIN(plast - p + 1, vmker.pd_npages);
                	if (rc = vcs_release(sid, p, chunksize, V_KEEP))
				break;
        	}
		goto closeout;
	}

	/* working storage segment
	 */
	rc = 0;
        for(p = pfirst; p <= plast; p += np)
        {
                /* calculate number of pages from p to end of
                 * interval covered by same direct xpt block.
                 */
                first  = p & (~(XPTENT - 1));
                last   = first + XPTENT - 1;
                last = MIN(last,plast);
                np = last - p + 1;

                /* if xpt is NULL growxpt if segment has an
		 * ancestor. this is necessary so that the
		 * pages be marked logically zero in the xpt
		 * to prevent going to ancestor for page.
		 * there is no need to release past maxvpn
		 * or before minvpn because ancestor has
		 * nothing in this range.
                 */
		retry:
		if (v_findxpt(sidx,p) == NULL)
		{
			if (scb_parent(sidx) == 0)
				continue;
			if (p > scb_maxvpn(sidx) && last < scb_minvpn(sidx))
				continue;
			if (rc = vcs_growxpt(sidx,p))
				break;
		}

		rc = vcs_release(sid,p,np,V_KEEP|V_CHECKXPT);

		if (rc == VM_NOXPT)
			goto retry;	/* look for xpt again */

		if (rc)
			break;
        }

        closeout:
        (void)chgsr(VMMSR,savevmsr);
        (void)chgsr(PTASR,saveptasr);
        return(rc);
}

/*
 * vm_flushp(sid,pfirst,npages)
 *
 * flushes the range of pages specified.
 *
 * pageout will be initiated for the page range and the
 * the virtual memory resources for the page range will be
 * released. 
 *
 * INPUT PARAMETERS
 *
 * (1) sid is the base segment identifier
 *
 * (2) pfirst is the first page number within the range.
 *     pfirst must be none negative.
 *
 * (3) npages the number of page starting at pfirst. npages
 *     must be non-negative. all of the page must be in the
 *     same logical segment.
 *
 * this procedure should not be called only for working storage
 * segments.
 *
 *
 *  RETURN  VALUES
 *      0       - ok
 *	EINVAL  - pfirst < 0 or npages < 0 or page interval
 *		  not in one segment or invalid segment id or
 *		  invalid segment type.
 *
 */

vm_flushp(sid,pfirst,npages)
int  sid;	/* segment identifier */
int  pfirst;    /* address of first page */
int  npages;    /* number of page to flush */
{

        uint sidx,rc,savevmsr;


	/* check input parms.
	 */
	if (pfirst < 0 || npages < 0 || sid == INVLSID)
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

	/* check npages.
	 */
	if ( pfirst + npages - 1 > (MAXFSIZE/PSIZE - 1))
	{
		rc = EINVAL;
		goto closeout;
	}


        /* flush the pages in interval [pfirst,pfirst + npages - 1]
         */
	rc = vcs_flush(sid,pfirst,npages);

        closeout:
        (void)chgsr(VMMSR,savevmsr);
        return(rc);
}


/*
 * vm_invalidatep(sid,pfirst,npages)
 *
 * invalidate the page range specified for a non-journalled persistent
 * segment or client segment.
 *
 * INPUT PARAMETERS
 *
 * (1) sid is the base segment identifier
 *
 * (2) pfirst is the first page number within the range.
 *     pfirst must be none negative.
 *
 * (3) npages the number of page starting at pfirst. npages
 *     must be non-negative. all of the page must be in the
 *     same logical segment.
 *
 * The page frames associated with the address range are discarded
 * unless the associated disk block is unitialized in which case
 * the page is zero-filled and marked modified.  For deferred-update
 * segments any extension memory disk block is freed.
 *
 * This procedure is only called by vm_map_sync to invalidate pages.
 *
 *  RETURN  VALUES
 *      0       - ok
 *	EINVAL  - pfirst < 0 or npages < 0 or page interval
 *		  not in one segment or invalid segment id.
 *
 */

vm_invalidatep(sid,pfirst,npages)
int  sid;	/* segment identifier */
int  pfirst;    /* address of first page */
int  npages;    /* number of page to release */
{
        uint sidx,rc,savevmsr,saveptasr;
	uint maxpage;

	rc = 0;

	/* check input parms.
	 */
	if (pfirst < 0 || npages < 0 || sid == INVLSID)
		return EINVAL;

        /* get addressability to vmmdseg and ptaseg
         */
        savevmsr  = chgsr(VMMSR,vmker.vmmsrval);
        saveptasr = chgsr(PTASR,vmker.ptasrval);

        sidx = STOI(sid);
	if (!scb_valid(sidx) || scb_wseg(sidx))
	{
		rc = EINVAL;
		goto closeout;
	}

	/* check npages
	 */
	maxpage = MAXFSIZE/PSIZE - 1;
	if ( pfirst + npages - 1 > maxpage)
	{
		rc = EINVAL;
		goto closeout;
	}

        /* invalidate the pages in interval [pfirst,pfirst + npages - 1]
         */
	rc = vcs_invalidate(sid,pfirst,npages);

        closeout:
        (void)chgsr(VMMSR,savevmsr);
        (void)chgsr(PTASR,saveptasr);
        return(rc);
}
