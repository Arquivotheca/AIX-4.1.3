static char sccsid[] = "@(#)35	1.13.1.6  src/bos/kernext/lvm/hd_vgsa.c, sysxlvm, bos412, 9446A412a 10/25/94 00:35:00";

/*
 * COMPONENT_NAME: (SYSXLVM) Logical Volume Manager Device Driver - 35
 *
 * FUNCTIONS: hd_sa_strt, hd_sa_wrt, hd_sa_iodone, hd_sa_cont, hd_sa_hback,
 *	      hd_sa_rtn, hd_sa_whladv, hd_sa_update, hd_sa_qrmchk,
 *	      hd_sa_config, hd_bldpbuf, hd_sa_onerev, hd_reduce, hd_extend
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
 *  hd_vgsa.c -- LVM device driver Volume Group Status Area support routines
 *
 *
 *	These routines handle the volume Group Status Area(VGSA) used
 *	to maintain the state of physical partitions that are copies of each
 *	other.  The VGSA also indicates whether a physical volume is missing.
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
#include <sys/malloc.h>
#include <sys/sleep.h>
#include <sys/hd_psn.h>
#include <sys/dasd.h>
#include <sys/vgsa.h>
#include <sys/hd_config.h>
#include <sys/trchkid.h>
#include <sys/hd.h>

/*
 *  NAME:	hd_sa_strt
 *
 *  FUNCTION:	Process a new SA request.  Put the request on the hold list
 *		(sa_hld_lst).  If the wheel is not rolling start it.
 *
 *  NOTES:
 *
 *  PARAMETERS:
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: SUCCESS or FAILURE
 */
int
hd_sa_strt(
register struct pbuf	*pb,		/* physical device buf struct	   */
register struct volgrp	*vg,		/* volgrp pointer		   */
register int		type)		/* type of request		   */
{
	register struct pbuf	*hlst;	/* temporary sa_hld_lst ptr	   */
	register int		rc;	/* general function return code	   */
        register int  int_lvl;

	TRCHKL4T(HKWD_KERN_LVM | hkwd_LVM_SA_STRT,
		makedev(vg->major_num,0), pb, type, vg->flags);

        int_lvl = disable_lock(INTIODONE, &(vg->sa_intlock));
        
	/* if the VG is closing, don't start anything */
	if( vg->flags & VG_FORCEDOFF ) 
            {
            unlock_enable(int_lvl, &(vg->sa_intlock));
	    return( FAILURE );
            }

	/*
	 * If "pb" is NULL then this is a restart from the config routines.
	 * The config routines got control of the WHEEL but then found they
	 * did not change anything so they just want to restart it.
	 */
	if( pb ) {
	    /*
	     * Save the type of the request and hang it on the hold list
	     */
	    pb->pb_type = type;
	    pb->pb.av_forw = NULL;
	    if( vg->sa_hld_lst ) {
		/*
		 * Find end of list
		 */
		hlst = vg->sa_hld_lst;
		while( hlst->pb.av_forw )
		    hlst = (struct pbuf *)(hlst->pb.av_forw);
		hlst->pb.av_forw = (struct buf *)pb;
	    }
	    else
		vg->sa_hld_lst = pb;
	}

	/*
	 * Start the wheel if not rolling already
	 */
	if( !(vg->flags & (SA_WHL_ACT | SA_WHL_HLD)) ) {
	    vg->flags |= SA_WHL_ACT;

	    /*
	     * Generate a cross memory descriptor - see hd_sa_wrt()
	     * for reason why it is done here.
	     */
	    vg->sa_lbuf.b_xmemd.aspace_id = XMEM_INVAL;
	    rc = xmattach( vg->vgsa_ptr, sizeof( struct vgsa_area ), 
			&(vg->sa_lbuf.b_xmemd), SYS_ADSPACE);
	    ASSERT( rc == XMEM_SUCC );

	    hd_sa_cont( vg, 0 );
	}
        unlock_enable(int_lvl, &(vg->sa_intlock));
	return( SUCCESS );
}

/*
 *  NAME:	hd_sa_wrt
 *
 *  FUNCTION:	Build a buf structure to do logical IO to write the next
 *		SA on the wheel.
 *
 *  NOTES:
 *
 *  PARAMETERS:
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 */
void
hd_sa_wrt(
register struct volgrp	*vg)		/* volgrp pointer		   */
{
    register struct buf		*lb;	/* ptr to lbuf in volgrp struct	   */
    register int		widx;	/* VG wheel index		   */
    register int		rc;	/* function return code		   */
    struct   xmem		xmemd;	/* area to save the xmem descriptor*/
    struct pbuf  *ready_list=NULL;

    widx = vg->wheel_idx;
    /*
     * Save the cross memory descriptor then zero the buf structure
     * then stuff it with the necessary fields
     *
     * Saving the cross memory descriptor is faster than attaching/
     * detaching on each PV write.  This way we can attach when
     * the wheel is started and not detach until it stops.
     */

    lb = &(vg->sa_lbuf);
    xmemd = lb->b_xmemd;
    bzero( lb, sizeof(struct buf) );

    lb->b_flags		= B_BUSY;
    lb->b_iodone	= hd_sa_iodone;
    lb->b_dev		= makedev( vg->major_num, 0);
    lb->b_blkno		= GETSA_LSN( vg, widx );
    lb->b_baddr		= (caddr_t)(vg->vgsa_ptr);
    lb->b_bcount	= sizeof( struct vgsa_area );
    lb->b_options	= REQ_VGSA;
    lb->b_event		= EVENT_NULL;

    /* restore the cross memory descriptor */
    lb->b_xmemd = xmemd;

    /*
     * Save the wheel sequence number that is being written to this
     * VGSA
     */
    SETSA_SEQ( vg, widx, vg->whl_seq_num );

    /*
     * Call hd_regular() to translate the logical request then hd_start()
     * to issue it to the disk drivers.
     *
     * NOTE: hd_regular() will use the embedded pbuf in the volgrp 
     *	     structure, therefore it will never fail due to no
     *	     pbufs available.  This also means that LV0 does not
     *	     have to be open!
     */
    TRCHKL5T(HKWD_KERN_LVM | hkwd_LVM_SA_WRT, makedev(vg->major_num,0),
	vg->wheel_idx, vg->whl_seq_num, vg->pvols[widx>>1]->dev, lb->b_blkno);

    hd_regular(lb, &ready_list, vg);
    hd_start(ready_list);
}

