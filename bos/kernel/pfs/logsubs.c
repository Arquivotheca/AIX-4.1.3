static char sccsid[] = "@(#)00	1.61  src/bos/kernel/pfs/logsubs.c, syspfs, bos41J, 9512A_all 3/22/95 07:38:22";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: loginit, logx_init, logmvc, logopen, dologmvc,
 *            logwrite, movedata, nextpage, logactive, logwrite,
 *            logsync, logshutdown, eopm_enqueue, eopm_dequeue,
 *	      logclose, groupcommit
 *
 * ORIGINS: 27
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

#include  "jfs/jfslock.h" 
#include  "jfs/commit.h"
#include  "sys/errno.h"
#include  "sys/syspest.h"
#include  "sys/sysinfo.h"
#include  "vmm/vmsys.h"
#include  "vmm/vmlock.h"
#include  "sys/malloc.h"
#include  "sys/sleep.h"
#include  "jfs/ilogx.h"

int logxlock_count = 0;

static int  movedata(struct inode *, int, int, char *, struct comdata *);
static int  nextpage(struct inode *);
static int  logsync(struct inode *);
static int  ilogx_init(struct inode *);
static int  loginit(struct inode *);
static int  logshutdown(struct inode *);
static int  logactive(struct inode *, dev_t, int *, int);
static void logwrite(struct inode *, int, int, int, int);
static void eopm_enqueue(struct inode *);
static void eopm_dequeue(struct inode *);
static int  dologmvc(struct inode *, struct logrdesc *, caddr_t, 
			struct comdata *);

int logclose(struct inode *, dev_t);
int logopen(dev_t, dev_t, int *);
int groupcommit(struct comdata *);

/*
 * NAME:	logmvc(ip, ld, dataptr, cd)
 *
 * FUNCTION:	Log record consisting of the data followed by the 
 *		descriptor is put into the log. on entry the logptr      
 *		is set to the current page in which to put the record
 *		(the page is at the head of the log and the free space
 *		on the page is non-zero). on exit logptr has the same
 *		property. the value returned is the "address" of the 
 *		log record which is the offset in the log of the byte
 *		just past where the descriptor was written. a value of
 *		-1 indicates an error.
 *		
 *		Sreg addressing: this program uses sreg 13 to address
 *		the log. neither dataptr or ld can be in sreg 13.
 *
 * PARAMETERS:	ip	- pointer to inode for log
 *		ld	- log descriptor
 *		dataptr - data to move in to log
 *		cd	- commit data structure
 *
 * RETURN :	errors from movedata() subroutine
 *
 */

int 
logmvc(struct inode *ip, 	/* inode of log */
       struct logrdesc *ld,	/* pointer to a log rec descriptor */
       caddr_t dataptr,		/* pointer to data (length in descriptor */
       struct comdata *cd)
{
	int sr13save, srvmsave, logaddr, diff, lsidx, rc;
	label_t jbuf;

	/* map log into VM using sreg 13 and lock ip.
	 */ 
	sr13save = chgsr(13,SRVAL(ip->i_seg,0,0));
	srvmsave = chgsr(VMMSR, vmker.vmmsrval);

	LOG_LOCK(ip);

	if (rc = setjmpx(&jbuf))
	{
		LOG_UNLOCK(ip);
		(void)chgsr(13,sr13save);
		(void)chgsr(VMMSR,srvmsave);
		longjmpx(rc);
	}

	/* move the data to the log.
	 */
	logaddr = dologmvc(ip,ld,dataptr,cd);

	/* update log address in scb
	 */
	lsidx = STOI(ip->i_seg);
	scb_loglast(lsidx) = logaddr;

	/* how many bytes in log since last sync ?
	 * if big enough do a log sync.
	 */
	diff = logaddr - ip->i_logsync;
	if (diff < 0) 
		diff += (ip->i_logsize - 2) << L2PSIZE;
	if (diff >= ip->i_nextsync)
		logsync(ip);

	clrjmpx(&jbuf);

	/* restore sregs and release lock on ip.
	 */
	LOG_UNLOCK(ip);
	(void)chgsr(13,sr13save);
	(void)chgsr(VMMSR,srvmsave);

	return logaddr;
}


/*
 * NAME:	dologmvc(ip,ld,dataptr,cd)
 *
 * FUNCTION:	Implements logmvc function.  On entry log is mapped into 
 *		virtual memory using sreg 13 and either the i_loglock or 
 *		the normal inode lock is held in such a way that access 
 *		is serialized. (logmvc uses i_loglock, the inode lock is 
 *		held during logopen or logclose functions).  
 *
 * RETURNS:	Address of log record or -1 if there is an error
 * 
 * SERIALIZATION: log lock is held on entry and exit
 */
static int
dologmvc(struct inode *ip, 	/* log inode pointer			 */
	 struct logrdesc *ld,	/* pointer to a log rec descriptor 	 */
	 caddr_t dataptr,	/* pointer to data (length in descriptor */
	 struct comdata *cd)	/* commit control data			 */
{
	struct logrdesc dum;
	int p, q, space, rlen, logaddr;
	int dumlen, diff;

	/* p is offset at which to begin writing
	 */
	p  = ip->i_logend;		
	space = PAGESIZE - 8 - p;
	rlen = (ld->length + 3) & 0xfffffffc;
	rlen += sizeof(struct logrdesc);

	/* determine if a dummy record is required to make this
	 * record fit on at most two pages or to have at least one
	 * record end on every page.
	 */
	dumlen = 0;
	if (rlen > space)
	{
		/* does it fit on the next page?
		 */
		if (rlen > space + PAGESIZE - 16)
		{
			/* dummy ends on next page
			 */
			dumlen = MAX (32, space + 4);
		}
		else	/* no record ending on this page? */
			if (p == 8 && rlen > PAGESIZE - 16)
				dumlen = 32;
	}

