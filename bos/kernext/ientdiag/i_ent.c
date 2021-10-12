static char sccsid[] = "@(#)52  1.33.1.6  src/bos/kernext/ientdiag/i_ent.c, diagddient, bos412, 9447C 11/22/94 17:26:48";
/*
 * COMPONENT_NAME:(SYSXIENT) Ethernet Device Driver-Integrated Ethernet adapter
 *
 * FUNCTIONS:
 *         xxx_initdds, xxx_act, build_sms
 *	       ent_recv_setup, ent_xmit_setup, ent_action_setup
 *	       ent_recv_free, ent_xmit_free, 
 *	       ent_action, ent_action_done
 *	       xxx_inact, xxx_close, xxx_halt
 *	       xxx_badext, xxx_startblk, xxx_ioctl
 *         entsetpos, ent_logerr
 *
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
 *	routines with a xxx_ prefix interface with the CIO subsystem
 *	routines with a ent  prefix are used internally by the driver
 *
 *      SERIALIZATION
 *
 *	The driver is responsible for serializing accesses to all of
 *	its structures. The driver disables to the interrupt priority 
 *	passed down from the configuration method.
 *
 *      SYSTEM INTERACTION
 *
 *	The driver makes use of the watchdog timer. This timer is used
 *	when the driver expects an interrupt to ocurr. It is used  when:
 *	- Any action command is issued to the Ethernet controller
 *
 *
 */

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
#include <sys/types.h>
#include <sys/user.h>
#include <sys/watchdog.h>
#include <sys/xmem.h>
#include <sys/comio.h>
#include <sys/ciouser.h>
#include <sys/entuser.h>
#include "i_entddi.h"
#include "i_cioddi.h"
#include "i_cioddhi.h"
#include "i_entdslo.h"
#include "i_entdshi.h"
#include "i_ciodds.h"
#include "i_cioddlo.h"

extern int fastwrite(
	int devno,
	struct mbuf *p_mbuf);


extern int cioconfig ( 
	dev_t 	devno, 
	int	cmd, 
	struct	uio	*uiop);

/*    
 * NAME: xxx_initdds
 *                 
 * FUNCTION: Device specific initialization of the dds.
 *                                                    
 * EXECUTION ENVIRONMENT:                            
 *                                                  
 *     	     This routine runs only under the process thread.
 *                                                          
 * NOTES:                                                  
 *                                                        
 *    Input:  pointer to a DDS structure. 		
 *                                                     
 *    Output: An updated dds, the Vital Product data read and the
 *            network address selected.                         
 *                                                             
 *    Called From: config_init                                
 *                                                           
 *    Calls To: 
 *                                                         
 * RETURN:  0 = successful				
 *          EINVAL = not enough TCWs 
 */                                                        
int 
xxx_initdds( dds_ptr )
	dds_t 	*dds_ptr;     		/* DDS pointer  */
{
	ulong	ioaddr;			/* IO handle */
	ulong	tcw_pool;		/* our allocated pool of TCW's */
	ulong	i, j; 
	uchar	*pos_ptr;	
	uchar	pos_id;	

	/* attach to IO space */
        ioaddr = io_att( DDI.cc.bus_id | IOCC_SELECT, 0 );
	pos_ptr = ioaddr | 0x004e0000;
	pos_id = *pos_ptr;
	WRK.do_ca_on_intr = 0;
	if ( pos_id == PID_LSB_STILWELL ) {
		WRK.machine = MACH_STILWELL;
		WRK.aram_mask = 0xFFF;	     /* generate ASIC RAM addresses */
	}
	else {
		WRK.machine = MACH_SALMON;   /* Salmon or Rainbow. */
		WRK.aram_mask = 0xFFFFFFFF;	
		if ( pos_id != PID_LSB_SALMON ) {
		  /* This is a hack for Rainbow, we have to make sure
                     that we do a channel_attention for every intr.
                     this will allow us to get the mc-intr reset. */
		  WRK.do_ca_on_intr |= IS_A_RAINBOW;  /* Is a Rainbow-3. */
		}
	}
	io_det( ioaddr );

	/*
	 *  this routine will get the configured values to set-up the device
	 *  driver workspace.  The configuration values were passed from 
	 *  the config methods. Divide the allocated bus memory between 
	 *  the Shared Memory Structure (SMS), the xmit area, the action 
	 *  command area and the receive area
	 */

	/*
	 *  there is one TCW per 4K page of bus memory space
	 */
	tcw_pool = (DDI.ds.tcw_bus_mem_size >> DMA_L2PSIZE);
        
	WRK.dma_base = DDI.ds.tcw_bus_mem_addr;  /* bus mem base addr */
        WRK.timr_priority     = DDI.cc.intr_priority;

	/* 
	 *  setup the transmit buffers bus address base at the
	 *  end of the SMS
	 */
	tcw_pool--;
	WRK.xmit_tcw_cnt = (tcw_pool / 2);

	/* 
	 *  setup the receive buffers bus address base at the
	 *  end of the transmit buffer region.
	 */
	WRK.recv_tcw_cnt = tcw_pool - WRK.xmit_tcw_cnt;

	/* 
	 *  if the mbuf data offset is not on a full word boundary,
	 *  force it to a word boundary
	 */
	if ((DDI.cc.rdto & 0x0003) != 0x0000) { 
	 	WRK.rdto = ((DDI.cc.rdto & 0xFFFC) + 0x0004);
	}
	else
		WRK.rdto = DDI.cc.rdto;

	/* Put Network Address in DDS  */
	for (j = 0; j < ent_NADR_LENGTH; j++,i++)	{
	   WRK.ent_vpd_addr[j] = DDI.ds.eth_addr[j];
        }

	/* 
	 *  determine which network address to use, 
	 *  either original or alternate   
	 */
	if (DDI.ds.use_alt_addr == 0) { 
		/* 
		 *  use the network address that was passed via the ds 
		 */
		for ( i=0; i < ent_NADR_LENGTH; i++) {
		 	WRK.ent_addr[i] = WRK.ent_vpd_addr[i];
		}
	}
	else { 
		/* 
		 *  use the network address that was passed in the DDS 
		 */
		for ( i=0 ; i < ent_NADR_LENGTH ; i++) {
		 	WRK.ent_addr[i] = DDI.ds.alt_addr[i];
		}
	} 

	WRK.channel_allocated = FALSE;		/* got a DMA channel */
	WRK.adapter_state  = NOT_STARTED;	/* update adapter state */
	WRK.multi_count = 0; 
	return (0);
} 


/*
 * NAME: entsetpos
 *  
 * FUNCTION: Populate the POS registers with the configuration
 *	     information found in the DDS.
 *                                                        
 * EXECUTION ENVIRONMENT:                                   
 *                                                         
 *           This routine runs only under the process thread.
 *                                                          
 * NOTES:                                                  
 *                                                        
 *    Input:  DDS pointer - device specific information	
 *                                                     
 *    Output: updated POS registers 		
 *                                             
 *    Called From: xxx_act		
 */                                     
void 
entsetpos( dds_ptr )
	dds_t	*dds_ptr;
{
	ulong	ioaddr;
	int     i;

	/*
	 *  update the POS values in the DDS, then store to POS
	 */
	WRK.pos_reg[POS_3] = 0x00; 
	WRK.pos_reg[POS_6] = 0x00;  
	WRK.pos_reg[POS_5] = 0xC0;

	/*
	 *  set DMA arbitration level and ethernet fairness in POS 4
	 *  set interrupt level, ethernet card enable, parity
	 *  and select feedback exception in POS 2
	 */
	if (SALMON()) {
		WRK.pos_reg[POS_4] =  DDI.ds.dma_arbit_lvl;
		WRK.pos_reg[POS_2] =  POS2_CARD | POS2_PARITY_SALMON | 
								POS2_CSF_SALMON
;
	} else {
		WRK.pos_reg[POS_4] =  DDI.ds.dma_arbit_lvl << 
			POS4_ARBSHIFT_STILWELL | POS4_FAIR_STILWELL;

		WRK.pos_reg[POS_2] =  POS2_CARD | POS2_INT_ENABLE | 
				POS2_82596_STILWELL |  POS2_PARITY_STILWELL |
				POS2_CSF_STILWELL | POS2_82596_STILWELL;
	}

	/*
	 *  use the configuration value to determine the bus interrupt level
	 */
	switch ( DDI.cc.intr_level ) {
		case 5:
			WRK.pos_reg[POS_2] |= POS2_IRQ5;
			break;
		case 7:
			WRK.pos_reg[POS_2] |= POS2_IRQ7;
			break;
		case 9:
			WRK.pos_reg[POS_2] |= POS2_IRQ9;
			break;
		case 10:
			WRK.pos_reg[POS_2] |= POS2_IRQ10;
			break;
		case 11:
			WRK.pos_reg[POS_2] |= POS2_IRQ11;
			break;
		case 12:
			WRK.pos_reg[POS_2] |= POS2_IRQ12;
			break;
		default:
			panic("entdd:  invalid interrupt level");
	}

        /* attach to IO space */
        ioaddr = io_att( (DDI.cc.bus_id | IOCC_SELECT) & NO_ADDR_CHECK_MASK,
                                 POS_OFFSET );

#ifdef DEBUG

	WRK.pos_reg[POS_0] = BUS_GETC(ioaddr + POS_0);	/* POS ID = 0xF38E */
	WRK.pos_reg[POS_1] = BUS_GETC(ioaddr + POS_1);

#endif /* DEBUG */

	if( WRK.pos_request )
	{
		/*
		 *  diagnostics requested non-default
		 *  values in the POS
		 */
		uchar	*pos_val;
		pos_val = (uchar *) &WRK.pos;
		if( SALMON() ) {
                        WRK.pos_reg[POS_2] |= pos_val[0] & 0x7F;
                        WRK.pos_reg[POS_4] |= pos_val[1] & 0xF0;
                        WRK.pos_reg[POS_5] = pos_val[2];
		}
		else {
			WRK.pos_reg[POS_2] &= 0xE9;
                        WRK.pos_reg[POS_2] |= pos_val[0] & 0x3F;
			WRK.pos_reg[POS_4] &= 0xF0;
                        WRK.pos_reg[POS_4] |= pos_val[1] & 0x0F;
                        WRK.pos_reg[POS_5] = pos_val[2];
		}
		WRK.pos_request = FALSE;
	}


	/* 
	 *  update the POS registers 
	 *  NOTE: cannot generate an exception when accessing POS
	 */
	BUS_PUTC(ioaddr + POS_4, WRK.pos_reg[POS_4]);
	BUS_PUTC(ioaddr + POS_5, WRK.pos_reg[POS_5]);
	BUS_PUTC(ioaddr + POS_6, WRK.pos_reg[POS_6]);
	BUS_PUTC(ioaddr + POS_2, WRK.pos_reg[POS_2]); /* enable ethernet last *
/

	io_det( ioaddr );  			/* detach IO handle */

}