/*
 *  NAME:	hd_sa_iodone
 *
 *  FUNCTION:	Return point for end of VGSA write operation.
 *
 *  NOTES:	Process any error on the write.  This means marking the
 *		PV as missing.  Then call hd_sa_cont() to start the next
 *		SA write if more to do.
 *
 *		If a PV is marked as missing there is no pbuf needed to
 *		remember when this happened.  BECAUSE, there is no 
 *		specific request waiting on any one particular SA write
 *		request.  THEREFORE, the only thing that must be done
 *		is to ensure the wheel keeps rolling for at least one more
 *		revolution from this point.  This is done by bumping the
 *		whl_seq_num variable.
 *
 *  PARAMETERS:
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 */
void
hd_sa_iodone(
register struct buf	*lb)		/* ptr to lbuf in VG just completed */
{
    register int	sa_updated = 0;	/* nonzero indicates SA updated	    */
    struct volgrp	*vg;	/* VG volgrp ptr from devsw table	*/
    int  int_lvl;		/* interrupt level save */

    TRCHKL5T(HKWD_KERN_LVM | hkwd_LVM_SA_IODONE, lb, lb->b_flags,
		lb->b_error, lb->b_dev, lb->b_blkno);

    /* get the volgrp ptr from device switch table	*/
    (void) devswqry( lb->b_dev, NULL, &vg );

    int_lvl = disable_lock(INTIODONE, &(vg->sa_intlock));

    lb->b_flags &= ~B_BUSY;
    lb->b_flags |= B_DONE;			/* since MP iodone() doesn't */

    /*
     * If error on write mark the PV missing
     */
    if( lb->b_flags & B_ERROR ) {
	/*
	 * Change pvstate to missing.  Set pvmissing flag in VGSA.  Check
	 * for quorum.  Change VGSA timestamp and sequence number.
	 * Log an error message concerning the missing PV.
	 *
	 */
	register struct pvol	*pvol;	/* ptr to pvol of missing pv	*/

	pvol = vg->sa_pbuf.pb_pvol;

	pvol->pvstate = PV_MISSING;
	SETSA_PVMISS( vg->vgsa_ptr, pvol->pvnum );
	(void) hd_sa_qrmchk( vg );
	sa_updated = 1;
	hd_sa_update( vg );

	/* Log failure of MWCC write */
	hd_logerr( (unsigned)ERRID_LVM_SA_WRTERR, (ulong)(pvol->dev),
		   (ulong)(lb->b_error), (ulong)0 );
    }

    /* Continue to next VGSA write */

    vg->wheel_idx = hd_sa_whladv(vg, vg->wheel_idx);
    hd_sa_cont( vg, sa_updated );
    unlock_enable(int_lvl, &(vg->sa_intlock));
    reschedule();
}

