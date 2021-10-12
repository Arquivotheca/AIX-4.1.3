static char sccsid[] =
"@(#)07  1.7  src/bos/kernel/ios/bio_pin.c, sysios, bos411, 9428A410j 6/29/94 08:33:13";

/*
 * COMPONENT_NAME: (SYSIOS) Buffer Cache Management Services -- MP safe
 *
 * FUNCTIONS:
 *	Buffer allocation services:
 *		getblk,		geteblk,	bread,		breada
 *	Buffer free services:
 *		brelse,		bwrite,		bdwrite,	bawrite
 *	Buffer management services:
 *		bflush,		blkflush,	binval,		purblk
 *	Miscellaneous services:
 *		geterror,	clrbuf
 *	I/O services:
 *		iowait,		iodone
 *	Internal routines (private to bio)
 *		bsteal,		iodone_offl	biodone,	biodone
 *		bufhw_xtnd,	incore,		internal_brelse
 *
 * ORIGINS: 3, 27, 83
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Level 1, 5 years, Bull confidental information
 */
/*
 * @BULL_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: bio_pin.c,v $
 * $EndLog$
 */
/*
 * NOTES:
 *	There are two basic states that a buffer can be in.
 *	They are "busy" and "not busy". A busy buffer has
 *	B_BUSY on and is NOT on the free list. A buffer
 *	that is not busy does not have B_BUSY on and is on
 *	the free list. A buffer is made busy when it is
 *	allocated or when I/O is initiated for it. It is
 *	busy when brelse is called, either directly
 *	or when asynchronous I/O is completed.
 *
 *	The following several routines allocate and free
 *	buffers with various side effects.  In general the
 *	arguments to an allocate routine are a device and
 *	a block number, and the return value is a pointer to
 *	to the buffer header; the buffer is marked "busy"
 *	so that no one else can touch it.  If the block was
 *	already in memory, no I/O need be done; if it is
 *	already busy, the process waits until it becomes free.
 *	The following routines allocate a buffer:
 *		getblk
 *		geteblk
 *		bread
 *		breada
 *
 *	Eventually the buffer must be released (marked as not
 *	"busy", possibly with the side effect of writing it out,
 *	by using one of:
 *		brelse
 *		bwrite
 *		bdwrite
 *		bawrite
 *
 *	A buffer should only be keep "busy" for a short period
 *	of time. It must not be keep "busy" across a long wait
 *	or a return to user level code.
 *
 *	The b_forw and b_back pointers are used to link all
 *	buffer cache headers. A named buffer is a buffer that
 *	has a device's block assigned to it. Named buffers are
 *	placed on a circular list anchored in hbuf by hash class.
 *	This list is used to reclaim named buffers.
 *
 *	Unnamed buffers are placed on a circular list anchored by
 *	bfreelist. This list is there only to simplify the logic.
 *
 *	Most of the buffer cache services only execute under a
 *	process and use the BIO module MUTEX lock to serialize their
 *	operations.
 *	They must be very careful to not alter the buffer when it is
 *	busy because I/O may be in progress. The only field altered
 *	in this case is b_event by BIO_SLEEP().
 */

/* This is the "MP-safe" version of BIO, i.e. in some places short but */
/* not optimal pieces of code are in use. If the performance tests show */
/* that this they are  bottlenecks then the more efficient but longer */
/* ones will be activated  by defining: */

/*#define	_EFFICIENT_BIO						/**/

#include <sys/buf.h>
#include <sys/var.h>
#include <sys/sysconfig.h>
#include <sys/errno.h>
#include <sys/sysinfo.h>
#include <sys/syspest.h>
#include <sys/systm.h>
#include <sys/intr.h>
#include <sys/xmem.h>
#include <sys/device.h>
#include <sys/user.h>
#include <sys/uio.h>
#include <sys/low.h>
#include <sys/processor.h>
#include "bio.h"
#include "bio_locks.h"

struct buf	*buf_hdr;
caddr_t		buffer;
struct cfgncb	buf_cfgcb;		/* config notification ctl blk	*/

#include <sys/ppda.h>			/* Each CPU has its own private */
					/* 'iodonelist', see in ppda.h */

#ifdef _POWER_MP

IODONELIST_LOCK_DECLARE;		/* Declare the IODONE-list lock */
mpc_msg_t	bio_mpc_msg;		/* Inter-CPU off-level IT */

STRATLIST_LOCK_DECLARE;			/* Declare the strategy-list lock */
mpc_msg_t	strat_mpc_msg;		/* Inter-CPU off-level IT */

#endif

BIO_MUTEX_LOCK_DECLARE;			/* Declare the BIO module MUTEX lock */


/*
 *		buffer cache free list
 *
 * Each buffer that is not currently busy is on the free list. The
 * free list is anchored by bfreelist. It is a doubly-linked circular
 * list using the av_forw and av_back pointers. The b_bcount field
 * contains the number of free buffers. The b_event field is used
 * to control waiting for free buffer cache buffers.
 *
 * The NOTAVAIL macro is used to remove a buffer from the free list.
 * It is a statement. Buffers are only put on the free list by brelse.
 *
 * The b_forw and b_back fields are used as the head of a circular
 * list of unnamed buffers (b_dev == NODEVICE). This list is equivalent
 * to one hash class in hbuf.
 */
struct	buf	bfreelist;
#define NOTAVAIL(bp)	\
{\
	(bp)->av_back->av_forw = (bp)->av_forw;\
	(bp)->av_forw->av_back = (bp)->av_back;\
	(bp)->b_flags |= B_BUSY;\
	bfreelist.b_bcount--;\
}

/*
 *		named buffer hash table
 *
 * Each named buffer is in a hash table anchored by hbuf. hbuf is an
 * array of anchors of doubly-linked circular lists of buffer headers.
 * The hash class is determined by BHASH.
 */
struct	hbuf	*hbuf;
/*
 * struct buf *	BHASH(dev,blkno)
 * dev_t	dev		device containing block
 * daddr_t	blkno		desired block
 */
#define BHASH(d,b) \
    ((struct buf *)(hbuf + (((int)d+(int)b) & (NHBUF - 1))))

/*
 * NAME:  getblk
 *
 * FUNCTION:  Assign a buffer to the given block.  If the specified
 *	block is already in the buffer cache return its buffer header.
 *	Otherwise steal a free buffer and assign it to the specified
 *	block.
 *
 *	The buffer is allocated to the caller and marked as busy.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can only be called under a process.
 *	It can page fault.
 *
 *	Anyone calling this routine should be at interrupt priority
 *	INTBASE.
 *
 * NOTES:
 *	The caller must fully initialize the contents of the buffer.
 *
 * RETURNS: Address of the selected buffer's header.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	lock/unlock routines, sleep service
 */
struct buf *
getblk(register dev_t dev, register daddr_t blkno)
/* register dev_t	dev;		the device containing block	*/
/* register daddr_t	blkno;		the block to be allocated	*/
{
	register struct buf *bp;	/* block's buffer		*/
	register int ipri;
	struct buf *bsteal();		/* assign a free buffer		*/
	struct buf *incore();		/* is block in buffer cache	*/

	ASSERT(CSA->prev == NULL);

	/*
	 * Keep trying until a buffer for the specific block is available.
	 * Everything is rechecked each time the "lock" is
	 * released due to a wait. This is because the state of the buffer
	 * cache can change during the wait.
	 */
	ipri = BIO_MUTEX_LOCK();	/* Acquire module MUTEX lock */

	ASSERT(ipri == INTBASE);

	for (;;) {
		/*
		 * Reclaim the block's buffer if it is in the buffer cache.
		 */
		if ((bp = incore(dev, blkno)) != NULL) {
			/*
			 * The block's buffer has been found. Wait for the
			 * buffer to be freed and then retry the operation,
			 * if the buffer is busy.
			 */
			if (bp->b_flags & B_BUSY) {
				INCREMENT_h(syswait.iowait);
				BIO_SLEEP(&(bp->b_event));
				/*
				 * BIO module MUTEX lock was released
				 * during BIO_SLEEP()
				 */
				DECREMENT_h(syswait.iowait);
				continue;
			}
			/*
			 * The block's buffer has been found with valid
			 * data and it is not in use. Take it off the free
			 * list, mark it as busy (in use), and return it
			 * to the caller.
			 */
			ASSERT(bp->b_flags & B_DONE);
			NOTAVAIL(bp);
			break;
		}
		/*
		 * The desired block in not in the buffer cache. Therefore
		 * steal a buffer. Initialize the block specific fields
		 * in the buffer header if a buffer is returned.
		 */
		bp = bsteal(ipri, BHASH(dev, FsLTOP(dev, blkno)));
		if (bp != NULL) {
			/*
			 * A buffer has been stolen
			 * (BIO MUTEX lock has not been released yet)
			 */
			bp->b_dev = dev;
			bp->b_blkno = FsLTOP(dev, blkno);
			bp->b_bcount = FsBSIZE(dev);
			break;
		}
	}
	BIO_MUTEX_UNLOCK(ipri);		/* Release module MUTEX lock */

	return(bp);

}   /* end getblk */

