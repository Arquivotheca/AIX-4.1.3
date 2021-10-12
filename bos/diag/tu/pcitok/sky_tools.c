static char sccsid[] = "@(#)15	1.2  src/bos/diag/tu/pcitok/sky_tools.c, tu_pcitok, bos41J 3/30/95 14:04:33";
/*
 *   COMPONENT_NAME: tu_pcitok
 *
 *   FUNCTIONS: adapter_close
 *		adapter_open
 *		byte_swap
 *		error_handler
 *		get_card_id
 *		initialize_adapter_info
 *		post_error
 *		print_error_details
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
#include <stdio.h>
#include <sys/sysconfig.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/param.h>

#include "skytu_type.h"


/******************************************************************************
*
* NAME:  post_error
*
* FUNCTION:  displays error and logs in adapter_info
*
* INPUT PARAMETERS:     error = error to be logged.
*                       adapter_info = general card information.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: none.
*
* EXTERNAL PROCEDURES CALLED: printf
*
******************************************************************************/
void post_error( int error, ADAPTER_STRUCT *adapter_info )
{

	adapter_info->error_log[adapter_info->num_errors] = error;
	adapter_info->num_errors++;

}

/******************************************************************************
*
* NAME:  get_card_id
*
* FUNCTION:  If the device is a PCI adapter, the device ID is read from the
*            DID register of the configuration header.  If the device is a
*            MCA adapter, the device ID is read from POS 0 and POS 1 registers. 
*
* INPUT PARAMETERS:     adapter_info = general card information.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns ID of card else 0
*
* EXTERNAL PROCEDURES CALLED: diag_pos_read, cfg_rd 
*
******************************************************************************/
unsigned int get_card_id(ADAPTER_STRUCT *adapter_info)
{

  int  card_id_high;
  int  card_id_low;
  uchar vid_lo;
  uchar vid_hi;
  uchar did_lo;
  uchar did_hi;
  int  card_id;
  int  rc;


  DEBUG_0("Reading PCI card id\n");
  /* Read upper card id byte from DID */

  
  rc = config_reg_read(VID_REG+1,&vid_hi,adapter_info->dds);

  if ( rc != 0) {
	DEBUG_0("Read of high byte failed\n");
	post_error(errno, adapter_info);
	return 0;
  }
  
  card_id = (int)vid_hi << 24;

  /* Read lower card id byte from DID */
  rc = config_reg_read(VID_REG,&vid_lo,adapter_info->dds);
  
  if ( rc != 0) {
	DEBUG_0("Read of low byte failed\n");
	post_error(errno, adapter_info);
	return 0;
  }

  card_id = card_id | (int)vid_lo << 16;

  rc = config_reg_read(DID_REG+1,&did_hi,adapter_info->dds);

  if ( rc != 0) {
	DEBUG_0("Read of high byte failed\n");
	post_error(errno, adapter_info);
	return 0;
  }
  
  card_id = card_id | (int)did_hi << 8;

  /* Read lower card id byte from DID */
  rc = config_reg_read(DID_REG,&did_lo,adapter_info->dds);
  
  if ( rc != 0) {
	DEBUG_0("Read of low byte failed\n");
	post_error(errno, adapter_info);
	return 0;
  }
  
  card_id = card_id | (int)did_lo;

  return card_id;

}


