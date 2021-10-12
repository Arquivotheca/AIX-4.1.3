static char sccsid[] = "@(#)94  1.16  src/bos/kernel/vmm/POWER/vmwrite.c, sysvmm, bos411, 9428A410j 2/9/94 15:09:19";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS: 	vm_write,  checkrange
 *
 * ORIGINS: 27 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#include "vmsys.h"
#include <sys/inline.h>
#include <sys/errno.h>
#include <sys/syspest.h>

/*
 * vm_write(vaddr,nbytes,force)
 *
 * initiates pageout for the range of pages specified.
 *
 * INPUT PARAMETERS
 *
 * (1) vaddr is the address of first byte and nbytes the
 *     number of bytes starting at vaddr. nbytes must be
 *     non-negative and all of the bytes must be in the
 *     same physical segment .
 *
 * (2) force applies only to journalled segments and if
 *     true (non-zero) the pages are written to disk
 *     regardless of how recently they have been written.
 *
 * for journalled segments care should be taken that the
 * caller actually holds the write lock on the page. they
 * are NOT checked here.
 *
 * except for journalled segments the page out is initiated.
 * for journalled segments page outs are only initiated if
 * either the parameter force is true or for pages that have
 * not been "recently" written to disk. if  pageout is not
 * initiated the page is marked "homeok" so that it can be
 * written to its home disk address by the page replacement
 * code.
 *
 * for all segment types if the page is written it is written
 * to its home address.
 *
 * caller must have key 0 access to the segment.
 *
 *  RETURN  VALUES
 *
 *      0       - ok
 *
 *      EINVAL  -  no segment or i/o segment or nbytes illegal.
 *
 *      EACCES  -  not key 0
 *
 */

vm_write(vaddr,nbytes,force)
int  vaddr;     /* address of first byte */
int  nbytes;    /* number of bytes to release */
int  force;     /* write page regardless of when last written */
{

        uint sid, pfirst,plast, rc, npages;

	/* check validity of parms. get base sid, first and 
	 * last page relative to base sid.
	 */
	if (rc = checkrange(vaddr, nbytes, &sid, &pfirst, &plast))
		return rc;

	/* write them
	 */
	npages = plast - pfirst + 1;
	return (vcs_write(sid, pfirst, npages, force));
}

/*
 * checkrange(vaddr,nbytes,sid,pfirst,plast)
 *
 * checks that the range of addresses [vaddr, vaddr + nbytes -1]
 * is in one segment. sets sid to the corresponding base segment 
 * identifier. sets pfirst and plast to the first and last pages
 * numbers relative to base sid that intersect the range of pages.
 *
 * also checks that the segment is a normal VM segment and that
 * access key is zero.
 *
 * return values:
 *	0	- ok
 *	EINVAL  - nbytes < 0 or interval not in one segment or invalid 
 *		  segment.
 *	EACCESS - not key 0.
 */

int
checkrange(vaddr, nbytes, sid, pfirst, plast)
uint vaddr; 
int  nbytes;
uint * sid;
uint * pfirst;
uint * plast;
{
        uint srno,srval, offset, lastoffset, mysid;

        /* check for silly values of nbytes
         */
        if(nbytes < 0)
		return EINVAL;

        /* get srval. check validity of sid.
         */
        srno = vaddr >> L2SSIZE;
        srval = ldfsr(srno);
        if ((mysid = SRTOSID(srval)) == INVLSID)
                return(EINVAL);

        /* access key in sreg must be zero.
         */
        if(SRTOKEY(srval))
                return(EACCES);

        /* check for iosegment
         */
        if (srval & IOSEGMENT)
                return(EINVAL);

	/* check that range is in one segment.
	 */
        offset = vaddr & SOFFSET;
	lastoffset = offset + nbytes - 1;
	if ( (offset >> L2SSIZE) != (lastoffset >> L2SSIZE) )
		return EINVAL;

	/* everything is ok
	 * set sid to base segment id and pfirst and plast to
	 * pages relative to base sid.
	 */
	*sid = BASESID(mysid);
        *pfirst = (offset >> L2PSIZE) + SCBPNO(mysid,0);
	*plast  = (lastoffset >> L2PSIZE) + SCBPNO(mysid, 0);
	return 0;

}
/*
 * vm_writep(sid,pfirst,npages)
 *
 * initiates pageout for the range of pages specified .
 *
 * INPUT PARAMETERS
 *
 * (1) sid is the base segment identifier. 
 *
 * (2) pfirst is the first page number within the range.
 *     pfirst must be non-negative.
 *
 * (3) npages the number of page starting at pfirst. npages
 *     must be non-negative. all of the page must be in the
 *     same logical segment. 
 *
 * page out for modified pages in the page range [pfirst,
 * pfirst + npages - 1] is initiated.  Unchanged pages within
 * the range are left in memory with their reference bit set 
 * to zero.
 * 
 *
 *  RETURN  VALUES
 *      0       - ok
 *	EINVAL  - pfirst < 0 or npages < 0 or
 *	          pages not in one logical segment
 *	          or invalid segment id or type.
 *		  
 *
 */

vm_writep(sid,pfirst,npages)
int  sid;	/* segment identifier */
int  pfirst;    /* number of first page */
int  npages;    /* number of page to release */
{

        uint sidx,rc,savevmsr,saveptasr;
	uint maxpage;

	/* check input parms. 
	 */
	if (pfirst < 0 || npages < 0 || sid == INVLSID)
		return EINVAL;

        /* get addressability to vmmdseg and ptaseg
         */
        savevmsr  = chgsr(VMMSR,vmker.vmmsrval);
        saveptasr = chgsr(PTASR,vmker.ptasrval);

	/* check for valid npages
	 */
        sidx = STOI(sid);
	ASSERT(scb_valid(sidx));
	maxpage = (scb_wseg(sidx)) ? MAXVPN : MAXFSIZE/PSIZE - 1;
	if (pfirst + npages - 1 > maxpage)
	{
		rc = EINVAL;
		goto closeout;
	}

        /* write the pages in interval [pfirst,pfirst + npages - 1]
         */
	rc = vcs_write(sid, pfirst, npages,V_FORCE);

        closeout:
        (void)chgsr(VMMSR,savevmsr);
        (void)chgsr(PTASR,saveptasr);
        return(rc);
}
