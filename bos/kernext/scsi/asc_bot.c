static char sccsid[] = "@(#)45	1.7  asc_bot.c, sysxscsi, bos411 9/1/94 08:10:55";
/*
 *   COMPONENT_NAME: SYSXSCSI
 *
 *   FUNCTIONS: asc_alloc_tcws
 *		asc_dequeue
 *		asc_get_start_free
 *		asc_intr
 *		asc_output
 *		asc_process_elem
 *		asc_retry_get
 *		asc_retry_put
 *		asc_rtov
 *		asc_sendlist_dq
 *		asc_start
 *		asc_translate
 *		asc_update_eq
 *		asc_vtor
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992, 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include	"ascincl.h"

/*
 * NAME:	asc_intr 
 *
 * FUNCTION: 
 *		Services SCSI adapter interrupts.
 *
 * EXECUTION ENVIRONMENT:
 *		Interrupt level only.
 *
 * NOTES:
 *		When the adapter is operating in move mode,
 *		an interrupt is posted when the inbound pipe
 *		goes from empty to non-empty. After it has
 *		been determined that a valid interrupt has
 *		occurred, asc_dequeue() will continue to dequeue 
 *		control	elements until the pipe is empty. Request
 *		elements may be enqueued on behalf of the
 *		protocol driver(ie.SEND SCSI) or the adapter
 *		driver(ie. abort). Control elements popped
 *		off the inbound pipe and belonging to the
 *		protocol layer are passed up to the protocol
 *		driver's recv() entry point. All others
 *		are processed by the adapter driver.
 *
 *		In move mode, the interrupt is cleared at the
 *		adapter when the BSR register is read and
 *		subsequently cleared at the system by a i_reset().
 *
 * EXTERNAL CALLS:
 *		i_reset, disable_lock, lock_enable
 *		
 * INPUT:
 *		intr	- pointer the the intr structure
 *
 * RETURNS:  
 *		INTR_SUCC if the interrupt was ours
 *		INTR_FAIL if the interrupt was not ours
 */
int
asc_intr( 
	struct intr *intr )
{
struct	adapter_info	*ap;		/* adapter structure 		  */
ulong	ioaddr;				/* IO handle 		  	  */
ulong	ipri;				/* device interrupt priority	  */
uchar	bsr;				/* Basic Status register contents */
uchar	pio_error = 0;			/* pio exception flag		  */
int	rc;				/* return code 	  		  */

	/* Get the adapter structure. */
	ap = (struct adapter_info *) intr;	
	ipri = disable_lock(ap->ddi.int_prior, &(ap->ndd.ndd_demux_lock));
	DDHKWD1(HKWD_DD_SCSIDD, DD_ENTRY_INTR, 0, ap);

	/* Attach to IO space. */
	ioaddr = (ulong) io_att( ap->ddi.bus_id, NULL ); 
	
	/* 
	 *  Read the Basic Status Register(BSR). 
	 *  The BSR contains HW status including the
	 *  interrupt request bit that indicates if our
	 *  adapter has interrupted us.
	 */
	READ_BSR(ap->ddi.base_addr, &bsr );
	if( pio_error == TRUE ) {
		if(( rc = asc_reset_adapter( ap )) != 0 ) {
			ap->adapter_check = TRUE;
		}
		io_det( ioaddr );
		unlock_enable( ipri, &(ap->ndd.ndd_demux_lock));
		DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_INTR, rc, ap);
		return( INTR_FAIL );
	}

	/*
	 *  Determine if our adapter posted an interrupt.
  	 */
	if ( ! (bsr & BSR_INTERRUPT )) {
		io_det( ioaddr );
		unlock_enable( ipri, &(ap->ndd.ndd_demux_lock));
		DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_INTR, bsr, ap);
		return( INTR_FAIL );
	}

	/*
	 *  Determine if an exception was experienced.
	 */
	if ( bsr & BSR_EXCEPTION ) {
		/* 
		 *  Process exception. 
		 *  Finish by resetting exception condition
		 *  via the BCR.
		 */	
		switch( bsr & BSR_EXCP_MASK ) {
			case	BSR_EXCP_INV_CMD:
			case	BSR_EXCP_HW_FAIL:
			case	BSR_EXCP_CC:
			case	BSR_EXCP_ILLOGICAL:
			case	BSR_EXCP_PIPE_CORRUPTED:
			case	BSR_EXCP_PIPE_ERROR:
				    asc_log_error(ap,ERRID_SCSI_ERR2,0,0,bsr,7);

				    /*
				     *  Reset the adapter and reestablish
				     *  previous state.
				     */
				    if((rc = asc_reset_adapter(ap)) != 0) {
					ap->adapter_check = TRUE;
				    }
				    unlock_enable(ipri,&ap->ndd.ndd_demux_lock);
				    DDHKWD1(HKWD_DD_SCSIDD,DD_EXIT_INTR,bsr,ap);
				    io_det( ioaddr );
				    i_reset( intr );
				    return( INTR_SUCC );
			default:
					ASSERT( 0 );
						break;
			}
	}

	/*
	 *  Determine if adapter is operating in 
	 *  Move mode or Locate mode. Servicing
	 *  the interrupt is significantly different
	 *  in each mode.
	 */
	if ( ap->adapter_mode == MOVE ) {
		/*
		 *  Service the dequeue(inbound) pipe.
		 */
		asc_dequeue( ap );
	}
	else {
		/* 
		 *  This is a Locate mode interrupt.
		 *  Invoke the asc_process_locate() routine to
		 *  service the Locate mode interrupt.
		 *  It will among other things read the
		 *  Interrupt Status Register(ISR) which
		 *  contains command completion info and issue
		 *  an EIO to the adapter.
		 */  
		asc_process_locate( ap, ioaddr );
	}

	/*
	 *  Clear the interrupt at the system.	
	 *  Note: In move mode, the adapter has been
	 *  configured to clear the interrupt at the device
	 *  when the BSR is read.
	 */
	i_reset( intr );

	/* 
	 *  Detach from IO space and return with
	 *  an indication to the system that
	 *  this was indeed our interrupt. 
	 */
	io_det( ioaddr );
	unlock_enable( ipri, &(ap->ndd.ndd_demux_lock));
	DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_INTR, INTR_SUCC, ap);
	return( INTR_SUCC );
}

/*
 * NAME:	asc_retry_get
 *
 * FUNCTION: 
 *		PIO load routine with exception handling.
 *
 * EXECUTION ENVIRONMENT:
 *		Process or interrupt level.
 *
 * NOTES:
 *		Will retry on IO exception three times before
 *		closing adapter with a permanent hardware error.
 *
 * INPUTS:
 *		ap	- adapter structure
 *		ioaddr	- IO address to load from.
 *		size	- size of load
 *			  1 - byte, 2 - short, 3 - word
 *		value	- address to load result into.
 *
 * RETURNS:  
 *		0		- success
 *		HW_FAILURE	- if unable to complete PIO.
 *		EINVAL		- if invalid size specified.
 */
int
asc_retry_get(
	struct	adapter_info	*ap,
	ulong	ioaddr,
	ulong	size,
	void	*value)
{
int	i;
int	rc = 1;

	for ( i = 0; rc != 0 && i < 3; i++ )
	{
		switch (size)
		{
		case 1:
			rc = BUS_GETCX((char *)ioaddr, value);
			break;
		case 2:
			rc = BUS_GETSX((short *)ioaddr, value);
			break;
		case 4:
			rc = BUS_GETLX((long *)ioaddr, value);
			break;
		default:
			return( EINVAL );
		}
	}
	if (rc) {
		/*
		 *  Log PIO error.
		 */
		asc_log_error( ap, ERRID_SCSI_ERR2, 0,ioaddr,size, 8 );
		ap->locate_state = HW_FAILURE;
		ap->adapter_check = TRUE;
		return( HW_FAILURE );
	}
	else {
		return( 0 );
	}
}

/*
 * NAME:	asc_dequeue 
 *
 * FUNCTION: 
 *		Continue to pop SCB control elements off of the dequeue pipe
 *		until the pipe is empty.
 *
 * EXECUTION ENVIRONMENT:
 *		Interrupt level only.
 *
 * NOTES:	
 *		Dequeues all control elements from the inbound pipe.
 *		Updates pipe pointers and surrogate areas.
 *
 *		The correlation field in the dequeued control
 *		element is used to index the proper ctrl_elem_blk
 *		in the send list
 *
 *		A d_complete() call to flush the IO buffers is not
 *		necessary to ensure valid surrogate area. 
 *		The buffers are flushed when the adapter writes the 
 *		signalling area which is in a separate cache line.
 *
 * CALLED FROM:
 *		asc_intr
 *
 * EXTERNAL CALLS:
 *		d_cflush
 *		
 * INPUT:
 *		ap	- adapter structure
 *
 * RETURNS:  
 *		none
 */
void
asc_dequeue(
	struct	adapter_info	*ap )
{
ushort	*reply;
ushort	status;
ulong	surrogate;
uchar	*surrogate_start_free;
struct	buf_pool_elem	*bp;

