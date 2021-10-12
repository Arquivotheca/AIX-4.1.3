static char sccsid[] = "@(#)77	1.38  src/bos/kernext/lvm/hd_sched.c, sysxlvm, bos411, 9428A410j 5/23/94 10:07:05";

/*
 * COMPONENT_NAME: (SYSXLVM) Logical Volume Manager Device Driver - hd_sched.c
 *
 * FUNCTIONS: hd_schedule, hd_avoid, hd_resyncpp, hd_freshpp, hd_mirread,
 *	      hd_fixup, hd_stalepp, hd_staleppe, hd_xlate, hd_regular,
 *	      hd_finished, hd_sequential, hd_seqnext, hd_seqwrite,
 *	      hd_parallel, hd_freeall, hd_append, hd_nearby, hd_parwrite
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
 *  hd_sched.c -- LVM device driver block request scheduler.
 *
 *
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
 *
 *  Function:
 *
 *	These functions associate one or more physical buf structs with
 *	each pending logical request and schedule them to the
 *	appropriate physical device driver(s).  When a physical I/O
 *	operation is complete, the scheduler initiates the next phase
 *	of the corresponding logical request, or notifies the originator
 *	that this logical operation has been completed.
 *
 *	Different scheduling policies seem to be best for different
 *	applications, depending on the recovery strategy they employ.
 *	Therefore, this scheduler is state-driven, and the initial
 *	state determines the policy used for a given logical volume.
 *	Additional scheduling policies can easily be added.
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
#include <sys/lvmd.h>
#include <sys/dasd.h>
#include <sys/vgsa.h>
#include <sys/trchkid.h>
#include <sys/hd.h>
#include <sys/systemcfg.h>

/*
 *  Scheduling policy switch.
 *
 *	This array is used to select the initial scheduler policy,
 *	indexed by `i_sched' in the lvol structure.  This table must be
 *	kept in sync with the SCH_* defines in dasd.h
 *
 *	The basic declaration is a pointer to a function that returns an int
 */
typedef int (*PFI) (struct buf *, struct pbuf **ready_list, struct volgrp *);

PFI hd_schedsw[] = {
	hd_regular,
	hd_sequential,
	hd_parallel,
	hd_sequential,
	hd_parallel,
	hd_stripe
};

/* ------------------------------------------------------------------------
 *
 *		M A I N   S C H E D U L E R   R O U T I N E S
 *
 * ------------------------------------------------------------------------
 */


/*
 *  NAME:         hd_schedule
 *
 *  FUNCTION:     Schedule logical and physical operations.
 *
 *  NOTES:
 *	input:	pending_Q contains logical buf structs to schedule.
 *		ready_list contains physical buf structs to start immediately.
 *	output:	all possible pending logical and ready physical operations
 *		started.
 *
 *		The first phase of the scheduler is to scan the pending_Q
 *		for mirrored write requests.  This is done by the mirror
 *		write consistency manager in hd_mircach.c.
 *
 *  PARAMETERS:   ready_list - physical request list being built
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 *
 */

void  
hd_schedule(
struct pbuf  **ready_list)
{
register struct buf  *lb;
register int  rc;
struct volgrp  *vg;

BUGLPR(debuglvl, BUGNTA, ("hd_schedule: *** started ***\n"))

/* schedule each request according to its logical volume schedule policy */
while (pending_Q.head)  
    {
    lb = pending_Q.head;
    pending_Q.head = lb->av_forw;

    BUGLPR(debuglvl, BUGGID, ("   scheduling lb = 0x%x\n", lb))

    /* Get the VG volgrp ptr from device switch table */
    (void) devswqry( lb->b_dev, NULL, &vg );

    /* call the appropriate startup policy routine */
    rc = (*hd_schedsw[VG_DEV2LV(vg, lb->b_dev)->i_sched])(lb, ready_list, vg);
    if (rc == FAILURE) 
        {
        hd_pendqblked++;        /* Bump blocked count   */
        if (pending_Q.head == NULL)
                pending_Q.tail = lb;
        lb->av_forw = pending_Q.head;
        pending_Q.head = lb;
        break;                   /* let the pending_Q hang */
        }
    }

if (!pending_Q.head) 
    pending_Q.tail = NULL;

/* Start any MWC cache writes if any waiting */
if (hd_vg_mwc)
    hd_ca_wrt();
}


/*
 *  NAME:	reschedule
 * 
 *  FUNCTION:	Schedule previously blocked i/o requests (redrive pending_Q)
 *
 *  NOTES:	Invoked to redrive i/o after bad block or VGSA operation.
 *
 *  PARAMETERS:	none
 *
 *  RETURN VALUE: none
 *
 */
void  
reschedule()
{
struct buf  *ready_list=NULL;
int  int_lvl;

int_lvl = disable_lock(INTIODONE, &glb_sched_intlock);
hd_schedule(&ready_list);
unlock_enable(int_lvl, &glb_sched_intlock);

hd_start(ready_list);
}


/* ------------------------------------------------------------------------
 *
 *		S C H E D U L E R   S E R V I C E   F U N C T I O N S
 *
 * ------------------------------------------------------------------------
 */


/*
 *  NAME:         hd_avoid
 *
 *  FUNCTION:     Compute mask of mirrors to avoid.
 *
 *  NOTES:
 *	Compute mirror avoidance mask for this request.  Mirrors are
 *	avoided for several reasons.  They may be:
 *	    1)	explicitly masked by request options;
 *	    2)	non-existent for this logical volume; or
 *	    3)	currently unsynchronized for the blocks being requested.
 *
 *	If mirror resync is in progress, we must carefully avoid the
 *	mirrors included in the sync_mask for:
 *	    1)	reads at or beyond current sync_trk.
 *	    2)	writes beyond current sync_trk.
 *
 *	output:	returns mirror mask value.
 *
 *  PARAMETERS:   lb - logical requset buf struct
 *		  vg - VG volgrp pointer
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 *
 */
int
hd_avoid(
register struct buf	*lb,		/* logical request buf	*/
register struct volgrp	*vg)		/* VG volgrp ptr	*/
{
	register int	miravoid;	/* mirror avoidance mask to return */
	register int	p_no;		/* logical partition number	   */
	register int	rdwr;		/* 0-request is write 1-read	   */
	register struct lvol *lv;	/* logical volume stucture	   */
	register struct part *ppart=0;	/* primary pp structure	ptr	   */
	register struct part *spart=0;	/* secondary pp structure ptr	   */
	register struct part *tpart=0;	/* tertiary pp structure ptr	   */

	lv = VG_DEV2LV( vg, lb->b_dev );

	miravoid = X_AVOID(lb->b_options) | MIRROR_EXIST(lv->nparts);

	BUGLPR( debuglvl, BUGNTA,
	    ("hd_avoid: lb = 0x%x vg = 0x%x  miravoid = 0x%x\n",
							lb, vg, miravoid))

	/* A check must be done to verify that copies exists for each	*/
	/* logical partition and it is not stale.  One copy will	*/
	/* exist at this level since it was verified by hd_initiate().	*/
	/* Therefore no check is made to ensure the primary partition	*/
	/* is present.							*/

	p_no = BLK2PART(vg->partshift, lb->b_blkno);
	rdwr = lb->b_flags & B_READ;
	/* Primary mirror */
	ppart = PARTITION(lv, p_no, PRIMMIRROR);
	/*
	 * if request is a read then avoid any stale, reducing,
	 * or missing PV.
	 * 
	 * if request is a write then avoid any reducing or stale
	 * PP that is not changing from active to stale.
	 */
	if( rdwr ) { /* Read request */
	    if( (ppart->ppstate & (PP_STALE | PP_REDUCING)) ||
		(ppart->pvol->pvstate == PV_MISSING) ) {

		miravoid |= PRIMARY_MIRROR;
	    }
	}
	else { /* Write request */
	    if( (ppart->ppstate & PP_REDUCING) ||
		((ppart->ppstate & (PP_STALE | PP_CHGING)) == PP_STALE) ) {

		miravoid |= PRIMARY_MIRROR;
	    }
	}
	/*
	 * For the secondary & tertiary mirrors several other checks must made
	 * IF lv->nparts says the mirror should exist
	 * THEN
	 *	find parts structure
	 *	IF a pp is allocated to this mirror (i.e. no hole)
	 *	THEN
	 *	    check to see if it should be avoided
	 */
	/* Secondary mirror */
	if( !(miravoid & SECONDARY_MIRROR) ) {
	    spart = PARTITION(lv, p_no, SINGMIRROR);
	    if( spart->pvol ) {
		/*
		 * if request is a read then avoid any stale, reducing,
		 * or missing PV.
		 * 
		 * if request is a write then avoid any reducing or stale
		 * PP that is not changing from active to stale.
		 */
		if( rdwr ) { /* Read request */
		    if( (spart->ppstate & (PP_STALE | PP_REDUCING)) ||
			(spart->pvol->pvstate == PV_MISSING) ) {

			miravoid |= SECONDARY_MIRROR;
		    }
		}
		else { /* Write request */
		    if( (spart->ppstate & PP_REDUCING) ||
			((spart->ppstate & (PP_STALE|PP_CHGING))==PP_STALE)) {

			miravoid |= SECONDARY_MIRROR;
		    }
		}
	    }
	    else {
		miravoid |= SECONDARY_MIRROR;
		spart = NULL;
	    }
	}

	/* Tertiary mirror */
	if( !(miravoid & TERTIARY_MIRROR) ) {
	    tpart = PARTITION(lv, p_no, DOUBMIRROR);
	    if( tpart ->pvol ) {
		/*
		 * if request is a read then avoid any stale, reducing,
		 * or missing PV.
		 * 
		 * if request is a write then avoid any reducing or stale
		 * PP that is not changing from active to stale.
		 */
		if( rdwr ) { /* Read request */
		    if( (tpart->ppstate & (PP_STALE | PP_REDUCING)) ||
			(tpart->pvol->pvstate == PV_MISSING) ) {

			miravoid |= TERTIARY_MIRROR;
		    }
		}
		else { /* Write request */
		    if( (tpart->ppstate & PP_REDUCING) ||
			((tpart->ppstate & (PP_STALE|PP_CHGING))==PP_STALE)) {

			miravoid |= TERTIARY_MIRROR;
		    }
		}
	    }
	    else {
		miravoid |= TERTIARY_MIRROR;
		tpart = NULL;
	    }
	}

	/*
	 * If the request is a resync partition or mirror write
	 * consistency recover operation build the sync mask
	 */
	if( lb->b_options & (RESYNC_OP | MWC_RCV_OP) ) {
	    if( lb->b_options & MWC_RCV_OP ) {
		/*
		 * We want to recover all active mirrors with the contents
		 * of the most preferred good copy.  hd_resyncpp() will
		 * handle which mirror was used to read from and write to
		 * the others.
		 */
		ppart->sync_msk = (~miravoid) & ALL_MIRRORS;
	    }
	    else {
		ppart->sync_msk = 0;
		/* Primary mirror */
		if( ((ppart->ppstate & (PP_SYNCERR | PP_STALE | PP_REDUCING))
								== PP_STALE)){
		    if( ppart->pvol->pvstate != PV_MISSING )
			/* Resync this mirror */
			ppart->sync_msk |= PRIMARY_MIRROR;
		    else
			ppart->ppstate |= PP_SYNCERR;
		}

		/* Secondary mirror */
		if( (spart) &&
		    ((spart->ppstate & (PP_SYNCERR | PP_STALE | PP_REDUCING))
								== PP_STALE)){
		    if( spart->pvol->pvstate != PV_MISSING )
			/* Resync this mirror */
			ppart->sync_msk |= SECONDARY_MIRROR;
		    else
			spart->ppstate |= PP_SYNCERR;
		}

		/* Tertiary mirror */
		if( (tpart) &&
		    ((tpart->ppstate & (PP_SYNCERR | PP_STALE | PP_REDUCING))
								== PP_STALE)){
		    if( tpart->pvol->pvstate != PV_MISSING )
			/* Resync this mirror */
			ppart->sync_msk |= TERTIARY_MIRROR;
		    else
			tpart->ppstate |= PP_SYNCERR;
		}

		/* If not any mirrors to resync then return error to user */
		if( !(ppart->sync_msk) || (miravoid == ALL_MIRRORS) ) {
		    miravoid = ALL_MIRRORS;
		    ppart->sync_trk = NO_SYNCTRK;
		    ppart->ppstate &= ~PP_RIP;
		}
	    }
	}
	/*
	 * If the request is a write and a resync is in progress in this
	 * partition then check the address of the write.  If it is behind the
	 * resyncing track then don't avoid the mirror.  If it is in front
	 * then avoid the mirrors.
	 */
	else if( !rdwr && (ppart->sync_trk != NO_SYNCTRK) ) {
		/* current sync track and request track in partition */
		register s_trk = ppart->sync_trk;
		register r_trk = TRK_IN_PART(vg->partshift, lb->b_blkno);

		if ( r_trk < s_trk ) {
		    /*
		     * The request is behind the current sync track so
		     * write to all real mirrors unless it is reducing or
		     * has already had write error behing the sync track.
		     * In that case it should be marked as stale.  If
		     * all copies are being reduced then the request
		     * will fail because there is no copy to write 
		     * to.
		     */
		    if( !(ppart->ppstate & PP_REDUCING) &&
			!(ppart->ppstate & PP_SYNCERR) )

			miravoid &= ~PRIMARY_MIRROR;

		    if( (spart) && !(spart->ppstate & PP_REDUCING) &&
			!(spart->ppstate & PP_SYNCERR) )

			    miravoid &= ~SECONDARY_MIRROR;

		    if( (tpart) && !(tpart->ppstate & PP_REDUCING) &&
			!(tpart->ppstate & PP_SYNCERR) )

			    miravoid &= ~TERTIARY_MIRROR;
		}
	}

	BUGLPR( debuglvl, BUGGID,
	    ("   returning miravoid = 0x%x\n", miravoid))

	return(miravoid);
}

