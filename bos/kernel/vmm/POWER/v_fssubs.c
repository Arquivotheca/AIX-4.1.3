static char sccsid[] = "@(#)13	1.81  src/bos/kernel/vmm/POWER/v_fssubs.c, sysvmm, bos41J, 9520B_all 5/19/95 09:09:38";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:	v_sync, v_getlock, v_getblk, v_frtblk, v_frlock, v_rmlock,
 *		v_findlw, v_freelw, v_insertlw, v_deletelws, v_lockseg
 *		v_unlockseg, v_iowait, v_purge, vm_initlock, v_makelogp,
 *		v_mapd, v_defer, v_allocdq, v_freedq, v_upddqlbks,
 *		v_chkdqlock, v_unlockdq, v_powait, v_getlw
 *
 * ORIGINS: 27, 26, 83
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

#include  "sys/types.h"
#include  "jfs/inode.h"
#include  "jfs/log.h"
#include  <sys/proc.h>
#include  "sys/user.h"
#include  "sys/errno.h"
#include  "vmsys.h"
#include <sys/uprintf.h>
#include <sys/unixmsg.h>
#include <sys/syspest.h>
#include <sys/systm.h>
#include <sys/inline.h>
#include <sys/sleep.h>
#include "mplock.h"

/*
 * v_sync(ip, iplog)
 *
 * i/o is initiated to page out modified pages which have
 * their homeok bit set. if the homeok bit is not set the
 * syncpt bit is set so that the page will be written by
 * commit processing.
 *
 * for each log the variable logsync is maintained as
 * an upper bound of the oldest transaction or page.
 * its value is suitable for the log address in a log
 * syncpt record. 
 *
 * the variable logcur is maintained as the smallest 
 * difference in age from the last sync value, logsync.
 * thus logsync + logcur is the oldest log value. logcur
 * is updated by v_sync and transaction end (v_frtblk).
 *
 * the sid value of zero indicates that a complete scan
 * has been executed. when a scan is completed new log
 * sync point addresses are "published". if a NULL pointer
 * is passed for iplog then new sync point addresses are
 * published for every log in the system. otherwise, a new
 * sync point address is published for that particular log.
 *
 * the current transaction ages are included in the computation
 * of logcur so that pages in extension memory are accounted
 * for as well as the updates to the disk and inode maps which
 * may not yet have been applied.
 *
 * this program executes at VMM interrupt level on a
 * fixed stack without back-tracking enabled.
 *
 * Return value - 0
 */

int 
v_sync(ip, iplog)
struct inode *ip;
struct inode *iplog;
{
        int sid,k,next,m,diff,logval,lsidx,sidx;

	/* the sid value of zero signifies a complete
	 * scan in xix_logsync or xix_sync. thus we want
	 * to "publish" a new sync point address for each 
	 * log in the system. However, if an inode log
	 * pointer is passed, we will only publish a new
	 * sync point for that log.
	 */
	if (ip == NULL)
	{
		ASSERT(!(CSA->backt = 0)); 
		LW_MPLOCK();

        	/* update logcur by taking into account the current
         	 * transactions in the system. loop starts at 1
         	 * because tid 0 is never allocated. if we are
		 * updating the logcur for just one log, skip
		 * over all other transactions not associated 
		 * with the particular log.
         	 */
        	for (k = 1; k <= lanch.maxtid; k++)
        	{
                	if(lanch.tblk[k].logage == 0)
                        	continue;
                	lsidx = lanch.tblk[k].lsidx;
			if (iplog && (STOI(iplog->i_seg) != lsidx))
				continue;
                	logdiff(diff,lanch.tblk[k].logage,lsidx);
                	scb_logcur(lsidx) = MIN(diff,scb_logcur(lsidx));
        	}

	        /* calculate new logsync values for each log
        	 * initialize logcur to reflect loglast.
         	 */
        	for(k = 0; k < MAXLOGS; k++)
        	{
                	if((lsidx = pf_logsidx[k]) == 0)
                        	continue;

			/* if we are only doing it for one
			 * particular log then we skip other
			 * logs.
			 */
			if (iplog && (STOI(iplog->i_seg) != lsidx))
				continue;

                	/* compute new logsync value
                 	 */
                	logval = scb_logsync(lsidx) + scb_logcur(lsidx);
                	if (logval > scb_logsize(lsidx))
                        	logval = logval - scb_logsize(lsidx) + 2*PSIZE;
                	scb_logsync(lsidx) = logval;

	               /* initialize logcur to reflect loglast
         	        */
                	logdiff(diff,scb_loglast(lsidx),lsidx);
                	scb_logcur(lsidx) = diff;
        	}

		LW_MPUNLOCK();
		return 0;
	}
	else
	{
		if ((sid = ip->i_seg) == 0)
			return 0;
	}


	/* sid is not zero. process a segment
	 */
	sidx = STOI(sid);

	ASSERT(!(CSA->backt = 0)); 
	SCB_MPLOCK(sidx);

	/*
	 * For .indirect or disk map segments acquire the FS lock
	 * to serialize pageout of pages in these segments with
	 * references in other critical sections.
	 */
	if (scb_indseg(sidx) || scb_dmapseg(sidx))
		FS_MPLOCK(scb_devid(sidx));
	LW_MPLOCK();

	lsidx = scb_logsidx(sidx);
	for (k = scb_sidlist(sidx); k >= 0; k = next)
	{
		next = pft_sidfwd(k);
		if (!ISMOD(k))
			continue;

                /* update logage ?
                 */
                if(logval = pft_logage(k))
		{
                	/* calculate difference between logval
                	 * and logsync and update logcur.
                	 */
                	logdiff(diff,logval,lsidx);
                	scb_logcur(lsidx) = MIN(diff,scb_logcur(lsidx));
		}	

                /* initiate pageout or mark pft_syncpt bit.
                 */
                if (pft_homeok(k) & pft_inuse(k))
                        v_pageout(sidx,k,NOFBLRU);
                else
                        pft_syncpt(k) = 1;
        }

#ifndef _POWER_MP
	/* initiate i/o if there is any to do.
	 */
	if (pf_iotail >= 0)
		v_pfsio();
#endif /* _POWER_MP */

	LW_MPUNLOCK();
	if (scb_indseg(sidx) || scb_dmapseg(sidx))
		FS_MPUNLOCK(scb_devid(sidx));
	SCB_MPUNLOCK(sidx);

	return 0;
}

/*
 * NAME:	vm_gettlock(vaddr, length)
 *
 * FUNCTION: glue function for v_gettlock()
 *	     for multi-segment file system object.
 *
 * note: see v_gettlock() for further information.
 *
 */
vm_gettlock(vaddr, length)
int	vaddr;
int	length;
{
	int	sid;

	sid = mfsri(vaddr); 
	return (vcs_gettlock(sid, vaddr, length));
}


