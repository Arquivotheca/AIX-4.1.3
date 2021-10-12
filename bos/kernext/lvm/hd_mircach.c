static char sccsid[] = "@(#)10	1.7.1.5  src/bos/kernext/lvm/hd_mircach.c, sysxlvm, bos411, 9428A410j 4/14/94 17:09:05";

/*
 * COMPONENT_NAME: (SYSXLVM) Logical Volume Manager Device Driver - 10
 *
 * FUNCTIONS: hd_ca_ckcach, hd_ca_use, hd_ca_new, hd_ca_wrt,
 *	      hd_ca_wend, hd_ca_sked, hd_ca_fnd, hd_ca_clnup, hd_ca_qunlk
 *	      hd_ca_pvque, hd_ca_end, hd_ca_term, hd_ca_mvhld
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *
 *  hd_mircach.c -- LVM device driver write consistency cache routines
 *
 *
 *	These routines handle the mirror write consistency cache used
 *	to maintain a record of possible mirrors that may be inconsistent
 *	if the system crashes.  They are part of the bottom
 *	half of this device driver and run in the scheduler layer.
 *
 *  Function:
 *
 *  Execution environment:
 *
 *	All these routines run on interrupt levels, so they are not
 *	permitted to page fault.  They run within critical sections
 *	that are serialized with block I/O offlevel iodone() processing.
 */


#include <sys/types.h>
#include <sys/errno.h>
#include <sys/intr.h>
#include <sys/sleep.h>
#include <sys/dasd.h>
#include <sys/trchkid.h>
#include <sys/user.h>
#include <sys/lockl.h>
#include <sys/hd.h>

/*
 *  NAME:       hd_ca_ckcach
 *
 *  FUNCTION:   Scan write consistency cache for request
 *
 *  NOTES:	The following actions are taken under the described 
 *		conditions:
 *
 *		1. LV/LTG pair is in cache and it is not changing.
 *		   i.e. has been written to all PVs.
 *
 *		   Increment IO count and return CA_HIT, i.e. the request
 *		   can be scheduled
 *
 *		2. LV/LTG pair is in cache but it is changing.
 *		   i.e. cache is currently being written to all PVs
 *
 *			if the cache is in flight
 *			   put request on cache hold queue.
 *			else
 *			   Increment IO count, move to head of hash queue,
 *			   put request pointer on PV write cache queue,
 *			   set PV cache write flag
 *			return CA_MISS
 *
 *		3. LV/LTG pair is not in cache.
 *
 *			if the cache is in flight
 *			   put request on cache hold queue.
 *			else
 *			   find an available cache entry, fill it with request
 *			   information, move to head of hash queue,
 *			   put request pointer on PV write cache queue,
 *			   set PV cache write flag
 *			return CA_MISS
 *
 *		   If there are no cache entries available put request
 *		   on cache hold queue, set the CA_FULL flag, and
 *		   return CA_MISS
 *
 *  PARAMETERS:   none
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: CA_HIT for a cache hit
 *		  CA_MISS for a cache miss or full
 *
 */
