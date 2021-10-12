static char sccsid[] = "@(#)40	1.16  src/bos/kernext/scsi/asc_misc.c, sysxscsi, bos412, 9446B 11/15/94 13:48:22";
/*
 *   COMPONENT_NAME: SYSXSCSI
 *
 *   FUNCTIONS: asc_abort
 *		asc_activate_cmd
 *		asc_async_status
 *		asc_cleanup
 *		asc_clear_pos
 *		asc_clear_queues
 *		asc_create_bufs
 *		asc_disable_bufs
 *		asc_enable_bufs
 *		asc_entity_id
 *		asc_epow
 *		asc_get_adp_info
 *		asc_get_adp_resources
 *		asc_get_ctl_blk
 *		asc_init_pipes
 *		asc_init_pos
 *		asc_ioctl
 *		asc_log_error
 *		asc_parse_reply
 *		asc_process_adp_elem
 *		asc_process_locate
 *              asc_reassign_eids
 *		asc_reset_adapter
 *		asc_reset_bus
 *		asc_rir
 *		asc_stub
 *		asc_tm_rtov
 *		asc_wdt_intr
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

#include	"ascincl.h"

/*
 *  Allocate and initialize global storage area 
 *  used to manage all adapters.
 */
adp_ctrl_t	adp_ctrl = { 0,0,0,0 };

/* 
 *  EPOW interrupt handler struct and EPOW MP lock struct. 
 *  All adapters share a common structure.   
 */
struct intr epow;
Simple_lock  epow_mp_lock;

/* 
 *  Global device driver component dump table pointer.
 */
struct asc_cdt_tab *asc_cdt = NULL;

/* Control element used to reset busses during EPOW. */
struct	adp_ctl_elem	adp_pool;

#ifdef ASC_TRACE
struct trace_element asc_trace_tab[TRACE_ENTRIES];
struct trace_element *asc_trace_ptr = NULL;
#endif /* ASC_TRACE */

/*
 * NAME:	asc_init_pipes
 *
 * FUNCTION: 
 *		Setup the move mode delivery pipes.
 *
 * EXECUTION ENVIRONMENT:
 *		Process level only.
 *
 * NOTES:
 *		This routine configures the delivery pipes and leaves the
 *		adapter in move mode.
 *	 	Issue a management request element in locate mode.
 *
 *		The enqueue and dequeue pipes will be 4k in length.
 *		4K will support approximately 250 SCB's.
 *
 * EXTERNAL CALLS:
 *		disable_lock, lock_enable, d_master, vm_cflush, 
 *		w_start, e_sleep_thread
 *
 * CALLED FROM:
 *		asc_init_adapter
 *
 * INPUT:
 *		ap	- adapter structure
 *		ioaddr	- IO handle
 *
 * RETURNS:  
 *		EINVAL    - invalid request
 *		EIO       - kernel service failure
 */
int
asc_init_pipes( 
	struct	adapter_info	*ap,
	ulong	ioaddr )
{
ulong	ipri;
uchar	value;
uchar	pio_error = 0;
ushort	*reply; 
struct	cfg_mre	*mre; 

	/*
	 *  No commands are active at this adapter
	 *  when this command is initiated. Do not issue 
	 *  any commands till this  command completes.
	 *  Currently no need to enforce this since the caller's
	 *  open() has not succeeded yet and we trust that they
	 *  will not initiate any activity till it does.
	 */

	/* 
	 *  Map the delivery pipes and the control areas for DMA.
	 */ 
	ap->xmem.aspace_id = XMEM_GLOBAL;
	d_master(ap->dma_channel, DMA_NOHIDE, ap->eq_vaddr, PIPESIZE * 2,
			&ap->xmem, ap->eq_raddr );
	d_master(ap->dma_channel, DMA_NOHIDE, (char *)ap->surr_ctl.eq_ssf,
		 CONTROL_SIZE, &ap->xmem, (char *)ap->surr_ctl.eq_ssf_IO );
	
	/* Ensure that the pipes are page aligned. */
	ASSERT(((int) ap->eq_vaddr &  PAGE_OFFSET)  == 0);
	ASSERT(((int) ap->dq_vaddr &  PAGE_OFFSET ) == 0);

	/*
	 *  Build a Configure Delivery Pipes Management
	 *  Request element.
	 *  Use the Small Transfer Area to build the 
	 *  request element.
	 *  Note: the STA has already been mapped for DMA.
	 */
	mre = (struct cfg_mre *) ap->sta_vaddr;
	qrev_writel(MGNT_LENGTH, (uint *)&(mre->header.length));
	qrev_writel(MANAGEMENT | REQUEST_EID,(uint *)&(mre->header.indicators));
	qrev_writes(0x0, &(mre->header.dst));
	qrev_writes((ap->adp_uid << 8), &(mre->header.src));
	qrev_writel(0x0, (uint *)&(mre->header.correlation));
	qrev_writes(0x0, &(mre->id));
	qrev_writes(0x8010, &(mre->function));
	qrev_writes(ap->adp_uid, &(mre->uids));
	qrev_writes(0xFFFF, &(mre->cfg_status));
	qrev_writel((uint)ap->surr_ctl.pusa_IO, (uint *)&(mre->pusa)); 
	qrev_writes((ushort)ap->surr_ctl.ausa_IO, (ushort *)&(mre->ausa)); 
	qrev_writes(ap->ddi.base_addr, &(mre->aioa));
	qrev_writes(0x0, &(mre->pioa));
	qrev_writes(0x0, (ushort *)&(mre->timer_ctrl));
	qrev_writes(SHARED_MEMORY | ADAPTER | EMPTY_TO_NOTEMPTY,
		     &(mre->adp_cfg_options));
	qrev_writes(SHARED_MEMORY | SYSTEM | EMPTY_TO_NOTEMPTY,
		     &(mre->sys_cfg_options));
	qrev_writes(PIPESIZE , &(mre->eq_pipe_size));
	qrev_writes(PIPESIZE , &(mre->dq_pipe_size));
	qrev_writel((uint)ap->dq_raddr, (uint *)&(mre->dq_pipe_addr));
	qrev_writel((uint)ap->surr_ctl.dq_sds_IO, (uint *)&(mre->dq_sds_addr));
	qrev_writel((uint)ap->surr_ctl.dq_sse_IO, (uint *)&(mre->dq_sse_addr));
	qrev_writel((uint)ap->surr_ctl.dq_ses_IO, (uint *)&(mre->dq_ses_addr));
	qrev_writel((uint)ap->surr_ctl.dq_ssf_IO, (uint *)&(mre->dq_ssf_addr));
	qrev_writel((uint)ap->surr_ctl.ausa_IO, (uint *)&(mre->ausa));
	qrev_writel((uint)ap->eq_raddr, (uint *)&(mre->eq_pipe_addr));
	qrev_writel((uint)ap->surr_ctl.eq_sds_IO, (uint *)&(mre->eq_sds_addr));
	qrev_writel((uint)ap->surr_ctl.eq_sse_IO, (uint *)&(mre->eq_sse_addr));
	qrev_writel((uint)ap->surr_ctl.eq_ses_IO, (uint *)&(mre->eq_ses_addr));
	qrev_writel((uint)ap->surr_ctl.eq_ssf_IO, (uint *)&(mre->eq_ssf_addr));

	/*  Flush the request element out of the system cache. */
	vm_cflush((caddr_t) ap->sta_vaddr, MGNT_LENGTH );

	/*
	 *  Disable interrupts around this sequence to ensure
	 *  that the e_wakeup(interrupt) is not generated before
	 *  e_sleep is registered.
	 */
	ipri = disable_lock(ap->ddi.int_prior, &(ap->ndd.ndd_demux_lock));

	/* 
	 *  Ensure the BUSY bit is clear before issuing
	 *  command. If the BUSY bit is set at this point,
	 *  we have a logic or HW problem, return error.
	 */
	READ_BSR(ap->ddi.base_addr, &value );
	if( pio_error ) {
		unlock_enable( ipri, &(ap->ndd.ndd_demux_lock));
		return( EIO );
	}

	if( value & BUSY ) {
 		asc_log_error( ap, ERRID_SCSI_ERR1, 0,0,value,1 );
		ASSERT( 0 );
		unlock_enable( ipri, &(ap->ndd.ndd_demux_lock));
		return( EIO );
	}

	/* Set state flag. */
	ap->locate_state = SCB_PENDING;

	/*  
	 *  Load the Command Interface registers with the
	 *  IO address of the management request element.
	 */
	WRITE_CIR((ap->ddi.base_addr + CIR3), ((int) ap->sta_raddr >> 24 ));
	if( pio_error ) {
		unlock_enable( ipri, &(ap->ndd.ndd_demux_lock));
		return( EIO );
	}
	WRITE_CIR((ap->ddi.base_addr + CIR2), ((int) ap->sta_raddr >> 16 ));
	if( pio_error ) {
		unlock_enable( ipri, &(ap->ndd.ndd_demux_lock));
		return( EIO );
	}
	WRITE_CIR((ap->ddi.base_addr + CIR1), ((int) ap->sta_raddr >> 8 ));
	if( pio_error ) {
		unlock_enable( ipri, &(ap->ndd.ndd_demux_lock));
		return( EIO );
	}
	WRITE_CIR((ap->ddi.base_addr + CIR0), ((int) ap->sta_raddr) );
	if( pio_error ) {
		unlock_enable( ipri, &(ap->ndd.ndd_demux_lock));
		return( EIO );
	}
	
	/* 
	 *  Issue a Attention request to the adapter.
	 *  0x90 tells the adapter to execute the management request
	 *  element whose address is located in the Command Interface 
	 *  registers.
	 */
	ATTENTION( ap->ddi.base_addr, ATN_SIGNAL );
	if( pio_error ) {
		unlock_enable( ipri, &(ap->ndd.ndd_demux_lock));
		return( EIO );
	}

	/*
	 *  Kick off a watchdog timer to ensure that the  SCB command
	 *  completes in reasonable time. If the timer trips,
	 *  the adapter is assumed dead. asc_intr() will w_stop()
	 *  the watchdog timer.
	 */
	ap->wdog.dog.restart = SCB_CMD_DURATION;
	ap->wdog.reason = LOCATE_TMR;
	w_start( &ap->wdog.dog );
	 
	/*  sleep to allow cmd to complete. */
	e_sleep_thread((int *)&ap->locate_event, &ap->ndd.ndd_demux_lock, 
				LOCK_HANDLER);

	/*
	 *  Determine if the the pipes were configured successfully.
	 */
	if( ap->locate_state == SCB_COMPLETE ) {
		/*  
		 *  Although no exceptions were reported, verify success by
		 *  parsing the reply element.
		 *  The reply element was stored at the address where 
		 *  the Management Request element was located.
	 	*/
		reply = (ushort *)ap->sta_vaddr;
		if ( !( reply[8] == REPLY_SUCCESS ) && 
				!(reply[8] == REPLY_SUCCESS_Q )) { 	
			unlock_enable( ipri, &(ap->ndd.ndd_demux_lock));
			return( EIO );
		}
	}
	else {
		unlock_enable( ipri, &(ap->ndd.ndd_demux_lock));
		return( EIO );
	}

	unlock_enable( ipri, &(ap->ndd.ndd_demux_lock));

	/*
	 *  Initialize the local control areas
	 *  for both pipes.
	 */
	ap->local.eq_sf = ap->eq_vaddr;
	ap->local.eq_end = ap->eq_vaddr + (PIPESIZE - 1 );
	ap->local.eq_se = ap->local.eq_end; 
	ap->local.eq_top = ap->local.eq_end - WRAP_SIZE;
	ap->local.eq_ef = ap->local.eq_top;
	ap->local.eq_status = 0;
	ap->local.eq_wrap = 0;
	ap->local.dq_wrap = 0;
	ap->local.dq_ee = ap->dq_vaddr;
	ap->local.dq_se = ap->dq_vaddr;
	ap->local.dq_end =ap->dq_vaddr + (PIPESIZE - 1 );
	ap->local.dq_top =  ap->local.dq_end - WRAP_SIZE;
	ap->local.dq_status = 0;

	/*
	 *  Initialize and flush the write_only portions of the surrogate 
	 *  control areas for both pipes.
	 */
	qrev_writes( EMPTY_BIT, ap->surr_ctl.dq_sds );
	*ap->surr_ctl.dq_sse = 0;
	*ap->surr_ctl.eq_ses  = 0;
	*ap->surr_ctl.eq_ssf =  0;
	vm_cflush( (caddr_t)ap->surr_ctl.eq_ssf, SURR_SIZE );
	return( 0 );
}

/*
 * NAME:	asc_rir
 *
 * FUNCTION: 
 *		Register a read immediate request with the adapter.
 *
 * EXECUTION ENVIRONMENT:
 *		Process level only.
 *
 * EXTERNAL CALLS:
 *		disable_lock, lock_enable, e_sleep_thread, w_start, vm_cflush
 *
 * NOTES:
 *		If a read immediate request is active at the adapter,
 *		the adapter will notify the adapter driver of
 *		events such as SCSI bus resets. The adapter does this
 *		by sending a read immediate event element.
 *		It is necessary to always have a read immediate request
 *		active at the adapter.
 *
 * CALLED FROM:
 *		asc_init_adapter, asc_cleanup
 *
 * INPUT:
 *		ap	- adapter structure
 *		cmd	- register or unregister a RIR cmd.
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *		EIO	- kernel service failure or invalid operation
 */
int
asc_rir(
	struct	adapter_info	*ap,
	int	cmd )
{
int	i, rc;
uint	*free_ptr;
ulong	ipri;


	/*
	 *  Workaround for adapter's read of surrogate area when
	 *  EIO is issued after pipe configuration. The d_complete
	 *  will invalidate the prefetched IO buffer. This buffer
	 *  flush is strategically placed after the EIO for configuring
	 *  pipes and before the first request element put onto the
	 *  pipe. If the driver changes and the RIR is no longer
	 *  the first element put onto the pipe, move this 
	 *  delay/d_complete() accordingly.
	 */
	delay(HZ / 10);
	rc = d_complete(ap->dma_channel, DMA_NOHIDE,
			(char *)ap->surr_ctl.eq_ssf, PIPESIZE,
			&ap->xmem, (char *)ap->surr_ctl.eq_ssf_IO );

	/*
	 *  Build a Read Immediate Request control element 
	 *  and issue it to the adapter.
	 *  Build the request element directly onto the 
	 *  outbound pipe.
	 *
	 *  Acquire resources necessary to complete an adapter
	 *  command. If the outbound pipe is full, the cmd is queued
	 *  onto the wait queue. A pointer into the enqueue pipe is
	 *  returned if the call succeeds.
	 */
	ipri = disable_lock(ap->ddi.int_prior, &(ap->ndd.ndd_demux_lock));
	if(( i = asc_get_adp_resources( ap, &free_ptr, RIR_LENGTH )) < 0 ) {
		unlock_enable( ipri, &(ap->ndd.ndd_demux_lock));
		return( EIO );
	}

	/*  Build the request element directly on the enqueue pipe. */
	if( cmd == ACTIVATE_RIR ) {
		/*
		 *  Build the Read Immediate Request element. It is nothing
		 *  more than a SCB header with the Read Immediate function id.
		 */
		qrev_writel(sizeof(struct scb_header), (uint *)&free_ptr[0]);
		qrev_writel(READ_IMMEDIATE | REQUEST_EID,(uint *)&free_ptr[1]);
		qrev_writel((ap->adp_uid << 8), (uint *)&free_ptr[2]);
		qrev_writel((uint)&(ap->adp_pool[i].ctl_blk),
			    (uint *)&free_ptr[3] );
	}
	else {
		/*
		 *  Build the Cancel Request element. It is nothing
		 *  more than a SCB header with the Read Immediate function id.
		 */
		ASSERT( cmd == DEACTIVATE_RIR );
		qrev_writel(sizeof(struct scb_header), (uint *)&free_ptr[0]);
		qrev_writel(CANCEL_RIR | REQUEST_EID, (uint *)&free_ptr[1]);
		qrev_writel((ap->adp_uid << 8), (uint *)&free_ptr[2]);
		qrev_writel((uint)&(ap->adp_pool[i].ctl_blk), 
			    (uint *)&free_ptr[3] );
	}

	/*
	 *  Activate the command by updating surrogate areas,
	 *  pushing element onto the active list and notifying
	 *  the adapter that a new element exists.
	 */
	if(( rc = asc_activate_cmd( ap, free_ptr, RIR_LENGTH, i )) != 0 ) {
		(void) asc_get_ctl_blk( ap, FREE, i );
		unlock_enable( ipri, &(ap->ndd.ndd_demux_lock));
		return( EIO );
	}

	/*
	 *  Kick off a watchdog timer to ensure that the command
	 *  completes in reasonable time. If the timer trips,
	 *  the adapter is assumed dead. 
	 */
	ap->adp_cmd_pending++;
	ap->wdog.dog.restart = SCB_CMD_DURATION;
	ap->wdog.reason = MOVE_TMR;
	w_start( &ap->wdog.dog );

	/*
	 *  Sleep waiting for the command to complete. 
	 *  The sleep event word(ap->rir_event) is used only
	 *  for this event.
	 */
	e_sleep_thread((int *)&ap->rir_event, &ap->ndd.ndd_demux_lock, 
			LOCK_HANDLER);

	/*
	 *  Check return status in the reply element.
	 */
	rc = asc_parse_reply( ap, &(ap->adp_pool[i].ctl_blk) );
	if( ap->adp_pool[i].status & CMD_TIMED_OUT ) {
		ap->adp_pool[i].status &= ~CMD_TIMED_OUT;
 		asc_log_error( ap, ERRID_SCSI_ERR2, 0,0,0,2 );
		rc = EIO;
	}
	
	/* Return the ctl_elem_blk to the adapter pool. */
	(void) asc_get_ctl_blk( ap, FREE, i );
	ap->adp_cmd_pending--;
	unlock_enable( ipri, &(ap->ndd.ndd_demux_lock));
	return( rc );
}

