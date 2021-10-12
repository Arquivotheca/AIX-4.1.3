static char sccsid[] = "@(#)71	1.4.3.6  src/bos/kernext/scsi/hscstrat.c, sysxscsi, bos411, 9428A410j 5/5/94 10:59:52";
/*
 * COMPONENT_NAME: (SYSXSCSI) IBM SCSI Adapter Driver
 *
 * FUNCTIONS:	hsc_strategy, hsc_start, hsc_start_dev,
 *		hsc_build_and_send, hsc_issue
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
/* NAME:        hscstrat.c                                              */
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
/* NAME:        hsc_strategy                                            */
/*                                                                      */
/* FUNCTION:    Adapter Driver Strategy Routine                         */
/*                                                                      */
/*      This routine validates the passed command and places it on      */
/*      the appropriate device queue.  The hsc_start routine is called  */
/*      to further process the request.                                 */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called by a process.                        */
/*      It can page fault only if called under a process                */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* NOTES:                                                               */
/*      The structure pointed to by the input parameter is not          */
/*      simply a buffer structure.  The beginning of the structure      */
/*      is exactly the same as a buffer structure, but there are        */
/*      fields appended to the end of the buffer structure              */
/*      which are required to initiate the SCSI command.                */
/*                                                                      */
/*      N.B.:  the av_forw field must be NULL here, chained sc_bufs     */
/*             are not supported.                                       */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      sc_buf  - input/output request struct used between the adapter  */
/*                driver and the calling SCSI device driver             */
/*                                                                      */
/* INPUTS:                                                              */
/*      bp      - pointer to the passed sc_buf                          */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      (these values are placed in the b_error field)                  */
/*      EIO     - a fatal error of some type occurred                   */
/*      ENXIO   - the request cannot be accepted because queue is       */
/*                halted                                                */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      disable_lock       unlock_enable                                */
/*      iodone                                                          */
/************************************************************************/
int
hsc_strategy(
	     struct sc_buf * bp)
{
    struct adapter_def *ap;
    struct io_parms iop;
    int     i_hash;
    int     ret_code;
    int     dev_index;
    int     new_pri;
    dev_t   devno;
    struct gwrite *current_gw_free, *back_ptr;
    struct gwrite *head_to_free, *save_ptr;
    int     i, rc, required_size;

    ret_code = 0;	/* default to no errors found  */

    devno = bp->bufstruct.b_dev;
    DDHKWD5(HKWD_DD_SCSIDD, DD_ENTRY_STRATEGY, ret_code, devno, bp,
	    bp->bufstruct.b_flags, bp->bufstruct.b_blkno,
	    bp->bufstruct.b_bcount);
    /* search adapter list for this devno */
    /* build the hash index */
    i_hash = minor(bp->bufstruct.b_dev) & ADAP_HASH;
    ap = adapter_ptrs[i_hash];
    while (ap != NULL) {
	if (ap->devno == bp->bufstruct.b_dev)
	    break;
	ap = ap->next;
    }	/* endwhile */

    if ((ap == NULL) || (!ap->opened)) {
	ret_code = EIO;	/* error--adapter not inited/opened */
	bp->bufstruct.b_error = EIO;
	bp->bufstruct.b_flags |= B_ERROR;
	bp->bufstruct.b_resid = bp->bufstruct.b_bcount;	/* none sent */
	bp->status_validity = SC_ADAPTER_ERROR;	/* flag problem */
	bp->general_card_status = SC_ADAPTER_SFW_FAILURE;
	iodone((struct buf *) bp);
	goto end;
    }

    /* keep intrpt hdlr out */
    new_pri = disable_lock(ap->ddi.int_prior, &(hsc_mp_lock));

    /* get index into device table for this device */

    dev_index = INDEX(bp->scsi_command.scsi_id,
		      (bp->scsi_command.scsi_cmd.lun) >> 5);

    /* miscellaneous validation of request */
    if ((!ap->dev[dev_index].opened) ||
	(bp->bufstruct.b_bcount > ap->maxxfer) ||
	(ap->dev[dev_index].qstate == STOP_PENDING)) {
	ret_code = EIO;	/* error--bad command */
	bp->bufstruct.b_error = EIO;
	bp->bufstruct.b_flags |= B_ERROR;
	bp->bufstruct.b_resid = bp->bufstruct.b_bcount;	/* none sent */
	bp->status_validity = SC_ADAPTER_ERROR;	/* flag problem  */
	bp->general_card_status = SC_ADAPTER_SFW_FAILURE;
	iodone((struct buf *) bp);
	goto exit;
    }

    /* if fatal hardware problem exists, shunt requests here */
    if ((ap->adapter_check != 0) || (ap->close_state != 0)) {
	ret_code = EIO;	/* error--adapter dead or close pending */
	bp->bufstruct.b_error = EIO;
	bp->bufstruct.b_flags |= B_ERROR;
	bp->bufstruct.b_resid = bp->bufstruct.b_bcount;	/* none sent */
	bp->status_validity = SC_ADAPTER_ERROR;	/* flag problem  */
	bp->general_card_status = SC_ADAPTER_HDW_FAILURE;
	iodone((struct buf *) bp);
	goto exit;
    }

    /* if device queue is halted, and sc_buf resume flag not set */
    /* then fail the command                                     */
    if ((ap->dev[dev_index].qstate == HALTED) &&
	!(bp->flags & SC_RESUME)) {
	ret_code = ENXIO;	/* error--device halted */
	bp->bufstruct.b_error = ENXIO;
	bp->bufstruct.b_flags |= B_ERROR;
	bp->bufstruct.b_resid = bp->bufstruct.b_bcount;	/* none sent */
	bp->status_validity = 0;
	iodone((struct buf *) bp);
	goto exit;
    }

