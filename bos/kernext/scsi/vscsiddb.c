static char sccsid[] = "@(#)44	1.16  src/bos/kernext/scsi/vscsiddb.c, sysxscsi, bos41J, 9511A_all 3/2/95 11:00:05";
/*
 *   COMPONENT_NAME: SYSXSCSI
 *
 *   FUNCTIONS: vsc_async_notify
 *		vsc_async_stat
 *		vsc_buf_free
 *		vsc_cdt_func
 *		vsc_clear_dev
 *		vsc_deq_active
 *		vsc_dump
 *		vsc_dump_start
 *		vsc_dump_write
 *		vsc_fail_cmd
 *		vsc_free_b_link
 *		vsc_free_rdbufs
 *		vsc_get_b_link
 *		vsc_get_cmd_elem
 *		vsc_halt_dev
 *		vsc_init_gw_buf
 *              vsc_internal_trace
 *		vsc_iodone
 *		vsc_issue_bdr
 *		vsc_log_err
 *		vsc_need_disid
 *		vsc_need_enaid
 *		vsc_process_scsi_reset
 *		vsc_recv
 *		vsc_scsi_reset
 *		vsc_start
 *		vsc_start_pending_cmds
 *		vsc_strategy
 *		vsc_target_receive
 *		vsc_watchdog
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*
 *
 * COMPONENT:   SYSXSCSI 
 *                      
 * NAME:	vscsidd.c
 *                      
 * FUNCTION:	IBM SCSI Protocol Driver Source File
 *                                                 
 *		This protocol driver is the interface between a 
 *		SCSI device driver and the adapter device driver.  
 *		It executes commands from multiple drivers which 
 *		contain generic SCSI device commands, and manages 
 *		the execution of those commands. Several ioctls 
 *		are supported to provide for system and device
 *		management.
 */


/*
 *  GENERAL NOTES
 *  -------------
 *
 *  The protocol driver needs several attributes of the parent adapter 
 *  in order to function properly. These are passed to the protocol 
 *  via the DDS structure at CFG_INIT time.
 *
 *  SCSI id 
 *  The SCSI id of the virtual device is an attribute of the
 *  parent adapter. At SCIOSTART, the protocol driver can ensure 
 *  that the started device's scsi id 
 *  does not conflict with the adapter's SCSI id.
 *
 *  WIDE ENABLE 
 *  If the adapter's wide enable attribute is disabled(8 bit SCSI bus),
 *  then at SCIOSTART, the SCSI id of the device is checked to ensure
 *  that it is not greater then 7.
 *
 *  INTERRUPT PRIORITY
 *  It is necessary for the protocol driver to know the adapter driver's
 *  interrupt priority so it can serialize critical sections of code
 *  without interruption from the adapter driver.
 *
 *  PARENT NAME
 *  It is necessary for the protocol driver to know the parent adapter's
 *  logical name. This is needed for the NS services to establish a channel
 *  between the protocol and adapter drivers.
 *
 *
 *
 *  SOME FLAGS AND COUNTERS 
 *  vsc_info.num_of_cfgs
 *	- used to determine the first CONFIG_INIT to the protocol 
 *	  driver and the last CONFIG_TERM to the protocol driver.
 *  	  operations such as for devswadd and devswdel occur at this time.
 *	  vsc_ctrl is a static global structure.
 *
 *
 *  shared->num_of_cfgs
 *	- shared is a structure associated with a group of virtual
 *	  devices that have a common parent adapter.
 *	  used to determine first config_init call and the last config_term
 *	  call for a virtual device pair.
 *	  used for xmfree of shared struct.
 *
 *
 *  DEVICE REGISTRATION
 *	adapters may choose to address destination devices
 *	in different ways. In order the to give the adapter driver
 *	the oppurtunity to register the device with the adapter, a NDD_ADD_DEV
 *	ioctl will be issued to the adapter driver at SCIOSTART and a 
 *      NDD_DEL_DEV ioctl at SCIOSTOP. the adapter may or may not transform 
 *      this combination before communicating with the device.
 */

/* INCLUDED SYSTEM FILES */
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/malloc.h>
#include <sys/buf.h>
#include <sys/pin.h>
#include <sys/sleep.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/uio.h>
#include <sys/lockl.h>
#include <sys/priv.h>
#include <sys/watchdog.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dump.h>
#include <sys/ddtrace.h>
#include <sys/trchkid.h>
#include <sys/trcmacros.h>
#include <sys/ndd.h>
#include <sys/cdli.h>
#include <sys/scb_user.h>
#include <sys/scsi.h>
#include <sys/scsi_scb.h> 
#include <sys/vscsidd.h>
#include <sys/errids.h>
/* END OF INCLUDED SYSTEM FILES  */



/*****************************************************************************/
/* Global device driver static data areas                                    */
/*****************************************************************************/

/* array containing pointer to scsi bus information (two per bus)            */
struct scsi_info *vsc_scsi_ptrs[MAX_ADAPTERS*2] = {NULL};

/* global device driver component dump table pointer                         */
struct vsc_cdt_tab *vsc_cdt = NULL;

#ifdef VSC_TRACE
#define STRT  0x53545254  /* ASCII representation of 'STRT' */
uint vsc_trace_tab[TRACE_TABLE_LENGTH] = {(uint) STRT};
uint *vsc_trace_ptr = NULL;
#endif /* VSC_TRACE */

static int (*dump_entry_ptr) () = NULL;

/*****************************************************************************/
/* End of global device driver static data areas                             */
/*****************************************************************************/


/*
 * NAME:	vsc_clear_dev 
 *
 * FUNCTION: 
 *              This routine performs the necessary operations to stop a device
 *              and deallocate its resource.
 *
 * EXECUTION ENVIRONMENT:
 *		process level only
 *
 * NOTES:       Outstanding commands to this device are allowed to finish, but
 *              further commands are rejected.
 *             
 * CALLED FROM:
 *		vsc_ioctl(SCIOSTOP)
 *
 * EXTERNAL CALLS:
 *              disable_lock              unlock_enable
 *              xmfree                    e_sleep_thread
 *              w_clear
 *		
 * INPUT:
 *		scsi	  - scsi_info structure ptr for this device
 *		dev_index - index to device info structure 
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *		EINVAL- device not started (opened)
 */
int
vsc_clear_dev(
	struct	scsi_info *scsi,
        int dev_index)
{
    int  ret_code,                     /* return code for this function     */
         arg,                          /* argument for adapter ndd ioctl    */
         old_pri;                      /* returned from disable_lock        */

    ret_code = 0;  /* set default return code */

    /* validate the device has been started (opened) */
    if (scsi->dev[dev_index] == NULL) {
        ret_code = EINVAL; /* not started */ 
        return(ret_code);
    }

    /* serialize with intrpts */
    old_pri = disable_lock(scsi->ddi.intr_priority,
                           &(scsi->shared->ndd->ndd_demux_lock));  
    /* set stop_pending to reflect an SCIOSTOP in progress */
    scsi->dev[dev_index]->stop_pending = TRUE;   
 
    /* if any commands active, wait for them to finish.     */
    while((scsi->dev[dev_index]->head_act != NULL) ||
          (scsi->dev[dev_index]->head_pend != NULL)||
          (scsi->dev[dev_index]->command_element.cmd_state != INACTIVE)) {
        e_sleep_thread(&scsi->dev[dev_index]->dev_event,
                       &scsi->shared->ndd->ndd_demux_lock, LOCK_HANDLER);
    } /* end while */

    /* allow recv handler in */
    unlock_enable(old_pri, &(scsi->shared->ndd->ndd_demux_lock));

   /*
    * build the argument to the adapter device registration ioctl
    * it consists of a SCSI id/lun combination as created by the INDEX
    * macro, and the location of the bus on the adapter is stored in the
    * two most signifigant bits of the word : bits 0 - 4 are lun
    * bits 5-8 are id and bits 14-15 are bus
    */
    arg = dev_index | (scsi->ddi.location  << 14);
    /* issue the adapter driver device deregistration ioctl */
    (void) scsi->shared->ndd->ndd_ctl(scsi->shared->ndd, NDD_DEL_DEV, arg);

    /* unregister the watchdog timer for this device */
#ifdef _POWER_MP
    while(w_clear(&scsi->dev[dev_index]->wdog.dog));
#else
    w_clear(&scsi->dev[dev_index]->wdog.dog);
#endif

    /* release the memory for this dev_info struct */
    (void) xmfree((void *) scsi->dev[dev_index], pinned_heap);

    /* clear the pointer to the dev_info struct */
    scsi->dev[dev_index] = NULL;

    /* free any gathered write buffers which may be left around */
    vsc_free_gwrite (scsi);

    return(ret_code);

}   /* end vsc_clear_dev */ 


/*
 * NAME:        vsc_free_gwrite           
 *                                       
 * FUNCTION:    Free adapter gwrite buffers
 *                                         
 *      This internal routine frees up the gwrite memory
 *      buffers that were used for gathered writes
 *
 * EXECUTION ENVIRONMENT
 *      This routine can be called only on the process level
 *
 * DATA STRUCTURES
 *      gwrite      - gathered write buffer management structure
 *
 * INPUTS
 *      scsi    - pointer to adapter structure
 *
 * RETURN VALUE DESCRIPTION:  The following are the return values
 *      none
 *
 * EXTERNAL PROCEDURES CALLED
 *      disable_lock       unlock_enable
 *      unpin           xmfree
 */
void
vsc_free_gwrite (struct scsi_info *scsi)
{
    int           i, old_pri;
    struct gwrite *current, *save_ptr;

    /* serialize with intrpts */
    old_pri = disable_lock(scsi->ddi.intr_priority, 
                           &scsi->shared->ndd->ndd_demux_lock);

    current = scsi->head_gw_free;
    scsi->head_gw_free = NULL;
    scsi->tail_gw_free = NULL;
    unlock_enable(old_pri, &scsi->shared->ndd->ndd_demux_lock); 

    /* loop while gwrite free list is non-empty */
    while (current != NULL)
    {
	save_ptr = current;
	current = current->next;
	/* unpin and free buffer first */
	(void) unpin (save_ptr->buf_addr, save_ptr->buf_size);
	(void) xmfree (save_ptr->buf_addr, kernel_heap);

	/* unpin and free gwrite struct */
	(void) unpin (save_ptr, sizeof (struct gwrite));
	(void) xmfree ((caddr_t) save_ptr, kernel_heap);
    }	/* end while */
}  /* end vsc_free_gwrite */


/*
 *
 * NAME:	vsc_halt_dev 
 *
 * FUNCTION: 
 *		This function issues a SCSI abort message to the specified 
 *              device.  This will cause all active commands to the device
 *              to be aborted.
 *
 * EXECUTION ENVIRONMENT:
 *		process level only
 *
 * NOTES:       This routine causes an abort command to be issued to a SCSI   
 *              device.  Note that this action will also halt the device queue.
 *              The calling process is responsible for NOT calling this rou- 
 *              tine if the SCIOSTART failed.  Such a failure would indicate
 *              that another process has this device open and interference
 *              could cause improper error reporting. 
 *
 *
 * CALLED FROM:
 *		vsc_ioctl(SCIOHALT)
 *
 * EXTERNAL CALLS:
 *              disable_lock              unlock_enable
 *              e_sleep_thread            w_start     
 *              w_stop
 *		
 *		
 * INPUT:
 *		scsi	  - scsi_info structure ptr for this device
 *		dev_index - index to device info structure 
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *              EINVAL    - Device not opened 
 *              ETIMEDOUT - The abort message timed out.
 *              EIO       - I/O error occurred
 *	
 */
int
vsc_halt_dev(
	struct	scsi_info *scsi,
        int dev_index)
{
    int  ret_code,                      /* return code for this function     */
         rc,                            /* locally used return code          */
         old_pri,                       /* returned from disable_lock        */
         save_time;                     /* used for converting device timer  */

    ret_code = 0;  /* set default return code */

    /* validate device is started (opened) */
    if (scsi->dev[dev_index] == NULL) {
        ret_code = EINVAL; /* device not started */
        return(ret_code);
    }

    /* serialize with intrpts */
    old_pri = disable_lock(scsi->ddi.intr_priority, 
                           &scsi->shared->ndd->ndd_demux_lock);
    /* if device queue is halted, then an abort, bdr, or SCSI bus reset is   */
    /* currently pending */
    if (scsi->dev[dev_index]->qstate & (HALTED | RESET_IN_PROG)) {
        ret_code = EIO;    /* return error indication */
        /* reenable interrupts     */
        unlock_enable(old_pri, &scsi->shared->ndd->ndd_demux_lock); 
        return(ret_code);
    }

    /* build the abort control element */
    scsi->dev[dev_index]->command_element.request.scsi_cdb.scsi_id = 
                       SID(dev_index);
    scsi->dev[dev_index]->command_element.request.scsi_cdb.scsi_lun = 
                       LUN(dev_index);
    scsi->dev[dev_index]->command_element.request.scsi_cdb.media_flags |= 
                       (VSC_ABORT | VSC_RESUME);
    scsi->dev[dev_index]->command_element.cmd_state = IS_ACTIVE;
    scsi->dev[dev_index]->command_element.cmd_type  = PROC_LVL_CANCEL;

    /* clear the proc results flag */
    scsi->proc_results = 0;
    /* mark queue for this device as hatled to prevent commands from being */
    /* issued while the abort is in progress */
    scsi->dev[dev_index]->qstate |= HALTED;

    /* start timer for this command */
    /* use the watchdog timer for this device to time the abort  */
    /* stop the timer and restart it with the timeout value for  */
    /* the abort.  It is ok that active commands are no longer   */
    /* being timed because completion or a time out of the abort */
    /* will cause the device's active queue to be cleared.       */
    w_stop(&scsi->dev[dev_index]->wdog.dog);
    /* save off the current restart value so that device timer can be   */
    /* restarted if call to output fails */
    save_time = scsi->dev[dev_index]->wdog.dog.restart;
    scsi->dev[dev_index]->wdog.timer_id = SCSI_CANCEL_TMR;
    scsi->dev[dev_index]->wdog.dog.restart = RESET_CMD_T_O;
    w_start(&scsi->dev[dev_index]->wdog.dog);

#ifdef VSC_TRACE
    vsc_internal_trace(scsi, dev_index, 
                       (uint *) &scsi->dev[dev_index]->command_element, 2, 0); 
#endif /* VSC_TRACE */

    /* issue the command to the adapter drivers output function */
    rc = (scsi->shared->ndd->ndd_output) (scsi->shared->ndd, 
                          &scsi->dev[dev_index]->command_element.ctl_elem);

    if (rc) {  /* call to output returned error; clean up */
        /* convert device timer back to timing active cmd_elems */
        w_stop(&scsi->dev[dev_index]->wdog.dog);
        scsi->dev[dev_index]->wdog.timer_id = SCSI_DEV_TMR;
        scsi->dev[dev_index]->wdog.dog.restart = save_time;
        w_start(&scsi->dev[dev_index]->wdog.dog);
        scsi->dev[dev_index]->command_element.cmd_state = INACTIVE;
        scsi->dev[dev_index]->qstate &= ~(HALTED);
        /* reenable interrupts     */
        unlock_enable(old_pri, &scsi->shared->ndd->ndd_demux_lock); 
        ret_code = EIO;    /* return error indication */
        return(ret_code);
    }

    e_sleep_thread(&scsi->ioctl_event, &scsi->shared->ndd->ndd_demux_lock,  
                   LOCK_HANDLER);
    
    ret_code = scsi->proc_results;
    /* if the ABORT did not timeout call vsc_fail_cmd to ensure the queue */
    /* (active and pending) for this device has been cleared */
    if (ret_code != ETIMEDOUT) {
        /* clear the halted indication for this device queue */
        scsi->dev[dev_index]->qstate &= ~(HALTED);
        vsc_fail_cmd(scsi, dev_index);
    }
    /* reenable interrupts     */
    unlock_enable(old_pri, &scsi->shared->ndd->ndd_demux_lock); 

    return(ret_code);

}  /* end vsc_halt_dev */


/*
 *
 * NAME:	vsc_issue_bdr 
 *
 * FUNCTION: 
 *		issues a SCSI bus device reset message to a device (SCSI id)
 *
 * EXECUTION ENVIRONMENT:
 *		process level only
 *
 * NOTES:
 *		the BDR message affects all LUNs attached to the selected SCSI
 *		device.  commands in progress are aborted, and each LUN is   
 *		sent back to its initial, power-on state. 
 *
 *              This routine causes an reset command to be issued to a SCSI
 *              device.  Note that this action will also halt the device queue.
 *              The calling process is responsible for NOT calling this rou- 
 *              tine if the SCIOSTART failed.  Such a failure would indicate
 *              that another process has this device open and interference
 *              could cause improper error reporting.
 *
 * CALLED FROM:
 *		vsc_ioctl(SCIORESET)
 *		
 *
 * EXTERNAL CALLS:
 *              disable_lock              unlock_enable
 *              w_start                   e_sleep_thread
 *              w_stop
 *		
 * INPUT:
 *		devno	- major/minor number
 *		arg 	- SCSI ID / LUN
 *		scsi	- scsi_info structure ptr for this device
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *              EINVAL    - Device not opened 
 *              ETIMEDOUT - The BDR message timed out.
 *              EIO       - I/O error occurred
 */
int
vsc_issue_bdr(
	struct	scsi_info *scsi,
        int dev_index)
{
    int  ret_code,                      /* return code for this function     */
         rc,                            /* locally used return code          */
         temp_lun,                      /* for loop variable                 */
         t_index,                       /* index into dev_info struct        */
         save_time,                     /* used for converting device timer  */
         old_pri;                       /* returned from disable_lock        */


    ret_code = 0;  /* set default return code */
    /* validate device is started (opened) */
    if (scsi->dev[dev_index] == NULL) {
        ret_code = EINVAL; /* device not started */
        return(ret_code);
    }

    /* serialize with intrpts */
    old_pri = disable_lock(scsi->ddi.intr_priority,
                           &scsi->shared->ndd->ndd_demux_lock);
    /* if device queue is halted, then an abort, bdr, or SCSI bus reset is   */
    /* currently pending  */
    if (scsi->dev[dev_index]->qstate & (HALTED | RESET_IN_PROG)) {
        ret_code = EIO;    /* return error indication */
        /* reenable interrupts     */
        unlock_enable(old_pri, &scsi->shared->ndd->ndd_demux_lock);
        return(ret_code);
    }

    /* a BDR will affect all luns at the specified SCSI id.    Mark all such */
    /* device queues as halted to prevent commands from being sent during    */
    /* BDR.  Also set CDAR_ACTIVE flag for these devices.                    */
    for (temp_lun = 0; temp_lun < MAX_LUNS; temp_lun++) {
        t_index = INDEX(SID(dev_index), temp_lun);
        if (scsi->dev[t_index]!= NULL)  /* if device is open */
            scsi->dev[t_index]->qstate |= (HALTED | CDAR_ACTIVE);
    } /* end for */

    /* build the abort control element */
    scsi->dev[dev_index]->command_element.request.scsi_cdb.scsi_id = 
                  SID(dev_index);
    scsi->dev[dev_index]->command_element.request.scsi_cdb.scsi_lun = 
                  LUN(dev_index);
    scsi->dev[dev_index]->command_element.request.scsi_cdb.media_flags |= 
                  (VSC_BDR | VSC_RESUME);
    scsi->dev[dev_index]->command_element.cmd_state = IS_ACTIVE;
    scsi->dev[dev_index]->command_element.cmd_type  = PROC_LVL_CANCEL;

    /* clear the proc results flag */
    scsi->proc_results = 0;

    /* start timer for this command */
    /* use the watchdog timer for this device to time the abort  */
    /* stop the timer and restart it with the timeout value for  */
    /* the abort.  It is ok that active commands are no longer   */
    /* being timed because completion or a time out of the abort */
    /* will cause the device's active queue to be cleared.       */
    w_stop(&scsi->dev[dev_index]->wdog.dog);
    /* save off the current restart value so that device timer can be   */
    /* restarted if call to output fails */
    save_time = scsi->dev[dev_index]->wdog.dog.restart;
    scsi->dev[dev_index]->wdog.timer_id = SCSI_CANCEL_TMR;
    scsi->dev[dev_index]->wdog.dog.restart = RESET_CMD_T_O;
    w_start(&scsi->dev[dev_index]->wdog.dog);

#ifdef VSC_TRACE
    vsc_internal_trace(scsi, dev_index, 
                       (uint *) &scsi->dev[dev_index]->command_element, 2, 0);
#endif /* VSC_TRACE */

    /* issue the command to the adapter drivers output function */
    rc = (scsi->shared->ndd->ndd_output) (scsi->shared->ndd, 
                          &scsi->dev[dev_index]->command_element.ctl_elem);

    if (rc) {  /* call to output returned error; clean up */
        /* convert device timer back to timing active cmd_elems */
        w_stop(&scsi->dev[dev_index]->wdog.dog);
        scsi->dev[dev_index]->wdog.dog.restart = save_time;
        scsi->dev[dev_index]->wdog.timer_id = SCSI_DEV_TMR;
        w_start(&scsi->dev[dev_index]->wdog.dog);
        for (temp_lun = 0; temp_lun < MAX_LUNS; temp_lun++) {
            t_index = INDEX(SID(dev_index), temp_lun);
            if (scsi->dev[t_index]!= NULL) {  /* if device is open */
                scsi->dev[t_index]->qstate &= ~(HALTED | CDAR_ACTIVE);
            }
        } /* end for */
        scsi->dev[dev_index]->command_element.cmd_state = INACTIVE;
        /* reenable interrupts     */
        unlock_enable(old_pri, &scsi->shared->ndd->ndd_demux_lock);
        ret_code = EIO;    /* return error indication */
        return(ret_code);
    }

    e_sleep_thread(&scsi->ioctl_event, &scsi->shared->ndd->ndd_demux_lock,
                   LOCK_HANDLER);

    /* initiate command delay after reset timer for this SCSI id */
    w_start(&scsi->cdar_wdog.dog);

    /* clear the halted indication for this device queue and fail the  */
    /* command queue for any device which has active commands          */
    for (temp_lun = 0; temp_lun < MAX_LUNS; temp_lun++) {
        t_index = INDEX(SID(dev_index), temp_lun);
        if (scsi->dev[t_index]!= NULL) {  /* if device is open */
            /* if the BDR did not timeout and active list is not empty */
            /* fail the queue for this device */ 
            if (scsi->proc_results != ETIMEDOUT) {
                scsi->dev[t_index]->qstate &= ~(HALTED);
                if (scsi->dev[t_index]->head_act != NULL) {
                    vsc_fail_cmd(scsi, t_index);
                }
                else { /* device open but no cmds affected by BDR so try */
                       /* to initiate any i/o which may be pending       */
                    vsc_start(scsi, t_index);
                }
            } /* end if proc_results != ETIMEDOUT */
        } /* end if device is open */
    } /* end for */
 
    ret_code = scsi->proc_results;
    /* reenable interrupts     */
    unlock_enable(old_pri, &scsi->shared->ndd->ndd_demux_lock);

    return(ret_code);
	
}  /* end vsc_issue_bdr */

/*
 * NAME:	vsc_free_rdbufs
 *
 * FUNCTION: 
 *              Function to free all the buffers for a particular initiator
 *              that are on the read_buf list (queued to the head). This is
 *              done when an SCIOSTOPTGT ioctl is called to disable a path 
 *              from an inititator to this target.  Any buffers with data
 *              from the disabled initiator will be taken back from the head
 *              and returned to the buffer pool.
 *	
 *
 * EXECUTION ENVIRONMENT:
 *
 *		This function can only be called from the process level. 
 *
 * NOTES:
 *
 *
 * EXTERNAL CALLS:
 *              disable_lock                   unlock_enable
 *
 * RETURNS:  
 *
 */
