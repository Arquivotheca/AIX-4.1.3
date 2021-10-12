static char sccsid[] = "@(#)66	1.4.2.11  src/bos/kernext/scsi/hscintr.c, sysxscsi, bos411, 9435A411a 8/26/94 14:04:02";
/*
 * COMPONENT_NAME: (SYSXSCSI) IBM SCSI Adapter Driver
 *
 * FUNCTIONS:	hsc_intr
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
/* NAME:        hscintr.c                                               */
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
/* NAME:        hsc_intr                                                */
/*                                                                      */
/* FUNCTION:    Adapter Driver Interrupt Handler                        */
/*                                                                      */
/*      This routine processes adapter interrupt conditions.            */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine runs on the interrupt level, therefore, it must    */
/*      only perform operations on pinned data.                         */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      sc_buf  - input/output request struct used between the adapter  */
/*                driver and the calling SCSI device driver             */
/*      mbstruct - driver structure which holds information related to  */
/*                 a particular mailbox and hence a command             */
/*      intr    - kernel interrupt information structure                */
/*                                                                      */
/* INPUTS:                                                              */
/*      handler - pointer to the intrpt handler structure               */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      INTR_FAIL - interrupt was not processed                         */
/*      INTR_SUCC - interrupt was processed                             */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      w_stop          w_start                                         */
/*      iodone          e_wakeup                                        */
/*      i_reset         pio_assist                                      */
/*      bzero                                                           */
/*                                                                      */
/************************************************************************/
int
hsc_intr(
	 struct intr * handler)
{
    struct adapter_def *ap;
    struct mbstruct *mbp;
    struct sc_buf *scp, *tptr;
    struct io_parms iop;
    int     dev_index;
    int     ret_code, rc, sys_dma_rc;
    int     i, j, mask_val, n;
    int     dma_err_detected, fail_queue;
    int     good_completion;
    int     no_data_copy;
    int     old_pri;
    volatile caddr_t mem_addr;
    ulong   save_isr;
    uint    mb30_intr, local_resid;
    uchar   local_scsi_validity;
    int     tm_dev;
    uchar   tag, user_id, temp_id;
    struct tm_mb31_info *tmb31p;
    struct tm_send_info *sp;
    struct dev_info *d, *owner, *user, *puser;
    struct b_link *p, *buf, *tm_buf_p, *end_buf_p;

    ret_code = INTR_SUCC;	/* setup default return code */

    if (opened_adapters == 0) {	/* if no opened adapters */
	DDHKWD1(HKWD_DD_SCSIDD, DD_ENTRY_INTR, 0, 0);
	ret_code = INTR_FAIL;	/* not my interrupt */
	DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_INTR, 0, 0);
	goto end;
    }

    /* get pointer to this adapter's info struct */
    ap = (struct adapter_def *) handler;

    if (!ap->opened) {	/* if this adapter not opened */
	DDHKWD1(HKWD_DD_SCSIDD, DD_ENTRY_INTR, 0, 0);
	ret_code = INTR_FAIL;	/* cannot handle the intrpt */
	DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_INTR, 0, 0);
	goto end;
    }
    old_pri = disable_lock(ap->ddi.int_prior, &(hsc_mp_lock));

    DDHKWD1(HKWD_DD_SCSIDD, DD_ENTRY_INTR, 0, ap->devno);
    iop.ap = ap;
    iop.opt = READ_ISR;
    iop.data = 0;
    iop.iocc_err = FALSE;

    mem_addr = BUSMEM_ATT(ap->ddi.bus_id, ap->ddi.base_addr);
    rc = BUS_GETLRX(mem_addr+ISR, &save_isr);
    BUSMEM_DET(mem_addr);
    if (rc != 0) {

        /* read the card interrupt status register */
        if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
            /* for unrecovered error, force on the ISR error bit to ensure */
	    /* that mb31 status is checked and cleared.  All other pending */
	    /* commands will experience a timeout due to loss of intrpt.   */
	    iop.data = 0x00000080;	/* set error bit */
        }
        /* get here for no error or recovered error */
        /* if an IOCC internal error, interrupt status may have been lost.   */
        /* the recovery routine will force the error bit on so mb31 status    */
        /* will be checked.                                                   */

        save_isr = WORD_REVERSE(iop.data);	/* swap data from io struct */
    }
#ifdef HSC_NEVER
    hsc_internal_trace(ap, TRC_ISR, save_isr, 0, 0);
