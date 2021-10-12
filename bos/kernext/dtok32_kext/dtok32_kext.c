static char sccsid[] = "@(#)49  1.4  src/bos/kernext/dtok32_kext/dtok32_kext.c, tu_mps, bos41J, 9523A_all 6/5/95 11:43:04";
/*****************************************************************************
 * COMPONENT_NAME: (tu_mps)  Wildwood LAN adapter test units
 *
 * FUNCTIONS:  start_up_adapter
 *             mps_setup
 *             mps_recv_setup
 *             mps_tx_setup
 *             mps_recv_undo
 *             mps_tx_undo
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
 *****************************************************************************/


/*** header files ***/
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mdio.h>
#include <sys/sleep.h>
#include <net/spl.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dma.h>
#include <sys/adspace.h>
#include <stddef.h>
#include <sys/errids.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/intr.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/malloc.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/timer.h>
#include <sys/trchkid.h>
#include <sys/user.h>
#include <sys/uio.h>
#include <sys/watchdog.h>

/* local header files */
#include "mps_regs.h"
#include "mpstu_type.h"

#define   BUFPRO   0x01000000
/****************************************************************************
*
* FUNCTION NAME =  start_up_adapter()
*
* DESCRIPTION   =  This function starts the initialization process
*                  for the MPS adapter.
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   non-zero
*
* INVOKED FROM = oncard_test
*
*****************************************************************************/
int start_up_adapter (ADAPTER_STRUCT *adapter_info)
{
  int         rc = 0;
  int         ioa;

  DEBUG_8("Entering start_up_adapter adapter_info = %x\n", adapter_info);
  DEBUG_8("MPSKEX: Adapter card id = %x \n", adapter_info->card_id);

  /**************************
  *  Enable DMA Channel
  **************************/
  if (adapter_info->dma_channel != DMA_FAIL) {
  	d_unmask(adapter_info->dma_channel);
  	adapter_info->channel_alocd = TRUE;
  } else {
	return(D_INIT_ERR);
  }


  /*****************************************
  *  Set up Control Block for Diagnostics
  *****************************************/
  if ((rc = mps_setup(adapter_info))) {
	return(rc);
  }

  /*********************************************************
  *  Get access to the I/O bus to access the I/O registers.
  *********************************************************/
  ioa = (int)BUSIO_ATT(adapter_info->dds.bus_id, adapter_info->dds.bus_io_addr);

  /*********************************************************************
   *  Set up Master Interrupt Status Mask Register to enable Interrupts
   *********************************************************************/
  BUS_PUTSRX((short *)(ioa + MISRM_SUM), MISR_MSK);

  /******************************************
  *  Start Adapter Initialization Sequence
  ******************************************/
  BUS_PUTSRX((short *)(ioa + SISRM), SISR_MSK);

  /********************
  *  Restore I/O bus
  *******************/
  BUSIO_DET(ioa);

  DEBUG_7("Exiting start_up_adapter\n");

  return(0);

} /* end start_up_adapter */


/**************************************************************************** *
*
* FUNCTION NAME =  mps_open_adapter
*
* DESCRIPTION   =  This function issues an open adapter command.
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*              tucb_ptr       - ptr to test unit control block
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR = error code
*
* INVOKED FROM = oncard_test
*
*****************************************************************************/
int mps_open_adapter (ADAPTER_STRUCT *adapter_info, int open_option)
{
  struct {
  	uchar    cmd;
  	uchar    reserved6[6];
  	uchar    option2;
  	ushort   option1;
  	uchar    reserved2[2];
  	ushort   node_address[3];
  	uchar    group_address[6];
  	uchar    func_address[4];
  	ushort   trb_buffer_length;
  	uchar    rcv_channel_options;
  	uchar    number_local_addr;
  	uchar    product_id[18];
  } o_parm;

  int         ioa;
  ushort  *parm = (ushort *)&o_parm;
  ushort  id;
  int     i, j, rc;

  /*********************************************************
  *  Get access to the I/O bus to access the I/O registers.
  *********************************************************/
  ioa = (int)BUSIO_ATT(adapter_info->dds.bus_id, adapter_info->dds.bus_io_addr);

  /********************
  *  Set open options
  ********************/
  bzero(parm, sizeof(o_parm));
  o_parm.cmd = OPEN_ADAPTER;

  switch (open_option) {
	case INT_WRAP_TEST:  
		o_parm.option1 = 0x0008;
		break;

	case EXT_WRAP_TEST:  
		o_parm.option1 = 0x8000;
		break;

	default:             
		o_parm.option1 = 0x00;
		o_parm.option2 = 0x00;
		break;
	} /* end switch */

  /************************
   *  Set up LAP registers
   ************************/
  BUS_PUTSRX((short *)(ioa + LAPE), 0x00);
  BUS_PUTSRX((short *)(ioa + LAPA), adapter_info->srb_address);
  for (i = 0; i < sizeof(o_parm)/2; i++) {
	BUS_PUTSX((short *)(ioa + LAPDInc), *(parm + i));
  } /* end for */

  BUS_PUTC( ioa + LISR_SUM, 0x20);

  /********************
   *  Restore I/O bus 
   *******************/
  BUSIO_DET(ioa);

  return(0);

} /* end mps_open_adapter */


