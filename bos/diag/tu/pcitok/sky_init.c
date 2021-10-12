/*
 *   COMPONENT_NAME: tu_pcitok
 *
 *   FUNCTIONS: get_adapter_address
 *		get_diagnostic_codes
 *		initialize_adapter
 *		recv_setup
 *		set_up_adapter
 *		start_up_adapter
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*** header files ***/
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mdio.h>
#include <sys/errno.h>
#include <sys/dma.h>
#include <sys/param.h>

/* local header files */
#include "skytu_type.h"
#include "sky_regs.h"

/**************************************************************************** *
*
* FUNCTION NAME =  initialize_adapter
*
* DESCRIPTION   =  This function initializes the adapter
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*              tucb_ptr       - ptr to test unit control block
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR = error code
*
* INVOKED FROM = oncard_test, connect_test
*
*****************************************************************************/
int initialize_adapter(ADAPTER_STRUCT *adapter_info,TUTYPE *tucb_ptr,int ttype)
{
  ushort    i, srb_address;
  int       int_level;
  int       rc = 0;
  BOOLEAN   SUCCESS = FALSE;
  ushort  data, SISR_status_reg;

  adapter_info->opened = FALSE;
  DEBUG_0("Initialize adapter\n");
  /**********************
  *   Start Up Adapter
  **********************/
  adapter_info->wd_setter = WDT_INIT;
  if (rc = start_up_adapter(adapter_info, tucb_ptr, ttype)) {
	return (error_handler(tucb_ptr, ttype, rc, STARTUP_ERR,0,0,0));
  }

  tucb_ptr->header.adap_flag = TRUE;

  /************************************************************
  * Wait for interrupt to indicate diagnostics have completed
  ************************************************************/
  DEBUG_0("Waiting for interrupt \n");
  rc = diag_watch4intr(adapter_info->handle, SRB_RSP, TIMEOUT_LIMIT);
	
  /**************************************
  * Check for error from diag_watch4intr
  **************************************/
  if (rc) {
	DEBUG_0("Interrupt after initailize adapter not received\n");
	rc = (process_timeout(adapter_info, tucb_ptr, ttype, SRB_RSP));
	if (rc)
		return(rc);
}

  /********************************
  *  Get SRB address from LAPWWO
  ********************************/
  rc = io_rd_swap(adapter_info, IOSHORT, LAPWWC, &srb_address);
  if (rc = io_rd_swap(adapter_info, IOSHORT, LAPWWO, &srb_address)) {
	return (error_handler(tucb_ptr, ttype, LAPWWO_ERR + READ_ERR,
		REGISTER_ERR, LAPWWO, 0, 0));
  }

  adapter_info->srb_address = srb_address;

  /**************************
  *  Process the interrupt
  **************************/
  if (!(rc = process_interrupts(adapter_info, tucb_ptr, ttype, SRB_RSP))) {
	SUCCESS = TRUE;
  } else {
	mps_clean_up(adapter_info, tucb_ptr, ttype);
	return(rc);
  }

  return(0);

} /* end initialize_adapter */
/*****************************************************************************
*
* FUNCTION NAME =  start_up_adapter
*
* DESCRIPTION   =  This function initializes transmit and receive areas and
*                  starts the adapter.
*
* INPUT   =    adapter_info   - adapter information structure
*
* RETURN-NORMAL =  0
* RETURN-ERROR =   D_MAP_ERR
*		   MEMORY_ERR
*
*****************************************************************************/
int start_up_adapter(ADAPTER_STRUCT *adapter_info, TUTYPE *tucb_ptr, int ttype)

