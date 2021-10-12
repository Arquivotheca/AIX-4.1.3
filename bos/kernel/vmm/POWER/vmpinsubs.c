static char sccsid[] = "@(#)42  1.14.1.12  src/bos/kernel/vmm/POWER/vmpinsubs.c, sysvmm, bos41J, 9507A 2/2/95 15:58:55";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS: 	pin, ltpin, unpin, ltunpin, pinx, checksrval
 *
 * ORIGINS: 27, 83
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

#include "vmsys.h"
#include <sys/intr.h>
#include <sys/errno.h>
#include <sys/syspest.h>
#include <sys/atomic_op.h>
#include <sys/except.h>

#define	AtomicAdd(x,y)	(fetch_and_add(x,y))

/*
 * pin_com(vaddr,nbytes,typ)
 *
 * pin pages in a working storage segment.
 *
 * adds one to the pin count for each page that intersect
 * the interval [vaddr, vaddr + nbytes - 1]. all of the
 * bytes must be in the same segment and nbytes must be
 * greater than zero.
 *
 * RETURN VALUE
 *
 *      0       - ok
 *
 *      EINVAL  -  nbytes or vaddr invalid or segment is NOT
 *		   a working storage segment.
 *
 *	ENOMEM  -  too many pages already pinned.
 *
 *	ENOSPC  -  out of paging space resource.
 *
 */

static int
pin_com(vaddr,nbytes,typ)
uint    vaddr;  /* 32-bit address of first byte */
int     nbytes; /* number of bytes starting from vaddr */
int	typ;	/* pin type: SHORT_TERM or LONG_TERM */
{
	uint addr, nextaddr, srval;
        int sidx, sid, sreg, rc, bytes;
	int savevmsr;

        /* check offset and nbytes ok.
         */
        if ((vaddr & SOFFSET) + nbytes  >  SEGSIZE || nbytes <= 0)
                return(EINVAL);

        /* check srval for validity. 
         */
        sreg = (vaddr & SREGMSK) >> L2SSIZE;
	srval = ldfsr(sreg);
	if (rc = checksrval(srval))
		return rc;

        /* get addressability to vmmdseg.
         */
        savevmsr = chgsr(VMMSR,vmker.vmmsrval);

	/* check for working storage or short-term pin of mmap range.
	 */
        sid = SRTOSID(srval);
	sidx = STOI(sid);
	if (!(scb_wseg(sidx) || (scb_mseg(sidx) && typ == SHORT_TERM)))
	{
        	(void)chgsr(VMMSR,savevmsr);
		return(EINVAL);
	}

        /* pin the pages.
         */
        for (addr = vaddr; addr < vaddr + nbytes; addr += bytes)
        {
		nextaddr = (addr + PSIZE) & (~POFFSET);
		bytes = nextaddr - addr;

		if (rc = pinpage(addr,sid,typ))
		{
			if (typ == SHORT_TERM)
				unpin(vaddr,addr - vaddr);
			else /* LONG_TERM */
				ltunpin(vaddr,addr - vaddr);
			break;
		}
        }

        (void)chgsr(VMMSR,savevmsr);
        return(rc);
}

pin(vaddr,nbytes)
uint    vaddr;  /* 32-bit address of first byte */
int     nbytes; /* number of bytes starting from vaddr */
{
	return( pin_com( vaddr,nbytes,SHORT_TERM ) );
}

ltpin(vaddr,nbytes)
uint    vaddr;  /* 32-bit address of first byte */
int     nbytes; /* number of bytes starting from vaddr */
{
	return( pin_com( vaddr,nbytes,LONG_TERM ) );
}

/*
 * unpin_com(vaddr,nbytes,typ)
 *
 * subtracts one from the pin count for each page that intersects
 * the interval [vaddr, vaddr + nbytes - 1]. all of the bytes
 * must be in the same segment and nbytes must be greater than
 * zero.  pages should have been previously pinned using pin or
 * ltpin.  caller is assumed to be in supervisor state.
 *
 * unpin may be called (i.e. unpin_com(SHORT_TERM)) from any processor
 * priority.  ltunpin cannot: it may need to allocate backing storage
 * and can be waitlisted, and thus must run at INTBASE.  also, unpin of
 * an mmap region must run at INTBASE.
 *
 * RETURN VALUE
 *
 *      0       - ok
 *
 *      EINVAL  -  nbytes or vaddr invalid or an unfixed page
 *                 was encountered.
 *
 */