/**************************************************************************** *
*
* FUNCTION NAME =  mps_enable_channel
*
* DESCRIPTION   =  This function enables the adapter channel.
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*              tucb_ptr       - ptr to test unit control block
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR = error code
*
* INVOKED FROM = oncard_test
*
*****************************************************************************/
int mps_enable_channel (ADAPTER_STRUCT *adapter_info)
{
  int         ioa;

  /*********************************************************
  *  Get access to the I/O bus to access the I/O registers.
  *********************************************************/
  ioa = (int)BUSIO_ATT(adapter_info->dds.bus_id, adapter_info->dds.bus_io_addr);

  /**************************
  * Enable the Rx Channel
  **************************/
  BUS_PUTSRX((short *)( ioa + BMCtl_RUM), 0xFBBF);

  /************************************************
  * Give the buffer descriptor address to adapter
  ************************************************/
  BUS_PUTLRX((long *)(ioa + RxBDA_LO),  (long)adapter_info->recv_list[0]);
  BUS_PUTLRX((long *)(ioa + RxLBDA_LO), 
	    (long)adapter_info->recv_list[MAX_RX_LIST-1]);

  /********************
   *  Restore I/O bus
   *******************/
  BUSIO_DET(ioa);

  return(0);

} /* end mps_enable_channel */

/****************************************************************************
*
* FUNCTION NAME =  mps_setup
*
* DESCRIPTION   =  This function sets up the control block needed
*                  for communications.
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*              tucb_ptr       - ptr to test unit control block
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   non-zero
*
* INVOKED FROM = start_up_adapter
*
*****************************************************************************/
int mps_setup (ADAPTER_STRUCT *adapter_info)
{
  int         rc = 0;
  uchar       *tx_list_base;
  
  DEBUG_7("MPSKEX: Entering mps_setup\n");

  /**********************
   * Set up for receive
   **********************/

  /* get Page size chunk of memory */
  adapter_info->r_p_mem_block = xmalloc(PAGESIZE, PGSHIFT, pinned_heap);

  /* see if memory allocation was successful */
  if (adapter_info->r_p_mem_block == NULL) {
	DEBUG_7("Failed to allocate the memory\n");
	return(XMALLOC_ERR);
  }  /* end if no memory available */

  /* set up cross memory descriptor */
  adapter_info->rx_mem_block.aspace_id   = XMEM_INVAL;
  adapter_info->rx_mem_block.subspace_id = NULL;

  rc = xmattach(adapter_info->r_p_mem_block, PAGESIZE,
	    &(adapter_info->rx_mem_block), SYS_ADSPACE);

  if (rc == XMEM_FAIL ) {
	DEBUG_7("Failed to set up cross memory descriptor\n");
	xmfree(adapter_info->r_p_mem_block, pinned_heap);
	return(XMATTACH_ERR);
  }

  /*  set up DMA for the adapter control area via d_master */
  d_master(adapter_info->dma_channel, DMA_READ | DMA_NOHIDE,
	    	adapter_info->r_p_mem_block, PAGESIZE,
		&(adapter_info->rx_mem_block), 
		(uchar *)adapter_info->dds.dma_bus_mem);

  vm_cflush(adapter_info->r_p_mem_block, PAGESIZE);

  if ((rc = mps_recv_setup(adapter_info))) {
	/* Free up any resources that were allocated */
  	d_complete(adapter_info->dma_channel, DMA_READ | DMA_NOHIDE,
            adapter_info->r_p_mem_block, PAGESIZE,
            &(adapter_info->rx_mem_block),
            (uchar *)adapter_info->dds.dma_bus_mem);
	xmdetach(&(adapter_info->rx_mem_block));
	xmfree(adapter_info->r_p_mem_block, pinned_heap);
	return(rc);
  }

  /**********************
    * Set up for transmit
    **********************/

  /* get Page size chunk of memory */
  adapter_info->t_p_mem_block = xmalloc(PAGESIZE, PGSHIFT, pinned_heap);

  /* see if memory allocation was successful */
  if (adapter_info->t_p_mem_block == NULL) {
	/* Free up any resources that were allocated */
	mps_recv_undo (adapter_info);

	DEBUG_7("Failed to allocate the memory\n");
	return(XMALLOC_ERR);
}  /* end if no memory available */

  /* set up cross memory descriptor */
  adapter_info->tx_mem_block.aspace_id   = XMEM_INVAL;
  adapter_info->tx_mem_block.subspace_id = NULL;

  rc = xmattach(adapter_info->t_p_mem_block, PAGESIZE,
	    &(adapter_info->tx_mem_block), SYS_ADSPACE);

  if (rc == XMEM_FAIL ) {
	/* Free up any resources that were allocated */
	mps_recv_undo (adapter_info);
	xmfree(adapter_info->t_p_mem_block, pinned_heap);

	DEBUG_7("Failed to set up cross memory descriptor\n");
	return(XMATTACH_ERR);
  }

  /*  set up DMA for the adapter control area via d_master */
  tx_list_base = (uchar *)(adapter_info->dds.dma_bus_mem + PAGESIZE);

  d_master(adapter_info->dma_channel, DMA_READ | DMA_NOHIDE,
	    adapter_info->t_p_mem_block, PAGESIZE,
	    &(adapter_info->tx_mem_block), tx_list_base);

  vm_cflush(adapter_info->t_p_mem_block, PAGESIZE);

  if ((rc = mps_tx_setup(adapter_info))) {
	/* Free up any resources that were allocated */
	mps_recv_undo(adapter_info);

	d_complete(adapter_info->dma_channel, DMA_READ | DMA_NOHIDE,
		    adapter_info->t_p_mem_block, PAGESIZE,
		    &(adapter_info->tx_mem_block), tx_list_base);
	xmdetach(&(adapter_info->tx_mem_block));
	xmfree(adapter_info->t_p_mem_block, pinned_heap);

	return(rc);
  }

  DEBUG_7("MPSKEX: Exiting mps_setup\n");

  return(0);

} /* end mps_setup */