/*
 * NAME: xxx_act
 *             
 * FUNCTION: activate and initalize the adapter: load the POS registers, 
 *           allocate resources, start the sequence for 		
 *	     hard reset of the adapter.               	
 *                                                     
 * EXECUTION ENVIRONMENT:                            
 *                                                 
 *      This routine runs only under the process thread.
 *                                                     
 * NOTES:                                             
 *                                                   
 *    Input: DDS pointer			
 *                                             
 *    Output: An initialized adapter.         
 *                                           
 *    Called From: cioioctl  ( CIO_START )
 *                                       
 *    Calls To:                         
 *		ent_action - issue an action command to the ethernet device 
 *              ent_action_setup - sets up the queue for the action cmd CBL
 *              ent_xmit_setup - sets up buffers, CBL, DMA etc.     	  
 *              ent_recv_setup - sets up TCWs & control variables for recv
 *              ent_xmit_free - release transmit buffer resourses      
 *              ent_recv_free - release receive buffer resourses      
 *              ent_act_free - release miscellaneous resources	
 *    	 	entsetpos - configure the POS registers.       
 *              ent_logerr   - makes error log entries for fatal errors 
 *              build_sms   - creates the Shared Memory Structure
 *              cio_conn_done - set status to acknowledge the start.
 *              d_init - get DMA channel id                        
 *              d_clear - free the DMA channel                    
 *              w_start - start the watch dog timer              
 *                                                     
 * RETURN:  0 = OK                                    
 *                                                   
 */
int 
xxx_act( dds_ptr )
	dds_t  	 *dds_ptr;
{
	ulong    ioaddr;	/* IO handle */
	ulong    ipri;		/* interrupt priority */
	ulong    i;             /* loop counters */
	ulong    rc, rc_sum;	/* return code */
	ulong	 busy;
	ulong	 status;

	TRACE1("actS");

	/*
	 *  ensure the DMA channel was d_clear'd on the last close?
	 */
        WRK.adapter_state = STARTING;
	if (WRK.channel_allocated == FALSE)
	{
		/*  allocate a DMA channel */
		if ( ( WRK.dma_channel = d_init(DDI.ds.dma_arbit_lvl,
		             MICRO_CHANNEL_DMA, DDI.cc.bus_id )) != DMA_FAIL )
			WRK.channel_allocated = TRUE; 
		else
		{
			/* notify DD that the 1st start completed */
			WRK.connection_result = (ulong)EFAULT;
			cio_conn_done(dds_ptr);

			/* log the error */
			ent_logerr(dds_ptr,ERRID_IENT_ERR2,DDI.ds.dma_arbit_lvl
,
					WRK.dma_channel, DDI.cc.bus_id);
                        WRK.adapter_state=NOT_STARTED;
                        TRACE2 ("actX", 0);
			return( -1 );
		}
	}

	/* 
	 *  populate the adapter's POS registers 
	 */
	entsetpos(dds_ptr);

	/*
	 *  register the interrupt handler to the kernel
	 */
	ipri = i_disable( DDI.cc.intr_priority );
        if ( (WRK.restart == FALSE) && (!CIO.intr_registered) )
        {
                rc = i_init ((struct intr *)(&(IHS)));
                CIO.intr_registered = TRUE;
        }

        i_mask ((struct intr *)(&(IHS)));
        i_enable (ipri);
        ioaddr=io_att (DDI.cc.bus_id, 0);
	WRK.control_pending = TRUE;
	port( dds_ptr, ioaddr, 0x00 );
        DELAYMS( 1000 );

	/* build the Shared Memory Structure */
	if (( rc = build_sms( dds_ptr, ioaddr )) != 0 )
	{
		/* notify comon DD that the 1st start failed */
		WRK.connection_result = (ulong)EFAULT;
		cio_conn_done(dds_ptr);

		/* log the error */
		ent_logerr(dds_ptr, ERRID_IENT_ERR2, rc, 0, 0);
                io_det (ioaddr);
        	i_unmask ((struct intr *)(&(IHS)));
                WRK.adapter_state=NOT_STARTED;
                TRACE2 ("actX", 1);
		return( -1 );
	}
		
	/* 
	 *  setup control structures and buffers
	 *  for transmit, receive and misc. commands
	 */
	rc = ent_recv_setup( dds_ptr, ioaddr );	
	rc_sum = rc;
	rc = ent_xmit_setup( dds_ptr, ioaddr ); 
	rc_sum |= rc;
	rc = ent_action_setup( dds_ptr, ioaddr );
	rc_sum |= rc;
	if ( rc_sum != 0 )
	{
		/* 
	 	 *  free up any resources that were allocated 
	 	 *  for receive, transmit and action commands.
		 *  note: the error was logged in the setup routine
         	 */
		ent_recv_free( dds_ptr );
		ent_xmit_free( dds_ptr );

		/* 
		 *  free the DMA channel
		 */
		if (WRK.channel_allocated == TRUE)
		{
			d_clear (WRK.dma_channel);
			WRK.channel_allocated = FALSE;
		}
        	i_unmask ((struct intr *)(&(IHS)));

		/* 
		 *  notify the device driver that the first start failed
		 */
		if( WRK.restart == FALSE )
		{
			WRK.connection_result = (ulong)ENOMEM;
			cio_conn_done(dds_ptr);
		}
                io_det (ioaddr);
                WRK.adapter_state=NOT_STARTED;
                TRACE2 ("actX", 2);
		return( -1 );
	}

	/*
	 *  relocate the default SCP
	 *  requires a port access
	 *  Note: the SCP must be on a 16-byte boundary. This is 
	 *  necessary because the low-order four bits are used to
	 *  distinguish the port access command. scp is on a page boundary
	 */
	port( dds_ptr, ioaddr, (int)WRK.scp_addr | 0x2 );
        WRITE_SHORT(WRK.scb_ptr->command, CMD_CUC_NOP);
 	CHANNEL_ATTENTION();

	/*
	 *  how will the DD determine for sure
	 *  that the initialization sequence worked
	 *  properly - ISCP should have been cleared?
	 */
        for (i = 0; i < 10; i++) {
	  DELAYMS( 100 );
          READ_LONG(WRK.iscp_ptr->busy, &busy);
	  if( busy ) { 
           ulong   io_addr;
	        i_reset ((struct intr *)(&(IHS)));
        	WRK.control_pending = TRUE;
		io_addr = io_att(  DDI.cc.bus_id | IOCC_SELECT, 0 );
		entsetpos(dds_ptr);
		io_det( io_addr );
		port( dds_ptr, ioaddr, 0x00 );
       		WRK.control_pending = FALSE;
       		DELAYMS( 100 );
		port( dds_ptr, ioaddr, (int)WRK.scp_addr | 0x2 );
       		WRITE_SHORT(WRK.scb_ptr->command, CMD_CUC_NOP);
 		CHANNEL_ATTENTION();
          } else {
            break;
          }
        }
        if (busy) {
		ent_logerr(dds_ptr, ERRID_IENT_ERR1, EBUSY, WRK.sysmem,
			 WRK.dma_channel);
                WRK.adapter_state = NOT_STARTED;
        	i_unmask ((struct intr *)(&(IHS)));
                io_det (ioaddr);
                TRACE2 ("actX", 3);
		return( -1 );
	} 

	WRK.control_pending = FALSE;

	/*
	 *  determine wether to use the original network address 
	 *  or a user specified address
	 *  issue an Individual Address Setup action command
	 *  to load the 82596 with its IA address
	 */
	rc = ent_action( dds_ptr, IAS_CMD, WRK.ent_addr );
	DELAYMS( 100 ); 
	rc = ent_action( dds_ptr, CFG_CMD, NULL );

	/*
	 *  start the receive unit
	 *  necessary to start  RU after the
	 *  IA command has been sent to the adapter
	 *  an interrupt is not posted as a result of this control cmd
	 */
	rc = start_ru( dds_ptr, ioaddr );
	
	/* 
	 *  send notification that the first start has completed
	 *  successfully.
         */
	WRK.connection_result = (ulong)CIO_OK;
	cio_conn_done( dds_ptr );

        WRK.adapter_state = STARTED;  
        io_det (ioaddr);
        i_unmask ((struct intr *)(&(IHS)));
	TRACE1("actE");
        return(0);
}

/*
 *
 * NAME: build_sms
 *                                          
 * FUNCTION: build the Shared Memory Structure
 *           this includes the SCP, ISCP, SCP, and the CBL's	
 *                                                             
 * EXECUTION ENVIRONMENT:                                     
 *                                                           
 *      This routine runs only under the process thread.    
 *                                                         
 * NOTES:                                                
 *                                                      
 *    Output: initialized SMS including bus memory allocation	 
 *                                                              
 *    Called From: xxx_act				
 *                                                     
 *    Calls To: d_master, bzero, xmalloc, xmfree, xmattach
 *
 *
 */
