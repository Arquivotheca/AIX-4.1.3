static char sccsid[] = "@(#)78  1.43  src/bos/kernext/lvm/hd_strat.c, sysxlvm, bos411, 9428A410j 5/27/94 17:59:23";

/*
 * COMPONENT_NAME: (SYSXLVM) Logical Volume Manager Device Driver - 78
 *
 * FUNCTIONS: hd_strategy, hd_initiate, hd_reject, hd_quiescevg, hd_quiet,
 *	      hd_add2pool, hd_deallocpbuf, hd_numpbufs, hd_terminate, 
 *            hd_unblock, hd_quelb, hd_kdis_initmwc, hd_kdis_dswadd, 
 *            hd_kdis_chgqrm, hd_kproc
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

/*
 *
 *  hd_strat.c -- DASD device driver strategy routines.
 *
 *
 *	These functions process logical block requests for the DASD
 *	manager pseudo-device driver.  They belong to the bottom half
 *	of this device driver, whose structure is described below.
 *
 *	Each volume group has a separate device switch entry; its logical
 *	volumes are distinguished by their minor numbers.
 *
 *
 *  Function:
 *
 *	The routines in this source file deal only with logical requests.
 *	Translation of these requests to physical addresses, scheduling
 *	of mirrored operations, and bad block relocation are all handled
 *	at a lower level in the driver.
 *
 *	Logical operations to overlapping block ranges must complete in
 *	FIFO order.  These routines keep track of all outstanding requests
 *	using the work_Q for this logical volume.  Whenever a new request
 *	arrives, it blocks until all earlier conflicting requests
 *	have completed.  When each request finishes, those that blocked
 *	waiting for it will be scheduled.  Serializing requests at the
 *	logical layer makes things much simpler for the lower-level
 *	physical operations.  Bad block relocation and mirror retries
 *	can be scheduled without fear that an overlapping request has
 *	slipped into the queue out of order and changed the data.
 *
 *	This "exclusive lock" on the block range of each request is
 *	sufficient to ensure FIFO operation, but does not provide
 *	maximum overlap for concurrent reads.  This is not a problem,
 *	because most users of this service (file system, virtual memory
 *	manager, etc.) are combining concurrent reads of the same block
 *	at a higher level.  In practice, the opportunity to schedule
 *	overlapping reads concurrently will be rare, so the simplicity
 *	of exclusive locks for both reads and writes is attractive.
 *
 *
 *  Execution environment:
 *
 *	All these routines run on interrupt levels, so they are not
 *	permitted to page fault.
 *
 *	Much of this code runs within critical sections that are
 *	serialized with block I/O offlevel iodone() processing.
 *	Except where noted, individual functions either run on the
 *	iodone() interrupt level, or with its interrupt priority.
 *
 * 	NOTE:  hd_strategy can NOT be called at a higher interrupt
 *	 	priority level than INTIODONE.
 */

/*
 *			S T R U C T U R E   O V E R V I E W
 *
 *
 *	The bottom half of this device driver is structured in three
 *	"layers", corresponding to the following source files:
 *
 *	    strategy layer
 *		hd_strat.c  --	logical request validation, initiation
 *				and termination.  Serializes logical
 *				requests when their block ranges overlap.
 *
 *	    scheduler layer
 *		hd_sched.c  --	scheduling physical requests for logical
 *				operations.
 *		hd_mircach.c--	Handles the mirror write consistency cache.
 *		hd_vgsa.c   --	Handles updating the VGSA and the WHEEL
 *				
 *	    physical layer
 *		hd_phys.c   --	physical request startup and termination.
 *		hd_bbrel.c  --	Handles relocated blocks and relocates
 *				newly grown defects.
 *		hd_bbdir.c  --	Responsible for updating the bad block
 *				directories on the physical volumes.
 *
 *	Here is how a simple request flows through these layers:
 *
 *
 *   original	|	    -- DASD device driver --		|    device
 *   requestor	|    hd_strat.c	    hd_sched.c	   hd_phys.c	|    driver
 *   -----------+-----------------------------------------------+-----------
 *		|						|
 *	    ------->						|
 *		|   hd_strategy					|
 *		|	    ------->				|
 *		|		    hd_schedule			|
 *		|			    ------->		|
 *		|				    hd_begin	|
 *		|					    ------->
 *		|						|   device
 *		|						|   strategy
 *		|						|   routine
 *		|						|
 *		|						|	*
 *		|						|	*
 *		|						|	*
 *		|						|
 *		|						|	<-----
 *		|						|   device
 *		|						|   interrupt
 *		|						|   handler
 *		|						|
 *		|					    <-------
 *		|				    hd_end	|
 *		|			    <-------		|
 *		|		    hd_finished			|
 *		|	    <-------				|
 *		|   hd_terminate				|
 *	    <-------						|
 *    iodone	|						|
 *		|						|
 */

#include <sys/types.h>
#include <sys/intr.h>
#include <sys/errno.h>
#include <sys/sleep.h>
#include <sys/malloc.h>
#include <sys/pin.h>
#include <sys/signal.h>
#include <sys/hd_psn.h>
#include <sys/device.h>			/* defines for devsw routines */
#include <sys/sysconfig.h>		/* rc values for devsw routines */
#include <sys/dasd.h>
#include <sys/vgsa.h>
#include <sys/trchkid.h>
#include <sys/hd.h>
#include <sys/hd_config.h>
#include <sys/bbdir.h>

