static char sccsid[] = "@(#)52	1.12  src/bos/kernext/lvm/hd_dump.c, sysxlvm, bos411, 9428A410j 3/14/94 17:41:46";

/*
 * COMPONENT_NAME: (SYSXLVM) Logical Volume Manager Device Driver - 52
 *
 * FUNCTIONS: hd_dump, hd_dmpxlate
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *
 *  hd_dump.c -- Logical Volume Manager dump interface
 *
 *	This module contains the routines to allow the LVM
 *	device driver to act as a system dump device. 
 *
 *  EXECUTION ENVIRONMENT:
 *
 *	These routines are pinned at DUMPINIT time.  When called after
 *	DUMPINIT they can not page fault and should not use any system
 *	services other than the underlying disk device drivers.
 */

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/pin.h>
#include <sys/uio.h>
#include <sys/dump.h>
#include <sys/device.h>
#include <sys/dasd.h>
#include <sys/hd.h>
#include <sys/seg.h>
#include <sys/malloc.h>

struct uio	lv_uiop;	/* LVM uio structure			*/
struct iovec	lv_iov;		/* LVM iovec structure			*/
struct xmem	lv_xmem;	/* LVM xmem structure			*/
struct dmp_query lv_dqry;	/* LVM dmp_query structure		*/

dev_t		lv_pdev;	/* DUMPWRITE target physical device	*/
dev_t		lv_cnt_dev=-1;	/* Device represented by lv_dmp_cnt/dev	*/
int		lv_dmp_cnt=0;			/* LV dump device count	*/
struct pvol	*lv_dmp_dev[MAXPVS + 1];	/* LV dump device table	*/

static int	nvglist;		/* array entries of vglistp	*/

static struct vglist {
	struct volgrp	*vg;		/* address of volgrp struct	*/
	long		major_num;	/* major number of volume group	*/
} *vglistp;

static struct {				/* LVM component dump table	*/
    struct cdt_head  _cdt_head;
    struct cdt_entry  cdt_entry[2];
} lvmcdt = {
    { DMP_MAGIC, "lvm", sizeof(lvmcdt) },
    {{ "dmpbuf", sizeof(struct pbuf *), (char *) &hd_dmpbuf, KERNELSEGVAL },
     { "vglist", 0 /* len */, 0 /* addr */, KERNELSEGVAL }}
};

/*
 *  NAME:         hd_dump 
 * 
 *  FUNCTION:     Provides system interface to LVM dump device
 *  
 *  EXECUTION ENVIRONMENT:
 *
 *	This function is pinned at DUMPINIT time.  When called after
 *	DUMPINIT can not page fault and should not use any system
 *	services other than the underlying disk device drivers dump
 *	interfaces.
 *
 *  NOTES:
 *
 *  DATA STRUCTS: 
 *
 *  RETURNS: Lower Level	Any errors from lower layers or hd_dmpxlate
 *	     ENXIO		LV(minor number) does not exist
 *	     EROFS		LV options indicate read only
 *	     ENXIO		LV has more than 1 copy(mirror)
 *	     EINVAL		The dump is out of sequence
 *
 *  EXTERNAL PROCEDURES:
 *		  devdump
 *		  hd_dmpxlate
 *
 */
int
hd_dump(

dev_t		dev,	/* major/minor of LV				*/
struct uio	*uiop,	/* ptr to uio struct describing operation	*/
int		cmd,	/* dump command					*/
char		*arg,	/* command dependent - ptr to dmp_query struct	*/
int		chan,	/* not used					*/
int		ext)	/* not used					*/

