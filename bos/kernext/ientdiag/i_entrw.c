static char sccsid[] = "@(#)56  1.22.1.3  src/bos/kernext/ientdiag/i_entrw.c, diagddient, bos411, 9428A410j 11/11/93 14:27:14";
/*
 * COMPONENT_NAME: (SYSXIENT) Ethernet Device Driver - Integrated Ethernet adapter
 *
 * FUNCTIONS:
 *             xxx_intr, ack_interrupt
 *	       ent_xmit_done, ent_recv
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * GLOBAL NOTES:
 *
 *      SERIALIZATION
 *
 *	The driver is responsible for serializing accesses to all of
 *	its structures
 *
 */

#include <sys/types.h>
#include <sys/vmker.h>
#include <sys/vminfo.h>
#include <sys/vmuser.h>
#include <sys/malloc.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dma.h>
#include <sys/adspace.h>
#include <sys/errno.h>
#include "ient_comio_errids.h"
#include <sys/except.h>
#include <sys/file.h>
#include <sys/intr.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/ioctl.h>
#include <sys/mbuf.h>
#include <sys/param.h>
#include <sys/pin.h>
#include <sys/poll.h>
#include <sys/sleep.h>
#include <sys/timer.h>
#include <sys/user.h>
#include <sys/watchdog.h>
#include <sys/xmem.h>
#include <sys/uio.h>
#include <sys/trchkid.h>
#include <sys/mstsave.h>
#include <sys/comio.h>
#include <sys/ciouser.h>
#include <sys/entuser.h> 
#include "i_entddi.h"
#include "i_cioddi.h"
#include "i_entdslo.h"
#include "i_cioddhi.h"
#include "i_entdshi.h"
#include "i_ciodds.h"
#include "i_cioddlo.h"
#include "i_ddctl.h"

int
ent_xmit_done(
	dds_t	*dds_ptr,
	ushort	stat_value,
	ulong	ioaddr );

/* activate ("open" and/or connect) the adapter */
extern int xxx_act (dds_t *dds_ptr);

/*
 *
 * NAME: xxx_restart    
 *                  
 * FUNCTION: restart the Ethernet adapter
 *                                                     
 * EXECUTION ENVIRONMENT:                             
 *                                                   
 *      This routine restarts the adapter once a fatal error occurs.
 *                                                  
 * NOTES:                                          
 *                                                
 *    Input: dds_ptr
 *                                                                
 *    Output: N/A                                                
 *                                                              
 *    Calls To: ack_interrupt
 *                                                            
 * RETURN:  INTR_SUCC - if it was our interrupt         
 *          INTR_FAIL - if it was not our interrupt
 *                                                
 */
int 
xxx_restart( dds_ptr )
dds_t *dds_ptr;
{
	ulong ioaddr;
	ulong rc, k;
	ushort cmd_value;
	int   saved_intr_level;
#ifdef DEBUG
         TRACE1 ("xxrS");
#endif
 	DISABLE_CL2_INTRS (saved_intr_level);
	/* Until the adapter is fully operational,
		place it in the disconnected state */
	CIO.device_state = DEVICE_NOT_CONN;
	WRK.adapter_state = NOT_STARTED;
	WRK.pos_request = FALSE;
	WRK.restart = TRUE;

	/* disable the Ethernet adapter */
        ATT_MEMORY();
        port( dds_ptr, ioaddr, 0x00 );
        DET_MEMORY();

	ENABLE_INTERRUPTS (saved_intr_level);
	/* release all of the resources */
	ent_recv_free( dds_ptr );
	ent_xmit_free( dds_ptr );

        ioaddr = io_att( (DDI.cc.bus_id | IOCC_SELECT) & NO_ADDR_CHECK_MASK,
                        POS_OFFSET );

	BUS_PUTC(ioaddr + POS_2, 0x00 );
	io_det( ioaddr ); 

	if ( !SALMON() ) {
		if (WRK.channel_allocated == TRUE) {
			rc = d_complete(WRK.dma_channel,DMA_NOHIDE, WRK.sysmem, WRK.alloc_size,
								(struct xmem *)&WRK.xbuf_xd, (char *)WRK.dma_base);
			if ( rc ) {
				ent_logerr(dds_ptr, ERRID_IENT_ERR3, rc, WRK.sysmem, WRK.dma_channel, WRK.dma_base);
			}
		}
	}

	/*  release allocated memory  */
	if ( WRK.sysmem != 0 ) {
          if ( xmfree( WRK.sysmem, pinned_heap ) != 0 )
        	ent_logerr(dds_ptr, ERRID_IENT_ERR4,0x162,rc,dds_ptr);
	   WRK.sysmem = 0;
	}
	if (WRK.channel_allocated == TRUE) {
		d_clear (WRK.dma_channel);
		WRK.channel_allocated = FALSE;
	}

	/* activate ("open" or "start" or "connect") the adapter */
   	DISABLE_CL2_INTRS (saved_intr_level);
	CIO.device_state = DEVICE_CONN_IN_PROG;
	WRK.restart = TRUE;

	ENABLE_INTERRUPTS (saved_intr_level);
	if (rc = xxx_act (dds_ptr)) {
		CIO.device_state = DEVICE_NOT_CONN;
	}
	else {
		WRK.adapter_state = STARTED;
		CIO.device_state = DEVICE_CONNECTED;
	}
	WRK.restart = FALSE;
#ifdef DEBUG
       TRACE1 ("xxrE");
#endif
}

/*
 *
 * NAME: xxx_intr    
 *                  
 * FUNCTION: Interrupt handler for the Ethernet device	
 *                                                     
 * EXECUTION ENVIRONMENT:                             
 *                                                   
 *      This routine runs on the interrupt level only
 *                                                  
 * NOTES:                                          
 *                                                
 *    Input: intr structure - this is also the beginning of the DDS
 *                                                                
 *    Output: N/A                                                
 *                                                              
 *    Calls To: ack_interrupt
 *                                                            
 * RETURN:  INTR_SUCC - if it was our interrupt         
 *          INTR_FAIL - if it was not our interrupt
 *                                                
 */
int 
xxx_intr( 
	struct	intr	 *ihsptr )
{
	ulong	status; 
	int	rc;
	ulong	save_status;
	ushort		stat_value;
	ulong		stat_reg;
	ulong	ioaddr;
	dds_t	*dds_ptr = (dds_t *)ihsptr;	/* cast ihsptr to dds ptr*/
	int saved_intr_level;
	
        ioaddr = (void *)io_att(DDI.cc.bus_id, 0);
	GET_STATUS_REG( &status );  		/* ethernet status reg */

#ifdef	DEBUG
	TRACE2("intS", status);
#endif	/* DEBUG */

