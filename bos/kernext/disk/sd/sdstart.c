static char sccsid[] = "@(#)00  1.15.1.3  src/bos/kernext/disk/sd/sdstart.c, sysxdisk, bos411, 9428A410j 3/16/94 10:16:43";
/*
 * COMPONENT_NAME: (SYSXDISK) Serial Dasd Subsystem Device Driver
 *
 * FUNCTIONS: sd_start(), sd_coalesce(), sd_relocate(), sd_start_cmd()
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
#include <sys/lvdd.h>
#include <sys/errno.h>
#include <sys/ddtrace.h>
#include <sys/trchkid.h>


/*
 * NAME: sd_start_disable
 *
 * FUNCTION: Starts the processing of the next command(s) for an adapter.
 *
 * EXECUTION ENVIRONMENT: This routine takes the MP spin lock and then calls
 *			  sd_start.
 *
 *
 * (DATA STRUCTURES:)   sd_adap_info     - Adapter info structure
 *                      sd_dasd_info     - DASD info structure
 *                      sd_ctrl_info     - Controller info structure
 *                      buf              - Buf structure
 *
 * RETURNS:     Results of sd_start_cmd, if error
 *              0         - successful completion
 *              EIO       - PIO error
 */

void     sd_start_disable(
struct sd_adap_info *ap )
{

        uint    opri;


   /* 
    * disable interrupts and on MP machines acquire a spin lock.
    */
   opri = disable_lock(ap->dds.intr_priority,&(ap->spin_lock));

   sd_start(ap);

   unlock_enable(opri,&(ap->spin_lock)); 
}
/*
 * NAME: sd_start
 *
 * FUNCTION: Starts the processing of the next command(s) for an adapter.
 *
 * EXECUTION ENVIRONMENT: This routine is called by sd_strategy, sd_intr, and
 *      other processes to begin execution of a command. It is called on both
 *      the process and interrupt levels, and can not page fault.
 *
 * (NOTES:) Possible operations : This routine follows the DASD start chain
 *      for an adapter, attempting to start pending requests.  This routine
 *      follows the following hierarchy when determining what commands to
 *      start:
 *              Adapter Error Queue/Stack
 *                Adapter Ioctl Queue
 *                  Controller Error Queue/Stack
 *                    Controller Ioctl Queue
 *                      DASD Error Queue/Stack
 *                        DASD concurrent mode command queue
 *                          DASD Ioctl Queue
 *                            DASD Elevator
 *      Notice: The starting of any Error or IOCTL command results in the
 *              exit of the start routine, since it is possible for start_cmd
 *              to NOT start a command and invoke a recursive call to start
 *
 * (RECOVERY OPERATION:) If a PIO error is encountered while attempting to
 *      start execution of a command, the proper errno is returned, and
 *      recovery is up to the caller.
 *
 * (DATA STRUCTURES:)   sd_adap_info     - Adapter info structure
 *                      sd_dasd_info     - DASD info structure
 *                      sd_ctrl_info     - Controller info structure
 *                      buf              - Buf structure
 *
 * RETURNS:     Results of sd_start_cmd, if error
 *              0         - successful completion
 *              EIO       - PIO error
 */

void     sd_start(
struct sd_adap_info *ap )
{

        uint    base;
        int     rc = 0,
                prc = 0,
		startcmd_rc =0;
        char    wrapped = FALSE,
                rm_dp_from_list = FALSE,
                cmdsbuilt = FALSE,
                force_again = FALSE,
                again;
        struct  sd_ctrl_info    *cp;
        struct  sd_dasd_info    *last_dp,
                                *dp;
        struct sd_cmd *cmd;
        struct  buf     *bp;
	struct  conc_cmd *ccptr;
	


#ifdef DEBUG
#ifdef SD_GOOD_PATH
   sd_trc(ap,start, entry,(char)0, (uint)ap, (uint)ap->status ,(uint)ap->errhead, (uint)ap->ioctlhead,(uint)0);
#endif
#endif