{
	register struct lvol		*lv;	/* LV ptr from volgrp	  */
	register struct part		*part;	/* PP part structure ptr  */
	register struct dmp_query	*dmp_ptr;
	register struct iovec		*iovp;	/* iovec structure ptr	*/

	register int			i, j;
	register int			rc;	/* function return code	*/
	register int			xfercnt;/* orignal write xfer count */

	struct volgrp			*vg;	/* VG ptr from devsw	  */

	/*
	 * Get volgrp pointer from device switch table and LV pointer from it
	 */
	(void) devswqry( (dev_t) dev, (uint *) NULL, (caddr_t *) &vg );
	lv = VG_DEV2LV( vg, dev );

	/*
         * verify that the logical volume is defined to the driver
	 */
	if( lv == NULL )
		return(ENXIO);  	/* no such LV in VG */

	/*
	 * If logical volume is read only then error off.
	 */
	if( lv->lv_options & LV_RDONLY )
		return(EROFS);

	/*
	 * Verify the LV has at least 1 partition and only 1 copy
	 */
	if( lv->nparts != 1 ) {
		rc = ENXIO;
	}

	if( lv_cnt_dev != dev ) {
		/*
		 * Scan array of parts structures and build a list
		 * of disk devices(pvol pointers) the LV spans
		 */
		lv_cnt_dev = dev;	/* Remember device number	*/
		lv_dmp_cnt = 1;
		lv_dmp_dev[0] = lv->parts[0]->pvol;
		lv_dmp_dev[1] = NULL;
		for( i=1; i < BLK2PART(vg->partshift,lv->nblocks); i++ ) {
		    part = PARTITION(lv, i, PRIMMIRROR);
		    /*
		     * If there is a pvol pointer then scan list of known
		     * LV disks
		     */
		    if( part->pvol )
		      for( j=0; j <= i; j++ ) {
			/*
			 * If pvol pointer matches then this partition is on
			 * a same device as a previous one
			 */
			if( part->pvol == lv_dmp_dev[j] )
			    break;

			/* If at end of list save pvol pointer */
			if( lv_dmp_dev[j] == NULL ) {
			    lv_dmp_dev[j] = part->pvol;
			    lv_dmp_dev[j+1] = NULL;
			    lv_dmp_cnt++;
			    break;
			}
		      }
		}
	}
	switch( cmd ) {

	    case DUMPINIT:
		
		/* If LV_DMPDEV is not 0 then we have a DUMPINIT 
		 * without a DUMPTERM */
		if( lv->lv_options & LV_DMPDEV ) {
		    rc = EINVAL;
		    break;
		}

		/*
		 * For each entry in LV dump device table call the driver
		 * so it can do it's DUMPINIT stuff
		 */
		for( i=0; i < lv_dmp_cnt; i++ ) {
		   rc=devdump(lv_dmp_dev[i]->dev,NULL,DUMPINIT,NULL,NULL,NULL);
		   if( rc ) {
			for( j=(i - 1); j >= 0; j-- ) {

			    /* We have an error from below.  Attempt to back
			     * out by issuing a DUMPTERM to the drivers that
			     * we have issued a DUMPINIT to. Ignore the 
			     * return value. */
			    devdump(lv_dmp_dev[j]->dev,NULL,DUMPTERM,
							NULL,NULL,NULL);
			}

			break;
		   }
		}

		/* If there were no problems pin hd_dump() and the data */
		if( rc == 0 ) {
		    rc = (int) pincode( (int (*)()) hd_dump );
		    if( rc ) {
			for( j=(lv_dmp_cnt - 1); j >= 0; j-- ) {

			    /* We have a pincode error.  Attempt to back
			     * out by issuing a DUMPTERM to the drivers that
			     * we have issued a DUMPINIT to. Ignore the 
			     * return value. */
			    devdump(lv_dmp_dev[j]->dev,NULL,DUMPTERM,
							NULL,NULL,NULL);
			}
			break;
		    }

		    /* No problems set flag showing the LV is a DUMP device */
		    lv->lv_options |= LV_DMPDEV;
		}
		break;

	    case DUMPSTART:

		/* If LV_DMPDEV is 0 then we have not had a successful 
		 * DUMPINIT */
		if( (lv->lv_options & LV_DMPDEV) == 0 ) {
		    rc = EINVAL;
		    break;
		}

		/* If dump in progress flag is on then return error */
		if( lv->lv_options & LV_DMPINPRG ) {
		    rc = EINVAL;
		    break;
		}

		/*
		 * For each entry in LV dump device table call the driver
		 * so it can do it's DUMPSTART stuff
		 */
		for( i=0; i < lv_dmp_cnt; i++ ) {
		  rc=devdump(lv_dmp_dev[i]->dev,NULL,DUMPSTART,NULL,NULL,NULL);
		  if( rc ) {
			for( j=(i - 1); j >= 0; j-- ) {

			    /* We have an error from below.  Attempt to back
			     * out by issuing a DUMPEND to the drivers that
			     * we have issued a DUMPSTART to. Ignore the 
			     * return value. */
			    devdump(lv_dmp_dev[j]->dev,NULL,DUMPEND,
							NULL,NULL,NULL);
			}
			break;
		  }
		}

		/* If no errors then set dump in progress flag */
		if( rc == 0 )
		    lv->lv_options |= LV_DMPINPRG;

		break;

	    case DUMPQUERY:

		dmp_ptr = (struct dmp_query *)arg;
		/*
		 * These constants are not entirely arbitrary.  Of course
		 * DBSIZE is reasonable and many assumptions are made using
		 * this value.  But BYTEPTRK may seem a bit odd since it
		 * seems the maximum value for LVM should be considerably 
		 * higher.  BYTEPTRK is used to limit the number of bad block
		 * hash chains that must be scanned to check for any relocated
		 * blocks the request spans.  These chains are maintained on a
		 * Logical Track Group basis.  A LTG is 128K.
		 */
		dmp_ptr->min_tsize = DBSIZE;
		dmp_ptr->max_tsize = BYTEPTRK;

		/*
		 * For each entry in LV dump device table call the driver
		 * so it can do it's DUMPQUERY stuff
		 */
		for( i=0; i < lv_dmp_cnt; i++ ) {
		    rc=devdump(lv_dmp_dev[i]->dev,NULL,DUMPQUERY,&lv_dqry,
								NULL,NULL);
		    if( rc ) {		/* Error from below	*/
			dmp_ptr->min_tsize = 0;
			dmp_ptr->max_tsize = 0;
			break;
		    }

		    /* Save off the largest minimum and the smallest maximum */
		    if( lv_dqry.min_tsize > dmp_ptr->min_tsize )
			dmp_ptr->min_tsize = lv_dqry.min_tsize;

		    if( lv_dqry.max_tsize < dmp_ptr->max_tsize )
			dmp_ptr->max_tsize = lv_dqry.max_tsize;
		}
		if( rc == 0 ) {
		    /* If there is no error check for the following:
		     * If min is not equal to DBSIZE, or max is zero,
		     * or min > max then we have an error.
		     * *NOTE* relocation is done in DBSIZE blocks.  Therefore
		     * if we have a device that does not have a minimum of
		     * DBSIZE we have a problem.
		     */
		    if(	 (dmp_ptr->min_tsize != DBSIZE) ||
			!(dmp_ptr->max_tsize) ||
			 (dmp_ptr->min_tsize > dmp_ptr->max_tsize) ) {

			dmp_ptr->min_tsize = 0;
			dmp_ptr->max_tsize = 0;
			rc = EINVAL;
		    }
		}
		break;

	    case DUMPEND:

		/* For each entry in LV dump device table call the driver
		 * so it can do it's DUMPEND stuff.  Do all drivers and
		 * remember the first error value if any. */
		j = 0;
		for( i=0; i < lv_dmp_cnt; i++ ) {
		   rc=devdump(lv_dmp_dev[i]->dev,NULL,DUMPEND,NULL,NULL,NULL);
		   /* remember the first error return value */
		   if( rc && (j == 0) ) {
			j = rc;
			break;
		   }
		}
		rc = j;

		/* Reset dump in progress flag	*/
		lv->lv_options &= ~LV_DMPINPRG;

		break;

	    case DUMPTERM:

		/* For each entry in LV dump device table call the driver
		 * so it can do it's DUMPTERM stuff. Do all drivers and
		 * remember the first error value if any. */
		j = 0;
		for( i=0; i < lv_dmp_cnt; i++ ) {
		   rc=devdump(lv_dmp_dev[i]->dev,NULL,DUMPTERM,NULL,NULL,NULL);
		   /* remember the first error return value */
		   if( rc && (j == 0) ) {
			j = rc;
			break;
		   }
		}

		/* Unpin code and data.  If an error is returned then return
		 * that error else reset LV_DMPDEV flag and return 
		 * any errors from lower drivers */
		rc = (int) unpincode( (int (*)()) hd_dump );
		if( rc == 0 ) {
		    lv->lv_options &= ~LV_DMPDEV;
		    lv_cnt_dev = -1;	/* Don't remember device number	*/
		    rc = j;
		}

		break;

	    case DUMPWRITE:

		if( (lv->lv_options & LV_DMPINPRG) == 0 ) {
		    rc = EINVAL;
		    break;
		}

		/* Call hd_dmpxlate and devdump until the request is
		 * satisfied.  If an error is returned then
		 * abort and bubble the error back to the caller.
		 */
		rc = 0;		/* Initialize this */
		xfercnt = uiop->uio_resid;	/* save initial xfer count */
		while( (uiop->uio_iovcnt) && (rc == 0) ) {
		    iovp = uiop->uio_iov;
		    rc = hd_dmpxlate( dev, uiop, vg );
		    if( rc ) {
			/* Error during translation.  The LV does not 
			 * allow relocation, or a non relocated block
			 * was encountered, or the request was at or
			 * past the end of the LV. */
			if( rc == ENXIO ) {
			    /* Request is at End of Media(EOM).  If no
			     * bytes transfered then request started at EOM
			     * and ENXIO is returned.  If some bytes xfered
			     * then request crosses EOM, no error is
			     * returned and resid equals length of request
			     * not completed. */
			    if( xfercnt != uiop->uio_resid )
				rc = 0;
			}
			break;
		    }

		    /* save transfer length in "i" for now	*/
		    i = lv_uiop.uio_resid;
		    rc=devdump(lv_pdev, &lv_uiop, DUMPWRITE,NULL,NULL,NULL);
		    if( rc ) {
			/* Error during write	*/
			break;
		    }
		    /* No problems adjust uio_resid, uio_offset, iov_len,
		     * and iov_base.	*/
		    uiop->uio_resid -= i;
		    uiop->uio_offset += i;
		    iovp->iov_len -= i;
		    iovp->iov_base += i;

		    /* If that is all for this iovec then increment uio_iov
		     * pointer, iovec done count, and decrement uio_iovcnt.
		     * If there is a xmem pointer bump that also.
		     */
		    if( (iovp->iov_len) == 0 ) {
			uiop->uio_iovcnt--;
			uiop->uio_iovdcnt++;
			uiop->uio_iov++;
			if( uiop->uio_xmem )
			    uiop->uio_xmem++;
		    }
		}

		break;

	    default:

		/* Invalid dump command */
		rc = EINVAL;
		break;

	} /* End of switch */

	return( rc );
}