int
hd_ca_ckcach( 
register struct buf	*lb,		/* current logical buf struct	*/
register struct volgrp	*vg,		/* ptr to volgrp structure	*/
register struct lvol	*lv)		/* ptr to lvol stucture		*/
{
    register struct ca_mwc_mp	*hq_ptr;/* hash queue ptr		*/
    register struct ca_mwc_mp	**hq_anchor;/* hash queue anchor ptr	*/
    register struct pvol *tar_pvol;	/* target pvol ptr		*/
    register dev_t	rminor;		/* request minor number		*/
    register ulong	rltg;		/* request logical track group	*/
    register ulong	rlp;		/* request logical partition	*/

    register int	rc;		/* function return code		*/

    TRCHKL3T(HKWD_KERN_LVM | hkwd_LVM_CA_CHKCACH, lb->b_dev,
			lb->b_blkno, BLK2TRK(lb->b_blkno));
    /*
     * Find cache index, get pointer to hash queue
     * save request minor number, request LTG
     */
    hq_anchor = &(vg->ca_hash[ CA_HASH( lb ) ]);
    hq_ptr = *hq_anchor;
    rminor = minor(lb->b_dev);
    rltg   = BLK2TRK(lb->b_blkno);

    /*
     * If hash queue not empty scan list for a match
     */
    while( hq_ptr ) {
	if( (hq_ptr->part1->lv_ltg == rltg) &&
	    (hq_ptr->part1->lv_minor == rminor) ) {
	    /*
	     * We have a cache hit.  If the cache entry is not changing
	     * bump iocnt, move entry to head of list, set the
	     * Request Is Cached flag, and return CA_HIT so
	     * the caller will know to go ahead and schedule the 
	     * request.  If the the entry is changing then the cache
	     * must be check for in flight.  If it is in flight this
	     * request must hold.  If it is not in flight then the
	     * request can be queued to the pv_wait queues.
	     */
	    if( hq_ptr->state != CACHG ) {
		    hq_ptr->iocnt++;
		    hd_ca_use( vg, hq_ptr, CAHEAD );
		    lb->b_options |= REQ_IN_CACH;
		    TRCHKL3T(HKWD_KERN_LVM | hkwd_LVM_CA_CHKHIT, lb->b_dev,
				lb->b_blkno, BLK2TRK(lb->b_blkno));
		    return( CA_HIT );
	    }
	    else {
		if( vg->flags & CA_INFLT ) {
		    /*
		     * When this request comes off of the hold queue it
		     * should be scheduled since it will get a cache 
		     * hit and it will not be changing.
		     */
		    hd_quelb( lb, &(vg->ca_hld) );
		    return( CA_MISS );
		}
		/*
		 * Go ahead and queue this request to the PV wait queues
		 */
		if( (rc = hd_ca_pvque( lb, vg, lv )) != SUCCESS ) {
		    /*
		     * if not enough pv_wait structures hold this request
		     * until some become available
		     */
		    if( rc == CA_LBHOLD ) {
			hd_quelb( lb, &(vg->ca_hld) );
			return( CA_MISS );
		    }
		    /*
		     * must have returned FAILURE which means there were
		     * no PVs available to write the cache to so this 
		     * logical request will have to be errored off and
		     * CA_MISS returned to the call will not schedule it
		     */
		    lb->b_flags |= B_ERROR;
		    lb->b_error = EIO;
		    lb->b_resid = lb->b_bcount;
		    hd_terminate( lb );
		    return( CA_MISS );
		}

		/*
		 * PV cache write queues have been updated so must now
		 * wait for the cache writes to finish.
		 */
		hq_ptr->iocnt++;
		lb->b_options |= REQ_IN_CACH;
		hd_ca_use( vg, hq_ptr, CAHEAD );
		CA_VG_WRT( vg )
		return( CA_MISS );
	    }
	}
	hq_ptr = hq_ptr->hq_next;
    }
    /*
     * We have a cache miss.  If the cache is in flight hang request on
     * cache hold queue.
     */
    if( vg->flags & CA_INFLT ) {
	hd_quelb( lb, &(vg->ca_hld) );
	return( CA_MISS );
    }
    /*
     * Go get a cache entry.  If the cache is full hang request on cache
     * hold queue and return CA_MISS so caller will not schedule it.
     */
    hq_ptr = hd_ca_new( vg );
    if( hq_ptr == NULL ) {
	/* cache is full */
	hd_quelb( lb, &(vg->ca_hld) );
	return( CA_MISS );
    }
    
    /*
     * We have a cache entry.  Go ahead and queue this request to the PV
     * wait queues then  fill it with information about request,
     * put it at head of list if it was successfully queued to the PVs. 
     */
    if( (rc = hd_ca_pvque( lb, vg, lv )) != SUCCESS ) {
	/*
	 * if not enough pv_wait structures hold this request
	 * until some become available, clean up the cache entry we
	 * just got
	 */
	hd_ca_use( vg, hq_ptr, CATAIL );
	hq_ptr->state		= CACLEAN;
	hq_ptr->part1->lv_ltg   = 0;
	hq_ptr->part1->lv_minor = 0;

	if( rc == CA_LBHOLD ) {
	    hd_quelb( lb, &(vg->ca_hld) );
	    return( CA_MISS );
	}
	/*
	 * must have returned FAILURE which means there were
	 * no PVs available to write the cache to so this 
	 * logical request will have to be errored off and
	 * CA_MISS returned to the caller so the request will not
	 * be scheduled
	 */
	lb->b_flags |= B_ERROR;
	lb->b_error = EIO;
	lb->b_resid = lb->b_bcount;
	hd_terminate( lb );
	return( CA_MISS );
    }
    hd_ca_use( vg, hq_ptr, CAHEAD );
    hq_ptr->state		= CACHG;
    hq_ptr->part1->lv_ltg	= rltg;
    hq_ptr->part1->lv_minor	= rminor;
    hq_ptr->iocnt		= 1;
    CA_VG_WRT( vg )

    lb->b_options |= REQ_IN_CACH;

    /*
     * Put cache entry at head of its hash queue
     */
    hq_ptr->hq_next = *hq_anchor;
    *hq_anchor = hq_ptr;

    return( CA_MISS );
}

/*
 *  NAME:       hd_ca_use
 *
 *  FUNCTION:   Put cache entry at head or tail of in use list
 *
 *  NOTES:	This is a simple most recently used(MRU) and least
 *		recently used(LRU) mechanism.
 *
 *  PARAMETERS:   none
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 *
 */
