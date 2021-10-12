static char sccsid[] = "@(#)34	1.9  src/bos/kernext/lvm/hd_bbrel.c, sysxlvm, bos411, 9428A410j 5/23/94 10:06:28";

/*
 * COMPONENT_NAME: (SYSXLVM) Logical Volume Manager Device Driver - 34
 *
 * FUNCTIONS: hd_chkblk, hd_bbend, hd_baddone, hd_badblk, hd_swreloc,
 *	      hd_assignalt, hd_fndbbrel, hd_nqbblk, hd_dqbblk
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *
 * hd_bbrel.c -	These routines handle the part of a request that crosses
 *		a relocated block or one that was found to be defective
 *		by a previous request.
 *
 *  Execution environment:
 *
 *	All these routines run on interrupt levels, so they are not
 *	permitted to page fault.  They run within critical sections that are
 *	serialized with block I/O offlevel iodone() processing.
 *
 *  NOTES:	In the future it would be nice to record blocks in the
 *		relocation pool in the bad block directory that fail.
 *		This will prevent trying to reuse them if 2 fail in a
 *		row that cause the PV to go into read only relocation.
 */

#include <sys/types.h>
#include <sys/intr.h>
#include <sys/errno.h>
#include <sys/sleep.h>
#include <sys/trchkid.h>
#include <sys/errids.h>
#include <sys/hd_psn.h>
#include <sys/bbdir.h>
#include <sys/lvmd.h>
#include <sys/dasd.h>
#include <sys/hd.h>

/* !!!!
 * !!!! Added to inhibit any relocations due to grown defects.
 * !!!! */
extern int hd_allow_reloc;
#define ASCHW	0x4857		/* 4857 = "HW" */


/* ------------------------------------------------------------------------
 *
 *		B A D   B L O C K   R E L O C A T I O N
 *
 * ------------------------------------------------------------------------
 */

/*
 *  NAME:         hd_chkblk 
 * 
 *  FUNCTION:     Check for bad block relocation.
 *
 *  NOTES:     	  See if bad block relocation is required.  If so, set up 
 *		  relocation request.
 *
 *	input:	physical buf struct to be started, containing:
 *			b_blkno = unrelocated physical block address
 *			b_bcount = remaining byte count for this request
 *			pb_pvol = address of physical volume struct
 *
 *	output: physical buf struct contents:
 *			b_blkno	= (possibly) relocated physical address
 *			b_bcount = byte count for next attempt
 *			b_options requests physical relocation, if needed
 *			pb_bad = address of bad block struct, if relocating;
 *				NULL if no relocation being performed
 *
 *  PARAMETERS:   pb - physical buf struct 
 *
 *  DATA STRUCTS: 
 *
 *  RETURN VALUE:	SUCCESS if operation can proceed;
 *			FAILURE if unable to relocate bad block.
 *
 */
