static char sccsid[] = "@(#)94  1.7.2.2  src/bos/kernext/disk/sd/sdclose.c, sysxdisk, bos411, 9428A410j 3/16/94 09:40:09";
/*
 * COMPONENT_NAME: (SYSXDISK) Serial Dasd Subsystem Device Driver
 *
 * FUNCTIONS: sd_close(), sd_adap_close(), sd_ctrl_close(), sd_dasd_close()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/priv.h>
#include <sys/intr.h>
#include <sys/ddtrace.h>
#include <sys/errno.h>
#include <sys/malloc.h>
#include <sys/lockl.h>
#include <sys/trchkid.h>
#include <sys/sd.h>

extern	struct	intr	*sd_epow_ptr;
extern	uchar	sd_open_adaps;
extern	uchar	sd_open_ctrls;
extern	uchar	sd_open_dasd;
extern	struct	watchdog activity_timer;

/*
 * NAME: sd_close
 *
 * FUNCTION: Close Entry Point for Serial DASD Subsystem Device Driver
 *
 * EXECUTION ENVIRONMENT: This routine is called on the process level
 *	and can page fault.                           
 *
 * (NOTES:) Possible operations : Determines which layer of the subsystem
 *	is to be closed (adapter, controller, dasd), and calls the
 *	appropriate routine.
 *
 * (RECOVERY OPERATION:)  If a failure occurs, the correct errno is
 *	returned, and recovery is up to the caller.
 *
 * RETURNS: 	Results of adapter, controller, or dasd close routine
 */

int	sd_close(
dev_t	devno,
int	chan,
int	ext)
{
	int			min,		/* minor number 	*/
				rc;		/* return code 		*/
	struct sd_adap_info 	*ap;            /* adapter pointer 	*/
	struct sd_ctrl_info 	*cp;		/* controller pointer 	*/
	struct sd_dasd_info 	*dp;		/* DASD pointer 	*/



	DDHKWD1(HKWD_DD_SERDASDD, DD_ENTRY_CLOSE, 0, devno);


	min = minor(devno);	/* get the minor number for this devno */


	chan = 1;   /* set chan to one to indicate file system close */

	lockl((&sd_global_lock), LOCK_SHORT); /* serialize using global lock */

	if (min & SD_DASD_MASK) {
		/*
		 * if this is a dasd call
		 */
		/*
		 * get dasd info pointer 
		 */
		dp = (struct sd_dasd_info *)sd_hash(devno); 	
		unlockl(&sd_global_lock);	/* unlock the global lock */
		if (dp == NULL)
			/*
			 * if dasd not configured, return error
			 */
			return(ENXIO);
		lockl((&dp->dev_lock), LOCK_SHORT);
		rc = sd_dasd_close(dp, chan, 0);
		unlockl(&dp->dev_lock);	
	} else if (min & SD_CTRL_MASK) {
		/*
		 * else if this is a controller call
		 */
		/*
		 * get controller info pointer 
		 */
		cp = (struct sd_ctrl_info *)sd_hash(devno); 	
		unlockl(&sd_global_lock);	/* unlock the global lock */
		if (cp == NULL)
			/*
			 * if adapter not configured, return error
			 */
			return(ENXIO);
		lockl((&cp->dev_lock), LOCK_SHORT);
		rc = sd_ctrl_close(cp, chan, 0);
		unlockl(&cp->dev_lock);	
	} else {
		/*
		 * else must be an adapter call
		 */
		/*
		 * get adapter info pointer 
		 */
		ap = (struct sd_adap_info *)sd_hash(devno); 	
		unlockl(&sd_global_lock);	/* unlock the global lock */
		if (ap == NULL)
			/*
			 * if adapter not configured, return error
			 */
			return(ENXIO);
		lockl((&ap->dev_lock), LOCK_SHORT);
		rc = sd_adap_close(ap, min, chan, 0);
		unlockl(&ap->dev_lock);	/* unlock the global lock */
	}

	DDHKWD1(HKWD_DD_SERDASDD, DD_EXIT_CLOSE, rc, devno);

	return(rc);			/* return our return code */
}


/*
 * NAME: sd_adap_close
 *
 * FUNCTION: Closes Serial DASD Adapter
 *
 * EXECUTION ENVIRONMENT: This routine is called on the process level
 *	and can page fault.                              
 *
 * (NOTES:) Possible operations : Frees up resources allocated on first
 *	open to adapter.
 *
 * (RECOVERY OPERATION:)  If a failure occurs, the correct errno is
 *	returned, and recovery is up to the caller.
 *
 * (DATA STRUCTURES:) 	sd_adap_info - adapter info structure
 *
 * RETURNS: 	0         - successful completion
 *      		  - not open
 *		ENXIO	  - adapter not configured
 */

