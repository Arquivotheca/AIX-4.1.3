static char sccsid[] = "@(#)64	1.1.2.3  src/bos/kernext/scsi/hscdump.c, sysxscsi, bos411, 9428A410j 1/24/94 11:22:36";
/*
 * COMPONENT_NAME: (SYSXSCSI) IBM SCSI Adapter Driver
 *
 * FUNCTIONS:	hsc_cdt_func, hsc_dump, hsc_dumpwrite
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991, 1992
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
/* NAME:        hscdump.c                                               */
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
/* NAME:        hsc_cdt_func                                            */
/*                                                                      */
/* FUNCTION:    Adapter Driver Component Dump Table Routine             */
/*                                                                      */
/*      This routine builds the driver dump table during a system dump. */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine runs on the interrupt level, so it cannot malloc   */
/*      or free memory.                                                 */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      cdt     - the structure used in the master dump table to keep   */
/*                track of component dump entries.                      */
/*                                                                      */
/* INPUTS:                                                              */
/*      arg     - =1 dump dd is starting to get dump table entries.     */
/*                =2 dump dd is finished getting the dump table entries.*/
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      Return code is a pointer to the struct cdt to be dumped for     */
/*      this component.                                                 */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      bcopy           bzero                                           */
/*      strcpy                                                          */
/*                                                                      */
/************************************************************************/
struct cdt *
hsc_cdt_func(
	     int arg)
{
    int     i, d_index, size;
    struct adapter_def *ap;

    if (arg == 1) {
	/* only build the dump table on the initial dump call */

	/* init the table */
	bzero((char *) hsc_cdt, sizeof(struct hsc_cdt_tab));

	/* init the head struct */
	hsc_cdt->hsc_cdt_head._cdt_magic = DMP_MAGIC;
	strcpy(hsc_cdt->hsc_cdt_head._cdt_name, "hscsi");
	/* _cdt_len is filled in below */

	/* now begin filling in elements */
	hsc_cdt->hsc_entry[0].d_segval = NULL;
	strcpy(hsc_cdt->hsc_entry[0].d_name, "tracetbl");
#ifdef HSC_TRACE
	hsc_cdt->hsc_entry[0].d_ptr = (char *) hsc_trace_tab;
	hsc_cdt->hsc_entry[0].d_len = TRACE_ENTRIES *
	    (sizeof(struct trace_element));
#endif HSC_TRACE
	size = sizeof(struct cdt_head) + sizeof(struct cdt_entry);

	/* loop through adapter pointers */
	for (d_index = 1, i = 0; i < MAXADAPTERS; i++) {

	    /* skip over unused slots in adapter table */
	    if (adapter_ptrs[i] == NULL)
		continue;
	    else {
		/* fill in the next element */
		ap = adapter_ptrs[i];
		hsc_cdt->hsc_entry[d_index].d_segval = NULL;
		/* copy name to element:  s, d, l */
		bcopy((char *) ap->ddi.resource_name,
		      (char *) hsc_cdt->hsc_entry[d_index].d_name,
		      8);
		hsc_cdt->hsc_entry[d_index].d_ptr = (char *) ap;
		hsc_cdt->hsc_entry[d_index].d_len =
		    (sizeof(struct adapter_def))
		    + ap->num_tcws + ap->num_tgt_tcws;
		size += sizeof(struct cdt_entry);
		d_index++;
	    }

	}	/* end for */

	/* fill in the actual table size */
	hsc_cdt->hsc_cdt_head._cdt_len = size;
    }
    return ((struct cdt *) hsc_cdt);

}  /* end hsc_cdt_func */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_dump                                                */
/*                                                                      */
/* FUNCTION:    Adapter Driver Dump Routine                             */
/*                                                                      */
/*      This routine handles requests for dumping data to a previously  */
/*      opened device.                                                  */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine is called when there is certain to be limited      */
/*      functionality available in the system.  However, system         */
/*      dma kernel services are available.  The passed data is already  */
/*      pinned by the caller.  There are no interrupt or timer kernel   */
/*      services available.  This routine should run at INTMAX level.   */
/*                                                                      */
/* NOTES:                                                               */
/*      The driver should ignore extra dumpinit and dumpstart calls.    */
/*      The driver must check for the availability of internal          */
/*      resources, as these are not guaranteed to be available          */
/*      when the dump routine is called, but the system will have       */
/*      tried to quiesce itself before making the first dump call.      */
/*      Any lack of resources, or error in attempting to run any        */
/*      command, is considered fatal in the context of the dump.        */
/*      It is assumed that normal operation will not continue once      */
/*      this routine has been executed.  Once the DUMPWRITE logic has   */
/*      been executed, in fact, it will be impossible to use the        */
/*      driver's normal path successfully.                              */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      sc_buf  - input/output request struct used between the adapter  */
/*                driver and the calling SCSI device driver             */
/*      uio     - user i/o area struct                                  */
/*      dmp_query - kernel dump query structure                         */
/*                                                                      */
/* INPUTS:                                                              */
/*      devno   - device major/minor number                             */
/*      uiop    - pointer to uio struct data area                       */
/*      cmd     - parameter specifying the dump operation               */
/*      arg     - pointer to sc_buf if cmd is dumpwrite                 */
/*      chan    - channel number, unused and should be 0                */
/*      ext     - extended system parameter, unused and s/b 0           */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      A zero will be returned on successful completion, otherwise,    */
/*      one of the errno values listed below will be given.             */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      0       - successful completion                                 */
/*      EIO     - kernel service failure, not opened, or general        */
/*                failure running DUMPWRITE option.                     */
/*      EINVAL  - invalid request                                       */
/*      ETIMEDOUT - the DUMPWRITE option timed-out                      */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      i_disable       i_enable                                        */
/*                                                                      */
/************************************************************************/
int
hsc_dump(
	 dev_t devno,
	 struct uio * uiop,
	 int cmd,
	 int arg,
	 int chan,
	 int ext)
{
    struct adapter_def *ap;
    int     i_hash;
    int     rc, ret_code;
    struct dmp_query *dump_ptr;

    ret_code = 0;

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
	goto end;
    }

    switch (cmd) {

      case DUMPINIT:
	ap->dump_inited = TRUE;
	break;

      case DUMPQUERY:
	dump_ptr = (struct dmp_query *) arg;
	dump_ptr->min_tsize = 0;
	dump_ptr->max_tsize = ap->maxxfer;
	break;

      case DUMPSTART:
	if (!ap->dump_inited)
	    ret_code = EINVAL;	/* report error */
	else {
	    ap->dump_started = TRUE;
	    ap->dump_pri = i_disable(INTMAX);	/* keep out intrpts */
	}
	break;

      case DUMPWRITE:
	/* call internal routine to execute the command */
	ret_code = hsc_dumpwrite(ap, (struct sc_buf *) arg);
	break;

      case DUMPEND:
	if ((!ap->dump_inited) || (!ap->dump_started))
	    ret_code = EINVAL;	/* report error */
	else {
	    ap->dump_started = FALSE;
	    i_enable(ap->dump_pri);	/* allow intrpts */
	}
	break;

      case DUMPTERM:
	if ((!ap->dump_inited) || (ap->dump_started))
	    ret_code = EINVAL;	/* report error */
	else
	    ap->dump_inited = FALSE;
	break;

      default:
	ret_code = EINVAL;
	break;

    }	/* end switch (cmd) */

