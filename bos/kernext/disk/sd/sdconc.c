static char sccsid[] = "@(#)31  1.2  src/bos/kernext/disk/sd/sdconc.c, sysxdisk, bos411, 9428A410j 3/28/94 10:04:20";
/*
 * COMPONENT_NAME: (SYSXDISK) Serial Dasd Subsystem Device Driver
 *
 * FUNCTIONS: sd_concurrent(), sd_q_conc_cmd, sd_d_q_conc_cmd(),
 *            sd_build_conc_cmd(), sd_return_conc_cmd
 * 
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/malloc.h>
#include <sys/sd.h>
#include <sys/errno.h>
#include <sys/lvdd.h>
#include <sys/sleep.h>
#include <sys/except.h>
#include <sys/errids.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <sys/ddconc.h>

/*
 * NAME: sd_concurrent
 *
 * FUNCTION: The concurrent mode entry point into the sd device driver.
 *           This function's address will be returned to another kernel 
 *           extension which issues a DD_CONC_REGISTER ioctl.
 *           Thereafter this function can be called from the interrupt 
 *           or process environments to execute concurrent mode commands.
 *
 * *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level
 *      and can not page fault.
 *      The caller's interupt priority level MUST be lower priority than
 *      the sd_device driver's interrupt priority. 
 *
 * (RECOVERY OPERATION:)  None.
 *
 * (DATA STRUCTURES:)     sd_dasd_info 	- DASD Information Structure
 *
 * EXTERNAL PROCEDURES CALLED:
 *   
 *     disable_lock			unlock_enable
 *
 * RETURNS:     return_code.
 */
int sd_concurrent(struct conc_cmd *ccptr)
{  
    uint                opri;
    int                 min;         
    struct sd_dasd_info *dp; 
    
    if (!ccptr)
	return (EINVAL);
    
    min = minor(ccptr->devno);                    /* get minor number */ 
    if (!(min & SD_DASD_MASK))
	return (EINVAL);

    dp = (struct sd_dasd_info *)sd_hash(ccptr->devno); 
    
    if (dp == NULL)
	return (EINVAL);
    
    /*
     * Check that this dasd has been registered for concurrent access
     * (also proves it is open) and he is not unregistering
     */

    if ((!dp->conc_registered) || (dp->unregistering))
	return (EINVAL);

    opri = disable_lock(dp->ap->dds.intr_priority,&(dp->ap->spin_lock)); 
    
    switch (ccptr->cmd_op)
    {
      case DD_CONC_SEND_REFRESH:
      case DD_CONC_LOCK:
      case DD_CONC_UNLOCK:
      case DD_CONC_TEST:
	sd_q_conc_cmd(ccptr,dp);
	break;
	
      default:
	return(EINVAL);
	break;
     }
    sd_start(dp->ap);  
    unlock_enable(opri,&(dp->ap->spin_lock));
    return (0);
}	

/*
 * NAME: sd_q_conc_cmd 
 *
 * FUNCTION: queue a concurrent mode command for processing.
 *
 * EXECUTION ENVIRONMENT: This routine is called by sd_concurrent at the
 *                        interrupt level on which the calling kernel 
 *                        extension is running. It must not page fault.
 *
 * (RECOVERY OPERATION:)  None.
 *
 * (DATA STRUCTURES:)     sd_dasd_info 	- DASD Information Structure
 *                        conc_cmd      - Concurrent mode command request 
 *                                        structure.
 *
 * RETURNS:    none
 */ 
void sd_q_conc_cmd (struct conc_cmd *ccptr, struct sd_dasd_info *dp)
{  
    struct conc_cmd *stepptr;
    
    if (dp->conc_cmd_list == NULL)
    {
	dp->conc_cmd_list = ccptr;
    }
    else 
    {
	stepptr = dp->conc_cmd_list;
	while (stepptr->next != NULL) 
	{
	    stepptr = stepptr->next;
	}
	stepptr->next = ccptr;
    }
    ccptr->next = NULL;
    /*
     * Make sure the affected dasd is in the start chain
     */
    sd_add_chain(dp);
}