int
build_sms( dds_ptr, ioaddr )
	dds_t   *dds_ptr;
	ulong	ioaddr;
{
	ulong	i;
	ulong	rc;

	/*  
	 *  xmalloc a block of memory and initialize it to be the block
	 *  of shared memory that the CPU and 82596 use to communicate
	 *  with one another. This block contains the Shared Memory Structure
	 *  (SMS), which includes the ISCP, SCP, SCB and the CBL's.
	 *  if (SALMON()), malloc a chunk of system memory for the SMS. Otherwi
se,
	 *  setup the IO registers to indicate the base address of the ASIC RAM
	 *  in IO space where the SMA will reside.
	 */
	if (SALMON())
	{
		WRK.alloc_size =
			sizeof(struct scp) +
			sizeof(struct iscp) +
			sizeof(struct scb) +
			sizeof(struct xcbl) * DDI.cc.xmt_que_size +
			sizeof(struct acbl) * 1 +
			sizeof(xmt_elem_t) * DDI.cc.xmt_que_size +
			sizeof(struct xmit_buffer) * XMIT_BUFFERS +
			sizeof(struct tbd) * XMIT_BUFFERS +
			sizeof(struct recv_buffer) * ADP_RCV_QSIZE +
			sizeof(struct rfd) * ADP_RCV_QSIZE +
			sizeof(struct rbd) * ADP_RCV_QSIZE;
		WRK.sysmem = (ulong)xmalloc(WRK.alloc_size, 12, pinned_heap);
		WRK.sysmem_end = WRK.sysmem;
		WRK.scp_ptr = ( struct scp *) WRK.sysmem;
		WRK.scp_addr = (struct scp *)SMTOIOADDR(WRK.scp_ptr);
	}
	else
	{
		WRK.alloc_size =
			sizeof(xmt_elem_t) * DDI.cc.xmt_que_size +
			sizeof(struct xmit_buffer) * XMIT_BUFFERS +
			sizeof(struct recv_buffer) * ADP_RCV_QSIZE + 0x80;
		WRK.sysmem = (ulong)xmalloc(WRK.alloc_size, 12, pinned_heap);
		WRK.sysmem_end = WRK.sysmem;
		WRK.asicram = DDI.ds.bus_mem_addr;
		WRK.asicram_end = WRK.asicram;
		BUSPUTC(RAM_ADDR_LOW, ((ulong)WRK.asicram_end >> 12) & 0xFF);
		BUSPUTC(RAM_ADDR_HIGH, ((ulong)WRK.asicram_end >> 20) & 0xFF);
		WRK.scp_ptr = ( struct scp *) WRK.asicram_end;
		WRK.scp_addr = (struct scp *)((ulong)WRK.scp_ptr & ARAM_MASK);
	}

	if ( WRK.sysmem == NULL || ( !SALMON() && WRK.asicram == NULL ))
	{ 
		/* 	
		 *  xmalloc or PIO failed
		 *  notify caller and return error
		 *  error is logged in xxx_act
		 */
		if (WRK.sysmem != NULL) xmfree(WRK.sysmem, pinned_heap);
		return ( -1 );
	}

	/*  clear the control block page  */
	bzero( WRK.sysmem, WRK.alloc_size);	

	if( ! SALMON() ) {
		for (i = 0; i < PAGESIZE / sizeof (int); ++i) {
			BUSPUTL((ulong *)WRK.asicram + i, 0); 
		}
	}
 
	/*  
	 *  divide malloc'd memory between the SCP, ISCP, SCB and the CBL's
         */
        WRK.iscp_ptr = (struct iscp *)(WRK.scp_ptr  + 1);
        WRK.iscp_addr = (struct iscp *)(WRK.scp_addr + 1);
        WRK.scb_ptr  = (struct scb *)(WRK.iscp_ptr + 1);
        WRK.scb_addr = (struct scb *)(WRK.iscp_addr + 1);
        WRK.xcbl_ptr = (struct xcbl *)(WRK.scb_ptr  + 1);
        WRK.xcbl_addr = (struct xcbl *)(WRK.scb_addr + 1);
        WRK.acbl_ptr = (struct acbl *)(WRK.xcbl_ptr + DDI.cc.xmt_que_size);
        WRK.acbl_addr = (struct acbl *)(WRK.xcbl_addr + DDI.cc.xmt_que_size);

	if (SALMON())
		WRK.sysmem_end = WRK.acbl_ptr + 1;
	else
		WRK.asicram_end = WRK.acbl_ptr + 1;

        /*
         *  initialize the SCP, ISCP  and the SCB
         */
	if( SALMON() )
        	WRITE_LONG(WRK.scp_ptr->sysbus, SCP_CFG_SALMON)
	else
        	WRITE_LONG(WRK.scp_ptr->sysbus, SCP_CFG_STILWELL);
        WRITE_LONG_REV(WRK.scp_ptr->iscp_addr, WRK.iscp_addr); 
        WRITE_LONG(WRK.iscp_ptr->busy, BUSY);
      	WRITE_LONG_REV(WRK.iscp_ptr->scb_addr, WRK.scb_addr);
	WRITE_SHORT(WRK.scb_ptr->status, 0x0);
	WRITE_SHORT(WRK.scb_ptr->command, CMD_RUC_ABORT);
	WRITE_LONG(WRK.scb_ptr->cbl_addr, 0x0);
	WRITE_LONG(WRK.scb_ptr->rfa_addr, 0x0);

	/* 
	 *  create the cross_memory descriptor to be used
	 *  for all DMA setup's.
	 */
	WRK.xbuf_xd.aspace_id = XMEM_INVAL;
	if ( xmattach( WRK.scp_ptr, PAGESIZE, &(WRK.xbuf_xd), 
		      SYS_ADSPACE) != XMEM_SUCC)
	{
		TRACE1("txsF");
		xmfree(WRK.sysmem, pinned_heap);
		return(-1);
	}

	/*  
	 *  map the Shared Memory Structure(SCB) for DMA 
	 */
	d_master(WRK.dma_channel, DMA_NOHIDE, WRK.sysmem,
		 WRK.alloc_size, (struct xmem *)&WRK.xbuf_xd, 
		 (char *)WRK.dma_base);
	return( 0 );
}

/*
 *
 * NAME: ent_xmit_setup  
 *                      
 * FUNCTION:  Sets up the transmit buffers and queues
 *                                                  
 * EXECUTION ENVIRONMENT:                          
 *      This routine runs only under the process thread.
 *                                                     
 * NOTES:                                             
 *                                                   
 *    Input: Channel Name - Device Dependent Structure
 *                                                   
 *    Output: N/A                                   
 *                                                 
 *    Called From: xxx_act                        
 *                                               
 *    Calls To: d_master, xmalloc, entlogerr, cio_conn_done, bzero 
 *                                                             
 * RETURN:  0 = successful					
 *         -1 = on error				
 *                                                     
 */
int
ent_xmit_setup( dds_ptr, ioaddr )
 	dds_t	*dds_ptr;
	ulong	ioaddr;
{
	ulong	cache_align, i;
	ulong boundary;

	/*  
	 *  initialize the transmit Command Block List (CBL) 
	 */
        for (i = 0; i <  DDI.cc.xmt_que_size; i++)
        {
              WRITE_LONG_REV(WRK.xcbl_ptr[i].xmit.next_cb,0xFFFFFFFF);
              WRITE_LONG(WRK.xcbl_ptr[i].xmit.csc, CB_EL | CB_SUS | CB_INT);
              WRITE_SHORT(WRK.xcbl_ptr[i].xmit.tcb_cnt, 0x00 );
        } 

	/* initialize transmit queue indices */
        WRK.readyq_in = 0;
        WRK.readyq_out = 0;
        WRK.waitq_out = 0xFFFFFFFF;
        WRK.waitq_in = 0xF00DF00D;
        WRK.xmits_queued = 0;
        WRK.xmits_pending = 0;

	/* allocate xmit elements from system memory */
	WRK.xmit_elem = (xmt_elem_t *) WRK.sysmem_end;
	WRK.sysmem_end = (ulong)(WRK.xmit_elem + DDI.cc.xmt_que_size);

	/* 
	 *  allocate transmit buffers from system memory
	 *  align the transmit buffers on cache line
	 *  boundaries. Here we assume a cache line size
	 *  of 0x80. Replace this assumption with the
	 *  d_align routine found in LIBSYSP
	 */
        cache_align = WRK.sysmem_end & 0x000000FF;
	/* make sure that the transmit buffers start on a 64 byte boundary */
	if( cache_align <= 0x40 )
		boundary = 0x40;
	else if( cache_align <= 0x80 )
		boundary = 0x80;
	else if( cache_align <= 0xC0 )
		boundary = 0xC0;
	else
		boundary = 0x100;
        cache_align = boundary - cache_align;
        WRK.sysmem_end += cache_align;
	WRK.xmit_buf_ptr = ( struct xmit_buffer *) WRK.sysmem_end;
	WRK.sysmem_end = (ulong)(WRK.xmit_buf_ptr + XMIT_BUFFERS);
	WRK.xmit_buf_addr = (struct xmit_buffer *)SMTOIOADDR(WRK.xmit_buf_ptr);

	/* allocate transmit buffers descriptors */
	if( SALMON() )
	{
		WRK.tbd_ptr = ( struct tbd *) WRK.sysmem_end;
		WRK.sysmem_end = (ulong)(WRK.tbd_ptr + XMIT_BUFFERS);
		WRK.tbd_addr = (struct tbd *)SMTOIOADDR(WRK.tbd_ptr);
	}
	else
	{
		WRK.tbd_ptr = ( struct tbd *) WRK.asicram_end;
		WRK.asicram_end = (ulong)(WRK.tbd_ptr + XMIT_BUFFERS);
		WRK.tbd_addr = (struct tbd *)((ulong)WRK.tbd_ptr & ARAM_MASK);
	}

	/*  clear the transmit buffers */
	bzero (&WRK.xmit_buf_ptr[0],XMIT_BUFFERS * sizeof(struct xmit_buffer));

	/*  
	 *  initialize the transmit buffer descriptors
	 */
	for (i = 0; i < XMIT_BUFFERS; i++) {
	   WRITE_LONG_REV(WRK.tbd_ptr[i].next_tbd, 0xFFFFFFFF ); 
	   WRITE_LONG_REV(WRK.tbd_ptr[i].tb_addr,&WRK.xmit_buf_addr[i].buf);
	   WRITE_LONG(WRK.tbd_ptr[i].control, 0);
        }
 
	WRITE_LONG_REV(WRK.scb_ptr->cbl_addr, &WRK.xcbl_addr[0]);

	/*
	 *  initialize flags and counters
	 */
	WRK.xmit_buffers_allocd = XMIT_BUFFERS;
	WRK.xmit_buffers_used = 0;
	WRK.buffer_in = 0;
	WRK.xmit_que_active = TRUE;

	return(0);
}


/* 
 * NAME: ent_recv_setup 
 *                     
 * FUNCTION:  Sets up the receive buffers and the receive queue
 *                                                            
 * EXECUTION ENVIRONMENT:                                    
 *      This routine runs only under the process thread.   
 *                                                        
 * NOTES:                                                
 *                                                      
 *    Input: Channel Name - Device Dependent Structure
 *                                                   
 *    Output: N/A                                   
 *                                                 
 *    Called From: xxx_act                       
 *                                              
 *    Calls To: d_master, xmalloc, entlogerr, cio_conn_done, bzero 
 *                                                                
 * RETURN:  0 = successful					
 *         -1 = on error				
 *                                                     
 */
int 
ent_recv_setup( dds_ptr, ioaddr )
	dds_t	*dds_ptr;
	ulong	ioaddr;
{
	ulong	i;			
	ulong	rc;			
	ulong	bufs;

	/*  
	 *  set-up the receive frame area ( RFA )
	 *  malloc a block of memory and initialize it to be the 
	 *  receive frame descriptors (RFD's) and receive buffers 
	 */

	bufs = ADP_RCV_QSIZE;

	if (SALMON())
	{
		/* allocate RFDs and RBDs from system memory */
		WRK.rfd_ptr = (struct rfd *) WRK.sysmem_end;
		WRK.sysmem_end = (ulong)(WRK.rfd_ptr + bufs);
		WRK.rfd_addr = SMTOIOADDR(WRK.rfd_ptr);
		WRK.rbd_ptr = (struct rbd *) WRK.sysmem_end;
		WRK.sysmem_end = (ulong)(WRK.rbd_ptr + bufs);
		WRK.rbd_addr = SMTOIOADDR(WRK.rbd_ptr);
	}
	else
	{
		/* allocate RFDs and RBDs from ASIC RAM */
		WRK.rfd_ptr = (struct rfd *) WRK.asicram_end;
		WRK.asicram_end = (ulong)(WRK.rfd_ptr + bufs);
		WRK.rfd_addr = (ulong)WRK.rfd_ptr & ARAM_MASK;
		WRK.rbd_ptr = (struct rbd *) WRK.asicram_end;
		WRK.asicram_end = (ulong)(WRK.rbd_ptr + bufs);
		WRK.rbd_addr = (ulong)WRK.rbd_ptr & ARAM_MASK;
	}

	/* allocate receive buffers from system memory */
	WRK.recv_buf_ptr = (struct recv_buffer *) WRK.sysmem_end;
	WRK.sysmem_end = (ulong)(WRK.recv_buf_ptr + bufs);
	WRK.recv_buf_addr = SMTOIOADDR(WRK.recv_buf_ptr);

	WRK.recv_buffers = bufs;

	/* zero out the receive buffers */
	bzero ( &WRK.recv_buf_ptr[0], bufs * sizeof (struct recv_buffer) );
	vm_cflush ( &WRK.recv_buf_ptr[0], bufs * sizeof (struct recv_buffer) );


	/*  
	 *  initialize the RFDs and RBDs
	 */
        for (i = 0; i < bufs; i++)
        {
		/* initialize RFD fields */
        	WRITE_LONG_REV(WRK.rfd_ptr[i].next_rfd, &WRK.rfd_addr[i+1]);
        	WRITE_LONG_REV(WRK.rfd_ptr[i].rbd, 0xffffffff);
        	WRITE_SHORT_REV(WRK.rfd_ptr[i].size, NBR_DATA_BYTES_IN_RFD);
        	WRITE_SHORT(WRK.rfd_ptr[i].count, 0);
		WRITE_LONG(WRK.rfd_ptr[i].csc, CB_SF);

		/* initialize RBD fields */
		WRITE_LONG_REV(WRK.rbd_ptr[i].next_rbd, &WRK.rbd_addr[i+1]);
		WRITE_LONG_REV(WRK.rbd_ptr[i].rb_addr, &WRK.recv_buf_addr[i]);
		WRITE_LONG_REV(WRK.rbd_ptr[i].size, sizeof WRK.recv_buf_ptr[i].
buf);
        }
        WRITE_LONG(WRK.rfd_ptr[i - 1].csc, CB_EL | CB_SUS | CB_SF);
        WRITE_LONG_REV(WRK.rbd_ptr[i - 1].size,sizeof WRK.recv_buf_ptr[i-1].buf
 								| 0x00008000);

	WRK.el_ptr = i - 1;
	WRK.end_fbl = i - 1;
        WRK.rbd_el_ptr=&WRK.rbd_ptr[i-1];


	/* make the RFA circular */
        WRITE_LONG_REV(WRK.rfd_ptr[i-1].next_rfd, &WRK.rfd_addr[0]);
	WRITE_LONG_REV(WRK.rbd_ptr[i-1].next_rbd, &WRK.rbd_addr[0]);

	/* make the SCB point to the head of the RFD list */
	WRITE_LONG_REV(WRK.scb_ptr->rfa_addr, &WRK.rfd_addr[0]);
	WRK.begin_fbl = 0;			/* 1st free buffer */
        WRITE_LONG_REV(WRK.rfd_ptr[0].rbd, &WRK.rbd_addr[0]);
	WRK.prev_netid = CIO.netid_table_ptr[0].netid;
	WRK.prev_netid_length = CIO.netid_table_ptr[0].length;
	WRK.prev_netid_indice = 0;

	return(0);
}