  do {
   again = FALSE;
   if (ap->status & (SD_SUSPEND | SD_RESET_PENDING | SD_DELAY)) {
        /*
         * Don't look any further if adapter suspended, pending a reset, or
	 * delayed
         */
        continue; /* to bottom of do loop */
   }
   if ((ap->errhead != NULL) && (!(ap->status & SD_DOWNLOAD_PENDING))) {
        /*
         * In order to issue a command off the adapter error queue, there
         * first must not be an adapter download pending (download goes off
         * ioctl queue)
         */
	(void)sd_start_cmd(ap->errhead);
        continue; /* to bottom of do loop */
   }
   if (ap->ioctlhead != NULL) {
        /*
         * If we get here, we've met all criteria for issueing a command
         * off the adapter ioctl queue. if this adapters ioctl q is not empty
         * then start the head of the ioctl queue
         */
	(void)sd_start_cmd(ap->ioctlhead);
        continue; /* to bottom of do loop */
   }
   if (ap->ctrl_errhead != NULL) {
	/*
	 * If there is a controller command for some controller on this
	 * adapter
	 */
	cmd = ap->ctrl_errhead;
	if ((!(cmd->cp->status & (SD_DOWNLOAD_PENDING | SD_DELAY))) && 
		(!(ap->status & SD_QUIESCE_PENDING))) {
		/*
                 * In order to issue a command off the controller error
                 * queue, he must not have a download pending or be delayed, 
		 * and his adapter must not have a quiesce pending
                 */
		startcmd_rc = sd_start_cmd(cmd);
                if (startcmd_rc != SD_NOT_AGAIN)
			/*
			 * if start command wants us to make another pass
			 */
                        again = TRUE;
		continue;
	}
   }
   if (ap->ctrl_ioctlhead != NULL) {
	/*
	 * If there is a controller ioctl command for some controller
	 * on this adapter     
	 */
	cmd = ap->ctrl_ioctlhead;
	if ((!(ap->status & (SD_QUIESCE_PENDING | SD_DOWNLOAD_PENDING))) &&
	       (!(cmd->cp->status & SD_DELAY))) {
                /*
                 * To issue a controller command off the ioctl queue, the
                 * adapter must not be quiesceing or downloading, and this 
		 * controller must not be delayed.
                 */
		startcmd_rc = sd_start_cmd(cmd);
                if (startcmd_rc != SD_NOT_AGAIN)
			/*
			 * if start command wants us to make another pass
			 */
                        again = TRUE;
	}
	continue;
   }
   /*
    * If we get here, then we will try to process a DASD. There are no
    * adapter or controller error/ioctl commands in the way.
    */
   if ((!ap->adap_resources) ||
        (ap->status & (SD_QUIESCE_PENDING | SD_DOWNLOAD_PENDING))) {
        /*
         * If adapter has quiesce pending, download pending, or no resources,
         * no need to go any further
         */
        continue; /* to bottom of do loop */
   }

   if (ap->nextdp == NULL)
        /*
         * if next dasd we are to process is null, it must
         * be our first time thru for this adapter, so
         * set pointer to head of dasd list
         */
        dp = ap->dphead;
   else
        /*
         * else set dp to next dasd to process
         */
        dp = ap->nextdp;

   ap->starting_dp = dp; 

   /*
    * save devno of the dasd we started with and set wrapped flag to false
    */
   wrapped = FALSE;
   while ((!wrapped) && ap->adap_resources && (dp != NULL)) {
        /*
         * as long as we still have adapter resources, dasd in the list, and
         * we haven't wrapped around , keep building commands.
         * Notice the only time we should get here and not have
         * adapter resources is possible from a strategy call,
         * when from interrupt, we just set flag to true
         */
        cp = dp->cp; /* set cp to this dasd's controller */

        if ((!(dp->status & (SD_SUSPEND | SD_DELAY))) && 
	    (!(cp->status & (SD_RESET_PENDING | SD_QUIESCE_PENDING | 
	    SD_DOWNLOAD_PENDING | SD_DELAY))) &&
	    (!(ap->status & (SD_QUIESCE_PENDING | SD_DOWNLOAD_PENDING)))) {
                /*
                 * If this DASD is not suspended or delayed, and his ctrl is
                 * not resetting, quiescing, downloading, or delayed
                 */
	        if ((dp->errhead == NULL) && (dp->conc_cmd_list != NULL) &&
		    (dp->concurrent.status == SD_FREE))
		{
		    /*
		     * If the error queue is empty and there is a 
		     * concurrent mode command waiting and our 
		     * concurrent mode command jacket is free,
		     * then we can queue the concurrent mode command on
		     * the dasd'd error queue. It will be started by
		     * the error queue processing below.
		     */
		    ccptr = sd_d_q_conc_cmd (dp);
		    sd_build_conc_cmd (dp,ccptr); 
		}
		
                if (dp->errhead != NULL) {
                        /*
                         * if this dasd's error queue is not
                         * empty, then start head of his
                         * error queue
                         */
			startcmd_rc = sd_start_cmd(dp->errhead);
	                if (startcmd_rc != SD_NOT_AGAIN) {
				/*
				 * if start command wants us to 
				 * make another pass
				 */
                                again = TRUE;
				if (startcmd_rc == SD_FORCE_ANOTHER_PASS)
					/*
					 * If startcmd REALLY wants us to
					 * make another pass, set force flag
					 * Notice: This may be when start cmd
					 * didn't start a command (out of 
					 * retries, etc.), but wants us to 
					 * make another pass even if we've
					 * wrapped.
					 */
					force_again = TRUE;
			} /* if startcmd_rc != SD_NOT_AGAIN */
		    } else if (dp->conc_cmd_list == NULL) 
		    if (!(dp->status &
		 (SD_LOCKED | SD_VERIFY_PENDING | SD_REQ_SNS_PENDING))) {
                        /*
                         * if DASD is not locked, awaiting a verify, or
                         * awaiting a request sense
                         */
                        if (dp->ioctlhead != NULL) {
			   /*
                            * or if this dasd's ioctl queue is not
                            * empty, then start head of his
                            * ioctl queue
                            */
			   startcmd_rc = sd_start_cmd(dp->ioctlhead);
	                   if (startcmd_rc != SD_NOT_AGAIN) {
				/*
				 * if start command wants us to 
				 * make another pass
				 */
                                again = TRUE;
				if (startcmd_rc == SD_FORCE_ANOTHER_PASS)
					/*
					 * If startcmd REALLY wants us to
					 * make another pass, set force flag
					 * Notice: This may be when start cmd
					 * didn't start a command (out of 
					 * retries, etc.), but wants us to 
					 * make another pass even if we've
					 * wrapped.
					 */
					force_again = TRUE;
			   } /* if startcmd_rc != SD_NOT_AGAIN */
			} else {
                           /*
                            * Finally....NORMAL PATH...
                            */
                           cmdsbuilt = FALSE;      /* init # cmds to 0 */
                           while ((dp->currbuf != NULL) &&
                                   (dp->cmds_out < dp->dds.queue_depth)) {
                                /*
                                 * while this disks elevator is not empty and
                                 * we haven't built queue_depth cmds for
                                 * this disk yet
                                 */
#ifdef DEBUG
#ifdef SD_GOOD_PATH
                                sd_dptrc(dp,start, trc,(char)4, (uint)dp->cmds_out);
#endif
#endif
                                rc = sd_coalesce(dp, &rm_dp_from_list, 
					&cmdsbuilt,dp->currbuf);
                                if (rc) break;
                           } /* while currbuf and cmds out < q depth */

                           if (!ap->adap_resources && !cmdsbuilt)
                                /*
                                 * if we ran out of resources, and didn't
                                 * build any commands for this dasd, then break
                                 * so we'll look at this dasd first next time
                                 */
                                break;

                           if (cmdsbuilt) {
                                /*
                                 * if we built at least 1 cmd write the
                                 * LAST TAG REGISTER to kick off cmds
                                 * for this device
                                 */
                                base = (uint)BUSIO_ATT(ap->dds.bus_id,
                                        ap->dds.base_addr);
                                prc = BUS_PUTCX((char *)(base+SD_LTAG),
                                        (char)ap->last_tag);
                                if (prc) {
                                        /*
                                         * if PIO error, try to recover
                                         */
                                        prc = sd_pio_recov(ap, prc, (uchar)PUTC,
                                                (void *)(base + SD_LTAG),
                                                (long)ap->last_tag,
                                                (uchar)NON_VOLATILE);
                                        if (prc) {
                                                /*
                                                 * if recovery failed, just
                                                 * return, the adapter was
                                                 * reset in pio_recov, and
                                                 * this command will be placed
                                                 * on the errq for retries
                                                 */

                                                BUSIO_DET(base);
#ifdef DEBUG
#ifdef SD_ERROR_PATH
                                                sd_trc(ap,start, 
						       exit,(char)6, 
						       (uint)ap,(uint)prc,
						       (uint)ap->last_tag,
						       (uint)0,(uint)0);
#endif
#endif
                                                return;
                                        }
                                }
                                BUSIO_DET(base);
#ifdef  DEBUG
#ifdef SD_GOOD_PATH
                                sd_trc(ap,lasttag, trc,(char)0, (uint)0, 
				       (uint)ap, (uint)ap->last_tag ,
				       (uint)0,(uint)0);
#endif
#endif
                                DDHKWD1(HKWD_DD_SERDASDD, DD_EXIT_BSTART,
                                        0, dp->devno);
                                /*
                                 * Update IOSTAT statistics
                                 */
                                dp->dkstat.dk_xfers++;
                                dp->dkstat.dk_status |= IOST_DK_BUSY;
                           } /* if cmdsbuilt */
			}/* else Normal Path */
                } /* if DASD not locked, verify pending, req sns pend */
        } /* if DASD not suspended, and controller clear */

#ifdef DEBUG
#ifdef SD_GOOD_PATH
   sd_trc(ap,start, trc,(char)0, (uint)dp, (uint)dp->nextdp,
	  (uint)ap->dphead, (uint)ap->nextdp , (uint)ap->starting_dp);
#endif
#endif
        /*
         * Walk to next DASD in chain
         */
        last_dp = dp;   /* update last disk processed */
        if (dp->nextdp == NULL)
                /*
                 * if we are at the end of the disk list, set the current
                 * disk and the next disk to be processed to the head of the
                 * list
                 */
                dp = ap->nextdp = ap->dphead;
        else
                /*
                 * set the current disk and the next disk to be processed
                 * to the next disk in the list
                 */
                dp = ap->nextdp = dp->nextdp;

        if(((dp == NULL) || (dp == ap->starting_dp)) && (!force_again)) {
                /*
		 * If we have exhausted the start chain or have wrapped,
		 * AND we aren't supposed to force another pass,
                 * Say that we wrapped, and make sure we don't make
		 * another pass.
		 * NOTICE: We added the force_again to avoid our heart beat
		 * stopping.  We have to continually feed the devices....if 
 		 * they ever go idle (nothing in progress) and we have items
		 * in our queue, we are dead!!
                 */
                wrapped = TRUE;
		again = FALSE;
	}

        if (rm_dp_from_list) {
                /*
                 * if no more request queued for the last disk, then
                 * remove it from the list
                 */
                sd_del_chain(last_dp, (char) FALSE);
                rm_dp_from_list = FALSE;
        }
	if (ap->starting_dp == NULL)
		/*
		 * if starting dp went to NULL due to emptying the list
		 * try to set it to the head of the list.  NOTICE: This is
		 * to catch the cases when preemption out of start changes
		 * the starting mark.
		 */
		ap->starting_dp = ap->dphead;
#ifdef DEBUG
#ifdef SD_GOOD_PATH
   sd_trc(ap,start, trc,(char)1, (uint)dp, (uint)dp->nextdp,
	  (uint)ap->dphead, (uint)ap->nextdp , (uint)ap->starting_dp);
#endif
#endif
	force_again = FALSE;	/* clear force flag, we've already seen it */

   } /* while resources, dasd, and not wrapped */
 } while (again);
#ifdef DEBUG
#ifdef SD_GOOD_PATH
   sd_trc(ap,start, exit,(char)7, (uint)ap->status, 
	  (uint)ap->adap_resources , (uint)wrapped, (uint)0,(uint)0);
#endif
#endif