/*
 * NAME:  geteblk
 *
 * FUNCTION:  Allocate an unnamed buffer. The buffer is allocated
 *	to the caller and marked as busy.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can only be called under a process.
 *	It can page fault.
 *
 *	Anyone calling this routine should be at interrupt priority
 *	INTBASE.
 *
 * DATA STRUCTURES:
 *	bfreelist	buffer cache free list
 *
 * RETURNS: Address of the selected buffer's header.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	lock/unlock routines
 */
struct buf *
geteblk()
{
	register struct buf *bp;	/* block's buffer		*/
	register int ipri;
	struct buf *bsteal();		/* assign a free buffer		*/

	ASSERT(CSA->prev == NULL);

	ipri = BIO_MUTEX_LOCK();	/* Acquire module MUTEX lock */

	ASSERT(ipri == INTBASE);

	/*
	 * Grab a buffer off of the free list. May have to retry if
	 * bsteal had to wait.
	 */
	do
		bp = bsteal(ipri, &bfreelist);
	while (bp == NULL);

	/*
	 * From now on no one else can even to reference to this
	 * "unnamed" buffer
	 */
	BIO_MUTEX_UNLOCK(ipri);		/* Release module MUTEX lock */

	/*
	 * Initialize the block specific fields in the buffer header.
	 */
	bp->b_flags |= B_AGE;
	bp->b_vp = NULL;
	bp->b_dev = (dev_t)NODEVICE;
	bp->b_blkno = 0;
	bp->b_bcount = SBUFSIZE;

	return(bp);

}   /* end geteblk */

/*
 * NAME:  bread
 *
 * FUNCTION:  Assign a buffer to the given block.  If the specified
 *	block is already in the buffer cache return its buffer header.
 *	Otherwise steal a free buffer and assign it to the specified
 *	block. Then read the block's data into the buffer.
 *
 *	The buffer is allocated to the caller and marked as busy.
 *
 * ERROR RECOVERY:
 *	If an I/O error occurs when reading the buffer, the buffer's
 *	b_error field is set.  The caller must verify that bread() completed
 *	successfully before assuming that the buffer's data is valid.  The
 *	caller must also handle appropriately any error conditions.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can only be called under a process.
 *	It can page fault.
 *
 *	Anyone calling this routine should be at interrupt priority
 *	INTBASE.
 *
 * DATA STRUCTURES:
 *	devsw	device switch table
 *
 * RETURNS: Address of the selected buffer's header.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	pin service, devices' strategy routine
 */
struct buf *
bread(register dev_t dev, register daddr_t blkno)
/* register dev_t	dev;		the device containing block	*/
/* register daddr_t	blkno;		the block to be allocated	*/
{
	register struct buf *bp;	/* block's buffer		*/
	extern int pin();		/* routine to pin memory	*/
	register int rc;		/* return code from pin routine	*/
	void biodone();			/* buffer cache IO done routine	*/

	ASSERT(CSA->prev == NULL);
	ASSERT(CSA->intpri == INTBASE);

	INCREMENT(sysinfo.lread);

	/*
	 * Get the buffer containing the specified block.
	 */
	bp = getblk(dev, blkno);

	/*
	 * Read the block's data into the buffer if this buffer was stolen
	 * from the free list.
	 */
	if (!(bp->b_flags & B_DONE)) {

		ASSERT( bp->b_bcount == FsBSIZE(dev) );
		bp->b_iodone = biodone;
		bp->av_forw = NULL;
		bp->av_back = NULL;
#ifdef _POWER_MP
		bp->b_flags |= B_READ | B_MPSAFE;
#else
		bp->b_flags |= B_READ;
#endif
		/*
		 * Pin the buffer so that the device driver's interrupt
		 * handler can access it. It is ok to give up control
		 * here since the buffer is now marked busy.
		 */
		rc = pin(bp->b_baddr, bp->b_bcount);
		if (rc != 0){			/* On error ... */
#ifdef _POWER_MP
			bp->b_flags |= B_STALE | B_AGE | B_ERROR |
					B_DONTUNPIN | B_MPSAFE_INITIAL;
#else
			bp->b_flags |= B_STALE | B_AGE | B_ERROR |
					B_DONTUNPIN;
#endif
			bp->b_error = rc;
			iodone(bp);
			/* Note: no need to call iowait(bp) */
		} else {
			/*
			 * Call the device driver to initiate the read.
			 */
			rc = devstrat(bp);	/* Invoke strategy() */
			assert(rc == 0);
			INCREMENT(sysinfo.bread);
			if (getpid() != 0)	/* if we're not booting	*/
				INCREMENT(U.U_ru.ru_inblock);

			/*
			 * Wait for the read to complete.
			 */
			iowait(bp);
		}
	}	

	return(bp);

}   /* end bread */

/*
 * NAME:  breada
 *
 * FUNCTION:  Read in the block (like bread) and then start I/O on the
 *	read-ahead block (which is not allocated to the caller).
 *
 * ERROR RECOVERY:
 *	If an I/O error occurs when reading the buffer, the buffer's
 *	b_error field is set.  The caller must verify that bread() completed
 *	successfully before assuming that the buffer's data is valid.  The
 *	caller must also handle appropriately any error conditions.
 *	NOTE:  the read-ahead block will not be found (i.e. it will have been
 *	made non-reclaimable).
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can only be called under a process.
 *	It can page fault.
 *
 *	Anyone calling this routine should be at interrupt priority
 *	INTBASE.
 *
 * DATA STRUCTURES:
 *	devsw	device switch table
 *
 * RETURNS: Address of the selected buffer's header.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	pin service, devices' strategy routine, lock/unlock routines
 */