/*
 *
 * NAME: ent_action_setup 
 *                       
 * FUNCTION:  Sets up the action queue
 *                                   
 * EXECUTION ENVIRONMENT:          
 *      This routine runs only under the process thread.
 *                                                     
 * NOTES:                                             
 *                                                   
 *    Input: Channel Name - Device Dependent Structure
 *                                                   
 *    Output: N/A                                   
 *                                                 
 *    Called From: xxx_act                        
 *                                              
 *    Calls To: d_master, xmalloc, entlogerr, cio_conn_done, bzero
 *                                                               
 * RETURN:  0 = successful					
 *         -1 = on error				
 *                                                     
 */
int
ent_action_setup( dds_ptr, ioaddr )
	dds_t	*dds_ptr;
	ulong	ioaddr;
{

	/*
	 *  setup a Command Block List for
	 *  action commands other than the transmit commands.
	 *  only implement one action command block
	 */
	WRITE_LONG(WRK.acbl_ptr[0].next_cb, 0xFFFFFFFF);
	WRITE_LONG(WRK.acbl_ptr[0].csc, CB_EL | CB_SUS | CB_INT);
	WRK.action_que_active = FALSE;
	return( 0 );
}

/*
 *
 * NAME: ent_recv_free  
 *                     
 * FUNCTION: free receive resources
 *                                
 * EXECUTION ENVIRONMENT:        
 *                              
 *      This routine runs under the process level.
 *                                               
 * NOTES:                                       
 *                                             
 *                                            
 *                                          
 *    Called From: xxx_act - on failure      
 *                 xxx_inact - normal shutdown
 *                                           
 *                                          
 * RETURN:  0 - Successful completion      
 *                                        
 */
int 
ent_recv_free( dds_ptr, ioaddr )
	dds_t	*dds_ptr;  
	ulong	ioaddr;
{
	ulong 	rc;
	ushort	status;

        /*  stop the RU if necessary */
	SUSPEND_RU();

        WRK.begin_fbl = 0;                      /* 1st free buffer */
        WRK.save_bf = 0;                        /* save bad frames */
}

/*
 *
 * NAME: ent_xmit_free
 *                   
 * FUNCTION: Free transmit resources	
 *                                     
 * EXECUTION ENVIRONMENT:             
 *                                   
 *      This routine runs under both the interrupt level
 *      and the process thread.                        
 *                                                    
 * NOTES:                                            
 *                                                  
 *                                                 
 *                                                
 *    Called From: xxx_act - on failure          
 *                 xxx_inact - normal shutdown  
 *                                             
 *                                            
 * RETURN:  0 - Successful completion        
 *                                          
 */
int 
ent_xmit_free( dds_ptr )
	dds_t	*dds_ptr;  
{
	int     i;  
	ulong	ipri;

   	TRACE2 ("txuB",(ulong)dds_ptr); 	/* tx_free begin tracehook */

	ipri = i_disable( DDI.cc.intr_priority );

	if ( WRK.wdt_setter != WDT_INACTIVE )
	{
		w_stop (&(WDT)); 		/* stop the watchdog timer */
		WRK.wdt_setter = WDT_INACTIVE;
	}

	if (! XMITQ_EMPTY ) {
		/*
		 *  free the mbufs for all queued transmits
		 *  let's play it safe and check all indicies
		 */
		if ( WRK.xmits_queued > 0 ) {
		  for (i = 0; i < DDI.cc.xmt_que_size; i++) {
			if (WRK.xmit_elem[WRK.readyq_in].mbufp != NULL) {
			  m_freem( WRK.xmit_elem[WRK.readyq_in].mbufp );
			}
		  }
		}
		
	}
	i_enable( ipri );
	return( 0 );
}

/*
 *
 * NAME: xxx_inact
 *               
 * FUNCTION: Free up resources and re_initialize flags and counters
 *                                                                
 * EXECUTION ENVIRONMENT:                                        
 *                                                              
 *      This routine runs only under the process thread.       
 *                                                            
 * NOTES:                                                    
 *                                                          
 *    Input: Device dependent structure			
 *                                                     
 *    Output: N/A                                     
 *                                                   
 *    Called From:                                  
 *                                                 
 *    Calls To: N/A                               
 *                                               
 * RETURN:  					
 *              			
 *                                    
 */
int 
xxx_inact( dds_ptr )
	dds_t	*dds_ptr;
{
	int	i, k;
	ushort	cmd_value;
	ulong	ioaddr;
	ulong	ipri;
	ulong	rc;
    char    pos_val;

   	TRACE1 ("inaS");
	ipri = i_disable( DDI.cc.intr_priority );

	/* 
	 *  clear the multicast table 
	 */
	for (i=0; i < MAX_MULTI; i++)
	{
		WRK.multi_open[i] = NULL; 	/* invalidate each entry */
	}
	WRK.multi_count = 0;			/* no MC entries exist */
	
	WRK.adapter_state = NOT_STARTED; 	/* update the state variable */
	CIO.device_state = DEVICE_NOT_CONN;
       	i_reset ((struct intr *)(&(IHS)));

	/* Unregister the interrupt handler and */
        if (CIO.intr_registered)
        {
		i_clear ((struct intr *)dds_ptr);
                CIO.intr_registered = FALSE;
        }

    ioaddr = io_att((DDI.cc.bus_id | IOCC_SELECT) & NO_ADDR_CHECK_MASK,
                                                                POS_OFFSET );

    pos_val = BUS_GETC(ioaddr + POS_2);


    /*
    ** GBG - Had to put this in because the halt IOCTL disables the card and
    ** if we try to write to PORT this causes and IOCC exception which calls
    ** panic in the PORT routine.  So before we call port check to see if the
    ** card is enabled first.
    */

    if (pos_val & 0x01)
    {
        if (WRK.do_ca_on_intr & IS_A_RAINBOW) {
          /* disable the Ethernet adapter (for Rainbow POS2=0 is not enough) */
          /*
          ** Set WRK.restart to TRUE so that delay() routine is not called
          ** while interrupts are disabled.
          */

          WRK.restart = TRUE;
          port( dds_ptr, ioaddr, 0x00 );
          WRK.restart = FALSE;
        }

        BUS_PUTC(ioaddr + POS_2, 0x00 ); /* disable ethernet via POS */
    }

	io_det( ioaddr );

	/* Release dma channel ( flush IO buffer before releasing channel ) */
	if ( !SALMON() )
	{
		if (WRK.channel_allocated == TRUE)
		{
			rc = d_complete(WRK.dma_channel,DMA_NOHIDE,WRK.sysmem,
					WRK.alloc_size, (struct xmem *)&WRK.xbuf_xd,
							(char *)WRK.dma_base);
			if ( rc )
			{
				ent_logerr(dds_ptr, ERRID_IENT_ERR3, 
				 rc, WRK.sysmem, WRK.dma_channel, WRK.dma_base);
			}
		}
	}

	if (WRK.channel_allocated == TRUE)
	{
		d_clear (WRK.dma_channel); 	/* free the DMA channel */
		WRK.channel_allocated = FALSE;
	} 

	i_enable( ipri );
	/*  release allocated memory  */
	if ( WRK.sysmem != 0 )
	{
		if ((rc = xmfree( WRK.sysmem, pinned_heap )) != 0)
			ent_logerr(dds_ptr, ERRID_IENT_ERR4,0x1407,rc,dds_ptr);
	}

	unpin( &(IHS) , CIO.alloc_size );       /* unpin the DDS */
        unpincode( cioconfig );                 /* unpin the driver */
   	TRACE1 ("inaE");
}

/*
 *
 * NAME: xxx_halt 
 *               
 * FUNCTION: Build a halt done status block.
 *                                         
 * EXECUTION ENVIRONMENT:                 
 *                                       
 *      This routine runs only under the process thread.
 *                                                     
 * NOTES:                                             
 *                                                   
 *    Input: DDS pointer - tells which adapter     
 *           Open pointer - tells which open      
 *           Session Block pointer - tells which session status block
 *                                                                  
 *    Output: An updated status block that contains the current status
 *                                                                   
 *    Called From: cioioctl                                        
 *                                                                
 *    Calls To: cio_report_status                                
 *                                                              
 * RETURN:  N/A                                               
 *                                                           
 */