	/* dumlen if > 0 includes the 32 byte descriptor
	 */
	if (dumlen > 0)
	{
		if (dumlen > 32)
		{
			logaddr = movedata(ip, 0, dumlen - 32, dataptr, cd);
			if (logaddr < 0) 
				return(logaddr);
		}

		/* write the descriptor
		 */
		bzero(&dum, sizeof(struct logrdesc));
		dum.type = DUM;
		dum.length = dumlen - 32;
		logaddr = movedata(ip ,1 ,32, (caddr_t)&dum, cd);
		if (logaddr < 0)
			return(logaddr);
	}

	/* move data if there is any
	 */
	if (rlen > 32 )
	{
                logaddr  = movedata(ip, 0, rlen - 32, dataptr, cd);
		if (logaddr < 0)
			return(logaddr);
	}

	/* move record descriptor
	 */
        logaddr = movedata(ip, 1, 32, (caddr_t)ld, cd);

	/* return address of the descriptor.
	 */
	return(logaddr);
}


/*
 * NAME:        movedata (ip, type, datalen, dataptr, cd)
 *
 * FUNCTION:	Moves the data specified to the log. returns offset
 *		in the log where writing stopped (i.e. just past the 
 *		last byte written. datalen is assumed to be a integral  
 *		number of words (4-bytes). the fields h.eor and t.eor
 *		are not set until the descriptor has been written. on
 *		entry and exit i_logptr points to the current log page
 *		which is not full and i_logend is the offset to free
 *              space on the page.  h.xor and t.xor are computed and filled
 *              in after the record descriptor.  It can be recomputed by
 *              logredo to check if any of the sectors were in a split
 *              write state when the power went off.
 *
 * PARAMETERS:	ip	- inode for log
 *		type	- type of data. 0 for data 1 for descriptor
 *		datalen	- length of data
 *		dataptr	- address of data to move
 *		cd	- commit control data structure
 *
 * RETURN :	address of last byte moved + 1
 *			
 * SERIALIZATION: Log lock is held on entry and exit
 *		  Log extension is locked and unlocked during access
 *		  	and update of extension fields.
 */
static 
movedata(struct inode *ip,
         int type,
         int datalen,
         char *dataptr,
	 struct comdata *cd)
{
	int    space, nbytes, rc, target, freeptr, logaddr, i, xor;
	struct logpage *ptr;
	struct logrdesc *ld;
	int    *transid, *xorptr;

        /* save log record descriptor start address
         * in case it (dataptr) is updated
         */
        if (type == 1)
                ld = (struct logrdesc *)dataptr;

	/* Compute where next record should begin and retrieve redundancy
	 * check for all data in the page up to this point.
	 */
        xor  	= ip->i_logxor;
        freeptr = ip->i_logend;

	while (datalen > 0)
	{
		space = PAGESIZE - 8 - freeptr;	
		nbytes = MIN (space, datalen);
		target = SR13ADDR + (ip->i_logptr * PAGESIZE) + freeptr;

		bcopy(dataptr,target,nbytes);

		/* Compute a simple redundancy check for the page which
		 * consists of xor'ing the log data.
		 */
		xorptr = (int *)dataptr;
		for (i = 0;  i < (nbytes / sizeof(int)); i++, xorptr++)
			xor ^= *xorptr;

		datalen -= nbytes;	
		freeptr += nbytes;

		/* update offset and redundancy check with current usage 
		 * of the log page 
	 	 */
		ip->i_logend = freeptr;
		ip->i_logxor = xor;

		/* If we have just moved in descriptor (type = 1) a complete 
		 * log record has been written.  If the log record is COMMIT 
		 * record, insert the transaction at tail of commit queue.
		 */
                if (type == 1 && datalen == 0) 
		{
                        struct tblock *tblk;
			
                        /* update last log record eor */
                        ip->i_llogeor = ip->i_logend;
			ip->i_llogxor = xor;

                        /* enqueue tblk for non-trivial COMMIT.  Trivial 
			 * COMMITs are committed asynchronously and are
			 * ignored here.
                         */
                        if (ld->type == COMMIT &&
                            (cd->number != 3 || cd->iptr[0]->i_flag & IFSYNC)) 
			{
				LOGX_LOCK(ip);

                                tblk = &lanch.tblk[cd->tid];
                                tblk->flag   = GC_QUEUE;
                                tblk->cpn    = ip->i_logptr;
                                tblk->ceor   = ip->i_logend;
				tblk->cxor   = xor;
                                tblk->gcwait = EVENT_NULL;
                                tblk->cqnext = NULL;

                                /* enqueue transaction to commit queue
                                 */

                                if (ip->i_logcq.head == NULL) 
				{
                                        ip->i_logcq.head =
                                        ip->i_logcq.tail = tblk;
                                        ip->i_logcsn = 1;
                                } 
				else 
				{
                                        ip->i_logcq.tail->cqnext = tblk;
                                        ip->i_logcq.tail = tblk;
                                        ip->i_logcsn++;
                                }
                                tblk->csn = ip->i_logcsn;

				LOGX_UNLOCK(ip);
                        }
		}

		/* logaddr = just past last byte written
		 * if we haven't filled page we are done.
		 */
		logaddr = ip->i_logptr * PAGESIZE + freeptr;
		if (freeptr < PAGESIZE - 8)
			return logaddr;

		/* move on to next page. initialize freeptr.
		 */
		nextpage(ip);
		xor = 0;
		freeptr = 8;			
		dataptr += nbytes;
	}

	return (logaddr);
}


/*
 * NAME:	nextpage (ip)
 *
 * FUNCTION:	Moves on to next page. 
 *
 * PARAMETERS:	ip	- inode for log
 *
 * RETURN :	Zero
 *			
 * SERIALIZATION: Log lock is held on entry and exit
 * 
 */
