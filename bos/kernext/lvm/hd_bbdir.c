static char sccsid[] = "@(#)27	1.14  src/bos/kernext/lvm/hd_bbdir.c, sysxlvm, bos411, 9428A410j 12/8/93 16:54:21";

/*
 * COMPONENT_NAME: (SYSXLVM) Logical Volume Manager Device Driver - 27
 *
 * FUNCTIONS: hd_upd_bbdir, hd_bbdirend, hd_bbdirop, hd_bbadd, hd_bbdel,
 *	      hd_bbupd, hd_chk_bbhld, hd_bbdirdone, hd_logerr
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
 *  hd_bbdir.c -- These routines handle updating the Bad Block Directory
 *			on disk whenever a new bad block is found or 
 *			an entry needs to be deleted or updated.
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
#include <sys/sleep.h>
#include <sys/trchkid.h>
#include <sys/errids.h>
#include <sys/hd_psn.h>
#include <sys/bbdir.h>
#include <sys/lvmd.h>
#include <sys/dasd.h>
#include <sys/hd.h>

static char bb_dir_id[6] = { 'D', 'E', 'F', 'E', 'C', 'T' };
static char bb_dir_up[6] = { 'U', 'P', 'D', 'A', 'T', 'E' };
/*
 *  NAME:	hd_upd_bbdir 
 * 
 *  FUNCTION:	Update the Bad Block Directory on disk
 *
 *  NOTES:
 *	input:	physical device buf structure.
 *	output:	pb queued for read of BB directory
 *
 *  PARAMETERS:	pb - physical buf struct 
 *
 *  DATA STRUCTS: 
 *
 *  RETURN VALUE: none
 *
 */
void
hd_upd_bbdir(
register struct pbuf *pb)	/* physical request to process */
{

    register struct pvol *pvol;		/* pointer to target pvol	    */
    register int	 bbdir_bytes;	/* number of bytes of BBdir to read */

    /* get the pvol pointer */
    pvol = pb->pb_pvol;

    TRCHKL3T(HKWD_KERN_LVM | hkwd_LVM_UPD_BBDIR,
		pb, bb_pbuf->pb_bbop , bb_pbuf->pb.b_flags);

    BUGLPR( debuglvl, BUGNTA, 
	("hd_upd_bbdir: pb = 0x%x bb_pbuf = 0x%x bb_pbuf flags = 0x%x\n",
				pb,bb_pbuf,bb_pbuf->pb.b_flags))

    /* check if the bb_pbuf reserved for BBdir updating is available */
    if (bb_pbuf->pb.b_flags & B_BUSY)  {
	/*
	 * if it's busy, put this request on BB holding Q - it will
	 * be taken off and processed once current BBdir updating is
	 * completed.
	 */
	hd_quelb((struct buf *)pb, &bb_hld);
    }
    else {

	bb_pbuf->pb.b_flags = B_READ|B_BUSY;
	bb_pbuf->pb.av_forw = NULL;
	bb_pbuf->pb.av_back = NULL;
	bb_pbuf->pb.b_iodone = (void (*)()) hd_bbdirend;	
	bb_pbuf->pb.b_dev = pvol->dev;
	bb_pbuf->pb.b_blkno = PSN_BB_DIR;	/* primary BB directory */

	/* read header record + number of existing entries +
	 * 1 entry(in case need to add an entry for this bad blk update) bytes
	 */
	bbdir_bytes = sizeof(struct bb_hdr) +
	     (pvol->num_bbdir_ent * sizeof(struct bb_entry));
	bb_pbuf->pb.b_bcount = BLK2BYTE(BYTE2BLK(bbdir_bytes) + 1);

	/* remember how much we read */
	bb_pbuf->pb_bbcount = bb_pbuf->pb.b_bcount;

	bb_pbuf->pb.b_event = EVENT_NULL;
	bb_pbuf->pb.b_options = 0;
	bb_pbuf->pb_pvol = pvol;
	bb_pbuf->pb_bbop = RD_BBPRIM;   
	bb_pbuf->pb_bbstat = 0;   

	/* save ptr to original pbuf */
	bb_pbuf->pb.b_back = (struct buf *)pb;

	hd_start(bb_pbuf);		/* send it off to PV */
    }

    return;
}

/*
 *  NAME:	hd_bbdirend 
 * 
 *  FUNCTION:	iodone routine for Bad Block Directory IO requests
 *
 *  NOTES:
 *	input:	ptr to completing pbuf structure
 *
 *  PARAMETERS:	pb - physical buf struct 
 *
 *  DATA STRUCTS: 
 *
 *  RETURN VALUE: none
 *
 */
void
hd_bbdirend(
register struct pbuf *bbpb)	/* ptr to bb_pbuf		*/
{
    register struct pvol  *pvol;	/* pointer to target pvol */
    register int  int_lvl; 		/* interrupt level save */

    int_lvl = disable_lock(INTIODONE, &glb_sched_intlock);

    /* save pvol pointer */
    pvol = bbpb->pb_pvol;

    TRCHKL3T(HKWD_KERN_LVM | hkwd_LVM_BBDIREND,
	bbpb->pb.b_dev, bbpb->pb.b_flags, bbpb->pb.b_error);

    BUGLPR( debuglvl, BUGNTA, 
      ("hd_bbdirend: bbpb = 0x%x bb_pbuf flags = 0x%x\n",bbpb,bbpb->pb.b_flags))
    /*
     * if there is an error AND it is not ESOFT on any BB processing request
     * set the PV to ReadOnly Relocation and error off the original request.
     */
    if( (bbpb->pb.b_flags & B_ERROR) && (bbpb->pb.b_error != ESOFT) ) {

	pvol->pvstate = PV_RORELOC;
	((struct pbuf *)(bbpb->pb.b_back))->pb_bbstat = BB_ERROR;

	BUGLPR( debuglvl, BUGGID, 
	    ("hd_bbdirend: ERROR = 0x%x b_resid = 0x%x\n",
					bbpb->pb.b_error,bbpb->pb.b_resid))

	/* Log an IO error during BB directory operation */
	hd_logerr( (unsigned)ERRID_LVM_BBDIRERR, (ulong)(bbpb->pb.b_dev),
		   (ulong)(bbpb->pb.b_error), (ulong)(bbpb->pb_bbop) );

	hd_bbdirdone( (struct pbuf *)(bbpb->pb.b_back) );
    }
    else
	hd_bbdirop();		/* Continue with BB processing	*/

    unlock_enable(int_lvl, &glb_sched_intlock);
    reschedule();	/* schedule anything on the pending queue */
}