/*
 *  NAME:         hd_resyncpp
 *
 *  FUNCTION:     Resync stale partitions
 *
 *  NOTES:        This routine is the end of the first phase of a resync
 *		  operation.  The read phase and then it becomes the controling
 *		  point for the write phase.  If the read fails an attempt
 *		  to find another readable mirror is made.
 *
 *  PARAMETERS:   pb - physical requset buf struct
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 *
 */
void
hd_resyncpp(
register struct pbuf *pb)		/* physical device buf struct	*/
{
	register struct buf	*lb;		/* logical request buf	     */
	register struct lvol	*lv;		/* logical volume stucture   */
	register struct part	*ppart;		/* primary pp structure	ptr  */
	register int		p_no;		/* logical partition number  */
	register int		mirror;		/* mirror number	     */
        struct pbuf  		*ready_list=NULL;

	struct volgrp	*vg;			/* volume group stucture     */

	lb = pb->pb_lbuf;

	/* get volgrp ptr and LV ptr	*/
	(void) devswqry( lb->b_dev, NULL, &vg );
	lv = VG_DEV2LV( vg, lb->b_dev );

	/* Find the LP number and the primary parts structure */
	p_no = BLK2PART( vg->partshift, lb->b_blkno );
	ppart = PARTITION( lv, p_no, PRIMMIRROR );

	TRCHKL4T(HKWD_KERN_LVM | hkwd_LVM_RESYNCPP, pb, pb->pb.b_flags, p_no,
       		 (ppart->sync_trk<<16 | ppart->ppstate<<8 | ppart->sync_msk));

	if( pb->pb.b_flags & B_READ ) {	/* If a read check the results	*/
	    if( pb->pb.b_flags & B_ERROR ) { /* physical operation failed? */

 		/* remember we had an error on this partition	*/
 		pb->pb_mirbad |= MIRROR_MASK(pb->pb_mirror);

		/* if there is another mirror, retry the read */
		if( hd_seqnext(pb, vg) == SUCCESS )  {	/* mirror selected? */
		    pb->pb.b_flags &= ~B_ERROR;
		    hd_begin(pb, &ready_list, vg);
		}
		else {

		    /*
		     * No good mirrors to read from - error
		     * If a MWC recover mark all but the one
		     * mirror as stale and don't return an error.
		     * Avoided mirrors are either stale or on missing
		     * PVs.
		     */
		    if( lb->b_options & MWC_RCV_OP ) {
			pb->pb.b_flags &= ~B_ERROR;
 			pb->pb_mirbad |= pb->pb_miravoid;
			hd_stalepp( vg, pb );
		    }
		    else {
			/* Cancel resync operation */
			ppart->sync_trk = NO_SYNCTRK;
			ppart->ppstate &= ~PP_RIP;
			ppart->sync_msk = 0;
			hd_finished(pb);	/* all possible mirrors bad */
		    }
		}
	    }
	    else {	/* The read is complete now write to stale mirrors */

		/*
		 * if MWC recovery we will attempt to rewrite any mirrors
		 * that failed to read.  If we can't write them then declare
		 * them stale.
		 *
		 * the sync mask for MWC recovery was originally set by
		 * hd_miravoid.  All that has to be done now is turn off
		 * the bit corresponding to the mirror just read from.
		 */
		if( lb->b_options & MWC_RCV_OP ) {
		    ppart->sync_msk ^= MIRROR_MASK( pb->pb_mirror );
		}
		/* turn this request into a write */
		pb->pb.b_flags &= ~(B_ERROR|B_READ);

		/* select the first mirror that is stale/inconsistent  */
		mirror = FIRST_MASK(ppart->sync_msk);
		if( lb->b_options & MWC_RCV_OP ) {
		    /* pb_mirdone mirrors with write attempts/missing	*/
		    /* pb_mirbad  mirrors that failed failed to write	*/
		    /*		  or are on a missing pv		*/
		    pb->pb_mirbad = pb->pb_miravoid;
		    if( mirror == MAXNUMPARTS ) {
			/*
 			 * We have a partition with only one active PP
			 */
			pb->pb.b_flags |= B_READ;
			hd_stalepp( vg, pb );
			return;
		    }
		    pb->pb_mirdone = MIRROR_MASK( mirror ) | pb->pb_miravoid;
		}
		else {
		    /* pb_mirdone mirrors with completed resync tries	*/
		    /* pb_mirbad  mirrors that failed to resync		*/
		    pb->pb_mirdone = MIRROR_MASK( mirror );
		    pb->pb_mirbad = 0;
		}
		ppart->sync_msk &= ~MIRROR_MASK(mirror); /* mark done	*/
		hd_xlate(pb, mirror, vg); /* translate physical addr */
		hd_begin(pb, &ready_list, vg);
	    }
	}
	else {			/* Last physical operation was a write */
	    if( pb->pb.b_flags & B_ERROR ) { /* physical operation failed? */
		register struct part	*part;	/* part pp structure ptr   */

		/* set error flag in ppstate if a resync operation	*/
		if( lb->b_options & RESYNC_OP ) {
		    part = PARTITION( lv, p_no, pb->pb_mirror );
		    part->ppstate |= PP_SYNCERR;
		}
		pb->pb_mirbad |= MIRROR_MASK( pb->pb_mirror );
	    }
	    pb->pb.b_flags &= ~B_ERROR;

	    /* select the next mirror that is stale  */
	    mirror = FIRST_MASK(ppart->sync_msk);
	    if( mirror < MAXNUMPARTS ) {
		ppart->sync_msk &= ~MIRROR_MASK( mirror );/* this one done */
		pb->pb_mirdone |= MIRROR_MASK( mirror );
		hd_xlate( pb, mirror, vg );/* translate physical addr */
		hd_begin(pb, &ready_list, vg);
	    }
	    else {
		/* No more mirrors to do - turn request back to a read
		 * since the read worked then
		 * check for errors in resyncing/recovering mirrors
		 */
		pb->pb.b_flags |= B_READ;
		if( pb->pb_mirbad == pb->pb_mirdone ) {
		    /*
		     * If doing a consistency recovery and all writes 
		     * failed mark them all as stale.  Since we are here
		     * we have one good read so let it be the active one.
		     */
		    if( lb->b_options & MWC_RCV_OP ) {
			hd_stalepp( vg, pb );
		    }
		    else {
			/*
			 * If all mirrors failed to resync then return last 
			 * error to user and stop resyncing this partition
			 */
			pb->pb.b_flags |= B_ERROR;
			/* Cancel resync operation */
			ppart->sync_trk = NO_SYNCTRK;
			ppart->ppstate &= ~PP_RIP;
			hd_finished(pb);	/* all possible mirrors bad */
		    }
		}
		else {
		    /*
		     * If doing a consistency recovery and any writes
		     * failed.  Mark the mirrors as stale.  Don't return
		     * any errors since there is consistent data.
		     */
		    if( lb->b_options & MWC_RCV_OP ) {
			if( pb->pb_mirbad )
			    hd_stalepp( vg, pb );
			else
			    hd_finished( pb );
		    }
		    else {
			/*If this is the last LTG then go mark partition
			 * fresh. If not the last clean up after
			 * this request and return the request to the user. */
			if(ppart->sync_trk == (TRKPPART(vg->partshift) - 1)) {
			    hd_freshpp( vg, pb );
			}
			else {
			    ppart->ppstate &= ~PP_RIP;
			    ppart->sync_trk++;
			    hd_finished( pb );
			}
		    }
		}
	    }
	}
        hd_start(ready_list);
}