#endif HSC_NEVER
    if (save_isr == 0) {	/* if no interrupt on adapter */
	ret_code = INTR_FAIL;
	goto exit;
	/* other adapters' intrpts will get */
    }	/* handled on succeeding calls      */

    if (save_isr & 0x40000000) {	/* see if MB30 is interrupting */
	mb30_intr = TRUE;	/* flag MB30 interrupting */
	save_isr &= ~(0x40000000);	/* clear MB30 bit in status */
    }
    else {
	mb30_intr = FALSE;	/* flag MB30 not interrupting */
    }

    if ((save_isr & 0x80000000) == 0) {	/* is error bit off ?? */
/************************************************************************/
/*	handle all good completion interrupts here                      */
/************************************************************************/

	sys_dma_rc = 0;	/* default, accumulates d_complete results */
	mask_val = 0x01;	/* init mask to check for first mbox */

	/* determine where to start searching in the isr */
	if (save_isr & 0xffff) {
	    if (save_isr & 0x00ff)
		n = 0;	/* start search at least sig. byte */
	    else
		n = 8;	/* start search at next least sig. byte */
	}
	else {
	    if (save_isr & 0xff0000)
		n = 16;	/* start search at next most sig. byte */
	    else
		n = 24;	/* start search at most sig. byte */
	}

	while (save_isr != 0) {	/* not all interrupting mboxes handled? */

	    if (save_isr & (mask_val << n)) {	/* is this mbox intrpting? */
		/* handle mailbox interrupt */

		/* generate mbox pointer from "n" value */
		mbp = (struct mbstruct *) (&ap->MB[n].id0);

		/* make sure mailbox is not inactive (which would indicate */
		/* that the mailbox was not in use)                        */
		if (mbp->cmd_state != INACTIVE) {

                    /* if mbox waiting for int, this intrpt causes mb to be
                       freed */
                    if (mbp->cmd_state == WAIT_FOR_INTRPT) {
                        w_stop(&ap->wdog3.dog);
                        hsc_STA_dealloc(ap, mbp);
                        hsc_TCW_dealloc(ap, mbp);
                        hsc_MB_dealloc(ap, mbp);
			/* finish loop logic. */
			/* this marks mbox interrupt bit as handled */
			save_isr &= ~(mask_val << n);
			n++;	/* inc flag shift count */
			continue;	/* skip to next iteration of while
					   loop */
                    }

		    /* get index into device table for this device */
		    dev_index = INDEX(mbp->mb.m_xfer_id & 0x07,
				      (mbp->mb.m_scsi_cmd.lun) >> 5);


		    /* change mailbox state to indicate interrupt arrived */
		    if (mbp->cmd_state == ISACTIVE) {
			mbp->cmd_state = INTERRUPT_RECVD;
                	w_stop(&ap->dev[dev_index].wdog->dog);
        		if ( ap->dev[dev_index].num_act_cmds > 1 ) {
                		ap->dev[dev_index].wdog->dog.restart =
                                   ap->dev[dev_index].head_act->timeout_value;
                		w_start(&ap->dev[dev_index].wdog->dog);
        		}
		    }

		    scp = mbp->sc_buf_ptr;	/* get sc_buf ptr from
						   mailbox */
		    DDHKWD2(HKWD_DD_SCSIDD, DD_SC_INTR, 0, ap->devno, scp);

		    if (mbp->cmd_state == WAIT_FOR_T_O_2) {
			/* this indicates we are receiving the intrpt after */
			/* having already timed-out on this command.  Here, */
			/* do final cmd cleanup, fail the queue (keeping    */
			/* timeout error status), and loop to next intrpt.  */
                        w_stop(&ap->dev[dev_index].wdog->dog);
			mbp->cmd_state = INTERRUPT_RECVD;
			sys_dma_rc |= hsc_dma_cleanup(ap, scp, TRUE);
			hsc_fail_cmd(ap, dev_index);

			/* finish loop logic. */
			/* this marks mbox interrupt bit as handled */
			save_isr &= ~(mask_val << n);
			n++;	/* inc flag shift count */
			continue;	/* skip to next iteration of while
					   loop */
		    }

		    /* if sc_buf not the head element and not queuing    */
                    /*    to the device                                  */
                    /* disable this logic; it is unnecessary and does not*/
                    /* work.  It can lead to false error indications when*/
                    /* queueing at the adapter (not device) for a given  */
                    /* id/lun because the adapter can post multiple      */
                    /* mailbox completions for one device in a single    */
                    /* interrupt and there is no way to know which MB    */
                    /* completed first !!                                */
		    if ((scp != ap->dev[dev_index].head_act) &&
                            (FALSE) &&
                            (!(ap->dev[dev_index].dev_queuing))) {
			/* handle error in which dev act queue did not     */
			/* start with the sc_buf which corresponded to     */
			/* this mailbox (this is theoretically impossible  */
			/* on a good completion).  log the error, abort,   */
			/* then fail this dev queue, if it is active.      */
			hsc_logerr(ap, ERRID_SCSI_ERR6, mbp, 0, 9, 0);
			/* set status of failing element before abort */
			scp->status_validity = SC_ADAPTER_ERROR;
			scp->general_card_status = SC_ADAPTER_SFW_FAILURE;
			scp->bufstruct.b_resid = scp->bufstruct.b_bcount;
			scp->bufstruct.b_error = EIO;
			/* release TCWs associated with failing element */
			(void) hsc_dma_cleanup(ap, scp, TRUE);
			tptr = ap->dev[dev_index].head_act;
			if (tptr != NULL) {	/* if active sc_bufs */
			    /* initiate an "internal" abort, fail queue  */
			    /* set default status of head element before */
			    /* abort.                                    */
			    tptr->status_validity = SC_ADAPTER_ERROR;
			    tptr->general_card_status =
				SC_ADAPTER_SFW_FAILURE;
			    tptr->bufstruct.b_resid = scp->bufstruct.b_bcount;
			    tptr->bufstruct.b_error = EIO;
			    if (ap->dev[dev_index].state == 0) {
				ap->dev[dev_index].state =
				    WAIT_TO_SEND_INIT_LUN;
				mbp = (struct mbstruct *) tptr->
				    bufstruct.b_work;
				/* if MB30 is free, abort the command. */
				if (ap->MB30_in_use == -1) {
				    ap->MB30_in_use = dev_index;
				    ap->dev[dev_index].state =
					WAIT_FOR_INIT_LUN;

				    /* build MB30 Init LUN cmd to abort this
				       LUN */
				    hsc_build_mb30(ap, INITIALIZE, 0,
						   (0x01 << SID(dev_index)),
						   LUN(dev_index));
				    ap->wdog.dog.restart = INIT_CMD_T_O;
				    /* start timer */
				    w_start(&ap->wdog.dog);

				    /* send MB30 command to adapter */
				    iop.opt = WRITE_MB30;
				    iop.errtype = 0;
				    if ((pio_assist(&iop, hsc_pio_function,
						    hsc_pio_recov) == EIO) &&
					(iop.errtype != PIO_PERM_IOCC_ERR)) {
					/* handle unrecovered error sending */
					/* mbox 30 when other than an iocc  */
					/* internal error.  instead of      */
					/* aborting, this lets queue play   */
					/* out, if possible, down to the    */
					/* error sc_buf.                    */
					/* stop timer */
					w_stop(&ap->wdog.dog);
					/* free mb30 */
					ap->MB30_in_use = -1;
					/* reset state */
					ap->dev[dev_index].state = 0;
				        hsc_fail_cmd(ap, dev_index);
				    }

				}
				else {	/* mb30 is in use, wait for it */
				    ap->waiting_for_mb30 = TRUE;
				}
			    }
			    /* device already doing an init lun */
			}
			/* continue, no active elements to fail */
		    }
		    else {	/* cont with good completion */
			/* N.B.  scp is now pointing at this mbox's sc_buf */
			/* accumulate return status of d_complete calls here */
			sys_dma_rc |= hsc_dma_cleanup(ap, scp, FALSE);
			if (sys_dma_rc) {	/* if any return code so far */
			    /* handle error */
			    hsc_dev_DMA_err(ap, dev_index, scp);
			}

			else {	/* cont with good completion */
/************************************************************************/
/*	handle normal mailbox completion interrupt here                 */
/************************************************************************/
			    /* note that status_validity, b_error, and
			       b_resid fields are already zeroed        */
			    /* deallocate resources for the request */
			    if ((scp->bufstruct.b_bcount > ST_SIZE) ||
	    			(scp->resvd1)) {
				hsc_TCW_dealloc(ap, mbp);
			    }
			    else {	/* either STA used, or no data */
				hsc_STA_dealloc(ap, mbp);
			    }
			    hsc_MB_dealloc(ap, mbp);
			    /* decrement command counters */
			    ap->commands_outstanding--;
			    ap->dev[dev_index].num_act_cmds--;
			    /* dequeue the sc_buf */
                            hsc_deq_active(ap,scp,dev_index);

			    /* internal trace point */
#ifdef HSC_TRACE
			    hsc_internal_trace(ap, TRC_DONE, scp, mbp,
					       dev_index);
#endif HSC_TRACE

			    iodone((struct buf *) scp);

			    /* see if stop is sleeping on pending commands */
			    if (ap->dev[dev_index].qstate == STOP_PENDING)
				e_wakeup(&ap->dev[dev_index].stop_event);

			    /* see if close is sleeping on outstanding cmds
			       or mboxes */
			    if (ap->close_state == CLOSE_PENDING)
				e_wakeup(&ap->cl_event);

			    if (ap->dev[dev_index].head_act == NULL) {
				/* normal completion, no other active cmds. */
				/* update tail */
				if (ap->dev[dev_index].queuing_stopped ) {
				     ap->dev[dev_index].queuing_stopped = FALSE;
				     ap->dev[dev_index].queue_depth = Q_ENABLED;
				}
				hsc_start(ap, dev_index);
			    }
			    else {	/* active queue not empty */
				mbp = (struct mbstruct *) ap->dev[dev_index].
						head_act->bufstruct.b_work;
				mbp->preempt--;
				if( mbp->preempt == 0 ) {
				    ap->dev[dev_index].queue_depth = Q_DISABLED;
				    ap->dev[dev_index].queuing_stopped = TRUE;
				}
				/* check status of next command */
				if (ap->dev[dev_index].head_act->
				    bufstruct.b_error != 0) {
				    /* this indicates a cmd which failed    */
				    /* previously due to mbox error.  Now,  */
				    /* need to fail the queue for this one. */
				    hsc_fail_cmd(ap, dev_index);
				}
				else {	/* no error on next sc_buf */
				    /* normal completion, queued cmds, kick */
				    /* off next request, if necessary       */
				    hsc_start(ap, dev_index);
				}
			    }
			}
		    }
		}
		/* mbox inactive -- ignore, must be the interrupt from a    */
		/* previously handled timeout error; queue already cleared  */

		save_isr &= ~(mask_val << n);	/* indicate this mbox was
						   handled */
	    }
	    /* this mailbox not interrupting */

	    n++;	/* inc flag shift count */

	}	/* endwhile */

	if (sys_dma_rc) {	/* if error in some transfer */
	    /* for all other cmds with a data transfer in progress          */
	    /* go thru each dev head_active and if data xfer at head,       */
	    /* initiate an abort to that LUN.  Indicate SC_HOST_IO_BUS_ERR  */
	    /* on head sc_buf, and set resid to transfer length.            */
	    /* Indicate ENXIO on following sc_bufs.                         */
	    hsc_DMA_err(ap, 0);	/* handle system DMA error */
	}



	if (mb30_intr) {
/************************************************************************/
/*	handle good MB30 completion interrupt here                      */
/************************************************************************/
	    hsc_MB30_handler(ap, TRUE);
	}
	/* no MB30 interrupt */

	/* end of good interrupt completion handling logic */


    }
    else {
/************************************************************************/
/*	handle any error interrupt indications here                     */
/************************************************************************/
	/* get MB31 status area */
	iop.mbp = ap->MB31p;
	iop.opt = RD_ALL_MBOX_STAT;
	if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	    /* cannot get mb31 status, this may or may not be fatal, so   */
	    /* just try to go on here (following clears mb31 status area) */
	    bzero(&ap->MB31p->mb, MB_SIZE);
	}

