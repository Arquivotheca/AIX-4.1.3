static char sccsid[] =
"@(#)09 1.46 src/bos/kernel/ios/clist.c, sysios, bos411, 9428A410j 6/7/94 22:30:53";

/*
 * COMPONENT_NAME: (SYSIOS) Character I/O services
 *
 * FUNCTIONS:	getcf, putcf, getcx, putcx, getc, putc,
 *		putcfl, getcb, putcb, getcbp, putcbp,
 *		waitcfree, pincf,
 *		<put_pinned_front>, <put_unpinned>,
 *		<get_unpinned>,	<put_pinned_back>,
 *		<get_pinned>
 *		Routines enclosed in < > are for use by routines
 *		in this module to maintain the cblock array.
 *
 * ORIGINS: 3, 27, 83
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
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
 * $Log: clist.c,v $
 * $EndLog$
 */

#include	<sys/param.h>
#include	<sys/types.h>
#include	<sys/intr.h>
#include	<sys/cblock.h>
#include	<sys/sleep.h>
#include	<sys/pin.h>
#include	<sys/syspest.h>
#include	<sys/mstsave.h>	/* for *csa				*/
#include	<sys/low.h>	/* for *csa				*/
#include	"cio_locks.h"
#include	"cio.h"


/*
 * NOTES:
 *
 *    - Throughout this file comments speak of a single unpinned free list
 *	that has two end where free cblocks can be added to / removed from.
 *
 *	One had better to consider them to be two separate lists:
 *		- c_maint.cfreelist called "front" and 
 *		- c_maint.endcfree called "back" or "end"
 *	rather than the beginning and the end of a single list.
 *
 *    - Locking order:
 *
 *	CLIST_LOCK		--	acquired first
 *	CIO_MUTEX_LOCK		--	acquired last
 *
 *	and:
 *
 *	C_MAINT_PINCF_LOCK	--	acquired first
 *	C_MAINT_LOCK
 *	CIO_MUTEX_LOCK		--	acquired last
 *
 *    - get_unpinned(int) can be invoked as follows:
 *
 *	get_unpinned(NOT_LOCKED):	CLIST_LOCK is not acquired
 *
 *	get_unpinned(ipri):		CLIST_LOCK has been acquired,
 *					the orig. IT priority was 'ipri'
 *
 *	get_unpinned() releases CLIST_LOCK if applicable, and if a free
 *	cblock is available, then get_unpinned() reacquires CLIST_LOCK
 */
#define	NOT_LOCKED	-1


static void put_pinned_front(struct cblock *bp);
static void put_pinned_back(struct cblock *bp);
static void put_unpinned(struct cblock *bp);
static struct cblock *get_pinned();
static struct cblock *get_unpinned(int);

CIO_MUTEX_LOCK_DECLARE;		/* CIO module mutex lock */
CLIST_LOCK_DECLARE;		/* Global CLIST lock */

struct	c_maint	c_maint;	/* This structure contains all the vital*/
				/* information needed to maintain the	*/
				/* cblocks in the system (e.g. number,	*/
				/* high water mark, pointers to the free*/
				/* lists, etc.).			*/

enum c_state { c_unpinned, c_pinned, c_busy };

#ifdef DEBUG
#define VERIFY_CL(pcl) { \
    struct clist *tpcl = pcl;     \
    struct cblock *tbp = tpcl->c_cf; \
    int tot = 0; \
    while (tbp) { \
	tot += (tbp->c_last - tbp->c_first); \
	tbp = tbp->c_next; \
    } \
    assert( tot == tpcl->c_cc ); \
}
#else
#define VERIFY_CL(pcl) sizeof(pcl)
#endif

/*
 * NAME: getcf
 *
 * FUNCTION: Get a cblock from the appropriate free list and initialize
 *		its fields.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can be called from a process or an interrupt handler.
 *
 *	This routine only page faults when called under a process and the
 *	  stack is not in memory or when running at INTBASE and get_unpinned
 *        is called.
 *
 * NOTES: Whether this service is called by a process or an interrupt
 *		handler, the desired return value is a pointer to a
 *		pinned cblock.  If there are no free cblocks,
 *		then a NULL pointer is returned.  It is then the caller's
 *		responsibility (if the caller is a process) to call
 *		waitcfree().  The waitcfree() process will for a free one.
 *
 *	  If this service is called from a disabled critical section, control
 *		will not be given up.
 *
 * DATA STRUCTURES:  c_maint (contains all the data about the free lists)
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer to a pinned cblock or a NULL
 *				pointer if no cblock resides on the pinned
 *				free list.
 *
 * EXTERNAL PROCEDURES CALLED:  None
 */
struct cblock *
getcf()
{
    register struct	cblock	*bp;		/* ptr to block of chars   */

    bp = (CSA->intpri == INTBASE) ?
		get_unpinned(NOT_LOCKED) : get_pinned();
    if (bp != NULL) {				/* On success... */
	bp->c_next = NULL;
	bp->c_first = 0;
	bp->c_last = CLSIZE;
    }
    return (bp);
}