end:
    return (ret_code);

}  /* end hsc_dump */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_dumpwrite                                           */
/*                                                                      */
/* FUNCTION:    Execute a SCSI command to the dump adapter.             */
/*                                                                      */
/*      This routine handles execution of commands during a dump.       */
/*      See notes under the hsc_dump routine.                           */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine runs on the INTMAX interrupt level, so it cannot   */
/*      be interrupted by the interrupt handler.                        */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      sc_buf  - input/output request struct used between the adapter  */
/*                driver and the calling SCSI device driver             */
/*      mbstruct - driver structure which holds information related to  */
/*                 a particular mailbox and hence a command             */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to the adapter information                    */
/*      scp     - pointer to the passed sc_buf                          */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      A zero will be returned on successful completion, otherwise,    */
/*      one of the errno values listed below will be given.             */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      0       - successful completion                                 */
/*      EIO     - device not opened, adapter dead, or general failure   */
/*                executing command                                     */
/*      EINVAL  - invalid request                                       */
/*      ETIMEDOUT - the command timed out before completing             */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      curtime         d_master                                        */
/*      d_complete      pio_assist                                      */
/*                                                                      */
/************************************************************************/
int
hsc_dumpwrite(
	      struct adapter_def * ap,
	      struct sc_buf * scp)
{
    ulong   new_time, start_time;
    int     i, j, cmd_handled, rc, ret_code;
    int     dev_index;
    uint    timeout_value, dma_addr;
    ulong   save_isr;
    struct mbstruct *mbp;
    struct io_parms iop;