void 
xxx_halt( dds_ptr, open_ptr, sess_blk_ptr )
	dds_t		*dds_ptr;
	open_elem_t	*open_ptr;	/* opener */
	cio_sess_blk_t  *sess_blk_ptr; /* session status block */
{

	ushort	cmd_value;
	int k;
	ulong	ipri, rc;
	ulong	ioaddr;
	cio_stat_blk_t	stat_blk;

   	TRACE1 ("halS");
	stat_blk.code = (ulong)CIO_HALT_DONE;
	stat_blk.option[0] = (ulong)CIO_OK;
	stat_blk.option[1] = sess_blk_ptr->netid;
	cio_report_status( dds_ptr, open_ptr, &stat_blk );

	if (CIO.num_netids == 0) {
	  /* 
	   *  last halt, free resources and
	   *  disable device and interrupts
	   */
	  ipri = i_disable( DDI.cc.intr_priority );
	
	  WRK.adapter_state = NOT_STARTED; 	/* update the state variable */
	  CIO.device_state = DEVICE_NOT_CONN;
       	  i_reset ((struct intr *)(&(IHS)));

	  /* Unregister the interrupt handler and */
          if (CIO.intr_registered) {
	    i_clear ((struct intr *)dds_ptr);
            CIO.intr_registered = FALSE;
          }

	  i_enable ( ipri );
          ATT_MEMORY();
          port( dds_ptr, ioaddr, 0x00 );
          DET_MEMORY();  
	  ipri = i_disable( DDI.cc.intr_priority );

	  WRK.pos_request = FALSE; 
          ioaddr = io_att( (DDI.cc.bus_id | IOCC_SELECT) & 
			NO_ADDR_CHECK_MASK, POS_OFFSET );
  	  BUS_PUTC(ioaddr + POS_2, 0x00 ); /* disable ethernet via POS */
	  io_det( ioaddr );

	  if ( !SALMON() ) {
		if (WRK.channel_allocated == TRUE) {
			rc = d_complete(WRK.dma_channel,DMA_NOHIDE,
					WRK.sysmem, WRK.alloc_size, 
					(struct xmem *)&WRK.xbuf_xd,
					(char *)WRK.dma_base);
			if ( rc ) {
			  ent_logerr(dds_ptr, ERRID_IENT_ERR3, 
				rc, WRK.sysmem, WRK.dma_channel, WRK.dma_base);
			}
		}
	  }
	  i_enable ( ipri );
	  xmfree( WRK.sysmem, pinned_heap );
	  WRK.sysmem = 0;
	  if (WRK.channel_allocated == TRUE) {
		d_clear (WRK.dma_channel);
		WRK.channel_allocated = FALSE;
	  }
	}
   	TRACE1 ("halE");
}

/*
 *
 * NAME: xxx_close 
 *                
 * FUNCTION: Delete multicast entries for this open, if they exits.
 *                                                                
 * EXECUTION ENVIRONMENT:                                        
 *                                                              
 *      This routine runs only under the process thread.       
 *                                                            
 * NOTES:                                                    
 *                                                          
 *    Input: DDS pointer - tells which adapter             
 *           Open pointer - tells which open              
 *                                                       
 *    Output: An updated multicast address table.      
 *                                                    
 *    Called From: cioclose                          
 *                                                  
 *    Calls To:					
 *                                             
 * RETURN:  N/A                               
 *                                           
 */
xxx_close( dds_ptr, open_ptr )
	dds_t	*dds_ptr;
	open_elem_t *open_ptr; 		/*  opener */
{
	ulong    ipri;
	ulong    i, rc; 
	ulong    mc_found;			/* Multicast address found */

   	TRACE1 ("cloS");

	/* verify that the device has been started */
	if (WRK.adapter_state != STARTED)
		return(0);

	/* check for any remaining valid multicast addresses */
	if (WRK.multi_count <= 0)
		return(0);

	ipri = i_disable( DDI.cc.intr_priority );

	/* 
	 *  traverse MC table to match the current address to 
	 *  table and invalidate entry
	 */
	mc_found = FALSE;
	for (i=0; i < MAX_MULTI; i++) {
		if (WRK.multi_open[i] == open_ptr) {
			/* we have a match, invalidate entry */
			WRK.multi_open[i] = NULL;
			WRK.multi_count--;

			/*
			 *  setting flag true indicates
			 *  an MC address setup action command
			 *  to the adapter is required
			 */
			mc_found = TRUE;
		} 
	} 

	i_enable( ipri );			/* enable interrupts */

	if (mc_found == TRUE) {
	  /* issue a MC setup action command to the adapter */ 
	  /* give the updated multicast table to the adapter */
          /* ent_action( dds_ptr, MCS_CMD, &set_multi); */

	} 
   	TRACE1 ("cloE");
	return(0);
} 

/*
 *
 * NAME: ent_action              
 *                                        
 * FUNCTION:  Sets up and issues an action command to the adapter
 *                                                              
 * EXECUTION ENVIRONMENT:                                      
 *      This routine runs only under the process thread.      
 *                                                           
 * NOTES:                                                   
 *                                                         
 *    Input: DDS pointer, action command, optional arg	
 *                                                     
 *    Output: N/A                                     
 *                                                   
 *    Called From: xxx_act                          
 *                                                 
 *    Calls To: d_master, xmalloc, entlogerr, cio_conn_done, bzero    
 *                                                                   
 * RETURN:  0 = successful					
 *         -1 = on error				
 *                                                     
 * NOTE:  action commands are processed one at a time, no chaining	 
 *                                                                      
 */
int
ent_action(
	dds_t	*dds_ptr,		/* DDS pointer */
	ulong	cmd,			/* action cmd to be executed */
	ulong	*arg ) 			/* optional ptr to a structure */
{
	ulong	ipri;			/* save old priority */
	ulong	i, j, k;
	uchar	*net_addr; 		/* pointer to the network address */
	ushort	cmd_value;
	ushort	stat;
	ulong	ioaddr;
	ulong	rc;
	struct	cfg	*cfg; 		/* configure structure */
	struct	cfg	usercfg; 	/* config struct passed from caller */
	struct	ias	*ias; 		/* IAS command structure */
	struct	mc	*mc; 		/* MCS command structure */
	ent_set_multi_t *set_multi;     /* MC address data structure */

   	TRACE2 ("cmdS", cmd);
	ATT_MEMORY();

	ipri = i_disable( DDI.cc.intr_priority );

	switch ( cmd ) {
	  case CFG_CMD: {
	    cfg = (struct cfg *) &WRK.acbl_ptr[0].csc;
	    if ( arg == NULL ) {
		/*
		 *  called by the DD at first start and
		 *  activation, use default values
		 */
		if (SALMON())
		  WRK.cur_cfg.loopback   = CFG_LPBCK_SALMON;
	        else
		  WRK.cur_cfg.loopback    = CFG_LOOPBACK;
		WRK.cur_cfg.save_bf       = CFG_BAD_FRAMES;
		WRK.cur_cfg.fifo_limit    = CFG_FIFO_LIMIT;
		WRK.cur_cfg.byte_count    = CFG_BYTE_COUNT;
		WRK.cur_cfg.slot_time_up  = CFG_SLOT_HIGH;
		WRK.cur_cfg.slot_time_low = CFG_SLOT_LOW;
		WRK.cur_cfg.spacing       = CFG_SPACING;
		WRK.cur_cfg.linear_pri    = CFG_LINEAR;
		WRK.cur_cfg.preamble      = CFG_PREAMBLE;
		WRK.cur_cfg.frame_len     = CFG_FRAME_LEN;
		WRK.cur_cfg.carrier_sense = CFG_CSF;
		WRK.cur_cfg.promiscuous   = CFG_PROMISCUOUS;
		WRK.cur_cfg.dcr_num       = CFG_RESERVED;
		WRK.cur_cfg.dcr_slot      = CFG_RESERVED;
	    }
	    else {
		/* called via an IOCTL operation,
		   use the config structure passed in */
		if (copyin((char *)arg,&WRK.cur_cfg,sizeof(struct cfg)) != 0) {
			i_enable(ipri);
			DET_MEMORY();
			return(EFAULT);
		}
		WRK.cur_cfg.save_bf &= 0x40;
		/* Add some more range-checking here. */

		/* Interrupt the cmd-unit. */
		WRITE_SHORT(WRK.scb_ptr->command, CMD_RUC_ABORT);
	    }
	    /* Write out the new configuration. */
	    WRITE_CHAR(cfg->loopback, WRK.cur_cfg.loopback);
	    WRITE_CHAR(cfg->save_bf, WRK.cur_cfg.save_bf & 0x40);
	    WRITE_CHAR(cfg->fifo_limit, WRK.cur_cfg.fifo_limit);
	    WRITE_CHAR(cfg->byte_count, WRK.cur_cfg.byte_count);
	    WRITE_CHAR(cfg->slot_time_up, WRK.cur_cfg.slot_time_up);
	    WRITE_CHAR(cfg->slot_time_low, WRK.cur_cfg.slot_time_low);
	    WRITE_CHAR(cfg->spacing, WRK.cur_cfg.spacing);
	    WRITE_CHAR(cfg->linear_pri, WRK.cur_cfg.linear_pri);
	    WRITE_CHAR(cfg->preamble, WRK.cur_cfg.preamble);
	    WRITE_CHAR(cfg->frame_len, WRK.cur_cfg.frame_len );
	    WRITE_CHAR(cfg->carrier_sense, WRK.cur_cfg.carrier_sense);
	    WRITE_CHAR(cfg->promiscuous, WRK.cur_cfg.promiscuous);
	    WRITE_CHAR(cfg->dcr_num, WRK.cur_cfg.dcr_num);
	    WRITE_CHAR(cfg->dcr_slot, WRK.cur_cfg.dcr_slot );
		
	    if (arg != NULL) { /* If it essentially is not the 1st time. */
	      WRITE_SHORT(WRK.scb_ptr->command, CMD_RUC_START);
            }
	    WRITE_LONG(WRK.acbl_ptr[0].csc, CB_EL | CB_INT | CFG_CMD);
	    break;
	 }
	case INTERNAL_CFG_CMD: {
	  cfg = (struct cfg *) &WRK.acbl_ptr[0].csc;
	  /* Interrupt the cmd-unit. */
	  WRITE_SHORT(WRK.scb_ptr->command, CMD_RUC_ABORT); 

	  /* Write out the new configuration. */
	  WRITE_CHAR(cfg->loopback, WRK.cur_cfg.loopback); 
	  WRITE_CHAR(cfg->save_bf, WRK.cur_cfg.save_bf & 0x40);
	  WRITE_CHAR(cfg->fifo_limit, WRK.cur_cfg.fifo_limit);
	  WRITE_CHAR(cfg->byte_count, WRK.cur_cfg.byte_count);
	  WRITE_CHAR(cfg->slot_time_up, WRK.cur_cfg.slot_time_up);
	  WRITE_CHAR(cfg->slot_time_low, WRK.cur_cfg.slot_time_low);
	  WRITE_CHAR(cfg->spacing, WRK.cur_cfg.spacing);
	  WRITE_CHAR(cfg->linear_pri, WRK.cur_cfg.linear_pri);
	  WRITE_CHAR(cfg->preamble, WRK.cur_cfg.preamble);
	  WRITE_CHAR(cfg->frame_len, WRK.cur_cfg.frame_len );
	  WRITE_CHAR(cfg->carrier_sense, WRK.cur_cfg.carrier_sense); 
	  WRITE_CHAR(cfg->promiscuous, WRK.cur_cfg.promiscuous);
	  WRITE_CHAR(cfg->dcr_num, WRK.cur_cfg.dcr_num);
	  WRITE_CHAR(cfg->dcr_slot, WRK.cur_cfg.dcr_slot );  
	
	  /* Re-start the cmd-unit. */
	  WRITE_SHORT(WRK.scb_ptr->command, CMD_RUC_START); 

	  /* Do it. */
	  WRITE_LONG(WRK.acbl_ptr[0].csc, CB_EL | CB_INT | CFG_CMD);
	  break;
	}
	case IAS_CMD: {
		/*
		 *  inform the device which HW address to use
		 *  as a source address. 
		 *  will receive packets with this address
		 *  as a destination address as well as 
		 *  other packets. ie. broadcast, MC etc.
		 */
	 	net_addr = (uchar *) arg;
	 	ias = (struct ias *) &(WRK.acbl_ptr[0].csc);
	 	WRITE_LONG(WRK.acbl_ptr[0].csc, CB_EL | CB_INT | IAS_CMD);

		for( i = 0; i <= 5; i++ ) {
			WRITE_CHAR(ias->iaddr[i], net_addr[i]);
		}

		WRK.wdt_setter = WDT_IAS;
		break;
	  }
	case MCS_CMD: {
	 	mc = (struct mc *) &(WRK.acbl_ptr[0].csc);
		WRITE_SHORT_REV(mc->mc_byte[0], ( WRK.multi_count * 6 ) &
				 0x3FFF );
		WRITE_LONG(mc->csc, CB_EL | CB_INT | MCS_CMD);

		for ( i = 0, k = 0; i < MAX_MULTI; i++ ) {
			if ( WRK.multi_open[i] != NULL ) {
				for ( j = 0; j < ent_NADR_LENGTH; j++, k++ ) {
					WRITE_CHAR(mc->mc_byte[k + 2], WRK.multi_list[i][j]);
				}
			}
		}

		break;
	}

	case NOP_CMD: {
	 	WRITE_LONG(WRK.acbl_ptr[0].csc, CB_EL | CB_INT | NOP_CMD);
		break;
	}
        default:
           	break;
	}
	

	/*
	 *  put the bus address of the action CBL into the SCB
	 */
	WRITE_LONG_REV(WRK.scb_ptr->cbl_addr, &WRK.acbl_addr[0]);

	/*
	 *  ensure the CU is not in the acceptance phase
	 */
	COMMAND_QUIESCE();
	
	WRK.action_que_active = TRUE;

	START_CU();			/* start the command unit */
	i_enable( ipri );		/* enable interrupts */

	/*
	 *  start watchdog timer
	 *  who set the timer and the reason for setting WDT 
	 *  were set above
	 */
	/*  w_start ( &(WDT) ); */              /* start the watchdog timer */

	DET_MEMORY();
   	TRACE2 ("cmdE", cmd);
	return( 0 );
}