/*
 *  NAME:	hd_dmpxlate 
 * 
 *  FUNCTION:	Provides translation from the logical request to a
 *	 	physical request.
 *  
 *  EXECUTION ENVIRONMENT:
 *
 *	This function is pinned and DUMPINIT time and when call is not
 *	allowed to page fault.
 *
 *  NOTES:	If necessary the request is broken up to accommodate
 *		the end of a track group, partition, or any bad blocks.
 *		If the LV does not allow any relocation EMEDIA is returned.
 *		If it encounters a defected block that has not been relocated
 *		it returns EMEDIA.  An error is return since we assume
 *		the system has crashed.  Therefore we would not be able to
 *		update the bad block directory on the effected disk.
 *
 *  DATA STRUCTURES:	Uio struct lv_uiop, lv_iov, lv_xmem updated with
 *			request for lower physical layers.
 *	
 *  RETURNS: success		A uio struct ready for the lower levels
 *	     ENXIO		Request goes past end of the LV
 *	     EMEDIA		Request crosses a relocated block and the
 *				LV options does not allow relocation
 *	     EMEDIA		Request crosses a defective block that
 *				has not been relocated
 *
 *  EXTERNAL PROCEDURES:
 */
int
hd_dmpxlate(
register dev_t		dev,	/* major/minor of LV			*/
register struct uio	*luiop,	/* ptr to logical uio structure		*/
register struct volgrp	*vg)	/* ptr to VG from device switch tabel	*/
{
	register struct lvol	*lv = VG_DEV2LV(vg,dev);
	register struct part	*part;	/* PP part structure ptr	*/
	register struct bad_blk	*bad;	/* Bad block structure ptr	*/
	register struct iovec	*liov;	/* Logical iovec structure ptr	*/

	register long	reqblks;	/* Size of requests in blks	*/
	register long	lreqstrt;	/* Starting logical blk number	*/
	register long	preqstrt;	/* Starting physical blk number	*/
	register long	p_no;		/* Logical partition number	*/
	register long	l_trk_no;	/* Logical track group number	*/
	register long	temp;		/* Temp. variable		*/

	/* Set up logical iovec and xmem pointers	*/
	liov = luiop->uio_iov;
	/*
	 * Set up the constant fields in the physical uio structure
	 */
	lv_uiop.uio_iov		= &lv_iov;
	/* copy xmem structure if needed */
	if( luiop->uio_xmem ) {
	    lv_xmem		= *(luiop->uio_xmem);
	    lv_uiop.uio_xmem	= &lv_xmem;
	}
	else
	    lv_uiop.uio_xmem	= NULL;
	lv_uiop.uio_iovcnt	= 1;
	lv_uiop.uio_iovdcnt	= 0;
	lv_uiop.uio_segflg	= luiop->uio_segflg;
	lv_uiop.uio_fmode	= luiop->uio_fmode;
	lv_iov.iov_base		= liov->iov_base;

	/* Convert request offset and length to blocks, calculate the 
	 * logical track group, find the logical partition number and
	 * parts structure, calculate the starting physical block. */
	lreqstrt= BYTE2BLK( luiop->uio_offset );
	reqblks	= BYTE2BLK( liov->iov_len );
	l_trk_no= BLK2TRK( lreqstrt );
	p_no	= BLK2PART( vg->partshift, lreqstrt );
	part	= PARTITION( lv, p_no, PRIMMIRROR );
	preqstrt= part->start + (lreqstrt - PART2BLK(vg->partshift, p_no));

	/*
	 * Return ENXIO if starting block is past the end of the LV
	 */
	if( lreqstrt >= lv->nblocks )
		return( ENXIO );

	/*
	 * Determine if starting offset plus length crosses a logical 
	 * track group boundary.  This will also align to partition
	 * boundaries.
	 */
	temp = (lreqstrt + reqblks) - (TRK2BLK(l_trk_no + 1));
	if( temp > 0 ) {
	    /* request crosses a track boundary	*/
	    /* Adjust the request length to fit in LTG	*/
	    reqblks -= temp;
	}

	/*
	 * Determine if the request crosses any relocated blocks.
	 * If so and the LV allows Bad Block Relocation then the following
	 * scenario will apply:
	 *
	 * Adjust the request to write up to the relocated block.
	 * On the next pass the relocated block will be the first one in the
	 * request.  Write that one relocated block.  etc...
	 */
	bad = HASH_BAD_DMP( part->pvol, preqstrt );
	for( ;; ) {
	    if( bad == NULL )
		/* No bad blocks on this chain		*/
		break;

	    /* We have bad blocks on this chain.  See if request crosses any */
	    if( preqstrt > bad->blkno ) {
		/* If current request starts above this entry check next one */
		bad = bad->next;
		continue;
	    }

	    if( (preqstrt + reqblks) <= bad->blkno ) {
		/* If starting blk + length does not reach this entry this   */
		/* request does not cross any relocated blocks.		     */
		break;
	    }

	    if( preqstrt < bad->blkno ) {
		/* Good blks precede relocated one do them first	*/
		reqblks = bad->blkno - preqstrt;
		break;
	    }

	    if( lv->lv_options & LV_NOBBREL ) {
		/* LV does not allow BB relocation			*/
		return( EMEDIA );
	    }

	    /*
	     * We have found a block that has been detected as bad 
	     * If the block has not already been relocated return an error.
	     * If the block has been relocated continue.
	     */

	    if( bad->status != REL_DONE )
		return( EMEDIA );

	    preqstrt = bad->relblk;
	    reqblks = 1;		/* Request is for 1 block	*/
	    break;
	}

	/*
	 * We now have the physical starting block and number of blocks.
	 * Convert these to byte offset, byte length, and save major/minor
	 * number of physical device and return.
	 */
	lv_uiop.uio_offset	= BLK2BYTE( preqstrt );
	lv_uiop.uio_resid	= BLK2BYTE( reqblks );
	lv_iov.iov_len		= BLK2BYTE( reqblks );
	lv_pdev			= part->pvol->dev;

	return( 0 );
}

