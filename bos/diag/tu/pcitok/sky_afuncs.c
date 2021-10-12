/*
 *   COMPONENT_NAME: tu_pcitok
 *
 *   FUNCTIONS: get_arb_cmd
 *		get_srb_response
 *		mps_clean_up
 *		mps_enable_channel
 *		network_test
 *		process_interrupts
 *		process_misr_cmd
 *		process_timeout
 *		reset_adapter
 *		wrap_test
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
#include <unistd.h>
#include <sys/types.h>
#include <sys/mdio.h>
#include <sys/sleep.h>
#include <sys/errno.h>
#include <sys/sysconfig.h>
#include <sys/times.h>
#include <cf.h>

/* local header files */
#include "skytu_type.h"
#include "sky_regs.h"


/****************************************************************************
*
* FUNCTION NAME =  reset_adapter()
*
* DESCRIPTION   =  This function resets the adapter by setting the soft
*                  reset bit in the Basic Control Register.
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
int reset_adapter (ADAPTER_STRUCT *adapter_info, TUTYPE *tucb_ptr, int ttype)
{
  int     rc = 0;
  ushort  data;

  /*****************************************
   *  Disable transmit and receive channels
   *  Ignore errors. The 
   *  device still needs to be cleaned up.
   ****************************************/
  if (tucb_ptr->header.adap_flag)
  {
  	(void)io_rd_swap(adapter_info, IOSHORT, BMCtl_SUM, &data);
	if (( (data & 0x2000) || (data & 0x0200) || (data & 0x0020)))
	{
  		(void)io_wr_swap(adapter_info, IOSHORT, BMCtl_RUM, 0x8880);
  		(void)diag_watch4intr(adapter_info->handle, MISR_INT, 
			TIMEOUT_LIMIT);
  		(void)process_interrupts(adapter_info,tucb_ptr, ttype,MISR_INT);
	}

  }
#ifdef DEBUG
  io_rd_swap(adapter_info, IOSHORT, BMCtl_SUM, &data);
#endif
  
  /***********************
   *  Set soft reset bit
   **********************/
  if (rc = io_wr_swap(adapter_info, IOSHORT, BCtl, 0x8000)) {
	return (error_handler(tucb_ptr, ttype, BCtl_ERR + WRITE_ERR,
		    REGISTER_ERR, BCtl, 0x8000, 0));
  }

  /*********************************
   *  Wait for completion of reset
   *********************************/
  usleep(800);

  /************************************************
   *  Restore  reset bit to allow normal operation
   ************************************************/
  if (rc = io_wr_swap(adapter_info, IOSHORT, BCtl, 0x0)) {
	return (error_handler(tucb_ptr, ttype, BCtl_ERR + WRITE_ERR,
		    REGISTER_ERR, BCtl, 0, 0));
  }

  /*********************************
   *  Wait for completion of reset
   *********************************/
  usleep(200);

  return(0);

} /* end of reset_adapter */



