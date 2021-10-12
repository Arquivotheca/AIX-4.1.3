static char sccsid[] = "@(#)69	1.4.5.8  src/bos/kernext/scsi/hscmisc.c, sysxscsi, bos411, 9428A410j 1/24/94 11:19:57";
/*
 * COMPONENT_NAME: (SYSXSCSI) IBM SCSI Adapter Driver
 *
 * FUNCTIONS:	hsc_MB_alloc, hsc_MB_dealloc, hsc_MB_restore,
 *		hsc_TCW_alloc, hsc_TCW_dealloc, hsc_STA_alloc,
 *		hsc_STA_dealloc, hsc_build_mb30, hsc_iodone,
 *		hsc_DMA_err, hsc_dev_DMA_err, hsc_dma_cleanup,
 *		hsc_fail_cmd, hsc_epow, hsc_halt_lun, hsc_reset_dev,
 *		hsc_release, hsc_scsi_reset, hsc_logerr,
 *		hsc_internal_trace, hsc_read_POS, hsc_write_POS,
 *		hsc_pio_function, hsc_pio_recov, hsc_deq_active,
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
/* NAME:        hscmisc.c                                               */
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
/* NAME:        hsc_MB_alloc                                            */
/*                                                                      */
/* FUNCTION:    Allocate Adapter Mailboxes Routine                      */
/*                                                                      */
/*      This routine attempts to allocate an adapter mailbox for        */
/*      I/O request.                                                    */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called on priority levels greater      */
/*      than, or equal to that of the interrupt handler.                */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      sc_buf  - input/output request struct used between the adapter  */
/*                driver and the calling SCSI device driver             */
/*      mbstruct - driver structure which holds information related to  */
/*                 a particular mailbox and hence a command             */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter information structure              */
/*      scp     - pointer to sc_buf being processed                     */
/*      dev_index - index to information of device being processed      */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      FALSE   - MBs could not be allocated                            */
/*      TRUE    - MBes were successfully allocated                      */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:  none                                    */
/*                                                                      */
/*                                                                      */
/************************************************************************/
int
hsc_MB_alloc(
	     struct adapter_def * ap,
	     struct sc_buf * scp,
	     int dev_index)
{
    struct mbstruct *mbp;

    /* assume interrupts are disabled here */

    /* make sure at least two mailboxes are free */
    if (ap->head_MB_free == ap->tail_MB_free) {
	return (FALSE);	/* this means only 1 mbox is free */
    }

    /* if waiting to/for mb30 RESTART cmd to complete, don't alloc mailbox */
    /* N.B. this assumes RESTART is only mb30 adap err recovery used       */
    if (ap->restart_state) {

	if (!ap->restart_index_validity) {	/* if don't already have an
						   index to restart on     */
	    ap->restart_index_validity = TRUE;
	    ap->restart_index = dev_index;	/* save which dev to restart
						   on */
	}	/* else, already have a restart index */
	return (FALSE);	/* don't give out a mailbox */
    }


    /* take next MB off front of free list */
    mbp = ap->head_MB_free;	/* get pointer to it */
    ap->head_MB_free = mbp->next;	/* pt head to next one, tail ptr is
					   always correct */
    /* put MB on end of active list */
    if (ap->head_MB_act == NULL) {	/* is act list empty? */
	mbp->prev = NULL;	/* indicates head elem */
	mbp->next = NULL;	/* init the next ptr */
	ap->head_MB_act = mbp;	/* put on head of list */
	ap->tail_MB_act = mbp;	/* update tail ptr */
    }
    else {	/* list not empty */
	mbp->prev = ap->tail_MB_act;	/* set back-chain */
	mbp->next = NULL;	/* init the next ptr */
	ap->tail_MB_act->next = mbp;	/* put on end of list */
	ap->tail_MB_act = mbp;	/* update tail ptr */
    }

    /* init tcw/sta fields in mbstruct */
    mbp->tcws_allocated = 0;
    mbp->tcws_start = 0;
    mbp->sta_index = -1;
    mbp->mb.m_op_code &= 0x0F;		/* clear out Q tag message */
    /* save MB ptr in b_work field in sc_buf */
    scp->bufstruct.b_work = (int) mbp;
    /* store the sc_buf pointer in the mailbox struct */
    mbp->sc_buf_ptr = scp;

    return (TRUE);

}  /* end hsc_MB_alloc */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_MB_dealloc                                          */
/*                                                                      */
/* FUNCTION:    Deallocate Adapter Mailboxes Routine                    */
/*                                                                      */
/*      This routine frees an adapter mailbox which was allocated       */
/*      by hsc_MB_alloc for an I/O request.  It also performs           */
/*      mailbox state changes and may place a mailbox in a wait list    */
/*      if it is deallocated before its interrupt has been received.    */
/*                                                                      */
/* NOTES:                                                               */
/*      This routine should be called only after other command          */
/*      resources (TCWs, STAs) have been released.                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called on priority levels greater      */
/*      than, or equal to that of the interrupt handler.                */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      mbstruct - driver structure which holds information related to  */
/*                 a particular mailbox and hence a command             */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter information structure              */
/*      mbp     - pointer to mailbox being processed                    */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      none                                                            */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      e_wakeup                                                        */
/*                                                                      */
/************************************************************************/
void
hsc_MB_dealloc(
	       struct adapter_def * ap,
	       struct mbstruct * mbp)
{

    /* assume interrupts are disable here */

    if (mbp->cmd_state == INTERRUPT_RECVD) {
	/* this is the normal case for a deallocation--the mailbox has */
	/* been processed and the interrupt has already been received. */

	/* take MB off active list */
	if (mbp->prev == NULL) {	/* if on head of act list */
	    ap->head_MB_act = mbp->next;	/* update head act */
	    if (ap->head_MB_act == NULL)	/* was this end of list? */
		ap->tail_MB_act = NULL;	/* update tail act ptr */
	    else
		mbp->next->prev = NULL;	/* make new head of list */
	}
	else {	/* elsewhere on act list */
	    mbp->prev->next = mbp->next;	/* pull mbp off forw chain */
	    if (mbp->next == NULL)	/* if on end of act chain */
		ap->tail_MB_act = mbp->prev;	/* update tail act ptr */
	    else
		mbp->next->prev = mbp->prev;	/* pull mbp off back chain */
	}

	/* put MB on end of free list */
	/* N.B. the mailbox free list is never allowed to go empty!! */
	ap->tail_MB_free->next = mbp;	/* put on end of free list */
	/* write this MB's id num in previous MB's sequ num field */
	ap->tail_MB_free->mb.m_sequ_num = mbp->MB_num;
	ap->tail_MB_free = mbp;	/* update tail free ptr */
	mbp->next = NULL;	/* flag new end of chain */

	/* free this mailbox */
	mbp->cmd_state = INACTIVE;
    }
    else
	if (( mbp->cmd_state ==  ISACTIVE ) ||  ( mbp->cmd_state ==
						WAIT_FOR_T_O_2 )) {
	    /* this means the command's mbox is being released prior     */
	    /* to receiving the adapter interrupt for it.                */
	    /* DMA was previously cleaned-up in hsc_fail_cmd routine.    */

	    /* take the mailbox off the active list */
	    if (mbp->prev == NULL) {	/* if on head of act list */
		ap->head_MB_act = mbp->next;	/* update head act */
		if (ap->head_MB_act == NULL)	/* was this end of list? */
		    ap->tail_MB_act = NULL;	/* update tail act ptr */
		else
		    mbp->next->prev = NULL;	/* make new head of list */
	    }
	    else {	/* elsewhere on act list */
		mbp->prev->next = mbp->next;	/* pull mbp off forw chain */
		if (mbp->next == NULL)	/* if on end of act chain */
		    ap->tail_MB_act = mbp->prev;	/* update tail ptr */
		else
		    mbp->next->prev = mbp->prev;	/* pull mbp off back
							   chain */
	    }

	    /* put the mailbox on the end of the waiting list */
	    if (ap->head_MB_wait == NULL) {	/* is wait list empty? */
		mbp->prev = NULL;	/* indicates head elem */
		mbp->next = NULL;	/* init the next ptr */
		ap->head_MB_wait = mbp;	/* put on head of list */
		ap->tail_MB_wait = mbp;	/* update tail ptr */
	    }
	    else {	/* list not empty */
		mbp->prev = ap->tail_MB_wait;	/* set back-chain */
		mbp->next = NULL;	/* init the next ptr */
		ap->tail_MB_wait->next = mbp;	/* put on end of list */
		ap->tail_MB_wait = mbp;	/* update tail ptr */
	    }

	    /* put mailbox in wait state so we know where it is */
	    mbp->cmd_state = WAIT_FOR_INTRPT;

	    /* if putting mailbox onto an empty wait queue, start timer     */
	    /* so this waiting mailbox, which has       		    */
	    /* already had its dma cleaned up and been iodone'ed, will not  */
	    /* stay on the waiting list indefinitely (should be a short     */
	    /* period in all cases until interrupt arrives).                */
	    if( ap->head_MB_wait == ap->tail_MB_wait )
		w_start(&ap->wdog3.dog);

	}
	else {	/* state must be WAIT_FOR_INTRPT */
	    if (mbp->cmd_state == WAIT_FOR_INTRPT) {
		/* this branch is taken when the interrupt the mailbox was  */
		/* waiting for arrives and now the mailbox can be freed for */
		/* further use.                                             */


		/* take the mailbox off the waiting list */
		if (mbp->prev == NULL) {	/* if on head of wait list */
		    ap->head_MB_wait = mbp->next;	/* update head wait */
		    /* was this end of list? */
		    if (ap->head_MB_wait == NULL)
			/* update tail wait ptr */
			ap->tail_MB_wait = NULL;
		    else
			mbp->next->prev = NULL;	/* make new head of list */
		}
		else {	/* elsewhere on wait list */
		    mbp->prev->next = mbp->next;	/* pull mbp off forw
							   chain */
		    if (mbp->next == NULL)	/* if on end of wait chain */
			ap->tail_MB_wait = mbp->prev;	/* update tail ptr */
		    else
			mbp->next->prev = mbp->prev;	/* pull mbp off back
							   chain */
		}

		/* put MB on end of free list */
		/* N.B. the mailbox free list is never allowed to go empty! */
		ap->tail_MB_free->next = mbp;	/* put on end of free list  */
		/* write this MB's id num in previous MB's sequ num field   */
		ap->tail_MB_free->mb.m_sequ_num = mbp->MB_num;
		ap->tail_MB_free = mbp;	/* update tail free ptr */
		mbp->next = NULL;	/* flag new end of chain */

		/* free this mailbox */
		mbp->cmd_state = INACTIVE;

		/*  start the waitq timer q not empty */
                if (ap->head_MB_wait != NULL)
                        w_start(&ap->wdog3.dog);


		/* see if close is sleeping on waiting mailboxes */
		if (ap->close_state == CLOSE_PENDING)
		    e_wakeup(&ap->cl_event);
	    }
	}

}  /* end hsc_MB_dealloc */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_MB_restore                                          */
/*                                                                      */
/* FUNCTION:    Restore a Mailbox Struct to the Free List Routine       */
/*                                                                      */
/*      This routine restores an adapter mailbox to the head of the     */
/*      queue from which it was allocated.  It is used to give back     */
/*      a mailbox (which the adapter expects to be sent next) when      */
/*      resources are being waited upon. This way, the next command     */
/*      which is ready to be sent (for this or another device) may be   */
/*      processed.                                                      */
/*                                                                      */
/* NOTES:                                                               */
/*      This routine should be called only when it is guaranteed that   */
/*      no other mailbox has been allocated since this mailbox has      */
/*      been allocated.  If not, an adapter sequence error will be      */
/*      caused.  Thus, allocation of this resource must be an atomic    */
/*      operation.                                                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called on priority levels greater      */
/*      than, or equal to that of the interrupt handler.                */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      mbstruct - driver structure which holds information related to  */
/*                 a particular mailbox and hence a command             */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter information structure              */
/*      mbp     - pointer to mailbox being processed                    */
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
hsc_MB_restore(
	       struct adapter_def * ap,
	       struct mbstruct * mbp)
{
    struct sc_buf *scp;

    /* assume interrupts are disabled here */

    /* take MB off active list */
    if (mbp->prev == NULL) {	/* if on head of act list */
	ap->head_MB_act = mbp->next;	/* update head act */
	if (ap->head_MB_act == NULL)	/* was this end of list? */
	    ap->tail_MB_act = NULL;	/* update tail act ptr */
	else
	    mbp->next->prev = NULL;	/* make new head of list */
    }
    else {	/* elsewhere on act list */
	mbp->prev->next = mbp->next;	/* pull mbp off forw chain */
	if (mbp->next == NULL)	/* if on end of act chain */
	    ap->tail_MB_act = mbp->prev;	/* update tail act ptr */
	else
	    mbp->next->prev = mbp->prev;	/* pull mbp off back chain */
    }

    /* put MB on front of free list */
    /* N.B.  the mailbox free list is never allowed to be empty!!   */
    mbp->next = ap->head_MB_free;	/* insert mbox at head  */
    ap->head_MB_free = mbp;	/* update head free ptr */
    /* tail ptr is already correct  */