/*
 *
 * NAME: ent_action_done   
 *                        
 * FUNCTION:  completion processing for all miscellanoues action commands 
 *                                                                       
 * EXECUTION ENVIRONMENT:                                               
 *      This routine runs only under the process thread.               
 *                                                                    
 * NOTES:                                                            
 *                                                                  
 *    Input: Channel Name - Device Dependent Structure		
 *                                                             
 *    Output: N/A                                             
 *                                                           
 *    Called From: xxx_act                                  
 *                                                         
 *    Calls To: d_master, xmalloc, entlogerr, cio_conn_done, bzero 
 *                                                                 
 * RETURN:  0 = successful					
 *         -1 = on error				
 *                                                     
 */

int
ent_action_done( 
	dds_t	*dds_ptr,		/* DDS pointer */
	ushort	stat_value,		/* SCB status field */
	ulong	ioaddr )		/* attached I/O address */
{
	int	rc = 0;
	ulong	value, k;
	ushort	cmd_value;

   	TRACE2 ("cmDS", stat_value);
	/* 
	 *  check for errors
	 */
	READ_LONG(WRK.acbl_ptr[0].csc, &value);
	if ( ! (value & CB_OK))
	{
		ent_logerr(dds_ptr, ERRID_IENT_ERR4,0x1829,value,dds_ptr);
		rc = EFAULT;
	}
	WRK.action_que_active = FALSE;

	/*
	 *  acknowledge the interrupt, first
	 *  ensure that the 82596 has no pending control
	 *  commands in the acceptance phase
	 */
	COMMAND_QUIESCE();

	/*
	 *  set the proper acknowledge bits and issue
	 *  a CA. If a No Resource interrupt was received,
	 *  resume the CU.
	 */
	if ( stat_value & STAT_CNA )
	{
		WRITE_SHORT(WRK.scb_ptr->command, CMD_ACK_CNA | CMD_ACK_CX);
	}
	else
		WRITE_SHORT(WRK.scb_ptr->command, CMD_ACK_CX);

	CHANNEL_ATTENTION();
   	TRACE2 ("cmDE", stat_value);
	return( 0 );
}


/*
 * 
 * NAME: xxx_ioctl
 *               
 * FUNCTION: Assorted device specific I/O controls
 *                                              
 * EXECUTION ENVIRONMENT:                      
 *                                            
 *      This routine runs only under the process thread. 
 *                                                      
 * NOTES:                                              
 *                                                    
 *    Input: DDS pointer - tells which adapter       
 *           open pointer - tells who issued the command     
 *           op - tells which ioctl command is being issued 
 *           argument - address of the user data           
 *           dev flag - tells if user process or kernel process  
 *           extension - not used                               
 *                                                             
 *    Output: 						
 *             Updated multicast addresses             
 *                                                    
 *    Called From: cioioctl                          
 *                                                  
 *    Calls To:  d_master, d_complete, bcopy, copyin, copyout, xmalloc,
 *               xmfree                                               
 *                                                                   
 * RETURN:  0 = OK, EACCES, EINVAL, EFAULT, ENOTREADY, EAFNOSUPPORT, ENOSPC 
 *                                                                         
 */
int 
xxx_ioctl( 
	dds_t		*dds_ptr,
	open_elem_t 	*open_ptr, 		/* issuer of the IOCTL */
	int	op,    				/* ioctl cmd to be performed */
	int	arg,   				/* address user data */
	ulong	devflag,			/* user or kernel process */
	int	ext )
{
	ulong	i, j, k;                    
	ushort	cmd_value;
	ulong  	rc = 0;
	ulong  	ipri;
	ulong  	ioaddr;
	ulong   mc_found; 		/* multicast  address match */
	ulong   pos; 			/* contains POS data */
	ulong   test; 			/* dest. of self test port cmd */
	struct	devinfo	info;		/* IOCINFO data structure */
	struct	selftst	selftst;	/* self_test data structure */
	ent_set_multi_t set_multi;      /* MC address data structure */
	ulong	 status;
	int	saved_intr_level;

   	TRACE2 ("iocS", op);
	switch (op) {
	  case IOCINFO: {

		/* ensure device has been activated */
		if ( WRK.adapter_state != STARTED ) {
			return ((int)ENOTREADY);
		}

		/*  IOCINFO  IOCTL fill in the devinfo structure */
		info.un.ethernet.rdto = WRK.rdto;     /* rcv offset */
		info.devtype    = DD_NET_DH;
		info.flags      = NULL;
		info.devsubtype = DD_EN;

               /* SALMON does not support broadwrap, 
		   340/350 do.  apar ix29594 */
               if ( SALMON() ) {
                    info.un.ethernet.broad_wrap = 0;
               } else {
                    info.un.ethernet.broad_wrap = 1;
               }

		/* return the network addresses - */
		for (i=0; i < ent_NADR_LENGTH; i++) {
		       info.un.ethernet.haddr[i] = WRK.ent_vpd_addr[i];
		       info.un.ethernet.net_addr[i] = WRK.ent_addr[i];
		}

		/* pass this data to the caller */
		if (devflag & DKERNEL) {
		  /* kernel caller */
		  bcopy(&info, arg, sizeof(info));
		}
		else {
		  if (copyout(&info, arg, sizeof(info)) != 0) {
		    return(EFAULT);
		  }
		} 
			
		break;
	  }

	  case CIO_QUERY: {
		if ( WRK.adapter_state != STARTED ) {
			return ((int)ENOTREADY);
		}
		/* Query RAS Counters IOCTL */
		stat_count (dds_ptr);
		
		break;
	  }

	  case ENT_SET_MULTI: {
		/* 
		 *  add/delete an ethernet Multicast address
		 */
		if ( WRK.adapter_state != STARTED ) {
			return ((int)ENOTREADY);
		}

		/*  get the IOCTL data */
		if (devflag & DKERNEL) {
		       bcopy((void *)arg,&set_multi,sizeof(set_multi));
		}
		else {
		  if (copyin((void *)arg, &set_multi,sizeof(set_multi)) != 0) {
		    return (EFAULT);
		  }
		}

		/* check validity of opcode */
		if ((set_multi.opcode != (ushort)ENT_ADD) &&
				(set_multi.opcode != (ushort)ENT_DEL)) {
		  return (EINVAL);
		}

		/* multicast bit must be set */
		if ((set_multi.multi_addr[0] & MULTI_BIT_MASK)
			 != MULTI_BIT_MASK) {
		  return (EAFNOSUPPORT);
		}

		/* 
		 *  valid ranges and opcodes 
		 *  test for add or delete address    
		 *
		 *  if flag is set, action cmd to adapter needed
		 */
		mc_found = FALSE;

		if (set_multi.opcode == (ushort)ENT_ADD) {
		  /* 
		   *  request to add a MC address, ensure the limit
		   *  has not been met
		   */
		  if (WRK.multi_count >= MAX_MULTI) {
		    return (ENOSPC);
		  }

		  /* 
		   *  traverse multicast array to find 
		   *  a home for the address
		   */ 
		  for ( i=0; i < MAX_MULTI; i++ ) {
			if (WRK.multi_open[i] == NULL) { 

			  /* 
			   *  copy the address to the 
			   *  work section table
			   */
			  for ( j = 0; j < ent_NADR_LENGTH; j++ ) {
			     WRK.multi_list[i][j] = set_multi.multi_addr[j];
			  }
						
			  /*
			   *  validate the entry and increment 
			   *  the count of valid entries
			   */
			  WRK.multi_open[i] = open_ptr; 	
			  WRK.multi_count++;

			  /*
			   *  indicate that a MC setup action 
			   *  command is necessary
			   */
			  mc_found = TRUE;	
			  break;
			} 
		    } 
		}
		else { 
		  /* 
		   * request to delete a MC address, find the
		   * address in the table, remove it and issue a MC
		   *  setup action command to notify adapter 
		   */
		   for ( i = 0; i < MAX_MULTI; i++ ) {
		      if (WRK.multi_open[i] == open_ptr) {
			/*  
			 *  valid open pointer, try to match
			 *  multicast addreses
			 */
			for ( j=0; j < ent_NADR_LENGTH; j++ ) {
			   if ( WRK.multi_list[i][j] == 
				   set_multi.multi_addr[j] ) {
			     if (j == (ent_NADR_LENGTH - 1)) {
				/* 
				 *  have a match,
				 *  invalidate entry 
				 */
				WRK.multi_open[i]=NULL;
				WRK.multi_count--;
				mc_found = TRUE;
				break;
			     }
			   }
			   else {
			     /* 
			      *  no match, try next
			      *  open_ptr
			      */
			    break;
			   }
			} 
		    } 
		  } 
		}

		/* check if action command required */
		if ( mc_found ) {
			ent_action( dds_ptr, MCS_CMD, &set_multi);
			return (0);
		}
		else { 
			/* add or delete malfunctioned, return error */
			return ((int)EFAULT);
		} 
		break;    		/* end of multicast setup IOCTL */
	     }

		case ENT_CFG: {

			/* ensure device has been activated */
			if ( WRK.adapter_state != STARTED ) {
				return ((int)ENOTREADY);
			}
			/* issue a CONFIGURE action command to the 82596DX */
			rc = ent_action( dds_ptr, CFG_CMD, arg );
			break;
		}
		case ENT_SELFTEST: {
			/* ensure device has been activated */
			if ( WRK.adapter_state != STARTED ) {
				return ((int)ENOTREADY);
			}

			/* Rainbow does not support selftest, because
			   of the hw-workaround. */
			ioaddr = io_att(  DDI.cc.bus_id, 0 );

        		WRITE_LONG(WRK.scp_ptr->reserved, 0xFFFFFFFF);
		        /* This is a destructive test. */
			port(dds_ptr, ioaddr, (int)WRK.scp_addr | 0x1);

			READ_LONG( WRK.scp_ptr->reserved, &test );
			for( i = 0; i < 100; i++ ) {
				if ( test != 0xFFFFFFFF )
				break;
				READ_LONG( WRK.scp_ptr->reserved, &test );
			}
				
			selftst.result = test;
			READ_LONG( WRK.scp_ptr->sysbus, &selftst.signature );

			if (copyout(&selftst, arg, sizeof(selftst)) != 0) {
				io_det( ioaddr );
				return(EFAULT);
			}

			io_det( ioaddr );
			break;
		}
		case ENT_POS: {

			/* ensure device has not been started */
			if ( WRK.adapter_state == STARTED ) {
				return ((int)EBUSY);
			}
			/* 
			 *  allow user to specify what POS values
			 *  to use. driver must not already be started.
			 *  save these values and at Start time
			 */

			/* get the IOCTL data */
			if (devflag & DKERNEL) {
				/*
				 *  this IOCTL not supported for the kernel
				 */
				return ((int)EINVAL);
			}
			else {
			       if( copyin((void *)arg, &pos, sizeof(int)) != 0)
				{
					return (EFAULT);
				}
			}

			WRK.pos_request = TRUE;
			WRK.pos = pos;
			break;
		}
		case ENT_NOP: {
			/* 
			 *  Issue a NOP action command to the 82596DX
			 */

			rc = ent_action( dds_ptr, NOP_CMD, 0 );
			break;
		}
		case ENT_DUMP: {

			/* ensure device has been activated */
			if ( WRK.adapter_state != STARTED ) {
				return ((int)ENOTREADY);
			}
		
			ipri = i_disable( DDI.cc.intr_priority );
			ioaddr = io_att(  DDI.cc.bus_id, 0 );
			i_mask ((struct intr *)(&(IHS)));
                        WRK.restart = TRUE;
			port(dds_ptr, ioaddr, (int)WRK.scp_addr | 0x3);
			io_det( ioaddr );

			if (WRK.do_ca_on_intr & IS_A_RAINBOW) {
	  		  /* disable the Ethernet adapter (for Rainbow
			    POS2 = 0 is not enough)*/
          		  ATT_MEMORY();
          		  port( dds_ptr, ioaddr, 0x00 );
          		  DET_MEMORY();  
			}

                        WRK.restart = FALSE;

			ioaddr = io_att( (DDI.cc.bus_id | IOCC_SELECT) &
                                        NO_ADDR_CHECK_MASK, POS_OFFSET );

			BUS_PUTC(ioaddr + POS_2, 0x00 );   /* disable device */
			io_det( ioaddr );

			i_enable ( ipri );
			break;
		}

		default:
			return ((int)EINVAL);

	} 

   	TRACE2 ("iocE", *open_ptr);
	return ( 0 );

} 