	if ( ! SALMON() )
	{
		/*
		 *  make the interrupt bit consistent on all platforms
		 */
		/* Log micro-channel errors. */
		if (status & 0x2)
		  ent_logerr(dds_ptr, ERRID_IENT_ERR3, status, 0xdead, 0xcafe);
		if ( status & 0x7 ) { 
			status &= 0xFFFFFFFE;
		}
		else { 
			  status |= 0x1;
		}
	}
	else
	{
		if( status & MICRO_CHAN_ERR )
		{ /* Micro Channel error has occurred */
			/* CHANNEL CHECK, LEON AND CHAPS, PARITY, SELECT FEEDBACK */
			save_status = status;
		 	/*  make the interrupt bit consistent on all platforms */
			status &= 0xFFFFFFFE;
        		if (WRK.do_ca_on_intr & IS_A_RAINBOW) {
			  if (status & 0x10) {
			    /* Now acknowledge the intr.. */
 	  		    stat_reg = ioaddr + DDI.ds.io_port +
					 ENT_MEMBASED_STATUS_REG;
          		    if (BUS_PUTLX(stat_reg, 0x00000011)) 
            		      retry_put(stat_reg, 4, 0x11);   
			    i_reset (ihsptr);	/* reset interrupt */
			    return( INTR_SUCC );
			  }
			}
		}
	}
	
	if ( !(status & INTERRUPT)) {
		/*
		 *  the 82596 interrupted, check for errors
		 */
		if( SALMON() ) {
			if (status & MICRO_CHAN_ERR ) {
				io_det( ioaddr ); 
		/*
		* It appears that the only way that the software is able
		* to handle a LEON & CHAPS, or Channel Check is by resetting
		* the adapter. This is not a good workaround.
		*/
                                TRACE3 ("arst", status, save_status);
				xxx_restart( dds_ptr ); 
				ent_logerr(dds_ptr, ERRID_IENT_ERR3, save_status, 0xaaaa, WRK.dma_channel);
				i_reset (ihsptr);	/* reset interrupt */
				return( INTR_SUCC );
			}
		}

		/*
		 *  reset the interrupt at the device first,
		 *  then at the IO controller
		 */
                if (WRK.adapter_state != STARTING) {
		  ack_interrupt( dds_ptr, ioaddr, ihsptr ); /* ack interrupt */
                } else {
#ifdef DEBUG
                  TRACE1 ("intF");
#endif
               }
		rc = INTR_SUCC;
		i_reset (ihsptr); 			/* reset interrupt */

	} else {
		rc = INTR_FAIL;
	/* The following is an ugly kludge, I was forced by the hw-people to
	   do this, I will not put my name here. */
        if (WRK.do_ca_on_intr & IS_A_RAINBOW) {
      	  ulong   k;		/* Variables for the command_quiesce macro. */
          ushort  cmd_value, stat;

	  TRACE2("hack", WRK.do_ca_on_intr);
	  /* We are running on a Rainbow-3 system so we have to make sure
	     that we do a channel-attention (in order to get the mc-intr.
	     reset. Value IS_A_RAINBOW means Rainbow-3 and no CA done so far.*/

	  /* Do a no-op cmd. without getting an intr. */
  	  WRITE_LONG(WRK.acbl_ptr[0].csc, CB_EL | NOP_CMD);
 	  /*  put the bus address of the action CBL into the SCB */
          WRITE_LONG_REV(WRK.scb_ptr->cbl_addr, &WRK.acbl_addr[0]);

          /* ensure the CU is not in the acceptance phase */
          COMMAND_QUIESCE();

          START_CU();                     /* start the command unit */

	  /* Now acknowledge the intr. for the memory-based systems. */
 	  stat_reg = ioaddr + DDI.ds.io_port + ENT_MEMBASED_STATUS_REG;
          if (BUS_PUTLX(stat_reg, 0x00000001)) 
            retry_put(stat_reg, 4, 0x01);   

	  rc = INTR_SUCC;
	  i_reset (ihsptr); 			/* reset interrupt */
         } /* End of kludge. */
	}
#ifdef	DEBUG
	TRACE1("intE");
#endif	/* DEBUG */
	io_det( ioaddr );
	return(rc);
}


/*
 *
 * NAME: ack_interrupt 
 *                                                                     
 * FUNCTION:  Determines type of interrupt, calls the appropriate	
 *	      completion routines.
 *                                                                  
 * EXECUTION ENVIRONMENT:                                          
 *                                                                
 *      This routine runs on the interrupt level only		
 *                                                             
 * NOTES:                                                     
 *                                                           
 *    Input: dds_ptr, IO handle				
 *                                                     
 *    Output: N/A                                     
 *                                                   
 *    Called From: xxx_intr                         
 *                                                 
 *    Calls To:  ent_cmd_done, ent_recv		
 *                                             
 * RETURN:  0 							
 *                                                             
 */
ack_interrupt( 
	dds_t  *dds_ptr,
	ulong	ioaddr ,
	struct	intr	 *ihsptr )
	
{
	ushort		stat_value;
	ulong		stat_reg;
	ulong		i = 0;

	/*
	 *  the status field of the SCB will determine
	 *  the interrupt source ( CU or RU ), call the appropriate
	 *  processing routines.
	 */
        READ_SHORT(WRK.scb_ptr->status, &stat_value);
#ifdef	DEBUG
	TRACE2("ackS", stat_value);
#endif	/* DEBUG */
#ifdef  BASIC_DEBUG
        TRACE2("ackS", stat_value);
#endif  /* DEBUG */

	/* 
	 *  reset the interrupt at the device by
	 *  clearing out the interrupt bit in the status register
	 */
	if(!SALMON()) {
		uchar	value = 0;
		BUSPUTC( ENT_STATUS_STILWELL, 0x0 );
		BUSGETC( ENT_STATUS_STILWELL, &value );
		while (( value & 0x03 ) && (i < 10))
		{
		    BUSPUTC( ENT_STATUS_STILWELL, 0x0 );
		    BUSGETC( ENT_STATUS_STILWELL, &value );
		    i++;
		}
			
	} else {
          WRK.do_ca_on_intr &= IS_A_RAINBOW; /* Clear all but the Rainbow bit.*/
	}
        if(((stat_value & STAT_FR) && (stat_value & STAT_RUS_READY)) ||
            (stat_value & STAT_RNR)) 
        {
			ent_recv( dds_ptr, ioaddr );

	}  else {
          /* lets check for the condition where we got a interrupt */
          /* but it looks like nothing happened.  This could be    */
          /* the case where RNR is not indicated correctly.        */
          /* if the RU is ready, but nothing happened, call        */
          /* ent_recv anyways just in case.                        */
          if ((stat_value == STAT_RUS_READY) || 
              ((stat_value &  STAT_RUS_NO_RBD) ||
              (stat_value &  STAT_RUS_NO_RESOURCE) )) {
#ifdef DEBUG
            TRACE1 ("aRNm");
#endif
            ent_recv (dds_ptr, ioaddr); 
          }  
        }

	if((stat_value & STAT_CX ) || (stat_value & STAT_CNA))
	{
		/*
		 *  interrupt was a result of an action
		 *  command completing with its I bit set
		 *  or the CU becoming not ready
		 */
		if ( ! WRK.control_pending )
		{
			/*
			 *  determine type of command processing.
			 *  if no action command is pending, 
			 *  we have completed a transmit command
			 */
			if ( ! WRK.action_que_active )
			{
				ent_xmit_done( dds_ptr, stat_value, ioaddr );
			}
			else
			{
				ent_action_done( dds_ptr, stat_value, ioaddr );
			}
		}

	}
	else
	{
		if ( WRK.xmits_pending > 0 )
			ent_xmit_done( dds_ptr, stat_value, ioaddr );
	}