void
vsc_free_rdbufs (struct scsi_info *scsi, /* virtual scsi structure */
		 int dev_index)          /* index into device list */
{
    struct buf_pool_elem *pool_elem_list; /* list of buffers to free    */
    struct b_link        *b_link;         /* current b_link             */
    struct b_link        *next_b_link;    /* next b_link                */
    int                   old_pri;        /* saved interrupt priority   */
    struct buf_pool_elem *pool_elem;      /* data buffer for the b_link */

    pool_elem_list = NULL;
    /* mask out intrpts */
    old_pri = disable_lock(scsi->ddi.intr_priority, 
                           &scsi->shared->ndd->ndd_demux_lock);
    b_link = scsi->read_bufs;

    /*
     * Walk the entire read buf list, removing buffers that are being used by
     * the device being closed. 
     */
    while (b_link != NULL)
    {
	/*
	 * Skip buffers that are marked for other users of this device. 
	 */
	if (b_link->user_id != dev_index)
	{
	    b_link = b_link->forw;
	    continue;
	}

	next_b_link = b_link->forw;	/* save next buffer address */

	/*
	 * Place this buf_pool_elem at head of list of buffers to free. 
	 */
	pool_elem = (struct buf_pool_elem *) b_link->buf_addr;
	if (pool_elem != NULL)
	{
	    pool_elem->next = pool_elem_list;
	    pool_elem_list = pool_elem;
	}

	/*
	 * Remove from read_bufs list. 
	 */
	vsc_free_b_link (b_link);

	b_link = next_b_link;
    }

    /*
     * Return all free buf_pool_elem buffers to the adapter driver. 
     */
    if (pool_elem_list != NULL)
    {
#ifdef VSC_TRACE
	/* Trace all the buf_pool_elems being freed */
	pool_elem = pool_elem_list;
	while (pool_elem != NULL) 
	{
	    vsc_internal_trace(scsi, dev_index, (uint *) pool_elem, 4, 1);
	    pool_elem = pool_elem->next;
	}
#endif /* VSC_TRACE */

	/*
	 * Return the buffer to the adapter driver. The buf_pool_elem buffer
	 * stores its own free function. 
	 */
	pool_elem_list->buf_pool_free (pool_elem_list);
    }

    unlock_enable (old_pri, &scsi->shared->ndd->ndd_demux_lock);

}  /* end vsc_free_rdbufs */


/*
 * NAME:     vsc_get_cmd_elem
 *
 * FUNCTION:
 *           This routine, given a scsi_info structure will return pointer 
 *           to the first availible cmd_element in the cmd_pool of the 
 *           scsi_info struct.  It will also mark that cmd_elem in use.
 *
 * EXECUTION ENVIRONMENT:
 *           This routine can only be called on priority levels greater
 *           than, or equal to that of the interrupt handler.
 *
 * NOTES:
 *
 * EXTERNAL CALLS:
 *
 * INPUT:
 *		scsi	  - scsi_info structure ptr for the device from which
 *                          the command element is to be gotten.
 *
 * RETURNS:
 *
 *              pointer to a free cmd_elem if one is availible 
 *              NULL - if no cmd_elem is availible
 */

struct cmd_elem *
vsc_get_cmd_elem(
        struct scsi_info *scsi)
{
    struct  cmd_elem *cmd = NULL;
    uchar   tag;
    int     word;

    /* interrupts are assuemd disabled here */
    word = 0;
    do  {
        /* look for a free tag in this word */
        tag = vsc_getfree(scsi->free_cmd_list[word]);
                if (tag < VSC_BITS_PER_WORD) {
                    /*
                     * found free tag within this word
                     * set as in use, reset tag to real tag value
                     * depending on which word it was in, and set
                     * flag that we found one.
                     */
                    VSC_GETTAG(scsi->free_cmd_list[word],tag);
                    tag += (VSC_BITS_PER_WORD * word);
                    /* set cmd pointer to the one just allocated */
                    cmd = (struct cmd_elem *)&(scsi->cmd_pool[tag]);
                    /* increment the count of cmd_elems in use */
                    break;
                } else  {
                    /* try next word */
                    word++;
                }
        }  while (word < SC_NUM_CMD_WORDS);

        return(cmd);

}  /* end vsc_get_cmd_elem */


/*
 *
 * NAME:        vsc_iodone
 *
 * FUNCTION:
 *              This function handles completion of SCSI commands initiated 
 *              through IOCTL functions
 *
 * EXECUTION ENVIRONMENT:
 *              This routine runs on the IODONE interrupt level, so it can
 *              be interrupted by the interrupt handler.
 *
 * NOTES :
 *
 * EXTERNAL CALLS:
 *              e_wakeup             disable_lock
 *              unlock_enable
 *
 * INPUT:
 *              bp      - pointer to the passed sc_buf
 *
 *
 * RETURNS:
 */
void
vsc_iodone(
           struct sc_buf * bp)
{
#ifdef _POWER_MP
    struct vsc_buf *vbp;
    int old_pri;
#endif

#ifdef _POWER_MP
    vbp = (struct vsc_buf *) bp;
    old_pri = disable_lock(INTIODONE,&(vbp->scsi->ioctl_lock));

    bp->bufstruct.b_flags |= B_DONE;

    e_wakeup(&bp->bufstruct.b_event);
    unlock_enable(old_pri,&(vbp->scsi->ioctl_lock));
#else
    e_wakeup(&bp->bufstruct.b_event);
#endif

} /* end vsc_iodone */

/************************************************************************/
/*                                                                      */
/* NAME:        vsc_ioctl_sleep                                         */
/*                                                                      */
/* FUNCTION:    Waits for ioctl command to complete                     */
/*                                                                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called only on the process level.           */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*                                                                      */
/* INPUTS:                                                              */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      disable_lock     unlock_enable                                  */
/*      iowait                                                          */
/*                                                                      */
/************************************************************************/
void vsc_ioctl_sleep(struct buf *bp, struct scsi_info *scsi)
{
    int    old_pri;                           /* old interrupt level */


#ifdef _POWER_MP

    old_pri = disable_lock(INTIODONE,&(scsi->ioctl_lock));

    if (!(bp->b_flags & B_DONE)) {
        /* If buf has not completed then go to sleep  */
        e_sleep_thread((int *)&(bp->b_event),&(scsi->ioctl_lock),
                       LOCK_HANDLER);
    }

    /* Re-enable interrupts and unlock.  */
    unlock_enable(old_pri,&(scsi->ioctl_lock));

#else
    iowait(bp);
#endif
    return;

}

/*
 *
 * NAME:        vsc_fail_cmd
 *
 * FUNCTION:
 *              This function is called to clear out pending and active 
 *              commands for a specified device (flush the queue) upon 
 *              detection of a error or completion of an error recovery 
 *              command (Abort, BDR, SCSI bus reset). 
 *
 *
 * EXECUTION ENVIRONMENT:
 *              This routine can only be called on priority levels greater
 *              than, or equal to that of the interrupt handler.
 *
 * NOTES:       This function should only be called if an interrupt has been
 *              received for all active cmd_elems.  Otherwise it will iodone
 *              sc_bufs before the d_complete has been done by adapter 
 *              device driver.
 *        
 *
 * EXTERNAL CALLS:
 *              w_stop                   iodone
 *              e_wakeup
 *
 * INPUT:
 *              scsi     - scsi_info structure ptr for this device
 *              dev_index- index into array of devinfo structs for device to
 *                         be failed
 *
 *
 * RETURNS:
 *              NONE
 */
void
vsc_fail_cmd(
        struct  scsi_info       *scsi,
        int dev_index)
{
    struct  cmd_elem *cmd;              /* pointer to a cmd_elem on act list*/
    struct  sc_buf   *tptr;             /* temp pointer to walk pend and    */
                                        /* act lists of sc_bufs             */
    uchar  tag;                         /* used to free cmd_elem            */
    struct gwrite *gw_ptr;

    /* interrupts are assumed to be disabled */

    /* this assumes that for the failed command(s), the  */
    /* following are set previously:  sc_buf status,     */
    /* sc_buf resid value, and sc_buf b_error field.     */

    /* clear active queue for this device */
    while (scsi->dev[dev_index]->head_act != NULL) {
        tptr = scsi->dev[dev_index]->head_act;
        cmd = (struct cmd_elem *) tptr->bufstruct.b_work;
        /* see if an error indication is already set for this sc_buf */
        if (tptr->bufstruct.b_error == 0) {
            /* no error indication; this cmd needs to be set up for a retry */
            tptr->status_validity = 0; /* setup otherwise good status */
            /* indicate that no data was transferred */
            tptr->bufstruct.b_resid = tptr->bufstruct.b_bcount;
            /* mark as needing restart by the caller */
            tptr->bufstruct.b_error = ENXIO;
        } 

        /* set b_flags B_ERROR flag */
        tptr->bufstruct.b_flags |= B_ERROR;

	/* free the gwrite struct if it was a gathered write */
	if (cmd->bp->resvd1)
	{
	    gw_ptr = (struct gwrite *) cmd->bp->resvd1;
	    /* put the gwrite struct on tail of gwrite free list */
	    if (scsi->head_gw_free == NULL)
		scsi->head_gw_free = gw_ptr;
	    else
		scsi->tail_gw_free->next = gw_ptr;
	    scsi->tail_gw_free = gw_ptr;
	    gw_ptr->next = NULL;	/* mark new end of chain */
	    cmd->bp->resvd1 = NULL;
	}

        /* see if cmd_element is valid (non-NULL). if so free up the        */
        /* the cmd_elem.  If NULL, cmd_elem already freed.                  */
        if(cmd != NULL) {
            ASSERT(cmd->cmd_state == INTERRUPT_RECVD);
            if (cmd->cmd_state != INTERRUPT_RECVD) {
                vsc_log_err(scsi, ERRID_SCSI_ERR6, 34, NULL, 0);
                vsc_scsi_reset(scsi);
                return;
            }
            tag = cmd->tag;
            cmd->cmd_state = INACTIVE;
            VSC_FREETAG(scsi->free_cmd_list[(int)((tag)/VSC_BITS_PER_WORD)],
                        tag);
        }
        /* decrement device num_act_cmds counter */
        scsi->dev[dev_index]->num_act_cmds--;
        /* point to next sc_buf in active list, if any */
        scsi->dev[dev_index]->head_act =
                       (struct sc_buf *) tptr->bufstruct.av_forw;
        iodone((struct buf *) tptr);
    } /* end while */

    scsi->dev[dev_index]->tail_act = NULL; /* reset tail pointer */

    /* active queue now empty so stop watchdog timer for this device */
    w_stop(&scsi->dev[dev_index]->wdog.dog);

    /* clean up global waiting states */
    if (scsi->dev[dev_index]->waiting) {
        scsi->any_waiting--;      /* dec global counter */
    }

    scsi->dev[dev_index]->waiting = FALSE; /* reset device waiting flag */

    /* clear pending queue for this device */
    while (scsi->dev[dev_index]->head_pend != NULL) {
        tptr = scsi->dev[dev_index]->head_pend;
        /* set b_flags B_ERROR flag */
        tptr->bufstruct.b_flags |= B_ERROR;
        /* set up otherwise good status */
        tptr->status_validity = 0;
        /* indicate no data was transferred */
        tptr->bufstruct.b_resid = tptr->bufstruct.b_bcount;
        /* mark as needing restart by the caller */
        tptr->bufstruct.b_error = ENXIO;
        /* point to next sc_buf in pending chain, if any */
        scsi->dev[dev_index]->head_pend = 
                       (struct sc_buf *) tptr->bufstruct.av_forw;
        iodone((struct buf *) tptr);
    } /* end while */

    /* see if a SCSIOSTOP is sleeping on command completion */
    if (scsi->dev[dev_index]->stop_pending) {
        e_wakeup(&scsi->dev[dev_index]->dev_event);
    }

    scsi->dev[dev_index]->tail_pend = NULL; /* reset tail pointer */
    /* clear flag that may be halting queue for a CAC_ERROR */
    scsi->dev[dev_index]->qstate &= ~(CAC_ERROR);
    /* clear the cc_error_state for this device */
    scsi->dev[dev_index]->cc_error_state = 0;
    /* clear flag that may be halting queue for a PENDING_ERROR */
    scsi->dev[dev_index]->qstate &= ~(PENDING_ERROR);
    /* clear flag that may be halting queue for command timeout */
    scsi->dev[dev_index]->qstate &= ~(Q_STARVE);
    /* indicate this device is waiting for an SC_RESUME indication */
    scsi->dev[dev_index]->need_resume_set = TRUE;
  
    vsc_start(scsi, dev_index); /* see if another request can be started */

    return;
} /* end vsc_fail_cmd */

/*
 *
 * NAME:        vsc_deq_active
 *
 * FUNCTION:
 *              This function removes an sc_buf from the active queue for a 
 *              given scsi/device combination.  This sc_buf can be in any
 *              position within the doubly linked active list.
 *
 * EXECUTION ENVIRONMENT:
 *              This routine can only be called on priority levels greater
 *              than, or equal to that of the interrupt handler.
 *
 * NOTES :
 *
 * EXTERNAL CALLS:
 *
 * INPUT:
 *              scsi     - scsi_info structure ptr for this device
 *              dev_index- index into array of devinfo structs for device for
 *                         which the sc_buf is to be removed.
 *              scp      - pointer to sc_buf to be removed from devices active
 *                         list
 *
 *
 * RETURNS:
 *              NONE
 */
void
vsc_deq_active(
        struct  scsi_info *scsi,
        struct sc_buf *scp,
        int dev_index)
{

    if (scsi->dev[dev_index]->head_act == scsi->dev[dev_index]->tail_act) {
        scsi->dev[dev_index]->head_act = NULL;
        scsi->dev[dev_index]->tail_act = NULL;
    }
    else {
        if (scsi->dev[dev_index]->head_act == scp)  { /* first one */
            scsi->dev[dev_index]->head_act =
                              (struct sc_buf *)scp->bufstruct.av_forw;
            scsi->dev[dev_index]->head_act->bufstruct.av_back = NULL;
        }
        else {
            if (scsi->dev[dev_index]->tail_act == scp)  { /* last one */
                scsi->dev[dev_index]->tail_act =
                              (struct sc_buf *)scp->bufstruct.av_back;
            scsi->dev[dev_index]->tail_act->bufstruct.av_forw = NULL;
            }
            else {  /* in the middle */
                scp->bufstruct.av_back->av_forw =
                        (struct buf *)scp->bufstruct.av_forw;
                scp->bufstruct.av_forw->av_back =
                        (struct buf *)scp->bufstruct.av_back;
           }
       }
    }
    scp->bufstruct.av_forw = NULL;
    scp->bufstruct.av_back = NULL;


} /* end vsc_deq_active */


/*
 * NAME:     vsc_strategy
 *
 * FUNCTION: virtual SCSI device strategy routine
 *           This routine will accept commands from the device heads
 *           and queue them up to the appropriate device structure for
 *           execution.  The command, in the form of sc_bufs, contains
 *           a bufstruct at the beginning of the structure and pertinent
 *           data appended after cannot exceed the maximum transfer size
 *           allowed.  Note that the av_forw field of the bufstruct MUST
 *           be NULL as chained commands are not supported.
 *                                                        
 *
 * EXECUTION ENVIRONMENT:
 *           This routine can only be called by a process or interrupt
 *           handler.  It can page fault only if called under a process
 *           and the stack is not pinned.
 *
 * NOTES:
 *               
 *		this routine validates the passed in command and 
 *		passes it down to the adapter driver's output routine.
 *
 *		the structure pointed to by the input parameter is not
 *		simply a buffer structure.  the beginning of the structure
 *		is exactly the same as a buffer structure, but there are 
 *		fields appended to the end of the buffer structure      
 *		which are required to initiate the SCSI command.       
 *
 *		note:  the av_forw field must be NULL here, chained sc_bufs
 *		are not supported.                                     
 *		the fwd/back pointers may not be modified while the
 *		command is queued on the send/wait list.
 *
 * EXTERNAL CALLS:
 *              disable_lock          unlock_enable
 *              iodone                w_stop
 *              w_start
 *		
 * INPUT:
 *		sc_buf	- sc_buf structure passed by caller
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *              EIO    - Invalid i/o request 
 *              ENXIO  - The request cannot be accepted because the queue is
 *                       halted.
 *
 */
int
vsc_strategy(struct sc_buf *bp )
{
    int  ret_code,                /* return code for this function          */
         old_pri,                 /* returned from disable_lock             */
         dev_index,               /* index to dev_info struct for this cmd  */
         i_hash;                  /* index to hash table of scsi_info ptrs  */
    uchar lun;                    /* temp var for SCSI lun                  */
    struct   scsi_info  *scsi;    /* pointer to virtual scsi structure      */

    ret_code = 0;  /* set default return code */

    DDHKWD5(HKWD_DD_SCSIDD, DD_ENTRY_STRATEGY, ret_code, bp->bufstruct.b_dev,
            bp, bp->bufstruct.b_flags, bp->bufstruct.b_blkno,
            bp->bufstruct.b_bcount);

    /* search for scsi_info structure in configured list */
    /* build the hash index */
    i_hash = minor(bp->bufstruct.b_dev) & SCSI_HASH;
    scsi = vsc_scsi_ptrs[i_hash];
    while (scsi != NULL) {
        if (scsi->devno == bp->bufstruct.b_dev) 
            break;
        scsi = scsi->next;
    }   /* endwhile */

    if ((scsi == NULL) || (!scsi->opened)) {
        ret_code = EIO; /* error--scsi struct not inited/opened */
        bp->bufstruct.b_error = EIO;
        bp->bufstruct.b_flags |= B_ERROR;
        bp->bufstruct.b_resid = bp->bufstruct.b_bcount; /* none sent */
        bp->status_validity = SC_ADAPTER_ERROR; /* flag problem */
        bp->general_card_status = SC_ADAPTER_SFW_FAILURE;
        iodone((struct buf *) bp);
        DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_STRATEGY, ret_code, 
                bp->bufstruct.b_dev);
        return(ret_code);
    }
    /* serialize with intrpts */
    old_pri = disable_lock(scsi->ddi.intr_priority, 
                           &scsi->shared->ndd->ndd_demux_lock);

    if (bp->lun != 0) 
        lun = bp->lun;
    else
        lun = (bp->scsi_command.scsi_cmd.lun >> 5);

    /* build the dev_index for this device */
    dev_index = INDEX(bp->scsi_command.scsi_id, lun);


    /* miscellaneous validation of request */
    if ((scsi->dev[dev_index] == NULL) ||
        (bp->bufstruct.b_bcount > scsi->shared->ndd->ndd_mtu) ||
        (scsi->dev[dev_index]->stop_pending)) {
        ret_code = EIO; /* error--bad command */
        bp->bufstruct.b_error = EIO;
        bp->bufstruct.b_flags |= B_ERROR;
        bp->bufstruct.b_resid = bp->bufstruct.b_bcount; /* none sent */
        bp->status_validity = SC_ADAPTER_ERROR; /* flag problem  */
        bp->general_card_status = SC_ADAPTER_SFW_FAILURE;
        iodone((struct buf *) bp);
        /* restore intrpts */
        unlock_enable(old_pri, &scsi->shared->ndd->ndd_demux_lock);
        DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_STRATEGY, ret_code, 
                bp->bufstruct.b_dev);
        return(ret_code);
    }

    /* if device queue is halted, and sc_buf resume flag not set */
    /* then fail the command                                     */
    if((scsi->dev[dev_index]->need_resume_set) &&
       !(bp->flags & SC_RESUME)) {
        ret_code = ENXIO;       /* error--device halted */
        bp->bufstruct.b_error = ENXIO;
        bp->bufstruct.b_flags |= B_ERROR;
        bp->bufstruct.b_resid = bp->bufstruct.b_bcount; /* none sent */
        bp->status_validity = 0;
        iodone((struct buf *) bp);
        /* restore intrpts */
        unlock_enable(old_pri, &scsi->shared->ndd->ndd_demux_lock);
        DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_STRATEGY, ret_code, 
                bp->bufstruct.b_dev);
        return(ret_code);
    }

    scsi->dev[dev_index]->need_resume_set = FALSE;
    /* set flag to indicate if head is queueing to this device */
    if (bp->q_tag_msg) 
        scsi->dev[dev_index]->dev_queuing = TRUE;

    if (bp->resvd1)
    {
	ret_code = vsc_init_gw_buf (scsi, bp, &old_pri);
	if (ret_code)
	{
	    iodone ((struct buf *) bp);
	    if (ret_code == EINVAL)
		/*
		 * interrupts haven't been restored when this error occurs.
		 */
                unlock_enable(old_pri, &scsi->shared->ndd->ndd_demux_lock);
	    
	    DDHKWD1 (HKWD_DD_SCSIDD, DD_EXIT_STRATEGY, ret_code, 
		     bp->bufstruct.b_dev);
	    return (ret_code);
	}
    }

    bp->resvd7 = 0;                     /* ensure a zero value   */
    bp->bufstruct.av_forw = NULL;       /* only accept one cmd   */
    bp->bufstruct.av_back = NULL;       /* only accept one cmd   */
    bp->bufstruct.b_work = (int)NULL;   /* init to "no cmd_elem" */
    bp->bufstruct.b_error = 0;          /* init to "no error"    */
    bp->bufstruct.b_flags &= ~B_ERROR;  /* init to "no error"    */
    bp->bufstruct.b_resid = 0;          /* init to "no error"    */
    bp->status_validity = 0;            /* init to "no error"    */
    bp->general_card_status = 0;        /* init to "no error"    */
    bp->scsi_status = 0;                /* init to "no error"    */

    /* see if this sc_buf is the error recovery command for a check condition*/
    /* while command tag queuing.  If so, place it at the HEAD of the        */
    /* pending queue and set expedite flag so that it can be made active     */
    /* while the queue is halted (qstate & CAC_ERROR)                        */
    if ((scsi->dev[dev_index]->cc_error_state) &&
        (bp->flags & SC_RESUME) && (!(bp->flags & (SC_Q_CLR | SC_Q_RESUME))) &&
        (!(bp->q_tag_msg))) {
        /* set resvd7 to TRUE so that this command can be issued to the    */
        /* adapter while the device queue is halted */
        bp->resvd7 = TRUE;
        /* enqueue the command to the head of device pending queue */
        if (scsi->dev[dev_index]->head_pend == NULL) {    /* if queue empty */
            scsi->dev[dev_index]->head_pend = bp;  /* point head at new one */
            scsi->dev[dev_index]->tail_pend = bp;  /* point tail at new one */
        }
        else {
           bp->bufstruct.av_forw = 
                         (struct buf *)scsi->dev[dev_index]->head_pend;
           scsi->dev[dev_index]->head_pend = (struct sc_buf *) bp;
        }
    } /* end if */
    /* enqueue the command to the tail of device pending queue */
    else if (bp->scsi_command.scsi_length) {   /* this sc_buf has a scsi cmd */
            if (scsi->dev[dev_index]->head_pend == NULL) { /* if queue empty */
            scsi->dev[dev_index]->head_pend = bp;   /* point head at new one */
            scsi->dev[dev_index]->tail_pend = bp;   /* point tail at new one */
            }
            else {      /* pending queue not empty */
                 /* point last cmd's av_forw at the new request */
                 scsi->dev[dev_index]->tail_pend->bufstruct.av_forw =
                                       (struct buf *) bp;
                 /* point tail at new one */
                 scsi->dev[dev_index]->tail_pend = bp;
             }
         } /* end if bp->scsi_command_scsi_length */

    /* if awaiting contingent alleigance error recovery and this sc_buf has */
    /* SC_Q_CLR set, then issue abort to adapter to clear the queue         */
    /* for this device.  When the abort completes, vsc_fail_cmd is called   */
    /* to clear the pending and active queues for this device.              */
    if (bp->flags & SC_Q_CLR)  {
        if (!(scsi->dev[dev_index]->cc_error_state)) {
        /* this  is an unexpected SC_Q_CLR request, ignore it and iodone */
        /* the sc_buf if it has no scsi cmd. If it has a scsi command    */
        /* then this sc_buf was placed on the pending queue and will be  */
        /* executed but the SC_Q_CLR will be ignored                     */
            /*this sc_buf has no scsi cmd*/
            if (!(bp->scsi_command.scsi_length)) {
                vsc_log_err(scsi, ERRID_SCSI_ERR6, 5, NULL, 0);
                iodone((struct buf *) bp);
                /* restore intrpts */
                unlock_enable(old_pri, &scsi->shared->ndd->ndd_demux_lock);
                DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_STRATEGY, ret_code, 
                        bp->bufstruct.b_dev);
                return(ret_code);
            }
        }
        else {  /* queue clear indication is expected */

            /* if sc_buf does not have a scsi command it was not put on the  */
            /* pending queue.  If a queue clear was requested then put the   */
            /* sc_buf on the pending queue if it is not already there.  This */
            /* is ok if there is no scsi command since this queue will be    */
            /* failed without trying to send it to the adapter.              */

            /*this sc_buf has no scsi cmd */
            if (!(bp->scsi_command.scsi_length)){
                /* if queue empty */
                if (scsi->dev[dev_index]->head_pend == NULL) {
                    /* point head at new one */
                    scsi->dev[dev_index]->head_pend = bp;
                    /* point tail at new one */
                    scsi->dev[dev_index]->tail_pend = bp;
                }
                else {  /* pending queue not empty */
                     /* point last cmd's av_forw at the new request */
                     scsi->dev[dev_index]->tail_pend->bufstruct.av_forw =
                                           (struct buf *) bp;
                     /* point tail at new one */
                     scsi->dev[dev_index]->tail_pend = bp;
                }
            }
            /* if an abort, BDR, or reset not in progress then send one to */ 
            /* the device */
            if ((!(scsi->dev[dev_index]->qstate & HALTED)) && 
                (!(scsi->dev[dev_index]->qstate & RESET_IN_PROG))) {
                /* indicate an abort in progress */
                scsi->dev[dev_index]->qstate |= HALTED;
                /* build the abort control element */
                scsi->dev[dev_index]->command_element.request.scsi_cdb.scsi_id
                             = SID(dev_index);
                scsi->dev[dev_index]->command_element.request.scsi_cdb.scsi_lun
                             = LUN(dev_index);
                scsi->dev[dev_index]->command_element.request.scsi_cdb.
                             media_flags |= (VSC_ABORT | VSC_RESUME);
                scsi->dev[dev_index]->command_element.cmd_type  = 
                                                       INTR_LVL_CANCEL;
                /* use the watchdog timer for this device to time the abort */
                /* stop the timer and restart it with the timeout value for */
                /* the abort.  It is ok that active commands are no longer  */
                /* being timed because completion or a time out of the abort*/
                /* will cause the device's active queue to be cleared.      */
                w_stop(&scsi->dev[dev_index]->wdog.dog);
                scsi->dev[dev_index]->wdog.timer_id = SCSI_CANCEL_TMR;
                scsi->dev[dev_index]->wdog.dog.restart = RESET_CMD_T_O;
                w_start(&scsi->dev[dev_index]->wdog.dog);

                /* issue the abort to the adapter drivers output function */
                /* note : if output call failed, it will be treated as a  */
                /* command timeout and recovered as such                  */
#ifdef VSC_TRACE
                vsc_internal_trace(scsi, dev_index, 
                          (uint *) &scsi->dev[dev_index]->command_element,
				   2, 0);
#endif /* VSC_TRACE */
                (void) (scsi->shared->ndd->ndd_output) (scsi->shared->ndd,
                      &scsi->dev[dev_index]->command_element.ctl_elem);
            } 
        } /* end else */
    } /* end if */

    /* if the sc_buf did not have a SC_Q_CLR indication then check to see   */
    /* if it had a SC_Q_RESUME indication, meaning a reactivate SCSI queue  */
    /* request element should be sent to the adapter for this id/lun        */
    else if(bp->flags & SC_Q_RESUME) {
             if (!(bp->scsi_command.scsi_length)) { /* sc_buf does not have */
                                                   /* a scsi command */

                 if ((!(scsi->dev[dev_index]->qstate & HALTED)) && 
                     (!(scsi->dev[dev_index]->qstate & RESET_IN_PROG))) {
                     scsi->dev[dev_index]->cmd_save_ptr = bp;
                 }
                 else {
                     iodone((struct buf *) bp);
                 }
             }
             if (scsi->dev[dev_index]->cc_error_state > 0) {
                  scsi->dev[dev_index]->cc_error_state--;
             }
             if ((!(scsi->dev[dev_index]->qstate & HALTED)) && 
                 (!(scsi->dev[dev_index]->qstate & RESET_IN_PROG))) {
                /* indicate a resume in progress */
                scsi->dev[dev_index]->qstate |= HALTED;
                /* build the resume control element */
                scsi->dev[dev_index]->command_element.request.
                                     scsi_cdb.scsi_id = SID(dev_index);
                scsi->dev[dev_index]->command_element.request.
                                     scsi_cdb.scsi_lun = LUN(dev_index);
                scsi->dev[dev_index]->command_element.request.
                                     scsi_cdb.media_flags |= VSC_RESUME;
                scsi->dev[dev_index]->command_element.cmd_type  = 
                                                       INTR_LVL_CANCEL;
                /* use the watchdog timer for this device to time the resume*/
                /* stop the timer and restart it with the timeout value for */
                /* the resume.  It is ok that active commands are no longer */
                /* being timed because completion or a time out of the      */
                /* resume will result in the timer being reassiged to the   */
                /* devices active queue                                     */
                w_stop(&scsi->dev[dev_index]->wdog.dog);
                scsi->dev[dev_index]->wdog.timer_id = SCSI_CANCEL_TMR;
                scsi->dev[dev_index]->wdog.dog.restart = RESET_CMD_T_O;
                w_start(&scsi->dev[dev_index]->wdog.dog);

                /* issue the resume to the adapter drivers output function */
                /* note : if output call failed, it will be treated as a   */
                /* command timeout and recovered as such                   */
#ifdef VSC_TRACE
                vsc_internal_trace(scsi, dev_index, 
                                   (uint *) &scsi->dev[dev_index]->command_element,
				   2, 0);
#endif /* VSC_TRACE */
                (void) (scsi->shared->ndd->ndd_output) (scsi->shared->ndd,
                      &scsi->dev[dev_index]->command_element.ctl_elem);
             } 
         } /* end if bp->flags & SC_Q_RESUME */

    vsc_start(scsi, dev_index);       /* call start to see if this cmd */
                                          /*  can be kicked off */
    /* restore intrpts */
    unlock_enable(old_pri, &scsi->shared->ndd->ndd_demux_lock);

    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_STRATEGY, ret_code, bp->bufstruct.b_dev);
    return(ret_code);

} /* end vsc_strategy */


