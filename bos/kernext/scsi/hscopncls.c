static char sccsid[] = "@(#)70	1.2.2.7  src/bos/kernext/scsi/hscopncls.c, sysxscsi, bos411, 9428A410j 1/24/94 11:20:19";
/*
 * COMPONENT_NAME: (SYSXSCSI) IBM SCSI Adapter Driver
 *
 * FUNCTIONS:	hsc_open, hsc_close
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
/* NAME:        hscopncls.c                                             */
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
/* NAME:        hsc_open                                                */
/*                                                                      */
/* FUNCTION:    Adapter Driver Open Routine                             */
/*                                                                      */
/*      This routine opens the adapter and makes it ready.  It          */
/*      allocates adapter specific structures and initializes           */
/*      appropriate fields in them.  The adapter is marked as           */
/*      opened.                                                         */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called by a process.                        */
/*      It can page fault only if called under a process                */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      devno   - device major/minor number                             */
/*      devflag - unused                                                */
/*      chan    - unused                                                */
/*      ext     - extended data; this is 0 for normal use, or           */
/*                a value of SC_DIAGNOSTIC selects diagnostic mode      */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      0       - successful                                            */
/*      EIO     - kernel service failed or invalid operation            */
/*      EPERM   - authority error                                       */
/*      EACCES  - illegal operation due to current mode (diag vs norm)  */
/*      ENOMEM  - not enough memory available                           */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      lockl           unlockl                                         */
/*      pincode         unpincode                                       */
/*      privcheck       e_sleep_thread                                  */
/*      i_init          i_clear                                         */
/*      xmalloc         xmfree                                          */
/*      dmp_add         dmp_del                                         */
/*      d_init          d_clear                                         */
/*      d_mask          d_unmask                                        */
/*      d_master        d_complete                                      */
/*      disable_lock    unlock_enable                                   */
/*      pio_assist                                                      */
/*                                                                      */
/************************************************************************/
int
hsc_open(
	 dev_t devno,
	 ulong devflag,
	 int chan,
	 int ext)
{
    int     rc, ret_code;
    int     dma_addr, dev, i, i_hash;
    struct adapter_def *ap;
    struct io_parms iop;
    ulong   old_id;
    int     old_pri, data;
    uchar   trash;

    ret_code = 0;

    DDHKWD5(HKWD_DD_SCSIDD, DD_ENTRY_OPEN, ret_code, devno, devflag,
	    chan, ext, 0);
    rc = lockl(&(hsc_lock), LOCK_SHORT);	/* serialize this */
    if (rc != LOCK_SUCC) {
	ret_code = EIO;	/* error--kernel service call failed */
	goto end;
    }

    /* search adapter list for this devno */
    /* build the hash index */
    i_hash = minor(devno) & ADAP_HASH;
    ap = adapter_ptrs[i_hash];
    while (ap != NULL) {
	if (ap->devno == devno)
	    break;
	ap = ap->next;
    }	/* endwhile */

    if ((ap == NULL) || (!ap->inited)) {
	ret_code = EIO;	/* error--adapter not inited */
	goto exit;
    }

    if ((ap->opened) && (ap->adapter_mode == DIAG_MODE)) {
	ret_code = EACCES;	/* error--cannot open, diag mode */
	goto exit;
    }

    if ((ext & SC_DIAGNOSTIC) && (ap->opened)) {
	ret_code = EACCES;	/* error--cannot open diag, already open */
	goto exit;
    }

    /* if diagnostic mode and not ras_config authority */
    if ((ext & SC_DIAGNOSTIC) && (privcheck(RAS_CONFIG) != 0)) {
	ret_code = EPERM;	/* error--bad permissions */
	goto exit;
    }

    /* if normal mode requested, and not called from kernel process, */
    /* then dev_config authority must be set or open is not allowed  */
    if (!(ext & SC_DIAGNOSTIC) && !(devflag & DKERNEL)) {
	if (privcheck(DEV_CONFIG) != 0) {
	    ret_code = EPERM;	/* error--bad permissions */
	    goto exit;
	}
    }


    if (!ap->opened) {	/* if first open to adapter */

	if (opened_adapters == 0) {

	    /* here, this is first time any adapter is open, so execute */
	    /* things which affect the driver globally.                 */
	    /* pin the driver */
	    rc = pincode(hsc_intr);

	    if (rc != 0) {
		ret_code = EIO;	/* pincode failed */
		goto exit;
	    }
	    /* define epow handler once for all adapters */
	    /* init the EPOW interrupt handler struct */
	    INIT_EPOW(&epow_struct, (int (*) ()) hsc_epow, ap->ddi.bus_id);
#ifdef _POWER_MP
            epow_struct.flags |= INTR_MPSAFE;
#endif
	    rc = i_init(&epow_struct);
	    if (rc != INTR_SUCC) {
		(void) unpincode(hsc_intr);
		ret_code = EIO;	/* error--cannot define epow hdlr */
		goto exit;
	    }

	    /* add this device driver to the master dump table */
	    rc = dmp_add(hsc_cdt_func);
	    if (rc != 0) {	/* dmp_add failed */
		i_clear(&epow_struct);	/* clear epow handler */
		(void) unpincode(hsc_intr);
		ret_code = EIO;
		goto exit;
	    }

	}	/* already 1 or more opened adapters, continue */

	/* init adapter information structure fields */
	ap->ap_lock = LOCK_AVAIL;
	/* ap->devices_in_use = 0	this is done is hsc_alloc_adap() */
	ap->any_waiting = 0;
	ap->commands_outstanding = 0;
	ap->proc_waiting = 0;
        ap->download_pending = 0;
	ap->p_scsi_id = 0;
	ap->p_lun_id = 0;
	ap->proc_results = GOOD_COMPLETION;
	ap->mb30_resid = 0;
	ap->mb30_rc = 0;
	ap->mb30_extra_stat = 0;
	ap->mb30_byte30 = 0;
	ap->mb30_byte31 = 0;
	ap->mb31_resid = 0;
	ap->mb31_rc = 0;
	ap->mb31_extra_stat = 0;
	ap->mb31_byte30 = 0;
	ap->mb31_byte31 = 0;
	ap->channel_id = 0;
	/* ap->event = EVENT_NULL   this is done in hsc_alloc_adap() */
	ap->cl_event = EVENT_NULL;
	ap->xmem_buf.aspace_id = XMEM_GLOBAL;
	/* ap->head_MB_free = &ap->MB[0]    done in hsc_alloc_adap() */
	/* ap->tail_MB_free = &ap->MB[29]   done in hsc_alloc_adap() */
	ap->head_MB_act = NULL;
	ap->tail_MB_act = NULL;
	/* ap->head_MB_wait = NULL	see below, and hsc_alloc_adap() */
	/* ap->tail_MB_wait = NULL	see below, and hsc_alloc_adap() */
	ap->head_gw_free = NULL;
	ap->tail_gw_free = NULL;
	ap->adapter_check = 0;
	/* ap->epow_state = 0	  this is done in hsc_alloc_adap() */
	/* ap->IPL_tmr_cnt = 0	  this is done in hsc_alloc_adap() */
	ap->MB30_in_use = -1;
	ap->MB30_retries = 0;
	ap->waiting_for_mb30 = FALSE;
	ap->restart_state = 0;
	ap->restart_again = FALSE;
	ap->restart_index = 0;
	ap->restart_index_validity = FALSE;
	ap->restart_retries = 0;
	ap->close_state = 0;
	ap->dump_inited = FALSE;
	ap->dump_started = FALSE;
	ap->dump_pri = 0;
	ap->cdar_scsi_ids = 0;
	/* ap->trace_enable = TRUE	   done in hsc_alloc_adap() */
	ap->time_s.tv_sec = 0;
	ap->time_s.tv_nsec = 0;
	ap->last_dev_index = 0;
	/* ap->MB30p = &ap->MB[30]      done in hsc_alloc_adap() */
	/* ap->MB31p = &ap->MB[31]	done in hsc_alloc_adap() */
	/* ap->end_flag = 0xffff        done in hsc_alloc_adap() */

	/* ap->tm_start                     done in hsc_config() */
	/* ap->num_tgt_tcws                 done in hsc_config() */
	/* ap->num_tgt_tcws_used            done in hsc_config() */
	/* ap->tgt_req_begin                done in hsc_config() */
	/* ap->tgt_req_end                  done in hsc_config() */
	/* ap->tgt_next_req                 done in hsc_config() */
	ap->num_enabled = 0;
	ap->tgt_dma_err = 0;
	ap->num_tm_devices = 0;
	ap->enabuf_state = 0;
	ap->enaid_state = 0;
	ap->disid_state = 0;
	ap->waiting_disids = 0;
	ap->waiting_enaids = 0;
	ap->enabuf_retries = 0;
	ap->disbuf_retries = 0;
	ap->enaid_retries = 0;
	ap->disid_retries = 0;
	ap->head_free_mapped = NULL;
	ap->tail_free_mapped = NULL;
	ap->head_free_unmapped = NULL;
	ap->tail_free_unmapped = NULL;
	ap->mapped_rdbufs = NULL;
	ap->unmapped_rdbufs = NULL;
	ap->tm_garbage = NULL;
        ap->reset_pending = FALSE;
        ap->epow_reset = FALSE;
	for (i = 0; i < MAX_TAG; i++)
	    ap->tm_bufs[i] = NULL;

	/* clean-up device table entries in case stop ioctl not done */
	for (i = 0; i < DEVPOINTERS; i++) {
	    ap->dev[i].opened = FALSE;
	}

	/* clear mbox wait list (close routine attempts to clear this) */
	if (ap->head_MB_wait != NULL) {
	    ap->tail_MB_free->next = ap->head_MB_wait;
	    ap->tail_MB_free = ap->tail_MB_wait;
	}
	ap->head_MB_wait = NULL;
	ap->tail_MB_wait = NULL;

	/* init the adapter interrupt handler struct */
	ap->intr_struct.next = (struct intr *) NULL;
	ap->intr_struct.handler = hsc_intr;
	ap->intr_struct.bus_type = ap->ddi.bus_type;
#ifdef _POWER_MP
	ap->intr_struct.flags = INTR_MPSAFE;
#else
	ap->intr_struct.flags = 0;
#endif
	ap->intr_struct.level = ap->ddi.int_lvl;
	ap->intr_struct.priority = ap->ddi.int_prior;
	ap->intr_struct.bid = ap->ddi.bus_id;

	/* define one intrpt per card instance */
	/* do this regardless of mode          */
	rc = i_init(&(ap->intr_struct));
	if (rc != INTR_SUCC) {
	    if (opened_adapters == 0) {
		(void) dmp_del(hsc_cdt_func);
		i_clear(&epow_struct);	/* clear epow handler */
		(void) unpincode(hsc_intr);
	    }
	    ret_code = EIO;	/* error--cannot define inrpt hdlr */
	    goto exit;
	}

	ap->opened = TRUE;	/* mark adapter opened */
	opened_adapters++;	/* inc opened adapters counter */


/************************************************************************/
/*	here, determine mode being opened in                            */
/************************************************************************/
	if (ext == SC_DIAGNOSTIC) {
	    ap->adapter_mode = DIAG_MODE;
	    ap->errlog_enable = FALSE;

	    /* now, skip down to pick up end of open logic */
	}
/************************************************************************/
/*	the following is for normal path only                           */
/************************************************************************/
	else {
	    ap->adapter_mode = NORMAL_MODE;
	    ap->errlog_enable = TRUE;
          
	    /* de-assert SCSI Bus Reset line on internal bus */
	    iop.ap = ap;
	    iop.opt = SI_RESET;
	    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
		ap->opened = FALSE;	/* mark adapter closed */
		opened_adapters--;	/* dec opened adapters counter */
		i_clear(&(ap->intr_struct));
		if (opened_adapters == 0) {
		    (void) dmp_del(hsc_cdt_func);
		    i_clear(&epow_struct);
		    (void) unpincode(hsc_intr);
		}
		ret_code = EIO;	/* error--pio operation failed */
		goto exit;
	    }

	    /* de-assert SCSI Bus Reset line on both busses */
	    iop.opt = SE_RESET;
	    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
		ap->opened = FALSE;	/* mark adapter closed */
		opened_adapters--;	/* dec opened adapters counter */
		i_clear(&(ap->intr_struct));
		if (opened_adapters == 0) {
		    (void) dmp_del(hsc_cdt_func);
		    i_clear(&epow_struct);
		    (void) unpincode(hsc_intr);
		}
		ret_code = EIO;	/* error--pio operation failed */
		goto exit;
	    }

	    /* allocate storage for adap small DMA transfer areas */
	    ap->STA[0].stap = (char *) xmalloc((uint) STA_ALLOC_SIZE,
					       (uint) PGSHIFT, pinned_heap);
	    if (ap->STA[0].stap == NULL) {
		/* xmalloc failed */
		ap->opened = FALSE;	/* mark adapter closed */
		opened_adapters--;	/* dec opened adapters counter */
		i_clear(&(ap->intr_struct));
		if (opened_adapters == 0) {
		    (void) dmp_del(hsc_cdt_func);
		    i_clear(&epow_struct);
		    (void) unpincode(hsc_intr);
		}
		ret_code = ENOMEM;	/* error--cannot get storage */
		goto exit;
	    }	/* successfully malloc'ed and pinned STA */

	    /* incr total allocated page counter */
	    num_totl_pages += STA_ALLOC_SIZE / PAGESIZE;

	    /* init the tcw mgmt table */
	    for (i = 0; i <= ap->tgt_req_end; i++)
		ap->TCW_tab[i] = 0xff;	/* mark as not in use */

	    /* init the tcw search indexes */
	    ap->next_page_req = ap->page_req_begin;
	    ap->next_large_req = ap->large_req_begin;

	    /* init small DMA transfer area mgmt table */
	    for (i = 0; i < NUM_STA; i++) {
		ap->STA[i].stap = ap->STA[0].stap + (i * ST_SIZE);
		ap->STA[i].in_use = FALSE;
	    }

	    ap->channel_id = d_init(ap->ddi.dma_lvl, DMA_INIT,
				    ap->ddi.bus_id);
	    if (ap->channel_id == DMA_FAIL) {
		/* handle failed dma service call */
		(void) xmfree((void *) ap->STA[0].stap, pinned_heap);
		num_totl_pages -= STA_ALLOC_SIZE / PAGESIZE;
		ap->opened = FALSE;	/* mark adapter closed */
		opened_adapters--;	/* dec opened adapters counter */
		i_clear(&(ap->intr_struct));
		if (opened_adapters == 0) {
		    (void) dmp_del(hsc_cdt_func);
		    i_clear(&epow_struct);
		    (void) unpincode(hsc_intr);
		}
		ret_code = EIO;	/* error--cannot initialize DMA chan */
		goto exit;
	    }

	    d_unmask(ap->channel_id);	/* enable this DMA chan */

	    /* enable card DMA */
	    iop.opt = DMA_ENABLE;
	    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
		d_mask(ap->channel_id);	/* disable this DMA chan */
		d_clear(ap->channel_id);	/* free this DMA chan */
		(void) xmfree((void *) ap->STA[0].stap, pinned_heap);
		num_totl_pages -= STA_ALLOC_SIZE / PAGESIZE;
		ap->opened = FALSE;	/* mark adapter closed */
		opened_adapters--;	/* dec opened adapters counter */
		i_clear(&(ap->intr_struct));
		if (opened_adapters == 0) {
		    (void) dmp_del(hsc_cdt_func);
		    i_clear(&epow_struct);
		    (void) unpincode(hsc_intr);
		}
		ret_code = EIO;	/* error--pio operation failed */
		goto exit;
	    }

	    /* storage allocated for adap small DMA transfer areas */

	    /* execute d_master on the small xfer area */
	    dma_addr = DMA_ADDR(ap->ddi.tcw_start_addr, ap->sta_tcw_start);
	    d_master(ap->channel_id, DMA_TYPE, ap->STA[0].stap,
		     (size_t) STA_ALLOC_SIZE, &ap->xmem_buf,
		     (char *) dma_addr);

	    /* execute d_complete on the STA to allow access to it */
	    (void) d_complete(ap->channel_id, DMA_TYPE, ap->STA[0].stap,
			      (size_t) STA_ALLOC_SIZE, &ap->xmem_buf,
			      (char *) dma_addr);

