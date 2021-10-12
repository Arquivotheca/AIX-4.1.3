static char sccsid[] = "@(#)61	1.4.1.9  src/bos/kernel/ios/uio.c, sysios, bos411, 9433A411a 8/12/94 10:16:30";
/*
 * COMPONENT_NAME: (SYSIOS) User I/O (uio) services
 *
 * FUNCTIONS:	uiomove		ureadc		uwritec
 *		uiomove_chksum	pinu		unpinu
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	kern_subr.c	7.1 (Berkeley) 6/5/86
 */

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/errno.h>
#include <sys/user.h>
#include <sys/syspest.h>
#include <sys/low.h>
#include <sys/adspace.h>
#include <vmm/vmsys.h>
#include <sys/malloc.h>

struct pinu_block	*pinu_free = NULL;	/* ptr to free list	*/
Simple_lock pinu_lock;		/* alloc and init is done in uio_init() */
				/* under #ifdef _POWER_MP */

/*
 * NAME:  uiomove
 *
 * FUNCTION:  Move the data described by the uio structure.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine runs in kernel mode under the process
 *	address space described by the uio structure.
 *
 * NOTES: 
 *	This service is the equivalent of the ATT iomove service.
 *	The difference is that parameters are passed in the uio
 *	structure instead of the u block.
 *
 * RETURN VALUE DESCRIPTION:
 *	0	successful
 *	EFAULT  bad address
 *	ENOMEM  out of memory
 *	ENOSPC  out of disk space
 * 	EIO	io error 
 *
 * IO errors for paging space and running out of paging space are
 * also reported as signals.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	uiocopyin
 *	uiocopyout
 *	xmemin
 *	xmemout
 */
int
uiomove(
register caddr_t	cp,		/* address of kernel buffer	*/
register int		n,		/* #bytes to transfer		*/
enum uio_rw		rw,		/* direction of xfer(read/write)*/
register struct uio	*uio)		/* user area description	*/
{
	register struct iovec	*iov;
	register struct xmem	*xmem;
	int	cnt, rc, flag;

	if (uio->uio_resid <= 0)
		return(ENOMEM);
	rc = 0;
	if (uio->uio_iovcnt == 1) {
		/*
		 * Fastpath for most common case of iovcnt == 1.  Saves a
		 * few instructions.
		 */
		iov = uio->uio_iov;
		xmem = uio->uio_xmem;
		cnt = iov->iov_len;
		if (cnt <= 0)
		{
			uio->uio_iovcnt--;
			uio->uio_iov++;
			uio->uio_iovdcnt++;
			uio->uio_xmem++;
			return(0);
		}
		if (cnt > n)
			cnt = n;

		if (uio->uio_segflg == UIO_XMEM)
		{
			if (rw == UIO_READ)
			{
				rc = xmemout(cp,iov->iov_base,cnt,xmem);
			}
			else
			{
				rc = xmemin(iov->iov_base,cp,cnt,xmem);
			}
		}
		else
		{
			flag = uio->uio_segflg;
			if (rw == UIO_READ)
			{
				rc = uiocopyout(cp,iov->iov_base,&cnt,flag);
			}
			else
			{
				rc = uiocopyin(iov->iov_base,cp,&cnt,flag);
			}
		}

		iov->iov_base += cnt;
		iov->iov_len -= cnt;
		uio->uio_resid -= cnt;
		uio->uio_offset += cnt;
		return(rc);
	}
	while (n > 0 && uio->uio_resid && rc == 0)
	{
		if (uio->uio_iovcnt <= 0)
			return(ENOMEM);
		iov = uio->uio_iov;
		xmem = uio->uio_xmem;
		cnt = iov->iov_len;
		if (cnt <= 0)
		{
			uio->uio_iovcnt--;
			uio->uio_iov++;
			uio->uio_iovdcnt++;
			uio->uio_xmem++;
			continue;
		}
		if (cnt > n)
			cnt = n;

		if (uio->uio_segflg == UIO_XMEM)
		{
			if (rw == UIO_READ)
			{
				rc = xmemout(cp,iov->iov_base,cnt,xmem);
			}
			else
			{
				rc = xmemin(iov->iov_base,cp,cnt,xmem);
			}
		}
		else
		{
			flag = uio->uio_segflg;
			if (rw == UIO_READ)
			{
				rc = uiocopyout(cp,iov->iov_base,&cnt,flag);
			}
			else
			{
				rc = uiocopyin(iov->iov_base,cp,&cnt,flag);
			}
		}

		iov->iov_base += cnt;
		iov->iov_len -= cnt;
		uio->uio_resid -= cnt;
		uio->uio_offset += cnt;
		cp += cnt;
		n -= cnt;
	}
	return(rc);

}  /* end uiomove */

