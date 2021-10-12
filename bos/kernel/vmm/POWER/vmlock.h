/* @(#)96	1.17  src/bos/kernel/vmm/POWER/vmlock.h, sysvmm, bos41J, 145887.bos 3/3/95 09:20:09 */
#ifndef _h_VMLOCK
#define _h_VMLOCK

/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:
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

#include "mplock.h"

/*
 * lockwords. lockwords record the hardware locks granted
 * to a transaction. the structures allow efficient access
 * to locks on a page by virtual address (by hashing) and
 * to all locks held by a transaction (by list anchored
 * in its tblk). lockwords reside in the vmmdseg. they are
 * pageable.
 *
 * lockwords are also used to represent the fact a page was
 * written to disk prior to commit. this can occur for either
 * journalled or deferred-update segments. a tid of zero is
 * conventionally used to represent pages which are in extension
 * memory but for which there are no locks. 
 *
 * an array of lockwords is kept in the vmmdseg. the array is
 * placed on a PAGESIZE boundary and occupies the remainder of
 * the segment after the segment control blocks (see vmsys.h).
 * only the lockwords which fit entirely in a page are used.
 * i.e. the ones which straddle a page boundary are not put
 * on the free-list. this simplifies back-tracking code.
 */

struct lockword {
        int      next;          /* index next lock on hash chain or
                                   of next lockword on free list */
        int      tidnxt;        /* index next lockword of tid */
        unsigned flag :8;       /* flags */
        unsigned sid :24;       /* segment id */
        ushort   page;          /* page number */
        ushort   tid;           /* transaction id holding lock */
        ulong    bits;          /* lock bits */
        int      log;           /* log age of page */
	uint	 home;		/* home disk block number      */
	uint	 extmem;	/* temporary disk address      */
};

struct tblock 
{
	int logtid;     /* transaction id in log 		*/
	int next;       /* index first lockword of tid or	*/
      			/* of next tblk on free list. 		*/
	int tid;	/* transaction id			*/
	int flag;	/* transaction state			*/
	int cpn;	/* log page number of commit record     */
	int ceor;	/* commit eor				*/
	int cxor;	/* commit checksum			*/
	int csn;	/* commit sequence number or return code*/
      	int waitsid;    /* sid tid wants 			*/
	int waitline;   /* line number in sid tid wants 	*/
	int locker;     /* tid of holder of lock tid wants 	*/
	int lsidx;      /* logs index in scb table      	*/
	int logage;	/* log age of transaction 		*/
	int gcwait; 	/* group commit event list.  Ready      */
			/* transactions wait on this event for  */
			/* group commit completion.		*/
	struct thread 
	       *waitors;/* list head of tids waiting on tid 	*/
	struct tblock 
	       *cqnext; /* next tblk on commit queue		*/
};

/* defines for bits in lockword flag.
 */
#define WRITELOCK  1
#define FREELOCK   2

/*
 * lockanch is in vmmdseg and is page fixed. tblk structures
 * and tid register values are allocated when a process first
 * requests a lockwork in getlock or in commit code. the tid
 * value is the index of the tblk structure.
 */

#define NLOCKHASH       128
#define NUMTIDS         256

struct lockanch
{
        int    nexttid;          /* next tid value for logging */
        int    freetid;          /* index of a free tid structure */
        int    maxtid;           /* biggest tid ever used */
	struct lockword *lwptr;  /* begin of array of lock words */
        int    freelock;         /* index first free lock word */
	int    morelocks;	 /* addr next free page of lock words */
	int    syncwait;	 /* block new transactions if non-zero */	
	struct thread *tblkwait; /* waitors because of syncwait	*/
	struct thread *freewait; /* waitors on a free tblk */
#ifdef _VMM_MP_EFF
	Simple_lock _lock;	 /* lock word global lock */
#endif /* _VMM_MP_EFF */
	struct tblock tblk[NUMTIDS];
        int    lockhash[NLOCKHASH];  /* hash chain anchors */
};

/* transaction block flag */
#define GC_QUEUE        0x0001	/* tblk appears on ilogx group commit queue */
#define GC_READY        0x0002
#define GC_COMMIT       0x0004
#define GC_COMMITTED    0x0008
#define GC_WAIT         0x0010
#define GC_LEADER       0x0020
#define GC_ERROR        0x8000

#ifdef _VMM_MP_EFF
#define lw_lock		lanch._lock
#endif /* _VMM_MP_EFF */

/* compute the difference in bytes of logval from sync point
 */
#define logdiff(diff,logval,lsidx)                      \
        diff = (logval) - scb_logsync((lsidx));         \
        if (diff < 0)                                   \
                diff += scb_logsize((lsidx)) - 2*PSIZE;

#endif /* _h_VMLOCK */
