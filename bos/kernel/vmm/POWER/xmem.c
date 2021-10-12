static char sccsid[] = "@(#)99	1.43.2.8  src/bos/kernel/vmm/POWER/xmem.c, sysvmm, bos411, 9435B411a 8/30/94 16:25:33";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:	xmattach, xmdetach
 *		xmemin, xmemout,
 *		xmemqflush, xmemacc
 *
 * ORIGINS: 27, 83
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
#include <sys/errno.h>
#include <sys/mstsave.h>
#include <sys/low.h>
#include <sys/intr.h>
#include <sys/user.h>
#include <sys/xmem.h>
#include <sys/adspace.h>
#include <sys/inline.h>
#include <sys/syspest.h>

/*
 * NAME: xmemqra
 *
 * FUNCTION: Query the real address of a buffer
 *
 * EXECUTION ENVIRONMENT:
 *
 * This may be called from a process or an interrupt handler.
 *
 * NOTES:
 *
 * This is an internal kernel routine. It should not be used, unless
 * absolutely necessary, as insulating code from knowledge of real addresses
 * is of paramount importance, due, in particular, to consistency
 * problems that may arise, as well as reference/change bit problems, etc.
 *
 * RETURN VALUE DESCRIPTION:
 *      real address    successful - real address returned
 *      -1              unsuccessful - not in real memory (maybe I/O)
 *
 */

caddr_t
xmemqra(dp, xaddr)
struct xmem     *dp;                    /* cross memory descriptor */
caddr_t         xaddr;                  /* transfer address */
{
	uint sid;
	uint xsreg, dsreg, raddr;

        /* disallow if descriptor not in use
         */
        assert( dp->aspace_id != XMEM_INVAL );

	/* get segment id.
	 */
        if (dp->aspace_id == XMEM_GLOBAL)
        {
                sid = SRTOSID(mfsri(xaddr));
        }
	else if (dp->aspace_id != XMEM_PROC2)
        {
                sid = SRTOSID(dp->subspace_id);
        }
	else
	{
		xsreg = (uint)xaddr >> L2SSIZE;
		dsreg = (uint)dp->uaddr >> L2SSIZE;
		if (xsreg == dsreg)
			sid = SRTOSID(dp->subspace_id);
		else
		{
			assert(xsreg == dsreg + 1)
			sid = SRTOSID(dp->subspace_id2);
		}
	}


	raddr = lqra(sid,((uint)xaddr & SOFFSET));
	assert((int) raddr != -1);
	return((caddr_t) raddr);

}

/*
 * NAME: xmemqflush(dp, xaddr)
 *
 * FUNCTION: Flush the memory cache for the page containing the specified
 *           effective address and return the real address of xaddr
 *
 * EXECUTION ENVIRONMENT:
 *
 * This may be called from a process or an interrupt handler.
 *
 * NOTES:
 * This is an internal kernel routine.
 *
 * RETURN VALUE DESCRIPTION:
 *	real address	successful - real address returned
 *	-1		unsuccessful - not in real memory
 */

caddr_t
xmemqflush(dp, xaddr)
struct xmem     *dp;                    /* cross memory descriptor */
caddr_t         xaddr;                  /* transfer address */
{
	uint sid;
	uint srsave;
	uint srval;
	caddr_t faddr;
	uint xsreg, dsreg;

        /* disallow if descriptor not in use
         */
        assert( dp->aspace_id != XMEM_INVAL );

	/* get segment id.
	 */
        if (dp->aspace_id == XMEM_GLOBAL)
        {
		srval = mfsri(xaddr);
        }
	else if (dp->aspace_id != XMEM_PROC2)
        {
                srval = dp->subspace_id;
        }
	else
	{
		xsreg = (uint)xaddr >> L2SSIZE;
		dsreg = (uint)dp->uaddr >> L2SSIZE;
		if (xsreg == dsreg)
			srval = dp->subspace_id;
		else
		{
			assert(xsreg == dsreg + 1)
			srval = dp->subspace_id2;
		}
	}
	sid = SRTOSID(srval);

	/* round xaddr down to page boundary
	 */
	faddr = (caddr_t) (((uint)xaddr) & (~POFFSET));

        /* make buffer addressable and flush it.
         */
	srsave = chgsr(TEMPSR,srval);
	faddr = (caddr_t) (((uint)faddr & SOFFSET) | ( TEMPSR << L2SSIZE ));
	vm_cflush(faddr, PSIZE);
	(void)chgsr(TEMPSR, srsave );

	/* translate and return real address
	 */
	faddr = lqra(sid,((uint)xaddr & SOFFSET));
	assert((int) faddr != -1);
	return(faddr);

}