/*
 * NAME:     vsc_init_gw_buf
 *
 * FUNCTION: 
 * An adapter free list is maintained which will contain
 * previously allocated kernel_heap buffers, if any, for
 * gathered writes.  In strategy routine, we examine the
 * list and if empty, malloc and pin a buffer for transfer.
 * The buffer size is the next 8KB boundary which will satisfy
 * the requested size (and is on page boundaries).

 * If not empty, we search for a buffer which is in the same
 * 8KB-multiple size range.  Buffers found which are bigger
 * than 8KB but not in the same 8KB-multiple range are freed.
 * Once a buffer is found to be acceptable in size, we use it
 * and exit the list, leaving buffers which follow.  If none
 * are found that are correct, all >8KB end up freed and a
 * buffer is malloced and pinned.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *               
 *
 * EXTERNAL CALLS:
 *              iodone                w_stop
 *              disable_lock          unlock_enable
 *              w_start
 *		
 * INPUT:
 *		sc_buf	- sc_buf structure passed by caller
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *              EIO    - Invalid i/o request 
 *              ENXIO  - The request cannot be accepted because the queue is
 *                       halted.
 *
 */
int
vsc_init_gw_buf (struct scsi_info *scsi,
		 struct sc_buf *bp,
		 int *old_pri)
{
    struct gwrite *current_gw_free, *back_ptr;
    struct gwrite *head_to_free, *save_ptr;
    int           i, rc, required_size, ret_code;

    ret_code = 0;

    if ((bp->bufstruct.b_flags & B_READ) ||	/* if a read */
	(bp->bp != NULL) || (getpid() < 0))  	/* or not proc level */
    {
	ret_code = EINVAL;	/* error--bad command */
	bp->bufstruct.b_error = EINVAL;
	bp->bufstruct.b_flags |= B_ERROR;
	bp->bufstruct.b_resid = bp->bufstruct.b_bcount;	/* none sent */
	bp->status_validity = SC_ADAPTER_ERROR;	/* flag problem  */
	bp->general_card_status = SC_ADAPTER_SFW_FAILURE;
	return (ret_code);
    }

    /* calculate nominal size of buffer required for transfer.
       this algorithm uses 8KB granularity (note this is itself
       a multiple of the page size) */
    required_size = (((bp->bufstruct.b_bcount - 1) / (2 * LPAGESIZE)) + 1)*
	(2 * LPAGESIZE);
    
    /* set local pointer to head of free list */
    current_gw_free = scsi->head_gw_free;
    
    back_ptr = current_gw_free;
    head_to_free = NULL;	/* init list which will collect unused bufs */
    
    /* loop while gwrite free list is non-empty */
    while (current_gw_free)
    {
	if ((current_gw_free->buf_size == required_size) ||
	    (current_gw_free->buf_size > (2 * LPAGESIZE)))
	{
	    /* either correct size, or needs freeing; take off free list */
	    if (current_gw_free == scsi->head_gw_free)
	    {
		scsi->head_gw_free = current_gw_free->next;
		if (!scsi->head_gw_free)	/* if list empty, sync tail */
		    scsi->tail_gw_free = NULL;
	    }
	    else
	    {
		if (current_gw_free == scsi->tail_gw_free)
		{
		    scsi->tail_gw_free = back_ptr;
		    back_ptr->next = NULL;
		}
		else 	/* neither head or tail */
		    back_ptr->next = current_gw_free->next;
	    }

	    if (current_gw_free->buf_size == required_size)
	    {
		/* leave while.  do not process rest of buffers */
		break;
	    }
	    else
	    {
		/* not the required buffer size, so add to temp list */
		save_ptr = current_gw_free->next;
		current_gw_free->next = head_to_free;
		head_to_free = current_gw_free;
		current_gw_free = save_ptr;
		continue;	/* now loop */
	    }
	}
	
	/* if got here, buffer is not of required size, but should be
	   kept.  */
	
	/* update pointers */
	back_ptr = current_gw_free;
	current_gw_free = back_ptr->next;
    } /* end while */
    
    /* enable interrupts during the buffer freeing/mallocing */
    unlock_enable(*old_pri, &scsi->shared->ndd->ndd_demux_lock);
    
    /* see if any buffers are to be freed */
    while (head_to_free)
    {
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
    if (!current_gw_free)
    {
	/* malloc and pin gwrite struct */
	current_gw_free = (struct gwrite *) xmalloc((int) sizeof (struct
								  gwrite),
						    (int) 4,
						    kernel_heap);
	/* if got struct, then pin it */
	if ((!current_gw_free) || (pin((caddr_t) current_gw_free,
				       sizeof (struct gwrite))))
	{
	    /* either malloc or pin failed */
	    if (current_gw_free)
	    {
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
	    return (ret_code);
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
					           (int) required_size)))
	{
	    /* either malloc or pin failed */
	    if (current_gw_free->buf_addr)
	    {
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
	    return (ret_code);
	}
	
	/* initialize the gwrite struct here */
	current_gw_free->buf_size = required_size;
    }
    
    /* move user data to the gwrite buffer */
    rc = uiomove((caddr_t) current_gw_free->buf_addr,
		 (int) current_gw_free->buf_size,
		 UIO_WRITE,
		 (struct uio *) bp->resvd1);
    if (rc)
    {
	/* handle error on uiomove */
	ret_code = EFAULT;
	bp->bufstruct.b_error = EFAULT;
	bp->bufstruct.b_flags |= B_ERROR;
	bp->bufstruct.b_resid = bp->bufstruct.b_bcount;	/* none sent */
	bp->status_validity = SC_ADAPTER_ERROR;	/* flag problem  */
	bp->general_card_status = SC_ADAPTER_SFW_FAILURE;
	return (ret_code);
    }
    /* setup buffer address in bufstruct */
    bp->bufstruct.b_un.b_addr = current_gw_free->buf_addr;
    
    /* indicate this is kernel global memory */
    bp->bufstruct.b_xmemd.aspace_id = XMEM_GLOBAL;
    
    /* save gwrite pointer in bp->resvd1. uio struct ptr over written */
    bp->resvd1 = (uint) current_gw_free;
    
    /* disable interrupts again */
    *old_pri = disable_lock(scsi->ddi.intr_priority, 
                           &scsi->shared->ndd->ndd_demux_lock);

    return (ret_code);
}


/*
 * NAME:     vsc_start
 *
 * FUNCTION: virtual SCSI device start I/O routine
 *           This function initiates commands to the adapter device driver
 *           which have been scheduled by the vsc_strategy routine.
 *
 *
 * EXECUTION ENVIRONMENT:
 *           This routine can only be called on priority levels greater
 *           than, or equal to that of the interrupt handler.
 *
 * NOTES:
 *
 *
 * EXTERNAL CALLS:
 *
 * INPUT:
 *              scsi     - scsi_info structure ptr for this device
 *              dev_index- index into array of devinfo structs for device to
 *                         be started
 *
 * RETURNS:
 *              NONE
 *
 */
void
vsc_start (
    struct  scsi_info *scsi,
    int dev_index)
{
    int    rc,                   /* locally used return code               */
           i,                    /* loop variable for processing waiting   */
                                 /* pending cmd queues                     */
           t_index;              /* used to walk array of dev_info structs */

    if (!scsi->any_waiting) { /* no device is waiting for resources so try  */
                              /* to empty the pending queue for this device */
        rc = vsc_start_pending_cmds(scsi, dev_index);
        if (!rc) {  /* a zero rc indicates a failure to get a cmd_elem      */
                    /* meaning all are in use, set this device queue to     */
                    /* indicate that it is waiting for a free cmd_elem      */
            scsi->any_waiting++;
            scsi->dev[dev_index]->waiting = TRUE;
        } /* else resources obtained or device queue is frozen, so exit */
    }
    else {   /* some device is waiting for resources so before servicing the */
             /* queue for this device, attempt to start the pending queue    */
             /* for a device which has been waiting.                         */
    
        /* if this device  has pending requests and is not already in the    */
        /* waiting state,  then put it in the waiting state to schedule it   */
        /* for resource allocation.                                          */
        if ((scsi->dev[dev_index]->head_pend != NULL) &&
            (!scsi->dev[dev_index]->waiting)) {
            scsi->any_waiting++;      /* inc global count */
            scsi->dev[dev_index]->waiting = TRUE;      /* set device flag  */
        }
        /* the following will loop through device queues starting with the   */
        /* device at dev_index + 1, and attempt to allocate resources for a  */
        /* previously waiting device.  This code does not ensure that the    */
        /* device that has been waiting the longest will be the first to get */
        /* resources but this algorithm does ensure that every waiting device*/
        /* will eventually get resources (no starvation) and when resources  */
        /* are scarce, they will be distrubuted equally among all devices.   */

        /* loop thru devices, starting with next device and ending with      */
        /* this device.                                                      */
        i = dev_index + 1;
        while ((i - (dev_index + 1)) < DEVPOINTERS) {
            t_index = i % DEVPOINTERS; /* modulo number of devices */
            if ((scsi->dev[t_index] != NULL) && 
                (scsi->dev[t_index]->waiting)){
                rc = vsc_start_pending_cmds(scsi, t_index);
                if (rc)  {    /* device did not fail to get resources   */
                    /* decrement global count of waiting devices        */
                    scsi->any_waiting--;  
                    /* reset device waiting flag */
                    scsi->dev[t_index]->waiting = FALSE; 
                } 
                else { /* cant get resources so stop looping through devices */
                    break;
                }
            }
            i++; /* inc loop counter to process next device */
        } /* end while */
    } /* end else,  scsi->any_waiting == TRUE */

    return;

} /* end vsc_start */

/*
 * NAME:     vsc_start_pending_cmds
 *
 * FUNCTION: This function makes sure a device queue is not frozen, and then
 *           attempts to obtain a cmd_element for this command and issues
 *           the command to the adapter driver's output routine.
 *          
 * EXECUTION ENVIRONMENT:
 *           This routine can only be called on priority levels greater
 *           than, or equal to that of the interrupt handler.
 * 
 *
 * NOTES:
 *
 *
 * EXTERNAL CALLS:
 *              w_start
 *
 * INPUT:
 *              scsi     - scsi_info structure ptr for this device
 *              dev_index- index into array of devinfo structs for device to
 *                         be started
 *
 * RETURNS:
 *		TRUE indicates that there was not a failure getting resources
 *              (cmd_elem) and the pending queue for this device was either
 *              all made active or the queue for this device is frozen.
 * 
 *              FALSE indicates that while attempting to make active all the 
 *              pending cmds for this device, a failure was encountered
 *              allocating resources (cmd_elem all in use).
 *              
 *
 */
int
vsc_start_pending_cmds (
    struct  scsi_info *scsi,
    int dev_index)
{
    int  ret_code,                      /* return code for this function    */
         rc;                            /* locally used return code         */
    struct  cmd_elem *cmd;              /* pointer to cmd_elem for this cmd */
    struct  sc_buf *scp;                /* pointer to head pending sc_buf   */


    ret_code = TRUE;  /* default return code to say resources availible   */

    /* loop until no pending i/o, or until prevented from sending cmds    */
    while (scsi->dev[dev_index]->head_pend != NULL) {
        scp = scsi->dev[dev_index]->head_pend;  /* point to head element  */
        if (!scsi->dev[dev_index]->qstate) {  /* queue is not halted      */
            cmd = vsc_get_cmd_elem(scsi);
            if (cmd == NULL)  { /* could not get resource; all in use     */
                ret_code = FALSE;
                return(ret_code);
            }
        }
        else {     /* qstate != 0; depending upon which flags are set, it */
                   /* may still be possible to issue this command.        */
            /* if any of the following conditions are true: HALTED means  */
            /* reset, abort or BDR is active, command delay after reset   */
            /* may still be active, or a CAC condition where the sc_buf   */
            /* is not marked for expedite.(resvd7), then the queue for    */
            /* this device is halted so just return TRUE.  Note: returning*/
            /* true from this function indicates that there was not a     */
            /* failure trying to allocate resources.  It does not mean    */
            /* that the command was issued to the adapter.                */ 
            if((scsi->dev[dev_index]->qstate & HALTED) ||
               ((scsi->dev[dev_index]->qstate & Q_STARVE) && (!scp->resvd7)) ||
               (scsi->dev[dev_index]->qstate & PENDING_ERROR) ||
               (scsi->dev[dev_index]->qstate & RESET_IN_PROG) ||
               ((scsi->dev[dev_index]->qstate & CDAR_ACTIVE) &&
                (scp->flags & SC_DELAY_CMD)) ||
               ((scsi->dev[dev_index]->qstate & CAC_ERROR) &&
                (!scp->resvd7))) {  /* queue is frozen */

                return(ret_code);
            }
            else {  /* queue is not frozen, try to get resource */ 
                cmd = vsc_get_cmd_elem(scsi);
                if (cmd == NULL)  { /* could not get resource; all in use */
                    ret_code = FALSE;
                    return(ret_code);
                }
            }
               
        } /* end else qstate != 0 */ 

        /* at this point, it is determined that the queue for this device is */
        /* not frozen and a cmd_elem has been obtained.  Now convert the     */
        /* sc_buf into a cmd_elem and issue the command to the adapter       */
        /* driver's output function.                                         */ 

        DDHKWD5(HKWD_DD_SCSIDD, DD_ENTRY_BSTART, 0, scsi->devno, scp,
                scp->bufstruct.b_flags, scp->bufstruct.b_blkno,
                scp->bufstruct.b_bcount);

        /* the b_work field of the bufstruct in the sc_buf is used to store a*/
        /* pointer to the command element used for the command in the sc_buf */
         scp->bufstruct.b_work = (int) cmd;


        cmd->bp = scp;  /* set pointer to sc_buf */
        cmd->cmd_state = IS_ACTIVE; /* setup command state */ 
        cmd->preempt = MAX_PREEMPTS; 
        if (scp->resvd7) {  /* sc_buf marked for expedite */
            cmd->ctl_elem.flags = EXPEDITE;
        }
        else {  /* no flags need to be set */
            cmd->ctl_elem.flags = 0;
        }
        /* put total data lenght in pds_data_len field of cntl elem blk     */
        cmd->ctl_elem.pds_data_len = scp->bufstruct.b_bcount;
     
        /* put the SCSI ID of target in the scsi_cdb of the request elem    */
        cmd->request.scsi_cdb.scsi_id = scp->scsi_command.scsi_id;

        /* put the SCSI LUN of target in the scsi_cdb of the request elem   */
        cmd->request.scsi_cdb.scsi_lun = LUN(dev_index);

        /* put the SCSI command block in the scsi_cdb of the request elem   */
        cmd->request.scsi_cdb.scsi_cmd_blk = scp->scsi_command.scsi_cmd;

        /* put total data lenght in scsi_cdb  of request element            */
        cmd->request.scsi_cdb.scsi_data_length =  scp->bufstruct.b_bcount;

        /* put total data lenght in the total_len field of pd_info1 struct */
        cmd->pd_info1.total_len =  scp->bufstruct.b_bcount;
  
        /* set appropriate media flags in scsi_cdb of request element       */
        switch (scp->q_tag_msg) { 
            case  SC_NO_Q : 
                cmd->request.scsi_cdb.media_flags = VSC_NO_Q; 
                break;
            case SC_SIMPLE_Q  :
                cmd->request.scsi_cdb.media_flags = VSC_SIMPLE_Q; 
                break;
            case SC_HEAD_OF_Q :
                cmd->request.scsi_cdb.media_flags = VSC_HEAD_OF_Q; 
                break;
            case SC_ORDERED_Q :
                cmd->request.scsi_cdb.media_flags = VSC_ORDERED_Q; 
                break;
        }  /* end switch */
        
        if (scsi->dev[dev_index]->need_to_resume_queue) {
            cmd->request.scsi_cdb.media_flags |=  VSC_RESUME;
            scsi->dev[dev_index]->need_to_resume_queue = FALSE;
        }
        /* put the size of the SCSI CDB in the scsi_cdb of requeset element */
        cmd->request.scsi_cdb.media_flags |= 
                    (scp->scsi_command.scsi_length << 24);
        /* set the no disconnect and async media flags according to sc_buf */
        if (scp->scsi_command.flags & SC_NODISC) 
            cmd->request.scsi_cdb.media_flags |= VSC_NO_DISC; 
        if (scp->scsi_command.flags & SC_ASYNC) 
            cmd->request.scsi_cdb.media_flags |= VSC_ASYNC; 
        /* set the direction of data transfer media flags according to sc_buf*/
        if (scp->bufstruct.b_bcount) {  
            /* data transfer requested for this cmd*/
            if (scp->bufstruct.b_flags & B_READ) {
                /* this is a read request */ 
                cmd->request.scsi_cdb.media_flags |= VSC_READ; 
            }
            else  {  /* this is a write request */
                cmd->request.scsi_cdb.media_flags |= VSC_WRITE; 
            }
        } /* end if */
        /* set the p_buf_list pointer in pd_info1 to point to buf structs */
        /* for this command */
        if (scp->bp != NULL) {  /* if this is a spanned command */
            cmd->pd_info1.p_buf_list = (caddr_t)scp->bp;
        } 
        else { /* non spanned command, pass bufstruct */
            cmd->pd_info1.p_buf_list = (caddr_t)&scp->bufstruct;
        }
   
        /* dequeue sc_buf from head of pending list */
        scsi->dev[dev_index]->head_pend = (struct sc_buf *) 
                                           scp->bufstruct.av_forw;
        if (scsi->dev[dev_index]->head_pend == NULL) {
            scsi->dev[dev_index]->tail_pend = NULL;   /* update tail pointer */
        }

        /* enqueue sc_buf to end of active list; active queue doubly linked  */
        if (scsi->dev[dev_index]->head_act == NULL) { /* is act list empty?  */
            scsi->dev[dev_index]->head_act = scp;     /* put on head of list */
            scsi->dev[dev_index]->tail_act = scp;     /* update tail pointer */
        }
        else {      /* act list not empty  */
            /* update pointer of last element on active chain */
            scp->bufstruct.av_back = 
                        (struct buf *) scsi->dev[dev_index]->tail_act;
            scsi->dev[dev_index]->tail_act->bufstruct.av_forw  = 
                        (struct buf *) scp;
            scsi->dev[dev_index]->tail_act = scp;    /* update tail pointer */
        }

        /* mark new end of active chain */
        scp->bufstruct.av_forw = NULL;

        /* increment dev num_act_cmds counter */
        scsi->dev[dev_index]->num_act_cmds++;

        /* if putting cmd_elem on an empty active queue, start the timer for */
        /* this device queue */
        if (scsi->dev[dev_index]->num_act_cmds == 1) {
            scsi->dev[dev_index]->wdog.dog.restart = 
                          scsi->dev[dev_index]->head_act->timeout_value+1;
            w_start(&scsi->dev[dev_index]->wdog.dog);
        }
#ifdef VSC_TRACE
        vsc_internal_trace(scsi, dev_index, (uint *) cmd, 0, 0);
#endif /* VSC_TRACE */

        /* issue the command to the adapter drivers output function */
        rc = (scsi->shared->ndd->ndd_output) (scsi->shared->ndd,
                                              &cmd->ctl_elem);
        if (rc) {  /* an error returned from output */
            /* setup error status in the sc_buf */
            scp->status_validity = SC_ADAPTER_ERROR;
            scp->general_card_status = SC_ADAPTER_HDW_FAILURE;
            scp->bufstruct.b_resid = scp->bufstruct.b_bcount;
            scp->bufstruct.b_error = EIO;
            /* allow resources to be freed */
            cmd->cmd_state = INTERRUPT_RECVD;      
            if (scsi->dev[dev_index]->num_act_cmds == 1) {
            /* if this is the only active command, then fail the queue for */
            /* this device */
                vsc_fail_cmd(scsi, dev_index);
            }
            else {     
            /* there are other active cmds for this device, leave this cmd */
            /* on the active list and wait for other cmds to complete.  Set*/
            /* qstate & PENDING_ERROR to prevent any new cmds from being   */
            /* sent.  Eventually the wdog will expire for this device and  */
            /* since PENDING_ERROR is set, the queue will be failed without*/
            /* any error recovery, since the command which timed out never */               /* made it to the adapter device driver.                       */

                scsi->dev[dev_index]->qstate |= PENDING_ERROR;
            }
        } /* end if rc */

        DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_BSTART, 0, scsi->devno);

    } /* end while */
    return(ret_code);

} /* end vsc_start_pending_cmds */


