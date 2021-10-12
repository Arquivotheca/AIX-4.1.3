static char sccsid[] = "@(#)91        1.15  src/bos/kernext/disk/sd/sdactivity.c, sysxdisk, bos411, 9428A410j 3/16/94 09:34:06";
/*
 * COMPONENT_NAME: (SYSXDISK) Serial Dasd Subsytem Device Driver
 *
 * FUNCTIONS:  sd_activity(), sd_act_fill(),
 *             sd_query_timer()
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM 
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/sd.h>
#include <sys/errno.h>


/*
 *
 * NAME: sd_activity
 *
 * FUNCTION: Activity timer's principal routine
 *
 *      Check all configured adapters to see if any of their dasd are hung
 *      on a  command. If so then issue a query device mailbox to the
 *      effected dasd.  Ignore devices in diagnostic mode.  Also check that
 *      all adapter daemons  are still running.  If not start a new one.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine must be called in the interrupt environment.  It
 *      can not page fault and is pinned
 *
 * (RECOVERY OPERATION:)  If failure occurs no recovery action takes place
 *
 *
 * (DATA STRUCTURES:)  struct watchdog activity_timer - calls sd_activity
 *                     struct sd_adap_table[]         - search all adapters
 *                     struct sd_dasd_table[]         - search all dasds
 *
 * INPUTS:
 *      watchdog   - timer that will pop when activity of DASD is checked
 *
 * INTERNAL PROCEDURES CALLED:
 *      sd_act_fill
 *      sd_dumpend
 *
 * EXTERNAL PROCEDURES CALLED:
 *      getprio
 *      disable_lock
 *      unlock_enable
 *      w_start
 *
 *
 * (RECOVERY OPERATION:) If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 * RETURNS:      Void
 */

void sd_activity(
struct watchdog *w)
{
        int                 i;             /* general counter           */
        struct sd_adap_info *ap;           /* adapter pointer           */
        struct sd_ctrl_info *cp;           /* controller pointer        */
        struct sd_dasd_info *dp;           /* dasd pointer              */
        struct sd_cmd *cmd;          	   /* cmd pointer               */
        int                 old_ilevel;    /* old interrupt level       */
	uint		    tmpword;
	uchar		    tag, word;

	for (i = 0; i < SD_ADAP_TBL_SIZE;i++) {
          /*
	   * for each adapter in table
	   */
	  ap = sd_adap_table[i];
	  while (ap != NULL) {
	     /*
	      * if adapter present
	      */
	     old_ilevel = disable_lock(ap->dds.intr_priority,
				       &(ap->spin_lock));
             if (!ap->diag_mode) {
		/*
		 * if not in diagnostic mode 
                 * find all outstanding commands for this
                 * adapter
                 */
		word = 0;
		/*
		 * Create a bit map of only ACTIVE commands (i.e. ORing the
		 * the alloced list with the free list), remembering to clear
		 * tag 0 of this first word, since we never use 0.
		 */
		tmpword = (~(ap->mb_alloc_list[0] | ap->mb_free_list[0])) &
			    0x7FFFFFFF;
		do  {
		   /*
		    * find first used mailbox
		    */
		   tag = sd_getfree(tmpword);
		   if (tag < SD_BITS_PER_WORD) {
			/*
			 * found mail box in use within this word
			 */
			tag += (SD_BITS_PER_WORD * word);
			/*
			 * clear this one (in our tmpword), so we won't see
			 * it next time
			 */
			SD_GETTAG(tmpword,tag);
			/*
			 * get cmd for this tag
			 */
        	   	cmd = ap->cmd_map[tag];
		  	if ((cmd != NULL) && (cmd->cmd_info == SD_NORMAL_PATH)){
                	  /*
	                   * if there is an active command at this entry in the
       		           * map, and it is a normal path command, Notice: we
			   * have to make sure the cmd is not NULL, since we
			   * will check our "next_tag" which isn't ACTIVE yet,
			   * but marked as allocated.
                 	   */
			  dp = cmd->dp;
			  cp = dp->cp;
			  if ((!cp->diag_mode) && (!dp->diag_mode)) {
			    /*
			     * If DASD and his controller not in diag mode
                             * check if this dasd is still alive
                             */
			     if ((!dp->interrupted) && (!dp->buf_started)) {
				/*
				 * If this DASD hasn't interrupted, and we
				 * have not started a new buf
				 */
                                /*
                                 * Mark so we won't hit this one again
                                 */
                                dp->interrupted = TRUE;
				if (dp->query_count > SD_MAX_QUERIES) {
				   /*
				    * if we have tried too many query
				    * devs without success, meaning that
				    * the query devs keep completing with
				    * success, but we think the device is
				    * hung, Reset the controller
				    */
        			   sd_reset_quiesce((struct sd_adap_info *)
					dp->cp, (uchar)SD_RESET_OP,
					(uchar)SD_CTRL_CMD);
				} else { /* if too many queries */
                                   /*
                                    * build query device sd_cmd for this tarlun
                                    */
                                   if (dp->qrydev.status == SD_FREE) {
                                        cmd = &(dp->qrydev);
                                        cmd->cmd_info = SD_QRYDEV;
                                        cmd->type = SD_DASD_CMD;
                                        cmd->dp = dp;
                                        cmd->ap = dp->ap;
                                        cmd->cp = dp->cp;
                                        cmd->mbox_copy.op_code=SD_QUERY_DEV_OP;
                                        cmd->mbox_copy.mb7.dev_address = 
                                           SD_LUNTAR( dp->cp->dds.target_id, 
						dp->dds.lun_id, SD_LUNDEV);
#ifdef DEBUG
#ifdef SD_ERROR_PATH
					sd_trc(dp->ap,qrydev, trc,(char)0, 
					       (uint)dp, (uint)cmd ,(uint)0, 
					       (uint)0,(uint)0);
#endif
#endif
					/*
					 * set timeout to 4 seconds
					 */
					cmd->timeout = 4;
                                        /*
                                         * place on err queue
                                         */
                                        sd_q_cmd(cmd,(char)SD_STACK);
					/*
					 * Force Start to try and issue query
					 * for this dasd, even if out of 
					 * some resource
					 */
					ap->adap_resources = TRUE;
					ap->nextdp = dp;
                                        sd_start(ap); 
                                   } /* !qrydev_inuse */
                                   
				} /* else build query dev */
                             }  /* if !interrupted  && prev_tag == curr_tag*/
                	   } /* else: if mode == diagnostic */
        	         } /* if normal path */
                    } else {
			/*
			 * No more active commands in this word, check
			 * next
			 */
			if (++word < SD_NUM_MB_WORDS)
				/*
				 * more words to check
				 */
				tmpword = ~(ap->mb_alloc_list[word] | 
					    ap->mb_free_list[word]);
		    }
		} while (word < SD_NUM_MB_WORDS);
	    } /* if adapter not in diagnostic mode */
	    unlock_enable(old_ilevel,&(ap->spin_lock));
	    ap = ap->hash_next;

	} /* while ap != NULL */

      } /* for all adapters */