/*
 * NAME:	asc_entity_id 
 *
 * FUNCTION: 
 *		Add/delete entity id's.
 *
 * EXECUTION ENVIRONMENT:
 *		Process level for initiator mode commands.
 *		Interrupt level for target mode commands.
 *
 * EXTERNAL CALLS:
 *		e_sleep_thread, w_start
 *
 * NOTES:
 *  		Devices on the scsi bus are assigned unique entity id's.
 *  		Any operation directed to a device must have this entity id
 *  		in the destination field of the SCB. This routine calculates
 * 		a unique entity id using the SCSI_ID/LUN/BUS combination.
 *  		This routine registers entity id's by building Management
 *  		Assign Entity request elements and issuing them to the 
 *		adapter. The adapter supports entity ids 1 - 254.
 *
 *		DESIGN NOTE:
 *		This routine may be called on either the process or
 *		interrupt level and interrupts are always disabled.
 *		The cmd parameter indicates what environment we are 
 *		called under. Commands initiated on the process level
 *		are issued to the adapter and we sleep waiting on completion. 
 *		Calls may come in on interrupt thread and those are issued
 *		to the adapter and we return to the caller. 
 *
 *		Disable interrupts. Consequences of enabling interrupts
 *		would be corrupted pipe and pointers. ie. process level
 *		grabs lock, output comes in at int level and starts
 *		putting stuff in wrong location of pipe.
 *		
 * CALLED FROM:
 *		asc_ioctl
 *
 * INPUT:
 *		ap	- adapter structure
 *		cmd	- operation requested
 *		arg	- SCSI id, LUN and bus
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *		EIO	- kernel service failure or invalid operation
 */
int
asc_entity_id(
	struct	adapter_info	*ap,
	int	cmd,
	int	arg )
{
int	rc;
int	lun;
uint	*free_ptr;
uchar	eid_val;
ushort	eid;
char	bus, id;
ulong	punlunbus;
ulong	action = 0;
int	j, i = 0;

	/*
	 *  Acquire resources necessary to complete an adapter
	 *  command. If the outbound pipe is full or an adapter ctl_blk
	 *  is not availible, then this request is failed with EIO rc.
	 */
	i = asc_get_adp_resources( ap, &free_ptr, EID_LENGTH );
	if( i < 0 ) {
	    return( EIO );
	}

	/*
	 *  Calculate the entity id associated with this device
	 *  based on the SCSI id/LUN/bus information found in arg.
	 *  Each bit in the EID table has a one to one correspondence
	 *  with a SCSI id/LUN combination.
	 *   format of arg:
	 *   bits 14 - 15 contain the bus
	 *   bits 5 - 8 contain the SCSI id
	 *   bits 0 - 4 contain the LUN
	 */
	punlunbus = (int)arg;
	eid = CALC_ID( punlunbus );
	bus = punlunbus >> BUS_SHIFT;
	lun = ( punlunbus & LUN_MASK ) << 16;
	id =  (punlunbus & ID_MASK) >> 5;
	punlunbus = (((bus << 12) | (id << 4)) << ID_SHIFT);

	/*
	 *  Build a Management Request control element to
	 *  manage entity ids.
	 */
	qrev_writel( EID_LENGTH, (uint *)&free_ptr[0] );
	qrev_writel( MANAGEMENT | REQUEST_EID, (uint *)&free_ptr[1] );
	qrev_writel( (ap->adp_uid << 8), (uint *)&free_ptr[2] );
	qrev_writel( (uint)&(ap->adp_pool[i].ctl_blk), (uint *)&free_ptr[3] );
	qrev_writel( (lun | 0x00800000), (uint *)&free_ptr[6] );

	switch ( cmd) {
		case	NDD_ADD_DEV:
			    GET_ID( ap );
                            if (eid_val == 0) {
                                   return(EIO);
                            }
			    ap->dev_eid[eid] = eid_val;
			    qrev_writel(MNGT_ASSIGN_ID,(uint *)&free_ptr[4]);
			    qrev_writel(punlunbus | INITIATOR | eid_val, 
				       (uint *)&free_ptr[5]);
			    action = ALLOC;
			    break;
		case	NDD_DEL_DEV:
			    eid_val = ap->dev_eid[eid];
			    ap->dev_eid[eid] = 0;
			    CLEAR_ENTITY_ID( ap );
			    qrev_writel(MNGT_RELEASE_ID,(uint *)&free_ptr[4]);
			    qrev_writel(punlunbus | INITIATOR | eid_val, 
				       (uint *)&free_ptr[5]);
			    action = FREE;
			    break;
		case	NDD_TGT_ADD_DEV:  
			    GET_ID( ap );
                            if (eid_val == 0) {
                                return(EIO);
                            }
			    ap->tm_dev_eid[eid] = eid_val;
			    qrev_writel(MNGT_ASSIGN_ID, (uint *)&free_ptr[4]);
			    qrev_writel(punlunbus | TARGET | eid_val, 
			    	       (uint *)&free_ptr[5]);
			    action = ALLOC;
			    break;
		case	NDD_TGT_DEL_DEV:  
			    eid_val = ap->tm_dev_eid[eid];
			    ap->tm_dev_eid[eid] = 0;
			    CLEAR_ENTITY_ID( ap );
			    qrev_writel(MNGT_RELEASE_ID, (uint *)&free_ptr[4]);
			    qrev_writel(punlunbus | TARGET | eid_val, 
			    	       (uint *)&free_ptr[5]);
			    action = FREE;
			    break;
		case	NDD_TGT_SUS_DEV:  
			    eid_val = ap->tm_dev_eid[eid];
			    qrev_writel(MNGT_SUSPEND_ID, (uint *)&free_ptr[4]);
			    qrev_writel(punlunbus | TARGET | eid_val, 
			    	       (uint *)&free_ptr[5]);
			    break;
		case	NDD_TGT_RES_DEV:  
			    eid_val = ap->tm_dev_eid[eid];
			    qrev_writel(MNGT_RESUME_ID, (uint *)&free_ptr[4]);
			    qrev_writel(punlunbus | TARGET | eid_val, 
			               (uint *)&free_ptr[5]);
			    break;
		default:
			    (void) asc_get_ctl_blk( ap, FREE, i );
			    return( EINVAL );
	}

	/*
	 *  Activate the command by updating surrogate areas
	 *  and pushing element onto the active list.
	 */
	if(( rc = asc_activate_cmd( ap, free_ptr, EID_LENGTH, i )) != 0 ) {
		(void) asc_get_ctl_blk( ap, FREE, i );
		return( EIO );
	}

	/*
	 *  Kick off a watchdog timer to ensure that the command
	 *  completes in reasonable time. If the timer trips,
	 *  the adapter is assumed dead. 
	 */
	ap->adp_cmd_pending++;
	ap->wdog.dog.restart = SCB_CMD_DURATION;
	ap->wdog.reason = MOVE_TMR;
	w_start( &ap->wdog.dog );

	/*
	 *  If this is a target mode operation, do not sleep.
	 *  The call may be on the interrupt level.
	 */
	if(( cmd == NDD_TGT_SUS_DEV ) || ( cmd == NDD_TGT_RES_DEV )) {
		ap->adp_pool[i].ctl_blk.status |= TARGET_OP;
		return( 0 );
	}

	/*
	 *  Sleep waiting for the command to complete. The sleep 
	 *  event word(ap->eid_event) is used only for this event.
	 */
	e_sleep_thread((int *)&ap->eid_event, &ap->ndd.ndd_demux_lock, 
			LOCK_HANDLER);


	/*
	 *  Check return status in the reply element.
	 */
	rc = asc_parse_reply( ap, &(ap->adp_pool[i].ctl_blk) );
	if( ap->adp_pool[i].status & CMD_TIMED_OUT ) {
		ap->adp_pool[i].status &= ~CMD_TIMED_OUT;
 		asc_log_error( ap, ERRID_SCSI_ERR2, 0,0,0,3 );
		rc = EIO;
	}
	
	/* Return the ctl_elem_blk to the adapter pool. */
	(void) asc_get_ctl_blk( ap, FREE, i );
	
	if ( ! rc ) {
		if( action == ALLOC ) {
			if( eid >= 0x200 )
				ap->devs_in_use_E++;
			else
				ap->devs_in_use_I++;
		}
		else {
			if( action == FREE ) {
				if( eid >= 0x200 )
					ap->devs_in_use_E--;
				else
					ap->devs_in_use_I--;
			}
		}
		return( rc );
	}
	else {	
		if( action == ALLOC ) {
			CLEAR_ENTITY_ID( ap );
		}
		else {
			SET_ENTITY_ID( ap );
		}
		return( EIO );
	}
}

/*
 * NAME:	asc_cleanup
 *
 * FUNCTION: 
 *		This routine cleans up after a failure is 
 *		experienced in the asc_init_adapter routine.
 *
 * EXECUTION ENVIRONMENT:
 *		Process level only.
 *
 * EXTERNAL CALLS:
 *		d_clear, d_complete, w_clear, i_clear, dmp_del, xmfree
 *
 * NOTES:
 *		The cleanup_depth input parameter indicates
 *		how much cleanup is necessary(ie. how far
 *		into asc_init_adapter we got before failing).
 *		If cleanup_depth is 0, cleanup everything.
 *
 * INPUTS:
 *		ap		- adapter structure
 *		cleanup_depth	- amount to cleanup
 *
 * RETURNS:  
 *		none
 */
void
asc_cleanup(
	struct	adapter_info	*ap,
	ulong	cleanup_depth )
{

	if( cleanup_depth == 0 ) 
		cleanup_depth = 9;

	switch( cleanup_depth ) {
		case	9:
			/*
	 		 *  Issue a read immediate request element to 
			 *  unregister for asynchronous notification 
			 *  of SCSI bus resets.
	 		 */
			( void ) asc_rir( ap, DEACTIVATE_RIR );
			
			/*
			 *  Workaround - Adapter does interrupt processing
			 *  out of order. It posts an interrupt before writing
			 *  the surrogate area. Since the host CPU is
			 *  so much quicker than the CPU on the adapter,
			 *  it's possible for the host to service the 
			 *  interrupt and close the adapter(disable
			 *  adapter via POS) before the surrogate area
			 *  is written. When the adapter is reenabled on 
			 *  subsequent open, the adapter attempts to
			 *  complete its write to the surrogate area.
			 *  This is undesirable for many reasons one
			 *  being that the TCW may have been remapped
			 *  since the last open and catastrophe may strike.
			 *  Strategically place a delay() here to allow
			 *  write of surrogate area before disabling
			 *  the adapter.
			 */
			delay(HZ / 10);

		case	8:
			if(( adp_ctrl.num_of_opens == 0 ) || 
			  (( adp_ctrl.num_of_opens == 1) && (ap->opened))) {
				i_clear( &epow );
#ifdef _POWER_MP
				(void) lock_free((void *)&(epow_mp_lock));
#endif
			}
			i_clear( &ap->intr );
			( void ) asc_clear_pos( ap );
		case	7:
			/* 
			 *  May be overly cautious but a d_complete() 
			 *  is necessary because the system will abend
			 *  if we d_clear() and the IO buffer is dirty.
			 */
			d_complete(ap->dma_channel, DMA_NOHIDE, ap->eq_vaddr, 
					PIPESIZE * 2, &ap->xmem, ap->eq_raddr );
			(void) d_clear( ap->dma_channel );
		case	6:
			if(( adp_ctrl.num_of_opens == 0 ) || 
			  (( adp_ctrl.num_of_opens == 1) && (ap->opened))) {
				(void) dmp_del( asc_cdt_func );
			}
		case	5:
			if(( adp_ctrl.num_of_opens == 0 ) || 
			  (( adp_ctrl.num_of_opens == 1) && (ap->opened))) {
				(void) xmfree( asc_cdt, pinned_heap );
			}
		case	4:
			if( cleanup_depth < 8 ) {
				if(( adp_ctrl.num_of_opens == 0 ) || 
			  	(( adp_ctrl.num_of_opens == 1) && (ap->opened))) {
					i_clear( &epow );
#ifdef _POWER_MP
					(void) lock_free((void *)&(epow_mp_lock));
#endif
				}
			}
		case	3:
			if( cleanup_depth < 8 ) {
				i_clear( &ap->intr );
			} 
		case	2:
			(void) xmfree( ap->sysmem, pinned_heap );
		case	1:
#ifdef _POWER_MP
			while(w_clear(&ap->wdog.dog));
			while(w_clear(&ap->tm.dog));
#else
			w_clear( &ap->wdog.dog );
			w_clear( &ap->tm.dog );
#endif
			break;
		default:
			ASSERT(0);
			break;
	}
	ap->opened = FALSE;
	ap->adapter_mode = LOCATE;
	ap->ndd.ndd_flags = NDD_DEAD;
}

/*
 * NAME:	asc_create_bufs
 *		
 * FUNCTION: 
 *		Enable the initial pool of Target mode buffers.
 *
 * EXECUTION ENVIRONMENT:
 *		Process level only.
 *
 * EXTERNAL CALLS:
 *		disable_lock, lock_enable, e_sleep_thread
 *
 * NOTES:
 *
 * INPUT:
 *		ap		- adapter structure
 *		buf_pool_elem	- pointer to a buf_pool_elem(describes buffer).
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *		EIO	- kernel service failure or invalid operation
 */
int
asc_create_bufs(
	struct	adapter_info	*ap,
        int     num_bufs )
{
ulong	ipri;
int	rc;

        ap->num_buf_cmds = 0;
	ap->ebp_flag = FALSE;
	ipri = disable_lock(ap->ddi.int_prior, &(ap->ndd.ndd_demux_lock));
	rc = asc_enable_bufs( ap->tm_buf_info );
	if( rc != 0 ) {
		unlock_enable( ipri, &(ap->ndd.ndd_demux_lock));
		return( EIO );
	}

	/*
	 *  Sleep waiting for the command to complete. The sleep 
	 *  event word(ap->ebp_event) is used only for this event.
	 */
        while( ap->num_buf_cmds )
	    e_sleep_thread((int *)&ap->ebp_event, &ap->ndd.ndd_demux_lock, 
				LOCK_HANDLER);

	if( ap->ebp_flag == TRUE ) {
		ap->ebp_flag = FALSE;
		rc = 0;
	}
	else {
		rc = EIO;
	}
	unlock_enable( ipri, &(ap->ndd.ndd_demux_lock));
	return( rc );
}

/*
 * NAME:	asc_enable_bufs
 *		
 * FUNCTION: 
 *		Issue an Establish Buffer Pool cmd to 
 *		enable buffers at the adapter.
 *
 * EXECUTION ENVIRONMENT:
 *		Process or interrupt level.
 *
 * EXTERNAL CALLS:
 *		None.
 *
 * NOTES:
 *		Corvette has a MAX number of buffers of 320.
 *		Be sure that 320 is the MAX we attempt to enable
 *		in this routine.
 *
 * INPUT:
 *		buf_pool_elem	- pointer to a buf_pool_elem(describes buffer).
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *		EIO	- kernel service failure or invalid operation
 */
int
asc_enable_bufs(
	struct	buf_pool_elem *bp_elem )
{
int	i, rc;
uint	*free_ptr;
int     index;        /* index into adapter pool */
int     buf_start;    /* start of the buffer descriptors */
ulong	length;       /* length in bytes of Establish Buffer Pool Cmd Element */
int     num_enabled;  /* number of buffers in the request to free */
int     descriptor;   /* describes a buffer */
int     num_cmds;     /* maximum number of enable buffer commands to do */
uint    notify_threshold;    /* adapter notification threshold */
struct	buf_pool_elem	*bp;
struct	adapter_info	*ap;

	ap = (struct adapter_info *) bp_elem->adap_dd_info;
	ap->tm_bufs_blocked = FALSE;