/*
	DEBUG_1(3,"hsc_intr: ap = 0x%x\n",ap)
	hsc_dbg_dump("MB31 stat",(char *)&ap->MB31p->mb,32);
*/

	tmb31p = (struct tm_mb31_info *) & ap->MB31p->mb;

	/* trace this in the internal trace buffer */
	if ((tmb31p->send_info[0].flag & TM_VALID_DATA) ||
	    (ap->MB31p->mb.m_adapter_rc != 0))
#ifdef HSC_TRACE
	    hsc_internal_trace(ap, TRC_MB31, 0, 0, 0);
#endif HSC_TRACE

	/* mb31 status can hold information upto 3 SEND cmds	 */
	sys_dma_rc = 0;
	dma_err_detected = FALSE;
	tm_buf_p = NULL;
	end_buf_p = NULL;
	for (i = 0; i < 3; i++) {
	    sp = &tmb31p->send_info[i];
	    if (sp->flag & TM_VALID_DATA) {	/* target interrupt */
		user_id = sp->id & 0x7;	/* get id from mb31 status */
		tag = sp->tag & 0xf;	/* get tag value from mb31 */
		p = ap->tm_bufs[tag];	/* get buffer address	 */
		if (p == NULL) {
		    hsc_logerr(ap, ERRID_SCSI_ERR4, ap->MB31p,
			       UNKNOWN_CARD_ERR, 73, 0);
		    continue;	/* next i */
		}
		owner = &ap->dev[p->owner_id + IMDEVICES];
		user = &ap->dev[user_id + IMDEVICES];
		/* see if mb30 preceeds mb31 */
		if (p->owner_flag & TM_ENABLED) {
		    p->owner_flag &= ~(TM_ENABLED);
		    ap->num_enabled--;
		}
		else {	/* mb31 preceeds mb30 */
		    ap->head_free_mapped = ap->head_free_mapped->next;
		    if (ap->head_free_mapped == NULL)
			ap->tail_free_mapped = NULL;
		}
		ap->tm_bufs[tag] = NULL;	/* free the tag */
		p->data_len = WORD_REVERSE(sp->data_len) & 0x00ffffff;
		p->data_addr = p->buf_addr;	/* data offset */
		p->next = NULL;
		p->tag = tag;	/* save tag */
		p->user_id = user_id;
		p->tm_correlator = user->tm_correlator;
		sys_dma_rc |= hsc_tgt_tcw_dealloc(ap, p, FALSE);

		/* look for any errors	 */
		if ((sp->flag & TM_EXCEPTION) &&
		    (sp->tm_status == SYSTEM_BUS_DMA_ERROR)) {
		    dma_err_detected = TRUE;
		}

		/* check for general dma error */
		if (((!dma_err_detected) && (sys_dma_rc))
		    || (ap->tgt_dma_err) || (sys_dma_rc & SYS_ERR)) {
		    p->data_len = 0;
		    p->user_flag |= TM_ERROR;
		    if (!user->previous_err)
			user->previous_err = TM_DMA_ERR;
		    hsc_DMA_err(ap, 0);
		}
		if (user->opened) {	/* user is opened */
		    p->user_flag |= TM_HASDATA;

		    /* save statistics on data processed */
		    user->num_bufs_recvd++;
		    user->num_bytes_recvd += (uint) p->data_len;

		    /* catches error being reported when not */
		    /* end_of_send_command.		     */
		    if (sp->flag & TM_EXCEPTION) {
			if (!(sp->flag & TM_END_OF_SEND_CMD)) {
			    p->user_flag |= TM_ERROR;
			    /* unknown ucode error */
			    hsc_logerr(ap, ERRID_SCSI_ERR4, ap->MB31p,
				       UNKNOWN_CARD_ERR, 74, 0);
			    if (!user->previous_err)
				user->previous_err = TM_UCODE_ERR;
			}
			if (!(p->user_flag & TM_ERROR) &&
			    (sp->flag & TM_END_OF_SEND_CMD)) {
			    p->user_flag |= TM_ERROR;

			    /* Normal card detected error. decode error */
			    /* status and log as appropriate.	    */
			    switch (sp->tm_status) {
			      case NO_RESPONSE:
				p->status_validity = SC_ADAPTER_ERROR;
				p->general_card_status
				    = SC_NO_DEVICE_RESPONSE;
				break;
			      case BAD_FUSE:
				p->status_validity = SC_ADAPTER_ERROR;
				p->general_card_status
				    = SC_FUSE_OR_TERMINAL_PWR;
				break;
			      case SYSTEM_BUS_DMA_ERROR:
				hsc_logerr(ap, ERRID_SCSI_ERR2, ap->MB31p,
					   DMA_ERROR, 75, 0);
				p->status_validity = SC_ADAPTER_ERROR;
				p->general_card_status
				    = SC_HOST_IO_BUS_ERR;
				break;
			      case SCSI_BUS_RESET:
				p->status_validity = SC_ADAPTER_ERROR;
				p->general_card_status
				    = SC_SCSI_BUS_RESET;
				break;
			      case UNEXPECTED_BUS_FREE:
			      case ABORTED_SCSI_CMD:
				/* if dev_abort not set or this is
				   not first buffer received, handle
				   abort status */
				if (!(user->dev_abort &&
				     (user->num_bufs_recvd == 1))) {
				    hsc_logerr(ap, ERRID_SCSI_ERR10, ap->MB31p,
					       0, 76, 0);
				    p->status_validity = SC_ADAPTER_ERROR;
				    p->general_card_status
				        = SC_SCSI_BUS_FAULT;
				}
				else {
				    /* NOTE: this means this device cmd
				       is seeing abort status on first buf
				       after having closed while temp disabl.
				       We are ignoring this status as it is
				       extraneous.  Note we are lying about
				       this being the end of send command
				       so that the buffer will be freed */
				    user->dev_abort = FALSE;	/* reset */
				    sp->flag &= ~TM_END_OF_SEND_CMD;
				}
				break;
			      case ADAP_RECOVERD_ERR:
				hsc_logerr(ap, ERRID_SCSI_ERR2, ap->MB31p,
					   0, 77, 0);
				p->status_validity = SC_ADAPTER_ERROR;
				p->general_card_status
				    = SC_ADAPTER_HDW_FAILURE;
				break;
			      default:
				hsc_logerr(ap, ERRID_SCSI_ERR4, ap->MB31p,
					   UNKNOWN_CARD_ERR, 78, 0);
				p->status_validity = SC_ADAPTER_ERROR;
				p->general_card_status
				    = SC_ADAPTER_SFW_FAILURE;
				p->data_len = 0;
				break;
			    }	/* end switch */
			}
		    }
		    if (user->previous_err)
			p->user_flag |= TM_ERROR;

		    if ((p->user_flag & TM_ERROR) &&
			!(sp->flag & TM_END_OF_SEND_CMD)) {
			/* must be same err(card did not see) prior */
			/* to last SEND cmd buffer. ignore data and */
			/* report error on last send command buffer */

			(void) hsc_free_a_buf(p, FALSE);
			hsc_start_bufs(ap);	/* try to re-enable */
			continue;	/* skip to next interrupt */
		    }
		    else {

			/* either no error, regardless if end of Send */
			/* or, this is end of Send, regardless if there */
			/* was an error or not */

			if (!(sp->flag & TM_END_OF_SEND_CMD)) {
			    p->user_flag |= TM_MORE_DATA;
			}
			else {
			    /* end of send command */
			    if (user->previous_err) {

				switch (user->previous_err) {
				  case TM_UCODE_ERR:
				    /* this was logged previously */
				    p->status_validity = SC_ADAPTER_ERROR;
				    p->general_card_status =
					SC_ADAPTER_SFW_FAILURE;
				    p->data_len = 0;	/* throw out data */
				    break;
				  case TM_DMA_ERR:
				    /* this was logged previously */
				    p->status_validity = SC_ADAPTER_ERROR;
				    p->general_card_status =
					SC_HOST_IO_BUS_ERR;
				    p->data_len = 0;	/* throw out data */
				    break;
				  default:
				    /* unknown driver logic error */
				    hsc_logerr(ap, ERRID_SCSI_ERR6, ap->MB31p,
					       0, 79, 0);
				    break;
				}	/* end switch */
				user->previous_err = 0;	/* reset error state */
			    }
			}
		    }

		    /* if we get here, then this request is a valid buffer */
		    /* to send back to the head, thus it goes on the rdbuf */
		    /* list and the "num_bufs_qued" variable is incremented */
		    user->num_bufs_qued++;

		    /* see if the buffer has tcws allocated so we know */
		    /* which rdbuf list to put it on.                  */
		    if (p->tcws_alloced) {
		        /* add the buffer to the mapped_rdbuf list at head */
		        p->forw = ap->mapped_rdbufs;
		        p->back = NULL;
		        if (ap->mapped_rdbufs)
			    ap->mapped_rdbufs->back = p;
		        ap->mapped_rdbufs = p;
		    }
		    else {
		        /* add the buffer to the unmapped_rdbuf list at head */
		        p->forw = ap->unmapped_rdbufs;
		        p->back = NULL;
		        if (ap->unmapped_rdbufs)
			    ap->unmapped_rdbufs->back = p;
		        ap->unmapped_rdbufs = p;
		    }

		    /* tm_buf_p and end_buf_p are head and tail pointers to */
		    /* a list which contains the accumulated buffers (for   */
		    /* the same user id) to be sent back.  If a buffer comes */
		    /* in for another id, the existing list is sent to the */
		    /* head, and a new list started.  */
		    if (tm_buf_p == NULL)
			tm_buf_p = p;
		    else {
			if (tm_buf_p->user_id == p->user_id)
			    end_buf_p->next = p;
			else {

			    /* clear out the current list for current user. */
			    /* "puser" is set up to point at current list */
			    /* user id's device info. */
			    puser = &ap->dev[tm_buf_p->user_id + IMDEVICES];
			    temp_id = tm_buf_p->user_id;
			    (puser->recv_func) (tm_buf_p);

			    /* if buffers used up by this user are equal to */
			    /* or greater than the num of bufs allocated to */
			    /* the user.                                  */
			    if (puser->num_bufs_qued >= puser->num_bufs) {
				/* see if a disable id is really needed */
				hsc_need_disid(ap, puser, temp_id);
			    }
			    /* start new accumulated list with this buffer */
			    tm_buf_p = p;
			}
		    }
		    end_buf_p = p;
		}
		else {	/* user not opened */
		    /* since user not opened, throw away this buffer and */
		    /* its data, allowing the buffer to be re-enabled    */
		    (void) hsc_free_a_buf(p, FALSE);
		}
		/* initiate re-enable of all possible buffers here */
		hsc_start_bufs(ap);
	    }
            else break;
	}	/* end for */

	/* if accumulated buffers exist, send to the head now that we have */
	/* finished processing target mode buffers for this mb31 interrupt. */
	if (tm_buf_p != NULL) {
	    temp_id = tm_buf_p->user_id;
	    (user->recv_func) (tm_buf_p);

	    /* disable id if num_bufs_qued >= num bufs allocated */
	    if (user->num_bufs_qued >= user->num_bufs) {
		/* see if a disable id is really needed */
		hsc_need_disid(ap, user, temp_id);
	    }
	}

	if (ap->MB31p->mb.m_adapter_rc != 0) {

/************************************************************************/
/*	handle MB31 error interrupt indications here                    */
/************************************************************************/
	    /* store mb31 error data for either error log or calling progrm */
	    ap->mb31_resid = ap->MB31p->mb.m_resid;
	    ap->mb31_rc = ap->MB31p->mb.m_adapter_rc;
	    ap->mb31_extra_stat = ap->MB31p->mb.m_extra_stat;
	    ap->mb31_byte30 = ap->MB31p->mb.m_scsi_stat;
	    ap->mb31_byte31 = ap->MB31p->mb.m_resvd;

	    /* handle the various MB31 return codes here */
	    switch (ap->MB31p->mb.m_adapter_rc) {

	      case BAD_FUSE:
		/* this means the card detected term pwr loss.   */
		/* no further cmds will be accepted by the card. */
		/* fall through to logic below...                */
	      case ADAP_FATAL:
		/* this means a fatal hardware error was detected */
		/* by the card.  no further cmds will be accepted */
	        if ((ap->MB31p->mb.m_adapter_rc == BAD_FUSE) && 
                    (!(ap->ddi.has_fuse))) 
		    hsc_logerr(ap, ERRID_SCSI_ERR2, NULL, 0, 101, 0);
		else hsc_logerr(ap, ERRID_SCSI_ERR1, NULL, 0, 10, 0);
		ap->adapter_check = ADAPTER_DEAD;	/* kill future i/o */
		for (i = 0; i < MAX_TAG; i++) {
		    buf = ap->tm_bufs[i];
		    if (buf == NULL)
			continue;
		    (void) hsc_free_a_buf(buf, FALSE);
		    ap->tm_bufs[i] = NULL;
		}
		(void) hsc_async_notify(ap, ALL_DEVICES, SC_FATAL_HDW_ERR);
		/* continue handling interrupts below */
		break;
	      case SCSI_BUS_RESET:
                ap->reset_pending = FALSE;
                if (!ap->epow_reset) {
		    hsc_logerr(ap, ERRID_SCSI_ERR10, NULL, 0, 11, 0);
                }
                else { 
                    ap->epow_reset = FALSE;
                }
		/* wait to pick-up starting mbox number for restart  */
		/* until the restart command is actually sent.       */
		/* below assumes INIT DEV not used in error handling */
		/* command delay after reset logic follows: */
		ap->wdog2.dog.restart = ap->ddi.cmd_delay;
		w_start(&ap->wdog2.dog);	/* start delay timer */
		ap->cdar_scsi_ids = 0xff;	/* all SCSI IDs (may) be
						   affected       */
		if (ap->restart_state == WAIT_FOR_RESTART) {
		    ap->restart_again = TRUE;
		}
		else {	/* assume only other states are: not waiting, and
			   wait to restart */
		    ap->restart_state = WAIT_TO_SEND_RESTART;
		}
		/* clear any device state waiting to do init lun.    */
		/* do not clear adap err recov waiting for init dev, */
		/* as the reset may have been on internal bus, and   */
		/* init dev may be for a device on the external bus. */
		for (i = 0; i < IMDEVICES; i++) {
		    if (ap->dev[i].state == WAIT_TO_SEND_INIT_LUN) {
			ap->dev[i].state = 0;
                        ap->dev[i].cc_error_state = 0;
			hsc_fail_cmd(ap, i);
		    }
		}	/* endfor */
		/* other cmds (in prog) are handled below */
		(void) hsc_async_notify(ap, ALL_DEVICES, SC_SCSI_RESET_EVENT);
		break;
	      case UNKNOWN_SELECT:
		/* this means a device which the adapter did not have */
		/* command for attempted a valid reselection with the */
		/* adapter.  ignore this error, but log it.           */
		hsc_logerr(ap, ERRID_SCSI_ERR10, NULL, 0, 12, 0);
		/* continue normal operations below */
		break;
	      case COMMAND_PAUSED:
		/* this means the adapter encountered a hardware err  */
		/* during IPL diagnostics                             */
		hsc_logerr(ap, ERRID_SCSI_ERR1, NULL, 0, 13, 0);
		(void) hsc_async_notify(ap, ALL_DEVICES, SC_FATAL_HDW_ERR);
		break;
	      case ADAP_RECOVERD_ERR:
		/* this means an adapter recoverd hardware error occurred */
		/* Log and try to continue operations.			  */
		hsc_logerr(ap, ERRID_SCSI_ERR2, NULL, 0, 71, 0);
		break;
	      default:
		/* invalid mb31 return code */
		hsc_logerr(ap, ERRID_SCSI_ERR4, NULL, UNKNOWN_CARD_ERR, 14,
			   0);
		/* try to continue operations */
		break;

	    }	/* endswitch */

	}	/* end of MB31 handling */

	/* N.B.  Must write 0's to all of MB31 area, regardless of */
	/* previous status contents (i.e. good or bad).            */
	/* write 0's to MB31 (internal copy in driver) */
	ap->MB31p->mb.m_op_code = 0;
	ap->MB31p->mb.m_xfer_id = 0;
	ap->MB31p->mb.m_cmd_len = 0;
	ap->MB31p->mb.m_sequ_num = 0;
	ap->MB31p->mb.m_dma_addr = 0;
	ap->MB31p->mb.m_dma_len = 0;
	ap->MB31p->mb.m_scsi_cmd.scsi_op_code = 0;
	ap->MB31p->mb.m_scsi_cmd.lun = 0;
	for (j = 0; j < 10; j++)
	    ap->MB31p->mb.m_scsi_cmd.scsi_bytes[j] = 0;

	ap->MB31p->mb.m_resid = 0;
	ap->MB31p->mb.m_adapter_rc = 0;
	ap->MB31p->mb.m_extra_stat = 0;
	ap->MB31p->mb.m_scsi_stat = 0;
	ap->MB31p->mb.m_resvd = 0;

	/* send MB31 to adapter */
	/* N.B. this re-enables MB31 for next error intrpt */
	iop.mbp = ap->MB31p;
	iop.opt = WRITE_MB31;
	if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	    /* cannot send mb31 to clear intrpt.  this may or may */
	    /* not be fatal, so again attempt to continue here    */
	    ;
	}

	save_isr &= ~0x80000000;	/* mask out MB31 bit */

