static char sccsid[] = "@(#)35	1.19.1.26  src/bos/kernel/vmm/POWER/vmmove.c, sysvmm, bos41J, 9520B_all 5/19/95 08:38:49";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:	vm_move
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

#include "vmsys.h"
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/errno.h>
#include <sys/except.h>
#include <sys/lockl.h>
#include <sys/user.h>
#include <sys/vmdisk.h>
#include <sys/syspest.h>
#include <sys/mstsave.h>
#include <sys/adspace.h>
#include <sys/var.h>
#include <sys/systm.h>

static void iopace();
static void wrbehind();
static int vmfcopyin();
static int vmpcopy();
static int vmexception();

/*
 * vm_move(sid,offset,limit,rw,uio)
 *
 * moves data between a segment and the buffer specified
 * in a uio structure.
 *
 * vm_move is similar to uiomove, but the starting offset
 * within the segment is specified by a long address of
 * the form (sid,offset) instead of as a caddr_t address.
 *
 * this service does not support use of cross memory
 * descriptors.
 *
 * INPUT PARAMETERS
 *
 * (1) sid - base segment identifier
 *
 * (2) offset - starting offset within the segment
 *
 * (3) limit - maximum number of bytes which can be move.
 *
 * (4) rw - uio read/write flag.
 *
 * (5) uio - pointer to uio structure for this operation.
 *
 *
 * RETURN VALUES
 *
 *	0	- successful
 *
 *	EFAULT  - invalid address, exceptions outside errno range
 *
 *	ENOMEM  - out of memory
 *
 *	ENOSPC  - out of disk space
 *
 * 	EIO	- i/o error
 *
 *	other errno values
 */


int numclust = 1;
int maxrandwrt = 0;	/* random write-behind threshold, default 0 is none */


vm_move(sid,offset,limit,rw,uio)
uint		sid;	/* base segment id */
caddr_t   	offset; /* offset within the segment */
int		limit;	/* maximum number of bytes */
enum uio_rw	rw;	/* uio r/w flag */
struct uio	*uio;	/* pointer to uio structure */
{
	int	nbytes, rc, sidx, journal, persistent, defer, client;
	caddr_t cp0;
	uint_t	savevmm;
	struct iovec	*iov;
	struct inode	*ip;


        /* save current view in VMMSR and get addressability to vmmdseg
         */
        savevmm = chgsr(VMMSR,vmker.vmmsrval);

	/* get sidx and current journal value and defer.
         */
	sidx = STOI(sid);
	journal = scb_jseg(sidx);
	persistent = scb_pseg(sidx);
	client = scb_clseg(sidx);
	defer = scb_defseg(sidx);
	ip = GTOIP(scb_gnptr(sidx));

        /* restore original view in VMMSR
         */
        (void)chgsr(VMMSR,savevmm);

	/* if this is a write request involving multiple iovecs and
	 * involves data movement from system space to a non-journaled
	 * persistent segment, vm_move_wru() is used to perform the
	 * move.  vm_move_wru() gathers the source data described by
	 * the iovecs based upon destination pages and moves the data
	 * one destination page at a time, attempting to take advantage
	 * of v_movep() to avoid page faults and I/O on the complete
	 * overwrites of the destination pages.
	 */
        if (uio->uio_iovcnt > 1 && rw == UIO_WRITE &&
	    persistent && !journal &&
	    uio->uio_segflg == UIO_SYSSPACE &&
	    uio->uio_iovcnt <= MAXSIOVECS)
		return(vm_move_wru(sid,offset,limit,uio));

	while (limit > 0 && uio->uio_resid)
	{
		iov = uio->uio_iov;
		nbytes = iov->iov_len;

		/*  move to next io vector if no data remains
		 *  within current vector.
		 */
		if (nbytes <= 0)
		{
			uio->uio_iov++;
			uio->uio_iovcnt--;
			continue;
		}

		if (nbytes > limit)
		{
			nbytes = limit;
		}

		/* make beginning of trusted buffer addressable.
	 	 */
		sid = ITOS(sidx, (uint) offset >> L2PSIZE);
		cp0 = vm_att(SRVAL(sid,0,journal), offset);

		/* move the data.
		 */
		if (rw == UIO_READ)
		{
			rc = vmcopyout(cp0,iov->iov_base,&nbytes,
					uio->uio_segflg);
		}
		else 	
		{
			/* pace pageout i/o.
			 */
			iopace(sidx);

			if (client || (persistent && !journal))
			{
				rc = vmfcopyin(sid,iov->iov_base,cp0,&nbytes,
						uio->uio_segflg);
			}
			else
			{
				rc = vmcopyin(iov->iov_base,cp0,&nbytes,
						uio->uio_segflg);
			}

			/* perform write behind of non-journalled/non-defer
			 * persistent segment.
			 */
			if (persistent && !journal && !defer)
				wrbehind(sid,offset,nbytes,ip);

		}

		/* remove buffer addressability.
		 */
		vm_det(cp0);

		/* update values to reflect the amount of
		 * data moved.
		 */
		iov->iov_base += nbytes;
		iov->iov_len -= nbytes;
		uio->uio_resid -= nbytes;
		uio->uio_offset += nbytes;
		offset += nbytes;
		limit -= nbytes;

		/* check return code from vmcopyin (out).
		 */
		if (rc)
		{
			return(rc);
		}
	}

	return(0);

}

/*
 * iopace(sidx)
 *
 * paces non-fblru pageout i/o for a segment.
 *
 * if i/o pacing is enabled, iopace() checks if the number 
 * queued non-fblru pageouts for the segment has reached
 * the upper pageout limit (vmker.maxpout).  if the limit
 * has been reached, vcs_powait() is called to wait the 
 * process until the number of outstanding non-fblru pageouts
 * reaches  the lower pageout limit (vmker.minpout).
 *
 * this routine is only called by vm_move() for persistent
 * and client segments.
 *
 * INPUT PARAMETER:
 *
 *	sidx - scb index	
 *
 * RETURN VALUE:
 *
 *	NONE
 *
 */