struct buf *
breada(register dev_t dev, register daddr_t blkno, register daddr_t rablkno)
/* register dev_t	dev;		the device containing block	*/
/* register daddr_t	blkno;		the block to be allocated	*/
/* register daddr_t	rablkno;	the read ahead block number	*/
{
	register struct buf *bp;	/* block's buffer		*/
	register struct buf *rabp;	/* read ahead block's buffer	*/
	register int rc;		/* return code from pin routine	*/
	register int ipri;
	extern int pin();		/* routine to pin memory	*/
	void biodone();			/* buffer cache I/O done	*/
	struct buf *incore();		/* is block in buffer cache	*/

	ASSERT(CSA->prev == NULL);

	/*
	 * Only initiate the read for the block if it is not
	 * already in the buffer cache. The pre-check by
	 * incore allows overlapping of any I/O in progress
	 * to the specified block with starting the read of
	 * the read ahead block.
	 */
	bp = NULL;

	ipri = BIO_MUTEX_LOCK();	/* Acquire module MUTEX lock */

	ASSERT(ipri == INTBASE);

	if (incore(dev, blkno) == NULL) {
		/*
		 * Assign a buffer to the block and initiate
		 * the read of the block into the buffer.
		 */
		BIO_MUTEX_UNLOCK(ipri);		/* Release MUTEX lock */
		INCREMENT(sysinfo.lread);
		/*
		 * Note: the buffer pool may change meanwhile
		 */
		bp = getblk(dev, blkno);
		if (!(bp->b_flags & B_DONE)) {
			ASSERT(bp->b_bcount == FsBSIZE(dev));
			bp->b_iodone = biodone;
			bp->av_forw = NULL;
			bp->av_back = NULL;
#ifdef _POWER_MP
			bp->b_flags |= B_READ | B_MPSAFE;
#else
			bp->b_flags |= B_READ;
#endif
			rc = pin(bp->b_baddr, bp->b_bcount);
			if (rc != 0){			/* On error ... */
#ifdef _POWER_MP
				bp->b_flags |= B_STALE | B_AGE | B_ERROR |
					B_DONTUNPIN | B_MPSAFE_INITIAL;
#else
				bp->b_flags |= B_STALE | B_AGE | B_ERROR |
					B_DONTUNPIN;
#endif
				bp->b_error = rc;
				iodone(bp);
				/* Note: no need to call iowait(bp) */
				goto out;
			}
			rc = devstrat(bp);	/* Invoke strategy() */
			assert(rc == 0);
			INCREMENT(sysinfo.bread);
			INCREMENT(U.U_ru.ru_inblock);
		}
		(void) BIO_MUTEX_LOCK();	/* Acquire MUTEX lock */
	}
	/*
	 * Read in the read ahead block only if one was specified, there
	 * is at least one free block on the free list, and the read
	 * ahead block is not in memory.
	 */
	if (rablkno != 0 && bfreelist.b_bcount > 1 &&
					incore(dev, rablkno) == NULL) {
		/*
		 * Assign a buffer to the read ahead block and initiate
		 * the read of it. Force it to be released after the read
		 * is finished because this buffer is not allocated
		 * to the caller by this service.
		 */
		BIO_MUTEX_UNLOCK(ipri);		/* Release module MUTEX lock */
		/*
		 * Note: the buffer pool may change meanwhile
		 */
		rabp = getblk(dev, rablkno);
		if (rabp->b_flags & B_DONE)
			brelse(rabp);
		else {
			ASSERT(rabp->b_bcount == FsBSIZE(dev));
			rabp->b_iodone = biodone;
			rabp->av_forw = NULL;
			rabp->av_back = NULL;
#ifdef _POWER_MP
			rabp->b_flags |= B_READ | B_MPSAFE | B_ASYNC;
#else
			rabp->b_flags |= B_READ | B_ASYNC;
#endif
			rc = pin(rabp->b_baddr, rabp->b_bcount);
			if (rc != 0){			/* On error ... */
				/*
				 * Read-ahead request ignored
				 */
				rabp->b_flags |= B_STALE | B_AGE;
				brelse(rabp);
			} else {
				rc = devstrat(rabp);	/* Invoke strategy() */
				assert(rc == 0);
				INCREMENT(sysinfo.bread);
				INCREMENT(U.U_ru.ru_inblock);
			}
		}
	} else
		BIO_MUTEX_UNLOCK(ipri);		/* Release module MUTEX lock */
		/*
		 * Note: the buffer pool may change
		 */

	/*
	 * Make sure that the read block is mark as busy (allocated to
	 * the caller). Also wait for the read of its data to complete.
	 */
	if (bp == NULL)
		bp = bread(dev, blkno);
	else
		iowait(bp);

out:
	return(bp);

}   /* end breada */

/*
 * NAME:  brelse, internal_brelse
 *	"brelse" cares for the mutual exclusion, "internal_brelse"
 *	executes the actual buffer release operation
 *
 * FUNCTION:  Release the buffer, with no I/O implied. The buffer
 *	is just marked as not busy so that it can be either reclaimed
 *	or reallocated.
 *
 * ERROR RECOVERY:
 *	Blocks with I/O errors are made non-reclaimable.  brelse() is
 *	used for asynchronous I/O on an interrupt level.
 *
 * EXECUTION ENVIRONMENT:
 *	These routines can be called by a process or an interrupt handler.
 *	They can page fault only if called under a process and the stack
 *	is not pinned.	The buffer header must be pinned prior to calling
 *	this routine.
 *
 *	Anyone calling these routines should be at interrupt priority
 *	INTIODONE or at a less favored priority.
 *
 * DATA STRUCTURES:
 *	bfreelist	kernel buffer free list
 *
 * RETURNS: nothing
 *
 * EXTERNAL PROCEDURES CALLED:
 *	lock/unlock routines, wakeup service
 */

static void
internal_brelse(register struct buf *bp)
/* register struct buf	*bp;		buffer to be released	*/
{
	/*
	 * Wake up anyone waiting for this buffer.
	 */
	if (bp->b_event != EVENT_NULL)
		WAKEUP(&(bp->b_event));
	/*
	 * Wake up anyone waiting on a free buffer.
	 */
	if (bfreelist.b_event != EVENT_NULL)
		WAKEUP(&(bfreelist.b_event));
	/*
	 * Make the buffer non-reclaimable if an error
	 * has occured.
	 */
	if (bp->b_flags & B_ERROR) {
		bp->b_flags |= B_STALE | B_AGE;
		bp->b_flags &= ~B_DELWRI;
	}
	/*
	 * Put buffer on freelist.
	 */
	if (bp->b_flags & B_AGE) {
		/*
		 * B_AGE flag on --> put buf at head of list,
		 * which means that it will be stolen first
		 */
		bp->av_forw = bfreelist.av_forw;
		bp->av_back = &bfreelist;
		bfreelist.av_forw->av_back = bp;
		bfreelist.av_forw = bp;
	} else {
		/*
		 * B_AGE flag off --> put buf at end of list,
		 * which means that it will be stolen later/last
		 */
		bp->av_forw = &bfreelist;
		bp->av_back = bfreelist.av_back;
		bfreelist.av_back->av_forw = bp;
		bfreelist.av_back = bp;
	}
	bp->b_flags &= ~(B_BUSY | B_ASYNC | B_AGE);
	bfreelist.b_bcount++;

}   /* end internal_brelse */

void
brelse(register struct buf *bp)
/* register struct buf	*bp;		buffer to be released	*/
{
	register int ipri;		/* current interrupt priority	*/

	ipri = BIO_MUTEX_LOCK();	/* Acquire module MUTEX lock */

	ASSERT(ipri >= INTIODONE);

	internal_brelse(bp);
	BIO_MUTEX_UNLOCK(ipri);		/* Release module MUTEX lock */

}   /* end brelse */

/*
 * NAME:  bwrite
 *
 * FUNCTION:  Write the specified buffer. The buffer is marked as not busy
 *	by brelse when the write is complete.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can only be called under a process.
 *	It can page fault. The buffer header must be pinned prior to
 *	calling this routine.
 *
 *	Anyone calling this routine should be at interrupt priority
 *	INTBASE.
 *
 * RETURNS: 0 if the I/O is launched successfully; otherwise, the errno value.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	pin service, device's strategy routine
 */
int
bwrite(register struct buf *bp)
/* register struct buf	*bp;		buffer to write		*/
{
	register int flags;		/* remember if B_ASYNC		*/
	void biodone();			/* buffer cache iodone routine	*/
	extern int pin();		/* routine to pin memory	*/
	register int rc;		/* return code		 	*/

	ASSERT(CSA->prev == NULL);
	ASSERT(CSA->intpri == INTBASE);
	assert(bp->b_flags & B_BUSY);

	flags = bp->b_flags;		/* save current flag settings	*/
	bp->b_flags &= ~(B_READ | B_DONE | B_ERROR | B_DELWRI);
	bp->b_iodone = biodone;
	bp->av_forw = NULL;
	bp->av_back = NULL;
	INCREMENT(sysinfo.lwrite);
#ifdef _POWER_MP
	bp->b_flags |= B_MPSAFE;
#endif

	/*
	 * Pin the buffer.  Note that biodone unpins
	 * the buffer when the write is finished.
	 */
	rc = pin(bp->b_baddr, bp->b_bcount);
	if (rc != 0){			/* On error ... */
#ifdef _POWER_MP
		bp->b_flags |= B_STALE | B_AGE | B_ERROR |
					B_DONTUNPIN | B_MPSAFE_INITIAL;
#else
		bp->b_flags |= B_STALE | B_AGE | B_ERROR | B_DONTUNPIN;
#endif
		bp->b_error = rc;
		iodone(bp);
		/* Note: no need to call iowait(bp) */
	} else {
		/*
		 * Call the device driver to start the write.
		 */
		rc = devstrat(bp);	/* Invoke strategy() */
		assert(rc == 0);
		INCREMENT(sysinfo.bwrite);
		if( (flags & B_DELWRI) == 0 )
			INCREMENT(U.U_ru.ru_oublock);

		/*
		 * Wait for the write to complete and then free the buffer for
		 * a synchronous operation. Note that the buffer will be freed
		 * by biodone for an asynchronous operation.
		 */
		if (!(flags & B_ASYNC)) {
			rc = iowait(bp);
			brelse(bp);
		} else
			rc = 0;		/* Since the buffer may have been */
					/*	reassigned, do not access it */
	}
	return(rc);

}   /* end bwrite */