	/* The following is an ugly kludge, I was forced by the hw-people to
	   do this, I will not put my name here. */
        if (WRK.do_ca_on_intr == IS_A_RAINBOW) {
      	  ulong   k;		/* Variables for the command_quiesce macro. */
          ushort  cmd_value, stat;

	  TRACE2("hack", WRK.do_ca_on_intr);
	  /* We are running on a Rainbow-3 system so we have to make sure
	     that we do a channel-attention (in order to get the mc-intr.
	     reset. Value IS_A_RAINBOW means Rainbow-3 and no CA done so far.*/

	  /* Do a no-op cmd. without getting an intr. */
  	  WRITE_LONG(WRK.acbl_ptr[0].csc, CB_EL | NOP_CMD);
 	  /*  put the bus address of the action CBL into the SCB */
          WRITE_LONG_REV(WRK.scb_ptr->cbl_addr, &WRK.acbl_addr[0]);

          /* ensure the CU is not in the acceptance phase */
          COMMAND_QUIESCE();

          START_CU();                     /* start the command unit */
       } /* End of kludge. */

	/* Now acknowledge the intr. for the memory-based systems. */
	if( SALMON() ) {
 	  stat_reg = ioaddr + DDI.ds.io_port + ENT_MEMBASED_STATUS_REG;
          if (BUS_PUTLX(stat_reg, 0x00000001)) 
            retry_put(stat_reg, 4, 0x01);   
	}
#ifdef	DEBUG
	TRACE1("ackE");
#endif	/* DEBUG */
#ifdef	BASIC_DEBUG
	TRACE1("ackE");
#endif	/* DEBUG */
}

/*
 * 
 * NAME: ent_multicast         
 *                              
 * FUNCTION: 
 *                                                             
 * EXECUTION ENVIRONMENT:
 *      This routine runs on the interrupt level only		
 *                                                             
 * NOTES:                                                     
 *                                                           
 *    Input: Device Dependent Structure			
 *                                                     
 *    Output:
 *                                             
 *    Called From: ent_recv              
 *                                           
 *    Calls To: 
 *   
 *    Notes:
 *
 *                                         
 * RETURN:				
 *                            
 */
int ent_multicast( 
	dds_t	*dds_ptr,			/* DDS pointer */
	char 	*tmp_rfd_data,			/* attached I/O address */
	char 	*work_buffer)
{
  char     broad;
  unsigned index;
  uchar     *dest_addr;
  int      i;
  int      found = FALSE;

#ifdef DEBUG
  TRACE3 ("MC01", *dest_addr, dest_addr);
#endif
  /* Check first for a broadcast address */
  dest_addr = tmp_rfd_data;
  broad = TRUE;
  index = 0;
  /* Get the first bytes from the RFD. */
  while (broad && (index < ent_NADR_LENGTH) && 
		(index < NBR_DATA_BYTES_IN_RFD)) {
    if (*dest_addr != 0xff) {
      broad = FALSE;
    }
    dest_addr++;
    index++;
  }

  /* Now check the rest of the dest. addr. */
  /* This part we'll get from the buffer. */
  dest_addr = work_buffer;
  while (broad && (index < ent_NADR_LENGTH)) {
    if (*dest_addr != 0xff) {
      broad = FALSE;
    }
    dest_addr++;
    index++;
  }
  /* Then check the multicast list */
  if (!broad) {
    found = FALSE;
    for (i=0; i < MAX_MULTI && !found; i++) {
       if (WRK.multi_open[i] != NULL) {
         /* First check the RFD data bytes. */
    	 dest_addr = tmp_rfd_data;
         for (index = 0; (index < ent_NADR_LENGTH)
		&& (index < NBR_DATA_BYTES_IN_RFD); index++) {
            if (WRK.multi_list[i][index] != *dest_addr) {
              /* Not only break out of this loop but also
	       * make sure we do not execute the next for loop */
	      index = ent_NADR_LENGTH;
              break;
            } else {
                if (index == (ent_NADR_LENGTH -1)) {
                  found = TRUE;
                }
              }
            dest_addr++;
	 }
         dest_addr=work_buffer;
         for (; index < ent_NADR_LENGTH; index++) {
            if (WRK.multi_list[i][index] != *dest_addr) {
              break;
            } else {
                if (index == (ent_NADR_LENGTH -1)) {
                  found = TRUE;
                }
              }
#ifdef DEBUG
           TRACE2 ("Rmul", *dest_addr);
#endif
           dest_addr++;
         } /* of for */
       }
    }
  } else {
      found = TRUE;  /* To indicate that we found a broadcast. */
    }
  return(found);
}

/*
 * 
 * NAME: ent_find_netid         
 *                              
 * FUNCTION: 
 *                                                             
 * EXECUTION ENVIRONMENT:
 *      This routine runs on the interrupt level only		
 *                                                             
 * NOTES:                                                     
 *                                                           
 *    Input: Device Dependent Structure			
 *                                                     
 *    Output:
 *                                             
 *    Called From: ent_recv              
 *                                           
 *    Calls To: 
 *   
 *    Notes:
 *
 *                                         
 * RETURN: Matching netid index 0..n    , or
 *          -1 if not found
 */
int ent_find_netid( 
	dds_t	*dds_ptr,			/* DDS pointer */
	char 	*work_buffer) 			/* Data pointer */
{
  int     j = -1;				/* Index to return */
  int     netid_length;
  netid_t netid;

  netid = GET_NETID(((char *)work_buffer),DDI.ds.type_field_off );
  /* use the netid field to determine type of packet */
  if (netid < 0x0600) { 
    /* 
     *  get the 802.3 netid from the packet
     *  IEEE 802.3 packet - netid length is 1 byte & 
     *  offset is 14 
     */
    netid_length = 0x1; 
    netid = GET_NETID(((char *) work_buffer), DDI.ds.net_id_offset);
    netid = ((netid >> 8) & 0x00FF);
  }
  else { 
   /* 
    *  standard ethernet packet 
    *  netid length is 2 bytes & offset is 12 
    */
   netid_length = 0x2; 	/* set length of netid */
  }

  /* 
   *  if this packet belongs to the same caller as the 
   *  previous packet, no need to seach thru netid table,
   *  take the fast path.
   */
  if ((netid == WRK.prev_netid) && (netid_length == WRK.prev_netid_length)) {
    j = WRK.prev_netid_indice;
  }
  else {
    /* 
     *  match the netid to an entry in the netid table
     *  to find owner of packet
     */
   for (j = 0; j < CIO.num_netids; j++) {
      if ((netid == CIO.netid_table_ptr[j].netid) &&
	    (netid_length == CIO.netid_table_ptr[j].length)) { 
	/* 
	 *  found a home - remember indice for fastpath
	 *  try next time
	 */
	WRK.prev_netid        = netid;
	WRK.prev_netid_length = netid_length;
	WRK.prev_netid_indice = j;
	break;
      }
    } /* for loop */
    if (j >= CIO.num_netids)
      j = -1;  /* Not found */
  }
  return(j);
}