static
void
iopace(sidx)
uint sidx;
{
	uint savevmm;
	int hadlock;
	label_t jmpbuf;

	/* check for pageout i/o limit (i/o pacing 
	 * enabled).
	 */
	if (vmker.maxpout <= 0)
		return;

	/* make vmmdseg addressable.
	 */
      	savevmm = chgsr(VMMSR,vmker.vmmsrval);

	/* check if the segment has reached the pageout
	 * i/o limit.
	 */
	if (scb_npopages(sidx) >= vmker.maxpout)
	{
		/* free kernel lock if we have it.
		 */
		if (hadlock = IS_LOCKED(&kernel_lock))
			unlockl(&kernel_lock);

		/* wait for pageout i/o to finish.
		 * yes, we really want to ignore any exceptions that occur
		 * (process doesn't care about exceptions on previous pageouts)
		 */
		vcs_powait_excp(sidx);

		/* re-acquire kernel lock 
		 */
		if (hadlock) 
			lockl(&kernel_lock,LOCK_SHORT);
	}

       	(void)chgsr(VMMSR,savevmm);
	return;
}

/*
 * wrbehind(sid,offset,nbytes,ip)
 *
 * performs write behind for non-journalled persistent
 * segments.
 *
 * This routine checks if the current write operation is random
 * or is sequential and has crossed a cluster boundary.  The sequential 
 * writer is always written when crossing a disk cluster boundary.  The 
 * random writer must reach a threshold before initiating page out for 
 * dirty pages.
 *
 * INPUT PARAMETERS:
 *
 * 	sid 	- base segment id
 *
 * 	offset 	- beginning offset within the segment - current write
 *
 * 	nbytes 	- number of bytes to be written - current write
 *
 *	ip 	- inode pointer 
 *
 * RETURN VALUES:
 *
 *	NONE
 */
static void
wrbehind(uint	sid,
	 uint	offset,
	 int	nbytes,
	 struct inode *ip)
{
        int     clpages, first, last, clfirst, cllast, rc; 
        struct  wrbentry
        {
		/* "randcnt" is the threshold where random writers start
		 * pageout (<=1024).  "cluster" is the disk cluster number.
		 * Twenty two bits is sufficient for recording the cluster
		 * number for 64Gig of 16384 byte clusters.
		 */
#define MAXWRB  10
                unsigned randcnt : MAXWRB; /* Number of random writes */
                unsigned cluster : 22;     /* Current cluster number  */
        } *wrb;

        clpages = numclust * FSCLSIZE;   	     /* cluster pages         */
        first   = offset >> L2PSIZE;    	     /* first page in write   */
	if (nbytes > 0)
		last = offset + nbytes - 1 >> L2PSIZE;  /* last page in write */
	else
		last = first;
	clfirst = first / clpages;		     /* cluster's first page  */
	cllast  = last  / clpages;		     /* cluster's last page   */
	wrb     = (struct wrbentry *)&ip->i_cluster; /* previous write behind */

	/* If the current write starts in the same cluster as the previous
	 * write and crosses into another cluster, or if the write is in
	 * a contiguous cluster, then this is a sequential writer so 
	 * initiate pageout.
	 */
	if ((clfirst == wrb->cluster && cllast != wrb->cluster) ||
	    (clfirst == wrb->cluster + 1))
	{
		rc = vm_writep(sid, wrb->cluster * clpages, 
				(cllast - wrb->cluster) * clpages);
		ASSERT(rc == 0);

		wrb->cluster = cllast;
		if (wrb->randcnt)
			wrb->randcnt--;
	}
	else if (clfirst != wrb->cluster && maxrandwrt)	/* random writer */
	{
		/* If we have reached the random writer threshold, then
		 * initiate pageout.
		 */
		if (wrb->randcnt >= maxrandwrt)
		{
                	rc = vm_writep(sid, first, last - first + 1);
                	ASSERT(rc == 0);
		}
		else
			wrb->randcnt++;

		wrb->cluster = cllast;
	}
	else if (wrb->randcnt)	/* write was in same cluster */
		wrb->randcnt--;

}	
	
/*
 * vmcopyin(src,dest,nbytes,type)
 *
 * Copy bytes to trusted destination from source. source
 * must be in one segment, destination may cross a segment
 * segment boundary to next segment of a logical segment.
 *
 * type specifies whether source is in user or kernel space.
 *
 * RETURN VALUES:
 *
 *	0 	- ok
 *
 *	EIO 	- permanent i/o error file-space
 *
 *	ENOSPC	- out of file-space blocks.
 *
 *	EFAULT  - invalid address, exceptions outside errno range
 *
 *	other errno values
 */