/*
 *  NAME:	hd_freshpp
 *
 *  FUNCTION:	Change PP state from stale to fresh(active) in VGSA.
 *
 *  NOTES:	This operation does not have a second half like hd_stalepp().
 *		After the VGSAs have ben updated the request is returned to
 *		the caller by the VGSA routine via pb_sched.
 *
 *		If a loss of quorum happens while this operation is in
 *		progress an error will be returned to the originator.
 *		This is done to tell the originator that the VGSAs may
 *		not indicate the stale PP(s) are now active.  The
 *		resync operation may have to be repeated.
 *
 *	input:	pb->pb_mirdone indicates the mirror(s) that should should
 *		be made fresh(active).
 *
 *  PARAMETERS:
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 *
 */
void
hd_freshpp(
register struct volgrp	*vg,		/* VG volgrp ptr		*/
register struct pbuf	*pb)		/* physical request buf		*/
{
	register struct buf	*lb;	/* logical request buf		*/
	register struct lvol	*lv;	/* logical volume stucture	*/
	register struct part	*part;	/* physical partition structure	*/
	register int		mirror;	/* mirror number		*/
	register int		mirmsk;	/* local copy of pb_mirdone	*/
	register int		p_no;	/* logical partition number	*/


	pb->pb_sched = hd_finished;	/* new scheduling policy */
	lb = pb->pb_lbuf;		/* access logical requests params */

	lv = VG_DEV2LV( vg, lb->b_dev );

	/* Compute partition number */

	p_no = BLK2PART(vg->partshift, lb->b_blkno);

	mirmsk = pb->pb_mirdone;
	while( (mirror = FIRST_MASK(mirmsk)) < MAXNUMPARTS ) {
	    mirmsk &= ~(MIRROR_MASK(mirror));

	    /* find mirror structure address */
	    part = PARTITION(lv, p_no, mirror);

	    /*
	     * If there have been any errors behind this request don't
	     * reactivate it.  Adjusting the pb_mirdone field will
	     * prevent the VGSA routines from changing it's status.
	     */
	    if( part->ppstate & PP_SYNCERR )
		pb->pb_mirdone &= ~(MIRROR_MASK(mirror));
	    else
		part->ppstate &= ~PP_STALE;

	} /* END of while( mirror < MAXNUMPARTS) */

	/*
	 * Reset resync operation control variables
	 */
	pb->pb.b_flags &= ~B_ERROR;	/* Since 1 worked no error  */
	part = PARTITION(lv, p_no, PRIMMIRROR);
	part->sync_trk = NO_SYNCTRK;
	part->ppstate &= ~PP_RIP;

	if( hd_sa_strt(pb, vg, SA_FRESHPP) == FAILURE ) {
	    /*
	     * loss of quorum just return an error
	     */
	    pb->pb.b_flags |= B_ERROR;
	    pb->pb.b_error = EIO;
	    pb->pb_addr = lb->b_baddr;
	    hd_finished( pb );
	}

	return;
}

/*
 *  NAME:         hd_mirread
 *
 *  FUNCTION:     Mirrored read physical operation end routine.
 *
 *  NOTES:        Used for both sequential and parallel mirror scheduling
 * 		  policies.
 *
 *		  If any changes are made to this routine a check should be
 *		  ensure the change is not needed in hd_resyncpp() since it
 *		  has code just like this.
 *
 *  PARAMETERS:   pb - physical requset buf struct
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 *
 */
void
hd_mirread(
register struct pbuf *pb)		/* physical device buf struct */
{
	struct volgrp	*vg;		/* volume group stucture	*/
        struct pbuf  *ready_list=NULL;

	/* get volgrp ptr */
	(void) devswqry( pb->pb_lbuf->b_dev, NULL, &vg );

	if (pb->pb.b_flags&B_ERROR)  {	/* physical operation failed? */

		/* if media error, mark this mirror for future fixup */
		if (pb->pb.b_error == EMEDIA)
			pb->pb_mirbad |= MIRROR_MASK(pb->pb_mirror);

		/* if there is another mirror, retry the read */
		if (hd_seqnext(pb, vg) == SUCCESS )  {	/* mirror selected? */
			pb->pb.b_flags &= ~B_ERROR;
		        hd_begin(pb, &ready_list, vg);
                        hd_start(ready_list);
		}
		else
			hd_finished(pb);	/* all possible mirrors bad */
	}

	/* this read done successfully, see if any earlier ones failed */
	else if (pb->pb_mirbad && 		/* earlier mirror broken? */
	  (((VG_DEV2LV(vg,pb->pb_lbuf->b_dev))->lv_options&LV_NOBBREL) == 0)) {
		    hd_fixup(pb);	/* rewrite broken mirrors */
	}

	/* worked first time, nothing more to do */
	else
		hd_finished(pb);

	return;
}


/*
 *  NAME:         hd_fixup
 *
 *  FUNCTION:     Rewrite broken mirrors to attempt bad blk relocation.
 *
 *  NOTES: 	  Most media errors are discovered on reads, but bad block
 *		  relocation only works during writes.  Mirroring provides
 *		  an opportunity to repair broken blocks.  When a media error
 *		  was encountered on a read, but the overall operation has
 *		  completed successfully, this routine is called to initiate
 *		  writes of the good data to the bad mirror(s).  This routine
 *		  sets itself up as a new scheduling policy, which runs until
 *		  all repairs have been attempted.  I/O errors encountered
 *		  during repair operations are ignored.  The the original
 *		  logical operation has still succeeded.
 *
 *	input:	pb->pb_mirbad contains mask of broken mirrors.
 *		buffer contains the data successfully read somewhere else.
 *	output:	starts a physical operation to rewrite the first broken mirror.
 *
 *  PARAMETERS:   pb - physical requset buf struct
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 *
 */
void
hd_fixup(
register struct pbuf *pb)		/* physical request buf */
{
	register int mirror;		/* mirror number */
	struct volgrp *vg;		/* volume group stucture	*/
        struct pbuf  *ready_list=NULL;

	/* get volgrp ptr */
	(void) devswqry( pb->pb_lbuf->b_dev, NULL, &vg );

	/* turn this request into a write, and ignore any rewrite errors */
	pb->pb.b_flags &= ~(B_ERROR|B_READ);
	pb->pb_sched = hd_fixup;	/* new scheduling policy */

	/* select the next broken mirror.  */
	mirror = FIRST_MASK(pb->pb_mirbad);	/* returns 3 if none masked */
	if (mirror == MAXNUMPARTS) {/* all broken mirrors rewritten? */
		hd_finished(pb);	/* operation completed successfully */
	}
	else
	{
		pb->pb_mirbad &= ~MIRROR_MASK(mirror);/* mark this one done */
		hd_xlate(pb, mirror, vg);/* translate physical address */
		hd_begin(pb, &ready_list, vg);
		hd_start(ready_list);
	}
}

/*
 *  NAME:	hd_stalepp
 *
 *  FUNCTION:	Change PP state from active to stale
 *
 *  NOTES:	If all active mirrors failed to write one mirror will be
 *		selected to remain active while the others are marked
 *		stale.  The last error will be returned to the originator.
 *
 *		If a loss of quorum happens while this operation is in
 *		progress(i.e. between here and hd_staleppe()) the 
 *		request will be returned with an error.  Any recovery
 *		operation will attempt to use the MWC cache.
 *
 *	input:	pb->pb_mirbad contains mask of stale mirrors.
 *
 *  PARAMETERS:
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 *
 */