	/*
	 * When buffers are received they are placed at the end of a singly
	 * linked list. tm_head and tm_tail are anchors for this list which
	 * is connected via the next pointer. num_free_bufs is a counter of
	 * the number of buffers on this list.
	 */
	if (ap->tm_head == NULL) {
		ASSERT(ap->tm_bufs_to_enable == 0)
		ASSERT(ap->tm_tail == NULL)
		ap->tm_head = bp_elem; 
	}
	else {
		ASSERT( ap->tm_tail->next == NULL );
		ap->tm_tail->next = bp_elem;
	}

	ASSERT(ap->tm_bufs_to_enable >= 0)
	ap->tm_bufs_to_enable++;
	while (TRUE) {
		if(bp_elem->next == NULL) 
	    		break;
		bp_elem = bp_elem->next;
		ap->tm_bufs_to_enable++;
	} 
		ap->tm_tail = bp_elem;
		num_cmds = ap->tm_bufs_to_enable / ap->tm_enable_threshold;
		ASSERT (num_cmds <= 32);

	while (num_cmds && ap->tm_bufs_to_enable > 0) {
		/*
	 	 * First consider the case where a cmd can be built to enable 
	 	 * the maximum number of buffers(10). Each command consists of 
	 	 * a 20 byte header followed by 16 bytes for every buffer freed.
	 	 */
		if (ap->tm_bufs_to_enable >= 10) {
	    		num_enabled = 10;
	    		length = 180; /* 180 bytes required to enbl 10 bufs */
		}
		else { 
	    		num_enabled = ap->tm_bufs_to_enable;
	    		length = 20 + (num_enabled * 16);
		}
			
		/*
	 	 *  Acquire resources necessary to complete an adapter
	 	 *  command. If the outbound pipe is full, the cmd is queued
	 	 *  onto the wait queue. A pointer into the enqueue pipe is
	 	 *  returned if the call succeeds.
	 	 */
		if(( index = asc_get_adp_resources(ap,&free_ptr,length)) < 0 ) {
	    		ap->tm_bufs_blocked = TRUE;
			ap->tm.dog.restart = 1;
			ap->tm.reason = MOVE_TMR;
			w_start( &ap->tm.dog );
	    		return( 0 );
		}

		/*  
	 	 *  Build the Establish buffer pool  element directly on 
	 	 *  the enqueue pipe. 
	 	 */
		qrev_writel(length, (uint *)&free_ptr[0]);
		qrev_writel(ESTABLISH | REQUEST_EID, (uint *)&free_ptr[1]);
		qrev_writel((ap->adp_uid << 8), (uint *)&free_ptr[2]);
		qrev_writel((uint)&(ap->adp_pool[index].ctl_blk), 
			    (uint *)&free_ptr[3]);
		notify_threshold = ((uint)ap->tm_enable_threshold << 16) | 
				     0xFFFF; 
		qrev_writel( notify_threshold, (uint *)&free_ptr[4] );

		/* 
	 	 *  Fill the request element with the
	 	 *  buffer information.
	 	 */
		bp_elem = ap->tm_head;
		buf_start = 5; /* skip over the request header */
		for (i=0; i < num_enabled; i++) {
	    		descriptor = ((ulong)bp_elem - 
				      (ulong)ap->tm_buf_info) << 16;
	    		qrev_writel(descriptor | 0x8100,
				   (uint *)&free_ptr[buf_start++]);
	    		qrev_writel(PAGESIZE, (uint *)&free_ptr[buf_start++]);
	    		qrev_writel(0x0, (uint *)&free_ptr[buf_start++]);
	    		qrev_writel((uint)bp_elem->mapped_addr,
				    (uint *)&free_ptr[buf_start++]);
	    		if( __power_rs() )
				asc_inval(bp_elem->virtual_addr, PAGESIZE);
	    		bp_elem = bp_elem->next;
		}
		qrev_writel(descriptor | 0x0100,
			   (uint *)&free_ptr[buf_start - 4]);

		/*
	 	 *  Activate the command by updating surrogate areas,
	 	 *  pushing element onto the active list and notifying
	 	 *  the adapter that a new element exists.  If this command
	 	 *  fails all the buffers to enable will remain on ap->tm_head
	 	 *  list.
	 	 */
		if((rc = asc_activate_cmd(ap, free_ptr, length, index)) != 0) {
	    		( void ) asc_get_ctl_blk( ap, FREE, index );
	    		return( EIO );
		}
		ap->tm_bufs_at_adp += num_enabled;
		ap->tm_bufs_to_enable -= num_enabled;
		ap->num_buf_cmds++;

		/*  Update the queue pointers. */
		ap->tm_head = bp_elem;
		if( ap->tm_head == NULL )
	    		ap->tm_tail = NULL;

		/*
	 	 *  Kick off a watchdog timer to ensure that the command
	 	 *  completes in reasonable time. If the timer trips,
	 	 *  the adapter is assumed dead. 
	 	 */
		 ap->adp_cmd_pending++;
		 ap->wdog.dog.restart = SCB_CMD_DURATION;
		 ap->wdog.reason = MOVE_TMR;
		 w_start( &ap->wdog.dog );
		 num_cmds--;
	}  /* end while num_cmds != 0 */
	return( 0 );
}

/*
 * NAME:	asc_disable_bufs
 *		
 * FUNCTION: 
 *		Issue a Release Buffer Pool request element
 *		to disable buffers at the adapter.
 *
 * EXECUTION ENVIRONMENT:
 *		Process level only.
 *
 * EXTERNAL CALLS:
 *		disable_lock, lock_enable
 *
 * NOTES:
 *		The Release Buffer Pool request releases ALL
 *		buffers enabled at the adapter.
 *
 * INPUT:
 *		ap		- adapter structure
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *		EIO	- kernel service failure or invalid operation
 */
int
asc_disable_bufs(
	struct	adapter_info	*ap )
{
int	i, rc;
uint	*free_ptr;
ulong	ipri;

	ipri = disable_lock(ap->ddi.int_prior, &(ap->ndd.ndd_demux_lock));
	/*
	 *  Acquire resources necessary to complete an adapter
	 *  command. If the outbound pipe is full, the cmd is queued
	 *  onto the wait queue. A pointer into the enqueue pipe is
	 *  returned if the call succeeds.
	 */
	if(( i = asc_get_adp_resources(ap,&free_ptr,RELEASE_LENGTH)) < 0 ) {
		unlock_enable( ipri, &(ap->ndd.ndd_demux_lock));
		return( EIO );
	}

	/*
	 *  Build the Release Pool Request element. It is nothing
	 *  more than a SCB header with the Release Pool function id.
	 */
	qrev_writel(sizeof(struct scb_header), (uint *)&free_ptr[0]);
	qrev_writel(RELEASE | REQUEST_EID, (uint *)&free_ptr[1]);
	qrev_writel((ap->adp_uid << 8), (uint *)&free_ptr[2]);
	qrev_writel((uint)&(ap->adp_pool[i].ctl_blk), (uint *)&free_ptr[3]);

	/*
	 *  Activate the command by updating surrogate areas,
	 *  pushing element onto the active list and notifying
	 *  the adapter that a new element exists.
	 */
	if(( rc = asc_activate_cmd(ap, free_ptr, RELEASE_LENGTH, i)) != 0 ) {
		( void ) asc_get_ctl_blk( ap, FREE, i );
		unlock_enable( ipri, &(ap->ndd.ndd_demux_lock));
		return( EIO );
	}

	/*
	 *  Kick off a watchdog timer to ensure that the command
	 *  completes in reasonable time. If the timer trips,
	 *  the adapter is assumed dead. 
	 */
	ap->adp_cmd_pending++;
	ap->wdog.dog.restart = SCB_CMD_DURATION;
	ap->wdog.reason = MOVE_TMR;
	w_start( &ap->wdog.dog );

        ap->num_buf_cmds++;
	e_sleep_thread((int *)&ap->ebp_event, &ap->ndd.ndd_demux_lock, 
			LOCK_HANDLER);

	if( ap->ebp_flag == TRUE ) {
		ap->ebp_flag = FALSE;
		rc = 0;
	}
	else {
		rc = EIO;
	}
	ap->tm_bufs_blocked = FALSE;
	ap->tm_head = ap->tm_tail = NULL;
	unlock_enable( ipri, &(ap->ndd.ndd_demux_lock));
	return( rc );
}
/*
 * NAME:	asc_init_pos 
 *
 * FUNCTION: 
 *		Setup the POS registers.
 *
 * EXECUTION ENVIRONMENT:
 *		Process or interrupt level.
 *
 * NOTES:
 *		POS registers cannot experience exceptions so
 *		an exception handler need not be set up when
 *		accessing them. To ensure they are updated
 *		properly, a WRITE-READ-COMPARE sequence will
 *		be performed.
 *
 * CALLED FROM:
 *		asc_init_adapter
 *
 * EXTERNAL CALLS:
 *		io_att, io_det
 *
 * INPUT:
 *		ap	- adapter structure.
 *		ioaddr	- IO handle.
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *		EIO - invalid operation or IO exception on store
 */
int
asc_init_pos(
	struct	adapter_info	*ap )
{
uchar	val, val1;
uchar	id;
uchar	posioaddr;
ulong	ioaddr;
ulong	posaddr;
		
	/* 
	 *  Attach to IO space. 
	 *  Note: The Address Increment and IOCC select bits
	 *  must be set in order to access the POS registers.
	 *  The Address Check, Bypass and Compatible bits are
	 *  don't cares.
	 */
	ioaddr = (ulong) io_att(( ap->ddi.bus_id | 0x800C00E0 ), NULL ); 
	posaddr = 0x00400000 | ( ap->ddi.slot << 16 );

	/* 
	 *  Load pos 6 with zero. 
	 *  This allows access of POS registers 0 - 5.
	 */
	BUS_PUTC( ioaddr + posaddr + POS6, 0X00 );
	val = BUS_GETC(ioaddr + posaddr + POS6 );
	if( val != 0X00 ) {
		io_det( ioaddr );
		return( EIO );
	}

	/*
	 *  Load pos 3 with internal bus card 
	 *  scsi id and arb level. 
	 *  Note: Odd format to the SCSI ID bits for the
	 *  	  internal SCSI bus:
	 *	  bit 3 | bit 0 | bit 1 | bit 2
	 */
	id = ((ap->ddi.i_card_scsi_id << 1) & 0x0E);
	id |= ((ap->ddi.i_card_scsi_id >> 3) & 0x01);
	BUS_PUTC(ioaddr + posaddr + POS3,( ap->ddi.dma_lvl | ( id << 4 )));
	val = BUS_GETC(ioaddr + posaddr + POS3 );
	if( val !=  ( ap->ddi.dma_lvl | ( id << 4 ))) {
		io_det( ioaddr );
		return( EIO );
	}

	/* 
	 *  Load pos 4 with external bus card scsi id, the interrupt 
	 *  level and and control information.
	 */
	BUS_PUTC( ioaddr + posaddr + POS4,
	      ((ap->ddi.int_lvl & 0x01) | 0x4 | (ap->ddi.e_card_scsi_id << 3)));
	val = BUS_GETC(ioaddr + posaddr + POS4 );
	if(val != ((ap->ddi.int_lvl & 0x01) | 0x4 | 
				(ap->ddi.e_card_scsi_id << 3))) {
		io_det( ioaddr );
		return( EIO );
	}

	/* 
	 *  Load pos 5 enabling address parity, streaming, etc.
	 */
	BUS_PUTC( ioaddr + posaddr + POS5, 0XDF );
	val = BUS_GETC(ioaddr + posaddr + POS5 );
	if( val !=  0XDF ) {
		io_det( ioaddr );
		return( EIO );
	}

	/*
  	 *  Setup offset in POS6 to allow access to the extended
	 *  POS registers POS3B and POS4B.
	 */
	BUS_PUTC( ioaddr + posaddr + POS6, 0X01 );
	val = BUS_GETC(ioaddr + posaddr + POS6 );
	if( val !=  0X01 ) {
		io_det( ioaddr );
		return( EIO );
	}

	/* 
	 *  If SCSI wide has been disabled or SCSI Fast has been
	 *  enabled on the external bus, write POS 4B.
	 */
	val = 0x18;
	if( ap->ddi.int_wide_ena == FALSE ) {
		val |= 0x01;
	}
	if( ap->ddi.ext_wide_ena == FALSE ) {
		val |= 0x02;
	}
	if( ap->ddi.ext_bus_data_rate == 10 ) {
		val |= 0x04;
	}
	BUS_PUTC( ioaddr + posaddr + POS4, val );
	val1 = BUS_GETC(ioaddr + posaddr + POS4 );
	if( val !=  val1 ) {
		io_det( ioaddr );
		return( EIO );
	}

	/*
  	 *  Setup offset in POS6 to allow access to the non-extended
	 *  POS registers.
	 */
	BUS_PUTC( ioaddr + posaddr + POS6, 0X00 );
	val = BUS_GETC(ioaddr + posaddr + POS6 );
	if( val !=  0X00 ) {
		io_det( ioaddr );
		return( EIO );
	}

	/* 
	 *  Load pos 2 with the adapter's base IO 
	 *  address and enable the adapter. 
	 */
	switch ( ap->ddi.base_addr ) {
		case	0x3540:
			posioaddr = 0x00;
			break;
		case	0x3548:
			posioaddr = 0x02;
			break;
		case	0x3550:
			posioaddr = 0x04;
			break;
		case	0x3558:
			posioaddr = 0x06;
			break;
		case	0x3560:
			posioaddr = 0x08;
			break;
		case	0x3568:
			posioaddr = 0x0A;
			break;
		case	0x3570:
			posioaddr = 0x0C;
			break;
		case	0x3578:
			posioaddr = 0x0E;
			break;
		default:
			io_det( ioaddr );
			return( EIO );
	}
	BUS_PUTC( ioaddr + posaddr + POS2, (posioaddr | ENABLE_ADAPTER ));
	val = BUS_GETC(ioaddr + posaddr + POS2 );
	if( val != ( posioaddr | ENABLE_ADAPTER )) {
		io_det( ioaddr );
		return( EIO );
	}
#ifdef ASC_TRACE
	asc_trace_ptr = &asc_trace_tab[0];
#endif /* ASC_TRACE */
	io_det( ioaddr );
	return( 0 );
}

/*
 * NAME:	asc_clear_pos 
 *
 * FUNCTION: 
 *		Disable the adapter via the POS register.
 *
 * EXECUTION ENVIRONMENT:
 *		Process or interrupt level.
 *
 * EXTERNAL CALLS:
 *		io_att, io_det
 *
 * INPUT:
 *		ap	- adapter structure.
 *
 * RETURNS:  
 *		none
 */
void
asc_clear_pos(
	struct	adapter_info	*ap )
{
ulong	ioaddr;
ulong	posaddr;

	ioaddr = (ulong) io_att(( ap->ddi.bus_id | 0x800C00E0 ), NULL ); 
	posaddr = 0x00400000 | ( ap->ddi.slot << 16 );
	/* Disable the adapter via POS register 2. */
	BUS_PUTC( ioaddr + posaddr + POS2, 0X00 );
	io_det( ioaddr );
}

/*
 * NAME:	asc_ioctl 
 *
 * FUNCTION: 
 *		This routine provides device control functions.
 *
 * EXECUTION ENVIRONMENT:
 *		Process or interrupt level.
 *
 * EXTERNAL CALLS:
 *		disable_lock, enable_lock
 *
 * NOTES:
 *
 *	NDD_ADD_DEV	- Issues a assign entity Management request element to
 *			  the adapter. This registers a unique SCSI ID/LUN/BUS
 *			  combination to the adapter. Future references to
 *			  that device will be done with an entity id.
 *	NDD_DEL_DEV	- Issues a assign entity Management request element to
 *			  the adapter. This removes the registration of a
 *			  entity id from the adapter.
 *   	NDD_DEL_ALL_DEV - Given a cmd element to resume a suspended 
 *			  entity id, this ioctl will build the appropriate 
 *			  adapter command and issue it to the adapter.
 * **	NDD_TGT_ADD_DEV	- Given a cmd element to assign a target entity id
 *			  this ioctl will build the appropriate adapter command
 *			  and issue it to the adapter. 
 * **	NDD_TGT_DEL_DEV  - Given a cmd element to release a target entitiy id
 *			  this ioctl will build the appropriate adapter command
 *			  and issue it to the adapter. 
 * **	NDD_TGT_SUS_DEV - Given a cmd element to suspend (temporary disable) a
 *			  entity id, this ioctl will build the appropriate 
 *			  adapter command and issue it to the adapter.
 * **	NDD_TGT_RES_DEV - Given a cmd element to resume a suspended 
 *			  entity id, this ioctl will build the appropriate 
 *			  adapter command and issue it to the adapter.
 *	NDD_DUMP_ADDR   - Return the address of the DUMPWRITE entry point.
 *	NDD_RESET_ADP	- Issue a Hardware reset to the adapter.
 *
 *		** These ioctls are used exclusivley to support the 
 *      	   target mode function of the adapter.
 *
 *	The eid_lock is employed to serialize use of the eid_event word
 *	on a per adapter basis.
 *
 * INPUT:
 *		ndd	- ndd structure for this adapter/protocol connection.
 *		cmd	- operation requested
 *		arg	- pointer to cmd arguments
 *		
 * RETURNS:  
 *              0 for good completion,  ERRNO on error
 *              EIO     - kernel service failure or invalid operation
 */