/*
 * 
 * NAME: ent_recv         
 *                              
 * FUNCTION: Completion routine for received Ethernet packets		    
 *                                                             
 * EXECUTION ENVIRONMENT:
 *      This routine runs on the interrupt level only		
 *                                                             
 * NOTES:                                                     
 *                                                           
 *    Input: Device Dependent Structure			
 *                                                     
 *    Output: packet passed to caller		
 *                                             
 *    Called From: ack_interrupt              
 *                                           
 *    Calls To: d_complete, vm_cflush, cio_proc_recv
 *   
 *    Notes:
 *		The d_complete must occur AFTER the check of
 *		the Complete bit. If the order were reversed,
 *		the C bit could be set after the d_complete
 *		finished but before we looked at it, leaving
 *		a small portion of valid data in the IOCC buffers.
 *
 *		Never ever access in any way the receive buffers
 *		after they have been invalidated by vm_cflush.
 *		This will corrupt the cache and return bogus
 *		data to the ULCS.
 *
 *                                         
 * RETURN:				
 *                            
 */
int
ent_recv( 
	dds_t	*dds_ptr,			/* DDS pointer */
	ulong	ioaddr )			/* attached I/O address */
{
	open_elem_t	*open_elem; 		/* open element */
	rec_elem_t      recv_elem;      	/* receive element  */
        struct	mbuf    *mbufp;         	/* mbuf pointer */
	ushort		count;			/* # of bytes received */
	long		i, k, rc;
	int		j, rfd_nbr;		/* Must be a signed type. */
	int		index;
        ulong		fbl, start_fbl;
	ulong		csc_value, work, work1;
	ushort		cnt_value, stat_value, cmd_value, eof_flag;
        struct rbd	*work_rbd, *next_rbd, *this_rbd;  /* work vars */
        struct rbd      *end_rbd_list;
        char            *work_buffer, *dest_addr;
        ushort          num_good_frames = 0;
	uchar		tmp_rfd_data[NBR_DATA_BYTES_IN_RFD];

#ifdef DEBUG
        TRACE2("rcvS", dds_ptr);
#endif

#ifdef	DEBUG2
  ulong          work_var;
  ulong          work2, work3, work4, work5;
  ushort         wshort1, wshort2;
  READ_LONG_REV(WRK.scb_ptr->rfa_addr, &work_var);
  TRACE2("rcvS", dds_ptr);
  TRACE4("rcv1", WRK.begin_fbl, WRK.end_fbl, WRK.el_ptr);
  TRACE4("rcv2", work_var, &WRK.rfd_addr[WRK.begin_fbl], 
	 &WRK.rfd_ptr[WRK.begin_fbl]);
  READ_LONG_REV (WRK.rfd_ptr[WRK.begin_fbl].rbd, &work_var);
  TRACE4 ("rcv3", &WRK.rbd_ptr[WRK.begin_fbl], work_var, WRK.rbd_el_ptr);
  for (wshort1=0; wshort1 < WRK.recv_buffers; wshort1++) {
    READ_LONG (WRK.rfd_ptr[wshort1].csc, &work1);
    READ_SHORT (WRK.rfd_ptr[wshort1].count, &wshort2);
    READ_LONG_REV (WRK.rfd_ptr[wshort1].rbd, &work4);
    TRACE5 ("RFD1", wshort1, work1, work4, wshort2);
    READ_LONG_REV (WRK.rbd_ptr[wshort1].size, &work1);
    READ_SHORT (WRK.rbd_ptr[wshort1].count, &wshort2);
    TRACE3 ("RBD1", work1, wshort2);
  }
#endif	/* DEBUG */
	RAS.ds.recv_intr_cnt++;
 
        /* start_fbl will be the new head of list if we get an RNR */
        start_fbl=WRK.begin_fbl;

        /* make sure end_rbd_list has an initial value */
        end_rbd_list = WRK.rbd_el_ptr;

	READ_LONG(WRK.rfd_ptr[WRK.begin_fbl].csc, &csc_value);
	for ( rfd_nbr = WRK.recv_buffers ; rfd_nbr > 0; rfd_nbr-- ) {
#ifdef BASIC_DEBUG
 TRACE5("rcv4", rfd_nbr, csc_value, WRK.begin_fbl, &WRK.rfd_ptr[WRK.begin_fbl]);
#endif
#ifdef DEBUG
 TRACE5("rcv4", rfd_nbr, csc_value, WRK.begin_fbl, &WRK.rfd_ptr[WRK.begin_fbl]);
#endif
                /* csc value is set before the start of the for */
                /* loop, or at the bottom. The actions at the   */
                /* bottom and here must be the same             */
		if (( !( csc_value & CB_C )) && (csc_value != CB_SF)) {
			/* 
			 *  no received frames to be processed
			 */	
                         /* if no frames were processed during this    */
                         /* interrupt we want to just get out unless   */
                         /* we are in a RNR condition.  This is one    */
                         /* of those false RNR's because we moved the  */
                         /* el bit while the RFD or RBD was being used */
 			 if ( rfd_nbr == WRK.recv_buffers ) {
                           READ_LONG (WRK.scb_ptr->status, &stat_value);
                           if ((! (stat_value & STAT_RNR)) &&
                               (!(stat_value & STAT_RUS_NO_RSC_RBD))){
                             return;
                           }
                         }
#ifdef DEBUG
TRACE2 ("BRK2", stat_value);
#endif
			break; 
		}


		if ( !SALMON() ) {
			rc = d_complete(WRK.dma_channel, DMA_NOHIDE, 
				WRK.sysmem, WRK.alloc_size,
				(struct xmem *)&WRK.xbuf_xd,
					(char *)WRK.dma_base);

			if ( rc )
				ent_logerr(dds_ptr, ERRID_IENT_ERR3,rc,
				     WRK.sysmem, WRK.dma_channel, WRK.dma_base);
		}

       		if ( ! (csc_value & CB_OK )) {
			/*  
                         /* We always thought, and the intel book says
			 *  this case is only possible if configured 
		 	 *  to SAVE BAD FRAMES. But it is possible to get
                         *  these BAD Frames at any time so we need to handle
                         *  the situation. 
			 */
#ifdef DEBUG
READ_LONG (WRK.scb_ptr->status, &stat_value);
TRACE3 ("rbad", csc_value, stat_value);
#endif
			  goto manage_q;  
		}
		else {
                  num_good_frames++;
		}
		
                /* if the RU is not ready and we may have enough resources    */
                /* lets re-start the RU                                       */
                READ_LONG (WRK.scb_ptr->status, &stat_value);
                if ((stat_value & STAT_RNR ) && (rfd_nbr < (WRK.recv_buffers - 2))) {
#ifdef DEBUG
   TRACE5("RNR1", stat_value, WRK.begin_fbl, WRK.end_fbl, end_rbd_list);
#endif
                  /* now clear out old EL at former RFD and RBD end of lists */
                  /* preserving the csc and size information                 */
                  /* WRK.el_ptr = former RFD End of List index               */
                  /* WRK.rbd_el_ptr = former RBD End of List address         */

                  READ_LONG (WRK.rfd_ptr[WRK.el_ptr].csc, &work1);
                  work1=work1 & ((~ (CB_EL | CB_SUS)));
                  WRITE_LONG(WRK.rfd_ptr[WRK.el_ptr].csc, work1);

                  READ_LONG_REV (WRK.rbd_el_ptr->size, &work1);
                  work1=work1 & (~(0x00008000));
                  WRITE_LONG_REV(WRK.rbd_el_ptr->size, work1);

                  /* set EL bit at end of RFD and RBD lists     */
                  /* WRK.end_fbl = index to last RFD in list    */
                  /* WRK.end.rbd.list = addr of last free RBD   */

                  WRITE_LONG(WRK.rfd_ptr[WRK.end_fbl].csc,CB_EL | CB_SUS | CB_SF);
                  WRITE_LONG_REV(end_rbd_list->size,
                                 sizeof WRK.recv_buf_ptr[fbl].buf | 0x00008000);
       
                  /* set up where end of RFD and RBD lists are at           */
                  WRK.el_ptr=WRK.end_fbl;
                  WRK.rbd_el_ptr=end_rbd_list;
#ifdef DEBUG
    TRACE3 ("Rbrk", WRK.el_ptr, WRK.rbd_el_ptr);
#endif
                  break;
              }

              /* now we need to set up our work pointers      */
              READ_LONG_REV(WRK.rfd_ptr[WRK.begin_fbl].rbd, &work_rbd);

              /* make sure that the RBD address is not all ones */
              /* if it is go clean up the RFD                   */
              if (work_rbd == 0xffffffff) {
#ifdef DEBUG
TRACE1 ("rfff");
#endif
                goto manage_q;
              }

                /* convert the rbd ptr to an address */
                if (SALMON ()) {
                  work_rbd=IOADDRTOSM (work_rbd);
                } else {
                  work_rbd=(long)work_rbd + (long)((long)&WRK.rbd_ptr[0] &
				 (long) (~ARAM_MASK));
                }

                /* Get the buffer address for later on  */
                READ_LONG_REV (work_rbd->rb_addr, &work_buffer);
                work_buffer=IOADDRTOSM (work_buffer);
#ifdef DEBUG
TRACE3 ("rcvx", work_rbd, end_rbd_list);
#endif
		READ_SHORT(work_rbd->count, &cnt_value);
		count = reverse_short( cnt_value ) & 0x3FFF;
		/* Add the data bytes from the RFD. */
		count += NBR_DATA_BYTES_IN_RFD;

		/*  prepare to pass the packet up to the ULCS
		 *  first some sanity checking */
		if ((count < ent_MIN_PACKET) || (count > ent_MAX_PACKET)) { 
#ifdef DEBUG
TRACE2 ("errl", count);
#endif
		  /* invalid packet size, log error */
           	  ent_logerr(dds_ptr,ERRID_IENT_ERR4,0x683,count,WRK.begin_fbl);

		  if ( count > ent_MAX_PACKET ) {
			RAS.ds.too_long++;
		  }
		  goto manage_q;
		}

		/* Now read the first part of the data,
		   i.e. the data from the RFD.  */

                if (SALMON ()) {
		  dest_addr = WRK.rfd_ptr[WRK.begin_fbl].d_addr;
		  for (i = 0; i < NBR_DATA_BYTES_IN_RFD; i++, dest_addr++) {
			READ_CHAR(dest_addr, &tmp_rfd_data[i]);
		  }
		}
		else
		  READ_SHORT(WRK.rfd_ptr[WRK.begin_fbl].d_addr, tmp_rfd_data);

                /* Since the filtering for the 596 for multicast
                  * addresses is unreliable we need to do some checking here.
                  * If this is not a multicast address that we have set up,
                  * or a broadcast, .. throw away the data */
                if (*tmp_rfd_data & MULTI_BIT_MASK == MULTI_BIT_MASK) {
                  if (!ent_multicast(dds_ptr, tmp_rfd_data, work_buffer)) {
#ifdef DEBUG
TRACE2 ("nois", count);
#endif
                        goto manage_q;     /* Throw away. */
		  }


		}
                /* find the net ID to determine who gets the packet. */
                if ((j = ent_find_netid(dds_ptr, work_buffer)) >= 0) {
#ifdef DEBUG
TRACE2 ("neti", j);
#endif
			open_elem = CIO.open_ptr[CIO.netid_table_ptr[j].chan-1];
			/* found a home - start processing the packet
			 *  get a mbuf for the received packet
			 */
			if (count <= MHLEN - WRK.rdto) {
                        	mbufp = m_gethdr(M_DONTWAIT, MT_DATA);
                        }
                        else {
                                mbufp = m_getclust(M_DONTWAIT,MT_DATA);
                        }
			if (mbufp != NULL) {
			  mbufp->m_data += WRK.rdto;
			  mbufp->m_len = count;
#ifdef DEBUG
TRACE2 ("byte", count);
#endif
			  /* move the data into the mbuf */
			  bcopy((char *) tmp_rfd_data, MTOD(mbufp,char *),
					 NBR_DATA_BYTES_IN_RFD );
			  bcopy((char *) work_buffer, MTOD(mbufp,char *)
				 + NBR_DATA_BYTES_IN_RFD,
				 (count - NBR_DATA_BYTES_IN_RFD));

			  if (! SALMON())
			   vm_cflush(work_buffer,count - NBR_DATA_BYTES_IN_RFD);

			/* build a receive element */
			recv_elem.mbufp         = mbufp;
			recv_elem.bytes         = count;
			recv_elem.rd_ext.status = CIO_OK;

			/* passing open element to CIO code
			   identifies owner of packet  */
			if (open_elem != NULL) { 
				/* valid open element, pass 
				   packet to ULCS */
				cio_proc_recv(dds_ptr, open_elem, &recv_elem);
			}
			else {
			  m_free(mbufp);
			}
		      }
		      else {
#ifdef DEBUG
TRACE1 ("Rnmb");
#endif
			RAS.ds.rec_no_mbuf++;
			ent_logerr(dds_ptr,ERRID_IENT_ERR4,0x816,count,
					WRK.rdto);
			return (ENOMEM);
		      }
		}
manage_q:

#ifdef DEBUG2
  for (wshort1=0; wshort1 < WRK.recv_buffers; wshort1++) {
    READ_LONG (WRK.rfd_ptr[wshort1].csc, &work1);
    READ_SHORT (WRK.rfd_ptr[wshort1].count, &wshort2);
    READ_LONG_REV (WRK.rfd_ptr[wshort1].rbd, &work4);
    TRACE5 ("RFD1", wshort1, work1, work4, wshort2);
    READ_LONG_REV (WRK.rbd_ptr[wshort1].size, &work1);
    READ_SHORT (WRK.rbd_ptr[wshort1].count, &wshort2);
    TRACE3 ("RBD1", work1, wshort2);
  }
#endif

              /*  put the freed resources onto the free list         */
              /* first follow the RBD chain and clean out till EOF   */
              /* only follow the list if we had an  RBD address      */
              /* that is not all ones                                */
              /* need to set up work_rbd in case it was not set above*/
                
              READ_LONG_REV (WRK.rfd_ptr[WRK.begin_fbl].rbd, &work_rbd);
              if (work_rbd != 0xffffffff)
              {
                if (SALMON ()) {
                  work_rbd=IOADDRTOSM (work_rbd);
                } else {
                  work_rbd=(long)work_rbd + (long)((long)&WRK.rbd_ptr[0] & (long) (~ARAM_MASK));
                }

                /* It is possible to get an RFD that says C & OK, and is
		 * pointing to a valid RBD, but the RBD will have an actual
                 * count of ZERO, and No EOF bit.  The status looks good 
                 * in the RFD.  In this case we cannot go thru the while 
		 * loop because with the EOF bit missing we will "while" 
		 * too long, or even forever. We will assume that just 
		 * one RBD was used in this case. If the count and status
		 * bits look OK loop until the EOF is found           
	 	 */

                next_rbd=work_rbd;
                do {
                  work_rbd=next_rbd;
                  READ_SHORT (work_rbd->count, &eof_flag);
                  /* restore the size */
                  WRITE_LONG_REV (work_rbd->size, sizeof (WRK.recv_buf_ptr[0].buf));
                  /* clean count and EOF flag */
                  WRITE_SHORT (work_rbd->count, 0);
                  /* here is the special case  */
                  if ((reverse_short (eof_flag) & 0x3fff) == 0)  {
#ifdef DEBUG
                    TRACE3 ("rstb", eof_flag, csc_value);
#endif
                    break;
                  }
                  /* get the next RBD in chain */
                  READ_LONG_REV (work_rbd->next_rbd, &next_rbd);
                  if (SALMON ()) {
                    next_rbd = IOADDRTOSM (next_rbd);
                  } else {
                    next_rbd = (long) next_rbd + (long) ((long)&WRK.rbd_ptr[0] & (long) (~ARAM_MASK));
                  }
                } while (! (eof_flag & RBD_EOF));
              } else {
                /* set work_rbd to where the end of the free RBD list is at for the code below */
                work_rbd = end_rbd_list;
              }

		fbl = bump_que( WRK.begin_fbl, ADP_RCV_QSIZE );

                /* WARNING:  Do not modify csc_value after this, it must be  */
                /* preserved for testing at the top of the for loop          */
                /* so that this code and the "break" up there stay in sync   */

		READ_LONG(WRK.rfd_ptr[fbl].csc, &csc_value);
		if (( csc_value & CB_C ) && ( rfd_nbr > 1 ))
		{
                        /* if next RFD has data and not the last one to look at this time */
       			WRITE_LONG(WRK.rfd_ptr[WRK.begin_fbl].csc, CB_SF);

		}
		else
		{
                        /* only reset the EL if the are not going to end up in the same place */
                        /* if we allowed that then it could get erased                        */
                        if (WRK.begin_fbl != WRK.el_ptr) {
       			  WRITE_LONG(WRK.rfd_ptr[WRK.begin_fbl].csc, CB_EL | CB_SUS | CB_SF);
                          /* clear EL and SUS bits, but keep other info in case it has been used */
                          READ_LONG (WRK.rfd_ptr[WRK.el_ptr].csc, &work1);
                          work1=work1 & ((~ (CB_EL | CB_SUS)));
       			  WRITE_LONG(WRK.rfd_ptr[WRK.el_ptr].csc, work1);
			  WRK.el_ptr = WRK.begin_fbl;
                        }
                        /* If the el bit is going to be put into the */
                        /* same RBD, do NOT do it, otherwise we will */
                        /* end up with no RBD having the el bit      */
                        if (work_rbd != WRK.rbd_el_ptr) {
                          READ_SHORT (WRK.rbd_el_ptr->count, &count);
                          /* only move rbd el if it is not being used  */
                          if (! (count & RBD_F)) {
			    WRITE_LONG_REV(work_rbd->size, sizeof WRK.recv_buf_ptr[fbl].buf | 0x00008000);
                            READ_LONG_REV (WRK.rbd_el_ptr->size, &work1);
                            work1=work1 & (~ (0x00008000));
        		    WRITE_LONG_REV(WRK.rbd_el_ptr->size, work1);
                            WRK.rbd_el_ptr = work_rbd;
                          }
                        }
		}
		WRITE_SHORT(WRK.rfd_ptr[WRK.begin_fbl].count, 0);
                WRITE_LONG_REV(WRK.rfd_ptr[WRK.begin_fbl].rbd, 0xffffffff);

		WRK.end_fbl = WRK.begin_fbl;
		WRK.begin_fbl = bump_que( WRK.begin_fbl, ADP_RCV_QSIZE );
                end_rbd_list = work_rbd;

                READ_SHORT (WRK.scb_ptr->status, &stat_value);
#ifdef DEBUG
                READ_LONG_REV (WRK.scb_ptr->resource_errs, &work);
                TRACE3 ("rcv5", stat_value, work);
                TRACE4 ("rcv6", end_rbd_list, WRK.el_ptr, WRK.rbd_el_ptr);
#endif
	}


        READ_SHORT(WRK.scb_ptr->status, &stat_value);

	/*
	 *  start the RU if necessary
	 */
	if  ( ! (stat_value & STAT_RUS_READY) )
	{
                /* we need to start the Receive Unit, but we need to find */
                /* out if we are in the RNR state becuase we really ran   */
                /* out of resources or if the RU just thinks we are       */
                /* because we moved the EL pointer after the RFD/RBD was  */
                /* pre-fetched.                                           */
                /* We will start looking for the start of the RBD chain   */
                /* from the rbd_el_ptr, until we find the first free RBD. */
                /* This will be the start of the free RBD's.              */

                
                READ_LONG_REV (WRK.rbd_el_ptr->next_rbd, &this_rbd);
                for (k=0; k < WRK.recv_buffers; k++) {
                  if (SALMON ()) {
                    this_rbd=IOADDRTOSM (this_rbd);
                  } else {
                    this_rbd=(long) this_rbd + (long) ((long)&WRK.rbd_ptr[0] & (long) (~ARAM_MASK));
                  }
                  /* if this RBD is not used it is start of free RBD list */
                  READ_SHORT (this_rbd->count, &cnt_value);
                  if (! (cnt_value & RBD_F)) {
                    break;
                  }
                  /* the RBD is used */
                  READ_LONG_REV (this_rbd->next_rbd, &this_rbd);
               }
            
               /* now we need to find the start of the RFD list        */
               /* we start looking at begin_fbl, the first one         */
               /* without a complete frame is the start.               */
               /* We will also reset the RFD in case it was marked     */
               /* used but never got a complete frame.                 */

               start_fbl=WRK.begin_fbl;
               do {
                 READ_LONG (WRK.rfd_ptr[start_fbl].csc, &csc_value);
                 if (! (csc_value & CB_C)) {
                   break;
                 }
                 start_fbl = bump_que (start_fbl, ADP_RCV_QSIZE);
              } while (start_fbl != WRK.begin_fbl);

              
#ifdef DEBUG
               TRACE4 ("Rnr3", k, this_rbd, start_fbl);
#endif
               /* if there are free RBD fix the start of RBD list */
               if (k != WRK.recv_buffers) {
                 if (SALMON ()) {
                   work_rbd=SMTOIOADDR (this_rbd);
                 } else {
                   work_rbd=(long)this_rbd & ARAM_MASK;
                 }
               
                 /* set the RBD pointer in the RFD indicated by start_rfd */
                 WRITE_LONG_REV (WRK.rfd_ptr[start_fbl].rbd, work_rbd);
                 WRITE_LONG (WRK.rfd_ptr[start_fbl].csc, CB_SF);
        	 WRITE_LONG_REV(WRK.scb_ptr->rfa_addr, &WRK.rfd_addr[start_fbl] );
#ifdef DEBUG
                TRACE2 ("Rnr2", start_fbl);
#endif
                 /* go ahead and ack the out of resource condition */
	         COMMAND_QUIESCE();
		 WRITE_SHORT(WRK.scb_ptr->command, CMD_ACK_RNR | CMD_ACK_FR);
	         CHANNEL_ATTENTION();
		 start_ru( dds_ptr, ioaddr );
               }
	} else {
  	  /*
	   *  set the proper acknowledge bits and issue 
	   *  a CA. 
	   */
          if (num_good_frames > 0) {
	    COMMAND_QUIESCE();
	    WRITE_SHORT(WRK.scb_ptr->command, CMD_ACK_FR );
	    CHANNEL_ATTENTION();
	    COMMAND_QUIESCE();
         } else {
#ifdef BASIC_DEBUG
           TRACE1 ("rcvF");
#endif
         }
       }

#ifdef DEBUG2
  for (wshort1=0; wshort1 < WRK.recv_buffers; wshort1++) {
    READ_LONG (WRK.rfd_ptr[wshort1].csc, &work1);
    READ_SHORT (WRK.rfd_ptr[wshort1].count, &wshort2);
    READ_LONG_REV (WRK.rfd_ptr[wshort1].rbd, &work4);
    TRACE5 ("RFD1", wshort1, work1, work4, wshort2);
    READ_LONG_REV (WRK.rbd_ptr[wshort1].size, &work1);
    READ_SHORT (WRK.rbd_ptr[wshort1].count, &wshort2);
    TRACE3 ("RBD1", work1, wshort2);
  }
#endif	/* DEBUG */
#ifdef DEBUG
        READ_SHORT(WRK.scb_ptr->status, &stat_value);
        TRACE2 ("rcv9", stat_value);
#endif
		TRACE1("rcvE");
}