int	sd_adap_close(
struct sd_adap_info *ap,
int	minorno,
int	chan,
int	ext)
{
	int	i,				/* counter */
		rc;				/* return code */
	uint	base,				/* base address */
	daemon_minor = FALSE;                   /* whether the minor number */
	                                        /* is a daemon entry point */
                                                /* or not                  */
	uchar	daemon_open;			/* flag if daemon open     */
	struct sd_cmd cmd;                      /* command jacket for error*/
						/* logging purposes only   */


	if (!(ap->opened))		
		/*
		 * This adapter not open, so return 
		 */
		return(0);

	daemon_open = ap->daemon_open;

	if (minorno & SD_DAEMON_MASK)
		daemon_minor = TRUE;

	if (daemon_minor && (ap->daemon_pid == getpid()))
	    chan = 0;

	if ( chan ) {
		/*
		 * if this is a file system close, then clear the fs_open
		 * flag, and the diagnostic flag
		 */
		ap->fs_open = FALSE;
		ap->diag_mode = FALSE;
	 } else if (daemon_minor) {
		/*
		 * if this is the daemon closing, clear local daemon
		 * open flag. Notice: sd_adap_config (TERM) waits for
		 * ap->daemon_open to go to false, so don't clear that
		 * one yet.
		 */
		daemon_open = FALSE;
		if (!(ap->unconfiguring)) {
			/*
			  * If this is the daemon closing and we are not 
			  * unconfiguring, then the daemon has been killed
			  * prematurely by someone else.  So lets log this
			  * error by filling in a command jacket.  The
			  * free of this command jacket will automatically
			  * log this error.
			  */
			
			cmd.ap = ap;
			cmd.type = SD_ADAP_CMD;
			cmd.uec = 0x1ff;
			cmd.elog_validity = 0x0;
			cmd.elog_sys_dma_rc = 0x0;
			cmd.alert_tag = 0x0;
			cmd.dev_address = 0x0;
			cmd.controller_status = 0x0;
			cmd.adapter_status = 0x0;
			sd_log_error(&cmd);
			
		}
	}
	else 
		/*
		 * else this must be an internal close, so check open
		 * controllers
		 */
		if (!ap->open_ctrls) 
			/*
			 * if there are no more open controllers, then 
			 * clear the internal open flag
			 */
			ap->internal_open = FALSE;

	if (!ap->fs_open && !ap->internal_open && !daemon_open) {
		/*
		 * if no file system open, and no internal opens, and the
		 * daemon is not open, then perform a FULL close, 
		 * in other words, free  up any resources and get out
		 */

		while (ap->cmds_out || (ap->ioctlhead != NULL) || 
			(ap->errhead != NULL)) {  
			/*
			 * if any outstanding commands this adapter, wait 
			 * for them to complete
			 */
			unlockl(&ap->dev_lock);	/* unlock the global lock */
			delay(HZ);	
			lockl((&ap->dev_lock), LOCK_SHORT);

		} 

		ap->opened = FALSE;     /* clear opened flag */

		/* 
		 * Disable Interrupts on Adapter ... 
		 */
                base = (uint)BUSIO_ATT(ap->dds.bus_id,ap->dds.base_addr);
                SD_PUTC((base + SD_CTRL), SD_INT_DISABLE);
                BUSIO_DET(base);
		/*
		 * Mark TCW's in management tables as Free ...
		 */
	        for (i=ap->sta_tcw_start; i<(ap->sta_tcw_start + 
		   SD_NUM_STA_TCWS); i++)
        	   SD_FREETAG(ap->tcw_free_list[(int)(i/SD_BITS_PER_WORD)],i);
	        for (i=ap->mb_tcw_start; i<(ap->mb_tcw_start + 
			SD_NUM_MBA_TCWS); i++)
        	   SD_FREETAG(ap->tcw_free_list[(int)(i/SD_BITS_PER_WORD)],i);

		d_mask(ap->dma_channel);         /* disable this DMA chan */
		d_clear(ap->dma_channel);        /* free this DMA chan */

	        /*
	         * free the small transfer area
	         */
	        (void)xmfree((caddr_t)ap->STA[0].stap,pinned_heap);
		/* 
		 * disable this card but first verify that the card has
		 * finished any pending POSTS that may have been started.
		 */
		ap->IPL_tmr_cnt = 0;    /* set-up initial value */
		ap->reset_result = FALSE;	/* clear result flag */
		sd_wait_reset_disable(ap->reset_timer);
		while (!ap->reset_result) {
			/*
			 * loop waiting for either card ready or 
			 * timeout
			 */
			delay(HZ);
		}
		if (ap->reset_result != (char)-1) {
			ap->pos2 &= ~SD_CARD_ENABLE;
			sd_write_POS(ap,SD_POS2,ap->pos2);
		}
		/* 
		 * clear interrupt structure 
		 */
	        i_clear((struct intr *)&(ap->sdi));
		/*
		 * Stop and clear any Watchdog timers here ...
		 */
		w_stop(&ap->ioctl_timer.watch); /* clear watchdog */
		w_stop(&ap->cmd_timer.watch); /* clear watchdog */
		while(w_clear(&ap->ioctl_timer)); /* clear watchdog */
		while(w_clear(&ap->cmd_timer)); /* clear watchdog */
		sd_open_adaps--; 	/* decrement opened adapters counter */
		if (!sd_open_adaps) {
	                /*
       		         * if this is the last adapter in system
       		         */
			i_clear(sd_epow_ptr); /*clear the epow hndlr*/
			xmfree((char *)sd_epow_ptr, (heapaddr_t) pinned_heap);
	                (void)unpincode((int (*)()) sd_intr);
			/*
			 * Stop global Activity Timer Here
			 */
			w_stop(&activity_timer);
			while(w_clear(&activity_timer)); /* clear watchdog */

			/* 
			 * De-Register Comp Dump Tbl 
			 */
			(void) dmp_del((struct cdt (*)())sd_adap_cdt_func);
        	}
	}
	if ((!chan) && (getpid() == ap->daemon_pid))
		ap->daemon_open = FALSE;
	return(0);
}