	/*  
	 *  Invalidate the system cache before reading the
	 *  dequeue surrogate control area. The adapter
	 *  updated this area unbeknownst to the system cache.
	 */
	if( __power_rs() ) {
		asc_inval( (uchar *)ap->surr_ctl.dq_ssf, SURR_SIZE );
	}

	/*
	 *  Determine if elements exist on the pipe by comparing
	 *  the adapter's start_of_free to our local start_of_elements.
	 */
	surrogate_start_free = ADD_BASE( ap->dq_vaddr, 
				       REV_SHORT(*ap->surr_ctl.dq_ssf));

	if (( ap->local.dq_se == surrogate_start_free ) &&
			(( (ushort)*ap->surr_ctl.dq_ses & WRAP_BIT) ==
			( (ushort)ap->local.dq_status & WRAP_BIT) )) {
		/*
		 *  Empty pipe, update the surrogate 
		 *  control area. The offset and status
		 *  MUST be updated atomically.
		 *  Note: The local dq status is not byte 
		 *  reversed when updating the surrogate area.
		 *  This is because the local area is maintained
		 *  in a byte reversed format to expedite
		 *  parsing and updating.
		 *  EMPTY bit not set in dq_status.
		 */
		qrev_writel(( OFFSET(ap->dq_vaddr, ap->local.dq_se)) |
			   (( REV_SHORT(ap->local.dq_status)) << 16 ),
			   ( uint * ) ap->surr_ctl.dq_sse );
	}
	else {
		/*
		 *  While control elements remain on the inbound pipe,
		 *  process elements. 
		 */
		do {
			/*
			 *  Get the pointer to the first element.
			 *  First invalidate the system cache.
			 */
			if( __power_rs() ) {
				asc_inval( ap->local.dq_se, 0x2C );
			}

			reply = (ushort *)ap->local.dq_se;

			/* 
			 *  Set local end_of_elements to surrogate 
			 *  start_of_free.
			 */
			ap->local.dq_ee = ADD_BASE( ap->dq_vaddr, 
				       REV_SHORT(*ap->surr_ctl.dq_ssf));

			/*
			 * If element at head of pipe is a wrap element,
			 * discard it, reset the start of elements ptr
			 * to base, toggle the dequeue wrap state
			 * and set the dequeued bit.
			 */
			if (reply[OPCODE] == WRAP_REV ) {
				/* Dequeue and toss element. */
				ap->local.dq_se = ap->dq_vaddr;

				/* Toggle the wrap bit. */
				ap->local.dq_status ^=  WRAP_BIT;

				/* 
				 *  Update surrogate sds and sse atomically.
				 */
				status = *ap->surr_ctl.dq_sds;
 				status ^= WRAP_BIT;
				surrogate = status & 0x0000FFFF;
				qrev_writel( REV_LONG(surrogate), 
					   (uint *) ap->surr_ctl.dq_sse );

				/* Flush/invalidate cache. */
				(void)d_cflush(ap->dma_channel, 
					       (caddr_t)ap->surr_ctl.dq_sse,
					       SURR_SIZE,
					       (caddr_t)ap->surr_ctl.dq_sse_IO);
					
			}
			else {
				/*
				 *  Process the request element and update
				 *  the start_of_elements pointer.
				 */
				asc_process_elem( ap, (ulong *)reply );
				if(( reply[3] & 0xFF00 ) != SCSI_INFO_REV ) {
					ap->local.dq_se += 
						REV_SHORT(reply[LEN_FIELD]);
				}
				else {
					ap->local.dq_se += reply[0] >> 8;
				}
			}
		} while ( ap->local.dq_se != ap->local.dq_ee );
	}

	/*  
	 *  Update the surrogate area. 
	 *  Update dequeue status to indicate
	 *  empty pipe (preserving existing bits) and 
	 *  increment offset to reflects all elements 
	 *  dequeued from the pip.
	 */
	ap->local.dq_status |= EMPTY_BIT_REV;
	qrev_writel(( OFFSET(ap->dq_vaddr, ap->local.dq_se)) |
		   (( REV_SHORT(ap->local.dq_status)) << 16 ),
		    ( uint * ) ap->surr_ctl.dq_sse );


	/* Flush/invalidate cache. */
	(void) d_cflush( ap->dma_channel, (caddr_t)ap->surr_ctl.dq_sse,
			SURR_SIZE,
		        (caddr_t)ap->surr_ctl.dq_sse_IO );

	/*  Process the waitq. */
	if ( ap->num_cmds_queued > 0 ) {
		asc_start( ap );
	}

	/*  Process pending Target Mode work. */
	if ( ap->tm_bufs_blocked ) {
		w_stop( &ap->tm.dog );
		bp = ap->tm_head;
		ap->tm_head = ap->tm_tail = NULL;
		ap->tm_bufs_to_enable = 0;
		(void) asc_enable_bufs( bp );
	}
}

/*
 * NAME:	asc_process_elem 
 *
 * FUNCTION: 
 *		Process control elements dequeued off the inbound pipe.
 *
 * EXECUTION ENVIRONMENT:
 *		Interrupt level only.
 *
 * NOTES:	
 *		To process a dequeued control element, must determine
 *		if adapter or protocol driver should service it.
 *		If the protocol driver initiated it, pass element to 
 *		protocol's recv() routine.
 *
 * CALLED FROM:
 *		asc_dequeue
 *
 * INPUT:
 *		ap	- adapter structure ( determine proper pipe )
 *		reply	- ptr into pipe where reply/error element resides
 *
 * RETURNS:  
 *		none
 */
void
asc_process_elem(
	struct	adapter_info	*ap,
	ulong	*reply 	)
{
ulong	eid;
int	status;
struct	ctl_elem_blk	*ctl_elem;
int	i;
int	*ptr;

	/*
	 *  Determine type of control element using eid field
	 *  in header. Event elements are processed differently 
	 *  than reply/error elements.
	 */
	eid = reply[EID_WORD] & EID_MASK; 
        if (( eid == REPLY_REV) || ( eid == ERROR_REV )) {
		/*
		 *  Reply and error elements will have
		 *  ctrl_elem_blk's queued on the send_list.
		 *  Correlate a reply to a ctl_elem_blk and
		 *  copy the reply element from the pipe into
		 *  ctl_elem_blk.
		 */
		qrev_writel( reply[CORRELATION], (uint *) &ctl_elem );
		bcopy((char *)reply, ctl_elem->reply_elem, 
		      (reply[LEN_FIELD] >> 24) );

		/* Pop the ctl_elem_blk  off the send list. */
		asc_sendlist_dq( ap, ctl_elem );
		ap->num_cmds_active--;
		
		/* 
		 *  If an error occurred, cleanup any possible 
		 *  DMA errors. Disregard d_complete() errors.
		 */
       	        if ( eid == ERROR_REV ) {
			(void) d_complete(ap->dma_channel, DMA_NOHIDE, 
			    		  ap->sta_vaddr, PIPESIZE, 
					  &ap->xmem, ap->sta_raddr );
		}

#ifdef ASC_TRACE
	asc_trace(ap, (struct ctl_elem_blk *)reply, 1); 
#endif /* ASC_TRACE */
		
		/*
		 *  Determine if the control element should be processed
		 *  (was initiated) by the adapter driver or protocol 
		 *  driver. Control elements initiated by the adapter 
		 *  driver are processed by the adapter and control 
		 *  elements initiated from the protocol driver (thru
		 *  the  adapter driver's output() routine) are
		 *  passed up to the protocol driver's recv() routine
		 *  for processing.
		 */
		if(!(ctl_elem->status & ADAPTER_INITIATED )) {
			/*
			 *  This control element should be processed
			 *  by the protocol driver.
			 *
			 *  Byte reverse the reply element.
			 *  Note: For Error elements, the header portion 
			 *  should be reversed as 32 bit entities and the
			 *  Command status block portion as 16 bit entities.
			 *  The 5th word of of the Cmd Status Block is handled
			 *  specially to hide format of resid from protocol.
			 */
			for( i = 0; (i * 4) < (reply[LEN_FIELD] >> 24); i++ ) {
			    ptr = (int *) &ctl_elem->reply_elem[i * 4];
				if( i < 4 ) {
				    qrev_writel( reply[i], (uint *)ptr );
				}
				else {
        			    if( eid == REPLY_REV ) {
					qrev_writel(reply[i], (uint *)ptr);
				    }
				    else {
					if( i == 5 ) {
					    qrev_writel(reply[i], (uint *)ptr);
					}
					else {
		 			    *ptr = REV_LONG_SHORT(reply[i]);
					}
				    }
				}
			}

			/*
	 	 	 *  Translate virtual addresses to real addresses
			 *  and release bus resources.
			 */
			asc_rtov( ap, ctl_elem );

			/*
			 *  If this was a reply to an Initialize SCSI
			 *  (SCSI bus reset), manually clear the active
			 *  queue since we never get replies to
			 *  affected commands. If the SCSI bus reset
			 *  was initiated by the adapter driver(EPOW),
			 *  queues are cleared in asc_process_adp_elem().
			 */
			if( ctl_elem->reply_elem[5] == 0x52 ) {
				if( ctl_elem->status & ASC_INTERNAL ) {
					asc_clear_queues(ap, 0);
				}
				else {
					asc_clear_queues(ap, 1);
				}
				ctl_elem->status = 0;
			}

			/*
			 *  If a dump is occurring, do not return
			 *  this element via recv(). asc_dump_write()
			 *  will parse the reply element and return
			 *  the proper indication.
			 */
			if( ap->dump_state == DUMP_START ) {
				return;
			}

			/* 
			 *  Call the protocol driver's receive function.
			 *  OK to pass reply to protocol layer before
			 *  servicing waitq because asc_output() tacks
			 *  all request on the end of the wait queue.
			 *  ie. no danger of commands being sent out out 
			 *  of order.
			 */
			ap->recv_fn( &ap->ndd, ctl_elem->reply_elem );

		}
		else {
			/*
			 *  This control element should be processed
			 *  by the adapter driver. 
			 */
			asc_process_adp_elem( ap, ctl_elem );
		}
	}
	else {
		/*
		 *  This is an event element.
		 *  No corresponding ctrl_elem_blk is
		 *  queued on the send list, do not
		 *  attempt to use correlation field to pop 
		 *  ctrl_elem_blk off of the send list.
		 *  Currently, only three kinds of event elements
		 *  are expected:
		 *  A SCSI_INFO event element, a Read Immediate
		 *  event element or a Wrap event element. The SCSI_INFO 
		 *  indicates a target event and should be processed by the
		 *  protocol driver. The read immediate event is
		 *  notifying us of some SCSI bus event( ie. SCSI
		 *  bus reset). Notify the protocol driver via
		 *  its asynchronous stat()) entry point.
		 *  A Wrap element should be discarded and the
		 *  surrogate and local pipe pointers should
		 *  be updated. Wrap events are handled in asc_dequeue().
		 */
		if(( reply[EID_WORD] & OP_MASK ) == SCSI_INFO_REV ) {
			/*
			 *  Target mode resources are handled
			 *  separately from initiator mode.
			 */
			asc_tm_rtov( ap, reply );
		}
		else {
			/*
			 *  Read immediate event.
			 *  Fill in a async_status struct and call the 	
			 *  protocol driver's  stat function.
			 */
			status = reply[STATUS_WORD] & STATUS_MASK;
			switch( status ) {
				case	RESET_INT:
					asc_async_status(ap,NDD_SCSI_BUS_RESET,
						 	 0);
					break;
				case	RESET_EXT:
					asc_async_status(ap,NDD_SCSI_BUS_RESET,
							 1);
					break;
				case	TERM_POWER_LOSS_INT:
				    	asc_async_status(ap,NDD_TERM_POWER_LOSS,
							 0);
					break;
				case	TERM_POWER_LOSS_EXT:
					asc_async_status(ap,NDD_TERM_POWER_LOSS,
							 1);
					break;
				case	DIFF_SENSE_INT:
					asc_async_status(ap,
						     NDD_DIFFERENTIAL_SENSE, 0);
					break;
				case	DIFF_SENSE_EXT:
					asc_async_status(ap,
						     NDD_DIFFERENTIAL_SENSE, 1);
					break;
				case	DEV_RESELECT_INT:
					asc_async_status(ap,NDD_DEV_RESELECT,0);
					break;
				case	DEV_RESELECT_EXT:
					asc_async_status(ap,NDD_DEV_RESELECT,1);
					break;
				default:
					return;
			}
		}
	}
}

