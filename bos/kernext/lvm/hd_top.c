static char sccsid[] = "@(#)79	1.37.3.1  src/bos/kernext/lvm/hd_top.c, sysxlvm, bos41J, 9524D_all 6/8/95 14:01:48";

/*
 * COMPONENT_NAME: (SYSXLVM) Logical Volume Manager Device Driver - 79
 *
 * FUNCTIONS: hd_open, hd_close, hd_read, hd_write, hd_ioctl, hd_allocpbuf,
 *	      hd_frefrebb, hd_openbkout, hd_backout,
 *	      hd_allocbblk, hd_mincnt, hd_pbufdmpq, hd_alloca, hd_dealloca
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
 *  hd_top.c -- top half of Logical Volume Manager device driver.
 *
 *	This is the top half of the AIX pseudo-device driver for the 
 *	LVM component, which implements logical DASD volumes.  
 *	The term 'DASD' is IBM-ese for Direct Access Storage Device, a
 *	useful concept that includes such randomly-accessible devices as
 *	disks, drums, RAM disks, and nonvolatile block memories.
 *
 *	This component provides character and block entry points 
 *	just like a real disk driver, with compatible arguments.
 *
 *	Logical volumes differ from real devices in several respects:
 *
 *	    1.	Several can share a physical device.
 *	    2.	A single logical volume may span physical devices.
 *	    3.	They may grow dynamically.
 *	    4.	They can be mirrored for availability.
 *	    5.	Known bad blocks are relocated when possible,
 *		providing the appearance of contiguous defect-free
 *		block addresses.
 *
 *	Each volume group has a separate device switch entry; its logical
 *	volumes are distinguished by their minor numbers.
 *
 *
 *  Function:
 *
 *	This program handles both block and character requests, but
 *	most of the block read/write code is in the bottom half,
 *	which runs fully pinned, and has no access to user process context.
 *	Character reads and writes are transformed into block requests.
 *	See hd_strat.c for an overview of the structure
 *	of the bottom half.
 *
 *
 *  Execution environment:
 *
 *	The bottom half of the device driver, hd_strat.c,hd_sched.c,hd_phys.c, 
 *	runs on interrupt levels, and is not permitted to page fault.
 *	The top half of the device driver, hd_top.c, hd_config.c,
 *	driver runs in the context of a process address space, and can
 *	page fault.
 *
 */

#include <sys/types.h>
#include <sys/intr.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/devinfo.h>
#include <sys/lockl.h>
#include <sys/pin.h>
#include <sys/sleep.h>
#include <sys/malloc.h>
#include <sys/uio.h>
#include <sys/device.h>
#include <sys/hd_psn.h>
#include <sys/dasd.h>
#include <sys/trchkid.h>
#include <sys/hd.h>
#include <sys/hd_config.h>
#include <sys/vgsa.h>
#include <sys/bbdir.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#include <sys/systemcfg.h>

#define TRUE		1
#define FALSE		0

/* defines for hd_open & hd_backout */
#define PVBB		1
#define PWORKQ		2

/* defines for hd_open & hd_openbkout */
enum {
BO_MWC_P1=1,    /* deallocate MWC part 1 resource */
BO_MWC_P2,      /* free MWC part 2 resource */
BO_MWC_PVW,     /* free pv_wait queue memory */
BO_ALL          /* remove VG from dump table */
};

/*
 *  NAME:         hd_open 
 * 
 *  FUNCTION:     open logical volume
 *  
 *  PARAMETERS:   dev - dev_t (major,minor) of logical volume to be opened
 *		  flags - specifies if device being opened for read or write
 *		  chan,ext - not used              
 *
 *  DATA STRUCTS: 
 *
 *  NOTES:	  If the dev_t passed in points to a valid logical volume,
 * 		  allocate it's work_Q.  If 1st open for the VG,
 *		  allocate a buffer of disk bad block directory image.
 *
 *  RETURN VALUE: errno returned in return code
 *
 */
int
hd_open(

dev_t dev,	/* device number (major,minor) of LV to be opened */
int flags, 	/* read/write flag	*/
int chan,	/* not used		*/
int ext)	/* not used		*/