/*
 * NAME: sd_d_q_conc_cmd 
 *
 * FUNCTION: dequeue a concurrent mode command 
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level and
 *                        cannot page fault.
 *
 * (RECOVERY OPERATION:)  None.
 *
 * (DATA STRUCTURES:)     sd_dasd_info 	- DASD Information Structure
 *                        conc_cmd      - Concurrent mode command request 
 *                                        structure.
 *
 * RETURNS:    a pointer to the dequeued command if one was queued 
 *             otherwise NULL if no commands are waiting.
 */ 
struct conc_cmd *sd_d_q_conc_cmd (struct sd_dasd_info *dp)
{  
    struct conc_cmd *ccptr;
    ccptr = dp->conc_cmd_list;
    if (ccptr != NULL)
    {
	dp->conc_cmd_list = dp->conc_cmd_list->next; 	
    }
    return (ccptr);
}

/*
 * NAME: sd_build_conc_cmd 
 *
 * FUNCTION: builds a concurrent mode command and places it on the dasd's
 *           error queue
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level and
 *                        cannot page fault.
 *
 * (RECOVERY OPERATION:)  None.
 *
 * (DATA STRUCTURES:)     sd_dasd_info 	- DASD Information Structure
 *                        conc_cmd      - Concurrent mode command request 
 *                                        structure.
 *
 * RETURNS:  viod
 *             
 */ 
void sd_build_conc_cmd (struct sd_dasd_info *dp, struct conc_cmd *ccptr)
{  
    switch (ccptr->cmd_op)
    {
	
      case DD_CONC_LOCK:
	sd_reserve (dp,SD_LOCK);
	break;
	
      case DD_CONC_UNLOCK:	
	sd_release (dp,SD_UNLOCK);
	break;
	
      case DD_CONC_SEND_REFRESH:
	sd_send_msg (dp,ccptr->message);
	break;
	
      case DD_CONC_TEST:
	sd_test_unit_ready (dp,SD_TEST);
	break;
	
      default:
	/*
	 * badly formed conc_cmd_struct should
	 * be rejected by sd_concurrent
	 */
	ASSERT (FALSE);
	break;
    }
    /*
     * Finally, insert a reference to the conc_cmd structure
     * into the concurrent command jacket
     */
    dp->concurrent.conc_cmd_ptr = ccptr;
}

/*
 * NAME: sd_return_conc_cmd 
 *
 * FUNCTION: Deals with returning a concurrent mode command to the caller
 *           Acts as a common point from which all conc commands are
 *           passed back to the registered kernel extension.
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level and
 *                        cannot page fault.
 *
 * (RECOVERY OPERATION:)  None.
 *
 * (DATA STRUCTURES:)     sd_dasd_info 	- DASD Information Structure
 *                        conc_cmd      - Concurrent mode command request 
 *                                        structure.
 *
 * RETURNS:  viod
 *             
 */ 
void sd_return_conc_cmd (struct sd_dasd_info *dp, struct conc_cmd *ccptr)
{
    ASSERT (dp->conc_registered);
    /*
     * return command to caller
     */
    (dp->conc_intr_addr)(ccptr,ccptr->cmd_op,0,0);
    if ((dp->unregistering) && (dp->conc_cmd_list == NULL) &&
	(dp->concurrent.status == SD_FREE))
    {
	/*
	 * This was the last outstanding concurrent command
	 * holding out an unregister request.
	 * We can now wake up the unregister ioctl
	 */
	dp->ioctl_intrpt = 0;
	e_wakeup(&dp->ioctl_event);
						   
    }
}

/*
 * NAME: sd_process_conc_cmd
 *
 * FUNCTION: deals with good completion or failure of
 *           concurrent mode commands
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt levels,
 *      and can not page fault.
 *
 * (RECOVERY OPERATION:) none
 *
 * (DATA STRUCTURES:)  sd_cmd          - Command structure
 *
 * RETURNS:  void
 */
void sd_process_conc_cmd(
struct sd_cmd *cmd)
{
    struct conc_cmd *ccptr; 
    struct sd_dasd_info *dp;