int
hd_chkblk(
register struct pbuf *pb)		/* physical device buf struct */
{
	register struct bad_blk *bad;	/* bad block structure */

	struct volgrp *vg;		/* pointer to volgrp struct	*/

	/* search defects directory hash table */
	pb->pb_bad = NULL;
	bad = HASH_BAD(pb,pb->pb.b_blkno);

	BUGLPR( debuglvl, BUGNTA,
	    ("hd_chkblk: pb = 0x%x b_blkno = 0x%x\n", pb, pb->pb.b_blkno))

	for(;;)  {
		if (bad == NULL)		/* no bad blocks here */
			return(SUCCESS);
		/*
		 * If beg of request if > BB address, then BB cant be 
		 * 	in request area (badblk list in ascending order).
		 */
		if (pb->pb.b_blkno <= bad->blkno)
			break;		/* could be bad, check below */
		bad = bad->next;
	}

	/* see if bad->blkno is within the range of this request */
	if (pb->pb.b_blkno+BYTE2BLK(pb->pb.b_bcount) <= bad->blkno)
		return(SUCCESS);
	
	/* here is a bad block, see if this logical volume allows relocation */
	/* get volgrp pointer from the device switch table	*/
	(void) devswqry(pb->pb_lbuf->b_dev,(uint *)NULL,(caddr_t *)&vg);

	if ((VG_DEV2LV(vg, pb->pb_lbuf->b_dev))->lv_options & LV_NOBBREL)
		return(FAILURE);

	/* good blocks precede the bad one, process them first */
	if (pb->pb.b_blkno < bad->blkno)  {
	    pb->pb.b_bcount = BLK2BYTE(bad->blkno - pb->pb.b_blkno);

	    BUGLPR( debuglvl, BUGGID,
	        ("   bad/relocated block at = 0x%x\n", bad->blkno))

	    return(SUCCESS);
	}

	/* pb->pb.blkno is the first bad block in the range of this request. */
	pb->pb_bad = bad;
	pb->pb.b_bcount = DBSIZE;		/* do one block */

	switch(bad->status)  {		/* branch on bad block state */

	    /* S/W relocation previously completed? */
	    case REL_DONE:

		TRCHKL3T(HKWD_KERN_LVM | hkwd_LVM_RELOCINGBLK, pb,
			 pb->pb.b_blkno, bad->relblk);

		BUGLPR( debuglvl, BUGGID,
		    ("   relocating block 0x%x to 0x%x\n",
						pb->pb.b_blkno, bad->blkno))

		pb->pb.b_blkno = bad->relblk;
		break;

	    case REL_PENDING:
	    case REL_DEVICE:
		panic("hd_chkblk: invalid relocation status");
		break;

	    /* The relocation block has gone bad so try to fix it */
	    case REL_CHAINED:

		BUGLPR( debuglvl, BUGGID,
		    ("   chained relocation block 0x%x to 0x%x\n",
						pb->pb.b_blkno, bad->relblk))

		pb->pb.b_blkno = bad->relblk;
		/*
		 * find bad block structure for the relocated block
		 */
		bad = HASH_BAD(pb,pb->pb.b_blkno);
		while( pb->pb.b_blkno != bad->blkno ) {
		    bad = bad->next;
		    assert( bad != NULL );
		}

		/* switch to relocation block bad block structure */
		pb->pb_bad = bad;

		/*
		 * fall through to try HW relocation
		 */

	    /* relocation desired, but not yet performed? */
	    case REL_DESIRED:

		TRCHKL4T(HKWD_KERN_LVM | hkwd_LVM_OLDBADBLK,pb,pb->pb.b_flags,
			 pb->pb.b_blkno,pb->pb_pvol->pvstate);

		if( (pb->pb_lbuf->b_options & RORELOC) ||
		    (pb->pb_pvol->pvstate == PV_RORELOC) )
			return(FAILURE);

		/*
		 * set up request to come back into the bad block handling
		 * code.
		 */
		pb->pb.b_iodone = (void (*)())hd_bbend;
		if( pb->pb.b_flags & B_READ ) {	/* read operation? */
		    pb->pb_bbop = BB_RDDFCT;
		    break;
		}

		pb->pb_bbop = BB_WTDFCT;
		bad->status = REL_DEVICE;	/* try H/W reloc */
		pb->pb.b_options |= HWRELOC;
		pb->pb_bbfixtype = FIX_READ_ERROR;	/* fix for a read error */

		BUGLPR( debuglvl, BUGGID,
		    ("   HW relocation of block 0x%x started\n",pb->pb.b_blkno))

		break;

	}  /* end -- switch on bad block status */

	return(SUCCESS);
}

/*
 *  NAME:	hd_bbend 
 * 
 *  FUNCTION:	Physical operations that involve bad block processing
 *		iodone() routine
 *
 *  NOTES:	This routine is scheduled to run as an offlevel interrupt 
 *	  	handler when the physical device driver calls iodone().
 *		input:  physical buf for a completed request; b_resid and
 *			b_error set by physical device driver if
 *		    	b_flags=B_ERROR.
 *			b_error = EIO - non-media I/O error
 *				  EMEDIA - newly grown media error
 *				  ESOFT - request succeeded, but needs 
 *					relocation (may not work next 
 *					time)
 *
 *  PARAMETERS:	pb - physical buf struct 
 *
 *  DATA STRUCTS: 
 *
 *  RETURN VALUE: none
 *
 */