void
hd_ca_use(
register struct volgrp	  *vg,		/* ptr to volgrp structure	*/
register struct ca_mwc_mp *ca_ent,	/* cache entry pointer		*/
register int	h_t)			/* head/tail flag		*/
{
    register struct ca_mwc_mp *ca_anchor;/* cache head entry		*/

    ca_anchor = vg->ca_lst;		/* get anchor ptr		*/
    /*
     * If entry is to be move to the head of the list?
     and it is already at head
     * just return
     */
    if( h_t == CAHEAD ) {
	/*
	 * If it is already at head of the list just return
	 */
	if( ca_ent == ca_anchor )
	    return;
	/*
	 * If it is currently at the tail of the list just move the
	 * anchor to point to the previous entry.
	 */
	if( ca_ent == ca_anchor->prev ) {
	    vg->ca_lst = ca_ent;
	    return;
	}
    }
	/*
	 * Entry is to be move to the tail and it is already at the tail
	 * just return
	 */
    else {
	/*
	 * If it is already at the tail of the list just return
	 */
	if( ca_ent == ca_anchor->prev )
	    return;
	/*
	 * If it is currently at the head of the list just move the
	 * anchor to point to the next entry.
	 */
	if( (h_t == CATAIL) && (ca_ent == ca_anchor) ) {
	    vg->ca_lst = ca_anchor->next;
	    return;
	}
    }
    /*
     * Remove entry from current position
     */
    ca_ent->prev->next = ca_ent->next;
    ca_ent->next->prev = ca_ent->prev;

    /*
     * Add it back to the list
     */
    ca_ent->next = ca_anchor;
    ca_ent->prev = ca_anchor->prev;
    ca_anchor->prev->next = ca_ent;
    ca_anchor->prev = ca_ent;

    /*
     * If moving to head change anchor to point to it
     */
    if( h_t == CAHEAD )
	vg->ca_lst = ca_ent;

    return;
}

/*
 *  NAME:       hd_ca_new
 *
 *  FUNCTION:   Find an available entry in cache for a new request
 *
 *  NOTES:	Follow the LRU chain back looking for an entry that
 *		is available to use.  Available is defined as:
 *
 *		1. Entry has a zero iocnt
 *
 *		If the cache is full, no available entries, return NULL.
 *
 *  ASSUMPTIONS:Cache is not in flight on entry.
 *
 *  PARAMETERS:   none
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: Address of entry if found or NULL if cache full
 *
 */
struct ca_mwc_mp *
hd_ca_new(				/* New entry for cache		*/
register struct volgrp	*vg)		/* ptr to volgrp structure	*/
{
    register struct ca_mwc_mp *ca_ent;	/* ptr to entry to use		  */
    register struct ca_mwc_mp *lru_ent;	/* ptr to the least recently used */
    register struct ca_mwc_mp *lststrt;	/* ptr to where we started in list*/

    /*
     * Set lru_ent to the end of the chain, and initialize other stuff.
     */
    lru_ent = vg->ca_lst->prev;
    lststrt = lru_ent;
    ca_ent  = NULL;

    do {

	/*
	 * If entry has no IO outstanding then use it.
	 */
	if( lru_ent->iocnt == 0 ) {
	    ca_ent = lru_ent;
	    /*
	     * If the entry is not CACLEAN then it is on some hash
	     * queue and it must be removed from there.
	     */
	    if( ca_ent->state != CACLEAN )
		hd_ca_qunlk( vg, ca_ent );
	    break;
	}

	/*
	 * If the current entry is the one we started with we have gone
	 * through the entire list.
	 */
	lru_ent = lru_ent->prev;
    } while( lru_ent != lststrt );
    /*
     * Reset cache full flag and return the cache entry pointer if one.
     * If no entry was found set cache full flag and return NULL for
     * cache full.
     */
    if( ca_ent )
	vg->flags &= ~CA_FULL;
    else
	vg->flags |= CA_FULL;

    return( ca_ent );
}

/*
 *  NAME:       hd_ca_wrt
 *
 *  FUNCTION:   Start cache writes to PVs/VGs on pending VG list
 *
 *  NOTES:	Scan active VG list and for each PV with it's PV
 *		write flag on build a pbuf struct and put it on
 * 		the ready_Q.
 *
 *  PARAMETERS:   none
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 *
 */