/*
 * NAME:	asc_rtov
 *
 * FUNCTION: 
 *		Releases TCW resources and translates real addresses 
 *		to virtual addresses.
 *
 * EXECUTION ENVIRONMENT:
 *		Interrupt level only.
 *
 * NOTES:
 *		Releases bus memory(TCWs).
 *		Process reply or error elements here.
 *		Addresses for initiator requests do not need to be translated, 
 *		the virtual addresses are still intact in the caller's 
 *		ctrl_elem_blk. All that needs to be done is release TCW's 
 *		by clearing the appropriate allocated bit(s).
 *
 * EXTERNAL CALLS:
 *		xmemdma
 *		
 * CALLED FROM:
 *		adp_process_elem
 *		
 * INPUT:
 *		ap	  - adapter structure ( determine proper pipe )
 *		ctl_elem  - pointer to the ctrl_elem
 *
 * RETURNS:  
 *		none
 */
void
asc_rtov( 
	struct adapter_info	*ap,
	struct	ctl_elem_blk	*ctl_elem )
{
struct	buf	*bp;
ulong	dma_flags;
ulong	b_addr;
ushort	tcw_index;
ushort	num_tcws;
int	start_bit;
int	mask;
int	index;
int	rc;
int	running_count;
int	total_tcws = 0;
int	tcws_this_word = 0;
char	*naddr;

	/* 
	 *  Determine if TCW resources were allocated 
	 *  for this element. 
	 */
	if( ctl_elem->pd_info != NULL ) {
		if( ctl_elem->pd_info->mapped_addr == 0x0 ) {
			return;
		}
	}
	else {
		return;
	}

	/* Get the buf struct that describes the buffer(s). */
	bp = (struct buf *) ctl_elem->pd_info->p_buf_list;

	/*
	 *  Release STA resources if appropriate.
	 */
	if( ctl_elem->status & STA_USED ) {
		/*
		 *  Copy data out of STA and into callers buffer.
		 *  Release STA.
		 */
		if( bp->b_flags & B_READ) {
			index = ctl_elem->pd_info->num_tcws;
			if( __power_rs() ) {
				asc_inval( ap->sta[index].stap, STA_SIZE );
			}

			/* 
			 *  Buffer in kernel space - bcopy.
			 *  Note: if the caller's buffers was in 
			 *  user space, we did not use a STA. The
			 *  buffer was mapped and DMA'd into directly.
			 */
			bcopy(ap->sta[index].stap,bp->b_un.b_addr,bp->b_bcount);
		}
		ap->sta[ctl_elem->pd_info->num_tcws].in_use = FALSE;
		ctl_elem->status &= ~STA_USED;
		ctl_elem->pd_info->num_tcws = 0;
		return;
	}

	/* Save buffer address */
	b_addr = (ulong) bp->b_un.b_addr;

	/*
	 *  Get the real addr and total length found in the
	 *  pd_info struct. Calculate what bits in the TCW table 
	 *  correspond to these TCWs using the following equation:
	 *  start_bit = ((base_addr & PAGE_MASK ) - ap->dma_base) >> L2_PAGE;
	 *  Set these bits to ones indicating these TCWs are free.
	 */
	start_bit = ((ctl_elem->pd_info->mapped_addr & PAGE_MASK ) - 
			(ulong)ap->busmem_end) >> L2_PAGE;

	tcw_index = start_bit / WORDSIZE;
	start_bit =  start_bit - ( WORDSIZE * tcw_index );

	num_tcws = (( b_addr + ctl_elem->pds_data_len - 1 ) >> L2_PAGE ) - 
		   ( b_addr >> L2_PAGE ) + 1;

	if(( WORDSIZE - start_bit) > num_tcws ) {
		/*
		 *  Build a mask to free TCW bits.
		 *  This logic needs to be optimized!
		 */
		mask = ALL_ONES << num_tcws;
		mask = ~mask;
		mask = mask << (WORDSIZE - (start_bit + num_tcws));
		ap->tcw_table[tcw_index] |= mask;
	}
	else {
		/*
	 	 *  The TCW range spans more than one word.
	 	 *  Build a mask for each affected word and
		 *  free the TCW's.
	 	 */
		tcws_this_word = WORDSIZE - start_bit;
		mask = ALL_ONES << tcws_this_word;
		ap->tcw_table[tcw_index] |= ~mask;
		total_tcws += tcws_this_word;

		while( total_tcws < num_tcws ) {

			if(( num_tcws - total_tcws ) > WORDSIZE ) {
				tcws_this_word = WORDSIZE;
				mask = 0x0;
			}
			else {
				tcws_this_word = num_tcws - total_tcws;
				mask =  ALL_ONES >> tcws_this_word;
			}
			tcw_index++;
			ap->tcw_table[tcw_index] |= ~mask;
			total_tcws += tcws_this_word;
		}
	}

	/*
	 *  Complete DMA processing of the buffer. To reduce 
	 *  path-length, the d_complete() will not be used.
	 *  The IO buffer flush should never be necessary because
	 *  the IO buufer gets flushed when the adapter writes the
	 *  signalling area. DMA error cleanup(d_complete) is done
	 *  on the error path only. All we need to do here is UNHIDE 
	 *  pages(if necessary as indicated by the dma_flags). xmemdma()
	 *  is called directly to UNHIDE pages.
	 */ 
	if( bp->b_flags & B_NOHIDE ) {
		return;
	}
	else {
		dma_flags = ((bp->b_flags & B_READ) ? DMA_READ : 0) |
			    ((bp->b_flags & B_NOHIDE) ? DMA_WRITE_ONLY : 0);

		running_count = 0;
		while ( running_count < ctl_elem->pds_data_len ) {
		   naddr = (char *)((ulong)bp->b_un.b_addr & 
			   ( ~( DMA_PSIZE - 1 )));

		   while( naddr < ( bp->b_un.b_addr + bp->b_bcount ) ) {
		   	if(( rc = xmemdma( &bp->b_xmemd, naddr,
				      	   XMEM_UNHIDE )) == XMEM_FAIL ) {
				asc_log_error( ap, ERRID_SCSI_ERR2,0,0,rc,18);
		   	}
		        naddr += DMA_PSIZE;
		    }
		    running_count += bp->b_bcount;
		    bp = bp->av_forw;
		}
	}
	return;
}