static
nextpage(struct inode *ip)
{
	int    logseqpno, currentpno, nextpno, ppongpno;
	struct logpage  *ptr;
	struct tblock   *eopm,   /* End of Page Marker for page just filled */
			*xeopm;	 /* EOPM for previous page	*/

	/* get current log sequence page number and page number . 
	 */
	ptr = (struct logpage *)(SR13ADDR + (ip->i_logptr * PAGESIZE));
	logseqpno = ptr->h.page;
	currentpno = ip->i_logptr;

	LOGX_LOCK(ip);

	/* If no outstanding transactions in commit queue move in header,
	 * trailer data and move on to next page.
	 */
        if (ip->i_logcq.head == NULL) 
	{
		logwrite(ip, currentpno, ip->i_llogeor, ip->i_llogxor, 0);
	}
	else 
	{
                /* Locate end of page marker for the previous page. lsidx 
		 * field is overloaded as previous field
                 */
                eopm  = &ip->i_logeopmq[ip->i_logeopm];
                xeopm = (struct tblock *)&ip->i_logeopmq[eopm->lsidx];

                if (xeopm->flag & GC_QUEUE) 
		{
                        /* Since the previous page EOPM is still on the queue
			 * then it hasn't completed pageout.  Either the EOPM
			 * tblock will eventually be GC_COMMITTED with a group
			 * or be a group leader.  We will have to wait on the
			 * previous page to complete pageout.
                         */
                        xeopm->flag |= GC_WAIT;

			/* If the last transaction id is greater than zero,
			 * then a commit record appears in the current page.
			 * EOPM markers have negative tid.  Enqueue an EOPM 
			 * marker for the current page and wait on the 
			 * previous EOPM tblock.  The group commit leader 
			 * will handle the pageout for both pages. 
			 */
                        if (ip->i_logcq.tail->tid > 0) 
			{
                                eopm_enqueue(ip);

				e_sleep_thread(&xeopm->gcwait, 
					&ip->i_logxlock, LOCK_SIMPLE);
                        } 
			else 
			{
                                /* No COMMIT records in current page.  We
				 * first wait on the pageout completion of 
				 * the previous page.  Since we don't have 
				 * a commit record in the current page (and 
				 * therefore don't have a commit group leader) 
				 * move in the eor, xor and asynchronously
			 	 * write the page.
                                 */
				e_sleep_thread(&xeopm->gcwait, 
					&ip->i_logxlock, LOCK_SIMPLE);

                                /* commit leader paged out the first page:
                                 * pageout the current page.
                                 */
				logwrite(ip, currentpno, ip->i_llogeor,
					ip->i_llogxor, 0);
			}
                } 
		else 
		{
			/* The first page is filled up: enqueue EOPM to 
			 * commit queue.  Group commit leader will finalize 
			 * the page.
                         */
                        eopm_enqueue(ip);

                }
        }

	/* move i_logptr to next page. if log wraps the first
	 * data page of log is 2 (0 never used, 1 is superblock).
	 * set i_logend to empty page.
	 */
	ip->i_logend = 8;
	ip->i_logxor = 0;
	ip->i_logptr = (currentpno == ip->i_logsize - 1) ? 2 : currentpno + 1;

        ppongpno = ip->i_logptr;
        ip->i_logppong = (ppongpno == ip->i_logsize - 1) ? 2 : ppongpno + 1;
 
	LOGX_UNLOCK(ip);

	nextpno = ip->i_logptr;

	/* make next log page
	 */
	vcs_makelogp(ip->i_seg, nextpno, logseqpno+1);
	return 0;
}

/*
 * NAME:	eopm_enqueue()
 *
 * FUNCTION:	Enqueue end-of-page marker (EOPM) transaction
 * 
 * RETURNS:	void
 *
 * SERIALIZATION: Log extension lock held on entry and exit
 */
static void
eopm_enqueue(struct inode *ip)
{
	struct tblock	*eopm;

	/* get current EOPM */
	eopm = &ip->i_logeopmq[ip->i_logeopm];

	/* initialize EOPM
	 */
	eopm->flag   = GC_QUEUE;
	eopm->cpn    = ip->i_logptr;
	eopm->ceor   = ip->i_llogeor;
	eopm->cxor   = ip->i_llogxor;
	eopm->gcwait = EVENT_NULL;
	eopm->cqnext = NULL;

	/* enqueue EOPM at commit_queue tail
	 */
	if (ip->i_logcq.head == NULL)  
	{
		ip->i_logcq.head = 
		ip->i_logcq.tail = eopm;
	} 
	else 
	{
		ip->i_logcq.tail->cqnext = eopm;
		ip->i_logcq.tail = eopm;
	}

	ip->i_logeopmc++;

	/* advance pointer */
	ip->i_logeopm = eopm->next;
}


/*
 * NAME:	eopm_dequeue()
 *
 * FUNCTION: 	Remove End Of Page Marker from commit queue 
 *
 * NOTES:	caller holds i_loglock.
 *
 * RETURNS:	void
 * 
 * SERIALIZATION: Log extension lock held on entry and exit
 */
static void
eopm_dequeue(struct inode *ip)
{
	struct tblock	*eopm;

	/* dequeue EOPM at commit_queue head */
	eopm = ip->i_logcq.head;
	ip->i_logcq.head = eopm->cqnext;

	if (ip->i_logcq.head == NULL)
		ip->i_logcq.tail = NULL;

	ip->i_logeopmc--;

	/* if log writer is waiting on the page (EOPM transaction)
	 * to be paged out, wake it up to move on to the next page.
	 * (log writers filled up to the 2nd page while the last COMMIT
	 * in the 1st page is being paged out as partial page)
	 */
	if (eopm->flag & GC_WAIT) 
	{
		eopm->flag &= ~GC_WAIT;
		e_wakeupx(&eopm->gcwait, E_WKX_NO_PREEMPT);
	}

	/* free EOPM dummy transaction */
	eopm->flag = 0;
}
	

/*
 * NAME:	groupcommit(cd)
 *
 * FUNCTION:	Called by finicom() after writing COMMIT record to log page
 *
 * NOTES:	vmmdseg must be mapped on entry
 *
 * RETURNS:	
 */