void
hd_ca_wrt()
{
    register struct volgrp	*vg;	/* Current volgrp struct ptr	*/
    register struct pvol	*pvol;	/* Current pvol struct ptr	*/

    register int		pvidx;	/* Index into pvol ptr array	*/
    register int		rc;	/* general return code		*/

    struct xmem			dp;	/* Temporary cross memory descip*/
    /*
     * While there are still VGs with cache writes pending
     */
    while( hd_vg_mwc ) {

	/* Get the first volgrp structure from list */
	vg = hd_vg_mwc;
	hd_vg_mwc = vg->nxtactvg;
	vg->flags &= ~CA_VGACT;

	/* Set the cache in flight flag */
	vg->flags |= CA_INFLT;

	/* Get time stamp for cache update */
	hd_gettime( &(vg->mwc_rec->b_tmstamp) );
	vg->mwc_rec->e_tmstamp = vg->mwc_rec->b_tmstamp;

	/* Generate a cross memory descriptor */
	dp.aspace_id = XMEM_INVAL;
	rc = xmattach( vg->mwc_rec, sizeof( struct mwc_rec  ), 
			&dp, SYS_ADSPACE);
	ASSERT( rc == XMEM_SUCC );

	/*
	 * Scan the PV write flags and start cache writes as needed
	 */
	for( pvidx=0; TSTALLPVWRT( vg ); pvidx++ ) {

	    /* If the flag is on start cache write to this PV */
	    if( TSTPVWRT( vg, pvidx ) ) {

		vg->ca_inflt_cnt++;
		pvol = vg->pvols[pvidx];
		hd_bldpbuf( &(pvol->pv_pbuf), pvol, CATYPE_WRT, vg->mwc_rec,
			    sizeof(struct mwc_rec), &dp, hd_ca_wend );

		TRCHKL1T(HKWD_KERN_LVM | hkwd_LVM_CA_WRT, pvol->dev);

		hd_start( &(pvol->pv_pbuf) );
		CLRPVWRT( vg, pvidx );
	    }
	}
    }
}

/*
 *  NAME:       hd_ca_wend
 *
 *  FUNCTION:   Cache write end
 *
 *  NOTES:	Handles the completion of cache writes.  If there
 *		is an error declare the PV missing.  Otherwise 
 *		put the waiting requests on the sched_Q.
 *
 *  PARAMETERS:   none
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 *
 */
void
hd_ca_wend(
register struct pbuf *pb)		/* Address of pbuf completed	*/
{
    register struct pvol *pvol;		/* pvol ptr for this PV		   */
    register struct ca_mwc_mp  *c_ca;	/* ptr to current cache entry	   */

    struct volgrp	*vg;	/* VG volgrp ptr from devsw table	*/

    pvol = pb->pb_pvol;

    TRCHKL4T(HKWD_KERN_LVM | hkwd_LVM_CA_WEND, pb, pvol->dev,
		pb->pb.b_flags , pb->pb.b_error);

    /*
     * Get the volgrp structure pointer.
     */
    (void) devswqry( makedev(pvol->vg_num, 0), NULL, &vg );
    /*
     * If B_ERROR is on it means one of two things.
     *
     *	1. The actual MWC cache write failed.  In this case we must mark
     *		the offending PV as missing leaving the requests currently
     *		on the ca_pv list suspended until the VGSA has been updated
     *		with the missing PV information.
     *
     *	2. ALL the VGSAs have been updated and the pbuf is being returned
     *		to indicate this.  This is indicated by the pb_type field
     *		indicating a type of SA_PVMISSING.
     */
    if( (pb->pb.b_flags & B_ERROR) && (pb->pb_type != SA_PVMISSING) ) {
	hd_logerr( (unsigned)ERRID_LVM_MWCWFAIL, (ulong)(pb->pb.b_dev),
		(ulong)(pb->pb.b_blkno), (ulong)(pb->pb.b_error) );
	/*
	 * Give this pb to the WHEEL
	 */
	pb->pb_pvol->pvstate = PV_MISSING;
	if( hd_sa_strt(pb, vg, SA_PVMISSING) == SUCCESS )
	    return;
    }
    /*
     * Go schedule any requests that are ready. i.e. this was the last
     * cache write they were waiting on
     *
     * NOTE: It is possible to have no requests on the PV chain if this cache
     * write was caused by making an entry available in the cache or
     * due to a cache clean up operation.
     */
    hd_ca_sked( vg, pvol );

    /*
     * Decrement the cache write count.
     *	if zero
     *	    turn off cache in flight flag
     *	    set flag indicating need to redrive the scheduler
     *	    detach the cache
     *	    set the state of cache entries from CACHG or CASTILLDRT
     *	    to CANOCHG drive the cache hold queue for this VG
     */
     
    if( --vg->ca_inflt_cnt == 0 ) {
	vg->flags &= ~CA_INFLT;
	xmdetach( &(pb->pb.b_xmemd) );
	/*
	 * Scan cache chain MRU to LRU changing CACHG to CANOCHG
	 * until end or a CACLEAN one is encountered.  There should
	 * be no active entry after the first CACLEAN one.
	 */
	c_ca = vg->ca_lst;
	do {

	    if( c_ca->state == CACHG )
		c_ca->state = CANOCHG;
	    else if( c_ca->state == CACLEAN )
		break;

	    c_ca = c_ca->next;
	} while( c_ca != vg->ca_lst );

    /* if a process is waiting for cache write completion, leave CA_INFLT 
       on to reserve cache for process' use and awaken it */
    if (vg->ecachwait != EVENT_NULL)
        {
        vg->flags |= CA_INFLT;
        e_wakeup(&(vg->ecachwait));
        }
    else 
        hd_ca_mvhld(vg);
    }
}