/*
 * NAME:  uiomove_chksum
 *
 * FUNCTION:  Move the data described by the uio structure and perform
 *	TCP checksum during the move.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine runs in kernel mode under the process
 *	address space described by the uio structure.
 *
 * NOTES: 
 *	This service is the equivalent of the ATT iomove service.
 *	The difference is that parameters are passed in the uio
 *	structure instead of the u block.
 *
 *	The sum pointer is assumed to be valid and in kernel space.
 *	It is possible that the move will be successful, but the
 *	checksum can not be computed.  In this case the return code
 *	will be 0, but the upper 2 bytes of the checksum will be non-zero.
 *	The caller must use a separate checksum routine.
 *
 *      This routine only works for (uio->uio_iovcnt == 1).  If it's > 1,
 *	the caller should call uiomove() and a separate checksum routine.
 *
 * RETURN VALUE DESCRIPTION:
 *	0	successful
 *	EFAULT  bad address
 *	ENOMEM  out of memory
 *	ENOSPC  out of disk space
 * 	EIO	io error 
 *
 * IO errors for paging space and running out of paging space are
 * also reported as signals.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	uiocopyin_chksum
 *	uiocopyout_chksum
 */
int
uiomove_chksum(
register caddr_t	cp,		/* address of kernel buffer	*/
register int		n,		/* #bytes to transfer		*/
enum uio_rw		rw,		/* direction of xfer(read/write)*/
register struct uio	*uio,		/* user area description	*/
uint			*sum)		/* ptr where chksum goes	*/
{
	register struct iovec	*iov;
	register struct xmem	*xmem;
	int	cnt, rc, flag;

	if (uio->uio_iovcnt != 1) {
		*sum = 0xffff0000;
		return(uiomove(cp, n, rw, uio));
	}

	if (uio->uio_resid <= 0)
		return(ENOMEM);
	rc = 0;
	/*
	 * Fastpath for most common case of iovcnt == 1.  Saves a
	 * few instructions.
	 */
	iov = uio->uio_iov;
	xmem = uio->uio_xmem;
	cnt = iov->iov_len;
	if (cnt <= 0)
	{
		uio->uio_iovcnt--;
		uio->uio_iov++;
		uio->uio_iovdcnt++;
		uio->uio_xmem++;
		return(0);
	}
	if (cnt > n)
		cnt = n;

	if (uio->uio_segflg == UIO_XMEM)
	{
		rc = EINVAL;
	}
	else
	{
		flag = uio->uio_segflg;
		if (rw == UIO_READ)
		{
			rc = uiocopyout_chksum(cp, iov->iov_base, 
				&cnt, flag, sum);
		}
		else
		{
			rc = uiocopyin_chksum(iov->iov_base,
			    cp, &cnt, flag, sum);
		}
	}

	iov->iov_base += cnt;
	iov->iov_len -= cnt;
	uio->uio_resid -= cnt;
	uio->uio_offset += cnt;
	return(rc);
}  /* end uiomove */

/*
 * NAME:  ureadc
 *
 * FUNCTION:  Give next character to user as result of read.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine runs in kernel mode under the process
 *	address space described by the uio structure.
 *
 * NOTES: 
 *	This service is the equivalent of the ATT passc service.
 *	The difference is that parameters are passed in the uio
 *	structure instead of the u block.
 *
 * RETURN VALUE DESCRIPTION:
 *	0	successful
 *	ENOMEM	uio full
 *	EFAULT	bad address
 *
 * EXTERNAL PROCEDURES CALLED:
 *	subyte
 *	xmemin
 */