/*
 * NAME:        v_gettlock (sid, vaddr, length)
 *
 * FUNCTION:    Attempts to grant transaction data lock to transaction. 
 *
 *		If tid is zero on entry, a tid value and a tblk structure 
 *		is allocated. 
 *		If lock can be granted, 
 *		update lockword and page frame table entry, and
 *		returns 0.
 *		If tid must wait returns VM_WAIT. 
 *              If there are no free lockwords returns ENOMEM.
 *
 *		The region requested to lock is restricted within a page,
 *		and may consists of one or more lines. 
 *              we only have one lockword per page. 
 *
 *              This program runs as a VMM critical section with normal
 *              back-tracking rules.
 *
 *		The ordering of operations performed by this routine
 *		are followed by v_getlw().
 *
 * PARAMETERS:  sid - segment identifier
 *              vaddr - start virtual address to lock
 *              length - length of region to lock
 *
 * RETURN :     0   - ok lock granted.
 *              VM_WAIT - process must wait. process is v_waited.
 *              ENOMEM  - no lockwords.
 *
 */
v_gettlock(sid, vaddr, length)
int sid;                   
int vaddr;               
int length;            
{
        int page, f, laddr, line, t, k, t1, hash, rc ,lsidx;
        int diff, diff1, n, prev, sr13save;

#ifdef _VMM_MP_EFF
	ASSERT(SCB_LOCKMINE(STOI(sid)) || scb_combit(STOI(sid)));
#endif
	LW_LOCKHOLDER();

 	/* the region requested to lock is restricted within a page */
	ASSERT((vaddr & POFFSET) + length <= PSIZE);

	/* relocate input segment to TEMPSR (sr13):
	 * caller maps input segment .inode and .indirect in sr12, but 
         * vmvcs overrides sr11 as VMMSR and sr12 as PTASR.
	 */
	sr13save = chgsr(TEMPSR, SRVAL(sid, 0, 0));
	vaddr = SR13ADDR + (vaddr & SOFFSET);

	/* make sure pfte for the page is allocated and in_use state */
	TOUCH(vaddr);

	/* compute start line address and line number of
	 * the region to lock
	 */
	laddr = vaddr & ~(LINESIZE - 1);
	line = ((uint)vaddr & POFFSET) >> L2LINESIZE;

        /* allocate a tid and tblk structure ?
         */
        if ((t = u.u_fstid)  == 0)
                if (rc = v_gettblk (&t)) {
			(void)chgsr(TEMPSR, sr13save);
                        return rc;
		}

        /* search lock table for locks on page
         */
	page = ((uint)vaddr & SOFFSET) >> L2PSIZE;
	f = v_lookup(sid, page);
        hash = (sid ^ page ) & (NLOCKHASH - 1);

        for (k = lanch.lockhash[hash]; k > 0; k = lword[k].next)
        {
                if (sid != lword[k].sid || page != lword[k].page)
                        continue;

		/* if tid already has lock on page grant it.
		 */
                if (t == lword[k].tid)
                        goto grantlock;

		/* lword.tid  == 0. can occur if sid is a .indirect segment.
		 */
		if (lword[k].tid == 0)
		{
			/* remove from tid = 0 lock list
			 */
			prev = 0;
			n = lanch.tblk[0].next;
			while (n != k)
			{
				prev = n;
				n = lword[n].tidnxt;
			}
			if (prev)
				lword[prev].tidnxt = lword[k].tidnxt;
			else
				lanch.tblk[0].next = lword[k].tidnxt;

			/* assign lockword to t
			 */
			lword[k].tid = t;
			lword[k].bits = 0;
			lword[k].flag = WRITELOCK;
			goto common;
		}
                goto lockwait;
        }

        /* no locks held on page. allocate a lockword and put it
         * on hash chain and tblk list. 
         */
        if ((k = lanch.freelock) == 0)
        {
		if (rc = v_morelocks())
		{
			(void)chgsr(TEMPSR, sr13save);
			return ENOMEM;
		}
		k = lanch.freelock;
        }

        lword[k].sid = sid;
        lword[k].page = page;
        lword[k].tid  = t;
        lword[k].bits = 0;
        lword[k].flag = WRITELOCK;

        /* everything touched. we can now take it off the
         * free list safely.
         */
        lanch.freelock = lword[k].next;

        /* put lockword k on hash chain
         */
        lword[k].next = lanch.lockhash[hash];
        lanch.lockhash[hash] = k;

	/* fill in disk addresses
	 */
	lword[k].extmem = 0;
	lword[k].home = pft_dblock(f);

common:
	/* put lockword on transaction's list
	 */
        lword[k].tidnxt = lanch.tblk[t].next;
        lanch.tblk[t].next = k;

        /* set log age of page to last value in log or keep
         * its old value if it is non-zero. 
	 * set homeok bit to false.
	 * set modbit in case line lock is taken from unmodified
	 * page as unconditional guarantee of taking log of the line.
         */
        lsidx = scb_logsidx(STOI(sid));
        pft_logage(f) = (pft_logage(f)) ? pft_logage(f) : scb_loglast(lsidx);
        lword[k].log = pft_logage(f);
        pft_homeok(f) = 0;
	SETMOD(f);

        /* update logage of transaction
         */
        if(lanch.tblk[t].logage == 0)
        {
                lanch.tblk[t].logage = pft_logage(f);
                lanch.tblk[t].lsidx = lsidx;
        }
        else
        {
                /* must compare logage of page and logage of
                 * transaction. the assert says we only allow
                 * a transaction to use one log.
                 */
                ASSERT(lanch.tblk[t].lsidx == lsidx);
                logdiff(diff, pft_logage(f), lsidx);
                logdiff(diff1, lanch.tblk[t].logage, lsidx);
                if (diff < diff1)
                        lanch.tblk[t].logage = pft_logage(f);
        }

grantlock:
	/* set line bit map in lockword */
	for ( ; laddr < vaddr + length; laddr += LINESIZE, line++) {
        	lword[k].bits |= 1 << (NLOCKBITS - line - 1);
	}

	(void)chgsr(TEMPSR, sr13save);
        return 0;

lockwait:
        /* lockword k describes conflicting lock
         */
        lanch.tblk[t].waitsid = sid;
        lanch.tblk[t].waitline = line;
        t1 = lword[k].tid;                      /* holder of conflicting lock */
        lanch.tblk[t].locker = t1;
        v_wait (&lanch.tblk[t1].waitors);       /* put t on wait list of t1 */
	(void)chgsr(TEMPSR, sr13save);
        return VM_WAIT;
}

/*
 * NAME:        v_gettblk (tid)
 *
 * FUNCTION:    Allocates a tid value = index of tblk in lanch.
 *              loads tid reg with tid. returns 0 ok, VM_WAIT if
 *		syncwait is currrently set or out of tblk structures.
 *		sets tid value allocated in mstsave area in ublock.
 *
 *              This code is called on fixed stack at VMM interrupt
 *              level.
 *
 * PARAMETERS:  tid     - transaction id
 *
 * RETURN :     0 - ok
 *		VM_WAIT - wait until syncwait cleared or a free
 *			  tblk is availiable.
 *
 */