void
hd_stalepp(
register struct volgrp	*vg,		/* VG volgrp ptr		*/
register struct pbuf *pb)		/* physical request buf		*/
{
	register struct buf	*lb;	/* logical request buf		*/
	register struct lvol	*lv;	/* logical volume stucture	*/
	register struct part	*part;	/* physical partition structure	*/
	register struct part	*ppart;	/* primary pp structure		*/
	register int		mirror;	/* mirror number		*/
	register int		mirmsk;	/* local mirror mask		*/
	register int		actmir;	/* selected active mirror	*/
	register int		stlmirs;/* current stale mirrors	*/
	register int		mpvmirs;/* mirrors on missing PVs	*/
	register int		nomirs;	/* mirrors that don't exist	*/
	register int		p_no;	/* logical partition number	*/
	register int		p_shift;/* partition size shift value	*/


	pb->pb_sched = hd_staleppe;	/* new scheduling policy */

	/* access logical requests parameters */
	lb = pb->pb_lbuf;
	lv = VG_DEV2LV( vg, lb->b_dev );

	/* compute the partition number and find its structure address */
	p_shift = vg->partshift;
	p_no = BLK2PART(p_shift, lb->b_blkno);
	ppart = PARTITION(lv, p_no, PRIMMIRROR);

	/*
	 * Build image of what the LP looked like on entry
	 */
	actmir = stlmirs = mpvmirs = 0;
	nomirs = mirmsk = MIRROR_EXIST( lv->nparts );
	while( mirmsk != ALL_MIRRORS ) {
	    mirror = FIRST_MIRROR( mirmsk );
	    mirmsk |= MIRROR_MASK( mirror );
	    part = PARTITION(lv, p_no, mirror);
	    if( part->pvol ) {
		if( part->ppstate & PP_STALE )
		    stlmirs |= MIRROR_MASK( mirror );

		if( (part->pvol) && (part->pvol->pvstate == PV_MISSING) )
		    mpvmirs |= MIRROR_MASK( mirror );
	    }
	    else
		nomirs |= MIRROR_MASK( mirror );
	}

	/*
	 * Adjust pb_mirbad so it does not have mirrors marked bad that
	 * don't exist, i.e. holes or whole copies.  On recovery operations
	 * pb_mirbad was set to pb_miravoid so mirrors on missing PVs would
	 * not have recovery attempted but they would be marked stale.
	 * BUT, pb_miravoid also contains mirrors that don't exists, so
	 * that will be factored out here
	 */
	pb->pb_mirbad &= (~nomirs);

	/*
	 * Now bitwise or all these things together.  If the result comes
	 * up equal to ALL_MIRRORS then there is a problem.  i.e. if
	 * we are not carefull we could mark all existing mirrors stale.
	 */
	mirmsk = stlmirs | pb->pb_mirbad | pb->pb_miravoid;
	if( mirmsk == ALL_MIRRORS ) {
	    /*
	     * Determine if there is a currently active mirror on a currently
	     * active PV.  Also, set the B_ERROR flag to tell the originator
	     * that the write failed if the request is not a MWC recovery
	     * operation.  We don't return errors on recovery operations unless
	     * the VG is being forced off or all copies are on missing PVs.
	     *
	     * NOTE The write may have actually worked to one mirror, BUT
	     *	    that mirror was marked stale by a previous request(stlmirs).
	     *	    Since this is first come first served algorithm this
	     *	    request must live with the current state of the LP.
	     *	    What that means is:
	     *
	     *		If there is currently just one mirror not stale
	     *		then that mirror remains the master(active) one
	     *		reguardless.  Even if this request failed to
	     *		write correctly to it and wrote successfully
	     *		to another mirror that was marked stale by a
	     *		previous request.
	     */
	    if( !(pb->pb.b_flags & B_ERROR) && !(lb->b_options & MWC_RCV_OP) ) {
		/*
		 * The b_resid count may not reflect the correct count
		 * when leaving through here.  That information is not
		 * kept by the pbufs as they finish, only pass or fail
		 * is kept at this point.  hd_finished() uses the
		 * buffer addresses between the lbuf and pbuf to 
		 * calculate the resid count.
		 */
		pb->pb.b_flags |= B_ERROR;
		pb->pb.b_error = EIO;
		pb->pb_addr = lb->b_baddr;
	    }
	    mirmsk = pb->pb_miravoid | stlmirs | mpvmirs;
	    if( mirmsk == ALL_MIRRORS ) {

		/* No active mirror on an active PV */
		actmir = FIRST_MIRROR( pb->pb_miravoid | stlmirs );
	    }
	    else {

		/* There is an active mirror on an active PV.  */
		actmir = FIRST_MIRROR( mirmsk );
	    }

	    /*
	     * At this point actmir holds the mirror number to remain
	     * active.  So turn off that pb_mirbad flag.
	     */
	    pb->pb_mirbad &= ~MIRROR_MASK( actmir );

	    /*
	     * If there is a resync-in-progress in this LP we have a
	     * VERY special case.  We have a write error in the mirror
	     * being used to do the resync.  Therefore, all other 
	     * mirrors must have the PP_SYNCERR flag turned on.  This
	     * will stop the resync operation and retain a mirror that
	     * was active before this request.
	     */
	    if( ppart->sync_trk != NO_SYNCTRK ) {
		mirmsk = ((~(pb->pb_miravoid)) & (~MIRROR_MASK(actmir))) &
			 ALL_MIRRORS;
		pb->pb_mirbad = mirmsk;
	    }
	}
	else {
	    /*
	     * Since there is at least one mirror that will remain active
	     * just mark the ones in pb_mirbad as stale in the parts
	     * structure.  Turn off the B_ERROR flag also.
	     *
	     * b_resid will be zeroed by hd_finished().
	     */
	    pb->pb.b_flags &= ~B_ERROR;
	}
	mirmsk = pb->pb_mirbad;
	while( (mirror = FIRST_MASK(mirmsk)) < MAXNUMPARTS ) {
	    mirmsk &= (~(MIRROR_MASK( mirror )));
	    part = PARTITION(lv, p_no, mirror);
	    /*
	     * If there is a resync operation currently active in this
	     * LP set the syncerr flag so the PP will stay stale.  i.e
	     * there is a write failure behind the LTG being resynced and
	     * the mirrors are no longer consistent.
	     */
	    if( ppart->sync_trk != NO_SYNCTRK )
		part->ppstate |= PP_SYNCERR;
	    /*
	     * if not already marked as stale make it so, otherwise 
	     * some other request has done it.  If the changing flag
	     * is off then the stale state has been recorded in all 
	     * the VGSAs.  Therefore, turn off the mirbad bit to 
	     * indicate there is no need for further action concerning
	     * this PP.
	     */
	    if( !(part->ppstate & PP_STALE) )
		    part->ppstate |= (PP_STALE | PP_CHGING);
	    else if( !(part->ppstate & PP_CHGING) )
		pb->pb_mirbad &= (~(MIRROR_MASK(mirror)));
	}
	/*
	 * If any mirbad bits still on then hang this request on the
	 * wheel.
	 */
	if( pb->pb_mirbad ) {
	    if( hd_sa_strt(pb, vg, SA_STALEPP) == FAILURE ) {
		/*
		 * VG is closing
		 */
		if( !(pb->pb.b_flags & B_ERROR) ) {
		    /*
		     * The b_resid count may not reflect the correct count
		     * when leaving through here.  That information is not
		     * kept by the pbufs as they finish, only pass or fail
		     * is kept at this point.  hd_finished() uses the
		     * buffer addresses between the lbuf and pbuf to 
		     * calculate the resid count.
		     */
		    pb->pb.b_flags |= B_ERROR;
		    pb->pb.b_error = EIO;
		    pb->pb_addr = lb->b_baddr;
		}
		hd_finished( pb );
	    }
	}
	else {
	    /*
	     * Nothing to mark stale.  Return request to orginator
	     */
	    hd_finished( pb );
	}

	return;
}

/*
 *  NAME:	hd_staleppe
 *
 *  FUNCTION:	Ending phase of changing PP state from active to stale
 *
 *  NOTES:	Request comes here after all VGSAs have been updated.  If
 *		a loss of quorum happens while this operation is in progress
 *		the request will be returned with an error.
 *
 *  PARAMETERS:
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 *
 */
void
hd_staleppe(
register struct pbuf *pb)		/* physical request buf		*/
{
    register struct buf		*lb;	/* logical request buf		*/
    register struct lvol	*lv;	/* logical volume stucture	*/
    register struct part	*part;	/* physical partition structure	*/
    register int		mirror;	/* mirror number		*/
    register int		mirmsk;	/* local mirror mask		*/
    register int		p_no;	/* logical partition number	*/

    struct volgrp *vg;		/* volume group stucture	*/

    lb = pb->pb_lbuf;
    /* get volgrp ptr */
    (void) devswqry( lb->b_dev, NULL, &vg );
    lv = VG_DEV2LV( vg, lb->b_dev );

    /* compute the partition number and find its structure address */
    p_no = BLK2PART(vg->partshift, lb->b_blkno);

    /*
     * Reset the pp changing flag for each PP that was marked stale
     */
    mirmsk = pb->pb_mirbad;
    while( (mirror = FIRST_MASK(mirmsk)) < MAXNUMPARTS ) {
	mirmsk &= (~(MIRROR_MASK( mirror )));
	part = PARTITION(lv, p_no, mirror);
	part->ppstate &= ~PP_CHGING;
    }

    hd_finished( pb );

    return;
}

/*
 *  NAME:	hd_xlate
 *
 *  FUNCTION:	Translate logical to physical address.
 *
 *  NOTES:
 *	input:	pb	- pbuf to translate.
 *		mirror  - mirror number of partition to use (0, 1 or 2).
 *		vg	- VG volgrp pointer
 *		partptr - ptr to return parts ptr
 *
 *	output:	pb->pb_mirror = mirror number selected.
 *		pb->pb.b_dev = physical device containing this mirror.
 *		pb->pb_start = starting physical block number within b_dev.
 *		pb->pb_pvol = pointer to pvol structure for physical device.
 *		pb->pb_part = pointer to parts structure for this request.
 *			      NOTE This field is only valid for the time
 *				   between now and when the request is issued
 *				   to the disk driver!  The parts structure
 *				   address can be changed by hd_config routines
 *				   while the request is in flight.
 *
 *  PARAMETERS:
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 *
 */
void
hd_xlate(
register struct pbuf	*pb,		/* physical request buf		*/
register int		mirror,		/* mirror number		*/
register struct	volgrp	*vg)		/* VG volgrp ptr		*/
{
	register struct buf *lb;	/* logical request buf		*/
	register int p_no;		/* logical partition number	*/
	register int p_shift;		/* partition size shift value	*/
	register struct lvol *lv;	/* logical volume stucture	*/
	register struct part *part;	/* physical partition structure	*/

	/* access logical requests parameters */
	lb = pb->pb_lbuf;

	BUGLPR( debuglvl, BUGNTA,
	    ("hd_xlate: pb = 0x%x lb = 0x%x mirror = 0x%x\n", pb, lb, mirror))

	lv = VG_DEV2LV( vg, lb->b_dev );

	/* compute the partition number and find its structure address */
	p_shift = vg->partshift;
	p_no = BLK2PART(p_shift, lb->b_blkno);	/* logical partition number */
	part = PARTITION(lv, p_no, mirror);

	/*
	 * fill in physical operation parameters
	 * NOTE: If these change hd_nearby() may need to change also
	 */
	pb->pb_mirror = mirror;
	pb->pb.b_dev = part->pvol->dev;
	pb->pb_start = part->start + lb->b_blkno - PART2BLK(p_shift, p_no);
	pb->pb_pvol = part->pvol;
	pb->pb_part = part;

	return;
}