    /* handle gathered writes here */
    if (bp->resvd1 != NULL) {	/* do gathered writes */
	if ((bp->bufstruct.b_flags & B_READ) ||	/* if a read */
	    (bp->bp != NULL) || (getpid() < 0)) {	/* or not proc level */
	    ret_code = EINVAL;	/* error--bad command */
	    bp->bufstruct.b_error = EINVAL;
	    bp->bufstruct.b_flags |= B_ERROR;
	    bp->bufstruct.b_resid = bp->bufstruct.b_bcount;	/* none sent */
	    bp->status_validity = SC_ADAPTER_ERROR;	/* flag problem  */
	    bp->general_card_status = SC_ADAPTER_SFW_FAILURE;
	    iodone((struct buf *) bp);
	    goto exit;
	}
	/* this is a valid gathered write on the process level */

	/*
	   A brief summary of gathered write implementation:

	   An adapter free list is maintained which will contain
	   previously allocated kernel_heap buffers, if any, for
	   gathered writes.  In strategy routine, we examine the
	   list and if empty, malloc and pin a buffer for transfer.
	   The buffer size is the next 8KB boundary which will satisfy
	   the requested size (and is on page boundaries).

	   If not empty, we search for a buffer which is in the same
	   8KB-multiple size range.  Buffers found which are bigger
	   than 8KB but not in the same 8KB-multiple range are freed.
	   Once a buffer is found to be acceptable in size, we use it
	   and exit the list, leaving buffers which follow.  If none
	   are found that are correct, all >8KB end up freed and a
	   buffer is malloced and pinned.

	   Logic converges when we copy the user data to the buffer
	   and continue from there through normal command issue path.

	   When the interrupt comes in, hsc_dma_cleanup sees the gathered
	   write, and after d_complete, simply puts the buf on the adap
	   free list.  Note that the buffer cannot be freed here in any
	   case, since xmfree cannot be invoked on the intrpt level.
	   Free buffers are always put on the end of the free list.
	   This way, it is likely the list will be run to the end,
	   allowing larger buffers to be freed.  Note that TCWs are
	   given up after each transfer, to avoid fragmenting or otherwise
	   monopolizing the initiator large transfer TCWs.  (Could hang
	   if TCWs are not freed.)

	   As an added measure of freeing unused buffers, SCIOSTOP ioctl
	   will run the free list and free ALL buffers it finds.

	*/

	/* calculate nominal size of buffer required for transfer.
	   this algorithm uses 8KB granularity (note this is itself
	   a multiple of the page size) */
	required_size = (((bp->bufstruct.b_bcount - 1) / (2 * LPAGESIZE)) + 1)*
			(2 * LPAGESIZE);

	/* set local pointer to head of free list */
	current_gw_free = ap->head_gw_free;

	back_ptr = current_gw_free;
	head_to_free = NULL;	/* init list which will collect unused bufs */

	/* loop while gwrite free list is non-empty */
	while (current_gw_free) {


	    if ((current_gw_free->buf_size == required_size) ||
	        (current_gw_free->buf_size > (2 * LPAGESIZE))) {
		/* either correct size, or needs freeing; take off free list */
		if (current_gw_free == ap->head_gw_free) {
		    ap->head_gw_free = current_gw_free->next;
		    if (!ap->head_gw_free) {	/* if list empty, sync tail */
			ap->tail_gw_free = NULL;
		    }
		}
		else {
		    if (current_gw_free == ap->tail_gw_free) {
			ap->tail_gw_free = back_ptr;
			back_ptr->next = NULL;
		    }
		    else {	/* neither head or tail */
			back_ptr->next = current_gw_free->next;
		    }
		}


		if (current_gw_free->buf_size == required_size) {
		    /* leave while.  do not process rest of buffers */
		    break;
		}
		else {
	    	    /* bigger than minimum buffer size, so add to temp list */
		    save_ptr = current_gw_free->next;
		    current_gw_free->next = head_to_free;
		    head_to_free = current_gw_free;
		    current_gw_free = save_ptr;
		    /*back_ptr = current_gw_free;*/
		    continue;	/* now loop */
		}
	    }

	    /* if got here, buffer is not of required size, but should be
	       kept.  */

	    /* update pointers */
	    back_ptr = current_gw_free;
	    current_gw_free = back_ptr->next;

	    /* now loop again */

	} /* end while */

	/* enable interrupts during the buffer freeing/mallocing */
    	unlock_enable(new_pri, &(hsc_mp_lock));

	/* see if any buffers are to be freed */
	while (head_to_free) {
	    save_ptr = head_to_free;
	    head_to_free = head_to_free->next;
	    /* unpin and free buffer first */
	    (void) unpin(save_ptr->buf_addr, save_ptr->buf_size);
	    (void) xmfree(save_ptr->buf_addr, kernel_heap);

	    /* unpin and free gwrite struct */
	    (void) unpin(save_ptr, sizeof (struct gwrite));
	    (void) xmfree((caddr_t) save_ptr, kernel_heap);

	} /* end while */

	/* if current_gw_free NULL here, must need to malloc new buffer */
	if (!current_gw_free) {
	    /* malloc and pin gwrite struct */
	    current_gw_free = (struct gwrite *) xmalloc((int) sizeof (struct
								      gwrite),
							(int) 4,
							kernel_heap);
	    /* if got struct, then pin it */
	    if ((!current_gw_free) || (pin((caddr_t) current_gw_free,
					   sizeof (struct gwrite)))) {
		/* either malloc or pin failed */
		if (current_gw_free) {
		    /* pin must have failed, free struct */
	    	    (void) xmfree((caddr_t) current_gw_free, kernel_heap);
		}
		/* error exit here */
		ret_code = ENOMEM;
		bp->bufstruct.b_error = ENOMEM;
		bp->bufstruct.b_flags |= B_ERROR;
		bp->bufstruct.b_resid = bp->bufstruct.b_bcount;	/* none sent */
		bp->status_validity = SC_ADAPTER_ERROR;	/* flag problem  */
		bp->general_card_status = SC_ADAPTER_SFW_FAILURE;
		iodone((struct buf *) bp);
		goto end;	/* exit at end since intrpts not disabled */
	    }
	    /* initialize gwrite struct */
	    bzero((caddr_t) current_gw_free, sizeof (struct gwrite));

	    /* malloc and pin buffer */
	    current_gw_free->buf_addr = (caddr_t) xmalloc((int) required_size,
							 (int) PGSHIFT,
							 kernel_heap);
	    /* if got buffer, then pin it */
	    if ((!(current_gw_free->buf_addr)) || (pin((caddr_t)
						   current_gw_free->buf_addr,
					           (int) required_size))) {
		/* either malloc or pin failed */
		if (current_gw_free->buf_addr) {
		    /* pin must have failed, free buffer */
	    	    (void) xmfree((caddr_t) current_gw_free->buf_addr,
				  kernel_heap);
	    	    /* unpin and free gwrite struct */
	    	    (void) unpin((caddr_t) current_gw_free,
				 sizeof (struct gwrite));
	    	    (void) xmfree((caddr_t) current_gw_free, kernel_heap);
		}
		/* error exit here */
		ret_code = ENOMEM;
		bp->bufstruct.b_error = ENOMEM;
		bp->bufstruct.b_flags |= B_ERROR;
		bp->bufstruct.b_resid = bp->bufstruct.b_bcount;	/* none sent */
		bp->status_validity = SC_ADAPTER_ERROR;	/* flag problem  */
		bp->general_card_status = SC_ADAPTER_SFW_FAILURE;
		iodone((struct buf *) bp);
		goto end;	/* exit at end since intrpts not disabled */
	    }

	    /* initialize the gwrite struct here */
	    current_gw_free->buf_size = required_size;

	}

	/* move user data to the gwrite buffer */
	rc = uiomove((caddr_t) current_gw_free->buf_addr,
		     (int) current_gw_free->buf_size,
		     UIO_WRITE,
		     (struct uio *) bp->resvd1);
	if (rc) {
	    /* handle error on uiomove */
	    ret_code = EFAULT;
	    bp->bufstruct.b_error = EFAULT;
	    bp->bufstruct.b_flags |= B_ERROR;
	    bp->bufstruct.b_resid = bp->bufstruct.b_bcount;	/* none sent */
	    bp->status_validity = SC_ADAPTER_ERROR;	/* flag problem  */
	    bp->general_card_status = SC_ADAPTER_SFW_FAILURE;
	    iodone((struct buf *) bp);
	    goto end;	/* exit at end since intrpts not disabled */
	}
	/* setup buffer address in bufstruct */
	bp->bufstruct.b_un.b_addr = current_gw_free->buf_addr;

	/* indicate this is kernel global memory */
	bp->bufstruct.b_xmemd.aspace_id = XMEM_GLOBAL;

	/* save gwrite pointer in bp->resvd1. uio struct ptr over written */
	bp->resvd1 = (uint) current_gw_free;

	/* disable interrupts again */
    	new_pri = disable_lock(ap->ddi.int_prior, &(hsc_mp_lock));

    }