/*
 * NAME:	asc_retry_put
 *
 * FUNCTION: 
 *		PIO store routine with exception handling.
 *
 * EXECUTION ENVIRONMENT:
 *		Process or interrupt level
 *
 * NOTES:
 *		Will retry on IO exception three times before
 *		closing adapter with a permanent hardware error.
 *
 * INPUTS:
 *		ap	- adapter structure
 *		ioaddr	- IO address to store to.
 *		size	- size of store
 *			  1 - byte, 2 - short, 3 - word
 *		value	- value to store
 *
 * RETURNS:  
 *		0		- success
 *		HW_FAILURE	- if unable to complete PIO.
 *		EINVAL		- if invalid size specified.
 */
int
asc_retry_put(
	struct	adapter_info	*ap,
	ulong	ioaddr,
	ulong	size,
	ulong	value)
{
int	i;
int	rc = 1;

	for ( i = 0; rc != 0 && i < 3; i++ )
	{
		switch (size)
		{
		case 1:
			rc = BUS_PUTCX((char *)ioaddr, value);
			break;
		case 2:
			rc = BUS_PUTSX((short *)ioaddr, value);
			break;
		case 4:
			rc = BUS_PUTLX((long *)ioaddr, value);
			break;
		default:
			return( EINVAL );
		}
	}
	if (rc) {
		/*
		 *  Log PIO error.
		 */
		asc_log_error( ap, ERRID_SCSI_ERR2, 0,ioaddr,size, 9 );
		ap->locate_state = HW_FAILURE;
		ap->adapter_check = TRUE;
		return( HW_FAILURE );
	}
	else {
		return( 0 );
	}
}

/*
 * NAME:	asc_get_start_free 
 *
 * FUNCTION: 
 *		Returns a pointer to the start of the free space
 *		on the outbound(enqueue) pipe.
 *
 * EXECUTION ENVIRONMENT:
 *		Interrupt level only.
 *
 * NOTES:	
 *		Conforms to SCB Architecture.
 *
 *		The IO buffers have been flushed as a result of the 
 *		signalling areas being in a separate cache line than the
 *		surrogate areas. Thus the surrogate areas and associated
 *		data are valid.
 *
 *		This routine ensures that all control elements start
 *		on word(32-bit) boundaries.
 *
 * EXTERNAL CALLS:
 *		d_cflush, vm_cinval
 *		
 * INPUT:
 *		ap	 - adapter structure
 *		size	 - amount of space(bytes) needed
 *		free_ptr - pointer into pipe returned by reference
 *
 * RETURNS:  
 *		0 	 - for good completion,  ERRNO on error
 *		PIPEFULL - if not enough room exists on the pipe.
 */
int	    
asc_get_start_free(
	struct	adapter_info	*ap,
	int	size,
	uint	**free_ptr )
{
struct	scb_header	*wrap;
ushort	base;

	if( __power_rs() ) {
		asc_inval( (uchar *)ap->surr_ctl.dq_ssf, SURR_SIZE );
	}

	/* Zero out the pointer to start_of_free. */
	*free_ptr = NULL;

	/*
	 *  Read surrogate area to determine ptr to 
	 *  end_of_elements.
	 */
	base = REV_SHORT( *ap->surr_ctl.eq_sse );
	ap->local.eq_se = (uchar *)R_TO_V((ulong)ap->eq_raddr, 
					ap->eq_vaddr, base );

	/*
	 *  Update the local end of free space by tracking
	 *  where the surrogate start of elements is currently located.
	 */
	if (( ap->local.eq_ef > ap->local.eq_se ) ||
			(( ap->local.eq_ef == ap->local.eq_se) &&
			(( ap->local.eq_status & WRAP_BIT )  == 
			((ushort)*ap->surr_ctl.eq_sds & WRAP_BIT)))) {
		ap->local.eq_ef = ap->local.eq_top;
	}
	else {
		ap->local.eq_ef = ap->local.eq_se;
	}

	/*
	 *  Determine if we can fit the SCB onto the pipe.
	 *  If enough room exists, return pointer to start of
	 *  free space.
	 */
	if ( size <= ( ap->local.eq_ef - ap->local.eq_sf )) {
		/* 
		 *  Element will fit into the pipe, return
		 *  start_of_free.
		 *  NOTE: start_of_free will be incremented 
		 *  in the asc_update_eq() routine.
	 	 */
		*free_ptr = (uint *)ap->local.eq_sf;
		ap->local.eq_sf += size;
		return( 0 );
	}
	else {
		/*
		 *  The control element(s) will not fit onto the pipe.
		 *  The enqueue pipe is either full or needs
		 *  to be wrapped around to the beginning.
		 */
		if ( ap->local.eq_ef == ap->local.eq_top ) {
			/*
			 *  Wrap condition exists.
			 *  Build a wrap element directly onto the pipe.
			 */
			wrap = (struct scb_header *) ap->local.eq_sf;
			qrev_writel( WRAP_SIZE, (uint *)&wrap->length );
			qrev_writel( WRAP, (uint *)&wrap->indicators );

			/* Flush the data out of the system cache. */	
			(void) d_cflush(ap->dma_channel, (caddr_t)wrap,
					WRAP_SIZE, 
					V_TO_R(ap->eq_raddr, ap->eq_vaddr,
					       (uchar *)wrap));

			/*
			 *  Set the start_of_free to the base of the enqueue 
			 *  pipe. Set the end_of_free to the start of the
			 *  next element on the pipe.
			 */
			ap->local.eq_sf = ap->eq_vaddr;
			base = REV_SHORT( *ap->surr_ctl.eq_sse );
			ap->local.eq_ef = (uchar *)R_TO_V((ulong)ap->eq_raddr, 
					ap->eq_vaddr, base );

			/* 
			 *  Toggle the state of the wrap bit. 
			 */
			ap->local.eq_wrap = *ap->surr_ctl.eq_ses;
			ap->local.eq_wrap ^= WRAP_BIT;
			ap->local.eq_status ^= WRAP_BIT;

			/*
			 *  After wrapping, again check if we can fit
			 *  the control element onto the pipe.
			 */
			if ( size <= ( ap->local.eq_ef - ap->local.eq_sf )) {
				*free_ptr = (uint *)ap->local.eq_sf;
				ap->local.eq_sf += size;
				return( 0 );
			}
			else {
				/* 
				 *  Control element still will not fit
				 *  into the pipe, return pipe full 
				 *  condition.
				 */
				ap->local.eq_status |= EQ_PIPE_FULL;
				*ap->surr_ctl.eq_ses |= EQ_PIPE_FULL;
				ap->pipe_full_cnt++;
				return( PIPE_FULL );
			}
		}
		else {
			/* Pipe is full, did not try to wrap. */
			ap->local.eq_status |= EQ_PIPE_FULL;
			*ap->surr_ctl.eq_ses |= EQ_PIPE_FULL;
			ap->pipe_full_cnt++;
			return( PIPE_FULL );
		}
	}
}

/*
 * NAME:	asc_update_eq 
 *
 * FUNCTION: 
 *		Updates pipe management pointers and informs
 *		the adapter that request element(s) exist on
 *		the pipe and should be serviced.
 *
 * EXECUTION ENVIRONMENT:
 *		Interrupt level only.
 *
 * NOTES:	
 *		Conforms to SCB Architecture.
 *		Updates surrogate areas.
 *
 *		Early levels of the adapter misbehaved such
 *		that the adapter would get stale data.  
 *		If the adapter accessed the surrogate area after
 *		after the previous data transfer but before
 *		the driver flushed the surrogate area to mainstore,
 *		IO buffer would be in the prefetched state with
 *		stale data. When a signal is issued to the adapter, 
 *		the IOCC/XIO thinks it has the latest data prefetched
 *		in its IO buffer and pass it on to the adapter.
 *		This can result in command timeouts. A d_blush after
 *		the last d_cflush may be necessary in this routine.
 *		In practice, the adapter should not be reading the
 *		surrogate area at this time.
 *
 *
 * EXTERNAL CALLS:
 *		d_bflush, d_cflush, vm_cinval
 *		
 * INPUT:
 *		ap	 - adapter structure
 *		free_ptr - pointer into pipe returned by reference
 *		size	 - amount of space(bytes) needed
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *		EIO	- kernel service failure or invalid operation
 */