/*
 * NAME:  bdwrite
 *
 * FUNCTION:  Release the buffer, marking it so that the block will
 *	be written to the device if the buffer is stolen.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can only be called under a process.
 *	It can page fault. The buffer header must be pinned prior to
 *	calling this routine.
 *
 *	Anyone calling this routine should be at interrupt priority
 *	INTBASE.
 *
 * RETURNS: nothing
 *
 * EXTERNAL PROCEDURES CALLED: none.
 */
void
bdwrite(register struct buf *bp)
/* register struct buf	*bp;		buffer to write		*/
{
	ASSERT(CSA->prev == NULL);
	ASSERT(CSA->intpri == INTBASE);

	assert(bp->b_flags & B_BUSY);

	/*
	 * Put the buffer on the free list and mark it as not
	 * busy. From there it can be reclaimed. It can also
	 * be stolen so it is marked as delayed write so that
	 * the block's data will be writen to the device if
	 * the buffer is reassigned.
	 */
	INCREMENT(sysinfo.lwrite);
	if( (bp->b_flags & B_DELWRI) == 0 )
		INCREMENT(U.U_ru.ru_oublock);
	bp->b_flags |= B_DELWRI | B_DONE;
	bp->b_resid = 0;
	brelse(bp);

}   /* end bdwrite */

/*
 * NAME:  bawrite
 *
 * FUNCTION:  Write the block, but don't wait for it to complete.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can only be called under a process.
 *	It can page fault. The buffer header must be pinned prior to
 *	calling this routine.
 *
 *	Anyone calling this routine should be at interrupt priority
 *	INTBASE.
 *
 * NOTES:
 *	The test for the number of free buffers does not require
 *	disabling of interrupts because buffers can only be removed
 *	from the free list under a process.
 *
 * RETURNS: 0 if the I/O was successful; otherwise, the errno value.
 *
 * EXTERNAL PROCEDURES CALLED: none
 */
int
bawrite(register struct buf *bp)
/* register struct buf	*bp;		buffer to write		*/
{
	ASSERT(CSA->prev == NULL);
	ASSERT(CSA->intpri == INTBASE);

	assert(bp->b_flags & B_BUSY);

	/*
	 * Write the block out to the device. Wait for the write to
	 * complete if we are running out of free buffers.
	 */
	if (bfreelist.b_bcount > 4) /* unsafe read of bfreelist.b_bcount */
		bp->b_flags |= B_ASYNC;
	return bwrite(bp);

}   /* end bawrite */

/*
 * NAME:  bflush
 *
 * FUNCTION:  Make sure all write-behind blocks on the device, or all
 *	devices when NODEVICE is specified, are flushed from the buffer cache.
 *	This service is typically called from unmount.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can only be called by a process.
 *	It cannot page fault.
 *
 *	Anyone calling this routine should be at interrupt priority
 *	INTBASE.
 *
 * NOTES:
 *	Any dirty busy buffers are NOT written to the device. Therefore
 *	the device should not be in use when this service is called to
 *	flush all of the device's blocks from the buffer pool.
 *
 * DATA STRUCTURES:
 *	bfreelist	buffer cache free list
 *
 * RETURNS: nothing
 *
 * EXTERNAL PROCEDURES CALLED:
 *	lock/unlock routines
 */
void
bflush(register dev_t dev)
/* register dev_t		dev;		device to flush		*/
{
	register struct buf *bp;	/* buffer on the free list	*/
	register ipri;			/* current interrupt priority   */

	ASSERT(CSA->prev == NULL);

	ipri = BIO_MUTEX_LOCK();	/* Acquire module MUTEX lock */

	ASSERT(ipri == INTBASE);

	bp = bfreelist.av_forw;
	while (bp != &bfreelist) {
		/*
		 * Write any dirty buffer whose block is on the
		 * specified device. Restart from the beginning
		 * if one is written since the BIO_MUTEX_LOCK
		 * is released before bwrite and therefore one
		 * or more buffers may have been removed from the
		 * free list. 
		 */
		if ((bp->b_flags & B_DELWRI) &&
		    ((dev == NODEVICE) || (dev == bp->b_dev))) {
			NOTAVAIL(bp);
			BIO_MUTEX_UNLOCK(ipri);	/* Release MUTEX lock */
			bp->b_flags |= B_ASYNC | B_AGE;
			bwrite(bp);
			(void) BIO_MUTEX_LOCK(); /* Acquire MUTEX lock */
			bp = bfreelist.av_forw;
			continue;
		}
		ASSERT(bp->av_forw != NULL)
		bp = bp->av_forw;

	}

	BIO_MUTEX_UNLOCK(ipri);		/* Release MUTEX lock */

}   /* end bflush */

/*
 * NAME:  blkflush
 *
 * FUNCTION:  Flush (synchronously) the specified block if it is in the
 *	buffer cache. The block's buffer can still be reclaimed from
 *	the buffer cache upon return from this service if the last
 *	pass through the loop found it in the buffer cache.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can only be called under a process.
 *	It can page fault.
 *
 *	Anyone calling this routine should be at interrupt priority
 *	INTBASE.
 *
 * RETURNS: 1 if the block was flushed in this call, otherwise 0.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	lock/unlock routines, sleep service
 */
int
blkflush(register dev_t dev, register daddr_t blkno)
/* register dev_t	dev;		device containing the block	*/
/* register daddr_t	blkno;		block to be flushed		*/
{
	register struct buf *bp;	/* block's buffer		*/

	ASSERT(CSA->prev == NULL);

/* This is the "MP-safe" version of BIO, i.e. a short but not optimal */
/* piece of code is in use. If the performance tests show that this is a */
/* bottleneck then the more efficient but longer one will be activated */
/* by defining "_EFFICIENT_BIO" at the beggining of this file */
/* (For "MP-efficient" version of BIO only.) */

#if !defined(_EFFICIENT_BIO)		/* This is a "pessimized" code */

	bp = getblk(dev, blkno);	/* It is possible that the block */
					/* is not in cache, another one */
					/* is stollen in vain */
	if (bp->b_flags & B_DELWRI) {	/* Flush the buffer if it is dirty */
		bp->b_flags &= ~B_ASYNC;
		bwrite(bp);
		return 1;
	}
	brelse(bp);			/* The buffer was clean */
	return 0;

#else	/* #else branch of "#if !defined(_EFFICIENT_BIO)" */

{
	register int ipri;

	ipri = BIO_MUTEX_LOCK();	/* Acquire module MUTEX lock */

	ASSERT(ipri == INTBASE);

	/*
	 * Have to retry from the start if the block's buffer is busy.
	 * Its state may change before this process executes again.
	 */
	for(;;) {
		/*
		 * Nothing to flush if the block is not in the buffer cache.
		 */
		bp = incore(dev, blkno);
		if (bp == NULL)
			break;

		/*
		 * Got to wait and then retry if the block's buffer is in use.
		 */
		if (bp->b_flags & B_BUSY) {
			INCREMENT_h(syswait.iowait);
			BIO_SLEEP(&(bp->b_event));
			/*
			 * BIO module MUTEX lock was released
			 * during BIO_SLEEP()
			 */
			DECREMENT_h(syswait.iowait);
			continue;
		}

		/*
		 * Flush the block's buffer if it is dirty. The buffer is
		 * removed from the free list and marked as busy so that
		 * it can not be use by anyone else while it is being written.
		 * bwrite will wait for the write to finish and then free
		 * the block since this is not an asynchronous write.
		 */
		if (bp->b_flags & B_DELWRI) {
			NOTAVAIL(bp);
			BIO_MUTEX_UNLOCK(ipri);	/* Release MUTEX lock */
			bp->b_flags &= ~B_ASYNC;
			bwrite(bp);
			return 1;
		}

		/*
		 * A clean buffer was found containing the block. Therefore
		 * return indicating that the block's buffer was not flushed
		 * in this call.
		 */
		break;

/*NOTREACHED*/
	}

	BIO_MUTEX_UNLOCK(ipri);		/* Release module MUTEX lock */

	return 0;
}

#endif	/* #if !defined(_EFFICIENT_BIO) */

}  /* end blkflush */