/*
 *
 * NAME:	vsc_recv 
 *
 * FUNCTION: 
 *		This function is the virtual SCSI device's receive routine 
 *              (pseudo interrupt handler).  Request are sent to the adapter
 *              device driver through its output function specified in the 
 *              ndd obtained by an ns_alloc call.  Upon completion of a 
 *              request sent to the output function, this receive function
 *              is called to process completion of the request.
 *
 * EXECUTION ENVIRONMENT:
 *		this routine can only be called from the interrupt level
 *
 * NOTES:
 *		
 *
 * EXTERNAL CALLS:
 *              w_stop                w_start
 *              e_wakeup              iodone
 *		
 * INPUT:
 *		ndd		- parent adapter's ndd structure
 *		reply_elem      - a pointer to the reply element corresponding
 *                                to the completed request 
 *
 * RETURNS:  
 *              NONE
 *
 */
void
vsc_recv(
    ndd_t	*p_ndd,
    struct	rpl_element_def *reply_elem )
{

    struct cmd_elem *cmd,       /* cmd_elem corresponding to this reply_elem */
                    *tcmd;      /* temp cmd_elem used for for abort and BDR  */
                                /* completion processing.                    */
    struct scsi_info *scsi;     /* scsi_info struct for this reply elemennt  */
    int dev_index,              /* index into list of dev_info for this dev  */
        tag,                    /* tag used to mark cmd_elem as free in the  */
                                /* free_cmd_list                             */
        t_lun,                  /* for loop variable to look at every lun for*/
                                /* a BDR completion                          */
        t_index,                /* temp dev_index to look at dev_info structs*/
                                /* for every lun of a given id               */
        need_abort=TRUE;        /* flag to indicate if an abort is necessary */
                                /* to clear the queue at adapter for this err*/
    ushort queue_suspended;     /* indicates if the queue for this device was*/
                                /* suspended at the adapter for an error.    */
    uchar fatal_error = FALSE,  /* indicates if a fatal_error occurred for   */
                                /* the command completing.                   */
    need_to_start_dev = FALSE,  /* indicates that the vsc_start function     */
                                /* needs to be called for this device        */
    need_to_clear_halted = TRUE;/* indicates if the HALTED qstate flag needs */
                                /* to be cleared upon completion of a BDR or */
                                /* abort.                                    */

    struct gwrite *gw_ptr;
 
    /* get the cmd_elem structure corrsponding to this reply element */
    cmd =  (struct cmd_elem *) reply_elem;
    /* get the scsi_info struct corresponding to this cmd_elem */
    scsi = cmd->scsi;

    DDHKWD1(HKWD_DD_SCSIDD, DD_ENTRY_INTR, 0, scsi->devno);

    /* build the dev_index for this device */
    dev_index = INDEX(cmd->request.scsi_cdb.scsi_id, 
                      cmd->request.scsi_cdb.scsi_lun); 

    /* if this is a cancel request element completing */
    if (cmd->ctl_elem.flags & CANCEL) {

        if (cmd->cmd_state == WAIT_FOR_TO_2) {
            /* this is an interrupt received for a cancel command which has */
            /* timed out.  Set the state to indicate the cmd_elem is free,  */
            /* clear necessary qstate flags and return */
            cmd->cmd_state = INACTIVE;
            /* if this is a SCSI bus reset cmd_elem then clear the        */
            /* RESET_IN_PROG indication in the qstate of all open devices */
            if ((cmd->request.scsi_cdb.media_flags & VSC_SCSI_RESET)
                 == VSC_SCSI_RESET) {
#ifdef VSC_TRACE
                vsc_internal_trace(scsi, -1, (uint *) cmd, 5, 0);
#endif /* VSC_TRACE */
                /* clear reset in progress indication on all open devices */
                for (t_index = 0; t_index < DEVPOINTERS; t_index++) {
                    /* if device is open */
                    if (scsi->dev[t_index]!= NULL) {
                        scsi->dev[t_index]->qstate &= ~(RESET_IN_PROG);
                        if (scsi->dev[t_index]->head_act != NULL) {
                            vsc_fail_cmd(scsi, t_index);
                        }
                        else {  
                            if (scsi->dev[t_index]->head_pend != NULL) {
                                vsc_start(scsi, t_index);
                            }
                        }
                    }
                 
                } /* end for */
                DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_INTR, 0, scsi->devno);
                return;
            } /* end if cmd_elem is == VSC_SCSI_RESET */
#ifdef VSC_TRACE
            vsc_internal_trace(scsi, dev_index, (uint *) cmd, 3, 0);
#endif /* VSC_TRACE */
            /* at this point, the cancel cmd_elem must be a BDR, ABORT, or  */
            /* RESUME associated with a particular device.(dev_info struct) */
            /* clear the media flags to prepare for next use of */
            /* this cmd_elem */
            if (cmd->request.scsi_cdb.media_flags & VSC_ABORT ) {
                /* clear the halted indication for this device */
                /* unless a BDR is active for another lun. BDR */
                /* affects all luns so if BDR is active from   */
                /* a different lun, the HALTED indication      */
                /* should remain set.                          */
                for (t_lun = 0; t_lun < MAX_LUNS; t_lun++) {
                    t_index = INDEX(SID(dev_index), t_lun);
                    /* if device is open */
                    if (scsi->dev[t_index]!= NULL) {
                        tcmd = &scsi->dev[t_index]->command_element;
                        if ((tcmd->request.scsi_cdb.media_flags & VSC_BDR) &&
                             (tcmd->cmd_state != INACTIVE)) {
                            /* BDR active from another lun so do not    */
                            /* clear HALTED indication.                 */
                            need_to_clear_halted = FALSE;
                        }
                    }
                } /* end for */

                if (need_to_clear_halted) {
                    scsi->dev[dev_index]->qstate &= ~(HALTED);
                }

            } /* end if media_flags | VSC_ABORT */
            else {
                /* else if completion of an interrupt lvl BDR */
                if(cmd->request.scsi_cdb.media_flags & VSC_BDR ) {
                    for (t_lun = 0; t_lun < MAX_LUNS; t_lun++) {
                        t_index = INDEX(SID(dev_index), t_lun);
                        /* if device is open */
                        if (scsi->dev[t_index]!= NULL) {
                            /* if command element for that device is not   */
                            /* active then its ok to clear the HALTED flag */
                            tcmd = &scsi->dev[t_index]->command_element;
                            if (tcmd->cmd_state == INACTIVE) {
                                scsi->dev[t_index]->qstate &= ~(HALTED);
                            }
                        }
                    } /* end for */
                } /* end if media_flags | VSC_BDR */
                /* else it must be completion of interrupt lvl resume */
                else {
                    /* clear the CAC_ERROR indication for this device*/
                    scsi->dev[dev_index]->qstate &= ~(CAC_ERROR);
                    /* clear the halted indication for this device */
                    /* unless a BDR is active for another lun. BDR */
                    /* affects all luns so if BDR is active from   */
                    /* a different lun, the HALTED indication      */
                    /* should remain set.                          */
                    for (t_lun = 0; t_lun < MAX_LUNS; t_lun++) {
                        t_index = INDEX(SID(dev_index), t_lun);
                        /* if device is open */
                        if (scsi->dev[t_index]!= NULL) {
                            tcmd = &scsi->dev[t_index]->command_element;
                            if ((tcmd->request.scsi_cdb.media_flags & VSC_BDR)
                                && (tcmd->cmd_state != INACTIVE)) {
                                /* BDR active from another lun so do not    */
                                /* clear HALTED indication.                 */
                                need_to_clear_halted = FALSE;
                            }
                        }
                    } /* end for */

                    if (need_to_clear_halted) {
                        scsi->dev[dev_index]->qstate &= ~(HALTED);
                    }
                    if (scsi->dev[dev_index]->cmd_save_ptr != NULL) {
                        iodone((struct buf *)
                                scsi->dev[dev_index]->cmd_save_ptr);
                        scsi->dev[dev_index]->cmd_save_ptr = NULL;
                    }
                } /* end else completion of intr lvl resume */
            }
            cmd->request.scsi_cdb.media_flags &= 
                         (~(VSC_ABORT |VSC_ADAP_ONLY | VSC_RESUME | VSC_BDR));
            /* see if an SCIOSTOP is sleeping on this command completion */
            if (scsi->dev[dev_index]->stop_pending) { 
                e_wakeup(&scsi->dev[dev_index]->dev_event);
            }
            DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_INTR, 0, scsi->devno);
            return;
        } /* end if cmd_state == WAIT_FOR_TO_2 */

        cmd->cmd_state = INACTIVE;
        /* if this is a SCSI bus reset completion */
        if ((cmd->request.scsi_cdb.media_flags & VSC_SCSI_RESET)
             == VSC_SCSI_RESET) {
#ifdef VSC_TRACE
            vsc_internal_trace(scsi, -1, (uint *) cmd, 5, 0);
#endif /* VSC_TRACE */
            /* stop the timer for the SCSI bus reset */
            w_stop(&scsi->reset_wdog.dog);

            /* if this is an error element for a SCSI bus reset, then */
            /* place an error in the error log for this device        */
            if (reply_elem->header.options & VSC_EVENT_MASK) {
                vsc_log_err(scsi, ERRID_SCSI_ERR10, 6, cmd, 0);
            }
            else { /* no error for SCSI bus reset, but log an error     */
                   /* to indicate a system initiated bus reset occurred */
                vsc_log_err(scsi, ERRID_SCSI_ERR10, 256, cmd, 0);
                /* notify registered users of the SCSI bus reset event */
                vsc_async_notify(scsi, SC_SCSI_RESET_EVENT);
            }
            /* clear reset in progress indication on all open devices */
            for (t_index = 0; t_index < DEVPOINTERS; t_index++) {
                /* if device is open */
                if (scsi->dev[t_index]!= NULL) {
                    scsi->dev[t_index]->qstate &= ~(RESET_IN_PROG);
                }
            } /* end for */
            /* complete processing of SCSI bus reset completion */
            vsc_process_scsi_reset(scsi);
            DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_INTR, 0, scsi->devno);
            return;

        } /* end if SCSI bus reset completion */
        else {   /* this is a BDR, abort, or resume completion */
#ifdef VSC_TRACE
            vsc_internal_trace(scsi, dev_index, (uint *) cmd, 3, 0);
#endif /* VSC_TRACE */
            /* stop the timer for the cancel request element */
            w_stop(&scsi->dev[dev_index]->wdog.dog);
            /* convert the timer back to a SCSI_DEV_TMR */
            scsi->dev[dev_index]->wdog.timer_id = SCSI_DEV_TMR;
            if(cmd->cmd_type == PROC_LVL_CANCEL) {
                /* if this is an error element for a process level cancel */
                /* place an error in the error log for this device        */
                if (reply_elem->header.options & VSC_EVENT_MASK) {
                    vsc_log_err(scsi, ERRID_SCSI_ERR10, 8, cmd, 0);
                    scsi->proc_results = EIO;
                }
                else {
                    scsi->proc_results = GOOD_COMPLETION;
                }
                e_wakeup(&scsi->ioctl_event);
                /* if the queue was suspended for this error, set flag */
                /* to indication resume is needed on next command      */
                if (reply_elem->adap_status & VSC_Q_SUSPENDED) {
                    scsi->dev[dev_index]->need_to_resume_queue = TRUE;
                }
            }  /* end if cmd_type == PROC_LVL_CANCEL */  

            else {  /* this in an interrupt lvl cancel */

                if (reply_elem->header.options & VSC_EVENT_MASK) {
                    /* an error occured for this interrupt lvl cancel  */
                    /* place an error in the error log for this device */
                    vsc_log_err(scsi, ERRID_SCSI_ERR10, 9, cmd, 0);
                    /* interrupt lvl BDRs are sent only as a result of    */
                    /* a command timeout.  If this is an error completion */
                    /* of an interrupt lvl BDR and the device error code  */
                    /* indicates that a bus reset has occured or a bus    */
                    /* reset is needed, then issue a SCSI bus reset.      */
                    /* Also set fatal_error TRUE because with this type   */
                    /* of error, all active commands for the device may   */
                    /* not have been returned.  Queues for all devices    */
                    /* will be cleared upon completion of the SCSI bus    */
                    /* reset */
                    if ((reply_elem->cmd_error_code) ||
                        (reply_elem->device_error_code == VSC_SEL_TIMEOUT) ||
                        (reply_elem->device_error_code == VSC_NEED_I_RESET) ||
                        (reply_elem->device_error_code == VSC_NEED_E_RESET) ||
                        (reply_elem->device_error_code ==
                                              VSC_CANCELED_BY_RESET)) {
                        
                        vsc_scsi_reset(scsi); 
                        fatal_error = TRUE;
                    }  /* end if BDR failed needing reset */  
          
                    /* if the queue was suspended for this error, set flag */
                    /* to indication resume is needed on next command      */
                    if (reply_elem->adap_status & VSC_Q_SUSPENDED) {
                        scsi->dev[dev_index]->need_to_resume_queue = TRUE;
                    }

                } /* end if an error occurred */

                /* if this is the completion of an interrupt lvl abort */
                if (cmd->request.scsi_cdb.media_flags & VSC_ABORT ) {
                    /* clear the halted indication for this device */
                    /* unless a BDR is active for another lun. BDR */
                    /* affects all luns so if BDR is active from   */
                    /* a different lun, the HALTED indication      */
                    /* should remain set.                          */
                    for (t_lun = 0; t_lun < MAX_LUNS; t_lun++) {
                        t_index = INDEX(SID(dev_index), t_lun);
                        /* if device is open */
                        if (scsi->dev[t_index]!= NULL) {
                            tcmd = &scsi->dev[t_index]->command_element;
                            if ((tcmd->request.scsi_cdb.media_flags & VSC_BDR)
                                && (tcmd->cmd_state != INACTIVE)) {
                                /* BDR active from another lun so do not    */
                                /* clear HALTED indication.                 */
                                need_to_clear_halted = FALSE;
                            }
                        }
                    } /* end for */

                    if (need_to_clear_halted) {
                        scsi->dev[dev_index]->qstate &= ~(HALTED);
                    }
                    /* NOTE : upon completion of an abort, it is assumed */
                    /* all active cmds have already been returned for the*/
                    /* device being aborted.                             */

                    if ((!fatal_error) && (need_to_clear_halted)) {
                        /* clear the queue for this device */
                        vsc_fail_cmd(scsi, dev_index);
                    }
                } /* end if media_flags & VSC_ABORT */
                else {
                    /* else if completion of an interrupt lvl BDR */
                    if(cmd->request.scsi_cdb.media_flags & VSC_BDR ) {
                        /* a BDR will affect all luns at the specified   */
                        /* SCSI id.  Set CDAR_ACTIVE flag for these      */
                        /* devices and clear the HALTED flag now that   */
                        /* BDR has completed                            */
                        for (t_lun = 0; t_lun < MAX_LUNS; t_lun++) {
                            t_index = INDEX(SID(dev_index), t_lun);
                            /* if device is open */
                            if (scsi->dev[t_index]!= NULL) {
                                scsi->dev[t_index]->qstate |= CDAR_ACTIVE;
                                /* if command element for that device is not */
                                /* active then its ok to clear HALTED flag   */
                                tcmd = &scsi->dev[t_index]->command_element;
                                if (tcmd->cmd_state == INACTIVE) {
                                    scsi->dev[t_index]->qstate &= ~(HALTED);
                                    if (!(fatal_error)) {
                                       /* see if queue needs to be cleared */
                                       if (scsi->dev[t_index]->head_act !=
                                           NULL){
                                           vsc_fail_cmd(scsi, t_index);
                                        }
                                        else { 
                                        /* try to start any i/o which may*/
                                        /* be pending because of BDR     */
                                            vsc_start(scsi, t_index);
                                        }
                                    } /* end if !fatal_error */
                                }
                            }
                        } /* end for */

                        /* initiate command delay after reset timer */
                        w_start(&scsi->cdar_wdog.dog);

                    } /* end if completion of BDR */ 
                    /* else it must be completion of interrupt lvl resume */
                    else {
                        /* if this is the last resume for CAC error recovery */
                        /* then  clear the CAC_ERROR indication for this     */
                        /*  device */
                        if (!(scsi->dev[dev_index]->cc_error_state)) {
                            scsi->dev[dev_index]->qstate &= ~(CAC_ERROR);
                        }
                        /* clear the HALTED indication for this device */
                        scsi->dev[dev_index]->qstate &= ~(HALTED);
                        if (scsi->dev[dev_index]->cmd_save_ptr != NULL) {
                            iodone((struct buf *) 
                                    scsi->dev[dev_index]->cmd_save_ptr);
                            scsi->dev[dev_index]->cmd_save_ptr = NULL;
                        }
                        /* restart the timer for this device */
                        if (scsi->dev[dev_index]->num_act_cmds > 0 ) {
                            scsi->dev[dev_index]->wdog.dog.restart =
                            scsi->dev[dev_index]->head_act->timeout_value;
                            w_start(&scsi->dev[dev_index]->wdog.dog);
                        }

                        /* attempt to restart cmd queue for this device */
                        vsc_start(scsi, dev_index);
                    } /* end else completion of intr lvl resume */
                } /* end else not VSC_ABORT flag set */
            }  /* end else interrupt lvl cancel */

            /* clear the media flags to prepare for next use of */
            /* this cmd_elem */
            cmd->request.scsi_cdb.media_flags &= 
                         (~(VSC_ABORT | VSC_BDR | VSC_ADAP_ONLY | VSC_RESUME));

        } /* end else BDR, abort or resume completion */

        /* see if an SCIOSTOP is sleeping on this command completion */
        if (scsi->dev[dev_index]->stop_pending) { 
            e_wakeup(&scsi->dev[dev_index]->dev_event);
        }
        /* processing complete for this element */
        DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_INTR, 0, scsi->devno);
        return;

    } /* end if CANCEL completion */

#ifdef VSC_TRACE
    vsc_internal_trace(scsi, dev_index, (uint *) cmd, 1, 0);