int
groupcommit(struct comdata *cd)
{
	struct  inode	*ip;	/* log inode 				*/
	struct 	tblock	*tblk,  /* current transaction block		*/
			*gltblk,/* group leader transaction block   	*/
			*xtblk;
	int	rc = 0,		/* return code				*/
		glcpn,		/* group leader commit page number 	*/
		gcpn,		/* group commit page number 		*/
		gceor,		/* group commit eor 			*/
		gcxor,		/* group commit xor checksum 		*/
		gctc;		/* group commit transaction count 	*/

	/* locate the transaction block of the transaction */
	tblk = &lanch.tblk[cd->tid]; 

	/* locate the log inode of the transaction */
	ip = cd->ilog;

	/* i_logxlock protects log inode extension for group commit, 
	 * commit queue, and transaction blocks on commit queue
	 */
	LOGX_LOCK(ip);

	/* test whether group committed already */
	if (tblk->flag & GC_COMMITTED) 
	{
		if (tblk->flag & GC_ERROR)
			rc = tblk->csn;

		LOGX_UNLOCK(ip);
		return rc;
	}

	/* test for group commit pageout in progress 
	 */
	if (ip->i_logflag & LOGX_GCPAGEOUT) 
	{
		/* Group commit pageout is in progress so record our
		 * ready status and increment the transactions on the 
		 * ready queue.
		 */
		tblk->flag |= GC_READY;
		ip->i_logcrtc++;

		/* (SLEEP 1)
		 *
		 * Record the latest transaction that is ready.  This
		 * transaction may potentially become the next group leader. 
		 * Transactions which enter this point are in the queue but
		 * didn't arrive before the pageout, so they do not have their
		 * GC_COMMIT flag on (hence when the current pageout 
		 * completes the page will need to be rewritten to ensure 
		 * that the transaction is on disk).  The last commit ready
		 * transaction will be woken up below at (WAKEUP 1)
		 */
		if (tblk->csn > ip->i_loglcrt->csn)
			ip->i_loglcrt = tblk;

		e_sleep_thread(&tblk->gcwait, &ip->i_logxlock, LOCK_SIMPLE);

		/* We went to sleep waiting on the pageout to complete.  If
		 * a group leader committed us and woke us up then we are
		 * through.
		 */
		if (tblk->flag & GC_COMMITTED) 
		{
			/* group committed and removed from commit_queue 
			 */
			if (tblk->flag & GC_ERROR)
				rc = tblk->csn;

			LOGX_UNLOCK(ip);
			return rc;
		}

		/* If my flag wasn't GC_COMMITTED then I've been selected 
		 * as new/next group leader.  Clear the GC_READY flag
		 * since upon completion of the pageout this flag is used
		 * to signal that a wakeup is needed.  I won't need to 
		 * wake myself up.
		 */
		tblk->flag &= ~GC_READY;
		ip->i_logcrtc--;
	} 	
	else 
	{
		/* No pageout in progress so we are going to have to 
		 * become the group leader and initiate one.
		 */
		ip->i_logflag |= LOGX_GCPAGEOUT;
		ip->i_loglcrt = tblk;
	}

	/* If the transaction has reached this point in the code, 
	 * then it is the group leader.
	 */
	tblk->flag |= GC_LEADER;
	gltblk = tblk;
	glcpn  = gltblk->cpn;

next_page:

	/* Scan commit queue and make a commit group of all transactions 
	 * with COMMIT records on the same log page.  Transaction blocks
	 * in comit queue are in the order of COMMIT records on the log 
	 * page(s).  The group leader may not be the first in the commit
	 * queue (and may not be on the first page to pageout).  
	 */
	tblk = xtblk = ip->i_logcq.head;
	gcpn = tblk->cpn;
	gctc = 0; 

	/* Loop thru the transactions on the commit queue while there
	 * are still transactions and the transaction appears on the same
	 * page as the current commit.  Mark all the transaction on this page
	 * and record the largest eor and xor.
	 */
	while (tblk != NULL && tblk->cpn == gcpn) 
	{
		xtblk = tblk;
		tblk->flag |= GC_COMMIT;
		gceor = tblk->ceor;
		gcxor = tblk->cxor;

		/* Count transaction blocks for this page.  If tid is less
		 * than zero then this is the End of Page Marker for the page.
		 */
		if (tblk->tid > 0)
			gctc++;
		else 
			break;

		tblk = tblk->cqnext;
	}
	tblk = xtblk;	/* last tblk of the group */

	/* If EOPM is the only/last transaction for current page, pageout 
	 * the page asynchronously.  EOPM has been enqueued after last group 
	 * commit pageout;  Group leader COMMIT resides in the next page.
	 */
	if (gctc == 0) 
	{
		assert(tblk->tid < 0);
		logwrite(ip, tblk->cpn, tblk->ceor, tblk->cxor, 0);
		eopm_dequeue(ip);
		goto next_page;
	}

	/* Current page has outstanding COMMIT records.  This will require
	 * synchronous pageout to complete commit. 
	 */
	if (tblk->tid < 0)
		logwrite(ip, gcpn, gceor, gcxor, 0);
	else
		logwrite(ip, gcpn, gceor, gcxor, 1);
	
	LOGX_UNLOCK(ip);

        rc = vms_iowait(ip->i_seg);

#ifdef DEBUG
	if (rc) printf("gc(%d):%d:%d JFS LOG on fire!\n", gltblk->tid, gcpn, 
			gceor);
#endif

	LOGX_LOCK(ip);
	
	/* Group leader removes transactions from commit queue who were 
	 * group committed with the current commit page and wake them up
	 */
	while (tblk = ip->i_logcq.head) 
	{
		/* If the transaction was marked GC_COMMIT then we saw
		 * it above before we ever started the pageout.  Therefore
		 * we are sure that it made it to disk so inform the 
		 * transaction it is committed.
		 */
		if (tblk->flag & GC_COMMIT) 
		{
			tblk->flag |= GC_COMMITTED;

			/* If it was a real transaction then save the 
			 * pageout error code remove it from the queue
			 * and wake it up.
			 */
			if (tblk->tid > 0) 
			{
				if (rc) 
				{
					tblk->flag |= GC_ERROR;
					tblk->csn = rc;
				}

				ip->i_logcq.head = tblk->cqnext;
				if (ip->i_logcq.head == NULL)
					ip->i_logcq.tail = NULL;

				if (tblk->flag & GC_READY)  
				{
					ip->i_logcrtc--;
					e_wakeupx(&tblk->gcwait, 
						E_WKX_NO_PREEMPT);
				}
			} 
			else 
			{
				/* An EOPM transaction was selected as the 
				 * group leader.  In this instance the page
				 * has been written to its home location and
				 * was released, so dequeue the marker.
				 */
				eopm_dequeue(ip);
			}
			continue;
		}
		break;
	}

	/* Continue group commit up to and including group 
	 * leader commit page.
	 */
	if (gcpn != glcpn)
		goto next_page;

	if (tblk == NULL) 
	{
		/* If there are no outstanding transactions in commit queue 
		 * then we are complete.  
		 */
		ip->i_logflag &= ~LOGX_GCPAGEOUT;
	}
	else if (ip->i_logcrtc > 0) 
	{
		/* (WAKEUP 1)
		 *
		 * Other transactions have entered group commit after 
		 * the last pageout was started.  These transactions
		 * are waiting above at (SLEEP 1).  Select the latest 
		 * transaction as the group leader and wake him up.
		 *
  		 * Leave i_logflag in LOGX_GCPAGEOUT state guaranteeing the 
		 * transaction awoken will become the new leader.  This will
		 * avoid a wasted wakeup.
		 */
		tblk = ip->i_loglcrt;

		e_wakeupx(&tblk->gcwait, E_WKX_NO_PREEMPT);
	} 
	else if ((tblk->tid < 0) && (glcpn == tblk->cpn)) 
	{
		/* If first in commit queue is EOPM dummy transaction,
	 	 * initiate asynchronous page out to finalize the page 
		 * and dequeue the marker.
	 	 */
		logwrite(ip, tblk->cpn, tblk->ceor, tblk->cxor, 0);
		eopm_dequeue(ip);

		ip->i_logflag &= ~LOGX_GCPAGEOUT;

	} 
	else 
	{
		/* If there are outstanding GC_QUEUE commit transactions
	 	 * (which have not entered groupcommit()), the first GC_QUEUE 
	 	 * transaction entering groupcommit will elect itself 
	 	 * as new group leader.
	 	 */
		ip->i_logflag &= ~LOGX_GCPAGEOUT;
	}

	LOGX_UNLOCK(ip);

	return rc;
}