/************************************************************************/
/*	issue adapter "Set SCSI ID" command here                        */
/************************************************************************/
	    /* must disable intrpts here because an intrpt could be */
	    /* pending due to failed IPL diagnostics.  We don't want */
	    /* to see that intrpt until first adap cmd is issued.   */
	    old_pri = disable_lock(ap->ddi.int_prior, &(hsc_mp_lock));

	    /* enable card interrupts */
	    iop.opt = INT_ENABLE;
	    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
		unlock_enable(old_pri, &(hsc_mp_lock));
		iop.opt = DMA_DISABLE;
		(void) pio_assist(&iop, hsc_pio_function, hsc_pio_recov);
		d_mask(ap->channel_id);	/* disable this DMA chan */
		d_clear(ap->channel_id);	/* free this DMA chan */
		(void) xmfree((void *) ap->STA[0].stap, pinned_heap);
		num_totl_pages -= STA_ALLOC_SIZE / PAGESIZE;
		ap->opened = FALSE;	/* mark adapter closed */
		opened_adapters--;	/* dec opened adapters counter */
		i_clear(&(ap->intr_struct));
		if (opened_adapters == 0) {
		    (void) dmp_del(hsc_cdt_func);
		    i_clear(&epow_struct);
		    (void) unpincode(hsc_intr);
		}
		ret_code = EIO;	/* error--pio operation failed */
		goto exit;
	    }

	    /* set state flag for this command */
	    ap->proc_waiting = WAIT_TO_SEND_SET_ID;

	    if (ap->MB30_in_use != -1) {	/* if mb30 in use */
		ap->waiting_for_mb30 = TRUE;
	    }
	    else {	/* mb30 is free */

		/* load MB30 with "Set SCSI ID cmd" */
		ap->proc_results = 0;
		hsc_build_mb30(ap, SET_SCSI_ID, 0, 0, 0);

		/* need watchdog because adapter could be bad.          */
		/* make timeout long enough to cover still running diag. */
		ap->wdog.dog.restart = IPL_MAX_SECS;
		w_start(&ap->wdog.dog);
		ap->MB30_in_use = PROC_USING;	/* flag proc level cmd using */
		ap->proc_waiting = WAIT_FOR_SET_ID;	/* set state flag for
							   cmd */

		/* send MB30 command to adapter */
		iop.opt = WRITE_MB30;
		iop.errtype = 0;
		if ((pio_assist(&iop, hsc_pio_function, hsc_pio_recov)
		     == EIO) &&
		    (iop.errtype != PIO_PERM_IOCC_ERR)) {
		    /* if retries exhausted, and not an iocc internal error */
		    /* then gracefully back-out the open, else allow to     */
		    /* either complete or timeout                           */
		    ap->proc_waiting = 0;	/* reset state flag */
		    ap->MB30_in_use = -1;	/* free mb30 */
		    w_stop(&ap->wdog.dog);
		    /* disable card interrupts and DMA */
		    iop.opt = INT_DISABLE;
		    (void) pio_assist(&iop, hsc_pio_function, hsc_pio_recov);
		    unlock_enable(old_pri, &(hsc_mp_lock));
		    iop.opt = DMA_DISABLE;
		    (void) pio_assist(&iop, hsc_pio_function, hsc_pio_recov);
		    d_mask(ap->channel_id);	/* disable this DMA chan */
		    d_clear(ap->channel_id);	/* free this DMA chan */
		    (void) xmfree((void *) ap->STA[0].stap, pinned_heap);
		    num_totl_pages -= STA_ALLOC_SIZE / PAGESIZE;
		    ap->opened = FALSE;	/* mark adapter closed */
		    opened_adapters--;	/* dec opened adapters counter */
		    i_clear(&(ap->intr_struct));
		    if (opened_adapters == 0) {
			(void) dmp_del(hsc_cdt_func);
			i_clear(&epow_struct);
			(void) unpincode(hsc_intr);
		    }
		    ret_code = EIO;	/* error--pio operation failed */
		    goto exit;
		}

	    }


	    e_sleep_thread(&ap->event, &(hsc_mp_lock), LOCK_HANDLER);
	    unlock_enable(old_pri, &(hsc_mp_lock));
	    /* N.B. if an IPL diag failure occurred, control will pass */
	    /* to the intrpt handler, which will log an error, set     */
	    /* proc_results, and return to this point.                 */
	    /* OR  a timeout will occur, also returning here.          */
	    if (ap->proc_results != GOOD_COMPLETION) {
		/* error--handle bad adapter */
		if (ap->proc_results == SEE_RC_STAT) {
		    /* unknown card status--log adap error */
		    hsc_logerr(ap, ERRID_SCSI_ERR3, ap->MB30p,
			       UNKNOWN_CARD_ERR, 1, 0);
		}
		/* clean-up previous stuff */
		/* disable card interrupts */
		iop.opt = INT_DISABLE;
		(void) pio_assist(&iop, hsc_pio_function, hsc_pio_recov);
		/* disable card DMA */
		iop.opt = DMA_DISABLE;
		(void) pio_assist(&iop, hsc_pio_function, hsc_pio_recov);
		d_mask(ap->channel_id);	/* disable this DMA chan */
		d_clear(ap->channel_id);	/* free this DMA chan */
		(void) xmfree((void *) ap->STA[0].stap, pinned_heap);
		num_totl_pages -= STA_ALLOC_SIZE / PAGESIZE;
		ap->opened = FALSE;	/* mark adapter closed */
		opened_adapters--;	/* dec opened adapters counter */
		i_clear(&(ap->intr_struct));
		if (opened_adapters == 0) {
		    (void) dmp_del(hsc_cdt_func);
		    i_clear(&epow_struct);
		    (void) unpincode(hsc_intr);
		}
		ret_code = EIO;	/* error--adapter bad */
		goto exit;

	    }	/* MB30 command was successful */