int
asc_ioctl(
	ndd_t	*ndd,
	int	cmd,
	int	arg )
{
struct	adapter_info	*ap;
int     *function_ptr;           /* address to a function */
int	rc = 0;
ulong	ipri;


	/*  
	 *  Get the adapter structure for this adapter
	 *  and verify that it is open.
	 */
	ap = (struct adapter_info *) ndd->ndd_correlator;
	if(( ap == NULL ) || ( ap->opened != TRUE )) {
		return ( EIO );
	}

	switch ( cmd) {
		case	NDD_ADD_DEV:
				/*
				 *  Request to add/register a SCSI initiator 
				 *  mode entity to the adapter.
				 */
		case	NDD_DEL_DEV:
				/*
				 *  Request to delete/unregister a SCSI 
				 *  initiator mode entity from the adapter.
				 */
		case	NDD_TGT_ADD_DEV:  
				/*
				 *  Request to add/register a Automatic SCSI 
				 *  Target mode entity to the adapter.
				 */
		case	NDD_TGT_DEL_DEV:  
                                /*
				 *  Request to delete/unregister a Automatic 
				 *  SCSI Target mode entity to the adapter.
                                 */
		case	NDD_TGT_SUS_DEV:  
                                /*
				 *  Request to temporarily suspend an Automatic
				 *  SCSI Target mode entity from the adapter.
                                 */
		case	NDD_TGT_RES_DEV:  
                                /*
				 *  Request to resume a suspended Automatic
				 *  SCSI Target mode entity from the adapter.
                                 */
				if(( cmd != NDD_TGT_SUS_DEV ) &&
				   ( cmd != NDD_TGT_RES_DEV )) {
	    				(void)lockl(&(ap->eid_lock),LOCK_SHORT);
					ipri = disable_lock(ap->ddi.int_prior, 
						     &(ap->ndd.ndd_demux_lock));
					rc = asc_entity_id( ap, cmd, arg );
					unlock_enable( ipri, 
						&(ap->ndd.ndd_demux_lock));
	        			unlockl(&(ap->eid_lock));
				}
				else {
					rc = asc_entity_id( ap, cmd, arg );
				}
                                break;
                               
		case	NDD_DUMP_ADDR:  
                                /*
				 *  Request the address of the adapter driver's
				 *  dump entry point.
				 *  May want to add max transfer
				 *  size to this IOCTL.
                                 */
				function_ptr = (int *)arg;
				*function_ptr = (int) asc_dump;
                                break;
                               
		case	NDD_RESET_ADP:  
                                /*
				 *  Request to do a hard reset.
                                 */
				rc = asc_reset_adapter( ap );
                                break;
		default:
				rc = EINVAL;
                                break;
	
	}

	return ( rc );
}

/*
 * NAME:	asc_async_status 
 *
 * FUNCTION: 
 *		Send asynchronous notification of status to user.
 *
 * EXECUTION ENVIRONMENT:
 *		Process or interrupt level.
 *
 * NOTES:	
 *
 *		Reasons for asynchronous notification:
 *
 *		NDD_SCSI_BUS_RESET
 *		- Internal bus SCSI bus reset
 *		- External bus SCSI bus reset
 *		NDD_DEV_RESELECT
 *		- Unexpected device reselect on the internal SCSI bus.
 *		- Unexpected device reselect on the external SCSI bus.
 *		NDD_TERM_POWER_LOSS
 *		- Terminator power circuit breaker open on internal SCSI bus.
 *		- Terminator power circuit breaker open on external SCSI bus.
 *		NDD_DIFFERENTIAL_SENSE
 *		- Differential Sense error on internal SCSI bus.
 *		- Differential Sense error on external SCSI bus.
 *		NDD_ADAPTER_RESET
 *		- Completion of entity id assignment of target.
 *		- Completion of entity id removal of target.
 *		
 * INPUT:
 *		ap	- adapter structure
 *		op	- reason for asynchronous status
 *		bus	- bus associated with the status
 *
 * RETURNS:  
 *		none
 */
void
asc_async_status(
	struct	adapter_info	*ap,
	int	op,
	int	bus )
{
struct	ndd_statblk	stat_blk;

	/*
	 *  Fill in the status block and call the protocol's
	 *  async status routine.
	 */
	stat_blk.code = NDD_STATUS;
	stat_blk.option[0] = op;
	if( bus ) {
		if( ap->proto_tag_e == 0 )
			return;
		stat_blk.option[1] = (u_int)ap->proto_tag_e;
	}
	else {
		if( ap->proto_tag_i == 0 )
			return;
		stat_blk.option[1] = (u_int)ap->proto_tag_i;
	}
	if( ap->ndd.nd_status != NULL )
		ap->ndd.nd_status( &ap->ndd, &stat_blk );
}

/*
 * NAME:	asc_wdt_intr
 *
 * FUNCTION: 
 *		Watchdog timer interrupt handler.
 *
 * EXECUTION ENVIRONMENT:
 *		Interrupt level only.
 *
 * EXTERNAL CALLS:
 *		disable_lock, lock_enable, e_wakeup
 *
 * NOTES:
 *		Logic to handle premature timeouts is not necessary
 *		because the watchdog timer is restarted each time
 *		a command is initiated by the adapter driver. The 
 *		commands initiated by the adapter driver are few
 *		and far between ergo a complicated timeout handler
 *		is not necessary.
 *
 * INPUTS:
 *		dog	- watchdog timer structure
 *	
 * RETURNS:  
 *		none
 */
void
asc_wdt_intr(
	struct wtimer *dog)
{
struct	adapter_info	*ap;
struct	buf_pool_elem	*bp;
int	i;
ulong	ipri;

	/* Get the adapter structure. */
	ap = dog->ap;
	ipri = disable_lock(ap->ddi.int_prior, &(ap->ndd.ndd_demux_lock));

	switch( ap->wdog.reason ) {
		case LOCATE_TMR:
		case RESET_TMR:
			ap->locate_state = CMD_TIMED_OUT;
			e_wakeup(&ap->locate_event);
			break;
		case MOVE_TMR:
			/* Adapter command timed-out. */

			/* 
			 *  If it was a target mode op, restart 
			 *  Establish Buffer Pool cmds.
			 */
			if ( ap->tm_bufs_blocked ) {
				bp = ap->tm_head;
				ap->tm_head = ap->tm_tail = NULL;
				ap->tm_bufs_to_enable = 0;
				(void) asc_enable_bufs( bp );
				i_enable( ipri );
				return;
			}

			if( ap->reset_pending == BUS_RESET ) {
				ap->reset_pending = CMD_TIMED_OUT;
				ap->adapter_check = TRUE;
				break;
			}

			/*
			 *  Mark all outstanding adapter commands as
			 *  timed-out.
			 */
			for ( i = 0; i < (NUM_CTRL_ELEMS - 1); i++ ) {
				if(( ap->adp_pool[i].allocated == TRUE ) &&
				   (!(ap->adp_pool[i].ctl_blk.status & 
                                      TARGET_OP))) {
                                 
					ap->adp_pool[i].status |= CMD_TIMED_OUT;
                                }

                                if(ap->adp_pool[i].ctl_blk.status & TARGET_OP){
 		                    asc_log_error( ap, ERRID_SCSI_ERR2, 
                                                  0,0,0,14 );
                                    ap->adp_pool[i].ctl_blk.status &= 
                                                  ~TARGET_OP;
                                }
			} /* end for */
			/*
			 *  An adapter command timed out.
			 *  Wakeup all event words. NOOP
			 *  if event was not sleeping.
			 *  Adapter reset is the only error
			 *  recovery of an adapter command time-out.
			 */
			e_wakeup(&ap->rir_event);
			e_wakeup(&ap->vpd_event);
			e_wakeup(&ap->eid_event);
			ap->num_buf_cmds = 0;
			e_wakeup(&ap->ebp_event);
			if( ap->opened ) {
				asc_clear_q_adp( ap );
				(void) asc_reset_adapter( ap );
			}
			break;
		default:
			/* Log SW error. */
 			asc_log_error(ap,ERRID_SCSI_ERR6,0,0,ap->wdog.reason,4);
			break;
	}
	unlock_enable( ipri, &(ap->ndd.ndd_demux_lock));
}

/*
 * NAME:	asc_get_adp_info
 *
 * FUNCTION: 
 *		Issue a Get Adapter Information command.
 *
 * EXECUTION ENVIRONMENT:
 *		Process level only.
 *
 * NOTES:
 *		Will return adapter information including the Vital
 *		Product Data(VPD) and POS register values.
 *
 * EXTERNAL CALLS:
 *		disable_lock, lock_enable e_sleep_thread, w_start
 *
 * CALLED FROM:
 *		asc_config(CFG_QVPD)
 *
 * INPUT:
 *		ap	- adapter structure
 *
 * RETURNS:  
 *		0 for good completion
 *		EIO - invalid operation
 */
int
asc_get_adp_info(
		struct	adapter_info	*ap,
		struct	ndd_config	*cfg )
{
int	i, rc;
uint	*free_ptr;
ulong	ipri;

	ipri = disable_lock(ap->ddi.int_prior, &(ap->ndd.ndd_demux_lock));

	/*
	 *  Build an Execute Locate Mode Request control 
	 *  element and issue it to the adapter.
	 *
	 *  Acquire resources necessary to complete an adapter
	 *  command. If the outbound pipe is full, the cmd is queued
	 *  onto the wait queue. 
	 */
	if(( i = asc_get_adp_resources( ap, &free_ptr, GETVPD_LENGTH )) < 0 ) {
		unlock_enable( ipri, &(ap->ndd.ndd_demux_lock));
		return( EIO );
	}

	/*
	 *  Build the Execute Locate Mode Request element.
	 */
	qrev_writel(GETVPD_LENGTH, (uint *)&free_ptr[0]);
	qrev_writel(EXECUTE_LOCATE | REQUEST_EID, (uint *)&free_ptr[1]);
	qrev_writel((ap->adp_uid << 8), (uint *)&free_ptr[2] );
	qrev_writel((uint)&(ap->adp_pool[i].ctl_blk), (uint *)&free_ptr[3]);
	qrev_writel(0x00000834, (uint *)&free_ptr[4]);
	qrev_writel(0x0, (uint *)&free_ptr[5]);
	qrev_writel(0x0, (uint *)&free_ptr[6]);
	qrev_writel((uint)ap->sta_raddr, (uint *)&free_ptr[7]);

	/*
	 *  Build the Get POS and Adapter information command.
	 */
	qrev_writes(0x1C0A, (ushort *)&ap->sta_vaddr[0] );
	qrev_writes(0xE200, (ushort *)&ap->sta_vaddr[2] );
	qrev_writel(0x0, (uint *)&ap->sta_vaddr[4] );
	qrev_writel((uint)ap->sta_raddr, (uint *)&ap->sta_vaddr[8] );
	qrev_writel(0xFF, (uint *)&ap->sta_vaddr[12] );
	qrev_writel((uint)(ap->sta_raddr + 256), (uint *)&ap->sta_vaddr[16] );
	qrev_writel(0x0, (uint *)&ap->sta_vaddr[20] );
	qrev_writel(0x0, (uint *)&ap->sta_vaddr[24] );
	vm_cflush((caddr_t) ap->sta_vaddr, 0x64 );

	/*
	 *  Activate the command by updating surrogate areas,
	 *  pushing element onto the active list and notifying
	 *  the adapter that a new element exists.
	 */
	if(( rc = asc_activate_cmd( ap, free_ptr, GETVPD_LENGTH, i )) != 0 ) {
		(void) asc_get_ctl_blk( ap, FREE, i );
		unlock_enable( ipri, &(ap->ndd.ndd_demux_lock));
		return( EIO );
	}

	/*
	 *  Kick off a watchdog timer to ensure that the command
	 *  completes in reasonable time. If the timer trips,
	 *  the adapter is assumed dead. 
	 */
	ap->adp_cmd_pending++;
	ap->wdog.dog.restart = SCB_CMD_DURATION;
	ap->wdog.reason = MOVE_TMR;
	w_start( &ap->wdog.dog );

	/*
	 *  Sleep waiting for the command to
 	 *  complete. The sleep event word
	 *  ( ap->vpd_event ) is used only
	 *  for this event.
	 */
	e_sleep_thread((int *)&ap->vpd_event, &ap->ndd.ndd_demux_lock, 
			LOCK_HANDLER);

	/*
	 *  Check return status in the reply element.
	 */
	rc = asc_parse_reply( ap, &(ap->adp_pool[i].ctl_blk) );

	/* Return the ctl_elem_blk to the adapter pool. */
	(void) asc_get_ctl_blk( ap, FREE, i );
	ap->adp_cmd_pending--;

	if( ap->adp_pool[i].status & CMD_TIMED_OUT ) {
		ap->adp_pool[i].status &= ~CMD_TIMED_OUT;
 		asc_log_error( ap, ERRID_SCSI_ERR2, 0, 0, 0, 5 );
		unlock_enable( ipri, &(ap->ndd.ndd_demux_lock));
                return( EIO );
	}
	unlock_enable( ipri, &(ap->ndd.ndd_demux_lock));

        /*
         *  If get adapter info command was successful
         *  copy the VPD data to the callers uio structure
         *  if it failed, return error.
         */
	if( rc == 0 ) {
		rc = copyout((caddr_t)ap->sta_vaddr + 32,
			     (caddr_t)cfg->p_vpd, cfg->l_vpd );
		if (rc != 0) {
			return( EIO );
		}
		else {
			return( 0 );
		}
	}
	else {
		return( EIO );
	}
}

/*
 * NAME:	asc_log_error 
 *
 * FUNCTION: 
 *		Log an error to the system error log.
 *
 * EXECUTION ENVIRONMENT:
 *		Process level or interrupt level.
 *
 * EXTERNAL CALLS:
 *		errsave, io_att, io_det
 *
 * INPUT:
 *		ap	- adapter structure
 *		errid	- error template label
 *		reply	- ptr to error reply element
 *		pio	- if true, PIO exception, log exception struct
 *		data1	- return code
 *		errnum	- identifier
 *
 * RETURNS:	
 *		void
 */
void
asc_log_error(
	   struct adapter_info * ap,
	   int errid,
	   uchar *reply,
	   int pio,
	   int data1,
	   int errnum )
{
struct 	error_log_def	log;
ulong	posaddr;
ulong	ioaddr;


	bzero((char *)&log, sizeof(struct error_log_def));
	log.errhead.error_id = (uint) errid;
	bcopy((char *) &ap->ddi.resource_name[0],
	      (char *) &log.errhead.resource_name[0],
	      ERR_NAMESIZE);
	log.data.diag_validity = 0;
	log.data.diag_stat = 0;
	if ( reply != NULL) {
		log.data.ahs_validity = 0x01;
		log.data.ahs = (uchar) reply;

		/* 
		 *  bcopy the error reply element into the
		 *  error log.
		 */
		bcopy(reply, (char *)&log.data.un.card3.reply,
		      reply[LEN_FIELD] );

	}
	log.data.un.card3.errnum = (uint) errnum;

	/* 
	 *  Log the exception structure if error was a 
	 *  PIO exception.
	 */
	if( pio ) {
		log.data.un.card3.pio_addr = pio;
		log.data.un.card3.pio_size = data1;
	}

	/* Log the address of the adapter structure. */
	log.data.un.card3.adp_struct = (ulong) ap;

	/* Log the contents of the failing IO register . */
	log.data.un.card3.io_reg = data1;

	/* Log the return code from d_complete. */
	log.data.un.card3.dma_err = data1;
	 
	/* 
	 *  Get POS register data. Directly read the regs here
	 *  to avoid another call to error log routine due to
	 *  error.
	 */
	ioaddr = (ulong) io_att(( ap->ddi.bus_id | 0x800C00E0 ), NULL ); 
	posaddr = ioaddr | 0x00400000 | ( ap->ddi.slot << 16 );

	/* Setup access to standard POS registers. */
	BUS_PUTC( posaddr + POS6, 0X00 );

	log.data.un.card3.pos0_val = BUS_GETC( posaddr + POS0 );
	log.data.un.card3.pos1_val = BUS_GETC( posaddr + POS1 );
	log.data.un.card3.pos2_val = BUS_GETC( posaddr + POS2 );
	log.data.un.card3.pos3_val = BUS_GETC( posaddr + POS3 );
	log.data.un.card3.pos4_val = BUS_GETC( posaddr + POS4 );
	log.data.un.card3.pos5_val = BUS_GETC( posaddr + POS5 );

	/* Setup access to extended POS registers. */
	BUS_PUTC( posaddr + POS6, 0X01 );
	log.data.un.card3.pos3b_val = BUS_GETC( posaddr + POS3 );
	log.data.un.card3.pos4b_val = BUS_GETC( posaddr + POS4 );
	io_det( ioaddr );

	/* log the error here */
	errsave((char *)&log, sizeof(struct error_log_def));
} 