/*
 * NAME:	logwrite(ip, pno, eor, xor, ppong)
 *
 * FUNCTION:	Write log page 
 *		
 * PARAMETERS:	ip	- inode for log
 *		pno	- log page number
 *
 * RETURNS:	void
 *
 * SERIALIZATION: Logx lock hend on entry and exit.
 *			
 */
static void
logwrite(struct inode *ip, 
	 int pno,
	 int eor, 
	 int xor,
	 int ppong)
{
	int 	ppongpno, sr13save;
        struct	logpage *logpage;

	/* Map log.  Setup page number and eor in header and trailer
	 */
	sr13save = chgsr(13, SRVAL(ip->i_seg,0,0));

        logpage = (struct logpage *)(SR13ADDR + (pno << L2PSIZE));
        logpage->h.eor = logpage->t.eor = eor;
	logpage->t.xor = xor & 0xFFFF;
	logpage->h.xor = xor >> 16;

        /* restore address space */
	(void)chgsr(13,sr13save);

	if (ppong)
	{
		vcs_writelogp(ip->i_seg, pno, ip->i_logppong, 0);

		ppongpno = ip->i_logppong;

		/* Compute the next ping pong page.  It will either be 
		 * i_logptr plus one or plus two, depending on what the 
		 * last ping pong page was.
	 	 */
		if (ip->i_logptr > ppongpno)
			ppongpno += ip->i_logsize - 2;

		if (ppongpno - ip->i_logptr > 1)
			ppongpno--;
		else
			ppongpno++;

		if (ppongpno > ip->i_logsize - 1)
			ppongpno -= ip->i_logsize - 2;

		ip->i_logppong = ppongpno;
	}
	else	/* Write log page and release */
		vcs_writelogp(ip->i_seg, pno, pno, 1);

}

/*
 * NAME:	logsync(ip)
 *
 * FUNCTION:	write log syncpt record if new sync address
 *		is available (normally the case if sync()
 *		is executed by back-ground process). if not,
 *		explicitly run ilogsync() to initiate 
 *		getting of new sync address. calculate new
 *		value of i_nextsync which determines when
 *		this code is called again. 
 *
 *		this is called only from logmvc. 
 *		on entry VMM dataseg is mapped into VM
 *
 * PARAMETERS:	ip	- pointer to logs inode. i_loglock held
 *			  on entry.
 *
 * RETURN :	0
 *			
 */