{
    register struct lvol	*lv;	/* pointer to lvol struct	*/
    register int		lrc;	/* return code from lockl	*/
    register int	 	cnt,i;	/* pbuf pool counter		*/
    register int	 	size;	/* size to allocate		*/
    register int		rc;	/* general return code		*/
    struct volgrp 		*vg;	/* pointer to volgrp struct	*/
    pid_t			kp_pid; /* to access kproc TID 		*/

	BUGLPR( debuglvl, BUGNFO,
		("hd_open: opening dev 0x%x flags 0x%x\n", dev,flags) )

	TRCHKL2T(HKWD_KERN_LVM | hkwd_LVM_OPEN, dev, flags);

	(void) devswqry(dev,(uint *)NULL,(caddr_t *)&vg);/* get volgrp pointer*/
	lrc = lockl(&(vg->vg_lock), LOCK_SHORT); /* get volgrp struct lock */

	/*
         * verify that the VG is not in the process of being varied off.  We
	 * may have just been awakened due to the lockl.
	 */
	if( vg->flags & VG_FORCEDOFF ) {
		if (lrc == LOCK_SUCC) 	/* if no nested locks,unlock vg */
			unlockl(&(vg->vg_lock));
		return(ENXIO);
	}	

	/*
         * If we are varying on the VG, then only allow the varyon process
	 * to open LVs.
	 */
	if( (vg->flags & VG_OPENING) && (getpid() != vg->von_pid) ) {
		if (lrc == LOCK_SUCC) 	/* if no nested locks,unlock vg */
			unlockl(&(vg->vg_lock));
		return(ENXIO);
	}	

	lv = VG_DEV2LV(vg,dev);             /* to get lvol struct */

	/*
         * verify that the logical volume is defined to the LVM
	 */
	if (lv == NULL) {
		if (lrc == LOCK_SUCC) 	/* if no nested locks,unlock vg */
			unlockl(&(vg->vg_lock));
		return(ENXIO);   /* no such LV in VG */
	}	
	
	/* 
         * If logical volume is read only and the flags indicate the open
	 * is for appending or writing then error off. 
         */
	if ((flags & (DWRITE | DAPPEND)) && (lv->lv_options & LV_RDONLY)) {
		if (lrc == LOCK_SUCC) 	/* if no nested locks,unlock vg */
			unlockl(&(vg->vg_lock));
		return(EROFS);   /* Read only device */
	}	

        /* if VG has no open logical volumes */
	if (vg->open_count == 0) {

		if (hd_vgs_opn == 0)
			{
			if (init_global_locks)
                            {
			    init_global_locks = 0;

			    /* allocate/initialize global MP locks */
			    /* global scheduling lock */
			    lock_alloc(&glb_sched_intlock, LOCK_ALLOC_PIN, 
                                       LVM_LOCK_CLASS, 0);
			    simple_lock_init(&glb_sched_intlock);
                            /* global kernel proc lock */
			    lock_alloc(&glb_kp_lock, LOCK_ALLOC_PIN, 
                                       LVM_LOCK_CLASS, 1);
			    simple_lock_init(&glb_kp_lock);
                            }

                        /* if LVM kernel process does not exist, create it */
			simple_lock(&glb_kp_lock);
                        if (lvm_kp_tid == 0) 
                            {
                            if ((kp_pid = creatp()) != -1) 
                                rc = initp(kp_pid, hd_kproc, NULL, 0, "lvmbb");
                            if ((kp_pid == -1) || (rc != 0))
                                {
                                /* back out this open */
                                lvm_kp_tid = 0;
                                hd_backout( PVBB,(struct lvol *)NULL, vg );
                                if (lrc == LOCK_SUCC)
                                    unlockl(&(vg->vg_lock));
                                return((kp_pid == -1) ? EAGAIN : rc);
                                }
                            /* wait for kproc to store its thread id */
                            e_sleep_thread(&lvm_kp_wait, &glb_kp_lock, 
					   LOCK_SIMPLE);
                            }
			simple_unlock(&glb_kp_lock);
			}

		/* Increase the number of PVs in use in the system by the 
		   number in this VG so the correct number of pbufs will be 
		   allocated. */
		for(i = 0; i < MAXPVS; i++) 
			if (vg->pvols[i]) 
				hd_pvs_opn++;

		/* initialize these reserved variables if this is the 
		   first open of a VG in the system */
		if (!hd_vgs_opn)
			for (i=0; i < VGS_CA; i++) {
				ca_alloced[i] = 0;
				ca_grp_ptr[i] = NULL;
			}

		/* Go get some pbufs to access these new PVs */
		if( hd_allocpbuf() == LVDD_ERROR ) {
			/* Decrease the number of PVs in use in the system 
			   by the number in this VG */
			for (i=0; i < MAXPVS; i++) 
				if (vg->pvols[i])
					hd_pvs_opn--;
			if (lrc == LOCK_SUCC)
				unlockl(&(vg->vg_lock));
			return(ENOMEM);
		}
		
		/* If this is the first open of the first VG then 
		   allocate a pbuf and a data buffer for a disk image 
		   of BB directory for updating bad block directories.  */
		if (hd_vgs_opn == 0) 
			if ((rc = hd_init_bbdir()) != 0) {
				if (lrc == LOCK_SUCC)
					unlockl(&(vg->vg_lock));
				return(rc);
		}

		hd_vgs_opn++;	/* increment count of open VGs */

		/* if 1st open in VG, add to list of VGs for lvm dump */
		hd_dumpvglist( vg ); 
	}
        /* if opening an LV which is mirrored and does not have MWC turned 
           off, and its VG does not have MWC memory already allocated to it, 
           then allocate MWC memory for the VG */
	if ((lv->nparts > 1) &&
	    !(lv->lv_options & LV_NOMWC) && 
	    (vg->mwc_rec == NULL))
		if ((rc = alloc_mwc_mem(vg)) != LVDD_SUCCESS) {
			if (lrc == LOCK_SUCC)
				unlockl(&(vg->vg_lock));
			return(rc);
		}

        /* allocate bad block structures */
	if( hd_allocbblk() == LVDD_ERROR ) {
	    hd_backout( PVBB, (struct lvol *)NULL, vg );
	    if (lrc == LOCK_SUCC)
		unlockl(&(vg->vg_lock));
   	    return(ENOMEM);
	}

	/* if 1st open for this LV alloc work_Q	set lvol open flag */
 	if (lv->lv_status != LV_OPEN) {
		size = sizeof(struct buf *) * WORKQ_SIZE;
		lv->work_Q = (struct buf **)xmalloc( (uint)size,
						HD_ALIGN,pinned_heap);
		if (lv->work_Q == NULL) 
		{
			/* malloc failed, backout the malloc */
			hd_backout( PVBB, lv, vg );
			if (lrc == LOCK_SUCC) /*if no nested locks,unlock vg*/
				unlockl(&(vg->vg_lock));
   	   		return(ENOMEM);
		}
		bzero(lv->work_Q, sizeof(struct buf *)*WORKQ_SIZE);

		/*
		 * set LV status to open, kick the open count in the
		 * VG, and set bit indicating LV is open.
		 */
		hd_lvs_opn++;
		lv->lv_status = LV_OPEN;
		vg->open_count++;
		SETLVOPN( vg, minor(dev) );
	}			/* first open for this LV */
			
	if (lrc == LOCK_SUCC)			/* unlock volgrp structures */
		unlockl(&(vg->vg_lock));

	return( LVDD_SUCCESS );
}

/*
 *  NAME:         hd_allocpbuf 
 * 
 *  FUNCTION:     Allocate physical buffer (struct pbuf) structures to
 *                correspond to the number of active physical disks
 *                in the system.  pbuf count should equal 48 + (16 *
 *                # active disks in system) but does not shrink when
 *                disks go offline.  Total coalesced memory to allocate
 *                is broken into blocks sized at powers of 2 to reduce
 *                excess memory allocated by xmalloc but not used.
 *  
 *  PARAMETERS:   None.
 *
 *  RETURN VALUE: Successful completion returns LVDD_SUCCESS.
 *                Failures return error code returned by hd_allocpbufblk.
 */

int  
hd_allocpbuf(void)
{
unsigned int  pbufs_to_alloc, unit_size = sizeof(struct pbuf), powerof2;
int  status = LVDD_SUCCESS;

BUGLPR(debuglvl, BUGNTA, ("enter hd_allocpbuf\n"))

/* determine number of pbufs required for current number of active disks */
pbufs_to_alloc = hd_numpbufs();

/* if pbuf count is currently low, allocate more */
if (hd_pbuf_cnt < pbufs_to_alloc) 
    {
    /* subtract pbufs already in system from those needed by current system */
    pbufs_to_alloc -= hd_pbuf_cnt;

    /* while still more pbufs to allocate and allocation hasn't failed yet */
    while ((pbufs_to_alloc != 0) && (status == LVDD_SUCCESS))
        {
        /* calculate highest power of 2 lower than total memory required
           by block of coalesced pbufs remaining to be allocated */
        powerof2 = 16;
        while ((powerof2 * 2) <= (pbufs_to_alloc * unit_size))
            powerof2 *= 2;

        /* if next power wastes less than one pbuf, or this power is less 
           than one pbuf in size, or remaining pbuf block is a power of 2 
           itself, then increase to next power */
        if ((((powerof2 * 2) - (pbufs_to_alloc * unit_size)) < (unit_size))
            || (powerof2 < unit_size)
            || ((powerof2 / unit_size) == (pbufs_to_alloc / 2)))
            powerof2 *= 2;

        /* allocate block of pbufs */
        status = hd_allocpbufblk(powerof2 / unit_size);

        /* adjust number of pbufs remaining to be allocated */
        if ((powerof2 / unit_size) < pbufs_to_alloc)
            pbufs_to_alloc -= powerof2 / unit_size;
        else
            pbufs_to_alloc = 0;

        }
    }
return(status);
}

/*
 *  NAME:	hd_allocpbufblk
 *
 *  FUNCTION:	Allocate, logically subdivide, and integrate into
 *              system pbuf resource a block of pbuf structures 
 *
 *  PARAMETERS: 
 *      cnt: count of pbuf structures to block-allocate 
 *			
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: LVDD_SUCCESS or LVDD_FAILURE
 *
 */
