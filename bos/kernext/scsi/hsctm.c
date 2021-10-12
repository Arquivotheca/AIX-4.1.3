static char sccsid[] = "@(#)72	1.7  src/bos/kernext/scsi/hsctm.c, sysxscsi, bos411, 9428A410j 1/24/94 11:20:52";
/*
 * COMPONENT_NAME: (SYSXSCSI) IBM SCSI Adapter Driver
 *
 * FUNCTIONS:	hsc_alloc_tgt, hsc_dealloc_tgt, hsc_enable_buf,
 *		hsc_enbuf_cmd, hsc_get_tag, hsc_tgt_tcw_alloc,
 *		hsc_free_tmbufs, hsc_free_rdbufs, hsc_tgt_tcw_dealloc,
 *		hsc_free_tm_garbage, hsc_enable_id, hsc_enaid_cmd,
 *		hsc_start_bufs, hsc_start_ids, hsc_stop_ids,
 *		hsc_buf_free, hsc_free_a_buf, hsc_tgt_DMA_err,
 *		hsc_async_notify, hsc_need_disid, hsc_need_enaid
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
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
/* NAME:        hsctm.c                                                 */
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
/*	This module contains functions required to support target	*/
/*	in the adapter driver.						*/
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


/********************************************************************
 NAME		:hsc_alloc_tgt 
  
 FUNCTION	: allocates target mode buffers and enables them
		  and enables ids that can act as initiators.
  
 EXECUTION ENVIRONMENT
		This function is called from the process level
		through SCIOSTARTTGT ioctl. Only KERNEL processes
		can call this function.
  
 DATA STRUCTURES
		dev_info structure pointer;
		b_link   structure pointer;
		sc_strt_tgt structure into which arguments are copied.
 INPUTS
		ap	- adapter_def structure pointer;
		arg	- argument list address;
		devflag	- flag to indicate Kernel/user mode call;
  
 RETURN CODES
		EACCESS
		EPERM
		EINVAL
		ENOMEM
		EIO
  
 EXTERNAL FUNCTIONS
  
*********************************************************************/
int
hsc_alloc_tgt(
	      struct adapter_def * ap,
	      int arg,
	      ulong devflag)
{
    int     i, rc, tag, dev_index, old_pri;
    uint    tm_max_request;
    struct dev_info *d;
    struct b_link *bp;
    struct sc_strt_tgt stgt;

    /* adapter must have been opened in normal mode */
    if (ap->adapter_mode != NORMAL_MODE) {
	return (EACCES);	/* wrong adapter mode */
    }

    if (devflag & DKERNEL) {
	bcopy((char *) arg, &stgt, sizeof(struct sc_strt_tgt));
    }
    else {
	return (EPERM);
    }
    dev_index = stgt.id + IMDEVICES;	/* dev_index for target device */
    if (ap->dev[dev_index].opened) {
	return (EINVAL);	/* already opened */
    }

    /* make sure the passed scsi id is different from the card's */
    if ((stgt.id == ap->ddi.card_scsi_id) || (stgt.lun != 0)) {
	return (EINVAL);	/* bad SCSI ID */
    }

    /* check for validity of other parameters	 */
    /* eventually, check for: (stgt.buf_size & (TCWRANGE - 1)) */
    /* right now, buf_size is fixed */
    if ((stgt.recv_func == NULL) ||
	(stgt.buf_size != SC_TM_BUFSIZE) ||
	(stgt.num_bufs < SC_TM_MIN_NUMBUFS)) {
	return (EINVAL);
    }
    d = &ap->dev[dev_index];	/* dev_info struct ptr for this device	 */

    /* allocate buffers for this device.		 */
    /* if(can not allocate)return(ENOMEM).		 */

    d->buf_size = SC_TM_BUFSIZE;
    d->num_bufs = stgt.num_bufs;
    d->in_open = TRUE;	/* mark as in process of being opened */
    for (i = 0; i < d->num_bufs; i++) {
	bp = (struct b_link *) xmalloc(sizeof(struct b_link),
				       2, pinned_heap);	/* on word boundary */
	if (bp == NULL) {
	    hsc_free_tmbufs(ap, dev_index);
	    return (ENOMEM);
	}
	bzero(bp, sizeof(struct b_link));

	/* now, malloc the data area for the b_link */
	bp->buf_addr = xmalloc(d->buf_size, PGSHIFT, pinned_heap);
	if (bp->buf_addr == NULL) {
	    /* release this b_link */
	    (void) xmfree((caddr_t) bp, pinned_heap);
	    hsc_free_tmbufs(ap, dev_index);
	    return (ENOMEM);
	}

	/* init other static elements of the structure */
	bp->owner_id = stgt.id;	/* id of owning target device */
	bp->buf_size = d->buf_size;
	bp->ap = ap;	/* adapter def struct addr */
	bp->adap_devno = ap->devno;	/* adapter major/minor device num. */
	bp->owner_flag = TM_ALLOCATED;
	bp->next = NULL;

	/* add to unmapped list--now that everything is ready */

        /* serialize with intrpts */
	old_pri = disable_lock(ap->ddi.int_prior, &hsc_mp_lock);
	if (ap->head_free_unmapped == NULL)
	    ap->head_free_unmapped = bp;
	else
	    ap->tail_free_unmapped->next = bp;
	ap->tail_free_unmapped = bp;
	unlock_enable(old_pri, &hsc_mp_lock);

    }	/* end for */
    d->waiting = FALSE;
    d->num_act_cmds = 0;
    d->qstate = 0;
    d->state = 0;
    d->num_bufs_qued = 0;
    d->previous_err = 0;
    d->num_to_resume = 0;
    d->num_bufs_recvd = 0;	/* reset number of bufs received */
    d->num_bytes_recvd = 0;	/* reset number of bytes received */
    /* d->dev_abort             do not reset, as hsc_dealloc_tgt sets */
    d->async_func = NULL;	/* invalidate last registration */

    /* serialize with intrpts */
    old_pri = disable_lock(ap->ddi.int_prior, &hsc_mp_lock);

    /* map as many buffers as tcws available and move them from	 */
    /* unmapped free list to mapped free list			 */
    while ((ap->num_tgt_tcws_used < ap->num_tgt_tcws) &&
	   (ap->head_free_unmapped != NULL)) {
	rc = hsc_tgt_tcw_alloc(ap, ap->head_free_unmapped);
	if (rc != TRUE)
	    break;
	if (ap->head_free_mapped == NULL)
	    ap->head_free_mapped = ap->head_free_unmapped;
	else
	    ap->tail_free_mapped->next = ap->head_free_unmapped;
	ap->tail_free_mapped = ap->head_free_unmapped;
	ap->head_free_unmapped = ap->head_free_unmapped->next;
	ap->tail_free_mapped->next = NULL;
	if (ap->head_free_unmapped == NULL)
	    ap->tail_free_unmapped = NULL;
    }	/* end while */

    /* Enable as many buffers as possible. Fail to enable is not     */
    /* an error because they are on free_buffer list.If first target */
    /* device enable buffers in process level else just enable id    */
    /* call start_bufs to kick off enable buffers on interrupt level */
    if (ap->num_tm_devices == 0) {
	while (ap->num_enabled < MAX_TAG) {
	    if (ap->head_free_mapped == NULL)
		break;
	    rc = hsc_enable_buf(ap, ap->head_free_mapped, ENABLE, PROC_LVL, 0);
	    if (rc < 0) {	/* fatal error. free other bufs. */
                /* allow intrpt handler in */
		unlock_enable(old_pri, &hsc_mp_lock);
		hsc_free_tmbufs(ap, dev_index);
		return (EIO);
	    }
	    if (rc)
		break;	/* waiting for resources */
	}	/* end while */
    }
    /* Buffers are enabled. Now enable Initiator id.		 */
    /* if enable id fails free all buffers and fail open.	 */

    rc = hsc_enable_id(ap, stgt.id, ENABLE, PROC_LVL);
    if (rc != 0) {
	unlock_enable(old_pri, &hsc_mp_lock);
	hsc_free_tmbufs(ap, dev_index);
	return (EIO);
    }
    d->stopped = FALSE;	/* flag port enabled in sync with enable id */
    d->opened = TRUE;	/* mark this device opened -- this allows buffers to
			   be enabled on intr level */
    d->in_open = FALSE;	/* flag not being opened anymore */
    hsc_start_bufs(ap);	/* kick off remaining enabufs in intr lvl */
    unlock_enable(old_pri, &hsc_mp_lock);	/* allow intrpt handler in */

    /* copy out the arg structure to the caller with buf_free func addr */
    /* note that caller must have been kernel process, not user proc */
    stgt.free_func = (void (*) ()) hsc_buf_free;
    bcopy((char *) &stgt, (char *) arg, sizeof(struct sc_strt_tgt));

    /* init this device's info structure */
    d->scsi_id = stgt.id;
    d->lun_id = 0;
    /* the following sets the boundary for determining when to enable again */
    /* 75% in use (25% freed) enables again */
    d->num_to_resume = (d->num_bufs * 3) >> 2;
    d->tm_correlator = stgt.tm_correlator;
    d->recv_func = stgt.recv_func;
    ap->num_tm_devices++;

    return (0);
}  /* end hsc_alloc_tgt */