static
logsync(struct inode *ip)
{
	int	lsidx,nb,diff,logaddr, logbytes;
	struct logrdesc lr;

	/* if scb_logsync is same as last syncpt address 
	 * invoke ilogsync(). Pass the log inode pointer
	 * so that we only sync those inodes associated
	 * with this particular log.
	 */
	lsidx = STOI(ip->i_seg);
	if (scb_logsync(lsidx) == ip->i_logsync)
		ilogsync(ip);

	/* if scb_logsync is different from last sync address
	 * write a new syncpt record with addr = scb_logsync.
	 */
	if (scb_logsync(lsidx) != ip->i_logsync)
	{
		lr.backchain = 0;
		lr.transid = 0;
		lr.type = SYNCPT;
		lr.length = 0;
		ip->i_logsync = lr.log.sync = scb_logsync(lsidx);
		dologmvc(ip, &lr, (char *)NULL, (struct comdata *)NULL);
	}

	/* diff is the number of bytes put in log from last
	 * sync point address. nextsync is number of bytes 
	 * before next call to logsync.
	 */
	logbytes = (ip->i_logsize - 2) << L2PSIZE;
	logaddr = ip->i_logptr * PAGESIZE + ip->i_logend;

	if ((diff = logaddr - ip->i_logsync) < 0)
		diff += logbytes;

	nb = MIN(ip->i_logsize*PAGESIZE/8,512*1024); 
	ip->i_nextsync = diff + nb;

	/* for logs which are too small (e.g. 32 pages) we don't
	 * care if it wraps.
	 */
	assert(ip->i_nextsync < MAX(32*PAGESIZE, logbytes));

	/* if diff is more than 1/4 of the log size, stop new
	 * transactions from starting until all current transactions
	 * are complete this is done by setting syncwait flag
	 * through vcs_syncwait().
	 */
	if (diff > logbytes/4 && logbytes > 32*PAGESIZE)
		vcs_syncwait();

	return 0;
}

/*
 * NAME:	logopen (logdev,device, serial);
 *
 * FUNCTION:    open the log device specified and make it 
 *		accessable in virtual memory to the log code.
 *
 *		puts device in the active list in the log super
 *		block. (device is dev_t of filesystem).
 *
 *		sets serial to the serial number of the log.
 *
 *		the inode table entry for the log is (logdev,0) 
 *	        where logdev is the dev_t of the log and 0 is a
 *		fictitious inode number (i.e. a value never used
 *		for an inode number in any filesystem and special
 *		cased in iread).
 *		
 * PARAMETERS:	logdev	- dev_t of log device.
 *		device	- dev_t of filesystem
 *		serial  - pointer to returned log serial number. 
 *
 * RETURN :	errors from subroutines
 *			
 */
logopen(dev_t logdev,	/* dev_t of log device */
	dev_t device,	/* dev_t of filesystem */
	int *serial)
{
	struct hinode *hip;
	struct inode *ip;
	struct gnode *gnptr;
	int sid = 0;
	int type, rc, p;

	/* find the hash list where the inode resides
	 */
	IHASH(logdev, 0, hip);

	/* Get in-memory inode for log device. 
	 * the log device is not a file system and i_number zero 
	 * for dev_t of the log device is used for the in-memory inode for log.
	 */
	ICACHE_LOCK();
	rc = _iget(logdev, 0, hip, &ip, 1, NULL);
	ICACHE_UNLOCK();
	sysinfo.iget++;
	cpuinfo[CPUID].iget++;
	if (rc)
		return rc;

	/* use read/write lock to serialize open/close.
	 * if its already open put file system device in active list.
	 * acquire one i_count for every mount.
	 */
	IWRITE_LOCK(ip);
	if (ip->i_seg)
	{	
		if (rc = logactive(ip,device,serial,1))
			goto closeout1;

		IWRITE_UNLOCK(ip);
		return 0;		/* Already open */
	}

	/* put log device in pdt table. 
	 * allocate one buf struct for it.
	 * (only one to guarantee i/o occurs in serial order).
	 */
	if (rc = vm_mount(D_LOGDEV, logdev, 1))
		goto closeout1;

	/* XXX. FMOUNT says open only once and no other writers to logdev
	 * These semantics are not really strong enough.  We need to
	 * be able to stop opens from the file system after the log
	 * device is open by mount.  The O_NSHARE sematics are actually
	 * correct  but that's not implemented yet.
	 */

	gnptr = NULL;

	if (rc = rdevopen(logdev, FWRITE|FMOUNT, 0, NULL, &gnptr))
		goto closeout;

	/* create a vm segment for log. size parm is zero.
	 * actual of size of log is determined by reading 
	 * its superblock. 
	 */
	type = V_PERSISTENT | V_LOGSEG | V_SYSTEM;
	if (rc = vms_create(&sid, type, logdev, 0, 0, 0))
		goto closeout;	

	/* initialize log. 
	 */
	ip->i_logdgp = gnptr;
	ip->i_seg = sid;

	if (rc = loginit(ip))
		goto closeout;

	/* put file system device in active list.
	 */
	if (rc = logactive(ip,device,serial,1))
		goto closeout;

	IWRITE_UNLOCK(ip);	
	return 0;

closeout:
	/* remove log device from pdt table
	 */
	vm_umount(D_LOGDEV,logdev);

	/* Cascading error conditions */
	if (gnptr)
	{
	 	rdevclose(gnptr, FWRITE|FMOUNT, 0); 
		ip->i_logdgp = NULL;
		if (sid)
			isegdel(ip);
	}

closeout1:
	IWRITE_UNLOCK(ip);

	ICACHE_LOCK();
	iput(ip, NULL);
	ICACHE_UNLOCK();

	return rc;
}


/*
 * NAME:	loginit (ip)
 *
 * FUNCTION:	log initialization at first logopen.
 *
 *		On entry log is mapped into VM using sreg 13. logredo
 *		(or logformat)  should have been run previously. the
 *		fields in ip and the scb of the log are initialized.
 *		the redone flag in the superblock is set to zero and 
 *		a syncpt record is put in the log.
 *		
 * PARAMETERS:	ip	- pointer to log's inode. ilocked on entry.
 *
 * RETURN :	0	- if ok
 *		EINVAL	- bad log magic number
 *		EFORMAT	- log not processed by logredo
 *			
 */

#define	LOCK_ID(dev) 	(short)(((dev) >> 8 & 0xff00) | 0xff & (dev))

