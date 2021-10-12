static char sccsid[] = "@(#)68	1.2.3.2  src/bos/kernext/scsi/hscmb30intr.c, sysxscsi, bos411, 9428A410j 1/24/94 11:19:38";
/*
 * COMPONENT_NAME: (SYSXSCSI) IBM SCSI Adapter Driver
 *
 * FUNCTIONS:	hsc_MB30_handler
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
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
/* NAME:        hscmb30intr.c                                           */
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
/* NAME:        hsc_MB30_handler                                        */
/*                                                                      */
/* FUNCTION:    MB30 Interrupt Hanlding Routine                         */
/*      This internal routine is called by the interrupt handler        */
/*      to handle both good and bad MB30 interrupts.                    */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called on priority levels greater      */
/*      than, or equal to that of the interrupt handler.                */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      sc_buf  - input/output request struct used between the adapter  */
/*                driver and the calling SCSI device driver             */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter structure                          */
/*      flag    - indicate this call is due to a MB30 interrupt. If     */
/*                FALSE, logic for handling an interrupt is bypassed.   */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      w_start         w_stop                                          */
/*      e_wakeup        bzero                                           */
/*      pio_assist                                                      */
/*                                                                      */
/************************************************************************/
void
hsc_MB30_handler(
		 struct adapter_def * ap,
		 int flag)
{
    int     anyone_waiting, restart_recovery;
    int     new_error_state;
    int     i, t_index, dev_index;
    struct sc_buf *scp;
    struct io_parms iop;
    int     rc;
    struct dev_info *d, *owner;
    struct b_link *buf;
    uchar   burst, id, tag, cmd_failure, retrying;

    /* init iop structure */
    bzero(&iop, sizeof(struct io_parms));
    iop.ap = ap;

#ifdef HSC_NEVER
    hsc_internal_trace(ap, TRC_MB30INTR, 'ENT ', 0, 0, 0, 0);
#endif HSC_NEVER	
    if (flag) {	/* called due to a real MB30 interrupt */
	/* get mailbox 30 status area */
	iop.opt = RD_MB30_STAT;
	if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	    /* cannot get the mailbox status, ignore the interrupt.  */
	    return;	/* allow to timeout      */
	}
	/* execute internal trace point for this command */
#ifdef HSC_TRACE
	hsc_internal_trace(ap, TRC_MB30, 'DONE', 0, 0);
#endif HSC_TRACE
	/* Handle Target mode MB30 command intrs first	 */

	if (ap->MB30_in_use == TM_DEVICE_USING) {
	    w_stop(&ap->wdog.dog);
	    new_error_state = 0;	/* initial flag value */
	    cmd_failure = FALSE;
	    retrying = FALSE;
	    if ((ap->enabuf_state == WAIT_FOR_ENA_BUF) ||
		(ap->enabuf_state == WAIT_FOR_DIS_BUF) ||
		(ap->enaid_state == WAIT_FOR_ENA_ID) ||
		(ap->disid_state == WAIT_FOR_DIS_ID)) {
		switch (ap->MB30p->mb.m_adapter_rc) {
		  case COMPLETE_NO_ERRORS:
		    break;
		  case MB_PARAMETER_ERROR:
		  case MB_PARITY_ERROR:
		    if (ap->MB30_retries >= MAX_MB30_RETRIES) {
			cmd_failure = TRUE;
			if (ap->MB30p->mb.m_adapter_rc == MB_PARITY_ERROR) {
			    hsc_logerr(ap, ERRID_SCSI_ERR1, ap->MB30p,
				       PIO_WR_DATA_ERR, 80, 0);
			}
			else {
			    hsc_logerr(ap, ERRID_SCSI_ERR5, ap->MB30p,
				       0, 81, 0);
			}
		    }
		    else {	/* retry the MB30 command */
			if (ap->MB30p->mb.m_adapter_rc == MB_PARITY_ERROR) {
			    hsc_logerr(ap, ERRID_SCSI_ERR2, ap->MB30p,
				       PIO_WR_DATA_ERR, 82, 0);
			}
			else {
			    hsc_logerr(ap, ERRID_SCSI_ERR6, ap->MB30p,
				       0, 83, 0);
			}
			ap->MB30_retries++;	/* inc retry count */
			if ((ap->enabuf_state == WAIT_FOR_ENA_BUF) ||
			    (ap->enabuf_state == WAIT_FOR_DIS_BUF))
			    new_error_state = ap->enabuf_state;
			else
			    if (ap->enaid_state == WAIT_FOR_ENA_ID)
				new_error_state = ap->enaid_state;
			    else
				new_error_state = ap->disid_state;
			w_start(&ap->wdog.dog);	/* restart timer */
			retrying = TRUE;
			bzero(&ap->MB30p->mb.m_resid, MB_STAT_SIZE);

			/* send MB30 command to adapter */
			iop.opt = WRITE_MB30;
			iop.errtype = 0;
			if ((pio_assist(&iop, hsc_pio_function,
					hsc_pio_recov) == EIO) &&
			    (iop.errtype != PIO_PERM_IOCC_ERR)) {

			    /* handle unrecovered error sending mbox 30 */
			    /* when not an iocc internal error.         */
			    /* report fatal error to calling process.   */

			    cmd_failure = TRUE;
			    w_stop(&ap->wdog.dog);	/* stop timer */
			    new_error_state = 0;
			    retrying = FALSE;
			}
		    }
		    break;
		  case BAD_FUSE:
		    /* adapter is dead.log occurs via MB31 indication. */
		    cmd_failure = TRUE;
		    break;
		  case SCSI_BUS_RESET:
		    /* the command was still successful here */
		    break;
		  default:
		    /* unknown card interrupt status */
		    cmd_failure = TRUE;
		    hsc_logerr(ap, ERRID_SCSI_ERR4, ap->MB30p,
			       UNKNOWN_CARD_ERR, 84, 0);
		    break;
		}	/* end switch */
	    }
	    else {	/* not waiting for enable or disable! */
		if ((ap->enabuf_state == WAIT_FOR_ENABUF_T_O_2) ||
		    (ap->enabuf_state == WAIT_FOR_DISBUF_T_O_2) ||
		    (ap->enaid_state == WAIT_FOR_ENAID_T_O_2) ||
		    (ap->disid_state == WAIT_FOR_DISID_T_O_2)) {
		    cmd_failure = TRUE;
		}
		else {	/* unknown target mode intr lvl mb30 state */
		    hsc_logerr(ap, ERRID_SCSI_ERR6, ap->MB30p, 0, 85, 0);
		    ap->MB30_retries = 0;
		    ap->MB30_in_use = -1;
		    goto process_waiting_cmds;
		}
	    }
	    if (retrying == TRUE) {
		if ((new_error_state == WAIT_FOR_ENA_BUF) ||
		    (new_error_state == WAIT_FOR_DIS_BUF))
		    ap->enabuf_state = new_error_state;
		else
		    if (new_error_state == WAIT_FOR_ENA_ID)
			ap->enaid_state = new_error_state;
		    else
			ap->disid_state = new_error_state;
	    }
	    else {
		ap->MB30_retries = 0;
		ap->MB30_in_use = -1;
		if ((ap->disid_state == WAIT_FOR_DIS_ID) ||
		    (ap->disid_state == WAIT_FOR_DISID_T_O_2)) {
		    id = ap->MB30p->mb.m_sequ_num;
		    ap->disid_state = 0;
		    if (cmd_failure == TRUE) {
			ap->disid_retries++;
		    }
		    else {
			ap->disid_retries = 0;
			/* disable id done -- reset id and t-flag */
			ap->waiting_disids &= ~(1 << id);
			if (ap->t_dis_ids & (1 << id)) {
			    /* this was a temporary disable id */
			    ap->dev[id + IMDEVICES].stopped = TRUE;
			    ap->t_dis_ids &= ~(1 << id);
			    (void) hsc_async_notify(ap, (id + IMDEVICES),
						    SC_BUFS_EXHAUSTED);
			}
		    }
		    (void) hsc_stop_ids(ap);
		}
		else
		    if ((ap->enabuf_state == WAIT_FOR_ENA_BUF) ||
			(ap->enabuf_state == WAIT_FOR_ENABUF_T_O_2)) {
			tag = ap->MB30p->mb.m_sequ_num;
			buf = ap->tm_bufs[tag];
			ap->enabuf_state = 0;
			if (cmd_failure == TRUE) {
			    ap->enabuf_retries++;
			    ap->tm_bufs[tag] = NULL;
			    if (buf != NULL)
				(void) hsc_tgt_tcw_dealloc(ap, buf, FALSE);
			}
			else {
			    ap->enabuf_retries = 0;
			    /* see if mb30 preceeds mb31 */
			    if (buf != NULL) {
				buf->owner_flag |= TM_ENABLED;
				ap->num_enabled++;
				ap->head_free_mapped
				    = ap->head_free_mapped->next;
				if (ap->head_free_mapped == NULL)
				    ap->tail_free_mapped = NULL;
			    }
			}
			if (ap->tgt_dma_err)
			    hsc_tgt_DMA_err(ap);
			else
			    hsc_start_bufs(ap);
		    }
		    else
			if ((ap->enaid_state == WAIT_FOR_ENA_ID) ||
			    (ap->enaid_state == WAIT_FOR_ENAID_T_O_2)) {
			    id = ap->MB30p->mb.m_sequ_num;
			    ap->enaid_state = 0;
			    if (cmd_failure == TRUE) {
				ap->enaid_retries++;
			    }
			    else {
				ap->enaid_retries = 0;
				/* enable_id done -- reset id */
				ap->waiting_enaids &= ~(1 << id);
				ap->dev[id + IMDEVICES].stopped = FALSE;
			    }
			    (void) hsc_start_ids(ap);
			}
			else
			    if ((ap->enabuf_state == WAIT_FOR_DIS_BUF) ||
			      (ap->enabuf_state == WAIT_FOR_DISBUF_T_O_2)) {
				tag = ap->MB30p->mb.m_sequ_num;
				buf = ap->tm_bufs[tag];
				ap->enabuf_state = 0;
				if (cmd_failure == TRUE) {
				    ap->disbuf_retries++;
				}
				else {
				    ap->disbuf_retries = 0;

				    /* assumption: mb31 intr comes before
				       mb30 intr.  adapter guarantees mb31
				       buffer intr (if one occurs at all)
				       will preceed the mb30 intr. therefore,
				       if buf not NULL, we can free buffer,
				       as it was not used. */
				    if (buf != NULL) {	/* mb30 intr preceeds
							   mb31 */
					/* free tag */
					ap->tm_bufs[tag] = NULL;
					(void) hsc_tgt_tcw_dealloc(ap, buf,
								   FALSE);
					(void) hsc_free_a_buf(buf, FALSE);
					ap->num_enabled--;
				    }
				}
				hsc_tgt_DMA_err(ap);
			    }
	    }	/* end else (retrying == TRUE) */
	}
	else {	/* tm_device not using */
	    if (ap->MB30_in_use == PROC_USING) {
/************************************************************************/
/*	handle process level MB30 command completion here (good/bad)    */
/************************************************************************/
		w_stop(&ap->wdog.dog);
		new_error_state = 0;	/* initial flag value */
		/* always store the status--ioctls need to see this later */
		ap->mb30_resid = ap->MB30p->mb.m_resid;
		ap->mb30_rc = ap->MB30p->mb.m_adapter_rc;
		ap->mb30_extra_stat = ap->MB30p->mb.m_extra_stat;
		ap->mb30_byte30 = ap->MB30p->mb.m_scsi_stat;
		ap->mb30_byte31 = ap->MB30p->mb.m_resvd;
		if (ap->proc_results == 0) {
		    switch (ap->mb30_rc) {
		      case COMPLETE_NO_ERRORS:
/************************************************************************/
/*	handle MB30 good completion for proc level cmd here             */
/************************************************************************/
			ap->proc_results = GOOD_COMPLETION;
			break;
/************************************************************************/
/*	handle MB30 error during process level command below            */
/************************************************************************/
		      case MB_PARAMETER_ERROR:

			/* fall thru to logic which follows.. */

		      case MB_PARITY_ERROR:
			if (ap->MB30_retries >= MAX_MB30_RETRIES) {
			    /* retries exceeded on this MB30 command */
			    if (ap->mb30_rc == MB_PARITY_ERROR)
				hsc_logerr(ap, ERRID_SCSI_ERR1, ap->MB30p,
					   PIO_WR_DATA_ERR, 24, 0);
			    else
				hsc_logerr(ap, ERRID_SCSI_ERR5, ap->MB30p,
					   0, 25, 0);

			    ap->proc_results = FATAL_ERROR;
			}
			else {
			    /* retry the MB30 command */
			    if (ap->mb30_rc == MB_PARITY_ERROR)
				hsc_logerr(ap, ERRID_SCSI_ERR2, ap->MB30p,
					   PIO_WR_DATA_ERR, 26, 0);
			    else
				hsc_logerr(ap, ERRID_SCSI_ERR6, ap->MB30p,
					   0, 27, 0);
			    new_error_state = ap->proc_waiting;
			    ap->MB30_retries++;	/* inc retry count */

			    /* the restart value in watchdog is already set */
			    w_start(&ap->wdog.dog);	/* start timer */

			    /* zero-out the status area before sending */
			    bzero(&ap->MB30p->mb.m_resid, MB_STAT_SIZE);

			    /* send MB30 command to adapter */
			    iop.opt = WRITE_MB30;
			    iop.errtype = 0;
			    if ((pio_assist(&iop, hsc_pio_function,
					    hsc_pio_recov) == EIO) &&
				(iop.errtype != PIO_PERM_IOCC_ERR)) {
				/* handle unrecovered error sending mbox 30 */
				/* when not an iocc internal error.         */
				/* report fatal error to calling process.   */
				w_stop(&ap->wdog.dog);	/* stop timer */
				new_error_state = 0;
				ap->proc_results = FATAL_ERROR;
			    }
			}
			break;

		      case BAD_FUSE:
			/* this indicates the adapter is dead. logging occurs
			   via the MB31 indication.  simply return fatal
			   status here */
			ap->proc_results = FATAL_ERROR;
			break;

		      case OTHER_CMD_RUNNING:
			/* some mb30 command already running, preventing
			   execution */
			hsc_logerr(ap, ERRID_SCSI_ERR1, ap->MB30p, 0, 28, 0);
			/* try to recov. this so next mb30 cmd might succeed */
			(void) hsc_scsi_reset(ap);
			ap->proc_results = FATAL_ERROR;
			break;

		      case DIAGNOSE_PAUSED:
			/* fatal card ipl or running diag detected err */
			hsc_logerr(ap, ERRID_SCSI_ERR1, ap->MB30p, 0, 29, 0);
			ap->proc_results = FATAL_ERROR;
			break;

		      default:
			/* catch all other adapter status here for the
			   various  process level MB30 commands. Further
			   processing may be required when we return to the
			   process level. (error log on proc level, if
			   required.)  */
			ap->proc_results = SEE_RC_STAT;
			break;

		    }	/* endswitch */
		}	/* end of if(ap->proc_results == 0) */
		if (new_error_state != 0) {
		    ap->proc_waiting = new_error_state;
		}
		else {
		    ap->MB30_retries = 0;	/* reset retry counter  */

                    /* if we are using MB30 to download microcode to the   */
                    /* adapter then no other MB30 commands can be issued   */
                    /* while the download is occuring.  This is            */
                    /* guaranteed by not releasing MB30 from the           */
                    /* interrupt handler when it is in use by this command */ 
                    /* MB30 will be released by the download ioctl when it */
                    /* has completed the download.                         */

                    if (ap->proc_waiting != WAIT_FOR_DNLD_CMD) {
		        ap->proc_waiting = 0;	/* reset waiting state  */
		        ap->MB30_in_use = -1;	/* free MB30            */
                    }
		    e_wakeup(&ap->event);	/* return to proc level */
		}
	    }
	    else {



		if (ap->MB30_in_use == ADAP_USING) {
/************************************************************************/
/*	handle good/bad adapter error recovery cmds here                */
/************************************************************************/
		    w_stop(&ap->wdog.dog);
		    restart_recovery = FALSE;	/* default--good intrpt */
		    new_error_state = 0;	/* default--completed */

		    switch (ap->MB30p->mb.m_adapter_rc) {

		      case COMPLETE_NO_ERRORS:

			switch (ap->restart_state) {
			  case WAIT_FOR_RESTART:
/************************************************************************/
/*	handle good RESTART completion interrupt here                   */
/************************************************************************/
			    /* restart complete */
			    break;
			  default:
			    /* error: should have been waiting for the above */
			    hsc_logerr(ap, ERRID_SCSI_ERR6, ap->MB30p, 0,
				       30, 0);
			    break;
			}	/* endswitch */
			break;

/************************************************************************/
/*	handle MB30 error during adapter error recovery below           */
/************************************************************************/
		      case MB_PARAMETER_ERROR:

			/* fall thru to logic below.. */

		      case MB_PARITY_ERROR:
			if (ap->MB30_retries >= MAX_MB30_RETRIES) {
			    /* retries exceeded on this MB30 command */
			    if (ap->MB30p->mb.m_adapter_rc == MB_PARITY_ERROR)
				hsc_logerr(ap, ERRID_SCSI_ERR1, ap->MB30p,
					   PIO_WR_DATA_ERR, 31, 0);
			    else
				hsc_logerr(ap, ERRID_SCSI_ERR5, ap->MB30p, 0,
					   32, 0);

			    restart_recovery = TRUE;
			}
			else {
			    /* retry the MB30 command */
			    if (ap->MB30p->mb.m_adapter_rc == MB_PARITY_ERROR)
				hsc_logerr(ap, ERRID_SCSI_ERR2, ap->MB30p,
					   PIO_WR_DATA_ERR, 33, 0);
			    else
				hsc_logerr(ap, ERRID_SCSI_ERR6, ap->MB30p,
					   0, 34, 0);
			    new_error_state = ap->restart_state;
			    ap->MB30_retries++;	/* inc retry count */

			    /* the restart value in watchdog is already set */
			    w_start(&ap->wdog.dog);	/* start timer */

			    /* zero-out the status area before sending */
			    bzero(&ap->MB30p->mb.m_resid, MB_STAT_SIZE);

			    /* send MB30 command to adapter */
			    iop.opt = WRITE_MB30;
			    iop.errtype = 0;
			    if ((pio_assist(&iop, hsc_pio_function,
					    hsc_pio_recov) == EIO) &&
				(iop.errtype != PIO_PERM_IOCC_ERR)) {
				/* handle unrecovered error sending mbox 30 */
				/* when not an internal iocc error.         */
				/* treat as fatal error to restart cmd.     */
				w_stop(&ap->wdog.dog);	/* stop timer */
				new_error_state = 0;
				restart_recovery = TRUE;
			    }
			}
			break;

		      case BAD_FUSE:
			/* this means the adapter is effectively dead.  Any */
			/* clean-up is handled via the MB31 indication. */
			/* Simply continue here.                        */
			break;

		      default:
			/* handle other conditions returned by adapter */
			if (ap->restart_state == WAIT_FOR_RESTART) {
			    /* unknown adapter error for restart */
			    hsc_logerr(ap, ERRID_SCSI_ERR4, ap->MB30p,
				       UNKNOWN_CARD_ERR, 35, 0);
			    restart_recovery = TRUE;
			}
			    else {
				/* unknown driver sfw error */
				hsc_logerr(ap, ERRID_SCSI_ERR6, ap->MB30p, 0,
					   36, 0);
			    }

			break;

		    }	/* endswitch */

		    /* handle adapter error state changes */
		    if (new_error_state != 0) {
			/* go to new state */
			ap->restart_state = new_error_state;
		    }
		    else {
			ap->MB30_retries = 0;	/* reset retry count */
			ap->MB30_in_use = -1;	/* free MB30 */
			/* the test below assumes only RESTART is used in
			   adap err recov */
			/* need another restart? */
			if (ap->restart_again == TRUE) {
			    ap->restart_state = WAIT_TO_SEND_RESTART;
			    ap->restart_again = FALSE;	/* reset again flag */
			    /* flag waiting cmd */
			    ap->waiting_for_mb30 = TRUE;
			}
			else {
			    /* reset waiting state */
			    ap->restart_state = 0;
			    /* finish restart error recovery here !! */
			    /* the following kicks-off cmds which may have */
			    /* tried to execute while waiting for the  */
			    /* restart to complete.                    */
			    if (ap->restart_index_validity) {
				hsc_start(ap, ap->restart_index);
				/* reset which device to start */
				ap->restart_index = 0;
				/* say we saw index */
				ap->restart_index_validity = FALSE;
			    }
			    if (restart_recovery) {
				if (ap->restart_retries <
				    MAX_RESTART_RETRIES) {
				    ap->restart_retries++;
				    /* attempt scsi reset to recover.  Note:
				       if scsi reset fails due to pio error,
				       we may get sequence errors on all
				       future commands--no alternative to
				       this...  */
				    if (hsc_scsi_reset(ap) == EIO)
					ap->restart_retries = 0;
				}
				else {
				    ap->restart_retries = 0;
				}
			    }
			    else	/* no restart recovery required */
				ap->restart_retries = 0;
			}
		    }

		}
		else {



		    if (ap->MB30_in_use != -1) {
/************************************************************************/
/*	handle good/bad device error recovery cmd here                  */
/************************************************************************/
			/* N.B. MB30_in_use is the device index */
			dev_index = (int) ap->MB30_in_use;
			w_stop(&ap->wdog.dog);
			new_error_state = 0;
			if (ap->dev[dev_index].state == 0) {
			    /* unknown driver sfw error */
			    hsc_logerr(ap, ERRID_SCSI_ERR6, ap->MB30p,
				       0, 37, 0);
			    /* do not fail queue, as this device did not */
			    /* send the INIT LUN.                        */
			}
			else {
			    /* if head element exists, set up default status */
			    scp = ap->dev[dev_index].head_act;
			    if ((scp != NULL) &&
			        (ap->dev[dev_index].state != WAIT_FOR_RESUME)&&
                                (ap->dev[dev_index].cc_error_state != 
                                 CC_OCCURRED) &&
				(scp->bufstruct.b_error == 0)) {
				/* this sets up status for a halt */
				scp->status_validity = 0;
				scp->general_card_status = 0;
				scp->bufstruct.b_error = ENXIO;
				scp->bufstruct.b_resid = scp->bufstruct.
				    b_bcount;
			    }
			    switch (ap->MB30p->mb.m_adapter_rc) {

			      case COMPLETE_NO_ERRORS:

/************************************************************************/
/*	handle good INIT LUN completion interrupt here                  */
/************************************************************************/
				/* get here for normal completion of an INIT */
				/* LUN.  status for failed sc_buf previously */
				/* set.  the following is needed if doing a  */
				/* HALT, and handles the case where the mb30 */
				/* intrpt preceeds the terminated mailbox's  */
				/* interrupt.                                */
				if ((scp != NULL) && 
			        (ap->dev[dev_index].state != WAIT_FOR_RESUME)&&
                                (ap->dev[dev_index].cc_error_state != 
                                 CC_OCCURRED)) { 
				    (void) hsc_dma_cleanup(ap, scp, TRUE);
				}
                                /* note : it is necessary to clear the state */
                                /* before calling hsc_fail_cmd because in the*/
                                /* case that this is a resume init lun       */
                                /* completion, hsc_fail_cmd needs to initiate*/
                                /* the next command by calling hsc_start and */
                                /* state must be zero for the queue to resume*/
                                if (ap->dev[dev_index].state==WAIT_FOR_RESUME){ 
			            ap->dev[dev_index].state = 0;
                                }
				hsc_fail_cmd(ap, dev_index);
				break;

/************************************************************************/
/*	handle MB30 error during device error recovery here             */
/************************************************************************/
			      case MB_PARAMETER_ERROR:

				/* fall thru to logic below.. */

			      case MB_PARITY_ERROR:
				if (ap->MB30_retries >= MAX_MB30_RETRIES) {
				    /* retries exceeded on this MB30 command */
				    if (ap->MB30p->mb.m_adapter_rc ==
					MB_PARITY_ERROR)
					hsc_logerr(ap, ERRID_SCSI_ERR1,
						 ap->MB30p, PIO_WR_DATA_ERR,
						   38, 0);
				    else
					hsc_logerr(ap, ERRID_SCSI_ERR5,
						   ap->MB30p, 0, 39, 0);

				    /* attempt to recover this error.  note
				       that a potential exists for a panic
				       due to a dirty buffer if the reset
				       fails, but, there is no alternative. */
				    (void) hsc_scsi_reset(ap);
				    if (scp != NULL) {
					(void) hsc_dma_cleanup(ap, scp, TRUE);
				    }
				    hsc_fail_cmd(ap, dev_index);
				}
				else {
				    /* retry the MB30 command */
				    if (ap->MB30p->mb.m_adapter_rc ==
					MB_PARITY_ERROR)
					hsc_logerr(ap, ERRID_SCSI_ERR2,
						 ap->MB30p, PIO_WR_DATA_ERR,
						   40, 0);
				    else
					hsc_logerr(ap, ERRID_SCSI_ERR6,
						   ap->MB30p, 0, 41, 0);

				    ap->MB30_retries++;	/* inc retry count */
			            if ((ap->dev[dev_index].state != 
                                         WAIT_FOR_RESUME))
				        new_error_state = WAIT_FOR_INIT_LUN;
                                    else new_error_state = WAIT_FOR_RESUME;

				    /* start timer--the restart value in
				       watchdog is already set */
				    w_start(&ap->wdog.dog);

				    /* zero-out the status area before
				       sending */
				    bzero(&ap->MB30p->mb.m_resid,
					  MB_STAT_SIZE);

				    /* send MB30 command to adapter */
				    iop.opt = WRITE_MB30;
				    iop.errtype = 0;
				    if ((pio_assist(&iop, hsc_pio_function,
						    hsc_pio_recov) == EIO) &&
					(iop.errtype != PIO_PERM_IOCC_ERR)) {
					/* handle unrecovered error sending  */
					/* mbox 30 when not an internal iocc */
					/* error.  treat as fatal error--    */
					/* attempt to cleanup the device     */
					/* queue anyway.  note that a        */
					/* potential exists for a panic due  */
					/* to a dirty buffer if the scsi     */
					/* reset fails, but there is no      */
					/* alternative...                    */
					/* stop timer */
					w_stop(&ap->wdog.dog);
					new_error_state = 0;
					(void) hsc_scsi_reset(ap);
					if (scp != NULL) {
					    (void) hsc_dma_cleanup(ap, scp,
								   TRUE);
					}
					hsc_fail_cmd(ap, dev_index);
				    }
				}
				break;

			      case BAD_FUSE:
				/* this means the adapter is effectively
				   dead. Any clean-up is handled via the MB31
				   intrpt indication, simply continue here. */

				/* fall thru to logic below.. */

			      case SCSI_BUS_RESET:
				/* if scsi reset, consider init lun to have  */
				/* completed okay.                           */

				/* fall thru to logic below.. */

			      case PREVIOUS_ERROR:
				/* if previous error, a restart is being
				   waited for due to scsi reset, so consider
				   this init lun complete. */
				if (scp != NULL) {
				    (void) hsc_dma_cleanup(ap, scp, TRUE);
				}
				hsc_fail_cmd(ap, dev_index);
				break;

			      case COMPLETE_WITH_ERRORS:
				if (ap->MB30p->mb.m_extra_stat ==
				    UNEXPECTED_BUS_FREE) {
				    /* init LUN is complete */
				    if (scp != NULL) {
					(void) hsc_dma_cleanup(ap, scp, TRUE);
				    }
				    hsc_fail_cmd(ap, dev_index);
				}
				else {
				    /* unknown adapter error for init lun */
				    hsc_logerr(ap, ERRID_SCSI_ERR4, ap->MB30p,
					       UNKNOWN_CARD_ERR, 42, 0);
				    /* attempt to recover this error.  note */
				    /* that a potentail exists for a panic  */
				    /* due to a dirty buffer if the scsi    */
				    /* reset fails, but there is no other   */
				    /* alternative...                       */
				    (void) hsc_scsi_reset(ap);
				    if (scp != NULL) {
					(void) hsc_dma_cleanup(ap, scp, TRUE);
				    }
				    hsc_fail_cmd(ap, dev_index);
				}
				break;

			      default:
				/* unknown adapter error for init lun cmd */
				hsc_logerr(ap, ERRID_SCSI_ERR4, ap->MB30p,
					   UNKNOWN_CARD_ERR, 43, 0);
				/* attempt to recover this error.  note that
				   a potential exists for a panic due to a
				   dirty buffer if the scsi reset fails, but
				   there is no alternative... */
				(void) hsc_scsi_reset(ap);
				if (scp != NULL) {
				    (void) hsc_dma_cleanup(ap, scp, TRUE);
				}
				hsc_fail_cmd(ap, dev_index);
				break;

			    }	/* endswitch */

			}

			if (new_error_state != 0) {
			    ap->dev[dev_index].state = new_error_state;
			}
			else {
			    ap->MB30_retries = 0;	/* reset retry count */
			    ap->MB30_in_use = -1;	/* free MB30 */
			    /* reset device state */
                            if ((ap->dev[dev_index].state==WAIT_FOR_INIT_LUN) ||
                               (ap->dev[dev_index].state == WAIT_FOR_RESUME)) {
			        ap->dev[dev_index].state = 0;
                            }
			}



		    }
		    else {	/* no-one claims to be using MB30 */
			/* unknown sfw error */
			ap->MB30_retries = 0;
			hsc_logerr(ap, ERRID_SCSI_ERR6, ap->MB30p, 0, 44, 0);
		    }
		}
	    }
	}
    }	/* flag not set, skip interrupt handling */