/**************************************************************************** *
*
* FUNCTION NAME =  get_srb_response
*
* DESCRIPTION   =  This function gets the SRB response from the adapter.
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*              tucb_ptr       - ptr to test unit control block
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR = error code
*
* INVOKED FROM = process_interrupts
*
*****************************************************************************/
int get_srb_response (ADAPTER_STRUCT *adapter_info, TUTYPE *tucb_ptr, int ttype)
{
  ushort  i, data[25];
  ushort  command, status;
  int     rc = 0;
  int     hrc = 0,
  mrc = 0;


  /************************
   *  Set up LAP registers
   ************************/
  if (rc = io_wr_swap(adapter_info, IOSHORT, LAPE, 0x00)) {
  	return (error_handler(tucb_ptr, ttype, LAPE_ERR + WRITE_ERR,
		    REGISTER_ERR, LAPE, 0, 0));
	}

  if (rc = io_wr_swap(adapter_info, IOSHORT, LAPA, adapter_info->srb_address)) {
	return (error_handler(tucb_ptr, ttype, LAPA_ERR + WRITE_ERR,
		    REGISTER_ERR, LAPA, adapter_info->srb_address, 0));
  }

  /**********************************
  *  Get Initialization Information
  **********************************/
  for (i=0; i < 10; i++) {
	if ( rc = io_rd(adapter_info, IOSHORT, LAPDInc, &data[i])) {
		return (error_handler(tucb_ptr, ttype, LAPDInc_ERR + READ_ERR,
			    REGISTER_ERR, LAPDInc, 0, 0));
	}
  }
  DEBUG_0(" : SRB_RESP \n");

  command = data[0] >> 8;
  if (command == 0) {
  	return (command);
  }

  status = data[1] >> 8;

  switch (command) {
	case INITIALIZATION_COMPLETE :
		DEBUG_0("SRB command = INITIALIZATION_COMPLETE\n");
		if (rc = get_diagnostic_codes(adapter_info, tucb_ptr, ttype,
		    &hrc, &mrc)) {
			return(rc);
		}

		/* Initialization status */
		switch (data[3]) {
		case INIT_SUCCESSFUL :
			DEBUG_0("Initialization completed successfully\n");
			rc = get_adapter_address(adapter_info, tucb_ptr, ttype, 
			    data[4]);
			if (rc) return(rc);
			break;

		case PROCESSOR_INIT_ERR :
			DEBUG_0("Processor Initialization Failed\n");
			rc = data[3];
			return(rc);
			break;

		case ROS_TEST_ERR :
			DEBUG_0("ROS Test Diagnostic  Failed\n");
			rc = data[3];
			return(rc);
			break;

		case RAM_TEST_ERR :
			DEBUG_0("RAM Test Diagnostic  Failed\n");
			rc = data[3];
			return(rc);
			break;

		case INSTRUCTION_TEST_ERR :
			DEBUG_0("Instruction Test Diagnostic Failed\n");
			rc = data[3];
			return(rc);
			break;

		case INTERRUPT_TEST_ERR  :
			DEBUG_0("Interrupt test Diagnostic Failed\n");
			rc = data[3];
			return(rc);
			break;

		case MEMORY_INTERFACE_ERR  :
			DEBUG_0("Memory Interface HW Diagnostic Failed\n");
			rc = data[3];
			return(rc);
			break;

		case TOKEN_RING_HANDLER_ERR :
			DEBUG_0("T R Protocol Handler Diagnostic Failed\n");
			rc = data[3];
			return(rc);
			break;

		case CHANNEL_TEST_ERR :
			DEBUG_0("Rx/Tx Channel Test failed Failed\n");
			rc = data[3];
			return(rc);
			break;

		case ADDRESS_MATCH_RAM_ERR :
			DEBUG_0("Address Match RAM Failed\n");
			rc = data[3];
			return(rc);
			break;

		case ADDRESS_MATCH_CAM_ERR :
			DEBUG_0("Address Match CAM Failed\n");
			rc = data[3];
			return(rc);
			break;

		case ETHERNET_MAC_ERR :
			DEBUG_0("Ethernet MAC Loopback Test Failed\n");
			rc = data[3];
			return(rc);
			break;

		case ETHERNET_ENDEC_ERR :
			DEBUG_0("Ethernet ENDEC Loopback Test Failed\n");
			rc = data[3];
			return(rc);
			break;

		case ETHERNET_TRANSCEIVER_ERR :
			DEBUG_0("Ethernet Transceiver Test Failed\n");
			rc = data[3];
			return(rc);
			break;

		case ETHERNET_HANDLER_ERR :
			DEBUG_0("Ethernet Protocol Handler Test Failed\n");
			rc = data[3];
			return(rc);
			break;

		default :
			DEBUG_1("Unknown Initialization Error Code = %x\n",
			    data[3]);
			rc = data[3];
			return(rc);
			break;

		} /* end of init status switch */

		DEBUG_0("GET MICROCODE LEVEL\n");
  if (rc = io_wr_swap(adapter_info, IOSHORT, LAPE, 0x00)) {
  	return (error_handler(tucb_ptr, ttype, LAPE_ERR + WRITE_ERR,
		    REGISTER_ERR, LAPE, 0, 0));
  }

  if (rc = io_wr_swap(adapter_info, IOSHORT, LAPA, data[5])) {
	return (error_handler(tucb_ptr, ttype, LAPA_ERR + WRITE_ERR,
		    REGISTER_ERR, LAPA, adapter_info->srb_address, 0));
  }

  /**********************************
  *  Get Initialization Information
  **********************************/
  for (i=0; i < 5; i++) {
	if ( rc = io_rd(adapter_info, IOSHORT, LAPDInc, &data[i])) {
		return (error_handler(tucb_ptr, ttype, LAPDInc_ERR + READ_ERR,
			    REGISTER_ERR, LAPDInc, 0, 0));
	}
  }

		if (rc) {
			return (error_handler(tucb_ptr, ttype, rc,
			    ADAP_INIT_ERR, hrc, mrc, 0));
		}
		break;

	case OPEN_ADAPTER :
		DEBUG_0("Command called : OPEN_ADAPTER \n");
		if (status == OPERATION_SUCCESSFUL) {
			adapter_info->asb_address = data[4];
			adapter_info->arb_address = data[6];
			adapter_info->trb_address = data[8];

/* Connect test is exited after the open.  There is no need to setup
   the receive descriptors after the open is complete.
*/
			if (ttype != CONNECT_TEST) {
				recv_setup(adapter_info);
				/* Enable the Rx channel */
				mps_enable_channel(adapter_info);
			}
			adapter_info->opened = TRUE;

			rc = 0;
		} else {
			return (error_handler(tucb_ptr, ttype, status,
			    OPEN_ERR, data[3] & 0xFF , 0, 0));
		}
		break;

	default:
		return (error_handler(tucb_ptr, ttype, SRB_CMD_ERR,
		    SRB_CMD_ERR, command, 0, 0));
		break;

  } /* end switch */

  return(0);

} /* end get_srb_response */