static
loginit(struct inode *ip)
{
	int rc, sidx, nbytes, ppongpno;
	struct logsuper *logsuper;
	struct logpage	*logpage;
	struct logrdesc lr;
	volatile int sr13save;
	volatile int srvmsave;
	label_t jb;

	/* map log into VM using sreg 13 and vmmdseg at normal
	 * location.
	 */ 
	sr13save = chgsr(13, SRVAL(ip->i_seg,0,0));
	srvmsave = chgsr(VMMSR,vmker.vmmsrval);

	if (rc = setjmpx(&jb))
		goto closeout;

	/* check magic number. get size of log
	 */
	logsuper = (struct logsuper *)(SR13ADDR + PAGESIZE);

	if ((logsuper->magic != LOGMAGIC && logsuper->magic != LOGMAGICV4) ||
	    (ip->i_logsize = logsuper->size) > NUMPAGES)
	{
	 	rc = EINVAL;
		clrjmpx(&jb);
		goto closeout;
	}

        /* Initialize the log lock. Compress ip->i_dev into a short
	 * to generate the lock occurrence number.
	 */
        lock_alloc(&ip->i_loglock, LOCK_ALLOC_PAGED,
		   LOG_LOCK_CLASS, LOCK_ID(ip->i_dev));
        simple_lock_init(&ip->i_loglock);

	/* insist on logredo code having been run first.
	 */
	if (logsuper->redone != LOGREDONE)
	{
		rc = EFORMAT;
		clrjmpx(&jb);
		goto closeout;
	}

	/* record end of log info and checksum in log inode
	 */
	ip->i_logptr = logsuper->logend / PAGESIZE;
	ip->i_logend = logsuper->logend - (PAGESIZE * ip->i_logptr);

	logpage = (struct logpage *)(SR13ADDR + (ip->i_logptr << L2PSIZE));
        ip->i_logxor = logpage->h.xor;
        ip->i_logxor = (ip->i_logxor << 16) | (logpage->t.xor & 0xFFFF);
	
        /* initialize log inode extension area
         */
        if (rc = ilogx_init(ip))
                goto closeout;

        ppongpno = ip->i_logptr;
        ip->i_logppong = (ppongpno == ip->i_logsize - 1) ? 2 : ppongpno + 1;

        ip->i_llogeor = ip->i_logend;
	ip->i_llogxor = ip->i_logxor;

	/* If their is no free space move on to next page.  Or if we are
	 * transitioning from the old log style then move to a new page if
	 * any records appear on the old page.  We do this so the redundancy
	 * check value will begin fresh.  If we didn't and we powerfailed
	 * on this last page, the XOR value wouldn't match.
	 */
	if ((ip->i_logend  >= PAGESIZE - 8) ||
	    ((logsuper->magic == LOGMAGIC) && (ip->i_logend > 8)))
		nextpage(ip);

	/* i_nextsync is equal to number of bytes since sync to move
	 * in logmvc before call to logsync. we set it to a big
	 * value to prevent premature call to logsync, even when 
	 * log has just been formatted.
	 */
	ip->i_nextsync = (1 << 30);
	ip->i_logsync = ip->i_logend * PAGESIZE;

	/* write a SYNCPT record into log. set i_nextsync to the
	 * number of bytes to move into log 
	 */
	lr.backchain = 0;
	lr.transid = 0;
	lr.type = SYNCPT;
	lr.length = 0;
	lr.log.sync = 0;
	ip->i_logsync = dologmvc(ip, &lr, (char *)NULL, (struct comdata *)NULL);
	ip->i_nextsync = MIN(ip->i_logsize*PAGESIZE/8, 512*1024);

	/* initialize scb for sync processing
	 */
	sidx = STOI(ip->i_seg);
	scb_logsize(sidx) = ip->i_logsize*PAGESIZE ;
	scb_logsync(sidx) = ip->i_logsync;
	scb_loglast(sidx) = ip->i_logsync;
	scb_logcur(sidx) = 0;

        /* We are about to be writing new style log ping pong pages to the
         * log so switch the magic number so only a logredo which understands
         * this type will be able to replay it.  Write out superblock and
	 * wait for sync point record to go out.
	 */
	logsuper->magic = LOGMAGICV4;
	logsuper->redone = 0;
	logsuper->serial += 1;
	vm_write(logsuper, PAGESIZE, 0);

        /* wait for completion of synchronous write of SYNCPT record.
         * (log superblock pageout precedes the SYNCPT log page pageout.)
         */
	logwrite(ip, ip->i_logptr, ip->i_logend, ip->i_logxor, 1);
	rc = vms_iowait(ip->i_seg);

	clrjmpx(&jb);

closeout:
	(void)chgsr(13,sr13save);
	(void)chgsr(VMMSR,srvmsave);
	return rc;
}

/*
 * NAME: 	ilogx_init()
 *
 * FUNCTION:	initialize log inode extension area
 *
 * RETURNS:	ENOMEM 	- malloc failed
 *		0	- success
 */
static int
ilogx_init(struct inode *ip)
{
        struct ilogx   *ilogxp;

        if ((ilogxp = (struct ilogx *)malloc(sizeof(struct ilogx))) == NULL)
                return ENOMEM;
        bzero(ilogxp, sizeof(struct ilogx));

	ip->i_logx = ilogxp;
	fetch_and_add(&logxlock_count, 1);

        /* initialize  log lock
         */
	lock_alloc(&ip->i_logxlock, LOCK_ALLOC_PAGED, LOG_LOCK_CLASS, 
		logxlock_count);
	simple_lock_init(&ip->i_logxlock);

        /* end-of-page marker dummy transaction queue: establish circular, 
	 * doubly-linked list of 2 (double-buffer) queue.  EOPM transaction 
	 * blocks need not be pinned as they are never accessed by VMM 
	 * critical sections. NOTE: overload next and lsidx field for next 
	 * and prev.
         */
        ip->i_logeopm = 0;
        ip->i_logeopmq[0].next  = 1;
        ip->i_logeopmq[0].lsidx = 1;
        ip->i_logeopmq[1].next  = 0;
        ip->i_logeopmq[1].lsidx = 0;

        /* initialize as EOPM dummy transaction (tid < 0)
         */
	ip->i_logeopmq[0].tid = -1;
	ip->i_logeopmq[1].tid = -2;

        return 0;
}