int	    
asc_update_eq(
	struct	adapter_info	*ap,
	uchar	*free_ptr,
	int	size )
{
ulong	ioaddr;
uchar	pio_error = 0;
int	rc;

	/* 
	 *  Flush the system data cache.
	 */
	(void) d_cflush( ap->dma_channel, free_ptr, size, ap->eq_raddr );

	/*  
	 *  Update surrogate area. The start_of_free and 
	 *  status must be updated atomically.
	 */
	qrev_writel(( OFFSET(ap->eq_vaddr, ap->local.eq_sf )) |
		   (( REV_SHORT(ap->local.eq_wrap)) << 16 ),
		    ( uint * ) ap->surr_ctl.eq_ssf );


	/* Flush the surrogate area. */
	(void)d_cflush( ap->dma_channel, (caddr_t)ap->surr_ctl.eq_ssf,
		        SURR_SIZE, (caddr_t)ap->surr_ctl.eq_ssf_IO );

	/*
	 *  The pipe has gone from empty to non-empty.
	 *  Inform the adapter that the enqueue pipe 
	 *  has a new element on it that must be serviced. 
	 *  This is done by writing a 0x0D (Move Mode signal) 
	 *  to the Attention register.
	 */
	ioaddr = (ulong) io_att( ap->ddi.bus_id, NULL ); 
	ATTENTION( ap->ddi.base_addr, 0xD0 );
	io_det( ioaddr );
	if( ! pio_error ) {
		return( 0 );
	}
	else {
		/* 
		 *  Reset the adapter to recover from PIO errors.
		 *  Do not reset if on the open() path.
		 */
		if( ap->opened ) {
			if(( rc = asc_reset_adapter( ap )) != 0 ) {
				ap->adapter_check = TRUE;
			}
		}
		return( EIO );
	}
}

/*
 * NAME:	asc_sendlist_dq 
 *
 * FUNCTION: 	Remove an ctl_elem_blk from the sendlist.
 *
 * EXECUTION ENVIRONMENT:
 *		Interrupt level only.
 *
 * NOTES:	
 *		The caller must acquire the active_lock before
 *		calling this routine.
 *
 * INPUT:
 *		ap		- adapter structure
 *		ctl_elem_blk	- control element block
 *
 * RETURNS:  
 *		none
 */
void
asc_sendlist_dq(
	struct	adapter_info	*ap,
	struct	ctl_elem_blk	*ctl_blk )
{

	ASSERT( ap->active_head != NULL );
	ASSERT( ap->active_tail != NULL );
	if( ap->active_head == ctl_blk ) {
		/*  Remove the first element on the list. */
		ap->active_head = ctl_blk->next;
		if( ap->active_tail == ctl_blk ) {
			ap->active_tail = NULL;
		}
		else {
			ap->active_head->prev = NULL;
		}
	}
	else if( ap->active_tail == ctl_blk ) {
		/*  Remove the last element on the list. */
		ap->active_tail = ctl_blk->prev;
		if( ap->active_tail == NULL ) {
			ap->active_head = NULL;
		}
		else {
			ap->active_tail->next = NULL;
		}
	}
	else {
		/*  Remove an element from the middle of the list. */
	       ctl_blk->prev->next = ctl_blk->next;
	       ctl_blk->next->prev = ctl_blk->prev;
	}
}

/*
 * NAME:	asc_output
 *
 * FUNCTION: 
 *		Adapter driver' output routine.
 *
 * EXECUTION ENVIRONMENT:
 *		Interrupt level only.
 *
 * NOTES:
 *		This routine will build an adapter control element
 *		and enqueue it to the adapter. The control element is built
 *		using the information passed in the ctl_elem_blk. All of the
 *		addresses given are virtual and need to be translated to
 *		real addresses before being sent to the adapter. The resulting
 *		control element is enqueued to the adapter's inbound pipe.
 *  		This routine will chain the passed in ctl_elem_blks as a
 *		doubly linked list of elements. The correlation
 *		field of the adapter control element will have the address of 
 *		ctl_elem_blk.
 *
 *		If the adapter driver can not satisfy a request because of
 *		lack of resources(no TCW's, pipe full), it queues the 
 *		ctl_elem_blk on its wait queue. 
 *
 *		asc_output() can be called on the interrupt thread.
 *
 * EXTERNAL CALLS:
 *		None.
 *		
 * INPUT:
 *		ndd - ndd structure for this adapter
 *		ctl_blk - ptr to a ctl_elem_blk structure
 *			    that describes the scsi command and buffers
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *		EIO         - kernel service failure or invalid operation
 *		HW_FAILURE  - PIO error
 */
int
asc_output( 
	ndd_t	*ndd,
	struct	ctl_elem_blk 	*ctl_elem )
{
struct	adapter_info	*ap;

	/*
	 *  Get the adapter structure for this adapter.
	 *  Note: the ndd_correlator field contains the address
	 *  of the adapter structure.
	 */
	ap = (struct adapter_info *) ndd->ndd_correlator;
	if( ap == NULL ) {
		return ( EIO );
	}
	ASSERT( ctl_elem != NULL );

	/*
	 *  Ensure the adapter is not in a permanent hardware
	 *  failure condition.
	 */
	if ( ap->adapter_check ) {
		return( HW_FAILURE );
	}

	/*
	 *  Parse each request that comes through asc_output()
	 *  looking for a Cancel request element.
	 *  This means a abort. BDR or SCSI bus reset has
	 *  been requested. The wait queues must be cleared
	 *  of all elements that correlate to the SCSI id/LUN/bus
	 *  specified in the control element. 
	 *  The request resides in the request area 
	 *  of the ctl_elem_blk. A cancel request element
	 *  will be chained to a Perform SCSI request element.
	 *  The abort code field in the Perform SCSI 
	 *  request element will indicate whether to issue an
	 *  abort, reset(SCSI bus) or BDR(device reset). 
	 *  Cmds on the send list that are affected
	 *  will either complete before the reset/abort is sent or
	 *  will be removed from the send list as a result of the error
	 *  element produced by the abort/bdr/reset.
	 *
	 *  NOTE: The SCSI protocol driver does not chain a 
	 *  cancellation list but expects the adapter driver
	 *  to use the SCSI id/LUN/bus information in the
	 *  Perform SCSI element to clean the wait queue.
	 */
	if( ctl_elem->flags & CANCEL ) {
		asc_abort( ap , ctl_elem );
		return( 0 );
	}

	/*
	 *  Push the ctl_elem_blk onto the end of the wait queue.
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

	/*
	 *  Zero out the bus memory field so as not to confuse
	 *  driver into believing the resources have already been
	 *  allocated.
	 */
	if( ctl_elem->pd_info != NULL ) {
		ctl_elem->pd_info->mapped_addr = 0x0;
	}
	ctl_elem->status = 0;

	/*
	 *  Activate queued command(s).
	 */
	asc_start( ap );
	return( 0 );
}

/*
 * NAME:	asc_start 
 *
 * FUNCTION: 
 *		Service the adapter's wait queue.
 *
 * EXECUTION ENVIRONMENT:
 *		Interrupt level only.
 *
 * NOTES:	
 *		The output() routine enqueued all requests		
 *		onto the wait queue initially. asc_start()
 *		will try to satisfy the resource needs of the request,
 *		enqueue it onto the outbound pipe and 
 *		move the ctl_elem_blk from the wait queue to the
 *		send queue. asc_start() is also invoked after
 *		resources have been released and commands are queued.
 *
 * CALLED FROM:
 *		asc_output, asc_process_elem
 *		
 * INPUT:
 *		ap	- adapter structure
 *
 * RETURNS:	
 *		none
 */
void	    
asc_start(
	struct	adapter_info	*ap )
{
struct	ctl_elem_blk	*ctl_elem;		
struct	adp_ctl_elem	*adp_ctl_elem;
int	rc = 0;

	/*
	 *  If a HW reset is in progress, do not attempt
	 *  to activate commands.
	 */
	if(( ap->mm_reset_in_prog ) || ( ap->bus_reset_in_prog ) || 
	   ( ap->epow_state ))
		return;