/*
 * 
 * NAME: ent_xmit_done                                      
 *                                                         
 * FUNCTION:  Transmit complete processing. Free resources and acknowledge
 *	      transmit done interrupt. Moves transmit elements  
 *	      from the waitq to the transmit ready queue if necessary.		
 *                                                             
 * EXECUTION ENVIRONMENT:                                     
 *      This routine runs on the interrupt level only	
 *                                                     
 * NOTES:                                             
 *                                                   
 *    Input: Device Dependent Structure		
 *                                             
 *    Output: N/A                             
 *                                           
 *    Called From: ack_interrupt
 *                                         
 *    Calls To: cio_xmit_done, m_copydata
 *
 *    Notes:
 *		It appears that some CA's associated
 *		with transmits do not get latched if
 *		the 596 is busy with a higher priority
 *		task. It just gets lost, yet the SCB command
 *		field gets cleared. This is why this routine
 *		checks the status of the XMIT CBL. If no
 *		activity has taken place ( ie. busy bit not set )
 *		we issue another CA.
 *
 *		The condition where the busy bit is set and
 *		the CU is suspended is 596 errata and invalid.
 *		The busy bit indicates the CU is busy processing
 *		this CBL yet the CU is suspended. Restart the CU.
 *
 *		Move queued transmit elements to the ready queue
 *		before calling a kernel process's ( ie. TCP )xmit_done 
 *		routine. If this order is reversed, the kernel process
 *		would call our write ( before we could put que'd xmits on 
 *		the ready queue ) routine which would put the write packet
 *		directly on the readyq ( resources are sure to be 
 *		free at this point ). This causes packets to be transmitted
 *		out of order which may not be fatal but is undesirable.
 *		
 */