void
hd_bbend(
register struct pbuf *pb)		/* physical device buf struct	 */
{
	register int  int_lvl; 		/* interrupt level save */

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
	 ("hd_bbend: pb = 0x%x b_flags = 0x%x b_error = 0x%x b_resid = 0x%x\n",
			pb,pb->pb.b_flags,pb->pb.b_error,pb->pb.b_resid))

	/* update buffer address by number of bytes processed */
	pb->pb_addr += pb->pb.b_bcount - pb->pb.b_resid;

	if( !(pb->pb.b_flags & B_ERROR) || (pb->pb.b_error == ESOFT) ) {
	    if( pb->pb_bbop != BB_RDDFCT )
		hd_baddone(pb);			/* bb reloc done	   */
	    else {
		/*
		 * Read of a defective block was successful so we can now
		 * try to relocate the data
		 */
		 pb->pb.b_flags &= ~B_READ;	/* make request a write	   */
		 pb->pb_bbop = BB_WTDFCT;
		 pb->pb_bad->status = REL_DEVICE;
		 pb->pb.b_options |= HWRELOC;
		 pb->pb_bbfixtype = FIX_READ_ERROR;

                 /* Adjust addr back to beginning of block */
		 pb->pb_addr -= (pb->pb.b_resid ? 0 : DBSIZE);
		 hd_start(pb);
	    }
	}
	else {
	    switch( pb->pb_bbop ) {

		case BB_RDDFCT:
		    /*
		     * If the error was on a block already detected as defective
		     * we can just return the request with the error since all
		     * the bad block structures have been set up and the BB
		     * directory already has been updated.
		     */
		    HD_SCHED( pb );
		    break;

		case BB_WTDFCT:
		case BB_SWRELO:

		    /*
		     * Error was on a HW or SW relocate attempt.
		     */
		    hd_swreloc( pb );
		    break;
	    }
	}
	unlock_enable(int_lvl, &glb_sched_intlock);
	reschedule();
}
/*
 *  NAME:         hd_baddone 
 * 
 *  FUNCTION:     Successful relocated physical request completion.
 *
 *  NOTES:
 *	input:	physical buf structure after bad block I/O operation completed.
 *	output:	defect status updated, if necessary.
 *
 *  PARAMETERS:   pb - physical buf struct 
 *		  bad - pointer to bad block hash chain
 *
 *  DATA STRUCTS: 
 *
 *  RETURN VALUE: none
 *
 */
