static char sccsid[] = "@(#)01  1.12.1.3  src/bos/kernext/disk/sd/sdstrategy.c, sysxdisk, bos411, 9428A410j 3/16/94 10:19:10";
/*
 * COMPONENT_NAME: (SYSXDISK) Serial Dasd Subsystem Device Driver
 *
 * FUNCTIONS: sd_strategy(), sd_insert_q(), sd_dequeue()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1993
 * All Rights Reserved 
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/sd.h>
#include <sys/errno.h>
#include <sys/lvdd.h>
#include <sys/ddtrace.h>
#include <sys/trchkid.h> 

/*
 * NAME:  sd_strategy.c
 *
 * FUNCTION: Strategy Entry Point for Serial DASD Subsytem Device Driver
 *
 * EXECUTION ENVIRONMENT: This routine can be called from either the process
 *	or interrupt environments, and can not page fault.
 *
 * (NOTES:) Possible operations : This routine takes a buf structure, or chain
 *	of buf structures, validates them, and then places them in the
 *	appropriate device elevators.
 *
 * (RECOVERY OPERATION:)  None.
 *
 * (DATA STRUCTURES:) 	buf		- data transfer control block structure
 *			sd_dasd_info	- DASD info structure
 *			sd_adap_info	- Adapter info structure
 *
 * RETURNS: 	0         - successful completion
 */


int	sd_strategy(
struct buf *bp)
{

	dev_t	devno;
	uint	opri,
		last_rba,
		errcode = 0;
	char	zeroread = FALSE;
	struct sd_dasd_info *dp=NULL;
	struct sd_adap_info *ap = NULL;
	struct sd_adap_info *last_ap = NULL;
	struct buf *nextbp;



	if (bp != NULL) {
		/* 
		 * if bp not null, then pull of the devno for trace purposes
		 */
		devno = bp->b_dev;
		DDHKWD5(HKWD_DD_SERDASDD, DD_ENTRY_STRATEGY, 0, devno, bp,
			bp->b_flags, bp->b_blkno, bp->b_bcount);

	} else
	    return (0);
		