/*
 *  NAME:       hd_ca_sked
 *
 *  FUNCTION:   Traverse the ca_pv list decrementing lbuf reference counts.
 *		If a count is found to be zero then hang the lbuf back on
 *		the pending_Q so it can be scheduled.
 *
 *  PARAMETERS:
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 *
 */
void
hd_ca_sked(
register struct volgrp	*vg,		/* ptr to volgrp structure	*/
register struct pvol	*pvol)		/* pvol ptr for this PV		*/
{
    register struct pv_wait	*pwait;	/* ptr to structure that points	    */
					/* to the lbuf in question	    */
    register struct ca_mwc_mp	*ca_e;	/* ptr to MWC cache entry	    */
    register int		ref_cnt;/* ref counter in lbuf(av_forw)	    */

    while( pwait = pvol->ca_pv.head ) {
	pvol->ca_pv.head = pwait->nxt_pv_wait;

	/*
	 * Decrement the reference count and if it is zero abort it if the
	 * VG is closing or hang the request on the front of the pending_Q.
	 */
	ref_cnt = (int)(pwait->lb_wait->av_forw);
	ref_cnt--;
	pwait->lb_wait->av_forw = (struct buf *)ref_cnt;
	if( ref_cnt == 0 ) {
	    /*
	     * If the VG is closing then just error off the logical
	     * request
	     */
	    if( vg->flags & VG_FORCEDOFF ) {
		pwait->lb_wait->b_flags |= B_ERROR;
		pwait->lb_wait->b_error = EIO;
		pwait->lb_wait->b_resid = pwait->lb_wait->b_bcount;
		hd_ca_term( pwait->lb_wait );

		hd_terminate( pwait->lb_wait );
	    }
	    else {
		/*
		 * Hang request on head of the pending_Q
		 */
		pwait->lb_wait->av_forw = pending_Q.head;
		if( pending_Q.head == NULL )
		    pending_Q.tail = pwait->lb_wait;
		pending_Q.head = pwait->lb_wait;

		/* trace lbuf, device, block# */
		TRCHKL3T(HKWD_KERN_LVM | hkwd_LVM_MWCCWCOMP, pwait->lb_wait,
			pwait->lb_wait->b_dev, pwait->lb_wait->b_blkno);
	    }
	}
	REL_PVWAIT( pwait, vg )
    }

    pvol->ca_pv.tail = NULL;
    return;
}

/*
 *  NAME:       hd_ca_fnd
 *
 *  FUNCTION:   Find MWC cache entry for given request
 *
 *  NOTES:
 *
 *  PARAMETERS:   none
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: pointer to MWC cache entry - memory part
 *
 */
struct ca_mwc_mp *
hd_ca_fnd(
register struct volgrp	*vg,		/* ptr to volgrp structure	*/
register struct buf	*lb)		/* ptr to lbuf to find the entry*/
					/* for				*/
{
    register struct ca_mwc_mp	*hq_ptr;/* ptr to head of hash list	*/
    register dev_t		rminor;	/* request LV minor number	*/
    register ulong		rltg;	/* request logical track group	*/

    /*
     * Get the head of the list from hash anchor, then calculate the 
     * request minor number and LTG
     */
    hq_ptr = vg->ca_hash[ CA_HASH( lb ) ];
    rminor = minor( lb->b_dev );
    rltg   = BLK2TRK( lb->b_blkno );

    /*
     * Scan hash list for requests entry
     */
    while( hq_ptr ) {

	if( (hq_ptr->part1->lv_ltg   == rltg) &&
	    (hq_ptr->part1->lv_minor == rminor) ) {

	    break;
	}

	hq_ptr = hq_ptr->hq_next;
    }

    return( hq_ptr );
}

/*
 *  NAME:       hd_ca_clnup
 *
 *  FUNCTION:   Clean up cache entries that have no IO outstanding
 *
 *  NOTES:	This function is called at process level, close & ioctl,
 *		to clean up the MWCC.  It is here because it disables to
 *		INTIODONE.  Code that runs disabled cannot page fault.  
 *		This function walks the hash queues and removes entries
 *		with no IO outstanding, then writes the updated cache to
 *		all PVs in the VG.
 *
 *  ASSUMPTIONS: The VG is not closing (being forced off).
 *
 *  PARAMETERS: volume group for which to clean MWC 
 *              
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 *
 */