/****************************************************************************
*
* FUNCTION NAME =  recv_setup
*
* DESCRIPTION   =  This function sets up the TCW's for receive and
*                  initializes the receive list buffer descriptor indexes.
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   non-zero
*
* INVOKED FROM = mps_setup
*
*****************************************************************************/
int recv_setup (ADAPTER_STRUCT *adapter_info)
{
  volatile rx_list_t  recvlist;
  int                 i, x;
  int                 rc = 0;

  rc = d_complete (adapter_info->dma_channel, DMA_READ,
	    MTOD(adapter_info->recv_mbuf[0], uchar *),
	    adapter_info->recv_mbuf[0]->m_len,
	    &adapter_info->rx_xmemp[0],
	    adapter_info->recv_addr[0]);

  /* set up cross memory descriptor */
  adapter_info->rx_xmemp[0].aspace_id  = XMEM_GLOBAL;
  adapter_info->rx_xmemp[0].subspace_id = NULL;

  /* set up the DMA channel for block mode DMA transfer */
  d_master(adapter_info->dma_channel, DMA_READ | DMA_NOHIDE,
      MTOD(adapter_info->recv_mbuf[0], uchar *),
      adapter_info->recv_mbuf[0]->m_len,
      &adapter_info->rx_xmemp[0],
      adapter_info->recv_addr[0]);

  for (i = 0; i < MAX_RX_LIST; i++) {
  	/* set up buffer table entry */
  	recvlist.fw_pointer   = SWAP32((ulong)adapter_info->recv_list
							[(i+1) % MAX_RX_LIST]);
  	recvlist.recv_status  = 0;
  	recvlist.data_pointer = SWAP32((ulong)adapter_info->recv_addr [i]);
  	recvlist.data_len     = SWAP16((ushort)adapter_info->recv_mbuf
							[i]->m_len);
	recvlist.fr_len       = 0;

	/************************************************************/
	/*  Update the receive dma image of the list by d_moving it */
	/*  through the IOCC cache into system memory.     ?????    */
	/************************************************************/

	rc = d_kmove(&recvlist, adapter_info->recv_list[i], 
			(uint)RX_LIST_SIZE, adapter_info->dma_channel, 
			adapter_info->dds.bus_id, DMA_WRITE_ONLY);

	/* IOCC is NOT buffered */
	if (rc != DMA_SUCC) {
		DEBUG_8("IOCC is NOT buffered(recv_setup) : %d\n", rc);
		bcopy(&recvlist, adapter_info->recv_vadr[i],(uint)RX_LIST_SIZE);
	}
  }

  return(0);

} /* end recv_setup */