/*
 *  NAME:	hd_sa_cont
 *
 *  FUNCTION:	Continue writing VGSA areas
 *
 *  NOTES:	This function is used to start the wheel or keep it 
 *		rolling.  The only thing that stops the wheel once
 *		it is rolling is the whl_seq_num variables.  When the
 *		last write sa_seq_num matches the next one we are 
 *		complete.
 *
 *		If the VG is closing due to a loss of quorum then all
 *		active requests are returned with errors.  This will
 *		result in an error being returned with the original
 *		request.  Because of the loss of quorum we can not 
 *		guarantee the VGSA was updated with the correct information.
 *		Any user data will be recovered by the MWC cache.
 *
 *  PARAMETERS:
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 */
void
hd_sa_cont(
register struct volgrp	*vg,		/* volgrp pointer		    */
register int		sa_updated)	/* ptr to lbuf in VG just completed */
{

    register struct pbuf *hld_req;	/* ptr to request being moved to   */
					/* active list			   */
    register struct pbuf *alst;		/* temp sa_act_lst ptr		   */
    register struct buf	 **alst_forw;	/* address of sa_act_lst av_forw ptr*/
    register struct pbuf *new_req=NULL;	/* ptr to the first new request	   */
					/* that was put on the active list */
    register struct buf	 *alst_lb;	/* ptr to lbuf for active list pbuf*/
    register struct buf	 *hld_lb;	/* ptr to lbuf for hold request pbuf*/
    register int	 n_whl_idx;	/* new wheel index		   */
    register int 	 i;		/* general counter 		   */


    TRCHKL5T(HKWD_KERN_LVM | hkwd_LVM_SA_CONT, makedev(vg->major_num,0),
		vg->wheel_idx, vg->whl_seq_num, vg->flags, sa_updated);
    /*
     * Put the wheel on hold if some config process wants control of it and
     * that process is not waiting for the wheel to stop.  Then
     * wake that process up.  Said process will restart the wheel when
     * it is finished making it's changes
     *
     * *NOTE*	It is assumed the process has everything it needs in memory
     *		and it is all pinned.
     */


    if( (vg->config_wait != EVENT_NULL) && !(vg->flags & SA_WHL_WAIT) ) {
	vg->flags |= SA_WHL_HLD;
	vg->flags &= ~SA_WHL_ACT;
	xmdetach( &(vg->sa_lbuf.b_xmemd) );
	e_wakeup( &(vg->config_wait) );
	return;
    }

    /*
     * Move any requests currently on the hold list to the active list
     */
    while( vg->sa_hld_lst ) {

	/* Get pbuf at head of list */
	hld_req = vg->sa_hld_lst;
	vg->sa_hld_lst = (struct pbuf *)(hld_req->pb.av_forw);

	hld_req->pb.av_forw = NULL;
	hld_req->pb.av_back = NULL;

	/*
	 * Scan active list for any request that is doing the same
	 * type of request on the same PPs/PVs.  If one is found
	 * then hang this request on the av_back list.  Thus, this
	 * request will be allowed to continue when the head of the 
	 * av_back list is allowed.
	 */
	alst = vg->sa_act_lst;
	alst_forw = (struct buf **)(&(vg->sa_act_lst));
	/*
	 * Scan the active list until the end or we find a match
	 */
	while( hld_req && alst ) {
	    if( alst->pb_type != hld_req->pb_type ) {
		alst_forw = (struct buf **)(&(alst->pb.av_forw));
		alst = (struct pbuf *)(alst->pb.av_forw);
		continue;
	    }
	    switch( alst->pb_type ) {

		case SA_PVMISSING:
		case SA_PVREMOVED:

			/*
			 * Check the pvol addresses in the pbufs
			 */
			if( alst->pb_pvol == hld_req->pb_pvol ) {
			    /*
			     * We have a match.  Hang the new request
			     * on the av_back list.
			     */
			    hd_sa_hback( alst, hld_req );
			    hld_req = NULL;
			}
			break;

		case SA_STALEPP:

		    /*
		     * Check that the device number(b_dev) are the same
		     * in the corresponding lbufs.  Then that the LPs
		     * are the same. And finally the actual mirrors.
		     */
		    alst_lb = alst->pb_lbuf;
		    hld_lb = hld_req->pb_lbuf;
		    if( (alst_lb->b_dev == hld_lb->b_dev) &&
			(BLK2PART(vg->partshift, alst_lb->b_blkno) ==
			 BLK2PART(vg->partshift, hld_lb ->b_blkno)) ) {

			/*
			 * Check mirrors - if a mirror is stale on the
			 * active list pbuf but not in the new request
			 * pbuf count it as a match.  If the bits are
			 * reversed the new request must be put on the
			 * active list (av_forw) since it must wait
			 * for the PP to be marked as stale.
			 */
			for( i=0; i<MAXNUMPARTS; i++ ) {
			    if( (alst->pb_mirbad & (1 << i)) ^ 
				(hld_req->pb_mirbad & (1 << i)) ) {

				if( !(alst->pb_mirbad & (1 << i)) ) {
				    break;
				}
			    }
			}

			if( i == MAXNUMPARTS ) {
			    /*
			     * We have a match.  Hang the new request
			     * on the av_back list.
			     */
			    hd_sa_hback( alst, hld_req );
			    hld_req = NULL;
			}
		    }
		    break;

		case SA_FRESHPP:
		case SA_CONFIGOP:

		    /*
		     * Since there can only be one resync operation per 
		     * LP all fresh PP operations must be unique. 
		     * Therefore, we can go directly to the end of 
		     * the active list.
		     *
		     * The same thing holds true for config operations.
		     * There can only be one active in the VG at a time.
		     */
		    break;

		default:
		    panic("hd_sa_cont: unknown pbuf type");

	    } /* END switch on pb_type */
	    /*
	     * If the new request pointer is NULL then the request was
	     * put on the av_back list and we can carry on.  Otherwise,
	     * we must look further down the av_forw list.
	     */
	    if( hld_req ) {
		alst_forw = (struct buf **)(&(alst->pb.av_forw));
		alst = (struct pbuf *)(alst->pb.av_forw);
	    }
	} /* END while( hld_req && alst ) */
	/*
	 * If alst is NULL we are at the end of the active list.
	 * Put the new request on the list and modify the VGSA as per
	 * the type of request.
	 */
	if( !alst ) {

	    *alst_forw = (struct buf *)hld_req;
	    /*
	     * If the timestamp on the memory version of the VGSA has
	     * not been bumped do it now. Then remember the address of
	     * this first pbuf to be added to active list this pass.
	     */
	    if( !sa_updated ) {
		sa_updated = 1;
		hd_sa_update( vg );
	    }
	    if( !new_req ) new_req = hld_req;

	    switch( hld_req->pb_type ) {

		register struct lvol	*lv;	/* ptr to lvol stucture	  */
		register struct part	*part;	/* ptr to PP part stucture*/
		register ulong		lp;	/* request LP number	  */
		register ulong		pp;	/* mirror PP number	  */
		register int		mirrors;/* mirror mask for action */
		register int		i;	/* general		  */

		case SA_PVMISSING:
		case SA_PVREMOVED:

		    /*
		     * Change pvstate to missing.  Set pvmissing flag in
		     * VGSA.  (If removed PV, update the VG's quorum count
		     * before it is rechecked.)  Check the quroum.
		     * Log an error message concerning the missing/removed PV.
		     */
		    hld_req->pb_pvol->pvstate = PV_MISSING;
		    SETSA_PVMISS( vg->vgsa_ptr, hld_req->pb_pvol->pvnum );
		    if ( hld_req->pb_type == SA_PVREMOVED ) 
			vg->quorum_cnt = hld_req->pb.b_work;

		    /* Log PV discovered missing during Status Area write */

		    hd_logerr( (unsigned)ERRID_LVM_SA_PVMISS,
			(ulong)(hld_req->pb_pvol->dev), (ulong)0, (ulong)0 );

		    (void) hd_sa_qrmchk( vg );

		    break;

		case SA_STALEPP:
		case SA_FRESHPP:

		    /*
		     * For SA_STALEPP the pb_mirbad field in the pbuf
		     * indicates which mirrors should be marked as 
		     * stale.  For SA_FRESHPP the pb_mirdone field in
		     * the pbuf indicates which mirrors should be made
		     * fresh(active).
		     *
		     * Find the LV lvol structure and LP number of the
		     * logical request.
		     */
		    if( hld_req->pb_type == SA_STALEPP )
			mirrors = hld_req->pb_mirbad;
		    else
			mirrors = hld_req->pb_mirdone;
		    hld_lb = hld_req->pb_lbuf;
		    lv = VG_DEV2LV( vg, hld_lb->b_dev );
		    lp = BLK2PART( vg->partshift, hld_lb->b_blkno );
		    /*
		     * Now scan the mirrors bits and for each one that
		     * is set log an error message concerning the
		     * operation then set/reset corresponding
		     * bit in the in memory version of the VGSA.
		     */
		    while( mirrors ) {
			i = FIRST_MASK( mirrors );
			mirrors &= (~(MIRROR_MASK( i )));
			part = PARTITION( lv, lp, i );
			pp = BLK2PART( vg->partshift,
				    part->start - part->pvol->fst_usr_blk );
			if( hld_req->pb_type == SA_STALEPP ) {

			    /* Error log each pp made STALE (pp + 1) */
			    hd_logerr( (unsigned)ERRID_LVM_SA_STALEPP,
				(ulong)(part->pvol->dev),
				(ulong)(pp+1), (ulong)(hld_lb->b_dev));
			    SETSA_STLPP(vg->vgsa_ptr,part->pvol->pvnum,pp);
			}
			else {
			    CLRSA_STLPP(vg->vgsa_ptr,part->pvol->pvnum,pp);
			}
		    }

		    break;

		case SA_CONFIGOP:

		    /*
		     * No action needed on a hd_config routine request.
		     * the in memory version was modified when the wheel
		     * was put on hold and control passed to the config
		     * routines.
		     */

		    break;

		default:
		    panic("hd_sa_cont: unknown pbuf type");

	    } /* END of switch on pb_type */
	} /* END of if( !alst ) */
    } /* END while( sa_hld_lst ) */

    /*
     * At this point everything is on the active list and the appropriate
     * action taken.  If we have lost a quorum due to said action then
     * return all requests on the active lists with errors(ENXIO) if
     * they do not currently have an error indicated.  Before getting out
     * clear the active and hold flags and detach the VGSA memory area.
     */
    if( vg->flags & VG_FORCEDOFF ) {
	while( vg->sa_act_lst ) {
	    /* Get pbuf at head of list */
	    alst = vg->sa_act_lst;
	    vg->sa_act_lst = (struct pbuf *)(alst->pb.av_forw);
	    hd_sa_rtn( alst, RTN_ERR );
	}
	vg->flags &= (~(SA_WHL_ACT | SA_WHL_HLD));
	xmdetach( &(vg->sa_lbuf.b_xmemd) );
	/*
	 * If the wait flag is on then a config function is waiting for
	 * the wheel to stop.  So, inform that function that it has.  This
	 * is used so the varyoffvg function will wait, if the wheel is
	 * rolling, before removing the data structures.
	 */
	if( vg->flags & SA_WHL_WAIT ) {
	    vg->flags &= ~SA_WHL_WAIT;
	    e_wakeup( &(vg->config_wait) );
	}
	return;
    } /* END if( VG closing ) */

    if (!(vg->pvols[ (vg->wheel_idx) >> 1 ])  ||
        (vg->pvols[ (vg->wheel_idx) >> 1]->pvstate == PV_MISSING) ||
        !(GETSA_LSN(vg,vg->wheel_idx))) {
       vg->wheel_idx = hd_sa_whladv(vg, vg->wheel_idx);
    }

    while( new_req ) {
	new_req->pb_whl_stop = vg->wheel_idx; 
	new_req = (struct pbuf *)(new_req->pb.av_forw);
    }

    /*
     * Check to see if the current VGSA sequence number has been written
     * to the next VGSA.  If it has not then write it.  If it matches
     * then we have written the latest SA to all available VGSAs so
     * stop the wheel.
     */
    if( vg->whl_seq_num != GETSA_SEQ( vg, vg->wheel_idx )) hd_sa_wrt(vg);
    else {
	vg->flags &= ~SA_WHL_ACT;
	xmdetach( &(vg->sa_lbuf.b_xmemd) );
	/*
	 * If the wait flag is on then a config function is waiting for
	 * the wheel to stop.  So, inform that function that it has.  This
	 * is used so the varyoffvg function will wait, if the wheel is
	 * rolling, before removing the data structures.
	 */
	if( vg->flags & SA_WHL_WAIT ) {
	    vg->flags &= ~SA_WHL_WAIT;
	    e_wakeup( &(vg->config_wait) );
	}
    }
}

/*
 *  NAME:	hd_sa_hback
 *
 *  FUNCTION:	Hang a pbuf on the end of the given av_back list
 *
 *  NOTES:	This function is used to find the end of the given pbuf
 *		list via the av_back pointer.  Then, link the new pbuf
 *		on to the list there.  Assumes the av_back pointer in the
 *		new pbuf is NULL.
 *
 *  PARAMETERS:
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 */
void
hd_sa_hback(
register struct pbuf	*head_ptr,	/* head of pbuf list		    */
register struct pbuf	*new_pbuf)	/* ptr to pbuf to append to list    */
{
    while( head_ptr->pb.av_back )
	head_ptr = (struct pbuf *)(head_ptr->pb.av_back);

    head_ptr->pb.av_back = (struct buf *)new_pbuf;

    return;
}