/*
 *  NAME:	hd_bbdirop 
 * 
 *  FUNCTION:	Continue BB directory operation.
 *
 *  NOTES:	The value of the pb_bbop field in the volgrp
 *		pbuf(bb_pbuf) controls the next action to take place.
 *
 *	output:	bbpb queued for another read/write of the BB directory OR
 *		Bad Block directory updated and original pbuf processing 
 *		continued.
 *
 *  PARAMETERS:
 *
 *  DATA STRUCTS: 
 *
 *  RETURN VALUE: none
 *
 */
void
hd_bbdirop( void )
{
    register struct pbuf *bbpb;		/* ptr to bb_pbuf		*/
    register struct pbuf *origpb;	/* ptr to orig request's pbuf	*/
    register struct pvol *pvol;		/* pointer to target pvol	*/
    register int	 bbdir_bytes;	/* size of BB dir in bytes	*/
    register int	 rc;		/* general return code		*/

    bbpb = bb_pbuf;
    origpb = (struct pbuf *)(bbpb->pb.b_back);
    pvol = origpb->pb_pvol;

    TRCHKL3T(HKWD_KERN_LVM | hkwd_LVM_BBDIROP,
		bbpb->pb.b_dev, bbpb->pb_bbop, origpb->pb_bbop);

    BUGLPR( debuglvl, BUGNTA, 
      ("hd_bbdirop: bbpb = 0x%x bbpb->pb_bbop = 0x%x origpb->pb_bbop = 0x%x\n",
					bbpb,bbpb->pb_bbop,origpb->pb_bbop))
    /*
     * The pb_bbop field indicates the operation just completed
     */
    switch( bbpb->pb_bbop ) {

	/*
	 * The BB primary was just read in or a request was just
	 * taken off of the hold queue.  In either case the BB
	 * directory is currently in memory and ready for action
	 */
	case RD_BBPRIM:	

	    /*
	     * Check that the header record matches
	     */
	    if( strncmp( bbpb->pb_addr, bb_dir_id, sizeof(bb_dir_id)) != 0 ) {

		BUGLPR( debuglvl, BUGGID, 
		    ("hd_bbdirop: ERROR header is not DEFECT =0x%x%x%x%x%x%x\n",
			bbpb->pb_addr[0],bbpb->pb_addr[1],bbpb->pb_addr[2],
			bbpb->pb_addr[3],bbpb->pb_addr[4],bbpb->pb_addr[5]))

		/* Log a BB directory corrupt error */
		hd_logerr((unsigned)ERRID_LVM_BBDIRBAD,(ulong)(bbpb->pb.b_dev),
			   (ulong)-1, (ulong)0 );

		/* Set PV to ReadOnly Relocation and error off the
		 * original request
		 */
		pvol->pvstate = PV_RORELOC;
		origpb->pb_bbstat = BB_ERROR;
		hd_bbdirdone( origpb );
		break;
	    }
	    /*
	     * The original request's pbuf pb_bbop field contains
	     * the operation to be performed
	     */
	    switch (origpb->pb_bbop) {
		case BB_ADD:	/* add bad blk entry */
		    rc = hd_bbadd(bbpb);
		    break;

		case BB_DELETE:	/*delete bad blk entry*/
		    rc = hd_bbdel(bbpb);
		    break;

		case BB_UPDATE:	/*update bad blk entry*/
		    rc = hd_bbupd(bbpb);
		    break;

		default: 
		    panic("hd_bbdirop: unknown pbuf status");
						
	    }

	    /* if an error occurred in BBdir update routines, 
	     * quit BBdir updating & continue processing original
	     * request based on the type of error.
	     */
	    if (rc >= BB_ERROR) { 	/* an error occurred */

		/*
		 * Set PV to ReadOnly Relocation and error off the
		 * original request unless the directory is just full.
		 */
		if( rc != BB_FULL )
		    pvol->pvstate = PV_RORELOC;
		hd_bbdirdone( origpb );
		break;
	    }
	    /*
	     * setup bb_pbuf to write updated PRIMARY BB directory
	     * with UPDATE in the header id field
	     */
	    bbpb->pb.b_flags &= ~B_READ;
	    bbpb->pb.b_options = LV_WRITEV;		/* Write verify	*/
	    bbpb->pb.b_blkno = PSN_BB_DIR;		/* prim BBdir	*/
	    bbdir_bytes = sizeof(struct bb_hdr) +
		    (pvol->num_bbdir_ent * sizeof(struct bb_entry));
	    bbpb->pb.b_bcount = BLK2BYTE(BYTE2BLK(bbdir_bytes) + 1);
	    bbpb->pb_bbop = WT_UBBPRIM;

	    /* Put UPDATE in header record */
	    strncpy( bbpb->pb_addr, bb_dir_up, sizeof(bb_dir_up) );

	    /*
	     * If we backed up over a block boundry adjust b_bcount
	     * to write as much as we originally read.
	     */
	    if( bbpb->pb.b_bcount < bbpb->pb_bbcount ) {
		bbpb->pb.b_bcount = bbpb->pb_bbcount;
	    }
		/* If we advanced over a block boundry then zero
		 * memory after last entry stopping when we get to
		 * the length we are going to write.  Then adjust
		 * b_work to remember the new length for the backup
		 * directory.
		 */
	    else if( bbpb->pb.b_bcount > bbpb->pb_bbcount ) {
		bzero( (bbpb->pb_addr) + bbdir_bytes,
			bbpb->pb.b_bcount - bbdir_bytes );
		bbpb->pb_bbcount = bbpb->pb.b_bcount;
	    }

	    hd_start(bbpb);
	    break;

	/*
	 * The BB primary directory was just written with UPDATE in the
	 * header record.  Now rewrite first block with DEFECT
	 */
	case WT_UBBPRIM:

	    bbpb->pb.b_bcount = DBSIZE;
	    bbpb->pb_bbop = WT_DBBPRIM;

	    /* Put DEFECT in header record */
	    strncpy( bbpb->pb_addr, bb_dir_id, sizeof(bb_dir_id) );

	    hd_start(bbpb);
	    break;

	/*
	 * The first block of the BB primary directory was just
	 * written with DEFECT in the header record.  Now write
	 * the backup directory with UPDATE in the header record.
	 */
	case WT_DBBPRIM:

	    bbpb->pb.b_blkno = PSN_BB_BAK;
	    bbpb->pb.b_bcount = bbpb->pb_bbcount;
	    bbpb->pb_bbop = WT_UBBBACK;

	    /* Put UPDATE in header record */
	    strncpy( bbpb->pb_addr, bb_dir_up, sizeof(bb_dir_up) );

	    hd_start(bbpb);
	    break;

	/*
	 * The BB backup directory was just written with UPDATE in the
	 * header record.  Now rewrite first block with DEFECT
	 */
	case WT_UBBBACK:

	    bbpb->pb.b_bcount = DBSIZE;
	    bbpb->pb_bbop = WT_DBBBACK;

	    /* Put DEFECT in header record */
	    strncpy( bbpb->pb_addr, bb_dir_id, sizeof(bb_dir_id) );

	    hd_start(bbpb);
	    break;

	/*
	 * The first block of the BB backup directory was just
	 * written with DEFECT in the header record.  Time to 
	 * let the original request continue.
	 */
	case WT_DBBBACK:

	    hd_bbdirdone( origpb );

	    break;

	default:
	    panic("hd_bbdirop: unknown bad block operation");

    }	/* END switch( bbpb->pb_bbop ) */
}

