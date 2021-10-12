static char sccsid[] = "@(#)39	1.13  src/bos/kernext/scsi/asc_top.c, sysxscsi, bos41J, 9525G_all 6/22/95 09:00:08";
/*
 *   COMPONENT_NAME: SYSXSCSI
 *
 *   FUNCTIONS: asc_add_filter
 *		asc_add_status
 *		asc_close
 *		asc_config
 *		asc_del_filter
 *		asc_del_status
 *		asc_init_adapter
 *		asc_open
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
 * NAME:        asc_top.c
 *                      
 * FUNCTION:    IBM SCSI Adapter Driver Source File
 *                                                 
 *      This adapter driver is the interface between a SCSI device 
 *      driver and the actual SCSI adapter.  It executes commands 
 *      from multiple drivers which contain generic SCSI device  
 *      commands, and manages the execution of those commands.  
 *      Several ioctls are defined to provide for system management 
 *      and adapter diagnostic functions.                          
 */

#include	"ascincl.h"

/* Global adapter device driver lock word.  */
lock_t	asc_lock = LOCK_AVAIL;

/*  Global storage area used to manage all adapters. */
extern	adp_ctrl_t	adp_ctrl;

/*  EPOW handler struct. */
extern	struct	intr	epow;
extern	Simple_lock	epow_mp_lock;

/*  Global device driver component dump table pointer. */
extern	struct asc_cdt_tab *asc_cdt;

/*  Common demux structure used by the Network services. */
struct	ns_demuxer	demux;


/*
 * NAME:	asc_config 
 *
 * FUNCTION: 
 *		Adapter driver's configuration entry point.
 *
 * EXECUTION ENVIRONMENT:
 *		Process level only.
 *
 * NOTES:
 *
 *		If op is INIT, the driver is attached to the
 *		NS services, resources are allocated and 
 *		initialized.
 *
 *		If op is TERM, the driver is detached from the
 *		Network Services and resources are deallocated.
 *
 *		If op is  CFG_QVPD, return the adapter Vital Product Data.
 *		This adapter does not support the extended POS mechanism
 *		for getting VPD data. Instead, a command must be issued
 *		to retrieve the data from the adapter.
 *
 * EXTERNAL CALLS:
 *		lockl, unlockl, xmalloc, xmfree, ns_attach, ns_free
 *		ns_add_demux, ns_del_demux
 *		
 * INPUT:
 *		op	- option code(INIT, TERM, QVPD)
 *		uiop    - pointer to uio structure
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *		ENODEV	- device not defined
 *		ENOMEM	- error allocating resources
 *		EIO	- kernel service failure or invalid operation
 */
int
asc_config(
	int op,
	struct uio * uiop)
{
struct	adapter_info	*ap;
struct	ndd_config	cfg;		
int	rc = 0;

	/* Acquire the global lock. */
	(void) lockl(&(asc_lock), LOCK_SHORT);

	/*
	 *  Copy(uiomove) the passed in ndd_config structure
	 *  into the local ndd_config structure.
	 */
	if(( rc = uiomove((caddr_t)&cfg,(int) sizeof( 
			 struct ndd_config), UIO_WRITE,
			(struct uio *)uiop )) != 0 ) {
		unlockl( &asc_lock );
		return( EIO );
	}
	
	/* Get the adapter structure, hash on sequence number. */
 	HASH( ap, cfg.seq_number );