/*
 * NAME: putcf
 *
 * FUNCTION: Free a cblock by putting it back on one of the free lists.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can be called from a process or an interrupt handler.
 *
 *	This routine is serialized with CIO_MUTEX_LOCK.
 *
 *	This routine only page faults when called under a process and the
 *	  stack is not in memory.
 *
 * NOTES: Which free list the cblock gets put back on depends upon
 *	  the following algorithm:
 *		1)  It shall be put on the front of the pinned free
 *		    list if the number of cblocks on the pinned free
 *		    list (maintained as c_maint.pincfree) plus some
 *		    carefully determined constant (DELTA) is less than
 *		    the number of pinned cblocks that the drivers tell
 *		    us are required (maintained as c_maint.pincreq).
 *		2)  It shall be put on the back of the pinned free list
 *		    if the index of the cblock in the cblock array is
 *		    less than or equal to pincreq.
 *		3)  Failing the above two, it shall be unpinned and will
 *		    be added to the front of the unpinned free list if
 *		    its index in the cblock array is greater than the
 *		    highwater mark divided by 2, otherwise it will be
 *		    added to the back of the unpinned free list.
 *
 *	  In the case of the unpinned free list, we are attempting
 *	  to achieve a reasonable sense of ordering without paying
 *	  a heavy price.  Retrieving and pinning a cblock from a
 *	  completely unordered unpinned free list would increase
 *	  the chance that the pinned cblocks will span more pages,
 *	  and this we would like to discourage.
 *
 *	  If there are any processes sleeping (waiting for a free
 *	  cblock), they are woken up.
 *
 *	  The cblock passed as an input parameter MUST be pinned.
 *
 *	  If this service is called from a CLIST critical section, control
 *	  will not be given up.
 *
 * DATA STRUCTURES: c_maint (contains all the data about the free lists)
 *
 * RETURN VALUE DESCRIPTION:  putcf does not have a returned value.
 *
 * EXTERNAL PROCEDURES CALLED:  wakeup routine, locking routines
 */
void
putcf(
	struct cblock *bp)		/* ptr to a PINNED cblock	*/
{
    register int	ipri;		/* caller's interrupt level	*/
    register int	cbindex;	/* index of cblock in system	*/
					/* cblock array			*/

    ipri = CIO_MUTEX_LOCK ();		/* Acquire module mutex lock */

    if (c_maint.pincfreelist.c_size <= c_maint.pincreq) {
	/*
	 *  Our algorithm tells us to add this cblock to the front
	 *  of the pinned free list.
	 */
	put_pinned_front(bp);
    } else {
	/*
	 *  Determine which element of the cblock array is being
	 *  put on a free list.  This will be used as a heuristic
	 *  to provide some order of the free list based on page.
	 *  This should be a simple way of improving the odds of a
	 *  page of free cblocks while not having to pay the path
	 *  length to fully order the free list.
	 */
	if (bp <= (c_maint.cstart + c_maint.pincreq)) {
	    /*
	     *  Our algorithm tells us to add this cblock to
	     *  the END of the pinned free list.
	     */
	    put_pinned_back(bp);
	} else {
	    /*
	     *  Our algorithm tells us to add this cblock to
	     *  the unpinned free list so add the cblock to
	     *  that free list and then unpin it.
	     */
	    put_unpinned(bp);
	}
    }
    if (c_maint.waiters != EVENT_NULL)
	WAKEUP(&c_maint.waiters);
    CIO_MUTEX_UNLOCK ( ipri );		/* Release module mutex lock */
}

/*
 * NAME:  getcx
 *
 * FUNCTION:  Get last character put onto "queue" (i.e., treat as stack).
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can be called from a process or an interrupt handler.
 *
 *	This routine is serialized by CLIST_LOCK()
 *
 *	This routine only page faults when called under a process and the
 *	  stack is not in memory.
 *
 * NOTES:  This routine checks to see if there are any characters on the
 *         queue.
 *
 *         If so, the character count is decremented.
 *         The last character in the queue is retrieved; its sign bit
 *         is turned off.
 *         If this character is the last in its cblock, then the cblock
 *         is returned to the front of the free cblock list, and any
 *         processes that are sleeping (waiting for a free cblock) are
 *         woken up.
 *
 *         If there are no characters on the queue,  (-1) is returned.
 *
 * DATA STRUCTURES:  clist, cblock
 *
 * RETURN VALUE DESCRIPTION:  getcx returns an integer (c):
 *                              1) (-1) if no characters in queue, OR
 *                              2) last character in queue
 *
 * EXTERNAL PROCEDURES CALLED:  locking routines
 */
int
getcx(struct clist *p)			/* ptr to char list queue	*/
{
	register struct cblock	*bp;	/* ptr to block of chars	*/
	register struct cblock	*obp;	/* ptr to block of chars	*/
	register int	c;		/* char retrieved from queue	*/
	register int	ipri;		/* caller's interrupt level	*/

	ipri = CLIST_LOCK();		/* Acquire global CLIST lock */
	if (p->c_cc > 0) {		/* at least 1 character in the queue */
		--p->c_cc;
		bp = p->c_cl;
		/* Get last char in queue & turn off high-order (sign) bits */
		c = ((int)((uchar)(bp->c_data[--bp->c_last])));
		if (bp->c_last == bp->c_first){	/* cblock is now empty */
						/* Scan to end of clist.  */
			for (bp = p->c_cf, obp = (struct cblock *)NULL;
			     bp != p->c_cl;
			     obp = bp, bp = bp->c_next)
				;      		/* obp is last valid block */
			/*
			 *  Adjust pointers such that the old next-to-the-last
			 *  cblock is now the last cblock
			 */
			if (obp != NULL)
				obp->c_next = (struct cblock *)NULL;
			p->c_cl = obp;
			if (bp == p->c_cf) {
			    p->c_cf = (struct cblock *)NULL;
			    p->c_cc = 0;
			}
			/*  Return empty cblock to front of freelist.  */
			putcf(bp);
		}  /* if last = first */
	}  /* if p->c_cc > 0 */
	else {			 	/* no characters in queue */
		c = -1;
	}
	CLIST_UNLOCK(ipri);		/* Release global CLIST lock */
	return (c);
}

/*
 * NAME:  putcx
 *
 * FUNCTION:  Put a character onto front of "queue" (i.e., treat as stack).
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can be called from a process or an interrupt handler.
 *
 *	This routine is serialized by CLIST_LOCK()
 *
 *	This routine only page faults when called under a process and the
 *	  stack is not in memory or when get_unpinned is called (at INTBASE).
 *
 * NOTES:  This routine checks to see if the queue is empty, or if the
 *         0th position in the c_data array is already filled.
 *
 *         If either condition is true, then the freelist is checked.
 *         If there are no free cblocks in the freelist, (-1) is returned.
 *         Otherwise, the first free cblock is added to the front of the
 *         queue.
 *
 *         The character is then put in the 0th position of the c_data
 *         array of the first cblock in the queue.  The character count is
 *         then incremented.
 *
 * DATA STRUCTURES:  clist, cblock
 *
 * RETURN VALUE DESCRIPTION:  putcx returns an integer (c):
 *                              1) (-1) if no free cblocks in freelist, OR
 *                              2) 0 if 'put' is successful
 *
 * EXTERNAL PROCEDURES CALLED:  locking routines
 */