/*
 *  NAME:         hd_bbadd
 * 
 *  FUNCTION:     Add a bad block entry to the Bad Block Directory on disk
 *
 *  NOTES:	  The bad block directory on disk and the defects directory
 *		  in the kernel are basically the same.  They both have entries
 * 		  for bad blocks found on the disk which contain the block 
 *		  number of the bad block and the relocated block.  The entries
 * 		  are sorted in relocation block number order.
 *		  There is one MAJOR difference between the disk and kernel 
 * 		  copies: the copy on disk contains an entry for EVERY
 *		  bad block ever found on it's disk; the copy in the kernel
 *		  may not, as in cases where the relocated block goes bad.
 * 		  For example, if block 100 is relocated to block 20000 (both
 *		  copies, disk and kernel, contain this entry) and then block
 *		  20000 goes bad and is relocated to 20010, the disk copy 
 *		  will have 2 entries (100->20000 and 20000->20010), but the
 *		  kernel copy will only have 1 entry (100->20010).
 *		  These relocation chains can grow even longer, so for time
 *		  and space reasons, these are not kept in the kernel, but
 *		  are kept on disk for a complete history of bad blocks.
 * 	
 *	input:	ptr to pbuf with the BBdir to be updated
 *	output:	Bad Block Directory on disk updated with new bad block entry
 *
 *  PARAMETERS:   pb - physical buf structure
 *
 *  DATA STRUCTS: 
 *
 *  RETURN VALUE: BB_SUCCESS		Successful update
 *		  BB_ERROR		Error during update
 *
 */
int
hd_bbadd(
register struct pbuf *bbpb)	/* ptr to bb_pbuf		*/
{

    register struct bb_hdr	*bb_hdr;	/* header of BBdir	  */
    register struct bb_entry	*bb_fentry;	/* first entry of BBdir	  */
    register struct bb_entry	*bb_lentry;	/* last entry of BBdir	  */
    register struct bb_entry	*bb_centry;	/* current entry of BBdir */
    register struct bad_blk	*bad_ptr;	/* ptr to active bad blk  */
    register daddr_t		new_badblk;	/* PSN of new badblk	  */
    register struct pbuf	*origpb;	/* orig pbuf with the bad blk */

	/* get ptr to original pbuf */
	origpb = (struct pbuf *)(bbpb->pb.b_back);
	bad_ptr = origpb->pb_bad;
	bb_hdr = (struct bb_hdr *)(bbpb->pb_addr);/* get header of BBdir */

	TRCHKL5T(HKWD_KERN_LVM | hkwd_LVM_BBADD,
	    bbpb, bad_ptr, bad_ptr->blkno, bad_ptr->relblk, bad_ptr->status);
	
	BUGLPR( debuglvl, BUGNTA, 
	    ("hd_bbadd: bbpb = 0x%x bad_ptr = 0x%x bad_blk = 0x%x\n",
					bbpb, bad_ptr, bad_ptr->blkno))

	/* check if Bad Block Directory on disk already full */
	if (bb_hdr->num_entries == MAX_BBENTRIES) {

	    BUGLPR( debuglvl, BUGGID, 
		("hd_bbadd: ERROR bbdir full num_entries = 0x%x\n",
					bb_hdr->num_entries))

		/* Log a BB directory full error */
		hd_logerr((unsigned)ERRID_LVM_BBDIRFUL,(ulong)(bbpb->pb.b_dev),
			   (ulong)0, (ulong)0 );

		origpb->pb_bbstat = BB_FULL;
		return( BB_FULL );
	}

	/*
	 * Inform administrator if BB dir is more than 90% full
	 */
	if( bb_hdr->num_entries > ((MAX_BBENTRIES / 10) * 9) ) {

	    BUGLPR( debuglvl, BUGGID, 
		("hd_bbadd: ERROR bbdir 90 %% full num_entries = 0x%x\n",
					bb_hdr->num_entries))

	    /* Log a BB directory 90% full error */
	    hd_logerr( (unsigned)ERRID_LVM_BBDIR90, (ulong)(bbpb->pb.b_dev),
		(ulong)(bb_hdr->num_entries), (ulong)MAX_BBENTRIES );

	}
	/* set ptr to first and last entry in BBDir */
	bb_fentry = (struct bb_entry *)(bb_hdr + 1);
	bb_lentry = bb_fentry + (bb_hdr->num_entries - 1);

	new_badblk = bad_ptr->blkno;	

	/* if bad block is in the pool of blks used for relocation -
	 * starting from the end and searching through to the beginning of
	 * the bad block entries, look for the block number of the new bad
	 * block within the block numbers of the relocated blocks 
	 */ 
	if (new_badblk >= bbpb->pb_pvol->beg_relblk) {

	    for(bb_centry = bb_lentry; bb_centry >= bb_fentry; bb_centry--) {
		/*
		 * if a match, set error field in original pbuf to indicate
		 * bad blk number is changed to the blk number whose relocation
		 * blk has now gone bad & then change it
		 */
	      	if( bad_ptr->blkno == (daddr_t)(bb_centry->rel_lsn) ) {
		    origpb->pb_bbstat = BB_CRB;
		    /*
		     * Remember original bad block so we can get to the
		     * hash queue it is on later.
		     */
		    bad_ptr->blkno = (daddr_t)(bb_centry->bb_lsn);

		    /* if block just found (whose relocation block was the
		     * newly found bad block) is not contained in the pool
		     * of reloc blocks, which means that it was the original
		     * bad block, exit the loop; otherwise, continue search
		     * for original bad block
		     */
		    if( bb_centry->bb_lsn < bbpb->pb_pvol->beg_relblk ) 
		      		break;

		}

	    } /* END FOR bb_centry = bb_lentry */

	    /* if original bad blk not found */
	    if( bb_centry < bb_fentry ) {
		
		BUGLPR( debuglvl, BUGGID, 
		    ("hd_bbadd: ERROR original BB entry not found for - 0x%x\n",
					bad_ptr->blkno))

		/* Log a BB directory corrupt error */
		hd_logerr((unsigned)ERRID_LVM_BBDIRBAD,(ulong)(bbpb->pb.b_dev),
			   (ulong)(bad_ptr->blkno), (ulong)0 );

		origpb->pb_bbstat = BB_ERROR;
		return( BB_ERROR );
	     }
	}

	/* set the bad block entry pointer past the current end of the bad
	 * block directory, where the new entry will be added & add it
	 */

	bb_centry = bb_lentry + 1;

	/* setup the new entry */
	bb_centry->reason = BB_SYSTEM;		/* found by the system */
	bb_centry->bb_lsn = new_badblk;
	bb_centry->rel_stat = bad_ptr->status;
	bb_centry->rel_lsn = bad_ptr->relblk;

	/* increment number of BB Directory entries in header on disk and
	 * in pvol structure in kernel 
	 */
	bb_hdr->num_entries++;	
	bbpb->pb_pvol->num_bbdir_ent++;	

        /* tell LVM kernel process to check pinned bad block directory space
           and pin more if necessary */
        et_post(LVDD_KP_BB_PIN, lvm_kp_tid);

	return( BB_SUCCESS );
}