int
vmcopyin(src,dest,nbytes,type)
caddr_t src;
caddr_t dest;
int *nbytes;
int type;
{
	int rc, count, rcount, scount, dcount, count1, sid, hadlock;
	int partial = 0;
	int attach = 0;
	caddr_t src1 = src;
	caddr_t dest1 = dest;
	int sregval, newsregval;
	int putseg = 0;

	/* make sure number of bytes is <= 256MB.
	 * if not, copy only that many bytes.
	 */
	if (*nbytes == 0)
	{
		return(0);
	}
	else if (*nbytes > SEGSIZE)
	{
		*nbytes = SEGSIZE;
		partial = 1;
	}

	rcount = count = *nbytes;

	if (type != UIO_SYSSPACE)
	{
		/* We don't need to protect access to the
		 * process private segment.
		 */
		if (((uint)src >> SEGSHIFT) == PRIVSEG)
			sregval = as_getsrval(&u.u_adspace,src);
		else
		{
			sregval = as_geth(&u.u_adspace,src);
			putseg = 1;
		}
		src1 = vm_att(sregval,src);
		attach = 1;
	}

	/* free the kernel lock ?
	 */
	if (hadlock = (!CSA->prev && IS_LOCKED(&kernel_lock)))
		unlockl(&kernel_lock);

	while(rcount > 0)
	{
		/* move the number of bytes which is the minimum of:
		 * - the total # remaining
		 * - the # remaining in the current source segment
		 * - the # remaining in the current target segment
		 */
		scount = SEGSIZE - ((uint)src1 & SOFFSET);
		dcount = SEGSIZE - ((uint)dest1 & SOFFSET);
		count1 = MIN(rcount,MIN(scount,dcount));
		if (rc = exbcopy(src1, dest1, count1))
		{
			*nbytes = count1;
			rc = vmexception(src1,dest1,rc,nbytes);
			*nbytes += (count - rcount);
			goto done;
		}
		
		if (count1 != rcount)
		{
			/* we moved up to segment boundary.
			 */
			if (count1 == scount)
			{
				/* source (user buffer) crosses seg boundary.
				 * attach to next segment in user buffer
				 * using same sreg and starting at offset 0.
				 * We don't need to protect access to the
				 * process private segment.
				 */
				newsregval = as_geth(&u.u_adspace,src+count1);
				vm_seth(newsregval,src1);
				if (putseg)
					as_puth(&u.u_adspace, sregval);
				putseg = 1;
				sregval = newsregval;
				src1 = (caddr_t)((uint)src1 & SREGMSK);
				dest1 = dest1 + count1;
			}
			else
			{
				/* target (file) crosses segment boundary.
				 * attach to next segment in file
				 * using same sreg as assigned in vm_move
				 * and starting at offset zero.
				 */
				sid = SRTOSID(mfsri(dest1)) + NEXTSID;
				if ((sid & SIDMASK) == 0)
				{
					rc = EFAULT;
					*nbytes = count - rcount + count1;
					goto done;
				}
				vm_seth(SRVAL(sid,0,0), dest1);
				src1 = src1 + count1;
				dest1 = (caddr_t) ((uint)dest1 & SREGMSK);
			}
		}
		rcount -= count1;
	}

	done:

	/* free up sreg if attached re-acquire kernel lock.
	 */
	if (attach)
	{
		vm_det(src1);
		
		if (putseg)
			as_puth(&u.u_adspace,sregval);
	}
	if (hadlock)
		lockl(&kernel_lock,LOCK_SHORT);

	/* if partial copy return EFAULT - entire range
	 * not copied.
	 */
	if (partial && rc == 0)
		rc = EFAULT;

	return(rc);
}

/*
 * vmfcopyin(sid,src,dest,nbytes,type)
 *
 * Copy bytes to trusted persistent segment destination from
 * source. source must be in one segment, destination may
 * cross a segment segment boundary to next segment of a
 * logical segment.
 *
 * this routine is called only by vm_move() for non-journalled
 * persistent segments.
 *
 * type specifies whether source is in user or kernel space.
 *
 * RETURN VALUES:
 *
 *	0 	- ok
 *
 *	EIO 	- permanent i/o error file-space
 *
 *	ENOSPC	- out of file-space blocks.
 *
 *	EFAULT  - invalid address, exceptions outside errno range
 *
 *	other errno values
 */

static
int
vmfcopyin(sid,src,dest,nbytes,type)
uint sid;
caddr_t src;
caddr_t dest;
int *nbytes;
int type;
{
	int rc, count, rcount, scount, dcount, count1, hadlock;
	int sid1 = sid;
	int partial = 0;
	int attach = 0;
	caddr_t src1 = src;
	caddr_t dest1 = dest;
	int sregval, newsregval;
	int putseg = 0;

	/* make sure number of bytes is <= 256MB.
	 * if not, copy only that many bytes.
	 */
	if (*nbytes == 0)
	{
		return(0);
	}
	if (*nbytes > SEGSIZE)
	{
		*nbytes = SEGSIZE;
		partial = 1;
	}

	rcount = count = *nbytes;

	if (type != UIO_SYSSPACE)
	{
		/* We don't need to protect access to the
		 * process private segment.
		 */
		if (((uint)src >> SEGSHIFT) == PRIVSEG)
			sregval = as_getsrval(&u.u_adspace,src);
		else
		{
			sregval = as_geth(&u.u_adspace,src);
			putseg = 1;
		}
		src1 = vm_att(sregval,src);
		attach = 1;
	}

	/* free the kernel lock ?
	 */
	if (hadlock = (!CSA->prev && IS_LOCKED(&kernel_lock)))
		unlockl(&kernel_lock);

	while(rcount > 0)
	{
		/* move the number of bytes which is the minimum
		 * of the # remaining, the # left in the current
		 * source segment, or the # left in the current
		 * target segment.
		 */
		scount = SEGSIZE - ((uint)src1 & SOFFSET);
		dcount = SEGSIZE - ((uint)dest1 & SOFFSET);
		count1 = MIN(rcount,MIN(scount,dcount));
		if (rc = vmpcopy(sid1, src1, dest1, &count1))
		{
			*nbytes = count1 + count - rcount;
			goto done;
		}
		
		if (count1 != rcount)
		{
			/* we moved up to segment boundary.
			 */
			if (count1 == scount)
			{
				/* source (user buffer) crosses seg boundary.
				 * attach to next segment in user buffer
				 * using same sreg and starting at offset 0.
				 * We don't need to protect access to the
				 * process private segment.
				 */
				newsregval = as_geth(&u.u_adspace,src+count1);
				vm_seth(newsregval,src1);
				if (putseg)
					as_puth(&u.u_adspace, sregval);
				putseg = 1;
				sregval = newsregval;
				src1 = (caddr_t)((uint)src1 & SREGMSK);
				dest1 = dest1 + count1;
			}
			else
			{
				/* target (file) crosses segment boundary.
				 * attach to next segment in file
				 * using same sreg as assigned in vm_move
				 * and starting at offset zero.
				 */
				sid1 += NEXTSID;
				if ((sid1 & SIDMASK) == 0)
				{
					rc = EFAULT;
					*nbytes = count - rcount + count1;
					goto done;
				}
				vm_seth(SRVAL(sid1,0,0), dest1);
				src1 = src1 + count1;
				dest1 = (caddr_t) ((uint)dest1 & SREGMSK);
			}
		}
		rcount -= count1;
	}

	done:

	/* free up sreg if attached re-acquire kernel lock.
	 */
	if (attach)
	{
		vm_det(src1);
		if (putseg)
			as_puth(&u.u_adspace,sregval);
	}
	if (hadlock)
		lockl(&kernel_lock,LOCK_SHORT);

	/* if partial copy return EFAULT - entire range
	 * not copied.
	 */
	if (partial && rc == 0)
		rc = EFAULT;

	return(rc);
}