v_gettblk (tid)
int *tid;                       /* tid value and index of tblk */
{
        int t, rc;

	LW_LOCKHOLDER();

	if (lanch.syncwait)
	{
		v_wait(&lanch.tblkwait);
		return VM_WAIT;
	}

        if ((t = lanch.freetid) == 0)
	{
		v_wait(&lanch.freewait);
		return VM_WAIT;
	}
        else
        {
		TOUCH(tid);
                lanch.freetid = lanch.tblk[t].next;
                lanch.nexttid++;
                lanch.maxtid = MAX(t,lanch.maxtid);

		bzero(&lanch.tblk[t], sizeof(struct tblock));
                lanch.tblk[t].logtid = lanch.nexttid;
		lanch.tblk[t].tid = t;
		lanch.tblk[t].gcwait = EVENT_NULL;

                /* load tid in u-block
                 */
                u.u_fstid = t;
                *tid = t;
                rc = 0;
        }
        return rc;
}

/*
 * v_getlw (sid,pno,lw)
 *
 * set lw to the index of the lockword for the page specified and makes
 * sure the lockword is memory resident.  if no lockword exists lw is 
 * set to 0.  also, if pno is journaled and no lockword exists, this
 * routine checks that the resources needed in allocating a lockword
 * are available.  if the process must wait for resources to become
 * available VM_WAIT is returned.  this routine also assures that
 * one lw is avaliable for use as a movedfrag structure.
 *
 * this routine is only called by v_makefrag() and the ordering of
 * operations performed by this routine are based upon the ordering
 * of operations performed in lockword allocation (v_gettlock()).
 *
 *
 * PARAMETERS:
 *	sid	- relative segment id
 *	pno	- relative page number
 *	lw	- returned lockword index for the page or zero
 *
 * RETURN
 *	0 	- ok.
 *	ENOMEM 	- no free lockwords.
 *	VM_WAIT	- wait until syncwait cleared or a free tblk is
 *		  tblk is availiable.
 */
v_getlw(sid,pno,lw)
int sid;
int pno;
int *lw;
{
        int k, touch2 = 0;

	LW_LOCKHOLDER();

        /* check if a lockword currently exists for the page.  if so,
         * touch it.
         */
        if (*lw = k = v_findlw(sid,pno))
        {
                TOUCH(&lword[k]);
                TOUCH((char *)(&lword[k+1]) - 1);
        }
        else
        {
                /* no lockword for the page.
                 */
                if (scb_jseg(STOI(sid)))
                {
                        /* if a transaction block is required, make sure
                         * that one is avaliable.
                         */
                        if (u.u_fstid == 0)
                        {
                                if (lanch.syncwait)
                                {
                                        v_wait(&lanch.tblkwait);
                                        return VM_WAIT;
                                }

                                if (lanch.freetid == 0)
                                {
                                        v_wait(&lanch.freewait);
                                        return VM_WAIT;
                                }
                        }

                        /* make note of the fact that two lockwords
                         * (one lockword and one movedfrag) must be
                         * touched.
                         */
                        touch2 = 1;
                }
        }

        /* re-populate the lockword freelist if it is empty or
         * does not contain enough free lockwords.
         */
        k = lanch.freelock;
        if (k == 0 || (touch2 && lword[k].next == 0))
        {
                if (v_morelocks())
                        return(ENOMEM);
                k = lanch.freelock;
                assert(k);
        }

        /* touch the head of the free list.
         */
        TOUCH(&lword[k]);
        TOUCH((char *)(&lword[k+1]) - 1);

        /* if a second lockword (movedfrag) may be required touch
         * the next lockword on the freelist.
         */
        if (touch2)
        {

                k = lword[lanch.freelock].next;
                assert(k);
                TOUCH(&lword[k]);
                TOUCH((char *)(&lword[k+1]) - 1);
        }

        return 0;
}


/*
 * v_frtblk(tid)
 *
 * free transaction block with index = tid. set tid value
 * in tid register and in u-block to 0. also clear logage
 * field.
 *
 * updates the logcur value used in calculating logsync
 * in v_sync().
 *
 */

v_frtblk(tid)
int     tid;  /* index in tblk array = tid reg value */
{
        int lsidx,logage,diff, k;

	LW_LOCKHOLDER();

        /* update log sync value
         */
        lsidx = lanch.tblk[tid].lsidx;
        logage = lanch.tblk[tid].logage;
        logdiff(diff,logage,lsidx);
        scb_logcur(lsidx) = MIN(diff,scb_logcur(lsidx));

        /* put tblk back on free list and clear logage field.
         */
        lanch.tblk[tid].logage = 0;
        lanch.tblk[tid].next = lanch.freetid;
        lanch.freetid = tid;

        /* clear u-block tid
         */
        u.u_fstid = 0;

	/* wakeup any waitors waiting for a free tblk.
	 */
	while(lanch.freewait)
		v_ready(&lanch.freewait);

	/* return if syncwait not set
	 */
	if (lanch.syncwait == 0)
		return 0;

	/* see if there are any more active transactions
	 */
       	for (k = 1; k <= lanch.maxtid; k++)
       	{
               	if(lanch.tblk[k].logage != 0)
			return 0;
       	}

	/* clear syncwait. wakeup any waitors.
	 */
	lanch.syncwait = 0;
	while(lanch.tblkwait)
		v_ready(&lanch.tblkwait);

	return 0;
}

/*
 * NAME:        v_frlock (tid)
 *
 * FUNCTION:    Frees line locks marked FREELOCK of a transaction.
 *		makes ready any transactions waiting for a lock
 *		held by tid. 
 *
 *              the procedure executes at VMM interrupt level with
 *              back tracking enabled.
 *
 * PARAMETERS:  tid     - transaction id value
 *
 * RETURN :     0
 *
 */

v_frlock (tid)
int tid;          
{
        int k, nfr, sid, page,next;

	/*
	 * This transaction is frozen during commit and the
	 * lock word chain can be safely accessed w/o lock.
	 * However, we still need the LW lock to free the
	 * lockwords so we take it once across the entire
	 * operation.
	 */
	LW_MPLOCK_S();

        for(k = lanch.tblk[tid].next; k > 0; k = next)
        {
		next = lword[k].tidnxt;
                if ((lword[k].flag & FREELOCK) == 0)
			continue;

		/* free the lockword
		 */
		v_rmlock(k);
        }

        /* make ready any processes waiting on tid
	 */
        while(lanch.tblk[tid].waitors != NULL)
           v_ready(&lanch.tblk[tid].waitors);
	
	LW_MPUNLOCK_S();

	return 0;
}

/*
 * NAME:        v_rmlock (lw)
 *
 * FUNCTION:    removes lockword from hash-chain and transaction
 *		list and puts on free list. frees any extension
 *		memory disk-block associated with lockword.
 *
 * PARAMETERS:  lw - index of lockword.
 *
 * RETURN :     0
 *
 */