static int
unpin_com(vaddr,nbytes,typ)
uint    vaddr;  /* 32-bit address of first byte */
int     nbytes; /* number of bytes starting from vaddr */
int	typ;	/* pin type: SHORT_TERM or LONG_TERM */
{
        int     sid, pno, sreg, nfr, savevmsr, rc = 0;
	uint    offset, first, last, intpri, pincnt;
	int	sidx;

        /* determine if offset and nbytes are ok
         */
        offset = vaddr & SOFFSET;
        if (offset + nbytes  >  SEGSIZE || nbytes <= 0)
                return(EINVAL);

        /* get addressability to vmmdseg.
         */
        savevmsr = chgsr(VMMSR,vmker.vmmsrval);

        /* get sid
         */
        sreg = (vaddr & SREGMSK) >> L2SSIZE;
        sid = SRTOSID(ldfsr(sreg));
	sidx = STOI(sid);

        first = offset >> L2PSIZE;
        last = (offset + nbytes - 1) >> L2PSIZE;

	if (typ == SHORT_TERM && !scb_mseg(sidx))
	{
	        for (pno = first; pno <= last; pno += 1)
		{
			intpri = disable_ints();
			nfr = v_lookup(sid,pno);

			/* make sure that the page is in an inuse state and
			 * is pinned.  pin/unpin on page 1 (FBANCH) will
			 * corrupt the free list.
			 */
        		if(nfr < 0 || nfr == FBANCH || !pft_inuse(nfr) ||
			   (pft_pincntst(nfr) == 0))
			{
        			enable_ints(intpri);
        			(void)chgsr(VMMSR,savevmsr);
        			return(EINVAL);
			}

			/* decrement pincount and adjust count of pinned pages.
			 * if the short-term pincount underflows, it 'borrows'
			 * from long-term.
			 */
			pincnt  = AtomicAdd(&pft_pincount(nfr), STPIN_DEC);
			pincnt += STPIN_DEC;
			assert( (pincnt & STPIN_OVERFLOW) == 0);

			/* If the new pincount is zero, adjust pf_pfavail.
			 * This checks long-term and short-term counts at once.
			 */
			if (pincnt == 0)
				(void)AtomicAdd(&pf_pfavail, 1);

        		enable_ints(intpri);
		}
        }
	else /* typ == LONG_TERM or unpin of mmap region */
	{
		for (pno = first; pno <= last; pno += 1)
		{
			rc = vcs_unpin_excp(sid,pno,typ);
			if (rc != 0)
				break;
		}
	}

        (void)chgsr(VMMSR,savevmsr);
        return(rc);
}

unpin(vaddr,nbytes)
uint    vaddr;  /* 32-bit address of first byte */
int     nbytes; /* number of bytes starting from vaddr */
{
	return(unpin_com(vaddr,nbytes,SHORT_TERM));
}

ltunpin(vaddr,nbytes)
uint    vaddr;  /* 32-bit address of first byte */
int     nbytes; /* number of bytes starting from vaddr */
{
	return(unpin_com(vaddr,nbytes,LONG_TERM));
}

/*
 * pinx(vaddr,nbytes)
 *
 * pin pages in a persistent or client segment or in a
 * working storage segment in the same way as pin() above.
 *
 * for client and persistent  segments this procedure 
 * should only be used by PLOCK to pin a text segment. 
 *
 * adds one to the pin count for each page that intersect
 * the interval [vaddr, vaddr + nbytes - 1]. all of the
 * bytes must be in the same segment and nbytes must be
 * greater than zero . 
 *
 *
 * RETURN VALUE
 *
 *      0       - ok
 *
 *      EINVAL  -  nbytes or offset or sid invalid or
 *		   not presistent or client segment.
 *
 *	ENOMEM  - too many pages already pinned
 *
 */

pinx(vaddr,nbytes)
uint    vaddr;  /* 32-bit address of first byte */
int     nbytes; /* number of bytes starting from vaddr */
{
	uint addr, nextaddr, srval;
        int sidx, sid, sreg, rc, bytes, savevmsr;

        /* check offset and nbytes ok.
         */
        if ((vaddr & SOFFSET) + nbytes  >  SEGSIZE || nbytes <= 0)
                return(EINVAL);

        /* check srval for validity. 
         */
        sreg = (vaddr & SREGMSK) >> L2SSIZE;
	srval = ldfsr(sreg);
	if (rc = checksrval(srval))
		return rc;

        /* get addressability to vmmdseg.
         */
        savevmsr = chgsr(VMMSR,vmker.vmmsrval);

	/* check for persistent or client segment.
	 */
        sid = SRTOSID(srval);
	sidx = STOI(sid);
	if (!(scb_pseg(sidx) || scb_clseg(sidx)))
	{
        	(void)chgsr(VMMSR,savevmsr);
		return(EINVAL);
	}

        /* pin the pages.
         */
        for (addr = vaddr; addr < vaddr + nbytes; addr += bytes)
        {
		nextaddr = (addr + PSIZE) & (~POFFSET);
		bytes = nextaddr - addr;

		if (rc = pinpage(addr,sid,SHORT_TERM))
		{
			unpin(vaddr,addr - vaddr);
			break;
		}
        }

        (void)chgsr(VMMSR,savevmsr);
        return(rc);

}