	switch ( op ) {
		case	CFG_INIT:
			/*
			 *  Configure the adapter driver.
			 */
			if( ap != NULL) {
				/* Adapter already configured. */
				rc = EIO;	
				break;
			}

			/* 
			 *  Allocate memory for the adapter structure 
			 *  from the pinned_heap.
			 */
			ap = (struct adapter_info *) xmalloc(
						sizeof(struct adapter_info), 
						12, pinned_heap );

			if( ap == NULL ) {
				rc = ENOMEM;	
				break;
			}

			/* Zero out the adapter structure. */
			bzero((char *) ap, (int) sizeof(struct adapter_info));

			/*
			 *  Copy the DDS structure from user space.
			 */	
			if(( rc = copyin(cfg.dds, (caddr_t)&(ap->ddi),
				       sizeof(struct ascsi_ddi))) != 0) {
				(void) xmfree( ap, pinned_heap );
				rc = EIO;
				break;
			}

			/*
			 *  Calculate maximum transfer size(MTU)
			 *  based on the size of the TCW range.
			 */
			if( ap->ddi.tcw_length > ( 0X100000 + PAGESIZE ))
				ap->mtu = ap->ddi.tcw_length - (0X100000 +
						   PAGESIZE ) + MAXREQUEST;
			else
				ap->mtu = MAXREQUEST;


			/*
			 *  Register the driver to the network
			 *  services( ns_attach ).
			 *
			 *  Network services will add this ndd structure 
			 *  to its list of NDD devices.
		 	 *  The virtual device(vscsiN) will open 
			 *  us via ns_alloc.
			 *  The ns_attach should be done once per
			 *  adapter, one ndd structure per adapter
			 *  will be needed(ie. the ndd structure
		 	 *  describes one adapter only).
			 *  
			 *  The order in which ns_attach and 
			 *  ns_add_demux are performed is not important.
			 *
			 *  Fill in the ndd structure.
			 */
        		ap->ndd.ndd_name = ap->ddi.resource_name;
			ap->ndd.ndd_flags = 0;
			ap->ndd.ndd_correlator = ( caddr_t )ap;
        		ap->ndd.ndd_open = (int (*) ()) asc_open;
        		ap->ndd.ndd_close = (int (*) ()) asc_close;
        		ap->ndd.ndd_output = (int (*) ()) asc_output;
        		ap->ndd.ndd_ctl = (int (*) ()) asc_ioctl;
        		ap->ndd.nd_receive = (void (*) ()) asc_stub;
        		ap->ndd.nd_status = (void (*) ()) asc_stub;
        		ap->ndd.ndd_mtu = ap->mtu; 
        		ap->ndd.ndd_mintu = 0;
        		ap->ndd.ndd_type = NDD_SCSI;
        		ap->ndd.ndd_addrlen = 0;
        		ap->ndd.ndd_hdrlen = 0;
        		ap->ndd.ndd_physaddr = 0;
        		ap->ndd.ndd_demuxer = 0;

			/*
			 *  Attach our NDD to the chain of NDD structures.
			 */
			if(( rc = ns_attach( &ap->ndd )) != 0 ) {
				(void) xmfree( ap, pinned_heap );
				rc = EIO;
				break;
			}

			if ( adp_ctrl.num_of_cfgs == 0 ) {
				/*
				 *  Register the adapter driver as a demuxer.
				 *  
				 *  The function pointers in the demux struct
				 *  are initialized with stub routines
				 *  to guard against fatal accesses.	
				 *
				 *  For SCB compatibility, the demux struct is
				 *  a static struct. Some SCB demuxers 
				 *  depend on the same demux struct being used 
				 *  for both the ns_add_demux/ns_del_demux 
				 *  calls. 
				 */
				demux.nd_add_filter = asc_add_filter;
				demux.nd_del_filter = asc_del_filter;
				demux.nd_add_status = asc_add_status;
				demux.nd_del_status = asc_del_status;
				demux.nd_receive = (void (*) ()) asc_stub;
				demux.nd_status = 0;
				demux.nd_response = 0;
				demux.nd_inuse = 0;
				demux.nd_use_nsdmx = TRUE;
				if(( rc = ns_add_demux(NDD_SCSI,&demux)) != 0) {
					ns_detach( &ap->ndd );
					(void) xmfree( ap, pinned_heap );
					rc = EIO;
					break;
				}
			}
			adp_ctrl.num_of_cfgs++;

			/*
			 *  Initialize remaining ap fields.
			 */
			ap->seq_number = cfg.seq_number;
			ap->wdog.dog.func = (void (*) () ) asc_wdt_intr;
			ap->wdog.ap = ap;
			ap->wdog.reason = INVAL_TMR;
			ap->tm.dog.func = (void (*) () ) asc_wdt_intr;
			ap->tm.ap = ap;
			ap->tm.reason = MOVE_TMR;
			ap->adp_cmd_pending = 0;
			ap->locate_event = EVENT_NULL;
			ap->rir_event = EVENT_NULL;
			ap->vpd_event = EVENT_NULL;
			ap->eid_event = EVENT_NULL;
			ap->ebp_event = EVENT_NULL;
			ap->adp_uid = ap->ddi.slot + ((ap->ddi.bus_id & 
						BUS_MASK) * 7);

			/*
			 *  Save pointer to the adapter structure
			 *  in the global list of ap pointers.
			 */
			adp_ctrl.ap_ptr[cfg.seq_number] = ap;
			break;

		case	CFG_TERM:
			/*
			 *  Unconfigure the driver.
			 */
			if(ap == NULL) {
				/* Adapter is not configured. */
				rc = 0;
				break;
			}

			/* If adapter still in use, fail. */
			if ( ap->opened ) {
				rc = EBUSY;
				break;
			}

			/*
			 *  Remove the adapter driver as
			 *  a demuxer.
			 */
			if( adp_ctrl.num_of_cfgs == 1 ) {
				(void ) ns_del_demux( NDD_SCSI );
			}
			
			/*
			 *  Remove the adapter driver from 
			 *  the NS services
			 */
			if(( rc = ns_detach( &ap->ndd )) != 0 ) {
				ASSERT(0);
			}

			/*
			 *  xmfree the memory used for the 
			 *  adapter struct
			 */
			(void) xmfree( ap, pinned_heap );

			/* 
			 *  Remove the ap ptr from the global list.
			 */
			adp_ctrl.ap_ptr[cfg.seq_number] = NULL;
			adp_ctrl.num_of_cfgs--;

			break;

		case	CFG_QVPD:
			/*
			 *  Query Vital Product Data(VPD).
			 */
			if(ap == NULL) {
				/*
				 *  Device must be configured
				 *  before we can read VPD.
				 */
				rc = ENODEV;
				break;
			}

			/*
			 *  To get the adapter VPD, a
			 *  Get Adapter Information command 
			 *  must be issued to the adapter.
			 *  
			 *  Open and start the device,
			 *  read adapter info and close device.
			 * 
			 *  Note: open() and config() contend for the same
			 *  lockword. open()/close() must use a unique 
			 *  lockword(semaphore).
			 */
			if ( ! ap->opened ) {
				if( adp_ctrl.semaphore == FALSE )
					adp_ctrl.semaphore = TRUE;
				else {
					rc = EIO;
					break;
				}

				if(( rc = asc_open( &ap->ndd)) != 0 ) {
					rc = EIO;
					if( adp_ctrl.semaphore == TRUE )
						adp_ctrl.semaphore = FALSE;
					break;
				}
				else {
					/* Flag indicating close needed. */
					ap->vpd_close = TRUE;
				}
			}

			/*
			 *  Build and send a Get Adapter
			 *  Info cmd to get the VPD.
			 */
			if (( rc = asc_get_adp_info( ap, &cfg )) != 0 ) {
				if( ap->vpd_close )
					asc_close( &ap->ndd );
				rc = EIO;
				if( adp_ctrl.semaphore == TRUE )
					adp_ctrl.semaphore = FALSE;
				break;
			}

			if( ap->vpd_close ) {
				if ((rc = asc_close( &ap->ndd ))) {
					rc = EIO;
				}
				ap->vpd_close = FALSE;
			}
			if( adp_ctrl.semaphore == TRUE )
				adp_ctrl.semaphore = FALSE;
			break;

		default:
				rc = EINVAL;
				break;
	}
	unlockl( &asc_lock );
	return( rc );
}


/*
 * NAME:     	asc_open
 *
 * FUNCTION: 
 *		The adapter driver open routine.
 *
 * EXECUTION ENVIRONMENT:
 *		Process level only.
 *
 * NOTES:
 *		The open routine initializes and starts the adapter
 *		and allocates resources necessary to manage the adapter.
 *		The adapter driver's open routine will be called
 *		once per adapter( ie. the ns services invokes open on the 
 *		first ns_attach only ). 
 *
 * CALLED FROM:
 *		NS kernel services
 *
 * EXTERNAL CALLS:
 *		pincode, unpincode, lockl, unlockl
 *
 * INPUT:
 *		ndd struct - our ndd struct 
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *		EBUSY	- device already opened
 *		EIO	- kernel service failure or invalid operation
 */