   return;
}



/*
 * NAME: sd_coalesce
 *
 * FUNCTION: Processes Normal Path requests that may possibly be coalesced
 *
 * EXECUTION ENVIRONMENT: This routine is called by sd_start from both the
 *      process and interrupt levels, and can not page fault.
 *
 * (NOTES:) Possible operations : Attempts to coalesce as many requests as
 *      possible into one command.  Notice this path also detects and handles
 *      block relocation requests.
 *
 * (RECOVERY OPERATION:) If out of resources to handle the command, then
 *      the proper return code is returned and the request will remain on
 *      the queue to be handled later.
 *
 * (DATA STRUCTURES:)   sd_dasd_info     - DASD info structure
 *                      sd_adap_info     - Adapter info structure
 *                      sd_cmd           - Command structure
 *                      buf              - Buf structure
 *
 * RETURNS:
 *           0         - successful completion
 *          -1         - out of adapter resources.
 *                     - or reached queue depth for this dasd
 *                     - or want to move on to next dasd in start chain
 *           Results of sd_start_cmd, if error
 */

int sd_coalesce(
struct  sd_dasd_info *dp,
char    *rm_dp_from_list,
char    *cmdsbuilt,
struct	buf *curr)
{
        struct sd_adap_info     *ap;
        struct sd_cmd   *cmd;
        struct buf	*next;
        uint            flags = 0xFFFFFFFF,    /* flags accumulator */
                        dma_addr,
                        temp_addr;
        char            coalesce = FALSE;
        int             rc;

        ap = dp->ap;            /* get dasd's adapter pointer */
#ifdef DEBUG
#ifdef SD_GOOD_PATH
        sd_trc(ap,coales, entry,(char)0, (uint)dp, (uint)dp->currbuf , 
	       (uint)dp->currbuf->b_options,  (uint)0,(uint)0);
#endif
#endif
        cmd = sd_cmd_alloc(ap);  /* get a command structure for this command */
        if (cmd == NULL) {
                /*
                 * if couldn't get a command structure then set flag
                 */
                ap->adap_resources = FALSE;
                return(-1);
        }
        if (curr->b_options & (HWRELOC | UNSAFEREL)) {
                /*
                 * if this buf involves relocation
                 */
                if (*cmdsbuilt) {
                        /*
                         * if we have built some commands(s) then
                         * return and get this the next time around
                         * We've already allocated mailboxes for previous
                         * commands, which we have to start before starting
                         * the reassign or our mailbox chain will be hosed
                         */
			sd_free_cmd(cmd);
                        return(-1);
		}
                /*
                 * build commands to perform the relocate
                 */
                if (sd_relocate(dp,curr,cmd))
                	/*
			 * if we successfully built and queued reassign-write,
	                 * start the first command
	                 */
	                (void)sd_start_cmd(dp->errhead);
		else
			sd_free_cmd(cmd);
                return(-1);
        }
        /*
         * set command type and dp pointer here so that MB_ALLOC increments
         * DASD commands outstanding
         */
        cmd->type = SD_DASD_CMD;        /* set command type to disk command */
        cmd->dp = dp;   /* save pointer to this disk with command */
        cmd->cp = dp->cp;  /* save controller pointer for cmd */
        if (sd_MB_alloc(cmd)) {
                /*
                 * if could not get a mailbox for this command
                 * set flag for no more resources
                 */
                sd_free_cmd(cmd);
                ap->adap_resources = FALSE;
                return(-1);
        }
        /*
         * set next pointer to next element on same cylinder
         */
        next = (struct buf *)curr->b_work;
        cmd->b_length = curr->b_bcount; /* set xfer length to curr's bcount */
        cmd->b_addr = curr->b_un.b_addr; /* store starting buf address */
        cmd->rba = curr->b_blkno;       /* set target RBA to curr's block # */
        if ((cmd->rba + cmd->b_length/SD_BPS - 1) > dp->disk_capacity.lba) {
                /*
                 * if this request will take us past the last block on
                 * this disk set resid to the difference compute new
                 * transfer length.
                 */
                curr->b_resid = curr->b_bcount - (SD_BPS *
                        (dp->disk_capacity.lba - curr->b_blkno + 1));
                cmd->b_length -= curr->b_resid;
        } else
                /*
                 * else set resid to 0, sending all of it
                 */
                curr->b_resid = 0;
        /*
         * accumulate flags...actually this clears any bits not set
         * in curr->b_flags, therefore if any request has split read
         * or write disabled, all the requests will disable it
         */
        flags &= curr->b_flags;
        if ( sd_TCW_alloc(cmd) == FALSE ) {
                /*
                 * if couldn't get TCW's set flag
                 */
#ifdef DEBUG
#ifdef SD_ERROR_PATH
   		sd_trc(ap,notcws, trc,(char)0, (uint)ap, (uint)ap->status ,(uint)cmd, (uint)cmd->status,(uint)cmd->b_length);
#endif
#endif
                sd_free_MB(cmd, (char)SD_NOT_USED);        /* free mailbox */
                sd_free_cmd(cmd);       /* free command */
                ap->adap_resources = FALSE;
                return(-1);
        }
        /*
         * save new last tag value to write to LAST TAG register
         */
        ap->last_tag = cmd->mb->tag;
        *cmdsbuilt = TRUE;   /* set flag that cmd built */
        cmd->bp = curr;     /* save buf pointer with this command */
	DDHKWD3(HKWD_DD_SERDASDD, DD_COALESCE, 0, dp->devno, curr, cmd);

        /*
         * remove currbuf from this dasd's queue
         */
        sd_dequeue(dp);
	
	/*
	 * Let activity timer know we started something
	 */
	dp->buf_started = TRUE;
	
        if (dp->currbuf == NULL) {
                /*
                 * if the new current buf is empty
                 */
                *rm_dp_from_list = TRUE;
        }