v_rmlock (lw)
int	lw;
{
        int sid, page, m, n, hash, prev, tid;
	int pdtx , dblk, daddr;

	LW_LOCKHOLDER();

	/* find previous lockword on tid list
	 * do this first so that things are touched.
	 */
	prev = -1;
	tid = lword[lw].tid;
	n = lanch.tblk[tid].next;
	while( n != lw)
	{
		prev = n;
		n = lword[n].tidnxt;
	}

	/* remove lockword lw from hash chain
	 */
	sid = lword[lw].sid;
	page = lword[lw].page;

	hash = (sid ^ page) & (NLOCKHASH - 1);
	m = -1;
	n = lanch.lockhash[hash];
	while ( n != lw)
	{       m = n;
		n = lword[n].next;
	}
	if (m < 0)
		lanch.lockhash[hash] = lword[lw].next;
	else
		lword[m].next = lword[lw].next;

	/* remove lockword from tid list and put on free
	 * list. 
	 */
	lword[lw].next = lanch.freelock;
	lanch.freelock = lw;
	if (prev < 0)
		lanch.tblk[tid].next = lword[lw].tidnxt;
	else
		lword[prev].tidnxt = lword[lw].tidnxt;

	/* free paging space disk if any
	 */
	if (daddr = lword[lw].extmem)
	{
		pdtx = PDTIND(daddr);
		dblk = PDTBLK(daddr);

		PG_MPLOCK(pdtx);

		v_dfree(pdtx,dblk);

		PG_MPUNLOCK(pdtx);

		FETCH_AND_ADD(vmker.psfreeblks, 1);
	}

	return 0;
}

/*
 * v_findlw (sid,pno)
 * returns the index of the lockword for the page specified
 * or 0 if it is not found.
 */

v_findlw (sid,page)
int sid;
int page;
{
	int hash,k;

	LW_LOCKHOLDER();

        hash = (sid ^ page ) & (NLOCKHASH - 1);

        for (k = lanch.lockhash[hash]; k > 0; k = lword[k].next)
        {
                if (sid == lword[k].sid && page == lword[k].page)
			return(k);
        }
	return 0;
}

/*
 * v_freelw (sid,pno)
 *
 * removes the lock word specified from lock-table.
 * if there is a paging space disk block associated
 * with the lock word it is freed.
 *
 * you may call this procedure even if there is no
 * lock-word for the specified page.
 *
 */

v_freelw (sid,page)
int sid;
int page;
{
	int lw;

	SCB_LOCKHOLDER(STOI(sid));

	LW_MPLOCK_S();

	if ( lw = v_findlw(sid,page))
		v_rmlock(lw);

	LW_MPUNLOCK_S();

	return 0;
}

/*
 * v_insertlw (sid,page)
 * inserts lockword if not already in lock table.
 * returns its index or 0 if there is no memory.
 *
 * this procedure is intended for extension memory 
 * usage. if a lockword is inserted it is associated
 * with transaction id 0 (dummy).
 *
 * you may call this procedure even if the lockword
 * is already in the table.
 */

v_insertlw (sid,page)
int sid;
int page;
{
	int hash,k;

	SCB_LOCKHOLDER(STOI(sid));
	LW_MPLOCK_S();

	/* look for it in hash table
	 */
        hash = (sid ^ page ) & (NLOCKHASH - 1);
        for (k = lanch.lockhash[hash]; k > 0; k = lword[k].next)
        {
                if (sid == lword[k].sid && page == lword[k].page)
		{
			LW_MPUNLOCK_S();
			return (k);
		}
        }

	/* get a lockword
	 */
        if ((k = lanch.freelock) == 0)
        {
		if (v_morelocks())
		{
			LW_MPUNLOCK_S();
			return 0;
		}
		k = lanch.freelock;
        }

	/* remove from free list. insert on tid 0 list and hash chain.
	 */
	lanch.freelock = lword[k].next;
	lword[k].tidnxt = lanch.tblk[0].next;
	lanch.tblk[0].next = k;
	lword[k].next = lanch.lockhash[hash];
	lanch.lockhash[hash] = k;

	/* initialize fields in the lockword.
	 */
        lword[k].sid = sid;
        lword[k].page = page;
        lword[k].tid  = 0;
        lword[k].bits = 0;
        lword[k].flag = 0;
	lword[k].log = 0;
	lword[k].extmem = 0;
	lword[k].home = 0;

	LW_MPUNLOCK_S();

	return (k);
}

/*
 * v_deletelws(sid)
 *
 * frees all the lockwords associated with a segment and 
 * frees any paging space disk blocks associated with it.
 *
 * input parameters 
 *		sid - base segment id of a journalled
 *		      or deferred-update segment.
 *
 * this procedure is only called by vmdelete.c.
 *
 * the procedure executes at VMM interrupt level on a fixed 
 * stack with back-tracking enabled.
 *
 */

v_deletelws(sid)
int sid;
{
	int k,next,n;

	SCB_LOCKHOLDER(STOI(sid));
	LW_LOCKHOLDER();

	sid = BASESID(sid);

	for (k = 0; k <= NLOCKHASH - 1; k ++)
        {
		for (n = lanch.lockhash[k]; n ; n = next)
		{
			next = lword[n].next;
			if (BASESID(lword[n].sid) == sid)
				v_rmlock(n);
		}
        }

	return 0;
}

/*
 * NAME:        v_lockseg (sid)
 *
 * FUNCTION:    Sets commit bit in sidtable entry for segment sid
 *              which prevents allocation of new disk blocks in the
 *		segment. 
 * 
 *              this procedure is called on fixed stack at VMM 
 *              interrupt level via vcs_lockseg.
 *
 * PARAMETERS:  sid     - segment id
 *
 * RETURN :     0	- ok lock bit was not set
 *		1	- ok lock bit was already set.
 *
 */

int
v_lockseg (sid)
int sid;
{
	int sidx, retval;

	sidx = STOI(sid);

	SCB_LOCKHOLDER(sidx);

	retval = (scb_combit(sidx)) ? 1 : 0;
        scb_combit(sidx) = 1;

	return retval;
}

/*
 * NAME:        v_unlockseg (sid)
 *
 * FUNCTION: resets commit bit in sidtable entry for segment sid
 *           and wakes up any processes blocked because it tried
 *	     allocate disk while a commit was in progress. also
 *	     resets scb_newdisk to zero.	
 *
 *           this procedure is executes on the fixed stack at VMM
 *	     interrupt level.	
 *
 * PARAMETERS:  sid     - segment id
 *
 * RETURN :     zero
 *
 */

v_unlockseg(sid,dreset)
int sid;
int dreset;
{
        int sidx;

        sidx = STOI(sid);

	SCB_LOCKHOLDER(sidx);

        scb_combit(sidx) = 0;

	if (dreset)
		scb_newdisk(sidx) = 0;

        while(pf_extendwait != NULL)
                v_ready (&pf_extendwait);

        return 0;
}


/*
 * v_iowait(sid)
 *
 * calling process is v_waited until all pageouts for the
 * segment specified are complete.
 *
 * this procedure should be called on fixed stack but NOT
 * with back-tracking enabled.
 *
 * Return values
 *      0       -       no wait
 *      VM_WAIT -       process must wait
 *	EIO	-	I/O error occurred
 */