void
hd_baddone(
register struct pbuf *pb)		/* physical request to process */
{
	register struct bad_blk *bad; 	/* bad block structure pointer	 */

	TRCHKL1T(HKWD_KERN_LVM | hkwd_LVM_BADBLKDONE, pb);

	bad = pb->pb_bad;

	BUGLPR( debuglvl, BUGNTA,
	    ("hd_baddone: pb = 0x%x b_blkno = 0x%x bad->status = 0x%x\n",
					pb, pb->pb.b_blkno, bad->status))

	BUGLPR( debuglvl, BUGGID,
	    ("   pb_bbfixtype = 0x%x b_flags = 0x%x\n",pb->pb_bbfixtype,pb->pb.b_flags))

	/* branch on bad block state */
	switch(bad->status)  {

	    /* block previously relocated? */
	    case REL_DONE:
		panic("hd_baddone: invalid relocation status - REL_DONE");
		break;				/* nothing more to do */

	    /* S/W relocation in progress? */
	    case REL_PENDING:

		/* Log a successful relocation */
		hd_logerr( (unsigned)ERRID_LVM_SWREL, (ulong)(pb->pb.b_dev),
			   (ulong)(bad->blkno), (ulong)(bad->relblk) );

		bad->status = REL_DONE;
		/*
		 * reset to the original requests operation READ/WRITE
		 */
		pb->pb.b_flags &= ~((int) B_READ);
		pb->pb.b_flags |= pb->orig_bflags & (int) B_READ;

		if( pb->pb_bbfixtype == FIX_READ_ERROR ) {
		    /* already a Bad Block Directory entry; just update it*/
		    pb->pb_bbop = BB_UPDATE;
		}
		else {  /* for all others:  add BB Directory entry */
		    pb->pb_bbop = BB_ADD;
		}
		pb->pb_bbfixtype = 0;		/* reset fixtype field */
		hd_upd_bbdir(pb);
		break;

	    /* H/W relocation requested? */
	    case REL_DEVICE:

		/* Log a successful relocation */
		hd_logerr( (unsigned)ERRID_LVM_HWREL, (ulong)(pb->pb.b_dev),
			   (ulong)(bad->blkno), (ulong)0 );

		pb->pb.b_options &= ~HWRELOC;
		/*
		 * reset to the original requests operation READ/WRITE
		 */
		pb->pb.b_flags &= ~((int) B_READ);
		pb->pb.b_flags |= pb->orig_bflags & (int) B_READ;

		/* 
		 * delete the Bad Block Directory entry on disk (if there is
		 * one) & kernel bad_blk structure since HW relocation worked
		 */
		if (pb->pb_bbfixtype == FIX_READ_ERROR) {
			/* there is a BB Dir entry to be deleted */
			pb->pb_bbop = BB_DELETE;
			pb->pb_bbfixtype = 0;		/* reinit fixtype */
			bad->status = REL_DONE;
			hd_upd_bbdir(pb);
		}
		else {
		    /*
		     * remove kernel bad_blk struct from hash queue and 
		     * release it then allow the request to continue
		     */
		    hd_dqbblk( pb, pb->pb_bad->blkno );
		    REL_BBLK( pb->pb_bad );
		    pb->pb_bbfixtype = 0;		/* reinit fixtype */
		    pb->pb_bad = NULL;
		    pb->pb_bbop = 0;
		    pb->pb.b_iodone = (void (*)()) hd_end;
		    PB_CONT( pb );
		}
		break;

	    /* relocation desired? */
	    case REL_DESIRED:
		panic("hd_baddone: invalid relocation status - REL_DESIRED");
		break;

	}  /* end -- switch on bad block status */
}


/*
 *  NAME:	hd_badblk 
 * 
 *  FUNCTION:	Start bad block processing on a new bad block
 *
 *  NOTES::	EMEDIA and ESOFT can be reported on both reads and writes.
 *		Therefore, relocation will be attempted if the request is
 *		a write.  If the request is a read and the error is ESOFT
 *		relocation will be attempted.  If the error was EMEDIA then
 *		the BB directory will be updated with the new bad block 
 *		and the requests errored off.
 *
 *		*NOTE*	ESOFT errors actually redo the original request 
 *			from the point of the ESOFT.  This is done because
 *			we can't keep enough information around to remember
 *			where the ESOFT actually finished.  So the request
 *			is restarted at the block following the block
 *			that reported the ESOFT after bad block processing.
 *
 *  PARAMETERS:
 *
 *  DATA STRUCTS: 
 *
 *  RETURN VALUE: none
 *
 */