/******************************************************************************
*
* NAME:  initialize_adapter_info
*
* FUNCTION:  Sets adapter card id and checks for enable timeout
*
* INPUT PARAMETERS:     adapter_info = general card information
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns 0 of card elsE
*                           ADAPTER_TIMEOUT_ERROR.
*
* EXTERNAL PROCEDURES CALLED: get_card_id
*
******************************************************************************/
int initialize_adapter_info(ADAPTER_STRUCT *adapter_info)
{

  int timeout_length = 1;

  adapter_info->card_id = 0;
  
  while (timeout_length-- && (adapter_info->card_id == 0)) {
  	adapter_info->card_id = get_card_id(adapter_info);
  }

  if (timeout_length==0) {
	DEBUG_0("Card id read timeout\n");
  	return ADAPTER_TIMEOUT_ERROR;
  } else {
  	adapter_info->num_interrupts = 0;
  	adapter_info->int_pending = 0;
  	return 0;
  }

}
/******************************************************************************
*
* NAME:  adapter_open
*
* FUNCTION:  Initialize an adapter to be used with diag_ex and
*            verify its usability.
*
* INPUT PARAMETERS:     adapter_name = device name of adapter.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns address of handle else NULL
*
* EXTERNAL PROCEDURES CALLED: diag_open,
*                             initialize_adapter, malloc.
*
******************************************************************************/
int  adapter_open( 
TUTYPE *tucb_ptr,
ADAPTER_STRUCT *adapter_info, 
char *adapter_name)
{
  struct cfg_load  *cfg_ld;
  diag_struc_t     *adapter_handle;
  diagex_dds_t     *diag_dds;
  int              rc, errcode;
  char 		   ipath[512];

  diag_dds =       (diagex_dds_t *) malloc (sizeof (diagex_dds_t));
  if (diag_dds == NULL)
  {
	return(MEMORY_ERR);
  }

  if ((rc = getmps(tucb_ptr, adapter_name, &adapter_info->dds)) != 0) {
	DEBUG_0("ERROR in getmps\n");
  	free(diag_dds);
	return(rc);
  }

  diag_dds->intr_flags = NULL;               /* not used by PCI */
  diag_dds->dma_lvl = NULL;                  /* not used by PCI */
  diag_dds->bus_mem_addr = NULL;             /* not used by Skyline */ 
  diag_dds->bus_mem_length = NULL;           /* not used by Skyline */     
  diag_dds->bus_type = BUS_BID;
  diag_dds->dma_flags = DMA_MASTER;

  strcpy(diag_dds->device_name , adapter_info->dds.adpt_name);
  strcpy(diag_dds->parent_name , adapter_info->dds.par_name);
  diag_dds->slot_num = adapter_info->dds.slot_num;
  diag_dds->bus_intr_lvl   = adapter_info->dds.bus_intr_lvl;
  diag_dds->intr_priority = adapter_info->dds.intr_priority;
  diag_dds->bus_io_addr = adapter_info->dds.bus_io_addr;
  diag_dds->bus_io_length = adapter_info->dds.bus_io_length;
  diag_dds->dma_bus_mem = adapter_info->dds.dma_bus_mem;
  diag_dds->dma_bus_length = adapter_info->dds.dma_bus_length;
  diag_dds->bus_id = adapter_info->dds.bus_id;
  diag_dds->maxmaster = 32;
/*
 * Set up the I/O base address and enable I/O access on the adapter
 * before initializing the interrupt handler.
 */
  if (rc = set_up_adapter(adapter_info, tucb_ptr, TU_OPEN)) {
	return (rc);
  }
  
  /***************************************************
   Load the interrupt handler if not already loaded.
   This code then passes the DDS to the kernel extension.
  ***************************************************/
  cfg_ld = (struct cfg_load *) malloc (sizeof (struct cfg_load));
  if (cfg_ld == NULL)
  {
	free(diag_dds);
	diagex_initial_state(adapter_name);
	return(MEMORY_ERR);
  }
#ifdef DIAGPATH
  sprintf(ipath,"%s/%s",(char *)getenv("DIAGX_SLIH_DIR"),"sky_intr");
  cfg_ld->path = ipath;
#else
  cfg_ld->path = INTERRUPT_HANDLER_PATH;
#endif


  DEBUG_1("Interrupt Handler -> %s\n", cfg_ld->path);

  errno = 0;
  rc=sysconfig(SYS_SINGLELOAD, cfg_ld, (int)sizeof(struct cfg_load));
  if (rc) {
	DEBUG_0("Error: Failed to load interrupt handler\n");
#ifdef DEBUG
	perror("return from sysconfig ");
#endif
	DEBUG_1("Errno = %d\n",errno);
	free(diag_dds);
	free(cfg_ld);
	diagex_initial_state(adapter_name);
	return(rc);
  }
  DEBUG_0("Loaded interrupt handler\n");
  diag_dds->kmid = (int)cfg_ld->kmid;
  diag_dds->data_ptr = (char *)adapter_info;
  diag_dds->d_count = sizeof(ADAPTER_STRUCT);

  DEBUG_1("interrupt level = %d\n",diag_dds->bus_intr_lvl);
  DEBUG_0("Calling diag open\n");
  rc = (int)diag_open(diag_dds,&adapter_handle);
  DEBUG_0("RETURN FROM DIAG OPEN\n");
  if (rc) {
	DEBUG_1("Open RC = %x\n",rc);
  	free(diag_dds);
	free(cfg_ld);
	diagex_initial_state(adapter_name);
	return(OPEN_FAILED);
  }

  adapter_info->handle = adapter_handle;
  adapter_info->dma_channel = diag_dds->dma_chan_id;
  rc = initialize_adapter_info(adapter_info);

  free(diag_dds);
  free(cfg_ld);
  return (rc);
}