/********************************************************************
 NAME		hsc_dealloc_tgt
  
 FUNCTION	function to stop target device. It first disables id
		and then disables and frees buffers.
  
 EXECUTION ENVIRONMENT
		This function is called from the process level through
		SCIOSTOPTGT ioctl. Only KERNEL processes can call this
		function.
  
 DATA STRUCTURES
  
 INPUTS
		ap	- adapter_def structure address
		arg	- argument list pointer
		devflag	- flag which indicates kernel/user modes
  
 RETURN CODES
		EACCESS
		EINVAL
		EPERM
  
 EXTERNAL FUNCTIONS
*********************************************************************/
int
hsc_dealloc_tgt(
		struct adapter_def * ap,
		int arg,
		ulong devflag)
{
    int     rc, dev_index, old_pri;
    struct dev_info *d;
    struct sc_stop_tgt stop_tgt;

    /* adapter must have been opened in normal mode */
    if (ap->adapter_mode != NORMAL_MODE) {
	return (EACCES);	/* wrong adapter mode */
    }

    if (devflag & DKERNEL) {
	bcopy((char *) arg, &stop_tgt, sizeof(uchar));
    }
    else {
	return (EPERM);
    }
    dev_index = stop_tgt.id + IMDEVICES;	/* dev_index for target
						   device */
    d = &ap->dev[dev_index];

    /* validate the stop parameters */
    if (d->opened == FALSE) {
	return (EINVAL);	/* target device not started */
    }

    /* First mark the device closed, so that its buffers won't	 */
    /* get reenabled while its being stopped.			 */

    d->opened = FALSE;
    ap->num_tm_devices--;

    /* Need to set flag to indicate device disabled while in temporary
       disable state so that first buffer abort can be ignored (stopped is
       TRUE if temp disabled) */
    d->dev_abort = d->stopped;

    /* disable initiator's id before disabling buffers. */
    /* note that this permanent disable can be sent whether or not the device
       id is temporarily disabled.              */
    old_pri = disable_lock(ap->ddi.int_prior, &hsc_mp_lock);	
    (void) hsc_enable_id(ap, stop_tgt.id, DISABLE, PROC_LVL);
    unlock_enable(old_pri, &hsc_mp_lock);

    /* Now disable and free buffers, TCWs and tags for this device	 */
    /* hsc_free_tmbufs also frees any buffers qued for the device	 */
    /* and not yet returned by the device.				 */

    (void) hsc_free_tmbufs(ap, dev_index);

    /* Free tm garbage buffers if any for this adapter		 */
    (void) hsc_free_tm_garbage(ap);

    d->qstate = 0;
    d->num_bufs = 0;
    d->buf_size = 0;
    return (0);
}  /* end hsc_dealloc_tgt */


/************************************************************************
 NAME:		hsc_enable_buf
  
 FUNCTION:	Enables/disables a buffer to receive SEND cmd data.
  
 EXECUTION ENVIRONMENT:
		This function can be called from both process and
		interrupt levels.
  
 DATA STRUCTURES:
		dev_info structure pointer;
  
 INPUTS:
		ap		- adapter_def structure pointer;
		buf		- address of the b_link structure which
				  contains all the required info about the
				  buffer being enabled or disabled.
		option		- enable/disable buffer options:
                                  ENABLE, DISABLE
		level		- caller indicates if calling from
                                  PROC_LVL or INTR_LVL
		tag		- tag of the buffer being DISABLED.
				  NOT used on ENABLE option.
  
 RETURN VALUES:
		0		- successful completion
		-1		- error completion
		WAITING_FOR_RESOURCES - tags or TCWs were not available
		WAITING_FOR_MB30- Mailbox 30 is in use or kicked-off
                                  the command.
  
 EXTERNAL FUNCTINS:
*************************************************************************/
hsc_enable_buf(
	       struct adapter_def * ap,
	       struct b_link * buf,
	       uchar option,
	       uchar level,
	       uchar tag)
{
    int     rc, ena_tag;

#ifdef	HSC_NEVER
    hsc_internal_trace(ap, TRC_ENAB, 'ENT ', option, level, 0, buf);
#endif	HSC_NEVER
    rc = 0;
    if (level == PROC_LVL) {
	if (ap->adapter_check == ADAPTER_DEAD) {
#ifdef	HSC_NEVER
	    hsc_internal_trace(ap, TRC_ENAB, 'EXIT', option, level, 1, buf);
#endif	HSC_NEVER
	    return (-1);
	}
	ap->p_buf = buf;
	buf->option = option;
	if (ap->proc_waiting != 0) {
#ifdef	HSC_NEVER
	    hsc_internal_trace(ap, TRC_ENAB, 'EXIT', option, level, 2, buf);
#endif	HSC_NEVER
	    return (WAITING_FOR_MB30);
	}
	ap->proc_waiting = WAIT_TO_ENA_BUF;
	if (ap->MB30_in_use == -1) {	/* MB30 is free */
	    if (option == ENABLE) {
		ena_tag = hsc_get_tag(ap);
		if (ena_tag < 0) {
		    ap->proc_waiting = 0;
#ifdef	HSC_NEVER
		    hsc_internal_trace(ap, TRC_ENAB, 'EXIT', option, level,
				       3, buf);
#endif	HSC_NEVER
		    return (WAITING_FOR_RESOURCES);
		}
		rc = hsc_tgt_tcw_alloc(ap, buf);
		if (rc == FALSE) {
		    ap->proc_waiting = 0;
#ifdef	HSC_NEVER
		    hsc_internal_trace(ap, TRC_ENAB, 'EXIT', option, level,
				       4, buf);
#endif	HSC_NEVER
		    return (WAITING_FOR_RESOURCES);
		}
		ap->tm_bufs[ena_tag] = buf;
		buf->tag = ena_tag;
	    }
	    else {	/* disable */
		buf = ap->tm_bufs[tag];
	    }
	    ap->MB30_in_use = PROC_USING;
	    ap->proc_waiting = WAIT_FOR_ENA_BUF;
	    ap->proc_results = 0;
	    rc = hsc_enbuf_cmd(ap, buf, option, PROC_LVL);
	    if (rc == -1) {	/* pio op failed */
#ifdef	HSC_NEVER
		hsc_internal_trace(ap, TRC_ENAB, 'EXIT', option, level,
				   5, buf);
#endif	HSC_NEVER
		return (rc);
	    }
	}
	else {	/* mb30 in use */
	    ap->waiting_for_mb30 = TRUE;
	}

	/* NOTE:  while sleeping for interrupt, interrupts are re-enabled.
	   Since head_free_mapped is used for enable buf both in process and
	   interrupt level paths, it is imperative that the interrupt level
	   not attempt to enable a buffer until this process level enable buf
	   is finished */
	e_sleep_thread(&ap->event, &hsc_mp_lock, LOCK_HANDLER);

	/* handle errors and set rc accordingly		 */
	/* test the return status to set up the return code */

	switch (ap->proc_results) {
	  case GOOD_COMPLETION:
	    rc = 0;
	    break;
	  case TIMED_OUT:
	  case FATAL_ERROR:
	    rc = -1;
	    break;
	  case SEE_RC_STAT:
	    if (ap->mb30_rc == SCSI_BUS_RESET)
		rc = 0;	/* logged via mb31 */
	    else {
		rc = -1;
		hsc_logerr(ap, ERRID_SCSI_ERR4, ap->MB30p,
			   UNKNOWN_CARD_ERR, 87, 0);
	    }
	    break;
	  default:	/* if its none of them, then its an error */
	    rc = -1;
	    break;
	}	/* endswitch */

	/* if enable was successful, increment counts else free tag, TCWs */
	/* if disable, decrement counts, free tag and TCWs.		  */

	if (option == ENABLE) {
	    if (rc < 0) {
		/* handle failed enable buffer */
		if (buf != NULL) {
		    /* free TCWs. don't set forced flag, because another
		       device might try to enable before it is taken off the
		       free list */
		    (void) hsc_tgt_tcw_dealloc(ap, buf, FALSE);
		}
		ap->tm_bufs[ena_tag] = NULL;	/* free tag */
	    }
	    else {	/* successful enable buffer */
		if (ap->tm_bufs[buf->tag] == buf) {	/* mb30 before mb31 */
		    /* here, the mb30 completed, without data having already
		       come in and cleared the tag */
		    buf->owner_flag |= TM_ENABLED;
		    ap->num_enabled++;
		    ap->head_free_mapped = ap->head_free_mapped->next;
		    if (ap->head_free_mapped == NULL)
			ap->tail_free_mapped = NULL;
		}
	    }
	}
	else {	/* else disable buffer done 		 */
	    /*  if the buffer is still enabled (tag valid check), then force
		free the resources for this buffer/tag, as that is the only
		way the memory for this buffer can be freed.  Note that
		because we get to this path for both good and bad ret codes,
		the potential exists here for the adapter to be DMA'ing to
		the buffer when we try to free it and also that the 
		buffer could be left dirty. */
	    if (ap->tm_bufs[buf->tag] == buf) {	/* mb30 before mb31 */
		buf->owner_flag = TM_ALLOCATED;
		buf->user_flag = 0;
		buf->data_len = 0;
		ap->num_enabled--;	/* decr count	 */
		(void) hsc_tgt_tcw_dealloc(ap, buf, TRUE);	/* free TCWs */
		ap->tm_bufs[tag] = NULL;	/* free tag	  */
		/* can't xmfree the b_link and buffer here because interrupts
		   are disabled! instead, put b_link on the garbage
		   queue--let hsc_free_tmbufs handle */
		buf->next = ap->tm_garbage;
		ap->tm_garbage = buf;
	    }
	}
#ifdef	HSC_NEVER
	hsc_internal_trace(ap, TRC_ENAB, 'EXIT', option, level, 6, buf);
#endif	HSC_NEVER
    }
    else {	/* interrupt level entry */
	if (ap->enabuf_state != 0) {	/* if mb30 busy */
#ifdef HSC_NEVER
	    hsc_internal_trace(ap, TRC_ENAB, 'EXIT', option, level, 7, buf);
#endif	HSC_NEVER
	    return (WAITING_FOR_MB30);	/* return	 */
	}
	if (option == ENABLE) {
	    ap->ena_buf = buf;
	    ap->enabuf_state = WAIT_TO_ENA_BUF;
	}
	else {
	    ap->enabuf_state = WAIT_TO_DIS_BUF;
	    buf = ap->tm_bufs[tag];
	    ap->disable_tag = tag;	/* need it in mb30 handler */
	}
	buf->option = option;
	if (ap->MB30_in_use != -1) {	/* mb30 is in use */
	    ap->waiting_for_mb30 = TRUE;
#ifdef HSC_NEVER
	    hsc_internal_trace(ap, TRC_ENAB, 'EXIT', option, level, 8, buf);
#endif	HSC_NEVER
	    return (WAITING_FOR_MB30);
	}
	if (buf->option == ENABLE) {
	    ena_tag = hsc_get_tag(ap);
	    if (ena_tag < 0) {
		/* this should never happen	 */
		ap->enabuf_state = 0;
#ifdef HSC_NEVER
		hsc_internal_trace(ap, TRC_ENAB, 'EXIT', option, level,
				   9, buf);
#endif	HSC_NEVER
		return (WAITING_FOR_RESOURCES);
	    }
	    rc = hsc_tgt_tcw_alloc(ap, buf);
	    if (rc == FALSE) {
		/* this should never happen	 */
		ap->enabuf_state = 0;
#ifdef HSC_NEVER
		hsc_internal_trace(ap, TRC_ENAB, 'EXIT', option, level,
				   10, buf);
#endif	HSC_NEVER
		return (WAITING_FOR_RESOURCES);
	    }
	    ap->tm_bufs[ena_tag] = buf;
	    buf->tag = ena_tag;
	    ap->enabuf_state = WAIT_FOR_ENA_BUF;
	}
	else {	/* disable option */
	    ap->enabuf_state = WAIT_FOR_DIS_BUF;
	}
	ap->MB30_in_use = TM_DEVICE_USING;
	rc = hsc_enbuf_cmd(ap, buf, option, INTR_LVL);
#ifdef	HSC_NEVER
	hsc_internal_trace(ap, TRC_ENAB, 'EXIT', option, level, 11, buf);
#endif  HSC_NEVER
    }
    return (rc);
}  /* end hsc_enable_buf */