/************************************************************************/
/*	issue adapter "Restart" command here                            */
/************************************************************************/
	    /* disable interrupts around sending cmd and e_sleep call */
	    old_pri = disable_lock(ap->ddi.int_prior, &(hsc_mp_lock));

	    /* set state flag for this command */
	    ap->proc_waiting = WAIT_TO_SEND_RESTART;

	    if (ap->MB30_in_use != -1) {	/* if mb30 in use */
		ap->waiting_for_mb30 = TRUE;
	    }
	    else {	/* mb30 is free */

		ap->wdog.dog.restart = ADAP_CMD_T_O;
		w_start(&ap->wdog.dog);
		ap->MB30_in_use = PROC_USING;	/* flag proc level cmd using */
		ap->proc_waiting = WAIT_FOR_RESTART;	/* set command state */

		/* load MB30 with RESTART cmd (head of free list is mbox #) */
		ap->proc_results = 0;
		hsc_build_mb30(ap, RESTART, 0, 0, 0);

		/* send MB30 command to adapter */
		iop.opt = WRITE_MB30;
		iop.errtype = 0;
		if ((pio_assist(&iop, hsc_pio_function, hsc_pio_recov)
		     == EIO) &&
		    (iop.errtype != PIO_PERM_IOCC_ERR)) {
		    /* if retries exhausted, and not an iocc internal error */
		    /* then gracefully back-out the open, else allow to     */
		    /* either complete or timeout.                          */
		    ap->proc_waiting = 0;	/* reset state flag */
		    ap->MB30_in_use = -1;	/* free mb30 */
		    unlock_enable(old_pri, &(hsc_mp_lock));
		    w_stop(&ap->wdog.dog);
		    iop.opt = INT_DISABLE;
		    (void) pio_assist(&iop, hsc_pio_function, hsc_pio_recov);
		    iop.opt = DMA_DISABLE;
		    (void) pio_assist(&iop, hsc_pio_function, hsc_pio_recov);
		    d_mask(ap->channel_id);	/* disable this DMA chan */
		    d_clear(ap->channel_id);	/* free this DMA chan */
		    (void) xmfree((void *) ap->STA[0].stap, pinned_heap);
		    num_totl_pages -= STA_ALLOC_SIZE / PAGESIZE;
		    ap->opened = FALSE;	/* mark adapter closed */
		    opened_adapters--;	/* dec opened adapters counter */
		    i_clear(&(ap->intr_struct));
		    if (opened_adapters == 0) {
			(void) dmp_del(hsc_cdt_func);
			i_clear(&epow_struct);
			(void) unpincode(hsc_intr);
		    }
		    ret_code = EIO;	/* error--pio operation failed */
		    goto exit;
		}
	    }


	    e_sleep_thread(&ap->event, &(hsc_mp_lock), LOCK_HANDLER);
	    unlock_enable(old_pri, &(hsc_mp_lock));
	    /* N.B. if an error occurs, control will pass to the  */
	    /* intrpt handler, which will log an error, set       */
	    /* proc_results, and return to this point.            */
	    /* OR  a timeout will occur, also returning here.     */
	    if (ap->proc_results != GOOD_COMPLETION) {
		/* error--handle bad adapter */
		if (ap->proc_results == SEE_RC_STAT) {
		    /* unknown card status--log adap error */
		    hsc_logerr(ap, ERRID_SCSI_ERR3, ap->MB30p,
			       UNKNOWN_CARD_ERR, 2, 0);
		}
		/* clean-up previous stuff */
		/* disable card interrupts */
		iop.opt = INT_DISABLE;
		(void) pio_assist(&iop, hsc_pio_function, hsc_pio_recov);
		/* disable card DMA */
		iop.opt = DMA_DISABLE;
		(void) pio_assist(&iop, hsc_pio_function, hsc_pio_recov);
		d_mask(ap->channel_id);	/* disable this DMA chan */
		d_clear(ap->channel_id);	/* free this DMA chan */
		(void) xmfree((void *) ap->STA[0].stap, pinned_heap);
		num_totl_pages -= STA_ALLOC_SIZE / PAGESIZE;
		ap->opened = FALSE;	/* mark adapter closed */
		opened_adapters--;	/* dec opened adapters counter */
		i_clear(&(ap->intr_struct));
		if (opened_adapters == 0) {
		    (void) dmp_del(hsc_cdt_func);
		    i_clear(&epow_struct);
		    (void) unpincode(hsc_intr);
		}
		ret_code = EIO;	/* error--adapter bad */
		goto exit;

	    }	/* MB30 command was successful */

	    /* reserve TCWs for the small xfer area in the tcw table */
	    for (i = ap->sta_tcw_start;
		 i < (ap->sta_tcw_start + NUM_STA_TCWS); i++)
		ap->TCW_tab[i] = 0x77;

	}	/* end of non-diagnostic mode open logic */