int
putcx(
    char c,				/* char to be put on queue */
    struct clist *p)			/* ptr to char list queue */
{
    register struct cblock *bp;		/* ptr to block of chars  */
    register int ipri;			/* caller's interrupt level */

    ipri = CLIST_LOCK();		/* Acquire global CLIST lock */
    if ((bp = p->c_cf) == NULL || bp->c_first == 0) {
	/*
	 * An "in-line" version of getcf() follows:
	 */
	if (ipri == INTBASE) {		/* Get a free cblock */
	    if ((bp = get_unpinned(ipri)) == NULL)
		/*
		 * On failure, get_unpinned() did not reacquire CLIST_LOCK
		 */
		return -1;
	} else {
	    if ((bp = get_pinned()) == NULL) {
		CLIST_UNLOCK(ipri);	/* Release global CLIST lock */
		return -1;
	    }
	}

	bp->c_next = p->c_cf;		/* Put new cblock at */
	bp->c_first = 1;		/*	front of the queue */
	bp->c_last = 1;
	if (p->c_cl == NULL)
	    p->c_cl = bp;
	p->c_cf = bp;
    }
    bp->c_data[--bp->c_first] = c;	/* put char in 0th position of array */
    ++p->c_cc;				/* increment character count */
    CLIST_UNLOCK(ipri); 		/* Release global CLIST lock */
    return (0);
}

/*
 * NAME:  getc
 *
 * FUNCTION:  Get a character from the front of the queue
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can be called from a process or an interrupt handler.
 *
 *	This routine is serialized by CLIST_LOCK()
 *
 *	This routine only page faults when called under a process and the
 *	  stack is not in memory.
 *
 * NOTES:  This routine checks to see if there are any characters on the
 *         queue.
 *
 *         If so, the character count is decremented.
 *         The first character in the queue is retrieved; its sign bit
 *         is turned off.
 *         If this character is the last in its cblock, then the cblock
 *         is returned to the front of the free cblock list, and any
 *         processes that are sleeping (waiting for a free cblock) are
 *         woken up.
 *
 *         If there are no characters on the queue,  (-1) is returned.
 *
 * DATA STRUCTURES:  clist, cblock
 *
 * RETURN VALUE DESCRIPTION:  getc returns an integer (c):
 *                              1) (-1) if no characters in queue, OR
 *                              2) first character in queue
 *
 * EXTERNAL PROCEDURES CALLED:  locking routines
 */
int
getc(struct clist *p)				/* ptr to char list queue    */
{
	register struct cblock *bp;		/* ptr to block of chars     */
	register int c;				/* char retrieved from queue */
	register int ipri;			/* caller's interrupt level  */

	ipri = CLIST_LOCK();		/* Acquire global CLIST lock */
	if (p->c_cc > 0) {		/* any chars in the queue? */
		p->c_cc--;		/* decrement character count */
		bp = p->c_cf;
		/*  Get last char in queue & turn off high-order (sign) bit.  */
		c = ((int)((uchar)(bp->c_data[bp->c_first++])));
		if (bp->c_first == bp->c_last) {  /* cblock is empty */
			if ((p->c_cf = bp->c_next) == (struct cblock *)NULL) {
			    p->c_cl = (struct cblock *)NULL;
			    p->c_cc = 0;
			}
			/*  Return empty cblock to front of freelist.  */
			putcf(bp);
		}
	}  /* if p->c_cc > 0 */
	else  {
		c = -1;
	}
	CLIST_UNLOCK(ipri);		/* Release global CLIST lock */
	return (c);
}

/*
 * NAME:  putc
 *
 * FUNCTION:  Put a character onto end of "queue"
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can be called from a process or an interrupt handler.
 *
 *	This routine is serialized by CLIST_LOCK()
 *
 *	This routine only page faults when called under a process and the
 *	  stack is not in memory or get_unpinned is called (at INTBASE).
 *
 * NOTES:  This routine checks to see if the queue is empty, or if the
 *         last position in the c_data array is already filled.
 *
 *         If either condition is true, then the freelist is checked.
 *         If there are no free cblocks in the freelist, (-1) is returned.
 *         Otherwise, the first free cblock is added to the end of the
 *         queue.
 *
 *         The character is then put in the last position of the c_data
 *         array of the last cblock in the queue.  The character count
 *         is then incremented.
 *
 * DATA STRUCTURES:  clist, cblock
 *
 * RETURN VALUE DESCRIPTION:  putc returns an integer (c):
 *                              1) (-1) if no free cblocks in freelist, OR
 *                              2) 0 if 'put' is successful
 *
 * EXTERNAL PROCEDURES CALLED:  locking routines
 *
 */
int
putc(
    char c,				/* char to be put on queue */
    struct clist *p)			/* ptr to char list queue  */
{
    register struct cblock *bp;		/* ptr to block of chars   */
    register ipri;			/* caller's interrupt level */

    ipri = CLIST_LOCK();		/* Acquire global CLIST lock */
    if ((bp = p->c_cl) == NULL || (bp->c_last == CLSIZE)) {
	/*
	 * An "in-line" version of getcf() follows:
	 */
	if (ipri == INTBASE) {		/* Get a free cblock */
	    if ((bp = get_unpinned(ipri)) == NULL)
		/*
		 * On failure, get_unpinned() did not reacquire CLIST_LOCK
		 */
		return -1;
	} else {
	    if ((bp = get_pinned()) == NULL) {
		CLIST_UNLOCK(ipri);	/* Release global CLIST lock */
		return -1;
	    }
	}

	bp->c_next = (struct cblock *)NULL;	/* Put new cblock at the */
	bp->c_first = bp->c_last = 0;		/*	end of the equeue */
	if (p->c_cl == NULL)
	    p->c_cf = bp;
	else
	    p->c_cl->c_next = bp;
	p->c_cl = bp;
    }
    bp->c_data[bp->c_last++] = c;	/* put char in last position of array */
    p->c_cc++;				/* increment character count */
    CLIST_UNLOCK(ipri); 		/* Release global CLIST lock */
    return (0);
}