/******************************************************************************
*
* FUNCTION NAME =  get_arb_cmd
*
* DESCRIPTION   =  This function gets the ARB command from the adapter.
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*              tucb_ptr       - ptr to test unit control block
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR = error code
*
* INVOKED FROM = process_interrupts
*
*****************************************************************************/
int get_arb_cmd (ADAPTER_STRUCT *adapter_info, TUTYPE *tucb_ptr, int ttype)
{
  ushort  i, data[10];
  ushort  command, status;
  int     rc = 0;

  /************************
   *  Set up LAP registers
   ************************/
  if (rc = io_wr_swap(adapter_info, IOSHORT, LAPE, 0x00)) {
	return (error_handler(tucb_ptr, ttype, LAPE_ERR + WRITE_ERR,
		    REGISTER_ERR, LAPE, 0, 0));
  }

  if (rc = io_wr_swap(adapter_info, IOSHORT, LAPA, adapter_info->arb_address)) {
	return (error_handler(tucb_ptr, ttype, LAPA_ERR + WRITE_ERR,
		    REGISTER_ERR, LAPA, adapter_info->arb_address, 0));
  }

  /**********************************
  *  Get Initialization Information
  **********************************/
  for (i=0; i < 6; i++) {
	if ( rc = io_rd(adapter_info, IOSHORT, LAPDInc, &data[i])) {
		return (error_handler(tucb_ptr, ttype, LAPDInc_ERR + READ_ERR,
			    REGISTER_ERR, LAPDInc, 0, 0));
	}

  }
  DEBUG_0(" : ARB CMD \n");

  command = data[0] >> 8;

  DEBUG_1(" ARB Command = %x\n", command);

  switch (command) {
	case RECEIVE_DATA :
		rc = 0;
		DEBUG_0(" : RECEIVE_DATA \n");
		break;

	case LAN_STATUS_CHANGE:
		DEBUG_1(" : LAN Status = %x\n", data[3]);

		if (data[3] & SIGNAL_LOSS)
			error_handler(tucb_ptr, ttype,
			    SIGNAL_ERR, SIGNAL_ERR,
			    0, 0, 0);

		if (data[3] & HARD_ERROR)
			error_handler(tucb_ptr, ttype,
			    HARD_ERR, HARD_ERR,
			    0, 0, 0);

		if (data[3] & TRANSMIT_BEACON)
			error_handler(tucb_ptr, ttype,
			    BEACON_ERR, BEACON_ERR,
			    0, 0, 0);

		if (data[3] & LOBE_WIRE_FAULT)
			error_handler(tucb_ptr, ttype,
			    LOBE_ERR, LOBE_ERR,
			    0, 0, 0);

		if (data[3] & AUTO_REMOVAL_ERROR)
			error_handler(tucb_ptr, ttype,
			    AUTO_ERR, AUTO_ERR,
			    0, 0, 0);

		if (data[3] & REMOVE_RECEIVE)
			error_handler(tucb_ptr, ttype,
			    REMOVE_ERR, REMOVE_ERR,
			    0, 0, 0);


		if ((data[3] & SIGNAL_LOSS)         |
		    (data[3] & HARD_ERROR)          |
		    (data[3] & TRANSMIT_BEACON)     |
		    (data[3] & LOBE_WIRE_FAULT)     |
		    (data[3] & AUTO_REMOVAL_ERROR)  |
		    (data[3] & REMOVE_ERR))
		{
			return (1);
		}
		else
			return (0);
		break;


	case SEND_FLASH_DATA:
		rc = 0;
		break;

	default:
		rc = 0;
		break;

  } /* end switch */
  return(0);

} /* end get_arb_cmd */