int  
hd_allocpbufblk(
register int  cnt)
{
register struct pbuf  *pb;          /* pointer to pbuf struct  */
register struct pbuf  *pbsubpool;   /* ptr to beg of pbuf subpool*/
register int  	poolcnt;            /* count of pbufs in subpool */
struct pbuf  *dmpq;                 /* anchor for subpool for dump */

BUGLPR(debuglvl, BUGNTA, ("enter hd_allocpbufblk\n"))

/* allocate pbuf subpool for this LV & add it to central free list 
   (hd_freebuf).  This is done if the high water mark has not already 
   been reached.  If the maximum number of pbufs have been allocated 
   then none are added to the pool.  Also build the dump pointer lists.  
   This is so the allocated pbufs can be found by dump and crash. */

pbsubpool = NULL;
dmpq = NULL;
poolcnt = 0;

BUGLPR( debuglvl, BUGNTA,
       ("hd_allocpbufblk: cnt = 0x%x hd_pbuf_cnt = 0x%x\n", cnt, hd_pbuf_cnt))

/* allocate block of pbufs */
pb = (struct pbuf *) xmalloc((uint) (cnt * sizeof(struct pbuf)),
                             HD_ALIGN,pinned_heap);

if (pb == NULL)
  {
  assert( xmfree(pb, pinned_heap) == 0 );
  return( LVDD_ERROR );
  }

/* logically subdivide the block into pbufs and link them together */
while( poolcnt < cnt )
  {
  bzero(pb, sizeof(struct pbuf));	/* zero out the pbuf */

  /* set up links in pbuf chain */
  pb->pb.av_forw = (struct buf *)pbsubpool;
  pb->pb.b_event = EVENT_NULL;	/* init event field */
  pbsubpool = pb;

  /* add new pbuf to dump queue chain */
  hd_pbufdmpq( pb, &dmpq );
  poolcnt++;	/* Add to number of pbufs allocated */
  pb++;
  }   /* end alloc for loop */
	 
/* now add pbuf subpool to central pbuf pool for global use and add them 
   to the dump list */
hd_add2pool(pbsubpool, dmpq);
hd_pbuf_cnt += poolcnt;
return( LVDD_SUCCESS );
}

/*
 *  NAME:         hd_pbufdmpq
 *
 *  FUNCTION:     Append pbuf to dump queue.
 *
 *  NOTES:	  Append pb to a doubly-linked circular queue
 *  		  containing all the pbufs allocated to this queue.
 *
 *  PARAMETERS:   pb - physical buf struct
 *                qq - pointer to circualr list of pbuf's
 *			
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 *
 */
void
hd_pbufdmpq(
register struct pbuf *pb,		/* new pbuf for chain */
register struct pbuf **qq)		/* Ptr to queue anchor */
{
	register struct pbuf *q;	/* queue anchor */

	BUGLPR(debuglvl, BUGNTA, ("enter hd_pbufdmpq\n"))

	q = *qq;
	if (q == NULL)  {	/* first request */
		pb->pb_forw = pb;
		pb->pb_back = pb;
		*qq = pb;
	}
	else  {
		pb->pb_forw = q->pb_back->pb_forw;
		q->pb_back->pb_forw = pb;
		pb->pb_back = q->pb_back;
		q->pb_back = pb;
	}
}

/*
 *  NAME:	hd_openbkout 
 * 
 *  FUNCTION:	backout free all structures owned by the VG.
 *		When open encounters an error during the VG setup when
 *		the first LV of the VG is opened.
 *  
 *  NOTES:	Also used by hd_close via hd_backout when the last LV
 *		of a VG is closed.
 *
 *  PARAMETERS:	bopoint	- point in hd_open where error occurred & need 
 *			  to backout from or point to start on close.
 *		vg	- ptr to volgrp structure that needs the work
 *
 *  DATA STRUCTS: 
 *
 *  RETURN VALUE: none
 */
void
hd_openbkout(
int		bopoint,	/* point to start backing out		*/
struct volgrp	*vg)		/* struct volgrp ptr			*/
{

    BUGLPR( debuglvl, BUGNTA,
	("hd_openbkout: bopoint = 0x%x vg = 0x%x\n", bopoint, vg))

    /*
     * Starting at back out point fall through each case and do the
     * correct thing for each level
     */
    switch( bopoint ) {
	case BO_ALL:
	    hd_nodumpvg( vg );		/* remove VG from dump table	  */

            /* if VG has no MWC resources allocated, then stop here */
            if (vg->mwc_rec == NULL)
                break;

	case BO_MWC_PVW:		/* free pv_wait queue memory	  */
	    assert( xmfree( vg->ca_pvwmem, pinned_heap ) == 0 );
	    vg->ca_pvwmem = NULL;
	    vg->ca_freepvw = NULL;

	case BO_MWC_P2:			/* free MWC part2 buffer	  */
	    assert( xmfree( vg->ca_part2, pinned_heap ) == 0 );
	    vg->ca_part2 = NULL;
	    vg->ca_lst = NULL;

	case BO_MWC_P1:			/* free MWC part1 buffer	  */
	    hd_dealloca( vg->mwc_rec );
	    vg->mwc_rec = NULL;
    }

    return;
}
/*
 *  NAME: deallocate_mwc_try
 * 
 *  FUNCTION: Scan all open logical volumes in all this volume
 *            group to see if any of them need the mirror write
 *            consistency memory (a per VG resource).  If not,
 *            deallocate it.  A VG needs MWC iff it has at least 
 *            one LV for which all of the following conditions are 
 *            true:  1) it is open, 2) it is mirrored, 3) it has
 *            mirror write consistency turned on.
 *  
 *  PARAMETERS:	volume group
 *
 *  RETURN VALUE: none
 *
 */
void  
deallocate_mwc_try(
struct volgrp  *vg)
{
int  i, need_mwc;

/* if VG has mwc memory allocated to it */
if (vg->mwc_rec != NULL) 
    {
    need_mwc = FALSE;

    /* determine if any open LV in VG needs MWC */
    i = 0;
    while ((i < MAXLVS) && (need_mwc == FALSE))
        {
	if (vg->lvols[i] != NULL)
            if (((vg->lvols[i])->lv_status == LV_OPEN) &&
                ((vg->lvols[i])->nparts > 1) &&
                !((vg->lvols[i])->lv_options & LV_NOMWC))
                need_mwc = TRUE;
	i++;
        }

    if (need_mwc == FALSE)
        hd_openbkout(BO_MWC_PVW, vg);
    }
}

/*
 *  NAME:         hd_backout 
 * 
 *  FUNCTION:     clean up work_Q and VG structures if an error occurred
 *		  in hd_open.  Also used by hd_close when an LV is closed.
 *  
 *  PARAMETERS:	bopoint	- point in hd_open where error occurred & need 
 *			  to backout from or point to start on close.
 *		lv	- ptr to lvol to backout
 *		vg	- ptr to volgrp containing lv
 *
 *  DATA STRUCTS: 
 *
 *  RETURN VALUE: none
 *
 */
void
hd_backout(

int		bopoint,	/* backout point			*/
struct lvol	*lv,		/* ptr to lvol to backout		*/
struct volgrp	*vg)		/* struct volgrp ptr			*/
{
	register int	i;	/* counter for pvol/lvol index		*/
        struct lvol  *lv_ptr;


	BUGLPR( debuglvl, BUGNTA,
	   ("hd_backout: bopoint = 0x%x lv = 0x%x vg = 0x%x\n",
		bopoint, lv, vg))

	switch (bopoint) {

	    case PWORKQ:	/* unalloc the work_Q struct */

		assert( xmfree(lv->work_Q, pinned_heap) == 0 );
		lv->work_Q = NULL;
		/* do not break */

	    case PVBB:		/* deallocate pbufs if no open LVs */

		if (vg->open_count == 0) {

		    /*
		     * Decrease the number of PVs in use in the system by the
		     * number in this VG so the correct number of pbufs will be
		     * deallocated.
		     */
		    for( i=0; i < MAXPVS; i++ ) {
			if( vg->pvols[i] )
			    hd_pvs_opn--;
		    }

		    /* clean up structures associated with volgrp */
		    hd_openbkout( BO_ALL, vg );
		    hd_vgs_opn--;	/* decrement count of open VGs */

		    /*
		     * If there are no VGs online then release resources held
		     * in common for all VGs.
		     *    . bad block directory buffer
		     *    . bad block directory pbuf
		     *    . free bad_blk structs
		     *	  . kernel process
		     */
		    if( hd_vgs_opn == 0 ) {

                        /* directory buffer */
                        assert( xmdetach(&(bb_pbuf->pb.b_xmemd)) == XMEM_SUCC );

			if (pinned_bb_dir_memory)
				unpin(bb_pbuf->pb_addr, pinned_bb_dir_memory);

                        assert( xmfree( bb_pbuf->pb_addr, kernel_heap ) == 0 );

			/* directory pbuf */

			assert( xmfree( bb_pbuf, pinned_heap ) == 0 );
			bb_pbuf = NULL;

			/* bad_blk structs and kernel process */
			hd_frefrebb();
		    }
		}
                /* try to deallocate MWC memory for this VG */
                deallocate_mwc_try(vg);
		break;

	    default:
		break;
	    }
	return;
}		