/************************************************************************/
/*	handle MB 0-29 interrupts here                                  */
/************************************************************************/
	/* loop thru MB 0 to 29, handling each status as required */
	sys_dma_rc = 0;	/* default, accumulates d_complete results */
	dma_err_detected = FALSE;	/* flag to accumulate adap detected
					   error */
	mask_val = 0x01;	/* init mask to check for first mbox */

	/* determine where to start searching in the isr */
	if (save_isr & 0xffff) {
	    if (save_isr & 0x00ff)
		n = 0;	/* start search at least sig. byte */
	    else
		n = 8;	/* start search at next least sig. byte */
	}
	else {
	    if (save_isr & 0xff0000)
		n = 16;	/* start search at next most sig. byte */
	    else
		n = 24;	/* start search at most sig. byte */
	}

	while (save_isr != 0) {	/* not all interrupting mboxes handled? */

	    if (save_isr & (mask_val << n)) {	/* is this mbox intrpting? */
		/* handle mailbox interrupt here */

		/* generate mbox pointer from "n" value */
		/* n = the mailbox number               */
		mbp = (struct mbstruct *) (&ap->MB[n].id0);

		/* make sure mailbox is not inactive (which would indicate */
		/* the mailbox is not in use currently).                   */
		if (mbp->cmd_state != INACTIVE) {

		    /* if mbox waiting for int, this intrpt causes mb to be
		       freed */
		    if (mbp->cmd_state == WAIT_FOR_INTRPT) {
			w_stop(&ap->wdog3.dog);
			hsc_STA_dealloc(ap, mbp);
			hsc_TCW_dealloc(ap, mbp);
			hsc_MB_dealloc(ap, mbp);
			/* finish loop logic. */
			/* this marks mbox interrupt bit as handled */
			save_isr &= ~(mask_val << n);
			n++;	/* inc flag shift count */
			continue;	/* skip to next iteration of while
					   loop */
		    }
		    /* for other, active mailboxes, handle the interrupt */
		    else {

			/* get this mailbox status area */
			iop.mbp = mbp;
			iop.opt = RD_MBOX_STAT;
			if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov)
			    == EIO) {
			    /* cannot get the mailbox status, ignore the
			       interrupt.  */
			    /* indicate this mbox was handled */
			    save_isr &= ~(mask_val << n);
			    n++;	/* inc flag shift count */
			    continue;	/* skip to loop test portion */
			}

			/* get index into device table for this device  */
			dev_index = INDEX(mbp->mb.m_xfer_id & 0x07,
					  (mbp->mb.m_scsi_cmd.lun) >> 5);

		    	/* change mailbox state to indicate interrupt arrived */
		    	if (mbp->cmd_state == ISACTIVE) {
				mbp->cmd_state = INTERRUPT_RECVD;
                		w_stop(&ap->dev[dev_index].wdog->dog);
        			if ( ap->dev[dev_index].num_act_cmds > 1 ) {
                			ap->dev[dev_index].wdog->dog.restart = 							ap->dev[dev_index].head_act->
								timeout_value;
                			w_start(&ap->dev[dev_index].wdog->dog);
        			}
			}

			scp = mbp->sc_buf_ptr;	/* get sc_buf ptr from
						   mailbox */
			DDHKWD2(HKWD_DD_SCSIDD, DD_SC_INTR, 0, ap->devno,
				scp);

			if (mbp->cmd_state == WAIT_FOR_T_O_2) {
			    /* this indicates we are receiving the intrpt   */
			    /* after having already timed-out on this cmd.  */
			    /* Here, do final cmd cleanup, fail the queue   */
			    /* (retaining timeout error status), and loop   */
			    /* to handle the next interrupt, if any.        */
			    w_stop(&ap->dev[dev_index].wdog->dog);
			    mbp->cmd_state = INTERRUPT_RECVD;
			    sys_dma_rc |= hsc_dma_cleanup(ap, scp, TRUE);
			    hsc_fail_cmd(ap, dev_index);

			    /* finish loop logic */
			    /* this mbox was handled */
			    save_isr &= ~(mask_val << n);
			    n++;	/* inc flag shift count */
			    continue;	/* skip to next iteration of while
					   loop */
			}

			fail_queue = TRUE;	/* default to needing queue
						   cleared */
			good_completion = FALSE;	/* default to bad
							   completion       */
			no_data_copy = FALSE;	/* default to do the dma copy
						   for sta */

			/* set up corrected residual count */
			mbp->mb.m_resid = WORD_REVERSE(mbp->mb.m_resid);
			local_resid = mbp->mb.m_resid & 0x00ffffff;

			/* set up default scsi status validity */
			local_scsi_validity =
			    (mbp->mb.m_resid & 0x80000000) ? SC_SCSI_ERROR : 0;

#ifdef HSC_NEVER
			hsc_internal_trace(ap, TRC_XXX, n, mbp, dev_index);
#endif HSC_NEVER
			/* handle the various MB return codes here */
			switch (mbp->mb.m_adapter_rc) {

			  case COMPLETE_NO_ERRORS:
			    /* this mailbox completed successfully */
			    scp->status_validity = 0;
			    scp->bufstruct.b_resid = 0;
			    scp->bufstruct.b_error = 0;
			    fail_queue = FALSE;
			    good_completion = TRUE;
			    break;

			  case COMPLETE_WITH_ERRORS:
 
                            /* if a check condition occurred and commands */ 
                            /* are being queued to the device then set up */
                            /* cc_error_state so that command tag queuing */
                            /* error recovery can be initiated (queue not */
                            /* cleared for this error)                    */
                            if ((local_scsi_validity == SC_SCSI_ERROR) &&
                             (mbp->mb.m_scsi_stat == SC_CHECK_CONDITION) &&
                             (ap->dev[dev_index].dev_queuing)) {
                                ap->dev[dev_index].cc_error_state = 
                                CC_OCCURRED;

                                /* save a pointer to the sc_buf so that when */
                                /* hsc_fail_cmd is called after completion of*/
                                /* init lun, the sc_buf with the check cond  */
                                /* can be iodoned while not clearing the q   */ 
                                ap->dev[dev_index].cmd_save_ptr_cc = scp;
			         	 
                                /* keep pending commands from becoming       */
                                /* active (queue halted) until error recovery*/
                                /* is completed                              */
			        ap->dev[dev_index].pqstate = PENDING_ERROR;
                            }
                                

			    /* handle various extra status codes here */
			    switch (mbp->mb.m_extra_stat) {

			      case NO_STATUS:
				/* caller logs this */
				/* this result means there is scsi status */
				if (local_scsi_validity == SC_SCSI_ERROR) {
				    scp->status_validity = SC_SCSI_ERROR;
				    scp->scsi_status = mbp->mb.m_scsi_stat;
				}
				else {
				    scp->status_validity = SC_ADAPTER_ERROR;
				    scp->general_card_status =
					SC_ADAPTER_SFW_FAILURE;
				}
				scp->bufstruct.b_resid = local_resid;
				scp->bufstruct.b_error = EIO;
				ap->dev[dev_index].state =
				    WAIT_TO_SEND_INIT_LUN;
				break;

			      case NO_RESPONSE:
				/* caller logs this. */
				/* this means the device could not be */
				/* selected.                          */
				scp->status_validity = SC_ADAPTER_ERROR;
				scp->general_card_status =
				    SC_NO_DEVICE_RESPONSE;
				scp->bufstruct.b_resid = local_resid;
				scp->bufstruct.b_error = EIO;
				ap->dev[dev_index].state =
				    WAIT_TO_SEND_INIT_LUN;
				break;

			      case INCORRECT_NUM_BYTES:
				if (local_resid != 0) {
				    /* this means device asked for too few */
				    /* bytes. caller logs this, if it is a */
				    /* device error.                       */
				    if ((local_scsi_validity == SC_SCSI_ERROR)
					&& (mbp->mb.m_scsi_stat)) {
					scp->status_validity = SC_SCSI_ERROR;
					scp->scsi_status = mbp->mb.m_scsi_stat;
				    }
				    else
					scp->status_validity = 0;
				}
				else {
				    /* this means device read/asked for too */
				    /* much data. caller logs this, as it   */
				    /* is a device error.  log here to get  */
				    /* the detailed adapter status.         */
				    hsc_logerr(ap, ERRID_SCSI_ERR10, mbp,
					       0, 68, 0);
				    scp->status_validity = SC_ADAPTER_ERROR;
				    scp->general_card_status =
					SC_SCSI_BUS_FAULT;
				}

				scp->bufstruct.b_error = EIO;
				scp->bufstruct.b_resid = local_resid;
				ap->dev[dev_index].state =
				    WAIT_TO_SEND_INIT_LUN;
				break;

			      case RESIDUAL_COUNT:
				/* this means adapter read too few bytes.   */
				/* caller logs this, if it is a device err  */
				/* N.B. adapter does not halt queued cmds   */
				/* here.  if this is an error, calling prog */
				/* be aware that all queued commands will be */
				/* executed.                                */
				/* do NOT send an INIT LUN here, do not fail
				   queue. */
				scp->status_validity = 0;
				scp->bufstruct.b_resid = local_resid;
				scp->bufstruct.b_error = 0;
				fail_queue = FALSE;
				good_completion = TRUE;
				break;

			      case SYSTEM_BUS_DMA_ERROR:
				hsc_logerr(ap, ERRID_SCSI_ERR2, mbp,
					   DMA_ERROR, 15, 0);
				scp->status_validity = SC_ADAPTER_ERROR;
				scp->general_card_status = SC_HOST_IO_BUS_ERR;
				scp->bufstruct.b_resid = local_resid;
				scp->bufstruct.b_error = EIO;
				/* for DMA Writes, do NOT send an INIT      */
				/* LUN, as that can violate system data     */
				/* integrity.  Issue a scsi reset to clear  */
				/* the error.  For DMA Reads, INIT LUN is   */
				/* sent since data after the error will be  */
				/* ignored by the card.                     */
				if (!(scp->bufstruct.b_flags & B_READ))
				    /* handle DMA write */
				    /* the following will force a scsi reset */
				    dma_err_detected = TRUE;
				else	/* handle DMA read */
				    ap->dev[dev_index].state =
					WAIT_TO_SEND_INIT_LUN;
				break;

			      case UNEXPECTED_BUS_FREE:

				/* fall thru to logic for next case... */

			      case ABORTED_SCSI_CMD:
				/* this means the adapter could not complete
				   the cmd */
				/* caller logs this.  log here also to get
				   the detailed adapter status. */
				hsc_logerr(ap, ERRID_SCSI_ERR10, mbp, 0, 69, 0);
				scp->status_validity = SC_ADAPTER_ERROR;
				scp->general_card_status = SC_SCSI_BUS_FAULT;
				scp->bufstruct.b_resid = local_resid;
				scp->bufstruct.b_error = EIO;
				ap->dev[dev_index].state =
				    WAIT_TO_SEND_INIT_LUN;
				break;

			      default:	/* unknown adap extra status */
				hsc_logerr(ap, ERRID_SCSI_ERR4, mbp,
					   UNKNOWN_CARD_ERR, 16, 0);
				scp->status_validity = SC_ADAPTER_ERROR;
				scp->general_card_status =
				    SC_ADAPTER_SFW_FAILURE;
				scp->bufstruct.b_resid =
				    scp->bufstruct.b_bcount;
				scp->bufstruct.b_error = EIO;
				if (scp != ap->dev[dev_index].head_act) {
				    /* here, we cannot fail queue, since    */
				    /* there are outstanding requests ahead */
				    /* of this one                          */
				    ap->dev[dev_index].pqstate = PENDING_ERROR;
				    fail_queue = FALSE;
				    good_completion = FALSE;
				}
				else {	/* fail the queue */
				    ap->dev[dev_index].state =
					WAIT_TO_SEND_INIT_LUN;
				}
				no_data_copy = TRUE;	/* don't move data from
							   sta */
				break;

			    }	/* endswitch */
			    break;	/* end, complete with errors */
			  case ADAP_RECOVERD_ERR:
			    /* this means an adapter recovered hdw err */
			    /* occurred.  Log, and perform err recovery */
			    hsc_logerr(ap, ERRID_SCSI_ERR2, mbp, 0, 72, 0);
			    scp->status_validity = SC_ADAPTER_ERROR;
			    scp->general_card_status = SC_ADAPTER_HDW_FAILURE;
			    scp->bufstruct.b_resid = local_resid;
			    scp->bufstruct.b_error = EIO;
			    ap->dev[dev_index].state = WAIT_TO_SEND_INIT_LUN;
			    break;

			  case PREVIOUS_ERROR:
			    if (scp == ap->dev[dev_index].head_act) {
				/* caller logs this */
				/* this error had not been previously seen */
				/* on this particular device.              */
				/* do NOT send INIT LUN, as this must have */
				/* failed due to an err which requires     */
				/* restart.                                */
				/* queue will be failed */
				scp->status_validity = SC_ADAPTER_ERROR;
				scp->general_card_status =
				    SC_ADAPTER_HDW_FAILURE;
				scp->bufstruct.b_resid =
				    scp->bufstruct.b_bcount;
				scp->bufstruct.b_error = EIO;
			    }
			    else {
				/* this error is handled via the mbox at the */
				/* head of this queue.  Ignore this, but    */
				/* mark as error compl to avoid good logic  */
				ap->dev[dev_index].pqstate = PENDING_ERROR;
				fail_queue = FALSE;
				good_completion = FALSE;
			    }
			    no_data_copy = TRUE;	/* don't move data from
							   sta */
			    break;

			  case TERM_BY_INIT_CMD:
			    if (scp == ap->dev[dev_index].head_act) {
				/* nothing here to log */
				/* this occurs when either a HALT is being  */
				/* processed, or, the cmd was killed by an  */
				/* INIT DEV being issued.                   */
				/* do NOT send INIT LUN, queue will be      */
				/* failed.  if b_error already set, this is */
				/* due to a driver "internal" abort         */
				/* sequence--leave status as is.            */
				if (scp->bufstruct.b_error == 0) {
				    scp->status_validity = 0;
				    scp->general_card_status = 0;
				    /* halted */
				    scp->bufstruct.b_error = ENXIO;
				}
				scp->bufstruct.b_resid = local_resid;
			    }
			    else {
				/* this is a cmd which came in after an err  */
				/* was processed for this device.  this err  */
				/* is handled when the MB30 INIT LUN command */
				/* completes, or when the head element sees  */
				/* the init cmd (above).  So, ignore this    */
				/* indication, but flag as error completion  */
				/* to avoid good completion logic.           */
				ap->dev[dev_index].pqstate = PENDING_ERROR;
				fail_queue = FALSE;
				good_completion = FALSE;
			    }
			    break;

			  case BAD_FUSE:
			    /* logged via mb31 indication */
			    /* this means the command was in progress when  */
			    /* terminal power was lost.  The card is dead,  */
			    /* do NOT send INIT LUN, queue will be failed.  */
			    scp->status_validity = SC_ADAPTER_ERROR;
			    scp->general_card_status = SC_FUSE_OR_TERMINAL_PWR;
			    scp->bufstruct.b_resid = local_resid;
			    scp->bufstruct.b_error = EIO;
			    break;

			  case SCSI_BUS_RESET:
			    if (scp->bufstruct.b_error == 0) {
				/* logged via mb31 indication */
				/* this means the command was in progress   */
				/* when a scsi reset occurred.  the         */
				/* indication in mb31 will force a restart. */
				/* do NOT send INIT LUN, queue will be      */
				/* failed.                                  */
				scp->status_validity = SC_ADAPTER_ERROR;
				scp->general_card_status = SC_SCSI_BUS_RESET;
				scp->bufstruct.b_resid = local_resid;
				scp->bufstruct.b_error = EIO;
			    }
			    else {
				/* caller logs this (timeout) */
				/* this indicates a timeout occurred, which */
				/* generated a scsi reset, which is now     */
				/* seen here.  status was set in the watch- */
				/* dog handler.                             */
				/* do NOT send INIT LUN, queue will be      */
				/* failed.                                  */
			    }
			    break;

			  case MB_PARAMETER_ERROR:

			    /* fall thru to logic for next case... */

			  case MB_SEQUENCE_ERROR:
			    /* this means an unknown driver sfw err occurred */
			    hsc_logerr(ap, ERRID_SCSI_ERR6, mbp, 0, 17, 0);
			    /* below assumes INIT DEV not used in error
			       handling */
			    if (ap->restart_state == WAIT_FOR_RESTART) {
				ap->restart_again = TRUE;
			    }
			    else {	/* assume only other states are: not
					   waiting, and wait to restart */
				ap->restart_state = WAIT_TO_SEND_RESTART;
			    }
			    scp->status_validity = SC_ADAPTER_ERROR;
			    scp->general_card_status = SC_ADAPTER_SFW_FAILURE;
			    scp->bufstruct.b_resid = scp->bufstruct.b_bcount;
			    scp->bufstruct.b_error = EIO;
			    if (scp != ap->dev[dev_index].head_act) {
				ap->dev[dev_index].pqstate = PENDING_ERROR;
				fail_queue = FALSE;
			    }	/* head of queue--fail the queue */
			    no_data_copy = TRUE;	/* don't move data from
							   sta */
			    break;

			  case MB_PARITY_ERROR:
			    /* this means a hardware error occurred */
			    hsc_logerr(ap, ERRID_SCSI_ERR2, mbp,
				       PIO_WR_DATA_ERR, 18, 0);
			    /* below assumes INIT DEV not used in error
			       handling */
			    if (ap->restart_state == WAIT_FOR_RESTART) {
				ap->restart_again = TRUE;
			    }
			    else {	/* assume only other states are: not
					   waiting, and wait to restart */
				ap->restart_state = WAIT_TO_SEND_RESTART;
			    }
			    scp->status_validity = SC_ADAPTER_ERROR;
			    scp->general_card_status = SC_ADAPTER_HDW_FAILURE;
			    scp->bufstruct.b_resid = scp->bufstruct.b_bcount;
			    scp->bufstruct.b_error = EIO;
			    if (scp != ap->dev[dev_index].head_act) {
				ap->dev[dev_index].pqstate = PENDING_ERROR;
				fail_queue = FALSE;
			    }	/* head of queue--fail the queue */
			    no_data_copy = TRUE;	/* don't move data from
							   sta */
			    break;

			  default:	/* unknown adapter return code */
			    /* this means an unknown adap sfw err occurred */
			    hsc_logerr(ap, ERRID_SCSI_ERR4, mbp,
				       UNKNOWN_CARD_ERR, 19, 0);
			    /* assume nothing needed to get adapter going */
			    /* here if it hangs, timeouts will occur on   */
			    /* future cmds                                */
			    scp->status_validity = SC_ADAPTER_ERROR;
			    scp->general_card_status = SC_ADAPTER_SFW_FAILURE;
			    scp->bufstruct.b_resid = scp->bufstruct.b_bcount;
			    scp->bufstruct.b_error = EIO;
			    if (scp != ap->dev[dev_index].head_act) {
				ap->dev[dev_index].pqstate = PENDING_ERROR;
				fail_queue = FALSE;
			    }	/* head of queue--fail the queue */
			    no_data_copy = TRUE;	/* don't move data from
							   sta */
			    break;

			}	/* endswitch on adapter return code */

			/* accumulate results of d_completes */
			sys_dma_rc |= hsc_dma_cleanup(ap, scp, no_data_copy);

/************************************************************************/
/*  handle system IOCC detected errors for interrupting commands here   */
/************************************************************************/
			/* if card did not detect an error but there was a */
			/* system detected error, or if system detected a  */
			/* DMA_SYSTEM error, then if a transfer was going, */
			/* fail this queue.                                */
			if (((!dma_err_detected) && (sys_dma_rc)) ||
			    (sys_dma_rc & SYS_ERR)) {
			    if (scp->bufstruct.b_bcount) {
				scp->status_validity = SC_ADAPTER_ERROR;
				scp->general_card_status = SC_HOST_IO_BUS_ERR;
				scp->bufstruct.b_resid =
				    scp->bufstruct.b_bcount;
				scp->bufstruct.b_error = EIO;
				if (ap->dev[dev_index].state == 0)
				    ap->dev[dev_index].state =
					WAIT_TO_SEND_INIT_LUN;
			    }
			}

/************************************************************************/
/*	handle device errors here                                       */
/************************************************************************/
			if (ap->dev[dev_index].state ==
			    WAIT_TO_SEND_INIT_LUN) {
			    /* completion of the init lun will fail queue */
			    /* if MB30 is free */
			    if (ap->MB30_in_use == -1) {
				ap->MB30_in_use = dev_index;
				ap->dev[dev_index].state = WAIT_FOR_INIT_LUN;

				/* build MB30 Init LUN cmd to abort this LUN */
				hsc_build_mb30(ap, INITIALIZE, 0,
					       (0x01 << SID(dev_index)),
					       LUN(dev_index));
				ap->wdog.dog.restart = INIT_CMD_T_O;
				w_start(&ap->wdog.dog);	/* start timer */

				/* send MB30 command to adapter */
				iop.opt = WRITE_MB30;
				if (pio_assist(&iop, hsc_pio_function,
					       hsc_pio_recov) == EIO) {
				    /* handle unrecovered error sending     */
				    /* mbox30.  ignore error here, allowing */
				    /* timeout to hit.                      */
				    ;
				}

			    }
			    else {
				ap->waiting_for_mb30 = TRUE;
			    }
			}
			else {
			    /* fail queue anyway (no INIT LUN) */
			    if (fail_queue == TRUE) {
				/* clear the queue */
				hsc_fail_cmd(ap, dev_index);
			    }
			    else {
				/* good completion ? */
				if (good_completion == TRUE) {
				    /* either no error, or resid > 0 */
				    /* occurred deallocate resources */
				    /* for the request               */
				    hsc_STA_dealloc(ap, mbp);
				    hsc_TCW_dealloc(ap, mbp);
				    hsc_MB_dealloc(ap, mbp);
				    /* decrement command counters */
				    ap->commands_outstanding--;
				    ap->dev[dev_index].num_act_cmds--;
				    /* dequeue the sc_buf */
                                    hsc_deq_active(ap,scp,dev_index);
				    /* internal trace point */
#ifdef HSC_TRACE
				    hsc_internal_trace(ap, TRC_DONE, scp,
						       mbp, dev_index);
#endif HSC_TRACE

				    iodone((struct buf *) scp);

				    /* see if stop is sleeping on pending
				       commands */
				    if (ap->dev[dev_index].qstate ==
					STOP_PENDING)
					e_wakeup(&ap->dev[dev_index].
						 stop_event);

				    /* see if close is sleeping on
				       outstanding cmds or mboxes */
				    if (ap->close_state == CLOSE_PENDING)
					e_wakeup(&ap->cl_event);

				    if (ap->dev[dev_index].head_act == NULL) {
					/* normal completion, no queued cmds */
					/* update tail pointer */
				      if (ap->dev[dev_index].queuing_stopped ) {
					   ap->dev[dev_index].queuing_stopped =
									FALSE;
				           ap->dev[dev_index].queue_depth =
                                                 		     Q_ENABLED;
					}
					hsc_start(ap, dev_index);
				    }
				    else {	/* active queue not empty */
					mbp = (struct mbstruct *) 
                                              ap->dev[dev_index].head_act->
                                              bufstruct.b_work;
					mbp->preempt--;
					if( mbp->preempt == 0 ) {
					    ap->dev[dev_index].queue_depth =
                                            Q_DISABLED;
					    ap->dev[dev_index].queuing_stopped =
									TRUE;
				}
					/* check status of next command */
					if (ap->dev[dev_index].head_act->
					    bufstruct.b_error != 0) {
					    /* this indicates a command     */
					    /* which failed previously due  */
					    /* to mbox error. Now, need to  */
					    /* fail the queue for this one. */
					    hsc_fail_cmd(ap, dev_index);
					}
					else {	/* no error on next sc_buf */
					    /* normal completion, queued    */
					    /* cmds, kick off next request, */
					    /* if necessary                 */
					    hsc_start(ap, dev_index);
					}
				    }
				}	/* not good completion, continue */
			    }
			}
		    }
		}	/* mbox inactive -- ignore, must be the interrupt
			   from a previously handled timeout error; queue
			   already cleared */

		save_isr &= ~(mask_val << n);	/* indicate this mbox was
						   handled */
	    }	/* continue, this one not interrupting */

	    n++;	/* inc flag shift count */

	}	/* endwhile */