    /* if we get here, device is not halted anymore */
    ap->dev[dev_index].qstate = 0;	/* normal queue state */
    if ((bp->q_tag_msg) && (ap->enable_queuing))
     	ap->dev[dev_index].dev_queuing = TRUE;

    bp->resvd7 = 0;                     /* ensure a zero value */
    bp->bufstruct.av_forw = NULL;	/* only accept one cmd */
    bp->bufstruct.av_back = NULL;	/* only accept one cmd */
    bp->bufstruct.b_work = NULL;	/* init to "no MB"     */
    bp->bufstruct.b_error = 0;	/* init to "no error"  */
    bp->bufstruct.b_flags &= ~B_ERROR;	/* init to "no error"  */
    bp->bufstruct.b_resid = 0;	/* init to "no error"  */
    bp->status_validity = 0;	/* init to "no error"  */
    bp->general_card_status = 0;	/* init to "no error"  */
    bp->scsi_status = 0;	/* init to "no error"  */

    /* see if this sc_buf is the error recovery command for a check condition*/
    /* while command tag queuing.  If so, place it at the HEAD of the        */
    /* pending queue and set expedite flag so that it can be made active     */
    /* while the queue is halted (pqstate == PENDING_ERROR)                  */
    if ((ap->dev[dev_index].cc_error_state == WAIT_FOR_RECOVERY_CMD) &&
	(bp->flags & SC_RESUME) && (!(bp->flags & (SC_Q_CLR | SC_Q_RESUME))) && 
        (!(bp->q_tag_msg))) {
        /* set resvd7 to TRUE so that this command can be issued to the    */
        /* adapter while the device queue is halted */
        bp->resvd7 = TRUE;
        /* enqueue the command to the head of device pending queue */
        if (ap->dev[dev_index].head_pend == NULL) {	/* if queue empty  */
            ap->dev[dev_index].head_pend = bp;	/* point head at new one */
	    ap->dev[dev_index].tail_pend = bp;	/* point tail at new one */
        }
        else {
           bp->bufstruct.av_forw = (struct buf *)ap->dev[dev_index].head_pend;
           ap->dev[dev_index].head_pend = (struct sc_buf *) bp; 
        }
    } /* end if */
    /* enqueue the command to the tail of device pending queue */
    else if (bp->scsi_command.scsi_length) { /* this sc_buf has a scsi cmd */
            if (ap->dev[dev_index].head_pend == NULL) {	/* if queue empty  */
	    ap->dev[dev_index].head_pend = bp;	/* point head at new one */
	    ap->dev[dev_index].tail_pend = bp;	/* point tail at new one */
            }
            else {	/* pending queue not empty */
	         /* point last cmd's av_forw at the new request */
	         ap->dev[dev_index].tail_pend->bufstruct.av_forw = 
                                       (struct buf *) bp;
	         ap->dev[dev_index].tail_pend = bp; /* point tail at new one */
             }
         }