/************************************************************************/
/*	continue here for either normal or diag path                    */
/************************************************************************/

	/* don't do other adapter commands during open in diag mode */
	/* (may still need to download microcode!!)                 */



    }	/* continue, adapter already opened */

exit:
    unlockl(&(hsc_lock));	/* release the lock */
end:
    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_OPEN, ret_code, devno);
    return (ret_code);

}  /* end hsc_open */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_close                                               */
/*                                                                      */
/* FUNCTION:    Adapter Driver Close Routine                            */
/*                                                                      */
/*      This routine closes the adapter and marks it as not             */
/*      opened.                                                         */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called by a process.                        */
/*      It can page fault only if called under a process                */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      devno   - device major/minor number                             */
/*      offchan - 0; unused                                             */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      0       - successful                                            */
/*      EIO     - kernel service failed or invalid operation            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      lockl           unlockl                                         */
/*      disable_lock    unlock_enable                                   */
/*      e_sleep_thread                                                  */
/*      d_mask          d_clear                                         */
/*      unpincode       xmfree                                          */
/*      i_clear         pio_assist                                      */
/*                                                                      */
/************************************************************************/
int
hsc_close(
	  dev_t devno,
	  int offchan)
{
    struct adapter_def *ap;
    struct io_parms iop;
    int     ret_code, rc;
    int     i_hash, i;
    int     flag, old_pri;
    uchar   trash;
    int     t_index;
    struct sc_stop_tgt stop_tgt;

    ret_code = 0;	/* default to good completion */

    DDHKWD1(HKWD_DD_SCSIDD, DD_ENTRY_CLOSE, ret_code, devno);
    rc = lockl(&(hsc_lock), LOCK_SHORT);	/* serialize this */
    if (rc != LOCK_SUCC) {
	ret_code = EIO;	/* error--kernel service call failed */
	goto end;
    }

    /* search adapter list for this devno */
    /* build the hash index */
    i_hash = minor(devno) & ADAP_HASH;
    ap = adapter_ptrs[i_hash];
    while (ap != NULL) {
	if (ap->devno == devno)
	    break;
	ap = ap->next;
    }	/* endwhile */

    if ((ap == NULL) || (!ap->inited) || (!ap->opened)) {
	ret_code = EIO;	/* error--adapter not inited */
	goto exit;
    }

    /* init iop structure */
    bzero(&iop, sizeof(struct io_parms));

    /* keep intrpts out */
    old_pri = disable_lock(ap->ddi.int_prior, &(hsc_mp_lock));	

    /* the following assures all future requests are rejected,  */
    /* and all currently queued requests are completed.         */
    /* note that, worst case, all current commands may have to  */
    /* timeout before all commands will be complete.            */
    while (ap->commands_outstanding) {
	ap->close_state = CLOSE_PENDING;
        e_sleep_thread(&ap->cl_event, &(hsc_mp_lock), LOCK_HANDLER);
    }	/* endwhile */

    /* the following waits for waiting mailboxes to be freed.    */
    /* note that if the adapter is not posting interrupts for    */
    /* these mailboxes, this must wait until the mailbox timers  */
    /* pop to free all waiting mailboxes.                        */
    while (ap->head_MB_wait != NULL) {
	ap->close_state = CLOSE_PENDING;
        e_sleep_thread(&ap->cl_event, &(hsc_mp_lock), LOCK_HANDLER);
    }	/* endwhile */

    ap->adapter_check = 0;	/* reset ADAPTER_DEAD state */
    ap->close_state = 0;	/* reset CLOSE_PENDING state */
    ap->opened = FALSE;	/* mark adapter as closed */

    unlock_enable(old_pri, &(hsc_mp_lock)) ;	/* allow interrupts again */

    if (ap->adapter_mode == NORMAL_MODE) {
	iop.ap = ap;
	/* disable card DMA */
	iop.opt = DMA_DISABLE;
	(void) pio_assist(&iop, hsc_pio_function, hsc_pio_recov);

        /* deallocate and close all target devices			 */
        bzero((caddr_t) & stop_tgt, sizeof(struct sc_stop_tgt));
        for (t_index = IMDEVICES; t_index < DEVPOINTERS; t_index++) {
	    if (ap->dev[t_index].opened) {
	        ap->dev[t_index].head_pend = NULL;
	        ap->dev[t_index].tail_pend = NULL;
	        stop_tgt.id = t_index - IMDEVICES;
	        (void) hsc_dealloc_tgt(ap, &stop_tgt, DKERNEL);
	    }
        }
        /* now, try to free tm_garbage, if any */
        (void) hsc_free_tm_garbage(ap);

	/* disable card interrupts */
	iop.opt = INT_DISABLE;
	(void) pio_assist(&iop, hsc_pio_function, hsc_pio_recov);
	for (i = ap->sta_tcw_start;
	     i < (ap->sta_tcw_start + NUM_STA_TCWS); i++)
	    ap->TCW_tab[i] = 0xff;	/* free STA's TCWs */

	d_mask(ap->channel_id);	/* disable this DMA chan */
	d_clear(ap->channel_id);	/* free this DMA chan */
	/* free the small transfer area */
	(void) xmfree((void *) ap->STA[0].stap, pinned_heap);
	num_totl_pages -= STA_ALLOC_SIZE / PAGESIZE;
    }

    /* for any/all modes.. */
    i_clear(&(ap->intr_struct));
    opened_adapters--;	/* decrement opened adapters counter */
    if (opened_adapters == 0) {	/* if this is the last close */
	(void) dmp_del(hsc_cdt_func);
	i_clear(&epow_struct);
	(void) unpincode(hsc_intr);
    }



exit:
    unlockl(&(hsc_lock));	/* release the lock */
end:
    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_CLOSE, ret_code, devno);
    return (ret_code);

}  /* end hsc_close */