void
hd_badblk(
register struct pbuf *pb)	/* physical request to process */
{
	register struct bad_blk *bad;	/* bad block structure		*/
	register daddr_t badblkno;	/* block number of bad block	*/

	badblkno = pb->pb.b_blkno + BYTE2BLK(pb->pb.b_bcount -pb->pb.b_resid);
	TRCHKL4T(HKWD_KERN_LVM | hkwd_LVM_NEWBADBLK, pb, pb->pb.b_flags,
		 pb->pb.b_error, badblkno);

	BUGLPR( debuglvl, BUGNTA,
	    ("hd_badblk: pb = 0x%x badblkno = 0x%x\n", pb, badblkno))

	BUGLPR( debuglvl, BUGGID,
	    ("   hd_freebad = 0x%x pvstate = 0x%x lb->b_options = 0x%x\n",
		hd_freebad, pb->pb_pvol->pvstate, pb->pb_lbuf->b_options))

	/*
	 * if PV is in Read Only Relocation state or request ext has
	 * Read Only Relocation on, return and continue
	 * processing, just don't allow any new HW or SW relocation
	 *
	 * Of course, if the error was ESOFT then the request worked so
	 * adjust to the end of the successful request and carry on.
	 */
	if( (hd_freebad == NULL) ||
/* !!!! Add here */
	    (hd_allow_reloc == 0) ||
/* !!!! END add here */
	    (pb->pb_pvol->pvstate == PV_RORELOC) ||
	    (pb->pb_lbuf->b_options & RORELOC) ) {

		if( pb->pb.b_error == ESOFT ) {
		    pb->pb_addr += pb->pb.b_resid;
		    pb->pb.b_error = 0;
		    pb->pb.b_flags &= ~B_ERROR;
		}

		PB_CONT( pb );

		return;
	}

	/* get a bad_blk structure and fill it in */
	GET_BBLK( bad );
	/*
	 * if we hit the low water mark for bad_blk structs kick the
	 * kproc to get more
	 */
	if( hd_freebad_cnt <= LVDD_LFREE_BB ) {

	    BUGLPR( debuglvl, BUGGID,
		("   kicking kernel process - hd_freebad_cnt = 0x%x\n",
			hd_freebad_cnt))

	    et_post( LVDD_KP_BADBLK, lvm_kp_tid );
	}
	bad->dev = pb->pb.b_dev;
	bad->blkno = badblkno;
	bad->relblk = 0;
	bad->status = REL_DESIRED;
	pb->pb_bad = bad;

	/* add bad_blk struct to bad blk hash table */
	hd_nqbblk( pb );
	
	BUGLPR( debuglvl, BUGGID,
	    ("   new bad block at 0x%x b_flags = 0x%x b_error = 0x%x\n",
			badblkno, pb->pb.b_flags, pb->pb.b_error))

	/* if this was a MEDIA error from a READ request, add a BB Dir 
	 * entry & continue with read completion processing
  	 */
 	if ((pb->pb.b_flags & B_READ) && (pb->pb.b_error == EMEDIA)) {

	    pb->pb_bbop = BB_ADD;
	    hd_upd_bbdir(pb);		
	}
	else {
		/*
		 * else set up pbuf for HW relocation request
		 * on the one bad block
		 */
		bad->status = REL_DEVICE;
		pb->pb.b_flags &= ~B_ERROR; 	/* turn off error flag */
		pb->pb.b_options |= HWRELOC;   /* request HW relocation */
		pb->pb.b_blkno = bad->blkno;
		pb->pb.b_bcount = DBSIZE;	/* one block */
		pb->pb.b_resid = 0;
		
		/*
	 	 * turn it into a WRITE request to perform HW relocation
		 * and if the error was ESOFT remember it in bbfixtype.
		 */
		pb->pb.b_flags &= ~B_READ;
		if( pb->pb.b_error == ESOFT )
		    pb->pb_bbfixtype = FIX_ESOFT;
		else
		    pb->pb_bbfixtype = FIX_EMEDIA;

		pb->pb.b_iodone = (void (*)())hd_bbend;
		pb->pb_bbop = BB_WTDFCT;
		hd_start(pb);		
	}
	return;
}

/*
 *  NAME:	hd_swreloc 
 * 
 *  FUNCTION:	Performs software relocation when hardware relocation  
 *		fails.  An alternate block is found from the 
 *		relocate blk pool at the end of the disk and 
 *		the request is resent to the disk device driver
 * 		to write to this new block.
 *
 *  NOTES:
 *	input:	physical device buf structure.
 *	output:	request is put on ready queue to be sent to the disk
 *			device driver
 *
 *  PARAMETERS:   pb - physical buf struct 
 *
 *  DATA STRUCTS: 
 *
 *  RETURN VALUE: none
 *
 */