/****************************************************************************
*
* FUNCTION NAME =  mps_recv_setup
*
* DESCRIPTION   =  This function sets up the TCW's for receive and
*                  initializes the receive list buffer descriptor indexes.
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   non-zero
*
* INVOKED FROM = mps_setup
*
*****************************************************************************/
int mps_recv_setup (ADAPTER_STRUCT *adapter_info)
{
  volatile rx_list_t  recvlist;
  struct   mbuf       *mbufp;
  uchar               *data_p;
  uchar               *addr_p;
  caddr_t             busmem;
  int                 i, ioa;
  int                 rc = 0;
  uint                Lo_Addr, Hi_Addr, Addr;

  DEBUG_7("Entering mps_recv_setup\n");

  /**************************************************************************/
  /*  Create the receive chain by initializing the pointer to each receive  */
  /*  list.  Create both DMA and virtual address lists.                     */
  /**************************************************************************/
  adapter_info->recv_list[0]=(rx_list_t *)((int)adapter_info->dds.dma_bus_mem);
  adapter_info->recv_vadr[0]=(rx_list_t *)((int)adapter_info->r_p_mem_block);

  for (i = 1; i < MAX_RX_LIST; i++) {
	adapter_info->recv_list[i] = adapter_info->recv_list[i - 1] + 1;
	adapter_info->recv_vadr[i] = adapter_info->recv_vadr[i - 1] + 1;
  }

  addr_p = (uchar *)((int)adapter_info->dds.dma_bus_mem + (2 * PAGESIZE));

  /* Initialize the dma & mbuf pointers arrays  */
  for (i = 0; i < MAX_RX_LIST; i++) {
	if ((mbufp = m_getclust(M_DONTWAIT, MT_DATA)) == NULL) {
		DEBUG_7("No mbuf for mps_recv_setup\n");
		return (MBUF_ERR);
	}

	/* set up cross memory descriptor */
	adapter_info->rx_xmemp[i].aspace_id  = XMEM_GLOBAL;
	adapter_info->rx_xmemp[i].subspace_id = NULL;

	adapter_info->recv_mbuf[i]        = mbufp;        /* Virt addr  */
	adapter_info->recv_mbuf[i]->m_len = PAGESIZE;
	adapter_info->recv_addr[i]        = addr_p;       /* DMA  addr  */

	/* set up the DMA channel for block mode DMA transfer */
	d_master(adapter_info->dma_channel, DMA_READ | DMA_NOHIDE,
		    mtod(adapter_info->recv_mbuf[i], uchar *), 
		    adapter_info->recv_mbuf[i]->m_len,
		    &adapter_info->rx_xmemp[i], adapter_info->recv_addr[i]);

	/* set up buffer table entry */
	recvlist.fw_pointer   = SWAP32((ulong)adapter_info->recv_list[i]);
	recvlist.recv_status  = 0;
	recvlist.data_pointer = SWAP32((ulong)addr_p);
	recvlist.data_len     = SWAP16((ushort)mbufp->m_len);
	recvlist.fr_len       = 0;

	/************************************************************/
	/*  Update the receive dma image of the list by d_moving it */
	/*  through the IOCC cache into system memory.     ?????    */
	/************************************************************/

	rc = d_kmove(&recvlist, adapter_info->recv_list[i], (uint)RX_LIST_SIZE,
		    adapter_info->dma_channel, adapter_info->dds.bus_id, 
			DMA_WRITE_ONLY);

	/* IOCC is NOT buffered */
	if (rc != DMA_SUCC) {
		DEBUG_8("IOCC is NOT buffered(mps_recv_setup) : %d\n", rc);
		bcopy(&recvlist,adapter_info->recv_vadr[i],(uint)RX_LIST_SIZE);
	}

	addr_p += (int)CLBYTES;
	DEBUG_9("recv_vadr[%d] = %x\n", i, adapter_info->recv_vadr[i]);
	DEBUG_9("recv_list[%d] = %x\n", i, adapter_info->recv_list[i]);
	DEBUG_9("recv_mbuf[%d] = %x\n", i, adapter_info->recv_mbuf[i]);
  } /* end of for */


  /*********************************************************
   *  Get access to the I/O bus to access the I/O registers.
   *********************************************************/
  ioa = (int)BUSIO_ATT(adapter_info->dds.bus_id, adapter_info->dds.bus_io_addr);

  /************************************************
   * Give the buffer descriptor address to adapter
   ************************************************/
  Addr    = (int)adapter_info->recv_list[0];
  Hi_Addr = (Addr >> 16);
  Lo_Addr = (Addr & 0xffff);

  BUS_PUTSRX((short *)(ioa + RxBDA_LO),  Lo_Addr);
  BUS_PUTSRX((short *)(ioa + RxBDA_HI),  Hi_Addr);

  /********************
   *  Restore I/O bus
   *******************/
  BUSIO_DET(ioa);

  DEBUG_7("Exiting mps_recv_setup\n");

  return(0);

} /* end mps_recv_setup */