/*
 * vmcopyout(src,dest,nbytes,type)
 *
 * Copy bytes from trusted source to destination.  destination
 * must be in one segment, source may cross a segment boundry
 * to the next segment of the logical segment.
 *
 * type specifies whether destination is in user or kernel space.
 *
 * RETURN VALUES:
 *
 *	0 	- ok
 *
 *	EIO 	- permanent i/o error file-space
 *
 *	ENOSPC	- out of file-space blocks.
 *
 *	EFAULT  - invalid address, exceptions outside errno range
 *
 *	other errno values
 */

int
vmcopyout(src,dest,nbytes,type)
caddr_t src;
caddr_t dest;
int *nbytes;
int type;
{
	int rc, count, rcount, scount, dcount, count1, sid, hadlock;
	int partial = 0;
	int attach = 0;
	caddr_t src1 = src;
	caddr_t dest1 = dest;
	int sregval, newsregval;
	int putseg = 0;

	/* make sure number of bytes is <= 256MB.
	 * if not, copy only that many bytes.
	 */
	if (*nbytes == 0)
	{
		return(0);
	}
	if (*nbytes > SEGSIZE)
	{
		*nbytes = SEGSIZE;
		partial = 1;
	}

	rcount = count = *nbytes;

	if (type != UIO_SYSSPACE)
	{
		/* We don't need to protect access to the
		 * process private segment.
		 */
		if (((uint)dest >> SEGSHIFT) == PRIVSEG)
			sregval = as_getsrval(&u.u_adspace,dest);
		else
		{
			sregval = as_geth(&u.u_adspace,dest);
			putseg = 1;
		}
		dest1=vm_att(sregval,dest);
		attach = 1;
	}

	/* free the kernel lock ?
	 */
	if (hadlock = (!CSA->prev && IS_LOCKED(&kernel_lock)))
		unlockl(&kernel_lock);

	while(rcount > 0)
	{
		/* move the number of bytes which is the minimum
		 * of the # remaining, the # left in the current
		 * source segment, or the # left in the current
		 * target segment.
		 */
		scount = SEGSIZE - ((uint)src1 & SOFFSET);
		dcount = SEGSIZE - ((uint)dest1 & SOFFSET);
		count1 = MIN(rcount,MIN(scount,dcount));
		if (rc = exbcopy(src1, dest1, count1))
		{
			*nbytes = count1;
			rc = vmexception(src1,dest1,rc,nbytes);
			*nbytes += (count - rcount);
			goto done;
		}
		
		if (count1 != rcount)
		{
			/* we moved up to segment boundary.
			 */
			if (count1 == scount)
			{
				/* source (file) crosses segment boundary.
				 * attach to next segment in file
				 * using same sreg as assigned in vm_move
				 * and starting at offset zero.
				 */
				sid = SRTOSID(mfsri(src1)) + NEXTSID;
				if ((sid & SIDMASK) == 0)
				{
					rc = EFAULT;
					*nbytes = count - rcount + count1;
					goto done;
				}
				vm_seth(SRVAL(sid,0,0), src);
				src1 = (caddr_t) ((uint)src1 & SREGMSK);
				dest1 = dest1 + count1;
			}
			else
			{
				/* target (user buffer) crosses seg boundary.
				 * attach to next segment in user buffer
				 * using same sreg.
				 * We don't need to protect access to the
				 * process private segment.
				 */
				newsregval = as_geth(&u.u_adspace,dest+count1);
				vm_seth(newsregval,dest1);
				if (putseg)
					as_puth(&u.u_adspace, sregval);
				putseg = 1;
				sregval = newsregval;
				src1 = src1 + count1;
				dest1 = (caddr_t)((uint)dest1 & SREGMSK);
			}
		}
		rcount -= count1;
	}

	done:

	/* free up sreg if attached re-acquire kernel lock.
	 */
	if (attach)
	{
		vm_det(dest1);
		if (putseg)
			as_puth(&u.u_adspace,sregval);
	}
	if (hadlock)
		lockl(&kernel_lock,LOCK_SHORT);

	/* if partial copy return EFAULT - entire range
	 * not copied.
	 */
	if (partial && rc == 0)
		rc = EFAULT;

	return(rc);
}

int safewrite = 1;