/******************************************************************************
*
* FUNCTION NAME =  process_misr_cmd
*
* DESCRIPTION   =  This function processes a MISR interrupt command.
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*              tucb_ptr       - ptr to test unit control block
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR = error code
*
* INVOKED FROM = process_interrupts
*
*****************************************************************************/
int process_misr_cmd (ADAPTER_STRUCT *adapter_info, TUTYPE *tucb_ptr,
int ttype)
{
  ushort  command;
  int     rc = 0;
  int     tx_len, rx_len;

  command = adapter_info->cmd_reg;
  DEBUG_1("Processing misr cmd %x\n",command);

/* 
   Tx2_NOST and Rx_NOSTA are not checked here because of a known bug
   in the hardware.  It is possible for no status to be reported but
   the transmit completed successfully.

   Rx_NBA means there are no more available receive buffers for the
   next packet to come in.  This means that the next packet will be
   dropped.  In the controled test unit environment this is not a
   problem.  This is only a means of telling the host software to 
   clean up the receive buffers. It is not an error for the current
   buffer.
*/

  if (command & XMIT_DONE_MSK_2) {
	if ((command & Tx2_HALT) || (command & Tx2_EOF)) {
		if (rc = mps_tx_done(adapter_info)) {
			tx_len=strlen(adapter_info->transmit_string);
			rx_len = strlen(adapter_info->receive_string);
			return (error_handler(tucb_ptr, ttype, rc, 
			    WRAP_ERR, tx_len, rx_len, 0));
		}
	}
  }

  if (command & RECEIVE_MSK) {
	if ((command & Rx_EOF) || (command & Rx_EOB) || (command & Rx_HALT)) {
		if (rc = mps_recv(adapter_info)) {
			tx_len=strlen(adapter_info->transmit_string);
			rx_len = strlen(adapter_info->receive_string);
			return (error_handler(tucb_ptr, ttype, rc, 
			    WRAP_ERR, tx_len, rx_len, 0));
		}
	}
  }

  return(rc);


}  /* process_misr_cmd */