int
asc_open(
         ndd_t *ndd )
{
struct 	adapter_info	*ap;
int	rc;


	if( (rc = lockl(&(asc_lock), LOCK_NDELAY )) != LOCK_SUCC ) {
		if ( adp_ctrl.semaphore != TRUE )
			(void) lockl(&(asc_lock), LOCK_SHORT);
	}

	/*
	 *  Get the adapter structure for this adapter.
	 *  Note: the ndd_correlator field contains the address
	 *  of the adapter structure.
	 */
	ap = (struct adapter_info *) ndd->ndd_correlator;
	if(( ap == NULL ) || ( ap->opened )) {
		if ( adp_ctrl.semaphore != TRUE )
			unlockl( &asc_lock );
		return ( EIO );
	}

	/* 
	 * Ensure this flag is false so that completion or an RIR 
	 * does not cause an attempt to enable target mode buffers. 
	 */
	ap->tm_bufs_blocked = FALSE;
	
	/*
	 *  If this is the first open issued to
	 *  the adapter driver, pin the bottom
	 *  half of the driver.
	 */
	if ( adp_ctrl.num_of_opens == 0 ) {
		/* Pin the necessary portions of the driver. */ 
		if(( rc = pincode( asc_intr )) != 0 ) {
			if ( adp_ctrl.semaphore != TRUE )
				unlockl( &asc_lock );
			return( EIO );
		}
	}

	/*
	 *  Call the internal initialize routine to
	 *  initialize and enable the adapter.
	 */
	if(( rc = asc_init_adapter( ap )) != 0 ) {
		if ( adp_ctrl.num_of_opens == 0 ) {
			(void) unpincode( asc_intr );
		}
		if ( adp_ctrl.semaphore != TRUE )
			unlockl( &asc_lock );
		return ( EIO );
	}

	/* 
	 *  Set the state flags in the ndd and ap structures.
	 */
	ndd->ndd_flags = NDD_RUNNING;
	ap->opened = TRUE;
	ap->devs_in_use_I = 0;
	ap->devs_in_use_E = 0;
	ap->first_try = TRUE;
	ap->next_id = 1;
	adp_ctrl.num_of_opens++;
	if ( adp_ctrl.semaphore != TRUE )
		unlockl( &asc_lock );
	return ( 0 );
}

/*
 * NAME:     	asc_close
 *
 * FUNCTION: 
 *		The adapter driver close routine.
 *
 * EXECUTION ENVIRONMENT:
 *		Process level only.
 *
 * NOTES:
 *		This routine will halt all activity on the 
 *		adapter and release all resources assigned to it.
 *		It is assumed that no active commands exist
 *		at the adapter that is being closed.
 *  		The protocol layer should know if outstanding 
 *		commands exist and let them complete before
 *  		issuing close.
 *
 * CALLED FROM:
 *		NS services
 *
 * EXTERNAL CALLS:
 *		unpincode, lockl, unlockl
 *		
 * INPUT:
 *		ndd struct - our ndd struct 
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *		EIO	- kernel service failure or invalid operation
 */
int
asc_close(
         ndd_t *ndd )
{
struct 	adapter_info	*ap;
int	rc;

	if( (rc = lockl(&(asc_lock), LOCK_NDELAY )) != LOCK_SUCC ) {
		if ( adp_ctrl.semaphore != TRUE )
			(void) lockl(&(asc_lock), LOCK_SHORT);
	}

	/*
	 *  Get the adapter structure for this adapter.
	 *  Note: the ndd_correlator field contains the address
	 *  of the adapter structure.
	 */
	ap = (struct adapter_info *) ndd->ndd_correlator;
	if(( ap == NULL ) || (! ap->opened )) {
		if ( adp_ctrl.semaphore != TRUE )
			unlockl( &asc_lock );
		return ( EIO );
	}

	/* 
	 *  Disable the adapter via POS and release
	 *  adapter resources.
	 */
	asc_cleanup(ap, 0);

	/*
	 *  Unpin the device driver if this is the last
	 *  open instance.
	 */
	if ( adp_ctrl.num_of_opens == 1 ) {
		/* Unpin the driver. */
		(void) unpincode( asc_intr );
	}

	/* 
	 *  Update the status flags in the ndd and ap structures.
	 */
	ap->opened = FALSE;
	adp_ctrl.num_of_opens--;
	ndd->ndd_flags = 0;
	if ( adp_ctrl.semaphore != TRUE )
		unlockl( &asc_lock );
	return( 0 );
}

/*
 * NAME:	asc_add_filter
 *		
 * FUNCTION: 
 *		This routine registers a receive routine(filter) 
 *		for the specified device and/or allocates the
 *		specified number of buffers for unsolicited
 *		data(Target mode).
 *
 * EXECUTION ENVIRONMENT:
 *		Process level only.
 *
 * INPUT:
 *		ndd	- adapter's ndd structure
 *		filter	- points to the receiver filter to be added.
 *		len	- length of the receive filter.
 *		ns_user - pointer to a ns_user struct. describes user.
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *		EIO	- kernel service failure or invalid operation
 */
int
asc_add_filter(
	ndd_t	*ndd,
	struct  scb_filter	*filter,
	int	len,
	struct	ns_user	*ns_user )
{
int	i, rc;
uchar	*buf_ptr;
ulong	pool_size;
struct	adapter_info	*ap;

	/*
	 *  Get the adapter structure for this adapter.
	 */
	ap = (struct adapter_info *) ndd->ndd_correlator;
	if(ap == NULL) {
		return ( EIO );
	}
	ASSERT( ap->opened );

	/*
	 *  Ensure filtertype is correct.
	 */
	if( filter->filtertype != NDD_SCSI ) {
		return( EIO );
	}

	/* 
	 *  Register the receive function pointer.
	 *  If the entity_id field indicates this is a
	 *  call on behalf of an initiator, save
	 *  the recv_fn for initiators and our work
	 *  is done( no buffers should be requested
	 *  for initiators).
	 */
	if( filter->entity_info.entity_number == SCB_SCSI_INITIATOR ) {
		ASSERT( ns_user->isr != NULL );
		ap->recv_fn = (int (*) ()) ns_user->isr;
		return( 0 );
	}