/********************************************************************
 NAME		hsc_enbuf_cmd
  
 FUNCTION	Issues mb30 enable/disable buffer command
  
 EXECUTION ENVIRONMENT
		This function can be called from both interrrupt
		and process execution environments.
  
 DATA STRUCTURES
		iop	- io_parms structure for mb30 command
  
 INPUTS
		ap	- adapter_def structure address
		p	- b_link structure address
		option	- enable/disable option
		level	- interrupt/process level call
  
 RETURN CODES
		-1		 - pio error
		WAITING_FOR_MB30 - command successfully done.
  
 EXTERNAL FUNCTIONS
**********************************************************************/
hsc_enbuf_cmd(
	      struct adapter_def * ap,
	      struct b_link * p,
	      uchar option,
	      uchar level)
{
    uchar   burst;
    struct io_parms iop;
    int     ret_code;

    /* interrupts are assumed to be disabled on entry */

    ret_code = 0;	/* set default return code */

    /* init iop structure		 */
    bzero(&iop, sizeof(struct io_parms));

    /* build mb30 ENABLE/DISABLE BUF cmd	 */

    if (option == ENABLE) {
	burst = (DMA_BURST << 1) & 0xe;
	hsc_build_mb30(ap, ENA_BUF, burst, p->tag, 0);
    }
    else {
	hsc_build_mb30(ap, ENA_BUF, 1, p->tag, 0);
    }
    ap->wdog.dog.restart = ENABUF_CMD_T_O;
    w_start(&ap->wdog.dog);

    /* send MB30 command to adapter */
    iop.ap = ap;
    iop.opt = WRITE_MB30;
    iop.errtype = 0;
    if ((pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) &&
	(iop.errtype != PIO_PERM_IOCC_ERR)) {
	w_stop(&ap->wdog.dog);
	ap->MB30_in_use = -1;	/* release mbox 30 */
	if (level == PROC_LVL) {
	    ap->proc_waiting = 0;

	    /* if this is a process level disable buffer, and the buffer
		is still enabled (tag valid check), then force free the
		resources for this buffer/tag, as that is the only way
		the memory for this buffer can be freed.  Note that the
		potential exists here for the adapter to be DMA'ing to
		the buffer when we try to free it and also that the 
		buffer could be left dirty. */
	    if ((option == DISABLE) &&
	        (ap->tm_bufs[p->tag] == p)) {
		p->owner_flag = TM_ALLOCATED;
		p->user_flag = 0;
		p->data_len = 0;
		ap->num_enabled--;	/* decr count	 */
		/* force free TCWs */
		(void) hsc_tgt_tcw_dealloc(ap, p, TRUE);
		ap->tm_bufs[p->tag] = NULL;	/* free tag	  */
		/* can't xmfree the b_link and buffer here because interrupts
		   are disabled! instead, put b_link on the garbage
		   queue--let hsc_free_tmbufs handle */
		p->next = ap->tm_garbage;
		ap->tm_garbage = p;
	    }
	}
	else {	/* interrupt level */
	    ap->enabuf_state = 0;
	}
	if (option == ENABLE) {
	    ap->tm_bufs[p->tag] = NULL;
	    (void) hsc_tgt_tcw_dealloc(ap, p, FALSE);
	}
	return (-1);	/* indicate error */
    }
    else {
	return (WAITING_FOR_MB30);
    }

}  /* end hsc_enbuf_cmd */


/*************************************************************************
 NAME		:hsc_get_tag
  
 FUNCTION	: This internal function is used to allocate a free tag
		  for a read buffer before it is enabled. SCSI adapter
		  can have a max of 16 buffers enabled at any time.
  
 EXECUTION ENVIRONMENT
		This function can be called from both process and interrupt
		environments.
  
 DATA STRUCTURES
  
 INPUTS
		ap	- adapter_def structure address
		buf	- addr of b_link structure whose buffer
			  is being enabled.
  
 RETURN CODES
		>= 0	- tag,successful
		< 0	- No free tag available.
  
 EXTERNAL FUNCTIONS
**************************************************************************/
int
hsc_get_tag(struct adapter_def * ap)
{
    int     tag;

    for (tag = 0; tag < MAX_TAG; tag++) {
	if (ap->tm_bufs[tag] == NULL) {
	    return (tag);
	}
    }
    return (-1);
}  /* end hsc_get_tag */

/************************************************************************/
/*                                                                      */
/* NAME:        hsc_tgt_tcw_alloc                                       */
/*                                                                      */
/* FUNCTION:    Allocate Translation Control Words Routine              */
/*                                                                      */
/*      This routine attempts to allocate system TCWs from the          */
/*      range of TCWs specified in the adapter ddi area for the         */
/*      requested I/O.                                                  */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called on priority levels greater      */
/*      than, or equal to that of the interrupt handler.                */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter information structure              */
/*      p     - pointer to b_link structur of the buffer 		*/
/*		  being processed 					*/
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      TRUE    - TCWs were successfully allocated                      */
/*      FALSE   - TCWs could not be allocated                           */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:  none                                    */
/*                                                                      */
/*                                                                      */
/************************************************************************/
int
hsc_tgt_tcw_alloc(
		  struct adapter_def * ap,
		  struct b_link * p)
{
    int     badr, val, len, i, cnt;
    int     dev_index;