#endif /* VSC_TRACE */

    /* if cmd is a reply element; no error occurred; normal device i/o cmd */
    /* completion */
    if (!(reply_elem->header.options & VSC_EVENT_MASK)) {

        ASSERT(cmd->cmd_state == IS_ACTIVE);
        /* set cmd_state to indicate interrupt received */
        cmd->cmd_state = INTERRUPT_RECVD;

        DDHKWD2(HKWD_DD_SCSIDD, DD_SC_INTR, 0, scsi->devno, cmd->bp);

        /* verify the watchdog for this device is timing device i/o cmds */
        /* It is possible that a proc level abort or bdr was sent for    */
        /* this device, the timer was converted to a SCSI_CANCEL_TMR but */
        /* a cmd completed successfully before the abort went out.       */
        if (scsi->dev[dev_index]->wdog.timer_id == SCSI_DEV_TMR) {
            /* stop the watchdog timer for this device and check if it   */
            /* needs to be restarted */
            w_stop(&scsi->dev[dev_index]->wdog.dog);
            if (scsi->dev[dev_index]->num_act_cmds > 1 ) {
                scsi->dev[dev_index]->wdog.dog.restart =
                       scsi->dev[dev_index]->head_act->timeout_value;
                w_start(&scsi->dev[dev_index]->wdog.dog);
            }
        } /* end if timer_id == SCSI_DEV_TMR */
        /* Note : status_validity and b_error fields are already */
        /*        set to zero */
        cmd->bp->bufstruct.b_resid = reply_elem->resid_count;
        /* decrement count ouf active commands for this device */
        scsi->dev[dev_index]->num_act_cmds--;

	/* free the gwrite struct if it was a gathered write */
	if (cmd->bp->resvd1)
	{
	    gw_ptr = (struct gwrite *) cmd->bp->resvd1;
	    /* put the gwrite struct on tail of gwrite free list */
	    if (scsi->head_gw_free == NULL)
		scsi->head_gw_free = gw_ptr;
	    else
		scsi->tail_gw_free->next = gw_ptr;
	    scsi->tail_gw_free = gw_ptr;
	    gw_ptr->next = NULL;	/* mark new end of chain */
	    cmd->bp->resvd1 = NULL;
	}

        /* mark this cmd_elem as inactive and return it to the free pool */
        tag = cmd->tag;
        cmd->cmd_state = INACTIVE;
        VSC_FREETAG(cmd->scsi->free_cmd_list[(int)((tag)/
                    VSC_BITS_PER_WORD)], tag);
        vsc_deq_active(scsi, cmd->bp, dev_index);

        iodone((struct buf *) cmd->bp);
          
        if (scsi->dev[dev_index]->head_act == NULL) {
            if (scsi->dev[dev_index]->qstate & Q_STARVE) {
                need_to_start_dev = TRUE;
                scsi->dev[dev_index]->qstate &= (~Q_STARVE);
            }
        }
        else {   /* active list not empty */
            /* decrement the preempt count of the command at the head */
            cmd = (struct cmd_elem *) 
                  scsi->dev[dev_index]->head_act->bufstruct.b_work;
            cmd->preempt--;
            if (cmd->preempt == 0) {
                scsi->dev[dev_index]->qstate |= Q_STARVE;
            }
        } /* end else active list not empty */   
     
        /* see if any devices are waiting on the cmd_elem which was */
        /* just freed */
        if ((scsi->any_waiting) || (need_to_start_dev)) {
             vsc_start(scsi, dev_index);
        }

        /* see if an SCIOSTOP is sleeping on this command completion */
        if (scsi->dev[dev_index]->stop_pending) { 
            e_wakeup(&scsi->dev[dev_index]->dev_event);
        }

    }  /* end if good completion */

    else {  /* error completion */

        DDHKWD2(HKWD_DD_SCSIDD, DD_SC_INTR, 0, scsi->devno, cmd->bp);

        if (cmd->cmd_state == WAIT_FOR_TO_2) {
            /* this indicates we are receiving an interrupt for a cmd which */
            /* has already timed out.  Update the cmd_state to show that    */
            /* an interrupt has been received.  Leave the command on the    */
            /* active list for this device as it will be cleared upon       */
            /* completion of the BDR issued to recover from the timeout.    */
            cmd->cmd_state = INTERRUPT_RECVD;
            DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_INTR, 0, scsi->devno);
            return;
        } /* end if cmd_state == WAIT_FOR_TO_2 */

        /* verify the watchdog for this device is timing device i/o cmds */
        /* It is possible that a proc level abort or bdr was sent for    */
        /* this device, the timer was converted to a SCSI_CANCEL_TMR but */
        /* a cmd completed before the abort went out.                    */
        if (scsi->dev[dev_index]->wdog.timer_id == SCSI_DEV_TMR) {
            /* stop the watchdog timer for this device and check if it   */
            /* needs to be restarted */
            w_stop(&scsi->dev[dev_index]->wdog.dog);
            if (scsi->dev[dev_index]->num_act_cmds > 1 ) {
                scsi->dev[dev_index]->wdog.dog.restart =
                       scsi->dev[dev_index]->head_act->timeout_value;
                w_start(&scsi->dev[dev_index]->wdog.dog);
            }
        } /* end if timer_id == SCSI_DEV_TMR */

        /* set cmd_state to indicate interrupt received */
        cmd->cmd_state = INTERRUPT_RECVD;

        /* determine if the cmd queue for this device is suspended */
        queue_suspended =  reply_elem->adap_status & VSC_Q_SUSPENDED;
    
        /*********************************************************************/
        /*                                                                   */
        /* NOTE : error elements returned for device i/o cmds (those         */
        /* initiate through the strategy path) will be parsed as follows :   */
        /*                                                                   */
        /* step 1 :If the scsi_status byte is non zero, then the SCSI status */
        /*         is considered the primary error and will be the error     */
        /*         returned for this command.                                */
        /*                                                                   */
        /* step 2 :If the cmd_error_code byte is non zero then this is       */
        /*         considered the cause of the cmd failure and will be the   */
        /*         error returned for this command.                          */
        /*                                                                   */
        /* step 3 :If the device_error_code byte is non zero then this is    */
        /*         considered the cause of the cmd failure and will be the   */
        /*         error returned for this command.                          */
        /*                                                                   */
        /* step 4 :If the scsi_status byte, cmd_error_code byte, and         */
        /*         and device error code byte are all zero, then this is     */
        /*         considered an unknown error for this command and an       */
        /*         adapter software failure will be returned for this cmd    */
        /*                                                                   */
        /* Also : If the device_error_code or cmd_error_code contain values  */
        /*        that are invalid (do not match known values) this is       */
        /*        considered an unknown adapter failure and an adapter       */
        /*        software failure will be returned for this cmd             */
        /*                                                                   */
        /*********************************************************************/

        if (reply_elem->scsi_status != 0) {
        /* scsi status is valid so this is the error which should be returned*/

            /* if a check condition occurred and commands */
            /* are being queued to the device then set up */
            /* cc_error_state so that command tag queuing */
            /* error recovery can be initiated (queue not */
            /* cleared for this error)                    */
            if ((reply_elem->scsi_status == SC_CHECK_CONDITION) && 
                (scsi->dev[dev_index]->dev_queuing)) {
                /* set cc_error_state to expect error recovery cmd */
                scsi->dev[dev_index]->cc_error_state++;
                /* set the qstate for this device to indicat contingent */
                /* allegiance condition exists.                         */
                scsi->dev[dev_index]->qstate |= CAC_ERROR;
                /* set the queue suspended flag to false so that an abort is */
                /* not issued to clear the queue for this device.  This also */
                /* will cause the cmd to be iodoned.                         */
                queue_suspended = FALSE;
                /* set adap_q_status in the sc_buf to indicate contingent    */
                /* allegance condition exists for this device.               */
                cmd->bp->adap_q_status = SC_DID_NOT_CLEAR_Q;
            }

            cmd->bp->status_validity = SC_SCSI_ERROR;
            cmd->bp->scsi_status = reply_elem->scsi_status;
            cmd->bp->bufstruct.b_resid = reply_elem->resid_count;
            cmd->bp->bufstruct.b_error = EIO;
        } /* end if valid scsi_status */

        else {  /* scsi_status byte is zero, so continue to look for error */
            /* next see if cmd_error_code indicates the error */
            if (reply_elem->cmd_error_code != 0) {
                switch (reply_elem->cmd_error_code) {

                case VSC_ABORTED_BY_SYSTEM :
                /* this command was returned as a result of an abort or BDR  */
                /* issued by this device driver.  Set up cmd status to       */
                /* indicate that this command was canceled by the device     */
                /* driver.                                                   */
                    cmd->bp->status_validity = 0;
                    cmd->bp->general_card_status = 0;
                    cmd->bp->bufstruct.b_resid = cmd->bp->bufstruct.b_bcount;
                    cmd->bp->bufstruct.b_error = ENXIO;
                    /* by setting queue_suspended to TRUE and need_abort    */
                    /* to FALSE, this cmd will stay on the active list      */
                    /* without an abort being issued.                       */
                    queue_suspended = TRUE;
                    need_abort = FALSE;
                    break;

                case VSC_ABORTED_BY_RESET :
                /* this command was returned as a result of an adatper reset */
                /* performed during error recovery.  Set up cmd status to    */
                /* indicate that this command was canceled by the device     */
                /* driver.                                                   */
                    cmd->bp->status_validity = 0;
                    cmd->bp->general_card_status = 0;
                    cmd->bp->bufstruct.b_resid = cmd->bp->bufstruct.b_bcount;
                    cmd->bp->bufstruct.b_error = ENXIO;
                    /* by setting queue_suspended to FALSE and need_abort    */
                    /* to FALSE, this cmd will be iodoned now without an     */
                    /* abort being issued.                                   */
                    queue_suspended = FALSE;
                    need_abort = FALSE;
                    break;

                case VSC_ABORTED_BY_ADAP   :
                /* this command was returned due to being aborted by the     */
                /* SCSI adapter.  This error code is also used when the      */
                /* device driver issues an abort to clear commands.  If an   */
                /* abort is pending for this device then handle this code the*/
                /* same as VSC_ABORTED_BY_SYSTEM, else Log an error and set  */
                /* up cmd status to indicate the error.                      */
                    if (!(scsi->dev[dev_index]->qstate & HALTED))  {
                        vsc_log_err(scsi, ERRID_SCSI_ERR10, 10, cmd, 0);
                        cmd->bp->status_validity = SC_ADAPTER_ERROR;
                        cmd->bp->general_card_status = SC_SCSI_BUS_FAULT;
                        cmd->bp->bufstruct.b_resid = reply_elem->resid_count;
                        cmd->bp->bufstruct.b_error = EIO;
                    }
                    else { /* abort is pending so this is a device driver */
                           /* initiated cancel */
                        cmd->bp->status_validity = 0;
                        cmd->bp->general_card_status = 0;
                        cmd->bp->bufstruct.b_resid = reply_elem->resid_count;
                        cmd->bp->bufstruct.b_error = ENXIO;
                        /* by setting queue_suspended to TRUE and need_abort */
                        /* to FALSE, this cmd will stay on the active list   */
                        /* without an abort being issued.                    */
                        queue_suspended = TRUE;
                        need_abort = FALSE;
                    }
                    break;

                case VSC_DMA_ERROR :
                    vsc_log_err(scsi, ERRID_SCSI_ERR2, 11, cmd, DMA_ERROR);
                    cmd->bp->status_validity = SC_ADAPTER_ERROR;
                    cmd->bp->general_card_status = SC_HOST_IO_BUS_ERR;
                    cmd->bp->bufstruct.b_resid = reply_elem->resid_count;
                    cmd->bp->bufstruct.b_error = EIO;
                    break;

                default :
                /* all other values of cmd_error_code are treated as unknown */
                /* adapter errors.  Log an error and set up cmd status to    */
                /* indicate the error.                                       */
                    vsc_log_err(scsi, ERRID_SCSI_ERR3, 12, cmd, 0);
                    cmd->bp->status_validity = SC_ADAPTER_ERROR;
                    cmd->bp->general_card_status = SC_ADAPTER_SFW_FAILURE;
                    cmd->bp->bufstruct.b_resid = cmd->bp->bufstruct.b_bcount;
                    cmd->bp->bufstruct.b_error = EIO;
                    break;
                } /* end switch */
            }
            else { 
            /* device error code indicates error for this command */
                switch (reply_elem->device_error_code) {
               
                case VSC_CANCELED_BY_RESET :
                /* this command failed due to a SCSI bus reset.  Recovery  */
                /* for the SCSI bus reset will be handled from the async   */
                /* notification the adapter device driver will issue. Here */
                /* just set up cmd status to indicate the error.           */
                    cmd->bp->status_validity = SC_ADAPTER_ERROR;
                    cmd->bp->general_card_status = SC_SCSI_BUS_RESET;
                    cmd->bp->bufstruct.b_resid = reply_elem->resid_count;
                    cmd->bp->bufstruct.b_error = EIO;
                    /* by setting queue_suspended to FALSE and need_abort    */
                    /* to FALSE, this cmd will be iodoned now without an     */
                    /* abort being issued.                                   */
                    queue_suspended = FALSE;
                    need_abort = FALSE;
                    break;

                case VSC_SCSI_FAULT :
                    /* fall thru logic to next case */

                case VSC_BUS_FREE :
                    /* fall thru logic to next case */

                case VSC_INVALID_PHASE :
                    /* fall thru logic to next case */

                case VSC_MESSAGE_REJECTED :
                    vsc_log_err(scsi, ERRID_SCSI_ERR10, 13, cmd, 0);
                    cmd->bp->status_validity = SC_ADAPTER_ERROR;
                    cmd->bp->general_card_status = SC_SCSI_BUS_FAULT;
                    cmd->bp->bufstruct.b_resid = reply_elem->resid_count;
                    cmd->bp->bufstruct.b_error = EIO;
                    break;
                case VSC_I_TERM_PWR :
                /* the following errors are terminator power loss or        */
                /* differential sense errors.  They are handled the same.   */
                /* The adapter device driver will also send async           */
                /* notification for these types of errors.                  */

                    /* fall thru logic to next case */

                case VSC_E_TERM_PWR :
                    /* fall thru logic to next case */

                case VSC_I_DIFF_SENSE :
                    /* fall thru logic to next case */

                case VSC_E_DIFF_SENSE :
                    vsc_log_err(scsi, ERRID_SCSI_ERR10, 14, cmd, 0);
                    cmd->bp->status_validity = SC_ADAPTER_ERROR;
                    cmd->bp->general_card_status = SC_FUSE_OR_TERMINAL_PWR;
                    cmd->bp->bufstruct.b_resid = reply_elem->resid_count;
                    cmd->bp->bufstruct.b_error = EIO;
                    break;
 
                case VSC_SEL_TIMEOUT :
                /* this command failed because the target device could not */
                /* be selected (selection timeout).                        */
                    cmd->bp->status_validity = SC_ADAPTER_ERROR;
                    cmd->bp->general_card_status = SC_NO_DEVICE_RESPONSE;
                    cmd->bp->bufstruct.b_resid = reply_elem->resid_count;
                    cmd->bp->bufstruct.b_error = EIO;
                    break;

                default :
                /* all other values of cmd_error_code are treated as unknown */
                /* adapter errors.  Log an error and set up cmd status to    */
                /* indicate the error.                                       */
                    vsc_log_err(scsi, ERRID_SCSI_ERR3, 15, cmd, 0);
                    cmd->bp->status_validity = SC_ADAPTER_ERROR;
                    cmd->bp->general_card_status = SC_ADAPTER_SFW_FAILURE;
                    cmd->bp->bufstruct.b_resid = cmd->bp->bufstruct.b_bcount;
                    cmd->bp->bufstruct.b_error = EIO;
                    break;
                } /* end switch */

            } /* end else device error code indicates error */

        } /* end else scsi_status byte == 0 */

        /* if the queue was not suspended for this error, then remove cmd */
        /* from the active list and iodone it */
        if (!(queue_suspended)) {
            /* decrement count ouf active commands for this device */
            scsi->dev[dev_index]->num_act_cmds--;
            tag = cmd->tag;
            cmd->cmd_state = INACTIVE;

	    /* free the gwrite struct if it was a gathered write */
	    if (cmd->bp->resvd1)
	    {
		gw_ptr = (struct gwrite *) cmd->bp->resvd1;
		/* put the gwrite struct on tail of gwrite free list */
		if (scsi->head_gw_free == NULL)
		    scsi->head_gw_free = gw_ptr;
		else
		    scsi->tail_gw_free->next = gw_ptr;
		scsi->tail_gw_free = gw_ptr;
		gw_ptr->next = NULL;	/* mark new end of chain */
		cmd->bp->resvd1 = NULL;
	    }

            VSC_FREETAG(cmd->scsi->free_cmd_list[(int)
                        ((tag)/VSC_BITS_PER_WORD)], tag);
            vsc_deq_active(scsi, cmd->bp, dev_index);
            cmd->bp->bufstruct.b_flags |= B_ERROR;  /* an error occurred */

            iodone((struct buf *) cmd->bp);

            if (scsi->dev[dev_index]->head_act == NULL) {
                if (scsi->dev[dev_index]->qstate & Q_STARVE) {
                    need_to_start_dev = TRUE;
                    scsi->dev[dev_index]->qstate &= (~Q_STARVE);
                }
            }
            else {   /* active list not empty */
                /* decrement the preempt count of the command at the head */
                cmd = (struct cmd_elem *) 
                      scsi->dev[dev_index]->head_act->bufstruct.b_work;
                cmd->preempt--;
                if (cmd->preempt == 0) {
                    scsi->dev[dev_index]->qstate |= Q_STARVE;
                }
            } /* end else active list not empty */   
     
            /* see if any devices are waiting on the cmd_elem which was */
            /* just freed */
            if ((scsi->any_waiting) || (need_to_start_dev)) {
                 vsc_start(scsi, dev_index);
            }
            /* see if an SCIOSTOP is sleeping on this command completion */
            if (scsi->dev[dev_index]->stop_pending) { 
                e_wakeup(&scsi->dev[dev_index]->dev_event);
            }

        }
        else {   /* the queue was suspended for this error, so issue an     */
                 /* abort to clear the queue for this device at the adapter */
                 /* completion of the abort will cause the command queue for*/
                 /* this device to be cleared.                              */
            /* if an abort, BDR, or resume not in progress and flag         */
            /* indicates that one is required then send one to the device   */
            if ((!(scsi->dev[dev_index]->qstate & HALTED)) && (need_abort)
                 && (!(scsi->dev[dev_index]->qstate & RESET_IN_PROG))) {
                /* indicate an abort in progress */
                scsi->dev[dev_index]->qstate |= HALTED;
                /* build the abort control element */
                scsi->dev[dev_index]->command_element.request.scsi_cdb.scsi_id
                             = SID(dev_index);
                scsi->dev[dev_index]->command_element.request.scsi_cdb.scsi_lun
                             = LUN(dev_index);
                scsi->dev[dev_index]->command_element.request.scsi_cdb.
                             media_flags |= (VSC_ABORT | VSC_RESUME);
                scsi->dev[dev_index]->command_element.cmd_state =
                                                   IS_ACTIVE;

                /* if not queuing commands to the device, then this abort */
                /* is only needed to clear the queue of commands at the   */
                /* adapter.  The abort message should not be sent to the  */
                /* device since this could clear sense data if the error  */
                /* is a SCSI check condition.                             */
                if (!(scsi->dev[dev_index]->dev_queuing)) {
                    scsi->dev[dev_index]->command_element.request.scsi_cdb.
                                 media_flags |= (VSC_ADAP_ONLY);
                }

                scsi->dev[dev_index]->command_element.cmd_type  = 
                                                       INTR_LVL_CANCEL;
                /* use the watchdog timer for this device to time the abort */
                /* stop the timer and restart it with the timeout value for */
                /* the abort.  It is ok that active commands are no longer  */
                /* being timed because completion or a time out of the abort*/
                /* will cause the device's active queue to be cleared.      */
                w_stop(&scsi->dev[dev_index]->wdog.dog);
                scsi->dev[dev_index]->wdog.timer_id = SCSI_CANCEL_TMR;
                scsi->dev[dev_index]->wdog.dog.restart = RESET_CMD_T_O;
                w_start(&scsi->dev[dev_index]->wdog.dog);
#ifdef VSC_TRACE
                vsc_internal_trace(scsi, dev_index, 
                                   (uint *) &scsi->dev[dev_index]->command_element,
				   2, 0);
#endif /* VSC_TRACE */
                /* issue the abort to the adapter drivers output function */
                /* note : if output call failed, it will be treated as a  */
                /* command timeout and recovered as such                  */
                (void) (scsi->shared->ndd->ndd_output) (scsi->shared->ndd,
                      &scsi->dev[dev_index]->command_element.ctl_elem);
            }  /* end if !(qstate & HALTED) */
        } /* end else queue was suspended */

    }  /* end else error completion */
    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_INTR, 0, scsi->devno);
    return;

}  /* end vsc_recv */

/*
 * NAME:        vsc_log_err
 *
 * FUNCTION:    Places an entry in the system error log
 *              This function is called whenever an event is detected for 
 *              which it is appropriate to place information in the system
 *              error log for this device.  This function is the common point
 *              through which all error log entries for virtual SCSI devices
 *              originate.
 *
 * EXECUTION ENVIRONMENT:
 *              this routine can be called on the process or interrupt level
 *
 * INPUT:
 *              scsi  - pointer to the scsi_info struct corresponding to the
 *                      scsi bus which has been reset.
 *
 * RETURN VALUE DESCRIPTION:  none
 *
 * EXTERNAL PROCEDURES CALLED:
 *              bcopy                 errsave
 *
 */
void
vsc_log_err(
    struct scsi_info *scsi, 
    int errid,
    int errnum,
    struct cmd_elem *cmd,
    int ahs)
{
    struct error_log_def log;

    bzero((char *) &log, sizeof(struct error_log_def)); /* init the logging
                                                           struct */
    log.errhead.error_id = (uint) errid;
    bcopy((char *) &scsi->ddi.resource_name[0],
	  (char *) &log.errhead.resource_name[0],
          ERR_NAMESIZE);
    log.data.diag_validity = 0;
    log.data.diag_stat = 0;
    if (ahs != 0) {
        log.data.ahs_validity = 0x01;
        log.data.ahs = (uchar) ahs;
    }
    log.data.un.card4.errnum = (uint) errnum;
    log.data.un.card4.scsi_struct = (uint) scsi;
    /* copy the SCSI command descriptor block to the error log */
    if (cmd != NULL) {
        bcopy((char *) &cmd->request.scsi_cdb, 
	      (char *) &log.data.un.card4.cdb[0], 36);
        /* if this is not a command timeout, then write the SCSI status byte */
        /* command error code, and device error code the the error log.      */
        if (ahs != COMMAND_TIMEOUT) {
            log.data.un.card4.scsi_stat = cmd->reply.scsi_status;
            log.data.un.card4.cmd_err = cmd->reply.cmd_error_code;
            log.data.un.card4.dev_err = cmd->reply.device_error_code;
        }

    } /* end if cmd != NULL */

    /* log the error here */
    errsave(&log, sizeof(struct error_log_def));

} /* end vsc_log_err */

/*
 * NAME:        vsc_process_scsi_reset
 *
 * FUNCTION:    Process notification of a SCSI bus reset.
 *              This function is called when the device driver is notified of
 *              completion of a SCSI bus reset.  The command delay after 
 *              reset timer is started and the command queue for each open
 *              device is cleared if it had commands active during the reset.
 *              If a device had no commands active during the reset, then
 *              vsc_start is called to issue any commands which may have
 *              been pending during the SCSI bus reset.
 *
 * EXECUTION ENVIRONMENT:
 *              this routine can only be called on the interrupt level
 *
 * EXTERNAL PROCEDURES CALLED:
 *              w_stop                   w_start
 *
 * INPUT:
 *              scsi  - pointer to the scsi_info struct corresponding to the
 *                      scsi bus which has been reset. 
 *
 * RETURN VALUE DESCRIPTION:  none
 *
 */
void
vsc_process_scsi_reset(
    struct scsi_info * scsi)
{
    int i,                  /* for loop variable to index all devices        */
    need_abort;             /* flag to indicate if abort is needed to clear  */
                            /* an active list for a device.                  */
    struct sc_buf *t_ptr;   /* a temp sc_buf pointer to walk the active list */
                            /* of commands for a device.                     */
    struct cmd_elem *t_cmd; /* a temp cmd_elem pointer to walk the active    */                             /* list of commands for a device.                */
    