	/*
	 *  If num_of bufs is non_zero, the caller wants to
	 *  create a buffer pool for unsolicited data( Target mode ).
	 *
	 *  Determine how many buffers were requested and malloc
	 *  that many buf_pool_elem structures to maintain them.
	 *
	 *  When a buffer is filled, the base address will be used
	 *  to calculate an index to the proper buf_pool_elem structure.
	 *  
	 *  Allocate pinned system memory for the buffers.
	 *
	 *  Map the buffers for DMA.
	 *
	 *  Enable maximum allowable buffers to the adapter.
	 *  This adapter can support 256 enabled buffers at any
	 *  one time. If more than 256 were requested, enable
	 *  256 and enable more as they come back from the adapter.
	 */
	if( filter->entity_info.entity_number == SCB_SCSI_TARGET ) {
		if(( filter->entity_info.num_buffers1 > 0 
		    && filter->entity_info.num_buffers1 <= MAX_TM_BUFS )) {
			/*
			 *  Target mode has been enabled and the num_bufs
			 *  field is non_zero. Allocate resources necessary
			 *  for buffer management and fill in the function
			 *  pointers.
			 */
			ASSERT( ns_user->isr != NULL );
			ap->tm_recv_fn = (int (*) ()) ns_user->isr;

			/*
			 *  Verify number of bufs
			 *  does not exceed ap->ddi.tm_tcw_length!
			 */
			if(( filter->entity_info.num_buffers1 * PAGESIZE ) >
					ap->ddi.tm_tcw_length ) {
				return( EIO );
			}
			ap->tm_bufs_tot = filter->entity_info.num_buffers1;
			ap->tm_bufs_at_adp = 0;
			ap->tm_bufs_to_enable = 0;
			ap->tm_head = ap->tm_tail = NULL;

			/*
			 *  Determine the threshold at which to 
			 *  establish another buffer pool.
			 */
			ap->tm_enable_threshold = ap->tm_bufs_tot / 32;
			if(( ap->tm_bufs_tot % 32) != 0 )
			    ap->tm_enable_threshold += 1;

			/* 
			 *  Allocate memory for the target mode buffers,
			 *  the TCW table and the buf_info structures
			 *  to manage them.
			 *  initialize TCW  bit allocation table for the 
			 *  target buffer pool.
			 *  One bit per tcw is needed to manage the pool.
			 *  NOTE: table should be the size of the number
			 *  of buffers requested.
			 */
			pool_size = ( ap->ddi.tm_tcw_length / PAGESIZE) / 8;
			pool_size += TM_BUF_SIZE * BYTES_PER_WORD;
			pool_size += sizeof(struct buf_pool_elem) * 
					filter->entity_info.num_buffers1;
			pool_size += PAGESIZE * 
					filter->entity_info.num_buffers1;


			ap->tm_sysmem = (char *) xmalloc( pool_size, L2_PAGE,
							 pinned_heap );
			if( ap->tm_sysmem == NULL ) {
				return( ENOMEM );
			}
			bzero((char *) ap->tm_sysmem, (int)pool_size );

			/* Divide memory. */
			ap->tm_raddr = (uchar *)ap->ddi.tm_tcw_start_addr;

			ap->tm_buf = ap->tm_sysmem;

			/* Position the buf_pool_elems after the TM buffers */ 
			buf_ptr = (uchar *)ap->tm_buf + ( PAGESIZE * 
			          filter->entity_info.num_buffers1);
			ap->tm_buf_info = (struct buf_pool_elem *)buf_ptr;


			buf_ptr = (uchar *)ap->tm_buf_info  + (sizeof(struct 
			  	   buf_pool_elem) * filter->entity_info.num_buffers1);
			ap->tm_recv_buf = (int *)buf_ptr;


			buf_ptr = (uchar *)ap->tm_recv_buf + (TM_BUF_SIZE *
				  BYTES_PER_WORD);
			ap->tm_tcw_table = (int *)buf_ptr;

			/* Initialize the TCW table. */
			for(i = ((ap->ddi.tm_tcw_length / PAGESIZE) / WORDSIZE)
				   			- 1; i >= 0;i--) {
				ap->tm_tcw_table[i] = 0XFFFFFFFF;
			}
			ap->tm_tcw_table[((ap->ddi.tm_tcw_length / PAGESIZE ) /
				 	WORDSIZE) - 1 ] &= (ALL_ONES << ( 
					WORDSIZE - ((ap->ddi.tcw_length / 
			 		PAGESIZE) % WORDSIZE)));

			/*
			 *  Populate the buf_pool_elem structures with virtual 
			 *  and real addresses.
			 *  Map the buffers for DMA.
			 */
			for(i = 0; i < filter->entity_info.num_buffers1; i++) {
				ap->tm_buf_info[i].virtual_addr = 
					(caddr_t) ap->tm_buf + (i * PAGESIZE );
				ap->tm_buf_info[i].mapped_addr = (caddr_t)  
					((uint)ap->tm_raddr + (i * PAGESIZE));
				ap->tm_buf_info[i].buf_pool_free = 
						(void (*) ())asc_enable_bufs;
				ap->tm_buf_info[i].adap_dd_info = (caddr_t) ap;
				ap->tm_buf_info[i].next = &ap->tm_buf_info[i+1];

				/*
				 *  Map the buffers for DMA.
				 *  Note: should probably map the whole
				 *  buffer range with 1 d_master.
				 */
				d_master(ap->dma_channel, DMA_NOHIDE, 
				     ap->tm_buf_info[i].virtual_addr, PAGESIZE,
				     &ap->xmem, ap->tm_buf_info[i].mapped_addr);
			}
			ap->tm_buf_info[i-1].next = NULL;
			ap->num_buf_cmds = 0;

			/* 
			 *  Enable the buffers at the adapter. 
			 *  When the asc_enable_bufs() routine returns,
			 *  the enable control element has completed
			 *  and has been processed.
			 */
			rc = asc_create_bufs( ap,
					     filter->entity_info.num_buffers1 );
			if( rc != 0 ) {
				ap->tm_recv_fn = (int (*) ()) asc_stub;
				(void) xmfree( ap->tm_sysmem, pinned_heap );
				return( EIO );
			}
		}
		else {
			/* 
		 	 *  Invalid number of buffers were requested 
			 *  so do not allocate buffers.
		 	 */
			return( EIO );
		}
	}
	return( 0 );
}