    /* unlink the sc_buf and mailbox areas */
    scp = mbp->sc_buf_ptr;
    scp->bufstruct.b_work = (int) 0;
    mbp->sc_buf_ptr = NULL;

}  /* end hsc_MB_restore */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_TCW_alloc                                           */
/*                                                                      */
/* FUNCTION:    Allocate Translation Control Words Routine              */
/*                                                                      */
/*      This routine attempts to allocate system TCWs from the          */
/*      range of TCWs specified in the adapter ddi area for the         */
/*      requested I/O.                                                  */
/*                                                                      */
/* NOTES:                                                               */
/*      This routine should only be called when it is known that        */
/*      there is data requiring TCWs.  It is assumed that the caller    */
/*      has already checked appropriately.                              */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called on priority levels greater      */
/*      than, or equal to that of the interrupt handler.                */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      sc_buf  - input/output request struct used between the adapter  */
/*                driver and the calling SCSI device driver             */
/*      mbstruct - driver structure which holds information related to  */
/*                 a particular mailbox and hence a command             */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter information structure              */
/*      mbp     - pointer to mailbox being processed                    */
/*      scp     - pointer to sc_buf being processed                     */
/*      dev_index - index to information of device being processed      */
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
hsc_TCW_alloc(
	      struct adapter_def * ap,
	      struct mbstruct * mbp,
	      struct sc_buf * scp,
	      int dev_index)
{
    int     search_begin, search_end, start_index;
    int     badr, val, len, i, cnt;
    ulong   xfer_count;

    xfer_count = scp->bufstruct.b_bcount;

    /* get the starting address of the request */
    badr = (int) scp->bufstruct.b_un.b_addr;

    /* calculate number of TCWs required for this request */
    len = (((badr & (TCWRANGE - 1)) + (xfer_count - 1)) /
	   TCWRANGE) + 1;

    /* set range limits for TCW search, below */
    if (len == 1) {
	search_begin = ap->page_req_begin;
	search_end = ap->page_req_end;
	start_index = ap->next_page_req;
    }
    else {
	search_begin = ap->large_req_begin;
	search_end = ap->large_req_end;
	start_index = ap->next_large_req;
    }
    /* assume interrupts are disabled here */

    /* search for consecutive TCWs required for this request. */
    /* this first search starts where the last allocation    */
    /* ended as a hint to where free ones are.               */
    for (cnt = 0, i = start_index; i <= search_end; i++) {
	if (ap->TCW_tab[i] == 0xff)
	    cnt++;	/* found one */
	else
	    cnt = 0;	/* start over */
	if (cnt == len)
	    break;	/* done */
    }	/* endfor */

    /* if the TCWs were NOT found in the above search, must now */
    /* search the entire range, as it is possible to have missed */
    /* free TCWs above and below the "next" index.              */
    if (cnt != len) {
	for (cnt = 0, i = search_begin; i <= search_end; i++) {
	    if (ap->TCW_tab[i] == 0xff)
		cnt++;	/* found one */
	    else
		cnt = 0;	/* start over */
	    if (cnt == len)
		break;	/* done */
	}	/* endfor */
    }

    /* if the TCWs were found... */
    if (cnt == len) {
	/* set the "next" index up */
	if (i >= search_end)
	    start_index = search_begin;	/* wrap next index */
	else
	    start_index = (i + 1);

	if (xfer_count <= LPAGESIZE)
	    ap->next_page_req = start_index;
	else
	    ap->next_large_req = start_index;

	/* mark TCWs as "in use" for this device */
	for (val = (i + 1) - len; val <= i; val++)
	    ap->TCW_tab[val] = dev_index;	/* mark with device index */

	mbp->tcws_start = (i + 1) - len;	/* store starting TCW in mb */
	mbp->tcws_allocated = len;	/* store number of TCWs in mb */

	return (TRUE);	/* successful return */
    }
    else {
	return (FALSE);	/* tell caller TCWs not avail */
    }

}  /* end hsc_TCW_alloc */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_TCW_dealloc                                         */
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
/* NOTES:                                                               */
/*      This routine should be called before releasing the command's    */
/*      mailbox.                                                        */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      mbstruct - driver structure which holds information related to  */
/*                 a particular mailbox and hence a command             */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter information structure              */
/*      mbp     - pointer to mailbox being processed                    */
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
hsc_TCW_dealloc(
		struct adapter_def * ap,
		struct mbstruct * mbp)
{
    int     i;

    if (mbp->tcws_allocated == 0) {	/* if none allocated, return */
	return;
    }

    /* assume interrupts are disabled here */

    /* for mailbox states other than ISACTIVE, release TCWs */
    if (mbp->cmd_state != ISACTIVE) {
	/* mark TCWs as available */
	for (i = mbp->tcws_start;
	     i < (mbp->tcws_start + mbp->tcws_allocated); i++)
	    ap->TCW_tab[i] = 0xff;	/* release this TCW */

    }	/* if ISACTIVE, wait for ensuing intrpt before releasing TCWs */

}  /* end hsc_TCW_dealloc */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_STA_alloc                                           */
/*                                                                      */
/* FUNCTION:    Allocate A Small Transfer Area Routine                  */
/*                                                                      */
/*      This routine attempts to allocate a small transfer area (STA)   */
/*      from the pool of STAs.                                          */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called on priority levels greater      */
/*      than, or equal to that of the interrupt handler.                */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      sc_buf  - input/output request struct used between the adapter  */
/*                driver and the calling SCSI device driver             */
/*      mbstruct - driver structure which holds information related to  */
/*                 a particular mailbox and hence a command             */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter information structure              */
/*      mbp     - pointer to mailbox being processed                    */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      TRUE    - STAs were successfully allocated                      */
/*      FALSE   - STAs could not be allocated                           */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:  none                                    */
/*                                                                      */
/*                                                                      */
/************************************************************************/
int
hsc_STA_alloc(
	      struct adapter_def * ap,
	      struct mbstruct * mbp)
{
    struct sc_buf *scp;
    int     i, found_one;

    /* get pointer to the sc_buf of this mailbox */
    scp = mbp->sc_buf_ptr;

    if (scp->bufstruct.b_bcount == 0) {	/* if no data, return */
	return (TRUE);	/* successful return */
    }

    /* assume interrupts are disabled here */

    /* search for a free STA */
    found_one = FALSE;
    for (i = 0; i < NUM_STA; i++) {
	if (ap->STA[i].in_use == FALSE) {
	    found_one = TRUE;	/* we found one */
	    ap->STA[i].in_use = TRUE;	/* mark as in use */
	    mbp->sta_index = i;	/* save its index in mb */
	    break;	/* leave loop */
	}
    }	/* endfor */

    if (found_one)
	return (TRUE);	/* successful return */
    else
	return (FALSE);	/* unsuccessful return */

}  /* end hsc_STA_alloc */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_STA_dealloc                                         */
/*                                                                      */
/* FUNCTION:    Deallocate Small Transfer Area Routine                  */
/*                                                                      */
/*      This routine frees a device driver internal small transfer      */
/*      area previously allocated by hsc_STA_alloc for a request.       */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called on priority levels greater      */
/*      than, or equal to that of the interrupt handler.                */
/*                                                                      */
/* NOTES:                                                               */
/*      This routine should be called before releasing the command's    */
/*      mailbox.                                                        */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      mbstruct - driver structure which holds information related to  */
/*                 a particular mailbox and hence a command             */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter information structure              */
/*      mbp     - pointer to mailbox being processed                    */
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
hsc_STA_dealloc(
		struct adapter_def * ap,
		struct mbstruct * mbp)
{

    if (mbp->sta_index == -1) {	/* if no STA allocated */
	return;
    }

    /* assume interrupts are disabled here */

    /* for mailbox states other than ISACTIVE, release STAs */
    if (mbp->cmd_state != ISACTIVE) {
	ap->STA[mbp->sta_index].in_use = FALSE;	/* mark STA as available */
	mbp->sta_index = -1;	/* clear mailbox sta index */
    }	/* if ISACTIVE, wait for ensuing intrpt before releasing STAs */

}  /* end hsc_STA_dealloc */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_build_mb30                                          */
/*                                                                      */
/* FUNCTION:    Build a Selected Mailbox 30 Command Routine             */
/*                                                                      */
/*      This routine builds a mailbox 30 adapter command.               */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called from any other routine.              */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter information structure              */
/*      op      - operation to build command for                        */
/*      data1   - operation specific data (see cmd comments)            */
/*      data2   - operation specific data (see cmd comments)            */
/*      data3   - operation specific data (see cmd comments)            */
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
hsc_build_mb30(
	       struct adapter_def * ap,
	       int op,
	       int data1,
	       int data2,
	       int data3)
{
    uchar   tag;

    switch (op) {
      case SET_SCSI_ID:
	/* adapter command to change the card scsi id value */
	/* data1, data2, and data3 are unused               */
	ap->MB30p->mb.m_op_code = SET_SCSI_ID;
	ap->MB30p->mb.m_xfer_id = (uchar) ap->ddi.card_scsi_id;
	ap->MB30p->mb.m_cmd_len = 0;
	ap->MB30p->mb.m_sequ_num = 0;
	ap->MB30p->mb.m_dma_addr = 0;
	ap->MB30p->mb.m_dma_len = 0;
	break;
      case RESTART:
	/* adapter command to tell card where to get next */
	/* scsi cmd.  Use head of MB free list for the #  */
	/* data1, data2, and data3 are unused             */
	ap->MB30p->mb.m_op_code = RESTART;
	ap->MB30p->mb.m_xfer_id = 0;
	ap->MB30p->mb.m_cmd_len = 0;
	ap->MB30p->mb.m_sequ_num = ap->head_MB_free->MB_num;
	ap->MB30p->mb.m_dma_addr = 0;
	ap->MB30p->mb.m_dma_len = 0;
	break;
      case INITIALIZE:
	/* adapter command to abort commands to a device */
	/* data1 means:  0 = INIT LUN, 1 = INIT DEV 2= RESUME */
	/* data2 means:  the SCSI ID(s) of the device(s) */
	/* data3 means:  the LUN ID of the device        */
	ap->MB30p->mb.m_op_code = INITIALIZE;
	/* load the scsi id */
	ap->MB30p->mb.m_xfer_id = (uchar) data2;
	if (data1 == 1) {	/* this is an init dev cmd   */
	    ap->MB30p->mb.m_cmd_len = INITIALIZE;
	    /* lun not used on init dev cmd */
	    ap->MB30p->mb.m_sequ_num = 0;
	}
	else {	/* this is an init lun cmd   */
	    ap->MB30p->mb.m_cmd_len = 0x01;
            if (data1 == 2)     /* this is a resume init lun */
	        ap->MB30p->mb.m_cmd_len |= (0x01 << 4);
     
	    /* load the lun id */
	    ap->MB30p->mb.m_sequ_num = (uchar) data3;
	}
	ap->MB30p->mb.m_dma_addr = 0;
	ap->MB30p->mb.m_dma_len = 0;
	break;
      case DOWNLOAD:
	/* adapter command to download microcode         */
	/* data1 means:  0 = download, 1 = version only  */
	/* data2 means:  data length (in Kbytes)         */
	/* data3 is not used                             */
	ap->MB30p->mb.m_op_code = DOWNLOAD;
	ap->MB30p->mb.m_xfer_id = (uchar) data1;
	ap->MB30p->mb.m_cmd_len = 0;
	ap->MB30p->mb.m_sequ_num = 0;
	ap->MB30p->mb.m_dma_addr = 0;
	ap->MB30p->mb.m_dma_len = WORD_REVERSE(data2);
	break;
      case DIAGNOSTICS:
	/* adapter command to run diagnostics operation  */
	/* data1 means:  diag sub-command to run         */
	/* data2 and data3 are not used                  */
	ap->MB30p->mb.m_op_code = DIAGNOSTICS;
	ap->MB30p->mb.m_xfer_id = 0;
	ap->MB30p->mb.m_cmd_len = (uchar) data1;
	ap->MB30p->mb.m_sequ_num = 0;
	ap->MB30p->mb.m_dma_addr = 0;
	ap->MB30p->mb.m_dma_len = 0;
	break;
      case ENA_ID:
	/* adapter command to enable ids for target mode */
	/* data1 == 1 => disable id, 3 => temporary disable */
	/* data2 is the id to be enabled/disabled        */
	/* data3 not used				 */
	ap->MB30p->mb.m_op_code = ENA_ID;
	ap->MB30p->mb.m_cmd_len = (uchar) data1 & 0xff;
	ap->MB30p->mb.m_sequ_num = (uchar) data2 & 0xff;
	ap->MB30p->mb.m_xfer_id = 0;
	ap->MB30p->mb.m_dma_addr = 0;
	ap->MB30p->mb.m_dma_len = 0;
	break;
      case ENA_BUF:
	/* adapter command to enable target head buffers */
	/* data1 == 1 => disable buffer	else burst_len	 */
	/* data2 is buffer tag				 */
	/* data3 is not used				 */
	ap->MB30p->mb.m_op_code = ENA_BUF;
	ap->MB30p->mb.m_cmd_len = (uchar) (data1 & 0xff);
	tag = data2 & 0xff;
	ap->MB30p->mb.m_sequ_num = tag;
	ap->MB30p->mb.m_dma_addr =
	    WORD_REVERSE((uint) ap->tm_bufs[tag]->dma_addr);
	ap->MB30p->mb.m_dma_len =
	    WORD_REVERSE((uint) ap->tm_bufs[tag]->buf_size);

	ap->MB30p->mb.m_xfer_id = 0;
	break;
      default:
	break;

    }	/* endswitch */

    /* zero the mb30 status area before sending */
    ap->MB30p->mb.m_resid = 0;
    ap->MB30p->mb.m_adapter_rc = 0;
    ap->MB30p->mb.m_extra_stat = 0;
    ap->MB30p->mb.m_scsi_stat = 0;
    ap->MB30p->mb.m_resvd = 0;

}  /* end hsc_build_mb30 */