v_iowait(sid)
{
        int polevel,sidx,iotail,head,nfr;
	struct thread * tp;

	/* if an I/O error has occurred on the segment return EIO.
	 */
	sidx = STOI(sid);

	SCB_LOCKHOLDER(sidx);

	if (scb_eio(sidx))
		return(EIO);

        /* is there any io for segment ?
         */
        head = scb_sidlist(sidx);
        if (head < 0 || (iotail = pft_sidbwd(head)) < 0)
                return(0);

        /* determine the i/o level when all scheduled
         * pageouts are complete. pft_nonfifo is 0 for
         * pageins so it can be added into polevel.
         */
        polevel = scb_iolev(sidx);
        for(nfr = head; ; nfr = pft_sidfwd(nfr))
        {
                polevel += pft_nonfifo(nfr);
                if(nfr == iotail)
                        break;
        }

        /* were there any pageouts ?
         */
        if (polevel == scb_iolev(sidx))
                return(0);

        /* set polevel in proc block and v_wait caller
         * note : this works in conjuction with v_pfend
         * which together assume that v_wait and v_ready
         * service procs in FIFO order.
         */
	tp = curthread;
	tp->t_polevel = polevel;
        v_wait(&scb_iowait(sidx));

        return VM_WAIT;
}

/*
 * v_purge(sid,page)
 *
 * This procedure is used for freeing blocks in .indirect segments.
 * If any blocks are modified and not in a pageout state they will
 * be written to disk.  This must happen since .indirect blocks
 * are deferred in commit3().  If an inode is recycled and .indirect
 * has not been written during v_sync() then it will get written here.
 *
 * the page frame is released if it is in memory and
 * any lockword associated with the page is also freed.
 *
 * Return value -  0   ok
 */
v_purge(sid,page)
int  sid;    /* segment id */
int  page;   /* page number in segment */
{
        int sidx,pdtx,dblk,daddr,lw,nfr;

	SCB_LOCKHOLDER(STOI(sid));

	/*
	 * FS lock must be held to serialize pageout of .indirect
	 * with references to these pages in other critical sections.
	 */
	FS_LOCKHOLDER(scb_devid(STOI(sid)));

        /* if page is in memory release the frame.
	 * if it was in i/o state wait for it to finish
         */
        if((nfr = v_lookup(sid,page)) > 0)
        {
        	sidx = STOI(sid);
		if (ISMOD(nfr) && pft_inuse(nfr) && pft_homeok(nfr))
		{
			v_pageout(sidx, nfr, NOFBLRU);
#ifndef _POWER_MP
			if (pf_iotail >= 0)     /* initiate io */
				v_pfsio();
#endif /* _POWER_MP */
		}

		if (pft_pagein(nfr) || pft_pageout(nfr))
		{
			/*
			 * v_relframe will not release the frame,
			 * we need to wait for it.
			 */
			v_relframe(sidx,nfr);
			v_wait(&pft_waitlist(nfr));
			return VM_WAIT;
		}
		else
			v_relframe(sidx,nfr);
        }

	/* free the lockword if any and also paging space 
	 * disk block if there is one
	 */
	v_freelw(sid,page);

        return 0;
}

/*
 * NAME:        vm_initlock()
 *
 * FUNCTION:    Initialize transaction and lockword stuff. this is
 *              called from xix_init.
 *
 *              The first (i.e. index 0) entry in lword is not used
 *              index = 0 is the null value for list ptrs.
 *
 *		The first (i.e. index 0) entry in tblk is used as 
 *		an anchor for lockwords used solely for representing
 *		extension memory (i.e. deferred-update segments and 
 *		in some cases .indirect pages prior to commit/delete
 *		processing).
 *
 * RETURN :     Zero
 *
 */

vm_initlock()
{
        int k,begin,nlocks;

	/* MP note:
	 * No lock needed it is called only at system initialization
	 * time and thus with a single thread of execution.
	 * Moreover we are at base level.
	 */

        /* init lockanch.
         */
        lanch.nexttid = 1;
        lanch.freetid = 1;
        for (k = 1; k < NUMTIDS - 1 ; k++)
                lanch.tblk[k].next = k + 1;

        /* begin lockword array on next page after ames. see vmsys.h
         */
	begin = &vmmdseg.ame[NUMAMES];
	begin = (begin + PSIZE - 1) & (~(PSIZE - 1));
	lword  = (struct lockword *) begin;
	lanch.morelocks = begin + PSIZE;  

	/* initialize first page of lockwords. lockword 0
	 * is not used.
	 */
        lanch.freelock = 1;
	nlocks = PSIZE/sizeof(struct lockword);
	for (k = 1; k < nlocks - 1; k ++)
		lword[k].next = k + 1;

        return 0;
}

/*
 * v_morelocks()
 * 
 * add the next page of free lockwords to the free-list.
 * lockwords which straddle a page boundary are not added
 * to the list.
 *
 * Return value
 * 		0 -ok
 *		ENOMEM  - no more free lock words.
 */

int
v_morelocks()
{
	int nbytes,before,first,last,k, size;

	LW_LOCKHOLDER();

	/* is morelocks into the next segment ?
	 */
	if ( SOFFSET & lanch.morelocks == 0)
		return(ENOMEM);

	/* calculate index of first and last lockwords wholly
	 * contained in the page pointed to by morelocks. nbytes
	 * is the number of bytes used up to now for lockwords.
	 */
	nbytes = lanch.morelocks - (uint)lanch.lwptr;
	size = sizeof(struct lockword);
	before = nbytes/size;
	first = (before*size == nbytes) ? before : before + 1;
	last = (nbytes + PSIZE)/size - 1;

	/* put them on the free list
	 */
	for ( k = first; k < last - 1; k++)
		lword[k].next = k + 1;
	lword[last].next = lanch.freelock;
	lanch.freelock = first;
	lanch.morelocks += PSIZE;

	return 0;
}

/*
 * v_makelogp(sid,nextpno,lpage)
 *
 * materializes in memory next page and initializes
 * it according to the log seq page number lpage.
 *
 * input parameters 
 *	sid	- segment id of log
 *	nextpno - next page in log.
 *	lpage	- log page sequence number for nextpno.
 *
 * this program executes at VMM interrupt level with back-tracking
 * enabled.
 */