/*
 *                    
 * NAME:	hd_bbdel     
 *                   
 * FUNCTION:	This routine deletes an entry from the physical volume's bad  
 *		block directory.                                             
 *                                                               
 * NOTES:	SEE BAD BLOCK DIRECTORY DESCRIPTION IN hd_bbadd
 *
 *	input:	ptr to pbuf with the BBdir to be updated
 *	output:	Bad Block Directory on disk updated with new bad block entry
 *
 * PARAMETERS:   pb - physical buf structure
 *                                                    
 * RETURN VALUE: BB_SUCCESS		Successful delete
 *		 BB_ERROR		Error during delete
 */

int
hd_bbdel(
register struct pbuf *bbpb)	/* ptr to bb_pbuf		*/
{

    register struct bb_hdr	*bb_hdr;	/* header of BBdir	*/
    register struct bb_entry	*bb_fentry;	/* first ent of BBdir	*/
    register struct bb_entry	*bb_centry;	/* current ent of BBdir	*/
    register struct bb_entry	*bb_lentry;	/* last ent of BBdir	*/
    register struct bb_entry	*bb_nentry;	/* ent after ent to	*/
						/* be deleted		*/
    register struct bad_blk	*bad_ptr;	/* ptr to active bad blk*/
    register size_t		size;		/* # bytes to move	*/
    register struct pbuf	*origpb;	/* original pbuf with	*/
						/* the bad block	*/

    /* get ptr to original pbuf */
    origpb = (struct pbuf *)(bbpb->pb.b_back);
    bad_ptr = origpb->pb_bad;

    TRCHKL2T(HKWD_KERN_LVM | hkwd_LVM_BBDEL, bad_ptr, bad_ptr->blkno);
	
    BUGLPR( debuglvl, BUGNTA, 
	("hd_bbdel: bbpb = 0x%x bad_ptr = 0x%x bad_blk = 0x%x\n",
					bbpb, bad_ptr, bad_ptr->blkno))

    /* set ptr to BBdir header */
    bb_hdr = (struct bb_hdr *) bbpb->pb_addr;

    /* set ptr to first and last entry in BBDir */
    bb_fentry = (struct bb_entry *)(bb_hdr + 1);
    bb_lentry = bb_fentry + (bb_hdr->num_entries - 1);

    /* Find the entry that matches the bad block structure	*/
    for(bb_centry = bb_lentry; bb_centry >= bb_fentry; bb_centry--) {

	/* if found the entry, break */
  	if( bad_ptr->blkno == (daddr_t)(bb_centry->bb_lsn) )
	    break;
    } 
    /*
     * if entry not found, error log problem with BBdir, and return
     * an error
     */
    if( bb_centry < bb_fentry ) {

	BUGLPR( debuglvl, BUGGID, 
	    ("hd_bbdel: ERROR BB entry not found for - 0x%x\n",
					bad_ptr->blkno))

	/* Log a BB directory corrupt error */
	hd_logerr( (unsigned)ERRID_LVM_BBDIRBAD, (ulong)(bbpb->pb.b_dev),
		   (ulong)(bad_ptr->blkno), (ulong)0 );

	origpb->pb_bbstat = BB_ERROR;
    	return( BB_ERROR );
    }

    /*
     * if the entry to be deleted is not the last entry in the bad block
     * directory, then all following entries must be moved up 
     */
    if( bb_centry < bb_lentry) {
    	bb_nentry = bb_centry + 1;

        /* calculate the number of bytes which need to be moved */
    	size = (size_t)((caddr_t)bb_lentry - (caddr_t)bb_centry);
	bcopy( (caddr_t)bb_nentry, (caddr_t)bb_centry, size );
    } 

    /*
     * since one entry has been deleted and all following entries moved up
     * in the bad block directory, zero out the old last entry
     */
    bzero (bb_lentry, sizeof (struct bb_entry));

    /*
     * decrement number of BB Directory entries in header on disk and
     * in pvol structure in kernel 
     */
    bb_hdr->num_entries--;	
    bbpb->pb_pvol->num_bbdir_ent--;	

    /*
     * Adjust last entry ptr to new last entry
     */
    bb_lentry -= 1;

    if( bad_ptr->blkno >= bbpb->pb_pvol->beg_relblk) {

	register daddr_t 	savebblk;	/* chained block	*/
	register struct bad_blk	*bad_lst;/* ptr to list of badblk struct*/

	/*
	 * We are dealing with a relocation block.  Therefore the original
	 * should have a status of REL_CHAINED.  Find it and change it's
	 * status to REL_DONE.
	 */
	savebblk = bad_ptr->blkno;

	for(bb_centry = bb_lentry; bb_centry >= bb_fentry; bb_centry--) {

	    if( savebblk == (daddr_t)(bb_centry->rel_lsn) ) {
		/*
		 * if block just found is not contained in the pool
		 * of reloc blocks, which means that it was the original
		 * bad block, exit the loop; otherwise, continue search
		 * for original bad block
		 */
		if( bb_centry->bb_lsn < bbpb->pb_pvol->beg_relblk ) 
		    break;
		savebblk = (daddr_t)(bb_centry->bb_lsn);
	    }

	} /* END FOR bb_centry = bb_lentry */

	/* if original bad blk not found */
	if( bb_centry < bb_fentry ) {
		
	    BUGLPR( debuglvl, BUGGID, 
		("hd_bbdel: ERROR original BB entry not found for - 0x%x\n",
					savebblk))

	    /* Log a BB directory corrupt error */
	    hd_logerr( (unsigned)ERRID_LVM_BBDIRBAD, (ulong)(bbpb->pb.b_dev),
		   (ulong)savebblk, (ulong)0 );

	    origpb->pb_bbstat = BB_ERROR;
	    return( BB_ERROR );
	}
	/*
	 * Find hash queue this block number is on then go find
	 * the bad_blk structure.
	 */
	bad_lst = HASH_BAD( origpb, bb_centry->bb_lsn );
	while( bad_lst->blkno != bb_centry->bb_lsn ) {
	    bad_lst = bad_lst->next;
	    assert( bad_lst != NULL );
	}

	/* Set status from REL_CHAINED to REL_DONE */
	bad_lst->status = REL_DONE;
		
	BUGLPR( debuglvl, BUGGID, 
	    ("hd_bbdel: changed bad_blk entry to REL_DONE - 0x%x\n",
					bad_lst->blkno))

    }

    return( BB_SUCCESS );
} 