/*
 * vmpcopy(sid,src,dest,nbytes)
 *
 * Copy bytes to destination from source. 
 *
 * The copy operation is broken up into one or more copies to
 * destination pages.  For each destination page level copy 
 * operation, a determination is made as to whether fragment
 * allocation is applicable to the destination segment.  If
 * this is the case, v_movefrag() is called to perform the page
 * level copy operation and allocate fragments as required.
 *
 * If fragment allocation is not applicable to a page level
 * copy operation, the copy is handle either by bcopy() or
 * v_movep().  For each page level copy that does not fully
 * cover the destination page, the data is simply copied via
 * bcopy(). 
 *
 * For each copy that covers a full page within the destination,
 * a check is made to determine whether or not the destination
 * page is memory resident.  If so, the data is simply bcopy()ed.
 * Otherwise, v_movep() is called to copy the data.  v_movep()
 * optimizes the copy by creating a virtual memory page for the
 * destination page to be overwritten, thus avoiding the page
 * initialization (possibly I/O) that would occur if the page
 * was simply faulted in by bcopy().
 *
 * PARAMETER:
 *
 *	 sid	- relative source persistent or client segment id.
 *
 *	 src	- beginning source address.
 *
 *	 dest	- beginning target address.
 *
 *	 nbytes - pointer the the number of bytes to copy.  on
 *		  return, set to the number of bytes actually
 *		  copied.
 *
 *
 * RETURN VALUES:
 *
 *	0 	- ok
 *
 *	EIO 	- permanent i/o error file-space.
 *
 *	ENOSPC	- out of file-space blocks.
 *
 *	EDQUOT	- user or group reach quota limit.
 *
 *	EFAULT  - invalid address.
 */

static
int
vmpcopy(sid,src,dest,nbytes)
uint sid;
uint src;
uint dest;
int *nbytes;
{ 
	int fperpage, fsize, fragments, nb, bytes;
	int basecopy, touch, moved, rc, firstmove;
	uint msid, msidx, savevmm, exaddr, beg, sidx;
	struct vmsrclist slist;
	struct inode *ip;
	label_t jmpbuf;
	volatile int rc2 = 0;
	volatile uint dest0, dest1;
	volatile uint src0, src1;

	/* initialize volatiles and number of bytes
	 * to copy.
	 */
	src0 = src1 = src;
	dest0 = dest1 = dest;
	bytes = *nbytes;
	firstmove = 0;

	again:

	/* make the vmm data segment addressable.
	 */
        savevmm = chgsr(VMMSR,vmker.vmmsrval);

	/* get sidx and fragments per page for the source segment.
	 */
	sidx = STOI(sid);
	fperpage = pdt_fperpage(scb_devid(sidx));

	/* if the number of fragments per page is not 
	 * one, get inode pointer and fragment size.
	 */
	if (fragments = (fperpage != 1))
	{
		assert(scb_pseg(sidx));
		ip = GTOIP(scb_gnptr(sidx));
		fragments = (ip->i_compress == 0);
		fsize = PSIZE/fperpage;
	}

	/*
	 * If the source is an mmap region then we run the risk
	 * of a ping-pong deadlock by performing the copy in a
	 * critical section.  For the non-fragmented (vcs_movep)
	 * case we just perform a bcopy at base level while for
	 * the fragmented (vcs_movefrag) case the critical section
	 * is coded to deal with the source and target referring to
	 * the same mmap region.  A ping-pong deadlock can also occur
	 * with shmat when the source and target are the same file and
	 * the write includes non-existent file pages.
	 *
	 * On MP we may lose addressibility to the source during
	 * the copy in a critical section (since both the source
	 * and target SCBs cannot not be locked due to lock ordering
	 * problems).  For the non-fragmented case we just perform
	 * a bcopy at base level when we determine the source is at
	 * risk while for the fragmented case we copy the source into
	 * a per-CPU kernel buffer before calling the critical section.
	 * Loss of addressibility can occur on MP when:
	 *
	 * - The source is a file (shmat or mmap) and a sync causes
	 *   the source pages to be paged out
	 * - Another thread in the same process unmaps the source
	 *   via munmap, mprotect, shmdt
	 * - Another thread in the same process or another process
	 *   attached to the source (i.e. shmat) does a disclaim
	 *
	 * We protect against any mmap case and the case that the source
	 * is a file but (XXX) we do not protect against another thread
	 * doing a shmdt nor any case involving disclaim.
	 * We use the variable 'safewrite' to allow these tests to be
	 * disabled for the sake of performance.
	 */
	msid = SRTOSID(mfsri(src0));
	msidx = STOI(msid);
	basecopy = scb_mseg(msidx) || scb_pseg(msidx) || scb_clseg(msidx);
	basecopy &= safewrite;

	/* restore VMMSR.
	 */
        (void)chgsr(VMMSR,savevmm);

	/* we always use one iovec to represent the source.
	 */
	slist.nvecs = 1;

	/* set up an exception handler. this only handles exceptions
	 * in vcs_movep() or vcs_movefrag() so the exception addresses
	 * are "well behaved".
	 */
	if ((rc = setjmpx(&jmpbuf)) != 0)
	{
		/* translate the exception value. values out side of
		 * the errno range may be returned for exceptions on
		 * client segments and should be converted to EFAULT.
		 */
		rc = (rc > EXCEPT_ERRNO) ? EFAULT : rc;

		/* get the exception address (source or destination) and
		 * the applicable starting address of the failed copy
		 * operation.
		 */
		exaddr = CSA->o_vaddr;
		beg = ((exaddr & SREGMSK) == (src0 & SREGMSK)) ? src0 : dest0;
		assert(exaddr >= beg && exaddr <= beg + *nbytes - 1);

		/* set the number of bytes copied up to the error.
		 */
		*nbytes = exaddr - beg; 

		/* if the copy operation (vcs_movep or vcs_movefrag)
		 * failed at the beginning of a copy to a destination page
		 * we're done and we return the error.
		 * otherwise, the copy encountered the error on a
		 * page boundary within the source, and copied no data.  in
		 * this case, we redo the copy up to the bad source page
		 * and return the error and partial bytes copied.
		 *
		 * the copy operation is performed below one page aligned
		 * destination page at a time.  as a result, we know that
		 * the indivdiual operation was fully contained within a
		 * page within the destination (i.e. did not cross a 
		 * page boundry).
		 */
		if (exaddr == src1 || exaddr == dest1)
			return(rc);

		/* bytes is the maximum amount we will try to move
		 * in addition to what was moved up to now (firstmove).
		 */
		bytes = PSIZE - (src1 & POFFSET);
		assert(exaddr == src1 + bytes);
		firstmove = src1 - beg;
		rc2 = rc;
		goto again;
	}

	/* copy the data one destination page at a time.
	 */
	for (moved = 0; moved < bytes; moved += nb, src1 += nb, dest1 += nb)
	{
		/* determine the number of bytes to copy for this
		 * destination page.
		 */
		nb = MIN(bytes - moved,PSIZE - (dest1 & POFFSET));

		/* check if fragment allocation is applicable to the
		 * segment (i.e. fragments size < PSIZE and inode size
		 * is within the direct block range).  if so, call
		 * vcs_movefrag() to copy the data and allocation
		 * fragments as required.
		 */
		if (fragments && NOIND(ip->i_size + fsize))
		{
			/*
			 * If the source is at risk then copy it in to a
			 * kernel buffer before calling the critical section.
			 */
			if (basecopy)
			{
				if (rc=copyin_source(sid,&slist,dest1,nb,src1))
				{
					/* return the number of bytes moved
					 * so far if an error was encountered
					 */
					*nbytes = src1 - src0;
					break;
				}
				continue;
			}

			/* setup the vmsrclist iovec base.
			 */
			slist.vecs[0].iov_base = src1;
			slist.vecs[0].iov_len = nb;

			touch = 0;
			if (rc = vcs_movefrag(sid,&slist,dest1,nb,&touch))
			{
				/* return the number of bytes moved
				 * so far if an error was encountered
				 * from vcs_movefrag().
				 */
				*nbytes = src1 - src0;
				break;
			}
	
			continue;
		}

		/* fragments not applicable. simply perform an exbcopy if
		 * the copy is for less than a full page, or the destination
		 * page is in memory, or the source segment is an mmap
		 * segment or is otherwise at risk.
		 * otherwise, call vcs_movep() to do the copy.
		 * vcs_movep() optimizes full page copies by avoiding pagein
		 * i/o when the entire desitnation page is to be overwritten.
		 */
		if (nb != PSIZE || lra(dest1) >= 0 || basecopy)
		{
			if (rc = exbcopy(src1,dest1,nb))
			{
				*nbytes = nb;
				rc = vmexception(src1, dest1, rc, nbytes);
				*nbytes += moved + firstmove;
				break;
			}
		}
		else
		{
			/* setup the vmsrclist iovec.
			 */
			slist.vecs[0].iov_base = src1;
			slist.vecs[0].iov_len = nb;
			
			vcs_movep(sid,&slist,dest1);
		}
	}

	/* clear the exception handler.
	 */
	clrjmpx(&jmpbuf);

	return((rc2 == 0) ? rc : rc2);
}