void
hd_swreloc(
register struct pbuf *pb)	/* physical request to process */
{

	register struct bad_blk *bad = pb->pb_bad;

	TRCHKL4T(HKWD_KERN_LVM | hkwd_LVM_SWRELOC, pb, bad->status,
		 pb->pb.b_error, pb->pb_swretry);

	BUGLPR( debuglvl, BUGNTA,
	    ("hd_swreloc: pb = 0x%x b_blkno = 0x%x\n", pb, pb->pb.b_blkno))

	BUGLPR( debuglvl, BUGGID,
	    ("   bad->status = 0x%x b_error = 0x%x pb_swretry = 0x%x\n",
				bad->status, pb->pb.b_error,pb->pb_swretry))

	BUGLPR( debuglvl, BUGGID,
	    ("   pvstate = 0x%x pb_bbfixtype = 0x%x\n",
				pb->pb_pvol->pvstate, pb->pb_bbfixtype))

	/* if pv is in Read Only Relocate state, return and continue
	 * processing, just don't allow either SW or HW relocation */
	if( pb->pb_pvol->pvstate == PV_RORELOC ) {
	    /*
	     * reset to the original requests operation READ/WRITE
	     */
	     pb->pb.b_flags &= ~((int) B_READ);
	     pb->pb.b_flags |= pb->orig_bflags & (int) B_READ;

	    /*
	     * The call will have to handle the continuation of the
	     * request if this path is taken.
	     */
	    if( pb->pb_bbfixtype == FIX_ESOFT ) {
		pb->pb_addr += pb->pb.b_resid;
		pb->pb.b_flags &= ~B_ERROR;
		pb->pb.b_error = 0;
		pb->pb.b_resid = 0;

		hd_dqbblk( pb, pb->pb_bad->blkno );
		REL_BBLK( pb->pb_bad );
		pb->pb_bad = NULL;
	    }
	    else {
		pb->pb.b_error = EIO;
		bad->status = REL_DESIRED;
	    }
	    /* Let the request continue */
	    pb->pb_bbop = 0;

	    pb->pb.b_iodone = (void (*)())hd_end;
	    PB_CONT( pb );
	    return;
	}
	/* if haven't already tried SW relocation or SW reloc failed w/ EMEDIA,
	 * get a relocate block and set up pbuf to resend request to WRITE 
	 * to the new relocated block
	 */   
	pb->pb_bbstat = 1;
	if( (bad->status != REL_PENDING) ||
  	    ((bad->status == REL_PENDING) &&
	    ((pb->pb.b_error == EMEDIA) || (pb->pb.b_error == ESOFT))) ) {

		if( bad->status != REL_PENDING ) {

		    /* Log HW relocation failed */
		    hd_logerr((unsigned)ERRID_LVM_HWFAIL,(ulong)(pb->pb.b_dev),
				(ulong)(bad->blkno), (ulong)0 );
		}

		pb->pb_swretry++;
		if ( ((pb->pb_swretry) < MAX_SWRETRY) &&
		     ((bad->relblk = hd_assignalt(pb)) != 0) ) {

			bad->status = REL_PENDING;
			/* turn off error flag and HW relocate flag */
			pb->pb.b_flags &= ~B_ERROR;
			pb->pb.b_options &= ~HWRELOC;
			pb->pb.b_blkno = bad->relblk;	/* bad block */
			pb->pb.b_error = 0;
			pb->pb.b_resid = 0;
			pb->pb_bbstat = 0;
			pb->pb_bbop = BB_SWRELO;
			hd_start(pb);
			return;
		}
		else {

		    /* if can no longer continue with SW relocation (retried 
		     * the MAX number of times or can't get alternate block),
		     * then set the PV state to Read Only Relocation and
		     * continue processing this request 
		     */

		    if( pb->pb_swretry < MAX_SWRETRY )
			pb->pb_bbstat = 2;
		    else
			pb->pb_bbstat = 3;
		}
	}
	/*
	 * we have an error that is fatal to the request
	 */
	/* update the pv state to Read Only Relocate */
	pb->pb_pvol->pvstate = PV_RORELOC;
	switch( pb->pb_bbstat ) {

	    case 1:

		/* Log a non media error during relocation */
		hd_logerr( (unsigned)ERRID_LVM_BBFAIL, (ulong)(pb->pb.b_dev),
			   (ulong)(pb->pb.b_error), (ulong)0 );

		break;

	    case 2:

		/* Log max relocation attempts	*/
		hd_logerr( (unsigned)ERRID_LVM_BBRELMAX, (ulong)(pb->pb.b_dev),
			   (ulong)(bad->blkno), (ulong)0 );

		break;

	    case 3:

		/* Log empty relocation pool	*/
		hd_logerr( (unsigned)ERRID_LVM_BBEPOOL, (ulong)(pb->pb.b_dev),
			   (ulong)0, (ulong)0 );

		break;
	}

	/*
	 * reset to the original requests operation READ/WRITE
	 */
	pb->pb.b_flags &= ~((int) B_READ);
	pb->pb.b_flags |= pb->orig_bflags & (int) B_READ;

	if( pb->pb_bbfixtype == FIX_ESOFT ) {
	    /*
	     * if we're fixing a soft error, the original 
	     * request DID work
	     */
	    pb->pb.b_flags &= ~B_ERROR;
	    pb->pb.b_error = 0;
	    pb->pb_bbfixtype = 0;
	    pb->pb_addr += pb->pb.b_resid;
	    pb->pb.b_resid = 0;

	    /* remove bad_blk struct from hash queue and release
	     * it - since there is no entry in the disk BB dir
	     * there is no need to go delete it.
	     */
	    hd_dqbblk( pb, pb->pb_bad->blkno );
	    REL_BBLK( pb->pb_bad );
	    pb->pb_bad = NULL;
	    pb->pb_bbop = 0;

	    /* let the request continue */
	    pb->pb.b_iodone = (void (*)())hd_end;
	    PB_CONT( pb );
	    return;
	}
	/*
	 * fixing a READ error or fixing a normal write - 
	 * since can't relocate now, set to reloc desired
	 *
	 * If block is a relocation pool block set the original
	 * block status to REL_DESIRED and release the extra
	 * one.  It is not needed since the PV is in ReadOnly
	 * relocation.
	 */
	if( bad->blkno >= pb->pb_pvol->beg_relblk ) {
	    hd_dqbblk( pb, pb->pb_bad->blkno );
	    REL_BBLK( pb->pb_bad );
	    pb->pb_bad = bad;
	}
	else {
	    bad->status = REL_DESIRED;  
	    bad->relblk = 0;
	}
	pb->pb.b_error = EIO;

	pb->pb_bbop = 0;

	/* let the request continue */
	pb->pb.b_iodone = (void (*)())hd_end;
	PB_CONT( pb );
}
			