/******************************************************************************
*
* FUNCTION NAME =  process_timeout
*
* DESCRIPTION   =  This function determines what caused a time out
*                  interrupt to occur and sets the appropriate error
*                  code.
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*              tucb_ptr       - ptr to test unit control block
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR = error code
*
* INVOKED FROM = process_interrupts
*
*****************************************************************************/
int process_timeout (ADAPTER_STRUCT *adapter_info, TUTYPE *tucb_ptr, int ttype,
	int intr_mask)
{
  int     rc = 0;

  if (adapter_info->num_interrupts != 0)
  {
	rc = process_interrupts(adapter_info, tucb_ptr, ttype, intr_mask);
	return (rc);
  }

  switch (adapter_info->wd_setter) {
	case WDT_INIT:
		return (error_handler(tucb_ptr, ttype, WDT_INIT,
		    			TIMEOUT_ERR, 0, 0, 0));
		break;

	case WDT_OPEN:
		error_handler(tucb_ptr, ttype, WDT_OPEN, TIMEOUT_ERR, 0, 0, 0);
        if (rc = process_interrupts(adapter_info, tucb_ptr, ttype, intr_mask)) {
                return(rc);
        }
		error_handler(tucb_ptr, ttype, WDT_OPEN, WRITE_ERR_MSG, 0, 0,0);
		break;

	case WDT_XMIT:
		return (error_handler(tucb_ptr, ttype, WDT_XMIT,
		    			TIMEOUT_ERR, 0, 0, 0));
		break;

	default:
		return (error_handler(tucb_ptr, ttype, 0x999,
		    			TIMEOUT_ERR, 0, 0, 0));
		break;

  } /* end switch */

} /* end process_timeout */


/**************************************************************************** *
*
* FUNCTION NAME =  process_interrupts
*
* DESCRIPTION   =  This function processes interrupts
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*              tucb_ptr       - ptr to test unit control block
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR = error code
*
* INVOKED FROM =
*
*****************************************************************************/
int process_interrupts(ADAPTER_STRUCT *adapter_info,TUTYPE *tucb_ptr,int ttype,
	int intr_mask)
{
  ushort  i = 0;
  int     rc = 0;
  ushort data;

/* Disable interrupts */
  if (rc = io_wr_swap(adapter_info,IOSHORT,SISRM_RUM, 0x7fff)) {
	return(WRITE_ERR);
  }
  DEBUG_1("Processing interrupt, num interrupts = %d\n",
	adapter_info->num_interrupts);
  for (i = 0; i < adapter_info->num_interrupts; i++) {
  	switch (adapter_info->int_type[i]) {
		case ARB_CMD:
			if (rc = get_arb_cmd(adapter_info, tucb_ptr, ttype)) {
				adapter_info->int_pending = 0;
  				adapter_info->num_interrupts = 0;
  				adapter_info->cmd_reg = 0;
				return(rc);
			}
			break;

		case MISR_INT:
			if (rc = process_misr_cmd(adapter_info,tucb_ptr,ttype)){
				adapter_info->int_pending = 0;
  				adapter_info->num_interrupts = 0;
  				adapter_info->cmd_reg = 0;
				return(rc);
			}
			break;

		case SRB_RSP:
			if (rc = get_srb_response(adapter_info,tucb_ptr,ttype)){
				adapter_info->int_pending = 0;
  				adapter_info->num_interrupts = 0;
  				adapter_info->cmd_reg = 0;
				return(rc);
			}
			break;

		default:
			adapter_info->int_pending = 0;
  			adapter_info->num_interrupts = 0;
  			adapter_info->cmd_reg = 0;
			return (error_handler(tucb_ptr, ttype,
		    		BAD_INTERRUPT,
		    		ADAP_INT_ERR,
		    		adapter_info->int_type[i],
		    		0, 0));
			break;
  	} /* end switch */
  }	
  adapter_info->int_pending = 0;
  adapter_info->num_interrupts = 0;
  adapter_info->cmd_reg = 0;

  if (rc = io_wr_swap(adapter_info,IOSHORT,SISRM, SISR_MSK)) {
	return(WRITE_ERR);
  }
  DEBUG_1("num interrupts = %d\n",adapter_info->num_interrupts);
  return(0);

} /* end process_interrupts */