/*
 * NAME:  binval
 *
 * FUNCTION:  Invalidate (make non-reclaimable) all of the device's blocks
 *	in the buffer cache. This service should be called to remove all
 *	of a device's blocks from the buffer cache prior to removing the
 *	device from the system. Note that all of the device's blocks should
 *	have been flushed prior to calling this service.  Typically, this
 *	happens after the last close of the device.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can only be called under a process.
 *	It can page fault.
 *
 *	Anyone calling this routine should be at interrupt priority
 *	INTBASE.
 *
 * NOTES:
 *	The block is not unnamed by this service. Maybe it 
 *	should be so that the hash chain can be made smaller.
 *
 * DATA STRUCTURES:
 *	hbuf		buffer cache hash lists
 *
 * RETURNS: nothing
 *
 * EXTERNAL PROCEDURES CALLED:
 *	lock/unlock routines, sleep service
 */
void
binval(register dev_t dev)

/* register dev_t	dev;		device to be purged		*/
{
	register int i;			/* index in hbuf		*/
	register struct buf *dp;	/* next in hash class		*/
	register struct buf *bp;	/* block's buffer		*/
	register int ipri;

	ASSERT(CSA->prev == NULL);

	ipri = BIO_MUTEX_LOCK();	/* Acquire module MUTEX lock */

	ASSERT(ipri == INTBASE);

	/*
	 * Scan all of the named buffers to see if they contain a block
	 * on the specified device.
	 */
	i = 0;
	dp = (struct buf *)hbuf;
	bp = dp->b_forw;
	for (;;) {
		if (bp != dp) {
			/*
		 	 * Check this hash class for a buffer containing one of
		 	 * the device's blocks.
		 	 */
			if (bp->b_dev == dev && !(bp->b_flags & B_STALE)) {
				if (bp->b_flags & B_BUSY) {
					INCREMENT_h(syswait.iowait);
					BIO_SLEEP(&(bp->b_event));
					/*
					 * BIO module MUTEX lock was released
					 * during BIO_SLEEP()
					 */
					DECREMENT_h(syswait.iowait);
					i = 0;
					dp = (struct buf *)hbuf;
					bp = dp->b_forw;
					continue;
				}
				/*
			 	 * Make the block not reclaimable and put the
			 	 * buffer at the head of the free list.
			 	 */
				assert(!(bp->b_flags & B_DELWRI));
				NOTAVAIL(bp);
				BIO_MUTEX_UNLOCK(ipri); /* Release MUTEX lock */
				bp->b_flags |= B_STALE | B_AGE;
				brelse(bp);
				(void) BIO_MUTEX_LOCK(); /* Acquire MUTEX lock */
				i = 0;
				dp = (struct buf *)hbuf;
				bp = dp->b_forw;
				continue;
			}
			bp = bp->b_forw;
		} else {			/* bp == dp		*/
			i++;			/* next hash tbl entry	*/
			if (i >= NHBUF)
				break;		/* end of hash table	*/
			dp = (struct buf *)(hbuf + i);
			bp = dp->b_forw;
		}

	}  /* end for loop	*/

	BIO_MUTEX_UNLOCK(ipri);			/* Release module MUTEX lock */

}   /* end binval */

/*
 * NAME:  purblk
 *
 * FUNCTION:  Purge a block from the buffer cache. The block is
 *	not reclaimable from the buffer cache when this service
 *	completes.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can only be called under a process.
 *	It cannot page fault.
 *
 *	Anyone calling this routine should be at interrupt priority
 *	INTBASE.
 *
 * NOTES:
 *	The block is not unnamed by this service. Maybe it 
 *	should be so that the hash chain can be made smaller.
 *
 * RETURNS: nothing
 *
 * EXTERNAL PROCEDURES CALLED:
 *	lock/unlock routines, sleep service
 */
void
purblk(dev, blkno)
/* register dev_t	dev;		/* device containing the block	*/
/* register daddr_t	blkno;		/* block to be purged		*/
{
	register struct buf *bp;	/* block's buffer		*/

	ASSERT(CSA->prev == NULL);

/* This is the "MP-safe" version of BIO, i.e. a short but not optimal */
/* piece of code is in use. If the performance tests show that this is a */
/* bottleneck then the more efficient but longer one will be activated */
/* by defining "_EFFICIENT_BIO" at the beggining of this file */
/* (For "MP-efficient" version of BIO only.) */

#if !defined(_EFFICIENT_BIO)		/* This is a "pessimized" code */

	bp = getblk(dev, blkno);	/* It is possible that the block */
					/* is not in cache, another one */
					/* is stollen in vain */
	bp->b_flags |= B_STALE | B_AGE;	/* Make the block not reclaimable */
	brelse(bp);

#else	/* #else branch of "#if !defined(_EFFICIENT_BIO)" */

{
	register int ipri;

	ipri = BIO_MUTEX_LOCK();	/* Acquire module MUTEX lock */

	ASSERT(ipri == INTBASE);

	while ((bp = incore(dev, blkno)) != NULL) {

		/*
		 * The block's buffer has been found. Wait for the
		 * buffer to be freed and then retry the operation,
		 * if the buffer is busy.
		 */
		if (bp->b_flags & B_BUSY) {
			INCREMENT_h(syswait.iowait);
			BIO_SLEEP(&(bp->b_event));
					/*
					 * BIO module MUTEX lock was
					 * released during BIO_SLEEP()
					 */
			DECREMENT_h(syswait.iowait);
			continue;
		}
		/*
		 * The block's buffer has been found with vaild
		 * data and it is not in use. Take it off the free
		 * list.
		 */
		ASSERT(bp->b_flags & B_DONE);
		NOTAVAIL(bp);
		BIO_MUTEX_UNLOCK(ipri);	/* Release module MUTEX lock */

		/*
		 * Make the block not reclaimable.
		 * It is put at the head of the free list since it
		 * now does not have anything useful in it.
		 */
		bp->b_flags |= B_STALE | B_AGE;
		brelse(bp);
		return;
/*NOTREACHED*/
	}

	BIO_MUTEX_UNLOCK(ipri);		/* Release module MUTEX lock */
}

#endif	/* #if !defined(_EFFICIENT_BIO) */

}   /* end purblk */

/*
 * NAME:  geterror
 *
 * FUNCTION:  Determine the completion status of the buffer.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can be called under a process or an interrupt handler.
 *	It can page fault only if called by a process and the stack,
 *	buffer header, or buffer are not pinned.
 *
 *	The caller protects this routine.
 *
 * NOTES:
 *	The buffer must be either allocated to the caller or this service
 *	must be called in a buffer cache critical section.
 *
 * RETURNS: 0 if the I/O was successful, otherwise the errno value.
 *
 * EXTERNAL PROCEDURES CALLED:	none 
 */
int
geterror(bp)
register struct buf *bp;		/* buffer to check status of	*/
{
	register int error;		/* error status			*/

	error = 0;
	if (bp->b_flags & B_ERROR) {
		if (bp->b_error == 0)
			bp->b_error = EIO;
		error = bp->b_error;
	}
	return(error);

}  /* end geterror */

/*
 * NAME:  clrbuf
 *
 * FUNCTION:  Zero the buffer's memory.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can be called under a process or an interrupt handler.
 *	It can page fault only if called by a process and the stack,
 *	buffer header, or buffer are not pinned.
 *
 * NOTES:
 *	The buffer must already be allocated to the caller. The getblk,
 *	geteblk, bread, or breada services must be called to allocate
 *	a buffer cache buffer.
 *
 * RETURNS: nothing
 *
 * EXTERNAL PROCEDURES CALLED:
 *	bzero
 */