/*
 * NAME:  putcfl
 *
 * FUNCTION:  Return entire free clist to global list.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can be called from a process or an interrupt handler.
 *
 *	This routine is serialized by CLIST_LOCK()
 *
 *	This routine only page faults when called under a process and the
 *	  stack is not in memory.
 *
 * NOTES:  This routine uses getcb to remove a cblock from the front of
 *         the queue; putcf is used to add this cblock to the front of
 *         the free cblock list.
 *
 *         This process is repeated until there are no more cblocks in
 *         the queue.
 *
 * DATA STRUCTURES:  clist, cblock
 *
 * RETURN VALUE DESCRIPTION:  putcfl does not have a returned value.
 *
 * EXTERNAL PROCEDURES CALLED:  locking routines
 */
void
putcfl(struct clist *p)		/* PINNED ptr to char list queue*/
{
    register int ipri;		/* caller's interrupt level	*/
    register struct cblock *last;	/* ptr to block of chars	*/
    register struct cblock *tmpcb1;	/* temp ptr to free cblocks	*/
    register struct cblock *tmpcb2;	/* temp ptr to free cblocks	*/

    ipri = CLIST_LOCK();		/* Acquire global CLIST lock */
    if ((last = p->c_cl) != NULL) {
	last->c_next = (struct cblock *)NULL;
	tmpcb1 = p->c_cf;
	do  {
	    tmpcb2 = tmpcb1->c_next;
	    putcf(tmpcb1);		/* free the next cblock	*/
	    tmpcb1 = tmpcb2;
	} while (tmpcb2);
	p->c_cf = p->c_cl = NULL;	/* pointers cleared	*/
	p->c_cc = 0;			/* zero the character count	*/
    }
    CLIST_UNLOCK(ipri); 		/* Release global CLIST lock */
}

/*
 * NAME:  getcb
 *
 * FUNCTION:  Get entire block from front of queue.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can be called from a process or an interrupt handler.
 *
 *	This routine is serialized by CLIST_LOCK()
 *
 *	This routine only page faults when called under a process and the
 *	  stack is not in memory.
 *
 * NOTES:  This routine checks to see if there are any cblocks in the queue.
 *
 *         If so, the clist character count is decremented by the number
 *         of characters in the first cblock.  The pointers are then
 *         adjusted so that the 2nd cblock in the queue is now the first.
 *         A pointer to the old first cblock is returned.
 *
 *         If there are no cblocks in the queue, NULL is returned.
 *
 * DATA STRUCTURES:  clist, cblock
 *
 * RETURN VALUE DESCRIPTION:  getcb returns a pointer:
 *                              NULL if no cblocks in the queue, OR
 *                              pointer to cblock struct
 *
 * EXTERNAL PROCEDURES CALLED:  locking routines
 */
struct cblock *
getcb(struct clist *p)			/* PINNED ptr to char list queue*/
{
	register struct cblock *bp;	/* ptr to block of chars	*/
	register int ipri;		/* caller's interrupt level	*/

	ipri = CLIST_LOCK();		/* Acquire global CLIST lock */
	if ((bp = p->c_cf) != (struct cblock *)NULL) {	/* cblocks in queue? */
		p->c_cc -= bp->c_last - bp->c_first; /* decr. char count     */
						/* more cblocks in queue?*/
		if ((p->c_cf = bp->c_next) == (struct cblock *)NULL)
			p->c_cl = (struct cblock *)NULL;
		bp->c_next = NULL;
	}
	CLIST_UNLOCK(ipri);		/* Release global CLIST lock */
	return (bp);
}

/*
 * NAME:  putcb
 *
 * FUNCTION:  Put entire block to end of queue.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can be called from a process or an interrupt handler.
 *
 *	This routine is serialized by CLIST_LOCK()
 *
 *	This routine only page faults when called under a process and the
 *	  stack is not in memory.
 *
 * NOTES:  This routine adjusts the clist pointers so that the new cblock
 *         is now the last one in the queue.  The clist character count
 *         is then incremented by the number of characters in the new
 *         cblock.  An integer value of 0 is returned.
 *
 * DATA STRUCTURES:  clist, cblock
 *
 * RETURN VALUE DESCRIPTION:  putcb does not have a returned value.
 *
 * EXTERNAL PROCEDURES CALLED:  locking routines
 */
void
putcb(
    struct cblock *bp,			/* ptr to block of chars   */
    struct clist *p)			/* ptr to char list queue  */
{
    register int	ipri;			/* caller's interrupt level*/

    ipri = CLIST_LOCK();		/* Acquire global CLIST lock */
    if (!p->c_cl)
	p->c_cf = bp;
    else
	p->c_cl->c_next = bp;
    p->c_cl = bp;			/* new cblock now last in clist  */
    bp->c_next = NULL;
    p->c_cc += bp->c_last - bp->c_first; /* increase character count */
    CLIST_UNLOCK(ipri); 		/* Release global CLIST lock */
}