/****************************************************************************
*
* FUNCTION NAME =  mps_recv_undo
*
* DESCRIPTION   =  This function undoes the effects of mps_recv_setup.
*                  Frees the mbufs allocated & frees the region.
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   non-zero
*
* INVOKED FROM = mps_setup
*
*****************************************************************************/
int mps_recv_undo (ADAPTER_STRUCT *adapter_info)
{
  struct mbuf            *m;
  volatile rx_list_t     recvlist;
  register int           x,i;
  int                    rc = 0;

  DEBUG_8("Entering mps_recv_undo adapter_info = %x\n", adapter_info);

  /***********************************************************************
   *  Update the receive dma image of the list by d_moving it through the
   *  IOCC cache into system memory.
   ***********************************************************************/
  adapter_info->read_index = 0;
  bzero(&recvlist, RX_LIST_SIZE);           /* clear receive list   */

  for (i = 0; i < MAX_RX_LIST; i++)
  {
	DEBUG_9("recv_vadr[%d] = %x**\n", i, adapter_info->recv_vadr[i]);
	DEBUG_9("recv_list[%d] = %x**\n", i, adapter_info->recv_list[i]);
	DEBUG_9("recv_mbuf[%d] = %x**\n", i, adapter_info->recv_mbuf[i]);
	DEBUG_8("dma_channel = %x **\n", adapter_info->dma_channel);

	rc = d_kmove (&recvlist, adapter_info->recv_list[i], (uint)RX_LIST_SIZE,
		    adapter_info->dma_channel, adapter_info->dds.bus_id, 
		    DMA_WRITE_ONLY);

	/* if IOCC is NOT buffered */
	if (rc != DMA_SUCC) {
		DEBUG_8("IOCC is NOT buffered (mps_recv_undo) : %d\n", rc);
		bcopy(&recvlist, adapter_info->recv_vadr[i],(uint)RX_LIST_SIZE);
	}

	/*****************************************************
         *  If an mbuf is set up for this receive list, call
         *  d_complete to "un_dma" it, then return it.
         *****************************************************/
	if (m = adapter_info->recv_mbuf[i]) {
		rc = d_complete (adapter_info->dma_channel, DMA_READ, 
				 MTOD(m, uchar *), m->m_len, 
				 &adapter_info->rx_xmemp[i],
			         adapter_info->recv_addr[i]);

		if (rc != DMA_SUCC) {
			DEBUG_8("Error in d_complete(mps_recv_undo)rc=%d\n",rc);
			return(DMAMASK1+rc);
		}

		m_freem(m);
		adapter_info->recv_mbuf[i] = NULL;
	}
  } /* end of for */

  DEBUG_8("adapter_info->r_p_mem_block = %x\n", adapter_info->r_p_mem_block);
  rc = d_complete(adapter_info->dma_channel, DMA_READ | DMA_NOHIDE,
	    adapter_info->r_p_mem_block, PAGESIZE,
	    &(adapter_info->rx_mem_block), 
	    (uchar *)adapter_info->dds.dma_bus_mem);

  if (rc != DMA_SUCC) {
	return(DMAMASK2+rc);
	DEBUG_8("Error in d_complete (mps_recv_undo)) rc = %d\n",rc);
	}

  /*****************************
    * undo adapter control area
    *****************************/
  DEBUG_8("adapter_info->rx_mem_block = %x\n", adapter_info->rx_mem_block);
  rc = xmdetach(&(adapter_info->rx_mem_block));
  rc = xmfree(adapter_info->r_p_mem_block, pinned_heap);

  DEBUG_7("Exiting mps_recv_undo\n");

  return(0);

} /* end mps_recv_undo */



/****************************************************************************
*
* FUNCTION NAME =  mps_recv
*
* DESCRIPTION   =  This function reads completed mbufs.
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   non-zero
*
* INVOKED FROM = process_misr
*
*****************************************************************************/
int mps_recv(ADAPTER_STRUCT *adapter_info)
{
  volatile rx_list_t  recvlist;
  struct   mbuf       *m;
  int                 i, rc = 0, len;

  DEBUG_7("Entering mps_recv\n");

  rc = d_kmove (&recvlist, adapter_info->recv_list[0], (uint)RX_LIST_SIZE,
	    adapter_info->dma_channel, adapter_info->dds.bus_id, DMA_READ);

  /* if IOCC is NOT buffered */
  if (rc != DMA_SUCC) {
	DEBUG_8("IOCC is NOT buffered (mps_recv) : %d\n", rc);
	bcopy(adapter_info->recv_vadr[0], &recvlist, (uint)RX_LIST_SIZE);
  }

  recvlist.recv_status = SWAP32(recvlist.recv_status);
  recvlist.fr_len      = SWAP16(recvlist.fr_len);

  if (!(recvlist.recv_status & BUFPRO)) {
	return(0xD03);
  }

  /*******************
    * Get receive data
    *******************/
  m = adapter_info->recv_mbuf[0];
  len = SWAP16(recvlist.data_len) - 20;

  if ((len < 0) | (len > MAX_BUF_LEN)) {
	return(0xD04);
  }

  /*
   * Copys data from d_master'ed mbuf 
   */
  bcopy(mtod(m, caddr_t) + 16, adapter_info->receive_string, len);
  adapter_info->receive_string[len] = '\0';
  cache_inval(mtod(m, caddr_t), len);

  DEBUG_7("\nExiting mps_recv\n");

  rc = strcmp(adapter_info->transmit_string, adapter_info->receive_string);
  if (rc) {
	return(0xD05);
  }

  return(rc);

} /* end mps_recv */