        coalesce = FALSE;
        if ((next != NULL) && 
	   (!((int)(curr->b_un.b_addr + curr->b_bcount) & (SD_TCWSIZE-1))))
                /*
                 * set coalesce flag to TRUE, since we possibly have
                 * adjacent requests on the same cylinder.  The curr
                 * buf is considered active even though we haven't
                 * issued it, so set the current buf to the next one
                 * and set curr's forward to null
		 * NOTICE: In order to coalesce, the first buffer doesn't
		 * need to start on a page boundary, but must end on one.
		 * All subsequent coalesced requests must begin and end on
		 * page boundaries.
                 */
                coalesce = TRUE;


        rc = 0;
        while ( coalesce ) {
                /*
                 * while we should try to coalesce
                 */
                if ((next != NULL) &&
                   (!(next->b_options & (HWRELOC | UNSAFEREL))) &&
                   ((curr->b_blkno+(curr->b_bcount/SD_BPS)) == next->b_blkno)&&
                   ((curr->b_flags&B_READ)==(next->b_flags&B_READ)) &&
                   ((curr->b_options&WRITEV)==(next->b_options&WRITEV)) &&
		   (!((int)next->b_un.b_addr & (SD_TCWSIZE-1))) &&
		   (!(next->b_bcount & (SD_TCWSIZE-1))) &&
                   ((cmd->b_length + next->b_bcount) <= dp->dds.max_coalesce)){
                        /*
			 * Conditions to be able to coalesce:
                         * 1) there is a next one to look at
                         * 2) the next one does not involve relocation
                         * 3) the next request is sequential with current one
                         * 4) the next one is the same direction transfer 
			 *    as this one (i.e. curr is read, next is read) 
                         * 5) the next one handles write verify same as curr
                         * 6) the next one begins on a page boundary
			 * 7) the next one will end on a page boundary
			 * 8) the next one won't exceed coalesce maximum
                         */
#ifdef DEBUG
#ifdef SD_GOOD_PATH
                        sd_trc(ap,coales, trc,(char)0, (uint)curr, (uint)next, (uint)cmd,  (uint)cmd->b_length,(uint)0);
                        sd_dptrc(dp,coales, trc,(char)0, (uint)next);
#endif
#endif

                        /*
                         * compute new transfer size
                         */
                        cmd->b_length += next->b_bcount;

                        if ( sd_TCW_realloc(cmd) == FALSE) {
#ifdef DEBUG
#ifdef SD_ERROR_PATH
   				sd_trc(ap,notcws, trc,(char)0, (uint)ap, (uint)ap->status ,(uint)cmd, (uint)cmd->status,(uint)cmd->b_length);
#endif
#endif
                                /*
                                 * if could not reallocate TCW's set flag
                                 */
                                ap->adap_resources = FALSE;
                                /*
                                 * go back to old transfer size
                                 */
                                cmd->b_length -= next->b_bcount;
                                rc = -1;        /* set return code */
                                break;  /* out of while (coalesce) */
                        }
                        DDHKWD3(HKWD_DD_SERDASDD, DD_COALESCE, 0, dp->devno,
                                next, cmd);

                        curr = next; /* set current pointer to next element */
                        /*
                         * set next pointer to the new next
                         */
                        next = (struct buf *)curr->b_work;

                        if ((cmd->rba + (cmd->b_length/SD_BPS)-1 ) >
                                dp->disk_capacity.lba) {
                                /*
                                 * if the addition of this next request will
                                 * put us past the end of this disk, then
                                 * compute the residual, and compute new
                                 * transfer length of what we can send.
                                 */
                                curr->b_resid = curr->b_bcount - (SD_BPS *
                                        (dp->disk_capacity.lba - curr->b_blkno
                                        + 1));
                                cmd->b_length -= curr->b_resid;
                        } else
                                /*
                                 * else we can send it all
                                 */
                                curr->b_resid = 0;
                        /*
                         * accumulate flags...actually this clears
                         * any bits not set in curr->b_flags, therefore
                         * if any request has split read or write disabled,
                         * all the requests will disable it
                         */
                        flags &= curr->b_flags;

                        /*
                         * Remove curr from list, bump pointers
                         */
                        sd_dequeue(dp);
	
			/*
			 * Let activity	timer know we started something
			 */	
			dp->buf_started = TRUE;

                        if (dp->currbuf == NULL) {
                                /*
                                 * if the new current buf is empty
                                 */
                                *rm_dp_from_list = TRUE;
                        }
                        if (next == NULL)
                                /*
                                 * no more on this cylinder, so stop trying to
                                 * coalesce
                                 */
                                coalesce = FALSE;
                } else {
                        /*
                         * else, we didn't pass the tests, so set flag to
                         * stop trying to coalesce, and terminate coalesced
                         * list
                         */
                        coalesce = FALSE;
                }

        } /* while (coalesce) */

        curr->b_work = (uint)NULL;

        /*
         */
        curr = cmd->bp;                 /* reset curr to starting buf */
        /*
         * calculate the starting dma address, which  equals the
         * beginning offset into tcw space + this buffers
         * offset from a page boundary
         */
        dma_addr = (uint)(SD_DMA_ADDR(ap->dds.tcw_start_addr, cmd->tcws_start)+
                ((uint)cmd->b_addr & (SD_TCWSIZE - 1)));
        temp_addr = dma_addr;           /* let temp_addr be the dma_addr */
        while ( curr != NULL) {
                /*
                 * scan thru buf structs, and map the TCWs for each
                 */
                d_master((int)ap->dma_channel, SD_DMA_TYPE |
                        ((curr->b_flags & B_READ) ? DMA_READ : 0) |
                        ((curr->b_flags & B_NOHIDE) ? DMA_WRITE_ONLY : 0) ,
                        curr->b_un.b_addr, (curr->b_bcount - curr->b_resid), 
			 &curr->b_xmemd,(char *)temp_addr);
                /*
                 * increment the dma_addr
                 */
                temp_addr += curr->b_bcount;
                /*
                 * point to next buf struct in chain
                 */
                curr = (struct buf *)curr->b_work;
        } /* endwhile */


        /*
         *  Finish building of MB here
         */
        sd_build_cmd(cmd,flags,dma_addr);

        /*
         * flush MB cache lines
         */
        d_cflush(ap->dma_channel, (char *)cmd->mb, (size_t)SD_MB_SIZE,
                cmd->mb_dma_addr);

#ifdef DEBUG
#ifdef SD_GOOD_PATH
        sd_trc(ap,coales, exit,(char)0, (uint)dp, (uint)dp->currbuf , (uint)cmd,  (uint)0,(uint)0);
#endif
#endif
	DDHKWD5(HKWD_DD_SERDASDD, DD_ENTRY_BSTART, 0, dp->devno, cmd->bp, 
		cmd->bp->b_flags, cmd->bp->b_blkno, cmd->b_length);
        return(rc);
}


/*
 * NAME: sd_relocate
 *
 * FUNCTION: Builds and queues up a SCSI Reassign command, followed by a
 *      SCSI Write command, to handle block relocation requests
 *
 * EXECUTION ENVIRONMENT: This routine is called by sd_coalesce on the process
 *      or interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:)  None.
 *
 * (DATA STRUCTURES:)   sd_dasd_info     - DASD info structure
 *                      sd_cmd           - Command Structure
 *                      sd_mbox          - Mailbox structure
 *                      sc_cmd           - SCSI command structure
 *
 * RETURNS:     TRUE	- if successful in building relocate and write
 *		FALSE   - if unsuccessful, i.e. reassign currently active
 */