/* ------------------------------------------------------------------------
 *
 *		R E G U L A R   N O N - M I R R O R E D   P O L I C Y
 *
 * ------------------------------------------------------------------------
 */


/*
 *  NAME:         hd_regular
 *
 *  FUNCTION:     Initiate regular, non-mirrored physical operation.
 *
 *  NOTES:
 *
 *  PARAMETERS:   lb - logical request buf struct
 *		  vg - volgrp structure ptr
 *		  ready_list - physical request list being built
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE:  SUCCESS if physical operation begun;
 *		   FAILURE if no physical buf structs remain.
 *
 */
int 
hd_regular(
register struct buf  *lb,
register struct pbuf  **ready_list,
register struct volgrp  *vg)
{
	register struct pbuf *pb;	/* physical request buf		*/

	BUGLPR( debuglvl, BUGNTA, ("hd_regular: lb = 0x%x vg = 0x%x\n", lb, vg))
	BUGLPR( debuglvl, BUGGID, ("   b_options = 0x%x\n", lb->b_options))

	/* Error off any attempt to avoid this only copy.
	 * This is for readx calls only
	 */
	if( X_AVOID(lb->b_options) & PRIMARY_MIRROR ) {
		lb->b_flags |= B_ERROR;
		lb->b_error = EIO;
		lb->b_resid = lb->b_bcount;
		hd_terminate( lb );
		return( SUCCESS );
	}

	/*
	 * If this is a special VGSA write then use the pbuf that is
	 * imbedded in the volgrp structure.
	 */
	if( lb->b_options & REQ_VGSA ) {
	    pb = &(vg->sa_pbuf);
	}
	else
	{
	    /* get a buf struct from the pool for the operation*/
	    if (hd_freebuf == NULL)		/* no buf's available?	*/
		return(FAILURE);		/* let pending_Q hang	*/
	    GET_PBUF(pb) 
	}

	pb->pb_lbuf = lb;
	pb->pb.b_flags = lb->b_flags;
	pb->pb_sched = hd_finished;	/* regular I/O done handler	*/

	hd_xlate(pb, PRIMMIRROR, vg);/* translate physical address	*/
	/*
	 * If the PP is being removed error off the request
	 */
	if( (pb->pb_part->ppstate & PP_REDUCING) ||
	    (pb->pb_pvol->pvstate == PV_MISSING) ) {

	    /* so hd_finished() will work correctly */
	    pb->pb_addr = lb->b_baddr;
	    pb->pb.b_flags |= B_ERROR;
	    pb->pb.b_error = EIO;
	    hd_finished( pb );

	}
	else
	    hd_begin(pb, ready_list, vg);

	return(SUCCESS);
}


/*
 *  NAME:         hd_finished
 *
 *  FUNCTION:     End of last physical operation for this logical request.
 *
 *  NOTES:
 *	output:	physical buf struct freed.
 *		logical request terminated.
 *
 *  PARAMETERS:   pb - physical request buf struct
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE:  none
 *
 */
void
hd_finished(
register struct pbuf *pb)		/* physical buf struct */
{
	register struct buf *lb;	/* logical buf struct */

	lb = pb->pb_lbuf;

	BUGLPR( debuglvl, BUGNTA,
	    ("hd_finished: pb = 0x%x lb = 0x%x\n", pb, lb))

	/* fatal error from physical device? */
	if (pb->pb.b_flags & B_ERROR)  {
		lb->b_flags |= B_ERROR;
		lb->b_error = pb->pb.b_error;
		lb->b_resid = lb->b_bcount - (pb->pb_addr - lb->b_baddr);
	}
	else
	   /*reinit resid since used when figuring pbuf before PVDD called*/
	   lb->b_resid=0;

	BUGLPR( debuglvl, BUGGID,
	  ("   b_flags = 0x%x b_error = 0x%x b_resid = 0x%x b_options = 0x%x\n",
			lb->b_flags, lb->b_error, lb->b_resid, lb->b_options))

	/*
	 * If this is a special VGSA write request there is no pbuf to
	 * release since it uses the inbedded one in the volgrp structure
	 */
	if( !(lb->b_options & REQ_VGSA) ) {
	    REL_PBUF( pb )
	}

	if( lb->b_options & REQ_IN_CACH )
	    hd_ca_term( lb );

	hd_terminate( lb );		/* terminate the logical operation */

	return;
}


/* ------------------------------------------------------------------------
 *
 *	S E Q U E N T I A L   W R I T E  M I R R O R E D   P O L I C Y
 *
 * ------------------------------------------------------------------------
 */


/*
 *  NAME:         hd_sequential
 *
 *  FUNCTION:     Initiate sequential mirrored physical operation.
 *
 *  NOTES:
 *
 *  PARAMETERS:   lb - physical request buf struct
 *		  vg - volgrp structure ptr
 *		  ready_list - physical request list being built
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: SUCCESS if physical operation begun;
 *		  FAILURE if no physical buf structs remain.
 *
 */
int 
hd_sequential(
register struct buf  *lb,
register struct pbuf  **ready_list,
register struct volgrp  *vg)
{
	register struct pbuf *pb;	/* physical request buf		*/
	register struct lvol *lv;	/* logical volume stucture	*/

	register int	mask;		/* mirror avoidance mask	*/

	/* check that a pbuf struct is available for the physical operation */
	if (hd_freebuf == NULL)			/* no buf's available?	*/
		return(FAILURE);		/* let pending_Q hang	*/

	/*
	 * Check to see if this request should be handed off to the MWCM
	 */
	lv = VG_DEV2LV( vg, lb->b_dev );
	mask = hd_avoid( lb, vg );

	if( mask == ALL_MIRRORS ) {
	    lb->b_flags |= B_ERROR;
	    lb->b_error = EIO;
	    lb->b_resid = lb->b_bcount;
	    if( lb->b_options & REQ_IN_CACH )
		hd_ca_term( lb );

	    hd_terminate( lb );	/* notify requestor		*/

	    return(SUCCESS);		/* scheduling may continue	*/
	}

	/*
	 * The REQ_IN_CACH flag indicates this request has already
	 * been cached, i.e. the second time through here.  The NO_MWC
	 * flag indicates the requests does not want mirror write
	 * consistency.  The LV_NOMWC flag means the entire LV does 
	 * not want mirror write consistency.  Finally there is no
	 * reason to worry about mirror write consistency if only
	 * one active copy exists.
	 */
	if( !(lb->b_flags & B_READ) &&
	    !(lb->b_options & (REQ_IN_CACH | NO_MWC)) &&
	    !(lv->lv_options & LV_NOMWC) &&
	     ( MIRROR_COUNT(mask) > 1) ) {

	    if( hd_ca_ckcach( lb, vg, lv ) == CA_MISS )
		return( SUCCESS );
	}

	GET_PBUF( pb )			/* Get pbuf to do physical req	*/

	/*
	 * If the request is a read and the read scheduling policy is
	 * parallel or the closest THEN select the closest copy ELSE
	 * select in primary, secondary, tertiary order.
	 */
	if( (lb->b_flags & B_READ) && (lv->i_sched == SCH_SEQWRTPARRD) ) {
	    hd_nearby( pb, lb, mask, vg, lv );
	    hd_begin(pb, ready_list, vg);
	}
	else {
	    pb->pb_lbuf = lb;
	    pb->pb.b_flags = lb->b_flags;
	    if( lb->b_options & (RESYNC_OP | MWC_RCV_OP) )
		pb->pb_sched = hd_resyncpp;
	    else
		pb->pb_sched = lb->b_flags & B_READ ? hd_mirread : hd_seqwrite;
	    pb->pb_mirdone = pb->pb_mirbad = 0;
	    pb->pb_miravoid = mask;

	    /* select first mirror and try to start the physical operation */
	    if (hd_seqnext(pb, vg) == SUCCESS )  {/* partition selected? */
	        hd_begin(pb, ready_list, vg);
	    }
	    else  {			/* all mirrors are broken or masked */
		pb->pb.b_flags |= B_ERROR;
		pb->pb.b_error = EIO;
		/* Dummy the address up so b_resid will be correct */
		pb->pb_addr = lb->b_baddr;
		hd_finished(pb);
	    }
	}
	return(SUCCESS);
}


/*
 *  NAME:         hd_seqnext
 *
 *  FUNCTION:     Select next sequential mirror.
 *
 *  NOTES:
 *	input:	pb->pb_miravoid contains mask of mirrors to avoid.
 *		pb->pb_mirdone contains mask of mirrors already done.
 *	output: physical request ready to start.
 *		pb->pb_mirdone updated to avoid the mirror selected.
 *
 *  PARAMETERS:   pb - physical request buf struct
 *		  vg - VG volgrp pointer
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: SUCCESS if another mirror was selected;
 *		  FAILURE if no more mirrors to try.
 *
 */