/****************************************************************************
*
* FUNCTION NAME =  mps_tx_setup
*
* DESCRIPTION   =  This function sets up the TCW's for transmit.
*                  initializes the transmit list variables.
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   non-zero
*
* INVOKED FROM = mps_setup
*
*****************************************************************************/
int mps_tx_setup (ADAPTER_STRUCT *adapter_info)
{
  volatile tx_list_t  xmitlist;
  uchar               *addr_p;
  int                 i, j, ioa;
  int                 rc = 0;
  uint                Lo_Addr, Hi_Addr, Addr;

  DEBUG_7("Entering mps_tx_setup\n");

  /***************************************************************************
   *  Create the transmit chain by initializing the pointer to each transmit
   *  list.  Create both DMA and virtual address lists.
   ***************************************************************************/
  adapter_info->xmit_list[0] = (tx_list_t *)
			(adapter_info->dds.dma_bus_mem + PAGESIZE);
  adapter_info->xmit_vadr[0] = (tx_list_t *)(adapter_info->t_p_mem_block);

  for (i = 1; i < MAX_TX_LIST; i++) {
	adapter_info->xmit_list[i] = adapter_info->xmit_list[i - 1] + 1;
	adapter_info->xmit_vadr[i] = adapter_info->xmit_vadr[i - 1] + 1;
  } /* end of for */

  adapter_info->tx_buf_size = 5 * CLBYTES;

  addr_p =(uchar *)( adapter_info->dds.dma_bus_mem + 
					((MAX_RX_LIST + 2) * PAGESIZE));

  for (i = 0; i < MAX_TX_LIST; i++) {
	adapter_info->xmit_mbuf[i] = xmalloc(adapter_info->tx_buf_size,
		    PGSHIFT, pinned_heap);

	/* set up cross memory descriptor */
	adapter_info->tx_xmemp[i].aspace_id   = XMEM_INVAL;
	adapter_info->tx_xmemp[i].subspace_id = NULL;

	rc = xmattach(adapter_info->xmit_mbuf[i], adapter_info->tx_buf_size,
		    &(adapter_info->tx_xmemp[i]), SYS_ADSPACE);

	if (rc == XMEM_FAIL) {
		DEBUG_7("Failed to set memory cross descriptor\n");

		for (j = 0; j < i; j++) {
			rc = d_complete(adapter_info->dma_channel, 
					DMA_WRITE_ONLY,
					adapter_info->xmit_mbuf[j],
					adapter_info->tx_buf_size,
					&adapter_info->tx_mem_block,
					adapter_info->xmit_addr[j]);

			xmdetach(&(adapter_info->tx_xmemp[i]));
			xmfree(adapter_info->xmit_mbuf[j], pinned_heap);

			adapter_info->xmit_mbuf[j] = NULL;
			adapter_info->xmit_addr[j] = NULL;
		}
		return(ENOBUFS);
	}

	adapter_info->xmit_addr[i] = addr_p + (adapter_info->tx_buf_size * i);

	/* set up the DMA channel for block mode DMA transfer                */
	d_master(adapter_info->dma_channel, DMA_WRITE_ONLY,
		    adapter_info->xmit_mbuf[i], adapter_info->tx_buf_size,
		    &adapter_info->tx_xmemp[i], adapter_info->xmit_addr[i]);

  } /* end of for */

  for (i = 0; i < MAX_TX_LIST; i++) {
	/* set up buffer table entry  */
	xmitlist.fw_pointer   = SWAP32((ulong)adapter_info->xmit_list[(i + 1) 
					% MAX_TX_LIST]);
	xmitlist.xmit_status  = 0;
	xmitlist.buf_count    = 0x0100;
	xmitlist.frame_len    = SWAP16((ushort)TX_LIST_SIZE);
	xmitlist.data_pointer = SWAP32((ulong)adapter_info->xmit_addr[i]); 
	xmitlist.buf_len      = 0;

	/* move the TX list element into the adapter control area. */
	rc = d_kmove(&xmitlist, adapter_info->xmit_list[i], (uint)TX_LIST_SIZE,
		     adapter_info->dma_channel, adapter_info->dds.bus_id, 
		     DMA_WRITE_ONLY);

	/* IOCC is NOT buffered */
	if (rc != DMA_SUCC) {
		DEBUG_8("IOCC is NOT buffered(xxx_xmit) : %d\n", rc);
		bcopy(&xmitlist, adapter_info->xmit_vadr[i],(uint)TX_LIST_SIZE);
	}

  } /* end of for */

  /* Set up variable for Transmit list */
  adapter_info->tx_list_next_buf = 0;
  adapter_info->tx_list_next_out = 0;
  adapter_info->tx_list_next_in  = 0;
  adapter_info->tx_tcw_use_count = 0;
  adapter_info->xmits_queued     = 0;


  /*********************************************************
   *  Get access to the I/O bus to access the I/O registers.
   *********************************************************/
  ioa = (int)BUSIO_ATT(adapter_info->dds.bus_id, adapter_info->dds.bus_io_addr);

  /************************************************
   * Give the buffer descriptor address to adapter
   ************************************************/
  Addr    = (int)adapter_info->xmit_list[0];
  Hi_Addr = (Addr >> 16);
  Lo_Addr = (Addr & 0xffff);

  BUS_PUTSRX((short *)(ioa + Tx2LFDA_LO),  Lo_Addr);
  BUS_PUTSRX((short *)(ioa + Tx2LFDA_HI),  Hi_Addr);

  /**********************************
   *    Enable Transmit Channels    *
   **********************************/
  BUS_PUTSRX((short *)(ioa + BMCtl_SUM), 0x2220);

  /********************
   *  Restore I/O bus
   *******************/
  BUSIO_DET(ioa);
  
  DEBUG_7("Exiting mps_tx_setup\n");

  return(0);

}  /* end mps_tx_setup  */