/*
 * NAME:	asc_del_filter
 *		
 * FUNCTION: 
 *		Clear a receive routine for the specified device.
 *
 * EXECUTION ENVIRONMENT:
 *		Process level only.
 *
 * EXTERNAL CALLS:
 *		None.
 *		
 * INPUT:
 *		ndd	- adapter's ndd structure
 *		filter	- points to the receiver filter to be added.
 *		len	- length of the receive filter.
 *		ns_user - pointer to a ns_user struct. describes user.
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *		EIO	- kernel service failure or invalid operation
 */
int
asc_del_filter(
	ndd_t	*ndd,
	struct  scb_filter	*filter,
	int	len,
	struct	ns_user	*ns_user )
{
struct	adapter_info	*ap;
int	rc;

	/*
	 *  Get the adapter structure for this adapter.
	 *  Note: the ndd_correlator field contains the address
	 *  of the adapter structure.
	 */
	ap = (struct adapter_info *) ndd->ndd_correlator;
	if(ap == NULL) {
		return ( EIO );
	}
	ASSERT( ap->opened );

	/*
	 *  Ensure filtertype is correct.
	 */
	if( filter->filtertype != NDD_SCSI ) {
		return( EIO );
	}

	/* 
	 *  Unregister the receiver function pointer.
	 *  If the entity_id field indicates this a
	 *  call on behalf of an initiator, we clear
	 *  the recv_fn for initiators and our work
	 *  is done(ie. no buffers should be released
	 *  for initiators).
	 */
	if( filter->entity_info.entity_number == SCB_SCSI_INITIATOR ) {
		ASSERT( ap->recv_fn != NULL );
        	ap->recv_fn = (int (*) ()) asc_stub;
		return( 0 );
	}

	/*
	 *  This is a request to disable the Target mode buffers.
	 */
	if( filter->entity_info.entity_number == SCB_SCSI_TARGET ) {
		rc = asc_disable_bufs( ap );
		ap->tm_recv_fn = (int (*) ()) asc_stub;
		(void) xmfree( ap->tm_sysmem, pinned_heap );
		if( rc != 0 ) {
			return( rc );
		}
	}
	return( 0 );
}

/*
 * NAME:	asc_add_status
 *
 * FUNCTION: 
 *		Register a function pointer for asynchronous status.
 *
 * EXECUTION ENVIRONMENT:
 *		Process level only.
 *
 * INPUTS:
 *		ndd	- adapter's ndd structure
 *		status	- points to the status filter to be added.
 *		len	- length of the status filter.
 *		ns_stat - pointer to a ns_user struct.
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *		EIO	- kernel service failure or invalid operation
 */
int
asc_add_status(
	ndd_t	*ndd,
	struct  ns_com_status	*status,
	int	len,
	struct	ns_statuser	*ns_stat )
{
struct	adapter_info	*ap;

	/*
	 *  Get the adapter structure for this adapter.
	 */
	ap = (struct adapter_info *) ndd->ndd_correlator;
	if(( ap == NULL) || (! ap->opened )) {
		return ( EIO );
	}
	
        if( (ulong)status->mask & TAGMASK ) {
        	ap->proto_tag_e = (uchar *)((ulong)status->mask & ~TAGMASK);
	}
	else {
        	ap->proto_tag_i = (uchar *)status->mask;
	}
        ap->ndd.nd_status = ns_stat->isr;
	return( 0 );
}

/*
 * NAME:	asc_del_status
 *
 * FUNCTION: 
 *		Clear a function pointer for asynchronous status.
 *
 * EXECUTION ENVIRONMENT:
 *		Process level only.
 *
 * INPUTS:
 *		ndd	- adapter's ndd structure
 *		statu	- points to the status filter to be added.
 *		len	- length of the status filter.
 *		ns_stat - pointer to a ns_statuser struct.
 *
 *	
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *		EIO	- kernel service failure or invalid operation
 */
int
asc_del_status(
	ndd_t	*ndd,
	struct  ns_com_status	*status,
	int	len,
	struct	ns_statuser	*ns_stat )
{
struct	adapter_info	*ap;

	/*
	 *  Get the adapter structure for this adapter.
	 */
	ap = (struct adapter_info *) ndd->ndd_correlator;
	if(ap == NULL) {
		return ( EIO );
	}
	
        if( (ulong)status->mask & TAGMASK ) {
        	ap->proto_tag_e = 0;
	}
	else {
        	ap->proto_tag_i = 0;
	}
	
        if(( ap->proto_tag_i == 0 ) && ( ap->proto_tag_e == 0 )) {
        	ap->ndd.nd_status = (void (*) ())NULL;
	}
	return( 0 );
}

/*
 * NAME:	asc_init_adapter 
 *
 * FUNCTION: 
 *		Initialize (start) the adapter.
 *
 * EXECUTION ENVIRONMENT:
 *		Process level only.
 *
 * NOTES:
 *		Reset that adapter, set up POS and IO registers, 
 *		allocate and initialize structures needed to 
 *		manage the adapter.
 *
 * CALLED FROM:
 *		asc_open ( at first open of the adapter )
 *
 * EXTERNAL CALLS:
 *		w_init, d_init, d_master, i_init, lock_alloc
 *		e_sleep, io_att, io_det, xmalloc, dump_add
 *		
 * INPUT:
 *		ap	- adapter structure for a particular adapter
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *		ENOMEM	- error allocating resources
 *		EIO	- kernel service failure or invalid operation
 *
 */
int
asc_init_adapter(
	struct	adapter_info	*ap )
{
int	rc, i;
int	pool_size;
ulong	cache_size;
ulong	ioaddr;
ulong	cleanup_depth = 0;
uchar	pio_error = 0;

	/*
	 *  Verify that the adapter structure is in a known state.
	 */
        ASSERT( ap->adapter_mode == LOCATE );
        ASSERT( ap->opened == FALSE );
	ASSERT( ap->active_head == NULL );
	ASSERT( ap->active_tail == NULL );
	ASSERT( ap->wait_head == NULL );
	ASSERT( ap->wait_tail == NULL );
	ASSERT( ap->num_cmds_queued == 0 );