/* Used to generate a bit mask indicating which 4k segment(s) have requests
   out standing */
long signed_shftmsk = SIGNED_SHFTMSK;

/* ------------------------------------------------------------------------
 *
 *		L O G I C A L   R E Q U E S T   P R O C E S S I N G
 *
 * ------------------------------------------------------------------------
 */

/*
 *  NAME:	hd_strategy 
 * 
 *  FUNCTION:	Block i/o request interface
 *              This function processes the entire request list.  
 *              Request validation, logical-to-physical translation, 
 *              and mirror write consistency operations are performed
 *              while locked.  Calls to underlying device strategy
 *              functions are performed while unlocked.  This ensures
 *              that LVM does not unnecessarily serialize underlying
 *              i/o operations.
 *
 *  PARAMETERS:	req_list - list of i/o requests (buf structs)
 *
 *  RETURN VALUE: none
 *
 */
void  
hd_strategy(
register struct buf *req_list)
{
int  int_lvl;
struct buf  *ready_list=NULL;

    BUGLPR(debuglvl, BUGNTA, ("hd_strategy: req_list = 0x%x\n", req_list))
    
    int_lvl = disable_lock(INTIODONE, &glb_sched_intlock);
    ASSERT(int_lvl >= INTIODONE);	
    hd_initiate(req_list);
    hd_schedule(&ready_list);
    unlock_enable(int_lvl, &glb_sched_intlock);
    
    hd_start(ready_list);
}

/*
 *  NAME:	validate_req
 * 
 *  FUNCTION:	Validate an i/o request.  Determines whether the request 
 *              should be scheduled or returned.
 *
 *  PARAMETERS:	lb - logical buf struct 
 *              lv - logical volume of request
 *              vg - volume group of request
 *
 *  RETURN VALUE: 0 - schedule request
 *                1 - return request, do not schedule
 *
 */

/* validate_req() return codes */
#define SCHEDULE  0              /* request is valid and should be scheduled */
#define RETURN  1    /* request should be returned (not necessarily invalid) */ 

int  
validate_req(
struct buf  *lb, 
struct lvol  *lv,
struct volgrp  *vg)
{
register struct buf  *next;	/* next buf			*/
register ulong bct,		/* block count			   */
               bwp,		/* block within page		   */
               pwt,		/* page within logical track group */
               pctm1;		/* page count minus 1		   */
register int  p_no;		/* logical partition number	   */
register struct part  *part;	/* physical partition structure	   */
int  rc;


    /* VG must be active and LV must exist and be open */
    if ((vg->flags & VG_FORCEDOFF) || (lv == NULL) || 
        (lv->lv_status != LV_OPEN))
        return(invalid_req(lb, ENXIO));
    
    /* can't write read-only LVs */
    if (lv->lv_options&LV_RDONLY && !(lb->b_flags&B_READ))  
        return(invalid_req(lb, EROFS));
    
    /* Several things are implied with EOM checks
    *  1. Requests from file system/buffer cache are on
    *     8/4 block boundaries and are for 8/4 blocks.
    *  2. Requests from uphysio(character) are guaranteed
    *     to be aligned at a track group boundaries by 
    *     hd_mincnt().
    *
    * Therefore only the beginning block number must be checked
    * to ensure we are not starting at or past the end of the LV. */
    
    if ((ulong)lb->b_blkno >= lv->nblocks)  
        {
        if ((ulong)lb->b_blkno == lv->nblocks) 
            {
            lb->b_resid = lb->b_bcount;	/* Indicate no xfer */
    
            if (lb->b_flags & B_READ)
                lb->b_error = 0;
            else 
                {
                lb->b_flags |= B_ERROR;
                lb->b_error = ENXIO;
                }
            return(RETURN);
            }
        else  
            return(invalid_req(lb, ENXIO));
        }
    
    /* Check that the LP has at least one copy on a PV */
    p_no = BLK2PART(vg->partshift, lb->b_blkno);
    part = PARTITION(lv, p_no, PRIMMIRROR);
    if (part->pvol == NULL) 
        return(invalid_req(lb, ENXIO));
    
    /* SAFETY FEATURE: if minor=0 (special LV) and physical starting block=0,
       then don't allow writes below the reserved block on this PV */
    if ((minor(lb->b_dev) == 0) && (part->start == 0) && !(lb->b_flags & B_READ)) 
        if (((ulong)lb->b_blkno - PART2BLK(vg->partshift,p_no)) < PSN_NONRSRVD) 
            return(invalid_req(lb, ENXIO));
    
    /*
    *  compute mask of pages affected by this operation:
    * 	bwp:	offset of 1st block within its page
    * 	bct:	count of blocks transferred
    *	pctm1:	count of pages affected less 1
    *	pwt:	offset of 1st page within logical track group
    */
    bwp = lb->b_blkno & (BPPG-1);
    bct = BYTE2BLK(lb->b_bcount);
    pctm1 = BLK2PG(bwp + bct - 1);
    pwt = BLK2PG(lb->b_blkno) & (PGPTRK-1);
    lb->b_work = ((unsigned)(signed_shftmsk>>pctm1)) >> pwt;
    