void  
hd_ca_clnup(
register struct volgrp  *vg)
{
    register struct ca_mwc_mp	*hq_ptr;/* queue entry ptr		*/
    register struct ca_mwc_mp	*ul_ptr;/* queue entry ptr to unlink	*/
    register struct pvol	*pv_ptr;/* pvol ptr			*/

    register int	ca_idx;		/* hash queue index		*/
    register int	chg_flg=0;	/* a cache entry was changed	*/
    int  int_lvl;
    struct pbuf  *ready_list=NULL;


    TRCHKL1T(HKWD_KERN_LVM | hkwd_LVM_CA_CLNUP, makedev(vg->major_num,0));

    int_lvl = disable_lock(INTIODONE, &glb_sched_intlock);

    /* if MWC cache in use, sleep until available; CA_INFLT will be set when
       awakened to reserve the cache for this process */
    if (vg->flags & CA_INFLT) 
        e_sleep_thread(&(vg->ecachwait), &glb_sched_intlock, LOCK_HANDLER);
    else
        vg->flags |= CA_INFLT;	

    /* for each hash queue */
    for( ca_idx=0; ca_idx < CAHHSIZE; ca_idx++ ) 
        {
	/* go down the queue if anything on it */
	hq_ptr = vg->ca_hash[ca_idx];
	while( hq_ptr ) 
            {
	    /* if no io outstanding, unlink it from hash queue, move it 
               to LRU, and clean it up */
	    ul_ptr = hq_ptr;
	    hq_ptr = hq_ptr->hq_next;
	    if( ul_ptr->iocnt == 0 ) 
                {
		chg_flg = 1;
		hd_ca_qunlk( vg, ul_ptr );
		hd_ca_use( vg, ul_ptr, CATAIL );
		ul_ptr->state = CACLEAN;
		ul_ptr->part1->lv_ltg = 0;
		ul_ptr->part1->lv_minor = 0;
	        }
	    }
        }
    /* if any entry changed then write cache to all PVs in the VG */
    if( chg_flg ) 
	for( ca_idx=0; ca_idx < MAXPVS; ca_idx++ ) 
            {
	    pv_ptr = vg->pvols[ca_idx];
	    if( pv_ptr && (pv_ptr->pvstate != PV_MISSING) )
		SETPVWRT( vg, pv_ptr->pvnum );
	    }

    /* if MWC cache changed, write it to disks */
    if (TSTALLPVWRT(vg)) 
        {
	CA_VG_WRT(vg)
	hd_ca_wrt();

        /* wait for MWC cache writes to complete */
        e_sleep_thread(&(vg->ecachwait), &glb_sched_intlock, LOCK_HANDLER);
        }
    vg->flags &= ~CA_INFLT;

    /* redrive blocked MWC and normal i/o queues */
    hd_ca_mvhld(vg);

    hd_schedule(&ready_list);
    unlock_enable(int_lvl, &glb_sched_intlock);
    hd_start(ready_list);
}


/*
 *  NAME:       hd_ca_qunlk
 *
 *  FUNCTION:   Unlink given cache entry for the hash queue it is on
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
hd_ca_qunlk(
register struct volgrp	*vg,		/* ptr to volgrp structure	*/
register struct ca_mwc_mp  *ca_ent)	/* ptr to cache entry to unlink	*/
{
    register struct ca_mwc_mp	*hq_ptr;    /* hash queue entry ptr	*/
    register struct ca_mwc_mp	**hq_prev;  /* prev hash queue entry ptr*/

    register int	ca_idx;		/* hash queue index		*/

    ca_idx  = CA_THASH( ca_ent->part1->lv_ltg );
    hq_prev = &(vg->ca_hash[ca_idx]);
    hq_ptr  = *hq_prev;

    /*
     * Go down hash queue looking for match
     */
    while( hq_ptr != ca_ent ) {
	hq_prev = (struct ca_mwc_mp **)hq_ptr;
	hq_ptr = hq_ptr->hq_next;
	ASSERT( hq_ptr != NULL );
    }
    ASSERT( hq_ptr != NULL );

    /* Unlink it from chain */
    ((struct ca_mwc_mp *)hq_prev)->hq_next = hq_ptr->hq_next;

    /* Zero out the hash queue ptr */
    hq_ptr->hq_next = NULL;

    return;
}

/*
 *  NAME:       hd_ca_pvque
 *
 *  FUNCTION:   For a given lbuf allocate and place a pv_wait structure
 *		on each PVs queue that has a LTG for the lbuf.
 *
 *  NOTES:	The av_forw ptr in the lbuf is used as a reference count
 *		For each PV queue the lbuf is referenced by this count
 *		is incremented.  As the cache write for each PV completes
 *		this reference count is decrement.  When the count gets
 *		back to 0 the lbuf is ready to be scheduled, i.e. all the
 *		cache writes for this lbuf are completed.
 *
 *  PARAMETERS:
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: CA_LBHOLD	Not enough pv_wait structures for this 
 *				lbuf.  It will have to hold until some are
 *				freed and available.
 *		  SUCCESS	This lbuf has been successfully queued to all
 *				PV queues.
 *		  FAILURE	There are no PVs available, i.e. all are 
 *				missing, for this lbuf.  It must be errored
 *				off.
 *
 */