/*
 *  NAME:         hd_close 
 * 
 *  FUNCTION:     close logical volume
 *  
 *  PARAMETERS:   dev - dev_t (major,minor) of logical volume to be closed
 *		  chan,ext - not used
 *
 *  DATA STRUCTS: 
 *
 *  RETURN VALUE: ERRNO returned in return code
 *
 */
int
hd_close(
dev_t	dev,	/* device number (major,minor) of LV to be closed */
int	chan,	/* not used */
int	ext)	/* not used */
{
	register struct lvol *lv;	/* lvol pointer		  */
	register int lrc, lrc_head;	/* return code from lockl */
	register int rc;		/* return code		  */
	register int i_prty;		/* saved interrupt priority	*/
  	struct volgrp   *vg;		/* volgrp pointer	  */

	TRCHKL1T(HKWD_KERN_LVM | hkwd_LVM_CLOSE, dev);

	BUGLPR( debuglvl, BUGNFO, ("hd_close: dev = 0x%x\n", dev))

	(void) devswqry(dev,(uint *)NULL,(caddr_t *)&vg); /*get volgrp pointer*/

        /* lock the list of active VGs and search for this entry */
        lrc_head = lockl (&(hd_vghead.lock), LOCK_SHORT);

	lrc = lockl(&(vg->vg_lock), LOCK_SHORT); /* get volume group lock */

	lv = VG_DEV2LV(vg,dev);		   /* get lvol pointer */

	/*
	 * The following code is written with these assumptions.
	 * 
	 * hd_close is called:
	 *	1. The LV is being closed due to a process closing it and
	 *	   it is the last close of the LV.
	 *
	 *	2. A LV open failed and hd_close is being called because the
	 *	   the file system open count is 0.  In this case there maybe
	 *	   no LV really open at this major/minor number.
	 */
	if( vg->flags & VG_FORCEDOFF ) {

	    if( (lv == NULL) || (lv->lv_status == LV_CLOSED) ) {
		if( vg->open_count == 0 ) {
		    /*
		     * Attempt to remove the driver from the device switch table
		     * if it succeeds then free the volgrp structure
		     * otherwise just return.
		     */
		    rc = devswdel( dev );

		    if( rc == LVDD_SUCCESS ) {
                        lock_free(&(vg->sa_intlock));
			assert( xmfree(vg, pinned_heap) == LVDD_SUCCESS );
			assert( unpincode((int(*) ())hd_upd_bbdir)==LVDD_SUCCESS);
		    }
		}
		if (lrc == LOCK_SUCC) /* unlock volume group structures */
			unlockl(&(vg->vg_lock));

                if (lrc_head == LOCK_SUCC)    /* unlock global VG list */
                    unlockl(&(hd_vghead.lock));
		return( LVDD_SUCCESS );	/* return ok			    */
	    }
	}

	/* verify that the logical volume is defined to the LVM */
	if (lv == NULL) {
	    if (lrc == LOCK_SUCC) /* unlock volume group structures */
			unlockl(&(vg->vg_lock));
            if (lrc_head == LOCK_SUCC)    /* unlock global VG list */
                unlockl(&(hd_vghead.lock));
	    return(ENXIO);
        }

	/*
	 *  Wait until all current I/O to this logical volume has
	 *  finished,
	 */

	if (lv->lv_status == LV_CLOSED) {
		/* unlock volume group structures */
		if (lrc == LOCK_SUCC)
			unlockl(&(vg->vg_lock));
                if (lrc_head == LOCK_SUCC)    /* unlock global VG list */
                    unlockl(&(hd_vghead.lock));
		return( LVDD_SUCCESS );
	}
	assert(lv->lv_status == LV_OPEN);/* LV should never be CLOSING state */
	lv->lv_status = LV_CLOSING;	 /* LV status is OPEN-set to CLOSING */
	dl_quiet(dev, vg);
	lv->lv_status = LV_CLOSED;

	vg->open_count--;		/* Decrement open LV count in VG    */
	hd_lvs_opn--;

	/* Reset open and pinned flag */
	CLRLVOPN( vg, minor(dev) );

	/*
	 * If this is a normal close then go clean up the
	 * mirror write consistency cache
	 */
	if( !(vg->flags & VG_FORCEDOFF) && (vg->mwc_rec != NULL))
	    hd_ca_clnup( vg );

	/*
	 * Release resources, by calling backout. 
	 */
	hd_backout( PWORKQ, lv, vg );

	/*
	 * If the VG is being forced off and this close is closing the
	 * last open LV in the VG then free all memory resources associated
	 * with this VG.
	 */
	if( (vg->flags & VG_FORCEDOFF) && (vg->open_count == 0) ) {
	    hd_vgcleanup( vg );
	    /*
	     * Attempt to remove the driver from the device switch table
	     * if it succeeds then free the volgrp structure
	     * otherwise just return.
	     */
	    rc = devswdel( dev );

	    if (lrc == LOCK_SUCC) /* unlock volume group structures */
		unlockl(&(vg->vg_lock));

	    if( rc == LVDD_SUCCESS ) {
                lock_free(&(vg->sa_intlock));
		assert( xmfree(vg, pinned_heap) == LVDD_SUCCESS );
		assert( unpincode((int(*) ())hd_upd_bbdir)==LVDD_SUCCESS);
	    }
	}
	else {
	    /* all done, unlock volume group structures */
	    if (lrc == LOCK_SUCC)
		unlockl(&(vg->vg_lock));
	}
        if (lrc_head == LOCK_SUCC)    /* unlock global VG list */
            unlockl(&(hd_vghead.lock));
	return( LVDD_SUCCESS );
}
	 
/*
 *  NAME:	hd_frefrebb 
 * 
 *  FUNCTION:	free the hd_freebad list of bad_blk structs
 *  
 *  PARAMETERS:
 *
 *  DATA STRUCTS: 
 *
 *  NOTE:
 *
 *  RETURN VALUE: none
 *
 */
void
hd_frefrebb( void )
{
    register struct bad_blk *bad;	/* ptr to bad block struct	*/

    register int	sizebblk;	/* size of bad_blk struct	*/
    register int  lrc;			/* return code from lockl */

    lrc = lockl(&hd_freebad_lk, LOCK_SHORT); /* lock this global list */

    BUGLPR( debuglvl, BUGNTA,
	("hd_frefrebb: hd_freebad = 0x%x hd_freebad_cnt = 0x%x\n",
			hd_freebad, hd_freebad_cnt))

    sizebblk = sizeof(struct bad_blk);

    /* free the bad_blk structs on the freebad chain */
    while( hd_freebad != NULL ) {
	bad = get_bblk();
	assert( xmfree(bad, pinned_heap) == 0 );
    }
    hd_freebad_cnt = 0;

    if (lrc == LOCK_SUCC)
	unlockl(&hd_freebad_lk);	/* Unlock the global list	*/
}
 