/*
 * vmexception(src,dest,exval,nbytes)
 *
 * sets the number of bytes successfully copied by 
 * vmcopyin/vmcopyout in the case of an exception.
 *
 * Return values:
 *
 *	exval if it is an errno value, otherwise EFAULT
 */

static
int
vmexception(src,dest,exval,nbytes)
caddr_t src;	/* beginning source address 	 */
caddr_t dest;	/* beginning destination address */
int exval;	/* exception value               */	
int *nbytes;	/* pointer to number of bytes moved */
{
	int ssreg, dsreg, exsreg, rc;
	struct mstsave *mst;
	caddr_t exvaddr;

	mst = CSA;

	/* any exception values outside errno range become EFAULT.
	 */
	rc = (exval > EXCEPT_ERRNO) ? EFAULT : exval;

	/* get sreg for exception, source, and destination
	 * virtual addresses.  The address that caused this
	 * exception is saved in mst->o_vaddr.
	 */
	exvaddr = (caddr_t)mst->o_vaddr;
	exsreg = ((uint)exvaddr >> L2SSIZE);
	ssreg = ((uint)src >> L2SSIZE);
	dsreg = ((uint)dest >> L2SSIZE);
	
	/* check if exception was within source or destination.
	 */
	if (exsreg == ssreg || exsreg == dsreg)
	{
		/* determine whether exception occurred on source or
		 * destination and set nybtes.
		 */
		if (exsreg == ssreg)
		{
			/* check if exception vaddr is within source
			 * range. machine may report exaddr beyond
			 * first failing byte. if exception is in
			 * same page as src no bytes were moved. otherwise
			 * transfer stopped at page boundary.
			 */
			if (((uint)exvaddr & ~(POFFSET)) ==
			    ((uint)src & ~(POFFSET)))
				exvaddr = src;
			else
				exvaddr = (caddr_t)((uint)exvaddr & ~(POFFSET));

			if (exvaddr >= src && exvaddr <= src + *nbytes - 1)
			{
				*nbytes = exvaddr - src;
				goto closeout;
			}
		}
		else
		{
			/* check if exception vaddr is within destination
			 * range. see comment above.
			 */
			if (((uint)exvaddr & ~(POFFSET)) ==
			    ((uint)dest & ~(POFFSET)))
				exvaddr = dest;
			else
				exvaddr = (caddr_t)((uint)exvaddr & ~(POFFSET));

			if (exvaddr >= dest && exvaddr <= dest + *nbytes - 1)
			{
				*nbytes = exvaddr - dest;
				goto closeout;
			}
		}
	}

	/* exception cannot be handled here - call
	 * longjmpx().
	 */
	longjmpx(exval);

	closeout:

	return(rc);
}

