static char sccsid[] = "@(#)73	1.1.1.7  src/bos/kernext/scsi/hscwdog.c, sysxscsi, bos411, 9428A410j 1/24/94 11:21:03";
/*
 * COMPONENT_NAME: (SYSXSCSI) IBM SCSI Adapter Driver
 *
 * FUNCTIONS:	hsc_watchdog
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/************************************************************************/
/*                                                                      */
/* COMPONENT:   SYSXSCSI                                                */
/*                                                                      */
/* NAME:        hscwdog.c                                               */
/*                                                                      */
/* FUNCTION:    IBM SCSI Adapter Driver Source File                     */
/*                                                                      */
/*      This adapter driver is the interface between a SCSI device      */
/*      driver and the actual SCSI adapter.  It executes commands       */
/*      from multiple drivers which contain generic SCSI device         */
/*      commands, and manages the execution of those commands.          */
/*      Several ioctls are defined to provide for system management     */
/*      and adapter diagnostic functions.                               */
/*                                                                      */
/* STYLE:                                                               */
/*                                                                      */
/*      To format this file for proper style, use the indent command    */
/*      with the following options:                                     */
/*                                                                      */
/*      -bap -ncdb -nce -cli0.5 -di8 -nfc1 -i4 -l78 -nsc -nbbb -lp      */
/*      -c4 -nei -nip                                                   */
/*                                                                      */
/*      Following formatting with the indent command, comment lines     */
/*      longer than 80 columns will need to be manually reformatted.    */
/*      To search for lines longer than 80 columns, use:                */
/*                                                                      */
/*      cat <file> | untab | fgrep -v sccsid | awk "length >79"         */
/*                                                                      */
/*      The indent command may need to be run multiple times.  Make     */
/*      sure that the final source can be indented again and produce    */
/*      the identical file.                                             */
/*                                                                      */
/************************************************************************/

#include	"hscincl.h"
#include	"hscxdec.h"

/************************************************************************/
/*                                                                      */
/* NAME:        hsc_watchdog                                            */
/*                                                                      */
/* FUNCTION:    Adapter Watchdog Timer Handler                          */
/*                                                                      */
/*      This internal routine handles watchdog timer timeout            */
/*      conditions.                                                     */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine is called on the timer handler intrpt level,       */
/*      therefore, it cannot cause a page-fault.  It can be             */
/*      interrupted by the interrupt handler.                           */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      sc_buf  - input/output request struct used between the adapter  */
/*                driver and the calling SCSI device driver             */
/*      mbstruct - driver structure which holds information related to  */
/*                 a particular mailbox and hence a command             */
/*      watchdog - kernel watchdog timer structure                      */
/*      timer    - driver structure which holds information related to  */
/*                 a particular watchdog timer                          */
/*                                                                      */
/* INPUTS:                                                              */
/*      w       - pointer to watchdog timer structure which timed-out   */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  none                                      */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      i_disable       i_enable                                        */
/*      e_wakeup                                                        */
/*                                                                      */
/************************************************************************/
void
hsc_watchdog(
	     struct watchdog * w)
{
    struct adapter_def *ap;
    struct mbstruct *mbp;
    struct sc_buf *scp, *tscp;
    struct timer *tdog;
    int     old_pri;
    int     dev_index, t_index;
    int     cmd_done;
    int     base_time;
    uchar   reset_needed;
    uchar   id, tag;
    struct io_parms iop;
    struct b_link *buf;
    struct dev_info *d;

    tdog = (struct timer *) w;	/* get my timer struct ptr */
    ap = tdog->adp;	/* get adap pointer */

    /* keep intrpts out   */
    old_pri = disable_lock(ap->ddi.int_prior, &(hsc_mp_lock));