int sd_relocate(
struct sd_dasd_info *dp,
struct buf *curr,
struct sd_cmd *cmd)
{

        struct sd_cmd *rcmd;
        struct sd_mbox *m;
        struct sc_cmd *scsi;

#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(dp->ap,relocate, entry,(char)0, (uint)dp, (uint)curr, (uint)curr->b_blkno,  (uint)0,(uint)0);
        sd_dptrc(dp,relocate, entry,(char)0,(uint)curr);
#endif
#endif

        if (dp->reassign.status != SD_FREE) {
                ASSERT(FALSE);
                if (dp->reassign.status & SD_ACTIVE) {
                        /*
                         * This should not happen, but if it does, just
                         * return. The original request will remain on
                         * the queue, and the activity timer will clean
                         * up if necessary
                         */
#ifdef DEBUG
#ifdef SD_ERROR_PATH
                        sd_trc(dp->ap,relocate, exit,(char)0, (uint)dp, (uint)curr, (uint)curr->b_blkno,  (uint)0,(uint)0);
                        sd_dptrc(dp,relocate, exit,(char)0, (uint)curr);
#endif
#endif
                        return(FALSE);
                } else {
                        /*
                         * make sure command is dequeued
                         */
                        sd_d_q_cmd(&dp->reassign);
                        sd_free_cmd(&dp->reassign);
                }
        }

	/*
	 * Set Reassign Pending Flag
	 */
	dp->status |= SD_REASSIGN_PENDING;

        /*
         * Build a defect list for the reassignment
         */
        dp->def_list.header = 0x00000004;
        dp->def_list.lba = curr->b_blkno;

        rcmd = &(dp->reassign);
        rcmd->nextcmd = NULL;            /* clear next pointer */
        rcmd->dp = dp;                   /* set dasd pointer */
        rcmd->ap = dp->ap;               /* set dasd pointer */
        rcmd->cp = dp->cp;               /* set controller pointer */
        rcmd->bp = NULL;                 /* clear buf pointer */
        rcmd->xmem_buf = &(dp->xmem_buf);/* set cross mem descrip */
        rcmd->xmem_buf->aspace_id = XMEM_GLOBAL;
        rcmd->b_length = 0x08;           /* set length to header and descrip */
        rcmd->rba = 0;                   /* clear rba */
        rcmd->b_addr = (caddr_t)&(dp->def_list);  /*   set transfer address */
        rcmd->tcw_words = 0;             /* clear number of tcw's alloc*/
        rcmd->tcws_start = 0;            /* clear starting tcw */
        rcmd->sta_index = (signed char)-1;/* init small xfer area index*/
        rcmd->retry_count = 0;           /* clear retry count */
        rcmd->retries = 0;               /* clear number of retries */
        rcmd->status_validity = 0;       /* clear status validity */
        rcmd->erp = 0;                   /* clear error recov proc */
        rcmd->uec = 0;                   /* clear Unit Error Code type */
        rcmd->dev_address = 0;           /* clear device address */
        rcmd->dma_flags = 0;             /* set to write */
        rcmd->type = SD_DASD_CMD;        /* set command type */
        rcmd->cmd_info = SD_REASSIGN;    /* set command info */

        /*
         * build mailbox copy
         */
        m = &(rcmd->mbox_copy);
        m->op_code = SD_SEND_SCSI_OP;              /* Send SCSI Command */
        m->mb6.qc_scsiext = SD_Q_UNORDERED;       /* Unordered queue control  */
        /*
         * set device address generated by this dasd's lun and his
         * controllers target address
         */
        m->mb7.dev_address =
                SD_LUNTAR(dp->cp->dds.target_id,dp->dds.lun_id,SD_LUNDEV);
        /*
         * Initialize SCSI cmd for operation
         */
        scsi = &(m->mb8.fields.scsi_cmd);
        scsi->scsi_op_code = SCSI_REASSIGN_BLOCK;
        scsi->lun = 0x00;
        scsi->scsi_bytes[0] = 0x00;
        scsi->scsi_bytes[1] = 0x00;
        scsi->scsi_bytes[2] = 0x00;
        scsi->scsi_bytes[3] = 0x00;

        rcmd->timeout = 120; /* set timeout to 2 minutes*/
        sd_q_cmd(rcmd,(char)SD_QUEUE);

        /*
         * Save reference to reassigned write
         */
        dp->reassign_write = cmd;
        cmd->nextcmd = NULL;            /* clear next pointer */
        cmd->dp = dp;                   /* set dasd pointer */
        cmd->ap = dp->ap;               /* set dasd pointer */
        cmd->cp = dp->cp;               /* set controller pointer */
        cmd->sta_index = (signed char)-1;/* init small xfer area index*/
        cmd->b_length = curr->b_bcount;
        cmd->b_addr = curr->b_un.b_addr; /* store starting buf address */
        cmd->rba = curr->b_blkno; /* set target RBA to curr's block # */
        if ((cmd->rba + cmd->b_length/SD_BPS - 1) > dp->disk_capacity.lba) {
                /*
                 * if this request will take us past the last block on
                 * this disk set resid to the difference compute new
                 * transfer length.
                 */
                curr->b_resid = curr->b_bcount - (SD_BPS *
                        (dp->disk_capacity.lba - curr->b_blkno + 1));
                cmd->b_length -= curr->b_resid;
        } else
                /*
                 * else set resid to 0, sending all of it
                 */
                curr->b_resid = 0;
        cmd->dp = dp;   /* save pointer to this disk with command */
        cmd->cp = dp->cp;   /* save ctrl pointer to this disk with command */
        cmd->ap = dp->ap;               /* set dasd pointer */
        cmd->type = SD_DASD_CMD;        /* set command type to disk command */
	/*
	 * Treat this write like a retried command off of the error queue
	 * Notice, retry_count set to -1, to allow for this first "retry"
	 * since a timer will be started, this tell us to stop the timer
	 * upon completion
	 */
        cmd->status |= SD_RETRY;   
        cmd->retry_count = (signed char)-1;
        cmd->bp = curr;            /* save buf pointer with this command */
        cmd->dma_flags = 0;

        /*
         * remove currbuf from this dasd's queue
         */
        sd_dequeue(dp);
	
	/*
	 * Let activity timer know we started something
	 */
	dp->buf_started = TRUE;

	/*
	 * Clean up his down link so we don't mistake him as coalesced
	 */
        curr->b_work = (uint)NULL;

	/*
	 * build remainder of write command
	 */
        m = &(cmd->mbox_copy);
        m->op_code = SD_SEND_SCSI_OP;
	/*
	 * set queue control to Unordered,
	 * and scsi extension if applicable, such as enabling split read/write
	 * if the flags indicate this transfer can have split enabled.
	 */
        m->mb6.qc_scsiext = ((curr->b_flags & B_SPLIT) ? 
	   (SD_SPLIT_WRITE | SD_SPLIT_READ) : SD_NO_EXT ) | SD_Q_UNORDERED;

        /*
         * set the device address
         */
        m->mb7.dev_address = SD_LUNTAR(cmd->dp->cp->dds.target_id,
                                          cmd->dp->dds.lun_id, SD_LUNDEV);
        scsi = &(m->mb8.fields.scsi_cmd);
        if (curr->b_options & WRITEV) 
		/*
		 * Initialize SCSI cmd for Write with verify
		 */
		scsi->scsi_op_code = SCSI_WRITE_AND_VERIFY;
	else
		/*
		 * Build SCSI extended write
		 */
		scsi->scsi_op_code = SCSI_WRITE_EXTENDED;