/*
 *  NAME:	hd_sa_rtn
 *
 *  FUNCTION:	Return the given av_back list of request to their 
 *		respective caller.
 *
 *  NOTES:
 *
 *  PARAMETERS:
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 */
void
hd_sa_rtn(
register struct pbuf	*head_ptr,	/* head of pbuf list		    */
register int		err_flg)	/* if true return requests with	    */
					/* ENXIO error			    */
{
    register struct pbuf	*lst_ptr;	/* anchor for av_back list  */

    while( head_ptr ) {

	/*
	 * piggybacked requests are on the av_back chain
	 */
	lst_ptr = (struct pbuf *)(head_ptr->pb.av_back);
	/*
	 * if the request should be returned with an error but the
	 * B_ERROR flag is off TURN IT ON.  Dummy up address so it
	 * looks like none of the request worked.
	 */
	if( (err_flg == RTN_ERR) && (!(head_ptr->pb.b_flags & B_ERROR)) ) {
	    head_ptr->pb.b_flags |= B_ERROR;
	    head_ptr->pb.b_error = EIO;
	    head_ptr->pb_addr = head_ptr->pb_lbuf->b_baddr;
	}

	/* Set the B_DONE flag to indicate the request is done */
	head_ptr->pb.b_flags |= B_DONE;

	/*
	 * return the request via wakeup or function call
	 *
	 * it is possible for b_event to still be EVENT_NULL because of
	 * some error and pb_sched to be NULL.  If this condition exists
	 * just drop the request and the caller will see it is complete
	 * by checking the B_DONE
	 */
	TRCHKL5T(HKWD_KERN_LVM | hkwd_LVM_SA_RTN, head_ptr->pb.b_dev,
	    head_ptr->pb.b_blkno, head_ptr->pb.b_error, head_ptr->pb.b_flags,
	    head_ptr->pb_lbuf);

	if( head_ptr->pb.b_event != EVENT_NULL )
	    e_wakeup( &(head_ptr->pb.b_event) );
	else if( head_ptr->pb_sched )
	    HD_SCHED(head_ptr);

	/*
	 * get the next one off of the list
	 */
	head_ptr = lst_ptr;

    } /* END while( head_ptr ) */

    return;
}

/*
 *  NAME:	hd_sa_whladv
 *
 *  FUNCTION:	Advance wheel index to next VGSA
 *
 *  NOTES:	The wheel index has 2 components.  A primary/secondary
 *		bit, the low order bit of the index.  This controls which
 *		VGSA is being indexed on any particular PV.  The second
 *		component is the PV index.  It is the remaining bits of
 *		index.  It is used as the index into the pvols array in
 *		the volgrp structure.  This mechanism assumes that the
 *		maximum number of PVs in a VG is a power of 2.
 *		
 *		If MAXPVS is a power of 2 this function will be much
 *		more efficient.
 *
 *  PARAMETERS:
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: next VGSA on the wheel
 */
int
hd_sa_whladv(
register struct volgrp	*vg,		/* volgrp pointer		    */
register int		c_whl_idx)	/* current wheel index		    */
{
    int                      i;
    register struct pbuf    *alst;

    for ( i=0; i < MAXPVS * 2 ; i++ ) {
	
	c_whl_idx++;
	c_whl_idx %= (MAXPVS * 2);
	/*
	 * If no pvol pointer then advance index to next PV.
	 * If pvol pointer then look to see if there is a logical sector
	 * number associated with the index.  If so we have found the
	 * next VGSA index.  If not bump the index and look again.
	 */
	/* return the done requests */
	while ( vg->sa_act_lst) {
		if (vg->sa_act_lst->pb_whl_stop == c_whl_idx) {
    			alst = vg->sa_act_lst;
   			vg->sa_act_lst = (struct pbuf *)(alst->pb.av_forw);
   			hd_sa_rtn( alst, RTN_NORM );
		}
		else break;
	}

	if (vg->pvols[ c_whl_idx >> 1 ] && GETSA_LSN( vg, c_whl_idx )) {
		if (( vg->pvols[ c_whl_idx >> 1]->pvstate == PV_MISSING)  ||
		 			NUKESA(vg,c_whl_idx)==TRUE) {
			if (NUKESA(vg,c_whl_idx)==TRUE) {
			  SETSA_LSN( vg, c_whl_idx, 0);
			  SET_NUKESA( vg, c_whl_idx, FALSE);
			}
		}
	        else break;
	     }
    }
    return( c_whl_idx );
}

/*
 *  NAME:	hd_sa_update
 *
 *  FUNCTION:	Update the in memory version the the VGSA timestamps
 *		and sequence number.
 *
 *  NOTES:
 *
 *  PARAMETERS:
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 */
void
hd_sa_update(
register struct volgrp	*vg)		/* volgrp pointer		    */
{

	hd_gettime( &(vg->vgsa_ptr->b_tmstamp) );
	vg->vgsa_ptr->e_tmstamp = vg->vgsa_ptr->b_tmstamp;

	/* bump sequence number */
	vg->whl_seq_num++;

	return;
}

/*
 *  NAME:	hd_sa_qrmchk
 *
 *  FUNCTION:	Check the VG for a quorum of SAs
 *
 *  NOTES:	Count the number of active VGSAs.  If the count
 *		is less than the threshold(quorum_cnt) set the
 *		VG_FORCEDOFF flag so the VG will unwind and shutdown.
 *
 *  PARAMETERS:
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: count of active VGSAs
 */
int
hd_sa_qrmchk(
register struct volgrp	*vg)		/* volgrp pointer		    */
{
    register int	act_cnt;	/* count of active VGSAs	    */
    register int	idx;		/* PV index			    */

    /*
     * loop thru the pvols array in the volgrp structure
     */
    for( act_cnt=0, idx=0; idx < MAXPVS; idx++ ) {
	if( (vg->pvols[idx]) && (vg->pvols[idx]->pvstate != PV_MISSING) ) {
	    if( vg->pvols[idx]->sa_area[0].lsn )
		act_cnt++;
	    if( vg->pvols[idx]->sa_area[1].lsn )
		act_cnt++;
	}
    }

    /*
     * If the VG is already closing there is no need to do this all again
     */

    if( !(vg->flags & VG_FORCEDOFF) && (act_cnt < vg->quorum_cnt) ) {
      if ( !(vg->flags & VG_NOQUORUM) || (act_cnt == 0)) {
	vg->flags |= VG_FORCEDOFF;
	/* Error log loss of quorum VG is closing */
	hd_logerr( (unsigned)ERRID_LVM_SA_QUORCLOSE,
		(ulong)makedev(vg->major_num, 0), (ulong)(vg->quorum_cnt),
		(ulong)(act_cnt) );
      }
    }
    return( act_cnt );
}

/*
 *  NAME:	hd_sa_config
 *
 *  FUNCTION:	Interface for hd_config routines to access the
 *		VGSA wheel.
 *
 *  NOTES:	Assumes the hd_config routine has the VG lock.  
 *		Thus preventing more than one operation at a time.
 *				AND
 *		The arg variable(array) is in memory and PINNED.  Since
 *		this routine may be executed during offlevel interrupt
 *		processing it can not page fault or rely on any disk IO.
 *
 *		There are 3 phases to the hd_config routines modifying
 *		the VGSAs.
 *		     1.	Getting control of the wheel if it is rolling.
 *		     2.	Modifying the in memory VGSA.
 *		     3.	Restarting the wheel and waiting for one 
 *			revolution.
 *
 *		This function takes care of all of these for the caller.
 *
 *  PARAMETERS:
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: SUCCESS or FAILURE
 */