    ret_code = 0;	/* set default return code */

    /* validate entry to this routine */
    if ((!ap->dump_inited) || (!ap->dump_started)) {
	ret_code = EINVAL;	/* return error */
	goto end;
    }

    /* get index into device table for this device */
    dev_index = INDEX(scp->scsi_command.scsi_id,
		      (scp->scsi_command.scsi_cmd.lun) >> 5);

    /* force off coallesced or gathered commands */
/* !!! */
    scp->bp = NULL;
    scp->resvd1 = NULL;

    /* miscellaneous validation of request */
    if ((!ap->dev[dev_index].opened) ||
	(scp->bp != NULL) ||
	(scp->resvd1 != NULL) ||
	(scp->bufstruct.b_bcount > ap->maxxfer))
    {
	ret_code = EIO;	/* return error */
	goto exit;
    }

    /* check for prior conditions which prevent running a command */
    if ((ap->adapter_check != 0) || (ap->dev[dev_index].state != 0)
	|| (ap->dev[dev_index].pqstate != 0)
	|| (ap->dev[dev_index].init_cmd != 0)) {
	ret_code = EIO;	/* return error */
	goto exit;
    }

    /* go get a mailbox */
    rc = hsc_MB_alloc(ap, scp, dev_index);
    if (rc == FALSE) {	/* if could not get mailbox */
	ret_code = EIO;	/* return error */
	goto exit;
    }

    /* generate mailbox pointer */
    mbp = (struct mbstruct *) scp->bufstruct.b_work;

    if (scp->bufstruct.b_bcount) {	/* see if TCWs needed */
	/* now, attempt to get TCWs */
	rc = hsc_TCW_alloc(ap, mbp, scp, dev_index);
	if (rc == FALSE) {	/* if could not get TCWs */
	    hsc_MB_restore(ap, mbp);	/* return mailbox */
	    ret_code = EIO;	/* return error */
	    goto exit;
	}
    }
    /* if got here, all resources were available */

    /* build the send SCSI command block. */
    /* set scsi id, data trans/dir,      */
    /* nodisc flag, and async flag       */
    mbp->mb.m_xfer_id =
	(scp->scsi_command.scsi_id & 0x07) |
	((scp->bufstruct.b_bcount) ? HSC_TRANS : 0) |
	((scp->bufstruct.b_flags & B_READ) ? HSC_READ : 0) |
	((scp->scsi_command.flags & SC_NODISC) ? HSC_NODISC : 0) |
	((scp->scsi_command.flags & SC_ASYNC) ? HSC_NOSYNC : 0);

    /* set scsi cmd length, set DMA burst size */
    mbp->mb.m_cmd_len = (scp->scsi_command.scsi_length & 0x0f) | DMA_BURST;
    /* set the DMA addr below */
    mbp->mb.m_dma_addr = 0;
    /* set the DMA length field */
    mbp->mb.m_dma_len = WORD_REVERSE(scp->bufstruct.b_bcount);
    /* store scsi cmd into mb scsi cmd block area */
    mbp->mb.m_scsi_cmd.scsi_op_code = scp->scsi_command.scsi_cmd.scsi_op_code;
    mbp->mb.m_scsi_cmd.lun = scp->scsi_command.scsi_cmd.lun;
    for (i = 0; i < 10; i++) {
	mbp->mb.m_scsi_cmd.scsi_bytes[i] =
	    scp->scsi_command.scsi_cmd.scsi_bytes[i];
    }
    /* init mailbox status area to all 0's */
    mbp->mb.m_resid = 0;
    mbp->mb.m_adapter_rc = 0;
    mbp->mb.m_extra_stat = 0;
    mbp->mb.m_scsi_stat = 0;
    mbp->mb.m_resvd = 0;