/*
 * NAME: sd_ctrl_close
 *
 * FUNCTION: Closes Serial DASD Controller
 *
 * EXECUTION ENVIRONMENT: This routine is called on the process level
 *	and can page fault.                              
 *
 * (RECOVERY OPERATION:)  If a failure occurs, the correct errno is
 *	returned, and recovery is up to the caller.
 *
 * (DATA STRUCTURES:) 	sd_ctrl_info - controller info structure
 *
 * RETURNS: 	0         - successful completion
 *      		  - not open
 *		ENXIO	  - controller not configured
 */

int	sd_ctrl_close(
struct sd_ctrl_info *cp,
int	chan,
int	ext)
{

	if (!(cp->opened))
		/*
		 * This controller not open, so return 
		 */
		return(0);

	if ( chan ) {
		/*
		 * if this is a file system close, then clear the fs_open
		 * flag, and the diagnostic flag
		 */
		cp->fs_open = FALSE;
		cp->diag_mode = FALSE;
	} else 
		/*
		 * else this must be an internal close, so check open
		 * dasd
		 */
		if (!cp->open_dasd) 
			/*
			 * if there are no more open dasd, then 
			 * clear the internal open flag
			 */
			cp->internal_open = FALSE;

	if (!cp->fs_open && !cp->internal_open ) {
		/*
		 * if no file system open, and no internal opens
		 * then perform a full close of the controller
		 */
		while (cp->cmds_out || cp->cmds_qed) {
			/*
			 * if this controller has commands out or queued,
			 * let them finish
			 */
			delay(HZ);		
		} 					

		if (--sd_open_ctrls == 0) {
			/* 
			 * if this is the last controller in the system
			 * then De-register Component Dump table ..(dmp_del)
			 */
			(void) dmp_del((struct cdt (*)())sd_ctrl_cdt_func);
		}

		cp->opened = FALSE;	/* clear open flag */

		/*
		 * Stop and clear any Watchdog timers here ...
		 */
		w_stop(&cp->ioctl_timer.watch); /* clear watchdog */
		w_stop(&cp->cmd_timer.watch); /* clear watchdog */
		while(w_clear(&cp->ioctl_timer)); /* clear watchdog */
		while(w_clear(&cp->cmd_timer)); /* clear watchdog */

		cp->ap->open_ctrls--; /* dec count of open ctrls on his adap */
		/*
		 * call this controllers adapter's close... 
		 */
		lockl((&(cp->ap->dev_lock)), LOCK_SHORT);
		sd_adap_close(cp->ap, minor(cp->dds.adapter_devno), 0, 0);	
		unlockl(&(cp->ap->dev_lock));	/* unlock the global lock */
	}

	return(0);
}