int 
v_makelogp(sid, nextpno, lpage)
int	sid;
int	nextpno;
int	lpage;
{
	struct logpage *ptr;
	int nfr,sidx, rc;

	sidx = STOI(sid);

	SCB_LOCKHOLDER(sidx);

	/* if there are no free frames try to get some.
	 */
	if (rc = v_spaceok(sidx, 1))
	{
		return(rc);
	}

	/* a log is NOT a file so it does not have an external
	 * page table (inode + indirect blocks). however, the disk
	 * block associated with page p of a log is block p.
	 */
	ASSERT(scb_logseg(sidx));

	/* get a page frame unless nextpno is still in memory.
	 * (not a likely case).
	 */
	if (v_lookup(sid,nextpno) == -1)
	{
        	nfr = v_getframe(sid,nextpno);
        	pft_devid(nfr) = scb_devid(sidx);
        	pft_dblock(nfr) = nextpno;

		/* Record correct protection key for the page.
		 * Insert the page at its normal address.
	 	 */
		pft_key(nfr) = FILEKEY;
		P_ENTER(NORMAL,sid,nextpno,nfr,pft_key(nfr),pft_wimg(nfr));
		pft_inuse(nfr) = 1;
		v_insscb(sidx,nfr);
		scb_maxvpn(sidx) = MAX(scb_maxvpn(sidx), nextpno);
	}

	/* make it addressable and format it.
	 */
	(void)chgsr(TEMPSR, SRVAL(sid,0,0));
	ptr = (struct logpage *)((TEMPSR << L2SSIZE) + (nextpno << L2PSIZE));
	TOUCH(ptr);
	ZEROPAGE(sid,nextpno);
	ptr->h.page = ptr->t.page = lpage;
	ptr->h.xor = ptr->t.xor = 0;
	ptr->h.eor = ptr->t.eor = 8;

        return(0);
}

/*
 * v_writelogp(sid,cpno,ppongpno,release)
 *
 * initiates write of current log page to ping pong address
 *
 * input parameters 
 *	sid	- segment id of log
 *	cpno	- current page number in log of origin
 *	ppongpno - ping pong destination page to write
 *	release	 - release page after write
 * 
 * RETURNS: 0 	   - success
 *
 * this program executes at VMM interrupt level with back-tracking
 * enabled.
 */
int 
v_writelogp(int sid, int cpno, int ppongpno, int release)
{
	int nfr,sidx;

	sidx = STOI(sid);
	SCB_LOCKHOLDER(sidx);

        if ((nfr = v_lookup(sid,cpno)) >= 0)
        {
		ASSERT(!pft_pageout(nfr));
		pft_dblock(nfr) = ppongpno;
	}
	else 
		assert(0);

	/* Ensure cache gets flushed for POWER platforms.
         * Keep page addressible but mark it for pageout.
         */
	P_ENTER(STARTIO,pft_ssid(nfr),pft_spage(nfr),nfr,pft_key(nfr),
		pft_wimg(nfr));
	pft_pageout(nfr) = 1;

	v_mtioscb(sidx,nfr);

	PDT_MPLOCK();
	v_pdtqiox(pft_devid(nfr),nfr,0);
	PDT_MPUNLOCK();

#ifndef _POWER_MP
	/* initiate i/o if any was enqued by above.
	 */
	if (pf_iotail >= 0)
		v_pfsio();
#endif /* _POWER_MP */

	if (release)
		v_release(sid, cpno, 1, 1);

	return 0;
}


/*
 * v_mapd(sid,daddr)
 *
 * the block daddr is marked as allocated in both work
 * and permanent disk map.
 *
 * this procedure is only used by the xix_cntl for adding the
 * inode blocks of a new allocation unit. 
 *
 * this procedure executes at VMM interrupt level with back-tracking.
 *
 * return value - 0.
 */

int
v_mapd(sid,daddr)
int	sid;
int	daddr;
{
	int sidx, rc;
	uint p,w,rem,bit, pdtx, nag;
	struct vmdmap * p0 , *p1;
	uint version, *wmap, *pmap, dbperpage;

	/* In mp, scb lock not needed.
	 */
	sidx = STOI(sid);

	/* get pointer to page zero of map
	 */
	pdtx = scb_devid(sidx);
	(void)chgsr(TEMPSR,pdt_dmsrval(pdtx));

	FS_LOCKHOLDER(pdtx);

	p0 = (struct vmdmap *) ( TEMPSR << L2SSIZE);
	version = p0->version;
	dbperpage = WBITSPERPAGE(version);
	
	/* calculate page, word, and bit number corresponding
	 */
	p = daddr/dbperpage;
	rem  = daddr - p*dbperpage;
	w = rem >> L2DBWORD;
	bit = (rem - (w << L2DBWORD));
	nag = rem/p0->agsize;

	if (version == ALLOCMAPV3)
	{
		p1 = p0 + p;
		wmap = (uint *)p1 + LMAPCTL/4;
		pmap = wmap + WPERPAGE;
	}
	else
	{
		p1 = p0 + 9*(p>>3);
		wmap = (uint *) (p1 + 1 + (p & 0x7));
		pmap = wmap + WPERPAGEV4;
		p1 = VMDMAPPTR(p1, p);
		TOUCH(p1);
	}
		

	/* mark it as allocated and update control info.
	 * update tree.
	 */
	wmap[w] |= (UZBIT >> bit);
	pmap[w] |= (UZBIT >> bit);
	p0->freecnt += -1;
	p1->agfree[nag] += -1;
	v_updtree(p1, w);

	return 0;
}

/*
 * v_defer (sid)
 *
 * changes persistent segment to deferred update.
 * v_write() is called to write modified pages to
 * home location. segment is maded deferred update.
 *
 */

v_defer(sid)
int sid;
{
	int sidx;
	 
	/* get sidx.
	 */
	sidx = STOI(sid);

	SCB_LOCKHOLDER(sidx);

	/* sync the segment.
	 */
	v_write(sid,0,MAXFSIZE/PSIZE,V_FORCE);

	/* set defer segment.
	 */
	scb_defseg(sidx) = 1;

	return 0;
}

/*
 *	v_setlog(sid, pno, logage, dirty)
 *
 * sets the logage of a page frame if it is in memory.
 *
 * returns  - 0 OK
 *	      VM_NOTIN - page frame is not in memory.
 *
 * this program executes at VMM interrupt level on
 * a fixed stack without back-tracking.
 *
 */

v_setlog(sid, pno, logage, dirty)
int sid;
int pno;
int logage;
int dirty;
{
	int nfr;

	SCB_LOCKHOLDER(STOI(sid));

	if ((nfr = v_lookup(sid,pno)) > 0 && pft_inuse(nfr))
	{
		if (dirty)
			pft_logage(nfr) = logage;
		else
			pft_homeok(nfr) = 1;

		return 0;
	}

	return VM_NOTIN;
}

/*
 * NAME:        v_allocdq (ip,newfrags,fperpage)
 *
 * FUNCTION:	check if new fragments can be allocated for a inode.  issue 
 *		error message if a hard limit is reached or a warning 
 *		message if the soft limit is reached.
 * 
 *              this procedure is called the the page fault handler and
 *		fragment allocator when allocating file system disk
 *		fragments for an inode.
 *
 * PARAMETERS:  ip	  - inode pointer.
 *		newfrags  - number of fragments to be allocated for inode.
 *		fperpage  - fragments per page for the file system.
 *
 * RETURN :     0	- success.
 *		EDQUOT	- reached disk block hard limit.
 *		VM_WAIT - a locked dquot was found - wait.
 *
 */