int
ureadc(
register int		c,		/* character to give to user	*/
register struct uio	*uio)		/* user area description	*/
{
	register struct iovec	*iov;
	char			x;

	if (uio->uio_resid <= 0)
		return(ENOMEM);
	for (;;)
	{
		if (uio->uio_iovcnt <= 0)
			return(ENOMEM);
		iov = uio->uio_iov;
		if (iov->iov_len > 0)
			break;
		else
		{
			uio->uio_iovcnt--;
			uio->uio_iov++;
			uio->uio_iovdcnt++;
			uio->uio_xmem++;
		}
	}

	switch (uio->uio_segflg)
	{

		case UIO_USERISPACE:
		case UIO_USERSPACE:
			if (subyte(iov->iov_base,c) != 0)
				return(EFAULT);
			break;

		case UIO_SYSSPACE:
			*iov->iov_base = c;
			break;

		case UIO_XMEM:
			x = c;
			if(xmemout(&x,iov->iov_base,sizeof(char),uio->uio_xmem))
				return(EFAULT);
			break;
	}
	iov->iov_base++;
	iov->iov_len--;
	uio->uio_resid--;
	uio->uio_offset++;
	return(0);

}  /* end ureadc */

/*
 * NAME:  uwritec
 *
 * FUNCTION:  Get next character written in by user from uio.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine runs in kernel mode under the process
 *	address space described by the uio structure.
 *
 * NOTES: 
 *	This service is the equivalent of the ATT cpass service.
 *	The difference is that parameters are passed in the uio
 *	structure instead of the u block.
 *
 * RETURN VALUE DESCRIPTION:
 *	0	successful
 *	-1	unsuccessful
 *
 * EXTERNAL PROCEDURES CALLED:
 *	fubyte
 *	xmemout
 */
int
uwritec(
struct uio	*uio)			/* user area description	*/
{
	register struct iovec	*iov;
	register int		c;
	char			x;

	if (uio->uio_resid <= 0)
		return(-1);
	for (;;)
	{
		if (uio->uio_iovcnt <= 0)
			return(-1);
		iov = uio->uio_iov;
		if (iov->iov_len > 0)
			break;
		else
		{
			uio->uio_iovcnt--;
			uio->uio_iov++;
			uio->uio_iovdcnt++;
			uio->uio_xmem++;
		}
	}

	switch (uio->uio_segflg)
	{

		case UIO_USERISPACE:
		case UIO_USERSPACE:
			c = fubyte(iov->iov_base);
			break;

		case UIO_SYSSPACE:
			c = (int)(uchar)*iov->iov_base;
			break;

		case UIO_XMEM:
			if(xmemin(iov->iov_base,&x,sizeof(char),uio->uio_xmem))
				return(-1);
			c = x;
			break;
	}
	if (c == -1)
		return(-1);
	iov->iov_base++;
	iov->iov_len--;
	uio->uio_resid--;
	uio->uio_offset++;
	return(c);

}  /* end uwritec */

/*
 * NAME:  pinu
 *
 * FUNCTION:	Pin user memory.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can only be called under a process.
 *	It cannot page fault.
 *
 * NOTES:	This routine provides a way to pin addresses that are
 *	in either kernel or user memory.  The segflg parameter specifies
 *	where the data to pin is:  user space or kernel space.  See the
 *	values of the uio struct's uio_segflg field in sys/uio.h
 *
 * DATA STRUCTURES:	none.
 *
 * RETURN VALUES DESCRIPTION:	0 upon successful completion;
 *				EFAULT or non-zero return code from xmempin()
 *				upon failure.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	xmattach	cross-memory attach
 *	xmdetach	cross-memory detach
 *	xmalloc		allocate memory
 *	xmempin		pin memory
 */