/*
 * NAME:	logclose (ip,device)
 *
 * FUNCTION:	remove device from active list of a log.
 *		close the log device if this is last use.
 *
 * PARAMETERS:	ip	- pointer for log device inode
 *		device  - dev_t of file system.
 *
 * RETURN :	errors from subroutines
 *			
 */
int
logclose(struct inode *iplog,
	 dev_t device)
{
	int rc, rc1, serial;

	/* use read/write lock to serialize open/close.
	 */
	IWRITE_LOCK(iplog);

	/* remove file system device from active list
	 */
	rc = logactive(iplog,device,&serial,0);

	/* shut down log if this is last reference.
	 */
	if (iplog->i_count == 1)
	{
	 	rc = ((rc1 = logshutdown(iplog)) && rc == 0) ? rc1 : rc;

		rc = ((rc1 = isegdel(iplog)) && rc == 0) ? rc1 : rc;

		rc = ((rc1 = vm_umount(D_LOGDEV,iplog->i_dev)) && rc == 0) ?
			rc1 : rc;

		rc = ((rc1 = rdevclose(iplog->i_logdgp, FWRITE|FMOUNT, 0)) &&
			rc == 0) ? rc1 : rc; 
	}

	IWRITE_UNLOCK(iplog);

	/* free the in-memory log inode. 
	 * i_mode of the log inode is zero therefore iput()
	 * will just put ip back on free list on last reference release.
	 */
	ICACHE_LOCK();
	iput(iplog, NULL);
	ICACHE_UNLOCK();

	return rc;
}


/*
 * NAME:	logshutdown (ip)
 *
 * FUNCTION:	log shutdown at last logclose.
 *
 *		write log syncpt record.  
 *		update super block to set redone flag to 0.
 *
 * PARAMETERS:	ip	- pointer to logs inode. ilocked on entry.
 *
 * RETURN :	0	- success
 *		errors from vms_iowait()
 *
 * SERIALIZATION: Data movement into the log due to regular meta data
 *		  transactions is now complete.  We have processed all
 *		  active transactions due to iactivity() shutdown and 
 * 		  waited on all log io to stop.  At this point our log
 *		  writes are single threaded.
 */
static
logshutdown(struct inode *ip)
{
	struct logrdesc lr;
	int logaddr, lsidx, rc;
	struct logsuper *ptr;
	volatile int sr13save;
	label_t jb;

	/* map log into VM using sreg 13
	 */ 
	sr13save = chgsr(13, SRVAL(ip->i_seg,0,0));

	if (rc = setjmpx(&jb))
		goto closeout;

	/* write a new SYNCPT record
	 */
	lr.backchain = 0;
	lr.transid = 0;
	lr.type = SYNCPT;
	lr.length = 0;
	lr.log.sync = 0;
	logaddr = dologmvc(ip, &lr, (caddr_t) NULL, (struct comdata *)NULL);

	/* Write last page to its origin location.  The move of the 
	 * last SYNCPT may have filled the page, in which case this
	 * logwrite() will write and release the next page created 
	 * under nextpage().
	 */
	logwrite(ip, ip->i_logptr, ip->i_logend, ip->i_logxor, 0);

	if (rc = vms_iowait(ip->i_seg))
	{
		clrjmpx(&jb);
		goto closeout;
	}

        /* free log inode extension lock and data area 
	 */
	lock_free(&ip->i_logxlock);
        free(ip->i_logx);

	/* update superblock to say it was shutdown.
         * Log does not need to be replayed.  Set magic number to earliest
         * version since this filesystem can be taken to earlier levels of
         * the operating system.
	 */
	ptr = (struct logsuper *) (SR13ADDR + PAGESIZE);
	ptr->magic = LOGMAGIC;
	ptr->redone = 1;
	ptr->logend = logaddr;
	vm_write(ptr,PAGESIZE,0);
	rc = vms_iowait(ip->i_seg);
	clrjmpx(&jb);

closeout:
	/* free the LOG_LOCK()
	 */
	lock_free(&ip->i_loglock);

	(void)chgsr(13,sr13save);
	return rc;
}


/*
 * NAME:	logactive(ip,device,serial,activate)
 *
 * FUNCTION:	puts device into active list of log if activate
 *		is true; removes it from active list if false.
 *		sets serial to the logserial number.
 *
 * PARAMETERS:	ip	- pointer to logs inode. ilocked on entry.
 *		device	- dev_t of filesystem.
 *		serial  - pointer to returned log serial number
 *		activate - insert/remove device from active list.
 *
 * RETURN :	0	- success
 *		errors returned by vms_iowait().
 *			
 */

static int
logactive(struct inode *ip, dev_t device, int *serial, int activate)
{
	struct logsuper *ptr;
	int rc, bit, word;
	volatile int sr13save;
	label_t jb;

	/* map log into VM using sreg 13
	 */ 
	sr13save = chgsr(13, SRVAL(ip->i_seg,0,0));

	if (rc = setjmpx(&jb))
		goto closeout;

	/* remove or add device to active list.
	 */
	ptr = (struct logsuper *) (SR13ADDR + PAGESIZE);
	bit = minor(device);
	word = bit/32;
	bit -= 32*word;
	if (activate)
		ptr->active[word] |= (UZBIT >> bit);
	else
		ptr->active[word] &= (~(UZBIT >> bit));

	/* write log super block out.
	 */
	*serial = ptr->serial;
	vm_write(ptr, PAGESIZE, 0, 0);
	rc = vms_iowait(ip->i_seg);
	clrjmpx(&jb);

closeout:
	(void)chgsr(13,sr13save);
	return rc;
}