/*
 *	vm_dirfrag(ip,newsize,type)
 *
 * Allocates fragments for a small directory to cover newsize.
 *
 * This routine is called by the JFS directory routine each
 * time a new logical directory block is added to a directory.
 * New fragments will be allocated to the directory if the
 * file system fragment size is less than the full block size,
 * the current directory has not grow up to or beyond the
 * indirect block range, and the directory's current allocation
 * in not sufficient to cover newsize.  Allocation is performed
 * by a call to v_dirfrag().
 *
 * All other disk allocations for directories are performed under
 * the page fault handler.
 *
 * This routine assumes that directories grow sequentially by
 * fragment size or less.
 *
 * PARAMETERS:
 *
 *	ip 	- directory inode pointer.
 *
 *	newsize - new directory size.
 *
 * RETURN VALUES:
 *	0 	- ok.
 *		type = 0 - no new allocation, i.e., covered by old page
 *		       1 - new fragment in old page
 *			   (i_movedfrag != 0)
 *		       2 - new page or new fragment in new page
 *	EIO 	- permanent i/o error file-space
 *	ENOSPC	- out of file-space blocks.
 *	EDQUOT	- reached quota limit.
 */

vm_dirfrag(ip,newsize,type)
struct	inode *ip;
int	newsize;
int	*type;
{
	int pno, fperpage, nfrags, oldsize;

	/* must be a directory.
	 */
	assert((ip->i_mode & IFMT) == IFDIR);
	assert(newsize > ip->i_size);

	/* get oldsize, and get page number for newsize
	 */
	oldsize = ip->i_size;
	pno = BTOPN(newsize);
	
	/*
	 * no-fragment file system
	 */
	if ((fperpage = ip->i_ipmnt->i_fperpage) == 1)
	{
		/* check if newsize grow into new page */
		if (pno > BTOPN(oldsize))
			*type = 2; /* new page */
		return 0;
	}
	/*
	 * fragment file system
	 */
	else
	{
		assert(newsize - oldsize <= PSIZE/fperpage);

		/* newsize in direct block range
	 	 */
		if (NOIND(newsize))
		{
			nfrags = BTOFR(newsize,fperpage);

			/* newsize covered by old page */
			if (pno == BTOPN(oldsize))
			{	
				/* newsize covered by old fragment */
				if (nfrags == BTOFR(oldsize,fperpage))
					return 0;
				else
					*type = 1; /* new fragment in old page */
			}
			/* newsize grow into new fragment in new page */
			else
				*type = 2; /* new fragment in new page */
		}
		/* newsize beyond direct block range
	 	 */
		else
		{
			/* check if newsize grow into new page */
			if (pno > BTOPN(oldsize))
				*type = 2; /* new page */
			return 0;
		}
	}

	/* allocate fragment */
	return(vcs_dirfrag(ip->i_seg,pno,nfrags));
}

/*
 * vm_move_wru(sid,offset,limit,uio)
 *
 * moves data from a system segment described by the passed uio 
 * structure to a non-journalled persistent segment described by 
 * the passed base segment id.
 *
 * vm_move_wru() is similar to vm_move(), but gathers the source
 * data described by the uio iovecs such that data movement is
 * performed once for each page of the destination.  the data is
 * moved in this manner in an attempt to take advantage of v_movep()
 * to avoid page faults and i/o on the complete overwrite of
 * destination pages.
 *
 * this routine only supports moves from system space (i.e.
 * uio_segflg == UIO_SYSSPACE) to non-journaled persistent segments
 * and should not be used when the uio structure described more than
 * MAXSIOVECS iovecs.
 *
 * INPUT PARAMETERS
 *
 * (1) sid - base segment identifier of non-journaled persistent
 *     segment
 *
 * (2) offset - starting offset within the segment
 *
 * (3) limit - maximum number of bytes which can be move.
 *
 * (4) uio - pointer to uio structure for this operation.
 *
 *
 * RETURN VALUES
 *
 *	0	- successful
 *
 *	EFAULT  - invalid address, exceptions outside errno range
 *
 *	ENOMEM  - out of memory
 *
 *	ENOSPC  - out of disk space
 *
 * 	EIO	- i/o error
 *
 *	other errno values
 */

vm_move_wru(sid,offset,limit,uio)
uint sid;  
caddr_t offset;
int limit;
struct uio *uio;
{
	int i, nbytes, nb, len, rc, touch;
	int fsize, fragments, pagex, defer, n, numbytes;
	uint exaddr, sidx, savevmm, sidnext, s;
	struct iovec *begvec, *vec, *v;
	struct inode *ip;
	struct vmsrclist src;
	label_t jmpbuf;
	volatile int hadlock, partial, resid = uio->uio_resid;
	char * volatile cp0;
	char * volatile cp1;

	ASSERT(uio->uio_iovcnt <= MAXSIOVECS);
        ASSERT(uio->uio_segflg == UIO_SYSSPACE);

	/* get the number of bytes to move.
	 */
	nbytes = MIN(limit,uio->uio_resid); 

	/* anything to do ?
	 */
	if (nbytes == 0)
		return(0);

	/* pace pageout I/O.
	 */
	sidx = STOI(sid);
	iopace(sidx);

	/* like vmcopyin(), we'll only move up to a segment's
	 * worth of data.
	 */
	if (partial = (nbytes > SEGSIZE))
		nbytes = SEGSIZE;

	/* get addressability to the vmm data segment.
	 */
        savevmm = chgsr(VMMSR,vmker.vmmsrval);

	/* this routine only support moves to non-journaled persistent
	 * segments.
	 */
	ASSERT(scb_pseg(sidx) && !scb_jseg(sidx));

	/* pick up the fragment size, inode pointer and defer
	 * value.
	 */
	fsize = PSIZE/pdt_fperpage(scb_devid(sidx));
	ip = GTOIP(scb_gnptr(sidx));
	defer = scb_defseg(sidx);

	/* restore VMMSR.
	 */
        (void)chgsr(VMMSR,savevmm);

	/* free the kernel lock ?
         */
        if (hadlock = (IS_LOCKED(&kernel_lock)))
                unlockl(&kernel_lock);

	/* determine whether the destination segment is within a
	 * fragmented file system.
	 */
	fragments = (fsize != PSIZE && ip->i_compress == 0);

	/* get the destination sid corresponding to the offset and
	 * make it addressable.
	 */
	pagex = (uint) offset >> L2PSIZE;
        sid = ITOS(sidx, pagex);
        cp1 = cp0 = vm_att(SRVAL(sid,0,0), offset);