    /* if there is data to transfer */
    if (scp->bufstruct.b_bcount) {

	/* calculate the DMA address */
	dma_addr = (uint) (DMA_ADDR(ap->ddi.tcw_start_addr, mbp->tcws_start) +
		      ((uint) scp->bufstruct.b_un.b_addr & (TCWRANGE - 1)));

	/* set mailbox dma address field (swap bytes as needed) */
	mbp->mb.m_dma_addr = WORD_REVERSE(dma_addr);

	/* map the TCWs for the transfer */
	d_master((int) ap->channel_id,
	     DMA_TYPE | ((scp->bufstruct.b_flags & B_READ) ? DMA_READ : 0) |
		 ((scp->bufstruct.b_flags & B_NOHIDE) ? DMA_WRITE_ONLY : 0),
		 scp->bufstruct.b_un.b_addr,
		 (size_t) scp->bufstruct.b_bcount,
		 &scp->bufstruct.b_xmemd,
		 (char *) dma_addr);
    }

    mbp->d_cmpl_done = FALSE;	/* default to d_complete not done */
    /* include any queue tag message value that is in the sc_buf */
    mbp->mb.m_op_code = mbp->mb.m_op_code | (scp->q_tag_msg<<4);


    /* send the command to the adapter */
    iop.ap = ap;
    iop.mbp = mbp;
    iop.opt = WRITE_MBOX;
    iop.errtype = 0;
    if ((pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) &&
	(iop.errtype != PIO_PERM_IOCC_ERR)) {
	/* if retries exhausted, and not an iocc internal error */
	/* then gracefully back-out, else, allow to either      */
	/* complete or timeout                                  */
	ret_code = EIO;
	mbp->cmd_state = INTERRUPT_RECVD;	/* allow mbox to be freed */
	(void) hsc_dma_cleanup(ap, scp, TRUE);	/* cleanup after dma */
	hsc_TCW_dealloc(ap, mbp);	/* release TCWs */
	hsc_MB_dealloc(ap, mbp);	/* release mbox */
	goto exit;	/* leave routine */
    }