int
pinu(
caddr_t		base,		/* address of first byte to pin		*/
int		len,		/* #bytes to pin			*/
short		segflg)		/* specifies where data to pin is	*/
{
	int		rc;
	caddr_t		user_addr;
	caddr_t		user_addr1;
	int		len1, len2, tmprc, num_pages1, num_pages2;
	int		seg_num1, seg_num2;
	int		sregval, newsregval;
	int		opri;
	int		detach_n_free1, detach_n_free2;
	struct pinu_block *pinu_block1, *pinu_block2,
			  *tmp_ptr1, *tmp_ptr2, *free_list, *prev_ptr;

	/*
	 * Nothing can be pinned on an interrupt level.
	 */
	ASSERT(csa->prev == NULL);

	rc = 0;
	seg_num1 = (uint) base >> SEGSHIFT;
	if (U.U_procp->p_active == 1) {
		/*
		 * No need to get a cross-memory descripter, etc.
		 * If address is in user memory, then convert the virtual
		 * address to an accessible effective address.
		 */
		if ((segflg == UIO_USERSPACE) || (segflg == UIO_USERISPACE))
		{
			sregval = as_geth(&U.U_adspace, base);
			user_addr = vm_att(sregval, base);
			len1 = MIN(len, SEGSIZE - ((uint)base & SOFFSET));
		}
		else
		{
			user_addr = base;
			len1 = len;
		}

		/*
		 * Now we have an accessible address, so
		 * the "regular" pin() service is called.
		 */
		rc = pin(user_addr, len1);

		if ((segflg == UIO_USERSPACE) || (segflg == UIO_USERISPACE))
		{
			if (rc == 0 && len1 != len)
			{
				/*
				 * User buffer crosses over into next segment.
				 * Make the rest of the user buffer accessible
				 * (using the same segment register as before)
				 * and pin it.
				 */
				newsregval = as_geth(&U.U_adspace, base + len1);
				vm_seth(newsregval, user_addr);
				user_addr1 = (caddr_t) ((uint)user_addr &
				    SREGMSK);
				rc = pin(user_addr1, len-len1);
				if (rc)
				{
					/*
					 * If the second pin call fails then we
					 * must clean up after the first one.
					 */
					as_puth(&U.U_adspace, newsregval);
					vm_seth(sregval, user_addr);
					tmprc = unpin(user_addr, len1);
				}
				else
				{
					as_puth(&U.U_adspace, sregval);
					sregval = newsregval;
				}
			}
			/*
			 * Clean up.
			 */
			vm_det(user_addr);
			as_puth(&U.U_adspace, sregval);
		}
		return rc;
	}

	len1 = MIN(len, SEGSIZE - ((uint)base & SOFFSET));
	len2 = len-len1;
	num_pages1 = ((((int) base & PAGESIZE-1) + len1-1) / PAGESIZE) + 1;
	num_pages2 = ((((int) (base + len1) & PAGESIZE-1) + len2-1) /
	    PAGESIZE) + 1;
	detach_n_free1 = detach_n_free2 = FALSE;
	
	/*
	 *  Assume this is a new region, get a new pinu_block.
	 */
	pinu_block1 = (struct pinu_block *) xmalloc(
	    sizeof(struct pinu_block), 0, pinned_heap);
	if (pinu_block1 == NULL) {
		return(ENOMEM);
	}
	if (len2) {
		pinu_block2 = (struct pinu_block *) xmalloc(
		    sizeof(struct pinu_block), 0, pinned_heap);
		if (pinu_block2 == NULL) {
			xmfree(pinu_block1, pinned_heap);
			return(ENOMEM);
		}
	}
			
	/*
	 *  Call xmattach to build the cross-memory descripter.
	 */
	if ((segflg == UIO_USERSPACE) || (segflg == UIO_USERISPACE)) {
		pinu_block1->xd.aspace_id = XMEM_INVAL;
		rc = xmattach(base, len1, &(pinu_block1->xd),
		    (int) segflg);
		if (rc) {
			/*
			 * Free the pinu_block and return error.
			 */
			xmfree(pinu_block1, pinned_heap);
			if (len2) {
				xmfree(pinu_block2, pinned_heap);
			}
			return rc;
		}
		if (len2) {
			pinu_block2->xd.aspace_id = XMEM_INVAL;
			rc = xmattach(base+len1, len2, &(pinu_block2->xd),
			    (int) segflg);
			if (rc) {
				/*
				 * Free the pinu_block and return error.
				 */
				xmdetach(&(pinu_block1->xd));
				xmfree(pinu_block1, pinned_heap);
				xmfree(pinu_block2, pinned_heap);
				return rc;
			}
		}
	} else {
		pinu_block1->xd.aspace_id = XMEM_GLOBAL;
	}

	/*
	 *  Now call the xmempin service with the cross-memory
	 *  descripter.
	 */
	rc = xmempin(base, len1, &(pinu_block1->xd));
	if (rc) {
		/*
		 *  call xmdetach and free the pinu_block on error
		 */
		if ((segflg == UIO_USERSPACE) ||
		    (segflg == UIO_USERISPACE)) {
			xmdetach(&(pinu_block1->xd));
		}
		xmfree(pinu_block1, pinned_heap);
		if (len2) {
			xmdetach(&(pinu_block2->xd));
			xmfree(pinu_block2, pinned_heap);
		}
		return rc;
	}
	if (len2) {
		/*
		 *  If crosses segment boundary, repeat for 2nd segment
		 */
		rc = xmempin(base+len1, len2, &(pinu_block2->xd));
		if (rc) {
			/*
			 *  call unpin, xmdetach and free the pinu_blocks
			 */
			(void) xmemunpin(base, len1, &(pinu_block1->xd));
			xmdetach(&(pinu_block1->xd));
			xmfree(pinu_block1, pinned_heap);
			xmdetach(&(pinu_block2->xd));
			xmfree(pinu_block2, pinned_heap);
			return rc;
		}
	}

	/*
	 *  Get the lock and check the list of pinu_blocks to see if
	 *  this is already on the list.
	 */
	opri = disable_lock(INTMAX, &pinu_lock);

	/*
	 * Save off the free list and set it to NULL so they can be freed.
	 */
	free_list = pinu_free;
	pinu_free = NULL;

	/*
	 *  Check (possibly) both segment numbers to see if there's already
	 *  a pinu_block on the list for it.
	 */
	tmp_ptr1 = U.U_pinu_block;
	while (tmp_ptr1 != NULL) {
		if (tmp_ptr1->seg_num == seg_num1) {
			break;
		}
		tmp_ptr1 = tmp_ptr1->next;
	}
	if (len2) {
		seg_num2 = seg_num1 + 1;
		tmp_ptr2 = U.U_pinu_block;
		while (tmp_ptr2 != NULL) {
			if (tmp_ptr2->seg_num == seg_num2) {
				break;
			}
			tmp_ptr2 = tmp_ptr2->next;
		}
	}

	if (tmp_ptr1 != NULL) {
		/*
		 *  It's already on the list.
		 *  Check if the subspace_id's also match.
		 */
		if (tmp_ptr1->xd.subspace_id ==
		    pinu_block1->xd.subspace_id) {
			/*
			 *  It matches, so just increment pincount.
			 */
			tmp_ptr1->pincount += num_pages1;
		} else {
			/*
			 *  error = sreg value changed.
			 */
			(void) xmemunpin(base, len1, &(pinu_block1->xd));
			if (len2) {
				(void) xmemunpin(base+len1, len2,
				    &(pinu_block2->xd));
			}
			rc = EFAULT;
		}
		/*
		 *  xmdetach and free the pinu_block.
		 */
		detach_n_free1 = TRUE;
	} else {
		/*
		 *  Not on the list.  Just add this pinu_block to the list.
		 */
		pinu_block1->pincount = num_pages1;
		pinu_block1->seg_num = seg_num1;
		pinu_block1->next = U.U_pinu_block;
		U.U_pinu_block = pinu_block1;
	}

	/*
	 * Now do the same thing for the second segment if necessary.
	 */
	if ((!rc) && (len2)) {
		if (tmp_ptr2 != NULL) {
			/*
			 *  It's already on the list.
			 *  Check if the subspace_id's also match.
			 */
			if (tmp_ptr2->xd.subspace_id ==
			    pinu_block2->xd.subspace_id) {
				/*
				 *  It matches, so just increment pincount.
				 */
				tmp_ptr2->pincount += num_pages2;
			} else {
				/*
				 *  error = sreg value changed.
				 *  Need to remove the first pinu_block
				 *  if we just added it, or decrement the count
				 */
				if (tmp_ptr1) {
					tmp_ptr1->pincount -= num_pages1;
				} else {
					U.U_pinu_block = U.U_pinu_block->next;
				}
				(void) xmemunpin(base,len1,&(pinu_block1->xd));
				(void) xmemunpin(base+len1, len2,
				    &(pinu_block2->xd));
				rc = EFAULT;
				detach_n_free1 = TRUE;
			}
			unlock_enable(opri, &pinu_lock);
			/*
			 *  xmdetach and free the pinu_block.
			 */
			detach_n_free2 = TRUE;
		} else {
			/*
			 *  Not on the list.  Just add this pinu_block
			 *  to the list.
			 */
			pinu_block2->pincount = num_pages2;
			pinu_block2->seg_num = seg_num2;
			pinu_block2->next = U.U_pinu_block;
			U.U_pinu_block = pinu_block2;
			unlock_enable(opri, &pinu_lock);
		}
	} else {
		unlock_enable(opri, &pinu_lock);
		detach_n_free2 = len2;
	}

	if (detach_n_free1) {
		xmdetach(&(pinu_block1->xd));
		xmfree(pinu_block1, pinned_heap);
	}
	if (detach_n_free2) {
		xmdetach(&(pinu_block2->xd));
		xmfree(pinu_block2, pinned_heap);
	}

	/*
	 *  Free the rest of the free blocks on the list.
	 */
	while (free_list) {
		tmp_ptr1 = free_list->next;
		xmfree(free_list, pinned_heap);
		free_list = tmp_ptr1;
	}
	return rc;
}  /* end pinu */