    /* if awaiting check condition error recovery and this sc_buf has    */
    /* SC_Q_CLR set, then issue init lun to adapter to clear the queue   */
    /* for this device.  When init lun completes, hsc_fail_cmd is called */
    /* to clear the pending and active queues for this device.           */
    if (bp->flags & SC_Q_CLR)  {
        if (!(ap->dev[dev_index].cc_error_state == WAIT_FOR_RECOVERY_CMD)) {
        /* this  is an unexpected SC_Q_CLR request, ignore it and iodone */
        /* the sc_buf if it has no scsi cmd. If it has a scsi command    */
        /* then this sc_buf was placed on the pending queue and will be  */
        /* executed but the SC_Q_CLR will be ignored                     */
            /*this sc_buf has no scsi cmd*/
            if (!(bp->scsi_command.scsi_length))  {
	        iodone((struct buf *) bp);
                goto exit;
            }
        }
        else {
            /* if sc_buf does not have a scsi command it was not put on the  */
            /* pending queue.  If a queue clear was requested then put the   */
            /* sc_buf on the pending queue if it is not already there.  This */
            /* is ok if there is no scsi command since this queue will be    */
            /* failed without trying to send it to the adapter.              */

            /*this sc_buf has no scsi cmd */
            if (!(bp->scsi_command.scsi_length)){ 
                if (ap->dev[dev_index].head_pend == NULL) {/* if queue empty */
                   /* point head at new one */
	            ap->dev[dev_index].head_pend = bp;
                    /* point tail at new one */
	            ap->dev[dev_index].tail_pend = bp;
                }
                else {	/* pending queue not empty */
	             /* point last cmd's av_forw at the new request */
	             ap->dev[dev_index].tail_pend->bufstruct.av_forw = 
                                           (struct buf *) bp;
                     /* point tail at new one */
	             ap->dev[dev_index].tail_pend = bp;
                }
            }
            if (ap->dev[dev_index].state == 0) {
                ap->dev[dev_index].state = WAIT_TO_SEND_INIT_LUN;
                if (ap->MB30_in_use != -1) {    /* MB30 in use ? */
                    ap->waiting_for_mb30 = TRUE;
                }
                else {  /* MB30 is free */
                    ap->dev[dev_index].state = WAIT_FOR_INIT_LUN;
                    ap->MB30_in_use = dev_index;
                    /* build MB30 INIT LUN cmd */
                    hsc_build_mb30(ap, INITIALIZE, 0, (0x01 << SID(dev_index)),
                                   LUN(dev_index));
                    ap->wdog.dog.restart = INIT_CMD_T_O;
                    w_start(&ap->wdog.dog);     /* start timer */

                    /* send MB30 command to adapter */
                    iop.ap = ap;
                    iop.opt = WRITE_MB30;
                    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) 
                        == EIO) {
                        /* handle unrecovered error sending mbox 30.  */
                        /* ignore error here, allowing timeout to hit. */
                        ;
                    }

                } /* end else */
            } /* end if state==0 */
        } /* end else */
    } /* end if */

    /* if the sc_buf did not have a SC_Q_CLR indication then check to see   */
    /* if it had a SC_Q_RESUME indication, meaning a init lun with the      */
    /* resume bit set should be sent                                        */
    else if(bp->flags & SC_Q_RESUME) {
             if (!(bp->scsi_command.scsi_length)){ /* sc_buf does not have */
                                                   /* a scsi command */
                 ap->dev[dev_index].cmd_save_ptr_res = bp;
             }
             if (ap->dev[dev_index].cc_error_state == WAIT_FOR_RECOVERY_CMD){
                 ap->dev[dev_index].cc_error_state = 0;
             } 
             ap->dev[dev_index].state = WAIT_TO_SEND_RESUME;
             if (ap->MB30_in_use != -1) {    /* MB30 in use ? */
                 ap->waiting_for_mb30 = TRUE;
             }
             else {  /* MB30 is free */
                 ap->dev[dev_index].state = WAIT_FOR_RESUME;
                 ap->MB30_in_use = dev_index;
                 /* build MB30 INIT LUN cmd */
                 hsc_build_mb30(ap, INITIALIZE, 2, (0x01 << SID(dev_index)),
                           LUN(dev_index));
                 ap->wdog.dog.restart = INIT_CMD_T_O;
                 w_start(&ap->wdog.dog);     /* start timer */

                 /* send MB30 command to adapter */
                 iop.ap = ap;
                 iop.opt = WRITE_MB30;
                 if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) 
                     == EIO) {
                     /* handle unrecovered error sending mbox 30.  */
                     /* ignore error here, allowing timeout to hit. */
                     ;
                 }

             } /* end else */
         } /* end if bp->flags & SC_Q_RESUME */

    /* handle initiator mode requests here */
    ap->commands_outstanding++;	/* inc total cmd counter */

    /* if any waiting for resources, no reason to kick this one off */
    if (ap->any_waiting) {
	/* if this device is not waiting, put in waiting state to   */
	/* ensure it gets seen in a later pass thru hsc_start       */
	if (!ap->dev[dev_index].waiting) {
	    ap->any_waiting++;
	    ap->dev[dev_index].waiting = TRUE;
	}
    }
    else {
	hsc_start(ap, dev_index);	/* call start to see if this cmd can
					   be kicked off     */
    }