    if (lb->b_options & (RESYNC_OP | MWC_RCV_OP)) 
        {
        if ((rc = validate_resync_req(p_no, part, lb, lv, vg)) != 0)
            return(invalid_req(lb, rc));
        }
    else 
        /* check for invalid length or track group misalignment */
        if ((pwt+pctm1 >= PGPTRK) || (lb->b_bcount & (DBSIZE-1))) 
            return(invalid_req(lb, EINVAL));
    return(SCHEDULE);
}


/*
 *  NAME:	hd_initiate
 * 
 *  FUNCTION:	Validate and enqueue a list of i/o requests.  Each request
 *              on the list is individually validated and either returned
 *              or enqueued on the pending_Q for subsequent scheduling.
 *
 *  PARAMETERS:	req_list - list of i/o requests
 *
 *  RETURN VALUE: 
 */
void 
hd_initiate(
struct buf  *req_list)
{
struct buf  *next_req;
struct lvol  *lv;
struct volgrp  *vg;

    BUGLPR(debuglvl, BUGNTA,
           ("hd_initiate: lb = 0x%x pending_Q.head = 0x%x .tail = 0x%x\n",
           req_list, pending_Q.head, pending_Q.tail))

    /* process each request in list */
    while (req_list)
        {
        BUGLPR(debuglvl, BUGGID, ("   initiating lb = 0x%x\n", req_list))

        TRCHKL5T(HKWD_KERN_LVMSIMP | hkwd_LVM_LSTART, req_list,
                 (req_list->b_options<<16 | (req_list->b_flags & 0xffff)),
                 req_list->b_dev, req_list->b_blkno, req_list->b_bcount);

        next_req = req_list->av_forw;
    
        /* locate the volume group and logical volume structures */
        devswqry(req_list->b_dev, NULL, (caddr_t *) &vg);
        lv = VG_DEV2LV(vg, req_list->b_dev);
    
        /* validate logical request */
        switch (validate_req(req_list, lv, vg))
            {
            case SCHEDULE:
                /* initialize resid to indicate no transfer yet */
                req_list->b_resid = req_list->b_bcount;

                /* block this request if overlapping request is outstanding */
                serialize_data_access(req_list, lv);
                if (req_list->b_error != ELBBLOCKED)
                    {
                    /* enqueue to schedule list */
                    req_list->b_error = 0;

                    req_list->av_forw = NULL;
                    if (!pending_Q.head)
                        pending_Q.head = req_list;
                    if (pending_Q.tail)
                        pending_Q.tail->av_forw = req_list;
                    pending_Q.tail = req_list;
                    }
            break;
    
            case RETURN:
                iodone(req_list);
            break;
    
            default:
                ASSERT(0);
            }
        req_list = next_req;
        }
}

/*
 *  NAME:	serialize_data_access
 * 
 *  FUNCTION:	Enforce serial access to data.  This function determines
 *              whether a new request overlaps an outstanding i/o request.
 *              If so, the request is marked ELBBLOCKED to block it.
 *              Granularity of overlap calculation is 4K.
 *
 *  PARAMETERS:	lb - logical request
 *              lv - logical volume of request
 *
 *  RETURN VALUE: none
 *
 */
void  
serialize_data_access(
struct buf  *lb, 
struct lvol  *lv)
{
register struct buf  **p;	/* pointer to next request */
register int  conflicts,	/* conflicts in this logical trk */
              hash;		/* hash class of this request	   */
register ulong  rltg;		/* request logical track group */

    /* search hash class for conflicting requests */
    conflicts = 0;

    hash = HD_HASH(lb);			/* work in progress hash class */
    rltg = BLK2TRK(lb->b_blkno);

    for (p = &(lv->work_Q[hash]); *p != NULL; p = &((*p)->av_back))
        if (BLK2TRK((*p)->b_blkno) == rltg)
            conflicts |= (*p)->b_work;	

    /* append buf to hash list */
    *p = lb;
    lb->av_back = NULL;
    
    /* is there a conflict? */
    if (conflicts & lb->b_work)
        {
        TRCHKL1T(HKWD_KERN_LVMSIMP | hkwd_LVM_RBLOCKED, lb);
        BUGLPR( debuglvl, BUGGID,
                ("   conflict block lb = 0x%x\n", lb))
        lb->b_error = ELBBLOCKED;/* lb is blocked	  */
        }
}

/*
 *  NAME:	validate_resync_req
 * 
 *  FUNCTION:	Validate resynchronization requests.  Resync requests are
 *              extended reads on an entire logical track group (128K) and
 *              must target the next LTG in the logical volume since the
 *              last successful resync request.  The containing physical
 *              partition must not have another resync ongoing.
 *
 *  PARAMETERS:	lb - logical request
 *              log_part_no - logical partition of request
 *              part - physical partition of request
 *             	lb - logical request
 *              lv - logical volume of request
 *              vg - volume group of request
 *
 *  RETURN VALUE: EINVAL - invalid resync request
 *                0 - valid resync request
 */
int  
validate_resync_req(
int log_part_no,
struct part  *part, 
struct buf  *lb, 
struct lvol  *lv, 
struct volgrp  *vg)
{
register int	trk_in_part;

    /* 
    * If this is a resync request there several more checks to
    * be made before we allow it to continue:
    *
    *   RESYNC_OP:  Part of a resync partition operation
    *	1. The request must start on block 0 of the LTG and
    *	   be for the entire length of the LTG.
    *	2. No other resync operation in the partition.
    *	3. This LTG must be one greater than the previous.
    *			or
    *	4. This LTG is the first in the partition.
    *
    *   MWC_RCV_OP:  Mirror Write Consistency recover operation
    *	1. The request must start on block 0 of the LTG and
    *	   be for the entire length of the LTG.
    */
    