int
hd_ca_pvque(
register struct buf	*lb,		/* current logical buf struct	*/
register struct volgrp	*vg,		/* ptr to volgrp structure	*/
register struct lvol	*lv)		/* ptr to lvol stucture		*/
{

    register ulong		rlp;	/* request target LP		*/
    register struct pv_wait	*pptr;	/* ptr to structure controlling	*/
					/* the waiting of a lbuf	*/

    register struct pvol	*cpvol;/* ptr to current pvol structure*/

    register int		i;	/* number of copy count		*/
    register int		ref_cnt;/* unwind copy count		*/

    struct pvol	*apvol[MAXNUMPARTS];	/* an array of ptrs to pvol	*/
					/* structures used to backout	*/
					/* when pv_wait_free is empty	*/

    ref_cnt = 0;
    lb->av_forw = (struct buf *)ref_cnt;/* used as a reference count	*/

    /* calculate the LP number */
    rlp = BLK2PART( vg->partshift, lb->b_blkno );

    for( i=0; i < (lv->nparts); i++ ) {

	cpvol = (PARTITION( lv, rlp, i ))->pvol;

	/* Hang on PV Que if PV not missing */
	if( cpvol && (cpvol->pvstate != PV_MISSING) ) {

	    /* if no free ones then we can not do anything with this
	     * request but hang it on the cache hold queue.  Before 
	     * that is done we must unwind what has been done so far.
	     */
	    if( TST_PVWAIT( vg ) ) {
		/*
		 * Remove last pv_wait from PV queues that have already
		 * had them added before we ran out
		 */
		vg->ca_pvwblked++;
		for( --i; i >= 0; i-- ) {
		    /*
		     * Find the next to the last and the last(tail) of the
		     * queue.
		     */
		    if( cpvol = apvol[i] ) {
			pptr = (struct pv_wait *)(&(cpvol->ca_pv.head));
			while( pptr->nxt_pv_wait ) {
			    if( pptr->nxt_pv_wait == cpvol->ca_pv.tail ) {
				/*
				 * Found them so release it and unlink tail
				 */
				REL_PVWAIT( pptr->nxt_pv_wait, vg )
				pptr->nxt_pv_wait = NULL;
				if( cpvol->ca_pv.head == NULL )
				    cpvol->ca_pv.tail = NULL;
				else
				    cpvol->ca_pv.tail = pptr;
			    }
			    else
				pptr = pptr->nxt_pv_wait;
			}
		    }
		}
		return( CA_LBHOLD );
	    }
	    GET_PVWAIT( pptr, vg )

	    apvol[i] = cpvol;		/* in case we have to back out	*/

	    pptr->lb_wait = lb;

	    ref_cnt++;			/* bump the reference count in	*/
					/* the lbuf			*/
	    lb->av_forw = (struct buf *)ref_cnt;
	    /*
	     * Hang this pv_wait structure on the PV wait queue
	     */
	    pptr->nxt_pv_wait = NULL;
	    if( cpvol->ca_pv.head == NULL )
		cpvol->ca_pv.head = pptr;
	    else
		cpvol->ca_pv.tail->nxt_pv_wait = pptr;

	    cpvol->ca_pv.tail = pptr;
	}
	else
	    apvol[i] = NULL;		/* in case we have to back out	*/
    }

    /*
     * If the reference count is zero then there are no PVs to write to
     * and the request should fail
     */
    if( ref_cnt == 0 )
	return( FAILURE );

    /*
     * set the PV cache write flags for the PVs that must have the cache
     * written to them
     */
    for( i=0; i < (lv->nparts); i++ ) {
	if( apvol[i] ) 
	    SETPVWRT( vg, apvol[i]->pvnum );
    }

    return( SUCCESS );
}

/*
 *  NAME:	hd_ca_end 
 * 
 *  FUNCTION:	LVM PV cache write operation iodone() routine
 *
 *  NOTES:	This routine is scheduled to run as an offlevel interrupt 
 *	  	handler when the physical device driver calls iodone(). 
 *		It will reissue the cache write with hardware relocation
 *		turned on if the first one fails with an EMEDIA or ESOFT.
 *		If the relocation request fails it passes the error on to
 *		the cache write scheduler.  It will declare the PV missing.
 *
 *		input:  physical buf for a completed request; b_resid and
 *				b_error set by physical device driver if
 *		    		b_flags = B_ERROR.
 *				b_error = EIO - non-media I/O error
 *					  EMEDIA - newly grown media error
 *					  ESOFT - request succeeded, but needs 
 *						relocation (may not work next 
 *						time)
 *
 *  PARAMETERS:	pb - physical buf struct 
 *
 *  DATA STRUCTS: 
 *
 *  RETURN VALUE: none
 *
 */