/************************************************************************/
/*                                                                      */
/* NAME:        hsc_iodone                                              */
/*                                                                      */
/* FUNCTION:    Adapter Driver Iodone Routine                           */
/*                                                                      */
/*      This routine handles completion of commands initiated through   */
/*      the hsc_ioctl routine.                                          */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine runs on the IODONE interrupt level, so it can      */
/*      be interrupted by the interrupt handler.                        */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      sc_buf  - input/output request struct used between the adapter  */
/*                driver and the calling SCSI device driver             */
/*                                                                      */
/* INPUTS:                                                              */
/*      bp      - pointer to the passed sc_buf                          */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  none                                      */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      e_wakeup                                                        */
/*                                                                      */
/************************************************************************/
int
hsc_iodone(
	   struct sc_buf * bp)
{
#ifdef _POWER_MP
    int old_pri;     /* old interrupt priority */
#endif



#ifdef _POWER_MP


    old_pri = disable_lock(INTIODONE,&(hsc_ioctl_scbuf_lock));

    bp->bufstruct.b_flags |= B_DONE;

    e_wakeup(&bp->bufstruct.b_event);
    unlock_enable(old_pri,&(hsc_ioctl_scbuf_lock));
#else
    e_wakeup(&bp->bufstruct.b_event);
#endif

}  /* end hsc_iodone */

/************************************************************************/
/*                                                                      */
/* NAME:        hsc_DMA_err                                             */
/*                                                                      */
/* FUNCTION:    System Detected General DMA Error Handling              */
/*                                                                      */
/*      This interrupt handler subroutine is used to process system     */
/*      detected DMA errors.  Note that since the system cannot         */
/*      tell which TCW, etc. had the error, all commands in progress    */
/*      are aborted.  This routine is called from both the good         */
/*      completion path and the error completion path.                  */
/*      This routine is called after handling all devices which have    */
/*      interrupted.  It must handle all other devices with a xfer      */
/*      in progress.                                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      sc_buf  - input/output request struct used between the adapter  */
/*                driver and the calling SCSI device driver             */
/*      mbstruct - driver structure which holds information related to  */
/*                 a particular mailbox and hence a command             */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter information structure              */
/*      dev_index - index to start looking through device table with    */
/*                                                                      */
/* RETURN VALUE DESCRIPTION: none                                       */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      w_stop                                                          */
/*                                                                      */
/************************************************************************/
void
hsc_DMA_err(
	    struct adapter_def * ap,
	    int dev_index)
{
    struct sc_buf *scp;
    int     t_index, i;


    /* error was logged in hsc_dma_cleanup() */
    i = dev_index;
    while ((i - dev_index) < IMDEVICES) {
	t_index = i % IMDEVICES;	/* generate new index */
	if (ap->dev[t_index].head_act != NULL) {	/* if any commands */
	    scp = ap->dev[t_index].head_act;	/* point at head element */
	    if (scp->bufstruct.b_bcount) {	/* if data xfer */
		/* the next test assumes the only device states are: not   */
		/* waiting wait to send init lun, and wait for init lun.   */
		if (ap->dev[t_index].state == 0) {	/* if not waiting on
							   recov */
		    /* handle this device */
		    hsc_dev_DMA_err(ap, t_index, scp);

		}
	    }
	}

	i++;	/* increment loop count */

    }	/* endwhile */


    /* handle target mode dma errors  */
    hsc_tgt_DMA_err(ap);

}  /* end hsc_DMA_err */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_dev_DMA_err                                         */
/*                                                                      */
/* FUNCTION:    Device DMA Error Handling                               */
/*                                                                      */
/*      This routine handles a specific device on a DMA error.          */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      sc_buf  - input/output request struct used between the adapter  */
/*                driver and the calling SCSI device driver             */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter information structure              */
/*      dev_index - index to information for this device                */
/*      scp     - pointer to sc_buf being processed                     */
/*                                                                      */
/* RETURN VALUE DESCRIPTION: none                                       */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      w_start         pio_assist                                      */
/*                                                                      */
/************************************************************************/
void
hsc_dev_DMA_err(
		struct adapter_def * ap,
		int dev_index,
		struct sc_buf * scp)
{
    struct io_parms iop;


    scp->status_validity = SC_ADAPTER_ERROR;
    scp->general_card_status = SC_HOST_IO_BUS_ERR;
    scp->bufstruct.b_resid = scp->bufstruct.b_bcount;
    scp->bufstruct.b_error = EIO;
    if (ap->dev[dev_index].state == 0) {
	ap->dev[dev_index].state = WAIT_TO_SEND_INIT_LUN;
	if (ap->MB30_in_use != -1) {	/* MB30 in use ? */
	    ap->waiting_for_mb30 = TRUE;
	}
	else {	/* MB30 is free */
	    ap->dev[dev_index].state = WAIT_FOR_INIT_LUN;
	    ap->MB30_in_use = dev_index;
	    /* build MB30 INIT LUN cmd */
	    hsc_build_mb30(ap, INITIALIZE, 0, (0x01 << SID(dev_index)),
			   LUN(dev_index));
	    ap->wdog.dog.restart = INIT_CMD_T_O;
	    w_start(&ap->wdog.dog);	/* start timer */

	    /* send MB30 command to adapter */
	    iop.ap = ap;
	    iop.opt = WRITE_MB30;
	    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
		/* handle unrecovered error sending mbox 30.  */
		/* ignore error here, allowing timeout to hit. */
		;
	    }

	}
    }

}  /* end hsc_dev_DMA_err */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_dma_cleanup                                         */
/*                                                                      */
/* FUNCTION:    Finish Processing DMA Routine                           */
/*                                                                      */
/*      This interrupt handler subroutine completes processing of       */
/*      DMA transfers.  It returns an unsuccessful return code if       */
/*      there was a system detected error during a DMA transfer.        */
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
/*      scp     - pointer to the sc_buf of mailbox being processed      */
/*      dma_err - if TRUE, indicates call from DMA error handler        */
/*                (indicates data copy should be bypassed for STAs)     */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      0       = successful result                                     */
/*      1       = IOCC detected DMA error other than DMA_SYSTEM         */
/*                or, xmemout returned an error return code             */
/*      SYS_ERR = IOCC detected DMA_SYSTEM error                        */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      d_master        d_complete                                      */
/*      xmemout                                                         */
/*                                                                      */
/************************************************************************/
int
hsc_dma_cleanup(
		struct adapter_def * ap,
		struct sc_buf * scp,
		int dma_err)
{
    struct mbstruct *mbp;
    struct buf *tempbp;
    int     i;
    uint    dma_addr;
    int     ret_code, rc, rcx;
    uint    temp_addr;
    struct gwrite *gw_ptr;

    ret_code = 0;	/* default to good return code */
    rc = 0;	/* default to good return code */

    /* generate the mailbox pointer */
    mbp = (struct mbstruct *) scp->bufstruct.b_work;

    /* make sure we only go through this code once */
    if (!(mbp->d_cmpl_done)) {
	mbp->d_cmpl_done = TRUE;
	if ((scp->bufstruct.b_bcount > ST_SIZE) ||
	    (scp->resvd1))
	{
	    if (scp->bp == NULL) {
/************************************************************************/
/* handle normal (non-STA/non-spanned) DMA data transfer completion here*/
/************************************************************************/
		/* handle non-gathered write case */
	    	if (!(scp->resvd1))
		    /* issue the dma complete on the data transfer */
		    rc = d_complete((int) ap->channel_id,
				    DMA_TYPE |
				    ((scp->bufstruct.b_flags & B_READ) ?
				     DMA_READ : 0) |
				    ((scp->bufstruct.b_flags & B_NOHIDE) ?
				     DMA_WRITE_ONLY : 0),
				    scp->bufstruct.b_un.b_addr,
				    (size_t) scp->bufstruct.b_bcount,
				    &scp->bufstruct.b_xmemd,
				 (char *) WORD_REVERSE(mbp->mb.m_dma_addr));
		else {
		    /* handle gathered writes here */
		    gw_ptr = (struct gwrite *) scp->resvd1;

		    /* handle DMA completion for gathered writes here */
		    rc = d_complete((int) ap->channel_id,
				    DMA_WRITE_ONLY | DMA_TYPE,
				    gw_ptr->buf_addr,
				    (size_t) scp->bufstruct.b_bcount,
				    &scp->bufstruct.b_xmemd,
				    (char *) gw_ptr->dma_addr);

		    /* put the gwrite struct on tail of gwrite free list */
		    if (ap->head_gw_free == NULL)
			ap->head_gw_free = gw_ptr;
		    else
			ap->tail_gw_free->next = gw_ptr;
		    ap->tail_gw_free = gw_ptr;
		    gw_ptr->next = NULL;	/* mark new end of chain */

		}

		if (rc != DMA_SUCC) {
		    hsc_logerr(ap, ERRID_SCSI_ERR2, mbp, DMA_ERROR, 20,
			       (uint) rc);
		}

		if (rc == DMA_SYSTEM)
		    ret_code = SYS_ERR;	/* flag DMA_SYSTEM error */
	    }
	    else {
/************************************************************************/
/*	handle spanned DMA data transfer completion here                */
/************************************************************************/
		tempbp = scp->bp;	/* get bp ptr from sc_buf */
		/* get the start dma address */
		temp_addr = WORD_REVERSE(mbp->mb.m_dma_addr);
		while (tempbp != NULL) {	/* scan thru buf structs */
		    rcx = d_complete((int) ap->channel_id, DMA_TYPE |
			((scp->bufstruct.b_flags & B_READ) ? DMA_READ : 0) |
				     ((tempbp->b_flags & B_NOHIDE) ?
				      DMA_WRITE_ONLY : 0),
				     tempbp->b_un.b_addr,
				     (size_t) tempbp->b_bcount,
				     &tempbp->b_xmemd,
				     (char *) temp_addr);

		    /* incr the dma address */
		    temp_addr += tempbp->b_bcount;
		    /* point to next buf */
		    tempbp = tempbp->av_forw;

		    if (rcx != DMA_SUCC) {
			hsc_logerr(ap, ERRID_SCSI_ERR2, mbp, DMA_ERROR, 21,
				   (uint) rcx);
		    }

		    rc |= rcx;	/* accumulate DMA errors */

		    if (rcx == DMA_SYSTEM)
			ret_code = SYS_ERR;	/* flag DMA_SYSTEM error */

		}	/* endwhile */
	    }

	    /* if other DMA errors */
	    if ((ret_code != SYS_ERR) && (rc != DMA_SUCC)) {
		ret_code = 1;	/* flag the error to caller */
	    }

	}
	else {
	    if (scp->bufstruct.b_bcount != 0) {
/************************************************************************/
/*	handle STA DMA data transfer completion here                    */
/************************************************************************/

		/* issue the dma complete on the data transfer. */
		/* DMA_WRITE_ONLY prevents the unhide of the page */
		rc = d_complete((int) ap->channel_id,
				(DMA_TYPE | DMA_WRITE_ONLY),
				ap->STA[0].stap,
				(size_t) STA_ALLOC_SIZE,
				&ap->xmem_buf,
				(char *) DMA_ADDR(ap->ddi.tcw_start_addr,
						  ap->sta_tcw_start));

		if (rc != DMA_SUCC) {
		    hsc_logerr(ap, ERRID_SCSI_ERR2, mbp, DMA_ERROR, 22,
			       (uint) rc);
		}

		if (rc == DMA_SYSTEM)
		    ret_code = SYS_ERR;	/* flag system DMA errors */
		else
		    if (rc != DMA_SUCC)
			ret_code = 1;	/* flag other DMA errors */

		if (ret_code) {	/* if any DMA error */
		    /* try to clean up various errors by doing a re- */
		    /* execute of d_master on the small xfer area    */
		    dma_addr = DMA_ADDR(ap->ddi.tcw_start_addr,
					ap->sta_tcw_start);
		    d_master(ap->channel_id, DMA_TYPE, ap->STA[0].stap,
			     (size_t) STA_ALLOC_SIZE, &ap->xmem_buf,
			     (char *) dma_addr);

		    /* execute d_complete on the STA to allow access to it */
		    (void) d_complete(ap->channel_id, DMA_TYPE,
				      ap->STA[0].stap,
				      (size_t) STA_ALLOC_SIZE, &ap->xmem_buf,
				      (char *) dma_addr);
		}
		else {	/* path for no DMA error */
		    /* if transfer was a read, and not called via DMA error  */
		    /* cleanup path, then finish transfer by copying data to */
		    /* caller                                                */
		    if ((scp->bufstruct.b_flags & B_READ) &&
			(!dma_err)) {

			/* must copy read data to caller's buffer */
			if (scp->bufstruct.b_xmemd.aspace_id == XMEM_GLOBAL) {
			    /* copy data from kernel STA to caller's area */
			    for (i = 0;
				 i < (scp->bufstruct.b_bcount -
				      scp->bufstruct.b_resid);
				 i++)
				*(scp->bufstruct.b_un.b_addr + i) =
				    *(ap->STA[mbp->sta_index].stap + i);
			}
			else {
			    /* copy data to user space */
			    rc = xmemout(
			    /* kern addr */
					 ap->STA[mbp->sta_index].stap,
			    /* user addr */
					 scp->bufstruct.b_un.b_addr,
			    /* num bytes */
					 scp->bufstruct.b_bcount -
					 scp->bufstruct.b_resid,
			    /* ptr to xmem */
					 &scp->bufstruct.b_xmemd);

			    if (rc != XMEM_SUCC) {	/* handle bad copy */
				/* log the error, and continue on */
				ret_code = 1;	/* flag xmemout problem */
				hsc_logerr(ap, ERRID_SCSI_ERR8, mbp, 0, 23,
					   rc);
			    }
			}
		    }
		}
	    }	/* else, no data transfer, finished */
	}
    }	/* end of d_cmpl_done check */

    return (ret_code);

}  /* end hsc_dma_cleanup */

