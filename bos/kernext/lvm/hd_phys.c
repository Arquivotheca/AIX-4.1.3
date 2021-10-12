static char sccsid[] = "@(#)76	1.27.1.7  src/bos/kernext/lvm/hd_phys.c, sysxlvm, bos411, 9428A410j 5/23/94 10:06:50";

/*
 * COMPONENT_NAME: (SYSXLVM) Logical Volume Manager Device Driver - 76
 *
 * FUNCTIONS: hd_begin, hd_end, hd_resume, hd_ready, hd_start, hd_gettime
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
 *  hd_phys.c -- DASD device driver physical block request handler.
 *
 *
 *	These routines schedule block DASD requests for the DASD
 *	manager pseudo-device driver.  They are part of the bottom
 *	half of this device driver, which is structured in three
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
 *	See hd_strat.c for a description of how requests flow through
 *	these layers.
 *
 *  Function:
 *
 *	These routines handle physical request startup (hd_begin), and
 *	termination (hd_end).  The physical layer performs bad block relocation,
 *	when necessary, hiding the messy details from the other two layers.
 *
 *	Bad block processing requires multiple trips through the physical
 *	layer before the entire physical operation is finished.
 *	When an physical operation must be continued, a call to hd_resume()
 *	will initiate the next phase of the operation.
 *
 *	When the entire physical operation is completed, the HD_SCHED()
 *	macro is invoked.  This calls the appropriate scheduler policy routine 
 *	via the b_sched function pointer in the physical buf structure.
 *
 *
 *  Execution environment:
 *
 *	All these routines run on interrupt levels, so they are not
 *	permitted to page fault.  They run within critical sections that are
 *	serialized with block I/O offlevel iodone() processing.
 */

#include <sys/types.h>
#include <sys/intr.h>
#include <sys/errno.h>
#include <sys/trchkid.h>
#include <sys/errids.h>
#include <sys/lvmd.h>
#include <sys/dasd.h>
/*
 * This define controls how and where global variables are declared.
 * hd_phys.o will contain all the global variables declared for the
 * LVDD.  This define, LVDD_PHYS, should not be put in any other
 * LVDD source module.
 */
#define LVDD_PHYS
#include <sys/hd.h>

/* !!!!
 * !!!! Added to inhibit any relocations due to grown defects.
 * !!!! */
int hd_allow_reloc = 1;

#ifdef DEBUG
int allow_HW_rel = 0;
#define ASCHW	0x4857		/* 4857 = "HW" */
#endif

/* ------------------------------------------------------------------------
 *
 *		P H Y S I C A L   R E Q U E S T   P R O C E S S I N G
 *
 * ------------------------------------------------------------------------
 */


/*
 *  NAME:         hd_begin 
 * 
 *  FUNCTION:     Begin physical I/O operation.
 *
 *  NOTES:   	  input:  physical buf struct contents:
 *				pb_start = starting physical block address.
 *
 *		  output: if successful, physical request added to ready_list;
 *			  otherwise, calls scheduler's operation end routine
 *			  with simulated I/O error status.
 *
 *  PARAMETERS:   pb - physical buf struct 
 *                ready_list - ptr to head of ready_list being built
 *
 *
 *  DATA STRUCTS: 
 *
 *  RETURN VALUE: none
 *
 */
void  
hd_begin(
register struct pbuf *pb,
register struct pbuf  **ready_list,
register struct volgrp *vg)
{
	register struct buf *lb;        /* logical device buf struct	  */

	/* fill in physical request parameters from logical buf struct */
	lb = pb->pb_lbuf;

	BUGLPR( debuglvl, BUGNTA,
	    ("hd_begin: pb = 0x%x vg = 0x%x b_flags = 0x%x lb = 0x%x\n",
				pb,vg,pb->pb.b_flags,lb))

	if( !(pb->pb.b_flags & B_READ) ) {
	/*
	 * if request is a write then move the WRITEV bit
	 * from ext parameters and the write verify flag
	 * from the LV options into the pbuf b_options to be passed
	 * on down to the disks drivers.
	 */
	    pb->pb.b_options = (lb->b_options & WRITEV);
	    pb->pb.b_options |= ((VG_DEV2LV(vg,lb->b_dev)->lv_options) &
								LV_WRITEV);
	}
	else
	    pb->pb.b_options = 0;

	BUGLPR( debuglvl, BUGGID,
	    ("          pb->pb.b_options = 0x%x\n", pb->pb.b_options))

	pb->pb_addr = lb->b_baddr;
	pb->pb.b_bcount = lb->b_bcount;
	pb->orig_addr = lb->b_baddr;
        pb->orig_count = lb->b_bcount;
        pb->orig_bflags = pb->pb.b_flags;
	pb->pb.b_blkno = pb->pb_start;
	pb->pb.b_iodone = (void (*)()) hd_end;
	pb->pb.b_xmemd = lb->b_xmemd;		/* copy xmem structure */
        pb->pb.b_flags |= B_MPSAFE;

	pb->pb_swretry = 0;
	pb->pb_bbop = 0;
	pb->pb_bbstat = 0;
	pb->pb_type = 0;
	pb->pb_whl_stop = 0;
	/*
	 * This ptr not valid after this point.  NULL it out just in
	 * case someone tries to play with it.
	 */
	pb->pb_part = NULL;

	if( hd_chkblk(pb) == SUCCESS )	/* check for bad block relocation */
		hd_ready(pb, ready_list);
	else  {				/* incorrigible bad block? */
		pb->pb.b_flags |= B_ERROR;
		pb->pb.b_error = EMEDIA;	/* simulate a media error */
		HD_SCHED(pb);		/* inform scheduler */
	}
}