int
v_allocdq(ip,newfrags,fperpage)
struct inode *ip; 
int newfrags;
int fperpage;
{
	int rc, type, nbytes, nblocks, npart, cblocks, cpart, msgno;
	struct dquot *dp;

	SCB_LOCKHOLDER(STOI(ip->i_gnode.gn_seg));
	FS_LOCKHOLDER(scb_devid(STOI(ip->i_gnode.gn_seg)));

	/* return if no blocks allocated.
	 */
	if (newfrags == 0)
		return(0);

	/* Check if the inode's dquotas are locked.  This also ensures
	 * that all quota relevent data is touched.
	 */
	if (rc = v_chkdqlock(ip))
		return(rc);

        /* return if root.
         */
        if (U.U_uid == 0)
                return(0);
	
	/* convert newfrags into units we can work with.
	 */
	nbytes = newfrags * (PSIZE/fperpage);
	nblocks = nbytes / DQBSIZE;
	npart = (nbytes & (DQBSIZE-1)) ? 1 : 0;

	for (type = 0; type < MAXQUOTAS; type++)
        {
		/* continue if no dquot for this type.
	         */
                if ((dp = ip->i_dquot[type]) == NODQUOT)
                        continue;

		/* determine what current usage is with the new
		 * fragments added in.
		 */
		cpart = DQCARRY(dp->dq_btime);
		cblocks = nblocks + dp->dq_curblocks + (cpart & npart);

		/* check if the new blocks will cause the hard limit to
	 	 * be reached.
	 	 */
		if (dp->dq_bhardlimit && cblocks >= dp->dq_bhardlimit)
		{
			/* send hard limit error message if appropriate.
			 */
			if (!(dp->dq_flags & DQ_BLKS) && ip->i_uid == U.U_uid)  
			{
				msgno = (dp->dq_type == USRQUOTA) ?
					MSG_SYSPFS_01 : MSG_SYSPFS_02;
				v_dqmsg(msgno,dp->dq_dev);
				dp->dq_flags |= DQ_BLKS;
			}
			return(EDQUOT);
		}

		/* check if the hard limit has been reached because the
		 * time limit has expired for the soft limit.
		 */
		if (dp->dq_bsoftlimit && cblocks >= dp->dq_bsoftlimit &&
		    dp->dq_curblocks >= dp->dq_bsoftlimit &&
		    time > DQBTIME(dp->dq_btime))
		{
			/* send hard limit error message if required.
		  	 */
			if (!(dp->dq_flags & DQ_BLKS) && ip->i_uid == U.U_uid)
			{
				msgno = (dp->dq_type == USRQUOTA) ?
						MSG_SYSPFS_13 : MSG_SYSPFS_14;
				v_dqmsg(msgno,dp->dq_dev);
				dp->dq_flags |= DQ_BLKS;
			}
			return(EDQUOT);
		}
	}
	return(0);
}

/*
 * NAME:        v_freedq (ip,nfrags,fperpage)
 *
 * FUNCTION:    update disk fragment usage quota info for an inode
 *              to reflected freed disk fragments.
 * 
 *              this procedure is called as a backtracking critical
 *		section via vcs_freedq().
 *
 * PARAMETERS:  ip	 - inode pointer.
 *		nfrags	 - number of fragments freed.
 *		fperpage - fragments per page for the file system.
 *
 * RETURN :     0	- success.
 *		VM_WAIT - a locked dquot was found - wait.
 *
 */

int
v_freedq(ip,nfrags,fperpage)
struct inode *ip; 
int nfrags;
int fperpage;
{

	/* return if no blocks freed.
	 */
	if (nfrags == 0)
		return(0);

	/* check if the inode's dquotas are locked.
	 */
	if (v_chkdqlock(ip))
	{
		/* return code can be only VM_WAIT
		 */
		return(VM_WAIT);
	}

	/* update the inode's dquots to reflect the
	 * freed fragments.
	 */
	v_upddqblks(ip,-nfrags,fperpage);

	return(0);
}

/*
 * NAME:        v_upddqblks (ip,newfrags,fperpage)
 *
 * FUNCTION:	update an inode's disk quota information to reflect the
 *		addition or subtraction of disk fragments.
 * 
 *              this procedure is called the the page fault handler and
 *		fragment allocator when allocating file system disk
 *		fragments for an inode and v_freedq() when disk fragments
 *		are freed for an inode.
 *
 * PARAMETERS:  ip	  - inode pointer.
 *		newfrags  - number of fragments to be allocated for inode.
 *			    newfrags may be less than zero, reflecting the
 *			    deallocation of fragments for the inode.
 *		fperpage  - fragments per page for the file system.
 *
 * RETURN :     0	- success.
 *
 */

v_upddqblks(ip,nfrags,fperpage)
struct inode *ip;
int nfrags;
int fperpage;
{
	int type, nbytes, nblocks, npart, cpart, cblocks, msgno;
	struct dquot *dp;
	struct jfsmount *jmp;

	/* convert newfrags into units we can work with.
	 */
	nbytes = nfrags * (PSIZE/fperpage);
	nblocks = nbytes / DQBSIZE;
	npart = (nbytes & (DQBSIZE-1)) ? 1 : 0;
	if (nfrags < 0)
		nblocks -= npart;

	/* update the inode's dquot to reflect the addition or
	 * subtraction of fragments.
	 */
	for (type = 0; type < MAXQUOTAS; type++)
        {
                if ((dp = ip->i_dquot[type]) == NODQUOT)
                        continue;

		jmp = dp->dq_jmp;
		FS_LOCKHOLDER(v_devpdtx(D_FILESYSTEM,jmp->jm_dev));

		/* determine the new current usage value with newfrags
		 * factored in.
		 */
		cpart = DQCARRY(dp->dq_btime);
		cblocks  = nblocks + dp->dq_curblocks + (cpart & npart);
		/* can not have negative blocks */
		if (cblocks < 0)
			cblocks = 0;

		/* if new fragments are being added to the inode by anyone
		 * other than root and a softlimit exists, check if the
		 * softlimit will be reached.  if the limit is reached
		 * set the time limit and send a softlimit warning
		 * message.
		 */
		if (nfrags > 0 && dp->dq_bsoftlimit != 0 && U.U_uid != 0)
		{
			if (cblocks >= dp->dq_bsoftlimit &&
			    dp->dq_curblocks < dp->dq_bsoftlimit)
			{
				dp->dq_btime =
					DQBTIME(time + jmp->jm_btime[type]);
				if (ip->i_uid == U.U_uid)
				{
					msgno = (dp->dq_type == USRQUOTA) ?
						MSG_SYSPFS_05 : MSG_SYSPFS_06;
					v_dqmsg(msgno,dp->dq_dev);
				}
			}
        	}

		/* update current usage.  dquota marked as
		 * modified.
		 */
		dp->dq_curblocks = cblocks;
		dp->dq_btime = DQSBTIME((npart ^ cpart),dp->dq_btime);
                dp->dq_flags &= ~DQ_BLKS;
                dp->dq_flags |= DQ_MOD;
	}
	return 0;
}

/*
 * NAME:        v_dqmsg (msgno,devid)
 *
 * FUNCTION:    construct and print kernel disk quota message.
 * 
 * PARAMETERS:  msgno	- NLS message number.
 *		devid	- devid of file system associated with the
 *			  disk quota message.
 *
 * RETURN :     NONE
 *
 */