/*
 * NAME:  getcbp
 *
 * FUNCTION:  Get n-length string from front of queue.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can be called from a process or an interrupt handler.
 *
 *	This routine is serialized by CLIST_LOCK()
 *
 *	This routine only page faults when called under a process and the
 *	  stack is not in memory.
 *
 * NOTES:  This routine checks to see if there are any cblocks in the queue.
 *
 *         If so, as many characters as are needed are retrieved from the
 *         first cblock's c_data array.  If the requested number of
 *         characters was found in that cblock, then the process is
 *         complete, and the number of characters retrieved is returned to
 *         the caller.  If there weren't enought characters in that cblock,
 *         then it is returned to the free cblock list (since it is now
 *         empty), and the next cblock is used to get more characters.
 *         This process continues until the requested number of characters
 *         is retrieved, or until there are no more cblocks in the queue.
 *         The number of characters retrieved (whether or not the requested
 *         number was found) is returned.
 *
 *         If there are no cblocks in the queue, (0) is returned.
 *
 * DATA STRUCTURES:  clist, cblock
 *
 * RETURN VALUE DESCRIPTION:  getcbp returns an integer (n):
 *                              (0) if no cblocks in queue, OR
 *                              number of characters retrieved from queue
 *
 * EXTERNAL PROCEDURES CALLED:  locking routines
 *                              bcopy
 */
int
getcbp(
    struct clist *p,	/* ptr to char list queue		*/
    char *cp,		/* ptr to char (where chars will be put)*/
    int n)		/* length of string to be retrieved	*/
{
	register struct cblock *bp;	/* ptr to block of chars	*/
	register char	*op;		/* ptr to 1st char in c_data array*/
	register int	on;		/* #chars in c_data array	*/
	register char	*acp = cp;	/* ptr to char where char string*/
					/* will be put			*/
	register int ipri;		/* caller's interrupt level	*/

	ipri = CLIST_LOCK();		/* Acquire global CLIST lock */
	while (n) {			/* more characters are needed	*/
		if ((bp = p->c_cf) == (struct cblock *)NULL) {
			break;		/* no cblocks in clist		*/
		}
		op = &bp->c_data[bp->c_first];  /* get first char in array */
		on = bp->c_last - bp->c_first;  /* get # of chars in array */
		if (n >= on) {
			/*
			 *  Need more characters than exist in this cblock
			 *  use bcopy to copy 'on' chars from op to cp
			 */
			bcopy(op, cp, on);
			cp += on;	/* point past last char retrieved   */
			n -= on;	/* decrement number of chars needed */
			p->c_cc -= on;  /* decrement number of chars in clist */
			if ((p->c_cf = bp->c_next) == (struct cblock *)NULL) {
				/*  no more cblocks in clist.  */
				p->c_cl = (struct cblock *)NULL;
			}
			/*  Return empty cblock to the freelist.  */
			putcf(bp);
		} else {
			/*  Need only characters that exist in this cblock.  */
			bcopy(op, cp, n);  /* copy 'n' chars from op to cp   */
			bp->c_first += n;  /* increment position of 1st char */
			cp += n;	/* point past last char retrieved */
			p->c_cc -= n;	/* decrement number of chars in clist */
			n = 0;		/* no more chars needed           */
			break;
		}
	}  /* while n */
	CLIST_UNLOCK(ipri);		/* Release global CLIST lock */
	return (cp - acp);
}

/*
 * NAME:  putcbp
 *
 * FUNCTION:  Put n-length string to end of queue.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can be called from a process or an interrupt handler.
 *
 *	This routine is serialized by CLIST_LOCK()
 *
 *	This routine only page faults when called under a process and the
 *	  stack is not in memory or if get_unpinned is called (at INTBASE).
 *
 * NOTES:  This routine checks to see if there is any space in the last
 *         cblock in the queue.
 *
 *         If not, the free cblock list is checked to see if there are
 *         any free cblocks.  If not, (0) is returned to the caller.
 *         If there is a free cblock, it is added to the end of the queue.
 *
 *         As many characters as exist are put into the last cblocks's
 *         c_data array.  If the requested number of characters fits in
 *         that cblock, then the process is complete, and the number of
 *         characters written is returned to the caller.
 *         If there was not enough space in that cblock, then another cblock
 *         is gotten (if any exist) from the freelist and more data is
 *         written to it.
 *
 *         This process continues until the requested number of characters
 *         is written, or until there are no more cblocks in the freelist.
 *         The number of characters written (whether or not the requested
 *         number was written) is returned.
 *
 * DATA STRUCTURES:  clist, cblock
 *
 * RETURN VALUE DESCRIPTION:  putcbp returns an integer (n):
 *                              (0) if no space in clist and no free cblocks
 *                                  in the freelist, OR
 *                              number of characters put in the queue
 *
 * EXTERNAL PROCEDURES CALLED:  locking routines
 *                              bcopy
 */
int
putcbp(
    struct clist *p,	/* ptr to char list queue			*/
    char *cp,		/* ptr to char (where char string will be put)	*/
    int n)		/* integer length of string to be retrieved	*/
{
    register struct cblock *bp;		/* ptr to block of chars	*/
    register struct cblock *obp;	/* ptr to block of chars	*/
    register char	*op;		/* ptr to 1st char in c_data array*/
    register int	on;		/* #chars in c_data array	*/
    register char	*acp = cp;	/* ptr to where char strg will be put */
    register ipri;			/* caller's interrupt level	*/

    ipri = CLIST_LOCK();		/* Acquire global CLIST lock */
    while (n) {				/* more chars to be written */
	if ((bp = p->c_cl) == NULL || (bp->c_last == CLSIZE)) {
		/*
		 * An "in-line" version of getcf() follows:
		 */
	    if (ipri == INTBASE) {	/* Get a free cblock */
		if ((bp = get_unpinned(ipri)) == NULL)
			/*
			 * On failure, get_unpinned() did not reacquire
			 * CLIST_LOCK
			 */
		    return cp - acp;	/* actual number of chars written */
	    } else {
		if ((bp = get_pinned()) == NULL) {
		    CLIST_UNLOCK(ipri);	/* Release global CLIST lock */
		    return cp - acp;	/* actual number of chars written */
		}
	    }

	    bp->c_next = (struct cblock *)NULL;	/* Add new cblock to end */
	    bp->c_first = bp->c_last = 0;	/*	of queue */
	    if (p->c_cl == NULL)
		p->c_cf = bp;
	    else
		p->c_cl->c_next = bp;
	    p->c_cl = bp;
	    op = bp->c_data;
	    on = CLSIZE;
	} else { /*  There is room in the cblock for data to be written.  */
	    op = &bp->c_data[bp->c_last];	/* 1st free position */
						/* in array          */
	    on = CLSIZE - bp->c_last;		/* # free 	     */
						/* positions         */
	}
	if (n >= on) {
	    /* Have more characters than will fit in this cblock. */
	    bcopy(cp, op, on);	/* copy chars from cp to op   */
	    cp += on;		/* point past last char       */
				    /* written                    */
	    bp->c_last += on;	/* increment position of last */
				    /* char in array              */
	    n -= on;		/* decrement #chars still to  */
				    /* be written                 */
	    p->c_cc += on;	/* increment number of chars in */
				    /* clist */
	} else {
	    /*  Need only characters that exist in this cblock.  */
	    bcopy(cp, op, n);	/* copy chars from cp to op*/
	    cp += n;		/* point past last char    */
				    /* written		   */
	    bp->c_last += n;	/* increment position of   */
				    /* last char in array */
	    p->c_cc += n;	/* increment number of chars in */
				    /* clist */
	    break;
	}
    }
    CLIST_UNLOCK(ipri);		/* Release global CLIST lock */
    return cp - acp;		/* actual number of chars written     */
}