    /* loop through and find all affected devices */
    for (i = 0; i < DEVPOINTERS; i++) {
        if (scsi->dev[i] != NULL) {
        /* if this device is open then indicate a CDAR_ACTIVE */
            scsi->dev[i]->qstate |= CDAR_ACTIVE;
            /* walk the active list for this device to see if interrupts have*/
            /* been received for all active commands. If there are still cmds*/
            /* oustanding for any device, then issue an abort to clear the   */
            /* queue for that device. */
            need_abort = FALSE;
            t_ptr = scsi->dev[i]->head_act;
            while (t_ptr != NULL) {
                t_cmd = (struct cmd_elem *)t_ptr->bufstruct.b_work;
                if (t_cmd->cmd_state != INTERRUPT_RECVD) {
                    need_abort = TRUE;
                    break;
                }
                t_ptr = (struct sc_buf *)t_ptr->bufstruct.av_forw;
            } /* end while */
            /* if an abort is necessary and the cancel cmd_elem for this  */
            /* device is not currently active, then issue abort to clear  */
            /* the active queue for this device.                          */
            if ((need_abort) && (!(scsi->dev[i]->qstate & HALTED))) { 
                scsi->dev[i]->qstate |= HALTED;
                /* build the abort control element */
                scsi->dev[i]->command_element.request.scsi_cdb.scsi_id
                             = SID(i);
                scsi->dev[i]->command_element.request.scsi_cdb.scsi_lun
                             = LUN(i);
                scsi->dev[i]->command_element.request.scsi_cdb.
                             media_flags |= 
                                     (VSC_ADAP_ONLY | VSC_ABORT | VSC_RESUME);
                scsi->dev[i]->command_element.cmd_type  = 
                                                       INTR_LVL_CANCEL;
                /* use the watchdog timer for this device to time the abort */
                /* stop the timer and restart it with the timeout value for */
                /* the abort.  It is ok that active commands are no longer  */
                /* being timed because completion or a time out of the abort*/
                /* will cause the device's active queue to be cleared.      */
                w_stop(&scsi->dev[i]->wdog.dog);
                scsi->dev[i]->wdog.timer_id = SCSI_CANCEL_TMR;
                scsi->dev[i]->wdog.dog.restart = RESET_CMD_T_O;
                w_start(&scsi->dev[i]->wdog.dog);
#ifdef VSC_TRACE
                vsc_internal_trace(scsi, i,
                                   (uint *) &scsi->dev[i]->command_element,
				   2, 0);
#endif /* VSC_TRACE */
                /* issue the abort to the adapter drivers output function */
                /* note : if output call failed, it will be treated as a  */
                /* command timeout and recovered as such                  */
                (void) (scsi->shared->ndd->ndd_output) (scsi->shared->ndd,
                      &scsi->dev[i]->command_element.ctl_elem);

            } /* end if abort needed */

            /* if abort was not needed call vsc_fail_cmd to clear the     */
            /* queue for this device */
            if (!(need_abort)) {
                vsc_fail_cmd(scsi, i);
            }

        } /* end if device open */
    } /* end for */
    /* initiate command delay after reset timer */
    w_start(&scsi->cdar_wdog.dog);

    return; 

} /* end vsc_process_scsi_reset */

/*                      
 * NAME:        vsc_watchdog
 *                     
 * FUNCTION:    Device watchdog timer handler 
 *		this function is called to process timeouts of either stratey
 *              commands (i/o from the head device drivers) or internally
 *              initiated commands (SCSI bus error recovery).  The command
 *              delay after reset timer is also handled in this function.
 *                                                        
 * EXECUTION ENVIRONMENT:                                
 *		this routine is called on the interrupt 
 *		level (INTTIMER), therefore, it can be 
 *		interrupted by our interrupt handler.                    
 *                                                              
 *                                                                   
 * INPUT:                                                          
 *      	wdog     - pointer to watchdog timer structure which timed-out
 *                                                                   
 * RETURN VALUE DESCRIPTION:  none                                  
 *                                                                 
 * EXTERNAL PROCEDURES CALLED:                                    
 *      disable_lock    unlock_enable                                 
 *      e_wakeup        w_start                                           
 *                                                             
 */ 
void
vsc_watchdog(
             struct watchdog * wdog)
{
    struct wtimer *w_timer;        /* pointer to vsc timer type structure   */
    struct scsi_info *scsi;        /* pointer to scsi_info struct for timer */
    struct cmd_elem *cmd;          /* pointer to a cmd_elem on active list  */
    struct sc_buf *sc_buf,         /* pointer to sc_buf on head active list */
                 *tsc_buf;         /* pointer to walk entire active list    */
    int i,                         /* for loop variable                     */
        rc,                        /* locally used return code              */
        dev_index,                 /* index into array of dev_info structs  */
                                   /* for this device */
        t_index,                   /* dev_index used when walking array of  */
                                   /* dev_info structs */
        old_pri,                   /* returned from disable_lock            */
        base_time;                 /* save the largest timeout value on the */
                                   /* active list; used to detect real t_o  */


    /* get the entire wtimer structure from the passed in watchdog */
    w_timer = (struct wtimer *) wdog;
    scsi = w_timer->scsi; /* get the pointer to the scsi_info for this timer */
    /* serialize with intrpts */
    old_pri = disable_lock(scsi->ddi.intr_priority, 
                           &scsi->shared->ndd->ndd_demux_lock);
    /* switch on the type of timer which expired */
    switch(w_timer->timer_id) {
        case SCSI_CDAR_TMR  :      /* command delay after reset timer */
            /* this indicates the end of the delay period following a */
            /* scsi reset or BDR message--restart delayed devices     */

            /* loop through and find all affected devices */
            for (i = 0; i < DEVPOINTERS; i++) {
                if ((scsi->dev[i] != NULL)&&
                    (scsi->dev[i]->qstate & CDAR_ACTIVE)) {  
                /* if this device is open, and was affected by the cdar */
                /* then clear the cdar indication and resume the queues */
                    scsi->dev[i]->qstate &= ~(CDAR_ACTIVE);
                    vsc_start(scsi, i);
                }
            } /* end for */

            break;

        case SCSI_DEV_TMR   :   /* normal device i/o timer */
            /* caller(device heads) is responsible for error logging timeouts*/

            /* get the device index for this device timer  */
            dev_index = w_timer->index; 

            /* validate device is open */
            if (scsi->dev[dev_index] == NULL) {
                /* device not opened, log an error and return */
                vsc_log_err(scsi, ERRID_SCSI_ERR6, 16, NULL, 0);
                unlock_enable(old_pri, &scsi->shared->ndd->ndd_demux_lock);
                return;
            }

            sc_buf = scsi->dev[dev_index]->head_act;
            if ((sc_buf == NULL) || 
                (sc_buf->bufstruct.b_work == (int)NULL)) {
            /* the timer expired but there are no active cmds or there is */
            /* no cmd_elem with the head of the active list; this is a    */
            /* software programming error and should be logged as such    */
                vsc_log_err(scsi, ERRID_SCSI_ERR6, 17, NULL, 0);
                unlock_enable(old_pri, &scsi->shared->ndd->ndd_demux_lock);
                return;
            }
            /* check the PENDING_ERROR qstate for this device, if it is  */
            /* set and there is only one cmd on the active list, then    */
            /* this timeout occurred because of a failed ndd_output call */
            /* in vsc_start_pending_cmds.  No need to send BDR, just     */
            /* clear the queue for this device.                          */
            if((scsi->dev[dev_index]->num_act_cmds == 1) &&
               (scsi->dev[dev_index]->qstate & PENDING_ERROR)) {
                vsc_fail_cmd(scsi, dev_index);
                unlock_enable(old_pri, &scsi->shared->ndd->ndd_demux_lock);
                return;
            }
            /* get the cmd_elem for cmd at head of active list */
            cmd = (struct cmd_elem *)sc_buf->bufstruct.b_work;
            if (cmd->cmd_state == IS_ACTIVE) {
            /* this is original cmd timeout */

                /* determine if this is a valid timeout.          */
                /* walk the list of commands on this device's     */
                /* active queue to ensure that no cmd on the list */
                /* has a timeout value greater than that at the   */
                /* head of the list. If a greater value exists,   */
                /* an erroneous timeout may have occurred as a    */
                /* result of the device reordering commands.      */

                base_time = sc_buf->timeout_value;
                tsc_buf = sc_buf;
                while( tsc_buf != NULL ) {
                    if ( tsc_buf->timeout_value > base_time ) {
                        base_time = tsc_buf->timeout_value;
                    }
                    tsc_buf = (struct sc_buf *) 
                                  tsc_buf->bufstruct.av_forw;
                } /* end while */
                 
                /* if a command on the active list has a timeout    */
                /* value greater than that of the head command,     */
                /* then restart the wdog with the difference of     */
                /* of the two timeout values; this was not a true   */
                /* timeout.   save_time is used so the code does    */
                /* restart the timer for the same command more than */
                /* once.  Without using save_time, the timer would  */
                /* again expire and we would find the same command  */
                /* with a timeout value greater than the head and   */
                /* perpetually restart the timer.                   */
                if ((base_time > sc_buf->timeout_value) && (base_time >
                         scsi->dev[dev_index]->wdog.save_time)) {
                    scsi->dev[dev_index]->wdog.dog.restart = 
                        base_time - sc_buf->timeout_value;
                    scsi->dev[dev_index]->wdog.save_time = base_time;
                    w_start(&scsi->dev[dev_index]->wdog.dog);
                    unlock_enable(old_pri, &scsi->shared->ndd->ndd_demux_lock);
                    return;
                }
                else {  /* a real timeout has been detected */
                    /* reset save_time */
                    scsi->dev[dev_index]->wdog.save_time = 0;  
                }

                /* set up sc_buf status for sc_buf at head of active  */
                /* list in prepartion for iodone call                 */
                sc_buf->status_validity = SC_ADAPTER_ERROR;
                sc_buf->general_card_status = SC_CMD_TIMEOUT;
                sc_buf->scsi_status = SC_GOOD_STATUS;
                sc_buf->bufstruct.b_resid = sc_buf->bufstruct.b_bcount;
                sc_buf->bufstruct.b_error = EIO;

            } /* end if cmd_state == IS_ACTIVE */

            else { /* cmd_state != IS_ACTIVE; log an error and issue BDR */
                   /* to clear the queue for this device.                */
                   vsc_log_err(scsi, ERRID_SCSI_ERR10, 18, cmd, 
                               COMMAND_TIMEOUT );
                   scsi->dev[dev_index]->wdog.save_time = 0;  
            }

            /* set cmd_state for all cmd_elems on active list to */
            /* WAIT_FOR_TO_2 so that they are processed properly */
            /* if an interrupt is receieved.                     */
            tsc_buf = sc_buf;
            while (tsc_buf != NULL) {
                cmd = (struct cmd_elem *)tsc_buf->bufstruct.b_work;
                if(cmd->cmd_state == IS_ACTIVE) {
                    cmd->cmd_state = WAIT_FOR_TO_2;
                }
                tsc_buf = (struct sc_buf *) 
                              tsc_buf->bufstruct.av_forw;
            } /* end while */
            /* issue a BDR to this id to recover from the command */
            /* timeout.  Upon completion of the BDR the queue for */
            /* this device will be cleared back to the head dd.   */

            /* if device queue is halted, then an abort or bdr is */
            /* currently pending  */
            if ((scsi->dev[dev_index]->qstate & HALTED) ||
                (scsi->dev[dev_index]->qstate & RESET_IN_PROG)) {

                unlock_enable(old_pri, &scsi->shared->ndd->ndd_demux_lock);
                return;
            }
            /* abort or BDR is not pending so issue BDR  */
            /* a BDR will affect all luns at the specified SCSI id. */
            /* Mark all such device queues as halted to prevent     */
            /* commands from being sent during BDR.                 */
            for (i = 0; i < MAX_LUNS; i++) {
                t_index = INDEX(SID(dev_index), i);
                /* if device is open */
                if (scsi->dev[t_index]!= NULL) {
                   scsi->dev[t_index]->qstate |= HALTED ;
                } 
            } /* end for */

            /* build the abort control element */
            scsi->dev[dev_index]->command_element.request.
                  scsi_cdb.scsi_id = SID(dev_index);
            scsi->dev[dev_index]->command_element.request.
                  scsi_cdb.scsi_lun = LUN(dev_index);
            scsi->dev[dev_index]->command_element.request.
                  scsi_cdb.media_flags |= (VSC_BDR | VSC_RESUME);
            scsi->dev[dev_index]->command_element.cmd_state = 
                                                   IS_ACTIVE;
            scsi->dev[dev_index]->command_element.cmd_type  = 
                                                       INTR_LVL_CANCEL;
            /* start timer for this command */
            /* use the watchdog timer for this device to time the */
            /* abort. Stop the timer and restart it with the      */
            /* timeout value for the abort.                       */
            scsi->dev[dev_index]->wdog.timer_id = SCSI_CANCEL_TMR;
            scsi->dev[dev_index]->wdog.dog.restart = RESET_CMD_T_O;
            w_start(&scsi->dev[dev_index]->wdog.dog);
#ifdef VSC_TRACE
            vsc_internal_trace(scsi, dev_index, 
                               (uint *) &scsi->dev[dev_index]->command_element,
			       2, 0);
#endif /* VSC_TRACE */

            /* issue the command to the adapter drivers output */
            rc = (scsi->shared->ndd->ndd_output) (scsi->shared->ndd,
                     &scsi->dev[dev_index]->command_element.ctl_elem);

            if (rc) {  /* call to output returned error */
             /* ignore the error here and allow BDR to timeout */
            }

            break;
        case SCSI_CANCEL_TMR :   /* abort, BDR, or resume timer */
            /* get the device index for this device timer  */
            dev_index = w_timer->index; 
            /* update the cmd_state to handle interrupt properly if it occurs*/
            scsi->dev[dev_index]->command_element.cmd_state = WAIT_FOR_TO_2;
            /* convert the timer back to a SCSI_DEV_TMR */
            scsi->dev[dev_index]->wdog.timer_id = SCSI_DEV_TMR;

            /* issue a SCSI bus reset to attempt to recover timed-out cancel */
            vsc_scsi_reset(scsi);
            if (scsi->dev[dev_index]->command_element.cmd_type == 
                PROC_LVL_CANCEL) {
                /* log an error to indicate proc lvl cancel timed out */
                vsc_log_err(scsi, ERRID_SCSI_ERR10, 19, 
                            &scsi->dev[dev_index]->command_element, 
                            COMMAND_TIMEOUT);
                scsi->proc_results = ETIMEDOUT;
                e_wakeup(&scsi->ioctl_event);
            }
            else {    /* INTR_LVL_CANCEL */
                /* log an error to indicate an intr lvl cancel timed out */
                vsc_log_err(scsi, ERRID_SCSI_ERR10, 20,
                            &scsi->dev[dev_index]->command_element, 
                            COMMAND_TIMEOUT);
            }
        
            break;
        case SCSI_RESET_TMR :   /* a SCSI bus reset cmd_elem timer */
            /* update the cmd_state to handle interrupt properly if it occurs*/
            scsi->reset_cmd_elem.cmd_state = WAIT_FOR_TO_2;
            
            /* log an error to record SCSI bus reset timeout */
            vsc_log_err(scsi, ERRID_SCSI_ERR10, 21,
                        &scsi->reset_cmd_elem, COMMAND_TIMEOUT);
           /* if a SCSI bus reset cmd_elem times out, then the error         */
           /* recovery is to issue a hardware reset to the adapter.          */
           (void) (scsi->shared->ndd->ndd_ctl) 
                           (scsi->shared->ndd, NDD_RESET_ADP, NULL);
           break;
        default             :   /* programming error */
            ASSERT(0);
            /* log a software programming error */
            vsc_log_err(scsi, ERRID_SCSI_ERR6, 22, NULL, 0);
            break;
    } /* end switch timer_id */

    unlock_enable(old_pri, &scsi->shared->ndd->ndd_demux_lock);
    return;

} /* end vsc_watchdog */

/*
 * NAME:        vsc_scsi_reset
 *
 * FUNCTION:    This function issues a SCSI bus reset cmd_elem to the adapter 
 *              device driver if there is not already a SCSI bus reset in
 *              progress. 
 *
 * EXECUTION ENVIRONMENT:
 *              This routine can only be called on priority levels greater
 *              than, or equal to that of the interrupt handler.
 *
 * DATA STRUCTURES:
 *
 * INPUT:
 *              scsi  - pointer to the scsi_info struct corresponding to the
 *                      scsi bus to be reset.
 *
 * RETURN VALUE DESCRIPTION:
 *              NONE
 *
 * EXTERNAL PROCEDURES CALLED:
 *              w_start
 *
 */
void
vsc_scsi_reset(
    struct scsi_info *scsi)
{
    int t_index;                /* for loop variable for walking all devices */

    /* if a reset is not currently in progress then issue the reset_cmd_elem */
    if (scsi->reset_cmd_elem.cmd_state == INACTIVE) { 
        /* mark all open devices to indicate that a reset is in progress */
        for (t_index = 0; t_index < DEVPOINTERS; t_index++) {
            /* if device is open */
            if (scsi->dev[t_index]!= NULL) {
                scsi->dev[t_index]->qstate |= RESET_IN_PROG;
            } 
        } /* end for */
        /* set the cmd_state to indicate reset is active */
        scsi->reset_cmd_elem.cmd_state = IS_ACTIVE;
        /* cmd_type was set during the cmd_elem initialization in open;   */
        /* this value will never change because this cmd_elem is dedicated*/
        /* to sending interrupt level SCSI bus resets.                    */

        /* start the timer for the SCSI bus reset */
        w_start(&scsi->reset_wdog.dog);
#ifdef VSC_TRACE
        vsc_internal_trace(scsi, -1, (uint *) &scsi->reset_cmd_elem, 4, 0);
#endif /* VSC_TRACE */
        /* issue the reset command to the adapter drivers output function */
        /* ignore the return code from ndd_output, allowing cmd_elem to   */
        /* timeout if call to ndd_output fails.                           */
        (void) (scsi->shared->ndd->ndd_output) (scsi->shared->ndd, 
                          &scsi->reset_cmd_elem.ctl_elem);

    } /* end if cmd_state == INACTIVE */
    /* else reset is already in progress so nothing to do */

    return;

} /* end vsc_scsi_reset */


/*
 * NAME:        vsc_async_stat
 *
 * FUNCTION:    virtual SCSI bus asychronous status receiving function.  This
 *              function receives async status from the adapter device driver.
 *              During vsc_open, the ns_add_status function is called to 
 *              register this function with the adapter device driver.
 *              
 * EXECUTION ENVIRONMENT:
 *              this routine can be  called on the interrupt or process level
 *
 *
 * INPUT:
 *               p_ndd   - parent adapter's ndd structure
 *               statblk - pointer to the status block struct containing the
 *               async status.
 *
 * RETURN VALUE DESCRIPTION:
 *              NONE
 *
 * EXTERNAL PROCEDURES CALLED:
 *              e_wakeup
 *
 */
void
vsc_async_stat(
    ndd_t	*p_ndd,
    ndd_statblk_t  *statblk)
{
    struct scsi_info *scsi;              /* pointer to the scsi_info struct  */
                                         /* corresponding to this async event*/
                                         /* this is a correlator returned by */
                                         /* the adapter driver in option[1]  */
                                         /* of the statblk structure.        */

    /* get the correlator from option[1] of the statblk struct */
    scsi = (struct scsi_info *)statblk->option[1];
    ASSERT(scsi);

    switch (statblk->code) {

    case NDD_STATUS :

        switch (statblk->option[0]) {
   
        case NDD_SCSI_BUS_RESET :
            /* log an error to indicate a SCSI bus reset, not initiated by */
            /* this device driver, has occurred. */
            vsc_log_err(scsi, ERRID_SCSI_ERR10, 257, NULL, 0);
            /* notify registered users of the SCSI bus reset event */
            vsc_async_notify(scsi, SC_SCSI_RESET_EVENT);
            /* complete processing of SCSI bus reset completion */
            vsc_process_scsi_reset(scsi);
            break;

        case NDD_DEV_RESELECT :
            /* this indicates that a device reselected the adapter when the */
            /* adapter had no outstanding command to the device.  Log an    */
            /* error to record this event.                                  */
            vsc_log_err(scsi, ERRID_SCSI_ERR10, 24, NULL, 0);
            break;
 
        case NDD_TERM_POWER_LOSS :
            /* the adapter has detected a loss of terminator power on the  */
            /* SCSI bus.  Log an error to record this event.               */
            vsc_log_err(scsi, ERRID_SCSI_ERR10, 25, NULL, 0);
            break;

        case NDD_DIFFERENTIAL_SENSE :
            /* the adapter has detected a differential sense error. Log an */
            /* error to record this event.                                 */
            vsc_log_err(scsi, ERRID_SCSI_ERR10, 26, NULL, 0);
            break;

        default :
            /* unknown status is returned by the adapter driver.  Log an   */
            /* error to record this event.                                 */
            vsc_log_err(scsi, ERRID_SCSI_ERR6, 27, NULL, 0);
            break;
        } /* end switch statblk->options[0] */
  
        break;

    case NDD_CONNECTED :
        /* async status of an open completing */
        e_wakeup(&scsi->open_event);
    break;

    default :
    break;

    } /* end switch statblk->code */

    return;

} /* end vsc_async_stat */

/*
 * NAME:        vsc_cdt_func
 *
 * FUNCTION:    virtual SCSI bus component dump table function.
 *              This function builds the driver dump table during a system dump
 *
 * EXECUTION ENVIRONMENT:
 *              this routine is called on the interrupt level 
 *
 * DATA STRUCTURES:
 *
 * INPUT:
 *              arg     -  1 dump dd is starting to get dump table entries
 *                         2 dump dd is finished getting the dump table entries
 *
 * RETURN VALUE DESCRIPTION:
 *              Return code is a pointer to the struct cdt to be dumped for
 *              this component.
 *
 * EXTERNAL PROCEDURES CALLED:
 *
 */
struct cdt *
vsc_cdt_func(
            int    arg) 
{
    int size,                    /* used to compute size of this cdt entry   */
        i,                       /* for loop variable to walk vsc_scsi_ptrs  */
        d_index;                 /* index into array of cdt_entry structs    */
    struct scsi_info *scsi;      /* ptr to scsi_info struct in vsc_scsi_ptrs */

    if (arg == 1) {
        /* only build the dump table on the initial dump call */
        
        /* init the table */
        bzero((char *) vsc_cdt, sizeof(struct vsc_cdt_tab));
      
        /* init the head structure */
        vsc_cdt->vsc_cdt_head._cdt_magic = DMP_MAGIC;
        strcpy(vsc_cdt->vsc_cdt_head._cdt_name, "vscsi");
        /* _cdt_len is filled in below */

        size = sizeof(struct cdt_head);

        /* now begin filling in elements */
        d_index = 0;
        /* loop through scsi_info pointers */
        for (i = 0; i < (MAX_ADAPTERS * 2); i++) {
        
            /* skip over unused slots in scsi_info table */
            if (vsc_scsi_ptrs[i] == NULL) {
                continue;
            }
            else {
                scsi = vsc_scsi_ptrs[i];
                /* follow the chain to find collisions in the hash table */
                while (scsi != NULL) {
                    vsc_cdt->vsc_entry[d_index].d_segval = 0x0;
                    /* copy name to element */
                    bcopy((char *) scsi->ddi.resource_name,
                          (char *) vsc_cdt->vsc_entry[d_index].d_name,
                           8);
                    vsc_cdt->vsc_entry[d_index].d_ptr = (char *) scsi;
                    vsc_cdt->vsc_entry[d_index].d_len = 
                         (sizeof(struct scsi_info));
                    scsi = scsi->next;
                    d_index++;
                    size += sizeof(struct cdt_entry);
                }   /* end while */
 
            } /* end else */
        } /* end for */
        /* fill in the actual table size */
        vsc_cdt->vsc_cdt_head._cdt_len = size;

    } /* end else */ 

    return((struct cdt *) vsc_cdt);

} /* end vsc_cdt_func */