	/*  Register the watchdog timer. */
#ifdef _POWER_MP
	while( w_init(&ap->wdog.dog) );
	while( w_init(&ap->tm.dog) );
#else
	w_init( &ap->wdog.dog );
	w_init( &ap->tm.dog );
#endif
	cleanup_depth++;

	/*
	 *  Allocate one large pool of pinned system memory.
	 *  The pool size is determined to accommodate the following needs: 
	 *  delivery pipes
	 *  ctl_elem_blk pool
	 *  Small Transfer Area
	 *  control area(signalling, status and surrogate areas).
	 *  entity id table(bitmap to manage 256 possible entities).
	 *  TCW table
	 */
	pool_size = PIPESIZE * 2;
	pool_size += sizeof(struct adp_ctl_elem) * NUM_CTRL_ELEMS;
	pool_size += STA_ALLOC_SIZE;
	pool_size += CONTROL_SIZE + 0x140;
	
	/*  Determine number of words needed for the TCW table. */
	if(( ap->ddi.tcw_length / PAGESIZE ) % WORDSIZE ) {
		ap->num_tcw_words = (( ap->ddi.tcw_length / PAGESIZE ) / 
				      WORDSIZE) + 1;
	}
	else {
		ap->num_tcw_words = (( ap->ddi.tcw_length / PAGESIZE ) / 
				       WORDSIZE ); 
	}

	pool_size += ap->num_tcw_words * BYTES_PER_WORD;

	ap->sysmem = (uchar *) xmalloc((uint) pool_size, L2_PAGE, pinned_heap);
 	if( ap->sysmem == NULL ) {
		asc_cleanup(ap, cleanup_depth);
		return( ENOMEM );
	}
	cleanup_depth++;
	ap->sysmem_end = ap->sysmem;

	/* Zero out the allocated memory. */
	bzero((char *) ap->sysmem, (int) pool_size );

	/*
	 *  Allocate(partition) bus memory and system memory.
	 *  We were given the requested bus memory resource
	 *  and have allocated an equivalent system memory resource.
	 *  All memory that the adapter and system share(ie. delivery pipes) 
	 *  will have a system memory address and a corresponding bus 
	 *  memory address. The system accesses the entity via the 
	 *  system memory address and the adapter via the bus memory address.
	 */
	ap->busmem = (uchar *)ap->ddi.tcw_start_addr;
	ap->busmem_end = ap->busmem;

	/*  
	 *  Allocate the delivery pipes.
	 *  Each delivery pipe will be PIPE_SIZE(currently 4K)
	 *  in length. One TCW will be reserved for the Small 
	 *  Transfer Area. Residual space is used for the buffers 
	 *  needed for SCSI commands.
	 */
	ap->eq_raddr = ap->busmem_end;
	ap->busmem_end += PIPESIZE;
	ap->eq_vaddr = ap->sysmem_end;
	ap->sysmem_end += PIPESIZE;
	ap->dq_raddr = ap->busmem_end;
	ap->busmem_end += PIPESIZE;
	ap->dq_vaddr = ap->sysmem_end;
	ap->sysmem_end += PIPESIZE;

	/*
	 *  Allocate the Small Transfer Area(STA).
	 */
	ap->sta_raddr = ap->busmem_end;
	ap->busmem_end += STA_ALLOC_SIZE;
	ap->sta_vaddr = ap->sysmem_end;
	ap->sysmem_end += STA_ALLOC_SIZE;

	/*
	 *  Initialize the STA.
	 */
	for( i = 0; i < NUM_STA; i++ ) {
		ap->sta[i].stap = ap->sta_vaddr + (i * STA_SIZE);
		ap->sta[i].in_use = FALSE;
	}

	/*
	 *  Allocate the Control areas. This includes
	 *  the signalling and status areas.
	 *  The layout of the control area is:
	 *
	 *  0  -----------------------------
	 *    |  OSSF	    |   OSES	    |
	 *  4  -----------------------------
	 *    |  ISSE	    |   ISDS	    |
	 * 80  ----------------------------------> cache line boundary
	 *    |  ISSF	    |   ISES	    |
	 * 84  -----------------------------
	 *    |  OSSE	    |   OSDS	    |
	 *100  ----------------------------------> cache line boundary
	 *    |  PEER UNIT SIGNALLING AREA  |
	 *104  -----------------------------
	 *    |ADAPTER UNIT SIGNALLING AREA |
	 *     -----------------------------
	 *  
	 *  NOTE:
	 *  It is critical that the signalling areas
	 *  are aligned on a cache line boundary. This way
	 *  all status is flushed out of the IO buffers when
	 *  the adapter access the signalling area(ie. crosses
	 *  a cache line boundary).
	 *
	 *  Divide up the delivery pipes further to include
	 *  the signalling and surrogate areas.
	 *  The signalling area will be aligned on a system
	 *  cache boundary and the surrogate areas will reside
	 *  in the previous cache lines. After updating the pipes and
	 *  the surrogate area, the adapter will always write to
	 *  the signalling area, flushing the IO cache. This saves
	 *  the driver from having to issue a d_complete() before
	 *  accessing the pipe. To get the proper system cache line
	 *  size for this platform, use the d_align() library
	 *  routine. d_align() returns the cache line size in LOG2.
	 */

	/* Calculate the cache line size. */
	cache_size = d_align();
	cache_size = 0x1 << cache_size;
	
	ap->surr_ctl.eq_ssf = (ushort *) ap->sysmem_end;
	ap->surr_ctl.eq_ssf_IO = (ushort *) ap->busmem_end;
	ap->surr_ctl.eq_ses  = (ushort *) ap->surr_ctl.eq_ssf + 0x1;
	ap->surr_ctl.eq_ses_IO = (ushort *) ap->surr_ctl.eq_ssf_IO + 0x1;
	ap->surr_ctl.dq_sse = (ushort *) ap->surr_ctl.eq_ssf + 0x2;
	ap->surr_ctl.dq_sse_IO = (ushort *) ap->surr_ctl.eq_ssf_IO + 0x2;
	ap->surr_ctl.dq_sds = (ushort *) ap->surr_ctl.eq_ssf + 0x3;
	ap->surr_ctl.dq_sds_IO = (ushort *) ap->surr_ctl.eq_ssf_IO + 0x3;