{
   int i;
   int j;
   int rc, rc1, rc2;
   uint Addr;
   ulong data;

   /* Setup receive buffer and descriptor*/

  DEBUG_0("setup recv buffers and descriptors\n");
  for (i = 0; i < MAX_RX_LIST; i++) {
   	if ((adapter_info->recv_usr_buf[i] = (uchar *)malloc(2*PAGESIZE)) 
		== NULL)
   	{
		for (j = 0; j < i; j++){
			diag_unmap_page(adapter_info->handle,
				&adapter_info->recv_dma_addr[j]);
			free(adapter_info->recv_usr_buf[j]);
		}
		return (MEMORY_ERR);
   	}

  DEBUG_1("RX BUFFER ADDR = %x\n",adapter_info->recv_usr_buf[i]);
  /* Adjust buffers so that they are memory alligned */
   	adapter_info->recv_buf[i] = 
		(uchar *)((int)adapter_info->recv_usr_buf[i] + 0x1000);
   	adapter_info->recv_buf[i] = 
		(uchar *)((int)adapter_info->recv_buf[i] & 0xfffff000);

  /* Map buffers for DMA */
   	rc = diag_map_page(adapter_info->handle,DMA_READ,
		adapter_info->recv_buf[i],&adapter_info->recv_dma_addr[i],
		PAGESIZE);
   	if (rc != DGX_OK)
   	{
		DEBUG_1("diag_map_page for receive = 0x%08x\n",rc);
		free(adapter_info->recv_usr_buf[i]);
		for (j = 0; j < i; j++){
			diag_unmap_page(adapter_info->handle,
				&adapter_info->recv_dma_addr[j]);
			free(adapter_info->recv_usr_buf[j]);
		}
		return(D_MAP_ERR);
   	}
  }

  /* Allocate memory for receive descriptors */
  for (i = 0; i < MAX_RX_LIST; i++) {
   	if ((adapter_info->recv_usr_desc[i] = 
		(rx_list_t *)malloc(256 + PAGESIZE)) == NULL)
   	{
		for (j = 0; j < MAX_RX_LIST; j++){
			diag_unmap_page(adapter_info->handle,
				&adapter_info->recv_dma_addr[j]);
			free(adapter_info->recv_usr_buf[j]);
		}
		for (j = 0; j < i; j++){
			diag_unmap_page(adapter_info->handle,
				&adapter_info->recv_desc_addr[j]);
			free(adapter_info->recv_usr_desc[j]);
		}
		return (MEMORY_ERR);
   	}

/* Allign the memory returned from malloc */
   	adapter_info->recv_list[i] = 
		(rx_list_t *)((int)adapter_info->recv_usr_desc[i] + 0x1000);
   	adapter_info->recv_list[i] = 
		(rx_list_t *)((int)adapter_info->recv_list[i] & 0xfffff000);

/* Initialize and DMA map descriptors */
  	adapter_info->recv_list[i]->recv_status = 0;
  	adapter_info->recv_list[i]->data_pointer = 
		SWAP32((ulong)adapter_info->recv_dma_addr[i]);
  	adapter_info->recv_list[i]->data_len = SWAP16((ushort)PAGESIZE);
  	adapter_info->recv_list[i]->fr_len = 0;

   	rc = diag_map_page(adapter_info->handle,DMA_READ,
		adapter_info->recv_list[i],
		&adapter_info->recv_desc_addr[i],
		(sizeof(rx_list_t) * MAX_RX_LIST));
   	if (rc != DGX_OK)
   	{
		DEBUG_1("diag_map_page for receive desc = 0x%08x\n",rc);
		for (j = 0; j < MAX_RX_LIST; j++){
			diag_unmap_page(adapter_info->handle,
				&adapter_info->recv_dma_addr[j]);
			free(adapter_info->recv_usr_buf[j]);
		}
		free(adapter_info->recv_usr_desc[i]);
		for (j = 0; j < i; j++){
			diag_unmap_page(adapter_info->handle,
				&adapter_info->recv_desc_addr[j]);
			free(adapter_info->recv_usr_desc[j]);
		}
		return(D_MAP_ERR);
   	}

  } /* end of for */

  adapter_info->recv_list[0]->fw_pointer = 
	SWAP32((ulong) adapter_info->recv_desc_addr[1]);
  adapter_info->recv_list[1]->fw_pointer = 
	SWAP32((ulong) adapter_info->recv_desc_addr[0]);

  DEBUG_1("RX DESC DMA ADDR 0 = %08x\n",adapter_info->recv_desc_addr[0]);
  DEBUG_1("RX DESC DMA ADDR 1 = %08x\n",adapter_info->recv_desc_addr[1]);
  DEBUG_1("RX BUF DMA ADDR 0 = %08x\n",adapter_info->recv_dma_addr[0]);
  DEBUG_1("RX BUF DMA ADDR 1 = %08x\n",adapter_info->recv_dma_addr[1]);
  DEBUG_0("DUMP LOCAL RX DESC\n");
#ifdef DEBUG
  hexdump((char *)&adapter_info->recv_list[0]->fw_pointer,20);
  hexdump((char *)&adapter_info->recv_list[1]->fw_pointer,20);
#endif

  /************************************************
  * Give the buffer descriptor address to adapter
  ************************************************/

  Addr    = (uint)adapter_info->recv_desc_addr[0];
  rc1 = io_wr_swap(adapter_info,IOLONG,RxBDA_LO, Addr);
  rc2 = io_wr_swap(adapter_info,IOLONG,RxLBDA_LO, Addr);

  if (rc1 || rc2) {
	for (j = 0; j < MAX_RX_LIST; j++){
		diag_unmap_page(adapter_info->handle, 
			&adapter_info->recv_dma_addr[j]);
		free(adapter_info->recv_usr_buf[j]);
		diag_unmap_page(adapter_info->handle, 
			&adapter_info->recv_desc_addr[j]);
		free(adapter_info->recv_usr_desc[j]);
	}
	return(WRITE_ERR);
  }
  (void)io_rd_swap(adapter_info,IOLONG,RxBDA_LO, &data);
  (void)io_rd_swap(adapter_info,IOLONG,RxLBDA_LO, &data);

  DEBUG_0("setup transmit buffers and descriptors\n");
  for (i = 0; i < MAX_TX_LIST; i++) {
  	if ((adapter_info->xmit_usr_buf[i] = (uchar *)malloc(2*PAGESIZE))==NULL)
  	{
		for (j = 0; j < MAX_RX_LIST; j++){
			diag_unmap_page(adapter_info->handle, 
				&adapter_info->recv_dma_addr[j]);
			free(adapter_info->recv_usr_buf[j]);
			diag_unmap_page(adapter_info->handle, 
				&adapter_info->recv_desc_addr[j]);
			free(adapter_info->recv_usr_desc[j]);
		}
		for (j = 0; j < i; j++){
			diag_unmap_page(adapter_info->handle, 
				&adapter_info->xmit_dma_addr[j]);
			free(adapter_info->xmit_usr_buf[j]);
		}
		return (MEMORY_ERR);
  	}

/* Allign the memory returned from malloc */
   	adapter_info->xmit_buf[i] = 
		(uchar *)((int)adapter_info->xmit_usr_buf[i] + 0x1000);
   	adapter_info->xmit_buf[i] = 
		(uchar *)((int)adapter_info->xmit_buf[i] & 0xfffff000);

  	rc = diag_map_page(adapter_info->handle,0,
		adapter_info->xmit_buf[i],&adapter_info->xmit_dma_addr[i],
		PAGESIZE);
        DEBUG_2("xmit buf malloc addr = %08x, dma addr = %08x\n",adapter_info->xmit_buf[i], adapter_info->xmit_dma_addr[i]);
  	if (rc != DGX_OK)
  	{
		DEBUG_1("diag_map_page for transmit = 0x%08x\n",rc);
		free(adapter_info->xmit_usr_buf[i]);
		for (j = 0; j < i; j++){
			diag_unmap_page(adapter_info->handle, 
				&adapter_info->xmit_dma_addr[j]);
			free(adapter_info->xmit_usr_buf[j]);
		}
		for (j = 0; j < MAX_RX_LIST; j++){
			diag_unmap_page(adapter_info->handle, 
				&adapter_info->recv_dma_addr[j]);
			free(adapter_info->recv_usr_buf[j]);
			diag_unmap_page(adapter_info->handle, 
				&adapter_info->recv_desc_addr[j]);
			free(adapter_info->recv_usr_desc[j]);
		}
 		return(D_MAP_ERR);
  	}
  }
  for (i = 0; i < MAX_TX_LIST; i++) {
   	if ((adapter_info->xmit_usr_desc[i] = 
		(tx_list_t *)malloc(256 + PAGESIZE)) == NULL)
   	{
		for (j = 0; j < i; j++){
			diag_unmap_page(adapter_info->handle, 
				&adapter_info->xmit_desc_addr[j]);
			free(adapter_info->xmit_usr_desc[j]);
		}
		for (j = 0; j < MAX_TX_LIST; j++){
			diag_unmap_page(adapter_info->handle, 
				&adapter_info->xmit_dma_addr[j]);
			free(adapter_info->xmit_usr_buf[j]);
		}
		for (j = 0; j < MAX_RX_LIST; j++){
			diag_unmap_page(adapter_info->handle, 
				&adapter_info->recv_dma_addr[j]);
			free(adapter_info->recv_usr_buf[j]);
			diag_unmap_page(adapter_info->handle, 
				&adapter_info->recv_desc_addr[j]);
			free(adapter_info->recv_usr_desc[j]);
		}
		return (MEMORY_ERR);
   	}

   	adapter_info->xmit_list[i] = 
		(tx_list_t *)((int)adapter_info->xmit_usr_desc[i] + 0x1000);
   	adapter_info->xmit_list[i] = 
		(tx_list_t *)((int)adapter_info->xmit_list[i] & 0xfffff000);

  	adapter_info->xmit_list[i]->xmit_status  = 0;
  	adapter_info->xmit_list[i]->buf_count    = SWAP16(0x1);
  	adapter_info->xmit_list[i]->frame_len    = 0;
  	adapter_info->xmit_list[i]->data_pointer = 
		SWAP32((ulong)adapter_info->xmit_dma_addr[i]);
  	adapter_info->xmit_list[i]->buf_len      = 0;
   	rc = diag_map_page(adapter_info->handle,0,
		adapter_info->xmit_list[i],
		&adapter_info->xmit_desc_addr[i],
		(sizeof(tx_list_t) * MAX_TX_LIST));
   	if (rc != DGX_OK)
   	{
		free(adapter_info->xmit_usr_desc[i]);
		for (j = 0; j < i; j++){
			diag_unmap_page(adapter_info->handle, 
				&adapter_info->xmit_desc_addr[j]);
			free(adapter_info->xmit_usr_desc[j]);
		}
		for (j = 0; j < MAX_TX_LIST; j++){
		 	diag_unmap_page(adapter_info->handle, 
		 		&adapter_info->xmit_dma_addr[j]);
		 	free(adapter_info->xmit_usr_buf[j]);
		}

		for (j = 0; j < MAX_RX_LIST; j++){
			diag_unmap_page(adapter_info->handle, 
				&adapter_info->recv_dma_addr[j]);
			free(adapter_info->recv_usr_buf[j]);
			diag_unmap_page(adapter_info->handle, 
				&adapter_info->recv_desc_addr[j]);
			free(adapter_info->recv_usr_desc[j]);
		}

		return(D_MAP_ERR);
   	}
        DEBUG_1("xmit desc dma addr = %08x\n", adapter_info->xmit_desc_addr[i]);
  } /* end of for */
  adapter_info->xmit_list[0]->fw_pointer = 
		SWAP32((ulong) adapter_info->xmit_desc_addr[0]);
  /* Set up variable for Transmit list */

  /*********************************************************************
  *  Set up Master Interrupt Status Mask Register to enable Interrupts
  *********************************************************************/
  if (rc = io_wr_swap(adapter_info,IOSHORT,MISRM_SUM, 0xff37)) {
	DEBUG_0("io wr failed more unmapping taking place\n");
	mps_clean_up(adapter_info, tucb_ptr, ttype);
	return(WRITE_ERR);
  }
  /******************************************
  *  Start Adapter Initialization Sequence
  ******************************************/
  if (rc = io_wr_swap(adapter_info,IOSHORT,SISRM, SISR_MSK)) {
	DEBUG_0("io wr failed more unmapping taking place\n");
	mps_clean_up(adapter_info, tucb_ptr, ttype);
	return(WRITE_ERR);
  }

  return (0);
}  /* End start_up_adapter */
/****************************************************************************
*
* FUNCTION NAME =  set_up_adapter()
*
* DESCRIPTION   =  This function sets up the adapter by updating
*                  the cfg registers with default information obtained
*                  from the ODM.
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*              tucb_ptr       - ptr to test unit control block
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   non-zero
*
* INVOKED FROM = io_test
*                oncard_test
*
*****************************************************************************/
int set_up_adapter (ADAPTER_STRUCT *adapter_info, TUTYPE *tucb_ptr, int ttype)
{
  uchar          PCR_Reg_HI,PCR_Reg_LO,PCT_value,BC_Reg_LO, BC_Reg_HI; 
  uchar          BM_Reg_HI,BM_Reg_LO;
  uint           int_bits, int_level, rom_bits;
  int            rc = 0;
  mps_dds        dds;
  ulong		 w_data;

  /***********************************************************
   *    Get access to ODM Information through dds pointer    *
   ***********************************************************/
 dds = adapter_info->dds;

/* write base io address*/
    w_data = dds.bus_io_addr;
  if (rc = config_reg_write(BA0+1,(w_data & 0x0000ff00) >> 8,dds)) {
  	return (error_handler(tucb_ptr, ttype, CFG_ERR + WRITE_ERR,
                          REGISTER_ERR, BA0+1, (w_data & 0x0000ff00 >> 8), 0));
  }
  if (rc = config_reg_write(BA0+2,(w_data & 0x00ff0000) >> 16,dds)) {
  	return (error_handler(tucb_ptr, ttype, CFG_ERR + WRITE_ERR,
                          REGISTER_ERR, BA0+2, (w_data & 0x00ff0000 >> 16), 0));
  }
  if (rc = config_reg_write(BA0+3,(w_data & 0xff000000) >> 24,dds)) {
  	return (error_handler(tucb_ptr, ttype, CFG_ERR + WRITE_ERR,
                          REGISTER_ERR, BA0+3, (w_data & 0x00ff0000 >> 24), 0));
  }

  /***********************************
  * Enable BusMaster and give access to I/O and Memory
  ************************************/
  PCR_Reg_HI = 0x01;  /* enable SERR buffer */

  if (rc = config_reg_write(PCR+1,PCR_Reg_HI,dds)) {
  	return (error_handler(tucb_ptr, ttype, CFG_ERR + WRITE_ERR,
                          REGISTER_ERR, PCR, PCR_Reg_HI, 0));
  }

  PCR_Reg_LO = 0x57; /* enable parity response, memory write & invalidate,
                        PCI master, memory cycles, I/O cycles */
  
  if (rc = config_reg_write(PCR,PCR_Reg_LO,dds)) {
  	return (error_handler(tucb_ptr, ttype, CFG_ERR + WRITE_ERR,
                          REGISTER_ERR, PCR + 1, PCR_Reg_LO, 0));
  }
  
  return(0);


} /* end of set_up_adapter */

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
* INVOKED FROM = get_srb_responce 
*
*****************************************************************************/
int recv_setup (ADAPTER_STRUCT *adapter_info)
{
  int                 i, x;
  int                 rc = 0;

  /* reset the block descriptors for the receive buffers */

 adapter_info->recv_list[0]->fw_pointer = SWAP32((ulong)
	adapter_info->recv_desc_addr[1]);
 adapter_info->recv_list[0]->recv_status = 0;
 adapter_info->recv_list[0]->data_pointer = SWAP32((ulong)
        adapter_info->recv_dma_addr[0]);
 adapter_info->recv_list[0]->data_len = SWAP16((ushort) PAGESIZE);
 adapter_info->recv_list[0]->fr_len = 0;
 
 adapter_info->recv_list[1]->fw_pointer = SWAP32((ulong)
	adapter_info->recv_desc_addr[0]);
 adapter_info->recv_list[1]->recv_status = 0;
 adapter_info->recv_list[1]->data_pointer = SWAP32((ulong)
        adapter_info->recv_dma_addr[1]);
 adapter_info->recv_list[1]->data_len = SWAP16((ushort) PAGESIZE);
 adapter_info->recv_list[1]->fr_len = 0;
 
#ifdef BEST
	DEBUG_1("best orw = %d\n",adapter_info->best_orw);
  	if (adapter_info->best_orw == 2) 
	{
		adapter_info->recv_list[0]->data_len = SWAP16(16);
		adapter_info->recv_list[1]->data_pointer = 
			SWAP32(adapter_info->buf_addr);
		
	}
#endif
  DEBUG_0("dump rx desc\n");
#ifdef DEBUG
  hexdump((char *)&adapter_info->recv_list[0]->fw_pointer,20);
#endif
  return(0);

}
/**************************************************************************** *
*
* FUNCTION NAME =  get_diagnostic_codes
*
* DESCRIPTION   =  This function gets the dianostic return codes
*                  from the adapter.
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
int get_diagnostic_codes (ADAPTER_STRUCT *adapter_info, TUTYPE *tucb_ptr,
int ttype, int *host_rc, int *man_rc)
{
  ushort  i, data[10];
  ushort  command, hrc, mrc;
  int     rc = 0;

  /*********************************
   *  Get Manufacturing Return Code
   *********************************/
  if ( rc = io_rd_swap(adapter_info, IOSHORT, LAPWWC, &mrc)) {
	return (error_handler(tucb_ptr, ttype, LAPWWC_ERR + READ_ERR,
		    REGISTER_ERR, LAPWWC, 0, 0 ));
  }

  /************************
   *  Get Host Return Code
   ************************/
  if ( rc = io_rd_swap(adapter_info, IOSHORT, LAPA, &hrc)) {
		return (error_handler(tucb_ptr, ttype, LAPA_ERR + READ_ERR,
		    REGISTER_ERR, LAPA, 0, 0));
  }

  DEBUG_1("HRC = %x\n", hrc);
  DEBUG_1("MRC = %x\n", mrc);

  *host_rc = hrc;
  *man_rc  = mrc;

  return(0);

} /* end get_diagnostic_codes */