	scsi->lun = 0x00;
	scsi->scsi_bytes[0] = ((cmd->rba >> 24) & 0xff);
	scsi->scsi_bytes[1] = ((cmd->rba >> 16) & 0xff);
	scsi->scsi_bytes[2] = ((cmd->rba >> 8) & 0xff);
	scsi->scsi_bytes[3] = (cmd->rba & 0xff);
	scsi->scsi_bytes[4] = 0x00;
	scsi->scsi_bytes[5] = 0x00;
	scsi->scsi_bytes[6] = 0x01;
	scsi->scsi_bytes[7] = 0x00;

        cmd->timeout = 30; /* set timeout to 30 seconds */

        sd_q_cmd(cmd,(char)SD_QUEUE);
#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(dp->ap,relocate, exit,(char)1, (uint)dp, (uint)curr, (uint)curr->b_blkno,  (uint)0,(uint)0);
        sd_dptrc(dp,relocate, exit,(char)1, (uint)curr);
#endif
#endif
	return(TRUE);
}


/*
 * NAME:  sd_start_cmd
 *
 * FUNCTION: Attempts to start a pre-built command.
 *
 * EXECUTION ENVIRONMENT: This command is called by sd_start on both the
 *      process and interrupt levels, and can not page fault.
 *
 * (NOTES:) Possible operations : This routine firsts checks to see if the
 *      command to start is a retried command, and if so checks to see if it
 *      has exceeded retries.
 *
 *      This routine could also recursively call start, if it was unable to
 *      start the command (e.g. exceeded retries)
 *
 * (RECOVERY OPERATION:)  If a PIO error occurs, then the proper errno is
 *      returned, and recovery is up to the caller.
 *
 * (DATA STRUCTURES:)   sd_cmd          - Command structure
 *                      sd_adap_info    - Adapter info structure
 *                      sd_watchdog     - Watchdog structure
 *                      buf             - Buf structure
 *
 * RETURNS:
 *           SD_TRY_ANOTHER_PASS   - Notify start to try another pass, 
 *				     if it hasn't wrapped
 *           SD_FORCE_ANOTHER_PASS - Notify start to definitely make another
 *                       	     pass, regardless if it's wrapped.
 *           SD_NOT_AGAIN      	   - Notify start not to make another pass
 */

int sd_start_cmd(
struct  sd_cmd  *cmd)
{

        struct sd_watchdog *w;
        struct sd_adap_info *ap;
        struct sd_cmd *next_cmd;
        struct buf *curr;
        uint    base,
                dma_addr,
                temp_addr;
        uchar  err_flag;
        int     rc,
                prc,
                i;

#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(cmd->ap,startcmd, entry,(char)0, (uint)cmd, (uint)cmd->cmd_info, (uint)cmd->type, (uint)cmd->status,(uint)0);
#endif
#endif
        ASSERT(cmd->status != SD_FREE);
        ap = cmd->ap;
        if ((cmd->status & SD_TIMEDOUT) &&
           ((cmd->cmd_info == SD_IOCTL) || (cmd->cmd_info == SD_QRYDEV))){
                /*
                 * This command already timed out, and it was an ioctl or
                 * query device,dequeue and free, otherwise, attempt retries.
                 */
                sd_d_q_cmd(cmd);
                sd_free_cmd(cmd);
                /*
                 * Didn't start anything, try again
                 * Want start to make another pass
                 */
                return(SD_FORCE_ANOTHER_PASS);
        }

      if (!(cmd->status & SD_DAEMON_PRI)) {
	/*
	 * if this is not a daemon priority command (quiesce, or download),
	 * then perform appropriate checks 
	 */
        if (cmd->type == SD_ADAP_CMD) {
                /*
                 * Adapter Command
                 */
                if ((ap->status & SD_QUIESCE_PENDING) &&
                    (cmd->cmd_info != SD_RST_QSC)) {
                        /*
                         * if this adapter has a quiesce pending, but
                         * this is not the quiesce
                         */
		    if (ap->quiesce.status & SD_QUEUED) {
			/*
			 * The quiesce has been queued, so set cmd to it and
			 * proceed with the quiesce
			 */
			cmd = &ap->quiesce;
		    } else 
			/*
			 * else assume it is already active, or hasn't
			 * been queued by halt_adap yet
                         * Don't Want start to make another pass
			 */
                        return(SD_NOT_AGAIN);
	     }
        } else if (cmd->type == SD_CTRL_CMD) {
                /*
                 * Controller Command
                 */
		if (cmd->cp->status & SD_REQ_SNS_PENDING) {
			/*
			 * if this controller has a request sense pending
			 */
			if (cmd->cmd_info != SD_REQSNS) {
                        /*
                         * BUT, this is not it,
                         * we need to check if the request is queued or active
                         */
				if (cmd->cp->reqsns.status & SD_QUEUED) {
					/*
					 * Request Sense is queued, so 
					 * set cmd to the request
					 * sense and proceed
					 */
					cmd = &cmd->cp->reqsns;
				} else
					 /*
					  * if this controller has a 
					  * request sense pending, but
					  * this is not it, just return.
					  * if this is not it, 
					  * then it must be in progress
					  * Don't Want start to make 
					  * another pass
					  */
					return(SD_NOT_AGAIN);

			} /* if not reqsns command */

		}
		else {
			/*
			 * else No request Sense pending, so see if 
			 * reset/quiesce may be pending
			 */
			if ((cmd->cp->status & 
			     (SD_RESET_PENDING|SD_QUIESCE_PENDING)) &&
			    (cmd->cmd_info != SD_RST_QSC)) {
				/*
				 * else if this controller has a reset/quiesce 
				 * pending, but this is not the
				 *  reset/quiesce 
				 * See if it is queued or active
				 */
				if (cmd->cp->status & SD_RESET_PENDING) {
					if (cmd->cp->reset.status & 
					    SD_QUEUED) {
						/*
						 * Reset is queued, so set 
						 * cmd to the Reset
						 * and proceed
						 */
						cmd = &cmd->cp->reset;
					} else
						/*
						 * else his reset must be 
						 * ACTIVE, so just return
						 * Don't want start to
						 * make another pass
						 */
						return(SD_NOT_AGAIN);
				}
				if (cmd->cp->status & SD_QUIESCE_PENDING) {
					if (cmd->cp->quiesce.status & 
					    SD_QUEUED) {
						/*
						 * Quiesce is queued, so set 
						 * cmd to
						 * the Quiesce and proceed
						 */
						cmd = &cmd->cp->quiesce;
					} else
						/*
						 * else his Quiesce must be 
						 * ACTIVE, so jsut return
						 * Don't want start to
						 * make another pass
						 */
						return(SD_NOT_AGAIN);
				}
				
			}
	
		}


        } else {
                /*
                 * DASD Command
                 */
                if ((cmd->dp->status & SD_LOCKED) &&
                    (cmd->cmd_info != SD_RELEASE)) {
                        /*
                         * if this DASD is locked, then only allow a
                         * release, fail all others
                         */
                        sd_d_q_cmd(cmd);
                        sd_fail_cmd(cmd, (char) TRUE);
                        /*
                         * Didn't start anything, try again
                         * Want start to make another pass
                         */
                        return(SD_FORCE_ANOTHER_PASS);
                }

                if (cmd->dp->status & SD_REQ_SNS_PENDING) {
                  /*
                   * if this dasd has a request sense pending
                   */
                  if ((cmd->cmd_info != SD_REQSNS) &&
                      (cmd->cmd_info != SD_QRYDEV)) {
                        /*
                         * BUT, this is not it, and its not a query device,
                         * we need to check if the request is queued or active
                         */
                        if (cmd->dp->reqsns.status & SD_QUEUED) {
				/*
				 * Request Sense is queued, so set cmd to
				 * the request sense and proceed
				 */
				cmd = &cmd->dp->reqsns;
			} else
                        	/*
				 * else his request sense must be ACTIVE, so
				 * tell start to look at another DASD
	                         */
	                        return(SD_TRY_ANOTHER_PASS);
                   } /* if not reqsns command or query device */
                } else
                  /*
                   * else No request Sense pending, so see if reset/quiesce
                   * may be pending
                   */
                  if ((cmd->dp->status & 
		      (SD_RESET_PENDING|SD_QUIESCE_PENDING)) &&
                      (cmd->cmd_info != SD_RST_QSC) &&
                      (cmd->cmd_info != SD_QRYDEV)) {
                        /*
                         * else if this DASD has a reset/quiesce pending, but
                         * this is not the reset/quiesce or a query device,
			 * See if it is queued or active
                         */
		      if (cmd->dp->status & SD_RESET_PENDING) {
                        if (cmd->dp->reset.status & SD_QUEUED) {
				/*
				 * Reset is queued, so set cmd to
				 * the Reset and proceed
				 */
				cmd = &cmd->dp->reset;
			} else
                        	/*
				 * else his reset must be ACTIVE, so
				 * tell start to look at another DASD
	                         */
	                        return(SD_TRY_ANOTHER_PASS);
		      }
		      if (cmd->dp->status & SD_QUIESCE_PENDING) {
                        if (cmd->dp->quiesce.status & SD_QUEUED) {
				/*
				 * Quiesce is queued, so set cmd to
				 * the Quiesce and proceed
				 */
				cmd = &cmd->dp->quiesce;
			} else
                        	/*
				 * else his Quiesce must be ACTIVE, so
				 * tell start to look at another DASD
	                         */
	                        return(SD_TRY_ANOTHER_PASS);
		      }
		}
                if ((cmd->dp->status & SD_VERIFY_PENDING) &&
                    (!(cmd->cmd_info & SD_OK_DURING_VERIFY)))
                        /*
                         * if verify pending for this DASD, but this command
                         * is not allowed through during a verify, then
                         * return and leave this command on the queue
                         * Must be caught between having the next command
                         * on the queue
                         * Want start to make another pass
                         */
                        return(SD_TRY_ANOTHER_PASS);
                if ((cmd->dp->status & SD_REASSIGN_PENDING) &&
                    (!(cmd->cmd_info & SD_OK_DURING_REASSIGN)))
                        /*
                         * if reassign pending for this DASD, but this command
                         * is not allowed through during a reassign, then
                         * return and leave this command on the queue
                         * Want start to make another pass
                         */
                        return(SD_TRY_ANOTHER_PASS);
        }
      } /* if not daemon priority */