	/*
	 *  Process(move elements from wait state to active) the wait 
	 *  queue. Start with the control element at the head of the 
	 *  wait queue.
	 */
	while ( ap->wait_head != NULL ) {

		/* Get the first element off of the wait list. */
		ctl_elem = ap->wait_head;

		if( ctl_elem->status & ADAPTER_INITIATED ) {
			/*  
			 *  This element was queued on the list as 
			 *  a placeholder to remind us to wakeup!.
			 */
			adp_ctl_elem = (struct  adp_ctl_elem *) ctl_elem;
			e_wakeup(&adp_ctl_elem->event);
			return;
		}

		if( ctl_elem->flags & CANCEL ) {
			ap->wait_head = ctl_elem->next;
			if( ap->wait_head == NULL )
				ap->wait_tail = NULL;
			else
				ap->wait_head->prev = NULL;
			ap->num_cmds_queued--;
			rc = asc_abort( ap, ctl_elem );
			if( rc == 0 )
				continue;
			else
				return;
		}
		
		/*
		 *  Determine if ctl_elem_blk was queued because of
		 *  lack of TCW's or pipe full condition. If 
		 *  pd_info->mapped_addr is non-NULL, then TCW's have 
		 *  already been allocated for this command.
		 */
		if(( ctl_elem->pd_info != NULL ) &&
			( ctl_elem->pd_info->mapped_addr == 0x0 )) {
			/*
			 *  Bus memory resources have not been allocated
			 *  for this command yet, try to allocate them.
		 	 *  If resources are unavailable, leave the request 
			 *  on the wait queue. The command will be processed 
			 *  when resources have been released.
			 *
		 	 *  asc_vtor() will translate the virtual addresses 
			 *  to real addresses and allocates and maps the
		 	 *  bus memory space(TCWs).
		 	 */
			if(( rc = asc_vtor( ap, ctl_elem )) != 0 ) {
				/*
			 	 *  No resources available to issue
			 	 *  command. Leave the cmd on the waitq.
			 	 */
				return;
			}
		}

		/*
		 *  Translate the control element to an adapter 
		 *  control element and enqueue onto pipe.
		 */
		if(( rc = asc_translate( ap, ctl_elem )) != 0 ) {
			/*
			 *  The element was not enqueued onto the 
			 *  outbound pipe successfully.
			 */
			if( rc == PIPE_FULL ) {
				/*  
				 *  Pipe full condition.
				 *  ctl_elem remains on the wait list.
				 *  Note: the allocated TCW's are kept
				 *  to expedite servicing of the waitq later.
				 */
				return;
			}
			else {
				/*  
			 	 *  Error updating pipe - PIO error.
			 	 */
				ap->adapter_check = TRUE;
				return;
			}
		}
	}
	return;
}

/*
 * NAME:	asc_vtor 
 *
 * FUNCTION: 
 *		Allocates resources necessary to issue a SCSI cmd.
 *		Translates virtual addresses to real addresses,
 *		allocates and maps bus memory.
 *
 * EXECUTION ENVIRONMENT:
 *		Interrupt level only.
 *
 * NOTES:
 *		All address in the ctl_elem_blk are virtual addresses.
 *		The adapter driver manages all
 *		resources(including bus memory) and thus will allocate
 *		bus memory to map the passed in virtual buffers.
 *		The virtual buffers are described in the passed in
 *		pd_info structure(s).
 *
 *  		Allocate necessary TCW's to handle transfer.
 *  		Translate addresses in addr_info from virtual to real.
 *  		Map buffers for DMA.
 *
 *  		TCW allocation and Management
 *		-----------------------------
 *		A bit map implementation is employed to manage the TCW pool.
 *   		The cntlz(count leading zeros) instruction provides a quick 
 * 		and easy mechanism to find contiguous blocks of bus memory
 *		to satisfy a request. Each TCW is represented by one bit and 
 *		is allocated if the bit is set to zero. The routine that 
 *		that manages the pool explains the implementation in detail.
 *		The real memory address and length of the transfer will be all
 *		that is necessary when releasing resources.
 *
 *		The adapter driver maintains two pools, one for Small Transfer 
 *		Area(less than 256 bytes) and a large pool for everything else.
 *
 * CALLED FROM:
 *		asc_start
 *
 * EXTERNAL CALLS:
 *		d_master
 *		
 * INPUT:
 *		ap	  - adapter structure ( to determine proper TCW pool )
 *		ctl_elem - ctl_elem structure passed in by caller
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *		ENOMEM	- could not allocate resources
 */
int
asc_vtor( 
	struct	adapter_info	*ap,
	struct	ctl_elem_blk	*ctl_elem )
{
struct	buf	*bp;
ulong	dma_flags;
ulong	num_tcws;
ulong	start_addr;
ulong	start_bit;
ulong	running_count = 0;
ulong	i;

	/* Determine if TCW resources are required. */
	if( ctl_elem->pds_data_len == 0 ) {
		return( 0 );
	}

	/*
	 *  Get a pointer to the first buf struct.
	 *  If bp pointer is NULL, no bus memory resources
	 *  are required for this command.
	 */
	if( ctl_elem->pd_info != NULL ) {
		bp = (struct buf *) ctl_elem->pd_info->p_buf_list;
		if( bp == NULL ) {
			return( 0 );
		}
	}
	else {
		return( 0 );
	}

	/*
	 *  Determine if the STA resources can be used.
	 */
	if(( bp->b_bcount <= STA_SIZE ) && (( bp->b_bcount > 0 ) &&
	   ( bp->b_xmemd.aspace_id == XMEM_GLOBAL ))) {
		/*
		 *  Allocate a STA for the transfer.
		 */
		for( i = 0; i < NUM_STA; i++ ) {
			if( ap->sta[i].in_use == FALSE ) {
				/*
			 	 *  Set index of STA in pd_info and indicate
			 	 *  that this element used a STA.
			 	 */
				ap->sta[i].in_use = TRUE;
				ctl_elem->status |= STA_USED;
				ctl_elem->pd_info->num_tcws = i;
				ctl_elem->pd_info->mapped_addr = (uint)
					      (ap->sta_raddr + (i * STA_SIZE));

				/* 
				 *  If write direction is specified, copy
				 *  user data into the STA.
				 */
				if(!(bp->b_flags & B_READ)&&(bp->b_bcount > 0)){
					bcopy( bp->b_un.b_addr, ap->sta[i].stap,
				      	       bp->b_bcount);
				}

				/*
				 *  Flush/invalidate the system and IO cache.
				 */
				(void)d_cflush(ap->dma_channel, ap->sta_vaddr +
				                (i * STA_SIZE), STA_SIZE,
						ap->sta_raddr + (i * STA_SIZE));
				return( 0 );
			}
		}
		if( i == NUM_STA ) {
			return( ENOMEM );
		}
	}

	/*
	 *  Calculate the number of TCW's needed for the transfer.
	 *  We must map one TCW for each page that contains part of
	 *  the input buffer.
	 */
	num_tcws = ((ulong)(bp->b_un.b_addr + ctl_elem->pds_data_len - 1 ) >> 
		   L2_PAGE ) - ((ulong)bp->b_un.b_addr >> L2_PAGE ) + 1;


	if( num_tcws > 0 ) {
		/*
		 *  asc_alloc_tcws() will find the necessary number
		 *  of contiguous TCW's to satisfy the transfer, 
		 *  marks TCW's as allocated and bumps a count 
		 *  of the number of TCW's used. The count is used to ensure
		 *  that the threshold is not surpassed. A threshold of 30 
		 *  is maintained so page faults can be serviced quickly. 
		 *  The threshold ensures that if 30 or less TCWs remain in 
		 *  the pool, only requests of 1 page or less are satisfied. 
		 */
		start_bit = asc_alloc_tcws( ap, ctl_elem );
		if( start_bit == -1 ) {
			/* No bus memory resources available. */
			return ( ENOMEM );
		}
		
		/*
		 *  Translate start bit to a bus memory address.
		 *
		 *  A start bit has a one to one correspondence to
		 *  a TCW(ie. if base_addr is 0x40000 then corresponding
		 *  start bit is 0, 0x41000 is 1, etc).
		 * 
		 *  Given a range of TCW map bits, use the following
		 *  equation to determine the corresponding contiguous 
		 *  bus memory addr's:
		 *  start = ((base_addr & 0xFFFFF000 ) - ap->dma_base) >> 12;
		 */
		start_addr = (( start_bit << 12 ) + (ulong) ap->busmem_end );

		/* 
		 *  Add the page offset of the virtual address to the 
		 *  real address.
		 */
		start_addr |=  (int)bp->b_un.b_addr & PAGE_OFFSET;

		/* Save the beginning addr to give to the adapter. */
		ctl_elem->pd_info->mapped_addr = start_addr;

		/*
		 *  Map the buffer for DMA.
		 */
		dma_flags = ((bp->b_flags & B_READ) ? DMA_READ : 0) |
			    ((bp->b_flags & B_NOHIDE) ? DMA_WRITE_ONLY : 0);

		/*
		 *  Map the buffer for DMA transfer.
		 */
		d_master( ap->dma_channel, dma_flags, bp->b_un.b_addr, 
			  bp->b_bcount, &bp->b_xmemd, (char *)start_addr);

		/*  Bump start address to next available TCW. */
		start_addr += bp->b_bcount;

		/*
		 *  Walk the linked list of buf structs. Allocate and map 
		 *  TCW(s) for each. bp has already been assigned to the
		 *  beginning of the chain of bufs.
		 */
		running_count = bp->b_bcount;
		bp = bp->av_forw;
		while ( running_count < ctl_elem->pds_data_len ) {
			ASSERT( bp != NULL );
			ASSERT(( (ulong)bp->b_un.b_addr & PAGE_OFFSET ) == 0 );

			/*
			 *  Determine number of TCW's
			 *  used for this buf struct.
			 */
			num_tcws = ((bp->b_bcount - 1) >> 12) + 1;

			/*
			 *  Map the buffer for DMA transfer.
			 */
			d_master( ap->dma_channel, dma_flags, bp->b_un.b_addr, 
				  bp->b_bcount,&bp->b_xmemd,(char *)start_addr);

			/*
			 *  Bump start address to next available TCW.
			 */
			start_addr += bp->b_bcount;
			running_count += bp->b_bcount;
			bp = bp->av_forw;
		}
	}
	return ( 0 );
}