/*
 *  NAME:	hd_vgcleanup 
 * 
 *  FUNCTION:	Called by the varyoff config routine or by hd_close it
 *		if the VG is being forced off and the last LV was just
 *		closed.
 * 		
 *  PARAMETERS:	vg	- ptr to volgrp to be cleaned up
 *
 *  DATA STRUCTS: 
 *
 *  RETURN VALUE: none
 *
 */
void
hd_vgcleanup(
struct volgrp	*vg)		/* struct volgrp ptr			*/
{
    struct volgrp *curvg_ptr;	/* current VG struct in list */
    struct volgrp *prevvg_ptr;	/* previous VG struct in list */
    struct pvol *pv;		/* PV struct */
    int hidx;			/* defect dir hash index */
    struct bad_blk *bb_ptr;	/* bad block struct */
    struct bad_blk *save_ptr;	/* tmp bad block pointer */
    int i;	

    BUGLPR(debuglvl, BUGNTA, ("enter hd_vgcleanup\n"))

    hd_free_lvols(vg, HD_RSRVDALV, MAXLVS);		/* free ALL lvols */
	
    /* free all pvol and defects directories */
    for (i = 0; i < MAXPVS; i++) {
	pv = vg->pvols[i];
	if (pv != NULL) {
	    /* if this PV device is opened, close it */
	    if (pv->fp) {
		fp_close (pv->fp);
		pv->fp = NULL;
	    }

	    /* If PV has a bad block table, go thru the hash chains,
	     * delete each bad block struct and delete the hash 
	     * anchor table.
	     */
	    if (pv->defect_tbl) {
		for (hidx=0; hidx<HASHSIZE; hidx++) {
		    bb_ptr = pv->defect_tbl->defects [hidx];
		    while (bb_ptr) {
			 save_ptr = bb_ptr;
			 bb_ptr = bb_ptr->next;
			 assert(xmfree((caddr_t) save_ptr,
				 pinned_heap) == LVDD_SUCCESS);
		    } 
		} 
		assert(xmfree ((caddr_t)pv-> defect_tbl,
				 pinned_heap) == LVDD_SUCCESS);
	    } 
	    assert(xmfree((caddr_t) pv, pinned_heap) == LVDD_SUCCESS);
	} 
    }

    /* free the vgsa area for this VG */
    if (vg->mwc_rec != NULL)
      assert(xmfree((caddr_t) vg->vgsa_ptr, pinned_heap) == LVDD_SUCCESS);

    curvg_ptr = hd_vghead.ptr;		
    prevvg_ptr = NULL;		
    while (curvg_ptr) {
	if (vg == curvg_ptr)	/* if found it, break */
	    break;
	prevvg_ptr = curvg_ptr;	
	curvg_ptr = curvg_ptr->nextvg;	
    }

    /* 
     * remove this VG entry from the linked list (if in middle of 
     * list, lock the previous VG struct during pointer manipulation)
     */
    if (prevvg_ptr == NULL)		/* if its at the head of list */
	hd_vghead.ptr = curvg_ptr->nextvg;
    else
	prevvg_ptr->nextvg = curvg_ptr->nextvg;
}

/*
 *  NAME:	hd_allocbblk
 *
 *  FUNCTION:	Allocate bad_blk structures on the first open of an
 *		LV.
 *
 *  NOTES:
 *
 *  PARAMETERS:
 *			
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: LVDD_SUCCESS or LVDD_FAILURE
 *
 */
int
hd_allocbblk( void )
{
    register struct bad_blk *bad;	/* ptr to bad block struct	*/
    register struct bad_blk *lbad;	/* last ptr to BB struct	*/
    register int	sizebblk;	/* size of bad_blk struct	*/
    register int	bb;		/* bad_blk structure counter	*/
    register int  lrc;			/* return code from lockl */

    lrc = lockl( &hd_freebad_lk, LOCK_SHORT ); /* lock this global list */

    BUGLPR( debuglvl, BUGNTA,
	("hd_allocbblk: allocating bad_blk structs - hd_freebad_cnt = 0x%x\n",
		hd_freebad_cnt))

    sizebblk = sizeof(struct bad_blk);

    /* allocate bad_blk structures */
    while( hd_freebad_cnt < LVDD_HFREE_BB ) 
        {
	bad = (struct bad_blk *)xmalloc( (uint)sizebblk, HD_ALIGN,
								pinned_heap );
	/* if xmalloc fails, undo what has been done so far */
	if( bad == NULL ) 
            {
            unlockl( &hd_freebad_lk );	/* Unlock the global list*/
	    return (LVDD_ERROR);
            }

	/* zero the new one and put it on the end of the free chain */
	bzero( bad, sizebblk );
	rel_bblk( bad );
        }
    if (lrc == LOCK_SUCC)
	unlockl( &hd_freebad_lk );	/* Unlock the global list	*/

    return( LVDD_SUCCESS );
}

/*
 *  NAME:         hd_read 
 * 
 *  FUNCTION:     raw device read entry point
 *  
 *  PARAMETERS:   dev - dev_t (major,minor) of logical volume requested.       
 *		  uiop -pointer to uio structure that specifies location &
 *			length of caller's data buffer.
 *		  chan -not used.
 * 		  ext - I/O extension word (from readx() system call). The
 * 		        fields within this word are defined in <sys/lvdd.h>.
 *
 *  DATA STRUCTS: 
 *
 *  NOTE: 	  calls uphysio to allocate lvm_bufcnt buf structs to use
 *			for queuing multiple requests to hd_strategy
 *			until all of request is completed.
 *			
 *  RETURN VALUE: If error, return ERRNO value in return code 
 *
 */

int
hd_read(
dev_t	   dev,		/* device number (major,minor) of LV to be read	*/
struct uio *uiop,	/* pointer to uio structure that specifies	*/
			/* location & length of caller's data buffer.	*/
int	   chan,	/* not used */
int	   ext)		/* extension parameter				*/
{
	int rc;
	unsigned int  iov, bufcnt;

	BUGLPR( debuglvl, BUGNTA,
	    ("hd_read: dev = 0x%x uiop = 0x%x ext = 0x%x\n", dev, uiop, ext))

	TRCHKL2T(HKWD_KERN_LVM | hkwd_LVM_READ, dev, ext);

	/* Let's do some parameter validation before going to far */

	/*
	 * Can't do WRITEV, HWRELOC, UNSAFEREL or NO_MWC on reads
	 */
	if( ext & (WRITEV | HWRELOC | UNSAFEREL | NO_MWC) ) 
		return( EINVAL );
	/*
	 * No other parameters allowed with RESYNC_OP or MWC_RCV_OP
	 */
	if( (ext & RESYNC_OP) && (ext & ~RESYNC_OP) ) 
		return( EINVAL );

	if( (ext & MWC_RCV_OP) && (ext & ~MWC_RCV_OP) ) 
		return( EINVAL );


	/* RS hardware needs IOCC cache consistency protection for i/o 
	   reads from different adapters to adjacent memory which is not
	   aligned on a cache line boundary (64-bytes) */
	bufcnt = lvm_bufcnt;
	if (__power_rs())
            for (iov = 0; iov < uiop->uio_iovcnt; iov++)

		/* if any target base address is not aligned */
		if ((unsigned int) uiop->uio_iov[iov].iov_base & 0x3f)
		    {
		    /* serialize raw i/o requests to avoid data corruption */
		    bufcnt = 1;
		    break;
		    }

	/* call uphysio to allocate buf structs, pin the user buffer,
	   and send request(s) to hd_strategy. */
	rc=uphysio(uiop,B_READ,bufcnt,dev,hd_strategy,hd_mincnt, (void *)ext);
	return(rc);

}
	