/*
 * NAME:  waitcfree
 *
 * FUNCTION:  Wait until there is/are a/some cblock(s) available.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can only be called from a process.
 *
 *	This routine is serialized with CIO_MUTEX_LOCK.
 *
 *	This routine may only page faults when 1) the stack is not in
 *	memory, 2) a call to pin is made for a cblock taken from the
 *	unpinned free list, or 3) when the number of available cblocks
 *	must be increased by a call to get_unpinned().
 *
 * NOTES:  This routine has been redesigned since AIX 3.x.
 *
 *	If this routine is invoked at IT level of INTBASE, then it will
 *	wait until an UNPINNED cblock is available.
 *
 *	If this routine is invoked at an IT level more preferred than
 *	INTBASE, then it will wait until as many as DELTA pieces of
 *	PINNED cblock is available.
 *	While waiting, it tries to move a cblock from the unpinned free
 *	list(s) to the pinned one.
 *
 * RETURN VALUE DESCRIPTION:  This routine returns an integer:
 *                            EVENT_SUCC if there is a cblock on the
 *			      one of the free lists, EVENT_SIG if the
 *			      sleep was terminated by a signal.
 *
 * EXTERNAL PROCEDURES CALLED: locking and sleep services
 */
int waitcfree()
{
    register int old_priority;
    register struct cblock *tmp;

    /*  Make sure this is being called from a process.  */
    assert(CSA->prev == NULL);

    old_priority = CIO_MUTEX_LOCK();		/* Acquire CIO mutex lock */

    if (old_priority == INTBASE)
	/*
	 * Wait while the UNPINNED free list(s) are empty
	 */
	while (c_maint.cfreelist.c_next == NULL && c_maint.endcfree == NULL)
		/*
		 * CIO_MUTEX_LOCK will be released
		 * during CIO_RELEASE_SLEEP()
		 */
	    if (CIO_RELEASE_SLEEP(&c_maint.waiters) == THREAD_INTERRUPTED) {
		CIO_MUTEX_UNLOCK(old_priority);	/* Release CIO mutex lock */
		return EVENT_SIG;
	    }
    else
	/*
	 * Wait while the number of PINNED cblocks is low on the free list
	 */
	while (c_maint.pincfreelist.c_size < DELTA) {
	    CIO_MUTEX_UNLOCK(old_priority);	/* Release CIO mutex lock */
		/*
		 * Try to move a cblock from the unpinned free
		 * lists to the pinned one
		 */
	    if ((tmp = get_unpinned(NOT_LOCKED)) != NULL) { 
		put_pinned_front(tmp);
		/*
		 * No "WAKEUP(&c_maint.waiters);" in order
		 * to give a big chance to ourselves
		 */
		(void) CIO_MUTEX_LOCK();	/* Acquire CIO mutex lock */
		continue;
	    }
		/*
		 * On failure, get_unpinned() did not reacquire
		 * CIO_MUTEX_LOCK
		 */
	    (void) CIO_MUTEX_LOCK();		/* Acquire CIO mutex lock */
	    if (c_maint.pincfreelist.c_size >= DELTA)
		break;
		/*
		 * CIO_MUTEX_LOCK will be released
		 * during CIO_RELEASE_SLEEP()
		 */
	    if (CIO_RELEASE_SLEEP(&c_maint.waiters) == THREAD_INTERRUPTED) {
		CIO_MUTEX_UNLOCK(old_priority);	/* Release CIO mutex lock */
		return EVENT_SIG;
	    }
	}

    CIO_MUTEX_UNLOCK(old_priority);		/* Release CIO mutex lock */
    return EVENT_SUCC;
}

/*
 * NAME: pincf
 *
 * FUNCTION: Provides the ability for a driver to specify a modification
 *		to the number of pinned cblocks required (c_maint.pincreq).
 *		This modification is specified as the number of cblocks
 *		to change the number of pinned cblocks by.  Thus a
 *		positive number will cause "delta" cblocks to be pinned
 *		and moved from the unpinned free list to the pinned one.
 *		Similarly, a negative number will cause -"delta" cblocks
 *		to be unpinned and moved from the pinned free list to
 *		the unpinned one.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can be called from a proces.
 *
 *	This routine is serialized with C_MAINT_PINCF_LOCK.
 *
 *	This routine only page faults when the stack is not in memory.
 *
 * DATA STRUCTURES: c_maint (contains all the data about the free lists)
 *
 * RETURN VALUE DESCRIPTION:  Returns the amount by which this service changed
 *				the number of required pinned cblocks
 *				(c_maint.pincreq).
 *
 * EXTERNAL PROCEDURES CALLED: locking routines
 *
 * NOTE: It is not allowed to assign all of the cblocks for the PINNED
 *	 free list. At least as many as LEFT_OVER pieces of cblocks
 *	 should be (potentially) available for the UNPINNED free list.
 */