/*
 *  NAME:         hd_end 
 * 
 *  FUNCTION:     Physical operation iodone() routine
 *
 *  NOTES:	  This routine is scheduled to run as an offlevel interrupt 
 *	  	  handler when the physical device driver calls iodone().
 *		  input:  physical buf for a completed request; b_resid and
 *				b_error set by physical device driver if
 *		    		b_flags=B_ERROR.
 *				b_error = EIO - non-media I/O error
 *					  EMEDIA - newly grown media error
 *					  ESOFT - request succeeded, but needs 
 *						relocation (may not work next 
 *						time)
 *
 *  PARAMETERS:   pb - physical buf struct 
 *
 *  DATA STRUCTS: 
 *
 *  RETURN VALUE: none
 *
 */
void
hd_end(
register struct pbuf *pb)		/* physical device buf struct	 */
{
struct pbuf  *ready_list=NULL;
int  int_lvl;

/* serialize with scheduling and completion processing */
int_lvl = disable_lock(INTIODONE, &glb_sched_intlock);

#ifdef DEBUG
	/*
	 * if in debug mode and not allowing HW relocation and this
	 * request was orginally a HW relocation request, pb_hw_rel = HW
	 * then dummy up that this HW relocation failed.
	 */
	if( pb->pb_hw_reloc == ASCHW ) {
	    pb->pb.b_flags |= B_ERROR;
	    pb->pb.b_resid = DBSIZE;
	    pb->pb.b_error = EINVAL;
	}
#endif

	TRCHKL4T(HKWD_KERN_LVMSIMP | hkwd_LVM_PEND, pb, pb->pb.b_flags,
		 pb->pb.b_error, pb->pb.b_resid);

	BUGLPR( debuglvl, BUGNTA,
	    ("hd_end: pb = 0x%x b_flags = 0x%x b_error = 0x%x b_resid = 0x%x\n",
			pb,pb->pb.b_flags,pb->pb.b_error,pb->pb.b_resid))

	pb->pb_pvol->xfcnt--; 

	/* update buffer address by number of bytes processed */
	pb->pb_addr += pb->pb.b_bcount - pb->pb.b_resid;

	/* check for media surface error from the physical driver */
	if (pb->pb.b_flags & B_ERROR  &&
	    ((pb->pb.b_error == EMEDIA) || (pb->pb.b_error == ESOFT))) {
			/* residual must not be 0 if operation failed */
			assert(pb->pb.b_resid != 0);
			hd_badblk(pb); 
	}
	else 
	    /* Let the physical operation continue.  */
	    PB_CONT( pb )

	/* schedule any pending logical or physical requests to PVDDs */
	if (!(pb->pb.b_flags & B_MORE_DONE))
		hd_schedule(&ready_list);			

        unlock_enable(int_lvl, &glb_sched_intlock);
        hd_start(ready_list);
}


/*
 *  NAME:        hd_resume 
 * 
 *  FUNCTION:    Resume physical I/O operation.         
 *
 *  NOTES:     	 input:   physical buf struct contents:
 *		    pb_start = physical address translation for lb->b_blkno.
 *		    b_baddr = next address in buffer.
 *
 *		 output:  if successful, physical request is started;
 *		    otherwise, calls scheduler's operation end routine
 *		    with simulated I/O error status.
 *
 *  PARAMETERS:  pb - physical buf struct 
 *
 *  DATA STRUCTS: 
 *
 *  RETURN VALUE: none
 *
 */
void  
hd_resume(
register struct pbuf *pb)
{
	register struct buf *lb;	/* logical device buf struct */
	int bytes_done;			/* bytes already processed */

	/* compute next physical block number and remaining byte count */
        bytes_done = pb->pb_addr - pb->orig_addr;
        pb->pb.b_bcount = pb->orig_count - bytes_done;
	pb->pb.b_blkno = pb->pb_start + BYTE2BLK(bytes_done);

	BUGLPR( debuglvl, BUGNTA,
	    ("hd_resume: pb = 0x%x b_bcount = 0x%x b_blkno = 0x%x\n",
			pb,pb->pb.b_bcount,pb->pb.b_blkno))

	if( hd_chkblk(pb) == SUCCESS )	/* check for bad block relocation */
		hd_start(pb);
	else  {				/* incorrigible bad block? */
		pb->pb.b_flags |= B_ERROR;
		pb->pb.b_error = EMEDIA;	/* simulate a media error */
		HD_SCHED(pb);		/* inform scheduler */
	}
}