/* 
 * pinpage(vaddr,sid,typ)
 *
 * pin the page associated with vaddr and sid.
 *
 * the page will either be pinned directly or through a call to vcs_pin().
 * the page will be pinned directly if the page is memory resident 
 * (after touching the page and disabling interrupts), pinning the page
 * will not cause the pinned page threshold to be reached, and, for 
 * working storage segment, the page has a disk block allocated.  if
 * the page cannot be pinned directly, vcs_pin() is called to pin the page.
 * The pin fast-path runs only on UP systems.  On UP, a disable is used to
 * serialize access to the pincounts, etc. and thus atomic operations are
 * not required.
 *
 * RETURN VALUE
 *
 *      0       - successful
 *      EINVAL	- invalid address
 *	ENOMEM  - too many pages already pinned
 *	ENOSPC  - out of paging space resource.
 *
 */

static
pinpage(vaddr,sid,typ)
uint vaddr;
int  sid;
int  typ;
{
	uint intpri, nfr, pincnt;
	int  rc, pno;

	/* The fast-path only works when disable_ints prevents pageouts
	 * system-wide.  This is not the case on a multiprocessor system,
	 * and no MP lock is held here.
	 * System-wide use of Long Term pin runs almost exclusively on pages
	 * which are not pinned.  With this knowledge, for performance reasons,
	 * there is no fast path for Long Term pin.
	 */


	pno = (vaddr & SOFFSET) >> L2PSIZE;

#ifndef _POWER_MP

	if (typ != LONG_TERM)
	{
		/* disable.
		 */
		intpri = disable_ints();

		/* touch the page.
		 */
		if (rc = touchrc(vaddr))
		{
			switch(rc)
			{
			case ENOSPC:
			case EIO:
				break;
			default:
				rc = EINVAL;
			}
			/* restore interrupt priority.
			 */
			enable_ints(intpri);
			return(rc);
		}

		nfr =  v_lookup(sid,pno);

		/* check if the page can be pinned here or if
		 * vcs_pin() must be used to pin the page.
		 */
		if (nfr >= 0 && pinok(nfr,STOI(sid)) == 0)
		{
			/* pin/unpin on page 1 (FBANCH) corrupts the free list
			 */
			if (nfr == FBANCH)
				return EINVAL;

			/* bump pincount and decrement count of
			 * non-pinned pages if new pinned page.
			 */
			pft_pincntst(nfr) += 1;
			assert((pft_pincntst(nfr) & STPIN_OVERFLOW)==0);

			/* if pinok found the page either long or
			 * short-term pinned, pf_pfavail is OK.
			 */
			if (pft_pincount(nfr) == STPIN_ONE)
				pf_pfavail += -1;

			enable_ints(intpri);
			return(0);
		}

		/* restore interrupt priority.
		 */
		enable_ints(intpri);
	}

#endif /* _POWER_MP */

	/* call vcs_pin() to pin the page using the call with built-in
	 * exception handler.
	 */
	rc = vcs_pin_excp(sid,pno,typ);

	/*
	 * We need to be careful to return only exception
	 * values within the range of errno.
	 */
	return(rc > EXCEPT_ERRNO ? EINVAL : rc);
}

/* 
 * pinok(nfr,sidx)
 *
 * check if page can be pinned outside of vcs_pin().
 *
 * RETURN VALUE
 *
 *      0       - page can be pinned without calling vcs_pin().
 *      1       - vcs_pin must be called to pin the page.
 *
 */

static
pinok(nfr,sidx)
uint nfr;
uint sidx;
{
	/* force pins of alias addresses to vcs_pin.
	 */
	if (scb_mseg(sidx))
		return(1);

	/* check if page is already pinned.  this catches long and short-term.
	 */ 
	if (pft_pincount(nfr) == 0)
	{
		/* check if this is a working storage page
		 * which has not been allocated a disk block
		 * (vmapped page or zero filled page created by
		 * a load with no store access).
		 */
		if (scb_wseg(sidx) && pft_dblock(nfr) == 0)
			return(1);

		/* check if we are at or above the pinned storage
		 * threshold.
		 */
		if (pf_pfavail <= vmker.pfrsvdblks)
			return(1);
	}

	return(0);
}

checksrval(srval)
uint srval;
{
	/* check for invalid sid 
	 */
        if (SRTOSID(srval) == INVLSID)
                return(EINVAL);

        /* check for iosegment
         */
        if (srval & IOSEGMENT)
                return(EINVAL);

	return 0;
}