static
v_dqmsg(msgno,devid)
int msgno;
dev_t devid;
{
	struct uprintf up;

	/* fill out the uprintf struct with message info.  Note
	 * that defmsg will be provided by the uprintfd for
	 * SET_SYSPFS messages; however, defmsg in uprintf struct must
	 * contain a non-string conversion specification so that
	 * devid (fsid) will be handled correctly by NLuprintfx.
	 */
	up.upf_defmsg = "%x";
	up.upf_NLcatname = MF_UNIX;
	up.upf_NLsetno = SET_SYSPFS;
	up.upf_NLmsgno = msgno;
	up.upf_args[0] = (void *) devid;

	/* print the message.
	 */
	NLuprintfx(&up);
}

/*
 * NAME:	v_dqlock (dp)
 *
 * FUNCTION:	lock a dquot. if already locked, set want bit and
 *		sleep.
 *
 * PARAMETERS:	dp 	- pointer to dquot to lock
 *
 * RETURN :	NONE
 *			
 */
v_dqlock(dp)
struct dquot *dp;
{
	int pdtx;

	/* Touch dp->dq_lock not to fault after
	 * and avoid the scoreboarding.
	 */
	TOUCH(dp->dq_lock);

	pdtx = v_devpdtx(D_FILESYSTEM,dp->dq_jmp->jm_dev);
	ASSERT(pdtx >= 0);
	FS_MPLOCK(pdtx);

	if (dp->dq_lock & DQ_LOCK)
	{
		dp->dq_lock |= DQ_WANT;
		v_wait(&pf_dqwait); 
		FS_MPUNLOCK(pdtx);
		return(VM_WAIT);
	}
	dp->dq_lock |= DQ_LOCK;

	FS_MPUNLOCK(pdtx);

	return 0;
}

/*
 * NAME:        v_chkdqlock (ip)
 *
 * FUNCTION:    check if the inode's dquots are free.  also, touch
 *		in the dquotas and the file system's jfsmount structure.
 * 
 * PARAMETERS:  ip	  - inode pointer.
 *
 * RETURN :     0	- no locks.
 *		VM_WAIT - a locked dquot was found - wait.
 *
 */

int
v_chkdqlock(ip)
struct inode *ip; 
{
	struct dquot *dp;
	struct jfsmount *jmp;
	int type;


	/* check for locks on the inode's dquots.  if a lock 
	 * is found, set waitlist to pf_dqwait and return VM_WAIT.
	 */
	for (type = 0; type < MAXQUOTAS; type++)
	{
		if ((dp = ip->i_dquot[type]) == NULL)
			continue;

		/* touch the dquot.
		 */
		TOUCH(dp);
        	TOUCH((char *)(dp + 1) - 1);

		/* touch the jfsmount.
		 */
		jmp = dp->dq_jmp;

		FS_LOCKHOLDER(v_devpdtx(D_FILESYSTEM,jmp->jm_dev));

		TOUCH(jmp);
        	TOUCH((char *)(jmp + 1) - 1);


		/* Check if the dquot is locked.  Not relevent for the 
		 * root user
	 	 */
		if ((dp->dq_lock & DQ_LOCK) && U.U_uid != 0)
		{
			dp->dq_lock |= DQ_WANT;
			v_wait(&pf_dqwait);
			return(VM_WAIT);
		}
	}
	return(0);
}

/*
 * NAME:        v_unlockdq (dp)
 *
 * FUNCTION:    wakes up all process waiting on for a dquot.
 * 
 *              this procedure is a backtracking critical section called
 *		from vcs_unlockdq.
 *
 * PARAMETERS:  dp - pointer to dquot to be unlocked.
 *
 * RETURN :     0
 *
 */

int
v_unlockdq (dp)
struct dquot *dp; 
{
	int pdtx;

	/* Touch dp->dq_lock not to fault after
	 * and avoid the scoreboarding.
	 */
	TOUCH(dp->dq_lock);

	pdtx = v_devpdtx(D_FILESYSTEM,dp->dq_jmp->jm_dev);
	ASSERT(pdtx >= 0);
	FS_MPLOCK(pdtx);

	assert(dp->dq_lock & DQ_LOCK);

	/* check if anyone is waiting.  if so, wake all waiters
	 * wanting the dquot and ready all waiters sleeping in
	 * pager wait on pf_dqwait.
	 */
	if (dp->dq_lock & DQ_WANT)
	{
        	while (pf_dqwait != NULL)
                	v_ready(&pf_dqwait);
	}

	/* clear the lock and want bits.
	 */
	dp->dq_lock &= ~(DQ_LOCK|DQ_WANT);

	FS_MPUNLOCK(pdtx);

	return(0);
}

/*
 * NAME:        v_powait(sidx)
 *
 * FUNCTION:    v_waits the calling process if non-fblru pageout up limit
 *		(vmker.maxpout) is reached for the segment.  the i/o
 *		level the process is waited on is set based upon the
 *		global pageout pacing down limit (vmker.minpout).
 * 
 * 		this procedure should be called on fixed stack but NOT
 * 		with back-tracking enabled.  it is called only by iopace()
 *		via vcs_powait().
 *
 *
 * PARAMETERS:  sidx - segment index
 *
 * RETURN :     0	
 *		VM_WAIT
 *
 */

v_powait(sidx)
uint sidx;
{
        int polevel, numpos, pos, head, nfr;

	SCB_LOCKHOLDER(sidx);

	/* check if the pageout up limit exists.
	 */
	if (vmker.maxpout <= 0)
		return(0);

	assert(vmker.minpout >= 0 && vmker.maxpout > vmker.minpout);

	/* check if the number of pageouts queued for the
	 * segment has reached the pageout limit.
	 */
	if (scb_npopages(sidx) < vmker.maxpout)
		return(0);

	head = scb_sidlist(sidx);
	assert(head >= 0);
	assert(pft_sidbwd(head) >= 0);

        /* determine the number of non-fblru pageouts which
	 * must completed before the process is readied.
         */
	numpos = scb_npopages(sidx) - vmker.minpout;

	/* determine the pageout i/o level.
	 */
	polevel = scb_iolev(sidx);
        for (pos = 0, nfr = head; pos < numpos; nfr = pft_sidfwd(nfr))
        {
                polevel += pft_nonfifo(nfr);
		if (pft_pageout(nfr) && !pft_fblru(nfr))
			pos += 1;

		assert(!(nfr == pft_sidbwd(head) && pos < numpos));
        }

        /* set polevel in proc block and wait the caller.  this
	 * works in conjuction with v_pfend which together assume
	 * that v_wait and v_ready service procs in FIFO order.
         */
	curthread->t_polevel = polevel;
        v_wait(&scb_iowait(sidx));

        return VM_WAIT;
}

/*
 * NAME:        v_syncwait()
 *
 * FUNCTION:    sets the lanch.syncwait flag
 *	 	to block new transactions.
 *
 *
 * PARAMETERS:  none
 *
 * RETURN :     0	
 *
 */

v_syncwait()
{
	lanch.syncwait = 1;	
	return 0;
}