/*
 * NAME:	asc_translate 
 *
 * FUNCTION: 
 *		Translate callers request control element to an 
 *		adapter request element.
 *
 * EXECUTION ENVIRONMENT:
 *		Interrupt level only.
 *
 * NOTES:	
 *		The protocol driver passes a SCB control element that
 *		is of a different format than what this adapter understands.
 *		This routine translates the protocols control element
 *		to an element supported by this adapter.
 *		
 *  		Layout of the option field of a request element.
 *		TTTT TTTT TTTT TTTT M0IC LLLL 0NSL AURD
 *
 *		T - Global timeout duration for the request.
 *		M - Units of time indicated by T.
 *		X - Request specific.
 *		I - IO port data enable, indicates buffer in IO space.
 *		C - Constant address enable, used for Slave peers.
 *		L - Length of SCSI cmd block to send to adapter.
 *		N - No disconnect.
 *		S - Supress exception short.
 *		L - Loop scatter/gather enable.
 *		A - Automatic Request Sense enable.
 *		U - Untagged command.
 *		R - Reactivate SCSI command queue.
 *		D - Direction of data transfer.
 *
 * CALLED FROM:
 *		adp_start
 *		
 * INPUT:
 *		ap		- adapter structure
 *		ctrl_elem_blk 	- control element block
 *
 * RETURNS:	
 *		0		- on success
 *		EIO		- if PIO exception
 *		PIPE_FULL	- if pipe full condition
 */
int	    
asc_translate(
	struct	adapter_info	*ap,
	struct	ctl_elem_blk 	*ctl_elem )
{
ulong	rc;
ulong	punlunbus;
ulong	id, lun;
ulong	options = 0;
ulong	exp = 0;
ushort	*type2;	
ushort	eid;
uint	*scb;
uchar	eid_val;
uchar	chain;
struct	send_scsi	*free_ptr;
struct	pd_info		*pd;

	/* 
	 *  Get pointer to the request control element(located in 
	 *  the ctl_elem_blk) and the pointer to the first parameter
	 *  descriptor. The first parameter descriptor contains a pointer
	 *  to a buf struct(s) that describes the buffer. The SCSI command 
	 *  buffer is contained in the ctl_elem.
	 */
        pd = ctl_elem->pd_info;
        ASSERT( pd->buf_type == P_BUF );
        scb = (uint *)ctl_elem->ctl_elem + 
				(sizeof(struct scb_header) / BYTES_PER_WORD) + 
				( PD_SIZE * 2 );

	/*
	 *  Calculate the entity id associated with this device
	 *  based on the SCSI id/LUN/bus information. 
	 *  The bus number is located in the type 2 PD in the ctl_elem.
	 *  The SCSI id and LUN are located in the SCSI command buffer
	 *  which is also in the ctl_elem block.
	 */
	type2 = (ushort *) ctl_elem->ctl_elem + (sizeof(struct scb_header) / 2);
	punlunbus = type2[3] << BUS_SHIFT;
	id = scb[IDLUN_WORD] >> ID_SHIFT;
	lun = scb[IDLUN_WORD] & LUN_MASK;
	punlunbus |= (id << 5) | lun;
	eid = CALC_ID( punlunbus );
	eid_val = ap->dev_eid[eid];

	/* 
	 *  Build the option field of the request element.
	 */

	/*  Set chaining bits based on QTAG message field. */
	switch( scb[FLAGS_WORD] & QTAG_MASK ) {
		case	ASC_SIMPLE_Q:
		case	ASC_NO_Q:
				chain = NOCHAIN;
				break;
		case	ASC_ORDERED_Q:
				chain = MIDDLECHAIN;
				break;
		case	ASC_HEAD_OF_Q:
				chain = MIDDLECHAIN;
				exp = EXPEDITED;
				break;
		default:
				break;
	}

	options = SUPRESS_SHORT;

	/*  Determine if the No Disconnect option was requested. */
	if( scb[FLAGS_WORD] & ASC_NO_DISC ) {
		options |= ( 0x00000040 | ASC_UNTAG_CMD );
	}

	/* Determine if the Untagged command bit shoud be set. */
	if(( scb[FLAGS_WORD] & QTAG_MASK ) == ASC_NO_Q ) {
		options |= ASC_UNTAG_CMD;
	}

	/*  Calculate the length of the SCSI command. */
	options |= ( scb[FLAGS_WORD] & SCSI_LENGTH_MASK ) >> 16 ;

	/*  Determine the direction of the data transfer. */
	options |= ( scb[FLAGS_WORD] & SCSI_DIRECTION_MASK ) >> 12;

	/*  Determine if the reactivate device queue bit should be set. */
	options |= ( scb[FLAGS_WORD] & SCSI_REACTIVATE_MASK ) >> 9;

	/*  Determine if the command should be expedited. */
	if( ctl_elem->flags & EXPEDITE ) {
		exp = EXPEDITED;
	}

	/*
	 *  Get a pointer to the beginning of free space on the
	 *  outbound(enqueue) pipe.
	 */
	rc = asc_get_start_free( ap, SEND_SCSI_LENGTH, (uint **)&free_ptr );
	if (rc != 0) {
		/* 
		 *  Pipe full condition, the element
		 *  will not fit onto the pipe.
		 *  Return PIPE_FULL error.
		 */
		return( PIPE_FULL );
	}
	ASSERT( free_ptr != NULL );

	/*  
	 *  BUILD THE REQUEST ELEMENT DIRECTLY ONTO PIPE. 
	 *
	 *  The buffer(s) associated with this command are
	 *  described in the pd_info structure.
	 *  The addresses have already been translated to real 
	 *  addresses and mapped for DMA.
	 *  If no pd_info structures exist, then no buffers
	 *  are associated with this command.
	 *  Note: The SCSI command block should not be byte-reversed. 
	 */
	qrev_writel( SEND_SCSI_LENGTH, (uint *)&(free_ptr->header.length));
	qrev_writel( REQUEST_EID | SEND_SCSI | chain | exp, 
		    (uint *)&(free_ptr->header.indicators));
	qrev_writes( eid_val | (ap->adp_uid << 8), &(free_ptr->header.dst));
	free_ptr->header.src = 0x0;
	qrev_writel( (uint)ctl_elem, (uint *)&(free_ptr->header.correlation));
	qrev_writel( options, (uint *)&(free_ptr->options));
	free_ptr->auto_rs = 0x0;
	free_ptr->auto_rs_len = 0x0;
	bcopy((char *)&scb[4], &free_ptr->scsi_cmd[0], 0xC);
	qrev_writel( ctl_elem->pds_data_len, (uint *)&(free_ptr->total_len));
	qrev_writel( ctl_elem->pd_info->mapped_addr,
		    (uint *)&(free_ptr->buf_addr));
	qrev_writel( ctl_elem->pds_data_len, (uint *)&(free_ptr->buf_size));

	/*
	 *  Notify the adapter that new element(s)
	 *  exist on the pipe and update the queue pointers.
	 */
	if(( rc = asc_update_eq( ap, (uchar *)free_ptr, 
				SEND_SCSI_LENGTH )) != 0 ) {
		return( EIO );
	}

#ifdef ASC_TRACE
	asc_trace(ap, ctl_elem, 0); 
#endif /* ASC_TRACE */

	/*
	 *  The element was enqueued on the outbound 
	 *  pipe successfully.
	 *  Remove the ctl_elem from the head of
	 *  wait list, append to send list,
	 *  and decrement queued counter.
	 */
	ASSERT( ctl_elem == ap->wait_head );
	ap->wait_head = ctl_elem->next;
	if( ap->wait_head == NULL )
		ap->wait_tail = NULL;
	else
		ap->wait_head->prev = NULL;
	ap->num_cmds_queued--;
	
	/*
	 *  Add the ctl_elem to the end of the active list.
	 *  Note: critical that interrupts remain disabled
	 *  till after the element is enqueued onto the send
	 *  list. If driver serviced an interrupt for a command 
	 *  before it was pushed onto the send list, the driver
	 *  would get very sick.
	 */
	if ( ap->active_head == NULL ) {
		ap->active_head = (struct ctl_elem_blk *)ctl_elem;
		ctl_elem->next = NULL;
		ctl_elem->prev = NULL;
		ap->active_tail = (struct ctl_elem_blk *)ctl_elem;
	}
	else {
		ap->active_tail->next = 
				(struct ctl_elem_blk *)ctl_elem;
		ctl_elem->prev = ap->active_tail;
		ctl_elem->next = NULL;
		ap->active_tail = (struct ctl_elem_blk *)ctl_elem;
	}
	ap->num_cmds_active++;
	return(0);
}

/*
 * NAME:	asc_alloc_tcws 
 *
 * FUNCTION: 
 *		Allocate a contiguous pool of Translation Control
 *		Words( TCWS ).
 *
 * EXECUTION ENVIRONMENT:
 *		Interrupt level only.
 *
 * NOTES:	
 *		TCWs are used to map system memory to bus memory
 *		in order for the adapter to do DMA.
 *
 * INPUT:
 *		ap		- adapter structure
 *		ctrl_elem_blk 	- control element block
 *
 * RETURNS:	
 *		-1		- if no TCW's were allocated.
 *		start_addr	- index of the 1st TCW in the range.
 */		