    if (p->tcws_alloced == 0) {	/* unmapped buffer */
	/* get index into device table for this device */
	dev_index = p->owner_id + IMDEVICES;	/* owner of the buf */
	badr = (int) p->buf_addr;

	/* calculate number of TCWs required for this request */
/*
	len = ((((badr + p->buf_size) & ~(TCWRANGE - 1)) -
		(badr & ~(TCWRANGE - 1))) / TCWRANGE) + 1;
	len = (((p->buf_size + (TCWRANGE - 1)) & TCWRANGE) / TCWRANGE);
*/

	/* calculate number of TCWs required for this request. this assumes
	   buf_size was previously forced to a multiple of the TCW pagesize. */
	len = p->buf_size / TCWRANGE;

	/* assume interrupts are disabled here */

	/* search for consecutive TCWs required for this request. */
	/* this first search starts where the last allocation    */
	/* ended as a hint to where free ones are.               */

	for (cnt = 0, i = ap->tgt_next_req; i <= ap->tgt_req_end; i++) {
	    if (ap->TCW_tab[i] == 0xff)
		cnt++;	/* found one */
	    else
		cnt = 0;	/* start over */
	    if (cnt == len)
		break;	/* done */
	}
	/* if the TCWs were NOT found in the above search, must now */
	/* search the entire range, as it is possible to have missed */
	/* free TCWs above and below the "next" index.              */

	if (cnt != len) {
	    for (cnt = 0, i = ap->tgt_req_begin; i <= ap->tgt_req_end; i++) {
		if (ap->TCW_tab[i] == 0xff)
		    cnt++;	/* found one */
		else
		    cnt = 0;	/* start over */
		if (cnt == len)
		    break;	/* done */
	    }
	}

	if (cnt != len)
	    return (FALSE);	/* no tcws */

	/* set the "next" index up */
	if (i >= ap->tgt_req_end)
	    ap->tgt_next_req = ap->tgt_req_begin;	/* wrap next index */
	else
	    ap->tgt_next_req = (i + 1);

	/* mark TCWs as "in use" for this device */
	for (val = (i + 1) - len; val <= i; val++)
	    ap->TCW_tab[val] = dev_index;	/* mark with device index */
	p->tcw_start = (i + 1) - len;	/* store starting TCW */
	p->tcws_alloced = len;	/* store number of TCWs */
	/* increment the global adapter count of tgt tcws in use */
	ap->num_tgt_tcws_used += p->tcws_alloced;

	/* IM tcw_start_addr is used to compute dma addr because    */
	/* p->tcw_start is offset from the begining of the tcw table */

	p->dma_addr = (uint) (DMA_ADDR(ap->ddi.tcw_start_addr, p->tcw_start)
			      + ((uint) p->buf_addr & (TCWRANGE - 1)));

	/* normal non spanned DMA transfer		 */

	d_master((int) ap->channel_id, DMA_READ, p->buf_addr,
		 p->buf_size, &ap->xmem_buf, (caddr_t) p->dma_addr);

	(void) xmemdma(&ap->xmem_buf, p->buf_addr, XMEM_UNHIDE);
    }
    else {
	vm_cflush((uint) p->buf_addr, p->buf_size);
    }
    return (TRUE);
}  /* end hsc_tgt_tcw_alloc */

/**************************************************************************
 NAME		hsc_free_tmbufs
  
 FUNCTION	disables buffers, frees tags,TCWs and buffer memory.
  
		It first disables buffers in the tm_bufs array
		and frees their TCWs and memory.
		It then tries to free the TCWs and memory for
		buffers on the drain buffers queue.
		At the end it frees memory for all the buffers
		the free buffers queue.
  
 EXECUTION ENVIRONMENT
		This routine can ONLY be called from the process
		environment, since it frees memory.
  
 DATA STRUCTURES
  
 INPUTS
  
 RETURN CODES
  
 EXTERNAL FUNCTIONS
**************************************************************************/
hsc_free_tmbufs(
		struct adapter_def * ap,
		int dev_index)
{
    uchar   id, tag;
    int     rc, old_pri;
    struct dev_info *d, *d1;
    struct b_link *head_to_free;
    struct b_link *p, *backp, *save_p;


    if (dev_index < IMDEVICES)
	return (0);	/* if not target mode device, return */
    id = dev_index - IMDEVICES;	/* scsi_id of the device being freed */
    d = &ap->dev[dev_index];	/* dev_info ptr of the device	     */
    d->in_open = FALSE;	/* flag not being opened anymore. Note that this is
			   an error path cleanup for hsc_alloc_tgt.  */

    /* first disable all enabled buffers for this device	 */

    /* mask out intrpts */
    old_pri = disable_lock(ap->ddi.int_prior, &hsc_mp_lock);	
    for (tag = 0; tag < MAX_TAG; tag++) {
	p = ap->tm_bufs[tag];
	if ((p == NULL) || (p->owner_id != id))
	    continue;
	/* disable the buffer */
	(void) hsc_enable_buf(ap, p, DISABLE, PROC_LVL, tag);
    }
    unlock_enable(old_pri, &hsc_mp_lock);

    /* now free the buffers in the rdbufs used by this device */

    (void) hsc_free_rdbufs(ap, id, MAPPED_BUFS);
    (void) hsc_free_rdbufs(ap, id, UNMAPPED_BUFS);
    head_to_free = NULL;

    /* free all the buffers owned by this device */
    /* both in mapped and unmapped free queues.	 */

    /* mask out intrpts */
    old_pri = disable_lock(ap->ddi.int_prior, &hsc_mp_lock);

    p = ap->head_free_unmapped;
    backp = p;
    while (p != NULL) {
	if (p->owner_id != id) {
	    backp = p;
	    p = p->next;
	    continue;
	}
	if (p == ap->head_free_unmapped) {
	    ap->head_free_unmapped = p->next;
	}
	else
	    if (p == ap->tail_free_unmapped) {
		ap->tail_free_unmapped = backp;
		ap->tail_free_unmapped->next = NULL;
	    }
	    else {
		backp->next = p->next;
	    }
	save_p = p->next;
	p->next = head_to_free;
	head_to_free = p;
	p = save_p;
    }	/* end while */
    if (ap->head_free_unmapped == NULL)
	ap->tail_free_unmapped = NULL;

    /* now free mapped free list for this device */
    p = ap->head_free_mapped;
    backp = p;
    while (p != NULL) {
	if (p->owner_id != id) {
	    backp = p;
	    p = p->next;
	    continue;
	}
	if (p == ap->head_free_mapped) {
	    ap->head_free_mapped = p->next;
	}
	else
	    if (p == ap->tail_free_mapped) {
		ap->tail_free_mapped = backp;
		ap->tail_free_mapped->next = NULL;
	    }
	    else {
		backp->next = p->next;
	    }
	(void) hsc_tgt_tcw_dealloc(ap, p, TRUE);	/* force free tcws */
	save_p = p->next;
	p->next = head_to_free;
	head_to_free = p;
	p = save_p;
    }	/* end while */
    if (ap->head_free_mapped == NULL)
	ap->tail_free_mapped = NULL;

    /* now that both free lists have been cleaned up, invoke hsc_start_bufs
       since it may have been prevented from running while device was closed */
    hsc_start_bufs(ap);
    unlock_enable(old_pri, &hsc_mp_lock);

    /* free buffers and b_link structures  */
    /* N.B.:  interrupts MUST be enabled for the xmfree's */
    while (head_to_free != NULL) {
	p = head_to_free;
	head_to_free = head_to_free->next;
	(void) xmfree(p->buf_addr, pinned_heap);	/* release buf space */
	(void) xmfree((caddr_t) p, pinned_heap);	/* release b_link */
    }

    /* free garbage list	 */
    (void) hsc_free_tm_garbage(ap);
    return (0);
}  /* end hsc_free_tmbufs */

/**************************************************************************/
/*	Name : hsc_free_rdbufs						  */
/*									  */
/*	Environment: process level invocation only			  */
/**************************************************************************/
 /* move rdbufs qued for this device (still with target head)     */
 /* to mapped or unmapped free lists of the devices that own them */

void
hsc_free_rdbufs(
		struct adapter_def * ap,
		uchar id,
		uchar flag)
{
    int     old_pri;
    struct b_link *p, *next_p, *head_to_free;

    head_to_free = NULL;
    old_pri = disable_lock(ap->ddi.int_prior, &hsc_mp_lock);
    if (flag == MAPPED_BUFS)
	p = ap->mapped_rdbufs;
    else
	p = ap->unmapped_rdbufs;
    while (p != NULL) {
	if (p->user_id != id) {	/* not being used by this device   */
	    p = p->forw;
	    continue;
	}

	/* first remove from rdbufs list		 */

	if (p->tcws_alloced == 0) {	/* remove from unmapped_rdbufs */
	    if (p->back == NULL) {	/* if on head of list */
		ap->unmapped_rdbufs = p->forw;
		if (ap->unmapped_rdbufs != NULL)
		    ap->unmapped_rdbufs->back = NULL;
	    }
	    else {	/* other than head of list */
		if (p->forw != NULL)
		    p->forw->back = p->back;
		p->back->forw = p->forw;
	    }
	}
	else {	/* remove from mapped_rdbufs */
	    if (p->back == NULL) {
		ap->mapped_rdbufs = p->forw;
		if (ap->mapped_rdbufs != NULL)
		    ap->mapped_rdbufs->back = NULL;
	    }
	    else {
		if (p->forw != NULL)
		    p->forw->back = p->back;
		p->back->forw = p->forw;
	    }
	}
	next_p = p->forw;	/* save next buffer address */
	p->forw = NULL;
	p->back = NULL;

	if ((p->owner_id == id) ||
	    (ap->dev[p->owner_id + IMDEVICES].opened == 0)) {

	    /* owned by this device or owner is closed. if got tcws release */
	    /* tcws and free the buffer and the b_link structure area.	 */

	    (void) hsc_tgt_tcw_dealloc(ap, p, TRUE);
	    p->next = head_to_free;
	    head_to_free = p;
	}
	else {

	    /* not owned by this dev and owner is still opened.	 */
	    /* add to owners mapped/unmapped free list		 */

	    p->next = NULL;
	    if (p->tcws_alloced == 0) {	/* goes on unmapped free list */
		if (ap->head_free_unmapped == NULL)
		    ap->head_free_unmapped = p;
		else
		    ap->tail_free_unmapped->next = p;
		ap->tail_free_unmapped = p;
	    }
	    else {	/* goes on mapped free list */
		if (ap->head_free_mapped == NULL)
		    ap->head_free_mapped = p;
		else
		    ap->tail_free_mapped->next = p;
		ap->tail_free_mapped = p;
	    }
	}
	p = next_p;
    }
    unlock_enable(old_pri, &hsc_mp_lock);
    while (head_to_free != NULL) {
	p = head_to_free;
	head_to_free = p->next;
	(void) xmfree(p->buf_addr, pinned_heap);	/* release buf space */
	(void) xmfree((caddr_t) p, pinned_heap);	/* release b_link */
    }
}  /* end hsc_free_rdbufs */