/************************************************************************/
/*                                                                      */
/* NAME:        hsc_fail_cmd                                            */
/*                                                                      */
/* FUNCTION:    Fail Active and Pending Commands Routine                */
/*                                                                      */
/*      This internal routine is called by the interrupt handler and    */
/*      timer handler to clear out queued and active commands for a     */
/*      device which has experienced some sort of failure which is      */
/*      not recoverable by the adapter driver.                          */
/*      This routine is also called by the deallocate device routine    */
/*      when it is necessary to forcibly stop a device.  In this case,  */
/*      the STOP_PENDING device flag tells this routine to not call     */
/*      iodone, etc, as the device will not continue running.           */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called on priority levels greater      */
/*      than, or equal to that of the interrupt handler.                */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      sc_buf  - input/output request struct used between the adapter  */
/*                driver and the calling SCSI device driver             */
/*      mbstruct - driver structure which holds information related to  */
/*                 a particular mailbox and hence a command             */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter structure                          */
/*      dev_index - index to device information structure               */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      disable_lock    unlock_enable                                   */
/*      iodone          w_stop                                          */
/*      e_wakeup                                                        */
/*                                                                      */
/************************************************************************/
void
hsc_fail_cmd(
	     struct adapter_def * ap,
	     int dev_index)
{
    struct sc_buf *tptr;
    struct mbstruct *mbp;
    int     old_pri;

    /* this assumes that for the failed command(s), the  */
    /* following are set previously:  sc_buf status,     */
    /* sc_buf resid value, and sc_buf b_error field.     */

    /* check cc_error_state to see if queue does not need to be cleared--   */
    /* the case where a check condition occurred when queueing to a device  */
    if((ap->dev[dev_index].cc_error_state == CC_OCCURRED) &&
       (!(ap->MB30p->mb.m_cmd_len == 0x11))) {

	mbp = (struct mbstruct *)
               ap->dev[dev_index].cmd_save_ptr_cc->bufstruct.b_work;
	/* see if mbp is valid (non-NULL).  if so, free up resources */
	/* normally.  if NULL, resources already freed.              */
	if (mbp) {
	    hsc_STA_dealloc(ap, mbp);
	    hsc_TCW_dealloc(ap, mbp);
	    hsc_MB_dealloc(ap, mbp);
	}
        /* set b_flags B_ERROR flag */
        ap->dev[dev_index].cmd_save_ptr_cc->bufstruct.b_flags |= B_ERROR;
        ap->commands_outstanding--;     /* dec total command count */
        ap->dev[dev_index].num_act_cmds--;      /* dec this dev cmd count  */
        /* remove sc_buf pointed to by cmd_save_ptr_cc from the active queue*/
        /* and iodone it with queue not cleared indication                  */
        hsc_deq_active(ap,ap->dev[dev_index].cmd_save_ptr_cc, dev_index);
#ifdef HSC_TRACE
        hsc_internal_trace(ap, TRC_DONE, tptr, mbp, dev_index);
#endif HSC_TRACE
        ap->dev[dev_index].cmd_save_ptr_cc->adap_q_status = SC_DID_NOT_CLEAR_Q;
        iodone((struct buf *) ap->dev[dev_index].cmd_save_ptr_cc);
        /* clear cmd_save_ptr_cc */
        ap->dev[dev_index].cmd_save_ptr_cc = NULL;
        /* set cc_error_state to expect expidited error recovery */
        ap->dev[dev_index].cc_error_state = WAIT_FOR_RECOVERY_CMD;
        hsc_start(ap, dev_index);  /* see if another request can be started */
        return;
    }
   
    if(ap->MB30p->mb.m_cmd_len == 0x11) { /* a resume init lun has completed */
        ap->MB30p->mb.m_cmd_len = 0x01; /* clear the resume bit */
        if (!(ap->dev[dev_index].cc_error_state == CC_OCCURRED)) {
            ap->dev[dev_index].pqstate = 0; /* clear pending error state */
        }
        if (ap->dev[dev_index].cmd_save_ptr_res) {
            iodone((struct buf *) ap->dev[dev_index].cmd_save_ptr_res);
            /* clear cmd_save_ptr_res */
            ap->dev[dev_index].cmd_save_ptr_res = NULL;
            ap->commands_outstanding--; /* dec total command count */
        }
        /* see if another request can be started */
        hsc_start(ap, dev_index);
        return;
    }
             
    /* clean up active cmd queue */
    while (ap->dev[dev_index].head_act != NULL) {
	tptr = ap->dev[dev_index].head_act;
	mbp = (struct mbstruct *) tptr->bufstruct.b_work;
        /* if the mailbox pointer is valid and a d_complete has not been done */
        /* then call hsc_dma_cleanup to do one                                */
        if ((mbp) && (mbp->d_cmpl_done == 0))
	    (void) hsc_dma_cleanup(ap, tptr, TRUE); 
	/* N.B. if mbp is NULL, b_error will be non-zero */
	if (tptr->bufstruct.b_error == 0) {
	    /* no error indicates this one simply needs restart */
	    /* set up otherwise good status */
	    tptr->status_validity = 0;
	    /* say no data was sent */
	    tptr->bufstruct.b_resid = tptr->bufstruct.b_bcount;
	    /* mark as needing restart by the caller */
	    tptr->bufstruct.b_error = ENXIO;
	    /* free the transfer area */
	}	/* error already set, leave status area as is */

	/* set b_flags B_ERROR flag */
	tptr->bufstruct.b_flags |= B_ERROR;

	/* see if mbp is valid (non-NULL).  if so, free up resources */
	/* normally.  if NULL, resources already freed.              */
	if (mbp) {
	    hsc_STA_dealloc(ap, mbp);
	    hsc_TCW_dealloc(ap, mbp);
	    hsc_MB_dealloc(ap, mbp);
	}
	ap->commands_outstanding--;	/* dec total command count */
	ap->dev[dev_index].num_act_cmds--;	/* dec this dev cmd count  */
	/* point to next sc_buf in active chain, if any */
	ap->dev[dev_index].head_act = (struct sc_buf *) tptr->
	    bufstruct.av_forw;

	/* internal trace point */
#ifdef HSC_TRACE
	hsc_internal_trace(ap, TRC_DONE, tptr, mbp, dev_index);
#endif HSC_TRACE

	iodone((struct buf *) tptr);

	/* see if stop is sleeping on pending commands */
	if (ap->dev[dev_index].qstate == STOP_PENDING)
	    e_wakeup(&ap->dev[dev_index].stop_event);

	/* see if close is sleeping on outstanding cmds or mboxes */
	if (ap->close_state == CLOSE_PENDING)
	    e_wakeup(&ap->cl_event);

    }	/* endwhile */

    ap->dev[dev_index].tail_act = NULL;	/* reset tail pointer */

    /* active queue now empty so stop watchdog timer for this device */
    w_stop(&ap->dev[dev_index].wdog->dog);
    if (ap->enable_queuing) {
        ap->dev[dev_index].queue_depth = Q_ENABLED;
        ap->dev[dev_index].queuing_stopped = FALSE;
    }
    else ap->dev[dev_index].queue_depth = Q_DISABLED;



    /* clean up global waiting states */
    if (ap->dev[dev_index].waiting)
	ap->any_waiting--;	/* dec global counter */

    ap->dev[dev_index].waiting = FALSE;	/* reset device waiting flag */

    /* clean up pending cmd queue */
    while (ap->dev[dev_index].head_pend != NULL) {
	tptr = ap->dev[dev_index].head_pend;
	/* set b_flags B_ERROR flag */
	tptr->bufstruct.b_flags |= B_ERROR;
	/* set up otherwise good status */
	tptr->status_validity = 0;
	/* say no data was sent */
	tptr->bufstruct.b_resid = tptr->bufstruct.b_bcount;
	/* mark as needing restart by the caller */
	tptr->bufstruct.b_error = ENXIO;
	/* dec total command count */
	ap->commands_outstanding--;
	/* point to next sc_buf in pending chain, if any */
	ap->dev[dev_index].head_pend = (struct sc_buf *) tptr->
	    bufstruct.av_forw;

	/* internal trace point */
#ifdef HSC_TRACE
	hsc_internal_trace(ap, TRC_DONE, tptr, NULL, dev_index);
#endif HSC_TRACE

	iodone((struct buf *) tptr);

	/* see if stop is sleeping on pending commands */
	if (ap->dev[dev_index].qstate == STOP_PENDING)
	    e_wakeup(&ap->dev[dev_index].stop_event);

	/* see if close is sleeping on outstanding cmds or mboxes */
	if (ap->close_state == CLOSE_PENDING)
	    e_wakeup(&ap->cl_event);

    }	/* endwhile */

    ap->dev[dev_index].tail_pend = NULL;	/* reset tail pointer */

    ap->dev[dev_index].pqstate = 0;	/* reset pend que state */
    ap->dev[dev_index].cc_error_state = 0; /* reset cc_error_state */

    if (ap->dev[dev_index].qstate != STOP_PENDING) {
	ap->dev[dev_index].qstate = HALTED;	/* set dev state--wait for
						   resume */
    }	/* don't change state if being stopped */

    hsc_start(ap, dev_index);	/* see if another request can be started */

}  /* end hsc_fail_cmd */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_epow                                                */
/*                                                                      */
/* FUNCTION:    Adapter Driver Early Power-Off Warning Handler          */
/*                                                                      */
/*      This routine processes early power-off warnings.                */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine runs on the INTEPOW level, therefore, it must      */
/*      only perform operations on pinned data.                         */
/*                                                                      */
/* NOTES:                                                               */
/*      There is a single EPOW handler defined for all SCSI adapters    */
/*      under control of this adapter driver.  This was chosen over     */
/*      separate handlers in the interest of speed in handling the      */
/*      EPOW.                                                           */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      intr    -  kernel interrupt handler information structure       */
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
/*      disable_lock    unlock_enable                                   */
/*      pio_assist                                                      */
/*                                                                      */
/************************************************************************/
int
hsc_epow(
	 struct intr * handler)
{
    struct adapter_def *ap, *save_ap;
    struct io_parms iop;
    int     i, t_index, any_found;
    int     old_pri1, old_pri2;
    uchar   trash;