int
hd_seqnext(
register struct pbuf	*pb,		/* physical request buf */
register struct	volgrp	*vg)		/* VG volgrp pointer	*/
{
    register int mirror;		/* mirror number		*/
    register struct lvol *lv;		/* logical volume stucture	*/
    register struct part *ppart;	/* physical partition structure	*/
    register int	 p_no;		/* logical partition number	*/

    /*
     *  Select the next available mirror, subject to the restrictions
     *  of the avoidance and done masks.
     */
    if( pb->pb_lbuf->b_flags & B_READ ) {
	mirror = FIRST_MIRROR(pb->pb_mirdone|pb->pb_miravoid);
	if (mirror == MAXNUMPARTS)		/* all mirrors masked? */
	    return(FAILURE);

	pb->pb_mirdone |= MIRROR_MASK(mirror);	/* avoid mirror from now on */
	hd_xlate(pb, mirror, vg);		/* xlate physical address */
    }
    else {
	while( (mirror = FIRST_MIRROR(pb->pb_mirdone|pb->pb_miravoid))
							< MAXNUMPARTS ) {
	    pb->pb_mirdone |= MIRROR_MASK(mirror);/* avoid mir from now on */
	    hd_xlate(pb, mirror, vg);	/* xlate physical address */

	    /*
	     * If PP is stale and there is no resync in this partition or PV
	     * is missing set the pb_mirbad bit to indicate this mirror is
	     * broken and no longer consistent.
	     *
	     * NOTE This PP had to have been changing states when the original
	     *	    request went through hd_avoid() or is behind a resync
	     *	    operation that is in progress.  Otherwise, it would
	     *	    have been avoided by the pb_miravoid mask.
	     */

	    /* Find primary part structure to look at resync info */
	    lv = VG_DEV2LV( vg, pb->pb_lbuf->b_dev );
	    p_no = BLK2PART(vg->partshift, pb->pb_lbuf->b_blkno);
	    ppart = PARTITION(lv, p_no, PRIMMIRROR);
	    if( ((pb->pb_part->ppstate & PP_STALE) &&
					(ppart->sync_trk == NO_SYNCTRK)) ||
		(pb->pb_part->pvol->pvstate == PV_MISSING) ) {

		pb->pb_mirbad |= MIRROR_MASK( mirror );
	    }
	    else
		break;
	}
	if( mirror == MAXNUMPARTS )		/* all mirrors masked? */
	    return(FAILURE);
    }

    return(SUCCESS);
}


/*
 *  NAME:         hd_seqwrite
 *
 *  FUNCTION:     Mirrored sequential write physical operation end routine.
 *		  Writes to all mirrors are attempted before marking any
 *		  mirrors stale.
 *
 *  NOTES:
 *
 *  PARAMETERS:   pb - physical request buf struct
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 *
 */
void
hd_seqwrite(
register struct pbuf *pb)		/* physical device buf struct */
{
	struct volgrp *vg;		/* volume group stucture	*/
        struct pbuf  *ready_list=NULL;

	/* get volgrp ptr */
	(void) devswqry( pb->pb_lbuf->b_dev, NULL, &vg );

	/* if physical operation failed mark this mirror as broken */
	if ( pb->pb.b_flags & B_ERROR ) {
		pb->pb_mirbad |= MIRROR_MASK(pb->pb_mirror);
	}

	/* select next mirror and begin another physical write */
	if (hd_seqnext(pb, vg) == SUCCESS )  {	/* more partitions to write? */
		pb->pb.b_flags &= ~B_ERROR;
		hd_begin(pb, &ready_list, vg);
                hd_start(ready_list);
	}
	else  {				/* no more mirrors to try */
	    /*
	     * if there are no broken mirrors or the only writeable
	     * mirror fails finish up the request.
	     */
	    if( (pb->pb_mirbad == 0) || (MIRROR_COUNT(pb->pb_miravoid) == 1) )
		hd_finished(pb);
	    else
		hd_stalepp( vg, pb );
	}

	return;
}


/* ------------------------------------------------------------------------
 *
 *	P A R A L L E L   W R I T E  M I R R O R E D   P O L I C Y
 *
 * ------------------------------------------------------------------------
 */


/*
 *  NAME:         hd_parallel
 *
 *  FUNCTION:     Initiate parallel mirrored physical operations.
 *
 *  NOTES:
 *
 *  PARAMETERS:   lb - logical request buf struct
 *		  vg - volgrp structure ptr
 *		  ready_list - physical request list being built
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: SUCCESS if operation scheduled or terminated;
 *		  FAILURE if not enough physical buf structs
 *			to perform the entire operation.
 *
 */
int 
hd_parallel(
register struct buf  *lb,
register struct pbuf  **ready_list,
register struct volgrp  *vg)
{
	register struct pbuf	*pb;	/* physical request buf		 */
	register struct pbuf	*forw;	/* b_forw ptr from a pbuf	 */
	register struct lvol	*lv;	/* logical volume stucture	 */
	register struct part	*ppart;	/* physical partition structure	*/
	register int		p_no;	/* logical partition number	*/
	register int		mask;	/* mirror avoidance mask	 */
	register int		smask;	/* mirror avoidance mask	 */
	register int		mirror;	/* current mirror		 */
	register int		cnt=0;	/* count of pbufs built		 */
	register int		iss=0;	/* count of pbufs issued	 */

	struct pbuf *q;			/* queue of writes being started */

	/*
	 *  Compute mirror avoidance mask, and make sure that
	 *  there is at least one mirror available to perform
	 *  this operation.
	 */
	mask = hd_avoid(lb, vg);
	if (mask == ALL_MIRRORS)  {	/* no mirrors available? */
	    lb->b_flags |= B_ERROR;
	    lb->b_error = EIO;
	    lb->b_resid = lb->b_bcount;
	    if( lb->b_options & REQ_IN_CACH )
		hd_ca_term( lb );

	    hd_terminate( lb );		/* notify requestor	*/

	    return(SUCCESS);		/* scheduling may continue */
	}

	lv = VG_DEV2LV( vg, lb->b_dev );
	if (lb->b_flags & B_READ)  {	/* read operation? */

		/* get a pbuf struct for doing the read operation */
		if (hd_freebuf == NULL)
			return(FAILURE);	/* hang the pending_Q */
		GET_PBUF(pb)

		/*
		 * Look at read policy
		 */
		if( lv->i_sched == SCH_PARWRTSEQRD ) {
		    
		    pb->pb_lbuf = lb;
		    pb->pb.b_flags = lb->b_flags;
		    if( lb->b_options & (RESYNC_OP | MWC_RCV_OP) )
			pb->pb_sched = hd_resyncpp;
		    else
			pb->pb_sched = hd_mirread;
		    pb->pb_mirdone = pb->pb_mirbad = 0;
		    pb->pb_miravoid = mask;

		    /*
		     * select first mirror and try to start the physical
		     * operation
		     */
		    if (hd_seqnext(pb, vg) == SUCCESS )  {
	                hd_begin(pb, ready_list, vg);
		    }
		    else  {		/* all mirrors are broken or masked */
			pb->pb.b_flags |= B_ERROR;
			pb->pb.b_error = EIO;
			/* Dummy the address up so b_resid will be correct */
			pb->pb_addr = lb->b_baddr;
			hd_finished(pb);
		    }
		}
		else {
		    hd_nearby(pb, lb, mask, vg, lv);/* select a nearby mirror */
	            hd_begin(pb, ready_list, vg);
		}

	}  /* end -- read request */

	/* write operation processing: */
	else  {
	    /*
	     * The REQ_IN_CACH flag indicates this request has already
	     * been cached, i.e. the second time through here.  The NO_MWC
	     * flag indicates the requests does not want mirror write
	     * consistency.  The LV_NOMWC flag means the entire LV does 
	     * not want mirror write consistency.  Finally there is no
	     * reason to worry about mirror write consistency if only
	     * one active copy exists.
	     */
	    if( !(lb->b_options & (REQ_IN_CACH | NO_MWC)) &&
		!(lv->lv_options & LV_NOMWC) &&
		( MIRROR_COUNT(mask) > 1) ) {

		if( hd_ca_ckcach( lb, vg, lv ) == CA_MISS )
		    return( SUCCESS );
	    }
	    smask = mask;
	    q = NULL;
	    for( mirror = FIRST_MIRROR(mask); mask != ALL_MIRRORS;
		    mirror = FIRST_MIRROR(mask) )  {

		/* get a pbuf struct for this write operation */
		if (hd_freebuf == NULL)  {
			/*
			 *  If any pbuf's were already allocated,
			 *  we free them to avoid deadlocking.
			 *  Otherwise, multiple writes might each
			 *  acquire one and wait for more.
			 */
			hd_freeall(q);
			return(FAILURE);
		}

		GET_PBUF(pb)
		hd_append(pb, &q);	/* append pb to write queue */
		cnt++;			/* bump count built	    */

		/* set up physical request parameters */
		pb->pb_lbuf = lb;
		pb->pb.b_flags = lb->b_flags;
		pb->pb_sched = hd_parwrite;
		pb->pb_mirbad = 0;
		pb->pb_miravoid = smask;
		hd_xlate(pb, mirror, vg);

		/* cross this mirror off, go on to the next */
		mask |= MIRROR_MASK(mirror);

	    }  /* end -- loop for all available mirrors */

	    /* initiate the write for each pbuf in the work queue */
	    /* Find primary part structure to look at resync info */
	    pb = q;
	    p_no = BLK2PART(vg->partshift, pb->pb_lbuf->b_blkno);
	    ppart = PARTITION(lv, p_no, PRIMMIRROR);
	    while( cnt )  {
		/*
		 * If PP is stale or PV is missing set the pb_mirbad
		 * bit to indicate this mirror is broken and no longer
		 * consistent.  Then remove it from the circular list.
		 *
		 * NOTE This PP had to have been changing states when
		 *	the original request went through hd_avoid().
		 *	Otherwise, it would have been avoided by the
		 *	pb_miravoid mask.
		 */
		if( ((pb->pb_part->ppstate & PP_STALE) &&
					(ppart->sync_trk == NO_SYNCTRK)) ||
		    (pb->pb_part->pvol->pvstate == PV_MISSING) ) {

		    /*
		     * Remember this one broken and unlink it from
		     * list
		     */
		    forw = (struct pbuf *)pb->pb.b_forw;
		    forw->pb_mirbad |= (MIRROR_MASK( pb->pb_mirror ) |
					pb->pb_mirbad);
		    forw->pb.b_back = pb->pb.b_back;
		    pb->pb.b_back->b_forw = pb->pb.b_forw;
		    /*
		     * If the count is 1 and the issue is 0 then 
		     * we are going to fail the request because there
		     * is not copy to write to.
		     */
		    if( (cnt == 1) && (iss == 0) ) {
			/* no copy available to write to */
			pb->pb.b_flags |= B_ERROR;
			pb->pb.b_error = EIO;
			/*
			 * Dummy the address so b_resid will be
			 * correct
			 */
			pb->pb_addr = lb->b_baddr;
			hd_finished(pb);
			break;
		    }
		    else
			REL_PBUF( pb )
	    	}
		else {
		    iss++;	/* bump count of pbufs issued */
	            hd_begin(pb, ready_list, vg);
		}

		pb = (struct pbuf *) pb->pb.b_forw;
		cnt--;		/* decrement count built */
	    }

	}  /* end -- write request */

	return(SUCCESS);
}