/*
 *         
 * NAME:	hd_bbupd    
 *                        
 * FUNCTION:	This routine updates an entry in the physical volume's bad    
 *		block directory.                                             
 *                                                               
 * NOTES:	SEE BAD BLOCK DIRECTORY DESCRIPTION IN hd_bbadd
 *	
 *	input:	ptr to pbuf with the BBdir to be updated
 *	output:	Bad Block Directory entry on disk updated with new 
 * 			relocation address
 *
 * PARAMETERS:   pb - physical buf structure
 *                                                      
 * RETURN VALUE: BB_SUCCESS		Successful update
 *		 BB_ERROR		Error during update
 */


int
hd_bbupd(
register struct pbuf *bbpb)	/* ptr to bb_pbuf		*/
{
    register struct bb_hdr	*bb_hdr;	/* header of BBDir	  */
    register struct bb_entry	*bb_fentry;	/* first entry of BBdir	  */
    register struct bb_entry	*bb_lentry;	/* last entry of BBdir	  */
    register struct bb_entry	*bb_centry;	/* current entry of BBdir */
    register struct bb_entry	*bb_sentry;	/* saved entry of BBdir	    */
    register struct bad_blk	*bad_ptr;	/* ptr to active bad blk    */
    register daddr_t		sav_relblk;	/* save relocation blk addr */
    register size_t		size;		/* number of bytes to move  */
    register struct pbuf	*origpb;	/* orig pbuf with the bad blk */

    struct bb_entry	sav_bb;			/* place to save an entry   */
						/* during an update	    */

	/* get ptr to original pbuf */
	origpb = (struct pbuf *)(bbpb->pb.b_back);
	bad_ptr = origpb->pb_bad;
	/*
	 *  Search for an entry in the bad block directory 
	 *  to be udpated
	 */

	TRCHKL4T(HKWD_KERN_LVM | hkwd_LVM_BBUPD,
	    bad_ptr, bad_ptr->blkno, bad_ptr->relblk, bad_ptr->status);

	BUGLPR( debuglvl, BUGNTA, 
	    ("hd_bbupd: bbpb = 0x%x bad_ptr = 0x%x bad_blk = 0x%x\n",
					bbpb, bad_ptr, bad_ptr->blkno))

	bb_hdr = (struct bb_hdr *) bbpb->pb_addr;	/* pointer to header */

	/* set ptr to first and last entry in BBDir */
	bb_fentry = (struct bb_entry *)(bb_hdr + 1);
	bb_lentry = bb_fentry + (bb_hdr->num_entries - 1);

	/* Find the entry that matches the bad block structure	*/
	for(bb_centry = bb_lentry; bb_centry >= bb_fentry; bb_centry--) {
	    /* if found the entry, break */
  	    if( bad_ptr->blkno == (daddr_t)(bb_centry->bb_lsn) )
		break;
	} 

	/* if BB entry to be updated not found, log error and then return
	 * an error.
	 */
	if (bb_centry < bb_fentry) {

	    BUGLPR( debuglvl, BUGGID, 
		("hd_bbupd: ERROR BB entry not found for - 0x%x\n",
							bad_ptr->blkno))

	    /* Log a BB directory corrupt error */
	    hd_logerr( (unsigned)ERRID_LVM_BBDIRBAD, (ulong)(bbpb->pb.b_dev),
			(ulong)(bad_ptr->blkno), (ulong)0 );

	    origpb->pb_bbstat = BB_ERROR;
    	    return( BB_ERROR );
    	} 

	/*
	 *  If matching bad block entry shows that the bad block was      
	 *  previously relocated, then search forward through the bad block
	 *  directory looking for the relocated block to be recorded as a bad
	 *  block.  Continue search until the last relocated block in a    
	 *  possible chain has been found (a bad block with status of     
	 *  relocation desired).   The last entry in the relocation chain
	 *  is then the one that needs to be updated with the new relocation
	 *  block number.
	 */

	bb_sentry = bb_centry;		/* save ptr to entry to be updated */

	if( bb_centry->rel_stat != REL_DESIRED) {
	  	
	    /* if reloc status for the found bad block was not relocation
	     * desired, meaning that it was previously relocated & an error
	     * has now been found on a relocation block, then search forward
	     * through the bad block directory to find this entry's reloc
	     * block as a bad block entry
	     */

	    /* save reloc blk address of entry to be updated */
	    sav_relblk = (daddr_t)(bb_centry->rel_lsn);

	    BUGLPR( debuglvl, BUGGID, 
		("hd_bbupd: scanning forward for BB entry for - 0x%x\n",
								sav_relblk))

	    /* search forward from the entry to be updated to find
	     * an entry where the bad block number matches the saved
	     * reloc blk number 
	     */
	    for( bb_centry++; bb_centry <= bb_lentry; bb_centry++ ) {

		if( sav_relblk == bb_centry->bb_lsn ) {
		    /* now save this reloc block and ptr to entry, then
		     * keep searching to see if the relocation chain
		     * continues
		     */
	  	    sav_relblk = (daddr_t)(bb_centry->rel_lsn);
		    bb_sentry = bb_centry;
		    if( bb_centry->rel_stat == REL_DESIRED )
			break;
		}
	    }
	} 

	/* 
	 *  Now, update the saved entry in the bad block directory. 
	 */

	bb_sentry->rel_stat = bad_ptr->status;
	bb_sentry->rel_lsn = bad_ptr->relblk;


	/*
	 *  See if the bad block entry that was just updated 
	 *  needs to be moved in order to keep the bad block directory entries
	 *  in ascending order by relocation block number (other than for    
	 *  entries with a status of relocation desired, which may be       
	 *  interspersed).                                                 
	 */

	/* starting from the end of the directory, search backwards up until
	 * the entry following the updated entry, looking for the first entry
	 * whose status is not relocation desired (because last entry with a
	 * status other than relocation desired should be entry with largest
	 * relocation block number and all new relocation blocks will be
	 * greater than previous values)
	 */
	for (bb_centry = bb_lentry; bb_centry > bb_sentry; bb_centry-- ) {

	    if( bb_centry->rel_stat == REL_DONE )
		break;		/* exit, entry with largest rel blk has
				 * been found */
	}

	/*
	 * if current entry is greater than the saved entry then save the
	 * data in the saved entry and move all entries from the saved 
	 * entry + 1 to the current entry up 1 entry.  Then put the saved
	 * data in the current entry.
	 */
	if( bb_centry > bb_sentry ) {

	    BUGLPR( debuglvl, BUGGID, 
		("hd_bbupd: moving BB entry for 0x%x to end of directory\n",
							bb_sentry->bb_lsn))

	    size = (size_t)((caddr_t)bb_centry - (caddr_t)bb_sentry);
	    sav_bb = *bb_sentry;

	    /* move data forward one entry, leaving the pointer which
	     * previously pointed at the entry with largest relocation block
	     * as the position where the data for the updated entry will now
	     * be placed
	     */
	    bcopy( (caddr_t)(bb_sentry + 1), (caddr_t)bb_sentry, size );

	    *bb_centry = sav_bb;
	}

	if( bad_ptr->blkno >= bbpb->pb_pvol->beg_relblk ) {

	    for(bb_centry = bb_lentry; bb_centry >= bb_fentry; bb_centry--) {
		/*
		 * if a match, set error field in original pbuf to indicate
		 * bad blk number is changed
		 */
	      	if( bad_ptr->blkno == (daddr_t)(bb_centry->rel_lsn) ) {

		    origpb->pb_bbstat = BB_CRB;
		    /*
		     * Remember original bad block so we can get to the
		     * hash queue it is on later.
		     */
		    bad_ptr->blkno = (daddr_t)(bb_centry->bb_lsn);

		    /*
		     * if block just found is not contained in the pool
		     * of reloc blocks, which means that it was the original
		     * bad block, exit the loop; otherwise, continue search
		     * for original bad block
		     */
		    if( bb_centry->bb_lsn < bbpb->pb_pvol->beg_relblk ) 
		      		break;
		}

	    } /* END FOR bb_centry = bb_lentry */

	    /* if original bad blk not found */
	    if( bb_centry < bb_fentry ) {
		
		BUGLPR( debuglvl, BUGGID, 
		    ("hd_bbupd: ERROR original BB entry not found for - 0x%x\n",
					bad_ptr->blkno))

		/* Log a BB directory corrupt error */
		hd_logerr((unsigned)ERRID_LVM_BBDIRBAD,(ulong)(bbpb->pb.b_dev),
			(ulong)(bad_ptr->blkno), (ulong)0 );

		origpb->pb_bbstat = BB_ERROR;
		return( BB_ERROR );
	     }
	}
	return( BB_SUCCESS );
}