/************************************************************************/
/* handle system IOCC detected errors for in progress commands here     */
/************************************************************************/
	/* if card did not detect an error but there was a system         */
	/* detected error, or if system detected a DMA_SYSTEM             */
	/* error, then fail all queues with DMA in progress.              */
	if (((!dma_err_detected) && (sys_dma_rc)) ||
	    (sys_dma_rc & SYS_ERR)) {
	    hsc_DMA_err(ap, 0);
	}

/************************************************************************/
/*	handle adapter error recovery prior to MB30 handling here       */
/************************************************************************/
	if (dma_err_detected == TRUE) {
	    /* must clear this condition via scsi reset to avoid data  */
	    /* integrity problem for system.  if reset fails due to    */
	    /* unrecovered pio error, ignore, as queue already failed, */
	    /* and let future commands simply timeout.                 */
	    (void) hsc_scsi_reset(ap);
	    /* below we assume that INIT DEV is not used for any recovery */
	    /* assume only other states are not waiting and wait for      */
	    /* restart                                                    */
	    if (ap->restart_state == WAIT_TO_SEND_RESTART) {
		ap->restart_state = 0;	/* reset any waiting adap recov */
	    }
	    ap->restart_again = FALSE;	/* reset any pending recovery */
	}
	else {

	    if ((ap->adapter_mode == DIAG_MODE) &&
		(ap->restart_state == WAIT_TO_SEND_RESTART)) {
		ap->restart_state = 0;	/* don't allow restart in diag mode */
		ap->restart_again = FALSE;	/* reset pending recovery */
	    }

	    if (ap->restart_state == WAIT_TO_SEND_RESTART) {
		if (ap->MB30_in_use == -1) {	/* if MB30 is free   */
		    ap->MB30_in_use = ADAP_USING;
		    ap->restart_state = WAIT_FOR_RESTART;
		    /* build MB30 RESTART cmd */
		    hsc_build_mb30(ap, RESTART, 0, 0, 0);
		    ap->wdog.dog.restart = ADAP_CMD_T_O;
		    w_start(&ap->wdog.dog);	/* start adap watchdog */

		    /* send MB30 command to adapter */
		    iop.opt = WRITE_MB30;
		    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov)
			== EIO) {
			/* handle unrecovered error sending mbox 30.   */
			/* ignore error here, allowing timeout to hit. */
			;
		    }
		}
		else {
		    ap->waiting_for_mb30 = TRUE;
		}
	    }	/* not waiting to send restart */

	}



	if (mb30_intr) {
/************************************************************************/
/*	handle good/bad MB30 completion interrupt here                  */
/************************************************************************/
	    hsc_MB30_handler(ap, TRUE);

	}	/* no MB30 interrupt */

	/* end of error interrupt completion handling logic */

    }

    i_reset(&(ap->intr_struct));	/* reset system IOCC intrpt logic */


exit:
    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_INTR, 0, ap->devno);
    unlock_enable(old_pri, &(hsc_mp_lock));
end:

    return (ret_code);

}  /* end hsc_intr */