/*
 *  NAME:         hd_write 
 * 
 *  FUNCTION:     raw device write entry point
 *  
 *  PARAMETERS:   dev - dev_t (major,minor) of logical volume to be written
 *		  uiop -pointer to uio structure that specifies location &
 *			length of caller's data buffer.
 *		  chan -not used.
 * 		  ext - I/O extension word (from write() system call). The
 * 		        fields within this word are defined in <sys/dasd.h>.
 *
 *  DATA STRUCTS: 
 *
 *  NOTE: 	  calls uphysio to allocate lvm_bufcnt buf structs to use
 *			for queuing multiple requests to hd_strategy
 *			until all of request is completed.
 *
 *  RETURN VALUE: If error, return ERRNO value in return code 
 *
 */
int
hd_write(
dev_t	   dev,		/* device number (major,minor) of LV to be written*/
struct uio *uiop,	/* pointer to uio structure that specifies	*/
			/* location & length of caller's data buffer.	*/
int	   chan,	/* not used */
int	   ext)		/* extension parameter				*/
{
	int rc;

	BUGLPR( debuglvl, BUGNTA,
	    ("hd_write: dev = 0x%x uiop = 0x%x ext = 0x%x\n", dev, uiop, ext))

	TRCHKL2T(HKWD_KERN_LVM | hkwd_LVM_WRITE, dev, ext);

	/* Let's do some parameter validation before going to far */

	/*
	 * Can't avoid mirrors, do resyncs, or hardware rolocates on writes
	 */
	if( ext & (AVOID_MSK | RESYNC_OP | MWC_RCV_OP | HWRELOC | UNSAFEREL) ) 
		return( EINVAL );

	/* call uphysio to allocate buf structs, pin the user buffer,
	 * and send request(s) to hd_strategy.  	
	 */
	rc=uphysio(uiop,B_WRITE,lvm_bufcnt,dev,hd_strategy,hd_mincnt,(void *)ext);
	return(rc);

}


	
/*
 *  NAME:         hd_mincnt   
 * 
 *  FUNCTION:     called by uphysio to split up request to logical track
 *			group (128K) size, if needed.  Also fills in 
 *		        the b_options field with minparms value sent to
 *			uphysio by hd_read/hd_write.
 *  
 *  PARAMETERS:   bp - pointer to buf struct to be checked            
 *		  minparms - pointer to ext value sent to uphysio by 
 *			hd_read/hd_write.
 *
 *  DATA STRUCTS: 
 *
 *  RETURN VALUE: EINVAL - request is not a multiple of blocks
 *
 */
hd_mincnt(
struct buf	*bp,		/* ptr to buf struct to be checked	*/
void		*minparms)	/* ptr to ext value sent to uphysio by	*/
				/* hd_read/hd_write.			*/
{
	register int rc;		/* return code */
	register int blkcnt;		/* block count */
	register int blkwt;		/* block within LTG */

	BUGLPR( debuglvl, BUGNTA,
	    ("hd_mincnt: bp = 0x%x minparms = 0x%x\n", bp, minparms))

	rc=0;

	/* set b_options field with minparm parameter (ext value sent by
	 * hd_read/hd_write routines)
	 */
	bp->b_options = (int) minparms;

	/* error if request not a multiple of blocks */          
	if (bp->b_bcount & DBSIZE-1)
		rc = EINVAL;

	/* request CANNOT cross Logical Track Group boundaries - break up
	 * request if it does.
	 */

	else {

		/* count of blocks transfered */
		blkcnt = BYTE2BLK(bp->b_bcount);
		/* offset of 1st blk within LTG */
		blkwt = (bp->b_blkno & BLKPTRK-1);

		/* if request crosses LTG, change bcount to have only
		 * the amount in the first LTG afftected by this request 
		 */
		if ((blkwt + (blkcnt-1)) >= BLKPTRK) 
			bp->b_bcount = BLK2BYTE(BLKPTRK-blkwt);	
	}

	return(rc);
	
}

/*
 *  NAME:         hd_ioctl 
 * 
 *  FUNCTION:     ioctl entry point                    
 *			IOCINFO:  return size of logical volume:
 *					CYL=partition
 *					TRK=logical track group
 *					SECT=block
 *
 * 			XLATE:    translate logical address to physical address
 *  
 *			GETVGSA:  get a copy of the memory version of the
 *				  VGSA
 *
 *  PARAMETERS:   dev - dev_t (major,minor) of logical volume to be used
 * 		  cmd - specific ioctl command to be performed
 * 	 	  arg - address of parameter block for the specific ioctl cmd
 * 		  mode- request origination
 *		  chan,ext - not used
 *
 *  DATA STRUCTS: 
 *
 *  RETURN VALUE: ERRNO returned in return code
 *
 */

