static char sccsid[] = "@(#)26  1.3  src/bos/diag/tu/mps/mps_tools.c, tu_mps, bos411, 9440E411e 10/11/94 16:57:09";
/*****************************************************************************
 * COMPONENT_NAME: (tu_mps)  Wildwood LAN adapter test units
 *
 * FUNCTIONS: adapter_open
 *            adapter_close
 *            error_handler
 *            print_error_details
 *            get_card_id
 *            initialize_adapter_info
 *            byte_swap
 *            post_error
 *
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
 ********************************************************************************/

/*** header files ***/
#include <stdio.h>
#include <sys/sysconfig.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/param.h>

#include "mpstu_type.h"

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
* FUNCTION:  Retrieves Microchannel Card ID from POS 0 and 1.
*
* INPUT PARAMETERS:     adapter_info = general card information.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns ID of card else 0
*
* EXTERNAL PROCEDURES CALLED: reg_read8
*
******************************************************************************/
unsigned int get_card_id(ADAPTER_STRUCT *adapter_info)
{

  uchar  card_id_high;
  uchar  card_id_low;
  ushort card_id;
  int    rc;

  /* Read Pos Register 0 for upper card id byte */
  rc = diag_pos_read(adapter_info->handle,
      0, &card_id_high, NULL ,PROCLEV);

  if (rc != 0) {
  	post_error(errno, adapter_info);
  	return 0;
  }

  /* Read Pos Register 1 for lower card id byte */
  rc = diag_pos_read(adapter_info->handle, 1, &card_id_low, NULL ,PROCLEV);

  if (rc != 0) {
  	post_error(errno, adapter_info);
  	return 0;
  }

  card_id = (card_id_high << 8) + card_id_low;

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

  int         timeout_length = 10;

  adapter_info->card_id = 0;
  
  while (timeout_length-- && (adapter_info->card_id == 0)) {
  	adapter_info->card_id = get_card_id(adapter_info);
  }

  if (timeout_length==0) {
  	return ADAPTER_TIMEOUT_ERROR;
  } else {
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
  diag_struc_t     *adapter_handle;
  diagex_dds_t     *diag_dds;
  struct cfg_load  *cfg_ld;
  int              rc, errcode;
  char		   ddpath[512];

  if ((rc = getmps(tucb_ptr, adapter_name, &adapter_info->dds)) != 0) {
	DEBUG_0("ERROR in getmps\n");
	return(rc);
  }

  diag_dds = (diagex_dds_t *) malloc (sizeof (diagex_dds_t));
  adapter_handle = (diag_struc_t *) malloc (sizeof (diag_struc_t));
  strcpy(diag_dds->device_name , adapter_info->dds.adpt_name);
  strcpy(diag_dds->parent_name , adapter_info->dds.par_name);
  diag_dds->slot_num = adapter_info->dds.slot_num;
  diag_dds->bus_intr_lvl   = adapter_info->dds.bus_intr_lvl;
  diag_dds->intr_priority = adapter_info->dds.intr_priority;
  diag_dds->intr_flags = adapter_info->dds.intr_flags;
  diag_dds->dma_lvl = adapter_info->dds.dma_lvl;
  diag_dds->bus_io_addr = adapter_info->dds.bus_io_addr;
  diag_dds->bus_io_length = adapter_info->dds.bus_io_length;
  diag_dds->bus_mem_addr = adapter_info->dds.bus_mem_addr;
  diag_dds->bus_mem_length = adapter_info->dds.bus_mem_length;
  diag_dds->dma_bus_mem = adapter_info->dds.dma_bus_mem;
  diag_dds->dma_bus_length = adapter_info->dds.dma_bus_length;
  diag_dds->bus_id = adapter_info->dds.bus_id;
  diag_dds->bus_type = adapter_info->dds.bus_type;
  diag_dds->dma_flags = MICRO_CHANNEL_DMA;
  diag_dds->maxmaster = 1;

  /***************************************************
   Load the interrupt handler if not already loaded.
   This code then passes the DDS to the kernel extension.
  ***************************************************/
  cfg_ld = ( struct cfg_load *) malloc (sizeof (struct cfg_load));
#ifdef DIAGPATH
  sprintf(ddpath, "%s/%s", (char *)getenv("DIAGX_SLIH_DIR"), "tok32_intr");
  cfg_ld->path = ddpath;
#else
  cfg_ld->path=INTERRUPT_HANDLER_PATH;
#endif

  DEBUG_1("Interrupt Handler -> %s\n", cfg_ld->path);

  errno = 0;
  if (rc=sysconfig(SYS_KLOAD, cfg_ld, (int)sizeof(cfg_ld))) {
	DEBUG_0("Error: Failed to load interrupt handler\n");
	DEBUG_1("Errno = %d\n",errno);
	free(cfg_ld);
	return(rc);
  }

  diag_dds->kmid = (int)cfg_ld->kmid;
  diag_dds->data_ptr = (char *)adapter_info;
  diag_dds->d_count = sizeof(ADAPTER_STRUCT);
  free(cfg_ld);

  adapter_handle = (diag_struc_t *) malloc (sizeof (diag_struc_t));
  rc = (int)diag_open(diag_dds, &adapter_handle);
  if (rc) {
	DEBUG_1("Open RC = %x\n",rc);
	return(OPEN_FAILED);
  }

  DEBUG_1("adapter_handle = %x \n", adapter_handle);
  adapter_info->handle = adapter_handle;
  adapter_info->dma_channel = diag_dds->dma_chan_id;
  free(adapter_handle);
  free(diag_dds);
  rc = initialize_adapter_info(adapter_info);

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


  swapped_long = ((old_long & 0x000000ff) << 24) |
	    ((old_long & 0x0000ff00) << 8)  |
	    ((old_long & 0x00ff0000) >> 8)  |
	    ((old_long & 0xff000000) >> 24);

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
  char            errbuf[256];
  error_details   *errs;

  /***********************************
   *  Get Access to Error Information
  ***********************************/
  errs = tucb_ptr->errinfo;

  /**************************************
   *  Determine Prefix for Error Message
   **************************************/
  switch (tunum) {
	case TU_OPEN:     
		strcpy(errbuf, INIT_PREFIX);
		break;

	case POS_TEST:    
		strcpy(errbuf, POS_PREFIX);
		break;

	case IO_TEST:     
		strcpy(errbuf, IO_PREFIX);
		break;

	case ONCARD_TEST: 
		strcpy(errbuf, ONCARD_PREFIX);
		break;

	case CONNECT_TEST: 
		strcpy(errbuf, CONNECT_PREFIX);
		break;

	case INT_WRAP_TEST: 
		strcpy(errbuf, INT_WRAP_PREFIX);
		break;

	case EXT_WRAP_TEST: 
		strcpy(errbuf, EXT_WRAP_PREFIX);
		break;

	case NETWORK_TEST: 
		strcpy(errbuf, NETWORK_PREFIX);
		break;

	case TU_CLOSE:    
		strcpy(errbuf, TERM_PREFIX);
		break;

	default:          
		strcpy(errbuf, DEFAULT_PREFIX);
		break;
  }

  /********************
   *  Check Error Type
   ********************/
  switch (errtype)
	{
	case REGISTER_ERR:   
		if (errcode & READ_ERR) {
			strcat(errbuf, READ_ERR_MSG);
		} else if (errcode & WRITE_ERR) {
			strcat(errbuf, WRITE_ERR_MSG);
		} else {
			strcat(errbuf, COMPARE_ERR_MSG);
		}

		errs->un.reg.bad_address    = val1;
		errs->un.reg.expected_value = val2;
		errs->un.reg.actual_value   = val3;
		break;

	case ADAP_INT_ERR:
	case ADAP_INIT_ERR:  
		strcat(errbuf, ADAPTER_ERR_MSG);
		errs->un.adapter.hrc = val1;
		errs->un.adapter.mrc = val2;
		break;

	case CLEANUP_ERR:    
		strcat(errbuf, CLEANUP_ERR_MSG);
		break;

	case CLOSE_ERR:      
		strcat(errbuf, CLOSE_ERR_MSG);
		break;

	case FILE_ERR:       
		strcat(errbuf, FILE_ERR_MSG);
		break;

	case FRAME_LEN_ERR:  
		strcat(errbuf, FRAME_LEN_ERR_MSG);
		break;

	case HARD_ERR:       
		strcat(errbuf, HARD_ERR_MSG);
		break;

	case SOFT_ERR:       
		strcat(errbuf, SOFT_ERR_MSG);
		break;

	case BEACON_ERR:     
		strcat(errbuf, BEACON_ERR_MSG);
		break;

	case AUTO_ERR:       
		strcat(errbuf, AUTO_ERR_MSG);
		break;

	case REMOVE_ERR:     
		strcat(errbuf, REMOVE_ERR_MSG);
		break;

	case LOBE_ERR:       
		strcat(errbuf, LOBE_ERR_MSG);
		break;

	case OPEN_ERR:       
		strcat(errbuf, OPEN_ERR_MSG);
		errs->un.sys.return_code   = val1;
		break;

	case STARTUP_ERR:    
		strcat(errbuf, STARTUP_ERR_MSG);
		break;

	case SIGNAL_ERR:     
		strcat(errbuf, SIGNAL_ERR_MSG);
		break;

	case SYSTEM_ERR:     
		strcat(errbuf, SYSTEM_ERR_MSG);
		break;

	case TIMEOUT_ERR:    
		strcat(errbuf, TIMEOUT_ERR_MSG);
		break;

	case WRAP_ERR:       
		strcat(errbuf, WRAP_ERR_MSG);
		errs->un.adapter.hrc = val1;
		errs->un.adapter.mrc = val2;
		break;

	default:             
		break;
  }

  /******************************************
   *  Set Remaining Error Information Values
   ******************************************/
  errs->error_type = errtype;
  errs->error_code = errcode;
  errs->tunum      = tunum;
  errs->error_msg  = (char *) malloc(strlen(errbuf));
  strcpy(errs->error_msg, errbuf);

  /***********************************
   *  Set the Error Info Structure
   ***********************************/
  tucb_ptr->errinfo = errs;

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

  /*****************************************
   *  Print Specific Error Type Information
   *****************************************/
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