    ccptr = cmd->conc_cmd_ptr;
    dp = cmd->dp;
    ASSERT (ccptr);
    ccptr->error = (uchar)cmd->b_error;
    if ((struct sd_cmd *)dp->cmd_timer.pointer == cmd)
	w_stop(&dp->cmd_timer.watch); /* stop timer */
    sd_free_cmd(cmd);
    sd_return_conc_cmd(dp,ccptr);
}
/*
 * NAME: sd_send_msg
 *
 * FUNCTION: Builds a Serial Dasd Volume Status Change command for the 
 *           appropriate dasd.
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_dasd_info    - DASD info structure
 *                      sd_cmd          - Command structure
 *                      sd_mbox         - Mailbox structure
 *                      sc_cmd          - SCSI command structure
 *
 * EXTERNAL PROCEDURES CALLED:
 *   
 *     disable_lock			unlock_enable
 *
 * RETURNS:     void
 */

void sd_send_msg(struct sd_dasd_info *dp, uchar message_code)
{
        struct sd_cmd *cmd;
        struct sd_mbox *m;
        struct sc_cmd *scsi;
	int index;
	struct sd_diag_event *parameter_block;
	
	uint    old_pri;


	cmd = &(dp->concurrent);
	ASSERT(cmd->status == SD_FREE);

#ifdef DEBUG
#ifdef SD_ERROR_PATH
	sd_trc(dp->ap, message,trc,(char)0,(uint)dp, (uint)cmd , (uint)message_code,(uint) 0,(uint)0);
	sd_dptrc(dp,message, trc,(char)0,(uint)cmd);
#endif
#endif
	
	parameter_block = &(dp->diag_event);	
	parameter_block->byte[0] = SD_EVENT_NOTIFICATION;
	parameter_block->byte[1] = 0;
	parameter_block->byte[2] = 0;
	parameter_block->byte[3] = 2;
     	parameter_block->byte[4] = 0;
     	parameter_block->byte[5] = message_code;
	
	cmd->nextcmd = NULL;                   /* clear next pointer        */
	cmd->dp = dp;                          /* set dasd pointer          */
	cmd->ap = dp->ap;                      /* set dasd pointer          */
	cmd->cp = dp->cp;                      /* set controller pointer    */
	cmd->b_addr =(char *)parameter_block;  /* set up buffer for         */
	                                       /* parameter block           */
        cmd->xmem_buf = &(dp->xmem_buf);       /* clear cross mem descrip   */
        cmd->xmem_buf->aspace_id = XMEM_GLOBAL;
        cmd->b_length = sizeof(struct sd_diag_event); /* set transfer length */
        cmd->tcw_words = 0;                    /* clear number of tcw's alloc*/
        cmd->tcws_start = 0;                   /* clear starting tcw        */
        cmd->sta_index = (signed char)-1;      /* init small xfer area index*/
	cmd->status_validity = 0;              /* clear status validity     */
	cmd->erp = 0;                          /* clear error recov proc    */
	cmd->type = SD_DASD_CMD;               /* set command type          */
	cmd->cmd_info = SD_REFRESH;            /* set command info          */
	
	/*
	 * build copy of eventual mailbox
	 */
	m = &(cmd->mbox_copy);
	m->op_code = 0x80;              /* Send SCSI Command */
	m->mb6.qc_scsiext = SD_Q_UNORDERED; /* unordered q control */
	/*
	 * set device address generated by this dasd's lun and his
	 * controllers target address
	 */
	m->mb7.dev_address =
	    SD_LUNTAR(dp->cp->dds.target_id,dp->dds.lun_id,
		      SD_LUNDEV);
	/*
	 * Initialize SCSI cmd for operation
	 */
	scsi = &(m->mb8.fields.scsi_cmd);
	scsi->scsi_op_code = SCSI_SEND_DIAGNOSTIC;
	scsi->lun = 0x00;
	scsi->scsi_bytes[0] = 0x00;
	scsi->scsi_bytes[1] = 0x00;
	scsi->scsi_bytes[2] = 0x06;       /* parameter list length  */
       	scsi->scsi_bytes[3] = 0x00;
	
	cmd->timeout = 20;              /* set timeout to 20 secs */
	sd_q_cmd(cmd, (char)SD_QUEUE);   /* queue up this command */

}