        if (cmd->status & SD_RETRY) {
                /*
                 * if this is a retry
                 */
                if (cmd->retry_count >= cmd->retries) {
                        /*
                         * Notice: all retried commands will come
                         * through this path, and their retries will
                         * have been initialized.  Check here whether
                         * retries are exceeded and fail the command
                         * if necessary
                         */
                        sd_d_q_cmd(cmd);
                        sd_fail_cmd(cmd, (char) FALSE);
                        /*
                         * didn't start anything, try again
                         * Want start to make another pass
                         */
                        return(SD_FORCE_ANOTHER_PASS);
                }
        }

        if (sd_MB_alloc(cmd)) {
                /*
                 * if could not get a mailbox for this command
                 * just return, the command will remain on queue
                 * Don't Want start to make another pass
                 */
                return(SD_NOT_AGAIN);
        }

        if ((cmd->b_length > SD_STA_SIZE) || 
	    (cmd->cmd_info == SD_NORMAL_PATH)) {
                /*
                 * if transfer is larger than our small transfer
                 * areas, or this is a normal path command, or is a
		 * write for reassign command, try to allocate
                 * TCW's for the transfer
                 */
                if (sd_TCW_alloc(cmd) == FALSE) {
                        /*
                         * if could not get TCW's for this command
                         * free the mailbox and the command will remain on
                         * the queue
                         * Don't Want start to make another pass
                         */
#ifdef DEBUG
#ifdef SD_ERROR_PATH
   			sd_trc(ap,notcws, trc,(char)0, (uint)ap, (uint)ap->status ,(uint)cmd, (uint)cmd->status,(uint)cmd->b_length);
#endif
#endif
                        sd_free_MB(cmd, (char)SD_NOT_USED);
                        return(SD_NOT_AGAIN);
                }
        } else {
                /*
                 * else transfer will fit in small transfer area
                 */
                if (sd_STA_alloc(cmd) == FALSE) {
                        /*
                         * if could not get STA for this command
                         * free the mailbox and the command will remain on
                         * the queue
                         * Don't Want start to make another pass
                         */
                        sd_free_MB(cmd, (char)SD_NOT_USED);
                        return(SD_NOT_AGAIN);
                }
        }

        /*
         * move tag and nextmb pointer from real mailbox into copy
         */
        cmd->mbox_copy.tag = cmd->mb->tag;
        cmd->mbox_copy.nextmb = cmd->mb->nextmb;
        /*
         * copy the "copy" of the mailbox into the real one
         */
        bcopy((char *) &cmd->mbox_copy, (char *) cmd->mb, SD_MB_SIZE);