/*
 * xmattach(vaddr,count,dp,segflag)
 * 
 * establish a cross memory descriptor for later use with xmemin() or
 * xmemout().
 *
 * input paramters:
 *
 *	vaddr	- 32 bit address of first byte of a buffer.
 *	count	- number of bytes in buffer. 
 *	dp	- pointer to the cross memory descriptor.
 *	segflag - defines whether vaddr is used as a system or
 *		  user address.
 *
 * the buffer must be wholly contained in a single segment of
 * 256 megabytes.
 *
 * RETURN VALUE DESCRIPTION:
 *
 *      XMEM_SUCC       successful, and the cross memory descriptor is
 *                      initialized accordingly.
 *
 *      XMEM_FAIL       buffer crosses a segment boundary,
 *                      or xmem descriptor not XMEM_INVAL on input.
 */

int
xmattach(vaddr,count,dp,segflag)
caddr_t		vaddr;   
int             count; 
struct xmem     *dp;   
int	segflag;
{
	int cross;
	uint sid, srval, srval2, sidx, sidx2, srsave;
	caddr_t endaddr;
	int rc = XMEM_SUCC;

	/* count must be >= 0 and <= 256MB
	 * xmem descriptor must be in invalid-state.
	 */
        if ( count < 0 || count > SEGSIZE || dp->aspace_id != XMEM_INVAL)
                return(XMEM_FAIL);

        /* Get srval corresponding to vaddr and segflag.
	 * We don't need to protect the process private segment.
         */
	if (segflag == SYS_ADSPACE)
		srval = mfsri(vaddr);
	else if (((uint)vaddr >> SEGSHIFT) == PRIVSEG)
		srval = as_getsrval(&U.U_adspace, vaddr);
	else
		srval = as_geth(&U.U_adspace, vaddr);

	/* check its validity
	 */
	if (checksrval(srval))
                return(XMEM_FAIL);

	/* determine if user buffer crosses segment boundary.
	 */
	endaddr = (count == 0) ? vaddr : vaddr + count - 1;
	cross = ((uint)vaddr >> L2SSIZE) != ((uint)endaddr >> L2SSIZE);

	if (cross)
	{
		/* get srval corresponding to endaddr.
		 * not valid to cross segment boundary in system space.
		 */
		if (segflag == SYS_ADSPACE)
			return(XMEM_FAIL);
		srval2 = as_geth(&U.U_adspace, endaddr);

		/* check its validity
		 */
		if (checksrval(srval2))
		{
			if (((uint)vaddr >> SEGSHIFT) != PRIVSEG)
				as_puth(&U.U_adspace, srval);
			return(XMEM_FAIL);
		}
	}

        /* get addressability to vmmdseg
         */
        srsave = chgsr(VMMSR,vmker.vmmsrval);
        sid = SRTOSID(srval);
        sidx = STOI(sid);

	/* Ensure segment is valid.
	 */
	assert(scb_valid(sidx));

	/* Make sure it is not in the process of being deleted.
	 */
	if (scb_delpend(sidx))
	{
		(void)chgsr(VMMSR,srsave);
		return(XMEM_FAIL);
	}

	if (cross)
	{
		sid = SRTOSID(srval2);
		sidx2 = STOI(sid);

		/* Ensure segment is valid.
		 */
		assert(scb_valid(sidx2));

		/* Make sure it is not in the process of being deleted.
		 */
		if (scb_delpend(sidx2))
		{
			(void)chgsr(VMMSR,srsave);
			return(XMEM_FAIL);
		}
	}

	/* increment the attach count (atomically)
 	 */
	fetch_and_add_h(&scb_xmemcnt(sidx), 1);

	/* initialize descriptor to indicate buffer in single
	 * segment and not globally addressable.
	 */
	dp->aspace_id	= XMEM_PROC;
	dp->subspace_id = srval;

	if (cross)
	{
		/* increment the attach count for 2nd segment.
		 * initialize descriptor to indicate buffer
		 * crosses segment boundary.
		 */
		fetch_and_add_h(&scb_xmemcnt(sidx2), 1);
		dp->aspace_id	 = XMEM_PROC2;
		dp->subspace_id2 = srval2;
		dp->uaddr	 = vaddr;
	}
	
closeout:
	(void)chgsr(VMMSR,srsave);
	if (segflag != SYS_ADSPACE)
	{
		if (((uint)vaddr >> SEGSHIFT) != PRIVSEG)
			as_puth(&U.U_adspace, srval);
		if (cross)
			as_puth(&U.U_adspace, srval2);
	}

        (void)chgsr(VMMSR,srsave);
        return(rc);
}