void
clrbuf(bp)
register struct buf *bp;		/* buffer to clear		*/
{
	bzero(bp->b_baddr, bp->b_bcount);
	bp->b_resid = 0;

}   /* end clrbuf */

/*
 * NAME:  iowait
 *
 * FUNCTION:  Wait for buffer I/O completion.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can only be called under a process.
 *
 *	It cannot page fault. The buffer header must be pinned prior
 *	to calling this service.
 *
 * NOTES:
 *	This service allows calls at interrupt priorities more favored
 *	than INTIODONE because some device drivers use it to wait for
 *	completion of buffers that are not in the buffer cache.
 *
 * RETURNS: 0 if the I/O was successful, otherwise the errno value.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	lock/unlock routines, sleep service
 */
int
iowait(register struct buf *bp)
/* register struct buf *bp;		buffer to wait on		*/
{
	register int ipri;		/* current interrupt priority	*/
	register int error;		/* error status			*/

	ASSERT(CSA->prev == NULL);

	ipri = BIO_MUTEX_LOCK();	/* Acquire module MUTEX lock */
	INCREMENT_h(syswait.iowait);
	while (!(bp->b_flags & B_DONE)) {
		BIO_SLEEP(&(bp->b_event));
		/*
		 * BIO module MUTEX lock was released during BIO_SLEEP()
		 */
	}
	DECREMENT_h(syswait.iowait);
	error = geterror(bp);
	BIO_MUTEX_UNLOCK(ipri);		/* Release module MUTEX lock */

	return(error);

}   /* end iowait */

#ifdef _POWER_MP
struct buf *g_iodonelist = NULL;	/* Global 'iodonelist' */
#endif

/*
 * NAME:  iodone
 *
 * FUNCTION:  Perform/schedule I/O done processing for the buffer.
 *	This service is call by block device drivers for completion
 *	of all requests, not just buffer cache requests.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can be called under a process or an interrupt
 *	handler.
 *
 *	It cannot page fault. The buffer header must be pinned prior
 *	to calling this service.
 *
 * NOTES:
 *	Private iodonelists are protected with IT lock-out.
 *	The global iodonelist (for funnelizing) is guarded with
 *	IODONELIST_LOCK().
 *
 * DATA STRUCTURES:
 *	iodonelist	delayed iodone list
 *	    |
 *	    |     *---------------------------------*
 *	    |     |    oldest            newest     |
 *	    |     |  *---------*       *---------*  |
 *	    *---->*->| av_forw |------>|         |--*
 *	          *--| av_back |<------|         |<-*
 *	          |  *---------*       *---------*  |
 *	          *---------------------------------*
 *
 *	Each CPU has its own private 'iodonelist', see in ppda.h.
 *	There is a global 'iodonelist' for funnelization purposes.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	lock/unlock routines, i_iodone, mpc_send
 */
void
iodone(register struct buf *bp)
/* register struct buf *bp;		I/O operation just finished	*/
{
	extern void i_iodone();		/* soft interrupt to INTIODONE	*/
	register int ipri;		/* current interrupt priority	*/
	register struct buf *iodonelist;/* actual 'iodonelist'		*/
#ifdef _POWER_MP
	register int flag;
#endif

	/*
	 * Callers of this routine should NOT set the B_DONE flag.
	 */
	assert(!(bp->b_flags & B_DONE));

	/*
	 * Just call the initiator's b_iodone routine if this call could
	 * not have pre-empted a buffer cache critical section.
	 */
#ifdef _POWER_MP
	if ((CSA->intpri > INTIODONE) && (bp->b_flags & B_MPSAFE_INITIAL)){
		ipri = i_disable(INTIODONE);
		(*bp->b_iodone)(bp);
		i_enable(ipri);
		return;
	}
#else
	if (CSA->intpri > INTIODONE){
		ipri = i_disable(INTIODONE);
		bp->b_flags |= B_DONE;		/* May be set twice */
		(*bp->b_iodone)(bp);
		i_enable(ipri);
		return;
	}
#endif

#ifdef _POWER_MP
	/*
	 * 'flag' will be TRUE if there is no need to funnelize
	 */
	flag = bp->b_flags & B_MPSAFE_INITIAL || CPUID == MP_MASTER;
	if (flag) {
#endif
		ipri = i_disable(INTMAX);
		iodonelist = PPDA -> iodonelist;
#ifdef _POWER_MP
	} else {
		ipri = IODONELIST_LOCK();
		iodonelist = g_iodonelist;	/* Funneled iodone()-s */
	}
#endif

	/*
	 * Schedule this buffer for off-level processing.
	 * Chain the buffer to the end of the off-level
	 * list. It is put at the end so that the
	 * buffers are processed in the order that
	 * they completed in.
	 */
	if (iodonelist == NULL) {
		iodonelist = bp;
		bp->av_forw = bp;
		bp->av_back = bp;
	} else {
		bp->av_forw = iodonelist;
		bp->av_back = iodonelist->av_back;
		bp->av_forw->av_back = bp;
		bp->av_back->av_forw = bp;
	}
	/*
	 * Generate a soft interrupt for the iodone
	 * off-level interrupt handler.
	 */
#ifdef _POWER_MP
	if (flag) {
#endif
		PPDA -> iodonelist = iodonelist;
		i_iodone();
		i_enable(ipri);
#ifdef _POWER_MP
	} else {
		g_iodonelist = iodonelist;
		mpc_send(MP_MASTER, bio_mpc_msg);
		IODONELIST_UNLOCK(ipri);
	}
#endif
}  /* end iodone */

/*
 * NAME:  incore
 *
 * FUNCTION:  Check to see if the block has a buffer assigned to it in
 *	the buffer cache.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can only be called under a process.
 *	It can page fault.
 *
 *      The caller protects this routine.
 *
 * DATA STRUCTURES:
 *	hbuf		buffer cache hash list
 *
 * RETURNS: The block's buffer if the block is in the buffer cache,
 *	otherwise NULL.
 */
static struct buf *
incore(dev, blkno)
register dev_t		dev;		/* the device containing block	*/
register daddr_t	blkno;		/* the block to be found	*/
{
	register struct buf *bp;	/* the block's buffer		*/
	register struct buf *dp;	/* the block's hash class list	*/

	ASSERT(CSA->prev == NULL);
	ASSERT(CSA->intpri == INTIODONE);

	/*
	 * Scan the hash list for a buffer containing the specified block.
	 * Make sure that the block does not contain stale data. This does
	 * not prove that the buffer contains valid data since the buffer
	 * can be busy.
	 */
	blkno = FsLTOP(dev, blkno);
	dp = BHASH(dev, blkno);
	for (bp = dp->b_forw; bp != dp; bp = bp->b_forw)
		if ((bp->b_blkno == blkno) &&
		    (bp->b_dev == dev)	   &&
		    (!(bp->b_flags & B_STALE)))
			return(bp);
	return(NULL);

}   /* end incore */

/*
 * NAME: bsteal  
 *
 * FUNCTION:  Steal a buffer from the free list. Write out the previous
 *	contents of the buffer if the selected one is dirty.
 *
 * ERROR RECOVERY:
 *	Errors are only reported for explicitly-requested I/O -- not for
 *	incidental I/O such as the writing out of a buffer that is currently
 *	dirty (i.e. marked as B_DELWRI).
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can only be called under a process.
 *	It can page fault. The buffer header must be pinned prior to
 *	calling this routine.
 *	Protection should be inherited from the caller
 *
 * NOTES:
 *	Unnamed buffers (dev == NODEVICE) are linked via b_forw and b_back
 *	to bfreelist. Therefore the dp parameter should be &bfreelist
 *	for an unnamed buffer.
 *
 *	Named buffers are linked by hash class in hbuf. Therefore the
 *	dp parameter should be the address of the anchor for the block's
 *	hash class in hbuf.
 *
 * DATA STRUCTURES:
 *	bfreelist	buffer cache free list
 *	hbuf		buffer cache hash list
 *
 * RETURNS: there are two cases:
 *	-  Address of the selected buffer's header
 *	-  NULL: no buffer available
 *
 * EXTERNAL PROCEDURES CALLED:
 *	lock/unlock routine, sleep_service
 */