    if ((handler->flags & EPOW_SUSPEND) || (handler->flags & EPOW_BATTERY)) {
	/* handle a suspend command */
	any_found = FALSE;	/* default flag value */

	/* scan list of adapter structures */
	for (i = 0; i < MAXADAPTERS; i++) {
	    ap = adapter_ptrs[i];	/* point at next chain anchor */
	    while (ap != NULL) {
		/* if there are devices opened on this adapter, and adapter */
		/* is not already in suspended state, suspend is allowed.   */
		if ((ap->devices_in_use) && (ap->epow_state != EPOW_PENDING)) {
		    /* if this is a normal suspend, or if this is a  */
		    /* battery-backed epow and this adapter is not   */
		    /* battery-backed, then suspend                  */
		    if ((handler->flags & EPOW_SUSPEND) ||
			((handler->flags & EPOW_BATTERY) &&
			 (!(ap->ddi.battery_backed)))) {
			save_ap = ap;	/* this saves last adapter's ptr */
			any_found = TRUE;	/* indicate delay needed */

                        /* set epow_reset flag so that a SCSI bus reset     */
                        /* error is not logged upon completion of the reset */
                        ap->epow_reset = TRUE;
	          	/* set the internal SCSI reset bit */
			iop.ap = ap;
			iop.opt = SI_SET;
			if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov)
			    == EIO) {
			    /* unrecovered error attempting to do internal */
			    /* reset.  ignore--there is no alternative     */
			    /* recovery                                    */
			    ;
			}
		    }
		}
		ap = ap->next;	/* look at next adapter struct */
	    }	/* endwhile */
	}	/* endfor */


	if (any_found) {

	    /* execute a delay loop for at least 25 microseconds */
	    iop.ap = save_ap;
	    iop.data = 30;
	    iop.opt = SYNC_DELAY;
	    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
		/* unrecovered error attempting to do sync. delay. */
		/* ignore--attempt to continue the epow logic.     */
		;
	    }

	    /* again, scan list of adapter structures */
	    for (i = 0; i < MAXADAPTERS; i++) {
		ap = adapter_ptrs[i];	/* point at next chain anchor */
		while (ap != NULL) {
		    /* if there are devices opened on this adapter, and  */
		    /* adapter is not already in suspended state,        */
		    /* suspend is allowed.                               */
		    if ((ap->devices_in_use) &&
			(ap->epow_state != EPOW_PENDING)) {
			if ((handler->flags & EPOW_SUSPEND) ||
			    ((handler->flags & EPOW_BATTERY) &&
			     (!(ap->ddi.battery_backed)))) {
			    /* set the adapter epow pending flag. */
			    ap->epow_state = EPOW_PENDING;


			    /* reset the internal SCSI Bus Reset line */
			    iop.ap = ap;
			    iop.opt = SI_RESET;
			    if (pio_assist(&iop, hsc_pio_function,
					   hsc_pio_recov) == EIO) {
				/* unrecovered error attempting to do */
				/* internal reset ignore--there is no */
				/* alternative recovery               */
				;
			    }
			}
		    }
		    ap = ap->next;	/* look at next adapter struct */
		}	/* endwhile */
	    }	/* endfor */
	}
    }
    else {
	if (handler->flags & EPOW_RESUME) {
	    /* handle a resume command */

	    /* scan list of adapter structures */
	    for (i = 0; i < MAXADAPTERS; i++) {
		ap = adapter_ptrs[i];	/* point at next chain anchor */
		while (ap != NULL) {

                    old_pri1 = disable_lock(ap->ddi.int_prior, &(hsc_mp_lock));

		    /* disable to close window around the test */
                    old_pri2 = disable_lock(INTEPOW, &(hsc_epow_lock));

		    if (((!(handler->flags & EPOW_BATTERY)) &&
			 (!(handler->flags & EPOW_SUSPEND))) &&
			(ap->epow_state == EPOW_PENDING)) {
			ap->epow_state = 0;	/* reset epow state */
		        unlock_enable(old_pri2, &(hsc_epow_lock));
			/* restart device queues */
			for (t_index = 0; t_index < IMDEVICES; t_index++) {
			    hsc_start(ap, t_index);
			}	/* endfor */
		    }
		    else {	/* either a SUSPEND has re-occurred, or this
				   adap was not put in epow pending state;
				   for these cases--leave adapter as is */
		        unlock_enable(old_pri2, &(hsc_epow_lock));

		    }
		    unlock_enable(old_pri1, &(hsc_mp_lock));	/* re-enable */
		    ap = ap->next;	/* look at next adapter struct */
		}	/* endwhile */
	    }	/* endfor */
	}
    }

    return (INTR_SUCC);

}  /* end hsc_epow */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_halt_lun                                            */
/*                                                                      */
/* FUNCTION:    Routine to Halt a Selected LUN                          */
/*                                                                      */
/*      This internal routine performs actions required to halt a       */
/*      device queue and all pending command.                           */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called from the process level.         */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter structure                          */
/*      dev_index - index to device structure                           */
/*      devflag - device flag                                           */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code = 0  for successful completion                      */
/*                  = EIO if a permanent I/O error occurs.              */
/*                  = EINVAL if device was not opened.                  */
/*                  = EACCES if adapter not in normal mode.             */
/*                  = ETIMEDOUT if the command did not complete.        */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      disable_lock    unlock_enable                                   */
/*      w_start         e_sleep_thread                                  */
/*      bzero           pio_assist                                      */
/*                                                                      */
/************************************************************************/
int
hsc_halt_lun(
	     struct adapter_def * ap,
	     int dev_index,
	     ulong devflag)
{
    int     ret_code;
    int     old_pri;
    struct io_parms iop;

    ret_code = 0;	/* set default return code */

    if (!ap->dev[dev_index].opened) {
	ret_code = EINVAL;	/* device not opened */
	goto end;
    }

    /* make sure adapter is not in diag mode */
    if (ap->adapter_mode != NORMAL_MODE) {
	ret_code = EACCES;	/* wrong adapter mode */
	goto end;
    }

    /* init iop structure */
    bzero(&iop, sizeof(struct io_parms));

    old_pri = disable_lock(ap->ddi.int_prior, &(hsc_mp_lock));

    ap->p_scsi_id = SID(dev_index);	/* save SCSI ID */
    ap->p_lun_id = LUN(dev_index);	/* save LUN ID */
    ap->proc_waiting = WAIT_TO_SEND_INIT_LUN;	/* flag the state */

    /* flag to prevent start routine from kicking off cmds during MB30 cmd */
    ap->dev[dev_index].init_cmd = INIT_CMD_IN_PROGRESS;

    if (ap->MB30_in_use != -1) {	/* MB30 in use ? */
	ap->waiting_for_mb30 = TRUE;
    }
    else {	/* MB30 is free */

	ap->MB30_in_use = PROC_USING;	/* flag that proc lvl using */
	ap->proc_waiting = WAIT_FOR_INIT_LUN;
	/* build mb30 Initialize LUN cmd to abort the LUN */
	ap->proc_results = 0;
	hsc_build_mb30(ap, INITIALIZE, 0, (0x01 << ap->p_scsi_id),
		       ap->p_lun_id);
	ap->wdog.dog.restart = INIT_CMD_T_O;
	w_start(&ap->wdog.dog);

	/* send MB30 command to adapter */
	iop.ap = ap;
	iop.opt = WRITE_MB30;
	iop.errtype = 0;
	if ((pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) &&
	    (iop.errtype != PIO_PERM_IOCC_ERR)) {
	    /* if retries exhausted, and not an iocc internal error */
	    /* then gracefully back-out, else, allow to either      */
	    /* complete or timeout                                  */
	    w_stop(&ap->wdog.dog);
	    ap->MB30_in_use = -1;	/* release mbox 30 */
	    ap->proc_waiting = 0;
	    ap->dev[dev_index].init_cmd = 0;	/* let future cmds be sent */
	    ret_code = EIO;	/* indicate error */
	    unlock_enable(old_pri, &(hsc_mp_lock));	/* unmask intrpts */
	    goto end;
	}
    }

    e_sleep_thread(&ap->event, &(hsc_mp_lock), LOCK_HANDLER);

    /* test the return status to set up the return code */
    switch (ap->proc_results) {
      case GOOD_COMPLETION:
	ret_code = 0;
	break;
      case TIMED_OUT:
	ret_code = ETIMEDOUT;
	break;
      case FATAL_ERROR:
	ret_code = EIO;
	break;
      case SEE_RC_STAT:
	if ((ap->mb30_rc == SCSI_BUS_RESET) ||
	    (ap->mb30_rc == PREVIOUS_ERROR))
	    ret_code = 0;
	else
	    if (ap->mb30_rc == COMPLETE_WITH_ERRORS)
		if (ap->mb30_extra_stat ==
		    UNEXPECTED_BUS_FREE)
		    ret_code = 0;
		else {	/* unknown response to INIT LUN */
		    ret_code = EIO;
		    hsc_logerr(ap, ERRID_SCSI_ERR4, ap->MB30p,
			       UNKNOWN_CARD_ERR, 50, 0);
		}
	break;
      default:
	ret_code = EIO;
	break;
    }	/* endswitch */

    ap->dev[dev_index].init_cmd = 0;	/* let future cmds be sent */

    /* typically, if cmds had been active, the interrupt handler would */
    /* fail them when it sees the "terminated by init cmd" status.     */
    /* However, if no commands had been active, and if one or more     */
    /* commands came in while the mb30 command was to be issued, they  */
    /* are left on the pending queue and must be cleared here.         */
    hsc_fail_cmd(ap, dev_index);
    unlock_enable(old_pri, &(hsc_mp_lock));	/* unmask intrpts */

end:
    return (ret_code);

}  /* end hsc_halt_lun */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_reset_dev                                           */
/*                                                                      */
/* FUNCTION:    Routine to Issue a Bus Device Reset to a SCSI Device    */
/*                                                                      */
/*      This internal routine performs actions required to send a       */
/*      SCSI bus device reset message to the selected SCSI controller.  */
/*                                                                      */
/* NOTES:                                                               */
/*                                                                      */
/*      The BDR message affects all LUNs attached to the selected SCSI  */
/*      device.  Commands in progress are aborted, and each LUN is      */
/*      sent back to its initial, power-on state.                       */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called from the process level.         */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter structure                          */
/*      dev_index - index to device structure                           */
/*      devflag - device flag                                           */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code = 0  for successful completion                      */
/*                  = EIO if a permanent I/O error occurs.              */
/*                  = EINVAL if device was not opened.                  */
/*                  = EACCES if adapter not in normal mode.             */
/*                  = ETIMEDOUT if the command did not complete.        */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      disable_lock    unlock_enable                                   */
/*      w_start         e_sleep_thread                                  */
/*      bzero           pio_assist                                      */
/*                                                                      */
/************************************************************************/
int
hsc_reset_dev(
	      struct adapter_def * ap,
	      int dev_index,
	      ulong devflag)
{
    int     ret_code;
    int     old_pri;
    uchar   temp_lun;
    int     t_index;
    struct io_parms iop;

    ret_code = 0;	/* set default return code */

    if (!ap->dev[dev_index].opened) {
	ret_code = EINVAL;	/* device not opened */
	goto end;
    }

    /* make sure adapter is not in diag mode */
    if (ap->adapter_mode != NORMAL_MODE) {
	ret_code = EACCES;	/* wrong adapter mode */
	goto end;
    }

    /* init iop structure */
    bzero(&iop, sizeof(struct io_parms));

    old_pri = disable_lock(ap->ddi.int_prior, &(hsc_mp_lock));

    ap->p_scsi_id = SID(dev_index);	/* save SCSI ID */
    ap->p_lun_id = 0;	/* zero LUN ID */
    ap->proc_waiting = WAIT_TO_SEND_INIT_DEV;	/* flag the state */

    /* set flag to prevent start routine from kicking off cmds during MB30
       cmd */
    for (temp_lun = 0; temp_lun < 8; temp_lun++) {
	t_index = INDEX(ap->p_scsi_id, temp_lun);
	ap->dev[t_index].init_cmd = INIT_CMD_IN_PROGRESS;
    }

    if (ap->MB30_in_use != -1) {	/* MB30 in use ? */
	ap->waiting_for_mb30 = TRUE;
    }
    else {	/* MB30 is free */

	ap->MB30_in_use = PROC_USING;	/* flag that proc lvl using */
	ap->proc_waiting = WAIT_FOR_INIT_DEV;
	/* build mb30 Initialize DEV cmd to reset the controller */
	ap->proc_results = 0;
	hsc_build_mb30(ap, INITIALIZE, 1, (0x01 << ap->p_scsi_id), 0);
	ap->wdog.dog.restart = INIT_CMD_T_O;
	w_start(&ap->wdog.dog);

	/* send MB30 command to adapter */
	iop.ap = ap;
	iop.opt = WRITE_MB30;
	iop.errtype = 0;
	if ((pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) &&
	    (iop.errtype != PIO_PERM_IOCC_ERR)) {
	    /* if retries exhausted, and not an iocc internal error */
	    /* then gracefully back-out, else, allow to either      */
	    /* complete or timeout                                  */
	    w_stop(&ap->wdog.dog);
	    ap->MB30_in_use = -1;	/* release mbox 30 */
	    ap->proc_waiting = 0;
	    for (temp_lun = 0; temp_lun < 8; temp_lun++) {
		t_index = INDEX(ap->p_scsi_id, temp_lun);
		ap->dev[t_index].init_cmd = 0;	/* allow cmds to be issued */
	    }
	    ret_code = EIO;	/* indicate error */
            unlock_enable(old_pri, &(hsc_mp_lock));	/* unmask intrpts */
	    goto end;
	}
    }

    e_sleep_thread(&ap->event, &(hsc_mp_lock), LOCK_HANDLER);

    /* test the return status to set up the return code */
    switch (ap->proc_results) {
      case GOOD_COMPLETION:
	ret_code = 0;
	break;
      case TIMED_OUT:
	ret_code = ETIMEDOUT;
	break;
      case FATAL_ERROR:
	ret_code = EIO;
	break;
      case SEE_RC_STAT:
	if ((ap->mb30_rc == SCSI_BUS_RESET) ||
	    (ap->mb30_rc == PREVIOUS_ERROR))
	    ret_code = 0;
	else
	    if (ap->mb30_rc == COMPLETE_WITH_ERRORS)
		if (ap->mb30_extra_stat ==
		    UNEXPECTED_BUS_FREE)
		    ret_code = 0;
		else {	/* unknown response to INIT cmd */
		    ret_code = EIO;
		    hsc_logerr(ap, ERRID_SCSI_ERR4, ap->MB30p,
			       UNKNOWN_CARD_ERR, 51, 0);
		}
	break;
      default:
	ret_code = EIO;
	break;
    }	/* endswitch */


    /* command delay after reset logic follows: */
    ap->wdog2.dog.restart = ap->ddi.cmd_delay;
    w_start(&ap->wdog2.dog);	/* start delay timer */
    ap->cdar_scsi_ids |= (0x01 << ap->p_scsi_id);	/* set SCSI ID which is
							   affected      */
    /* if any device had active commands when the reset was issued, the  */
    /* interrupt handler would have failed its queue due to the "term by */
    /* init cmd" status.  However, if no cmds had been active, and if    */
    /* one or more cmds came in while the mb30 cmd was being issued,     */
    /* then we must re-start those devices to kick-off pending I/O.      */
    for (temp_lun = 0; temp_lun < 8; temp_lun++) {
	t_index = INDEX(ap->p_scsi_id, temp_lun);
	ap->dev[t_index].init_cmd = 0;	/* allow cmds to be issued */
	if (ap->dev[t_index].opened)
	    hsc_start(ap, t_index);	/* kick-off this device */
    }

    unlock_enable(old_pri, &(hsc_mp_lock));	/* unmask intrpts */

end:
    return (ret_code);

}  /* end hsc_reset_dev */

/************************************************************************/
/*                                                                      */
/* NAME:        hsc_release                                             */
/*                                                                      */
/* FUNCTION:    Free an sc_buf from an Internal Command.                */
/*                                                                      */
/*      This routine frees the memory page which was allocated by       */
/*      hsc_bld_sc_buf.                                                 */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called only from the process level.         */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      sc_buf  - input/output request struct used between the adapter  */
/*                driver and the calling SCSI device driver             */
/*                                                                      */
/* INPUTS:                                                              */
/*      scp     - pointer to page to be freed                           */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  none                                      */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      xmfree                                                          */
/*                                                                      */
/************************************************************************/
void
hsc_release(
	    struct sc_buf * scp)
{