void
hd_ca_end(
register struct pbuf *pb)		/* physical device buf struct	 */
{
	register int  int_lvl;		/* interrupt level save */
        struct pbuf  *ready_list=NULL;

	/* serialize with all i/o request/completion processing */
        int_lvl = disable_lock(INTIODONE, &glb_sched_intlock);

	TRCHKL4T(HKWD_KERN_LVMSIMP | hkwd_LVM_PEND, pb, pb->pb.b_flags,
		 pb->pb.b_error, pb->pb.b_resid);

	/* turn off the BUSY flag */
	pb->pb.b_flags &= ~B_BUSY;

	/* if an EMEDIA or ESOFT then reissue the request attempting to 
           do a hardware relocate on it if hardware relocate has not been 
           tried already. */
	if( pb->pb.b_flags & B_ERROR ) {
	    if( (pb->pb.b_error == EMEDIA) || (pb->pb.b_error == ESOFT) ) {
		if( !(pb->pb.b_options & HWRELOC) ) {
		    pb->pb.b_options |= HWRELOC;
		    pb->pb.b_flags &= ~B_ERROR;
		    pb->pb.b_flags |= B_BUSY;
		    pb->pb.b_error = 0;
		    pb->pb.b_resid = 0;
                    unlock_enable(int_lvl, &glb_sched_intlock);
		    hd_start(pb);
		    return;
		}
	    }
	    /* since HW relocation already tried or some other error 
               other than EMEDIA or ESOFT just bubble the error back up */
	    pb->pb.b_options &= ~HWRELOC;
	    HD_SCHED( pb );

            hd_schedule(&ready_list);
            unlock_enable(int_lvl, &glb_sched_intlock);
            hd_start(ready_list);
	    return;
	}
	/* If HW relocation was successful then log an error messages to 
           indicate that. */
	if( pb->pb.b_options & HWRELOC ) {
	    hd_logerr( (unsigned)ERRID_LVM_HWREL, (ulong)(pb->pb.b_dev),
			(ulong)(pb->pb.b_blkno), (ulong)0 );
	    pb->pb.b_options &= ~HWRELOC;
	}
	HD_SCHED(pb);
        hd_schedule(&ready_list);
        unlock_enable(int_lvl, &glb_sched_intlock);
        hd_start(ready_list);
}

/*
 *  NAME:	hd_ca_term 
 * 
 *  FUNCTION:	A logical request that is cached has finished
 *
 *  NOTES:
 *
 *  PARAMETERS:
 *
 *  DATA STRUCTS: 
 *
 *  RETURN VALUE: none
 *
 */
void
hd_ca_term(
register struct buf	*lb)		/* current logical buf struct	*/
{
    register struct ca_mwc_mp	*hq_ptr;/* hash queue entry ptr		*/
    struct volgrp	*vg;	/* VG volgrp ptr from devsw table	*/

    /* This logical request has been completed, if in mirror write 
       consistency cache decrement IO count. */
    ASSERT( lb->b_options & REQ_IN_CACH );
    TRCHKL1T(HKWD_KERN_LVM | hkwd_LVM_CA_TERM, lb);
    
    (void) devswqry( lb->b_dev, NULL, &vg );

    /* Find the cache entry and decrement the IO count */
    hq_ptr = hd_ca_fnd( vg, lb );
    hq_ptr->iocnt--;
    lb->b_options &= ~REQ_IN_CACH;

    /*
     * If we just freed up a cache entry, and the cache is not in flight,
     * and the cache indicates it was full the last time an attempt to
     * find and entry was made, and there is something on the cache hold
     * queue then move the hold queue to the pending queue.  The pending
     * queue will be redriven by hd_schedule() via hd_end().
     */
    if( !(hq_ptr->iocnt) && !(vg->flags & CA_INFLT) &&
	(vg->flags & CA_FULL) && (vg->ca_hld.head) ) {
	hd_ca_mvhld( vg );
    }
}

/*
 *  NAME:	hd_ca_mvhld 
 * 
 *  FUNCTION:	Move the cache hold queue from the volgrp to the pending_Q
 *
 *  NOTES:	The cache hold queue is appended to the end of the 
 *		pending_Q.  If the VG is closing then all requests on the
 *		hold queue are returned with errors.
 *
 *  PARAMETERS:
 *
 *  DATA STRUCTS: 
 *
 *  RETURN VALUE: none
 *
 */
void
hd_ca_mvhld(
register struct volgrp	*vg)		/* ptr to volgrp structure	*/
{

    TRCHKL1T(HKWD_KERN_LVM | hkwd_LVM_MVHLD, makedev(vg->major_num,0));

    if( vg->ca_hld.head ) {
	/*
	 * If the VG is closing then error off all requests sitting on 
	 * the hold queue.  Since they were not cached this is pretty
	 * simple.
	 */
	if( vg->flags & VG_FORCEDOFF ) {

	    register struct buf *lb;	/* chain ptr			*/

	    while( lb = vg->ca_hld.head ) {
		vg->ca_hld.head = lb->av_forw;
		lb->b_flags |= B_ERROR;
		lb->b_error = EIO;
		lb->b_resid = lb->b_bcount;
		hd_terminate( lb );
	    }
	}
	else {
	    if( pending_Q.head == NULL )
		pending_Q.head = vg->ca_hld.head;
	    else
		pending_Q.tail->av_forw = vg->ca_hld.head;

	    pending_Q.tail = vg->ca_hld.tail;
	}

	vg->ca_hld.head = NULL;
	vg->ca_hld.tail = NULL;
    }
    return;
}