    /* 1. The request must start on block 0 of the LTG and be for the 
       entire length of the LTG. */
    if ((lb->b_blkno&(BLKPTRK-1)) || (lb->b_bcount != BYTEPTRK)) 
        return(EINVAL);
    
    if (lb->b_options & RESYNC_OP) 
        {
        /*	2. No other resync operation in the partition.	*/
        if (part->ppstate & PP_RIP) 
            return(EINVAL);
    
        /*	3. This LTG must be one greater than the previous. */
        trk_in_part = TRK_IN_PART( vg->partshift, lb->b_blkno );
        if (trk_in_part == part->sync_trk) 
            part->ppstate |= PP_RIP;
    
        /*	4. This LTG is the first in the partition.	*/
        else 
            if (trk_in_part == 0) 
                {
                /* set RIP bit, set current sync_trk to 0, and reset all 
                   SYNCERR flags */
                part->ppstate |= PP_RIP;
                part->ppstate &= ~PP_SYNCERR;
                part->sync_trk = 0;
                if (lv->nparts > SINGMIRROR) 
                    {
                    part = PARTITION( lv, log_part_no, SINGMIRROR );
                    part->ppstate &= ~PP_SYNCERR;
                    }
                if (lv->nparts > DOUBMIRROR) 
                    {
                    part = PARTITION( lv, log_part_no, DOUBMIRROR );
                    part->ppstate &= ~PP_SYNCERR;
                    }
                }
            else 
                return(EINVAL);
        }
    return(0);
}


/*
 *  NAME:	invalid_req
 * 
 *  FUNCTION:	Process invalid request.  Set error flag, error number,
 *              residual byte count, and return value which indicates
 *              that the request should be returned.
 *
 *  PARAMETERS:	lb - logical request
 *              errno - error number to insert in request
 *
 *  RETURN VALUE: RETURN - indicates that request should be returned
 */
int  
invalid_req(
struct buf  *lb, 
int  errno)
{
    BUGLPR( debuglvl, BUGNTA, ("invalid_req: lb = 0x%x errno = 0x%x\n", lb, errno))
    
    TRCHKL4T(HKWD_KERN_LVMSIMP | hkwd_LVM_LEND, lb, lb->b_flags,
             lb->b_error, lb->b_resid);
    lb->b_flags |= B_ERROR;
    lb->b_error = errno;
    lb->b_resid = lb->b_bcount;	/* set residual count */
    return(RETURN);
}

/*
 *  NAME:         hd_quiet
 *
 *  FUNCTION:     Wait until all current I/O to this logical volume has
 *		  finished.
 *
 *  NOTES:        In a critical section, set the B_INFLIGHT bit on all
 *		  requests in progress for this LV, and then sleep until
 * 		  they are all completed.  Then clean up the cache.
 *
 *  PARAMETERS:   dev - dev_t of the logical volume
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 *
 */
void
hd_quiet(
dev_t		dev,	/* device number (major,minor) of LV to quiesce */
struct volgrp	*vg)	/* pointer from device switch table		*/
{

	register struct lvol *lv = VG_DEV2LV(vg, dev);
	register struct buf **p, **q;   /* pointers to bufs in work_Q	*/
	register struct buf **qlimit;	/* max size of the work_Q hash table */


	BUGLPR( debuglvl, BUGNTA,
	    ("hd_quiet: dev = 0x%x vg = 0x%x\n", dev, vg))

	/*
	 *  Wait for any other process that is waiting on
         *  I/O requests to complete.
	 */

	while (lv->complcnt > 0)
	    e_sleep_thread(&(lv->waitlist), &glb_sched_intlock, LOCK_HANDLER);

	/*
	 *  Go thru lv workQ and for each request in progress,
	 *  set the B_INFLIGHT bit in the buf struct & increment
	 *  the completion counter.
	 */

	qlimit = lv->work_Q + WORKQ_SIZE;	/* get max size of work_Q */

      	for (q = lv->work_Q; q < qlimit; q++)  {
        	for (p = q; *p != NULL; p = &((*p)->av_back))  {
			(*p)->b_flags |= B_INFLIGHT;
			lv->complcnt++;
		}
	}	
	/*  Wait for pending requests on this LV to complete. */
	while (lv->complcnt)
	    e_sleep_thread(&(lv->waitlist), &glb_sched_intlock, LOCK_HANDLER);
}

/*  
 *  NAME:         dl_quiet
 *
 *  FUNCTION:     Same as hd_quiet except executes while disable_locked.
 *
 *  RETURN VALUES: none
*/
void
dl_quiet(
dev_t		dev,	/* device number (major,minor) of LV to quiesce */
struct volgrp	*vg)	/* pointer from device switch table		*/
{
int  int_lvl;

    int_lvl = disable_lock(INTIODONE, &glb_sched_intlock);
    hd_quiet(dev, vg);		 /* wait for pending reqsts to finish */
    unlock_enable(int_lvl, &glb_sched_intlock);
}