/*
 *  NAME:	hd_chk_bbhld 
 * 
 *  FUNCTION:	Check the volgrp's bad block hold queue.  Start up any request
 *		waiting on the volgrp's pbuf, if any.    
 *
 *  NOTES:
 *
 *	output:	request at head of BB queue, if any, will be taken off and
 * 		allowed to update the BBdir
 *
 *  DATA STRUCTS: 
 *
 *  RETURN VALUE: none
 *
 */
void
hd_chk_bbhld( void )
{
			
    register struct pbuf	*pb;	/* request removed from BB hld que  */
    register struct pvol	*pvol;	/* pointer to target pvol	    */
    register struct bad_blk	*bad;

    register int	bbdir_bytes;	/* number of bytes of BBdir to read */

    BUGLPR( debuglvl, BUGNTA, 
	("hd_chk_bbhld: bb_hld.head 0x%x\n", bb_hld.head))

    /* if bad block hold queue not empty, remove pbuf from head & 
     * restart it's processing 
     */
    if (bb_hld.head) {

	/* remove pbuf from head of queue */
	pb = (struct pbuf *) bb_hld.head;
	bb_hld.head = bb_hld.head->av_forw;
	if (bb_hld.head == NULL)
		bb_hld.tail = NULL;

	pb->pb.av_forw = NULL;

	/* get the pvol associated with this request */
	pvol = pb->pb_pvol;

	/*
	 * If this PV is read only relocation we can error off this
	 * request now.  Just have to figure out what to do.
	 */
	while( pvol->pvstate == PV_RORELOC ) {
	    /* If no error flag make it an error */
	    if( !(pb->pb.b_flags & B_ERROR) ) {
		pb->pb.b_flags |= B_ERROR;
		pb->pb.b_error = EIO;
	    }

	    BUGLPR( debuglvl, BUGGID, 
		("hd_chk_bbhld: unloading bb_hld Q 0x%x pb_bbop = 0x%x\n",
							pb, pb->pb_bbop))

	    bad = pb->pb_bad;
	    switch( pb->pb_bbop ) {

		case BB_ADD:
		    /*
		     * If the block is in the relocation pool then find
		     * the original one then fall through to mark it
		     * desired.
		     */
		    if( bad->blkno >= pvol->beg_relblk)
			bad = hd_fndbbrel( pb );

		case BB_DELETE:
		case BB_UPDATE:

		    bad->status = REL_DESIRED;
		    bad->relblk = 0;

		    break;

		default:
		    panic("hd_chk_bbhld: unknown bad block operation");
	    }
	    /*
	     * return request to scheduler
	     */
	    HD_SCHED( pb );
	    if( bb_hld.head ) {
		/* remove pbuf from head of queue */
		pb = (struct pbuf *) bb_hld.head;
		bb_hld.head = bb_hld.head->av_forw;
		if (bb_hld.head == NULL)
			bb_hld.tail = NULL;
		/* get the pvol associated with this request */
		pvol = pb->pb_pvol;
	    }
	    else {
		/*
		 * queue is empty, reset busy flag in the bb_pbuf
		 * and return
		 */
	
		bb_pbuf->pb.b_flags &= ~B_BUSY;
		bb_pbuf->pb_bbop = 0;
		return;
	    }
	}

	/* start up this request's updating of the BBdir */
	bb_pbuf->pb.b_flags &= ~B_BUSY;
	hd_upd_bbdir( pb );
    }
    else { /* queue is empty, reset busy flag in the bb_pbuf */
	
	bb_pbuf->pb.b_flags &= ~B_BUSY;
	bb_pbuf->pb_bbop = 0;
    }

    return;
}
		