/************************************************************************/
/*                                                                      */
/* NAME:        hsc_tgt_tcw_dealloc                                     */
/*                                                                      */
/* FUNCTION:    Deallocate Translation Control Words Routine            */
/*                                                                      */
/*      This routine frees system TCWs previously allocated by the      */
/*      hsc_TCW_alloc routine for a request.                            */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called on priority levels greater      */
/*      than, or equal to that of the interrupt handler.                */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      b_link	    - read buffer link structure which holds		*/
/*			information related to a buffer			*/
/*                                                                      */
/* INPUTS:                                                              */
/*      ap		- pointer to adapter information structure 	*/
/*      buf		- pointer to read buf b_link structure		*/
/*	forced_dealloc	- flag when true reclaims tcws			*/
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      0       = successful result                                     */
/*      1       = IOCC detected DMA error other than DMA_SYSTEM         */
/*      SYS_ERR = IOCC detected DMA_SYSTEM error                        */
/*                                                                      */
/* ERROR DESCRIPTION:							*/
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:  none                                    */
/*                                                                      */
/*                                                                      */
/************************************************************************/
hsc_tgt_tcw_dealloc(
		    struct adapter_def * ap,
		    struct b_link * buf,
		    int forced_dealloc)
{
    int     i, rc, rcx;

    rc = 0;

    if (buf->tcws_alloced == 0) {	/* if none allocated, return */
	return (rc);
    }

    /* assume interrupts are disabled here */

    /* issue dma complete on data transfer	 */
    rcx = d_complete((int) ap->channel_id, DMA_WRITE_ONLY,
		     buf->buf_addr, buf->buf_size, &ap->xmem_buf,
		     (caddr_t) buf->dma_addr);
    if (rcx != DMA_SUCC) {
	hsc_logerr(ap, ERRID_SCSI_ERR2, NULL, DMA_ERROR, 88, (uint) rc);
	rc = 1;
    }
    if (rcx == DMA_SYSTEM) {
	rc = SYS_ERR;
    }
    if ((forced_dealloc) || (ap->tgt_dma_err) || (rc)) {
	/* mark TCWs as available */
	for (i = buf->tcw_start; i < (buf->tcw_start + buf->tcws_alloced); i++)
	    ap->TCW_tab[i] = 0xff;	/* release this TCW */
	ap->num_tgt_tcws_used -= buf->tcws_alloced;
	buf->tcws_alloced = 0;	/* clear mailbox entry */
	buf->tcw_start = 0;	/* clear mailbox entry */
    }
    return (rc);
}  /* end hsc_tgt_tcw_dealloc */

/****************************************************************************
 NAME		:hsc_free_tm_garbage
  
 FUNCTION	:free target read buffers left out during device close.
  
		 This internal function frees up the read buffers memory
		 that could not be freed during the device close(because
		 it was in use).
  
 EXECUTION ENVIRONMENT
		This function can only be called from process level.
  
 DATA STRUCTURES
		Address of the buffer's b_link structure which contains
		all the info about the buffer.
  
 INPUTS
		address of the adapter_def structure of the adapter
		whose accumlated garbage is being freed.
  
 RETURN CODES
  
 EXTERNAL FUNCTIONS
*****************************************************************************/
void
hsc_free_tm_garbage(struct adapter_def * ap)
{
    int     old_pri;
    struct b_link *buf, *next_buf;

   /* mask out intrpts */
    old_pri = disable_lock(ap->ddi.int_prior, &hsc_mp_lock);	
    buf = ap->tm_garbage;
    ap->tm_garbage = NULL;
    unlock_enable(old_pri, &hsc_mp_lock);
    while (buf != NULL) {
	next_buf = buf->next;
	(void) xmfree(buf->buf_addr, pinned_heap);
	(void) xmfree(buf, pinned_heap);
	buf = next_buf;
    }
}  /* end hsc_free_tm_garbage */

/*************************************************************************
 NAME		hsc_enable_id
  
 FUNCTION	enables Initiator's id to  which this adapter responds
		as target.
  
 EXECUTION ENVIRONMENT
		This function is called from the process level
		execution environment only(through SCIOSTARTTGT ioctl)
  
 DATA STRUCTURES
		iop	- io_parms structure to build mb30 command.
  
 INPUTS
		ap	- address of adapter_def structure;
		option	- enable/disable option;
		id	- scsi id if the device being enabled/disabled;
  
 RETURN CODES
		EIO	- pio error
  
 EXTERNAL FUNCTIONS
**************************************************************************/
hsc_enable_id(
	      struct adapter_def * ap,
	      uchar id,
	      uchar option,
	      uchar level)
{
    int     rc;

#ifdef HSC_NEVER
    hsc_internal_trace(ap, TRC_ENID, 'ENT ', option, level, 0, id);
#endif	HSC_NEVER
    rc = 0;
    if (level == PROC_LVL) {
	if (ap->adapter_check == ADAPTER_DEAD)
	    return (-1);
	ap->p_id = id;
	ap->p_id_option = option;
	if (ap->proc_waiting != 0) {
#ifdef HSC_NEVER
	    hsc_internal_trace(ap, TRC_ENID, 'EXIT', option, level, 1, id);
#endif	HSC_NEVER
	    return (WAITING_FOR_MB30);
	}
	ap->proc_waiting = WAIT_TO_ENA_ID;
	if (ap->MB30_in_use == -1) {	/* MB30 in use ? */
	    ap->MB30_in_use = PROC_USING;	/* flag that proc lvl using */
	    ap->proc_waiting = WAIT_FOR_ENA_ID;
	    ap->proc_results = 0;
	    rc = hsc_enaid_cmd(ap, id, option, PROC_LVL);
	    if (rc == -1) {	/* pio op failed */
#ifdef HSC_NEVER
		hsc_internal_trace(ap, TRC_ENID, 'EXIT', option, level, 2, id);
#endif	HSC_NEVER
		return (rc);
	    }
	}
	else {	/* MB30 is free */
	    ap->waiting_for_mb30 = TRUE;
	}
	e_sleep_thread(&ap->event, &hsc_mp_lock, LOCK_HANDLER);

	/* test the return status to set up the return code */
	switch (ap->proc_results) {
	  case GOOD_COMPLETION:
	    rc = 0;
	    break;
	  case TIMED_OUT:
	  case FATAL_ERROR:
	    rc = -1;
	    break;
	  case SEE_RC_STAT:
	    if (ap->mb30_rc == SCSI_BUS_RESET)
		rc = 0;
	    else {
		rc = -1;
		hsc_logerr(ap, ERRID_SCSI_ERR4, ap->MB30p,
			   UNKNOWN_CARD_ERR, 89, 0);
	    }
	    break;
	  default:
	    rc = -1;
	    break;
	}	/* endswitch */
#ifdef	HSC_NEVER
	hsc_internal_trace(ap, TRC_ENID, 'EXIT', option, level, 3, id);
#endif	HSC_NEVER
    }
    else {	/* interrupt level entry */

	if (option == ENABLE) {
	    ap->waiting_enaids |= (1 << id);
	    if (ap->enaid_state != 0) {
#ifdef	HSC_NEVER
		hsc_internal_trace(ap, TRC_ENID, 'EXIT', option, level, 4, id);
#endif	HSC_NEVER
		return (WAITING_FOR_MB30);
	    }
	}
	else {
	    ap->waiting_disids |= (1 << id);
	    if (option == T_DISABLE)
		/* set flag for temporary disable */
		ap->t_dis_ids |= (1 << id);
	    else
		/* reset flag for permanent disable */
		ap->t_dis_ids &= ~(1 << id);

	    if (ap->disid_state != 0) {
#ifdef	HSC_NEVER
		hsc_internal_trace(ap, TRC_ENID, 'EXIT', option, level, 4, id);
#endif	HSC_NEVER
		return (WAITING_FOR_MB30);
	    }
	}

	if (option == ENABLE) {
	    ap->ena_id = id;
	    ap->enaid_state = WAIT_TO_ENA_ID;
	}
	else {	/* DISABLE/TDISABLE */
	    ap->disable_id = id;
	    ap->disid_state = WAIT_TO_DIS_ID;
	}
	if (ap->MB30_in_use != -1) {	/* mb30 is in use */
	    ap->waiting_for_mb30 = TRUE;
#ifdef HSC_NEVER	
	    hsc_internal_trace(ap, TRC_ENID, 'EXIT', option, level, 5, id);
#endif	HSC_NEVER
	    return (WAITING_FOR_MB30);
	}
	if (option == ENABLE)
	    ap->enaid_state = WAIT_FOR_ENA_ID;
	else
	    ap->disid_state = WAIT_FOR_DIS_ID;
	ap->MB30_in_use = TM_DEVICE_USING;
	rc = hsc_enaid_cmd(ap, id, option, INTR_LVL);
#ifdef HSC_NEVER
	hsc_internal_trace(ap, TRC_ENID, 'EXIT', option, level, 6, id);
#endif	HSC_NEVER
    }
    return (rc);
}  /* end hsc_enable_id */