/*
 *
 * NAME:	asc_abort 
 *
 * FUNCTION: 
 *		Issues an abort, BDR or a SCSI bus reset.
 *
 * EXECUTION ENVIRONMENT:
 *		Process or interrupt level.
 *
 * EXTERNAL CALLS:
 *		None.
 *
 * NOTES:
 *		The abort code field indicates whether a BDR, a SCSI bus
 *		reset or an Abort has been requested. BDR message will abort 
 *		all SCSI commands for all LUN's supported by the SCSI device.
 *		An Abort will send a Clear Queue message that will abort 
 *		all SCSI commands that have been queued at the SCSI device.
 *		The abort code field may indicate that a SCSI bus
 *		reset is required. This routine supports resetting one
 *		SCSI bus at a time.
 *
 *		After the issued command completes and before a reply is 
 *		returned, the interrupt handler will remove commands on 
 *		the adapter wait queue that correspond to the bus or
 *		device affected.
 *		
 * CALLED FROM:
 *		asc_output
 *
 * INPUT:
 *		ap		- adapter structure
 *		ctrl_elem	- describes the command and buffers.
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *		EIO	   - kernel service failure or invalid operation
 *		PIPEFULL   - pipe full condition
 */
int
asc_abort(
	struct	adapter_info	*ap, 
	struct	ctl_elem_blk	*ctl_elem )
{
int	rc;
int	abort_code;
int	abort_option;
int	code;
uint	*free_ptr;
uint    *scb;   
uchar   eid_val;
ushort  eid;
ushort  *type2;
ushort	bus = 0;
ulong	punlunbus = 0;
ulong	size;
ulong	options;
ulong	id, lun;
struct	ctl_elem_blk	*tmp_ctl_elem;

	/*
	 *  The ctrl_elem_blk contains a control element 
	 *  that includes the SCSI id/LUN/bus.
	 *  These SCSI id/LUN/bus combinations correspond
	 *  to commands that should be aborted.
	 *  All queued(on the wait queue) requests that
	 *  match the SCSI id/LUN/bus combination should
	 *  be returned with error immediately.
	 *  Commands may reside at the adapter or queued
	 *  at the adapter driver. If any commands are
	 *  currently on our wait queue, then remove any
	 *  queued commands that match the SCSI id/LUN/bus.
	 *
	 *  When the abort completes, commands on the waitq have already
	 *  been cleared. Commands on the active list will either complete
	 *  or be aborted.
	 *
	 *  The cmd/ctl buffer format conforms to the NDD architecture
	 *  and is described in  the SW Interface for High Speed Channel
	 *  Adapter document.
	 */

	/* Get the SCSI command buffer. */
        scb = (uint *)ctl_elem->ctl_elem + (sizeof(struct scb_header) / 
					BYTES_PER_WORD ) + (PD_SIZE * 2 );

	/*
	 *  Get the SCSI id, LUN and bus number.
	 *  Calculate the entity id associated with this device
	 *  based on the SCS  id/LUN/bus information. 
	 *  The bus number is located in the type 2 PD in the ctl_elem.
	 *  The SCSI id and LUN are located in the SCSI command buffer
	 *  also in the ctl_elem block.
	 */
	type2 = (ushort *)ctl_elem->ctl_elem + (sizeof(struct scb_header) / 2);
	id = scb[IDLUN_WORD] >> ID_SHIFT;
	lun = scb[IDLUN_WORD] & LUN_MASK;
	bus = type2[FLAGS_WORD];
	punlunbus = bus << BUS_SHIFT;
	punlunbus |= (id << 5) | lun;
	eid = CALC_ID( punlunbus );
	eid_val = ap->dev_eid[eid];

	/* Determine which operation is requested. */
	abort_code = scb[FLAGS_WORD] & ABORT_CODE_MASK;

	/* Determine if abort msg should be sent to the device. */
	if( scb[FLAGS_WORD] & ABORT_NODEV_MSK ) {
		abort_option = ABORT_NODEV;
	}
	else {
		abort_option = 0;
	}

	/*  
	 *  Clear the queues if an Abort, BDR or reset has been requested.
	 *  Format the SCSI id and LUN to traverse the waitq list. 
	 *  Traverse the wait list returning all affected requests with 
	 *  appropriate error indication.
	 */
	if (( abort_code != 0 ) && ( ap->num_cmds_queued > 0 )) {
		ASSERT( ap->wait_head != NULL );
		tmp_ctl_elem = ap->wait_head;
		while( tmp_ctl_elem != NULL ) {

			if( tmp_ctl_elem->status & ADAPTER_INITIATED ) {
				/*  
			 	 *  This element is an adapter command. 
			 	 *  Leave it on the wait queue.
			 	 */
				tmp_ctl_elem = tmp_ctl_elem->next;
				continue;
			}

			/*  Get the SCSI command buffer. */
        		scb = (uint *)tmp_ctl_elem->ctl_elem + 
					(sizeof(struct scb_header) / 
					 BYTES_PER_WORD) + ( PD_SIZE * 2 );

			/*  Get the type 2 PD that describes the bus. */
			type2 = (ushort *) ctl_elem->ctl_elem + 
					(sizeof(struct scb_header) / 2);

			/*
			 *  Remove all elements on the wait queue that
			 *  match the specified SCSI id/LUN/bus combo.
			 *  If processing a BDR, only the SCSI ID and bus
			 *  need to match(because a BDR affects all LUN's
			 *  for that ID. If processing an ABORT, the SCSI ID,
			 *  LUN and bus all need to match. For a SCSI bus 
			 *  reset, only the bus needs to match.
			 */
			if( abort_code == RESET_CODE ) {
			    if( bus != type2[FLAGS_WORD] ) {
				tmp_ctl_elem = tmp_ctl_elem->next;
				continue;
			    }
			}
			else {
			    if ((id != (scb[IDLUN_WORD] >> ID_SHIFT)) || 
			       (bus != ( type2[FLAGS_WORD] ))) {
				    tmp_ctl_elem = tmp_ctl_elem->next;
				    continue;
			    }

			    if( abort_code != BDR_CODE ) {
				if( lun != (scb[IDLUN_WORD] & LUN_MASK )) {
				    tmp_ctl_elem = tmp_ctl_elem->next;
				    continue;
				}
			    }
			}

			/* 
			 *  Remove the element from the wait queue. 
			 */
			if( ap->wait_head == tmp_ctl_elem ) {
				/* Rmv the 1st element from the list. */
				ap->wait_head = tmp_ctl_elem->next;
				if( ap->wait_head == NULL ) {
					ap->wait_tail = NULL;
				}
				else {
					ap->wait_head->prev = NULL;
				}
			}
			else if( ap->wait_tail == tmp_ctl_elem ) {
				/* Rmv the last element on the list. */
				ap->wait_tail = tmp_ctl_elem->prev;
				ap->wait_tail->next = NULL;
			}
			else {
				/* Rmv an elem from the middle. */
			       tmp_ctl_elem->prev->next = 
						tmp_ctl_elem->next;
			       tmp_ctl_elem->next->prev = 
						tmp_ctl_elem->prev;
			}
			ap->num_cmds_queued--;

			/* 
			 *  Return reply to protocol via recv().
			 *  For consistency sake, fill in the
			 *  reply element with the element id
			 *  of an error element.
			 */
			tmp_ctl_elem->reply_elem[4] = 0xC0;
			tmp_ctl_elem->reply_elem[0x1F] = 0x0;
			tmp_ctl_elem->reply_elem[0x20] = 0x04;
			ap->recv_fn(&ap->ndd,tmp_ctl_elem->reply_elem);
			tmp_ctl_elem = tmp_ctl_elem->next;
		}
	}
		
	/*
	 *  Parse the abort code field to determine if
	 *  an abort, BDR or SCSI bus reset is necessary.
	 *  Note: The Initialize SCSI control element must have the 
	 *  SCSI id/LUN info in it.
	 */
	switch( abort_code ) {
		case	BDR_CODE:
			/* Issue a BDR to a SCSI ID and all LUNS. */
			code = ABORT;
			size = ABORT_LENGTH;
			options = BDR | REACTIVATE;
			break;	
		case	ABORT_CODE:
			code = ABORT;
			size = ABORT_LENGTH;
			options = REACTIVATE | abort_option;
			break;	
		case	RESET_CODE:
			/* Reset one of the SCSI busses. */
			code = INITIALIZE;
			size = INIT_LENGTH;
			eid_val = 0x0;
			if ( bus ) {
				options = EXT_BUS;
        			ctl_elem->status |= ASC_EXTERNAL;
			}
			else {
				options = INT_BUS;
        			ctl_elem->status |= ASC_INTERNAL;
			}
			break;	
		default:	
			/* Caller wants to reactivate the queue. */
			code = REACT;
			size = REACT_LENGTH;
			break;	
	}

	/*
 	 *  Get a pointer to the start of free space on the
 	 *  enqueue pipe. If the element is too large to fit 
 	 *  onto the pipe, a NULL pointer is returned.
  	 */
	rc = asc_get_start_free( ap, size, &free_ptr );
	if (rc != 0) {
		/* 
		 *  Pipe full condition, the element
		 *  will not fit onto the pipe. Push
		 *  the ctl_elem_blk onto the end of the
		 *  wait queue.
		 */
		if ( ap->wait_head == NULL ) {
			ap->wait_head = (struct ctl_elem_blk *)ctl_elem;
			ap->wait_tail =  (struct ctl_elem_blk *)ctl_elem;
			ctl_elem->prev = NULL;
			ctl_elem->next = NULL;
		}
		else {
			ap->wait_tail->next = (struct ctl_elem_blk *)ctl_elem;
			ctl_elem->prev = ap->wait_tail;
			ap->wait_tail = (struct ctl_elem_blk *)ctl_elem;
			ctl_elem->next = NULL;
		}

		/*  Increment the number of q'd commands counter. */
		ap->num_cmds_queued++;
		return( PIPE_FULL );
	}

	/*
	 *  Build the Abort/Initialize SCSI request element.
	 */
	qrev_writel( size, (uint *)&free_ptr[0] );
	qrev_writel( code | REQUEST_EID | EXPEDITED, (uint *)&free_ptr[1] );
	qrev_writel( eid_val | (ap->adp_uid << 8), (uint *)&free_ptr[2] );
	qrev_writel( (uint)ctl_elem, (uint *)&free_ptr[3] );
	if( size > 0x10 ) {
		qrev_writel( options, (uint *)&free_ptr[4] );
	}

	/*
	 *  Notify the adapter that new element(s)
	 *  exist on the pipe and update the queue pointers.
	 */
	rc = asc_update_eq( ap, (uchar *)free_ptr, size );
	if( rc != 0 ) {
		/*  PIO error! Error logged by PIO routine. */
		return( EIO );
	}

#ifdef ASC_TRACE
	asc_trace(ap, ctl_elem, 2); 
#endif /* ASC_TRACE */

	/*
	 *  The element was enqueued onto the pipe successfully.
	 *  Append the command to the end of the adapter send list. 
	 */
	if ( ap->active_head == NULL ) {
		ap->active_head = (struct ctl_elem_blk *)ctl_elem;
		ctl_elem->next = NULL;
		ctl_elem->prev = NULL;
		ap->active_tail = (struct ctl_elem_blk *)ctl_elem;
	}
	else {
		ap->active_tail->next = (struct ctl_elem_blk *) ctl_elem;
		ctl_elem->prev = ap->active_tail;
		ctl_elem->next = NULL;
		ap->active_tail = (struct ctl_elem_blk *) ctl_elem;
	}
	ap->num_cmds_active++;
	return( 0 );
}

/*
 * NAME:	asc_process_adp_elem 
 *
 * FUNCTION: 
 *		Service dequeued elements that were initiated by the
 *		adapter driver.
 *
 * EXECUTION ENVIRONMENT:
 *		Interrupt level only.
 *
 * NOTES:
 *		This routine processes dequeued control elements that
 *		were initiated by the adapter(not the protocol layer).
 *
 * CALLED FROM:
 *		sc_process_elem
 *
 * EXTERNAL CALLS:
 *		e_wakeup
 *
 * INPUT:
 *		ap		- adapter structure
 *		ctrl_elem_blk 	- pointer control element
 */
void
asc_process_adp_elem(
	struct	adapter_info	*ap,
	struct	ctl_elem_blk	*ctl_elem )
{
ulong	rc;
ushort	*reply;
ushort	eid;
struct	adp_ctl_elem	*adp_ctl_elem;

	/* 
	 *  Stop the watchdog timer. 
	 */
	w_stop( &ap->wdog.dog );

	/*
	 *  Determine what kind of adapter initiated command
	 *  this is.
	 */
	reply = (ushort *) ctl_elem->reply_elem;
	eid = reply[3] & 0xFF00;
	switch ( eid ) {
		case	INITIALIZE_REV:
			/*
			 *  Reset or BDR complete.
			 *  DO NOT expect control elements back
			 *  from adapter. 
			 *  Cmds on send and wait lists will be returned
			 *  before the reply is returned to the caller.
			 *  RIR events are not returned for adapter
			 *  initiated SCSI bus resets?
			 */
			if( ctl_elem->status & ASC_INTERNAL ) {
				asc_clear_queues( ap, 0 );
			}
			else if( ctl_elem->status & ASC_EXTERNAL ) {
				asc_clear_queues( ap, 1 );
			}
			else {
				asc_clear_queues( ap, 0 );
				asc_clear_queues( ap, 1 );
			}
			ctl_elem->status &= 0xFFFFF0FF;
			ap->bus_reset_in_prog = FALSE;
			break;

		case	MANAGEMENT_REV:
			/*
			 *  Entity id assigned or removed.
			 */
			ap->adp_cmd_pending--;
			if(ctl_elem->status & TARGET_OP ) {
			        (void) asc_parse_reply(ap, ctl_elem);
				ctl_elem->status &= ~TARGET_OP;
				adp_ctl_elem = (struct adp_ctl_elem *)ctl_elem;
				adp_ctl_elem->allocated = FALSE;
			}
			else {
				e_wakeup(&ap->eid_event);
			}
			break;

		case	ESTABLISH_POOL_REV:
			/*
			 *  Establish Target mode buffer pool.
			 *  Parse reply element. 
			 */
			if((rc = asc_parse_reply(ap, ctl_elem)) == 0) {
				ap->ebp_flag = TRUE;
			}
			else {
				ap->ebp_flag = FALSE;
			}
				
			/* Decr. count and wakeup process. */
			ap->adp_cmd_pending--;
			if( ap->num_buf_cmds ) {
				ap->num_buf_cmds--;
				e_wakeup( &ap->ebp_event );
			}

			/* Mark this adapter ctl_elem_blk as FREE */
			adp_ctl_elem = (struct adp_ctl_elem *) ctl_elem;
			adp_ctl_elem->allocated = FALSE;
			break;

		case	RELEASE_POOL_REV:
			/*
			 *  Release Target mode buffer pool.
			 *  Parse reply element. 
			 */
			if((rc = asc_parse_reply(ap, ctl_elem)) == 0) {
				ap->ebp_flag = TRUE;
			}
			else {
				ap->ebp_flag = FALSE;
			}
				
			/* Decr. count and wakeup process. */
			ap->adp_cmd_pending--;
			if (ap->num_buf_cmds) {
			    ap->num_buf_cmds--;
			    e_wakeup( &ap->ebp_event );
			}

			/* Mark this adapter ctl_elem_blk as FREE */
			adp_ctl_elem = (struct adp_ctl_elem *) ctl_elem;
			adp_ctl_elem->allocated = FALSE;
			break;

		case	CANCEL_REV:
		case	READ_IMMEDIATE_REV:
			if( !ap->mm_reset_in_prog ) {
				e_wakeup( &ap->rir_event );
			}
			else {
				/* Mark this adapter ctl_elem_blk as FREE */
				adp_ctl_elem = (struct adp_ctl_elem *) ctl_elem;
				adp_ctl_elem->allocated = FALSE;

				/* Parse reply element. */
				if((rc = asc_parse_reply(ap, ctl_elem)) != 0)
					ap->adapter_check = TRUE;
				ap->mm_reset_in_prog = FALSE;
				ap->adapter_check = FALSE;
			}
			break;

		case	EXECUTE_LOCATE_REV:
			/*
			 *  Completed a Get Adapter Information
			 *  command. This is the only command
			 *  that is sent via an Execute Locate
			 *  mode command.
			 */
			e_wakeup( &ap->vpd_event );
			break;
		default:
 			asc_log_error( ap, ERRID_SCSI_ERR6, 0,0,eid,11 );
			break;
	}

	/*
	 *  Restart the watchdog timer if necessary.
	 */
	if( ap->adp_cmd_pending > 1 ) {
		ap->wdog.dog.restart = SCB_CMD_DURATION;
		ap->wdog.reason = MOVE_TMR;
		w_start( &ap->wdog.dog );
	}
}