/*
 *  NAME:	hd_bbdirdone 
 * 
 *  FUNCTION:	routine for completion of Bad Block Directory updating
 *
 *  NOTES:	This routine checks on the completion status of the BBdir 
 *		updating and then continues the processing of the original
 *		request that had the bad block.
 *
 *  PARAMETERS:
 *
 *  DATA STRUCTS: 
 *
 *  RETURN VALUE: none
 *
 */
void
hd_bbdirdone(
register struct pbuf *origpb)		/* original request to process */
{

    register struct bad_blk	*bad_lst;/* ptr to list of badblk struct*/
    register struct bad_blk	*bad_one;/* ptr to badblk struct from pbuf*/

    TRCHKL3T(HKWD_KERN_LVM | hkwd_LVM_BBDIRDONE,
		origpb, origpb->pb_bbstat, origpb->pb_bbop);

    BUGLPR( debuglvl, BUGNTA, 
	("hd_bbdirdone: origpb = 0x%x pb_bbstat = 0x%x\n",
					origpb, origpb->pb_bbstat))
    BUGLPR( debuglvl, BUGNTA, 
	("                        pb_bbop = 0x%x\n", origpb->pb_bbop))

    /*
     * Put the bad_blk structure ptr from pbuf here
     */
    bad_one = origpb->pb_bad;
    /* if BBdir processing worked, do any special processing
     * and then continue with the normal request processing 
     */
    if ( origpb->pb_bbstat < BB_ERROR) {

	/* if found that the bad block that was relocated
	 * was a SW reloc blk, then it changed the bad blkno to 
	 * the original bad blkno.  This can only happen on an add.
	 *
	 * Note:  the relocation chains are NOT kept in the kernel
	 * copy of the bad block directory - only the original bad
	 * block number and the relocation block number at the end
	 * of the relocation chain.
	 */
	if( origpb->pb_bbstat == BB_CRB ) { /* Change Reloc Blk */
	    /*
	     * find the original badblk structure for bad->blkno
	     * and change it's relblk and status field to the
	     * bad->relblk value
	     */
	    bad_lst = HASH_BAD( origpb, bad_one->blkno );
	    while((bad_lst->blkno != bad_one->blkno) && (bad_lst->next != NULL))
		bad_lst = bad_lst->next;
	    ASSERT(bad_lst->blkno == bad_one->blkno);
	    /*
	     * restore the original badblk struct's relblkno, 
	     * this is the old value of bad_one->blkno
	     * that we need to delete it's badblk struct 
	     */                          
	    bad_one->blkno  = bad_lst->relblk;
	    /*
	     * IF status is done then we can move new status and relocation
	     * block to original bad block structure and remove the one
	     * bad_one points to. ELSE we set the original bad block
	     * structure to chained so chkblk can find it.
	     */

	    BUGLPR( debuglvl, BUGGID, 
		("hd_bbdirdone: bad_one->status = 0x%x bad_lst = 0x%x\n",
					bad_one->status, bad_lst))

	    if( bad_one->status == REL_DONE ) {
		bad_lst->relblk = bad_one->relblk;
		bad_lst->status = bad_one->status;

		/* remove bad_blk struct from hash queue and release it */
		hd_dqbblk( origpb, bad_one->blkno);
		REL_BBLK( bad_one );
		origpb->pb_bad = NULL;
	    }
	    else {
		bad_lst->status = REL_CHAINED;
	    }
	}

	/* if BB_DELETE updating worked we still need to delete
	 * the kernel bad blk structure since the HW relocation worked.
	 */
	if (origpb->pb_bbop == BB_DELETE) {

	    /* remove bad_blk struct from hash queue and release it */
	    hd_dqbblk( origpb, bad_one->blkno);
	    REL_BBLK( bad_one );
	    origpb->pb_bad = NULL;
	}
    }
    else {	/* update failed */
	/*
	 * Now we have a mess.  The updating of the BB directory has
	 * failed.  Attempt to backout of this mess.  Since we can not
	 * assume that the BB directory contains the the last operation
	 * we must error off the original error and update the kernel
	 * structures to not allow IO to the block.
	 */
	origpb->pb.b_flags |= B_ERROR;
	origpb->pb.b_error = EIO;

	BUGLPR( debuglvl, BUGGID, 
	    ("hd_bbdirdone: bad_one = 0x%x\n", bad_one))

	switch( origpb->pb_bbop ) {

	    case BB_ADD:
		/*
		 * Since add may have been adding a bad relocation block
		 * find the entry that matches to blkno
		 */
		bad_lst = HASH_BAD( origpb, bad_one->blkno );
		while( (bad_lst->blkno != bad_one->blkno) &&
			(bad_lst->next != NULL)) {

		    bad_lst = bad_lst->next;
		}
		ASSERT(bad_lst->blkno == bad_one->blkno);
		/*
		 * Now mark it desired to error off any new requests
		 */
		bad_lst->relblk = 0;
		bad_lst->status = REL_DESIRED;

		BUGLPR( debuglvl, BUGGID, 
		    ("hd_bbdirdone: bad_lst = 0x%x\n", bad_lst))

		break;

	    case BB_DELETE:
	    case BB_UPDATE:

		bad_one->relblk = 0;
		bad_one->status = REL_DESIRED;

		break;

	    default:
		panic("hd_bbdirdone: unknown bad block operation");
	}
    }

    /* Let the request continue */
    origpb->pb_bbop = 0;

    origpb->pb.b_iodone = (void (*)())hd_end;
    PB_CONT( origpb );

    /*
     * Go see if any request waiting for bad block processing
     */
    hd_chk_bbhld();
}
		