	/* set up exception handler for v_movefrag() and v_movep().
	 * the exception addresses are well behaved and should only
	 * be for the start of destination address.  exceptions on
	 * the source will halt the machine since the source is a
	 * "system address" (i.e. UIO_SYSSPACE).
	 */
        if ((rc = setjmpx(&jmpbuf)) != 0)
        {
		/* should not take exception on the source.
		 */
                exaddr = CSA->o_vaddr;
                assert(exaddr == cp0);

		goto out;
        }

	/* now process the uio iovecs one destination page at a time. 
	 * for each destination page, a vmsrclist is contructed that
	 * describes up to 4k of data within the source and may include 
	 * multiple source locations and lengths (i.e. multiple vmsrclist
	 * iovecs).  the data described by the vmsrclist is moved to the
	 * destination page, and the uio and uio iovecs are updated to
	 * reflect the data moved.
	 */
	vec = uio->uio_iov;
        while (nbytes)
	{
		/* remember the starting uio iovec used to contruct 
		 * the vmsrclist.
		 */
		begvec = vec;

		numbytes = MIN(nbytes,PSIZE - ((uint)cp0 & POFFSET));

		/* contruct the vmsrclist for the move.
		 */
		src.nvecs = nb = 0;
		while (nb < numbytes)
		{
			/* skip over zero length iovecs.
			 */
			if (vec->iov_len <= 0)
			{
				vec++;
				continue;
			}

			/* get the source address for this iovec of the
			 * vmsrclist.
			 */
			src.vecs[src.nvecs].iov_base = vec->iov_base;

			/* get the source length for this iovec of the
			 * vmsrclist.
			 */
			if (nb + vec->iov_len <= numbytes)
			{
				len = vec->iov_len;
				vec++;
			}
			else
			{
				len = numbytes - nb;
			}
			src.vecs[src.nvecs].iov_len = len;

			src.nvecs++;
			nb += len;
		}
		
		ASSERT(nb == numbytes);

		/* check if the move is crossing into the next destination
		 * segment.  if so, make this segment addressable and 
		 * adjust cp0 to the start of the segment.
		 */
		sidnext = ITOS(sidx, pagex);
                if (sidnext != sid)
                {
			/* check if we are going beyond the last
			 * logical segment.
			 */
			if ((sidnext & SIDMASK) == 0)
			{
				rc = EFAULT;
				break;
			}

                        sid = sidnext;
			cp0 = cp0 - SEGSIZE;
                        (void)chgsr((uint)cp0 >> L2SSIZE,SRVAL(sid,0,0));
                }

		/* now move the page's worth of data.  if fragments are
		 * applicable, v_movefrag() is used to move the data.
		 * otherwise, exbcopy() is used if the destination page
		 * is in memory and v_movep() if the page is not in
		 * memory.
		 */
	        if (fragments && NOIND(ip->i_size + fsize))
        	{
                	touch = 0;
                	if (rc = vcs_movefrag(sid,&src,cp0,nb,&touch))
                               	nb = 0;
        	}
        	else if (lra(cp0) >= 0 || nb < PSIZE)
        	{
			/* exbcopy() the data described by the vmsrclist one
			 * iovec at a time.
			 */
			for (i = n = 0; i < src.nvecs; i++, n += len)
			{
				s = src.vecs[i].iov_base;
				len = src.vecs[i].iov_len;
                		if (rc = exbcopy(s, cp0+n, len))
                		{
					/* should only receive exceptions
					 * on the destination.
					 */
               				exaddr = CSA->o_vaddr;
					assert((exaddr & SREGMSK)
						== ((uint)cp0 & SREGMSK));

                        		rc = vmexception(s,cp0+n,rc,&len);
                        		nb = n + len;
					break;
                		}
			}
        	}
        	else
        	{
                	vcs_movep(sid,&src,cp0);
        	}

		/* adjust the uio to reflect the data moved.
		 */
                uio->uio_resid -= nb;
                uio->uio_offset += nb;
                cp0 += nb;
                nbytes -= nb;
		pagex += 1;

		/* adjust the uio iovecs to reflect the data moved.
		 */
		for (v = begvec; nb != 0; v++)
		{
			if (v->iov_len > 0)
			{
				len = MIN(nb,v->iov_len);
                		v->iov_base += len;
                		v->iov_len -= len;
				nb -= len;
			}

			if (v->iov_len <= 0)
			{
                                uio->uio_iov++;
                                uio->uio_iovcnt--;
			}
		}

		/* stop if we encountered an error.
		 */
		if (rc)
			break;
	}

	/* clear the exception handler.
	 */
	clrjmpx(&jmpbuf);

	out:

	/* drop addressability to the destination segment and
	 * regain the kernel lock if we previously released it.
	 */
	vm_det(cp1);
	if (hadlock)
		lockl(&kernel_lock,LOCK_SHORT);

	/* perform write behind for non-defer segments.
	 */
	if (!defer)
		wrbehind(BASESID(sid),offset,resid - uio->uio_resid, ip);

	/* return EFAULT if we successfully completed a partial
	 * move.
	 */
	rc = (rc == 0 && partial) ? EFAULT : rc;
	return(rc);
}

/*
 * Copy source to kernel buffer before calling critical section
 * move routine.  The source buffer is addressible in the kernel
 * address space so we just use xbcopy().
 */
static
copyin_source(sid, slist, dest1, nb, src1)
uint sid;
struct vmsrclist *slist;
uint dest1;
int nb;
uint src1;

{
	char buf[PSIZE];
	int touch, rc;
	
	if (rc = xbcopy(src1, buf, nb))
		return rc;
	(*slist).vecs[0].iov_base = buf;
	(*slist).vecs[0].iov_len = nb;
	touch = 0;
	return(vcs_movefrag(sid,slist,dest1,nb,&touch));
}