/*
 * NAME:  unpinu
 *
 * FUNCTION:	Unpin user memory.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can only be called under a process or on an
 *	interrupt level.  It cannot page fault.
 *
 * NOTES:	This routine provides a way to unpin addresses that are
 *	in either kernel or user memory.  The segflg parameter specifies
 *	where the data to unpin is:  user space or kernel space.  See the
 *	values of the uio struct's uio_segflg field in sys/uio.h
 *
 * DATA STRUCTURES:	none.
 *
 * RETURN VALUES DESCRIPTION:	0 upon successful completion;
 *				EFAULT or non-zero return code from xmemunpin()
 *				upon failure.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	xmdetach	cross memory detach
 *	xmemunpin	unpin memory
 */
int
unpinu(
caddr_t		base,		/* address of first byte to unpin	*/
int		len,		/* #bytes to unpin			*/
short		segflg)		/* specifies where data to unpin is	*/
{
	register int	rc;	/* unpin, setjmpx return values		*/
	caddr_t		user_addr;
	caddr_t		user_addr1;
	int		len1, len2, num_pages1, num_pages2, seg_num1, seg_num2;
	struct pinu_block *pinu_block1, *pinu_block2, *prev_ptr1, *prev_ptr2;
	int		sregval, newsregval;
	int		opri;

	seg_num1 = (uint) base >> SEGSHIFT;

	if (U.U_procp->p_active == 1) {
		/*
		 * There's no cross-memory descripter needed.
		 * If address is in user memory, then convert the virtual
		 * address to an accessible effective address.
		 */
		if ((segflg == UIO_USERSPACE) || (segflg == UIO_USERISPACE))
		{
			if (CSA->intpri == INTBASE) {
				sregval = as_geth(&U.U_adspace, base);
			} else {
				sregval = as_getsrval(&U.U_adspace, base);
			}
			user_addr = vm_att(sregval, base);
			len1 = MIN(len, SEGSIZE - ((uint)base & SOFFSET));
		}
		else
		{
			user_addr = base;
			len1 = len;
		}

		/*
		 * Now we have an accessible address, so
		 * the "regular" unpin() service is called.
		 */
		rc = unpin(user_addr, len1);

		if ((segflg == UIO_USERSPACE) || (segflg == UIO_USERISPACE))
		{
			if (rc == 0 && len1 != len)
			{
				/*
				 * User buffer crosses over into next segment.
				 * Make the rest of the user buffer accessible
				 * (using the same segment register as before)
				 * and unpin it.
				 */
				if (CSA->intpri == INTBASE) {
					newsregval = as_geth(&U.U_adspace,
					    base + len1);
					as_puth(&U.U_adspace, sregval);
				} else {
					newsregval = as_getsrval(&U.U_adspace,
					    base + len1);
				}
				vm_seth(newsregval, user_addr);
				sregval = newsregval;
				user_addr1 = (caddr_t) ((uint)user_addr &
				    SREGMSK);
				rc = unpin(user_addr1, len-len1);
			}
			vm_det(user_addr);
			if (CSA->intpri == INTBASE) {
				as_puth(&U.U_adspace, sregval);
			}
		}
		return rc;
	}

	/*
	 *  Walk the chain of pinu_block's for this ublock and find
	 *  a match for the segment number.
	 */
	len1 = MIN(len, SEGSIZE - ((uint)base & SOFFSET));
	len2 = len - len1;
	num_pages1 = ((((int) base & PAGESIZE-1) + len1-1) / PAGESIZE) + 1;
	num_pages2 = ((((int) (base+len1) & PAGESIZE-1) + len2-1) /
	    PAGESIZE) + 1;
	prev_ptr1 = NULL;
	opri = disable_lock(INTMAX, &pinu_lock);
	pinu_block1 = U.U_pinu_block;
	while ((pinu_block1 != NULL) && (seg_num1 != pinu_block1->seg_num)) {
		prev_ptr1 = pinu_block1;
		pinu_block1 = pinu_block1->next;
	}
	if (pinu_block1 == NULL) {
		unlock_enable(opri, &pinu_lock);
		return EFAULT;
	}
	if (len2) {
		seg_num2 = seg_num1 + 1;
		prev_ptr2 = NULL;
		pinu_block2 = U.U_pinu_block;
		while ((pinu_block2 != NULL) &&
		    (seg_num2 != pinu_block2->seg_num)) {
			prev_ptr2 = pinu_block2;
			pinu_block2 = pinu_block2->next;
		}
		if (pinu_block2 == NULL) {
			unlock_enable(opri, &pinu_lock);
			return EFAULT;
		}
	}

	/*
	 *  Now that we have a cross-memory descripter, call the xmemunpin()
	 *  service to unpin it.
	 */
	rc = xmemunpin(base, len1, &(pinu_block1->xd));
	
	if (!rc) {
		/*
		 *  Decrement the number of pages pinned.
		 */
		pinu_block1->pincount -= num_pages1;

		/*
		 *  If total number of pinned pages is 0, call xmdetach and
		 *  free the pinu_block.
		 */
		if (pinu_block1->pincount == 0) {
			if ((segflg == UIO_USERSPACE) ||
			    (segflg == UIO_USERISPACE)) {
				xmdetach(&(pinu_block1->xd));
			}
			if (prev_ptr1 == NULL) {
				U.U_pinu_block = pinu_block1->next;
			} else {
				prev_ptr1->next = pinu_block1->next;
			}
			pinu_block1->next = pinu_free;
			pinu_free = pinu_block1;
		}
		if (len2) {
			/*
			 *  Repeat the above for second segment.
			 *  Decrement the number of pages pinned.
			 */
			rc = xmemunpin(base+len1, len2, &(pinu_block2->xd));

			if (!rc) {
				/*
				 *  Decrement the number of pages pinned.
				 */
				
				pinu_block2->pincount -= num_pages2;

				/*
				 *  If total number of pinned pages is 0, call
				 *  xmdetach and free the pinu_block.
				 */
				if (pinu_block2->pincount == 0) {
					if ((segflg == UIO_USERSPACE) ||
					    (segflg == UIO_USERISPACE)) {
						xmdetach(&(pinu_block2->xd));
					}
					if (prev_ptr2 == pinu_block1) {
						prev_ptr2 = prev_ptr1;
					}
					if (prev_ptr2 == NULL) {
						U.U_pinu_block =
						    pinu_block2->next;
					} else {
						prev_ptr2->next =
						    pinu_block2->next;
					}
					pinu_block2->next = pinu_free;
					pinu_free = pinu_block2;
				}
			}
		}
	}
	unlock_enable(opri, &pinu_lock);
	return rc;
}  /* end unpinu */       