/*
 *  NAME:         hd_add2pool
 *
 *  FUNCTION:     Add pbuf subpool, allocated in hd_open, to global
 *		  pbuf pool - hd_freebuf.
 *
 *		  Also link this subpool into the pbuf chain that is used
 *		  by dump and crash.
 *
 *  PARAMETERS:   subpool - pointer to allocated&pinned pbuf subpool
 *		  dmpbuf  - pointer into circular list in subpool
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 *
 */
void
hd_add2pool(
register struct pbuf *subpool,		/* ptr to pbuf sub pool */
register struct pbuf *dmpq)		/* ptr to pbuf dump queue */
{

    register struct pbuf *pb;	/* ptr to pbuf structure */
    register int int_lvl;		/* saved interrupt priority */

    BUGLPR( debuglvl, BUGNTA,
	("hd_add2pool: subpool = 0x%x dmpq = 0x%x\n", subpool, dmpq))

    BUGLPR( debuglvl, BUGGID,
	("   hd_freebuf = 0x%x hd_dmpbuf = 0x%x\n", hd_freebuf, hd_dmpbuf))

    int_lvl = disable_lock(INTIODONE, &glb_sched_intlock);

    if (hd_freebuf) {
      for(pb=hd_freebuf;pb->pb.av_forw!=NULL;pb=(struct pbuf *)pb->pb.av_forw)
			;		/* get to end of current list */
      pb->pb.av_forw = (struct buf *)subpool; /* set up link */
    }
    else
	hd_freebuf = subpool;

    /*
     * Add this subpool dump chain to global one so dump and crash can 
     * find them.
     */
    if( hd_dmpbuf ) {
	hd_dmpbuf->pb_back->pb_forw = dmpq;
	dmpq->pb_back->pb_forw = hd_dmpbuf;
	pb = hd_dmpbuf->pb_back;
	hd_dmpbuf->pb_back = dmpq->pb_back;
	dmpq->pb_back = pb;
    }
    else
	hd_dmpbuf = dmpq;

    unlock_enable(int_lvl, &glb_sched_intlock);
}

/*
 *  NAME:         hd_numpbufs 
 * 
 *  FUNCTION:     Calculate the number of pbufs that should be currently 
 *		  allocated given the number of active PVs in the
 *		  system.  This is a crude method but it does leave some
 * 		  room for tuning and prevents LVM from grabbing large
 *		  quantities of memory that it may not use.
 *  
 *  NOTES:	The formula for this calculation is quite simple.  For
 *		each PV that is active in a VG, allocate hd_pbuf_grab
 *		pbufs with the exception of the first PV it will allocate
 *		hd_pbuf_min pbufs.
 *
 *  PARAMETERS:
 *
 *  DATA STRUCTS: 
 *
 *  RETURN VALUE: total number of pbuf structs that should be allocated
 *
 */
int
hd_numpbufs(void)
{
	register int	numpbufs;

	BUGLPR( debuglvl, BUGNTA,
	    ("hd_numpbufs: hd_pvs_opn = 0x%x\n", hd_pvs_opn))

	/* If no active PVs then we should not have any pbufs	*/
	if( hd_pvs_opn == 0 )
	    return( 0 );

	numpbufs = ((hd_pvs_opn - 1) * hd_pbuf_grab) + hd_pbuf_min;

	return( numpbufs );
}
/*
 *  NAME:         hd_terminate
 *
 *  FUNCTION:     Process completed logical operation
 *
 *  NOTE:         The logical request is removed from the work_Q, its requestor
 *		  is notified, and any subsequent logical requests that were
 *		  blocked waiting on this one are added to the pending_Q.
 *
 *	     	  Someone must run hd_schedule() to redrive the pending_Q.
 *		  If this routine was called from within the main hd_schedule()
 *		  loop, all required cleanup will be done automatically, after
 *		  the return.
 *
 *  PARAMETERS:   lb - pointer to a logical buf request
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 *
 */