/*
 *  NAME:         hd_assignalt 
 * 
 *  FUNCTION:     Assign an alternate block from relocate block pool at
 * 			end of the disk.
 *
 *  NOTES:
 *	input:	physical device buf structure.
 *	output:	block number of the alternate block
 *
 *  PARAMETERS:   pb - physical buf struct 
 *
 *  DATA STRUCTS: 
 *
 *  RETURN VALUE: blkno of alternate blk or 0 if none left 
 *
 */
daddr_t
hd_assignalt(
register struct pbuf *pb)	/* physical request to process */
{

	register daddr_t altblk;	/* alternate blkno to return */
	
	BUGLPR( debuglvl, BUGNTA,
	    ("hd_assignalt: pb = 0x%x next_relblk = 0x%x max_relblk = 0x%x\n",
			pb, pb->pb_pvol->next_relblk,pb->pb_pvol->max_relblk))

	if ((altblk = pb->pb_pvol->next_relblk) > pb->pb_pvol->max_relblk)
		return( 0 );
	else {
		/* get the next available relocate block and increment
		 * the next relocate block number for this pvol 
		 */
		pb->pb_pvol->next_relblk++;
		return(altblk);
	}
}

/*
 *  NAME:	hd_fndbbrel 
 * 
 *  FUNCTION:	Given a bad block structure with a blkno in the relocation
 *		pool find the corresponding original bad block structure.
 *
 *  NOTES:
 *
 *  PARAMETERS:	none 
 *
 *  DATA STRUCTS: 
 *
 *  RETURN VALUE: none
 *
 */