/*
 *  NAME:         hd_nodumpvg
 * 
 *  FUNCTION:     Remove volgrp from list of volgrp's that would be dumped
 *  
 *  PARAMETERS:   vg - a volgrp
 *
 *  RETURN VALUE: none
 */
void
hd_nodumpvg( struct volgrp *vg )
{
    struct vglist *v;
    
    BUGLPR( debuglvl, BUGNTA, ("hd_nodumpvg: vg = 0x%x\n", vg))

    if (vglistp)
	for (v = vglistp ; v < vglistp + nvglist; v++)
	    if (v->vg == vg) {
		v->vg = NULL;
		v->major_num = 0;
		break;
	    }
}

#define INCRVGLIST	16		/* increase number vglist entries */

/*
 * NAME: hd_lvmcdtf() - LVM component dump table function
 *
 * Called by dmp_do at dump time.
 * Return a pointer to a component dump table (cdt).
 */
static struct cdt_head *
hd_lvmcdtf(void)
{
    lvmcdt.cdt_entry[1].d_len = vglistp ? nvglist * sizeof(struct vglist): 0;
    lvmcdt.cdt_entry[1].d_ptr = (char *) vglistp;
    return ( (struct cdt_head *) &lvmcdt );
}

static void
hd_dump_init(void)
{
	BUGLPR(debuglvl, BUGNTA, ("enter hd_dump_init\n"))

	dmp_add(hd_lvmcdtf);
}