int
hd_sa_config(
register struct volgrp	*vg,		/* volgrp pointer		    */
register int		type,		/* type of hd_config request	    */
register caddr_t	arg)		/* ptr to arguments for the request */
{
    register struct pbuf	*pb;	/* ptr to a pbuf struct to use	    */
    register struct pvol	*pv;	/* ptr to target pvol struct	    */
    register struct cnfg_pp_state	*ppi;
    register struct cnfg_pv_ins 	*pv_info;
    register struct cnfg_pv_del		*pvdel_info;
    register struct cnfg_pv_vgsa	*vgsa_info;
    register struct pvol		*pvol;
    register int	o_prty = -1;	/* saved interrupt priority	    */
    register int	rc;		/* function return code		    */
    register int	i;		/* general counter		    */
    register struct sa_ext   *saext;    /* arg for HD_KEXTEND */
    register struct part    *oldpp;     /* old part structs */
    register struct part    *newpp;     /* new part structs */
    register struct sa_red  *sared;     /* arg for HD_KREDUCE */

    register int  clear_pv;		/* PV missing flags have changed   */
    register int  rollwheel;		/* indicates we should start wheel */
    register int  re_enable; 		/* shows a need to re-enable */
    register struct extred_part *pplist;/* ptr to pps to reduce */
    struct part *oldparts[MAXNUMPARTS]; /* ptrs to old part structs */
    register int ppcnt,cpcnt,ix;	/* for loop indexes */
    register struct part *pp, *pp1, *ppnew;/* ptrs to part structs in reduce */
    register short  ppnum;		/* pp number used in reduce */
    register int copy,cpymsk,lpmsk,redpps,stlpps,statechg;
				        /* mask variables */
    register int action;                /* maybe rollwheel and/or drainlv */
    register int leave_one_pp_active;
    int  int_lvl;		/* interrupt level save */
    

    TRCHKL3T(HKWD_KERN_LVM | hkwd_LVM_SA_CONFIG,
		makedev(vg->major_num,0), type, vg->flags);

    /* If the VG is closing return error */
    if (vg->flags & VG_FORCEDOFF)
	return( FAILURE );

    pb = (struct pbuf *)xmalloc(sizeof(struct pbuf),HD_ALIGN,pinned_heap);
    if( pb == NULL )
	return( FAILURE );

    rc = SUCCESS;

    int_lvl = disable_lock(INTIODONE, &glb_sched_intlock);

    /* Do what the caller wants */
    switch( type ) {

	case HD_KMISSPV:
	case HD_KREMPV:

	    /*
	     * Assumes that only one PV at a time can be marked as
	     * missing/removed.
	     */
	    pvdel_info = (struct cnfg_pv_del *) arg;

	    /* 
	     * zero out the DALV's LP on this PV 
	     */
	     bzero( pvdel_info->lp_ptr, pvdel_info->lpsize);

	    /*
	     * Go build a pbuf to give to the SA write routines.  This
	     * way they do all quorum checking and clean up.
	     * (If removing a PV, save the new quorum count in pbuf so 
	     * hd_sa_cont can update the VG's quourm count right before
	     * the quorum is rechecked.)
	     */
	    hd_bldpbuf( pb, (struct pvol *) pvdel_info->pv_ptr, type,
					 NULL, 0, NULL, NULL);
	    if ( type == HD_KREMPV ) {
		pb->pb.b_work = pvdel_info->qrmcnt;
		rc = hd_sa_strt( pb, vg, SA_PVREMOVED );
	    }
	    else
	     	rc = hd_sa_strt( pb, vg, SA_PVMISSING );
	    if( rc == FAILURE )
		break;
	
	    /*
	     * If the done flag is on at this point the pbuf has been
	     * completed and if we sleep the calling process will hang.
	     */
	    if( !(pb->pb.b_flags & B_DONE) )
                e_sleep_thread(&(pb->pb.b_event), &glb_sched_intlock,
                               LOCK_HANDLER);

	    /*
	     * If the error flag is set return FAILURE to the caller
	     */
	    if( pb->pb.b_flags & B_ERROR )
		rc = FAILURE;

	    break;

	case HD_KADDPV:
	    /*
	     * perform miscellaneous tasks that must be done disabled
	     */
	    pv_info = (struct cnfg_pv_ins *) arg;
	    pv = (struct pvol *)(pv_info->pvol);

	    if (vg->pvols[pv_info->pv_idx] == NULL) 
	       /* set pvol structure pointer for add of a new PV */
	       vg->pvols[pv_info->pv_idx] = pv;
	    else
	       /* copy new pvol data for add of a previously missing PV */
	       bcopy ((caddr_t)pv, (caddr_t)vg->pvols[pv_info->pv_idx], 
						sizeof(struct pvol));

	   if( vg->open_count != 0 )
	       hd_pvs_opn++;		/* bump number of open PVs	*/

	   /*
	    * If we're varying on the VG then return, 
	    * otherwise initialize the VGSA
	    * on this new PV via the WHEEL
 	    */
	   if (vg->flags & VG_OPENING) 
		break;

	    /*
	     * Get control of the wheel if it is rolling.
	     */
	    if( vg->flags & SA_WHL_ACT )
	        e_sleep_thread(&(vg->config_wait), &glb_sched_intlock, 
                               LOCK_HANDLER);

	    if( vg->flags & VG_FORCEDOFF ) {
		rc = FAILURE;
		break;
	    }

	    /* update VG's quorum count to include this new PV */
	    vg->quorum_cnt = pv_info->qrmcnt;	

	    /*
	     * initialize the SA_SEQ_NUM to a value that will
	     * make sure the VGSA on this new PV will be written,
	     * and then reset the PV missing flag in the memory
	     * copy of the VGSA.
	     */
	    if (pv->sa_area[0].lsn)
	       pv->sa_area[0].sa_seq_num = vg->whl_seq_num - 1;
	    if (pv->sa_area[1].lsn)
	       pv->sa_area[1].sa_seq_num = vg->whl_seq_num - 1;

	    CLRSA_PVMISS( vg->vgsa_ptr, pv->pvnum );

	    /*
	     * Now force the wheel one revolution.  Build a pbuf
	     * to give the the wheel, reset the SA holding flag,
	     * (re)start the wheel, wait for the wake up to signal
	     * the wheel has completed the operation, check status.
	     */
	    rc = hd_sa_onerev(vg, pb, type);
	    break;

      case HD_KEXTEND:
      case HD_KREDUCE:
	/*
	 * Get control of the wheel if it is rolling.
	 */
	if( vg->flags & SA_WHL_ACT )
	    e_sleep_thread(&(vg->config_wait), &glb_sched_intlock, 
                           LOCK_HANDLER);

	if( vg->flags & VG_FORCEDOFF ) {
	    rc = FAILURE;
	    break;
	}

	/* Now that the wheel is ours we can do what needs to be done. */
	switch( type ) {

        case HD_KEXTEND:
        
	    /*
	     * set up a pointer to the arguments passed in and loop     
	     * through the cnfg_pp_state structures to process the pps
	     * until we come to a ppstate that is CNFG_STOP
	     */
	    saext = (struct sa_ext *) arg;
	    for(ppi = saext->vgsa;(ppi->ppstate != CNFG_STOP);ppi++ ) {
		if((TSTSA_STLPP(vg->vgsa_ptr,ppi->pvnum,ppi->pp) ? STALEPP :
		   FRESHPP) != ppi->ppstate) {
		       XORSA_STLPP(vg->vgsa_ptr, ppi->pvnum,ppi->pp);
		       rollwheel = TRUE;
		}	
	    }
	    if(rollwheel == TRUE) {   /* we changed the VGSA */
		/*
		 * force the wheel one revolution. Build a pbuf to give
		 * to the wheel, reset the SA holding flag, (re)start
		 * the wheel, wait for the wake up to signal that the
		 * wheel has completed the operation, check status.
		 */
		hd_bldpbuf(pb, NULL, type, NULL, 0, NULL, NULL);
		vg->flags &= ~SA_WHL_HLD;
		rc = hd_sa_strt( pb, vg, SA_CONFIGOP );
		if( rc == FAILURE )
		    break;
		/*
		 * If the done flag is on at this point the pbuf has
		 * been completed and if we sleep, the calling process
		 * will hang.
		 */
		if( !(pb->pb.b_flags & B_DONE) )
	            e_sleep_thread(&(pb->pb.b_event), &glb_sched_intlock, 
                                   LOCK_HANDLER);

		/*
		 * If the error flag is set return FAILURE to the
		 * caller.
		 */
		if( pb->pb.b_flags & B_ERROR ) {
		    rc = FAILURE;
		    break;
		}
	    } /* end if roll wheel == TRUE */
	    /*
	     * call hd_extend() to check for resync in progress and to
	     * transfer the new lv information to the old lv information
	     */
	    rc = hd_extend(saext);
	    break;

	case HD_KREDUCE:
	    /* set up the needed pointers and variables */
	    sared = (struct sa_red *) arg;
	    rollwheel = FALSE;
	    pplist = sared->list;
	    for(i = 0; i < MAXNUMPARTS; i++) 
		oldparts[i] = sared->lv->parts[i];	
	    /*
	     * for the number of physical partitions being reduced, go through
	     * the logical partitions and build masks for the pps being
	     * reduced, pps that are stale, and the pps that exist; and,
	     * check that there are no resyncs in progress. Once the masks
	     * are built, go through and check that we aren't reducing the last
	     * good copy of the lp. After this, we have finished the validation
	     * phase and can then begin the process phase in which we
             * go through and turn on the PP_REDUCING bits and the 
	     * PP_STALE and PP_CHGING bits in the active pps that are being
	     * reduced.
	     */
            
	    for(ppcnt = 1; ppcnt <= sared->numred; pplist++, ppcnt++) {
		if(pplist->mask != 0) {
		   cpymsk = MIRROR_EXIST(sared->lv->nparts);
 	           lpmsk = stlpps = 0;
                   redpps = pplist->mask;
		   while(cpymsk != ALL_MIRRORS) {
		      copy = FIRST_MIRROR(cpymsk);
		      cpymsk |= MIRROR_MASK(copy);
	              pp = PARTITION(sared->lv,(pplist->lp_num - 1),copy);
	              if(pp->pvol) {
		         if(copy == 0) {
			    if(pp->sync_trk != NO_SYNCTRK) {
			       rc = FAILURE;
			       sared->error = CFG_SYNCER;
			       break;
		 	    }
		         }  
		         lpmsk |= MIRROR_MASK(copy);
		         if((pp->ppstate & (PP_STALE | PP_CHGING)) == PP_STALE) 
			    stlpps |= MIRROR_MASK(copy);
                      } /* end if there is a pvol in this part struct */
                   } /* end while */

	           if(rc == FAILURE)
		      break;

                   /*
		    * if we're not reducing all of the copies of this lp, check
		    * to be sure we're not reducing the last good copy 
	            */
	           if(redpps ^ lpmsk) {
		      /* if there are no good copies left */
		      if(!((stlpps | redpps) ^ lpmsk)) {   
		         rc = FAILURE;
		         sared->error = CFG_INLPRD;
		         break;
		      }
                      if ((lpmsk & ~stlpps & ~redpps) == 0 ) 
                              pplist->mask |= RM_ALL_ACTIVE_PPS;

                   } /* end if redpps ^ lpmsk */  
                   else
                      pplist->mask |= RM_ALL_ACTIVE_PPS;

               } /* end if */
            } /* end for */

	    if(rc == FAILURE)
		break;

	    /* now that we've validated the data, we can proceed */
            action = NOACTION;	
	    for(ppcnt = 1, pplist = sared->list; ppcnt <= sared->numred; 
						pplist++, ppcnt++) {

	      leave_one_pp_active = pplist->mask & RM_ALL_ACTIVE_PPS; 
	      pplist->mask &= ~RM_ALL_ACTIVE_PPS; 

	      for(cpymsk=pplist->mask; cpymsk; cpymsk &= ~MIRROR_MASK(copy)) {
	 	copy = FIRST_MASK(cpymsk);
	        pp = PARTITION(sared->lv,(pplist->lp_num - 1),copy);
		/* set reducing flag in any case */
		pp->ppstate |= PP_REDUCING;

	        if((pp->ppstate & (PP_STALE | PP_CHGING)) == PP_STALE)
                  /* state is not changing and it is stale, do nothing */
                  ;
		else
	        if(pp->ppstate &  PP_CHGING) {
		   /* partition is stale and changing */
		   action = ROLLWHEEL;
                }
		else { /* partition is active */
		   if (leave_one_pp_active) {
			leave_one_pp_active = 0;	
			if ( action == NOACTION) action = DRAINLV;
		   }
		   else {
		     pp->ppstate |= (PP_STALE | PP_CHGING);
		     ppnum = BLK2PART(vg->partshift,
		           (pp->start - pp->pvol->fst_usr_blk));
		     SETSA_STLPP(vg->vgsa_ptr,pp->pvol->pvnum,ppnum);
		     action = ROLLWHEEL;
		   }
		}	 
              } /* end for */ 
	    } /* end of outer for */
	    /* If we changed the VGSA */
            if (action == ROLLWHEEL) {
		/*
		 * force the wheel one revolution. Build 
		 * a pbuf to give to the wheel, reset the SA 
		 * holding flag, (re)start the wheel, wait for
		 * the wake up to signal the wheel has completed
		 * the operation, check status.
		 */
		hd_bldpbuf(pb, NULL, type, NULL, 0, NULL, NULL);
		vg->flags &= ~SA_WHL_HLD;
		rc = hd_sa_strt( pb, vg, SA_CONFIGOP );
		if( rc == FAILURE )
		    break;
		/*
		 * If the done flag is on at this point the
		 * pbuf has been completed and if we sleep the
		 * calling process will hang.
		 */
		if( !(pb->pb.b_flags & B_DONE) )
	            e_sleep_thread(&(pb->pb.b_event), &glb_sched_intlock,
                                   LOCK_HANDLER);
		
		/*
		 * If the error flag is set return FAILURE to
		 * the caller.
		 */
		if( pb->pb.b_flags & B_ERROR ) {
		    rc = FAILURE;
		    break;
		}
	        /*
                 * if the logical volume is open, then them
	         * drain the logical volume :  wait for all requests currently
	         * in the lv work queue to complete 
	         */  
                if(sared->lv->lv_status == LV_OPEN)
	            hd_quiet(makedev(vg->major_num,sared->min_num),vg);
             }
             else
	     if (action == DRAINLV) {
		if(vg->flags & SA_WHL_HLD) {
		   vg->flags &= ~SA_WHL_HLD;
		   rc = hd_sa_strt(NULL,vg,SA_CONFIGOP);
		   if(rc == FAILURE)
		      break;
                }
	        /*
                 * if the logical volume is open, then them
	         * drain the logical volume :  wait for all requests currently
	         * in the lv work queue to complete 
	         */  
                if(sared->lv->lv_status == LV_OPEN)
	            hd_quiet(makedev(vg->major_num,sared->min_num),vg);
	     }
	     else  /* action is NOACTION */

		if(vg->flags & SA_WHL_HLD) {
		   vg->flags &= ~SA_WHL_HLD;
		   rc = hd_sa_strt(NULL,vg,SA_CONFIGOP);
		   if(rc == FAILURE)
		      break;
                }
                 

	    /* reset the pplist pointer to the beginning of the list */
	    pplist = sared->list;
	    /*
	     * call hd_reduce()	to handle promotions and to transfer the
	     * new lv information to the old lv information
	     */
	    hd_reduce(sared,vg);  
	    break;
	} /* END of switch( type ) */
               
	break;

	case HD_KDELPV:

	    pvdel_info = (struct cnfg_pv_del *) arg;
	    pv = (struct pvol *)(pvdel_info->pv_ptr);

	    /* update the VG qourum count  ---
	     * For a PV to be deleted, NO partitions may be allocated, 
             * therefore, we don't have to be as carefull here as we 
             * are with REMOVEPV when we update the quorum count.
             */
	    vg->quorum_cnt = pvdel_info->qrmcnt;

	    /* zero out the VG's pvol ptr */
	    vg->pvols[ pv->pvnum ] = NULL;

	    /*
	     * Miscelaneous updates that must be made disabled:
	     * delete the DALV's LP on this PV, decrement the global
	     * PV open count, and update the VG's quorum count
	     */
	    bzero ( pvdel_info->lp_ptr, pvdel_info->lpsize);
	    if ( vg->open_count != 0 )
		hd_pvs_opn--;

	    break;

	case HD_KADDVGSA:
	case HD_KDELVGSA:
	
	    vgsa_info = (struct cnfg_pv_vgsa *) arg;
	    pv = vgsa_info->pv_ptr;

	    vg->quorum_cnt = vgsa_info->qrmcnt;

	    if (type == HD_KADDVGSA) {
		/* 
		 * ADDING VGSA(s) to this PV - fill in the VGSA LSNs
		 * and change the PV's VGSA sequence number so this 
		 * PV's vgsas will be written.
		 */
		if (vgsa_info -> sa_lsns[0]) {
		    pv->sa_area[0].lsn = vgsa_info -> sa_lsns[0];
		    pv->sa_area[0].sa_seq_num = vg->whl_seq_num - 1;
		}
		if (vgsa_info -> sa_lsns[1]) {
		    pv->sa_area[1].lsn = vgsa_info -> sa_lsns[1];
		    pv->sa_area[1].sa_seq_num = vg->whl_seq_num - 1;
		}

		/*
		 * get control of the wheel and wait for it to run 
		 * one full revolution.
		 */
	        if( vg->flags & SA_WHL_ACT )
	            e_sleep_thread(&(vg->config_wait), &glb_sched_intlock,
                                   LOCK_HANDLER);

	        if( vg->flags & VG_FORCEDOFF ) {
		    rc = FAILURE;
		    break;
	        }
		rc = hd_sa_onerev(vg, pb, type);
	    }
	    else {	
		/* 
		 * DELETING VGSA(s) from this PV - if the wheel is active,
		 * get control of it, set the flag for the VGSA(s) being
		 * deleted, and then wait for the wheel to run one 
		 * revolution (the LVDD code that runs the wheel will zero
		 * out the VGSA LSN when the nukesa flag is set).
		 * If the wheel is NOT active, then just zero out the VGSA
		 * LSN's now.
		 */
	        if( vg->flags & SA_WHL_ACT ) {

	            e_sleep_thread(&(vg->config_wait), &glb_sched_intlock,
                                   LOCK_HANDLER);

	            if( vg->flags & VG_FORCEDOFF ) {
		        rc = FAILURE;
		        break;
	            }
	
		    if (vgsa_info -> sa_lsns[0]) 
		        pv->sa_area[0].nukesa = TRUE;
		    if (vgsa_info -> sa_lsns[1]) 
		        pv->sa_area[1].nukesa = TRUE;

		    rc = hd_sa_onerev(vg, pb, type);
		}
		else { 	/* the wheel is NOT rolling */
		    if (vgsa_info -> sa_lsns[0]) 
		        pv->sa_area[0].lsn = 0;
		    if (vgsa_info -> sa_lsns[1]) 
		        pv->sa_area[1].lsn = 0;
		}

	    }

	    break;
		
        case HD_MWC_REC:
	    /*
	     * Just update the VGSA:
	     * get control of the wheel and wait for it to run 
	     * one full revolution.
	     */
	    if( vg->flags & SA_WHL_ACT )
	        e_sleep_thread(&(vg->config_wait), &glb_sched_intlock,
                               LOCK_HANDLER);

	    if( vg->flags & VG_FORCEDOFF ) {
		rc = FAILURE;
  	        break;
	    }
	    rc = hd_sa_onerev(vg, pb, type);
	    break;


	default:
	    panic("hd_sa_config: unknown request type");


    } /* END of switch( type ) */

    unlock_enable(int_lvl, &glb_sched_intlock);

    /* Give back the memory we borrowed for the pbuf struct */
    assert(xmfree(pb,pinned_heap) == LVDD_SUCCESS);

    return( rc );
}