void
hd_terminate(
register struct buf *lb)		/* logical buf struct */
{
	register struct buf **p;
	register struct buf *next;
	register struct lvol *lv;
	register int hash;

	struct volgrp	*vg;		/* VG volgrp pointer		*/

	BUGLPR( debuglvl, BUGNTA,
	    ("hd_terminate: lb = 0x%x b_options = 0x%x b_blkno = 0x%x\n",
			lb, lb->b_options, lb->b_blkno))

	/* get VG volgrp structure ptr	*/
	(void) devswqry( lb->b_dev, NULL, (caddr_t *)&vg );
	lv = VG_DEV2LV( vg, lb->b_dev );

	/*
	 *  Remove it from the the work_Q if it is not a VGSA write
	 *  request.  If this is a VGSA write then the request was dropped
	 *  directly into hd_regular() thereby going around the work_Q.
	 */
	if( !(lb->b_options & REQ_VGSA) ) {

	    /*
	     * If INFLIGHT flag, used in hd_quiet, is set, reset it
             * and decrement completion counter.
	     */
	    if (lb->b_flags & B_INFLIGHT) {
		lb->b_flags &= ~B_INFLIGHT;
		lv->complcnt--;
	    }

	    /* unlink lb from work_Q chain */
	    hash = HD_HASH(lb);

            for (p = &(lv->work_Q[hash]); *p != lb; p = &((*p)->av_back))
		ASSERT(*p != NULL);
		
            (*p) = lb->av_back;


	    /*
	     *  Unblock any logical requests waiting on the one that finished.
	     *  Scan newer blocked requests on the same hash chain to see
	     *  if they can be unblocked.  Don't look at older requests,
	     *  they cannot have been waiting on this event.  If the VG
	     *  is closing then unlink any blocked requests from the work_Q
	     *  and error them off instead of trying to schedule them.
	     */

	    next = *p;
	    while( next != NULL ) {
		if( next->b_error == ELBBLOCKED ) {
		    if( vg->flags & VG_FORCEDOFF ) {
			/*
			 * if there is a close waiting then turn off
			 * the inflight flag and decrement the complete
			 * count
			 */
			if( next->b_flags & B_INFLIGHT ) {
			    next->b_flags &= ~B_INFLIGHT;
			    lv->complcnt--;
			}

			/*
			 * unlink lbuf from work_Q chain and set it for
			 * an error return
			 */
			(*p) = next->av_back;
			next->b_flags |= B_ERROR;
			next->b_error = EIO;
			next->b_resid = next->b_bcount;

			/*
			 * Emit a trace hook and give the request back
			 */
			TRCHKL4T(HKWD_KERN_LVMSIMP | hkwd_LVM_LEND, next,
			    next->b_flags, next->b_error, next->b_resid);

			iodone( next );
		    }
		    else {
			hd_unblock( lv->work_Q[hash], next );
			p = &(next->av_back);
		    }
		}
		else
		    p = &(next->av_back);

		next = *p;
	    }
	}

	/*
	 * This changed added so LVM will never return an ESOFT error
	 * to the originator of a request.  There are paths through
	 * LVM, such as reread of a known defective block that returns
	 * an ESOFT, where LVM will not complete the request but return 
	 * an ESOFT to this point.  The upper layers believe they know
	 * what an ESOFT is and return a good return code to the 
	 * application.  Under these conditions the user gets bad data
	 * but no error indication.
	 *
	 * When all these paths are cleaned up this test can be removed
	 */
	if( (lb->b_flags & B_ERROR) && (lb->b_error == ESOFT) ) {
		lb->b_error = EIO;
		lb->b_resid = lb->b_bcount;
	}

	TRCHKL4T(HKWD_KERN_LVMSIMP | hkwd_LVM_LEND, lb, lb->b_flags,
		 lb->b_error, lb->b_resid);

	/* notify caller of completion */
	iodone(lb);

	/*
	 *  Notify hd_close() if the last transfer just finished
	 *  OR notify any other routine waiting on certain requests
	 *  to complete.
	 */
	if( lv->waitlist != EVENT_NULL && lv->complcnt == 0 )
		e_wakeup(&lv->waitlist);

	return;
}


/*
 *  NAME:         hd_unblock
 *
 *  FUNCTION:     Try to unblock a logical request.
 *
 *  NOTES:        output - logical request added to pending_Q, if no
 *			longer blocked.
 *
 *  PARAMETERS:   next - first buf struct on hash chain that lb belongs to
 *                lb - logical device buf structure to unblock
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 *
 */
void
hd_unblock(
register struct buf *next,		/* first request on hash chain */
register struct buf *lb)		/* logical request to reschedule */
{
	register int conflicts = 0;

	BUGLPR( debuglvl, BUGNTA,
	    ("hd_unblock: next = 0x%x lb = 0x%x\n", next, lb))

	for (; next != lb; next = next->av_back) {
		if (BLK2TRK(next->b_blkno) == BLK2TRK(lb->b_blkno))
			conflicts |= next->b_work;
	}

	/* if no more conflicts, put on pending Q */
	if (!(conflicts & lb->b_work)) {
        	lb->b_error = 0; 		/* reinitialize b_error */
		hd_quelb( lb, &pending_Q );
	}
}

/*
 *  NAME:	hd_quelb
 *
 *  FUNCTION:	Put buf struct on que
 *
 *  NOTES:	Zero av_forw and put request on tail of given queue
 *
 *  PARAMETERS:	none
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 *
 */
void
hd_quelb(
register struct buf	 *lb,		/* current logical buf struct	*/
register struct hd_queue *que)		/* queue structure ptr		*/
{

    lb->av_forw = NULL;
    if( que->head == NULL )
	que->head = lb;
    else
	que->tail->av_forw = lb;

    que->tail = lb;

    return;
}

/*
 *  The following routines are called from routines in the hd_config.c
 *  module and are located here for the purpose of disabling interrupts
 *  for certain critical sections of code which are executed during
 *  configuration.
 */

/*
 *
 *  NAME:	hd_kdis_initmwc
 *
 *  FUNCTION:	initialize the MWCC on each active PV in the VG
 *
 *  NOTES:
 *
 *  PARAMETERS:	none
 *
 *  RETURN VALUE: none
 */
int
hd_kdis_initmwc(

struct volgrp  *vg)         	/* volume group pointer */

{
    register int rc, lrc;
    register int i_prty;		/* interrupt priority */
    dev_t device;			/* device number */
    struct file *fp;			/* file pointer */
    register int ca_idx;		/* cach index */
    register struct pvol *pv_ptr;	/* pv pointer */
    int  int_lvl;                       /* interrupt level */

    /* open LV0 so that the MWCC structures will be allocated */
    /* NOTE: OPEN MUST BE DONE BEFORE INTERRUPTS DISABLED!!!! */
    device = makedev(vg->major_num, 0);
    rc = fp_opendev(device, DREAD, 0, 0, &fp);
    if (rc != LVDD_SUCCESS)  
       return(rc);   