/********************************************************************
 NAME		hsc_enaid_cmd
  
 FUNCTION	Issues mb30 enable/disable id command
  
 EXECUTION ENVIRONMENT
		This function can be called from both interrrupt
		and process execution environments.
  
 DATA STRUCTURES
		iop	- io_parms structure for mb30 command
  
 INPUTS
		ap	- adapter_def structure address
		dev_index-device index between 64-71
		p	- b_link structure address
		option	- enable/disable option
		level	- interrupt/process level call
  
 RETURN CODES
		-1		 - pio error
		WAITING_FOR_MB30 - command successfully done.
  
 EXTERNAL FUNCTIONS
**********************************************************************/
hsc_enaid_cmd(
	      struct adapter_def * ap,
	      uchar id,
	      uchar option,
	      uchar level)
{
    struct io_parms iop;
    int     rc;

    rc = 0;	/* set default return code */

    /* init iop structure		 */
    bzero(&iop, sizeof(struct io_parms));

    /* build mb30 ENABLE/DISABLE Id cmd		 */

    if (option == ENABLE) {	/* enable id */
	hsc_build_mb30(ap, ENA_ID, 0, id, 0);
    }
    else
	if (option == T_DISABLE) {	/* Tdisable id */
	    hsc_build_mb30(ap, ENA_ID, 3, id, 0);
	}
	else {
	    hsc_build_mb30(ap, ENA_ID, 1, id, 0);	/* disable id */
	}
    ap->wdog.dog.restart = ENAID_CMD_T_O;
    w_start(&ap->wdog.dog);

    /* send MB30 command to adapter */
    iop.ap = ap;
    iop.opt = WRITE_MB30;
    iop.errtype = 0;
    if ((pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) &&
	(iop.errtype != PIO_PERM_IOCC_ERR)) {
	w_stop(&ap->wdog.dog);
	ap->MB30_in_use = -1;	/* release mbox 30 */
	if (level == PROC_LVL) {
	    ap->proc_waiting = 0;
	}
	else {
	    if (option == ENABLE)
		ap->enaid_state = 0;
	    else
		ap->disid_state = 0;
	}
	return (-1);	/* indicate error */
    }
    else {
	return (WAITING_FOR_MB30);
    }
}  /* end hsc_enaid_cmd */

/**********************************************************************
 NAME		hsc_start_bufs
  
 FUNCTION	This function tries to enable buffers.
  
		If no other target device is waiting for enableing
		buffers this device buffers are enabled else the
		device is put in the waiting state, so it will get
		serviced later.
  
 EXECUTION ENVIRONMENT
		This function can be called only from interrupt
		level execution environment.
  
 DATA STRUCTURES
  
 INPUTS
	ap	- adapter_def structure pointer
  
 RETURN CODES
  
 EXTERNAL FUNCTIONS
***********************************************************************/
void
hsc_start_bufs(struct adapter_def * ap)
{
    int     i, id, rc, dev_index;
    struct b_link *bp;

#ifdef	HSC_NEVER
    hsc_internal_trace(ap, TRC_STRTBUF, 'ENT ', 0, 0, 0, 0);
#endif	HSC_NEVER
    if ((!ap->tgt_dma_err) && (!ap->adapter_check)) {	/* if no DMA err */
	while ((ap->num_enabled < MAX_TAG) &&
	       (ap->enabuf_retries < MAX_ENA_RETRIES)) {
	    if (ap->head_free_mapped == NULL) {
		if (ap->head_free_unmapped == NULL) {

		    /* both mapped & unmapped free lists are empty. */
		    /* must wait until a buffer becomes free        */
		    ap->enabuf_retries = 0;
		    ap->tail_free_mapped = NULL;
		    ap->tail_free_unmapped = NULL;
		    break;
		}
		else {	/* unmapped free list is not empty */

		    /* no buffers in the mapped free list. so	  */
		    /* move head of unmapped list to mapped list */

		    ap->head_free_mapped = ap->head_free_unmapped;
		    ap->tail_free_mapped = ap->head_free_mapped;
		    ap->head_free_unmapped = ap->head_free_unmapped->next;
		    ap->head_free_mapped->next = NULL;
		    if (ap->head_free_unmapped == NULL)
			ap->tail_free_unmapped = NULL;

		    /* see if there are tcws available for immediate */
		    /* use, or if we need to "steal" tcws from the   */
		    /* mapped rdbuf list.                            */
		    /* NOTE that the following only handles the case */
		    /* of a buffer needing only one tcw.  Additional */
		    /* logic required to allow for multiple-tcw bufs */
		    /* (which is not supported by this code).        */

		    if (ap->num_tgt_tcws_used >= ap->num_tgt_tcws) {

			/* there are no tcws available--"steal" them from
			   head of mapped rdbuf (if possible). set
			   ap->tgt_next_req to tcw_start of head
			   mapped_rdbufs so that next target tcw_alloc
			   function can find tcws fast. */

			if (ap->mapped_rdbufs != NULL) {
			    for (i = ap->mapped_rdbufs->tcw_start;
				 i < (ap->mapped_rdbufs->tcw_start +
				      ap->mapped_rdbufs->tcws_alloced); i++)
				ap->TCW_tab[i] = 0xff;	/* free them  */
			    ap->num_tgt_tcws_used -=
				ap->mapped_rdbufs->tcws_alloced;
			    ap->tgt_next_req = ap->mapped_rdbufs->tcw_start;
			    ap->mapped_rdbufs->tcw_start = 0;
			    ap->mapped_rdbufs->tcws_alloced = 0;

			    /* move now unmapped head mapped_rdbufs list */
			    /* to head of unmapped_rdbufs list 		 */

			    bp = ap->mapped_rdbufs;
			    ap->mapped_rdbufs = ap->mapped_rdbufs->forw;
			    if (ap->mapped_rdbufs != NULL)
				ap->mapped_rdbufs->back = NULL;
			    bp->forw = ap->unmapped_rdbufs;
			    if (ap->unmapped_rdbufs != NULL)
				ap->unmapped_rdbufs->back = bp;
			    ap->unmapped_rdbufs = bp;

			}
			/* mapped_rdbufs is empty, can't enable buffer */
			/* until something frees up.                   */
		    }
		    /* there are tcws available currently, allow */
		    /* tcw alloc to get them for this buffer     */
		}
	    }
	    if (ap->head_free_mapped == NULL) {
		hsc_logerr(ap, ERRID_SCSI_ERR6, NULL,
			   0, 90, 0);
		ap->enabuf_retries = 0;
		ap->tail_free_mapped = NULL;
		ap->tail_free_unmapped = NULL;
		break;
	    }
	    /* the owner is validated to be opened so that hsc_start_bufs
	       does not attempt to kick off an enable from the intpr level
	       while the process level is possibly enabling buffers.  If
	       owner of buffer is not opened here, hsc_start_bufs is called
	       later to re-attempt enable. This occurs two places: in
	       hsc_alloc_tgt while enabling from the free list, and in
	       hsc_dealloc_tgt before cleaning up free list.  */
	    id = ap->head_free_mapped->owner_id;
	    if (!(ap->dev[id + IMDEVICES].opened)) {
		break;
	    }
	    rc = hsc_enable_buf(ap, ap->head_free_mapped, ENABLE, INTR_LVL, 0);
	    if ((rc == WAITING_FOR_RESOURCES) || (rc == WAITING_FOR_MB30))
		break;
	    if (rc != 0) {	/* I/O error */
		ap->enabuf_retries++;
		continue;
	    }
	    else {	/* successful */
		ap->enabuf_retries = 0;
	    }
	}	/* end while */

	if (ap->enabuf_retries >= MAX_ENA_RETRIES) {
	    ap->enabuf_retries = 0;
	    hsc_logerr(ap, ERRID_SCSI_ERR1, ap->MB30p,
		       0, 91, 0);
	    (void) hsc_async_notify(ap, ALL_DEVICES, SC_ADAP_CMD_FAILED);
	}
    }	/* end if !tgt_dma_err */
#ifdef HSC_NEVER	
    hsc_internal_trace(ap, TRC_STRTBUF, 'EXIT', 0, rc, 0, 0);
#endif	HSC_NEVER
}  /* end hsc_start_bufs */