/*
 * NAME:  xmempin
 *
 * FUNCTION:	Pin user memory using cross memory descripter
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can only be called under a process.
 *	It cannot page fault.
 *
 * NOTES:	This routine provides a way to pin addresses that are
 *	in either kernel or user memory.  The effective address will
 *      be determined from the cross-memory descripter.
 *
 * DATA STRUCTURES:	none.
 *
 * RETURN VALUES DESCRIPTION:	0 upon successful completion;
 *				EFAULT or non-zero return code from pin()
 *				upon failure.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	vm_att		attach a seg. reg. to the kernel's addr space
 *	vm_det		detach a seg. reg. from the kernel's addr space
 *	pin		pin memory
 */
int
xmempin(
caddr_t		base,		/* address of first byte to pin		*/
int		len,		/* #bytes to pin			*/
struct xmem	*xd)		/* cross-memory descripter		*/
{
	caddr_t		user_addr;
	caddr_t		user_addr1;
	int		len1, rc, tmprc;

	/*
	 * If address is in user memory, then convert the virtual
	 * address to an accessible effective address.
	 */
	if (xd->aspace_id == XMEM_GLOBAL) {
		user_addr = base;
		len1 = len;
	} else {
		user_addr = vm_att(xd->subspace_id, base);
		len1 = MIN(len, SEGSIZE - ((uint)base & SOFFSET));
	}