int
hd_ioctl(
dev_t	dev,		/* device number (major,minor) of LV to be opened */
int	cmd,		/* specific ioctl command to be performed	*/
int	arg, 	 	/* addr of parameter blk for the specific cmd	*/
int	mode,		/* request origination				*/
int	chan,		/* not used */
int	ext)		/* not used */
{

	register struct lvol	*lv;	/* LV pointer			*/
	register int		p_no;	/* physical partition number	*/
	register struct part	*part;	/* part structure ptr		*/
	register int		rc = 0;
        register int		i;	/* loop counter                 */
    	register int		lrc;	/* return code from lockl */
	
	struct xlate_arg a;	/* kernel xlate arg struct to return	*/
	struct devinfo info;	/* device information			*/
        struct lpview lpinfo;	/* kernel lp_lview arg struct to return */
	struct volgrp *vg;	/* ptr from device switch table		*/
	void  *dummy;
	unsigned int  pbuf_cnt;

	BUGLPR( debuglvl, BUGNTA,
	    ("hd_ioctl: dev = 0x%x cmd = 0x%x arg = 0x%x mode = 0x%x\n",
			dev, cmd, arg, mode))

	TRCHKL3T(HKWD_KERN_LVM | hkwd_LVM_IOCTL, dev, cmd, arg);

	(void) devswqry(dev,(uint *)NULL,(caddr_t *)&vg); /*get volgrp pointer*/
	lv = VG_DEV2LV( vg, dev );		 /* get LV lvol ptr	*/

	lrc = lockl(&(vg->vg_lock), LOCK_SHORT); /* get volgrp struct lock */

	/*
         * verify that the logical volume is defined to the LVM
	 */
	if (lv == NULL)
		rc = ENXIO;   /* no such LV in VG */
        else
	  switch(cmd) {
	
	    case IOCINFO:		/* return info on LV */
		info.devtype = DD_DISK;		/* disk device type	  */
		info.flags = DF_RAND;		/* random access device	  */
		info.devsubtype = DS_LV;	/* logical volume subtype */
		info.un.dk.bytpsec = DBSIZE;	/* bytes per block	  */
		info.un.dk.secptrk = PGPTRK*BPPG;	/* blocks per LTG */
		/* LTGs per partition */
		info.un.dk.trkpcyl = TRKPPART(vg->partshift);
		info.un.dk.numblks = lv->nblocks;/* logical blocks this LV */
		/* copy struct back to caller area */
		if( mode & DKERNEL )
		    bcopy((char *)(&info),(char *)arg,sizeof(struct devinfo));
		else {
		    if (copyout(&info, (char *)arg, sizeof(struct devinfo)))
			rc = EFAULT;
		}
		break;

	    case XLATE:		/* translate L addr to P addr */

		/* copy caller info to our area */
		if( mode & DKERNEL )
		    bcopy((char *)arg,(char *)(&a),sizeof(struct xlate_arg));
		else {
		    if( copyin((char *)arg, &a, sizeof(struct xlate_arg))) {
			rc = EFAULT;
			break;
		    }
		}

		/* verify the lbn is within the LV */
		if( a.lbn >= lv->nblocks ) {
			rc = ENXIO;
			break;
		}
		/* verify the mirror parameter */
		if( (a.mirror > lv->nparts) || (a.mirror <= 0) ) {
			rc = ENXIO;
			break;
		}

		/* Return the physical volume dev_t and the physical block 
		   number where the logical block resides. */

		/* translate striped LVs using actual stripe xlate function */
		if (lv->i_sched == SCH_STRIPED) {
			/* xlate logical address to physical device address */
			rc = hd_stripe_xlate(vg->partshift+DBSHIFT, 
					     lv->stripe_exp,
					     lv->striping_width, 
					     lv->parts[0],
					     a.lbn, 
					     &(a.p_devt),
					     &(a.pbn),
					     (struct part *) &dummy,
					     (struct pvol *) &dummy);
			rc = (rc == SUCCESS) ? 0 : ENXIO; 
		}
		else {
		     /* Calculate the LP number and find the parts structure */
		     p_no = BLK2PART(vg->partshift, a.lbn);
		     part = PARTITION(lv, p_no, (a.mirror - 1));

		     /* verify the PV exists by looking for it's pvol ptr */
		     if( part->pvol == NULL ) {
		          rc = ENXIO;
		          break;
		     }
		     a.p_devt = part->pvol->dev;
		     a.pbn = part->start + 
				(a.lbn - PART2BLK(vg->partshift, p_no));
		}

		/* copy kernel area back to caller area */
		if( mode & DKERNEL )
		    bcopy((char *)(&a),(char *)arg,sizeof(struct xlate_arg));
		else {
		    if (copyout(&a, (char *)arg, sizeof(struct xlate_arg)))
			rc = EFAULT;
		}
		break;

	    case GETVGSA:		/* copy VGSA to area given	*/

		if( vg->vgsa_ptr ) {
		    if( mode & DKERNEL )
			bcopy((char *)(vg->vgsa_ptr), (char *)arg,
						sizeof(struct vgsa_area));
		    else if(copyout(vg->vgsa_ptr, (char *)arg,
						sizeof(struct vgsa_area)))
			rc = EFAULT;
		}
		else
		    rc = EFAULT;

		break;

            case CACLNUP:               /* Clean up MWC cache and write */
                                        /* it to PVs                    */
                /*
                 * If the VG is not closing then go clean up the
                 * mirror write consistency cache
                 */
                if( !(vg->flags & VG_FORCEDOFF) && (vg->mwc_rec != NULL))
                    hd_ca_clnup( vg );
                else
                    rc = EFAULT;

                break;

	    case LP_LVIEW:		/* send logical view of an lp */

		if(copyin((char *)arg,&lpinfo,sizeof(struct lpview))) {
		   rc = EFAULT;
	           break;
		}
		for(i=0; i < lv->nparts; i++) {
		   part = PARTITION(lv,(lpinfo.lpnum - 1),i);
		   if(part->pvol) {
		      lpinfo.copies[i].pvnum = part->pvol->pvnum + 1;
		      lpinfo.copies[i].ppnum = BLK2PART(vg->partshift,
					       (part->start - 
					       part->pvol->fst_usr_blk));
		      lpinfo.copies[i].ppnum ++;
		      if((part->ppstate & (PP_CHGING | PP_STALE)) == PP_STALE)
			  lpinfo.copies[i].ppstate = PP_STALE;
		      else
			  lpinfo.copies[i].ppstate = 0;
		    }
		}	
		if(copyout(&lpinfo,(char *)arg, sizeof(struct lpview)))
		    rc = EFAULT;
		break;

	    /* increase pbufs-per-disk count and adjust pbuf pool accordingly */
	    case PBUFCNT:
		if (copyin((char *) arg, &pbuf_cnt, sizeof(pbuf_cnt)) == 0) 
		    {
		    /* cannot exceed maximum allowed value */
		    if (pbuf_cnt <= PBSUBPOOLMAX)
			{
			/* if pbufs-per-disk value was actually increased */
		        if ((rc = incr_pbuf_grab(pbuf_cnt)) == 0)
			    /* adjust pbuf pool size */
		            rc = hd_allocpbuf();
			}
		    else
			rc = EINVAL;
		    }
		else
		    rc = EFAULT;
	    break;

	    default:				/* invalid command */
		rc = EFAULT;
		break;
	  }
	if (lrc == LOCK_SUCC) 	/* if no nested locks, unlock vg */
		unlockl(&(vg->vg_lock));
	return(rc);
}
/*
 *  NAME:	hd_alloca 
 * 
 *  FUNCTION:	Allocate a mirror write consistency cache
 *  
 *  NOTES:	Uses two arrays, ca_alloc and ca_grp_ptr, to allocate
 *		mirror write consistency caches.  This is a simple bit 
 *		map like allocation schema.  There are 8 caches in each
 *		4K byte page.
 *
 *  DATA STRUCTS: 
 *
 *  RETURN:	Address of the 512 byte region the VG can use for it's
 *		cache.
 *		NULL if malloc
 *
 */
struct mwc_rec *
hd_alloca( void )
{

	register struct mwc_rec *ca_ptr;/* cache pointer 		*/
	register int	i, j, k;
	register int	lrc;		/* return code from lockl	*/
	register int	rc;		/* general return code		*/

	int	size;			/* Size in bytes to malloc	*/

	BUGLPR(debuglvl, BUGNTA, ("enter hd_alloca\n"))

	/* Make sure we are the only ones in here */
	lrc = lockl( &hd_ca_lock, LOCK_SHORT );

	ca_ptr = NULL;
	for( i=0; i < VGS_CA; i++ ) {
	    /*
	     * If there is an available cache in this group get it
	     */
	    if( ca_alloced[i] != 0xff ) {
		/* 
		 * If nothing has been allocated for this group, go
		 * get it now.
		 */
		if( ca_grp_ptr[i] == NULL ) {
		    size = sizeof(struct mwc_rec) * NBPB;
	   	    ca_grp_ptr[i] = (struct mwc_rec *)
				xmalloc( (uint)size, HD_ALIGN, pinned_heap);

		    /* If null error on malloc */
		    if( ca_grp_ptr[i] == NULL )
			return( NULL );

		    /* Zero the entire area even though we zero the
		     * individual caches as we allocate them.  This 
		     * helps with debug and may be removed later. */
		    bzero( ca_grp_ptr[i], size );
		}
		/*
		 * Set base of group, then find first available cache
		 * in group, set the pointer to it, set bit indicating
		 * it is no longer free.
		 */
		ca_ptr = ca_grp_ptr[i];
		for( k=1, j=0; j < NBPB; j++, k <<= 1 ) {
		    if( (ca_alloced[i] & k) == 0 ) {
			ca_ptr += j;
			ca_alloced[i] |= k;
			break;
		    }
		}
		break;
	    }
	}

	bzero( ca_ptr, sizeof(struct mwc_rec) );

	/* if no nested locks,unlock cache arrays */
	if (lrc == LOCK_SUCC)
	    unlockl( &hd_ca_lock );

	return( ca_ptr );
}
/*
 *  NAME:	hd_dealloca 
 * 
 *  FUNCTION:	Deallocate a mirror write consistency cache
 *  
 *  NOTES:	Deallocates a cache allocated by hd_alloca().
 *
 *  DATA STRUCTS: 
 *
 *  RETURN:	none
 *
 */