/*
 *  NAME:         hd_freeall
 *
 *  FUNCTION:     Release all pbuf structs in the write queue.
 *
 *  NOTES:
 *
 *  PARAMETERS:   q - circualr list of pbuf's to free.
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 *
 */
void
hd_freeall(
register struct pbuf *q)		/* write request queue */
{
	register struct pbuf *pb;	/* physical request pbuf */

	if (q)  {
		pb = q;
		do  {
			REL_PBUF(pb)
    			pb = (struct pbuf *) pb->pb.b_forw;
		}  while(pb != q);
	}

	return;
}


/*
 *  NAME:         hd_append
 *
 *  FUNCTION:     Append pbuf to write queue.
 *
 *  NOTES:	  Append pb to a doubly-linked circular queue
 *  		  containing all the physical requests for this
 *  		  write operation.
 *
 *  PARAMETERS:   pb - physical buf struct
 *                qq - circualr list of pbuf's
 *			
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 *
 */
void
hd_append(
register struct pbuf *pb,		/* physical request pbuf */
register struct pbuf **qq)		/* Ptr to write request queue anchor */
{
	register struct pbuf *q;	/* write request queue anchor */

	q = *qq;
	if (q == NULL)  {	/* first request */
		pb->pb.b_forw = (struct buf *) pb;
		pb->pb.b_back = (struct buf *) pb;
		*qq = pb;
	}
	else  {
		pb->pb.b_forw = q->pb.b_back->b_forw;
		q->pb.b_back->b_forw = (struct buf *) pb;
		pb->pb.b_back = q->pb.b_back;
		q->pb.b_back = (struct buf *) pb;
	}

	return;
}


/*
 *  NAME:         hd_nearby
 *
 *  FUNCTION:     Setup pbuf to read a nearby mirror.
 *
 *  NOTES:	
 *	output:	pbuf set up to perform physical request.
 *		pb->pb_mirror = mirror number selected.
 *		pb->pb_mirdone = mask for the mirror selected.
 *		pb->pb_miravoid = mask of mirrors to avoid.
 *		pb->pb_sched -> hd_mirread or hd_resyncpp
 *		pb->pb_pvol -> pvol structure target for device.
 *
 *	Marks the mirror selected, so it will be skipped in the event the
 *	operation fails and hd_mirread() has to retry somewhere else.
 *
 *  PARAMETERS:
 *			
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 *
 */
void
hd_nearby(
register struct pbuf    *pb,            /* physical request pbuf         */
register struct buf     *lb,            /* logical request buf           */
register int            mask,           /* mirrors to avoid              */
register struct volgrp  *vg,            /* volume group stucture         */
register struct lvol    *lv)
{
        register int mirror, sel_mirror;/* current mirror, selected mirror */ 
	register daddr_t phys_addr;	/* physical address */
        register int p_no;              /* logical partition number      */
        register int blk_offset;        /* block offset within partition */
	struct part  *part;
        unsigned short  min_xfcnt, xfcnt;

        /* set up physical request parameters */
        pb->pb_lbuf = lb;
        pb->pb.b_flags = lb->b_flags;
        if( lb->b_options & (RESYNC_OP | MWC_RCV_OP) )
                pb->pb_sched = hd_resyncpp;
        else
                pb->pb_sched = hd_mirread;      /* I/O done scheduler policy */
        pb->pb_mirbad = 0;
        pb->pb_miravoid = mask;

        /* get the partition number and block offset */
        p_no = BLK2PART(vg->partshift, lb->b_blkno);
        blk_offset = lb->b_blkno - PART2BLK(vg->partshift, p_no);

        sel_mirror = mirror = FIRST_MIRROR(mask);
        part = PARTITION(lv, p_no, mirror);
        xfcnt = min_xfcnt = part->pvol->xfcnt;
        mask |= MIRROR_MASK(mirror);

        while ((xfcnt != 0) && (mask != ALL_MIRRORS))
            {
            mirror = FIRST_MIRROR(mask);
            part = PARTITION(lv, p_no, mirror);
            xfcnt = part->pvol->xfcnt;

            if (xfcnt < min_xfcnt)
                {
                sel_mirror = mirror;
                min_xfcnt = xfcnt; 
                }
            mask |= MIRROR_MASK(mirror);
            }

        phys_addr = part->start + blk_offset;
        pb->pb_mirror = sel_mirror;
        pb->pb_mirdone = MIRROR_MASK(sel_mirror);
        pb->pb.b_dev = part->pvol->dev;
        pb->pb_start = phys_addr;
        pb->pb_pvol = part->pvol;
}

/*
 *  NAME:         hd_parwrite
 *
 *  FUNCTION:     Parallel mirrored write physical operation done routine.
 *
 *  NOTES: 	  All the physical requests for this write operation have been
 *		  chained into a doubly-linked circular list.
 *
 *  PARAMETERS:   pb - physical buf struct
 *			
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 *
 */
void
hd_parwrite(
register struct pbuf *pb)		/* physical request buf */
{
	register struct pbuf *q;	/* physical request queue */

	struct volgrp *vg;		/* volume group stucture	*/

	/* if physical operation failed, mark this mirror as broken */
	if (pb->pb.b_flags&B_ERROR)  {
		pb->pb_mirbad |= MIRROR_MASK(pb->pb_mirror);
	}

	q = (struct pbuf *) pb->pb.b_forw;
	if (pb != q)  {			/* some request still outstanding? */
		q->pb_mirbad |= pb->pb_mirbad;
		q->pb.b_back = pb->pb.b_back;
		pb->pb.b_back->b_forw = pb->pb.b_forw;
		REL_PBUF(pb)
	}
	else {				/* last request completed */
	    /*
	     * if there are no broken mirrors or the only writeable
	     * mirror fails finish up the request.
	     */
	    if( (pb->pb_mirbad == 0) || (MIRROR_COUNT(pb->pb_miravoid) == 1) )
		    hd_finished(pb);
	    else {
		    /* get volgrp ptr */
		    (void) devswqry( pb->pb_lbuf->b_dev, NULL, &vg );
		    hd_stalepp( vg, pb );
	    }
	}

	return;
}

/* build_stripe_ready_list:  build sorted, physical stripe request list

	Invoked by hd_stripe after translation is completed.  This function
	receives a list of translated requests, initializes the remaining 
	physical buffer fields, and builds a sorted list of requests which
	are ready to pass to the disk driver(s). */
void
build_stripe_ready_list(
    struct volgrp  *vg,
    struct buf  *lb,
    struct pbuf *stripe_list, 
    struct pbuf  **ready_list)
{
struct pbuf  *pb, *head=NULL, *tail=NULL;

    /* process next request on list */
    tail = head = stripe_list;
    do  {
	/* if write request, copy write verify option from request and LV */
        if (!(head->pb.b_flags & B_READ)) 
            {
            head->pb.b_options = (lb->b_options & WRITEV);
            head->pb.b_options |= ((VG_DEV2LV(vg,lb->b_dev)->lv_options) &
                                   LV_WRITEV);
            }
        else
            head->pb.b_options = 0;

	/* initialize physical buffer fields */
        head->pb.b_iodone = (void (*)()) hd_end;
        head->pb.b_xmemd = lb->b_xmemd;
        head->pb.b_flags |= B_MPSAFE;
        head->pb_swretry = 0; 
        head->pb_bbop = 0; 
        head->pb_bbstat = 0;
        head->pb_type = 0;
        head->pb_whl_stop = 0;
        head->pb_part = NULL;
        head->pb.av_forw = NULL;
        head->orig_addr = head->pb.b_baddr;
        head->orig_count = head->pb.b_bcount;
        head->orig_bflags = head->pb.b_flags;
        head->pb_start = head->pb.b_blkno;

        /* translate bad blocks */
        if (hd_chkblk(head) == SUCCESS)
            hd_ready(head, ready_list);
        else  
            {
            /* simulate medium error */
            head->pb.b_flags |= B_ERROR;
            head->pb.b_error = EMEDIA;
            HD_SCHED(head);
            }
        head = head->stripe_next;
        } while (head != tail);
}

/* translation status from hd_stripe */
enum xlate_stat {XLATE_START, XLATE_PARTIAL, XLATE_COMPLETE, XLATE_FAIL};

/* hd_stripe:  striped i/o scheduling policy

	This function receives a logical request and builds a circular, 
	singly-linked list (stripe_next) of physical requests each of 
	which transfer a stripe-sized piece of the logical request.  These 
	requests are then built into a second sorted list (av_forw) and 
	returned in ready_list. */
int 
hd_stripe(
    struct buf  *lb,
    struct pbuf  **ready_list,
    struct volgrp  *vg)

{
    struct pbuf  *pb, *first_issued, *head=NULL, *tail=NULL;
    unsigned int  stripe_size, log_blkno, bytes_done, stripe_blks_left, 
                  blks_per_stripe, req_size;
    enum xlate_stat  status = XLATE_START;
    caddr_t  buf_addr, buf_st_addr, buf_end_addr;
    unsigned char  first_flag = TRUE;
    struct lvol  *lv;
    int  rc;

    lv = VG_DEV2LV(vg, lb->b_dev); 

    /* calculate stripe block size */
    stripe_size = 1 << lv->stripe_exp;
    blks_per_stripe = stripe_size >> DBSHIFT;