/******************************************************************************
*
* NAME:  adapter_close
*
* FUNCTION:  Close an adapter initialized  with diag_ex and
*            free all associated resources.
*
* INPUT PARAMETERS:     adapter_name = device name of adapter.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns address of handle else NULL
*
* EXTERNAL PROCEDURES CALLED: diag_close, post_error
*
******************************************************************************/
int adapter_close(ADAPTER_STRUCT *adapter_info)
{
  int rc = 0;
  
  rc = diag_close(adapter_info->handle);
  DEBUG_1("diag_close rc = 0x%x\n",rc);
  diagex_initial_state(adapter_info->dds.adpt_name);
  free(adapter_info);
  
  return(rc);

}


/******************************************************************************
*
* NAME: byte_swap
*
* FUNCTION:  returns a byte swapped version of a long integer.
*
* INPUT PARAMETERS:     old_long = long integer to be swapped.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: swapped_long = byte swapped long integer.
*
* EXTERNAL PROCEDURES CALLED: none.
*
******************************************************************************/
unsigned long byte_swap(unsigned long old_long)
{

  unsigned long swapped_long;

  swapped_long = ((old_long & 0x000000ff) << 8) |
	    ((old_long & 0x0000ff00) >> 8)  |
	    ((old_long & 0x00ff0000) << 8)  |
	    ((old_long & 0xff000000) >> 8);

  return swapped_long;
}


/******************************************************************************
*
* NAME: error_handler
*
* FUNCTION:
*
* INPUT PARAMETERS:     tucb_ptr
*
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: error code.
*
* EXTERNAL PROCEDURES CALLED: none.
*
******************************************************************************/
int error_handler(TUTYPE *tucb_ptr, int tunum, int errcode, int errtype,
int val1, int val2, int val3)
{

  /***********************************
   *  Get Access to Error Information
  ***********************************/

  DEBUG_3("TUNUM = %d, ERRCODE = %x, ERRTYPE = %x\n",tunum,errcode,errtype);
  DEBUG_3("VAL1 = %d, VAL2 = %x, VAL3 = %x\n",val1,val2,val3);

  /**************************************
   *  Determine Prefix for Error Message
   **************************************/
  switch (tunum) {
	case TU_OPEN:     
		strcpy(tucb_ptr->errinfo->error_msg, INIT_PREFIX);
		break;

	case CFG_TEST:    
		strcpy(tucb_ptr->errinfo->error_msg, CFG_PREFIX);
		break;

	case IO_TEST:     
		strcpy(tucb_ptr->errinfo->error_msg, IO_PREFIX);
		break;

	case ONCARD_TEST: 
		strcpy(tucb_ptr->errinfo->error_msg, ONCARD_PREFIX);
		break;

	case CONNECT_TEST: 
		strcpy(tucb_ptr->errinfo->error_msg, CONNECT_PREFIX);
		break;

	case INT_WRAP_TEST: 
		strcpy(tucb_ptr->errinfo->error_msg, INT_WRAP_PREFIX);
		break;

	case EXT_WRAP_TEST: 
		strcpy(tucb_ptr->errinfo->error_msg, EXT_WRAP_PREFIX);
		break;

	case NETWORK_TEST: 
		strcpy(tucb_ptr->errinfo->error_msg, NETWORK_PREFIX);
		break;

	case TU_CLOSE:    
		strcpy(tucb_ptr->errinfo->error_msg, TERM_PREFIX);
		break;

	default:          
		strcpy(tucb_ptr->errinfo->error_msg, DEFAULT_PREFIX);
		break;
  }

  /********************
   *  Check Error Type
   ********************/
  switch (errtype)
	{
	case REGISTER_ERR:   
		if (errcode & READ_ERR) {
			strcat(tucb_ptr->errinfo->error_msg, READ_ERR_MSG);
		} else if (errcode & WRITE_ERR) {
			strcat(tucb_ptr->errinfo->error_msg, WRITE_ERR_MSG);
		} else {
			strcat(tucb_ptr->errinfo->error_msg, COMPARE_ERR_MSG);
		}

		tucb_ptr->errinfo->un.reg.bad_address    = val1;
		tucb_ptr->errinfo->un.reg.expected_value = val2;
		tucb_ptr->errinfo->un.reg.actual_value   = val3;
		break;

	case ADAP_INT_ERR:
	case ADAP_INIT_ERR:  
		strcat(tucb_ptr->errinfo->error_msg, ADAPTER_ERR_MSG);
		tucb_ptr->errinfo->un.adapter.hrc = val1;
		tucb_ptr->errinfo->un.adapter.mrc = val2;
		break;

	case CLEANUP_ERR:    
		strcat(tucb_ptr->errinfo->error_msg, CLEANUP_ERR_MSG);
		break;

	case CLOSE_ERR:      
		strcat(tucb_ptr->errinfo->error_msg, CLOSE_ERR_MSG);
		break;

	case FILE_ERR:       
		strcat(tucb_ptr->errinfo->error_msg, FILE_ERR_MSG);
		break;

	case FRAME_LEN_ERR:  
		strcat(tucb_ptr->errinfo->error_msg, FRAME_LEN_ERR_MSG);
		break;

	case HARD_ERR:       
		strcat(tucb_ptr->errinfo->error_msg, HARD_ERR_MSG);
		break;

	case SOFT_ERR:       
		strcat(tucb_ptr->errinfo->error_msg, SOFT_ERR_MSG);
		break;

	case BEACON_ERR:     
		strcat(tucb_ptr->errinfo->error_msg, BEACON_ERR_MSG);
		break;

	case AUTO_ERR:       
		strcat(tucb_ptr->errinfo->error_msg, AUTO_ERR_MSG);
		break;

	case REMOVE_ERR:     
		strcat(tucb_ptr->errinfo->error_msg, REMOVE_ERR_MSG);
		break;

	case LOBE_ERR:       
		strcat(tucb_ptr->errinfo->error_msg, LOBE_ERR_MSG);
		break;

	case OPEN_ERR:       
		strcat(tucb_ptr->errinfo->error_msg, OPEN_ERR_MSG);
		tucb_ptr->errinfo->un.sys.return_code   = val1;
		break;

	case STARTUP_ERR:    
		strcat(tucb_ptr->errinfo->error_msg, STARTUP_ERR_MSG);
		break;

	case SIGNAL_ERR:     
		strcat(tucb_ptr->errinfo->error_msg, SIGNAL_ERR_MSG);
		break;

	case SYSTEM_ERR:     
		strcat(tucb_ptr->errinfo->error_msg, SYSTEM_ERR_MSG);
		break;

	case TIMEOUT_ERR:    
		strcat(tucb_ptr->errinfo->error_msg, TIMEOUT_ERR_MSG);
		break;

	case WRAP_ERR:       
		strcat(tucb_ptr->errinfo->error_msg, WRAP_ERR_MSG);
		tucb_ptr->errinfo->un.adapter.hrc = val1;
		tucb_ptr->errinfo->un.adapter.mrc = val2;
		break;

	default:             
		break;
  }

  DEBUG_1("Errbuf = %s\n",tucb_ptr->errinfo->error_msg);

  /******************************************
   *  Set Remaining Error Information Values
   ******************************************/
  tucb_ptr->errinfo->error_type = errtype;
  tucb_ptr->errinfo->error_code = errcode;
  tucb_ptr->errinfo->tunum      = tunum;

  DEBUG_1("Error info addr %x\n",tucb_ptr->errinfo);
  DEBUG_1("TUCB PTR = %x\n",tucb_ptr);
  return(errcode);

} /* end error_handler */