void
hd_dealloca(
register struct mwc_rec *ca_ptr)	/* ptr to cache to free		*/
{
	register struct mwc_rec *bca_ptr;/* beginning of group ptr	*/
	register struct mwc_rec *eca_ptr;/* end of group ptr		*/

	register int	i;
	register int	idx;
	register int	lrc;		/* return code from lockl	*/

	int	size;			/* Size in bytes of cache group	*/

	BUGLPR(debuglvl, BUGNTA, ("enter hd_dealloca\n"))

	/* Make sure we are the only ones in here */
	lrc = lockl( &hd_ca_lock, LOCK_SHORT );

	size = sizeof(struct mwc_rec) * NBPB;

	/*
	 * Scan allocation arrays to find which page this cache is in
	 */
	for( i=0; i < VGS_CA; i++ ) {
	    /*
	     * find beginning and ending address of this group
	     */
	    bca_ptr = ca_grp_ptr[i];
	    eca_ptr = bca_ptr + NBPB;
	    /*
	     * If cache to free is between beginning and end go free it
	     */
	    if( (ca_ptr >= bca_ptr) && (ca_ptr < eca_ptr) ) {

		idx = ca_ptr - bca_ptr;
		ca_alloced[i] &= (~(1 << idx));
		/* 
		 * If this is the last cache in this group release
		 * the memory back to the system.
		 */
		if( ca_alloced[i] == 0 ) {
		    assert( xmfree( bca_ptr, pinned_heap ) == 0 );
		    ca_grp_ptr[i] = NULL;
		}
		break;
	    }
	}
	/* if no nested locks,unlock cache arrays */
	if (lrc == LOCK_SUCC)
	    unlockl( &hd_ca_lock );

	return;
}

/*
 *  NAME:         hd_init_bbdir
 * 
 *  FUNCTION:     Allocate and initialize data structures used by the
 *                bad block directory update function.
 *
 *  RETURN VALUES:
 *      0:  successful completion
 *      ENOMEM:  memory allocation failed
 */
int  
hd_init_bbdir()
{
register int	rc;	/* return code */
register int	size;	/* size to allocate */

BUGLPR(debuglvl, BUGNTA, ("enter hd_init_bbdir\n"))

/* get bad block directory pbuf and initialize it */
bb_pbuf = (struct pbuf *)xmalloc((uint)sizeof(struct pbuf),
		  		HD_ALIGN, pinned_heap);

if (bb_pbuf == NULL) 
    return(ENOMEM);

bzero(bb_pbuf, sizeof(struct pbuf));	/* zero out the pbuf */
bb_pbuf->pb.b_event = EVENT_NULL;	/* init event field */

/* get bad block directory buffer */
size = UNPINNED_BBDIR_SIZE;
bb_pbuf->pb_addr = (caddr_t) xmalloc((uint)size, HD_ALIGN,
                                     kernel_heap);

/* if bbdir buffer malloc failed, free bad block directory pbuf */
if (bb_pbuf->pb_addr == NULL) {
	assert(xmfree(bb_pbuf, pinned_heap) == 0);
	return(ENOMEM);
}

/* generate a cross memory descriptor for the BB Directory data 
   buffer and put it in the bb_pbuf */
bb_pbuf->pb.b_xmemd.aspace_id = XMEM_INVAL;
rc = xmattach(bb_pbuf->pb_addr, size, &(bb_pbuf->pb.b_xmemd), SYS_ADSPACE);

/* if memory attach failed, free bad block directory buffer and pbuf */
if (rc != XMEM_SUCC) {
	assert( xmfree( bb_pbuf->pb_addr, kernel_heap ) == 0 );
	assert( xmfree( bb_pbuf, pinned_heap ) == 0 );
	return(ENOMEM);
}
/* pin initial bad block directory memory */
if ((rc = pin(bb_pbuf->pb_addr, BB_DIR_PIN_INCRMNT)) != 0)
    return(rc);

pinned_bb_dir_memory = BB_DIR_PIN_INCRMNT;

return(0);
}

/*
 *  NAME:        alloc_mwc_mem
 * 
 *  FUNCTION:     Allocate and initialize data structures used by
 *                the mirror write consistency function.
 *  
 *  RETURN VALUES: 
 *      LVDD_SUCCESS:  successful completion
 *      ENOMEM:  memory allocation failed
 */
int  
alloc_mwc_mem(
struct volgrp  *vg)
{
struct ca_mwc_dp  *ca_pt1;	/* ptr to a part 1 entry */
struct ca_mwc_mp  *ca_pt2;	/* ptr to a part 2 entry */
struct pv_wait  *pvw_ptr;	/* ptr to a pv_wait structure */
int  lrc,			/* return code from lockl */
     i, cnt, size;

BUGLPR(debuglvl, BUGNTA, ("enter alloc_mwc_mem\n"))

if( vg->ca_size == 0 )
  vg->ca_size = mwcc_entries;

/* get a pointer to a mirror write consistency cache */
vg->mwc_rec = hd_alloca();

if( vg->mwc_rec == NULL ) 
  {
  /* failed to get a disk cache - part 1 */
  if (lrc == LOCK_SUCC)
    unlockl(&(vg->vg_lock));
  return(ENOMEM);
  }
/* now get space for part 2 of cache */
size = sizeof(struct ca_mwc_mp) * vg->ca_size;
vg->ca_part2 = (struct ca_mwc_mp *) xmalloc((uint)size, HD_ALIGN,
                                            pinned_heap);
if( vg->ca_part2 == NULL ) 
  {
  hd_openbkout( BO_MWC_P1, vg );
  if (lrc == LOCK_SUCC)
    unlockl(&(vg->vg_lock));
  return(ENOMEM);
  }
/* set up part 1 & 2 of cache */
bzero( vg->ca_part2, size );
cnt = vg->ca_size;
ca_pt1 = &(vg->mwc_rec->ca_p1[0]);
ca_pt2 = vg->ca_part2;
vg->ca_lst = ca_pt2;

for( i=0; i < cnt; i++, ca_pt2++, ca_pt1++ ) 
  {
  ca_pt2->state  = CACLEAN;
  ca_pt2->part1 = ca_pt1;
  /* if first entry, set next to next and prev to last */
  if (i == 0) 
    {
    ca_pt2->next = ca_pt2 + 1;
    ca_pt2->prev = ca_pt2 + (cnt -1);
    }
  /* if last entry, set next to first and prev to prev */
  else 
    if( i == (cnt - 1) ) 
      {
      ca_pt2->next = vg->ca_lst;
      ca_pt2->prev = ca_pt2 - 1;
      }
    /* otherwise set next to next and prev to prev */
    else 
      {
      ca_pt2->next = ca_pt2 + 1;
      ca_pt2->prev = ca_pt2 - 1;
      }
  }

/* Allocate memory for pv_wait structures.  The number of
   structures is derived by multiplying the maximum number
   of partitions per LP, a mulitiplier, and the size of the
   cache.  Once allocated, set up free list. */

i = MAXNUMPARTS * CA_MULT * (vg->ca_size);
size = i * sizeof(struct pv_wait);

vg->ca_pvwmem = (struct pv_wait *)xmalloc( (uint)size, HD_ALIGN,
pinned_heap);
if(vg->ca_pvwmem == NULL) 
  {
  hd_openbkout( BO_MWC_P2, vg );
  if (lrc == LOCK_SUCC)
    unlockl(&(vg->vg_lock));
  return(ENOMEM);
  }

vg->ca_freepvw = vg->ca_pvwmem;
pvw_ptr = vg->ca_freepvw;

while(--i) 
  {
  pvw_ptr->nxt_pv_wait = pvw_ptr + 1;
  pvw_ptr->lb_wait = NULL;
  pvw_ptr++;
  }

/* do the last one in the chain */
pvw_ptr->nxt_pv_wait = NULL;
pvw_ptr->lb_wait = NULL;

return(LVDD_SUCCESS);
}