	/*
	 * Now we have an accessible address, so
	 * the "regular" pin() service is called.
	 */
	rc = pin(user_addr, len1);

	if (xd->aspace_id != XMEM_GLOBAL) {
		if (rc == 0 && len1 != len) {
			/*
			 * User buffer crosses over into next segment.
			 * Make the rest of the user buffer accessible
			 * (using the same segment register as before)
			 * and pin it.
			 */
			vm_seth(xd->subspace_id2, user_addr);
			user_addr1 = (caddr_t) ((uint)user_addr & SREGMSK);
			rc = pin(user_addr1, len-len1);
			if (rc) {
				/*
				 * If the second pin call fails then we
				 * must clean up after the first one.
				 */
				vm_seth(xd->subspace_id, user_addr);
				tmprc = unpin(user_addr, len1);
			}
		}
		/*
		 * Clean up.
		 */
		vm_det(user_addr);
	}
	return rc;
} /* end xmempin */

/*
 * NAME:  xmemunpin
 *
 * FUNCTION:	Unpin user memory using cross-memory descripter.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can only be called under a process or on an
 *	interrupt level.  It cannot page fault.
 *
 * NOTES:	This routine provides a way to unpin addresses that are
 *	in either kernel or user memory.
 *
 * DATA STRUCTURES:	none.
 *
 * RETURN VALUES DESCRIPTION:	0 upon successful completion;
 *				EFAULT or non-zero return code from unpin()
 *				upon failure.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	vm_att		attach a seg. reg. to the kernel's addr space
 *	vm_det		detach a seg. reg. from the kernel's addr space
 *	unpin		unpin memory
 */