/*
 * NAME: sd_dasd_close
 *
 * FUNCTION: Closes Serial DASD DASD
 *
 * EXECUTION ENVIRONMENT: This routine is called on the process level
 *	and can page fault.                              
 *
 * (RECOVERY OPERATION:)  If a failure occurs, the correct errno is
 *	returned, and recovery is up to the caller.
 *
 * (DATA STRUCTURES:) 	sd_dasd_info - dasd info structure
 *
 * RETURNS: 	0         - successful completion
 *      		  - not open
 *		ENXIO	  - dasd not configured
 */

int	sd_dasd_close(
struct sd_dasd_info *dp,
int	chan,
int	ext)
{

	if (!(dp->opened))		
		/*
		 * This dasd not open, so return 
		 */
		return(0);

	if ((dp->errhead != NULL) && (dp->errhead->cmd_info == SD_START_UNIT)) {
		/*
		 * we may have left a start unit on the errqueue if
		 * we failed the dasd, but that's OK, just dequeue it.
		 */
		sd_d_q_cmd_disable(dp->errhead);
		sd_free_cmd_disable(&dp->restart);
	}

	while ((dp->ioctlhead != NULL) || (dp->errhead != NULL) || 
  	      (dp->currbuf != NULL) || dp->cmds_out) {
		/*
		 * if this dasd's's Qs are not empty, or he is inited 
		 * as dump device, or he has commands outstanding...
		 */
		delay(HZ);	/* wait 1 second */
	} 					
	
	/*
	 * Now de-register if there is still a kernel
	 * extension registered with us.
	 */
	
	dp->conc_registered = FALSE;
	dp->conc_intr_addr = NULL;                      /* for safety */

	/*
	 * And clear any fence data
	 */

	dp->fence_data_valid = FALSE;
	dp->fence_data = 0;
	dp->fence_mask = 0;

	if (--sd_open_dasd == 0) {
		/*
		 * if this is last dasd in system, De-register 
		 * Component Dump table ...(dmp_del)
		 */
		(void) dmp_del((struct cdt (*)())sd_dasd_cdt_func);
	}
	if ((!dp->diag_mode) && (!dp->retain_reservation) && 
		(!dp->no_reserve)) {	
		/*
		 * if not diagnostic mode and not retain
		 * reservation, and we did reserve
		 */
		dp->seq_not_done = TRUE;
		sd_release_disable(dp,SD_RELEASE); 
		sd_start_disable(dp->ap);
		/*
		 * wait for completion of the release         
		 */
		sd_sleep(dp->ap,&dp->seq_not_done,&dp->dasd_event);
	}
	if ((dp->errhead != NULL) && (dp->errhead->cmd_info == SD_START_UNIT)) {
		/*
		 * we may have left a start unit on the errqueue if
		 * we failed the dasd, but that's OK, just dequeue it.
		 */
		sd_d_q_cmd_disable(dp->errhead);
		sd_free_cmd_disable(&dp->restart);
	}
	dp->opened = FALSE;		/* clear opened flag */
	dp->diag_mode = FALSE;		/* clear diag mode flag */
	dp->retain_reservation = FALSE; /* clear retain reservation flag */
	dp->no_reserve = FALSE; 	/* clear no reserve flag */

	/*
	 * Stop and clear any Watchdog timers here ...
	 */
	w_stop(&dp->ioctl_timer.watch); /* stop watchdog */
	while(w_clear(&dp->ioctl_timer)); /* clear watchdog */
	w_stop(&dp->cmd_timer.watch); /* stop watchdog */
	while(w_clear(&dp->cmd_timer)); /* clear watchdog */
	w_stop(&dp->query_timer.watch); /* stop watchdog */
	while(w_clear(&dp->query_timer)); /* clear watchdog */

	dp->cp->open_dasd--;  /* decrement his controllers dasd count */
	/*
	 * call this dasd's controllers close... 
	 */
	lockl((&(dp->cp->dev_lock)), LOCK_SHORT);
	sd_ctrl_close(dp->cp, 0, 0);	
	unlockl(&(dp->cp->dev_lock));	/* unlock the global lock */
	return(0);
}
   