process_waiting_cmds:

/************************************************************************/
/*	handle requests waiting for MB30 here                           */
/************************************************************************/
    if (ap->MB30_in_use == -1) {
	/* MB30 free, see if anyone waiting on it */
	if (ap->waiting_for_mb30 == TRUE) {
	    anyone_waiting = FALSE;	/* flag used to reset global flag */

	    /* check if target mode devices need mb30 */

	    /* Note that the following checks for disid_state, enabuf_state */
	    /* and enaid_state must be in the specified order.  In  */
	    /* particular, disable id must be before both enable buf and */
	    /* enable id.  Enable buf is before enable id because it will */
	    /* happen more frequently and thus requires shorter path length */

	    if (ap->disid_state == WAIT_TO_DIS_ID) {
		ap->MB30_in_use = TM_DEVICE_USING;
		ap->disid_state = WAIT_FOR_DIS_ID;
		if (ap->t_dis_ids & (1 << ap->disable_id)) {
		    /* build temporary disable id */
		    hsc_build_mb30(ap, ENA_ID, 3, ap->disable_id, 0);
		}
		else {
		    /* build permanent disable id */
		    hsc_build_mb30(ap, ENA_ID, 1, ap->disable_id, 0);
		}
		ap->wdog.dog.restart = ENAID_CMD_T_O;
		w_start(&ap->wdog.dog);

		/* send MB30 command to adapter */
		iop.opt = WRITE_MB30;
		if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
		    /* handle unrecovered error sending mbox 30.  */
		    /* ignore--allow to timeout                   */
		    ;
		}
		anyone_waiting = TRUE;	/* set local flag */
	    }	/* end wait_to_dis_id */
	    else {
		if (ap->enabuf_state == WAIT_TO_ENA_BUF) {
		    tag = hsc_get_tag(ap);
		    if (tag >= 0) {	/* if tag free */
			rc = hsc_tgt_tcw_alloc(ap, ap->ena_buf);
			if (rc) {	/* if got tcws */
			    ap->tm_bufs[tag] = ap->ena_buf;	/* get tag */
			    ap->ena_buf->tag = tag;	/* always save tag */
			    ap->enabuf_state = WAIT_FOR_ENA_BUF;
			    ap->MB30_in_use = TM_DEVICE_USING;
			    burst = (DMA_BURST << 1) & 0x0e;
			    /* build enable buffer command */
			    hsc_build_mb30(ap, ENA_BUF, (int) burst,
					   ap->ena_buf->tag, 0);
			    ap->wdog.dog.restart = ENABUF_CMD_T_O;
			    w_start(&ap->wdog.dog);

			    /* send MB30 command to adapter */
			    iop.opt = WRITE_MB30;
			    if (pio_assist(&iop, hsc_pio_function,
					   hsc_pio_recov) == EIO) {
				/* handle unrecovered error sending mbox 30. */
				/* ignore--allow to timeout                  */
				;
			    }
			    anyone_waiting = TRUE;	/* set local flag */
			}
			else {	/* did not get tcws--should never happen */
			    ap->enabuf_state = 0;	/* cancel enable */
			    /* wait for start_bufs to kick off next try */
			}
		    }
		    else {	/* did not get tag--should never happen */
			ap->enabuf_state = 0;	/* cancel enable */
			/* wait for start_bufs to kick off next try */
		    }
		}	/* end wait_to_ena_buf */
		/* must now re-check for use of MB30, since error path in
		   enable buf handling may not have used MB30 */
		if (!anyone_waiting) {
		    if (ap->enaid_state == WAIT_TO_ENA_ID) {
			ap->enaid_state = WAIT_FOR_ENA_ID;
			ap->MB30_in_use = TM_DEVICE_USING;
			/* build enable id command */
			hsc_build_mb30(ap, ENA_ID, 0, ap->ena_id, 0);
			ap->wdog.dog.restart = ENAID_CMD_T_O;
			w_start(&ap->wdog.dog);

			/* send MB30 command to adapter */
			iop.opt = WRITE_MB30;
			if (pio_assist(&iop, hsc_pio_function,
				       hsc_pio_recov) == EIO) {
			    /* handle unrecovered error sending mbox 30. */
			    /* ignore--allow to timeout                  */
			    ;
			}
			anyone_waiting = TRUE;	/* set local flag */

		    }	/* end wait_to_ena_id */
		    else
			if (ap->enabuf_state == WAIT_TO_DIS_BUF) {
			    ap->enabuf_state = WAIT_FOR_DIS_BUF;
			    ap->MB30_in_use = TM_DEVICE_USING;
			    /* build disable buffer command */
			    hsc_build_mb30(ap, ENA_BUF, 1,
					   ap->disable_tag, 0);
			    ap->wdog.dog.restart = ENABUF_CMD_T_O;
			    w_start(&ap->wdog.dog);

			    /* send MB30 command to adapter */
			    iop.opt = WRITE_MB30;
			    if (pio_assist(&iop, hsc_pio_function,
					   hsc_pio_recov) == EIO) {
				/* handle unrecovered error sending mbox 30. */
				/* ignore--allow to timeout                  */
				;
			    }
			    anyone_waiting = TRUE;	/* set local flag */

			}	/* end wait_to_dis_buf */

		}	/* end !anyone_waiting */
	    }	/* end else !wait_to_dis_id */

	    if (!anyone_waiting) {
		/* start looking for who needs mb30: begin with adap err
		   recov */
		if (ap->restart_state == WAIT_TO_SEND_RESTART) {
		    ap->MB30_in_use = ADAP_USING;
		    ap->restart_state = WAIT_FOR_RESTART;
		    /* build MB30 restart cmd */
		    hsc_build_mb30(ap, RESTART, 0, 0, 0);
		    ap->wdog.dog.restart = ADAP_CMD_T_O;
		    w_start(&ap->wdog.dog);	/* start adap watchdog */

		    /* send MB30 command to adapter */
		    iop.opt = WRITE_MB30;
		    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov)
			== EIO) {
			/* handle unrecovered error sending mbox 30.  */
			/* ignore--allow to timeout                   */
			;
		    }
		    anyone_waiting = TRUE;	/* set local flag */
		}
		    else {
			/* begin round-robin search through devices */
			for (i = 0, t_index = ap->last_dev_index;
			     i < IMDEVICES;
			     i++, t_index = (t_index + 1) % IMDEVICES)
			{	/* begin for */
			    if (ap->dev[t_index].state ==
				WAIT_TO_SEND_INIT_LUN) {
				ap->MB30_in_use = t_index;
				ap->dev[t_index].state = WAIT_FOR_INIT_LUN;

				/* build MB30 INIT LUN cmd */
				hsc_build_mb30(ap, INITIALIZE, 0,
					       (0x01 << SID(t_index)),
					       LUN(t_index));
				ap->wdog.dog.restart = INIT_CMD_T_O;
				w_start(&ap->wdog.dog);	/* start timer */

				/* send MB30 command to adapter */
				iop.opt = WRITE_MB30;
				if (pio_assist(&iop, hsc_pio_function,
					       hsc_pio_recov) == EIO) {
				    /* handle unrecovered error sending mbox
				       30 ignore--allow to timeout */
				    ;
				}
				anyone_waiting = TRUE;
				break;	/* leave loop */
			    }	/* not waiting for device err recovery,
				   continue */
			    if (ap->dev[t_index].state ==
				WAIT_TO_SEND_RESUME) {
				ap->MB30_in_use = t_index;
				ap->dev[t_index].state = WAIT_FOR_RESUME;

				/* build MB30 INIT LUN cmd */
				hsc_build_mb30(ap, INITIALIZE, 2,
					       (0x01 << SID(t_index)),
					       LUN(t_index));
				ap->wdog.dog.restart = INIT_CMD_T_O;
				w_start(&ap->wdog.dog);	/* start timer */

				/* send MB30 command to adapter */
				iop.opt = WRITE_MB30;
				if (pio_assist(&iop, hsc_pio_function,
					       hsc_pio_recov) == EIO) {
				    /* handle unrecovered error sending mbox
				       30 ignore--allow to timeout */
				    ;
				}
				anyone_waiting = TRUE;
				break;	/* leave loop */
			    }	/* not waiting for device err recovery,
				   continue */

			}	/* endfor */
			ap->last_dev_index = (ap->last_dev_index + 1) %
			    IMDEVICES;

			/* now, check to see if any process level command */
			/* which is waiting can be satisfied              */
			/* (if mb30 is still free)                        */
			/* N.B.: Check for waiting process level cmds MUST */
			/* be last check, since WAIT_TO_ENA_BUF may */
			/* end up freeing mb30, and since this is   */
			/* the lowest performance priority.         */
			/* Order of checks within switch do not make */
			/* a difference, since performance not an   */
			/* issue, and only one can ever be waiting. */
			if (ap->MB30_in_use == -1) {

			    switch (ap->proc_waiting) {

			      case WAIT_TO_SEND_INIT_LUN:
				ap->MB30_in_use = PROC_USING;
				ap->proc_waiting = WAIT_FOR_INIT_LUN;
				/* build MB30 init lun cmd, using stored scsi
				   id/lun */
				ap->proc_results = 0;
				hsc_build_mb30(ap, INITIALIZE, 0,
					       (0x01 << ap->p_scsi_id),
					       ap->p_lun_id);
				ap->wdog.dog.restart = INIT_CMD_T_O;
				w_start(&ap->wdog.dog);	/* start timer */

				/* send MB30 command to adapter */
				iop.opt = WRITE_MB30;
				if (pio_assist(&iop, hsc_pio_function,
					       hsc_pio_recov) == EIO) {
				    /* handle unrecovered error sending mbox
				       30 ignore--allow to timeout */
				    ;
				}
				anyone_waiting = TRUE;
				break;

			      case WAIT_TO_SEND_INIT_DEV:
				ap->MB30_in_use = PROC_USING;
				ap->proc_waiting = WAIT_FOR_INIT_DEV;
				/* build MB30 init dev cmd, using stored scsi
				   id */
				ap->proc_results = 0;
				hsc_build_mb30(ap, INITIALIZE, 1,
					       (0x01 << ap->p_scsi_id), 0);
				ap->wdog.dog.restart = INIT_CMD_T_O;
				w_start(&ap->wdog.dog);	/* start timer */

				/* send MB30 command to adapter */
				iop.opt = WRITE_MB30;
				if (pio_assist(&iop, hsc_pio_function,
					       hsc_pio_recov) == EIO) {
				    /* handle unrecovered error sending  */
				    /* mbox 30. ignore--allow to timeout */
				    ;
				}
				anyone_waiting = TRUE;
				break;

			      case WAIT_TO_SEND_SET_ID:
				/* flag proc level cmd using */
				ap->MB30_in_use = PROC_USING;
				/* set state flag for cmd */
				ap->proc_waiting = WAIT_FOR_SET_ID;
				/* load MB30 with "Set SCSI ID cmd" */
				ap->proc_results = 0;
				hsc_build_mb30(ap, SET_SCSI_ID, 0, 0, 0);
				/* make timeout long enough to cover still
				   running diag. */
				ap->wdog.dog.restart = IPL_MAX_SECS;
				w_start(&ap->wdog.dog);

				/* send MB30 command to adapter */
				iop.opt = WRITE_MB30;
				iop.errtype = 0;
				if (pio_assist(&iop, hsc_pio_function,
					       hsc_pio_recov) == EIO) {
				    /* handle unrecovered error sending  */
				    /* mbox 30. ignore--allow to timeout */
				    ;
				}

				anyone_waiting = TRUE;
				break;

			      case WAIT_TO_SEND_RESTART:
				/* flag proc level cmd using */
				ap->MB30_in_use = PROC_USING;
				/* set command state */
				ap->proc_waiting = WAIT_FOR_RESTART;
				/* load MB30 with "RESTART cmd" (head of free
				   list is mbox #) */
				ap->proc_results = 0;
				hsc_build_mb30(ap, RESTART, 0, 0, 0);
				ap->wdog.dog.restart = ADAP_CMD_T_O;
				w_start(&ap->wdog.dog);

				/* send MB30 command to adapter */
				iop.opt = WRITE_MB30;
				iop.errtype = 0;
				if (pio_assist(&iop, hsc_pio_function,
					       hsc_pio_recov) == EIO) {
				    /* handle unrecovered error sending  */
				    /* mbox 30. ignore--allow to timeout */
				    ;
				}

				anyone_waiting = TRUE;
				break;
			      case WAIT_TO_ENA_BUF:
				if (ap->p_buf->option == ENABLE) {
				    tag = hsc_get_tag(ap);
				    if (tag >= 0) {	/* tag is free */
					rc = hsc_tgt_tcw_alloc(ap, ap->p_buf);
					if (rc) {	/* got tcws */
					    ap->tm_bufs[tag] = ap->p_buf;
					    ap->p_buf->tag = tag;
					    ap->MB30_in_use = PROC_USING;
					    ap->proc_waiting =
						WAIT_FOR_ENA_BUF;
					    ap->proc_results = 0;
					    burst = (DMA_BURST << 1) & 0x0e;
					    /* build enable buffer command */
					    hsc_build_mb30(ap, ENA_BUF,
							   (int) burst,
							   tag, 0);
					    ap->wdog.dog.restart =
						ENABUF_CMD_T_O;
					    w_start(&ap->wdog.dog);

					    /* send MB30 command to adapter */
					    iop.opt = WRITE_MB30;
					    if (pio_assist(&iop,
							   hsc_pio_function,
							   hsc_pio_recov)
						== EIO) {
						/* handle unrecovered error
						   sending mbox 30.  ignore--
						   allow to timeout */
						;
					    }
					    anyone_waiting = TRUE;
					}
					else {	/* did not get tcws */
					    /* cancel the enable buf */
					    ap->proc_waiting = 0;
					}
				    }
				    else {	/* did not get tag */
					/* cancel the enable buf */
					ap->proc_waiting = 0;
				    }
				}
				else {	/* disable buf option */
				    tag = ap->p_buf->tag;
				    ap->MB30_in_use = PROC_USING;
				    ap->proc_waiting = WAIT_FOR_ENA_BUF;
				    ap->proc_results = 0;
				    /* build disable buffer command */
				    hsc_build_mb30(ap, ENA_BUF, 1, tag, 0);
				    ap->wdog.dog.restart = ENABUF_CMD_T_O;
				    w_start(&ap->wdog.dog);

				    /* send MB30 command to adapter */
				    iop.opt = WRITE_MB30;
				    if (pio_assist(&iop, hsc_pio_function,
						   hsc_pio_recov) == EIO) {
					/* handle unrecovered error sending
					   mbox 30.  ignore-- allow to
					   timeout */
					;
				    }
				    anyone_waiting = TRUE;
				}
				break;

			      case WAIT_TO_ENA_ID:
				ap->MB30_in_use = PROC_USING;
				ap->proc_waiting = WAIT_FOR_ENA_ID;
				ap->proc_results = 0;
				if (ap->p_id_option == ENABLE) {
				    /* build enable id cmd */
				    hsc_build_mb30(ap, ENA_ID, 0, ap->p_id, 0);
				}
				else {
				    if (ap->p_id_option == T_DISABLE) {
					/* build temporary disable cmd */
					hsc_build_mb30(ap, ENA_ID, 3,
						       ap->p_id, 0);
				    }
				    else {
					/* build permanent disable cmd */
					hsc_build_mb30(ap, ENA_ID, 1,
						       ap->p_id, 0);
				    }
				}
				ap->wdog.dog.restart = ENAID_CMD_T_O;
				w_start(&ap->wdog.dog);

				/* send MB30 command to adapter */
				iop.opt = WRITE_MB30;
				if (pio_assist(&iop, hsc_pio_function,
					       hsc_pio_recov) == EIO) {
				    /* handle unrecovered error sending  */
				    /* mbox 30. ignore--allow to timeout */
				    ;
				}
				anyone_waiting = TRUE;
				break;

  			      case WAIT_TO_SEND_DNLD_VERSION :
                                /* flag proc level cmd using */
                                ap->MB30_in_use = PROC_USING;
                                /* set waiting state */
                                ap->proc_waiting = 
                                                 WAIT_FOR_DNLD_VERSION;
  				ap->proc_results = 0;
                                hsc_build_mb30(ap, DOWNLOAD, 1, 0, 0);
                                ap->wdog.dog.restart = ADAP_CMD_T_O;
                                w_start(&ap->wdog.dog);
  				/* send MB30 command to adapter */
  				iop.opt = WRITE_MB30;
  				if (pio_assist(&iop, hsc_pio_function,
  					hsc_pio_recov) == EIO) {
  				    /* handle unrecovered error sending  */
  				    /* mbox 30. ignore--allow to timeout */
  					  ;
  				}
  				anyone_waiting = TRUE;
                                break;
                              
  			      case WAIT_TO_SEND_DNLD_CMD :
                                /* flag proc level cmd using */
                                ap->MB30_in_use = PROC_USING;
                                /* set waiting state */
                                ap->proc_waiting = 
                                                 WAIT_FOR_DNLD_CMD;
  				ap->proc_results = 0;
                                hsc_build_mb30(ap, DOWNLOAD, 0, 
                                      (ap->download_mc_len / MC_BLK_SIZE),
                                        0);
                                ap->wdog.dog.restart = ADAP_CMD_T_O;
                                w_start(&ap->wdog.dog);
  				/* send MB30 command to adapter */
  				iop.opt = WRITE_MB30;
  				if (pio_assist(&iop, hsc_pio_function,
  					hsc_pio_recov) == EIO) {
  				    /* handle unrecovered error sending  */
  				    /* mbox 30. ignore--allow to timeout */
  					  ;
  				}
  				anyone_waiting = TRUE;
                                break;
                              
                                break;

			      default:
				/* proc not waiting for mb30 */
				break;
			    }	/* end switch */
			}	/* mb30 in use, must continue to wait for it */
		    }

	    }	/* end if !anyone_waiting */
	    /* update global flag */
	    ap->waiting_for_mb30 = anyone_waiting;
	}	/* no-one waiting on mb30, done */

    }	/* MB30 still in use, done */
#ifdef HSC_NEVER	
    hsc_internal_trace(ap, TRC_MB30INTR, 'EXIT', 0, 0, 0, 0);
#endif HSC_NEVER
}  /* end hsc_MB30_handler */