	while (bp != NULL) {
	   /*
	    * while we have a buf to process
	    */
	   /*
 	    * initialize error and flags to good completion
	    */
	   bp->b_error = 0;
	   bp->b_flags &= ~B_ERROR;
	   devno = bp->b_dev;	/* save the devno */
	   /*
	    * get pointer to this dasd. ALL calls to strategy 
	    * will be for dasd only.
	    */
	   dp = sd_hash(devno); 
#ifdef DEBUG
#ifdef SD_GOOD_LOCK_PATH
	sd_trc_disable(dp->ap,strategy, entry,(char)0, (uint)devno, (uint)dp ,
		       (uint)bp, (uint)bp->av_forw,(uint)0);
	sd_dptrc_disable(dp,strategy, entry,(char)0, (uint)bp);
#endif
#endif
	   if ((dp == NULL) || (!dp->opened) || (dp->diag_mode))  
		/*
		 * if does not exist, or not opened, or in diagnostic mode
		 */
		errcode = ENXIO;
	   else {
		if (((int) bp->b_bcount & (SD_BPS-1)) || 
		    ((int) bp->b_bcount > dp->max_transfer))
		   /*
		    * if not multiple of blocksize, or larger than our maximum
		    * transfer.  NOTICE: We shouldn't ever receive a transfer
		    * through the block interface greater than our maximum
		    * transfer, the raw transfers get broken up before strategy
		    * call.
		    */
		   errcode = EINVAL;
		else {
		   /*
		    * save last relative block address on disk
		    */
		   last_rba = dp->disk_capacity.lba; 	
		   if (bp->b_blkno > last_rba) {
			/*
			 * if this requested blkno is beyond end of disk,
			 */
			if (bp->b_blkno == (last_rba+1)) {
			   /*
			    * if this requested blkno is the blk immediately 
			    * beyond the end of Media
			    */
			   if (bp->b_flags & B_READ)
				/*
				 * if read request
				 */
				zeroread=TRUE;	/* just a read of length 0. */
			   else
				/*
				 * else if write, then this is an error.
				 */
				errcode = ENXIO;	
			} else 
			   /*
			    * else I/O starting beyond EOM is error.
			    */
			   errcode = ENXIO;
		   } else {
			/*
			 * else we passed EOM tests
			 */
			if (dp->status & SD_LOCKED)
			   /*
			    * if we have locked this disk from taking requests
			    */
			   errcode = ENXIO;
			else {
			   if ((bp->b_flags & B_READ) && 
				(bp->b_options & (HWRELOC | UNSAFEREL))) 
				/*
				 * if read with relocation, set error
				 */
				errcode = EINVAL; 
			   else {
				if (bp->b_options & (HWRELOC | UNSAFEREL))
				   if ((bp->b_bcount > SD_BPS) ||
				       (dp->dds.safe_relocate ==
				       DK_NO_RELOCATION) ||
				       ((bp->b_options & HWRELOC) &&
			 	       (dp->dds.safe_relocate ==
				       DK_UNSAFE_RELOCATION)))
						/*
						 * if relocation but count
						 * greater than blocksize or
						 * no relocation for this disk,
						 * or relocation but unsafe,
						 * then set error.
						 */
						 errcode = EINVAL;

			   }
			}
		   }
		}
	   }
	   nextbp = bp->av_forw;	/* save next buf pointer  */
	   if (errcode) {
		/*
		 * if we have determined this buf to be invalid
		 * then set error and iodone	
		 */
		bp->b_flags |= B_ERROR;
		bp->b_error = errcode;
		bp->b_resid = bp->b_bcount;
		iodone(bp);
		bp = (struct buf *)nextbp;
		errcode = 0;
	   } else {
		if ((bp->b_bcount == 0) || zeroread) { 
		   /*
		    * if no transfer involved, set to no error and iodone
		    */
		   bp->b_resid = bp->b_bcount;
#ifdef DEBUG
#ifdef SD_GOOD_LOCK_PATH
  	   	   sd_trc_disable(dp->ap,sdiodone, trc,(char)1, (uint)devno, (uint)bp , (uint)bp->b_flags, (uint)bp->b_error,  (uint)bp->b_resid);
		   sd_dptrc_disable(dp,sdiodone, trc,(char)0, (uint)bp);
#endif
#endif
		   iodone(bp);
		} else {

		   if (bp->b_flags & B_READ) { 
			/*
			 * if read then update local statistics and 
			 * iostat statistics
			 */
			if ((dp->dds.byte_count += bp->b_bcount) > 
			   dp->dds.segment_size) {
			   dp->dds.byte_count %= dp->dds.segment_size;
			   dp->dds.segment_cnt++;
			}
			dp->dkstat.dk_rblks += bp->b_bcount / SD_BPS;
		   } else {
			/*
			 * else if write, just update iostat statistics
			 */
			dp->dkstat.dk_wblks += bp->b_bcount / SD_BPS;
		   }


		   opri = disable_lock(dp->ap->dds.intr_priority,
				       &(dp->ap->spin_lock));

		   /*
		    * get pointer to adapter that this dasd is on
		    */
		   ap = dp->ap;

		   if ((last_ap != ap) && (last_ap != NULL)) {
			   sd_start(last_ap);
		   }

		   /*
		    * insert this request on the proper "floor" of the elevator
		    */
		   sd_insert_q(dp,bp);	


		   if ( dp->start_chain != TRUE) { 
			/*
			 * if this dasd is not already in the 
			 * dasd-to-be-started chain, then add this dasd
			 * to the start chain
			 */
			dp->nextdp = ap->dphead;
			ap->dphead = dp;
			dp->start_chain = TRUE;
#ifdef DEBUG
#ifdef SD_GOOD_PATH
			sd_trc(dp->ap,indasdchain, trc,(char)0, (uint)dp, (uint)dp->nextdp,(uint)ap->dphead, (uint)ap->nextdp, (uint)ap->starting_dp);
			sd_dptrc(dp,indasdchain, trc,(char)0, (uint)dp->nextdp);
#endif
#endif
		   }

		   unlock_enable(opri,&(dp->ap->spin_lock));
		}
		bp = nextbp;	/* set current buf pointer to next buf */
		last_ap = ap;

	   }
   	}