/**************************************************************************/
/*                                                                        */
/* NAME: vsc_dump                                                         */
/*                                                                        */
/* FUNCTION: Virtual SCSI dump entry point.                               */
/*                                                                        */
/*      This driver is the interface between the virtual SCSI device      */
/*      driver and the SCSI adapter driver for handling requests for      */
/*      dumping data.                                                     */
/*                                                                        */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine is called when there is certain to be limited        */
/*      functionality available in the system.  However, system           */
/*      dma kernel services are available.  The passed data is already    */
/*      pinned by the caller.  There are no interrupt or timer kernel     */
/*      services available.  This routine should run at INTMAX level.     */
/*                                                                        */
/*  NOTES:  This routine handles the following operations :               */
/*      DUMPINIT   - initializes SCSI device as primary dump device.	  */
/*      DUMPSTART  - prepares device for dump                             */
/*      DUMPQUERY  - returns the maximum and minimum number of bytes that */
/*                   can be transferred in a single DUMPWRITE command     */
/*      DUMPWRITE  - performs write to dump device                        */
/*      DUMPEND    - cleans up device on end of dump                      */
/*      DUMPTERM   - terminates the bus attached disk as dump device      */
/*                                                                        */
/* INPUTS:                                                                */
/*      devno   - device major/minor number                               */
/*      uiop    - pointer to uio structure for data for the               */
/*                specified operation code                                */
/*      cmd     - Type of dump operation being requested                  */
/*      arg     - Pointer to dump query structure or buf_struct for cmd   */
/*      chan    - unused                                                  */
/*      ext     - unused                                                  */
/*                                                                        */
/*									  */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      A zero will be returned on successful completion, otherwise,      */
/*      one of the errno values listed below will be given.               */
/*                                                                        */
/*      EINVAL    -  Invalid operation requested                          */
/*      EIO       -  I/O error, scsi_info not configured, or opened       */
/*                                                                        */
/**************************************************************************/
int
vsc_dump (dev_t devno,       /* device major/minor number */
	  struct uio * uiop, /* uio structure             */
	  int cmd,           /* type of dump command      */
	  int arg,           /* arguments for cmd         */
	  int chan,          /* unused                    */
	  int ext)           /* unused                    */
{
    struct scsi_info *scsi;       /* virtual scsi structure                */
    int     i_hash;               /* index to hash table of scsi_info ptrs */
    int     rc;                   /* local return code                     */
    int     ret_code;             /* function return code                  */

    ret_code = 0; /* default to no error */
    /*
     * Search for scsi_info structure in configured list. Build the hash
     * index. 
     */
    i_hash = minor (devno) & SCSI_HASH;
    scsi = vsc_scsi_ptrs[i_hash];
    while (scsi != NULL)
    {
	if (scsi->devno == devno)
	    break;
	scsi = scsi->next;
    }

    /*
     * Ensure adapter and device are opened and started.
     */
    if ((scsi == NULL) || (!scsi->opened)) {
        ret_code = EIO;
	return (ret_code);
    }

    /* Determine what operation has been requested. */

    switch (cmd) {

    case DUMPINIT:
        /* Obtain the entry point to the adapter's dump routine. */
        rc = (scsi->shared->ndd->ndd_ctl) (scsi->shared->ndd, 
                                           NDD_DUMP_ADDR, &dump_entry_ptr);
        if (rc) {
            ret_code = EIO;
            return(ret_code);
        }

        /* Call the lower level dump routine  */
        rc = (dump_entry_ptr) (scsi->shared->ndd, cmd, arg);

        if (rc) {
            ret_code = EIO;
            return(ret_code);
        }

        scsi->dump_state = DUMP_INIT;
	break;

    case DUMPTERM:
        /* Unitialize this device as the target dump device */
	if (scsi->dump_state != DUMP_INIT) {
            ret_code = EINVAL;
	    return (ret_code);
        }

	/* Call the lower level dump routine */
	rc = (dump_entry_ptr) (scsi->shared->ndd, cmd, arg);

        if (rc) {
            ret_code = EIO;
            return(ret_code);
        }

	scsi->dump_state = DUMP_IDLE;
	break;

    case DUMPQUERY:
        /*
	 * Determine the maximum and minimum number of bytes that can be sent
	 * to the device in one DUMPWRITE command. 
	 *
	 * NOTE: addr of write entry point not necessary for non-network
	 * devices. 
	 */
	((struct dmp_query *) arg)->min_tsize = scsi->shared->ndd->ndd_mintu;
	((struct dmp_query *) arg)->max_tsize = scsi->shared->ndd->ndd_mtu;

	break;

    case DUMPSTART:
	/* Prepare a device for a kernel dump */
	if (scsi->dump_state != DUMP_INIT && scsi->dump_state != DUMP_START) {
            ret_code = EINVAL;
	    return (ret_code);
        }

	/* don't do anything if already done DUMPSTART */
	if (!(scsi->dump_state == DUMP_START)) {
	    /* vsc_dump_start will abort all outstanding commands to prepare */
	    /* for the system dump. */
	    rc = vsc_dump_start(scsi);
	    if (rc) {
		ret_code = EIO;
	    }
	    scsi->dump_state = DUMP_START;
	}
	break;

    case DUMPWRITE:
	if (scsi->dump_state != DUMP_START) {
            ret_code = EINVAL; 
	    return (ret_code);
        }
	rc = vsc_dump_write (scsi, (struct sc_buf *) arg);
        if (rc) {
            ret_code = EIO;
        }
	break;

    case DUMPEND:
	if (scsi->dump_state != DUMP_START) {
            ret_code = EINVAL;
        }
	scsi->dump_state = DUMP_INIT;
	break;

    default :
        ret_code = EINVAL;
	break;
    }

    return (ret_code);

} /* end vsc_dump */

/**************************************************************************/
/*									  */
/* NAME:	vsc_dump_write						  */
/*									  */
/* FUNCTION: 								  */
/*		protocol driver's dump output routine			  */
/*									  */
/* EXECUTION ENVIRONMENT:						  */
/*      This routine is called on the interrupt level.                    */
/*      This routine should run at INTMAX level.                          */
/*									  */
/* NOTES:								  */
/*									  */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*									  */
/* INPUT:								  */
/*              scsi  - pointer to the scsi_info struct on which the dump */
/*                      device resides.                                   */
/*		sc_buf	- sc_buf structure passed by caller		  */
/*									  */
/* RETURN VALUE DESCRIPTION:                                              */
/*      A zero will be returned on successful completion, otherwise,      */
/*      one of the errno values listed below will be given.               */
/*                                                                        */
/*		EIO	- unable to allocate resources or ndd_output      */
/*                        call failed.                                    */
/*									  */
/**************************************************************************/
int
vsc_dump_write (struct scsi_info * scsi,
		struct sc_buf * scp)
{
    int     rc,	                  /* locally used return code               */
            ret_code;             /* return code from this function         */
    struct  cmd_elem *cmd;        /* command element used to send the dump  */
    uchar  tag;                   /* used to free cmd_elem                  */

    ret_code = 0; /* init to no error */

    cmd = vsc_get_cmd_elem(scsi);
    if (cmd == NULL)  { /* could not get resource; all in use */
        ret_code = EIO;
        return(ret_code);
    }

    /* Build a ctl_elem_blk from the sc_buf */

    cmd->bp = scp;  /* set pointer to sc_buf */
    cmd->cmd_state = IS_ACTIVE; /* setup command state */ 
    cmd->preempt = MAX_PREEMPTS; 

    /* mark this command for expedite */
    cmd->ctl_elem.flags = EXPEDITE;

    /* put total data length in pds_data_len field of cntl elem blk     */
    cmd->ctl_elem.pds_data_len = scp->bufstruct.b_bcount;
    
    /* put the SCSI ID of target in the scsi_cdb of the request elem    */
    cmd->request.scsi_cdb.scsi_id = scp->scsi_command.scsi_id;
    
    /* put the SCSI LUN of target in the scsi_cdb of the request elem   */
    if (scp->lun) 
        cmd->request.scsi_cdb.scsi_lun = scp->lun;
    else
        cmd->request.scsi_cdb.scsi_lun = (scp->scsi_command.scsi_cmd.lun >> 5);
    
    /* put the SCSI command block in the scsi_cdb of the request elem   */
    cmd->request.scsi_cdb.scsi_cmd_blk = scp->scsi_command.scsi_cmd;
    
    /* put total data length in scsi_cdb  of request element            */
    cmd->request.scsi_cdb.scsi_data_length =  scp->bufstruct.b_bcount;
    
    /* put total data length in the total_len field of pd_info1 struct */
    cmd->pd_info1.total_len =  scp->bufstruct.b_bcount;
     
    /* set appropriate media flags in scsi_cdb of request element       */
    switch (scp->q_tag_msg) { 
        case SC_NO_Q : 
	  cmd->request.scsi_cdb.media_flags = VSC_NO_Q; 
	  break;
	case SC_SIMPLE_Q  :
	    cmd->request.scsi_cdb.media_flags = VSC_SIMPLE_Q; 
	  break;
	case SC_HEAD_OF_Q :
	    cmd->request.scsi_cdb.media_flags = VSC_HEAD_OF_Q; 
	  break;
	case SC_ORDERED_Q :
	    cmd->request.scsi_cdb.media_flags = VSC_ORDERED_Q; 
	  break;
      }  /* end switch */

    /* put the size of the SCSI CDB in the scsi_cdb of requeset element */
    cmd->request.scsi_cdb.media_flags |= (scp->scsi_command.scsi_length << 24);

    /* set the no disconnect and async media flags according to sc_buf */
    if (scp->scsi_command.flags & SC_NODISC) 
	cmd->request.scsi_cdb.media_flags |= VSC_NO_DISC; 
    if (scp->scsi_command.flags & SC_ASYNC) 
	cmd->request.scsi_cdb.media_flags |= VSC_ASYNC; 

    /* set the direction of data transfer media flags according to sc_buf*/
    if (scp->bufstruct.b_bcount) {
        /* data transfer requested for this cmd*/
        if (scp->bufstruct.b_flags & B_READ) {
            /* this is a read request */
            cmd->request.scsi_cdb.media_flags |= VSC_READ;
        }
        else  {  /* this is a write request */
            cmd->request.scsi_cdb.media_flags |= VSC_WRITE;
        }
    } /* end if */


    /* set the p_buf_list pointer in pd_info1 to point to buf structs */
    /* for this command */
    if (scp->bp != NULL) {  /* if this is a spanned command */
	cmd->pd_info1.p_buf_list = (caddr_t)scp->bp;
    }
    else { /* non spanned command, pass bufstruct */
	cmd->pd_info1.p_buf_list = (caddr_t)&scp->bufstruct;
    }

    /* Call ndd_output to do the write. */
    rc = (int) (scsi->shared->ndd->ndd_output) (scsi->shared->ndd,
						&cmd->ctl_elem);
    /* see if cmd_element is valid (non-NULL). if so free up the        */
    /* the cmd_elem.  If NULL, cmd_elem already freed.                  */
    if (cmd != NULL) {
	tag = cmd->tag;
	cmd->cmd_state = INACTIVE;
	VSC_FREETAG (scsi->free_cmd_list[(int)((tag)/VSC_BITS_PER_WORD)],
		     tag);
    }
    if (rc) {
        ret_code = EIO;
    }

    return (ret_code);

} /* end vsc_dump_write */

/**************************************************************************/
/*									  */
/* NAME:	vsc_dump_start						  */
/*									  */
/* FUNCTION: 								  */
/*		Protocol driver's dump routine to prepare for a dump.     */
/*              This function issues an abort to all devices with         */
/*              outstanding commands.                                     */
/*									  */
/* EXECUTION ENVIRONMENT:						  */
/*      This routine is called on the interrupt level.                    */
/*      This routine should run at INTMAX level.                          */
/*									  */
/* NOTES:								  */
/*									  */
/* INPUT:								  */
/*              scsi  - pointer to the scsi_info struct on which the dump */
/*                      device resides.                                   */
/*		sc_buf- sc_buf structure passed by caller		  */
/*									  */
/* RETURN VALUE DESCRIPTION:                                              */
/*      A zero will be returned on successful completion, otherwise,      */
/*      one of the errno values listed below will be given.               */
/*                                                                        */
/*		ENOMEM	- error allocating resources			  */
/*		EINVAL	- invalid input parameter			  */
/*		EIO	- kernel service failure or invalid operation	  */
/*									  */
/**************************************************************************/
int
vsc_dump_start (struct scsi_info * scsi)
{
    int     rc,                      /* locally used return code           */
            ret_code,                /* return code for this function      */
            i;                       /* for loop variable to loop through  */
                                     /* all devices on this scsi_info      */

    ret_code = 0; /* init to no error */

    /* Check all initiator devices on this virtual SCSI bus and abort all */
    /* outstanding commands */
    for (i = 0; i < DEVPOINTERS; i++) {
	if ((scsi->dev[i] != NULL) &&	/* device still open */
	    (scsi->dev[i]->head_act != NULL)) {	
            /* device has outstanding commands */
	    
            /* if device qstate is not halted  then build and send abort */
	    if (!(scsi->dev[i]->qstate & HALTED)) {

		/* indicate an abort in progress  */
		scsi->dev[i]->qstate |= HALTED;

		/* build the abort control element */
                scsi->dev[i]->command_element.request.scsi_cdb.scsi_id
                             = SID(i);
                scsi->dev[i]->command_element.request.scsi_cdb.scsi_lun
                             = LUN(i);
		/* Set the command to abort */
                scsi->dev[i]->command_element.request.scsi_cdb.media_flags
		    |= (VSC_ABORT | VSC_RESUME);
                scsi->dev[i]->command_element.cmd_type  = 
                                                       INTR_LVL_CANCEL;
#ifdef VSC_TRACE
                vsc_internal_trace(scsi, i, 
                                   (uint *) &scsi->dev[i]->command_element,
				   2, 0);
#endif /* VSC_TRACE */
		/* issue the abort to the adapter drivers output function */
		rc = (int) (scsi->shared->ndd->ndd_output) (scsi->shared->ndd,
				   &scsi->dev[i]->command_element.ctl_elem);
		if (rc) {
                    ret_code = EIO;
		    return (ret_code);
                }
	    } /* end if !(qstate & HALTED) */
	} /* end if device opened and active */
    } /* end for */

    /*
     * Call the lower level dump routine.  One function of this routine is to
     * overlay asc_output() (stored in ndd_output) with asc_dump_write(). 
     */
    rc = (dump_entry_ptr) (scsi->shared->ndd, DUMPSTART, NULL);
    if (rc) {
        ret_code = EIO;
        return (ret_code);
    }

    return (ret_code);

} /* end vsc_dump_start */



/*
 * NAME:	vsc_buf_free
 *
 * FUNCTION: 
 *              Function used by the target mode head device driver to
 *              return buffers after the data has been copied out. The
 *              address of this function is returned to the target mode
 *              head when it issues the SCIOSTARTTGT ioctl.
 *
 * EXECUTION ENVIRONMENT:
 *
 *		This function can only be  called from both the process and
 *              interrupt level execution enviornments. 
 *
 * NOTES:
 *
 * EXTERNAL CALLS:
 *              disable_lock                unlock_enable
 *
 * RETURNS:
 *
 */
void
vsc_buf_free (struct tm_buf * tm_ptr)  /* list to free */
{
    struct b_link    *b_link;                    /* current b_link           */
    struct b_link    *tb_link;                   /* temporary b_link         */
    struct tm_info   *user;                      /* target mode initiator    */
    struct tm_info   *tuser;                     /* target mode initiator    */
    struct scsi_info *scsi;                      /* virtual scsi structure   */
    int               scsi_id;                   /* SCSI ID                  */
    int               old_pri;                   /* saved interrupt priority */
    struct buf_pool_elem *pool_elem_list = NULL; /* list of buf_pool_elems   */
    struct buf_pool_elem *pool_elem;             /* current buf_pool_elem    */
    int               i;

    /* Verify that there is at least one buffer being returned. */
    if (tm_ptr != NULL)
    {
	/*
	 * Cast the tm_buf to a b_link structure.  At this level the data is
	 * referenced as a b_link structure. 
	 */
	b_link = (struct b_link *) tm_ptr;

	/*
	 * Get the scsi structure from the b_link.
	 */
	scsi = b_link->sp;
        /*  Serialize with intrpts.  */
        old_pri = disable_lock(scsi->ddi.intr_priority,
                               &scsi->shared->ndd->ndd_demux_lock);
	scsi_id = b_link->user_id;
	user = scsi->tm[scsi_id];

	/*
	 * All buffers in the list were used by one device.
	 */
	while (b_link != NULL)
	{
	    tb_link = b_link->next;

	    if (scsi->pending_err)
	    {
		/*
		 * This buffer needs to be reused to report an error back
		 * to the target head.
		 */
		b_link->next = NULL;

		/*
		 *  Find the instance that has an error outstanding.
		 */
		for (i=0; i<TMPOINTERS; i++)
		{
		    tuser = scsi->tm[i];
		    if (tuser && tuser->previous_error)
		    {
			b_link->tm_correlator = tuser->tm_correlator;
			b_link->adap_devno = scsi->ddi.parent_unit_no;
		    
			/* No data passed from the adapter. */
			b_link->data_len = 0;
			
			b_link->user_flag = TM_HASDATA;
		    
			b_link->user_id = i;
			b_link->status_validity = SC_ADAPTER_ERROR;

			switch (tuser->previous_error)
			{
			  case 0x01:
			    /* SCSI bus parity error / SCSI bus reset */
			    b_link->general_card_status = SC_SCSI_BUS_RESET;
			    vsc_log_err (scsi, ERRID_SCSI_ERR10, 31, NULL, 0);
			    break;
			  case 0x02:
			    /* hardware error / system parity error */
			    b_link->general_card_status = SC_HOST_IO_BUS_ERR;
			    vsc_log_err (scsi, ERRID_SCSI_ERR10, 32, NULL, 0);
			    break;
			  case 0x03:
			    /* command aborted by initiator */
			    b_link->general_card_status = SC_SCSI_BUS_FAULT;
			    break;
			  default:
			    b_link->general_card_status =
				SC_ADAPTER_SFW_FAILURE;
			    vsc_log_err (scsi, ERRID_SCSI_ERR10, 33, NULL, 0);
			    break;
			}
			tuser->previous_error = 0;
			
			scsi->pending_err--;

			/*
			 * The buffer may now be queued to a different device.
			 */
			user->num_bufs_qued--;
			tuser->num_bufs_qued++;
	
			/*
			 * Send the b_link list to the target mode head.
			 */
			tuser->recv_func (b_link);

			break;
		    }
		}
	    }
	    else
	    {
		/*
		 * Release this buffer since it is no longer needed.
		 */

		pool_elem = (struct buf_pool_elem *) b_link->buf_addr;
		/*
		 * Create a list of buf_pool_elem to free.
		 */
		if (pool_elem)
		{
		    if (pool_elem_list)
		    {
			pool_elem->next = pool_elem_list;
			pool_elem_list = pool_elem;
		    }
		    else
		    {
			pool_elem->next = NULL;
			pool_elem_list = pool_elem;
		    }
		}
		
		/*
		 * Put the buffer on the free list.
		 */
		vsc_free_b_link ((struct b_link *) b_link);
	    }
	    b_link = tb_link;
	}

	/*
	 * Return the buffer to the adapter driver. The buf_pool_elem 
	 * buffer stores its own free function. 
	 */
	if (pool_elem_list)
	{
#ifdef VSC_TRACE
	    /* Trace all the buf_pool_elems being freed */
	    pool_elem = pool_elem_list;
	    while (pool_elem != NULL) 
	    {
		vsc_internal_trace(scsi, scsi_id, (uint *) pool_elem, 4, 1);
		pool_elem = pool_elem->next;
	    }
#endif /* VSC_TRACE */

	    pool_elem_list->buf_pool_free (pool_elem_list);
	}

	/*
	 * Now that the head has freed some buffers and they have been added
	 * to the free list it is neccesary to check for condition requiring
	 * that an id needs to be enabled.
	 */
	if (user->stopped && (user->num_bufs_qued <= user->num_to_resume))
	{
	    /*
	     * Send an enable id, if that is really necessary. 
	     */
	    vsc_need_enaid (scsi, (char) scsi_id);
	}

    unlock_enable(old_pri, &scsi->shared->ndd->ndd_demux_lock);

    } /* end if tm_ptr != NULL */

}  /* end vsc_buf_free */


/*
 * NAME:	vsc_free_b_link
 *
 * FUNCTION: 
 *              Function to mark a target mode buffer as free and place
 *              it on the free list for this adapter.
 *
 * EXECUTION ENVIRONMENT:
 *
 *		This function can only be called from both the process and
 *              interrupt level execution enviornments.
 *               
 *
 * NOTES:
 *
 * EXTERNAL CALLS:
 *		
 * RETURNS:  
 *
 */
void
vsc_free_b_link (struct b_link * b_link)  /* b_link to put on free list */
{
    struct scsi_info     *scsi;     /* virtual scsi device       */

    scsi = b_link->sp;

    scsi->tm[b_link->user_id]->num_bufs_qued--;

    /*
     * Put on head of free list. 
     */
    b_link->next = scsi->head_free;
    scsi->head_free = b_link;

    /*
     * Don't use the data fields again.
     */
    b_link->data_addr = NULL;
    b_link->data_len = 0;
    b_link->buf_addr = NULL;
    b_link->buf_size = 0;

    /*
     * Remove the buffer from scsi_info struct's read_bufs list. 
     */
    if (b_link->back == NULL)
    {	/* first buffer */
	scsi->read_bufs = b_link->forw;
	if (scsi->read_bufs != NULL)
	    scsi->read_bufs->back = NULL;
    } else
    {
	if (b_link->forw != NULL)
	    b_link->forw->back = b_link->back;
	b_link->back->forw = b_link->forw;
    }
    b_link->forw = NULL;
    b_link->back = NULL;

} /* end vsc_free_b_link */


/*
 * NAME:	vsc_async_notify
 *
 * FUNCTION: 
 *              function to notify either all or a specified register user
 *              of a specified asyncrounous event.  The user must first
 *              register though the SCIOEVENT ioctl so that the address
 *              of the users asyncrounous event handler can be obtained.
 *
 * EXECUTION ENVIRONMENT:
 *
 *		This function can only be called from the interrupt level 
 *
 * NOTES:     
 *       
 *
 *
 * EXTERNAL CALLS:
 *              bzero
 *		
 * INPUT:
 *		scsi	  - scsi_info structure ptr for this device
 *              dev_index - the dev_index of the device to notify or -1
 *                          if all devices are to be notified 
 *              events    - the specific event that a registered device
 *                          is to be notified of 
 *
 *
 * RETURNS:  
 *
 */
void
vsc_async_notify (struct scsi_info *scsi,  /* virtual scsi structure  */
		  int events)              /* event to be notified of */
{
    int     dev_index;               /* index into device list   */
    struct tm_info *tm;              /* target mode device       */
    struct dev_info *d;              /* initiator device         */
    struct sc_event_info event_info; /* event information        */

    /* 
     * Zero out event info structure.
     */
    bzero ((char *) &event_info, sizeof (struct sc_event_info));
    event_info.events = events;
    event_info.adap_devno = scsi->devno;

    /* 
     * Notify all initiator devices.
     */
    for (dev_index = 0; dev_index < DEVPOINTERS; dev_index++)
    {
	d = scsi->dev[dev_index];
	/*
	 * Check for device being opened and async funtion exists. 
	 */
	if ((d == NULL) || (d->async_func == NULL))
	    continue;
	event_info.async_correlator = d->async_correlator;
	event_info.id = SID (dev_index);
	event_info.lun = LUN (dev_index);
	event_info.mode = SC_IM_MODE;
	(d->async_func) (&event_info);
    }	/* end for */

    /* 
     * Notify all target devices.
     */
    for (dev_index = 0; dev_index < TMPOINTERS; dev_index++)
    {
	tm = scsi->tm[dev_index];
	if ((tm == NULL) || (tm->async_func == NULL))
	    continue;
	event_info.async_correlator = tm->async_correlator;
	event_info.id = SID (dev_index);
	event_info.lun = 0;
	event_info.mode = SC_TM_MODE;
	(tm->async_func) (&event_info);
    }	/* end for */

}  /* end vsc_async */


/*
 * NAME:	vsc_need_disid
 *
 * FUNCTION: 
 *              Whenever the number of buffer filled with data from a
 *              partiuclar initiator reaches the number of buffers allocated
 *              for that initiator, this function is invoked to attempt to
 *              temporarily disable that initiator so that it does not use
 *              buffers allocated for another initiator. This function
 *              will determine if the disable id command actually needs to
 *              be sent based upon the current state of this target device.
 *              If a disable id command is determined to be necessary, then
 *              the initiator id is marked as needed to be disabled, and
 *              an adapter driver ioctl is called to initiate the disable
 *              command.
 *
 * EXECUTION ENVIRONMENT:
 *
 *		This function can only be called from the interrupt level. 
 *
 * NOTES:
 *
 *
 * EXTERNAL CALLS:
 *		
 * INPUT:
 *		scsi	  - scsi_info structure ptr for this device
 *              id        - an index into the tm_info array for the specific
 *                          tm_info structure  corresponding to an initiator
 *
 * RETURNS:
 *
 */