/*
 * NAME: xmdetach(dp)
 *
 * FUNCTION: Detach from a user buffer for cross memory operations
 *
 * EXECUTION ENVIRONMENT:
 *
 * Caller may be a process or an interrupt handler.
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION:
 *
 *      XMEM_SUCC       successful, and the cross memory descriptor is
 *                      set to XMEM_INVAL.
 *      XMEM_FAIL       xmem descriptor XMEM_INVAL on input.
 */

int
xmdetach(dp)
struct xmem     *dp;                    /* cross memory descriptor */
{
	uint  srval, srsave, sidx;

        /* check that descriptor is valid
         */
        if (dp->aspace_id == XMEM_INVAL)
		return (XMEM_FAIL);

        /* get addressability to vmmdseg
         */
        srsave = chgsr(VMMSR,vmker.vmmsrval);

	/* decrement attach count (atomically).
	 */
        srval = dp->subspace_id;
        sidx = STOI(SRTOSID(srval));
	if (sidx >= pf_hisid || !scb_valid(sidx))
	{
		(void)chgsr(VMMSR,srsave);
		return(XMEM_FAIL);
	}
	fetch_and_add_h( &scb_xmemcnt(sidx), - 1);

	/* determine if second sreg must be handled.
	 */
	if (dp->aspace_id == XMEM_PROC2)
	{
		/* decrement attach count (atomically).
		 */
		srval = dp->subspace_id2;
		sidx = STOI(SRTOSID(srval));
		if (sidx >= pf_hisid || !scb_valid(sidx))
		{
			(void)chgsr(VMMSR,srsave);
			return(XMEM_FAIL);
		}
		fetch_and_add_h( &scb_xmemcnt(sidx), - 1);
	}

        /* restore original view in VMMSR
	 * invalidate descriptor.
         */
        (void)chgsr(VMMSR,srsave);
        dp->aspace_id = XMEM_INVAL;

        return(XMEM_SUCC);
}

/*
 * NAME: xmemin
 *
 * FUNCTION: Transfer data from a user buffer to a kernel buffer.
 *
 * input parameters:
 *	uaddr	- address of user buffer.
 *	kaddr	- 32-bit address of target buffer.
 *	count   - number of bytes to transfer
 *	dp	- pointer to xmem descriptor. segment reg value
 *		  to address user buffer is in subspace_id field.
 *
 * EXECUTION ENVIRONMENT:
 *
 * This may be called from a process or an interrupt handler.
 *
 * RETURN VALUE DESCRIPTION:
 *
 *      XMEM_SUCC       successful
 *      XMEM_FAIL       an error occurred and the transfer was aborted
 *
 */

int
xmemin(uaddr, kaddr, count, dp)
caddr_t         uaddr;  
caddr_t         kaddr;   
int             count;  
struct xmem     *dp;   
{
	uint  srval, rc;
	caddr_t uaddr1;
	uint  count1, usreg, xsreg;

        /* disallow if descriptor is invalid.
         */
        if ( dp->aspace_id == XMEM_INVAL )
                return( XMEM_FAIL );

        /* disallow if count is invalid.
         */
	if ( count < 0 || count > SEGSIZE )
                return( XMEM_FAIL );