	/*
	 * Start last adapter (if any) processed above
	 */
	if ( ap != NULL )
	    sd_start_disable(ap);




#ifdef DEBUG
#ifdef SD_GOOD_LOCK_PATH
	if (dp != NULL) {
		sd_trc_disable(dp->ap,strategy, exit,(char)0, (uint)devno, 
			       (uint)bp ,(uint)0, (uint)0,(uint)0);
		sd_dptrc_disable(dp,strategy, exit,(char)0, (uint)bp);
	}
#endif
#endif
	DDHKWD1(HKWD_DD_SERDASDD, DD_EXIT_STRATEGY, 0, devno);
	return(0);
}


/*
 * NAME: sd_insert_q
 *
 * FUNCTION: Inserts a buf structure into the appropriate DASD elevator based
 *	on target cylinder and block number.
 *
 * EXECUTION ENVIRONMENT: This routine is called by sd_strategy, and can be on
 *	the process or interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) None. 
 *
 * (DATA STRUCTURES:) 	sd_dasd_info	- DASD info structure
 *			buf		- Buf structure
 *
 * RETURNS: 	Void.
 */

void sd_insert_q(
struct sd_dasd_info *dp,
struct buf *bp) 
{
	int	curr_cyl,
		target_cyl;
	char	done = FALSE;
	struct buf *curr,
		   *prev,
		   *vcurr,
		   *vprev;


        curr = dp->low_cyl;     /* set current pointer to low_cyl pointer */ 
        prev = NULL;            /* set previous pointer to NULL */

        if ( curr == NULL) {
		/*
		 * we started with an empty list
		 */
		dp->low_cyl = bp;	/* set low ptr to this one */
		dp->currbuf = bp;  /* set current ptr to this one */
		bp->av_forw = NULL;	/* clear his forward ptr */
		bp->av_back = NULL;	/* clear his back ptr */
		bp->b_work = (uint)NULL; /* set work (down) ptr to NULL*/
		return;
	}