/*
 *  NAME:         hd_sa_onerev
 *
 *  FUNCTION:     Force the WHEEL one revolution to update the VGSA       
 *		  on all active PVs
 *
 *  NOTES:
 *
 *  PARAMETERS:   vg   - pointer to volume group
 *		  pb   - pbuf pointer
 *		  type - type of VGSA config operation
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 *
 */

int
hd_sa_onerev(
register struct volgrp    *vg,		/* ptr to volgrp struct		*/
register struct pbuf      *pb,		/* ptr to pbuf struct 		*/
register int		  type)		/* type of pbuf to build	*/
{

	register int rc;		

	/*
	 * Now force the wheel one revolution.  Build a pbuf
	 * to give the the wheel, reset the SA holding flag,
	 * (re)start the wheel, wait for the wake up to signal
	 * the wheel has completed the operation, check status.
	 */
	hd_bldpbuf( pb, NULL, type, NULL, 0, NULL, NULL);
	vg->flags &= ~SA_WHL_HLD;
	rc = hd_sa_strt( pb, vg, SA_CONFIGOP );
	if( rc == FAILURE )
	    return(rc);
	/*
	 * If the done flag is on at this point the pbuf has been
	 * completed and if we sleep the calling process will hang.
	 */
	if( !(pb->pb.b_flags & B_DONE) )
	    e_sleep_thread(&(pb->pb.b_event), &glb_sched_intlock, LOCK_HANDLER);

	/*
	 * If the error flag is set return FAILURE to the caller
	 */
	if( pb->pb.b_flags & B_ERROR )
	    rc = FAILURE;

	return(rc);
}