/*
 *
 * NAME: ent_logerr
 *               
 * FUNCTION: Collect information for making of error log entry.
 *                                                            
 * EXECUTION ENVIRONMENT:                                    
 *                                                          
 *      This routine runs only under the process thread.   
 *                                                        
 * NOTES:                                                
 *                                                      
 *    Input: DDS pointer - tells which adapter         
 *           Error id    - tells which error is being logged  
 *           Error number - tells which error code is returned
 *           CMD - Adapter Command Register value  (if available)
 *           Status - Adapter Status Register value  (if available) 
 *                                                                 
 *    Output: Error log entry made via errsave.                   
 *                                                               
 *    Called From: entxmit, entrecv, xxx_act 	
 *                                                             
 *    Calls To: bcopy, bzero, errsave                         
 *                                                           
 * RETURN:  N/A                                             
 *                                                         
 */

ent_logerr( dds_ptr, errid, errnum, scb, status )
	dds_t	*dds_ptr;
	ulong	errid;
	ulong	errnum;
	ulong	scb;
	ulong	status;
{
	struct	error_log_def	log;
	int     i; 

	bzero( &log, sizeof( struct error_log_def ));	/* zero out log entry */

	/*
	 *  fill the error log entry 
	 */
	log.errhead.error_id = errid; 			/* error id */
	log.errhead.resource_name[0] = DDI.ds.lname[0];	/* DD logical name */
	log.errhead.resource_name[1] = DDI.ds.lname[1];
	log.errhead.resource_name[2] = DDI.ds.lname[2];
	log.errhead.resource_name[3] = DDI.ds.lname[3];
	log.errhead.resource_name[4] = ' ';
	log.errhead.resource_name[5] = ' ';
	log.errhead.resource_name[6] = ' ';
	log.errhead.resource_name[7] = ' ';

	log.errnum = errnum;				/* error return code */
	log.scb = scb;					/* SCB  status field */
	log.status = status;				/* ethernet stat reg */

	/* 
	 * POS register values
	 */
	for (i=0; i<8; i++)
	{
		log.pos_reg[i] = WRK.pos_reg[i];
	}

	/* 
	 * network address value
	 */
	for (i=0; i<ent_NADR_LENGTH; i++)
	{
		log.ent_addr[i] = WRK.ent_addr[i];
		log.ent_vpd_addr[i] = WRK.ent_vpd_addr[i];
	}

	log.type_field_off = DDI.ds.type_field_off;	/* type field offset */
	log.net_id_offset  = DDI.ds.net_id_offset;

	errsave (&log,sizeof(struct error_log_def));	/* log the error */

	return;

} 

/*
 *
 * NAME: xxx_startblk  
 *                     
 * FUNCTION: Build a start done status block.
 *                                         
 * EXECUTION ENVIRONMENT:                 
 *                                       
 *      This routine runs only under the process thread.
 *                                                     
 * NOTES:                                             
 *                                                   
 *    Input: DDS pointer - tells which adapter      
 *           Netid element pointer - tells which netid
 *           Status block pointer - tells where to build status block
 *                                                                  
 *    Output: An updated status block that contains the current    
 *            hardware address					
 *                                                             
 *    Called From: cio_conn_done, cioioctl                    
 *                                                           
 *    Calls To: bcopy                                       
 *                                                         
 * RETURN:  N/A                                           
 *                                                       
 */
void xxx_startblk( dds_ptr, netid_elem_ptr, sta_blk_ptr )
dds_t		*dds_ptr;
netid_elem_t	*netid_elem_ptr;
struct		status_block	*sta_blk_ptr;
{

	sta_blk_ptr->code = (ulong)CIO_START_DONE;
	sta_blk_ptr->option[0] = WRK.connection_result;
	sta_blk_ptr->option[1] = netid_elem_ptr->netid;
	sta_blk_ptr->option[3] = 0x00000000;
	bcopy (&(WRK.ent_addr[0]), &(sta_blk_ptr->option[2]), ent_NADR_LENGTH);
	return;
} 

/*
 * 
 * NAME: xxx_badext
 *                 
 * FUNCTION: Detect invalid filename extension at ddmpx time.
 *                                                          
 * EXECUTION ENVIRONMENT:                                  
 *                                                        
 *      This routine runs only under the process thread. 
 *                                                      
 * NOTES:                                              
 *                                                    
 *    Input: Channel Name - string of characters for the extention.
 *                                                               
 *    Output: N/A                                               
 *                                                             
 *    Called From: ciompx                                     
 *                                                           
 *    Calls To: N/A                                        
 *                                                          
 * RETURN:  0 = Channel extension is ok                   
 *          1 = Channel extension is not an 'E' or 'D' etc.
 *                                                        
 */
int 
xxx_badext ( channame )
	char	*channame;
{
	register int rc;

	/* 
	 *  neither exclusive use in normanl R/W mode or Diagnostic mode 
	 *  is specified
	 */
	rc = ((( channame[0] != 'E' ) && (channame[0] != 'D')) ||     
					 (channame[1] != '\0')); 
	return (rc);
}

/*
 *
 * NAME: reverse_long 
 *                    
 * FUNCTION: byte reverse a 32-bit value
 *                                     
 * EXECUTION ENVIRONMENT:             
 *      This routine runs under the process thread or interrupt level.
 *                                                                   
 * NOTES:                                                           
 *                                                                 
 *    Input: 32-bit word					
 *                                                             
 *    Output: byte reversed word                              
 *                                                           
 */
int
reverse_long( be_addr )
ulong	be_addr;			/* addr in big endian format */
{
	int	le_addr;
	
	le_addr =  be_addr << 24;
	le_addr |= ( be_addr & 0x0000FF00 ) << 8;
	le_addr |= ( be_addr & 0x00FF0000 ) >> 8;
	le_addr |= be_addr >> 24;
	return( le_addr );

}

/*
 * 
 * NAME: reverse_short
 *                   
 * FUNCTION: byte reverse a 16-bit value
 *                                     
 * EXECUTION ENVIRONMENT:             
 *      This routine runs under the process thread or interrupt level. 
 *                                                                    
 * NOTES:                                                            
 *                                                                  
 *    Input: 32-bit word					
 *                                                             
 *    Output: byte reversed word                              
 *                                                           
 */
short
reverse_short( be_addr )
ushort	be_addr;			/* addr in big endian format */
{
	short	le_addr;
	
	le_addr =  be_addr << 8;
	le_addr |= ( be_addr & 0xFF00 ) >> 8;
	return( le_addr );

}

/*               
 * NAME: bump_que 
 *              
 * FUNCTION: increment the indeces of a circular list	
 *                                                     
 * EXECUTION ENVIRONMENT:                             
 *      This routine runs under the process thread or interrupt level.  
 *                                                                     
 * NOTES:                                                             
 *                                                                   
 *    Input: index value and size of list			
 *                                                             
 *    Output: 						
 *                                                     
 */
int
bump_que( index, size )
ulong	index;				/* indice into queue */
ulong	size;				/* # of elements in the queue */
{
	ulong	rc;

	if ( index == ( size - 1 ))
		rc = 0;
	else
		rc = index + 1;

	return( rc );

}

/*
 *
 * NAME: wd_intr
 *
 * FUNCTION: This is the watchdog interrupt handler
 *	
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 *
 * RETURN VALUE DESCRIPTION: 0 on successful completion.
 *
 * NOTES:  On Salmon, this is the registered watchdog timer interrupt
 *	  routine.
 *
 */
void
wdt_intr( wds_ptr )
	struct	watchdog	*wds_ptr;
{
	dds_t	*dds_ptr;
	ulong   delta;
	ulong	rc;
	ulong	status;
	ulong	csc_value;
	ulong	ioaddr;
	ushort	stat_value;

	dds_ptr = (dds_t *) 0; 
	delta = ((ulong) (&(WDT))) - ((ulong) (dds_ptr));
	dds_ptr = (dds_t *) (((ulong) wds_ptr) - delta);
   	TRACE2 ("wdtS", WRK.wdt_setter);