void
vsc_need_disid (struct scsi_info *scsi, /* virtual scsi device               */
		uint              id)   /* SCSI ID/index of initiator device */
{
    struct tm_info *user;        /* target mode initiator                */
    ulong           punlunbus;   /* physical/logical unit number and bus */
    int             ret_code;	 /* return code for this function        */

    user = scsi->tm[id];

    /* 
     * Interrupts assumed to be disabled on entry.
     */

    if (!user->stopped)
    {
	/*
	 * Tell the adapter that target mode is suspended by issuing an
	 * adapter device driver ioctl. 
	 */
	punlunbus = scsi->ddi.location << 14;
	punlunbus |= id << 5;

#ifdef VSC_TRACE
	/* Trace this scsi id */
	vsc_internal_trace(scsi, id, &id, 3, 1);
#endif /* VSC_TRACE */

	(void) (scsi->shared->ndd->ndd_ctl) (scsi->shared->ndd,
					   NDD_TGT_SUS_DEV, punlunbus);

	user->stopped = TRUE; 
        vsc_async_notify(scsi, SC_BUFS_EXHAUSTED);
    }
    return;
}  /* end vsc_need_disid */


/*
 * NAME:	vsc_need_enaid
 *
 * FUNCTION: 
 *              When buffer are returned from the target mode device driver
 *              via vsc_buf_free, the number of buffer queued with data to
 *              the tm device driver for a particular initiator is monitored.
 *              When this number drops below a defined threshold, this
 *              function is involked to determine if the initiator id needs to
 *              be enabled.  Based upon the current state of the target device
 *              this function determines if an enable id command is necessary.
 *              If it is determined to be necessary, then this initiator id
 *              is  enabled through an adatper driver ioctl to initiate the
 *              enable command. 
 *
 * EXECUTION ENVIRONMENT:
 *
 *		This function can only be called from the interrupt level. 
 *
 * NOTES:
 *
 *
 * EXTERNAL CALLS:
 *		
 * INPUT:
 *		scsi	  - scsi_info structure ptr for this device
 *              id        - an index into the tm_info array for the specific
 *                          tm_info structure corresponding to an initiator
 *
 * RETURNS:  
 *
 */
void
vsc_need_enaid (struct scsi_info *scsi, /* virtual scsi device               */
		uint              id)   /* SCSI ID/index of initiator device */
{
    struct tm_info *user;        /* target mode initiator                */
    ulong           punlunbus;   /* physical/logical unit number and bus */
    int             ret_code;	 /* return code for this function        */

    user = scsi->tm[id];

    /* 
     * Interrupts assumed to be disabled on entry.
     */

    if (user->stopped)
    {
	/*
	 * Tell the adapter that target mode is enabled by issuing an
	 * adapter device driver ioctl. 
	 */
	punlunbus = scsi->ddi.location << 14;
	punlunbus |= id << 5;

#ifdef VSC_TRACE
	/* Trace this scsi id */
	vsc_internal_trace(scsi, id, &id, 2, 1);
#endif /* VSC_TRACE */

	(void) (scsi->shared->ndd->ndd_ctl) (scsi->shared->ndd,
					   NDD_TGT_RES_DEV, punlunbus);

	user->stopped = FALSE;
    }
    return;
}  /* end vsc_need_enaid */


/*
 * NAME:        vsc_target_receive
 *
 * FUNCTION:    When the receive function (pseudo interrupt handler) gets
 *              a control element which has is corresponding to a target 
 *              mode function, this function is called to process the target
 *              mode control element.  The control element can only be of the 
 *              event type. 
 *    
 *              Valid event elements would be for the SCSI info event 
 *              indicating that an initiator has sent data to the adapter
 *              as a target and this event element contains information 
 *              concerning which initiator sent the data and the buffers
 *              where the data resides.
 *
 *              A SCSI info event byte map is shown below.
 *
 * EXECUTION ENVIRONMENT:
 *
 *              This function can only be called from the interrupt level.
 *
 * NOTES:
 *
 * EXTERNAL CALLS:
 *
 * INPUT:
 *		scsi	  - scsi_info structure ptr for this device
 *              cmd       - pointer to the command element which is to
 *                          be processed
 *
 * RETURNS:
 *
 *
 *
 *                       SCSI info Event Element
 *
 *   31 30 ... 18 17 16  |  15 14 13 ... 2 1 0       Remarks
 *
 * +---------------------------------------------------------------+  
 * | 0000 0000 0000 0000   LLLL LLLL LLLL LLLL   TYPE / LENGTH     | 0
 * +---------------------------------------------------------------+  
 * | 1000 0000 0100 0111   0000 0000 0000 0000   SCSIINFO EVENT    | 1
 * |---------------------------------------------------------------+  
 * | <Adp UID> <EntityID> <System ID><Info ID>   SOURCE/DEST ID    | 2
 * +---------------------------------------------------------------+  
 * | CCCC CCCC CCCC CCCC   CCCC CCCC CCCC CCCC   CORRELATION ID    | 3
 * +---------------------------------------------------------------+  
 * | 0000 0000 0000 0000   C000 0010 1000 0EE0   INITIATOR DESC    | 4
 * +---------------------------------------------------------------+  
 * | 0000 0000 0000 PPPP   0000 0LLL 0000 BBBB   PUN/LUN/BUS       | 5
 * +---------------------------------------------------------------+  
 * | 0000 0000 0000 0000   0000 0000 0000 0000                     | 6
 * +---------------------------------------------------------------+  
 * | 0000 0000 0000 0000   0000 0000 0000 0000                     | 7
 * +---------------------------------------------------------------+  
 * | NNNN NNNN NNNN NNNN   C000 0001 0010 0EE0   BUFFER #1 DESC    | 8
 * +---------------------------------------------------------------+  
 * | LLLL LLLL LLLL LLLL   LLLL LLLL LLLL LLLL   BUFFER #1 SIZE    | 9
 * +---------------------------------------------------------------+
 * | 0000 0000 0000 0000   0000 0000 0000 0000   BUFFER #1 ADD MSW |10
 * +---------------------------------------------------------------+  
 * | 0000 0000 0000 0000   0000 0000 0000 0000   BUFFER #1 ADD LSW |11
 * +---------------------------------------------------------------+
 * |          .                     .                    .         |
 * |          .                     .                    .         |
 * |          .                     .                    .         |
 * +---------------------------------------------------------------+
 * | NNNN NNNN NNNN NNNN   C000 0001 0010 0EE0   BUFFER #N DESC    |
 * +---------------------------------------------------------------+  
 * | LLLL LLLL LLLL LLLL   LLLL LLLL LLLL LLLL   BUFFER #N SIZE    |
 * +---------------------------------------------------------------+
 * | 0000 0000 0000 0000   0000 0000 0000 0000   BUFFER #N ADD MSW |
 * +---------------------------------------------------------------+  
 * | 0000 0000 0000 0000   0000 0000 0000 0000   BUFFER #N ADD LSW |
 * +---------------------------------------------------------------+
 *
 * Words 0 - 3 are the control element header.  The total length 
 * (in bytes) of this element is in bits L..L.  The correlation id
 * (bits C..C) contains the address of the struct scsi_info that
 * this data applies to.
 *
 * Words 4 - 7 are a type 2 initiator descriptor that represents an
 * an initiator descriptor that identifies which initiator sent a 
 * SCSI Send command to the adapter (PPPP = physical unit number,
 * BBBB = SCSI bus number) and the target LUN which was selected.
 * Bits 2 - 1 (EE) are used to indicate if an error occured before
 * any buffers were taken from the buffer pool.
 * 
 * The remaining words are buffer descriptors of which there may be
 * up to 10.  Bit 15 (C) is used indicate whether this is the last
 * buffer used to receive data during processing of the Send command.
 * This bit will be set to 0 if the particular send command has 
 * completed.  All intermediate buffers will have this bit set to 1
 * (chain) indicating that more buffers are to be received.
 * Bits 2 - 1 (EE) are used to indicate if an error occured or the
 * command was aborted during processing.  The error codes are listed
 * below.  The second word (bits L..L) represents the length of the
 * buffer in bytes.
 *
 * EE = Error Code
 * ------------------------------------------
 * 00 = No Error
 * 01 = SCSI Bus Error/Reset
 * 10 = System/Adapter Error
 * 11 = Command Aborted by Initiator
 */

void
vsc_target_receive (struct ndd * adapter_ndd, /* network device driver       */
		                              /* interface                   */
		    struct info_event_elem_def *info_event) /* event         */
		                                            /* information   */
{
    uint   num_bufs;                 /* number of buf_pool_elems           */
    struct tm_info *user;            /* target mode initiator              */
    int    i;                        /* loop counter                       */
    uint   scsi_id;                  /* SCSI ID                            */
    struct b_link *b_list = NULL;    /* b_link list                        */
    struct b_link *new_b;            /* new b_link                         */
    struct b_link *prev_b = NULL;    /* previous b_link                    */
    struct b_link *next_b = NULL;    /* next b_link                        */
    struct buf_pool_elem *pool_elem; /* current buf_pool_elem              */
    struct type1_pd_def  *buffer;    /* type1 pd buffer                    */
    struct scsi_info *scsi;          /* scsi device                        */
    struct buf_pool_elem *pool_elem_list = NULL; /*  buf_pool_elem list    */

    /*
     * Point to first buffer descriptor in SCSI Info Event Element.
     */
    buffer = &info_event->buf_desc[0];

    /*
     * Compute the number of buffers.  The total length of SCSIinfo Event
     * Element minus the length of the control element header minus the
     * length of the initiator descriptor gives the length of all the buffer
     * descriptors in bytes.  The length of the buffer descriptors divided by
     * 16 bytes per descriptor gives the number of buffers.
     */
    num_bufs = (info_event->header.length - sizeof (struct ctl_elem_hdr) -
		sizeof (struct type2_pd_def)) / 16;
    ASSERT (num_bufs <= 10);

    scsi_id = (uint) info_event->init_desc.pun;

    scsi = (struct scsi_info *) info_event->header.correlation_id;
    ASSERT (scsi);

    user = scsi->tm[scsi_id];

#ifdef VSC_TRACE
    /* Trace this info event element */
    vsc_internal_trace(scsi, scsi_id, (uint *) info_event, 1, 1);
#endif /* VSC_TRACE */

    /*
     * Process this info_event if the device is still open.
     */
    if (user)
    {
	/*
	 * Check for the adapter reporting an error before any bufs are taken
	 * from buffer pool.
	 */
	if (!(info_event->init_desc.error))
	{
	    /*
	     * For each buffer_desc struct in the scsi info event element.
	     */
	    for (i = 0; i < num_bufs; i++)
	    {
		/*
		 * Point to the data described in the info event.
		 */
		pool_elem = (struct buf_pool_elem *) buffer->addr_lsw;

		/*
		 * Get a free b_link structure.
		 */
		new_b = vsc_get_b_link (scsi);
		
		ASSERT (new_b != NULL);
		
		/*
		 * Fill in the b_link structure fields. 
		 */
		new_b->tm_correlator = user->tm_correlator;
		
		/*
		 * Point to the buffer passed from the adapter.
		 */
		new_b->buf_addr = (caddr_t) pool_elem;
		new_b->buf_size = sizeof (struct buf_pool_elem);

		/*
		 * Initialize data fields that tm head will use.
		 */
		new_b->data_addr = (caddr_t) pool_elem->virtual_addr;
		new_b->data_len = buffer->size;

		new_b->user_flag = TM_HASDATA;
		/* 
		 * Check for end of data.
		 */
		if (buffer->chain)
		    new_b->user_flag |= TM_MORE_DATA;

		new_b->user_id = scsi_id;
		new_b->status_validity = 0;
		new_b->general_card_status = 0;

		/* 
		 * Chain the b_links for the tm head driver.
		 */
		new_b->next = NULL;
		if (prev_b)
		    prev_b->next = new_b;
		else	/* first b_link */
		    b_list = new_b;
		
		user->num_bufs_qued++;
		
		/* 
		 * Check for any errors in the current buffer.
		 */
		if (buffer->error)
		{
		    /*
		     * There's an error with this data from the adapter.
		     * Throw out all data received until now and pass up
		     * the current b_link that has the error.
		     */

		    while (b_list->next != NULL)
		    {
			next_b = b_list->next;

			pool_elem = (struct buf_pool_elem *) b_list->buf_addr;
			if (pool_elem_list)
			    pool_elem->next = pool_elem_list;
			else /* The pool_elem list is empty */
			    pool_elem->next = NULL;
			pool_elem_list = pool_elem;

			vsc_free_b_link (b_list);
			b_list = next_b;
		    }
	            if (pool_elem_list) 
	                pool_elem_list->buf_pool_free (pool_elem_list);

		    /*
		     * At this point b_list should consist of a single b link.
		     */
		    ASSERT (b_list == new_b)

		    /* Initialize data fields that tm head will use */
		    b_list->data_len = 0;

		    b_list->status_validity = SC_ADAPTER_ERROR;
		    
		    switch (buffer->error)
		    {
		      case 0x01:
			/* SCSI bus parity error / SCSI bus reset */
			b_list->general_card_status = SC_SCSI_BUS_RESET;
			vsc_log_err (scsi, ERRID_SCSI_ERR10, 28, NULL, 0);
			break;
		      case 0x02:
			/* hardware error / system parity error */
			b_list->general_card_status = SC_HOST_IO_BUS_ERR;
			vsc_log_err (scsi, ERRID_SCSI_ERR10, 29, NULL, 0);
			break;
		      case 0x03:
			/* command aborted by initiator */
			b_list->general_card_status = SC_SCSI_BUS_FAULT;
			break;
		      default:
			b_list->general_card_status = SC_ADAPTER_SFW_FAILURE;
			vsc_log_err (scsi, ERRID_SCSI_ERR10, 30, NULL, 0);
			break;
		    }
		    
		    b_list->next = NULL;
		}

		/*
		 * Check to see if the initiator should be disabled. 
		 */
		if (user->num_bufs_qued >= user->num_bufs)
		    vsc_need_disid (scsi, scsi_id);
		
		prev_b = new_b;
		
		/*
		 * Move to the next buffer descriptor.
		 */
		buffer++;
		
	    }	/* end for each buffer received */
	}
	else  /* There's an error in the initiator descriptor */
	{
            if (!user->previous_error)
            {
                scsi->pending_err++;
            }
	    /*
	     * Save the error code.
	     */
	    user->previous_error = (uchar) info_event->init_desc.error;
	    return;
	}
	
	/*
	 * Send the b_link list to the target mode head.
	 */
	user->recv_func (b_list);
    }
    else /* the device is no longer open. */
    {
	/*
	 * Return the buffers to the adapter driver.
	 */
	buffer = &info_event->buf_desc[0];
	for (i = 0; i < num_bufs; i++)
	{
	    pool_elem = (struct buf_pool_elem *) buffer->addr_lsw;
	    if (pool_elem)
	    {
#ifdef VSC_TRACE
		/* Trace the buf_pool_elems being freed */
		vsc_internal_trace (scsi, scsi_id, (uint *) pool_elem, 4, 1);
#endif /* VSC_TRACE */
		if (pool_elem_list)
		    pool_elem->next = pool_elem_list;
		else /* The pool_elem list is empty */
		    pool_elem->next = NULL;
		pool_elem_list = pool_elem;
	    }
	    buffer++;
	}
	if (pool_elem_list) 
	    pool_elem_list->buf_pool_free (pool_elem_list);
    } /* End else device not open */

}  /* vsc_target_receive */


/*
 * NAME:        vsc_get_b_link
 *
 * FUNCTION:
 *              This function returns a free b_link from the free b_link list
 *
 *
 * EXECUTION ENVIRONMENT:
 *              This function can be called from the proccess or interrupt
 *              level.
 *
 * NOTES:       
 *
 * EXTERNAL CALLS:
 *
 * RETURNS:
 *
 */
struct b_link *
vsc_get_b_link (struct scsi_info * scsi)  /* virtual scsi structure */
{
    struct b_link *b;    /* b_link to return */

    b = scsi->head_free;

    ASSERT (b);

    /* 
     * Remove from the free list.
     */
    scsi->head_free = scsi->head_free->next;

    /*
     * Add to the head of the active read list.
     */
    b->back = NULL;
    b->forw = scsi->read_bufs;
    if (scsi->read_bufs)
	scsi->read_bufs->back = b;
    scsi->read_bufs = b;

    return (b);

}  /* end vsc_get_b_link */

#ifdef VSC_TRACE
/*
 * NAME:        vsc_internal_trace
 *
 * FUNCTION:
 *              This function writes trace information to the internal trace
 *              table for all virtual scsi devices.
 *
 *
 * EXECUTION ENVIRONMENT:
 *              This function can be called from the proccess or interrupt
 *              level.
 *
 * NOTES:
 *
 * EXTERNAL CALLS:
 *
 * RETURNS:
 *              NONE
 *
 */
void
vsc_internal_trace(struct scsi_info *scsi, /* */
		   int   index,            /* index to device table           */
                   uint *buf,              /* data buffer to save             */
		   int   option,           /* type of data to trace           */
		   int   mode)             /* 0-initiator mode  1-target mode */
{
#define CMDO  0x434D444F  /* ASCII representation of 'CMDO' */
#define CMDI  0x434D4449  /* ASCII representation of 'CMDI' */
#define CANO  0x43414E4F  /* ASCII representation of 'CANO' */
#define CANI  0x43414E49  /* ASCII representation of 'CANI' */
#define RESO  0x5245534F  /* ASCII representation of 'RESO' */
#define RESI  0x52455349  /* ASCII representation of 'RESI' */
#define TRCV  0x54524356  /* ASCII representation of 'TRCV' */
#define TRES  0x54524553  /* ASCII representation of 'TRES' */
#define TSUS  0x54535553  /* ASCII representation of 'TSUS' */
#define TREL  0x5452454C  /* ASCII representation of 'TREL' */
#define TADD  0x54414444  /* ASCII representation of 'TADD' */
#define TDEL  0x5444454C  /* ASCII representation of 'TDEL' */
#define TADF  0x54414446  /* ASCII representation of 'TADF' */
#define TDLF  0x54444C46  /* ASCII representation of 'TDLF' */
#define WRAP  0x57524150  /* ASCII representation of 'WRAP' */
#define LAST  0x4C415354  /* ASCII representation of 'LAST' */

    uint    type;
    uint    size;
    struct trace_element *trace;
    struct cmd_elem *tmp_cmd;

    trace = (struct trace_element *)vsc_trace_ptr;

    if (mode == 0)
    {
	/* NOTE : a dev_index of -1 indicates a SCSI bus reset cmd_elem       */
	/*        a value of zero in trace_enabled indicates trace is enabled */
	if ((index == -1) || !(scsi->dev[index]->trace_enabled)) {
	    
	    switch (option) {
  
	      case 0 :
		  type = (uint) CMDO;
		  break;
	      case 1 :
		  type = (uint) CMDI;
		  break;
	      case 2 :
	          type = (uint) CANO;
		  break;
	      case 3 :
	          type = (uint) CANI;
		  break;
	      case 4 :
	          type = (uint) RESO;
		  break;
	      case 5 :
	          type = (uint) RESI;
		  break;
	      default :
		  return;
	    } /* end switch */
  
	    /*
	     * Check to see if this data will fit at the end of the
	     * trace table.  If not wrap back to the beginning.
	     */
	    size = sizeof (trace->data.cmd) / 4;
	    if ((vsc_trace_ptr + size + 2) >
		(&vsc_trace_tab[0] + TRACE_TABLE_LENGTH))
	    {
		*vsc_trace_ptr = (uint) WRAP;
		vsc_trace_ptr = &vsc_trace_tab[0] + 1;
		trace = (struct trace_element *)vsc_trace_ptr;
	    }
            tmp_cmd = (struct cmd_elem *)buf;
	    trace->type = type;
            bcopy ((char *) &tmp_cmd->request.scsi_cdb, (char *) &trace->data,
                   sizeof(struct scsi_cdb));
            if ((option % 2) &&
                (tmp_cmd->reply.header.options & VSC_EVENT_MASK)) {

                trace->data.cmd.error = 0xFF;
                trace->data.cmd.scsi_status = tmp_cmd->reply.scsi_status;
                trace->data.cmd.cmd_error = tmp_cmd->reply.cmd_error_code;
                trace->data.cmd.device_error = tmp_cmd->reply.device_error_code;
            }
            else {
                trace->data.cmd.error = 0;
                trace->data.cmd.scsi_status = 0;
                trace->data.cmd.cmd_error = 0;
                trace->data.cmd.device_error = 0;
            }
            trace->data.cmd.cdb.next_addr1 = (ulong) scsi;
            trace->data.cmd.cdb.next_addr2 = (ulong)tmp_cmd->bp;
            if (option == 0) {
                trace->data.cmd.cdb.scsi_extra =
                   (uint)scsi->dev[index]->num_act_cmds;
            }

	    vsc_trace_ptr += size + 1;
	    *vsc_trace_ptr = (uint) LAST;

	} /* end if trace_enabled */
    } /* end if initiator mode */
    else if (mode == 1)
    {
	if (!(scsi->tm[index]->trace_enabled)) {
	    switch (option) {
	      case 1 :
		  /* trace receiving info event elements */
	          type = (uint) TRCV;
		  size = sizeof (trace->data.info_event) / 4;
		  break;
	      case 2 :
		  /* trace NDD_TGT_RES_DEV ioctl */
	          type = (uint) TRES;
		  size = sizeof (trace->data.scsi_id) / 4;
		  break;
	      case 3 :
		  /* trace NDD_TGT_SUS_DEV ioctl */
	          type = (uint) TSUS;
		  size = sizeof (trace->data.scsi_id) / 4;
		  break;
	      case 4 :
		  /* trace releasing buf_pool_elem */
	          type = (uint) TREL;
		  size = sizeof (trace->data.pool_elem) / 4;
		  break;
	      case 5 :
		  /* trace NDD_TGT_ADD_DEV ioctl */
	          type = (uint) TADD;
		  size = sizeof (trace->data.scsi_id) / 4;
		  break;
	      case 6 :
		  /* trace NDD_TGT_DEL_DEV ioctl */
	          type = (uint) TDEL;
		  size = sizeof (trace->data.scsi_id) / 4;
		  break;
	      case 7 :
		  /* trace add_filter call */
	          type = (uint) TADF;
		  size = sizeof (trace->data.scsi_id) / 4;
		  break;
	      case 8 :
		  /* trace del_filter call */
	          type = (uint) TDLF;
		  size = sizeof (trace->data.scsi_id) / 4;
		  break;
	      default :
		  return;
	    }

	    /*
	     * Check to see if this data will fit at the end of the
	     * trace table.  If not wrap back to the beginning.
	     */
	    if ((vsc_trace_ptr + size + 2) >
		(&vsc_trace_tab[0] + TRACE_TABLE_LENGTH))
	    {
		*vsc_trace_ptr = (uint) WRAP;
		vsc_trace_ptr = &vsc_trace_tab[0] + 1;
		trace = (struct trace_element *)vsc_trace_ptr;
	    }
	    trace->type = type;
	    bcopy ((char *) buf, (char *) &trace->data, (size*4));
	    vsc_trace_ptr += size + 1;
	    *vsc_trace_ptr = (uint) LAST;
	} /* end if trace_enabled */
    } /* end if target mode */

} /* end vsc_internal_trace */
#endif /* VSC_TRACE */