struct bad_blk *
hd_fndbbrel(
register struct pbuf *pb)		/* physical request to process */
{
    register struct bad_blk	*bad;		/* ptr to relocated BB struct*/
    register struct defect_tbl	*hashtbl;	/* Hash table ptr	*/
    register struct bad_blk	**defhash;	/* ptr into hash table	*/
    register struct bad_blk	*bbent;		/* BB entry ptr		*/

    bad = pb->pb_bad;
    hashtbl = pb->pb_pvol->defect_tbl;

    BUGLPR( debuglvl, BUGNTA,
	("hd_fndbbrel: pb = 0x%x bad = 0x%x\n", pb, bad))

    /*
     * Search each hash queue list
     */
    for( defhash = (struct bad_blk **)hashtbl;
	 defhash < (struct bad_blk **)(hashtbl + 1);
	 defhash++ ) {

	bbent = *defhash;
	/*
	 * While there is something on list check it out
	 */
	while( bbent ) {
	    if( bbent->relblk == bad->blkno ) {

		/* Should return through here */
		return( bbent );
	    }
	    else
		bbent = bbent->next;
	}
    }
    panic("hd_fndbbrel: no bad_blk structure found");
}

/*
 *  NAME:	hd_nqbblk 
 * 
 *  FUNCTION:	adds a bad_blk structure to the pvol's defects table
 * 		hash queue in ascending bad blkno order
 *
 *  NOTES:
 *
 *  PARAMETERS:	none 
 *
 *  DATA STRUCTS: 
 *
 *  RETURN VALUE: none
 *
 */
void
hd_nqbblk(
register struct pbuf *pb)	/* physical request to process */
{
    register struct bad_blk **head_ptr;
    register struct bad_blk *bb_ptr;
    register struct bad_blk *cur_ptr;

    /*
     * get ptr to bad block struct and ptr to anchor of hash queue
     */
    bb_ptr = pb->pb_bad;
    head_ptr = &HASH_BAD( pb, bb_ptr->blkno );

    BUGLPR( debuglvl, BUGNTA,
	("hd_nqbblk: pb = 0x%x bb_ptr = 0x%x head_ptr = 0x%x\n",
					pb, bb_ptr, head_ptr))

    /* if hash chain is null or the bad block is less than the first
     * bad block entry on the hash chain, add it to the beginning
     */
    cur_ptr = *head_ptr;
    if (cur_ptr == NULL || bb_ptr->blkno < cur_ptr->blkno) {
	bb_ptr->next = cur_ptr;
	*head_ptr = bb_ptr;
    }
    else {
	/* find where the bad block should be inserted in the chain */
	while(cur_ptr->next != NULL && bb_ptr->blkno > cur_ptr->next->blkno)
	    cur_ptr = cur_ptr->next;

	/* insert bad blk struct after cur_ptr */
	bb_ptr->next = cur_ptr->next;
	cur_ptr->next = bb_ptr;
    }
    return;
}

/*
 *  NAME:	hd_dqbblk 
 * 
 *  FUNCTION:	remove a bad_blk structure from the pvol's defects table
 *		hash queue
 *
 *  NOTES:	The block number is passed in because if the operation
 *		involves a block from the relocation pool the block
 *		number in the bad_blk structure does not reflect the
 *		correct hash queue.
 *
 *  PARAMETERS:   none 
 *
 *  DATA STRUCTS: 
 *
 *  RETURN VALUE: none
 *
 */
void
hd_dqbblk(
register struct pbuf *pb,	/* physical request to process	*/
register daddr_t blkno)		/* block number to hash to	*/
{
	register struct bad_blk **head_ptr;
	register struct bad_blk *bb_ptr;
	register struct bad_blk *cur_ptr;

	/*
	 * get ptr to bad block struct and ptr to anchor of hash queue
	 */
	bb_ptr = pb->pb_bad;
	head_ptr = &HASH_BAD( pb, blkno );

	BUGLPR( debuglvl, BUGNTA,
	   ("hd_dqbblk: pb = 0x%x blkno = 0x%x bb_ptr = 0x%x head_ptr = 0x%x\n",
					pb, blkno, bb_ptr, head_ptr))

	/* if bad blk is at the head of the hash chain, change the headptr */
	cur_ptr = *head_ptr;
	if (cur_ptr == bb_ptr)
		*head_ptr = bb_ptr->next;
	else {
		/* find where the bad blk struct is in the chain */
		while (cur_ptr->next != bb_ptr &&
				cur_ptr->next != NULL)
			cur_ptr = cur_ptr->next;
		/* make sure the bad blk struct was there & delete it */
		ASSERT(cur_ptr->next != NULL);
		cur_ptr->next = bb_ptr->next;
	}
	return;
}