/*
 *  NAME:        hd_dumpvglist
 * 
 *  FUNCTION:     Add a volgrp to list of volgrp's that would be dumped
 *  
 *  PARAMETERS:   vg - a volgrp
 *
 *  RETURN VALUE: none
 */
void
hd_dumpvglist( struct volgrp *vg )
{
    int			oldnvglist = nvglist;
    struct vglist	*v, *oldvglistp;
    
    BUGLPR( debuglvl, BUGNTA, ("hd_dumpvglist: vg = 0x%x\n", vg))

    oldvglistp = vglistp;
    if (hd_vgs_opn >= nvglist) {
	if (nvglist == 0)
	    hd_dump_init();
	nvglist += INCRVGLIST;
	/* !!! word aligned ? !!! */
	vglistp = (struct vglist *)
	    xmalloc((uint)(nvglist * sizeof(struct vglist)), HD_ALIGN,
		    pinned_heap);
	if (vglistp == NULL) {
	    vglistp = oldvglistp;
	    nvglist = oldnvglist;
	} else if (oldnvglist) {
	    bcopy( (char *) oldvglistp, (char *) vglistp,
		  oldnvglist * sizeof(struct vglist));
	    xmfree(oldvglistp, pinned_heap);
            /* make sure rest of our block is zero */
            bzero( (char *) vglistp + oldnvglist * sizeof(struct vglist),
                INCRVGLIST * sizeof(struct vglist) );
	} else {
	    /* first xmalloc: zero the memory */
	    bzero( (char *) vglistp, nvglist * sizeof(struct vglist) );
        }
    }
    if (nvglist > hd_vgs_opn) {
	for (v = vglistp ; ; v++)
	    if (v->vg == NULL) {
		v->vg = vg;
		v->major_num = vg->major_num;
		break;
	    }
    }
}