	/* set up for a copy and do the copy. 
	 */
        if (dp->aspace_id == XMEM_GLOBAL)
        {
		/* Global copy so source buffer is addressible
		 * and contained within one segment.
		 */
		rc = xbcopy(uaddr,kaddr,count);
        }
	else if (dp->aspace_id != XMEM_PROC2)
	{
		/* Source buffer is contained within one segment.
		 */
		srval = dp->subspace_id;
		uaddr1 = vm_att(srval, uaddr);
		rc = xbcopy(uaddr1,kaddr,count);
		vm_det(uaddr1);
	}
	else
	{
		/* Source buffer may cross a segment boundary.
		 */
		usreg = (uint)uaddr >> L2SSIZE;
		xsreg = (uint)dp->uaddr >> L2SSIZE;
		if (usreg == xsreg)
		{
			/* Source buffer starts in 1st segment.
			 * Transfer at most up to end of 1st segment.
			 */
			srval = dp->subspace_id;
			uaddr1 = vm_att(srval, uaddr);
			count1 = MIN(count, SEGSIZE - ((uint)uaddr & SOFFSET));
			rc = xbcopy(uaddr1,kaddr,count1);
			vm_det(uaddr1);
			if (rc == 0 && count1 < count)
			{
				/* First transfer succeeded and more remains.
				 * Transfer rest starting at 2nd segment.
				 */
				srval = dp->subspace_id2;
				uaddr1 = vm_att(srval, 0);
				rc = xbcopy(uaddr1,kaddr+count1,count-count1);
				vm_det(uaddr1);
			}
		}
		else if (usreg == xsreg + 1)
		{
			/* Source buffer contained within 2nd segment.
			 */
			srval = dp->subspace_id2;
			uaddr1 = vm_att(srval, uaddr);
			rc = xbcopy(uaddr1,kaddr,count);
			vm_det(uaddr1);
		}
		else
		{
			/* Source buffer not within either segment.
			 */
			return(XMEM_FAIL);
		}
	}

	return ((rc == 0) ? XMEM_SUCC : XMEM_FAIL);
}

/*
 * NAME: xmemout
 *
 * FUNCTION: Transfer data from a kernel buffer to a user buffer.
 *
 * input parameters:
 *	kaddr	- 32-bit address of source.
 *	uaddr	- address of user buffer.
 *	count   - number of bytes to transfer
 *	dp	- pointer to xmem descriptor. segment reg value
 *		  to address user buffer is in subspace_id field.
 *	
 *
 * EXECUTION ENVIRONMENT:
 *
 * This may be called from a process or an interrupt handler.
 *
 * RETURN VALUE DESCRIPTION:
 *
 *      XMEM_SUCC       successful
 *      XMEM_FAIL       an error occurred and the transfer was aborted
 *
 */
xmemout(kaddr, uaddr, count, dp)
caddr_t         kaddr;  
caddr_t         uaddr; 
int             count;
struct xmem     *dp; 
{
	uint  srval, rc;
	caddr_t uaddr1;
	uint  count1, usreg, xsreg;

        /* disallow if descriptor is invalid.
         */
        if ( dp->aspace_id == XMEM_INVAL )
                return( XMEM_FAIL );

        /* disallow if count is invalid.
         */
	if ( count < 0 || count > SEGSIZE )
                return( XMEM_FAIL );

	/* set up for a copy and do the copy. 
	 */
        if (dp->aspace_id == XMEM_GLOBAL)
        {
		/* Global copy so target buffer is addressible
		 * and contained within one segment.
		 */
		rc = xbcopy(kaddr,uaddr,count);
        }
	else if (dp->aspace_id != XMEM_PROC2)
	{
		/* Target buffer is contained within one segment.
		 */
		srval = dp->subspace_id;
		uaddr1 = vm_att(srval, uaddr);
		rc = xbcopy(kaddr,uaddr1,count);
		vm_det(uaddr1);
	}
	else
	{
		/* Target buffer may cross a segment boundary.
		 */
		usreg = (uint)uaddr >> L2SSIZE;
		xsreg = (uint)dp->uaddr >> L2SSIZE;
		if (usreg == xsreg)
		{
			/* Target buffer starts in 1st segment.
			 * Transfer at most up to end of 1st segment.
			 */
			srval = dp->subspace_id;
			uaddr1 = vm_att(srval, uaddr);
			count1 = MIN(count, SEGSIZE - ((uint)uaddr & SOFFSET));
			rc = xbcopy(kaddr,uaddr1,count1);
			vm_det(uaddr1);
			if (rc == 0 && count1 < count)
			{
				/* First transfer succeeded and more remains.
				 * Transfer rest starting at 2nd segment.
				 */
				srval = dp->subspace_id2;
				uaddr1 = vm_att(srval, 0);
				rc = xbcopy(kaddr+count1,uaddr1,count-count1);
				vm_det(uaddr1);
			}
		}
		else if (usreg == xsreg + 1)
		{
			/* Target buffer contained within 2nd segment.
			 */
			srval = dp->subspace_id2;
			uaddr1 = vm_att(srval, uaddr);
			rc = xbcopy(kaddr,uaddr1,count);
			vm_det(uaddr1);
		}
		else
		{
			/* Target buffer not within either segment.
			 */
			return(XMEM_FAIL);
		}
	}