	/*
	 * compute our approximation of what cylinder this request 
	 * targets
	 */
	target_cyl = (int)(bp->b_blkno / dp->cyl_capacity.lba);
#ifdef DEBUG
#ifdef SD_GOOD_PATH
	sd_trc(dp->ap,insertq, entry,(char)0, (uint)dp, (uint)bp ,(uint)target_cyl, (uint)curr,(uint)0);
	sd_dptrc(dp,insertq, entry,(char)0, (uint)bp);
#endif
#endif
	do {
		/*
		 * while not at end of cylinder list and not done
		 */
		/*
		 * compute what cylinder the one we're looking at is on
		 */
		curr_cyl = (int)(curr->b_blkno / dp->cyl_capacity.lba);
                if (curr_cyl < target_cyl) {
			/*
			 * if this one is less than the one we're trying
			 * to insert set prev to this one, and walk the
			 * curr pointer to the next one
			 */
			prev = curr;
			curr = curr->av_forw;
                } else {
			/*
			 * else it should either go before this one or on
			 * this cylinder
			 */
			if (curr_cyl == target_cyl) {
				/*
				 * it should go on this cylinder so insert into 
				 * vertical list
				 */
				vcurr = curr;
				vprev = NULL;
				while ((vcurr != NULL) && (!done)) {
					/*
					 * while not at end of vertical list
					 * and not done
					 */
					if (vcurr->b_blkno <= bp->b_blkno) {
					   /*
					    * if this one is less then the
					    * one we want to insert, walk down
					    */
					   vprev = vcurr;
					   vcurr = (struct buf *)vcurr->b_work;
					} else {
					   /*
					    * else it should go before this 
					    * one
					    */
					   if (vprev == NULL) {
						/*
						 * make this the first one in
						 * vertical list
						 */
						if (vcurr == dp->currbuf)
							/*
							 * if this was our
							 * current buf, set
							 * currbuf to one we
							 * are inserting
							 */
							dp->currbuf = bp;
						bp->b_work = (uint)vcurr;
						bp->av_back = vcurr->av_back;
						bp->av_forw = vcurr->av_forw;
						vcurr->av_back = NULL;
						vcurr->av_forw = NULL;
						if (bp->av_back != NULL)
						   bp->av_back->av_forw = bp;
						else
						   dp->low_cyl = bp;
						if (bp->av_forw != NULL)
						   bp->av_forw->av_back = bp;
					   } else {
						/*
						 * else insert before current
						 */
						vprev->b_work = (uint)bp;
						bp->b_work = (uint)vcurr;
						bp->av_forw = NULL;
						bp->av_back = NULL;
					   }
					   done = TRUE;
				 	}
				}
				if (vcurr == NULL) {
					/*
					 * put on bottom of vertical list
					 */
					vprev->b_work = (uint)bp;
					bp->b_work = (uint)NULL; 
					bp->av_forw = NULL;
					bp->av_back = NULL;
					done = TRUE;
				}
			} else {
				/*
				 * else it should go before the current one
				 */
                        	if ( prev == NULL) {
					dp->low_cyl = bp;
					bp->av_back = NULL;
				} else {
					prev->av_forw = bp;
					bp->av_back = prev;
				}
				bp->av_forw = curr;
				curr->av_back = bp;
				bp->b_work = (uint)NULL;
				done = TRUE;
                	}
        	}
        } while ((curr != NULL) && (!done)) ;        

        if ( curr == NULL) {
		/*
		 * we walked to the end of the list
		 */
		prev->av_forw = bp; /* set prev's forward ptr to this */
		bp->av_back = prev; /* set this one's back ptr to prev*/
                bp->av_forw = curr;  /* set this ones forward to curr (NULL) */
		bp->b_work = (uint)NULL; /* set work (down) ptr to NULL*/
        }

	ASSERT(dp->currbuf != NULL); /* after inserting, should have currbuf*/
#ifdef DEBUG
#ifdef SD_GOOD_PATH
	sd_trc(dp->ap,insertq, exit,(char)0, (uint)bp, (uint)bp->av_forw , (uint)bp->av_back, (uint)bp->b_work,(uint)0);
	sd_dptrc(dp,insertq, exit,(char)0, (uint)bp);
#endif
#endif
	return;
}

/*
 * NAME: sd_dequeue
 *
 * FUNCTION: Removes the "current" buf structure from the DASD elevator
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level, and
 *	can not page fault.
 *
 * (RECOVERY OPERATION:)  None.
 *
 * (DATA STRUCTURES:) 	sd_dasd_info	- DASD info structure
 *			buf		- Buf structure
 *
 * RETURNS: 	Void.
 */

void sd_dequeue(
struct sd_dasd_info *dp)
{

	struct buf *oldbp,
		   *tmpbp;

	/*
	 * Relink list to remove the current buf  
	 *  
	 *   ------------    ------------     ------------
	 *   |          |    |          |     |          |
	 *   |   av_forw|--->|   av_forw|---->|   av_forw|--->
	 *   |          |    |          |     |          |
	 *<--|av_back   |<---|av_back   |<----|av_back   |
	 *   |  b_work  |    |  b_work  |     |  b_work  |
	 *   ------------    ------------     ------------
	 *        |               |                 |
	 *        |              \/                 |
	 *        |          ------------           |
	 *       \/          |          |          \/
	 *                   |   av_forw|---->
	 *                   |          |
	 *    NULL     <-----|av_back   |        NULL
	 *                   |  b_work  |
	 *                   ------------
	 *                        |
	 *                        |
	 *                       \/
	 */		