        /*
         * reset interrupted flag and record present status
         * of each dasd
         */
        for ( i = 0; i < SD_DASD_TBL_SIZE;i++) {
                dp = sd_dasd_table[i];
                while (dp != NULL) {
                        old_ilevel = disable_lock(dp->ap->dds.intr_priority,
						  &(dp->ap->spin_lock));
                        dp->interrupted = FALSE;
                        dp->buf_started = FALSE;
                        dp->prev_buff = dp->currbuf;
                        unlock_enable(old_ilevel,&(dp->ap->spin_lock));
                        dp = dp->hash_next;
                }
        }

        /*
         *  restart sd_activity after a watch.restart time has
         * elapsed
         */

        w_start(w);

        return;
}


/*
 *
 * NAME: sd_query_timer
 *
 * FUNCTION: Activity timer's error handler for query device.
 *
 *        If query device mailbox message issued by sd_activity() timed
 *        out thendo a full reset on the corresponding adapter.  If have
 *        difficulty reading adapter posid then mark all cmds of the
 *        effected dasd's adapter as done with error.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine must be called in the interrupt environment.  It
 *      can not page fault and is pinned
 *
 * (RECOVERY OPERATION:)  If failure occurs then all outstanding
 *                        commands to this adapter are failed.
 *
 *
 * (DATA STRUCTURES:)  struct sd_watchdog timer  -  Calls sd_query timer
 *                     struct trb         reset_timer -  delay for reset
 *
 * INPUTS:
 *      t   - trb timer that will pop when adapter reset should be complete
 *
 * CALLED BY:
 *      trb timer
 *
 * INTERNAL PROCEDURES CALLED:
 *
 *
 * EXTERNAL PROCEDURES CALLED:
 *      BUSIO_PUTC                  BUSIO_GETC
 *      IOCC_ATT                    tstart
 *      IOCC_DET
 *
 * RETURNS:      Void
 *
 */

void sd_query_timer(
struct watchdog *w)
{
        struct sd_watchdog         *timer;  /* query device timer       */
	struct sd_cmd		   *cmd;    /* query dev command pointer*/
        struct sd_adap_info        *ap;     /* dasd's adapter           */
        uint                       base;    /* base address             */
        uchar                      pos0;    /* POS register 0 value     */
        uchar                      pos1;    /* POS register 1 value     */
        uchar                      pos2;    /* POS register 2 value     */
        ushort                     id;      /* id of device POS regs 0,1*/
        uint    old_pri;



        /*
         * get pointer to effected dasd
         */
        timer = (struct sd_watchdog *) w;
        cmd = (struct sd_cmd *) timer->pointer;

        old_pri = disable_lock(cmd->ap->dds.intr_priority,
			       &(cmd->ap->spin_lock));

        ap = (struct sd_adap_info *) cmd->ap;
#ifdef DEBUG
#ifdef SD_ERROR_PATH
	sd_trc(ap,qrytimer, entry,(char)0, (uint)ap, (uint)cmd , 
	       (uint)w, (uint) old_pri,(uint)0);
#endif
#endif
	/*
	 * set query device status as timed out, so it will be freed
	 */
	cmd->status |= SD_TIMEDOUT;
        sd_reset_quiesce(ap,(uchar)SD_RESET_OP,(uchar)SD_ADAP_CMD);
#ifdef DEBUG
#ifdef SD_ERROR_PATH
	sd_trc(ap,qrytimer, exit,(char)0, (uint)ap, (uint)0 ,(uint)0, 
	       (uint)0,(uint)0);
#endif
#endif

        unlock_enable(old_pri,&(ap->spin_lock));

        return;
}