ent_xmit_done(
	dds_t	*dds_ptr,		 /* DDS pointer */
	ushort	stat_value,		 /* SCB status field  */
	ulong	ioaddr )		 /* IO handle */
{
	ulong	status;	
	ulong	csc_value;
	ulong	value;
	ulong	bytes;
	ulong	i, j, k;
	ushort	cmd_value;
	ushort	stat;
	int	rc;
	int	timeout;

#ifdef	DEBUG
	TRACE2("xmtS", stat_value);
#endif	/* DEBUG */
#ifdef	XMIT_DEBUG
	TRACE2("xmtS", stat_value);
#endif	/* DEBUG */
	RAS.ds.xmit_intr_cnt++;
	
	w_stop(&(WDT));
	WRK.wdt_setter = WDT_INACTIVE;
	READ_LONG(WRK.xcbl_ptr[ WRK.readyq_out ].xmit.csc, &csc_value);
	status = csc_value & STAT_FIELD_MASK;
	if (stat_value & STAT_CUS_TIMEOUT) {
#ifdef	DEBUG
		TRACE2("xmt1", status);
#endif	/* DEBUG */
		COMMAND_QUIESCE();
		ABORT_CU();
		timeout = 1;
	} else {
		timeout = 0;
		if ( status & CB_B ) {
			/*	
			 *  if CBL's busy bit is set and CU is suspended,
			 *  596 errata experienced. resume CU
			 */
			if ( stat_value & STAT_CUS_SUSPEND ) {
				COMMAND_QUIESCE ();
				WRITE_SHORT(WRK.scb_ptr->command, (ushort)CMD_CUC_RES);
				CHANNEL_ATTENTION();
#ifdef DEBUG
TRACE1 ("xmSU");
#endif
			}
			if (stat_value & STAT_CUS_ACTIVE) {
#ifdef DEBUG
TRACE1 ("xmt8");
#endif
				return (0);
			}
		}
	}

	WRK.xmits_pending = FALSE;	
	for ( i = XMIT_BUFFERS; i > 0; i-- ) {
		READ_LONG(WRK.xcbl_ptr[WRK.readyq_out].xmit.csc, &status);
		if (timeout) {
#ifdef DEBUG
			TRACE3 ("xmt3", i, status);
#endif
			timeout = 0;		/* only timeout first xmit */
			status |= XMIT_STAT_NCS | XMIT_STAT_TD;
			status |= CB_C;		/* become complete */
			status &= ~(CB_B);	/* not busy */
		}
#ifdef DEBUG
                TRACE3 ("xmt4", i, status);
#endif

		if ( status ==  ( CB_EL | CB_SUS | CB_INT | CB_SF | XMIT_CMD)) {
			COMMAND_QUIESCE();
			WRITE_LONG_REV(WRK.scb_ptr->cbl_addr, 
					&WRK.xcbl_addr[WRK.readyq_out]);
			START_CU();
#ifdef DEBUG
TRACE2 ("xmt5", WRK.readyq_out);
#endif
			WRK.xmits_pending = TRUE;
			break;
		}

		if ( !( status & CB_C )) {
			break;		
		}
		if( !SALMON() ) {
			rc = d_complete(WRK.dma_channel,DMA_NOHIDE,  WRK.xmit_buf_ptr[WRK.buffer_in].buf, PACKET_SIZE, (struct xmem *)&WRK.xbuf_xd, (char *)WRK.dma_base);
			if ( rc )
				ent_logerr(dds_ptr, ERRID_IENT_ERR3, 0x1149, rc, WRK.dma_channel, WRK.dma_base);
		}

		if ( status & CB_OK ) {
			rc = CIO_OK;		/* no errors detected */
		}
		else {
			ulong nbr_collisions;
			/*
			 *  check for transmit errors in the command block
			 */
			rc = CIO_OK;

			if ( status & XMIT_STAT_LC ) {
				RAS.ds.late_collision++;   
				rc = (ulong)ENT_TX_LATECOLL;
			}

			if ( status & XMIT_STAT_NCS ) {
				rc = (ulong)ENT_TX_CD_LOST; 
				RAS.ds.carrier_lost++;   
			}

			if ( status & XMIT_STAT_CSS ) {
				rc = (ulong)ENT_TX_CTS_LOST;
				RAS.ds.cts_lost++;   
			}

			if ( status & XMIT_STAT_DU ) {
				rc = (ulong)ENT_TX_UNDERRUN;
				RAS.ds.underrun++;
			}

			if ( status & XMIT_STAT_TD ) {
				rc = (ulong)ENT_TX_TIMEOUT;
				RAS.ds.xmit_timeouts++;
			}

			if (status & XMIT_STAT_MAXCOLL) {
			  rc = (ulong)ENT_TX_MAX_COLLNS;
			  nbr_collisions = (status & XMIT_STAT_MAXCOLL) >> 6;
			  RAS.ds.max_collision += nbr_collisions;
			}
			else {
			  if (status & XMIT_STAT_STOP) {
			    RAS.ds.max_collision += 16;
			    rc = (ulong)ENT_TX_MAX_COLLNS;
			  }
			}

		}

		/* free the transmit element and transmit buffers */
		WRK.xmit_buffers_used--;
                WRITE_LONG(WRK.xcbl_ptr[WRK.readyq_out].xmit.csc, CB_EL );

		/*
		 *  if any packet are on the wait queue and resources 
		 *  are available, move them to the ready queue
		 */
		if( !XMIT_BUFFERS_FULL  && ( WRK.xmits_queued > 0 )) {
#ifdef DEBUG
TRACE3 ("xmtX", WRK.waitq_out, WRK.xmits_queued);
#endif
			/*
			 *  copy data from the mbufs into the transmit buffers
			 *  all queued transmits are handled the same
			 *  regardless of caller
			 */

			m_copydata( WRK.xmit_elem[WRK.waitq_out].mbufp, 0,
				    WRK.xmit_elem[WRK.waitq_out].bytes, 
				    &WRK.xmit_buf_ptr[WRK.buffer_in].buf );

			if ( !SALMON() )
				vm_cflush( &WRK.xmit_buf_ptr[WRK.buffer_in].buf, PAGESIZE / 2 );

			/*  free the mbuf if necessary */
			if ( !(WRK.xmit_elem[WRK.waitq_out].wr_ext.flag & 
								CIO_NOFREE_MBUF )) {
			      m_freem(WRK.xmit_elem[WRK.waitq_out].mbufp);
			      WRK.xmit_elem[WRK.waitq_out].mbufp = NULL;
			}

			bytes = WRK.xmit_elem[WRK.waitq_out].bytes; 
			bytes = ((bytes & 0x00FF) << 8 ) | ((bytes & 0xFF00) >> 8);

			/* fill out the TBD and the XCBL, put the bus 
			   address of *  the transmit CBL into the SCB */
			WRITE_LONG_REV(WRK.xcbl_ptr[WRK.waitq_out].xmit.tbd,
						&WRK.tbd_addr[WRK.buffer_in]);
			WRITE_LONG(WRK.tbd_ptr[WRK.buffer_in].control, bytes << 16 |									 XMIT_EOF);
			WRITE_LONG(WRK.xcbl_ptr[WRK.waitq_out].xmit.csc,
					CB_EL | CB_SUS | CB_INT | CB_SF | XMIT_CMD);
			/* update the queue pointers */
			WRK.xmit_buffers_used++;
			WRK.buffer_in = bump_que( WRK.buffer_in, 
							WRK.xmit_buffers_allocd );
			WRK.waitq_out = bump_que(WRK.waitq_out, DDI.cc.xmt_que_size );
			WRK.xmits_queued--;

			if ( WRK.waitq_out == WRK.waitq_in ) {
				WRK.xmits_queued = 0;	
				WRK.readyq_in = WRK.waitq_out;
				WRK.waitq_out = 0xFFFFFFFF;
				WRK.waitq_in = 0xEEEEEEEE;
			}

			/*
			 *  if the CU is not in the acceptance phase,
			 *  start the Command Unit
			 */

			if ( WRK.wdt_setter == WDT_INACTIVE ) {
				if ( WRK.xmits_pending > 0 ) {
					WRK.wdt_setter = WDT_XMIT;
					w_start(( &WDT ));
				}
			}
		}

		/*
		 *  return transmit element to common code 
		 *  log an error if necessary 
		 */
		if (rc != CIO_OK) {
			if (rc != ENT_TX_UNDERRUN && rc != ENT_TX_MAX_COLLNS)
				ent_logerr(dds_ptr, ERRID_IENT_ERR5, rc, csc_value, 
						  		stat_value);
	     		cio_xmit_done( dds_ptr, &WRK.xmit_elem[WRK.readyq_out],
							CIO_TIMEOUT, rc);
		}
		else {
	     		cio_xmit_done( dds_ptr, &WRK.xmit_elem[WRK.readyq_out],
								rc, rc);
		}
	
		WRK.readyq_out = bump_que( WRK.readyq_out, DDI.cc.xmt_que_size);
	}

	/*
	 *  set the proper acknowledge bits and issue 
	 *  a CA. If a No Resource interrupt was received,
	 *  resume the CU. 
	 */
	COMMAND_QUIESCE();
	if ( stat_value & STAT_CNA ) {
		WRITE_SHORT(WRK.scb_ptr->command, CMD_ACK_CNA | CMD_ACK_CX);
	}
	else
		WRITE_SHORT(WRK.scb_ptr->command, CMD_ACK_CX);

	CHANNEL_ATTENTION();

#ifdef	DEBUG
	trace(3,0,WRK.xmits_pending ,WRK.readyq_in,WRK.readyq_out,WRK.waitq_in, WRK.waitq_out);
#endif	/* DEBUG */

	if ( WRK.xmits_pending > 0 ) {
		WRK.wdt_setter = WDT_XMIT;
		w_start(( &WDT ));
	}
		
#ifdef	DEBUG
        READ_SHORT(WRK.scb_ptr->status, &stat);
	TRACE2 ("xmt9", stat);
#endif	/* DEBUG */
#ifdef	DEBUG
	TRACE5 ("xmtE", WRK.readyq_out, WRK.readyq_in, WRK.waitq_out, WRK.waitq_in );
#endif	/* DEBUG */
	return( 0 );
}