#define	LEFT_OVER	30

int
pincf(int delta) 		/* How many cblocks the pinned free list*/
{
    register int	rv;		/* return value from pin/unpin	*/
    register int	ipri;		/* caller's interrupt level	*/
    register int	requested;	/* copy of the delta parameter	*/
    register struct	cblock *bp;	/* temp cblock pointer to move	*/
				    /* a cblock between freelists	*/

    assert(CSA->prev == NULL);

    C_MAINT_PINCF_LOCK ();		/* serialize changing pincreq */

    if(delta > 0)  {
	if (c_maint.pincreq + delta > c_maint.cmax - LEFT_OVER)
	    delta = c_maint.cmax - LEFT_OVER - c_maint.pincreq;
	requested = delta;
	c_maint.pincreq += delta;
	while (delta && (bp = get_unpinned(NOT_LOCKED)) != NULL) {
	    putcf(bp);
	    delta--;
	}
	c_maint.pincreq -= delta;
    } else {
	struct cblock *root_bp = NULL;
	assert(c_maint.pincreq >= -delta);
	requested = delta;

	/*
	 *  Modify the count of the number of pinned cblocks required.
	 *  Do this here and then try to unpin up to -"delta" from the
	 *  pinned free list and put them on the unpinned free list.
	 *  If we run out of cblocks on the pinned free list, then
	 *  having modified c_maint.pincreq here will force the desired
	 *  free list balance as cblocks get freed.
	 */
	c_maint.pincreq += delta;

	/*
	 *  While there are cblocks on the pinned free list and
	 *  there are more cblocks the caller wants unpinned,
	 */
	while (delta && (bp = get_pinned()) != NULL) {
	    bp->c_next = root_bp;
	    root_bp = bp;
	    delta++;
	}
	while ((bp = root_bp) != NULL) {
	    root_bp = bp->c_next;
	    putcf(bp);
	}
	/*
	 *  Tell the caller that the pinned free list has been
	 *  reduced by the requested number.  This needs to be done
	 *  because of the modification to pincreq.
	 */
	delta = 0;
    }

    C_MAINT_PINCF_UNLOCK ();

    return(requested - delta);
}

/*
 * NAME: put_pinned_front
 *
 * FUNCTION: Put a cblock on the front of the pinned free list.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can be called from a process or an interrupt handler.
 *
 *	This routine expects the caller to have serialized with
 *	  CIO_MUTEX_UNLOCK.
 *
 *	This routine does not page fault.
 *
 * DATA STRUCTURES:  c_maint (contains all the data about the free lists)
 *
 * RETURN VALUE DESCRIPTION: put_pinned_front does not have a returned value.
 *
 * EXTERNAL PROCEDURES CALLED: 
 */
static void 
put_pinned_front(struct cblock *bp)
{
    assert(bp->c_flags == c_busy);
    bp->c_flags = c_pinned;
    c_maint.pincfreelist.c_size++;

    /*
     *  If there are no cblocks on the pinned free list, then the pointer
     *  to the last cblock on that list needs to be initialized.
     */
    if (c_maint.pincfreelist.c_next == NULL)
	c_maint.endpincfree = bp;

    /*  Adjust pointers to add cblock to front of pinned free list.  */
    bp->c_next = c_maint.pincfreelist.c_next;
    c_maint.pincfreelist.c_next = bp;
}

/*
 * NAME: put_pinned_back
 *
 * FUNCTION: Put a cblock on the end of the pinned free list.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can be called from a process or an interrupt handler.
 *
 *	This routine expects the caller to have serialized with
 *	  CIO_MUTEX_UNLOCK.
 *
 *	This routine does not page fault.
 *
 * DATA STRUCTURES:  c_maint (contains all the data about the free lists)
 *
 * RETURN VALUE DESCRIPTION:  put_pinned_back does not have a returned value.
 *
 * EXTERNAL PROCEDURES CALLED:
 */
static void 
put_pinned_back(struct cblock *bp)
{
    assert(bp->c_flags == c_busy);
    bp->c_flags = c_pinned;

    /*  Increment the count of cblocks on the pinned free list.  */
    c_maint.pincfreelist.c_size++;

    /*
     *  If there are no cblocks on the pinned free list, then initialize
     *  the pointer of the front of the pinned free list.  Otherwise make
     *  the cblock currently at the end point to this one.
     */
    if (c_maint.pincfreelist.c_next == NULL)
	c_maint.pincfreelist.c_next = bp;
    else
	c_maint.endpincfree->c_next = bp;

    /*  Make this one the end of the pinned free list.  */
    c_maint.endpincfree = bp;

    /*  End the chain.  */
    bp->c_next = NULL;
}

/*
 * NAME: put_unpinned
 *
 * FUNCTION: Add a cblock to the appropriate unpinned free list.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can be called from a process or an interrupt handler.
 *
 *	This routine expects the caller to have serialized with
 *	  CIO_MUTEX_UNLOCK.
 *
 *      This routine expects the input cblock to be pinned (so we
 *        cannot page fault).
 *
 * DATA STRUCTURES:  c_maint (contains all the data about the free lists)
 *
 * RETURN VALUE DESCRIPTION:  put_unpinned does not have a returned value.
 *
 * EXTERNAL PROCEDURES CALLED: 
 */
static void 
put_unpinned(struct cblock *bp)
{
    register int	rv;		/* unpin() return value		*/

    assert(bp->c_flags == c_busy);
    bp->c_flags = c_unpinned;

    /*
     *  The cblocks below the high water mark are the cblocks currently
     *  in use in the system.  If the cblock being freed is in the
     *  first "half" of this group of cblocks, then the cblock should
     *  be added to the front of the unpinned free list, otherwise it
     *  should be added to the end.
     */
    if ((bp - c_maint.cstart) <
			((c_maint.cblkhiwater - c_maint.cstart) >> 1)) {
	bp->c_next = c_maint.cfreelist.c_next;
	c_maint.cfreelist.c_next = bp;
    } else {
	bp->c_next = c_maint.endcfree;
	c_maint.endcfree = bp;
    }
    c_maint.cfreelist.c_size++;		/* count number of free blocks	*/
    rv = unpin(bp, sizeof(*bp));
    assert(rv == 0);
}