/****************************************************************************
*
* FUNCTION NAME =  mps_tx_undo
*
* DESCRIPTION   =  This function undoes the effects of mps_tx_setup.
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   non-zero
*
* INVOKED FROM = mps_setup
*
*****************************************************************************/
int mps_tx_undo (ADAPTER_STRUCT *adapter_info)
{
  int         i, rc = 0;

  DEBUG_7("Entering mps_tx_undo\n");

  for (i = 0; i < MAX_TX_LIST; i++) {
	rc = d_complete(adapter_info->dma_channel, DMA_WRITE_ONLY,
		    adapter_info->xmit_mbuf[i], adapter_info->tx_buf_size,
		    &adapter_info->tx_mem_block, adapter_info->xmit_addr[i]);

	if (rc != DMA_SUCC) {
		DEBUG_8("Error in d_complete (mps_tx_undo) rc = %d\n", rc);
		return(DMAMASK3+rc);
	}

	xmdetach(&(adapter_info->tx_xmemp[i]));
	rc = xmfree(adapter_info->xmit_mbuf[i], pinned_heap);

	adapter_info->xmit_mbuf[i] = NULL;
	adapter_info->xmit_addr[i] = NULL;
  }

  rc = d_complete(adapter_info->dma_channel, DMA_READ | DMA_NOHIDE,
	    adapter_info->t_p_mem_block, PAGESIZE,
	    &(adapter_info->tx_mem_block), 
	    (uchar *)(adapter_info->dds.dma_bus_mem + PAGESIZE));

  /* undo adapter control area */
  xmdetach(&(adapter_info->tx_mem_block));
  rc = xmfree(adapter_info->t_p_mem_block, pinned_heap);

  /* Set up variable for Transmit list */
  adapter_info->tx_list_next_buf = 0;
  adapter_info->tx_list_next_out = 0;
  adapter_info->tx_list_next_in  = 0;
  adapter_info->tx_tcw_use_count = 0;
  adapter_info->xmits_queued     = 0;

  DEBUG_7("Exiting mps_tx_undo\n");

  return(0);

}  /* end mps_tx_undo */


/****************************************************************************
*
* FUNCTION NAME =  mps_fastwrt
*
* DESCRIPTION   =  Fast write function for kernel.
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   non-zero
*
* INVOKED FROM =
*
*****************************************************************************/
int mps_fastwrt (ADAPTER_STRUCT *adapter_info)
{
  volatile tx_list_t  xmitlist;
  register int        i, x;
  ulong               cdata;
  uint                Lo_Addr, Hi_Addr, Addr;
  ushort              wrap_len;
  int                 ioa, rc = 0;
  ushort              tr_header[8];

  DEBUG_7("Entering mps_fastwrt\n");

  /*********************************************************
   *  Get access to the I/O bus to access the I/O registers.
   *********************************************************/
  ioa = (int)BUSIO_ATT(adapter_info->dds.bus_id, adapter_info->dds.bus_io_addr);

  /**************************
   *  Build Transmit Header
   **************************/
  tr_header[0] = 0x1040;

  for (i = 1; i < 4; i++) {
	tr_header[i] = adapter_info->dest_address[i-1];
	tr_header[i+3] = adapter_info->source_address[i-1];
  }

  tr_header[7] = 0xaa00;

  /************************************
    *  Copy header into transmit buffer
    ************************************/
  DEBUG_7("Copying header data into buffer\n");
  bcopy(tr_header, adapter_info->xmit_mbuf[0], 16);

  /******************************************
    *  Copy test string into transmit buffer
    ******************************************/
  DEBUG_7("Copying test string into buffer\n");
  wrap_len = strlen(adapter_info->transmit_string);
  
  bcopy(adapter_info->transmit_string, adapter_info->xmit_mbuf[0]+16, wrap_len);
  DEBUG_8("Transmit len = %d : ", wrap_len);
  DEBUG_8("%s\n", adapter_info->transmit_string);

  wrap_len += 16;
  vm_cflush(adapter_info->xmit_mbuf[0], wrap_len);

  /* set up buffer table entry  */
  xmitlist.fw_pointer   = 0;
  xmitlist.xmit_status  = 0;
  xmitlist.buf_count    = 0x0100;
  xmitlist.frame_len    = SWAP16(wrap_len);
  xmitlist.data_pointer = SWAP32((ulong)adapter_info->xmit_addr[0]);
  xmitlist.buf_len      = SWAP16(wrap_len);

  /* move the TX list element into the adapter control area. */
  DEBUG_7("Moving list element into control area\n");
  rc = d_kmove(&xmitlist.xmit_status, &adapter_info->xmit_list[0]->xmit_status,
	        0x10, adapter_info->dma_channel, adapter_info->dds.bus_id, 
		DMA_WRITE_ONLY);

  /* IOCC is NOT buffered */
  if (rc != DMA_SUCC) {
	DEBUG_8("IOCC is NOT buffered(mps_fastwrt) : %d\n", rc);
	bcopy(&xmitlist, adapter_info->xmit_vadr[0], (uint)TX_LIST_SIZE);
  }

  /************************************************
   * Give the buffer descriptor address to adapter
   ************************************************/
  Addr    = (uint)adapter_info->xmit_list[0];
  Hi_Addr = (Addr >> 16);
  Lo_Addr = (Addr & 0xffff);

  BUS_PUTSRX((short *)(ioa + Tx2LFDA_LO),  Lo_Addr);
  BUS_PUTSRX((short *)(ioa + Tx2LFDA_HI),  Hi_Addr);

  /********************
   *  Restore I/O bus
   *******************/
  BUSIO_DET(ioa);
  
  DEBUG_7("Exiting mps_fastwrt\n");

  return(0);

}  /* end mps_fastwrt */