/**************************************************************************** *
*
* FUNCTION NAME =  wrap_test
*
* DESCRIPTION   =  This function performs an external wrap test
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*              tucb_ptr       - ptr to test unit control block
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   error code
*
* INVOKED FROM = tu005, tu006
*
*****************************************************************************/
int wrap_test (ADAPTER_STRUCT *adapter_info, TUTYPE *tucb_ptr, int ttype)
{
  int         rc1 = 0,
  rc2 = 0;
  int         retries;
  /*********************
   * Reset adapter
   *********************/

  if (rc1 = reset_adapter(adapter_info, tucb_ptr, ttype)) {
	return (rc1);
  }
  /***************************
   *  Initialize the Adapter
   ***************************/
  if (rc1 = initialize_adapter(adapter_info, tucb_ptr, ttype)) {
	tucb_ptr->header.adap_flag = FALSE;
	return(rc1);
  }
  /*******************
   * Open the adapter
   *******************/
  if (rc1 = open_adapter(adapter_info, tucb_ptr, ttype)) {
	DEBUG_1("return from open_adapter = %x\n",rc1);
	if (tucb_ptr->header.adap_flag == TRUE) {
		mps_clean_up(adapter_info, tucb_ptr, ttype);
	}
	return(rc1);
  }

  DEBUG_0("setting strings\n");
  /*********************************
   * Set the wrap strings for test
   *********************************/
  if (rc1 = set_strings(adapter_info, tucb_ptr, ttype)) {
	mps_clean_up(adapter_info, tucb_ptr, ttype);
	return(rc1);
  }

  DEBUG_0("Calling transmit data \n");
  /****************************
   * Transmit and receive data
   ****************************/
  if (rc1 = transmit_data(adapter_info, tucb_ptr, ttype)) {
	mps_clean_up(adapter_info, tucb_ptr, ttype);
	return(rc1);
  }

  /***************************************************
   *  Clean up resources allocated for initialization
   ***************************************************/
  if (tucb_ptr->header.adap_flag == TRUE) {
  	rc2 = mps_clean_up(adapter_info, tucb_ptr, ttype);
  }

  /**************************************************
   * Determine which, if any, return code to return.
   **************************************************/
  if (rc2) {
	return (error_handler(tucb_ptr, ttype, rc2, CLEANUP_ERR, 0, 0, 0));
  } else {
	return(0);
  }

} /* end wrap_test */


/**************************************************************************** *
*
* FUNCTION NAME =  network_test
*
* DESCRIPTION   =  This function performs an external network wrap test
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*              tucb_ptr       - ptr to test unit control block
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   error code
*
* INVOKED FROM = tu007
*
*****************************************************************************/
int network_test (ADAPTER_STRUCT *adapter_info, TUTYPE *tucb_ptr, int ttype)
{
  int         rc1 = 0,
  rc2 = 0;
  int         retries;
  int         trans_len, rec_len;

  /*********************
   * Reset adapter
   *********************/

  if (rc1 = reset_adapter(adapter_info, tucb_ptr, ttype)) {
	return (rc1);
  }

  /***************************
   *  Initialize the Adapter
   ***************************/
  if (rc1 = initialize_adapter(adapter_info, tucb_ptr, ttype)) {
  	tucb_ptr->header.adap_flag = FALSE;
  	return(rc1);
  }

  /*******************
   * Open the adapter
   *******************/
  if (rc1 = open_adapter(adapter_info, tucb_ptr, ttype)) {
  	mps_clean_up(adapter_info, tucb_ptr, ttype);
  	return(rc1);
  }

  /************************************************************
  *     Watch for interrupt to indicate lobe wire fault
  ************************************************************/
  DEBUG_0("Waiting for lobe interrupt\n");
/*  sleep(20);*/
  rc1 = diag_watch4intr(adapter_info->handle, ARB_CMD, TIMEOUT_LIMIT);
  if (rc1 == 0) {
  	rc1 = process_interrupts(adapter_info, tucb_ptr, ttype, ARB_CMD);
  	if (rc1) {
		DEBUG_0("process interrupts failed\n");
  		mps_clean_up(adapter_info, tucb_ptr, ttype);
  		return(rc1);
  	}
  }
  else {
	rc1 = process_timeout(adapter_info, tucb_ptr, ttype, ARB_CMD);
	if (rc1) {
  		mps_clean_up(adapter_info, tucb_ptr, ttype);
  		return(rc1);
  	}
  }

  /*********************************
   * Set the wrap strings for test
   *********************************/
  if (rc1 = set_strings(adapter_info, tucb_ptr, ttype)) {
  	mps_clean_up(adapter_info, tucb_ptr, ttype);
  	return(rc1);
  }

  /****************************
   * Transmit and receive data
   ****************************/
  if (rc1 = transmit_data(adapter_info, tucb_ptr, ttype)) {
	DEBUG_0("transmit data failed\n");
  	mps_clean_up(adapter_info, tucb_ptr, ttype);
  }

  /***************************************************
   *  Clean up resources allocated for initialization
   ***************************************************/
  if (tucb_ptr->header.adap_flag == TRUE) {
  	rc2 = mps_clean_up(adapter_info, tucb_ptr, ttype);
  }
  tucb_ptr->header.adap_flag = FALSE;  /* Set flag for HTX signal handler */

  /**************************************************
   * Determine which, if any, return code to return.
   **************************************************/
  if (rc1) 
  	return(rc1);
  else if (rc2) 
  	return (error_handler(tucb_ptr, ttype, rc2, CLEANUP_ERR, 0, 0, 0));
  else 
	return(0);


} /* end network test */

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
* INVOKED FROM = oncard_test, initialize_adapter, wrap_test, network_test,
*                connect_test
*
*****************************************************************************/