static struct buf *
bsteal(ipri, dp)
register int ipri;		/* IT priority before BIO_MUTEX_LOCK() */
register struct buf	*dp;	/* hash class anchor		*/
{
	register struct buf *bp;	/* stolen buffer		*/
	void bionone();			/* default I/O done routine 	*/

	ASSERT(CSA->prev == NULL);
	ASSERT(dp != NULL);
	ASSERT(CSA->intpri == INTIODONE);

	/*
	 * Wait for a buffer to be freed if the free list is empty.
	 * Return telling the caller to retry since the state of the
	 * buffer cache may change.
	 */
	if (bfreelist.av_forw == &bfreelist) {
		BIO_SLEEP(&(bfreelist.b_event));
				/*
				 * BIO module MUTEX lock was released
				 * during BIO_SLEEP()
				 */
		return NULL;
	}
	/*
	 * Remove the buffer from the head of the free list and mark it as
	 * busy. 
	 */
	bp = bfreelist.av_forw;
	ASSERT(!(bp->b_flags & B_BUSY));
	NOTAVAIL(bp);
	/*
	 * Initiate the write of the previous block assigned to the buffer
	 * if the buffer is dirty. Note that this buffer will be put at the
	 * head of the free list after it is written out. Return to the caller
	 * telling them to retry.
	 */
	if (bp->b_flags & B_DELWRI && (!(bp->b_flags & B_STALE))) {
		BIO_MUTEX_UNLOCK(ipri);		/* Release module MUTEX lock */
		bp->b_flags |= B_ASYNC | B_AGE;
		bwrite(bp);
		(void) BIO_MUTEX_LOCK();	/* Acquire module MUTEX lock */
		return NULL;
	}
	/*
	 * Remove the buffer from the hash list that it was on and put
	 * it on the specified one.
	 */
	bp->b_flags = B_BUSY;
	bp->b_back->b_forw = bp->b_forw;
	bp->b_forw->b_back = bp->b_back;
	bp->b_forw = dp->b_forw;
	bp->b_back = dp;
	dp->b_forw->b_back = bp;
	dp->b_forw = bp;
	/*
	 * Initialize the rest of the buffer header except
	 * for the block specific fields.
	 */
	bp->av_forw = NULL;
	bp->av_back = NULL;
	bp->b_iodone  = bionone;
	bp->b_error = 0;
	bp->b_work = 0;
	bp->b_options = 0;
	bp->b_event = EVENT_NULL;

	return(bp);

}   /* end bsteal */

/*
 * NAME:  iodone_offl, internal_iodone_offl
 *
 * FUNCTION:  These routines are the off-level I/O done scheduler.
 *	They processe the iodonelist and call the iodone
 *	routine for each buffer in the list.
 *
 * EXECUTION ENVIRONMENT:
 *	These routines are the off-level interrupt handler for interrupt
 *	level INT_IODONE. Ther are only called by the first level interrupt
 *	handler on an interrupt level at priority INTIODONE.
 *
 *	They cannot page fault. The buffer header must be pinned prior
 *	to calling this service.
 *
 * NOTES:
 *	The interrupt handler structure for this interrupt handler
 *	is defined and referenced in these routines to force the
 *	binder to locate it in the pinned part of the kernel.
 *
 * DATA STRUCTURES:
 *	g_iodonelist		global list of buffers waiting for iodone
 *				processing
 *	PPDA->iodonelist	local lists of buffers waiting for iodone
 *				processing
 *
 * RETURNS: INTR_SUCC
 *
 * EXTERNAL PROCEDURES CALLED:
 *	lock/unlock routines, initiator's b_iodone routine
 */

/*
 * This macro is used to unchain some buffers pointed at by 'bp'
 * from the actual 'iodonelist'
 */
#define	NEXT_FROM_IODONELIST(iodonelist,bp)\
/*	register struct buf **iodonelist,				*/\
/*	register struct buf *bp;	/* I/O operation just finished	*/\
{\
		bp->b_flags &= ~B_MORE_DONE;\
		if (bp->av_forw == *iodonelist) {\
			*iodonelist = NULL;\
		} else {\
			register struct buf *nbp;\
\
			/* Set the B_MORE_DONE flag if there is at least\
			 * one more buffer to be processed by the\
			 * same iodone routine\
			 */\
			nbp = bp->av_forw;\
			ASSERT(nbp != bp);\
			do {\
				if (nbp->b_iodone == bp->b_iodone) {\
					bp->b_flags |= B_MORE_DONE;\
					break;\
				}\
				nbp = nbp->av_forw;\
			}\
			while (nbp != bp);\
			/*\
			 * Remove buffer from list\
			 */\
			*iodonelist = bp->av_forw;\
			bp->av_forw->av_back = bp->av_back;\
			bp->av_back->av_forw = bp->av_forw;\
		}\
		bp->av_forw = NULL;\
		bp->av_back = NULL;\
}


#ifdef _POWER_MP

void
internal_iodone_offl(
	register struct buf **iodonelist,
	register int flag)		/* flag == 0: private iodonelist */
{
	register struct buf *bp;	/* I/O operation just finished	*/
	register int ipri;		/* current interrupt priority	*/

	ipri = flag == 0 ?		/* Enter critical section	*/
		i_disable(INTMAX): IODONELIST_LOCK();

	/*
	 * Process each of the buffers on the delayed I/O done list.
	 */
	while ((bp = *iodonelist) != NULL) {	/* Oldest buffer on list */
		NEXT_FROM_IODONELIST(iodonelist,bp);
		if (flag == 0)		/* End of critical section      */
			i_enable(ipri);
		else {
			IODONELIST_UNLOCK(ipri);
			ASSERT(!(bp->b_flags & B_MPSAFE_INITIAL));
		}
		/* For non- MP safe drivers only: */
		if (!(bp->b_flags & B_MPSAFE_INITIAL))
			bp->b_flags |= B_DONE;
		/*
		 * Call the device's I/O done routine
		 */
		(*bp->b_iodone)(bp);
		if (flag == 0)		/* Enter critical section	*/
			(void) i_disable(INTMAX);
		else
			(void) IODONELIST_LOCK();
	}
	if (flag == 0)			/* End of critical section      */
		i_enable(ipri);
	else
		IODONELIST_UNLOCK(ipri);

}  /* end internal_iodone_offl */

#endif


void
iodone_offl()
{
	register struct buf **iodonelist;
#ifndef _POWER_MP
	register struct buf *bp;	/* I/O operation just finished	*/
	register int ipri;		/* current interrupt priority	*/
#endif

	ASSERT(CSA->prev != NULL);
	ASSERT(CSA->intpri == INTIODONE);

	/*
	 * Process each of the buffers on the delayed I/O done list.
	 */

#ifdef _POWER_MP

	/*
	 * Accesses below via *iodonelist are unsafe. Some 'buf'-s
	 * might be missed. No problem: there will be another off-
	 * level IT
	 */

	/* Private "PPDA -> iodonelist" handling */
	iodonelist = &PPDA->iodonelist;
	if (*iodonelist != NULL)
		internal_iodone_offl(iodonelist, 0);

	/* "g_iodonelist" handling -- funneled requests */
	if (CPUID == MP_MASTER) {
		iodonelist = &g_iodonelist;
		if (*iodonelist != NULL)
			internal_iodone_offl(iodonelist, 1);
	}

#else

	/*
	 * Private "PPDA -> iodonelist" handling.
	 */

	iodonelist = & PPDA -> iodonelist;
	ipri = i_disable(INTMAX);	/* Enter critical section	*/
	while ((bp = *iodonelist) != NULL) {	/* Oldest buffer on list */
		NEXT_FROM_IODONELIST(iodonelist,bp);
		i_enable(ipri);		/* End of critical section      */
		bp->b_flags |= B_DONE;
		/*
		 * Call the device's I/O done routine
		 */
		(*bp->b_iodone)(bp);
		(void) i_disable(INTMAX); /* Enter critical section	*/
	}
	i_enable(ipri);			/* End of critical section      */

#endif

}  /* end iodone_offl */