        switch (cmd->cmd_info) {
                case SD_NORMAL_PATH:
                        /*
                         * This is a normal path read or write
                         */
                        /*
                         * calculate the starting dma address
                         */
                        dma_addr = (uint)(SD_DMA_ADDR(ap->dds.tcw_start_addr,
                                cmd->tcws_start) + ((uint)cmd->b_addr &
                                (SD_TCWSIZE - 1)));
                        cmd->mb->mb8.fields.dma_addr = dma_addr;
                        cmd->mb->mb8.fields.dma_length = cmd->b_length;
                        /*
                         * this was possibly a coalesced command
                         */
                        curr = cmd->bp; /* reset curr to start buf */
                        temp_addr = dma_addr;
                        while ( curr != NULL) {
                                /*
                                 * scan thru buf structs, and map
                                 * the TCWs for each
                                 */
                                d_master((int)ap->dma_channel,
                                   SD_DMA_TYPE | ((curr->b_flags &
                                   B_READ) ? DMA_READ : 0) |
                                   ((curr->b_flags & B_NOHIDE) ?
                                   DMA_WRITE_ONLY : 0) ,
                                   curr->b_un.b_addr, 
				   (curr->b_bcount - curr->b_resid),
                                   &curr->b_xmemd, (char *)temp_addr);
                                /*
                                 * increment the dma_addr
                                 */
                                temp_addr += curr->b_bcount;
                                /*
                                 * point to next buf struct in chain
                                 */
                                curr = (struct buf *)curr->b_work;
                        } /* endwhile */
                        w = &cmd->dp->cmd_timer;  /* point at dasd cmd timer */
                        w->watch.restart = 30;    /* allow 30 seconds timout */
                        break;
                case SD_IOCTL:
                        /*
                         * This is an ioctl command
                         * make sure validity field is clear
                         */
                        cmd->status_validity = 0;
                        /*
                         * Fall through
                         */
                default:
                        if (cmd->b_length) {
                                /*
                                 * if data transfer involved
                                 */
                                if (cmd->sta_index == (signed char)-1) {
                                        /*
                                         * Wasn't a small transfer, so map
                                         * TCW's
                                         */
                                        dma_addr = (uint)(SD_DMA_ADDR(
                                                ap->dds.tcw_start_addr,
                                                cmd->tcws_start) +
                                                ((uint)cmd->b_addr &
                                                (SD_TCWSIZE - 1)));
                                        /*
                                         * map the TCW's
                                         */
                                        d_master((int)ap->dma_channel,
                                                cmd->dma_flags, cmd->b_addr,
                                                cmd->b_length, cmd->xmem_buf,
                                                (char *)dma_addr);
                                } else {
                                        /*
                                         * Was a small transfer, so calculate
                                         * dma address from STA area, and
                                         * flush STA cache lines
                                         */
                                   dma_addr = (uint)(SD_DMA_ADDR(
                                    ap->dds.tcw_start_addr,
                                    ap->sta_tcw_start)) +
                                    ((uint)(ap->STA[cmd->sta_index].stap)
                                    - ((uint)(ap->STA[0].stap) &
                                    ~(SD_TCWSIZE - 1)));

                                   /*
                                    * the following handles Writes only
                                    */
                                   if (cmd->dma_flags != DMA_READ) {
                                     /*
                                      * this is a write, so copy data
                                      * from caller's buffer to the STA
                                      */
                                     if (cmd->xmem_buf->aspace_id ==
                                        XMEM_GLOBAL) {
                                        /*
                                         * copy data from kernel space
                                         */
                                        for (i = 0; i < cmd->b_length; i++)
                                         *(ap->STA[cmd->sta_index].stap + i) =
                                            *(cmd->b_addr + i);
                                     } else {
                                        /*
                                         * copy data from user space
                                         */
                                        rc = xmemin(cmd->b_addr,
                                           ap->STA[cmd->sta_index].stap,
                                           cmd->b_length, cmd->xmem_buf);
                                        if (rc != XMEM_SUCC) {
                                           /*
                                            * handle bad copy
                                            */
                                           ASSERT(rc == XMEM_SUCC);
                                           sd_free_MB(cmd, (char)SD_NOT_USED);
                                           sd_STA_dealloc(cmd);
                                           sd_TCW_dealloc(cmd);
                                           cmd->retries = 0;
                                           sd_d_q_cmd(cmd);
                                           sd_fail_cmd(cmd, (char) FALSE);
                                          /*
                                           *  Want start to make another pass
                                           */
                                           return(SD_FORCE_ANOTHER_PASS);
                                        }
                                      }
                                   }

                                   d_cflush(ap->dma_channel,
                                    (char *)ap->STA[cmd->sta_index].stap,
                                    (size_t)SD_STA_SIZE, dma_addr);

                                } /* else Small Transfer */
                                cmd->mb->mb8.fields.dma_addr = dma_addr;
				/*
				 * Set up dma address unless this is 
				 * an ioctl in which case this we use the
				 * value from the mailbox copy which was
				 * set up by ioctl.
				 */
				if (cmd->cmd_info != SD_IOCTL) { 
				    cmd->mb->mb8.fields.dma_length = cmd->b_length;
				} 
			}
		    			
                        /*
                         * Now determine which watchdog timer to use
                         */
			switch(cmd->type) {
				case SD_ADAP_CMD:
					/*
					 * if adapter command
					 */
					if (cmd->cmd_info == SD_IOCTL)
						w = &ap->ioctl_timer;
					else
						w = &ap->cmd_timer;
					break;
				case SD_CTRL_CMD:
					/*
					 * if controller command
					 */
					if (cmd->cmd_info == SD_IOCTL)
						w = &cmd->cp->ioctl_timer;
					else
						w = &cmd->cp->cmd_timer;
					break;
				case SD_DASD_CMD:
					/*
					 * if dasd command
					 */
					if (cmd->cmd_info == SD_IOCTL)
						w = &cmd->dp->ioctl_timer;
					else if (cmd->cmd_info == SD_QRYDEV)
						w = &cmd->dp->query_timer;
					else
						w = &cmd->dp->cmd_timer;
					break;
				default:
					/*
					 * catch illegal command types
					 */
					ASSERT(cmd);
			}
			w->watch.restart = cmd->timeout;
			break;
        }

        /*
         * flush MB cache lines
         */
        d_cflush(ap->dma_channel, (char *)cmd->mb, (size_t)SD_MB_SIZE,
                cmd->mb_dma_addr);

        /*
         * Unlink this command from its list
         */
        sd_d_q_cmd(cmd);


        if (cmd->status & SD_RETRY) {
                /*
                 * if this is a retry, increment retry count
                 */
		cmd->retry_count++;
	}

        /*
         * Update the last tag field with this cmd's mb's tag.  Also
         * write the Last tag register to begin execution of this command
         */
        ap->last_tag = cmd->mb->tag;
        base = (uint)BUSIO_ATT( ap->dds.bus_id, ap->dds.base_addr);
        prc = BUS_PUTCX((char *)(base + SD_LTAG), (char)ap->last_tag);
        if (prc) {
                prc = sd_pio_recov(ap, prc, (uchar)PUTC,
                        (void *)(base + SD_LTAG), (long)ap->last_tag,
                        (uchar)NON_VOLATILE);
                if (prc) {
                        /*
                         * if PIO error. NOTICE:
                         * the command has been dequeued, and the adap
                         * reset from pio_recov will place the command back
                         * on the queue
                         */
                        BUSIO_DET(base);
                        /*
                         *  Don't Want start to make another pass
                         */
                        return(SD_NOT_AGAIN);
                }
        }
        BUSIO_DET(base);
        if (cmd->cmd_info == SD_NORMAL_PATH) {
                /*
                 * Update IOSTAT statistics
                 */
                cmd->dp->dkstat.dk_xfers++;
                cmd->dp->dkstat.dk_status |= IOST_DK_BUSY;
        } else if (cmd->cmd_info == SD_START_UNIT)
		/*
		 * if this is a start unit, then increment restart count
		 */
		cmd->dp->restart_count++;
	else if (cmd->cmd_info == SD_RST_QSC)
		/*
		 * if this is a reset quiesce, then increment appropriate 
		 * retry count
		 */
		switch (cmd->type) {
			case SD_ADAP_CMD:
				cmd->ap->reset_count++;
				break;
			case SD_CTRL_CMD:
				cmd->cp->reset_count++;
				break;
			case SD_DASD_CMD:
				cmd->dp->reset_count++;
				break;
		}
#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(ap,lasttag, trc,(char)0, (uint)cmd, (uint)ap, (uint)ap->last_tag, (uint)0,(uint)0);
#endif
#endif

	/*
	 * store cmd pointer with watchdog info, and start timer this command
	 */
	w->pointer = (void *)cmd;
	w_start(&(w->watch));

#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(ap,startcmd, exit,(char)0, (uint)ap->status, (cmd->cp ? (uint)cmd->cp->status : 0), (cmd->dp ? (uint)cmd->dp->status : 0), (uint)0,(uint)0);
#endif
#endif
        /*
         *  Want start to make another pass
         */
        return(SD_TRY_ANOTHER_PASS);

}