    int_lvl = disable_lock(INTIODONE, &glb_sched_intlock);

    /*
     * turn on the cache write flag for each PV so all will
     * be updated with the initialized MWCC
     */
    for( ca_idx=0; ca_idx < MAXPVS; ca_idx++ ) {
        pv_ptr = vg->pvols[ca_idx];
        if( pv_ptr && (pv_ptr->pvstate != PV_MISSING) )
	    SETPVWRT( vg, pv_ptr->pvnum );
    }

    /* write the MWCC to all PVs */
    CA_VG_WRT( vg )
    hd_ca_wrt();

    /* wait for ongoing cache writes to complete */
    e_sleep_thread(&(vg->ecachwait), &glb_sched_intlock, LOCK_HANDLER);

    /* turn off the inflight flag, close LV0 and return */
    vg->flags &= ~CA_INFLT;

    unlock_enable(int_lvl, &glb_sched_intlock);

    /* NOTE: CLOSE MUST BE DONE AFTER INTERRUPTS ENABLED !!!
       this close will also deallocate MWC memory resources */
    fp_close(fp);

    return(LVDD_SUCCESS);
}


/*
 *
 *  NAME:	hd_kdis_dswadd
 *
 *  FUNCTION:	Add the device switch table entry for the VG being defined.
 *
 *  NOTES:
 *
 *  PARAMETERS:	none
 *
 *  RETURN VALUE: none
 */
int
hd_kdis_dswadd(

register dev_t          device,         /* device number of the VG      */
register struct devsw   *devsw)         /* address of the devsw entry   */

{
    int status;                 /* device switch status */

    /*
     * Query the devsw table to see if there already is an entry for
     * the major number we have.  If so, return error; otherwise,
     * add this VG's entry to the devsw table.
     */
    if (devswqry(device, (uint *)&status, (caddr_t *)NULL) != CONF_SUCC)
        return ENXIO;

    if (status & DSW_DEFINED)
        return CFG_MAJUSED;             /* this major number in use */

    return devswadd(device, devsw) == CONF_SUCC ? LVDD_SUCCESS : ENXIO;
}

/*
 *  NAME:       hd_kdis_chgqrm
 *
 *  FUNCTION:   Check and change the VG's quorum count
 *
 *  PARAMETERS:	none
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 *
 */
int
hd_kdis_chgqrm(
struct volgrp 	*vg, 
short 		newqrm)
{


    register int act_cnt;		/* VG's active quorum count */
    register int rc=LVDD_SUCCESS;
    int  int_lvl;

    int_lvl = disable_lock(INTIODONE, &glb_sched_intlock);
    act_cnt = hd_sa_qrmchk(vg);
    if (act_cnt < newqrm)
	rc = CFG_BELOWQRM;
    else
	vg->quorum_cnt = newqrm;
    unlock_enable(int_lvl, &glb_sched_intlock);
    return(rc);
}




	
/*
 *  NAME:       hd_kproc
 *
 *  FUNCTION:   LVDD kernel process function
 *
 *  NOTES:	This function works off of a volgrp, or list of volgrp 
 *		structures pointed to by kpvg_ptr.
 *
 *  PARAMETERS:	none
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 *
 */
int
hd_kproc(void)
{
    register struct volgrp	*vg;	/* ptr to volgrp with work to do  */
    register struct bad_blk	*bad;	/* ptr to a new bad_blk struct	  */
    register ulong		action;	/* return from e_wait		  */
    int		sizebblk;		/* size of a bad_blk struct	  */
    int		sigrc;			/* signal number return		  */
    register unsigned int  free_bb_entries;     /* available bb dir entries */
    int  rc, int_lvl, lrc;

    BUGLPR( debuglvl, BUGNTA,
	("hd_kproc: LVM kernel process started.\n"))

    /* change parent from proc which invoked hd_open to the swapper so
       hd_kproc will not die when init issues kill to all procs during IPL */
    setpswap();

    /* set kernel process thread id for communication */
    simple_lock(&glb_kp_lock);
    lvm_kp_tid = thread_self();
    e_wakeup(&lvm_kp_wait);
    simple_unlock(&glb_kp_lock);

    sizebblk = sizeof( struct bad_blk );

    for( ;; ) {

	BUGLPR( debuglvl, BUGNTA, ("hd_kproc: LVM kernel process waiting\n"))

	/* The real thing surrounded by debug stuff */
	action = et_wait(LVDD_KP_ACTMSK, LVDD_KP_ACTMSK, EVENT_SHORT);

	BUGLPR( debuglvl, BUGNTA,
	    ("hd_kproc: LVM kernel process active - action = 0x%x\n", action))

	/* if this is a request to allocate more bad_blk structures */
	if( action & LVDD_KP_BADBLK ) {

	    /* lock this global list */
	    lrc = lockl(&hd_freebad_lk, LOCK_SHORT);

	    BUGLPR( debuglvl, BUGGID,
		("   hd_freebad = 0x%x hd_freebad_cnt = 0x%x\n",
					hd_freebad, hd_freebad_cnt))

	    /* malloc bad_blk structs until the count gets back to the 
               high water mark */
	    while( hd_freebad_cnt <= LVDD_HFREE_BB ) {
		bad = (struct bad_blk *)xmalloc( (uint)sizebblk, HD_ALIGN,
								pinned_heap );
		/* if xmalloc failure, just quit */
		if( bad == NULL )
			break;
	    	/*
		 * zero the new one, put it on the freebad chain, and
		 * bump the count
		 */
		bzero( bad, sizebblk );
		rel_bblk(bad);
	    }
	    /* unlock the list if we have the lock */
	    if (lrc == LOCK_SUCC)
		unlockl(&hd_freebad_lk);
	} 
        /* if servicing request to pin more bad block directory memory */
        if (action & LVDD_KP_BB_PIN)
            {
            /* calculate free bad block directory entries currently in system */
            free_bb_entries = ((pinned_bb_dir_memory -
                                (2 * sizeof(struct bb_hdr)))
                               / sizeof(struct bb_entry))
                              - max_bb_entries();

            /* if actually low on pinned bb dir memory, then pin more */
            if (free_bb_entries < LVDD_HFREE_BB)
                  {
                  /* if pinning another block would not exceed max size */
                  if ((pinned_bb_dir_memory + BB_DIR_PIN_INCRMNT)
                      <= UNPINNED_BBDIR_SIZE)
                      {
                      /* pin another block of bb dir memory */
                      rc = pin(bb_pbuf->pb_addr + pinned_bb_dir_memory,
                               BB_DIR_PIN_INCRMNT);
                      if (rc == 0)
                          pinned_bb_dir_memory += BB_DIR_PIN_INCRMNT;
                      else
                          hd_logerr((unsigned)ERRID_LVM_BBDIRFUL,
                                    (ulong)(bb_pbuf->pb.b_dev),
                                    (ulong)(rc), (ulong)pinned_bb_dir_memory);
                      }
                  else
                      hd_logerr((unsigned)ERRID_LVM_BBDIRFUL,
                                (ulong)(bb_pbuf->pb.b_dev),
                                (ulong)(rc), (ulong)pinned_bb_dir_memory);
                  }
            } 
    } /* END for( ;; ) */
}