	ASSERT(dp->currbuf);
	oldbp = dp->currbuf;        	/* save old current bp */
	tmpbp = (struct buf *)oldbp->b_work;     /* point down */
#ifdef DEBUG
#ifdef SD_GOOD_PATH
	sd_trc(dp->ap,dequeue, entry,(char)0, (uint)dp, (uint)oldbp , (uint)tmpbp, (uint)dp->direction, (uint)dp->low_cyl);
	sd_dptrc(dp,dequeue, entry,(char)0, (uint)dp->currbuf);
#endif
#endif
	if (tmpbp != NULL) {
		/*
		 * if b_work not null, then more on this
		 * cylinder, so shuffle this up into
		 * av_forw/av_back list
		 */
		if (oldbp->av_back != NULL)
			/*
			 * if this has one before it, set his prev's next
			 * to the one we are shuffling up
			 */
			oldbp->av_back->av_forw = tmpbp;
		else
			/*
			 * else, this was the first, so set low cylinder to
			 * the one we are shuffling up
			 */
			dp->low_cyl = tmpbp;
		if (oldbp->av_forw != NULL)
			/*
			 * if this one has a next, set his next's back to
			 * this one we are shuffling up
			 */
			oldbp->av_forw->av_back = tmpbp;
		tmpbp->av_forw = oldbp->av_forw;
		tmpbp->av_back = oldbp->av_back;
		dp->currbuf = tmpbp; 	/* make this current buf */
	} else {
		/*
		 * else no more on this cylinder, so
		 * just bypass the old one
		 */
		if (oldbp->av_back != NULL)
			/*
			 * if this has one before it, link his prev's to his
			 * next
			 */ 
			oldbp->av_back->av_forw = oldbp->av_forw;
		else
			/*
			 * else this must have been the first, so set low
			 * cylinder to his next
			 */
			dp->low_cyl = oldbp->av_forw;
		if (oldbp->av_forw != NULL)
			/*
			 * if this one has a next, set his next's back to
			 * this one's back
			 */
			oldbp->av_forw->av_back = oldbp->av_back;
		if (dp->direction == SD_ELEVATOR_UP) {
			/*
			 * if the elevator is going up
			 */
			if (dp->currbuf->av_forw == NULL) {
			    /*
			     * if at the top 
			     * What we do depends upon whether this is a
			     * controller which supports seek ordered
			     * queueing or not.
			     * OLD CONTROLLER: - 2 way elevator
			     * change directions and 
			     * set current to the next one down
			     * NEW CONTROLLER: - 1 way elevator
			     * set current to lowest buffer.
			     */
			    if (!dp->cp->dds.fence_enabled) 
			    {
				dp->direction = ~dp->direction;
	               		dp->currbuf = dp->currbuf->av_back;
			    } else {
				dp->currbuf = dp->low_cyl;
			    }
			} else {
			    /*
			     * else set current to next one up
			     */
			    dp->currbuf = dp->currbuf->av_forw;
			}
		} else {
			/*
			 * else the elevator is going down
			 */
			if (dp->currbuf->av_back == NULL) {
				/*
				 * if at the bottom, change direction and 
				 * set current to next one up
				 */
	                       	dp->direction = ~dp->direction;
				dp->currbuf = dp->currbuf->av_forw;
			} else {
				/*
				 * else set current to next one down
				 */
				dp->currbuf = dp->currbuf->av_back;
			}
		}
	}
#ifdef DEBUG
#ifdef SD_GOOD_PATH
	sd_trc(dp->ap,dequeue, exit,(char)0, (uint)dp, (uint)dp->currbuf , (uint)dp->direction, (uint)dp->low_cyl,(uint)0);
	sd_dptrc(dp,dequeue, exit,(char)0, (uint)dp->currbuf);
#endif
#endif
}