/*
 *  NAME:         hd_bldpbuf
 *
 *  FUNCTION:     Initialize a pbuf structure for LVDD disk io.
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
hd_bldpbuf(
register struct pbuf    *pb,		/* ptr to pbuf struct		*/
register struct pvol	*pvol,		/* target pvol ptr		*/
register int		type,		/* type of pbuf to build	*/
register caddr_t	bufaddr,	/* data buffer address - system	*/
register unsigned	cnt,		/* length of buffer		*/
register struct xmem	*xmem,		/* ptr to cross memory descripto*/
					/* point to function returning void */
register void		(*sched)(register struct pbuf *))	
{
    register struct buf   *lb;		/* ptr to buf struct part of pbuf*/

    /*
     * Zero the pbuf then stuff it with the necessary fields
     */
    bzero( pb, sizeof(struct pbuf) );

    lb = (struct buf *)pb;
    if( pvol )
	lb->b_dev = pvol->dev;

    lb->b_baddr = bufaddr;
    lb->b_bcount = cnt;
    lb->b_event = EVENT_NULL;
    if( xmem )
	lb->b_xmemd = *xmem;

    pb->pb_sched = sched;
    pb->pb_pvol  = pvol;

    switch( type ) {

	/* mirror write consistency cache write type */
	case CATYPE_WRT:

	    lb->b_iodone = hd_ca_end;
	    lb->b_flags = B_BUSY | B_NOHIDE;
	    lb->b_blkno = PSN_MWC_REC0;
	    break;

	case HD_MWC_REC:
	case HD_KMISSPV:
	case HD_KREMPV:
	case HD_KREDUCE:
	case HD_KEXTEND:
	case HD_KADDPV:
	case HD_KDELPV:
        case HD_KADDVGSA:
        case HD_KDELVGSA:

	    lb->b_iodone = NULL;
	    lb->b_flags = B_BUSY;
	    break;

	default:
	    panic("hd_vgsa: unknown pbuf type");
	    break;
    }

    return;
}