/*
 * NAME:	asc_reset_bus
 *
 * FUNCTION: 
 *		Issue a SCSI bus reset.
 *
 * EXECUTION ENVIRONMENT:
 *		Interrupt level only.
 *
 * NOTES:
 *		Build a Initialize SCSI Request control element 
 *		and issue it to the adapter.
 *
 * CALLED FROM:
 *		asc_epow
 *
 * INPUT:
 *		ap	- adapter structure
 *		bus	- indicates what bus(ses) to reset.
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *		EIO	   - kernel service failure or invalid operation
 *		PIPEFULL   - pipe full condition
 */
int
asc_reset_bus(
	struct	adapter_info	*ap,
	int	bus )
{
int	rc;
uint	*free_ptr;


	/*
 	 *  Get a pointer to the start of free space on the
 	 *  enqueue pipe. If the element is too large to fit 
 	 *  onto the pipe, a NULL pointer is returned.
  	 */
	rc = asc_get_start_free( ap, INIT_LENGTH, &free_ptr );
	if (rc != 0) {
		/* 
		 *  Pipe full condition, the element
		 *  will not fit onto the pipe. Push
		 *  the ctl_elem_blk onto the end of the
		 *  wait queue.
		 */
		if ( ap->wait_head == NULL ) {
			ap->wait_head = (struct ctl_elem_blk *)&adp_pool;
			ap->wait_tail =  (struct ctl_elem_blk *)&adp_pool;
			adp_pool.ctl_blk.prev = NULL;
			adp_pool.ctl_blk.next = NULL;
		}
		else {
			ap->wait_tail->next = (struct ctl_elem_blk *)&adp_pool;
			adp_pool.ctl_blk.prev = ap->wait_tail;
			ap->wait_tail = (struct ctl_elem_blk *)&adp_pool;
			adp_pool.ctl_blk.next = NULL;
		}

		/*  Increment the number of q'd commands counter. */
		ap->num_cmds_queued++;
		return( PIPE_FULL );
	}
	adp_pool.ctl_blk.reply_elem = (caddr_t) &adp_pool.reply;

	/*
	 *  Build the Abort/Initialize SCSI request element.
	 */
	qrev_writel(INIT_LENGTH, (uint *)&free_ptr[0]);
	qrev_writel(INITIALIZE | REQUEST_EID | EXPEDITED,(uint *)&free_ptr[1]);
	qrev_writel(ap->adp_uid << 8, (uint *)&free_ptr[2]);
	qrev_writel((uint)&adp_pool, (uint *)&free_ptr[3]);
	qrev_writel(ASC_BOTH_BUSSES, (uint *)&free_ptr[4]);

	/*
	 *  Notify the adapter that new element(s)
	 *  exist on the pipe and update the queue pointers.
	 */
	rc = asc_update_eq( ap, (uchar *)free_ptr, INIT_LENGTH );
	if (rc != 0) {
		/* 
		 *  Must have been a PIO error.
		 *  Error logged by PIO routines.
		 */
		return( EIO );
	}

	/*
	 *  The element was enqueued onto the pipe successfully.
	 *  Append the command to the end of the adapter send list. 
	 */
	if ( ap->active_head == NULL ) {
		ap->active_head = (struct ctl_elem_blk *)&adp_pool;
		adp_pool.ctl_blk.next = NULL;
		adp_pool.ctl_blk.prev = NULL;
		ap->active_tail = (struct ctl_elem_blk *)&adp_pool;
	}
	else {
		ap->active_tail->next = (struct ctl_elem_blk *) &adp_pool;
		adp_pool.ctl_blk.prev = ap->active_tail;
		adp_pool.ctl_blk.next = NULL;
		ap->active_tail = (struct ctl_elem_blk *) &adp_pool;
	}
	adp_pool.ctl_blk.status |= ( ADAPTER_INITIATED | bus );
	ap->num_cmds_active++;
	ap->bus_reset_in_prog = TRUE;
	return( 0 );
}

/*
 * NAME:	asc_clear_queues
 *
 * FUNCTION: 
 *		Clears the active and wait queues.
 *
 * EXECUTION ENVIRONMENT:
 *		Interrupt level only.
 *
 * NOTES:
 *		Clear the queues returning an error indication
 *		that execution of this command was never attempted.
 *
 * EXTERNAL CALLS:
 *		None.
 * INPUT:
 *		ap	- adapter structure
 * RETURNS
 *		none
 */
void
asc_clear_queues(
	struct	adapter_info	*ap,
	uchar	bus )
{
ushort	*pd;
struct	ctl_elem_blk	*ctl_elem;

	/*
	 *  Traverse the wait list removing all elements
	 *  that are affected by the SCSI bus reset.
	 */
	if ( ap->num_cmds_queued > 0 ) {

		/* Traverse the wait queue list. */
		ctl_elem = ap->wait_head;
		while( ctl_elem != NULL ) {

			if( ctl_elem->status & ADAPTER_INITIATED ) {
				/*  
			 	 *  This element is an adapter command. 
			 	 *  Leave it on the wait queue.
			 	 */
				ctl_elem = ctl_elem->next;
				continue;
			}

			pd = (ushort *) ctl_elem->ctl_elem + 
						(sizeof(struct scb_header) / 2);

			 if( bus == pd[3] ) {
				/* 
				 *  Remove the element from the wait queue. 
				 */
				if( ap->wait_head == ctl_elem ) {
					/* Rmv the 1st element from the list. */
					ap->wait_head = ctl_elem->next;
					if( ap->wait_head == NULL ) {
						ap->wait_tail = NULL;
					}
					else {
						ap->wait_head->prev = NULL;
					}
				}
				else if( ap->wait_tail == ctl_elem ) {
					/* Rmv the last element on the list. */
					ap->wait_tail = ctl_elem->prev;
					ap->wait_tail->next = NULL;
				}
				else {
					/* Rmv an elem from the middle. */
				       ctl_elem->prev->next = ctl_elem->next;
				       ctl_elem->next->prev = ctl_elem->prev;
				}
				ap->num_cmds_queued--;

				/* Release adapter resources. */
				asc_rtov( ap, ctl_elem );

				/* 
				 *  Manually fill in the the reply element
				 *  to simplify the protocol driver's work.
				 */
				ctl_elem->reply_elem[4] = 0xC0;
				ctl_elem->reply_elem[0x1F] = 0x0;
                                if (!(ap->mm_reset_in_prog)) {
			 	 	ctl_elem->reply_elem[0x20] = 0x04;
                                }
                                else {
			 	 	ctl_elem->reply_elem[0x20] = 0x05;
                                }

				/*  Return reply to protocol via recv(). */
				ap->recv_fn(&ap->ndd,ctl_elem->reply_elem);
			}
			ctl_elem = ctl_elem->next;
		}
	}

	/*
	 *  Clear what remains on the active queue.
	 */
	ctl_elem = ap->active_head;
	while( ctl_elem != NULL ) {

		if( ctl_elem->status & ADAPTER_INITIATED ) {
			/*  
		 	 *  This element is an adapter command. 
		 	 *  Leave it on the active queue.
		 	 */
			ctl_elem = ctl_elem->next;
			continue;
		}

		pd = (ushort *) ctl_elem->ctl_elem + 
					(sizeof(struct scb_header) / 2);
		 if( bus == pd[3] ) {
                	asc_sendlist_dq( ap, ctl_elem );
                	ap->num_cmds_active--;
			ctl_elem->reply_elem[4] = 0xC0;
			ctl_elem->reply_elem[0x1F] = 0x0;
                        if (!(ap->mm_reset_in_prog)) {
		 	 	ctl_elem->reply_elem[0x20] = 0x04;
                        }
                        else {
		 	 	ctl_elem->reply_elem[0x20] = 0x05;
                        }

			/* Release adapter resources. */
			asc_rtov( ap, ctl_elem );

			/*  Return reply to protocol via recv(). */
			ap->recv_fn(&ap->ndd, ctl_elem->reply_elem);
		}
		ctl_elem = ctl_elem->next;
	}
}

/*
 * NAME:	asc_clear_q_adp
 *
 * FUNCTION: 
 *		Clears adapter commands from the active and wait queues.
 *
 * EXECUTION ENVIRONMENT:
 *		Interrupt level only.
 *
 * NOTES:
 *		Clear the queues returning an error indication
 *		that execution of this command was never attempted.
 *
 * EXTERNAL CALLS:
 *		None.
 * INPUT:
 *		ap	- adapter structure
 * RETURNS
 *		None.
 */
void
asc_clear_q_adp(
	struct	adapter_info	*ap )
{
struct	ctl_elem_blk	*ctl_elem;
struct	adp_ctl_elem	*adp_ctl_elem;

	/*
	 *  Traverse the wait list removing all elements
	 *  that are affected by the SCSI bus reset.
	 */
	if ( ap->num_cmds_queued > 0 ) {
		/* Traverse the wait queue list. */
		ctl_elem = ap->wait_head;
		while( ctl_elem != NULL ) {
			if( ctl_elem->status & ADAPTER_INITIATED ) {
				/*  Remove the element from the wait queue. */
				if( ap->wait_head == ctl_elem ) {
					/* Rmv the 1st element from the list. */
					ap->wait_head = ctl_elem->next;
					if( ap->wait_head == NULL ) {
						ap->wait_tail = NULL;
					}
					else {
						ap->wait_head->prev = NULL;
					}
				}
				else if( ap->wait_tail == ctl_elem ) {
					/* Rmv the last element on the list. */
					ap->wait_tail = ctl_elem->prev;
					ap->wait_tail->next = NULL;
				}
				else {
					/* Rmv an elem from the middle. */
				       ctl_elem->prev->next = ctl_elem->next;
				       ctl_elem->next->prev = ctl_elem->prev;
				}
				ap->num_cmds_queued--;
				ap->adp_cmd_pending--;
				adp_ctl_elem = (struct adp_ctl_elem *) ctl_elem;
				adp_ctl_elem->allocated = FALSE;
			}
			ctl_elem = ctl_elem->next;
		}
	}

	/*  Clear what remains on the active queue. */
	ctl_elem = ap->active_head;
	while( ctl_elem != NULL ) {
		if( ctl_elem->status & ADAPTER_INITIATED ) {
                	asc_sendlist_dq( ap, ctl_elem );
			ap->num_cmds_active--;
			ap->adp_cmd_pending--;
			adp_ctl_elem = (struct adp_ctl_elem *) ctl_elem;
			adp_ctl_elem->allocated = FALSE;
		}
		ctl_elem = ctl_elem->next;
	}
	e_wakeup(&ap->rir_event);
	e_wakeup(&ap->vpd_event);
	e_wakeup(&ap->eid_event);
	ap->num_buf_cmds = 0;
	e_wakeup( &ap->ebp_event );
}
/*
 * NAME:        asc_reassign_eids
 *
 * FUNCTION:
 *              reassigns all active initiator mode eids following an 
 *              interrupt lvl adapter reset.
 *
 * EXECUTION ENVIRONMENT:
 *              Interrupt level only.
 *
 * NOTES:
 *
 * EXTERNAL CALLS:
 *              None.
 * INPUT:
 *              ap      - adapter structure
 * RETURNS
 *              None.
 */
void
asc_reassign_eids(
        struct  adapter_info    *ap )
{
int	i, lun, cmd;
uint	*free_ptr;
ushort  eid;
uchar	bus, id, eid_val, rc, 
        need_to_time=FALSE;
ulong   punlunbus;


    for (i=0; i < 0x400; i++) {
        /* a non zero value indicates an active eid */
        if (ap->dev_eid[i] != 0) {

            /* acquire resources for the establish eid command.             */
            /* NOTE : There is no support for the condition where resources */
            /* are not availible.  If either pipe space or adapter control  */
            /* blocks are not availible, then the entity id will not be     */
            /* reassigned and the corresponding device will be unavailible  */

            cmd = asc_get_adp_resources( ap, &free_ptr, EID_LENGTH );
            if( cmd < 0 ) {
                    break;
            }
            /* create the punlunbus value from the index i */
            bus = (i & 0x200);
            lun = (i & 0x1F);
            id = ((i >> 5) & 0xF);
            eid_val = ap->dev_eid[i];
            punlunbus = (((bus << 12) | (id << 4)) << ID_SHIFT);
            /* Build a Management Request control element to establish */
            /*  entity id.                                             */
            qrev_writel(EID_LENGTH, (uint *)&free_ptr[0]);
            qrev_writel(MANAGEMENT | REQUEST_EID, (uint *)&free_ptr[1]);
            qrev_writel((ap->adp_uid << 8), (uint *)&free_ptr[2]);
            qrev_writel((uint)&(ap->adp_pool[cmd].ctl_blk),
			(uint *)&free_ptr[3]);
            qrev_writel(MNGT_ASSIGN_ID, (uint *)&free_ptr[4]);
            qrev_writel(punlunbus | INITIATOR | eid_val, (uint *)&free_ptr[5]);
            qrev_writel((lun | 0x00800000), (uint *)&free_ptr[6]);
            /* Activate the command by updating surrogate areas */
            /* and pushing element onto the active list.        */
            if((rc = asc_activate_cmd(ap, free_ptr, EID_LENGTH, cmd)) != 0 ) {
                (void) asc_get_ctl_blk( ap, FREE, cmd );
                return;
            }
            /* mark this ctl_blk as a TARGET_OP so that it is freed on the */
            /* interrupt level and the e_wakeup is not done.               */
            ap->adp_pool[cmd].ctl_blk.status |= TARGET_OP;

            need_to_time = TRUE;
            ap->adp_cmd_pending++;

        } /* end if ap->dev_eid[i] != 0 */

    } /* end for */
 
    if (need_to_time) {
        ap->wdog.dog.restart = SCB_CMD_DURATION;
        ap->wdog.reason = MOVE_TMR;
        w_start( &ap->wdog.dog );
    }   

}


/*
 * NAME:	asc_epow
 *
 * FUNCTION: 
 *		Services an Early Power Off Warning(EPOW) interrupt.
 *
 * EXECUTION ENVIRONMENT:
 *		Interrupt level only.
 *
 * NOTES:
 *		There is a single EPOW handler defined for all SCSI adapters
 *		under control of this adapter driver. This was chosen over
 *		separate handlers in the interest of speed in handling the
 *		EPOW.                                                    
 *
 * EXTERNAL CALLS:
 *		disable_lock, lock_enable
 *
 * INPUTS:
 *		intr	- the epow intr structure
 *
 * RETURNS:  
 *		INTR_SUCC
 */
int
asc_epow(
	struct	intr	*intr)
{
int	i;
ulong	pri, ipri;
struct	adapter_info	*ap;

	pri = disable_lock( INTEPOW, &(epow_mp_lock));

	/* Determine type of EPOW interrupt. */
	if((intr->flags & EPOW_SUSPEND) || (intr->flags & EPOW_BATTERY)) {
		/* 
		 *  Power has been suspended.
		 *  Walk the adapter structures. If devices are in
		 *  use on an adapter, reset the associated bus.
		 */
		for (i = 0; i < MAX_ADAPTERS; i++) {
		    if((ap = adp_ctrl.ap_ptr[i]) != NULL) {

			if( ap->adapter_mode == LOCATE ) {
				/*
				 *  No commands active for this
				 *  adapter because the adapter
				 *  has yet to be initialized.
				 */
				continue;
			}

			if((( ap->devs_in_use_I ) || ( ap->devs_in_use_E )) &&
					( ap->epow_state != EPOW_PENDING )) {

                    	   if ((intr->flags & EPOW_SUSPEND) ||
                        		((intr->flags & EPOW_BATTERY) &&
                         		(!(ap->ddi.battery_backed)))) {

				ap->epow_state = EPOW_PENDING;

				/*
				 *  Reset the SCSI bus(ses).
				 */
				if((ap->devs_in_use_I) && (ap->devs_in_use_E)) {
					asc_reset_bus(ap, ASC_BOTH_BUSSES );
				}
				else {
					if(ap->devs_in_use_I)
						asc_reset_bus(ap, ASC_INTERNAL);
					else
						asc_reset_bus(ap, ASC_EXTERNAL);
				}
			    }
			}
		    }
		}
	}
	else {
	    if ( intr->flags & EPOW_RESUME ) {
	        /* 
	         *  Power has been resumed.
	         */
		if (((!(intr->flags & EPOW_BATTERY)) &&
		     (!(intr->flags & EPOW_SUSPEND)))) {
		    for (i = 0; i < MAX_ADAPTERS; i++) {
		        if((ap = adp_ctrl.ap_ptr[i]) != NULL) {
		    	    if (ap->epow_state == EPOW_PENDING) {
			        ap->epow_state = EPOW_OFF;
			        ipri = disable_lock(ap->ddi.int_prior, 
			    			  &(ap->ndd.ndd_demux_lock));
			    	asc_start( ap );
			    	unlock_enable(ipri, &(ap->ndd.ndd_demux_lock));
			     }
			 }
		     }
		 }
	    }
	}
	unlock_enable( INTEPOW, &(epow_mp_lock));
	return( INTR_SUCC );
}	