int 
asc_alloc_tcws(
	struct	adapter_info	*ap,
	struct	ctl_elem_blk	*ctl_elem )
{
ulong	vaddr;				/* system memory virtual address */
ulong	need;				/* number of TCWs needed */
ulong	start_addr;	
ulong	new_need;
ulong	slide;
ulong	mask;
ulong	last_mask;
uchar	shift;
uchar	word;
uchar	last_word;
uchar	curr_word;
uchar	n_words_touched;
uchar	n_word_1;
uchar	start_bit;
uchar	num_free;
uchar	new_num_free;
uchar	pass2 = FALSE;
uchar	found = FALSE;
struct	buf	*bp;


	/* VERIFY THAT ALL BUFFERS ARE PAGE ALIGNED. */

	/*
	 *  Get the starting virtual address of the request.
	 */
	bp = (struct buf *) ctl_elem->pd_info->p_buf_list;
	vaddr = ( ulong ) bp->b_baddr;

	/*
	 * Calculate number of TCW's needed with following equation:
	 *
	 *  ((offset_into_page + length_of_xfer - 1) / (pagesize)) + 1
	 *
	 *  Example of Ending Page Aligned Transfer:
	 *
	 *  P              P               P              P
	 *  |--------------|---------------|--------------|  = Page boundaries
	 *       ^-------------------------^                 = data buffer
	 *  ^----^                                           = offset into page
	 *  |XXXXXXXXXXXXXX|XXXXXXXXXXXXXXX|--------------|  = TCW's needed
	 *
	 *     offset + length - 1 = 0x1FFF;  and (0x1FFF / 0x1000) + 1 = 2
	 *
	 *  Example of Beginning Page Aligned Transfer:
	 *
	 *  P              P               P              P
	 *  |--------------|---------------|--------------|  = Page boundaries
	 *  ^-------------------------^                      = data buffer
	 *  ^                                                = offset into page
	 *  |XXXXXXXXXXXXXX|XXXXXXXXXXXXXXX|--------------|  = TCW's needed
	 *
	 *     0 + length - 1 = 0x1???;  and (0x1??? / 0x1000) + 1 = 2
	 *
	 *  Example of Page Aligned Transfer:
	 *
	 *  P              P               P              P
	 *  |--------------|---------------|--------------|  = Page boundaries
	 *  ^------------------------------^                 = data buffer
	 *  ^                                                = offset into page
	 *  |XXXXXXXXXXXXXX|XXXXXXXXXXXXXXX|--------------|  = TCW's needed
	 *
	 *     0 + 0x2000 - 1 = 0x1FFF;  and (0x1FFF / 0x1000) + 1 = 2
	 *
	 *  Example of Un-Aligned Transfer:
	 *
	 *  P              P               P              P
	 *  |--------------|---------------|--------------|  = Page boundaries
	 *        ^------------------------------^           = data buffer
	 *  ^-----^                                          = offset into page
	 *  |XXXXXXXXXXXXXX|XXXXXXXXXXXXXXX|XXXXXXXXXXXXXX|  = TCW's needed
	 *
	 *     offset + length - 1 = 0x2???;  and (0x2??? / 0x1000) + 1 = 3
	 */
 
	/* Determine number of TCWs needed. */
	need = (((vaddr & (PAGESIZE-1)) + ctl_elem->pd_info->total_len - 1) / 
							PAGESIZE) + 1;

	/*
	 *  Initialize shift and word indexes to start searching
	 *  at optimal point in bitmap.
	 */
	shift = ap->shift;
	word = ap->tcw_word;
	n_words_touched = 1;    /* number of words touched */
	do {
	 	 /* Set slide to right word. */
		slide = ap->tcw_table[word] << shift;

		/* Find the first free resource. */
		start_bit = (uchar) asc_cntlz( slide );
		if ( start_bit == WORDSIZE ) {
			/*
			 *  If this word is empty, clear start bit, increment
			 *  word and go to top of loop.
			 */
			if( ++word >= ap->num_tcw_words ) {
				if (pass2) {
					/* Out of words, break out. */
					break;
				}
				else {
					word = ap->tcw_word = 0;
					shift = ap->shift = 0;
					pass2 = TRUE;
				}
			}
			shift = 0;
			continue;
		}

		/*
		 *  Shift out bits before first free
		 *  while complementing -> now 1 = used, 0 = free
		 */
		slide = ~(slide << start_bit);
		shift += start_bit;

		/*
		 * Count the number of free bits in this word.
		 */
		num_free = (uchar) asc_cntlz( slide );
		if( num_free >= need ) {
		 	 /* Found enough to satisfy need. */
			found = TRUE;
		} 
		else 
		if( (num_free + shift) == WORDSIZE ) {
			/*
			 *  See if we need to cross a window.
			 *  Compute new need.
			 */
			new_need = need - num_free;
			new_num_free = 0;
			curr_word = word;

			do {
				if (++curr_word >= ap->num_tcw_words) {
					/*  Out of words. */
					break;
				}

				/* inc # words touched */
				n_words_touched++;
		
				/* Set slide to complement of next word. */
				slide = ~ap->tcw_table[curr_word];
	
				/*
				 *  Compute how many free in next word.
				 */
				new_num_free = (uchar) asc_cntlz( slide );
				if (new_num_free >= new_need) {
					/*
					 * Found what we need, set
					 * found which will cause us to
					 * exit, and word and shift 
					 * will be set correctly.
					 */
					found = TRUE;
				} 
				else 
			  	if(new_num_free == WORDSIZE ) {
					/*
				 	 * Didn't find what we need, but
					 * found all we could in this word,
					 * update number needed and try next
					 * word.
					 */
					new_need -= WORDSIZE;
					new_num_free = 0;
				} 
				else {
					/*
					 * Didn't find them, update word to
					 * where we are and continue.
					 */
					break;
				}
		} while (new_need > new_num_free);

			if (!found) {
				/*
				 *  Update word to where we searched to.
				 */
				word = curr_word;
				if (word >= ap->num_tcw_words) {
					if (pass2) {
						/* Out of words, break out. */
						break;
					}
					else {
						word = ap->tcw_word = 0;
						shift = ap->shift = 0;
						pass2 = TRUE;
					}
				}
				shift = 0;

			 	/*  Reset number of words touched to 1. */
				n_words_touched = 1;
				continue;
			}

		} 
		else {
			/*
		 	 * Didn't find the amount we needed, so cleanup,
		 	 * update shift and try again.
		 	 */
			shift += num_free;
		}

	} while (!found);

	if (found) {
		/*
		 * Mark as in use.
		 */
		if (n_words_touched == 1) {
			/*  Only touched 1, all in word 1. */
			n_word_1 = need;

			/* Update shift for next time around. */
			ap->shift = shift + need;
			if (ap->shift == WORDSIZE)
			ap->shift = 0;
		} 
		else
			/*
		 	 * compute # in word 1 (i.e. #bits in word -
		 	 * # we shifted out
		 	 */
		 	 n_word_1 = ( WORDSIZE - shift);
	
		/*
		 * Create bit map to clear appropriate bits, setting those
		 * TCW's as in use
		 */
		mask = ((ALL_ONES << (WORDSIZE - n_word_1)) >> shift);
	
		/*
		 * Clear appropriate bits.
		 */
		ASSERT(((ap->tcw_table[word] & mask) == mask));
		ap->tcw_table[word] &= ~mask;
	
 		/* Record starting TCW */
		start_addr =  shift + (word * WORDSIZE );

		/*
		 * Now Handle spanned words.
		 */
		last_word = word + n_words_touched - 1;

		/*
		 * Update starting tcw word for next time around.
		 */
		ap->tcw_word = last_word;
		word++;
		for ( ; word <= last_word ; word++) {
			if (word == last_word){
				/*
				 * If this is the last word
				 * then build a mask for it, i.e. total number
				 * needed - how many we found in the first word,
				 * mod word size
				 * NOTICE: this mask set up so that an OR will
				 * mark the bits free, to clear we must NOT the
				 * mask and AND.
				 */
				last_mask = (ALL_ONES >>
					((need - n_word_1) % WORDSIZE));


				if (last_mask != ALL_ONES) {
				       mask = ~last_mask;
				       /*
				        * Update shift for the next time 
				        * so we'll immediately look at the 
				        * TCW's following these we've just 
				        * allocated
				        */
				       ap->shift =(uchar)asc_cntlz(last_mask);
				} 	
				else {
					mask = last_mask;

					/* Update shift to 0 for next time. */
					 ap->shift = 0;
				}

				/*
				 * Clear appropriate bits.
				 */
                                ASSERT((ap->tcw_table[word] & mask) == mask);
				ap->tcw_table[word] &= ~mask;
			} 
			else {
				/* Else mark them all. */
				ASSERT(ap->tcw_table[word] == (uint)ALL_ONES);
				ap->tcw_table[word] = 0;
			}
		}
		return( start_addr );
	} 
	else {
		/*
		 * TCW's not available.
		 */
		return( -1 );
	}
}