    /* calculate buffer address, device address, and byte count;
       b_resid is the byte count remaining to be transferred for this 
       logical request; b_resid tracks the true buffer address, device
       address, and byte count for partial translation processing */
    bytes_done = lb->b_bcount - lb->b_resid;
    buf_st_addr = lb->b_baddr + bytes_done;
    buf_end_addr = buf_st_addr + lb->b_resid;
    log_blkno = lb->b_blkno + BYTE2BLK(bytes_done);
    buf_addr = buf_st_addr;

    /* split and translate request at stripe block boundaries */
    if (hd_freebuf != NULL)
        {
        status = XLATE_PARTIAL;

        /* while physical buffers available and translation not complete */
        while ((hd_freebuf != NULL) && (status != XLATE_COMPLETE) && 
               (status != XLATE_FAIL))
            {
            GET_PBUF(pb)

            /* insert into singly linked list */
            if (head == NULL)
                head = tail = pb->stripe_next = pb;
            else
                {
                tail->stripe_next = pb;
                pb->stripe_next = head;
                tail = pb;
                }
            /* set physical request fields */
            pb->pb_lbuf = lb;
            pb->pb.b_flags = lb->b_flags;
            pb->pb_sched = hd_stripe_done;
            pb->pb.b_baddr = buf_addr;
            pb->stripe_status = STRIPE_NOTDONE;
            pb->first_issued = first_flag;
            pb->partial_stripe = FALSE;
            if (first_flag == TRUE)
                {
                first_issued = pb;
                first_flag = FALSE;
                }

            /* calculate bytes from this log_blkno to end of stripe */
            stripe_blks_left = blks_per_stripe - 
                               (log_blkno % blks_per_stripe);
            req_size = stripe_blks_left << DBSHIFT;

            /* determine if original request includes the rest of 
               this stripe */
            if ((buf_addr + req_size) > buf_end_addr)
                req_size = buf_end_addr - buf_addr;

            /* set request byte count and increment current 
               buffer address */
            pb->pb.b_bcount = req_size;
            buf_addr += req_size;

            if (buf_addr == buf_end_addr)
                status = XLATE_COMPLETE;

            /* translate logical device to physical device address */
            if (hd_stripe_xlate(vg->partshift+DBSHIFT, lv->stripe_exp, 
                                lv->striping_width, lv->parts[0], 
                                log_blkno, &(pb->pb.b_dev), 
                                &(pb->pb.b_blkno), &(pb->pb_part),
                                &(pb->pb_pvol)) == SUCCESS)
                /* increment logical address by disk blocks per stripe */
                log_blkno += req_size >> DBSHIFT;
            else
                status = XLATE_FAIL;

            /* striping to unaligned buffers on RS hw must be serialized 
               on reads to protect IOCC cache consistency */
            if (__power_rs() && ((unsigned int) lb->b_baddr & 0x3f) &&
                (lb->b_flags & B_READ))
                break;
            }
        }
    switch (status)
        {
        case XLATE_START:
            /* no pbufs, return request for requeueing */
            rc = FAILURE;
        break;

        case XLATE_PARTIAL:
            /* exhausted pbufs while translating, record this in 
               first issued */
            first_issued->partial_stripe = TRUE;

        case XLATE_COMPLETE:
            /* build physical requests for partial or 
               complete translation */
            build_stripe_ready_list(vg, lb, head, ready_list);
            rc = SUCCESS;
        break;

        case XLATE_FAIL:
            /* if translation failed, fail and terminate logical request */
            lb->b_flags |= B_ERROR;
            lb->b_error = EIO;
            lb->b_resid = lb->b_bcount;
            hd_terminate(lb);
            rc = SUCCESS;
        break;
        }
    return(rc);
}
/* release_stripe_pbufs:  release a stripe list of pbufs to the pbuf pool */
void
release_stripe_pbufs(
    struct pbuf  *pb)
{
    struct pbuf *tmp;

    /* release all pbufs on stripe_next list */
    tmp = pb;
    do {
       REL_PBUF(tmp);  
       tmp = tmp->stripe_next;
       } while (tmp != pb);
}

/* hd_stripe_done:  stripe done handler

	This function is invoked at iodone time for each stripe request 
	(pb_sched).  If all stripe requests for the logical request are 
	complete, this function terminates the logical request and returns 
	all associated pbufs.  If one or more stripe requests encountered 
	an error, the error from the request with the lowest logical address 
	is returned in the logical request error field.  If the logical
	request was only partially translated by the list of pbufs, then
	its resid field is modified to reflect the partial transfer and
	it is rescheduled for further translation. */ 
 
void
hd_stripe_done(
    struct pbuf  *pb)
{
    struct volgrp  *vg;
    struct lvol  *lv;
    struct buf  *lb;
    struct pbuf  *tmp, *first_issued = NULL;
    unsigned int  stripe_count = 0, bytes_xferred = 0;

    lb = pb->pb_lbuf;
    devswqry(lb->b_dev, NULL, &vg);
    lv = VG_DEV2LV(vg, lb->b_dev);

    pb->stripe_status = STRIPE_DONE;

    /* see if all stripe requests have completed */
    tmp = pb;
    do  {
        /* save pointer to first request issued */
        if (tmp->first_issued == TRUE)
            first_issued = tmp;

        /* quit if at least one stripe request still outstanding */
        if (tmp->stripe_status != STRIPE_DONE)
            break;

        /* increment total bytes transferred */
        bytes_xferred += tmp->pb.b_bcount;

        tmp = tmp->stripe_next;
        } while (tmp != pb);

    /* all stripe requests are complete */
    if (tmp->stripe_status == STRIPE_DONE)
        {
        /* search for first error encountered by these stripe requests */
        tmp = first_issued;
        do  {
            stripe_count++;
            if (tmp->pb.b_error & B_ERROR)
                break;
            tmp = tmp->stripe_next;
            } while (tmp != first_issued);

        /* if error found, set logical request error fields and terminate */
        if (tmp->pb.b_error & B_ERROR)
            {
            /* set error code and residual byte count */
            lb->b_flags |= B_ERROR;
            lb->b_error = tmp->pb.b_error;
            lb->b_resid = lb->b_bcount - (tmp->pb_addr - lb->b_baddr);
            release_stripe_pbufs(first_issued);
            hd_terminate(lb);
            } 
        /* no error, complete or continue (partial xlation) logical request */
        else 
            {
            release_stripe_pbufs(first_issued);

            /* if logical request was not completely xlated by stripe list */
            if (first_issued->partial_stripe == TRUE)
                {
                /* set logical resid field to reflect this partial transfer */
                lb->b_resid -= bytes_xferred;

                /* requeue adjusted logical request for rescheduling */
		hd_quelb(lb, &pending_Q);
                }
            /* logical request was completely transferred by stripe list */
            else
                {
                /* terminate successful, complete logical request */
                lb->b_flags &= ~B_ERROR;
                lb->b_error = 0;
                lb->b_resid=0;
                hd_terminate(lb);
                }
            }
        }
}

/* hd_stripe_xlate:  translate logical stripe request into physical request

	This function translates a logical block number of an LV into
	the corresponding physical disk address.  The partition size,
	stripe size and striping width are used to calculate the
	logical partition number and stripe number.   Stripe numbers
	identify stripe size segments of the logical address space and
	are uniformly distributed in the logical partitions of the LV.
        The partition number indexes the partition array which contains 
	the physical device address and block number of that physical 
	partition.  The stripe number is used to calculate the offset in 
	the physical partition at which that stripe begins.  Finally, the 
	logical block number of the request is used to calculate the offset 
	into that stripe at which the request begins. */
int
hd_stripe_xlate(
    unsigned int  part_exp,         /* 2**part_exp = partition size */
    unsigned int  stripe_exp,	    /* 2**stripe_exp = stripe block size */
    unsigned int  striping_width,   /* number of disks stripes span */
    struct part   *part_array,      /* partition array of logical volume */
    unsigned int  log_blkno,        /* logical block number to translate */
    dev_t         *device,          /* device on which stripe block resides */
    daddr_t       *block,           /* sector # corresponding to log_blkno */
    struct part   **partition,      /* xlated stripe blk part struct */
    struct pvol   **pvol)           /* xlated stripe blk pvol */
{
    unsigned int  stripe_num,	    /* stripe block number */
                  part_stripes,	    /* stripe blocks per partition */
                  set_stripes,	    /* stripe blocks per partition set */
                  set_num,	    /* partition set number */
                  set_part_num,	    /* partition number within set */
                  part_num,	    /* partition number */
                  stripe_start,	    /* starting sector # of stripe block */
                  stripe_exp_blks;  /* 2**stripe_exp_blks = # stripe disk blks*/
    int  status = SUCCESS;
        
              
    /* stripe size must be less than or equal to VG partition size */
    assert(part_exp >= stripe_exp);

    /* calculate stripe number and partition number */
    stripe_exp_blks = stripe_exp - DBSHIFT;
    stripe_num = log_blkno >> stripe_exp_blks;
    part_stripes = 1 << (part_exp - stripe_exp);
    set_stripes = part_stripes * striping_width;
    set_num = stripe_num / set_stripes;
    set_part_num = stripe_num % striping_width;
    part_num = (set_num * striping_width) + set_part_num;

    /* store pointer to partition and physical volume structures */
    *partition = &(part_array[part_num]);
    *pvol = part_array[part_num].pvol;

    /* if physical volume exists and partition is not marked for removal */
    if ((*pvol != NULL) &&
        (!(((*partition)->ppstate & PP_REDUCING) || 
           ((*pvol)->pvstate == PV_MISSING))))
        {
        /* store physical device number */
        *device = part_array[part_num].pvol->dev;

        /* calculate disk address of stripe */
        stripe_start = part_array[part_num].start + 
                       (((stripe_num / striping_width) % part_stripes) << 
                        stripe_exp_blks);

        /* calculate disk address of request (offset into stripe) */
        *block = stripe_start + (log_blkno % (1 << stripe_exp_blks));
        }
    else
        status = FAILURE;
return(status);
}