/*
 * NAME:	asc_get_adp_resources 
 *
 * FUNCTION: 
 *		Acquire resources necessary for an adapter command.
 *
 * EXECUTION ENVIRONMENT:
 *		Process level with interrupts disabled.
 *
 * NOTES:
 *		Get a ctrl_elem from the adapter pool and reserve
 *		space on the outbound pipe. If the
 *		pipe is full, queue the ctrl_elem onto the 
 *		wait queue.
 *
 *		Note: It is assumed the caller always calls this routine
 *		with interrupts disabled.
 *
 * EXTERNAL CALLS:
 *		e_sleep_thread
 *
 * INPUT:
 *		ap	- adapter structure
 *		size	- size(in bytes) of the request.
 *		eid	- entity id to add/del
 *
 * RETURNS:  
 *		index	- index of the ctl_elem_blk
 *		-1	- if unable to fit the element onto the pipe
 *		-2	- if unable to allocate a ctl_elem_blk
 */
int
asc_get_adp_resources(
	struct	adapter_info	*ap,
	uint	**free_ptr,
	int	size )
{
int	rc, i;
uint	*freep;


	/*  Acquire a ctl_elem_blk from the adapter pool. */
	i = asc_get_ctl_blk( ap, ALLOC, 0 ); 
	if( i == ( -1 )) {
		return( -2 );	
	}

	/*
	 *  Get a pointer to the start of free space on the
	 *  enqueue pipe. If the element is too large to fit 
	 *  onto the pipe, a NULL pointer is returned.
	 */
	rc = asc_get_start_free( ap, size, &freep );
	if (rc == 0) {
		*free_ptr = freep;
		return( i );
	}
	else {
		return( -1 );
	}
}

/*
 * NAME:	asc_activate_cmd 
 *
 * FUNCTION: 
 *		Activate a command by updating surrogate areas
 *		and moving from the wait to active queue.
 *
 * EXECUTION ENVIRONMENT:
 *		Process or interrupt level.
 *
 * NOTES:
 *		Free a ctrl_elem from the adapter pool and update
 *		update the surrogate areas.
 *		
 * INPUT:
 *		ap	- adapter structure
 *		free	- pointer to SOF of enqueue pipe
 *		size	- size(in bytes) of the request.
 *		i	- index of adp_ctl_blk
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *		EIO	- kernel service failure or invalid operation
 */
int
asc_activate_cmd(
	struct	adapter_info	*ap,
	uint	*free_ptr,
	int	size,
	int	i )
{
int	rc;

	/*
	 *  Update the surrogate and local control areas.
	 *  Notify the adapter that new element(s)
	 *  exist on the pipe and update the queue pointers.
	 */
	rc = asc_update_eq( ap, (uchar *)free_ptr, size );
	if (rc != 0) {
		return( EIO );
	}

	/*
	 *  The element was enqueued onto the pipe successfully.
	 *  Append the command to the end of the adapter send list. 
	 */
	if ( ap->active_head == NULL ) {
		ap->active_head = &(ap->adp_pool[i].ctl_blk);
		ap->adp_pool[i].ctl_blk.next = NULL;
		ap->adp_pool[i].ctl_blk.prev = NULL;
		ap->active_tail = &(ap->adp_pool[i].ctl_blk);
	}
	else {
		ap->active_tail->next = &(ap->adp_pool[i].ctl_blk);
		ap->adp_pool[i].ctl_blk.prev = ap->active_tail;
		ap->adp_pool[i].ctl_blk.next = NULL;
		ap->active_tail = &(ap->adp_pool[i].ctl_blk);
	}
	ap->num_cmds_active++;
	return( 0 );
}

/*
 * NAME:	asc_stub
 *
 * FUNCTION: 
 *		Used to initialize function pointers.
 *
 * NOTES:
 *		The use of a stub for function pointers prevents
 *		fatal results when a NULL function pointer is used.
 *
 * RETURNS:	
 *		none
 */
void
asc_stub(
	ndd_t   *ndd,
	ulong	var )
{
 	asc_log_error((struct adapter_info *)ndd->ndd_correlator, 
			ERRID_SCSI_ERR6,0,0,var,17 );
}

/*
 * NAME:	asc_reset_adapter 
 *
 * FUNCTION: 
 *		Issue a hard reset to the adapter.
 *
 * EXECUTION ENVIRONMENT:
 *		Process level or interrupt level
 *
 * EXTERNAL CALLS:
 *		io_att, io_det
 *
 * NOTES:	
 *		Reinitializes data structures.
 *		Recreates the delivery pipes.
 *		After resetting the adapter, all previously
 *		assigned entity ids must be registered and 
 *		pipe pointers reinitialized.
 *
 * INPUT:
 *		ap	- adapter structure
 *
 * RETURNS:	
 *		0	- on success
 *		EIO	- on error
 */
int	    
asc_reset_adapter(
	struct	adapter_info	*ap )
{
int	rc;
ulong	ioaddr;
uchar	pio_error = 0;


	/* Cleanup any DMA errors. */
	(void) d_complete( ap->dma_channel, DMA_NOHIDE, ap->sta_vaddr, 
	 		   PIPESIZE, &ap->xmem, ap->sta_raddr );
		

	/*  Initialize the POS registers. */
	(void) asc_clear_pos( ap );
	if(( rc = asc_init_pos( ap )) != 0 ) {
		return( EIO );
	}

	/* Adapter is operating in locate mode. */
	ap->adapter_mode = LOCATE;
	ap->locate_state = RESET_PENDING;
	ap->mm_reset_in_prog = TRUE;

	/* Attach to IO space. */
	ioaddr = (ulong) io_att( ap->ddi.bus_id, NULL ); 

	/*
	 *  Reset the adapter(use the subsystem reset bit in the BCR).
	 *  Toggle the reset bit in the BCR.
	 */
	WRITE_BCR( ap->ddi.base_addr, BCR_RESET);
	if( pio_error ) {
		io_det( ioaddr );
		return( EIO );
	}
  	WRITE_BCR( ap->ddi.base_addr, 0x00);
	if( pio_error ) {
		io_det( ioaddr );
		return( EIO );
	}
	WRITE_BCR( ap->ddi.base_addr, BCR_ENABLE_INTR | BCR_ENABLE_DMA ); 
	if( pio_error ) {
		io_det( ioaddr );
		return( EIO );
	}

	/* Detach IO handle */
	io_det( ioaddr );
	return( 0 );
}

/*
 * NAME:	asc_get_ctl_blk 
 *
 * FUNCTION:	Acquires a adp_ctl_blk from the adapter pool.
 *
 * EXECUTION ENVIRONMENT:
 *		Interrupt level only.
 *
 * NOTES:	
 *		adp_ctl_blk's are used to manage adapter
 *		initiated commands.
 *
 *		Note: It is assumed the caller always calls this routine
 *		with interrupts disabled.
 *		
 *
 * INPUT:
 *		ap	- adapter structure
 *		cmd	- indicates allocate or free option
 *		index	- index of element to free
 *
 * RETURNS:  
 *		-1	-  none available if ALLOC request
 *		nonzero -  index into the pool if successful(on ALLOC)
 *		0 	-  if the element was freed successfully(on FREE)
 */
int
asc_get_ctl_blk(
	struct	adapter_info	*ap,
	int	cmd,
	int	index )
{
int	i;

	if( cmd == ALLOC ) {
		for ( i = 0; i < (NUM_CTRL_ELEMS - 1); i++ ) {
			if( ap->adp_pool[i].allocated == FALSE ) {
				ap->adp_pool[i].allocated = TRUE;
				return( i );
			}
		}
		return( -1 );
	}
	else {
		ASSERT( cmd == FREE );
		ASSERT(( index >= 0 ) && ( index <= (NUM_CTRL_ELEMS - 1 ))); 
		ap->adp_pool[index].allocated = FALSE;
		return( 0 );
	}
}

/*
 * NAME:	asc_process_locate 
 *
 * FUNCTION: 
 *		Services Locate mode interrupts.
 *
 * EXECUTION ENVIRONMENT:
 *		Interrupt level only.
 *
 * NOTES:
 *		When the adapter is operating in locate mode,
 *		interrupts are serviced differently. The command
 *		completion status is located in the Interrupt Status
 *		register. The interrupt must be explicitly cleared(EOI'd)
 *		at the adapter. No pipes or queues exist.
 *
 *		The adapter driver operates in locate mode only
 * 		during adapter startup( initial resets and pipe 
 *		configuration).
 *
 * EXTERNAL CALLS:
 *		e_wakeup, w_stop
 *  		
 * INPUT:
 *		ap	- adapter structure
 *		ioaddr  - IO handle
 *
 * RETURNS:  
 *		none
 */
void
asc_process_locate(
		struct	adapter_info	*ap,
		ulong	ioaddr )
{
int	rc;
static	int	start_time;
char	isr;	
uchar	val, val1;
uchar	pio_error = 0;
ulong	posaddr, ioccaddr;

	/*  Stop the watchdog timer. */
	w_stop( &ap->wdog.dog );

	/* 
	 *  Read the Interrupt Status Register(ISR)
	 *  to determine type of interrupt.
	 */
	READ_ISR( ap->ddi.base_addr, &isr );
	if( pio_error == TRUE ) {
		if( ap->mm_reset_in_prog ) {
			ap->adapter_check = TRUE;
			return;
		}
		else if(( rc = asc_reset_adapter( ap )) != 0 ) {
			ap->adapter_check = TRUE;
			return;
		}
		return;
	}

	if( ap->locate_state == RESET_PENDING ) {
		if( isr == ISR_RESET_COMPLETE ) {
			/*  The adapter completed a HW reset. */
			ap->locate_state = RESET_COMPLETE;
		}
		else if(( isr == 0x8F ) || ( isr == 0xAF )) {
			/*
			 *  Internal or external SCSI bus interface
			 *  test failed during post.
			 *  This logic implemented to prevent adapter
			 *  from resetting the SCSI busses on its own.
			 *  If post test continues to fail for 30 seconds,
			 *  alter POS bit(POS4b) to allow the adapter
			 *  to reset the SCSI bus.
			 */
			ATTENTION( ap->ddi.base_addr, (0xE0 | (isr & 0x0F)));
	        	curtime(&ap->time_s);
			if( ap->first_try == TRUE ) {
				start_time = (long int) ap->time_s.tv_sec;
				ap->first_try = FALSE;
			}

			if(((long int)ap->time_s.tv_sec - start_time) > 30) {
				ioccaddr = (ulong) io_att(( ap->ddi.bus_id | 
					 0x800C00E0 ), NULL );
				posaddr = 0x00400000 | ( ap->ddi.slot << 16 );
				/* Setup access to extended POS registers. */
				BUS_PUTC( ioccaddr + posaddr + POS6, 0X01 );
				val = BUS_GETC(ioccaddr + posaddr + POS6 );
				if( val !=  0x01 ) {
				  ap->locate_state = HW_FAILURE;
				  ap->sleep_pending = FALSE;
				  e_wakeup( &ap->locate_event );
 				  asc_log_error(ap,ERRID_SCSI_ERR2,0,0,isr,13);
				  io_det( ioccaddr );
				  return;
				}
				val1 = BUS_GETC(ioccaddr + posaddr + POS4);
				val = val1 & 0xCF;
				BUS_PUTC( ioccaddr + posaddr + POS4, val );
				val1 = BUS_GETC(ioccaddr + posaddr + POS4 );
				if( val1 != ( val & 0xCF )) {
				  ap->locate_state = HW_FAILURE;
				  ap->sleep_pending = FALSE;
				  e_wakeup( &ap->locate_event );
 				  asc_log_error(ap,ERRID_SCSI_ERR2,0,0,isr,13);
				  io_det( ioccaddr );
				  return;
				}
				io_det( ioccaddr );

				if(((long int)ap->time_s.tv_sec - start_time) >
					      40) {
			    	  ap->locate_state = HW_FAILURE;
				  ap->sleep_pending = FALSE;
				  e_wakeup( &ap->locate_event );
 				  asc_log_error(ap,ERRID_SCSI_ERR10,0,0,isr,12);
			    	  return;
				}
			}

			WRITE_BCR( ap->ddi.base_addr, BCR_RESET);
			if( pio_error ) {
				ap->locate_state = HW_FAILURE;
				return;
			}
			WRITE_BCR( ap->ddi.base_addr, 0x00);
			if( pio_error ) {
				ap->locate_state = HW_FAILURE;
				return;
			}

			WRITE_BCR( ap->ddi.base_addr, BCR_ENABLE_INTR | 
				   BCR_ENABLE_DMA ); 
			if( pio_error ) {
				ap->locate_state = HW_FAILURE;
				return;
			}
			ap->wdog.dog.restart = RESET_DURATION;
			ap->wdog.reason = RESET_TMR;
			w_start( &ap->wdog.dog );
			return;
		}
		else {
			if( isr == 0x3F )
 				asc_log_error(ap,ERRID_SCSI_ERR10,0,0,isr,12);
			else
 				asc_log_error(ap,ERRID_SCSI_ERR1,0,0,isr,10);
			ap->locate_state = HW_FAILURE;
		}

		/*
		 *  Issue an End Of Interrupt(EOI) signal to
		 *  the adapter. This is done by setting the 
		 *  EIO bit in the Attention register.
		 *  NOTE: 0x0F indicates the device when resetting
		 *  the adapter.
		 */
		ATTENTION( ap->ddi.base_addr, (0xE0 | (isr & 0x0F)));

		/*  
		 *  Determine if the reset was initiated
		 *  for adapter bringup or for error
		 *  recovery.
		 */
		if( ! ap->mm_reset_in_prog ) {
			ap->sleep_pending = FALSE;
			e_wakeup( &ap->locate_event );
		}
		else {
			(void) asc_init_pipes_int( ap, ioaddr );
		}
	}
	else {
		/*
		 *  The only way we should get here is a result of
		 *  completing a Management Request element to 
		 *  configure the pipes. Flush the IO buffer so
		 *  we can verify the completion status.
		 */
		ASSERT( ap->locate_state == SCB_PENDING );

		/* DMA completion processing. */
		rc = d_complete(ap->dma_channel, DMA_NOHIDE, ap->sta_vaddr, 
				PIPESIZE, &ap->xmem, ap->sta_raddr );

		/* Acknowledge the interrupt to the adapter. */
		ATTENTION( ap->ddi.base_addr, 0xE0 );

		if( rc ) {
			ap->locate_state = HW_FAILURE;
			ap->adapter_check = TRUE;
 			asc_log_error( ap, ERRID_SCSI_ERR2, 0,0,rc,6);
			e_wakeup(&ap->locate_event);
		}
		else {
			ap->locate_state = SCB_COMPLETE;
			if( ! ap->mm_reset_in_prog ) {
				e_wakeup(&ap->locate_event);
				return;
			}

			/*
			 *  Initialize the local control areas
			 *  for both pipes.
			 */
			ap->local.eq_sf = ap->eq_vaddr;
			ap->local.eq_end = ap->eq_vaddr + (PIPESIZE - 1 );
			ap->local.eq_se = ap->local.eq_end; 
			ap->local.eq_top = ap->local.eq_end - WRAP_SIZE;
			ap->local.eq_ef = ap->local.eq_top;
			ap->local.eq_status = EQ_PIPE_EMPTY;
			ap->local.eq_wrap = 0;
			ap->local.dq_wrap = 0;
			ap->local.dq_ee = ap->dq_vaddr;
			ap->local.dq_se = ap->dq_vaddr;
			ap->local.dq_end =ap->dq_vaddr + (PIPESIZE - 1 );
			ap->local.dq_top =  ap->local.dq_end - WRAP_SIZE;
			ap->local.dq_status = 0;

			/*  Clear active and wait queues. */
			asc_clear_queues( ap, 0 );
			asc_clear_queues( ap, 1 );
			asc_clear_q_adp( ap );
			ASSERT( ap->active_head == NULL );
			ASSERT( ap->active_tail == NULL );
			ASSERT( ap->wait_head == NULL );
			ASSERT( ap->wait_tail == NULL );
			ASSERT( ap->num_cmds_queued == 0 );
			ASSERT( ap->num_cmds_active == 0 );

			ap->adapter_mode = MOVE;
			ap->bus_reset_in_prog = FALSE;
			ap->adapter_check = FALSE;

        		WRITE_BCR(ap->ddi.base_addr, BCR_ENABLE_INTR | 
					BCR_ENABLE_DMA | BCR_CLR_ON_READ);
			/*
			 *  Be sure asc_cleanup does not attempt
			 *  to call asc_rir here. Can't do a delay()
			 *  on an interrupt level.
			 */
			if( pio_error ) {
				ap->adapter_check = TRUE;
				asc_cleanup( ap, 8 );
				io_det( ioaddr );
				return;
			}

			if(( rc = asc_rir_int( ap )) != 0 ) {
				ap->adapter_check = TRUE;
			}

                        /* attempt to reassign the initiator enitity ids */
                        /* following this interrupt lvl adapter reset.   */
                        /* target mode eids and buffer pools is not      */
                        /* reassigned and will be handled by any error   */
                        /* recovery by the above layers.                 */
                        (void) asc_reassign_eids(ap);

			/* Wakeup all events */
			e_wakeup(&ap->rir_event);
			e_wakeup(&ap->vpd_event);
			e_wakeup(&ap->eid_event);
			ap->num_buf_cmds = 0;
			e_wakeup( &ap->ebp_event );
		}
	}
	return;
}