/****************************************************************************
*
* FUNCTION NAME =  mps_tx_done
*
* DESCRIPTION   =  process a completed transmission
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   non-zero
*
* INVOKED FROM =
*
*****************************************************************************/
int mps_tx_done (ADAPTER_STRUCT *adapter_info)
{
  volatile tx_list_t  xmitlist;
  ulong               status;
  int                 rc = 0;

  /* move the TX list element into the adapter control area. */
  rc = d_kmove(&xmitlist, adapter_info->xmit_list[0], (uint)TX_LIST_SIZE,
	    adapter_info->dma_channel, adapter_info->dds.bus_id, DMA_READ);

  /* IOCC is NOT buffered */
  if (rc != DMA_SUCC) {
	DEBUG_8("IOCC is NOT buffered(mps_tx_done) : %d\n", rc);
	bcopy(adapter_info->xmit_vadr[0], &xmitlist, TX_LIST_SIZE);
  }

  rc = 0;
  status = SWAP32(xmitlist.xmit_status);

  if (status & 0x78037E00) {
	if (status & 0x00020000) {
		DEBUG_8("mps_tx UNDERRUN. Status = %x\n", status);
		rc = 0xD06;	
	} else if (status & 0x00007E00) {
		DEBUG_8("Tx2 protocol error. Status = %x\n", status);
		rc = 0xD07;	
	} else {
		rc = d_complete(adapter_info->dma_channel, DMA_WRITE_ONLY,
			    adapter_info->xmit_mbuf[0], 
			    adapter_info->tx_buf_size,
			    &adapter_info->tx_mem_block, 
			    adapter_info->xmit_addr[0]);

		if (rc != DMA_SUCC) {
			DEBUG_8("Error in d_complete(mps_tx_undo) rc=%d\n", rc);
			rc = 0xD08;	
		}
	}
  }

  return(rc);

} /* mps_tx_done */


/****************************************************************************
*
* FUNCTION NAME =  mps_clean_up()
*
* DESCRIPTION   =  This function calls the routines to free up the
*                  resources allocated to initialize the adapter.
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   non-zero
*
* INVOKED FROM = oncard_test
*
*****************************************************************************/
int mps_clean_up(ADAPTER_STRUCT *adapter_info)
{
  int rc1 = 0, rc2 = 0, ioa;

  DEBUG_8("Entering clean up adapter_info = %x\n", adapter_info);

  /*********************************************************
  *  Get access to the I/O bus to access the I/O registers.
  *********************************************************/
  ioa = (int)BUSIO_ATT(adapter_info->dds.bus_id, adapter_info->dds.bus_io_addr);

  /*********************************************************************
   * Soft reset the adapter to force a known state
   *********************************************************************/
  BUS_PUTSRX((short *)(ioa + BCtl), 0x8000);
  io_delay(10);
  BUS_PUTSRX((short *)(ioa + BCtl), 0x0000);

  /********************
  *  Restore I/O bus
  *******************/
  BUSIO_DET(ioa);

  /***************************************************
   *  Clean up resources allocated for initialization
   ***************************************************/
  rc1 = mps_recv_undo(adapter_info);

  rc2 = mps_tx_undo(adapter_info);

  /**********************************
  *  Free up allocated DMA channel
  *********************************/
  d_mask(adapter_info->dma_channel);
  adapter_info->channel_alocd = FALSE;

  /***********************************************
   *  Return non-zero return code, if one exists.
   ***********************************************/

  DEBUG_7("Exiting clean up\n");

  if(rc1) {
  	return(rc1);
  } else {
  	return(rc2);
  }

} /* end mps_clean_up */