int mps_clean_up(ADAPTER_STRUCT	*adapter_info, TUTYPE *tucb_ptr, int ttype)
{
  int j, rc = 0, rc1 = 0 , rc2 = 0, rc3 = 0, rc4 = 0;
  int fail = FALSE;

  DEBUG_0("Starting cleanup\n");

  if (rc = reset_adapter(adapter_info, tucb_ptr, ttype)) {
	return (rc);
  }

  if (tucb_ptr->header.adap_flag == FALSE) return(0);

  DEBUG_1("ADAPT FLAG = %d\n",tucb_ptr->header.adap_flag);
  tucb_ptr->header.adap_flag = FALSE;  /* Set flag for HTX signal handler */
  for (j = 0; j < MAX_RX_LIST; j++){
  	rc1 = diag_unmap_page(adapter_info->handle, 
		&adapter_info->recv_dma_addr[j]);
	if (rc1) fail = TRUE; 
  DEBUG_1("Freeing rx buff, %x\n",adapter_info->recv_usr_buf[j]);
	free(adapter_info->recv_usr_buf[j]);
	rc2 = diag_unmap_page(adapter_info->handle, 
		&adapter_info->recv_desc_addr[j]);
	if (rc2) fail = TRUE; 
  DEBUG_0("Freeing rx desc\n");
	free(adapter_info->recv_usr_desc[j]);
  }

  /* Set up variable for Transmit list */

  for (j = 0; j < MAX_TX_LIST; j++){ 
  	rc3 = diag_unmap_page(adapter_info->handle, 
		&adapter_info->xmit_dma_addr[j]);
	if (rc3) fail = TRUE; 
  DEBUG_0("Freeing tx buff\n");
  	free(adapter_info->xmit_usr_buf[j]);
	rc4 = diag_unmap_page(adapter_info->handle, 
		&adapter_info->xmit_desc_addr[j]);
	if (rc4) fail = TRUE; 
  DEBUG_0("Freeing tx desc\n");
	free(adapter_info->xmit_usr_desc[j]);
  }

  if (fail)
  {
	rc=(rc1) ? rc1 : (rc2) ? rc2 : (rc3) ? rc3 : (rc4) ? rc4 : CLEANUP_ERR;
	return(rc); 
  }
  return(0);
}

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
* INVOKED FROM = get_srb_response
*
*****************************************************************************/
int mps_enable_channel (ADAPTER_STRUCT *adapter_info)
{
  uint                Addr;
  ulong		      data;
  /**************************
  * Enable the Rx and Tx2 Channels
  **************************/
  io_wr_swap(adapter_info,IOSHORT,BMCtl_RUM, 0x0000);

  /************************************************
  * Give the buffer descriptor address to adapter
  ************************************************/

  Addr    = (uint)adapter_info->recv_desc_addr[0];
  io_wr_swap(adapter_info,IOLONG,RxBDA_LO, Addr);
  io_wr_swap(adapter_info,IOLONG,RxLBDA_LO, Addr);
  io_rd_swap(adapter_info,IOLONG,RxBDA_LO,&data);
  io_rd_swap(adapter_info,IOLONG,RxLBDA_LO,&data);
  return(0);

} /* end mps_enable_channel */