    (void) xmfree((void *) scp, pinned_heap);
    return;

}  /* end hsc_release */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_scsi_reset                                          */
/*                                                                      */
/* FUNCTION:    Execute a SCSI Bus Reset.                               */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called by any other routine.                */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to the adapter structure                      */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      0       - successful                                            */
/*      EIO     - a permanent PIO error occurred                        */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      pio_assist      bzero                                           */
/*                                                                      */
/************************************************************************/
int
hsc_scsi_reset(
	       struct adapter_def * ap)
{
    uchar   trash;
    struct io_parms iop;


    if (ap->reset_pending) {
        /* indicates a bus reset has been executed without yet receiving */
        /* the MB31 interrupt.  There is therefore no need to reset the  */
        /* bus again */
        return(0);
    }
    /* init iop structure */
    bzero(&iop, sizeof(struct io_parms));
    iop.ap = ap;	/* init adap pointer */

    /* generate a SCSI Bus Reset on internal and external busses */
    iop.opt = SE_SET;
    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	return (EIO);	/* error--pio operation failed */
    }

    /* execute a delay loop for at least 25 microseconds */
    iop.data = 30;
    iop.opt = SYNC_DELAY;
    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	/* unrecovered error attempting to do sync. delay. */
	/* try to turn off reset line before exitting.     */
	iop.opt = SE_RESET;
	(void) pio_assist(&iop, hsc_pio_function, hsc_pio_recov);
	return (EIO);	/* error--pio operation failed */
    }

    /* de-assert SCSI Bus Reset line on both busses */
    iop.opt = SE_RESET;
    if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	return (EIO);	/* error--pio operation failed */
    }

    ap->reset_pending = TRUE;
    return (0);	/* successful SCSI Bus Reset */

}  /* end hsc_scsi_reset */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_logerr                                              */
/*                                                                      */
/* FUNCTION:    Adapter Driver Error Logging Routine                    */
/*                                                                      */
/*      This routine provides a common point through which all driver   */
/*      error logging passes.                                           */
/*                                                                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This is an internal routine which can be called from any        */
/*      other driver routine.                                           */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      mbstruct - driver structure which holds information related to  */
/*                 a particular mailbox and hence a command             */
/*      error_log_def - adapter driver error logging information        */
/*                      structure                                       */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to the adapter structure                      */
/*      errid   - the error log unique id for this entry                */
/*      mbp     - pointer to mailbox, if applicable                     */
/*      ahs     - additional hardware status for this entry             */
/*      errnum  - a unique value which identifies which error is        */
/*                causing this call to log the error                    */
/*      data1   - additional, error dependent data                      */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      bcopy           bzero                                           */
/*      errsave         pio_assist                                      */
/*                                                                      */
/************************************************************************/
void
hsc_logerr(
	   struct adapter_def * ap,
	   int errid,
	   struct mbstruct * mbp,
	   int ahs,
	   int errnum,
	   int data1)
{
    struct error_log_def log;
    caddr_t iocc_addr;
    struct io_parms iop;

    if (ap->errlog_enable) {

	bzero(&log, sizeof(struct error_log_def));	/* init the logging
							   struct */
	log.errhead.error_id = (uint) errid;
	bcopy(&ap->ddi.resource_name[0], &log.errhead.resource_name[0],
	      ERR_NAMESIZE);
	log.data.diag_validity = 0;
	log.data.diag_stat = 0;
	if (ahs != 0) {
	    log.data.ahs_validity = 0x01;
	    log.data.ahs = (uchar) ahs;
	}
	log.data.un.card1.errnum = (uint) errnum;

	if (mbp != NULL) {	/* if the mailbox pointer is valid */
	    bcopy(&mbp->id0, &log.data.un.card1.mb_num, 4);
	    log.data.un.card1.mb_num &= 0xff;
	    log.data.un.card1.mb_addr = ap->ddi.base_addr +
		(uint) MB_ADDR(mbp->MB_num);
	    if (mbp->sc_buf_ptr != NULL) {
		log.data.un.card1.buf_addr = (uint) mbp->sc_buf_ptr->
		    bufstruct.b_un.b_addr;
		log.data.un.card1.x_aspace_id =
		    mbp->sc_buf_ptr->bufstruct.b_xmemd.aspace_id;
		log.data.un.card1.x_subspace_id =
		    mbp->sc_buf_ptr->bufstruct.b_xmemd.subspace_id;
	    }
	    /* copy the mailbox data, from the sc_buf pointer to the end */
	    bcopy(&mbp->sc_buf_ptr, &log.data.un.card1.scb_ptr, 52);
	}

	/* fill in the mailbox 31 status */
	bcopy(&ap->MB31p->mb.m_resid, &log.data.un.card1.mb31_status[0],
	      MB_STAT_SIZE);

	/* additional data defaults to zero */

	/* see if additional data is needed due to error id */
	if (((uint) errid == (uint) ERRID_SCSI_ERR7) ||
	    ((uint) errid == (uint) ERRID_SCSI_ERR8))
	    log.data.sys_rc = (uint) data1;

	/* see if additional data is needed due to pio error id */
	if ((((uint) errid == (uint) ERRID_SCSI_ERR1) ||
	     ((uint) errid == (uint) ERRID_SCSI_ERR2))
	    && (data1 != 0) && (mbp == NULL)) {
	    log.data.sys_rc = ((struct pio_except *) data1)->pio_csr;
	    log.data.un.card1.mb_addr = ((struct pio_except *) data1)->pio_dar;
	}

	/* see if additional data is needed due to ahs value */
	if ((uint) ahs & DMA_ERROR)
	    log.data.sys_rc = (uint) data1;
	else
	    if ((uint) ahs & CARD_INTRPT_ERR)
		/* store isr (already swapped) */
		log.data.un.card1.isr_val = (uint) data1;
	    else
		if ((uint) ahs & UNKNOWN_CARD_ERR) {
		    iop.ap = ap;
		    iop.data = 0;
		    iop.opt = READ_BCR;
		    (void) pio_assist(&iop, hsc_pio_function, hsc_pio_recov);
		    /* whether pio worked or not, move data to err log */
		    /* struct.                                         */
		    log.data.un.card1.bcr_val = (uchar) iop.data;

		    /* get POS register data. directly read the regs here  */
		    /* to avoid another call to error log routine due to   */
		    /* error.                                              */
		    /* access the IOCC */
		    iocc_addr = IOCC_ATT(ap->ddi.bus_id, 0);
		    log.data.un.card1.pos0_val =
			BUSIO_GETC((iocc_addr + (ap->ddi.slot << 16) + POS0));
		    log.data.un.card1.pos1_val =
			BUSIO_GETC((iocc_addr + (ap->ddi.slot << 16) + POS1));
		    log.data.un.card1.pos2_val =
			BUSIO_GETC((iocc_addr + (ap->ddi.slot << 16) + POS2));
		    log.data.un.card1.pos6_val =
			BUSIO_GETC((iocc_addr + (ap->ddi.slot << 16) + POS6));
		    log.data.un.card1.pos7_val =
			BUSIO_GETC((iocc_addr + (ap->ddi.slot << 16) + POS7));
		    /* POS3 read undefined */
		    log.data.un.card1.pos3_val = 0x00;
		    log.data.un.card1.pos4_val =
			BUSIO_GETC((iocc_addr + (ap->ddi.slot << 16) + POS4));
		    log.data.un.card1.pos5_val =
			BUSIO_GETC((iocc_addr + (ap->ddi.slot << 16) + POS5));
		    IOCC_DET(iocc_addr);	/* remove access to the IOCC */
		}

	/* log the error here */
	errsave(&log, sizeof(struct error_log_def));
    }

}  /* end hsc_logerr */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_internal_trace                                      */
/*                                                                      */
/* FUNCTION:    Adapter Driver Internal Trace Routine                   */
/*                                                                      */
/*      This routine provides a common point through which all driver   */
/*      internal tracing takes place.                                   */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This is an internal routine which can be called from any        */
/*      other driver routine.                                           */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      sc_buf  - input/output request struct used between the adapter  */
/*                driver and the calling SCSI device driver             */
/*      mbstruct - driver structure which holds information related to  */
/*                 a particular mailbox and hence a command             */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter being traced                       */
/*      hookid  - identifier of trace point                             */
/*      data1   - operation specific data                               */
/*      data2   - operation specific data                               */
/*      data3   - operation specific data                               */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      disable_lock    unlock_enable                                   */
/*      bzero                                                           */
/*                                                                      */
/************************************************************************/
#ifdef HSC_TRACE
void
hsc_internal_trace(
		   struct adapter_def * ap,
		   int hookid,
		   int data1,
		   int data2,
#ifdef	HSC_NEVER
		   int data3,
		   int data4,
		   int data5)
#else
		   int data3)