/*
 * NAME: get_pinned
 *
 * FUNCTION: Remove a pinned cblock from the free list and return it.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can be called from a process or an interrupt handler.
 *
 *	This routine only page faults when called under a process and the
 *	  stack is not in memory.
 *
 *	This routine uses CIO_MUTEX_LOCK to serialize.
 *
 * DATA STRUCTURES:  c_maint (contains all the data about the free lists)
 *
 * RETURN VALUE DESCRIPTION:  Returns the cblock at the head of the pinned
 *		free list or NULL
 *
 * EXTERNAL PROCEDURES CALLED: locking routines
 */
static struct cblock *
get_pinned()
{
    register struct	cblock	*bp;	/* cblock to return to caller	*/
    register int ipri;

    ipri = CIO_MUTEX_LOCK ();		/* Acquire module mutex lock */

    if ((bp = c_maint.pincfreelist.c_next) != NULL) {
				/* Remove the cblock from the free list. */
	c_maint.pincfreelist.c_next = bp->c_next;
			/* Decrement the count of free, pinned cblocks.  */
	c_maint.pincfreelist.c_size--;
	assert(bp->c_flags == c_pinned);
	bp->c_flags = c_busy;
    }

    CIO_MUTEX_UNLOCK ( ipri );		/* Release module mutex lock */
    return(bp);
}

/*
 * NAME: get_unpinned
 *
 * FUNCTION: Remove an unpinned cblock, if any, and return it.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can only be called from a process.
 *
 *	This routine uses C_MAINT_LOCK to protect itself from itself.
 *
 *      This routine also uses CIO_MUTEX_UNLOCK to protect from put_unpinned.
 *
 *	This routine may page fault.
 *
 * DATA STRUCTURES:  c_maint (contains all the data about the free lists)
 *
 * RETURN VALUE DESCRIPTION:  if there is a cblock on the unpinned free list,
 *		that cblock is removed from the free list and returned, returns
 *		NULL if no pageable cblock on the free list at this moment.
 *
 * EXTERNAL PROCEDURES CALLED: locking routines
 *
 * NOTE: If has CLIST_LOCK been acqured, then it will be released and
 *	on successful return, reacquired
 */
static struct cblock *
get_unpinned(
	islocked)			/* has CLIST_LOCK been acqured ? */
/*	if islocked == NOT_LOCKED:	CLIST_LOCK is not acquired
 *
 *	otherwise:			CLIST_LOCK has been acquired,
 *					the orig. IT priority was 'islocked'
 */
{
    register int ipri;			/* caller's interrupt priority	*/
    register struct cblock *bp;		/* cblock to return		*/
    register struct cblock *fp;		/* cblock pointing to bp	*/
    register struct cblock **anch;	/* pointer to the anchor pointer*/

    if (islocked != NOT_LOCKED)
	CLIST_UNLOCK(islocked);		/* Release global CLIST lock */
    C_MAINT_LOCK ();			/* Acquire "c_maint" lock */
    
    /* Grab pointer to the block in an atomic fashion */
    ipri = CIO_MUTEX_LOCK ();		/* Acquire module mutex lock */
    if (c_maint.cfreelist.c_next != NULL)
	anch = &c_maint.cfreelist.c_next;
    else
	anch = &c_maint.endcfree;
    bp = *anch;
    CIO_MUTEX_UNLOCK ( ipri );		/* Release module mutex lock */

    if (bp != NULL) {
				/* If we can't pinned it, then we fail */
	if (pin(bp, sizeof(*bp))) {	/* can't pin it */
	    C_MAINT_UNLOCK ();		/* Release "c_maint" lock */
	    return(NULL);
	}

	/* 
	 * now, we have to find the place that points to this block
	 * which may be *anch or it may be a block down the list since
	 * blocks could have been added to the list.  We check *anch
	 * disabled to shutout put_unpinned, the others we check
	 * enabled since we could page fault each time.
	 */
	ipri = CIO_MUTEX_LOCK ();	/* Acquire module mutex lock */
	c_maint.cfreelist.c_size--;	/* this guy is logically gone */
	if (*anch == bp) {		/* guy hasn't moved yet */
	    *anch = bp->c_next;
	    CIO_MUTEX_UNLOCK ( ipri );	/* Release module mutex lock */
	} else {
	    CIO_MUTEX_UNLOCK ( ipri );	/* Release module mutex lock */
			    /* Find the block pointing to this block */
	    fp=*anch;
	    while (fp->c_next != bp) {
		fp = fp->c_next;
		ASSERT(fp != NULL);
	    }
	    fp->c_next = bp->c_next;	/* unlink this block */
	}
	C_MAINT_UNLOCK ();		/* Release "c_maint" lock */

	assert(bp->c_flags == c_unpinned);
	bp->c_flags = c_busy;
	if (islocked != NOT_LOCKED)
		(void) CLIST_LOCK();		/* Reacquire global CLIST lock */
	return(bp);
    }

    /* No more unpinned cblocks, so increase the high water mark */
    if (c_maint.cblkavail < c_maint.cmax &&
			!pin(bp = c_maint.cblkhiwater, sizeof(*bp))) {
	c_maint.cblkhiwater++;		/* Increment the number of cblocks */
	c_maint.cblkavail++;		/*	below the high water mark */
	C_MAINT_UNLOCK ();		/* Release "c_maint" lock */
	bp->c_flags = c_busy;
	if (islocked != NOT_LOCKED)
		(void) CLIST_LOCK();		/* Reacquire global CLIST lock */
	return(bp);
    }

    C_MAINT_UNLOCK ();		/* Release "c_maint" lock */
    return(NULL);
}