/*
 * NAME:  biodone
 *
 * FUNCTION:  Performs buffer cache I/O completion processing.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called by iodone under a process or iodone_offl
 *	on an interrupt level. In both cases it is called at interrupt
 *	priority INTIODONE.
 *
 *	It cannot page fault. The buffer header must be pinned prior
 *	to calling this service.
 *
 *	This routine is for MP safe/efficient callers only.
 *
 * NOTES:
 *	"bp->b_flags |= B_DONE" for MP safe/efficient callers was at a wrong
 *	place in iodone_offl(). It's been moved to here.
 *
 * RETURNS: nothing
 *
 * EXTERNAL PROCEDURES CALLED:
 *	wakeup service, unpin service, lock/unlock routines
 */
static void
biodone(bp)
register struct buf *bp;		/* I/O operation just finished	*/
{
	register int tmp;
	extern int unpin();		/* routine to unpin memory	*/

	ASSERT(bp->b_flags & B_BUSY);
	ASSERT(CSA->intpri == INTIODONE);
#ifdef _POWER_MP
	ASSERT(bp->b_flags & B_MPSAFE_INITIAL);
	ASSERT(!(bp->b_flags & (B_DELWRI | B_DONE)));
#else
	ASSERT(!(bp->b_flags & B_DELWRI));
	ASSERT(bp->b_flags & B_DONE);
#endif

	/*
	 * Some drivers will return an ESOFT error. This indicates that
	 * the command completed successfully after some recovery action.
	 * This action may cause a block to be scheduled for reassignment
	 * but does not translate into an error for the caller; therefore,
	 * the error is cleared here.
	 */
	if (geterror(bp) == ESOFT) {
		bp->b_resid = 0;
		bp->b_error = 0;
		bp->b_flags &= ~B_ERROR;
	}

	/*
	 * Note: it is possible that the previous pin() failed...
	 */
	if(bp->b_flags & B_DONTUNPIN)	/* If pin() failed ... */
		bp->b_flags &= ~B_DONTUNPIN;
	else {
		/*
		 * Unpin the buffer.
		 */
		tmp = unpin(bp->b_baddr, bp->b_bcount);
		assert(tmp == 0);	/* Should never fail */
	}

	tmp =  BIO_MUTEX_LOCK();	/* Acquire module MUTEX lock */

#ifdef _POWER_MP
	bp->b_flags |= B_DONE;		/* New place for setting B_DONE */
#endif

	/*
	 * For asynchronous I/O the buffer is freed and put on the free
	 * list. From there it can be either reclaimed or reassigned.
	 */
	if (bp->b_flags & B_ASYNC){
		internal_brelse(bp);
		BIO_MUTEX_UNLOCK(tmp);		/* Release module MUTEX lock */

	/*
	 * For synchronous I/O the buffer's state is not altered. The
	 * initiator of the I/O will finish processing it.
	 */
	} else {
		BIO_MUTEX_UNLOCK(tmp);		/* Release module MUTEX lock */
		WAKEUP(&(bp->b_event));
	}

}  /* end biodone */

/*
 * NAME:  bionone
 *
 * FUNCTION:  Quit, since someone forgot to set up the b_iodone field
 *	      in the buffer structure.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called by iodone under a process or iodone_offl
 *	on an interrupt level. In both cases it is called at interrupt
 *	priority INTIODONE.
 *
 *	It cannot page fault.
 *
 * RETURNS: nothing
 *
 * EXTERNAL PROCEDURES CALLED:
 *	panic
 */
void
bionone(bp)
register struct buf *bp;		/* I/O operation just finished	*/
{
#ifdef DEBUG
	panic("b_iodone field in buffer structure not initialized\n");
#endif DEBUG
} /* end bionone */

/*
 * NAME:  bufhw_xtnd
 *
 * FUNCTION:  1) Validate/invalidate proposed change to v.v_bufhw
 *		 (buffer cache high-water mark)
 *	      2) Make additional buffer headers/buffers accessable
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called by the sysconfig SYS_SETPARMS system call.
 *
 *	Anyone calling this routine should be at interrupt priority
 *	INTBASE.
 *
 * RETURNS:	0 upon successful completion:  proposed v.v_bufhw value is
 *	valid (for CFGV_PREPARE) or requested buffer cache high-water mark
 *	has been implemented by making additional buf headers/buffers
 *	accessable (for CFGV_COMMIT);
 *		byte-offset-of-v_bufhw upon failure: proposed value is invalid.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	locking/ulocking routines, pin(), xmattaxh(), panic()
 */
int
bufhw_xtnd(
register int	cmd,		/* command: CFGV_PREPARE/CFGV_COMMIT	*/
struct var	*cur,		/* current values of var structure	*/
struct var	*new)		/* proposed values of var structure	*/
{
	register int	rc;	/* return code: var struct offset or 0	*/
	register int	i;	/* loop counter: old bufhw-->new bufhw	*/
	register struct buf *bp;
	register struct buf *dp;
	register char *buf_pool;

	ASSERT(CSA->prev == NULL);
	ASSERT(CSA->intpri == INTBASE);
	rc = 0;

	switch (cmd) {
		case CFGV_PREPARE:
			/*
			 * The proposed v_bufhw is invalid if
			 *    it is larger than the size of the malloc'd
			 *    buffer cache (NBUF #buffers).
			 */
			if (new->v_bufhw > NBUF)
				rc = (int)(&(new->v_bufhw)) - (int)(new);
			/*
			 * if the proposed v_bufhw is smaller than
			 * the current v_bufhw, quietly set it back
			 * to the current v_bufhw (because the calling
			 * routine sets the current v_bufhw to new).
			 * we really can't decrease v_bufhw w/o
			 * major backflips.
			 */
			else if (new->v_bufhw < cur->v_bufhw)
				new->v_bufhw = cur->v_bufhw;
			break;
		case CFGV_COMMIT:
			if (new->v_bufhw != cur->v_bufhw) {
				/*
			 	 * Chain the additional buf structs onto
			 	 * the existing list -- make them accessable
			 	 * to the user.
			 	 * buf_hdr is the ptr to the 1st buf struct;
			 	 * buffers is the ptr to the 1st buffer;
			 	 */
				bp = buf_hdr + cur->v_bufhw;
				rc = pin((caddr_t)bp,
					(new->v_bufhw - cur->v_bufhw) *
							sizeof(struct buf));
				if (rc != 0)
					break;
				buf_pool = buffer + (cur->v_bufhw * PAGESIZE);
				/* 'dp' is a constant pointer to free list */
				dp = &bfreelist;

				for (i = cur->v_bufhw; i < new->v_bufhw;
					i++, bp++, buf_pool += PAGESIZE) {
					bp->b_flags = B_BUSY | B_AGE;
					rc = BIO_MUTEX_LOCK();
					bp->b_forw = dp->b_forw;
					bp->b_back = dp;
					dp->b_forw->b_back = bp;
					dp->b_forw = bp;
					BIO_MUTEX_UNLOCK(rc);
					bp->av_forw = NULL;
					bp->av_back = NULL;
					bp->b_iodone = bionone;
					bp->b_vp = NULL;
					bp->b_dev = NODEVICE;
					bp->b_blkno = 0;
					bp->b_baddr = buf_pool;
					bp->b_bcount = 0;
					bp->b_error = 0;
					bp->b_resid = 0;
					bp->b_work = 0;
					bp->b_options = 0;
					bp->b_event = EVENT_NULL;
					bp->b_start.tv_sec = 0;
					bp->b_start.tv_nsec = 0;
					bp->b_xmemd._u._aspace_id = XMEM_INVAL;
					bp->b_xmemd.subspace_id = 0;
					rc = xmattach(bp->b_baddr, PAGESIZE,
						&(bp->b_xmemd), UIO_SYSSPACE);
					assert(rc == XMEM_SUCC);
					brelse(bp);
					rc = 0;
				}
			}
			break;
#ifdef DEBUG
		default:
			panic("bufhw_xtnd: invalid cmd.");
#endif DEBUG
	}

	return(rc);

} /* end bufhw_xtnd */