/*****************************************************************************
*
* FUNCTION NAME =  get_adapter_address
*
* DESCRIPTION   =  This function gets the burned in adapter address
*                  from the adapter.
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*              tucb_ptr       - ptr to test unit control block
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR = error code
*
* INVOKED FROM = get_srb_response
*
*****************************************************************************/
int get_adapter_address (ADAPTER_STRUCT *adapter_info, TUTYPE *tucb_ptr,
int ttype, ushort addrptr)
{
  ushort  i, bia[3];
  ushort  command, hrc, mrc;
  int     rc = 0;


  /************************
   *  Set up LAP register
   ************************/
  if (rc = io_wr_swap(adapter_info, IOSHORT, LAPA, addrptr)) {
	return (error_handler(tucb_ptr, ttype, LAPA_ERR + WRITE_ERR,
		    REGISTER_ERR, LAPA, addrptr, 0));
  }

  /**************************
   *  Get Burned in Address
   **************************/
  for (i=0; i < 3; i++) {
	if (rc = io_rd(adapter_info, IOSHORT, LAPDInc, &bia[i])) {
		return (error_handler(tucb_ptr, ttype, LAPDInc_ERR + READ_ERR,
			    REGISTER_ERR, LAPDInc, 0, 0));
	}

	adapter_info->source_address[i] = bia[i];
	adapter_info->dest_address[i] = bia[i];   /* init for default */

	DEBUG_2(" Address #%d = %x\n", i, bia[i]);
  }

  return(0);

} /* end get_adapter_address */