/******************************************************************************
*
* NAME: print_error_details
*
* FUNCTION:
*
* INPUT PARAMETERS:     error_info
*
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: error code.
*
* EXTERNAL PROCEDURES CALLED: none.
*
******************************************************************************/
void print_error_details(TUTYPE *tucb_ptr, error_details *error_info)
{

  /********************************
   *  Print Error Message and Code
   ********************************/
  DEBUG_0(error_info->error_msg);
  DEBUG_1(RETURN_CODE_MSG, error_info->error_code);

  switch (error_info->error_type) {
	case REGISTER_ERR:  
		DEBUG_1(BAD_ADDRESS_MSG,
		    error_info->un.reg.bad_address);

		if (error_info->error_code & WRITE_ERR) {
			DEBUG_1(ATTEMPTED_WRITE_MSG,
			    error_info->un.reg.expected_value);
		}

		if (error_info->error_code & COMPARE_ERR) {
			DEBUG_1(EXPECTED_VALUE_MSG,
			    error_info->un.reg.expected_value);
			DEBUG_1(ACTUAL_VALUE_MSG,
			    error_info->un.reg.actual_value);
		}
		break;

	case ADAP_INIT_ERR: 
		DEBUG_1(HOST_RC_MSG, error_info->un.adapter.hrc);
		DEBUG_1(MANUFAC_RC_MSG, error_info->un.adapter.mrc);
		break;

	case ADAP_INT_ERR:  
		DEBUG_1(BAD_INT_MSG, error_info->un.adapter.hrc);
		break;

	case OPEN_ERR:      
		DEBUG_1(OPEN_CODE_MSG,error_info->un.sys.return_code);
		break;

	case WRAP_ERR:      
		DEBUG_1(WRAP_ERR_CODE,error_info->un.sys.return_code);
		break;

	default:            
		break;
  }

} /* end print_error_details */