exit:
    unlock_enable(new_pri, &(hsc_mp_lock)); /* let interrupt handler back in */
end:
    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_STRATEGY, ret_code, devno);
    return (ret_code);

}  /* end hsc_strategy */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_start                                               */
/*                                                                      */
/* FUNCTION:    Adapter Driver Start I/O Routine                        */
/*                                                                      */
/*      This routine intiates I/O to the adapter which has been         */
/*      scheduled by the hsc_strategy routine.  It also initiates       */
/*      the next I/O, if any, when called by the hsc_intr routine.      */
/*                                                                      */
/* NOTE:                                                                */
/*      This routine is designed to support command queuing to the      */
/*      SCSI adapter.                                                   */
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
/*      ap      - pointer to adapter information structure              */
/*      dev_index - index to device information structure               */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  none                                      */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:  none                                    */
/*                                                                      */
/*                                                                      */
/************************************************************************/
void
hsc_start(
	  struct adapter_def * ap,
	  int dev_index)
{
    struct sc_buf *tptr;
    int     t_index, i, rc, allocated;

    /* assume interrupts are disable here */

    if (ap->epow_state != EPOW_PENDING) {
	if (!ap->any_waiting) {	/* no device waiting for resources */
	    allocated = hsc_start_dev(ap, dev_index);
	    if (!allocated) {	/* if could not get resources */
		ap->any_waiting++;	/* inc global counter */
		ap->dev[dev_index].waiting = TRUE;	/* set device flag */
	    }	/* else, request was handled, exit */

	}
	else {	/* waiting on resources */

	    /* if the device has pending requests and is not waiting, then */
	    /* put in the waiting state to make sure it is handled later.  */
	    if ((ap->dev[dev_index].head_pend != NULL) &&
		(!ap->dev[dev_index].waiting)) {
		ap->any_waiting++;	/* inc global count */
		ap->dev[dev_index].waiting = TRUE;	/* set device flag  */
	    }


	    /* loop thru devices, starting with next device and ending   */
	    /* with this device.                                         */
	    i = dev_index + 1;
	    while ((i - (dev_index + 1)) < IMDEVICES) {
		t_index = i % IMDEVICES;	/* modulo-number of devices */

		/* this check assures that only previously waiting devices  */
		/* are handled.  this makes sure that a device which is     */
		/* already waiting gets priority when resources are scarce. */
		if (ap->dev[t_index].waiting) {
		    allocated = hsc_start_dev(ap, t_index);
		    if (allocated) {	/* if allocated this time */
			ap->any_waiting--;	/* dec global counter */
			/* reset device waiting flag */
			ap->dev[t_index].waiting = FALSE;
		    }	/* else, still can't allocate, flags already set */
		}


		i++;	/* inc loop count */

	    }	/* endwhile */
	}	/* end else, waiting on resources */
    }	/* skip start logic, leaving commands stuck on pending queue, since
	   EPOW is pending */

}  /* end hsc_start */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_start_dev                                           */
/*                                                                      */
/* FUNCTION:    Adapter Driver Normal Start of Device                   */
/*                                                                      */
/*      This routine determines what resources the request needs,       */
/*      then allocates those resources, if possible, and sends the      */
/*      request to the adapter.                                         */
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
/*      ap      - pointer to adapter information structure              */
/*      dev_index - index to information on device being processed      */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      TRUE    - indicates resources were found, cmd sent              */
/*      FALSE   - indicates resources were not found, cmd not sent      */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:  none                                    */
/*                                                                      */
/*                                                                      */
/************************************************************************/
int
hsc_start_dev(
	      struct adapter_def * ap,
	      int dev_index)
{
    struct sc_buf *scp;
    int     ret_code, rc;

    ret_code = TRUE;	/* default to say we got resources */
    /* loop until no pending i/o, or until prevented from sending cmds */
    /* note that head_pend is updated when a command is kicked-off, or */
    /* if error in kicking-off, when the queue is cleared.             */
    while (ap->dev[dev_index].head_pend != NULL) {
	scp = ap->dev[dev_index].head_pend;	/* point to head element */
	/* see if we are prevented from sending cmds to this device.     */
	/* criteria:  queue limit reached, device err recov in progress, */
	/* ioctl halt or reset in progress which affects this            */
	/* device, previous error on a pending command, or reset         */
	/* previously occurred for this SCSI ID and this sc_buf is to    */
	/* be delayed.                                                   */
	if ((ap->dev[dev_index].num_act_cmds >= 
                                ap->dev[dev_index].queue_depth) ||
	    (ap->dev[dev_index].state) ||
	    (ap->dev[dev_index].init_cmd) ||
	    ((ap->dev[dev_index].pqstate) && (!(scp->resvd7))) ||
            (ap->download_pending) ||
	    ((scp->flags & SC_DELAY_CMD) &&
	     (ap->cdar_scsi_ids & (0x01 << SID(dev_index))))) {

	    /* can't send more */
	    break;	/* leave loop */
	}
	else {	/* okay to process cmds */
	    /* try to get a mbox */
	    rc = hsc_MB_alloc(ap, scp, dev_index);
	    if (rc == FALSE) {	/* if did not get mailbox */
		ret_code = FALSE;	/* say we need resources */
		break;	/* leave loop */
	    }
	    else {	/* we got a mailbox */
		/* are TCWs needed ?  */
		if ((scp->bufstruct.b_bcount > ST_SIZE) ||
		    (scp->resvd1))
		{
		    rc = hsc_TCW_alloc(ap,	/* get required TCWs */
				  (struct mbstruct *) scp->bufstruct.b_work,
				       scp, dev_index);
		    if (rc == FALSE) {	/* if no TCWs avail  */
			/* put back the mailbox so other cmds won't be   */
			/* waiting on this command.                      */
			hsc_MB_restore(ap,
				 (struct mbstruct *) scp->bufstruct.b_work);
			ret_code = FALSE;	/* say we need resources */
			break;	/* leave loop */
		    }
		    else {	/* TCWs are available */
			/* build mbox, make active, and send command */
			hsc_build_and_send(ap, scp, dev_index);
		    }
		}
		else {
		    /* is an STA needed ?       */
		    if (scp->bufstruct.b_bcount != 0) {
			rc = hsc_STA_alloc(ap,	/* get an STA */
				 (struct mbstruct *) scp->bufstruct.b_work);
			if (rc == FALSE) {	/* if no STA avail */
			    /* put back the mailbox so other cmds won't be */
			    /* waiting on this command.                    */
			    hsc_MB_restore(ap,
				 (struct mbstruct *) scp->bufstruct.b_work);
			    ret_code = FALSE;	/* say we need resources */
			    break;	/* leave loop */
			}
			else {	/* an STA is available */
			    /* build mbox, make active, and send command */
			    hsc_build_and_send(ap, scp, dev_index);
			}
		    }
		    else {	/* handle no data case */
			/* build mbox, make active, and send command */
			hsc_build_and_send(ap, scp, dev_index);
		    }
		}
	    }
	}

    }	/* endwhile *//* no more pending i/o */


    return (ret_code);	/* tell caller what happened */

}  /* end hsc_start_dev */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_build_and_send                                      */
/*                                                                      */
/* FUNCTION:    Build Mailbox, Make Active, and Send Routine            */
/*                                                                      */
/*      This routine performs the necessary steps to build the          */
/*      driver's internal mailbox struct, moves the sc_buf from the     */
/*      pending to the active queue, and invokes hsc_issue to send      */
/*      the command. All the necessary resources had to be allocated    */
/*      successfully before calling this routine.                       */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called on priority levels greater      */
/*      than, or equal to that of the interrupt handler.                */
/*                                                                      */
/* DATA STRUCTURES:  none                                               */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      sc_buf  - input/output request struct used between the adapter  */
/*                driver and the calling SCSI device driver             */
/*      mbstruct - driver structure which holds information related to  */
/*                 a particular mailbox and hence a command             */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter information structure              */
/*      scp     - pointer to sc_buf being processed                     */
/*      dev_index - index to information for device being processed     */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      none                                                            */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:  none                                    */
/*                                                                      */
/*                                                                      */
/************************************************************************/
void
hsc_build_and_send(
		   struct adapter_def * ap,
		   struct sc_buf * scp,
		   int dev_index)
{
    struct mbstruct *mbp;
    struct mbstruct *tmbp;
    int     i;

    DDHKWD5(HKWD_DD_SCSIDD, DD_ENTRY_BSTART, 0, ap->devno, scp,
	    scp->bufstruct.b_flags, scp->bufstruct.b_blkno,
	    scp->bufstruct.b_bcount);

    /* to get here, all necessary resources had to be allocated */
    /* get mailbox pointer */
    mbp = (struct mbstruct *) scp->bufstruct.b_work;
    if (ap->dev[dev_index].tail_act != NULL) {	/* any active cmds? */
	/* build mailbox pointer from last sc_buf element */
	tmbp = (struct mbstruct *) ap->dev[dev_index].tail_act->
	    bufstruct.b_work;
    }
    else {	/* no active cmds */
	/* if timeout_value is non_zero,add at least 1 sec to */
	/* account for how timer works */
	if (scp->timeout_value == 0)
	    ap->dev[dev_index].wdog->dog.restart = (ulong) 0;
	else
	    ap->dev[dev_index].wdog->dog.restart=(ulong)scp->timeout_value + 1;
    }

    /* here, build the mailbox send scsi command block */
    /* set scsi id, data trans/dir, nodisc flag, and async flag */
    mbp->mb.m_xfer_id =
	(scp->scsi_command.scsi_id & 0x07) |
	((scp->bufstruct.b_bcount) ? HSC_TRANS : 0) |
	((scp->bufstruct.b_flags & B_READ) ? HSC_READ : 0) |
	((scp->scsi_command.flags & SC_NODISC) ? HSC_NODISC : 0) |
	((scp->scsi_command.flags & SC_ASYNC) ? HSC_NOSYNC : 0);

    /* set scsi cmd length, set DMA burst size */
    mbp->mb.m_cmd_len = (scp->scsi_command.scsi_length & 0x0f) | DMA_BURST;
    /* set the DMA addr in hsc_issue routine */
    /* set the DMA length field */
    mbp->mb.m_dma_len = WORD_REVERSE(scp->bufstruct.b_bcount);

    /* store scsi cmd into mb scsi cmd block area */
    mbp->mb.m_scsi_cmd = scp->scsi_command.scsi_cmd;

    /* store queue tag message from sc_buf into q_message field of mailbox */
    /* note : q_message field is defined as the upper nibble of the op_code*/
    if (ap->enable_queuing) {
        mbp->mb.m_op_code = mbp->mb.m_op_code | (scp->q_tag_msg<<4);
    }

    /* init mailbox status area to all 0's */
    mbp->mb.m_resid = 0;
    mbp->mb.m_adapter_rc = 0;
    mbp->mb.m_extra_stat = 0;
    mbp->mb.m_scsi_stat = 0;
    mbp->mb.m_resvd = 0;
    mbp->preempt = MAX_PREEMPTS;

    /* N.B. this section assumes interrupts are previously disabled */

    /* dequeue sc_buf from head of pending list */
    ap->dev[dev_index].head_pend = (struct sc_buf *) scp->bufstruct.av_forw;
    if (ap->dev[dev_index].head_pend == NULL)
	ap->dev[dev_index].tail_pend = NULL;	/* update tail pointer */

    /* enqueue sc_buf to end of active list ;NOTE active queue doubly linked*/
    if (ap->dev[dev_index].head_act == NULL) {	/* is act list empty?  */
	ap->dev[dev_index].head_act = scp;	/* put on head of list */
	ap->dev[dev_index].tail_act = scp;	/* update tail pointer */
    }
    else {	/* act list not empty  */
	/* update pointer of last element on active chain */
	scp->bufstruct.av_back = (struct buf *) ap->dev[dev_index].tail_act;
	ap->dev[dev_index].tail_act->bufstruct.av_forw = (struct buf *) scp;
	ap->dev[dev_index].tail_act = scp;	/* update tail pointer */
    }

    /* mark new end of active chain */
    scp->bufstruct.av_forw = NULL;

    /* increment dev num_act_cmds counter */
    ap->dev[dev_index].num_act_cmds++;
    hsc_issue(ap, mbp, scp, dev_index);	/* issue cmd to the adapter */

    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_BSTART, 0, ap->devno);

}  /* end hsc_build_and_send */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_issue                                               */
/*                                                                      */
/* FUNCTION:    Adapter Driver Issue Command Routine                    */
/*                                                                      */
/*      This routine issues a command to the adapter.                   */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called on priority levels greater      */
/*      than, or equal to that of the interrupt handler.                */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      sc_buf  - input/output request struct used between the adapter  */
/*                driver and the calling SCSI device driver             */
/*      buf     - standard kernel block i/o request structure           */
/*      mbstruct - driver structure which holds information related to  */
/*                 a particular mailbox and hence a command             */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter information structure              */
/*      mbp     - pointer to mailbox being processed                    */
/*      scp     - pointer to sc_buf being processed                     */
/*      dev_index - index to information for device being processed     */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  none                                      */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      d_master        xmemin                                          */
/*      d_cflush        w_start                                         */
/*      w_stop          pio_assist                                      */
/*                                                                      */
/************************************************************************/
void
hsc_issue(
	  struct adapter_def * ap,
	  struct mbstruct * mbp,
	  struct sc_buf * scp,
	  int dev_index)
{
    struct buf *tempbp;
    struct io_parms iop;
    int     i;
    uint    dma_addr, temp_addr;
    int     err_flag, rc;
    struct gwrite *gw_ptr;
    caddr_t mem_addr;
    err_flag = FALSE;	/* default to no error detected */

    if ((scp->bufstruct.b_bcount > ST_SIZE) ||
	(scp->resvd1))
    {	/* xfer length bigger than for an STA     */
	/* calculate the DMA address */
	dma_addr = (uint) (DMA_ADDR(ap->ddi.tcw_start_addr, mbp->tcws_start) +
		      ((uint) scp->bufstruct.b_un.b_addr & (TCWRANGE - 1)));

	/* set mailbox dma address field (swap bytes as needed) */
	mbp->mb.m_dma_addr = WORD_REVERSE(dma_addr);

	if (scp->bp == NULL) {	/* if non-spanned command */
/************************************************************************/
/*	handle normal (non-STA/non-spanned) DMA data transfer here      */
/************************************************************************/
	    if (!(scp->resvd1))
		/* map the TCWs for the non-spanned transfer */
		d_master((int) ap->channel_id,
			 DMA_TYPE |
			 ((scp->bufstruct.b_flags & B_READ) ? DMA_READ : 0) |
		 ((scp->bufstruct.b_flags & B_NOHIDE) ? DMA_WRITE_ONLY : 0),
			 scp->bufstruct.b_un.b_addr,
			 (size_t) scp->bufstruct.b_bcount,
			 &scp->bufstruct.b_xmemd,
			 (char *) dma_addr);
	    else {
		/* handle gathered writes here */
		gw_ptr = (struct gwrite *) scp->resvd1;

		gw_ptr->dma_addr = dma_addr;
		d_master((int) ap->channel_id,
			 DMA_WRITE_ONLY | DMA_TYPE,
			 gw_ptr->buf_addr,
			 (size_t) scp->bufstruct.b_bcount,
			 &scp->bufstruct.b_xmemd,
			 (char *) gw_ptr->dma_addr);

	    }
	}
	else {	/* this is a spanned transfer */
/************************************************************************/
/*	handle spanned DMA data transfer here                           */
/************************************************************************/
	    /* N.B.  for spanned transfer TCW mapping to work, each */
	    /* separate system buffer must be a page-length long,   */
	    /* and must be alligned on page boundaries.             */
	    tempbp = scp->bp;	/* get bp field from sc_buf */
	    temp_addr = dma_addr;	/* let temp_addr be the starting dma
					   address */
	    while (tempbp != NULL) {	/* scan thru buf structs */
		/* map the TCWs for the spanned transfer */
		d_master((int) ap->channel_id,
			 DMA_TYPE |
			 ((scp->bufstruct.b_flags & B_READ) ? DMA_READ : 0) |
			 ((tempbp->b_flags & B_NOHIDE) ?
			  DMA_WRITE_ONLY : 0),
			 tempbp->b_un.b_addr,
			 (size_t) tempbp->b_bcount,
			 &tempbp->b_xmemd,
			 (char *) temp_addr);

		/* increment the dma_addr */
		temp_addr += tempbp->b_bcount;
		/* point to next buf struct in chain */
		tempbp = tempbp->av_forw;
	    }	/* endwhile */
	}

    }
    else {
	if (scp->bufstruct.b_bcount != 0) {	/* if this is an STA xfer */
/************************************************************************/
/*	handle Small Transfers here                                     */
/************************************************************************/

	    /* calculate the dma address of the STA */
	    dma_addr = (uint) (DMA_ADDR(ap->ddi.tcw_start_addr,
					ap->sta_tcw_start)) +
		((uint) (ap->STA[mbp->sta_index].stap) -
		 ((uint) (ap->STA[0].stap) & ~(TCWRANGE - 1)));

	    /* set mailbox dma address field (swap bytes as needed) */
	    mbp->mb.m_dma_addr = WORD_REVERSE(dma_addr);

	    /* the following handles Writes only */
	    if (!(scp->bufstruct.b_flags & B_READ)) {
		/* copy data from caller's buffer to the STA */
		if (scp->bufstruct.b_xmemd.aspace_id == XMEM_GLOBAL) {
		    /* copy data from kernel space */
		    for (i = 0; i < scp->bufstruct.b_bcount; i++)
			*(ap->STA[mbp->sta_index].stap + i) =
			    *(scp->bufstruct.b_un.b_addr + i);
		}
		else {	/* copy data from user space */
		    rc = xmemin(scp->bufstruct.b_un.b_addr,	/* user addr */
				ap->STA[mbp->sta_index].stap,	/* kern addr */
				scp->bufstruct.b_bcount,	/* num bytes */
				&scp->bufstruct.b_xmemd);	/* xmem ptr */

		    if (rc != XMEM_SUCC) {	/* handle bad copy */
			hsc_logerr(ap, ERRID_SCSI_ERR8, mbp, 0, 3, rc);
			err_flag = TRUE;
		    }
		}

	    }
	    /* read or write request path */

	    /* flush system cache to real memory before write DMA */
	    d_cflush((int) ap->channel_id,
		     (char *) ap->STA[mbp->sta_index].stap,
		     (size_t) ST_SIZE,
		     (char *) dma_addr);


	    /* d_master for STA already done */

	}
	/* no data to transfer */

    }

    mbp->d_cmpl_done = FALSE;	/* default to d_complete not done  */

    if (!err_flag) {	/* if no previous error */

	/*  if putting a mailbox onto an empty active queue, start 
	    the timer for this queue  */
    	if ( ap->dev[dev_index].num_act_cmds == 1 ) {
		ap->dev[dev_index].wdog->dog.restart = ap->dev[dev_index].head_act->timeout_value;
		w_start(&ap->dev[dev_index].wdog->dog);
	}

	/* string move the mailbox contents to the adapter.        */
	/* error recovery note:  driver is informed of data parity */
	/* error on mailbox load by adapter after it reads in this */
	/* request.  Driver is informed of other pio errors        */
	/* synchronously with this request.                        */

        mem_addr = BUSMEM_ATT(ap->ddi.bus_id, ap->ddi.base_addr);
        rc = BUS_PUTSTRX(mem_addr + MB_ADDR(mbp->MB_num),
		       &mbp->mb.m_op_code, MB_SIZE);
        BUSMEM_DET(mem_addr);
        if (rc != 0) {
	    iop.ap = ap;
	    iop.mbp = mbp;
	    iop.opt = WRITE_MBOX;
	    iop.errtype = 0;
	    if ((pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) &&
	        (iop.errtype != PIO_PERM_IOCC_ERR)) {
    	        if ( ap->dev[dev_index].num_act_cmds == 1 )
	        	   w_stop(&ap->dev[dev_index].wdog->dog);
	        err_flag = TRUE;	/* error--pio operation failed */
	    }
	    else {	/* command succeeded, continue */
	        /* set MB to ISACTIVE state */
	        mbp->cmd_state = ISACTIVE;
	    }
	}
	else {	/* command succeeded, continue */
	    /* set MB to ISACTIVE state */
	    mbp->cmd_state = ISACTIVE;
	}
    } /* end if (!err_flag) */

    /* internal trace point */
#ifdef HSC_TRACE
    hsc_internal_trace(ap, TRC_CMD, mbp, scp->flags, dev_index);
#endif HSC_TRACE

    /* now, check err_flag again, handle if any previous error */
    if (err_flag) {	/* error handling */
	scp->status_validity = SC_ADAPTER_ERROR;
	scp->general_card_status = SC_HOST_IO_BUS_ERR;
	scp->bufstruct.b_resid = scp->bufstruct.b_bcount;
	scp->bufstruct.b_error = EIO;
	(void) hsc_dma_cleanup(ap, scp, TRUE);	/* release TCW mapping */
	mbp->cmd_state = INTERRUPT_RECVD;	/* allow resources to be
						   freed */
	/* free the resources which were allocated */
	hsc_STA_dealloc(ap, mbp);
	hsc_TCW_dealloc(ap, mbp);
	hsc_MB_restore(ap, mbp);	/* put back on head of free list to
					   avoid sequence error */

	/* fail queue if this is head element */
	if (scp == ap->dev[dev_index].head_act) {
	    hsc_fail_cmd(ap, dev_index);
	}
	else {
	    /* here, leave on queue and wait until previous cmds finish */
	    /* set the pqstate so that subsequent pending commands will */
	    /* not be sent since that could result in out of sequence   */
	    /* commands.  This state is cleared when queue is cleared.  */
	    ap->dev[dev_index].pqstate = PENDING_ERROR;
	}
    }

}  /* end hsc_issue */