/*
 * NAME:	asc_rir_int
 *
 * FUNCTION: 
 *		Register a read immediate request with the adapter
 *		on the interrupt level.
 *
 * EXECUTION ENVIRONMENT:
 *		Interrupt level only.
 *
 * EXTERNAL CALLS:
 *		vm_cflush
 *
 * CALLED FROM:
 *		asc_process_adp_elem
 *
 * INPUT:
 *		ap	- adapter structure
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *		EIO	- kernel service failure or invalid operation
 */
int
asc_rir_int(
	struct	adapter_info	*ap )
{
int	i, rc;
uint	*free_ptr;

	/*
	 *  See comment in asc_rir() about buffer flush. 
	 */
	for( i = 0; i < 1000; i++ );
	rc = d_complete(ap->dma_channel, DMA_NOHIDE,
			(uchar *)ap->surr_ctl.eq_ssf, 
			PIPESIZE, &ap->xmem,
			(uchar *)ap->surr_ctl.eq_ssf_IO );
	/*
	 *  Build a Read Immediate Request control element 
	 *  and issue it to the adapter.
	 *  Build the request element directly onto the 
	 *  outbound pipe.
	 *
	 *  Acquire resources necessary to complete an adapter
	 *  command. If the outbound pipe is full, the cmd is queued
	 *  onto the wait queue. A pointer into the enqueue pipe is
	 *  returned if the call succeeds.
	 */
	 if(( i = asc_get_adp_resources( ap, &free_ptr, RIR_LENGTH )) < 0 ) {
		return( EIO );
	}

	/*  Build the request element directly on the enqueue pipe. */
	/*
	 *  Build the Read Immediate Request element. It is nothing
	 *  more than a SCB header with the Read Immediate function id.
	 */
	qrev_writel(sizeof(struct scb_header), (uint *)&free_ptr[0]);
	qrev_writel(READ_IMMEDIATE | REQUEST_EID, (uint *)&free_ptr[1]);
	qrev_writel((ap->adp_uid << 8), (uint *)&free_ptr[2]);
	qrev_writel((uint)&(ap->adp_pool[i].ctl_blk), (uint *)&free_ptr[3]);
	vm_cflush( (caddr_t)free_ptr, RIR_LENGTH );

	/*
	 *  Activate the command by updating surrogate areas,
	 *  pushing element onto the active list and notifying
	 *  the adapter that a new element exists.
	 */
	if(( rc = asc_activate_cmd( ap, free_ptr, RIR_LENGTH, i )) != 0 ) {
		(void) asc_get_ctl_blk( ap, FREE, i );
		return( EIO );
	}

	return( 0 );
}

/*
 * NAME:	asc_init_pipes_int
 *
 * FUNCTION: 
 *		Setup the move mode delivery pipes on the
 *		 interrupt level.
 *
 * EXECUTION ENVIRONMENT:
 *		Interrupt level only.	
 *
 * EXTERNAL CALLS:
 *		vm_cflush
 *
 * CALLED FROM:
 *		asc_process_locate
 *
 * INPUT:
 *		ap	- adapter structure
 *		ioaddr  - IO handle
 *
 * RETURNS:  
 *		EINVAL    - invalid request
 *		EIO       - kernel service failure
 */
int
asc_init_pipes_int( 
	struct	adapter_info	*ap,
	ulong	ioaddr )
{
ulong	i;
uchar	pio_error = 0;
struct	cfg_mre	*mre; 
volatile uchar	value;

	/* 
	 *  Map delivery pipes and surrogate area for DMA.
	 *  Likely that the pipes are already mapped but
	 *  adapter reset could have occurred before pipes
	 *  are initially configured.
	 */
	ap->xmem.aspace_id = XMEM_GLOBAL;
	d_master(ap->dma_channel, DMA_NOHIDE, ap->eq_vaddr, PIPESIZE * 2,
			&ap->xmem, ap->eq_raddr );
	d_master(ap->dma_channel, DMA_NOHIDE, (char *)ap->surr_ctl.eq_ssf,
		 CONTROL_SIZE, &ap->xmem, (char *)ap->surr_ctl.eq_ssf_IO );

	/*
	 *  Build a Configure Delivery Pipes Management
	 *  Request element.
	 */
	mre = (struct cfg_mre *) ap->sta_vaddr;
	qrev_writel( MGNT_LENGTH, (uint *)&(mre->header.length));
	qrev_writel( MANAGEMENT | REQUEST_EID,
		    (uint *)&(mre->header.indicators));
	qrev_writes( 0x0, &(mre->header.dst));
	qrev_writes(( ap->adp_uid << 8 ), &(mre->header.src));
	qrev_writel( 0x0, (uint *)&(mre->header.correlation));
	qrev_writes( 0x0, &(mre->id));
	qrev_writes( 0x8010, &(mre->function));
	qrev_writes( ap->adp_uid, &(mre->uids));
	qrev_writes( 0xFFFF, &(mre->cfg_status));
	qrev_writel( (uint)ap->surr_ctl.pusa_IO, (uint *)&(mre->pusa)); 
	qrev_writes( (ushort)ap->surr_ctl.ausa_IO, (ushort *)&(mre->ausa)); 
	qrev_writes( ap->ddi.base_addr, &(mre->aioa));
	qrev_writes( 0x0, &(mre->pioa));
	qrev_writes( 0x0, (ushort *)&(mre->timer_ctrl));
	qrev_writes( SHARED_MEMORY | ADAPTER | EMPTY_TO_NOTEMPTY,
		     &(mre->adp_cfg_options));
	qrev_writes( SHARED_MEMORY | SYSTEM | EMPTY_TO_NOTEMPTY,
		     &(mre->sys_cfg_options));
	qrev_writes( PIPESIZE , &(mre->eq_pipe_size));
	qrev_writes( PIPESIZE , &(mre->dq_pipe_size));
	qrev_writel( (uint)ap->dq_raddr, (uint *)&(mre->dq_pipe_addr));
	qrev_writel( (uint)ap->surr_ctl.dq_sds_IO,(uint *)&(mre->dq_sds_addr));
	qrev_writel( (uint)ap->surr_ctl.dq_sse_IO,(uint *)&(mre->dq_sse_addr));
	qrev_writel( (uint)ap->surr_ctl.dq_ses_IO,(uint *)&(mre->dq_ses_addr));
	qrev_writel( (uint)ap->surr_ctl.dq_ssf_IO,(uint *)&(mre->dq_ssf_addr));
	qrev_writel( (uint)ap->surr_ctl.ausa_IO,(uint *)&(mre->ausa));
	qrev_writel( (uint)ap->eq_raddr,(uint *)&(mre->eq_pipe_addr));
	qrev_writel( (uint)ap->surr_ctl.eq_sds_IO,(uint *)&(mre->eq_sds_addr));
	qrev_writel( (uint)ap->surr_ctl.eq_sse_IO,(uint *)&(mre->eq_sse_addr));
	qrev_writel( (uint)ap->surr_ctl.eq_ses_IO,(uint *)&(mre->eq_ses_addr));
	qrev_writel( (uint)ap->surr_ctl.eq_ssf_IO,(uint *)&(mre->eq_ssf_addr));

	/*  Flush the request element out of the system cache. */
	vm_cflush((caddr_t) ap->sta_vaddr, MGNT_LENGTH );

	/*  
	 *  Ensure the BUSY bit is clear before issuing cmd.
	 *  BUSY bit is set(so I am told) as a result of the
	 *  EIO signal. During this error recovery, BUSY bit
	 *  tends to be set longer than normal. Loop on BUSY
	 *  bit to ensure we don't fail too hastily.
	 */
	READ_BSR(ap->ddi.base_addr, &value );
	if(( pio_error ) || ( value & BUSY )) {
		if( value & BUSY ) {
			for( i = 0; i < 1000; i++ ) {
				READ_BSR(ap->ddi.base_addr, &value );
				if(!(value & BUSY))
					break;
			}
			if( value & BUSY )
				return( EIO );
		}
		else
			return( EIO );
	}

	/* Set state flag. */
	ap->locate_state = SCB_PENDING;

	/*  
	 *  Load the Command Interface registers with the
	 *  IO address of the management request element.
	 */
	WRITE_CIR((ap->ddi.base_addr + CIR3), ((int) ap->sta_raddr >> 24 ));
	if( pio_error )
		return( EIO );
	
	WRITE_CIR((ap->ddi.base_addr + CIR2), ((int) ap->sta_raddr >> 16 ));
	if( pio_error )
		return( EIO );

	WRITE_CIR((ap->ddi.base_addr + CIR1), ((int) ap->sta_raddr >> 8 ));
	if( pio_error )
		return( EIO );
	
	WRITE_CIR((ap->ddi.base_addr + CIR0), ((int)ap->sta_raddr) );
	if( pio_error )
		return( EIO );
	
	/*  Issue a Attention request to the adapter. */
	ATTENTION( ap->ddi.base_addr, ATN_SIGNAL );
	if( pio_error )
		return( EIO );

	return( 0 );
}

/*
 * NAME:	asc_parse_reply 
 *
 * FUNCTION: 
 *		Parses the returned control element to
 *		determine success or error.
 *
 * EXECUTION ENVIRONMENT:
 *		Process or interrupt level.
 *
 * NOTES:
 *		When operation in Move mode, commands initiated
 *		by the adapter driver must be serviced by the adapter
 *		driver. This routine determines if the command succeeded.
 *
 *  		
 * INPUT:
 *		ap	-  adapter structure
 *		ctl_blk -  adp_ctl_blk
 *
 * RETURNS:  
 *		0	- on success
 *		EIO	- if it was an error element
 */
int
asc_parse_reply(
        struct  adapter_info    *ap,
        struct  ctl_elem_blk    *ctl_blk )
{
ulong	*reply;
ushort	eid;
struct	adp_ctl_elem	*adp_ctl_elem;

	adp_ctl_elem = (struct adp_ctl_elem *)ctl_blk;
	if( adp_ctl_elem->status & CMD_TIMED_OUT ) {
		return( 0 );
	}

	reply = (ulong *)ctl_blk->reply_elem;
        eid = reply[EID_WORD] & EID_MASK;
        if ( eid == REPLY_REV) {
		return( 0 );
	}
	else {
 		asc_log_error( ap, ERRID_SCSI_ERR3, (uchar *)reply, 0, 0, 16 );
		return( EIO );
	}
}

/*
 * NAME:	asc_sleep 
 *
 * FUNCTION: 
 *		The generic sleep routine.
 *
 * EXECUTION ENVIRONMENT:
 *		Process level only.
 *
 * EXTERNAL CALLS:
 *		disable_lock, lock_enable, e_sleep_thread
 *
 * NOTES:
 *		The caller initiates an action that it would
 *		like to sleep on. If the event dis not already
 *		occur(ie. if the sleep flag is not set), this
 *		routine will sleep until it does occur.
 *  		
 * INPUT:
 *		ap	-  adapter structure
 *
 * RETURNS:  
 *		none
 */
void
asc_sleep(
        struct  adapter_info    *ap,
	ulong	sleep_event )
{
ulong	ipri;

	ipri = disable_lock(ap->ddi.int_prior, &(ap->ndd.ndd_demux_lock));
	if( ap->sleep_pending ) {
		e_sleep_thread((int *)sleep_event, &ap->ndd.ndd_demux_lock, 
				LOCK_HANDLER);
	}
	unlock_enable( ipri, &(ap->ndd.ndd_demux_lock) );
}

/*
 * NAME:	asc_tm_rtov
 *
 * FUNCTION: 
 *		Translates real addresses to virtual addresses for
 *		target mode buffers.
 *
 * EXECUTION ENVIRONMENT:
 *		Interrupt level only.
 *
 * NOTES:
 *		The address manipulation occurs to the Event element
 *		directly on the dequeue pipe. The Event element is 
 *		passed to the target receive routine(in asc_process_adp_elem)
 *		and is valid only for the duration of the receive routine.
 *		
 * CALLED FROM:
 *		asc_process_adp_elem
 * INPUT:
 *		ap	  - adapter structure ( determine proper pipe )
 *		ctl_elem  - pointer to the ctrl_elem
 *
 * RETURNS:  
 *		none
 */
void
asc_tm_rtov( 
	struct adapter_info	*ap,
	ulong  *reply )
{
int	len;
int	index; 

	/* Invalidate cache associated with the DQ pipe. */
	if( __power_rs() )
		asc_inval( (uchar *) reply, 0xC0 );

        len = ( reply[LEN_FIELD] >> 24 );
	bcopy((char *)reply, ap->tm_recv_buf, len);

	/* 
	 *  Byte reverse the fields that describe the initiator.
	 */
	qrev_writel( ap->tm_recv_buf[0], (uint *)&ap->tm_recv_buf[0] );
	qrev_writel( ap->tm_recv_buf[4], (uint *)&ap->tm_recv_buf[4] );
	qrev_writel( ap->tm_recv_buf[5], (uint *)&ap->tm_recv_buf[5] );

	/*
	 *  Manipulate length field so asc_dequeue()
	 *  increments dq_se properly.
	 */
	len = ap->tm_recv_buf[0];
	ap->tm_recv_buf[0] |= ap->tm_recv_buf[0] << 24;

        /*
	 *  Pass the protocol's tag back.  
	 */
	if ( ap->tm_recv_buf[5] & 0xF ) {  /* Determine which bus this is */
	        ASSERT(ap->proto_tag_e);
		ap->tm_recv_buf[3] = (ulong)ap->proto_tag_e;
	}
	else {
	        ASSERT(ap->proto_tag_i);
		ap->tm_recv_buf[3] = (ulong)ap->proto_tag_i;
	}

	/*
	 *  Parse the event element. For each buffer described in 
	 *  the event element, replace the value the buffer address
	 *  field(LSW) with the address of the corresponding 
	 *  buf_elem_pool structure.
	 */
	for(index = 8; (len > (index * 4)); index += 4 ) {
		qrev_writel( ap->tm_recv_buf[index],
			   (uint *)&ap->tm_recv_buf[index] );
		qrev_writel( ap->tm_recv_buf[index + 1], 
			   (uint *)&ap->tm_recv_buf[index + 1] );
		ap->tm_recv_buf[index + 3] =  ((ulong)ap->tm_buf_info + 
		      		((ulong)ap->tm_recv_buf[index] >> 16));
		ap->tm_bufs_at_adp--;
	}
	ap->tm_recv_fn( &ap->ndd, ap->tm_recv_buf );
}

#ifdef ASC_TRACE
/*
 * NAME:        asc_trace
 *
 * FUNCTION:
 *              This function writes scsi adapter trace information to the 
 *              internal trace table.
 *
 * EXECUTION ENVIRONMENT:
 *              This function can be called from the proccess or interrupt
 *              level.
 *
 * EXTERNAL CALLS:
 *		NONE
 * RETURNS:
 *              NONE
 */
void
asc_trace(
	struct	adapter_info	*ap,
	struct	ctl_elem_blk	*ctl_elem, 
	int 	option )
{
#define CMDO  0x434D444F  /* ASCII representation of 'CMDO' */
#define CMDI  0x434D4449  /* ASCII representation of 'CMDI' */
#define CANO  0x43414E4F  /* ASCII representation of 'CANO' */
#define CANI  0x43414E49  /* ASCII representation of 'CANI' */
#define OTHO  0x4F54484F  /* ASCII representation of 'OTHO' */
#define OTHI  0x4F54484F  /* ASCII representation of 'OTHI' */
#define LAST  0x4C415354  /* ASCII representation of 'LAST' */

	switch (option) {
	case 0 : 
		asc_trace_ptr->type = (uint) CMDO;
		break;
	case 1 : 
		asc_trace_ptr->type = (uint) CMDI;
		break;
	case 2 : 
		asc_trace_ptr->type = (uint) CANO;
		break;
	case 3 : 
		asc_trace_ptr->type = (uint) CANI;
		break;
	case 4 : 
		asc_trace_ptr->type = (uint) OTHO;
		break;
	case 5 : 
		asc_trace_ptr->type = (uint) OTHI;
		break;
	default :
		break;
	} /* end switch */

	asc_trace_ptr->ctl_elem = *ctl_elem;
	asc_trace_ptr->ap = (uint *)ap;
	asc_trace_ptr++;
	if (asc_trace_ptr > &asc_trace_tab[TRACE_ENTRIES - 1])
	asc_trace_ptr = &asc_trace_tab[0];
	asc_trace_ptr->type = (uint) LAST;
}
#endif /* ASC_TRACE */