#endif	HSC_NEVER
{
    int     n, dev_index, i, trc_inc;
    struct mbstruct *mbp;
    struct sc_buf *scp;
    char   *temp_mbp;
    int     old_pri;


    /* if global trace, and if adapter trace enabled, continue */
    if ((hsc_trace) && (ap->trace_enable)) {

	trc_inc = FALSE;
	switch (hookid) {
	  case TRC_CMD:
	    /* data1 = mbp, data2 = unused, data3 = dev_index */
	    mbp = (struct mbstruct *) data1;
	    dev_index = data3;
	    /* if device trace enabled, continue */
	    if (!ap->dev[dev_index].trace_enable)
		break;
	    hsc_trace_ptr->id0 = (uint) 'CMD ';
	    hsc_trace_ptr->c00 = (uint) ap;
	    hsc_trace_ptr->c01 = (uint) mbp->sc_buf_ptr;
	    hsc_trace_ptr->c02 = (uchar) mbp->MB_num;
	    hsc_trace_ptr->c03 = 0;
	    hsc_trace_ptr->c04 =  data2;
	    hsc_trace_ptr->c05 = (uchar) ap->dev[dev_index].num_act_cmds;
	    hsc_trace_ptr->c06 = (ushort) mbp->tcws_allocated;
	    hsc_trace_ptr->c07 = (ushort) mbp->tcws_start;
	    hsc_trace_ptr->c08 = (uint) mbp->sta_index;
	    hsc_trace_ptr->mb = mbp->mb;
	    trc_inc = TRUE;
	    break;

	  case TRC_DONE:
	    /* data1 = scp, data2 = mbp, data3 = dev_index */
	    scp = (struct sc_buf *) data1;
	    mbp = (struct mbstruct *) data2;
	    dev_index = data3;
	    /* if device trace enabled, continue */
	    if (!ap->dev[dev_index].trace_enable)
		break;
	    hsc_trace_ptr->id0 = (uint) 'DONE';
	    hsc_trace_ptr->c00 = (uint) ap;
	    hsc_trace_ptr->c01 = (uint) scp;
	    hsc_trace_ptr->c03 = 0;
	    hsc_trace_ptr->c04 = 0;
	    hsc_trace_ptr->c05 = (uchar) scp->bufstruct.b_error;
	    hsc_trace_ptr->c06 = (ushort) ((scp->status_validity << 8) |
					   scp->scsi_status);
	    hsc_trace_ptr->c07 = (ushort) (scp->general_card_status << 8);
	    hsc_trace_ptr->c08 = (uint) scp->bufstruct.b_resid;
	    if (mbp != NULL) {
		hsc_trace_ptr->c02 = (uchar) mbp->MB_num;
		hsc_trace_ptr->mb = mbp->mb;
	    }
	    else {
		hsc_trace_ptr->c02 = 0;
		bzero(&hsc_trace_ptr->mb, MB_SIZE);
	    }
	    trc_inc = TRUE;
	    break;

	  case TRC_ISR:
	    /* data1 = isr value, data2 = unused, data3 = unused */
	    hsc_trace_ptr->id0 = (uint) 'ISR ';
	    hsc_trace_ptr->c00 = (uint) ap;
	    hsc_trace_ptr->c01 = (uint) data1;
	    bzero(&hsc_trace_ptr->c02, 44);
	    trc_inc = TRUE;
	    break;

	  case TRC_MB30:
	    /* data1 = trc flag, data2 = unused, data3 = unused */
	    hsc_trace_ptr->id0 = (uint) 'MB30';
	    hsc_trace_ptr->c00 = (uint) ap;
	    hsc_trace_ptr->c01 = (uint) data1;
	    bzero(&hsc_trace_ptr->c02, 12);
	    hsc_trace_ptr->mb = ap->MB30p->mb;
	    trc_inc = TRUE;
	    break;

	  case TRC_MB31:
	    /* data1 = unused, data2 = unused, data3 = unused */
	    hsc_trace_ptr->id0 = (uint) 'MB31';
	    hsc_trace_ptr->c00 = (uint) ap;
	    bzero(&hsc_trace_ptr->c01, 16);
	    hsc_trace_ptr->mb = ap->MB31p->mb;
	    trc_inc = TRUE;
	    break;

#ifdef	HSC_NEVER

	  case TRC_ENAB:
	    hsc_trace_ptr->id0 = (uint) 'ENAB';
	    hsc_trace_ptr->c00 = (uint) ap;
	    hsc_trace_ptr->c01 = (uint) data1;	/* ENT /EXIT */
	    hsc_trace_ptr->c02 = (uchar) data2 & 0xff;	/* option  */
	    hsc_trace_ptr->c03 = (uchar) data3 & 0xff;	/* level     */
	    hsc_trace_ptr->c04 = (uchar) ap->waiting_for_mb30;
	    hsc_trace_ptr->c05 = (uchar) 0;	/* state  */
	    hsc_trace_ptr->c06 = (ushort) ap->proc_waiting;
	    hsc_trace_ptr->c07 = (ushort) data4 & 0xff;	/* return code    */
	    hsc_trace_ptr->c08 = (uint) data5;	/* buf ptr   */
	    hsc_trace_ptr->mb = ap->MB30p->mb;
	    trc_inc = TRUE;
	    break;

	  case TRC_ENID:
	    hsc_trace_ptr->id0 = (uint) 'ENID';
	    hsc_trace_ptr->c00 = (uint) ap;
	    hsc_trace_ptr->c01 = (uint) data1;	/* ENT /EXIT */
	    hsc_trace_ptr->c02 = (uchar) data2 & 0xff;	/* option  */
	    hsc_trace_ptr->c03 = (uchar) data3 & 0xff;	/* level     */
	    hsc_trace_ptr->c04 = (uchar) ap->waiting_for_mb30;
	    hsc_trace_ptr->c05 = (uchar) 0;	/* state  */
	    hsc_trace_ptr->c06 = (ushort) ap->proc_waiting;
	    hsc_trace_ptr->c07 = (ushort) data4 & 0xff;	/* return code    */
	    hsc_trace_ptr->c08 = (uint) data5;	/* id */
	    hsc_trace_ptr->mb = ap->MB30p->mb;
	    trc_inc = TRUE;
	    break;

	  case TRC_STRTBUF:
	    hsc_trace_ptr->id0 = (uint) 'STBF';
	    hsc_trace_ptr->c00 = (uint) ap;
	    hsc_trace_ptr->c01 = (uint) data1;	/* ENT /EXIT */
	    hsc_trace_ptr->c02 = (uchar) 0;	/* devindex  */
	    hsc_trace_ptr->c03 = (uchar) data2 & 0xff;	/* level     */
	    hsc_trace_ptr->c04 = (uchar) ap->waiting_for_mb30;
	    hsc_trace_ptr->c05 = (uchar) 0;	/* state */
	    hsc_trace_ptr->c06 = (ushort) ap->proc_waiting;
	    hsc_trace_ptr->c07 = (ushort) data3 & 0xff;	/* rc from enabuf    */
	    hsc_trace_ptr->c08 = (uint) 0;
	    hsc_trace_ptr->mb = ap->MB30p->mb;
	    trc_inc = TRUE;
	    break;

	  case TRC_STRTID:
	    hsc_trace_ptr->id0 = (uint) 'STID';
	    hsc_trace_ptr->c00 = (uint) ap;
	    hsc_trace_ptr->c01 = (uint) data1;	/* ENT /EXIT */
	    hsc_trace_ptr->c02 = (uchar) 0;	/* devindex  */
	    hsc_trace_ptr->c03 = (uchar) data2 & 0xff;	/* level     */
	    hsc_trace_ptr->c04 = (uchar) ap->waiting_for_mb30;
	    hsc_trace_ptr->c05 = (uchar) 0;	/* state */
	    hsc_trace_ptr->c06 = (ushort) ap->proc_waiting;
	    hsc_trace_ptr->c07 = (ushort) data3 & 0xff;	/* rc from enaid */
	    hsc_trace_ptr->c08 = (uint) 0;
	    hsc_trace_ptr->mb = ap->MB30p->mb;
	    trc_inc = TRUE;
	    break;

	  case TRC_STOPID:
	    hsc_trace_ptr->id0 = (uint) 'SPID';
	    hsc_trace_ptr->c00 = (uint) ap;
	    hsc_trace_ptr->c01 = (uint) data1;	/* ENT /EXIT */
	    hsc_trace_ptr->c02 = (uchar) 0;	/* devindex  */
	    hsc_trace_ptr->c03 = (uchar) data2 & 0xff;	/* level     */
	    hsc_trace_ptr->c04 = (uchar) ap->waiting_for_mb30;
	    hsc_trace_ptr->c05 = (uchar) 0;	/* state */
	    hsc_trace_ptr->c06 = (ushort) ap->proc_waiting;
	    hsc_trace_ptr->c07 = (ushort) data3 & 0xff;	/* rc from enaid */
	    hsc_trace_ptr->c08 = (uint) 0;
	    hsc_trace_ptr->mb = ap->MB30p->mb;
	    trc_inc = TRUE;
	    break;

	  case TRC_MB30INTR:
	    hsc_trace_ptr->id0 = (uint) 'MB30';
	    hsc_trace_ptr->c00 = (uint) ap;
	    hsc_trace_ptr->c01 = (uint) data1;	/* ENT /EXIT */
	    hsc_trace_ptr->c02 = (uchar) 0;	/* devindex  */
	    hsc_trace_ptr->c03 = (uchar) ap->MB30_in_use;
	    hsc_trace_ptr->c04 = (uchar) ap->waiting_for_mb30;
	    hsc_trace_ptr->c05 = (uchar) 0;	/* state  */
	    hsc_trace_ptr->c06 = (ushort) ap->proc_waiting;
	    hsc_trace_ptr->c07 = (ushort) 0;
	    hsc_trace_ptr->c08 = (uint) 0;
	    hsc_trace_ptr->mb = ap->MB30p->mb;
	    trc_inc = TRUE;
	    break;

#endif HSC_NEVER	

	  case TRC_XXX:
	    /* data1 = n, data2 = mbp, data3 = dev_index */
	    n = data1;
	    mbp = (struct mbstruct *) data2;
	    dev_index = data3;
	    hsc_trace_ptr->id0 = (uint) 'XXX ';
	    hsc_trace_ptr->c00 = (uint) ap;
	    hsc_trace_ptr->c01 = (uint) mbp;
	    hsc_trace_ptr->c02 = (uchar) n;
	    bzero(&hsc_trace_ptr->c03, 11);
	    hsc_trace_ptr->mb = mbp->mb;
	    trc_inc = TRUE;
	    break;

	  default:
	    break;

	}	/* end switch */

	if (trc_inc) {
	    hsc_trace_ptr++;
	    if (hsc_trace_ptr > &hsc_trace_tab[TRACE_ENTRIES - 1])
		hsc_trace_ptr = &hsc_trace_tab[0];
	    hsc_trace_ptr->id0 = (uint) '<<<<';
	    hsc_trace_ptr->c00 = 'LAST';
	}
    }


}  /* end hsc_internal_trace */
#endif HSC_TRACE


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_read_POS                                            */
/*                                                                      */
/* FUNCTION:    Access the specified POS register.                      */
/*                                                                      */
/*      This routine reads from a selected adapter Programmable Option  */
/*      Select (POS) register and returns the data, performing          */
/*      appropriate error detection and recovery.                       */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This is an internal routine which can be called at any          */
/*      point to load a single (8-bit) POS register.                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to the adapter structure                      */
/*      offset  - offset to the selected POS register                   */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code = 8-bits of data if good or recovered error         */
/*                  = 32-bits of -1 if permanent I/O error encountered  */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      none                                                            */
/*                                                                      */
/************************************************************************/
int
hsc_read_POS(
	     struct adapter_def * ap,
	     uint offset)
{
    int     count, ret_code, err_flag;
    uchar   val1, val2;
    caddr_t iocc_addr;

    ret_code = 0;	/* set default return code */
    count = 0;
    iocc_addr = IOCC_ATT(ap->ddi.bus_id, 0);	/* access the IOCC */
    val1 = BUSIO_GETC((iocc_addr + (ap->ddi.slot << 16) + offset));
    val2 = BUSIO_GETC((iocc_addr + (ap->ddi.slot << 16) + offset));
    if (val1 != val2) {
	do {
	    val1 = BUSIO_GETC((iocc_addr + (ap->ddi.slot << 16) + offset));
	    if (count >= MAX_POS_RETRIES) {
		/* log as unrecoverable I/O bus problem */
		hsc_logerr(ap, ERRID_SCSI_ERR1, NULL, PIO_RD_DATA_ERR, 46, 0);
		ret_code = -1;
		goto end;
	    }

	    count++;
	    val2 = BUSIO_GETC((iocc_addr + (ap->ddi.slot << 16) + offset));

	} while (val2 != val1);

	/* log as recovered I/O bus problem */
	hsc_logerr(ap, ERRID_SCSI_ERR2, NULL, PIO_RD_DATA_ERR, 47, 0);
    }
    ret_code = val1;	/* good completion--return data */
end:
    IOCC_DET(iocc_addr);	/* release the IOCC */
    return (ret_code);
}  /* end hsc_read_POS */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_write_POS                                           */
/*                                                                      */
/* FUNCTION:    Store passed data in specified POS register.            */
/*                                                                      */
/*      This routine loads a selected adapter Programmable Option       */
/*      Select (POS) register with the passed data value, performing    */
/*      appropriate error detection and recovery.                       */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This is an internal routine which can be called at any          */
/*      point to load a single (8-bit) POS register.                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to the adapter structure                      */
/*      offset  - offset to the selected POS reg                        */
/*      data    - 8-bit value to be loaded                              */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code =  0 if good data or recovered error                */
/*                  = -1 if permanent I/O error encountered             */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      none                                                            */
/*                                                                      */
/************************************************************************/
int
hsc_write_POS(
	      struct adapter_def * ap,
	      uint offset,
	      uchar data)
{
    int     count, ret_code;
    caddr_t iocc_addr;
    uchar   val1;

    ret_code = 0;	/* set default return code */
    count = 0;
    iocc_addr = IOCC_ATT(ap->ddi.bus_id, 0);	/* access the IOCC */
    BUSIO_PUTC((iocc_addr + (ap->ddi.slot << 16) + offset), data);
    val1 = BUSIO_GETC((iocc_addr + (ap->ddi.slot << 16) + offset));
    if (data != val1) {
	do {
	    BUSIO_PUTC((iocc_addr + (ap->ddi.slot << 16) + offset), data);
	    if (count >= MAX_POS_RETRIES) {
		/* log as unrecoverable I/O bus problem */
		hsc_logerr(ap, ERRID_SCSI_ERR1, NULL, PIO_WR_DATA_ERR, 48, 0);
		ret_code = -1;
		goto end;
	    }

	    count++;
	    val1 = BUSIO_GETC((iocc_addr + (ap->ddi.slot << 16) + offset));

	} while (data != val1);

	/* log as recovered I/O bus problem */
	hsc_logerr(ap, ERRID_SCSI_ERR2, NULL, PIO_WR_DATA_ERR, 49, 0);
    }
end:
    IOCC_DET(iocc_addr);	/* remove access to the IOCC */
    return (ret_code);
}  /* end hsc_write_POS */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_pio_function                                        */
/*                                                                      */
/* FUNCTION:    Perform low-level PIO functions.                        */
/*                                                                      */
/*      This routine executes the low-level pio function to the hard-   */
/*      ware, setting up appropriate parameters to allow synchronous    */
/*      error recovery, if needed.                                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This is an internal routine which can be called at any          */
/*      time.                                                           */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      io_parms - structure to communicate parms needed to execute i/o */
/*                                                                      */
/* INPUTS:                                                              */
/*      parms   - pointer to parameters to perform the i/o              */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code =  0 for successful                                 */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      none                                                            */
/*                                                                      */
/************************************************************************/
int
hsc_pio_function(
		 caddr_t parms)
{
    struct io_parms *iop;
    uchar   trash;
    int     i, ret_code;
    ulong *tempptr;

    ret_code = 0;

    iop = (struct io_parms *) parms;	/* generate pointer to parameter blk */

    /* get addressability to I/O bus memory space */
    iop->mem_addr = BUSMEM_ATT(iop->ap->ddi.bus_id, iop->ap->ddi.base_addr);

    switch (iop->opt) {	/* determine which option to execute */

      case SI_RESET:	/* reset the internal SCSI Reset line */
	trash = BUS_GETC(iop->mem_addr + BCR_SI);
	break;
      case SE_RESET:	/* reset the external SCSI Reset line */
	trash = BUS_GETC(iop->mem_addr + BCR_SE);
	break;
      case DMA_DISABLE:	/* disable adapter DMA */
	trash = BUS_GETC(iop->mem_addr + BCR_DMA);
	break;
      case INT_DISABLE:	/* disable adapter Interrupts */
	trash = BUS_GETC(iop->mem_addr + BCR_INT);
	break;
      case READ_BCR:	/* read the basic control register */
	iop->data = BUS_GETC(iop->mem_addr + BCR);
	break;
      case READ_ISR:	/* read the interrupt status register */
	iop->data = BUS_GETL(iop->mem_addr + ISR);
	break;
      case READ_SRR:	/* read the shadow read register */
	iop->data = BUS_GETL(iop->mem_addr + SRR);
	break;
      case RD_MB30_STAT:	/* read mailbox 30 status area */
	/* get mb30 status (dest, source, count) */
	BUS_GETSTR(&iop->ap->MB30p->mb.m_resid,
		   iop->mem_addr + MB_ADDR(30) + 24,
		   MB_STAT_SIZE);
	break;
      case RD_MBOX_STAT:	/* read mailbox status area */
	/* get mbox status (dest, source, count) */
	BUS_GETSTR(&iop->mbp->mb.m_resid,
		   iop->mem_addr + MB_ADDR(iop->mbp->MB_num) + 24,
		   MB_STAT_SIZE);
	break;
      case RD_ALL_MBOX_STAT:	/* read mailbox status area. all 32 bytes */
	/* get mbox status (dest, source, count) */
	BUS_GETSTR(&iop->mbp->mb.m_op_code,
		   iop->mem_addr + MB_ADDR(iop->mbp->MB_num),
		   MB_SIZE);
	break;
      case SI_SET:	/* set the internal SCSI Reset line */
	BUS_PUTC((iop->mem_addr + BCR_SI), 0x00);
	break;
      case SE_SET:	/* set the external SCSI Reset line */
	BUS_PUTC((iop->mem_addr + BCR_SE), 0x00);
	break;
      case DMA_ENABLE:	/* enable adapter DMA */
	BUS_PUTC((iop->mem_addr + BCR_DMA), 0x00);
	break;
      case INT_ENABLE:	/* enable adapter interrupts */
	BUS_PUTC((iop->mem_addr + BCR_INT), 0x00);
	break;
      case WRITE_MB30:	/* write mailbox 30 */
	/* write mb30 cmd to card (dest, source, count) */
	BUS_PUTSTR(iop->mem_addr + MB_ADDR(30),
		   &iop->ap->MB30p->mb.m_op_code,
		   MB_SIZE);

	/* execute internal trace point for this command */
#ifdef HSC_TRACE
	hsc_internal_trace(iop->ap, TRC_MB30, 'BLD ', 0, 0);
#endif HSC_TRACE

	break;
      case WRITE_MBOX:	/* write mailbox */
	/* write mbox cmd to card (dest, source, count) */
	BUS_PUTSTR(iop->mem_addr + MB_ADDR(iop->mbp->MB_num),
		   &iop->mbp->mb.m_op_code,
		   MB_SIZE);
	break;
      case WRITE_MB31: /* write mailbox 31 */
        /* write first 24 bytes only in streaming mode */
        tempptr = (ulong *)(iop->mem_addr + MB_ADDR(31));
        BUS_PUTSTR(tempptr,
                   &iop->mbp->mb.m_op_code,
                   MB_SIZE-8);
        /* write last 8 bytes as two words (always 0 here) */
        BUS_PUTL((tempptr + 6), 0);
        BUS_PUTL((tempptr + 7), 0);
        break;
      case SYNC_DELAY:	/* write the IOCC delay register */
	/* this performs a synchronous delay via iocc facility */
	/* the data parameter specifies the number of mics delay */
	iop->iocc_addr = IOCC_ATT(iop->ap->ddi.bus_id, 0);
	for (i = 0; i < iop->data; i++) {
	    /* this delays for 1 microsecond */
	    BUSIO_PUTC((iop->iocc_addr + 0xe0), 0x00);
	}
	IOCC_DET(iop->iocc_addr);
	break;
      case WRITE_MC_BLOCK:	/* write a block of microcode to adapter */
	/* this performs a write of 1024 bytes of data (microcode) to    */
        /* the adapter.  The last two words must be sent a word at a time*/
        /* due to a defect in some adapters when used in an XIO machine  */
        tempptr = (ulong *)(iop->mem_addr + MB_ADDR(iop->mbp->MB_num));
	BUS_PUTSTR(tempptr,
		   iop->sptr,
		   MC_BLK_SIZE-8);
        BUS_PUTL((tempptr + ((MC_BLK_SIZE-8)/4)), *((ulong *)((iop->sptr)+MC_BLK_SIZE-8)));
        BUS_PUTL((tempptr + ((MC_BLK_SIZE-4)/4)), *((ulong *)((iop->sptr)+MC_BLK_SIZE-4)));
	break;
      default:
	break;


    }	/* end switch */

    /* release addressability to I/O bus memory space */
    BUSMEM_DET(iop->mem_addr);

    return (ret_code);

}  /* end hsc_pio_function */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_pio_recov                                           */
/*                                                                      */
/* FUNCTION:    Synchronous PIO-Error Exception Handler                 */
/*                                                                      */
/*      This routine handles recovery (if possible) of synchronous      */
/*      errors associated with PIO operations.                          */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This is an internal routine which is called by the system       */
/*      first level exception handler.                                  */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      io_parms - structure to communicate parms needed to execute i/o */
/*      pio_except - system PIO exception information structure         */
/*                                                                      */
/* INPUTS:                                                              */
/*      parms   - pointer to parameters to perform the i/o              */
/*      action  - flag from pio_assist kernel service to indicate if    */
/*                retries should be executed                            */
/*      infop   - pointer to exception information structure            */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code =  0 for successful                                 */
/*                     EIO for retries exhausted                        */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      none                                                            */
/*                                                                      */
/************************************************************************/
int
hsc_pio_recov(
	      caddr_t parms,
	      int action,
	      struct pio_except * infop)
{
    struct io_parms *iop;
    int     ret_code;
    int     data_read, data_err, logtype, not_recov;