int
xmemunpin(
caddr_t		base,		/* address of first byte to unpin	*/
int		len,		/* #bytes to unpin			*/
struct xmem	*xd)		/* cross memory descripter		*/
{
	caddr_t		user_addr;
	caddr_t		user_addr1;
	int		len1, rc, tmprc;

	/*
	 * If address is in user memory, then convert the virtual
	 * address to an accessible effective address.
	 */
	if (xd->aspace_id == XMEM_GLOBAL) {
		user_addr = base;
		len1 = len;
	} else {
		user_addr = vm_att(xd->subspace_id, base);
		len1 = MIN(len, SEGSIZE - ((uint)base & SOFFSET));
	}

	/*
	 * Now we have an accessible address, so
	 * the "regular" unpin() service is called.
	 */
	rc = unpin(user_addr, len1);

	if (xd->aspace_id != XMEM_GLOBAL) {
		if (rc == 0 && len1 != len) {
			/*
			 * User buffer crosses over into next segment.
			 * Make the rest of the user buffer accessible
			 * (using the same segment register as before)
			 * and unpin it.
			 */
			vm_seth(xd->subspace_id2, user_addr);
			user_addr1 = (caddr_t) ((uint)user_addr & SREGMSK);
			rc = unpin(user_addr1, len-len1);
		}
		vm_det(user_addr);
	}
	return rc;
}  /* end unpinu */       
