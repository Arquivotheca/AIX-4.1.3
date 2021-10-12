static char sccsid[] = "@(#)93	1.6  src/bos/kernel/vmm/POWER/vmprotect.c, sysvmm, bos411, 9428A410j 2/9/94 15:10:54";

/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:	vm_protect, vm_protectp 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "vmsys.h"
#include <sys/intr.h>
#include <sys/inline.h>
#include <sys/errno.h>

/*
 * vm_protect(vaddr,nbytes,key)
 *
 * sets storage protection key for pages for an address range
 * in a working storage segment.
 *
 * set storage protection key to the specified key for each page 
 * that intersects the interval [vaddr, vaddr + nbytes - 1]. all
 * of the bytes must be in the same segment and nbytes must be
 * greater than zero. the protection key is not set if a page is
 * logically zero and the default storage key equals the  specified
 * key. 
 *
 * RETURN VALUE
 *
 *      0       - ok
 *
 *      EINVAL  -  nbytes or vaddr invalid or segment is NOT
 *		   a working storage segment.
 *
 *      ENOSPC  -  out of paging space blocks.
 *
 */

vm_protect(vaddr,nbytes,key)
uint    vaddr;  /* 32-bit address of first byte */
int     nbytes; /* number of bytes to protect */
int     key; 	/* key to be used for target range */
{
	uint    offset,p,nextp,srval,savevmsr,saveptasr;
        int     sid,sreg,sidx,pno,rc,bytes;

        /* check offset and nbytes ok.
         */
        offset = vaddr & SOFFSET;
        if (offset + nbytes  >  SEGSIZE || nbytes <= 0)
                return(EINVAL);

        /* check srval for validity. 
         */
        sreg = (vaddr & SREGMSK) >> L2SSIZE;
	srval = ldfsr(sreg);
	if (rc = checksrval(srval))
		return rc;

        /* get addressability to vmmdseg and ptaseg
         */
        savevmsr  = chgsr(VMMSR,vmker.vmmsrval);
        saveptasr = chgsr(PTASR,vmker.ptasrval);

        /* get sid and sidx.
         */
        sid = SRTOSID(srval);
	sidx = STOI(sid);

        /* check for working storage segment.
         */
	if (!scb_wseg(sidx))
	{
		rc = EINVAL;
		goto closeout;
	}

        /* set the storage key for the pages.
         */
        for(p = offset; p < offset + nbytes; p += bytes)
        {
		nextp = (p + PSIZE) & (~POFFSET);
		bytes = nextp - p;
		pno = p >> L2PSIZE;

        	/* call vcs_protect() to set the key.
          	 */
                if (rc = vcs_protect(sid,pno,key))
			break;
        }


        closeout:
        (void)chgsr(VMMSR,savevmsr);
        (void)chgsr(PTASR,saveptasr);
        return(rc);
}

/*
 * vm_protectp(sid,pfirst,npages,key)
 *
 * set the storage protection key for a page range in a
 * persistent or client segment.
 *
 * set storage protection key to the specified key for each page 
 * that intersects the interval [pfirst, pfirst + npages - 1]. all
 * all of the pages must be in the same segment and pfirst and 
 * npages must be non-negative.
 *
 * RETURN VALUE
 *
 *      0       - ok
 *
 *      EINVAL  -  sid or pno or npages invalid, or segment is 
 *		   a working storage segment.
 *
 *
 */

vm_protectp(sid,pfirst,npages,key)
int sid;	/* base segment identifier */
int pfirst;	/* first page number in range */ 
int npages;	/* number of pages in range */
int key; 	/* key to be used for target range */
{
        int     sidx,p,plast,rc;
	uint    savevmsr;

	/* check input parms
	 */
	if (npages < 0 || pfirst < 0)
		return EINVAL;

        /* get addressability to vmmdseg.
         */
        savevmsr = chgsr(VMMSR,vmker.vmmsrval);

        /* check for invalid segment type.
         */
	sidx = STOI(sid);
	if (!scb_valid(sidx) || scb_wseg(sidx))
	{
		rc = EINVAL;
		goto closeout;
	}

	/* check npages 
	 */
	if (pfirst + npages - 1 > (unsigned)( MAXFSIZE/PSIZE - 1))
	{
		rc = EINVAL;
		goto closeout;
	}


        /* call vcs_protect() to set the protection key.
         */
	plast = pfirst + npages - 1;
	for (p = pfirst; p <= plast; p++)
        {
                if (rc = vcs_protectp(sid,p,key))
			break;
        }


        closeout:
        (void)chgsr(VMMSR,savevmsr);
        return(rc);
}