/*
 *  NAME: max_bb_entries 
 * 
 *  FUNCTION: Calculate maximum of bad block directory entries in all
 *            active physical volumes in system.  This is used to reserve
 *            the appropriate amount of pinned bad block directory memory
 *            which is used to update the bad block directories on disks.
 *
 *  RETURN VALUE: Number of entries in largest bad block directory of all
 *                active disks in system.
 *
 */
unsigned int  
max_bb_entries()
{
struct volgrp  *vg_ptr;
struct pvol  *pv_ptr;
unsigned int  max = 0, i, lrc;

BUGLPR(debuglvl, BUGNTA, ("enter max_bb_entries\n"))

lrc = lockl (&(hd_vghead.lock), LOCK_SHORT);
vg_ptr = hd_vghead.ptr;

/* for each active VG in the system */
while (vg_ptr != NULL)
        {

        /* for each physical volume entry in this VG */
        for (i=0; i < MAXPVS; i++)
                {
                pv_ptr = vg_ptr->pvols[i];

                /* if physical volume exists */
                if (pv_ptr)
                        {
                        /* if disk is active */
                        if ((pv_ptr->pvstate != PV_MISSING) &&
                            (pv_ptr->pvstate != PV_RORELOC))
 
                                /* if number of bad block directory entries
                                   for this disk is greater than current max */
                                if (pv_ptr->num_bbdir_ent > max)
                                        max = pv_ptr->num_bbdir_ent;
                        }
                }
        vg_ptr = vg_ptr->nextvg;
        }
if (lrc == LOCK_SUCC)
    unlockl(&(hd_vghead.lock));

return(max);
}

/*
 *  NAME: get_bblk 
 * 
 *  FUNCTION: Take a free bad block structure from the system pool
 *            and return it to the caller.  Executes while disabled
 *            and holding the glb_sched_intlock to protect list integrity.
 *
 *  RETURN VALUE: pointer to free bad block entry
 */
struct bad_blk *
get_bblk()
{
int  int_lvl;
struct bad_blk  *bb_ptr;

    int_lvl = disable_lock(INTIODONE, &glb_sched_intlock);
    bb_ptr = hd_freebad;
    hd_freebad = hd_freebad->next;
    hd_freebad_cnt--;
    unlock_enable(int_lvl, &glb_sched_intlock);
    return(bb_ptr);
}

/*
 *  NAME: rel_bblk 
 * 
 *  FUNCTION: Return a free bad block structure to the system pool.
 *            Executes while disabled and holding the glb_sched_intlock 
 *            to protect list integrity.
 *
 *  RETURN VALUE: none
 */

void
rel_bblk(
struct bad_blk  *bb_ptr)	
{
int  int_lvl;

    int_lvl = disable_lock(INTIODONE, &glb_sched_intlock);
    bb_ptr->next = hd_freebad;
    hd_freebad = bb_ptr;
    hd_freebad_cnt++;
    unlock_enable(int_lvl, &glb_sched_intlock);
}

/* incr_pbuf_grab:  conditionally increment pbufs-per-disk value atomically 
	This function is used for the PBUFCNT ioctl. */

int
incr_pbuf_grab
(unsigned int  pbuf_cnt)
{
int  int_lvl, rc=0;

    int_lvl = disable_lock(INTIODONE, &glb_sched_intlock);
    if (pbuf_cnt > hd_pbuf_grab)
	hd_pbuf_grab = pbuf_cnt;
    else
        rc = EINVAL;
    unlock_enable(int_lvl, &glb_sched_intlock);
return(rc);
}