	/*
	 *  Not certain d_align() is working properly on all boxes.
	 *  Use 128 cache line size(instead of cache_size) for now.
	 *  NOTE: read-only and write-only surrogate areas MUST
	 *  be in unique cache-lines!
	 */
	ap->sysmem_end += 0x80;
	ap->busmem_end += 0x80;

	ap->surr_ctl.dq_ssf = (ushort *) ap->sysmem_end;
	ap->surr_ctl.dq_ssf_IO = (ushort *) ap->busmem_end;
	ap->surr_ctl.dq_ses = (ushort *) ap->surr_ctl.dq_ssf + 0x1;
	ap->surr_ctl.dq_ses_IO = (ushort *) ap->surr_ctl.dq_ssf_IO + 0x1;
	ap->surr_ctl.eq_sse = (ushort *) ap->surr_ctl.dq_ssf + 0x2;
	ap->surr_ctl.eq_sse_IO = (ushort *) ap->surr_ctl.dq_ssf_IO + 0x2;
	ap->surr_ctl.eq_sds = (ushort *) ap->surr_ctl.dq_ssf + 0x3;
	ap->surr_ctl.eq_sds_IO = (ushort *) ap->surr_ctl.dq_ssf_IO + 0x3;

	ap->sysmem_end += 0x80;
	ap->busmem_end += 0x80;

	ap->surr_ctl.pusa = (ulong *) ap->sysmem_end; 
	ap->sysmem_end += sizeof( ulong );
	ap->surr_ctl.pusa_IO = (ulong *) ap->busmem_end;
	ap->busmem_end += sizeof( ulong );
	ap->surr_ctl.ausa = (ulong *) ap->sysmem_end;
	ap->sysmem_end += sizeof( ulong );
	ap->surr_ctl.ausa_IO = (ulong *) ap->busmem_end;
	ap->busmem_end += sizeof( ulong );

	ap->sysmem_end += cache_size;
	ap->busmem_end += cache_size;

	/*
	 *  Allocate the adapter's ctl_elem_blk pool.
	 *  This pool will be used by the adapter to issue adapter 
	 *  commands.
	 */
	ap->adp_pool = (struct adp_ctl_elem *) ap->sysmem_end; 
	ap->sysmem_end += (sizeof(struct adp_ctl_elem) * NUM_CTRL_ELEMS );

	/* Initialize the ctl_elem_blk pool. */
	for( i = 0; i < NUM_CTRL_ELEMS;  i++ ) {
		ap->adp_pool[i].allocated = FALSE;
		ap->adp_pool[i].event = EVENT_NULL;
		ap->adp_pool[i].ctl_blk.next = NULL;
		ap->adp_pool[i].ctl_blk.prev = NULL;
		ap->adp_pool[i].ctl_blk.reply_elem_len = REPLY_SIZE;
		ap->adp_pool[i].ctl_blk.status = ADAPTER_INITIATED;
		ap->adp_pool[i].ctl_blk.ctl_elem = NULL;
		ap->adp_pool[i].ctl_blk.pd_info = NULL;
		ap->adp_pool[i].ctl_blk.reply_elem = 
			       (caddr_t) &ap->adp_pool[i].reply;
	}

	/*
	 *  Reserve and setup the TCW bit allocation table for the buffer 
	 *  pool. One bit per TCW is needed to manage the pool.
	 *
	 *  NOTE: I think the count leading zeros logic depends on
	 *  a even number of 32-bit words for the tcw table. Check this!
	 */
	ap->tcw_table = (uint *)ap->sysmem_end;
	ap->sysmem_end += ap->num_tcw_words;

	/* Initialize the TCW table. */
	for( i = ((ap->ddi.tcw_length / PAGESIZE) / WORDSIZE) - 1;i >= 0;i--) {
		ap->tcw_table[i] = 0XFFFFFFFF;
	}
	ap->tcw_table[((ap->ddi.tcw_length / PAGESIZE ) / WORDSIZE) - 1 ] &=
			(ALL_ONES << ( WORDSIZE - ((ap->ddi.tcw_length / 
			 PAGESIZE) % WORDSIZE)));

	/*
	 *  Round the bus memory pointer to the next page boundary.
	 *  The remaining bus memory will be used to service
	 *  SCSI commands.
	 */
	ap->busmem_end = (uchar *)(((int)ap->busmem_end  & PAGE_MASK ) +
				   PAGESIZE);
	ap->bufs = ap->busmem_end;

	/* 
	 *  Initialize the intr structure and register 
	 *  the interrupt handler. 
	 */
	ap->intr.next = (struct intr *) NULL;
	ap->intr.handler = asc_intr;
	ap->intr.bus_type = ap->ddi.bus_type;
#ifdef _POWER_MP
	ap->intr.flags = INTR_MPSAFE;
#else
	ap->intr.flags = 0;
#endif
	ap->intr.level = ap->ddi.int_lvl;
	ap->intr.priority = ap->ddi.int_prior;
	ap->intr.bid = ap->ddi.bus_id;
	if(( rc = i_init( &ap->intr )) != INTR_SUCC ) {
		asc_cleanup( ap, cleanup_depth );
		return( EIO );
	}
	cleanup_depth++;
	
	/*
	 *  If this is the first open issued to
	 *  the adapter driver, setup the EPOW intr 
	 *  structure and register the  EPOW handler.
	 */
	if ( adp_ctrl.num_of_opens == 0 ) {
		INIT_EPOW(&epow, (int(*) ()) asc_epow, ap->ddi.bus_id);
		if(( rc = i_init( &epow )) != INTR_SUCC ) {
			asc_cleanup( ap, cleanup_depth );
			return( EIO );
		}
#ifdef _POWER_MP
	(void) lock_alloc((void *)&(epow_mp_lock), LOCK_ALLOC_PIN, 
				ASC_EPOW_LOCK_CLASS, -1 );
	simple_lock_init(&(epow_mp_lock));
	epow.flags |= INTR_MPSAFE;
#endif
		cleanup_depth++;
	
		/* 
	 	 *  Allocate and set up the component 
	 	 *  dump table entry.
	 	 */
		asc_cdt = (struct asc_cdt_tab *) xmalloc((uint)sizeof(
		   	   struct asc_cdt_tab), (uint) 2, pinned_heap);

		if( asc_cdt == NULL ) {
			asc_cleanup( ap, cleanup_depth );
			return ( ENOMEM );
		}
		cleanup_depth++;

		/* 
	 	 *  Initialize the storage for the dump table.
	 	 */
		rc = dmp_add( asc_cdt_func );
		if (rc != 0) {
			asc_cleanup( ap, cleanup_depth );
			return ( EIO );
		}
		cleanup_depth++;
	}
	else {
		cleanup_depth += 3;
	}