/*
 *  NAME:         hd_ready 
 * 
 *  FUNCTION:     Insert physical request into the ready list.
 *
 *  NOTE:         The ready list is kept sorted by device and block number.
 *
 *  PARAMETERS:   pb - physical buf struct 
 *                ready_list - physical request list being built
 *
 *  DATA STRUCTS: 
 *
 *  RETURN VALUE: none
 *
 */
void  
hd_ready(
register struct pbuf *pb, 
register struct pbuf **ready_list)
{
	register struct buf *imb = &(pb->pb);   /* embedded buf struct */
	register struct buf **p; 		/* previous pointer address */

	BUGLPR( debuglvl, BUGNTA,
	    ("hd_ready: pb = 0x%x *ready_list = 0x%x\n", pb, *ready_list))
	BUGLPR( debuglvl, BUGGID,
	    ("           b_flags = 0x%x b_blkno = 0x%x\n",
					pb->pb.b_flags, pb->pb.b_blkno))

#ifdef DEBUG
	/*
	 * if in debug mode and not allowing HW relocation then
	 * turn it off but remember this was a HW relocation
	 * request.  Then when the request comes back dummy
	 * up that it failed.
	 */
	if( (pb->pb.b_options & HWRELOC) && !(allow_HW_rel) ) {
	    pb->pb.b_options &= ~HWRELOC;
	    pb->pb_hw_reloc = ASCHW;
	}
	else 
	    pb->pb_hw_reloc = 0;
#endif

	TRCHKL5T(HKWD_KERN_LVMSIMP | hkwd_LVM_PSTART, pb, pb->pb_lbuf,
		 (pb->pb.b_options<<16 | (pb->pb.b_flags & 0xffff)),
		  pb->pb.b_blkno, pb->pb.b_dev);

	/* adding to empty list is quick and easy... */
	if (*ready_list == NULL)  
                {
		imb->av_forw = NULL;
		*ready_list = (struct pbuf*) imb;
		return;
		}

	/* sort request into non-empty list */
	for (p = (struct buf **) ready_list; *p != NULL; p = &((*p)->av_forw))  
		{
		if ((*p)->b_dev > imb->b_dev ||
		    (((*p)->b_dev==imb->b_dev) && 
                    (*p)->b_blkno>imb->b_blkno))
			break;		/* insert in front of *p entry */
		}
	imb->av_forw = *p;		/* insert pb in the chain */
	*p = imb;			/* ...at the place where p points */
}

/*
 *  NAME:         hd_start
 *
 *  FUNCTION:     Start list of physical I/O requests.
 *
 *  NOTES:
 *	input:	ready_list:  physical device buf structures sorted by the
 *			scheduler.  All requests to a single physical
 *			device are adjacent in this list.
 *	output:	For each unique dev_t which has ready physical requests,
 *			the physical driver's strategy routine is called
 *			once with all the requests for that drive in its
 *			argument list.
 *
 *  PARAMETERS:   ready_list - list of physical requests to start
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 *
 */

void  
hd_start(
struct pbuf *ready_list)
{
    /* these are really pbuf's, but we only use the imbedded buf here */
    register struct buf *rdy;	/* ready requests for this drive */
    register struct buf *pb;	/* current imbedded log buf */
    register struct pbuf *p;	/* current physical buf */
    register struct buf *next;	/* next physical buf in the queue */
    register int rc;		/* return code */

    BUGLPR(debuglvl, BUGNTA, ("hd_start: ready_list = 0x%x\n", ready_list))

    if (ready_list) {
	next = rdy = (struct buf *) ready_list;
	for (;;)  {			/* for each ready request */
		pb = next;

                /* clear B_DONE so iodone() will not get upset */
                pb->b_flags &= ~B_DONE;

		next = pb->av_forw;
		p = (struct pbuf *)pb;

		/* increment physical volume transaction counter */
                if (p->pb.b_iodone == hd_end) 
		    p->pb_pvol->xfcnt++;

		/* end of ready_list reached? */
		if (next == NULL)  {
			rc = devstrat(rdy);
			ASSERT(rc == 0);
			break;		/* all started */
		}

		/* next request for a different device? */
		if (next->b_dev != rdy->b_dev)  {
			pb->av_forw = NULL;
			rc = devstrat(rdy);
			ASSERT(rc == 0);
			rdy = next;	/* go on to next device */
		}
	}
    }
}

/*
 *  NAME:         hd_gettime
 *
 *  FUNCTION:     Get the current time
 *
 *  NOTES:
 *
 *  PARAMETERS:   none
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 *
 */
void
hd_gettime(
register struct timestruc_t *o_time)	/* old time			*/
{
    struct timestruc_t t;		/* hold new current time	*/

    /*
     * Get the current time
     */
    curtime( &t );

    /*
     * Check that the new time is greater than the old time.  If not
     * then bump old time low order.  If it wraps then bump high order
     */
    if( t.tv_sec <= o_time->tv_sec )
	if( t.tv_nsec <= o_time->tv_nsec ) {
	    o_time->tv_nsec++;
	    if( o_time->tv_nsec <= 0 ) {
		o_time->tv_sec++;
		o_time->tv_nsec = 0;
	    }
	    return;
	}

    *o_time = t;
    return;
}