    /* now, start a polling loop, waiting for completion, or timeout */
    /* read the system real time clock to set the start time         */
    timeout_value = scp->timeout_value + 1;
    curtime(&ap->time_s);	/* get current system time */
    cmd_handled = FALSE;	/* default flag to cmd not handled */
    for (new_time = start_time = ap->time_s.tv_sec;
	 ((int) new_time - (int) start_time) < (int) timeout_value;
	 curtime(&ap->time_s), new_time = ap->time_s.tv_sec) {

	/* read the interrupt status register */
	iop.opt = READ_ISR;
	iop.mbp = 0;
	iop.iocc_err = FALSE;
	iop.data = 0;
	if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov) == EIO) {
	    /* unrecovered error reading the ISR:  force error */
	    /* bit on so mb31 will be checked; try to continue */
	    /* (timeout may result)                            */
	    iop.data = 0x00000080;	/* set error bit */
	}
	save_isr = WORD_REVERSE(iop.data);	/* get data from io struct */


	if (save_isr == 0)	/* if no interrupt, keep looping */
	    continue;
	else {	/* else, some interrupt occurred */

	    /* if error bit is set */
	    if (save_isr & 0x80000000) {

		/* get mbox 31 status information */
		iop.mbp = ap->MB31p;
		iop.opt = RD_MBOX_STAT;
		if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov)
		    == EIO) {
		    /* cannot get mb31 status, this may or may not be     */
		    /* fatal, so just try to go on here (following clears */
		    /* mb31 return code so test will work).               */
		    ap->MB31p->mb.m_adapter_rc = 0;
		}

		/* handle the various MB31 return codes here */
		switch (ap->MB31p->mb.m_adapter_rc) {

		  case BAD_FUSE:
		    /* this means the card detected term pwr loss.   */
		    /* no further cmds will be accepted by the card. */
		    /* fall through to logic below...                */
		  case ADAP_FATAL:
		    /* this means a fatal hardware error was detected */
		    /* by the card.  no further cmds will be accepted */
		    ap->adapter_check = ADAPTER_DEAD;
		    ret_code = EIO;	/* return error */
		    break;
		  case SCSI_BUS_RESET:
		    /* this is fatal to dump.                        */
		    /* fall through to logic below...                */
		  case COMMAND_PAUSED:
		    /* this means the adapter encountered a hardware err  */
		    /* during IPL diagnostics.  this is fatal to dump.    */
		    ret_code = EIO;	/* return error */
		    break;
		  case UNKNOWN_SELECT:
		    /* this means a device which the adapter did not have */
		    /* a cmd for attempted a valid reselection with the   */
		    /* adapter.  ignore this error, and continue.         */
		    break;
		  default:
		    /* either a good or an        */
		    /* invalid mb31 return code,  */
		    /* try to continue operations */
		    break;

		}	/* endswitch */


		/* N.B.  Must write 0's to all of MB31 area, regardless of */
		/* previous status contents (i.e. good or bad).            */
		/* write 0's to MB31 (internal copy in driver).            */
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
		/* N.B. this re-enables MB31 for next error */
		iop.mbp = ap->MB31p;
		iop.opt = WRITE_MB31;
		if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov)
		    == EIO) {
		    /* cannot send mb31 to clear intrpt.  this may or may */
		    /* not be fatal, so again attempt to continue here    */
		    ;
		}

	    }

	    /* test for mailbox interrupt */
	    /* if mbox not interrupting */
	    if (!(save_isr & (0x01 << mbp->MB_num)))
		continue;	/* no interrupt, keep looping */
	    else {	/* else, mbox is interrupting */

		/* get the mailbox status */
		iop.mbp = mbp;
		iop.opt = RD_MBOX_STAT;
		if (pio_assist(&iop, hsc_pio_function, hsc_pio_recov)
		    == EIO) {
		    ret_code = EIO;	/* error during pio */
		    cmd_handled = TRUE;	/* say cmd handled */
		    /* allow mbox to be freed */
		    mbp->cmd_state = INTERRUPT_RECVD;
		    /* cleanup after dma */
		    (void) hsc_dma_cleanup(ap, scp, TRUE);
		    hsc_TCW_dealloc(ap, mbp);	/* release TCWs */
		    hsc_MB_dealloc(ap, mbp);	/* release mbox */
		    break;	/* leave routine */
		}

		if (mbp->mb.m_adapter_rc != COMPLETE_NO_ERRORS)
		    ret_code = EIO;	/* indicate error */

		/* cleanup after DMA transfer */
		rc = hsc_dma_cleanup(ap, scp, FALSE);

		if (rc != 0)
		    ret_code = EIO;	/* indicate error */

		cmd_handled = TRUE;	/* flag cmd handled */
		/* allow mbox to be freed */
		mbp->cmd_state = INTERRUPT_RECVD;
		hsc_TCW_dealloc(ap, mbp);	/* release TCWs */
		hsc_MB_dealloc(ap, mbp);	/* release mbox */

		break;	/* leave loop */

	    }
	}
    }	/* end for */


    /* find out if we got here due to timeout */
    if (cmd_handled == FALSE) {
	ret_code = ETIMEDOUT;	/* return error */
	mbp->cmd_state = INTERRUPT_RECVD;	/* allow mbox to be freed */
	(void) hsc_dma_cleanup(ap, scp, TRUE);	/* cleanup after dma */
	hsc_TCW_dealloc(ap, mbp);	/* release TCWs */
	hsc_MB_dealloc(ap, mbp);	/* release mbox */
    }

exit:
end:
    return (ret_code);

}  /* end hsc_dumpwrite */