	/* Allocate a DMA channel. */
	ap->dma_channel = d_init( ap->ddi.dma_lvl, MICRO_CHANNEL_DMA,
					ap->ddi.bus_id);
	if( ap->dma_channel == DMA_FAIL ) {
		asc_cleanup( ap, cleanup_depth );
		return( EIO );
	}
	cleanup_depth++;

	/* 
	 *  A cross-memory descriptor is necessary in order
	 *  to map memory even if its in kernel space.
	 *  Intialize the adspace_id fo kernel space.
	 *  Use this xmem handle for all address
	 *  spaces the driver manages.
	 */
	ap->xmem.aspace_id = XMEM_GLOBAL;

	/*
	 *  Map the Small Transfer Area for DMA.
	 */
	d_master(ap->dma_channel, DMA_NOHIDE, ap->sta_vaddr, STA_ALLOC_SIZE,
			&ap->xmem, ap->sta_raddr );

	/* 
	 *  Initialize the POS registers.
	 */
	if(( rc = asc_init_pos( ap )) != 0 ) {
		/* Initialize_pos failed. */
		asc_cleanup( ap, cleanup_depth );
		return( EIO );
	}
	cleanup_depth++;

	/*
	 *  Reset the adapter(use the subsystem reset bit in the BCR).
	 *
	 *  Disable interrupts around this sequence to ensure
	 *  that the e_wakeup(interrupt) is not generated before
	 *  e_sleep is registered.
	 *
	 *  Note: when Move mode has been configured, BCR bit 6 
	 *  will be set to allow Clear on Read of the interrupt bit.
	 *
	 *  The BCR's enable chip interrupts and enable DMA bits must be set 
	 *  in a separate store operation. It is not clear why the bits
	 *  cannot be set with the previous store. It appears the BCR is 
	 *  cleared when the reset occurs.
	 */
	ap->locate_state = RESET_PENDING;
	ap->mm_reset_in_prog = FALSE;
	ap->bus_reset_in_prog = FALSE;
	ap->sleep_pending = TRUE;
	ap->epow_state = EPOW_OFF;
	ap->eid_lock = LOCK_AVAIL;

	/* Adapter is initially operating in locate mode. */
	ap->adapter_mode = LOCATE;
		
	/* Attach to IO space. */
	ioaddr = (ulong) io_att( ap->ddi.bus_id, NULL ); 

	/*
	 *  Toggle the reset bit in the BCR.
	 *  WRITE_BCR macro adds the offset of BCR to the base IO addr.
	 */
	WRITE_BCR( ap->ddi.base_addr, BCR_RESET);
	if( pio_error ) {
		asc_cleanup( ap, cleanup_depth );
		io_det( ioaddr );
		return( EIO );
	}

	/* delay at least 1 microsecond between writes to the BCR */
	asc_delay (ap, 1);

  	WRITE_BCR( ap->ddi.base_addr, 0x00);
	if( pio_error ) {
		asc_cleanup( ap, cleanup_depth );
		io_det( ioaddr );
		return( EIO );
	}

	asc_delay (ap, 1);

	WRITE_BCR( ap->ddi.base_addr, BCR_ENABLE_INTR | BCR_ENABLE_DMA ); 
	if( pio_error ) {
		asc_cleanup( ap, cleanup_depth );
		io_det( ioaddr );
		return( EIO );
	}

	/*
	 *  Kick off a watchdog timer to ensure that the reset 
	 *  completes in reasonable time. If the timer trips,
	 *  the adapter is assumed dead. asc_process_locate() 
	 *  will w_stop() the watchdog timer.
	 */
	ap->wdog.dog.restart = RESET_DURATION;
	ap->wdog.reason = RESET_TMR;
	w_start( &ap->wdog.dog );

	/* 
	 *  Sleep until reset completes and 
	 *  interrupt handler services reset.
	 */
	asc_sleep( ap, (ulong)&ap->locate_event );

	/*  Verify that the reset completed properly. */
	if( ap->locate_state != RESET_COMPLETE ) {
		asc_cleanup( ap, cleanup_depth );
		io_det( ioaddr );
		return( EIO );
	}

	/* Initialize delivery pipes. */
	if(( rc = asc_init_pipes( ap, ioaddr )) != 0 ) {
		asc_cleanup( ap, cleanup_depth );
		io_det( ioaddr );
		return( EIO );
	}

	/* Adapter is now operating in move mode. */
	ap->adapter_mode = MOVE;

	/*
	 *  Setup the Basic Control Register( BCR ).
	 *  Enable chip interrupts, clear on read, and DMA.
	 *
	 *  BE VERY CERTAIN THAT THE DRIVER IS FULLY OPERATIONAL
	 *  (READY TO SERVICE INTERRUPTS, ETC.) AT THIS POINT!
	 */
        WRITE_BCR(ap->ddi.base_addr, BCR_ENABLE_INTR | BCR_ENABLE_DMA | 
				     BCR_CLR_ON_READ);
	if( pio_error ) {
		asc_cleanup( ap, cleanup_depth );
		io_det( ioaddr );
		return( EIO );
	}

	/* Detach IO handle */
	io_det( ioaddr );

	/*
	 *  Issue a Read Immediate request element to register
	 *  for asynchronous notification of SCSI bus resets.
	 */
	if(( rc = asc_rir( ap, ACTIVATE_RIR )) != 0 ) {
		asc_cleanup( ap, cleanup_depth );
		return( EIO );
	}

	return( 0 );
}