/*
 *  NAME:	hd_extend
 *
 *  FUNCTION:   Transfers old part struct information to new part struct
 *	        information.
 *
 *  NOTES:	
 *
 *  PARAMETERS:  saext  pointer to information structure for the extend
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: SUCCESS or FAILURE
 */

int
hd_extend(
   struct sa_ext *saext)  /* pointer to extend information structure */


{
    register int lpi,cpi;		/* loop counters */
    register int rc;			/* return code */
    register struct part *oldpp; 	/* pointer to old part struct */
    register struct part *newpp; 	/* pointer to new part struct */

    /*
     * for the old number of logical partitions on the
     * logical volume, go through and search each possible
     * old copy. If the logical partition is not being
     * resynced, put the old part struct information
     * into the new part struct entry
     */
    rc = SUCCESS;
    for(lpi = 0; lpi < saext->old_numlps; lpi++) {
	for(cpi = 0; cpi < saext->old_nparts; cpi ++) {
	    if(saext->klv_ptr->parts[cpi] != NULL) {
		oldpp = (struct part *)(saext->klv_ptr->parts[cpi]+lpi);
		if(oldpp->pvol != NULL) {
		    if(cpi == 0) {
			if(oldpp->sync_trk != NO_SYNCTRK) {
			    saext->error = CFG_SYNCER;
			    rc = FAILURE;
			    break;
			}
		    } 
		    newpp = (struct part *)
				(*(saext->new_parts + cpi) + lpi);
		    *newpp = *oldpp;
		} /* end if oldpp->pvol != NULL */  
	    } /* end if klv_ptr->parts != NULL */
	} /* end for number of old copies */
	if(rc == FAILURE)
	    break;
    } /* end for old number of lps */   
    /* 
     * if no errors were found, we can complete the
     * extend by filling in the lvol struct with the
     * new info.
     */
    if(rc == SUCCESS) {
	saext->klv_ptr->nparts = saext->nparts;	
	saext->klv_ptr->nblocks = saext->nblocks;	
	saext->klv_ptr->i_sched = saext->isched;
	for(cpi = 0; cpi < saext->nparts; cpi++) 
	    saext->klv_ptr->parts[cpi] =
		saext->new_parts[cpi];
    } /* end if rc == SUCCESS */
    return(rc);
}

/*
 *  NAME:	hd_reduce
 *
 *  FUNCTION:   Transfers old part struct information to new part struct
 *	        information, and handles promotion if needed.
 *
 *  NOTES:	
 *
 *  PARAMETERS:  sared  pointer to information structure for the reduce
 *   		 vg     pointer to volume group structure
 *
 *  DATA STRUCTS:
 *
 *  RETURN VALUE: none
 */

void
hd_reduce(
   struct sa_red *sared,   /* pointer to information on the reduce */
   struct volgrp *vg)	   /* pointer to volume group structure */
{
    register int i,ppcnt,lpcnt,cpcnt;	
			        /* loop counters */
    register struct part *pp,*op,*np,*sp,*tp; 
			        /* part struct pointers */
    register struct extred_part *pplist;
				/* pointer to array of ppinfo structs */
    register int ppsleft;       /* mask for pps left after reduction */
    register int copy;	        /* holds copy of lp we're processing */
    register int redpps, cpymsk;/* masks for the logical partition */
    register int zeromsk;       /* mask for copies to zero out */
    register int size;		/* size of old part structs to copy to new */

    struct part zeropp;         /* zeroed out part struct used to zero parts */

    pplist = sared->list;
    bzero((char *)(&zeropp), sizeof(struct part));

    /* 
     * go through the pps being reduced and update the old copy as needed. 
     * Do the necessary promotions and deletions in the old copy PRIOR to
     * copying things over to the new copy.
     */
    cpymsk = MIRROR_EXIST(sared->lv->nparts);
    for(ppcnt = 1; ppcnt <= sared->numred; pplist ++, ppcnt++) {
       if(pplist->mask != 0) {
	  redpps = cpymsk | pplist->mask;
	  /*
	   * NOTE: redpps is a 3 bit field that can have the values
	   * 0 (000) - 7 (111). The zero condition cannot exist on a reduce,  
	   * however.
	   */
	  switch(redpps) {
	  /* promote secondary to primary and tertiary to secondary */
	  case 1: pp = PARTITION(sared->lv,(pplist->lp_num-1),PRIMMIRROR);
		  sp = PARTITION(sared->lv,(pplist->lp_num-1),SINGMIRROR);
		  tp = PARTITION(sared->lv,(pplist->lp_num-1),DOUBMIRROR);
		  *pp = *sp;
		  /* 
		   * set up a mask to show the promoted lp
		   * the bits will be off for good copies and on for 
		   * the copies that are now invalid.
		   */
		  *sp = *tp;
		  zeromsk = TERTIARY_MIRROR;
		  break;
	  /* promote tertiary to secondary */
	  case 2: sp = PARTITION(sared->lv,(pplist->lp_num-1),SINGMIRROR);
		  tp = PARTITION(sared->lv,(pplist->lp_num-1),DOUBMIRROR);
		  *sp = *tp;
		  zeromsk = TERTIARY_MIRROR;
		  break;
	  /* promote tertiary to primary */
	  case 3: pp = PARTITION(sared->lv,(pplist->lp_num-1),PRIMMIRROR);
		  tp = PARTITION(sared->lv,(pplist->lp_num-1),DOUBMIRROR);
		  *pp = *tp;
		  zeromsk = (TERTIARY_MIRROR | SECONDARY_MIRROR);
		  break;
	  /* no promotion */
	  case 4:
	  case 6:
 	  case 7: zeromsk = redpps;
		  break;
	  /* promote secondary to primary */
	  case 5: pp = PARTITION(sared->lv,(pplist->lp_num-1),PRIMMIRROR); 
		  sp = PARTITION(sared->lv,(pplist->lp_num-1),SINGMIRROR);
		  *pp = *sp;
		  zeromsk = (TERTIARY_MIRROR | SECONDARY_MIRROR);
		  break;
	  } /* end switch */
	  /* set up a mask of copies to zero out */
	  zeromsk &= ~cpymsk;
	  /* zero out the necessary copies of the logical partition */
	  while(zeromsk != 0) {
	     copy = FIRST_MASK(zeromsk);
	     pp = PARTITION(sared->lv,(pplist->lp_num-1),copy);
	     *pp = zeropp;
	     zeromsk &= ~MIRROR_MASK(copy);
	  }
       } /* end if */
    } /* end for ppcnt */
    /* go through and transfer each copy to the new part structure */
    for(cpcnt = 0; cpcnt < sared->nparts; cpcnt ++) {
	  size = sared->numlps * sizeof(struct part);
	  bcopy(sared->lv->parts[cpcnt], sared->newparts[cpcnt],size);
	  sared->lv->parts[cpcnt] = sared->newparts[cpcnt];
    }
    /* NULL out the pointers to the copies that no longer exist */
    for(i = sared->nparts; i < sared->lv->nparts; i++)
	sared->lv->parts[i] = NULL;
    /*
     * reset the lvol structure with the values in the extred
     * structure and loop through to put the newparts pointers
     * into the lvol parts field 
     */
    sared->lv->nparts = sared->nparts;
    sared->lv->nblocks = PART2BLK(vg->partshift, sared->numlps);
    sared->lv->i_sched = sared->isched;

    return;
}