	return ((rc == 0) ? XMEM_SUCC : XMEM_FAIL);
}



/*
 * NAME: xmemacc(dp, raddr, modify)
 *
 * FUNCTION: Determine the access allowed for a page.
 *
 * EXECUTION ENVIRONMENT:
 *
 * This may be called from a process or an interrupt handler.
 *
 * NOTES:
 *
 * This is an internal kernel routine.
 *
 * RETURN VALUES:
 *
 *	XMEM_RDONLY - read only access allowed.
 *
 *	XMEM_RDWR   - read/write access allowed.
 *
 *	XMEM_NOACC  - no access allowed. 
 */

int
xmemacc(dp, xaddr, modify)
struct xmem     *dp;                    /* cross memory descriptor */
caddr_t         xaddr;                  /* transfer address */
int		modify;			/* non-zero if page modified */
{
        uint   srvmsave,srval;
        int    nfr,srkey,rc;
	int    ipri;
	uint   sid, pno;
	uint   ppkey, attr;
	uint   xsreg, dsreg;


        /* make vmmdseg addressable.
         */
        srvmsave = chgsr(VMMSR,vmker.vmmsrval);

        /* determine sid and page number in segment.
         */
        if (dp->aspace_id == XMEM_GLOBAL)
        {
                srval = mfsri(xaddr);
        }
	else if (dp->aspace_id != XMEM_PROC2)
        {
                srval = dp->subspace_id;
        }
	else
	{
		xsreg = (uint)xaddr >> L2SSIZE;
		dsreg = (uint)dp->uaddr >> L2SSIZE;
		if (xsreg == dsreg)
			srval = dp->subspace_id;
		else
		{
			assert(xsreg == dsreg + 1)
			srval = dp->subspace_id2;
		}
	}

	/* get srval key.
	 */
	srkey = SRTOKEY(srval);

	sid  = SRTOSID(srval);
        pno  =  ((uint)xaddr & SOFFSET) >> L2PSIZE;

	/* 
	 * Disable interrupts and 
	 * get real frame number and page protection key.
	 */
	
	ipri = disable_ints();
	nfr = v_lookupx(sid,pno,&ppkey,&attr);
	enable_ints(ipri);

	assert(nfr >= 0);

	/* check for privileged access srval key
	 */
	if (srkey == VM_PRIV)
	{
		/* is the page readonly ?  if not, the page is r/w
		 * for privileged srval key or XMEM_GLOBAL. 
		 */
		if (ppkey == RDONLY)
		{
			rc = XMEM_RDONLY;
		}
		else
		{
			rc = XMEM_RDWR;
		}
		
	}
	/* unprivileged access srval key.
	 */
	else
	{
		/* check for read/write access.
		 */
		if (ppkey == UDATAKEY)
		{
			rc = XMEM_RDWR;
		}
		else 
		{
			/* check for no access. if page is not no
			 * access, the access allowed is readonly.
			 */
			if (ppkey == KERKEY)
				rc = XMEM_NOACC;
			else
				rc = XMEM_RDONLY;
		}
	}

	/* set mod bit if page is writable, and it will be written to
	 */
	if (modify && (rc == XMEM_RDWR))
	{
		ipri = disable_ints();
		if (pft_pagein(nfr) == 0 && pft_pageout(nfr) == 0)
		{
			SETMOD(nfr);
		}
		enable_ints(ipri);
	}

        /* restore VMMSR sreg.
         */
        (void)chgsr(VMMSR,srvmsave);
	return(rc);
}