    ret_code = EXCEPT_HANDLED;	/* default to exception handled */
    not_recov = FALSE;	/* default to recoverable type error */
    data_err = FALSE;	/* default to non-data parity error */
    data_read = FALSE;	/* default to say data not actually read */
    logtype = 0;	/* default to no type information */

    iop = (struct io_parms *) parms;	/* generate pointer to parameter blk */

    /* resolve error characteristics */
    switch (infop->pio_csr & PIO_EXCP_MASK) {
      case PIO_EXCP_INV_OP:
	/* fall through to next case ... */
      case PIO_EXCP_LIMIT:
	/* fall through to next case ... */
      case PIO_EXCP_PROT:
	/* fall through to next case ... */
      case PIO_EXCP_PFAULT:
	/* this is an unrecoverable error which should not */
	/* occur after initial debug of PIO operations     */
	not_recov = TRUE;
	break;
      case PIO_EXCP_CHNCHK:
	/* some other adapter drove channel check, as this one */
	/* does not drive that signal.  This must have been due */
	/* to an address parity error, so the read/write did   */
	/* NOT take place.                                     */
	break;
      case PIO_EXCP_DPRTY:
	/* a data parity error occurred on the operation (implies */
	/* a read was attempted).  The PIO operation WAS performed */
	data_read = TRUE;	/* say data WAS actually read */
	data_err = TRUE;	/* say data parity error occurred */
	break;
      case PIO_EXCP_NORSP:
	/* the card did not respond to the address driven on the */
	/* I/O bus.  The PIO operation was NOT performed         */
	break;
      case PIO_EXCP_STECC:
	/* fall through to next case ... */
      case PIO_EXCP_STADR:
	/* fall through to next case ... */
      case PIO_EXCP_TPRTY:
	/* this is an unrecoverable error which should not */
	/* occur as TCWs are not used in this device       */
	/* driver's PIO operations                         */
	not_recov = TRUE;
	break;
      case PIO_EXCP_IOCC:
	/* the IOCC detected an internal error during the operation. */
	/* It is not known if the operation was performed or not.    */
	/* For the ISR read, treat as if operation did NOT occur.    */
	iop->iocc_err = TRUE;	/* flag iocc error   */
	iop->data = 0;	/* cannot trust the data     */
	break;
      default:
	/* Assume the PIO operation was NOT performed.        */
	break;
    }	/* end switch */

    /* determine type of error for logging purposes */
    if (iop->opt <= MAX_PIO_READ_DEFINE) {	/* if this is a read
						   operation */
	if (data_err)
	    logtype = PIO_RD_DATA_ERR;
	else
	    logtype = PIO_RD_OTHR_ERR;
    }
    else {	/* this is a write operation */
	if (data_err)
	    logtype = PIO_WR_DATA_ERR;
	else
	    logtype = PIO_WR_OTHR_ERR;
    }

    /* set ahs and eff_addr fields for diagnostics use */
    iop->ahs = (uchar) logtype;
    iop->eff_addr = infop->pio_dar;

    /* initially set errtype to either temp data or temp other error */
    if ((logtype == PIO_RD_DATA_ERR) || (logtype == PIO_WR_DATA_ERR))
	iop->errtype = PIO_TEMP_DATA_ERR;
    else
	iop->errtype = PIO_TEMP_OTHR_ERR;

    /* if IOCC op was in progress, release addressability to iocc space */
    if (iop->opt == SYNC_DELAY)
	IOCC_DET(iop->iocc_addr);

    /* release addressability to I/O bus memory space */
    BUSMEM_DET(iop->mem_addr);

    /* if no retries flag is set, log error and return */
    if (action == PIO_NO_RETRY) {
	/* log as permanent hardware error with appropriate status. the */
	/* passed infop will be used to get the csr and dar of the error */
	hsc_logerr(iop->ap, ERRID_SCSI_ERR1, NULL, logtype, 64, infop);
	iop->data = 0;	/* data is not reliable */
	iop->errtype = (iop->errtype == PIO_TEMP_DATA_ERR) ?
	    PIO_PERM_DATA_ERR : PIO_PERM_OTHR_ERR;
	/* override errtype if iocc err */
	if ((infop->pio_csr & PIO_EXCP_MASK) == PIO_EXCP_IOCC)
	    iop->errtype = PIO_PERM_IOCC_ERR;
	return (EIO);
    }

    /* if an error that should never occur, log and pass control back up */
    if (not_recov) {
	/* log as permanent hardware error with appropriate status.  the */
	/* passed infop will be used to get the csr and dar of the error */
	hsc_logerr(iop->ap, ERRID_SCSI_ERR1, NULL, logtype, 65, infop);
	iop->errtype = (iop->errtype == PIO_TEMP_DATA_ERR) ?
	    PIO_PERM_DATA_ERR : PIO_PERM_OTHR_ERR;
	return (EXCEPT_NOT_HANDLED);
    }

    /* if this is a read of ISR, and data read WAS performed */
    if ((iop->opt == READ_ISR) && (data_read)) {
	/* log as temporary hardware error with appropriate status.  the */
	/* passed infop will be used to get the csr and dar of the error */
	hsc_logerr(iop->ap, ERRID_SCSI_ERR2, NULL, logtype, 66, infop);
	/* errtype already set above, and this can't be an iocc error */

	/* now set up io_parm struct for the retry to the SRR */
	iop->opt = READ_SRR;
	iop->data = 0;
	(void) hsc_pio_function((caddr_t) iop);

	/* if any previous iocc internal error on an ISR/SRR read, set err
	   bit */
	if (((iop->opt == READ_ISR) || (iop->opt == READ_SRR)) &&
	    (iop->iocc_err == TRUE)) {
	    iop->data |= 0x00000080;	/* set isr error bit */
	}

	return (ret_code);
    }

    /* log as temporary hardware error with appropriate status */
    /* the passed infop will be used to get the csr and dar of the error */
    hsc_logerr(iop->ap, ERRID_SCSI_ERR2, NULL, logtype, 67, infop);
    /* override errtype if iocc err */
    if ((infop->pio_csr & PIO_EXCP_MASK) == PIO_EXCP_IOCC)
	iop->errtype = PIO_TEMP_IOCC_ERR;

    /* if mailbox write, and iocc internal error on this attempt, do not */
    /* retry--treat as if write occurred.  If it did not, timeout will   */
    /* finish the cmd.                                                   */
    if ((iop->opt >= MIN_STR_PIO_WR_DEF) &&
	((infop->pio_csr & PIO_EXCP_MASK) == PIO_EXCP_IOCC)) {
	return (ret_code);
    }

    /* now, recall pio_function to retry the failed pio operation */
    iop->data = 0;	/* throw out previous data */
    (void) hsc_pio_function((caddr_t) iop);

    /* if any previous iocc internal error on an ISR/SRR read, set err bit */
    if (((iop->opt == READ_ISR) || (iop->opt == READ_SRR)) &&
	(iop->iocc_err == TRUE)) {
	iop->data |= 0x00000080;	/* set isr error bit */
    }

    return (ret_code);

}  /* end hsc_pio_recov */

/************************************************************************/
/*                                                                      */
/* NAME:        hsc_deq_active                                          */
/*                                                                      */
/* FUNCTION:    remove sc_buf from the active queue  for an             */
/*              adapter/device combination                              */
/*              NOTE : the sc_buf can be in any position on the active  */
/*                     queue                                            */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This is an internal routine which can be called on the process  */
/*      or interrupt level but interrupt MUST be disabled when this     */
/*      routine is called                                               */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      sc_buf  - input/output request struct used between the adapter  */
/*                driver and the calling SCSI device driver             */
/*      dev_info    - device unique data structure  (one per device)    */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter information structure              */
/*      scp     - pointer to sc_buf being processed                     */
/*      dev_index - index to information of device being processed      */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      No value is returned (void function)                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      none                                                            */
/*                                                                      */
/************************************************************************/
void
hsc_deq_active(
	     struct adapter_def * ap,
	     struct sc_buf * scp,
	     int dev_index)
{
    if (ap->dev[dev_index].head_act == ap->dev[dev_index].tail_act) {
        ap->dev[dev_index].head_act = NULL;
        ap->dev[dev_index].tail_act = NULL;
    }
    else  
        if (ap->dev[dev_index].head_act == scp)  { /* first one */
            ap->dev[dev_index].head_act = 
                              (struct sc_buf *)scp->bufstruct.av_forw;
            ap->dev[dev_index].head_act->bufstruct.av_back = NULL;
        } 
        else
            if (ap->dev[dev_index].tail_act == scp)  { /* last one */
                ap->dev[dev_index].tail_act = 
                              (struct sc_buf *)scp->bufstruct.av_back;
            ap->dev[dev_index].tail_act->bufstruct.av_forw = NULL;
            }
            else {  /* in the middle */
                scp->bufstruct.av_back->av_forw =
                        (struct buf *)scp->bufstruct.av_forw;
                scp->bufstruct.av_forw->av_back =
                        (struct buf *)scp->bufstruct.av_back;
           }
    scp->bufstruct.av_forw = NULL;                  
    scp->bufstruct.av_back = NULL;                  
 
} /* end hsc_deq_active */