/***************************************************************************/
/*	NAME	: hsc_start_ids						   */
/*                                                                         */
/*      inputs:  ap  -  adapter structure pointer                          */
/*                                                                         */
/*       RETURN CODES                                                      */
/*              -1               - no mb30 command kicked off.             */
/*              WAITING_FOR_MB30 - a mb30 command was kicked off.          */
/*                                                                         */
/***************************************************************************/
int
hsc_start_ids(struct adapter_def * ap)
{
    uchar   id;
    int     rc, loop_inc;

    /* this is called only from intr levels. */
    /* this makes no assumption about whether MB30 is in use or not */

#ifdef	HSC_NEVER
    hsc_internal_trace(ap, TRC_STRTID, 'ENT ', 0, 0, 0, 0);
#endif	HSC_NEVER

    rc = -1;
    /* see if we have a live adapter */
    if (!ap->adapter_check) {
	/* always start search with last used id */
	id = ap->ena_id;
	/* as long as there is another waiting id, stay in the loop */
	while (ap->waiting_enaids) {
	    loop_inc = TRUE;
	    if (ap->waiting_enaids & (1 << id)) {
		/* this id is waiting, keep processing */
		if (ap->enaid_retries < MAX_ENA_RETRIES) {

		    /* issue the enable request */
		    rc = hsc_enable_id(ap, id, ENABLE, INTR_LVL);

		    /* return code possibilities are -1 for pio error, and
		       WAITING_FOR_MB30 for cmd kicked off */
		    if (rc == WAITING_FOR_MB30)
			/* kicked-off the command, exit now */
			break;
		    else {
			/* a PIO error occurred, attempt retry */
			ap->enaid_retries++;
			loop_inc = FALSE;
		    }
		}
		else {
		    /* retries exceeded, log, clear this one and notify */
		    hsc_logerr(ap, ERRID_SCSI_ERR1, ap->MB30p, 0, 92, 0);
		    ap->enaid_retries = 0;
		    ap->waiting_enaids &= ~(1 << id);
		    /* async notify all devices */
		    (void) hsc_async_notify(ap, ALL_DEVICES,
					    SC_ADAP_CMD_FAILED);
		}
	    }
	    /* else id not waiting, try next id */

	    /* see if we should go to next id, or retry this one */
	    if (loop_inc) {
		id++;	/* inc id index */
		id &= 0x7;	/* modulo-8 */
	    }
	    /* else, leave id alone for retry attempt */

	}	/* end while */
    }
    else {
	/* adapter died, clear pending state */
	ap->waiting_enaids = 0;
	ap->enaid_retries = 0;
	/* leave enaid_state alone as it is needed to finish off timeout */
    }
#ifdef	HSC_NEVER
    hsc_internal_trace(ap, TRC_STRTID, 'EXIT', 0, 0, 0, 0);
#endif	HSC_NEVER
    return (rc);
}  /* end hsc_start_ids */

/***************************************************************************/
/*	NAME	: hsc_stop_ids						   */
/*                                                                         */
/*      inputs:  ap  -  adapter structure pointer                          */
/*                                                                         */
/*       RETURN CODES                                                      */
/*              -1               - no mb30 command kicked off.             */
/*              WAITING_FOR_MB30 - a mb30 command was kicked off.          */
/*                                                                         */
/***************************************************************************/
int
hsc_stop_ids(struct adapter_def * ap)
{
    uchar   id;
    int     rc, loop_inc;

    /* this is called only from intr levels. */
    /* this makes no assumption about whether MB30 is in use or not */

#ifdef	HSC_NEVER
    hsc_internal_trace(ap, TRC_STOPID, 'ENT ', 0, 0, 0, 0);
#endif	HSC_NEVER
    rc = -1;
    /* see if we have a live adapter */
    if (!ap->adapter_check) {
	/* always start search with last used id */
	id = ap->disable_id;
	/* as long as there is another waiting id, stay in the loop */
	while (ap->waiting_disids) {
	    loop_inc = TRUE;
	    if (ap->waiting_disids & (1 << id)) {
		/* this id is waiting, keep processing */
		if (ap->disid_retries < MAX_DIS_RETRIES) {

		    /* see if temporary or permanent disable requested */
		    if (ap->t_dis_ids & (1 << id))
			/* temp disable requested */
			rc = hsc_enable_id(ap, id, T_DISABLE, INTR_LVL);
		    else
			/* permanent disable requested */
			rc = hsc_enable_id(ap, id, DISABLE, INTR_LVL);

		    /* return code possibilities are -1 for pio error, and
		       WAITING_FOR_MB30 for cmd kicked off */
		    if (rc == WAITING_FOR_MB30)
			/* kicked-off the command, exit now */
			break;
		    else {
			/* a PIO error occurred, attempt retry */
			ap->disid_retries++;
			loop_inc = FALSE;
		    }
		}
		else {
		    /* retries exceeded, log, clear this one and notify */
		    hsc_logerr(ap, ERRID_SCSI_ERR1, ap->MB30p, 0, 93, 0);
		    ap->disid_retries = 0;
		    ap->waiting_disids &= ~(1 << id);
		    ap->t_dis_ids &= ~(1 << id);
		    /* async notify all devices */
		    (void) hsc_async_notify(ap, ALL_DEVICES,
					    SC_ADAP_CMD_FAILED);
		}
	    }
	    /* else id not waiting, try next id */

	    /* see if we should go to next id, or retry this one */
	    if (loop_inc) {
		id++;	/* inc id index */
		id &= 0x7;	/* modulo-8 */
	    }
	    /* else, leave id alone for retry attempt */

	}	/* end while */
    }
    else {
	/* adapter died, clear pending state */
	ap->waiting_disids = 0;
	ap->disid_retries = 0;
	/* leave disid_state alone as it is needed to finish off timeout */
    }
#ifdef HSC_NEVER
    hsc_internal_trace(ap, TRC_STOPID, 'EXIT', 0, 0, 0, 0);
#endif HSC_NEVER	
    return (rc);
}  /* end hsc_stop_ids */

/****************************************************************************
 NAME		hsc_buf_free
  
 FUNCTION	function for for freeing the target mode read buffers.
  
 EXECUTION ENVIRONMENT
		This function is called from both interrupt level
		and process level execution environments.
  
 DATA STRUCTURES
		dev_info	- scsi device info structure
		sc_buf		- structure passed by target head device
  
 INPUTS
		ap	  -adapter_def structure address
		dev_index -index of the device 
  
 RETURN CODES
  
 EXTERNAL FUNCTIONS
****************************************************************************/
void
hsc_buf_free(struct tm_buf * tm_ptr)
{
    int     rc;
    int     old_pri;
    struct b_link *b_ptr;
    struct tm_buf *ttm_ptr;
    struct dev_info *user;
    struct adapter_def *ap;

    if (tm_ptr != NULL) {
	b_ptr = (struct b_link *) tm_ptr;
	ap = b_ptr->ap;

        /* serialize with intrpts */
        old_pri = disable_lock(ap->ddi.int_prior, &hsc_mp_lock);
	/* all buffers in the list were used by one device */

	user = &ap->dev[tm_ptr->user_id + IMDEVICES];
	while (tm_ptr != NULL) {
	    ttm_ptr = tm_ptr->next;
	    hsc_free_a_buf((struct b_link *) tm_ptr, TRUE);
	    tm_ptr = ttm_ptr;
	}
	if (user->num_bufs_qued <= user->num_to_resume) {
	    /* send an enable id, if that is really necessary */
	    hsc_need_enaid(ap, user, user->scsi_id);
	}
	/* re-enable buffers, where possible */
	hsc_start_bufs(ap);
	unlock_enable(old_pri, &hsc_mp_lock);
    }
}  /* end hsc_buf_free */

/****************************************************************************
NAME	: hsc_free_a_buf
*****************************************************************************/
void
hsc_free_a_buf(struct b_link * tm_ptr, uchar flag)
{
    int     save_tcws_alloced;
    struct adapter_def *ap;

    tm_ptr->next = NULL;
    tm_ptr->data_len = 0;
    tm_ptr->user_flag = 0;
    tm_ptr->owner_flag = TM_ALLOCATED;
    tm_ptr->status_validity = 0;
    tm_ptr->general_card_status = 0;
    ap = tm_ptr->ap;
    save_tcws_alloced = tm_ptr->tcws_alloced;	/* save in temp variable */
    if ((ap->dev[tm_ptr->owner_id + IMDEVICES].opened) ||
	(!(ap->dev[tm_ptr->owner_id + IMDEVICES].opened) &&
	 (ap->dev[tm_ptr->owner_id + IMDEVICES].in_open))) {
	if (tm_ptr->tcws_alloced > 0) {
	    if (ap->head_free_mapped == NULL)
		ap->head_free_mapped = tm_ptr;
	    else
		ap->tail_free_mapped->next = tm_ptr;
	    ap->tail_free_mapped = tm_ptr;
	    ap->tail_free_mapped->next = NULL;
	}
	else {
	    if (ap->head_free_unmapped == NULL)
		ap->head_free_unmapped = tm_ptr;
	    else
		ap->tail_free_unmapped->next = tm_ptr;
	    ap->tail_free_unmapped = tm_ptr;
	    ap->tail_free_unmapped->next = NULL;
	}

    }
    else {	/* owner stopped. garbage */
	(void) hsc_tgt_tcw_dealloc(ap, tm_ptr, TRUE);	/* force tcw release */
	tm_ptr->next = ap->tm_garbage;
	ap->tm_garbage = tm_ptr;
    }
    if (flag == TRUE) {
	ap->dev[tm_ptr->user_id + IMDEVICES].num_bufs_qued--;

	/* remove the buffer from adapter's rdbuf lists	 */
	/* regardless if owner is opened or not.		 */

	if (save_tcws_alloced > 0) {
	    if (tm_ptr->back == NULL) {	/* first buffer */
		ap->mapped_rdbufs = tm_ptr->forw;
		if (ap->mapped_rdbufs != NULL)
		    ap->mapped_rdbufs->back = NULL;
	    }
	    else {
		if (tm_ptr->forw != NULL)
		    tm_ptr->forw->back = tm_ptr->back;
		tm_ptr->back->forw = tm_ptr->forw;
	    }
	}
	else {
	    if (tm_ptr->back == NULL) {	/* first buffer */
		ap->unmapped_rdbufs = tm_ptr->forw;
		if (ap->unmapped_rdbufs != NULL)
		    ap->unmapped_rdbufs->back = NULL;
	    }
	    else {
		if (tm_ptr->forw != NULL)
		    tm_ptr->forw->back = tm_ptr->back;
		tm_ptr->back->forw = tm_ptr->forw;
	    }
	}
    }
}  /* hsc_free_a_buf */