/*
 *  NAME:	hd_logerr 
 * 
 *  FUNCTION:	Fill in error log entry and make the call to log it
 *
 *  NOTES: 	This function fills in an error log entry under control
 *		of the id that it is given.  It then makes the call to 
 *		log the error.  Since this function is called from the
 *		bottom half of the driver it runs at INTIODONE and therefore
 *		can not page fault.
 *
 *  PARAMETERS:
 *
 *  DATA STRUCTS: 
 *
 *  RETURN VALUE: none
 *
 */
void
hd_logerr(
register unsigned id,		/* original request to process	*/
register ulong dev,		/* device number		*/
register ulong arg1,
register ulong arg2)
{
    /*
     * the dd pointer will be used as pointer into the detailed data.  Since
     * this is handled as a character array and is used by all LVDD errors
     * special knowledge is needed since this data will be different dependent
     * on the error type.
     */
    register char		*dd;

    struct hd_errlog_ent	log_ent;	/* Error log entry 	  */

    BUGLPR( debuglvl, BUGNTA, 
	("hd_logerr: id = 0x%x dev = 0x%x arg1 = 0x%x arg2 = 0x%x\n",
					id, dev, arg1, arg2))
    /*
     * Fill in constants for all entries after zeroing it
     */
    bzero( &log_ent, sizeof(struct hd_errlog_ent) );
    log_ent.id.error_id = id;
    strcpy( log_ent.id.resource_name, RESRC_NAME );

    /* set up detailed data pointer and zero the detailed data area	*/
    dd = log_ent.de_data;

    /* Stuff the device number since it is common to all errors */
    *((ulong *)dd) = dev;
    dd += sizeof(ulong);	/* bump the detailed data pointer	*/

    /*
     * Now stuff the specific error data under control of the error type
     */
    switch( id ) {

	case ERRID_LVM_SWREL:		/* SW relocation successful	*/

	    /* arg1 - block number  arg2 - relocation block number */
	    *((ulong *)dd) = arg1;
	    dd += sizeof(ulong);
	    *((ulong *)dd) = arg2;

	    break;

	case ERRID_LVM_HWREL:		/* HW relocation successful	*/
	case ERRID_LVM_HWFAIL:		/* HW relocation failed		*/
	case ERRID_LVM_BBRELMAX:	/* Max BB retries reached	*/
	case ERRID_LVM_BBDIRBAD:	/* BB directory corrupted	*/

	    /* arg1 - block number  arg2 - undefined */
	    *((ulong *)dd) = arg1;

	    break;

	case ERRID_LVM_BBFAIL:		/* Non media error during SW rel*/

	    /* arg1 - block number  arg2 - b_error */
	    *((ulong *)dd) = arg1;
	    dd += sizeof(ulong);
	    *dd = (char)arg2;

	    break;

	case ERRID_LVM_BBDIRERR:	/* BB directory IO error	*/

	    /* arg1 - b_error  arg2 - directory operation */
	    *dd = (char)arg1;
	    dd++;
	    *dd = (char)arg2;

	    break;

	case ERRID_LVM_BBDIR90:		/* BB directory is over 90% full*/

	    /* arg1 - current num of entries  arg2 - max num of entries	*/
	    *((ushort *)dd) = (ushort)arg1;
	    dd += sizeof(ushort);;
	    *((ushort *)dd) = (ushort)arg2;

	    break;

	case ERRID_LVM_BBEPOOL:		/* Relocation pool is empty	*/
	case ERRID_LVM_BBDIRFUL:	/* BB directory is full		*/

	    break;

	default:

	    /* arg1 - ulong,  arg2 - ulong				*/
	    *((ulong *)dd) = (ulong)arg1;
	    dd += sizeof(ulong);;
	    *((ulong *)dd) = (ulong)arg2;

	    break;
    }

    /*
     * log the error
     */
    errsave( &log_ent, sizeof(struct hd_errlog_ent) );

    return;
}