	/* 
	 *  a watch dog timer interrupt has occurred 
	 *  determine where is was set and process it
	 */
	switch (WRK.wdt_setter)
	{

	case WDT_CONNECT:
		/* 
		 *  ensure that the device was never activated
		 *  ie. timer popped before device could complete
		 */
		if ( WRK.adapter_state != STARTED )
		{ 
			WRK.wdt_setter = WDT_INACTIVE;	/* reset reason code */

			/* 
			 *  notify call that the first start did not
			 *  complete successfully, log errors
			 */
			WRK.connection_result = (ulong)CIO_HARD_FAIL;
			cio_conn_done(dds_ptr);

			ent_logerr(dds_ptr ,(ulong)ERRID_IENT_ERR1,
			(ulong)CIO_HARD_FAIL, (uchar)0, (uchar)0);
		}
		break;

	case WDT_XMIT:
		/* 
		 *  watchdog tripped as a result of a
		 *  transmit not completing in due time
		 *  start CU regardless of what state we think its in
		 */
		w_stop (&(WDT));
		WRK.wdt_setter = WDT_INACTIVE;

		ioaddr = io_att(  DDI.cc.bus_id, 0 );
		READ_SHORT(WRK.scb_ptr->status, &stat_value);
		ent_xmit_done( dds_ptr, stat_value | STAT_CUS_TIMEOUT, ioaddr );

		/* 
		 *  log errors
		 */
		ent_logerr(dds_ptr ,ERRID_IENT_ERR5, EFAULT, 0, 0);


		/* 
		 *  if any outstanding transmits exist, the
		 *  watchdog timer must be restarted
		 *  make a flag/count to make determination
		 *
		 */
		if ( WRK.wdt_setter == WDT_INACTIVE )
		{
			if ( WRK.xmits_pending > 0 )
			{
				WRK.wdt_setter = WDT_XMIT; 
	 			w_start (&(WDT)); 
			}
		}
		io_det( ioaddr );
		break;


	case WDT_CLOSE:

		/* 
		 *  watchdog timer result of close
		 */
		WRK.wdt_setter = WDT_INACTIVE;	/* reset reason code */

		/* 
		 *  wake up the sleeping process 
		 */
		e_wakeup(&(WRK.close_event));

		break;

	default:
		/* 	
		 *  watchdog timer popped for an unknown reason
		 *  ignore for now
		 */
		break;
	}
   	TRACE1 ("wdtE");
}


/*****************************************************************************/
/*
 * NAME:     ent_fastwrt_entry
 *
 * FUNCTION: returns the address of the fastwrite entry point to a 
 *           kernel user along with
 *	     the parameters required to access the entry point.
 *
 * EXECUTION ENVIRONMENT: called from the process environment only
 *
 * NOTES:
 *
 * RETURNS:  0 or errno
 *
 */
/**********************************************************************/
int 
ent_fastwrt_entry(
	open_elem_t		*p_open,
	cio_get_fastwrt_t 	*p_arg,
	ulong			devflag)
{
	cio_get_fastwrt_t	fastwrt;	/* holds ent_fastwrt 
						   parameters */

	if (!(devflag & DKERNEL))
		return(EPERM);

	fastwrt.status = CIO_OK;
	fastwrt.fastwrt_fn = fastwrite;
	fastwrt.chan = 0;
	fastwrt.devno = p_open->devno;
	
	bcopy(&fastwrt, p_arg, sizeof(cio_get_fastwrt_t));

	return(0);
}


/*
 *
 * NAME: stat_count  
 *                  
 * FUNCTION:  This routine reads the statistical data/errors kept
 *	      by the device in the SCB 				
 *          					
 *                                             
 * EXECUTION ENVIRONMENT:                     
 *                                           
 *      This routine runs under both the process thread and interrupt thread.
 *                                                                          
 * NOTES:                                                                  
 *                                                                        
 *    Input: DDS pointer - tells which adapter                           
 *                                                                      
 *    Output: Updated RAS counters				
 *                                                             
 *    Called From: xxx_ioctl                                   
 *                                                           
 *                                                          
 * RETURN:  N/A                                            
 *                                                        
 */
stat_count ( dds_ptr )
register	dds_t	*dds_ptr;
{
	ulong	ioaddr;

	ATT_MEMORY();

	READ_LONG_REV(WRK.scb_ptr->crc_errs, &RAS.ds.crc_error);
	READ_LONG_REV(WRK.scb_ptr->align_errs, &RAS.ds.align_error);
	READ_LONG_REV(WRK.scb_ptr->overrun_errs, &RAS.ds.overrun);
	READ_LONG_REV(WRK.scb_ptr->frame_errs, &RAS.ds.too_short);
	READ_LONG_REV(WRK.scb_ptr->resource_errs, &RAS.ds.no_resources);

	DET_MEMORY();

	return;
} 


/*
 *
 * NAME: port
 *            
 * FUNCTION:  This routine issues a port command to the ethernet device      
 *          							            
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                 
 *                                                                       
 *    This routine runs under both the process thread and interrupt thread.
 *                                                                        
 * NOTES:                                                                
 *                                                                      
 *    Input: DDS pointer, port access value                            
 *                                                                    
 *    Output: none  						
 *                                                             
 *    Called From: xxx_act                                    
 *                                                           
 *    must delay for 10 system clocks and five transmit clocks
 *    after a reset or any port access has been issued.	
 *    after that, issue a CA signal to start initialization sequence.
 *                                                                  
 * RETURN:  N/A                                                    
 *                                                                
 */

port(	dds_t	*dds_ptr,
	ulong	ioaddr,
     	int	ivalue)
{
	uchar	value[4];
	ulong	lvalue;

   	TRACE2 ("prtS", ivalue);
	*(int *)&value = ivalue;
	if (SALMON())
	{
		ioaddr = (void *)io_att(DDI.cc.bus_id, NULL);
		lvalue = (value[3] << 24) | (value[2] << 16);
		BUSPUTL( DDI.ds.io_port, lvalue);
		DELAYMS( 10 );
		lvalue = (value[1] << 24) | (value[0] << 16);
		BUSPUTL( DDI.ds.io_port, lvalue);
		DELAYMS( 10 );
		io_det(ioaddr);
	}
	else
	{
		BUSPUTC( PORT_DATA_LOW, value[3]);
		BUSPUTC( PORT_DATA_HIGH, value[2]);
		BUSPUTC( PORT_CONTROL, 0x01);
		DELAYMS( 1 );
		BUSPUTC( PORT_DATA_LOW, value[1]);
		BUSPUTC( PORT_DATA_HIGH, value[0]);
		BUSPUTC( PORT_CONTROL, 0x01);
		DELAYMS( 1 );
	}
}

void
retry_put(
	ulong	ioaddr,
	ulong	size,
	ulong	value)
{
	int	i;
	int	rc;

	for ( rc = 1, i = 0; rc != 0 && i < 10; i++ )
	{
		switch (size)
		{
		case 1:
			rc = BUS_PUTCX((char *)ioaddr, (char)value);
			break;
		case 2:
			rc = BUS_PUTSX((short *)ioaddr, (short)value);
			break;
		case 4:
			rc = BUS_PUTLX((long *)ioaddr, (long)value);
			break;
		}
	}
   	TRACE2 ("prtE", rc);
	if (rc)
 	  fatal_error("retry_put");

}

void
retry_get(
	ulong	ioaddr,
	ulong	size,
	void	*value)
{
	int	i;
	int	rc;

	for ( rc = 1, i = 0; rc != 0 && i < 10; i++ )
	{
		switch (size)
		{
		case 1:
			rc = BUS_GETCX((char *)ioaddr, (char *)value);
			break;
		case 2:
			rc = BUS_GETSX((short *)ioaddr, (short *)value);
			break;
		case 4:
			rc = BUS_GETLX((long *)ioaddr, (long *)value);
			break;
		}
	}
	if (rc)
 	  fatal_error("retry_get");
}

/*
 *
 * NAME: start_ru  
 *                
 * FUNCTION:  This routine starts the ethernet adapter's receive unit.
 *          							     
 *                                                                  
 * EXECUTION ENVIRONMENT:                                          
 *                                                                
 *      This routine runs under both the process thread and interrupt thread.
 *                                                                          
 * NOTES:                                                                  
 *                                                                        
 *    Input: DDS pointer		                                 
 *                                                                      
 *    Output: none  						
 *                                                             
 *    Called From: xxx_act                                    
 *                                                           
 *                                                          
 * RETURN:  N/A                                            
 *                                                        
 */

int
start_ru( dds_ptr, ioaddr )
register	dds_t	*dds_ptr;
ulong			ioaddr;
{
	int	rc;
	int	i = 0;
	int	k = 0;
	ushort	status;
	ushort	cmd_value;
	ulong	ipri;

	RAS.ds.start_recp_cmd++;
	ipri = i_disable( DDI.cc.intr_priority );
        COMMAND_QUIESCE ();
        WRITE_SHORT(WRK.scb_ptr->command, CMD_RUC_START);
        CHANNEL_ATTENTION();
        COMMAND_QUIESCE ();
	READ_SHORT(WRK.scb_ptr->status, &status);
	while ( !( status &  STAT_RUS_READY ))
	{
		WRITE_SHORT(WRK.scb_ptr->command, CMD_RUC_START);
		CHANNEL_ATTENTION();

		if ( i >= 20 )
		{
			break;
		}

		i++;
                COMMAND_QUIESCE ();
		READ_SHORT(WRK.scb_ptr->status, &status);
	}
	i_enable( ipri );
#ifdef DEBUG
        TRACE3 ("sruE", status, i);
#endif
}

int
fatal_error( string )
char    *string;
{
        panic( string );
}


#ifdef	DEBUG
struct trace {
        ulong location;
        ushort command;
        ushort status;
        int    readyq_in;
        int    readyq_out;
        ulong   reserved1;
        ulong   reserved2;
        ulong   reserved3;
        ulong   reserved4;
};

struct trace trace_buf[256];
struct trace *trace_ptr = NULL;

int trace( a, b, c, d, e, f, g )
ulong 	a;
ushort  b;
ushort  c;
int    d;
int    e;
int    f;
int    g;
{

	ulong	ipri;
	ulong	i;

	ipri = i_disable(INTCLASS2);

        if ( trace_ptr == NULL )
        {
                for( i = 0; i < 255; i++ )
		{
                        trace_buf[i].location = 0;
                        trace_buf[i].command = 0;
                        trace_buf[i].status = 0;
                        trace_buf[i].readyq_in = 0;
                        trace_buf[i].readyq_out = 0;
                        trace_buf[i].reserved1 = 0;
                        trace_buf[i].reserved2 = 0;
                        trace_buf[i].reserved3 = 0;
                        trace_buf[i].reserved4 = 0;
                }
                trace_ptr = &trace_buf[0];
        }
        i = trace_ptr - &trace_buf[0];
	trace_buf[i].location = a;
	trace_buf[i].command = b;
	trace_buf[i].status = c;
	trace_buf[i].readyq_in = d;
	trace_buf[i].readyq_out = e;
	trace_buf[i].reserved1 = f;
	trace_buf[i].reserved2 = g;

        if ( i == 255 )
                trace_ptr = &trace_buf[0];
        else
                trace_ptr++;

        i_enable( ipri );
}
#endif /* DEBUG */

void xxx_init ()
{
	return;
}

void xxx_xmit(
   register dds_t *dds_ptr)     /* DDS pointer - tells which adapter.        */
{
}