    /* if we get here, some sort of fatal command timeout occurred */
    if (tdog->timer_id == SC_ADAP_TMR) {
/************************************************************************/
/*	handle MB30 command timeouts here                               */
/************************************************************************/
	/* N.B.  adapter driver logs adapter MB30 cmd timeouts */

	if (ap->MB30_in_use == PROC_USING) {	/* if proc lvl timed-out */
/************************************************************************/
/*	handle process level called MB30 command timeout here           */
/************************************************************************/
	    hsc_logerr(ap, ERRID_SCSI_ERR1, ap->MB30p, COMMAND_TIMEOUT, 4, 0);
	    cmd_done = FALSE;	/* flag adap cmd still in progress */
	    ap->proc_results = TIMED_OUT;	/* flag the error */
	    if (ap->proc_waiting == WAIT_FOR_INIT_DEV) {
		/* reset ioctl timed-out; must reset bus here */
		if (hsc_scsi_reset(ap) == 0) {	/* if good scsi reset */
		    /* state for waiting on t.o. or intrpt */
		    ap->proc_waiting = WAIT_FOR_INIT_T_O_2;
		    /* set t.o. value */
		    ap->wdog.dog.restart = INIT_CMD_T_O_2;
		    w_start(&ap->wdog.dog);	/* kick-off timer */
		}
		else {	/* unsuccessful scsi reset */
		    cmd_done = TRUE;	/* flag adap cmd done */
		}
	    }
	    else {
		if (ap->proc_waiting == WAIT_FOR_INIT_LUN) {
		    /* halt ioctl timed-out; must reset bus here */
		    /* if good scsi reset */
		    if (hsc_scsi_reset(ap) == 0) {
			/* state for waiting on t.o. or intrpt */
			ap->proc_waiting = WAIT_FOR_INIT_T_O_2;
			/* set t.o. value */
			ap->wdog.dog.restart = INIT_CMD_T_O_2;
			w_start(&ap->wdog.dog);	/* kick-off timer */
		    }
		    else {	/* unsuccessful scsi reset */
			cmd_done = TRUE;	/* flag adap cmd done */
		    }
		}
		else {
		    /* handle process level target mode */
		    /* enable/disable timeouts.		 */

		    if ((ap->proc_waiting == WAIT_FOR_ENA_ID) ||
			(ap->proc_waiting == WAIT_FOR_ENA_BUF)) {
			if (hsc_scsi_reset(ap) == 0) {
			    /* state for waiting on t.o. or intrpt */
			    ap->proc_waiting = WAIT_FOR_ENA_T_O_2;
			    /* set t.o. value */
			    ap->wdog.dog.restart = INIT_CMD_T_O_2;
			    w_start(&ap->wdog.dog);	/* kick-off timer */
			}
			else {	/* unsuccessful scsi reset */
			    cmd_done = TRUE;	/* flag adap cmd done */
			}
		    }
		    else {	/* handle other proc_waiting states */
			cmd_done = TRUE;	/* flag adap cmd done */
		    }
		}
		if (cmd_done) {	/* if done, release mb30 */
		    ap->proc_waiting = 0;	/* reset waiting flag */
		    ap->MB30_retries = 0;	/* reset retry count */
		    ap->MB30_in_use = -1;	/* free MB30 */
		    e_wakeup(&ap->event);	/* return to proc level */
		}
	    }
	}
	else {
	    if (ap->MB30_in_use == ADAP_USING) {
/************************************************************************/
/*	handle timeout during adapter error recovery using MB30 here    */
/************************************************************************/
		hsc_logerr(ap, ERRID_SCSI_ERR1, ap->MB30p,
			   COMMAND_TIMEOUT, 5, 0);
		/* N.B. below assumes restart is the only adap err recovery */
		/* (reset would be same recovery if INIT DEV is used)       */
		if (ap->restart_state == WAIT_FOR_RESTART) {
		    if (ap->restart_retries < MAX_RESTART_RETRIES) {
			ap->restart_retries++;
			/* attempt scsi reset to recover.  Note: if scsi */
			/* reset fails due to pio error, we may get      */
			/* sequence errors on all future commands--no    */
			/* alternative to this...                        */
			if (hsc_scsi_reset(ap) == EIO)	/* if pio error */
			    ap->restart_retries = 0;
		    }
		    else {
			ap->restart_retries = 0;
		    }
		}
		ap->MB30_in_use = -1;	/* free MB30 */
		/* need another restart? */
		if (ap->restart_again == TRUE) {
		    ap->restart_state = WAIT_TO_SEND_RESTART;
		    ap->restart_again = FALSE;	/* reset again flag */
		    /* flag the waiting cmd */
		    ap->waiting_for_mb30 = TRUE;
		}
		else {
		    ap->restart_state = 0;	/* reset waiting state */
		    /* finish restart error recovery here !!                */
		    /* the following kicks-off cmds which may have tried    */
		    /* to execute while waiting for the restart to complete */
		    if (ap->restart_index_validity) {
			hsc_start(ap, ap->restart_index);
			/* reset which device to start */
			ap->restart_index = 0;
			/* flag to say we saw index */
			ap->restart_index_validity = FALSE;
		    }
		}
	    }
	    else {
		/* check if initiator mode devices using */
		if ((ap->MB30_in_use >= 0) &&
		    (ap->MB30_in_use < IMDEVICES)) {
/************************************************************************/
/*	handle timeout during device error recovery using MB30 here     */
/************************************************************************/
		    /* here, the MB30_in_use flag holds the device index */
		    dev_index = (int) ap->MB30_in_use;
		    cmd_done = FALSE;	/* flag adap cmd still in progress */
		    hsc_logerr(ap, ERRID_SCSI_ERR2, ap->MB30p,
			       COMMAND_TIMEOUT, 6, 0);
		    if ((ap->dev[dev_index].state == WAIT_FOR_INIT_LUN) ||
		       (ap->dev[dev_index].state == WAIT_FOR_RESUME)) {
			/* if good scsi reset */
			if (hsc_scsi_reset(ap) == 0) {
			    /* set device state to waiting for init t.o. or
			       intrpt */
			    ap->dev[dev_index].state = WAIT_FOR_INIT_T_O_2;
			    /* set t.o. value */
			    ap->wdog.dog.restart = INIT_CMD_T_O_2;
			    w_start(&ap->wdog.dog);	/* kick-off timer */
			}
			else {	/* unsuccessful scsi reset */
			    cmd_done = TRUE;	/* flag adap cmd done */
			}
		    }
		    else {	/* states other than WAIT_FOR_INIT_LUN */
			cmd_done = TRUE;	/* flag adap cmd done */
		    }
		    /* if done, clear queue, release mb30 */
		    if (cmd_done) {
			if (ap->dev[dev_index].head_act != NULL) {
			    (void) hsc_dma_cleanup(ap,
					 ap->dev[dev_index].head_act, TRUE);
			}
			/* sc_buf status already set */
			hsc_fail_cmd(ap, dev_index);	/* fail the queue */
			/* reset waiting state */
			ap->dev[dev_index].state = 0;
			ap->MB30_in_use = -1;	/* free MB30 */
		    }
		}
		else {
		    /* check if target mode using  */
		    if (ap->MB30_in_use == TM_DEVICE_USING) {
			cmd_done = FALSE;
			hsc_logerr(ap, ERRID_SCSI_ERR1, ap->MB30p,
				   COMMAND_TIMEOUT, 95, 0);
			if ((ap->enabuf_state == WAIT_FOR_DIS_BUF) ||
			    (ap->enabuf_state == WAIT_FOR_ENA_BUF) ||
			    (ap->enaid_state == WAIT_FOR_ENA_ID) ||
			    (ap->disid_state == WAIT_FOR_DIS_ID)) {
			    if (hsc_scsi_reset(ap) == 0) {
				if (ap->enabuf_state == WAIT_FOR_DIS_BUF)
				    ap->enabuf_state = WAIT_FOR_DISBUF_T_O_2;
				else
				    if (ap->enabuf_state == WAIT_FOR_ENA_BUF)
					ap->enabuf_state =
					    WAIT_FOR_ENABUF_T_O_2;
				    else
					if (ap->enaid_state == WAIT_FOR_ENA_ID)
					    ap->enaid_state =
						WAIT_FOR_ENAID_T_O_2;
					else
					    if (ap->disid_state ==
						WAIT_FOR_DIS_ID)
						ap->disid_state =
						    WAIT_FOR_DISID_T_O_2;

				/* set t.o. value */
				ap->wdog.dog.restart = INIT_CMD_T_O_2;
				w_start(&ap->wdog.dog);	/* kick-off timer */
			    }
			    else {	/* unsuccessful scsi reset */
				/* flag adap cmd done */
				cmd_done = TRUE;
			    }
			}
			else {	/* get here for other TM device using states */
			    cmd_done = TRUE;
			}

			if (cmd_done == TRUE) {
			    ap->MB30_in_use = -1;
			    ap->MB30_retries = 0;
			    if ((ap->enabuf_state == WAIT_FOR_DIS_BUF) ||
			      (ap->enabuf_state == WAIT_FOR_DISBUF_T_O_2)) {
				ap->enabuf_state = 0;
				ap->disbuf_retries++;
				hsc_tgt_DMA_err(ap);
			    }
			    else
				if ((ap->enabuf_state == WAIT_FOR_ENA_BUF) ||
				    (ap->enabuf_state ==
				     WAIT_FOR_ENABUF_T_O_2)) {
				    tag = ap->MB30p->mb.m_sequ_num;
				    buf = ap->tm_bufs[tag];
				    ap->enabuf_state = 0;
				    ap->enabuf_retries++;
				    ap->tm_bufs[tag] = NULL;
				    (void) hsc_tgt_tcw_dealloc(ap, buf, FALSE);
				    if (ap->tgt_dma_err)
					hsc_tgt_DMA_err(ap);
				    else
					hsc_start_bufs(ap);
				}
				else
				    if ((ap->disid_state == WAIT_FOR_DIS_ID) ||
					(ap->disid_state ==
					 WAIT_FOR_DISID_T_O_2)) {
					id = ap->MB30p->mb.m_sequ_num;
					ap->disid_retries++;
					ap->disid_state = 0;
					(void) hsc_stop_ids(ap);
				    }
				    else
					if ((ap->enaid_state ==
					     WAIT_FOR_ENA_ID) ||
					    (ap->enaid_state ==
					     WAIT_FOR_ENAID_T_O_2)) {
					    id = ap->MB30p->mb.m_sequ_num;
					    ap->enaid_retries++;
					    ap->enaid_state = 0;
					    (void) hsc_start_ids(ap);
					}
			}
		    }
		    else {
			/* log as temp. only, as no-one was using MB30 */
			hsc_logerr(ap, ERRID_SCSI_ERR6, ap->MB30p,
				   COMMAND_TIMEOUT, 7, 0);
		    }
		}
	    }
	}
	/* note that interrupts are disabled here */
	hsc_MB30_handler(ap, FALSE);	/* handle waiting requests */
    }
    else {
	if (tdog->timer_id == SC_MBOX_TMR) {
/************************************************************************/
/*	handle MB 0-29 command timeouts here                            */
/************************************************************************/
	    /* N.B.  caller logs command timeouts */

	    /* get mbox pointer at head of active list */
	    dev_index = tdog->dev_index;
            if (ap->dev[dev_index].head_act == NULL ) {
                        hsc_logerr(ap, ERRID_SCSI_ERR6, NULL, 0, 100, 0);
                        /* restore intrpt priority */
                        unlock_enable(old_pri, &(hsc_mp_lock));
                        return;
            }
            if (( mbp = (struct mbstruct *) ap->dev[dev_index].
                                head_act->bufstruct.b_work ) == NULL ) {
                        /* restore intrpt priority */
                        unlock_enable(old_pri, &(hsc_mp_lock));
                        return;
            }

	    scp = mbp->sc_buf_ptr;      /* get sc_buf ptr from mbox */
	    tscp = ap->dev[dev_index].head_act;

	    switch( mbp->cmd_state ) {
		case	ISACTIVE:
	    	   /* if this is original timeout */
		   /* determine if this is a valid timeout.	     */
		   /* walk the list of commands on this device's     */
		   /* active queue to ensure that no cmd on the list */
		   /* has a timeout value greater than that at the   */
		   /* head of the list. If a greater value exists,   */
		   /* an erroneous timeout may have occurred as a    */
		   /* result of the device reordering commands.	     */
		   base_time = scp->timeout_value;
		   while( tscp != NULL ) {
		   	if ( tscp->timeout_value > base_time )
		      		base_time = tscp->timeout_value;
		        tscp = (struct sc_buf *) tscp->bufstruct.av_forw;
		   }



		   
		   if (( base_time > scp->timeout_value ) && ( base_time > 
				ap->dev[dev_index].wdog->save_time )) {
   			ap->dev[dev_index].wdog->dog.restart = base_time - 
							scp->timeout_value;
			ap->dev[dev_index].wdog->save_time = base_time; 
		   	w_start(&ap->dev[dev_index].wdog->dog);
                        /* restore intrpt priority */
                        unlock_enable(old_pri, &(hsc_mp_lock));
		   	return;
		   }
		   else
			ap->dev[dev_index].wdog->save_time = 0; 
		

		   /* set up sc_buf status in prepartion for io_done call */
		   scp->status_validity = SC_ADAPTER_ERROR;
		   scp->general_card_status = SC_CMD_TIMEOUT;
		   scp->scsi_status = SC_GOOD_STATUS;
		   scp->bufstruct.b_resid = scp->bufstruct.b_bcount;
		   scp->bufstruct.b_error = EIO;
		   
                   /* set cmd_state for all active queue elements to  */
                   /* WAIT_FOR_T_0_2 for proper completion processing */
		   /* reset SCSI bus to clean up this cmd and device. */
		   /* DO NOT issue abort or BDR, as data integrity    */
		   /* can be violated by doing that.  Note that a     */
		   /* failed scsi reset will result in a second       */
		   /* timeout, this time in WAIT_FOR_T_O_2 state.     */
                   tscp = ap->dev[dev_index].head_act;
                   while( tscp != NULL ) {
                           mbp = tscp->bufstruct.b_work;
                           mbp->cmd_state = WAIT_FOR_T_O_2;
                           tscp = (struct sc_buf *) tscp->bufstruct.av_forw;
                   }
                   (void) hsc_scsi_reset(ap);
                   ap->dev[dev_index].wdog->dog.restart = SCSI_RESET_T_O;
                   w_start(&ap->dev[dev_index].wdog->dog);
		   break;

		case	WAIT_FOR_T_O_2:
		    /* adapter died, no further interrupts will be    */
		    /* seen and final cmd cleanup must happen here.   */
		    /* change mailbox state of each cmd so that       */
		    /* resources will be freed properly		      */
                    tscp = ap->dev[dev_index].head_act;
                    while( tscp != NULL ) {
                            mbp = tscp->bufstruct.b_work;
                            mbp->cmd_state = INTERRUPT_RECVD;
                            tscp = (struct sc_buf *) tscp->bufstruct.av_forw;
                    }

		    /* if data transfer, do the d_complete, but do not copy */
		    /* data (if STA used).  Ignore return code, as failure  */
		    /* return code is already set above.                    */
		    (void) hsc_dma_cleanup(ap, scp, TRUE);

		    /* fail the device queue */
		    hsc_fail_cmd(ap, dev_index);
		    break;
		
		case	INTERRUPT_RECVD:
                        /* if we get here then an error has occured since   */
                        /* the head of the active queue should never be in  */
                        /* this state.  Log an error and reset the bus if   */
                        /* there are any commands still active then fail    */
                        /* the queue.                                       */
                        reset_needed = FALSE;
                   	tscp = ap->dev[dev_index].head_act;
                        hsc_logerr(ap, ERRID_SCSI_ERR2, tscp->bufstruct.b_work,
                                    0, 98, 0);
                   	while( tscp != NULL ) {
                       	    mbp = tscp->bufstruct.b_work;
                       	    if (mbp->cmd_state == ISACTIVE) {
                       	        mbp->cmd_state = WAIT_FOR_T_O_2;
                                reset_needed = TRUE;
                            }
                       	    tscp = (struct sc_buf *) tscp->bufstruct.av_forw;
                   	}
                        if (reset_needed) {
                   	    (void) hsc_scsi_reset(ap);
                            ap->dev[dev_index].wdog->dog.restart = 
                                            SCSI_RESET_T_O;
                            w_start(&ap->dev[dev_index].wdog->dog);
                        }
                        else hsc_fail_cmd(ap, dev_index); /* fail the queue */

			break;

		default:
                   	tscp = ap->dev[dev_index].head_act;
                        hsc_logerr(ap, ERRID_SCSI_ERR6, tscp->bufstruct.b_work,
                                    0, 99, 0);
                        hsc_fail_cmd(ap, dev_index); /* fail the queue */
			break;
	    }
	}
	else {
	    if (tdog->timer_id == SC_ADAP_TMR_2) {
/************************************************************************/
/*	handle command delay after reset timer here                     */
/************************************************************************/
		/* this indicates the end of the delay period following a */
		/* scsi reset or BDR message--restart delayed devices     */
		ap->cdar_scsi_ids = 0;	/* clear delayed SCSI ID flags    */
                ap->epow_reset = FALSE;
		for (t_index = 0; t_index < IMDEVICES; t_index++) {
		    hsc_start(ap, t_index);
		}	/* end for */
	    }
	    else if (tdog->timer_id == SC_ADAP_TMR_3) {
                     if((mbp = (struct mbstruct *) ap->head_MB_wait) == NULL) {
                             /* restore intrpt priority */
                             unlock_enable(old_pri, &(hsc_mp_lock));
                             return;
                     }
		    ASSERT(mbp->cmd_state == WAIT_FOR_INTRPT);
		    /* we get here when a cmd which was iodoned and dma */
		    /* cleaned up without having received its interrupt */
		    /* has waited too long to get the expected interrupt */
		    /* (cmd was iodoned because of error processing due */
		    /* to cmd queuing err hdlng, or term by init cmd    */
		    /* prior to seeing its interrupt). must free resrces */
		    /* N.B. leave mailbox state as it is so that the    */
		    /* MB_dealloc routine will move mbox to free list.  */
		    hsc_STA_dealloc(ap, mbp);
		    hsc_TCW_dealloc(ap, mbp);
		    hsc_MB_dealloc(ap, mbp);
	    }
	    else {
/************************************************************************/
/*	handle unknown command timeouts here                            */
/************************************************************************/
		/* log as temp. only, as this is an unknown condition */
		hsc_logerr(ap, ERRID_SCSI_ERR6, NULL, COMMAND_TIMEOUT, 8, 0);
	    }
	}
    }
     unlock_enable(old_pri, &(hsc_mp_lock)); /* restore intrpt priority */

}  /* end hsc_watchdog */