/**********************************************************************
	hsc_tgt_DMA_err
***********************************************************************/
void
hsc_tgt_DMA_err(struct adapter_def * ap)
{
    int     i, rc, tag;
    struct b_link *tmp_p, *p;
    int     disable_done, any_disabled;

    /* interrupts are assumed to be disabled here	 */
    any_disabled = FALSE;
    if (ap->enabuf_state != 0) {
	/* this means some enable is or will be sent.  must wait until enable
	   finishes to kick off this recovery.  setting the tgt_dma_err flag
	   will prevent other enables until recovery completes */
	ap->tgt_dma_err = TRUE;
	return;
    }
    for (tag = 0; tag < MAX_TAG; tag++) {
	disable_done = FALSE;
	if (ap->tm_bufs[tag] == NULL)
	    continue;
	/* get here when there is an active buffer tag to handle */
	p = ap->tm_bufs[tag];
	if (!(p->owner_flag & TM_ENABLED) ||
	    (ap->dev[p->owner_id + IMDEVICES].opened == 0)) {

	    /* check if owner is opened is to prevent INTR_LVL from */
	    /* trying to do a disable on buffers which will be disabled */
	    /* by dealloc_tgt in PROC_LVL (might already be happening) */
	    continue;	/* skip this entry */
	    /* do not set any_disabled to TRUE, as disable not sent */
	}
	while (ap->disbuf_retries < MAX_DIS_RETRIES) {
	    rc = hsc_enable_buf(ap, p, DISABLE, INTR_LVL, tag);
	    /* possible return codes are: -1 for pio err, and
	       WAITING_FOR_MB30 for mb30 cmd kicked off */
	    if (rc == WAITING_FOR_MB30) {
		any_disabled = TRUE;
		disable_done = TRUE;
		break;
	    }
	    ap->disbuf_retries++;	/* i/o error */
	}	/* end while */
	if (ap->disbuf_retries >= MAX_DIS_RETRIES) {
	    hsc_logerr(ap, ERRID_SCSI_ERR1, ap->MB30p,
		       0, 94, 0);
	    /* retries are exceeded. since it is not likely that further
	       disable buffer cmds will work, we give up on the dma err
	       recovery and exit the routine */
	    disable_done = TRUE;
	    ap->disbuf_retries = 0;
	    (void) hsc_async_notify(ap, ALL_DEVICES, SC_ADAP_CMD_FAILED);
	}
	if (disable_done == TRUE)
	    break;
    }	/* end for */
    ap->tgt_dma_err = any_disabled;
    if (ap->tgt_dma_err)
	return;
    hsc_start_bufs(ap);
}  /* end hsc_tgt_DMA_err */

/************************************************************************/
/* NAME		: hsc_async_notify					*/
/************************************************************************/
hsc_async_notify(
		 struct adapter_def * ap,
		 int dev_index,
		 int events)
{
    struct dev_info *d;
    struct sc_event_info event_info;

    bzero(&event_info, sizeof(struct sc_event_info));
    event_info.events = events;
    event_info.adap_devno = ap->devno;
    if ((dev_index < 0) || (dev_index >= DEVPOINTERS)) {
	/* notify all devices  */
	for (dev_index = 0; dev_index < DEVPOINTERS; dev_index++) {
	    d = &ap->dev[dev_index];
	    if ((d->opened == FALSE) || (d->async_func == NULL))
		continue;
	    event_info.async_correlator = d->async_correlator;
	    if (dev_index < IMDEVICES) {
		event_info.id = SID(dev_index);
		event_info.lun = LUN(dev_index);
		event_info.mode = SC_IM_MODE;
	    }
	    else {
		event_info.id = dev_index - IMDEVICES;
		event_info.lun = 0;
		event_info.mode = SC_TM_MODE;
	    }
	    (ap->dev[dev_index].async_func) (&event_info);
	} /* end for */
    }
    else {
	d = &ap->dev[dev_index];
	if ((d->opened == FALSE) || (d->async_func == NULL)) {
	    return;
	}
	event_info.async_correlator = d->async_correlator;
	if (dev_index < IMDEVICES) {
	    event_info.id = SID(dev_index);
	    event_info.lun = LUN(dev_index);
	    event_info.mode = SC_IM_MODE;
	}
	else {
	    event_info.id = dev_index - IMDEVICES;
	    event_info.lun = 0;
	    event_info.mode = SC_TM_MODE;
	}
	(ap->dev[dev_index].async_func) (&event_info);
    }
}  /* end hsc_async_notify */


/************************************************************************/
/* NAME		: hsc_need_disid  					*/
/************************************************************************/
void
hsc_need_disid(
	       struct adapter_def * ap,
	       struct dev_info * user,
	       uchar id)
{
    int     i;

    /* interrupts assumed to be disabled on entry */

    /* see if an enable id is scheduled for this id */
    if (!(ap->waiting_enaids & (1 << id))) {
	/* no enable scheduled for this id */
	if (!(user->stopped)) {	/* if not stopped--enable id last sent */
	    /* schedule a temporary disable id */
	    ap->waiting_disids |= (1 << id);
	    ap->t_dis_ids |= (1 << id);
	    (void) hsc_stop_ids(ap);
	}
	/* else--disable id last sent--ignore this disable id */
    }
    else {	/* enable id is scheduled for this id */
	if ((ap->ena_id == id) && (ap->enaid_state != WAIT_TO_ENA_ID)) {
	    /* if got here, then this enable id is being sent; regardless of
	       stopped state, schedule temp disable id */
	    ap->waiting_disids |= (1 << id);
	    ap->t_dis_ids |= (1 << id);
	    (void) hsc_stop_ids(ap);
	}
	else {	/* enable id waiting to be sent--wipe it out to keep it from
		   going AFTER this disable!! */
	    ap->waiting_enaids &= ~(1 << id);	/* clear waiting id bit */
	    if ((ap->ena_id == id) && (ap->enaid_state == WAIT_TO_ENA_ID)) {
		/* this means this is the currently pending enable. */
		/* if another bit set in waiting_enaids, must set ena_id to
		   that id and leave enaid_state as is. else, if no other
		   bits set, then simply clear enaid_state */
		if (!(ap->waiting_enaids))
		    ap->enaid_state = 0;	/* clear state completely */
		else {
		    /* at least one other id waiting for enable id. find it
		       starting with id 7 (highest priority) */
		    for (i = 7; i >= 0; i--) {
			if (ap->waiting_enaids & (1 << i)) {
			    ap->ena_id = i;	/* found new id to wait on */
			    break;
			}
			/* next i */
		    }	/* end for i */
		    /* leave enaid_state alone */
		}
	    }
	    /* else--enable id waiting but is not currently pending, so, now
	       that bit is cleared, leave other enaid_state as it is */
	    /* if not stopped--enable id last sent */
	    if (!(user->stopped)) {
		/* schedule a temporary disable id */
		ap->waiting_disids |= (1 << id);
		ap->t_dis_ids |= (1 << id);
		(void) hsc_stop_ids(ap);
	    }
	    /* else--disable id last sent--ignore this disable id */
	}
    }

}  /* end hsc_need_disid */


/************************************************************************/
/* NAME		: hsc_need_enaid  					*/
/************************************************************************/
void
hsc_need_enaid(
	       struct adapter_def * ap,
	       struct dev_info * user,
	       uchar id)
{
    int     i;

    /* interrupts assumed to be disabled on entry */

    /* see if a disable id is scheduled for this id */
    if (!(ap->waiting_disids & (1 << id))) {
	/* no disable scheduled for this id */
	if (user->stopped) {	/* if stopped--disable id last sent */
	    /* schedule an enable id */
	    ap->waiting_enaids |= (1 << id);
	    (void) hsc_start_ids(ap);
	}
	/* else--enable id last sent--ignore this enable id */
    }
    else {	/* disable id is scheduled for this id */
	if ((ap->disable_id == id) && (ap->disid_state != WAIT_TO_DIS_ID)) {
	    /* if got here, then this disable id is being sent; regardless of
	       stopped state, schedule enable id */
	    ap->waiting_enaids |= (1 << id);
	    (void) hsc_start_ids(ap);
	}
	else {	/* disable id is waiting to be sent */
	    /* must wipe out the scheduled disable, ignore the */
	    /* enable id (id is already enabled)               */
	    ap->waiting_disids &= ~(1 << id);	/* clear waiting id */
	    if ((ap->disable_id == id) &&
                (ap->disid_state == WAIT_TO_DIS_ID)) {
	        /* this means this is the currently pending disable. */
	        /* if another bit set in waiting_disids, must set
                disable_id to that id and leave disid_state as is.
	        else, if no other bits set, then simply clear
	        disid_state */
	        if (!(ap->waiting_disids))
	            ap->disid_state = 0;	/* clear state completely */
		else {
		    /* at least one other id waiting for disable id. find
		    it starting with id 7 (highest priority) */
	            for (i = 7; i >= 0; i--) {
		        if (ap->waiting_disids & (1 << i)) {
		            ap->disable_id = i;	/* found new id */
			    break;
			}
			    /* next i */
                     }	/* end for i */
			/* leave disid_state alone */
	        }
            }
	    /* else--disable id waiting but is not currently pending, so,
	       now that bit is cleared, leave other disid_state as it is */
        }
    } /* end else disable id scheduled for this id */

}  /* end hsc_need_enaid */


