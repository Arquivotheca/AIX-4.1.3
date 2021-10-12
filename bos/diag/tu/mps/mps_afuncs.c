static char sccsid[] = "@(#)23  1.5  src/bos/diag/tu/mps/mps_afuncs.c, tu_mps, bos41J, 9523A_all 6/5/95 11:23:24";
/*****************************************************************************
 * COMPONENT_NAME: (tu_mps)  Wildwood LAN adapter test units
 *
 * FUNCTIONS:   int set_up_adapter()
 *              int reset_adapter()
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
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mdio.h>
#include <sys/sleep.h>
#include <sys/errno.h>
#include <sys/sysconfig.h>
#include <cf.h>

/* local header files */
#include "mpstu_type.h"
#include "mps_regs.h"

/****************************************************************************
*
* FUNCTION NAME =  set_up_adapter()
*
* DESCRIPTION   =  This function sets up the adapter by updating
*                  the POS registers with default information obtained
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
  uchar          BC_Reg_LO, BC_Reg_HI, BM_Reg_LO, BM_Reg_HI;
  uint           int_bits, int_level, rom_bits, ring_bit;
  int            rc = 0;
  mps_dds        dds;

  /***********************************************************
   *    Get access to ODM Information through dds pointer    *
   ***********************************************************/
 dds = adapter_info->dds;

 /******************************
  *  Set Interrupt Level POS3  *
  ******************************/
 switch(dds.bus_intr_lvl) {
	case 0x09:
		int_bits = 0x00;
		break;

	case 0x03:
		int_bits = 0x10;
		break;

	case 0x0b:
		int_bits = 0x30;
		break;

	default:
		int_bits = 0x20;
		break;
  }

  /**************************************
  *  Set Ring Speed Bit w/Auto-sense  *
  **************************************/
  if (dds.ring_speed == 0) {
  	ring_bit = 0x0;  /* 4 Mbs */
  } else {
  	if (dds.ring_speed == 1) {
  		ring_bit = 0x80;  /* 16 Mbs */
  	} else {
  		ring_bit = 0x40;  /* Autosense */
  	}
  }

  /************************************************************
   *     Set up High Byte of Basic Configuration Register     *
   ************************************************************/
  BC_Reg_HI = ring_bit + int_bits;

  if (rc = pos_wr(adapter_info, POS3, BC_Reg_HI)) {
	return (error_handler(tucb_ptr, ttype, POS3_ERR + WRITE_ERR,
		    REGISTER_ERR, POS3, BC_Reg_HI, 0));
  }

  /*****************************************************
   *     Set up Bus Master Configuration Register POS4 *
   *****************************************************/
  BM_Reg_LO = 0x85;

  if (rc = pos_wr(adapter_info, POS4, BM_Reg_LO)) {
	return (error_handler(tucb_ptr, ttype, POS4_ERR + WRITE_ERR,
		    REGISTER_ERR, POS4, BM_Reg_LO, 0));
  }

  BM_Reg_HI = 0xe0 | dds.dma_lvl;

  if (rc = pos_wr(adapter_info, POS5, BM_Reg_HI)) {
	return (error_handler(tucb_ptr, ttype, POS5_ERR + WRITE_ERR,
		    REGISTER_ERR, POS5, BM_Reg_HI, 0));
  }

  /************************************************************
   *     Set up Low Byte of Basic Configuration Register      *
   ************************************************************/
  BC_Reg_LO  = 0x81 | (dds.bus_io_addr >> 9);

  if (rc = pos_wr(adapter_info, POS2, BC_Reg_LO)) {
	return (error_handler(tucb_ptr, ttype, POS2_ERR + WRITE_ERR,
		    REGISTER_ERR, POS2, BC_Reg_LO, 0));
  }

  return(0);

} /* end of set_up_adapter */

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
  int     rc = 0, i;
  ulong   dbas, dba;
  ushort  bmctl;

  if (rc = io_wr_swap(adapter_info, IOSHORT, BMCtl_SUM, 0x8880)) {
	return (error_handler(tucb_ptr, ttype, BMCtl_SUM_ERR + WRITE_ERR,
		    REGISTER_ERR, BMCtl_SUM, 0x8880, 0));
  }

  /* make sure DBA of Tx1 is not moving */
  i = 0;
  do {
          if (i++ > 20) {
                   break;
           }
           dbas = dba;
  	   usleep(200);
  	   if ( rc = io_rd_swap(adapter_info, IOLONG, Tx1DBA_LO, &dba)) {
        	return (error_handler(tucb_ptr, ttype, Tx1DBA_LO_ERR + READ_ERR,
                    REGISTER_ERR, Tx1DBA_LO, 0, 0 ));
  	   }
  } while (dbas != dba);

  /* make sure DBA of Tx2 is not moving */
  i = 0;
  do {
          if (i++ > 20) {
                   break;
           }
           dbas = dba;
  	   usleep(200);
  	   if ( rc = io_rd_swap(adapter_info, IOLONG, Tx2DBA_LO, &dba)) {
        	return (error_handler(tucb_ptr, ttype, Tx2DBA_LO_ERR + READ_ERR,
                    REGISTER_ERR, Tx2DBA_LO, 0, 0 ));
  	   }
  } while (dbas != dba);

  /* check for DMA disable */
  i = 0;
  do {
        if (i++ > 20) {
                break;
        }

  	usleep(2);
  	if ( rc = io_rd_swap(adapter_info, IOSHORT, BMCtl_SUM, &bmctl)) {
        	return (error_handler(tucb_ptr, ttype, BMCtl_SUM_ERR + READ_ERR,
                    REGISTER_ERR, BMCtl_SUM, 0, 0 ));
  	}
  } while ((bmctl != 4440) & (bmctl != 0x44A0));

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
  usleep(200);

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

	DEBUG_1("%04x ",  data[i]);
  }
  DEBUG_0(" : SRB_RESP \n");
  sleep(1);

  command = data[0] >> 8;
  if (command == 0) {
  	return (command);
  }

  status = data[1] >> 8;

  switch (command) {
	case INITIALIZATION_COMPLETE :
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
			break;

		case PROCESSOR_INIT_ERR :
			DEBUG_0("Processor Initialization Failed\n");
			rc = data[3];
			break;

		case ROS_TEST_ERR :
			DEBUG_0("ROS Test Diagnostic  Failed\n");
			rc = data[3];
			break;

		case RAM_TEST_ERR :
			DEBUG_0("RAM Test Diagnostic  Failed\n");
			rc = data[3];
			break;

		case INSTRUCTION_TEST_ERR :
			DEBUG_0("Instruction Test Diagnostic Failed\n");
			rc = data[3];
			break;

		case INTERRUPT_TEST_ERR  :
			DEBUG_0("Interrupt test Diagnostic Failed\n");
			rc = data[3];
			break;

		case MEMORY_INTERFACE_ERR  :
			DEBUG_0("Memory Interface HW Diagnostic Failed\n");
			rc = data[3];
			break;

		case TOKEN_RING_HANDLER_ERR :
			DEBUG_0("T R Protocol Handler Diagnostic Failed\n");
			rc = data[3];
			break;

		case CHANNEL_TEST_ERR :
			DEBUG_0("Rx/Tx Channel Test failed Failed\n");
			rc = data[3];
			break;

		case ADDRESS_MATCH_RAM_ERR :
			DEBUG_0("Address Match RAM Failed\n");
			rc = data[3];
			break;

		case ADDRESS_MATCH_CAM_ERR :
			DEBUG_0("Address Match CAM Failed\n");
			rc = data[3];
			break;

		case ETHERNET_MAC_ERR :
			DEBUG_0("Ethernet MAC Loopback Test Failed\n");
			rc = data[3];
			break;

		case ETHERNET_ENDEC_ERR :
			DEBUG_0("Ethernet ENDEC Loopback Test Failed\n");
			rc = data[3];
			break;

		case ETHERNET_TRANSCEIVER_ERR :
			DEBUG_0("Ethernet Transceiver Test Failed\n");
			rc = data[3];
			break;

		case ETHERNET_HANDLER_ERR :
			DEBUG_0("Ethernet Protocol Handler Test Failed\n");
			rc = data[3];
			break;

		default :
			DEBUG_1("Unknown Initialization Error Code = %x\n",
			    data[3]);
			rc = data[3];
			break;
		} /* end of init status switch */

		if (rc) {
			return (error_handler(tucb_ptr, ttype, rc,
			    ADAP_INIT_ERR, hrc, mrc, 0));
		}
		break;

	case OPEN_ADAPTER :
		DEBUG_0("Command called : OPEN_ADAPTER \n");
		if (status == OPERATION_SUCCESSFUL) {
			adapter_info->asb_address = data[4];
			adapter_info->srb_address = data[5];
			adapter_info->arb_address = data[6];
			adapter_info->trb_address = data[8];

			recv_setup(adapter_info);
			/* Enable the Rx channel */
			mps_enable_channel(adapter_info);

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

	DEBUG_1("%04x ",  data[i]);
  }
  DEBUG_0(" : ARB CMD \n");

  command = data[0] >> 8;

  DEBUG_1(" ARB Command = %x", command);

  switch (command) {
	case RECEIVE_DATA :
		rc = 0;
		DEBUG_0(" : RECEICE_DATA \n");
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

  adapter_info->cmd_reg = NULL;

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
	if ((command & Rx_EOF) || (command & Rx_EOB)) {
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
int process_timeout (ADAPTER_STRUCT *adapter_info, TUTYPE *tucb_ptr, int ttype)
{
  int     rc = 0;

  switch (adapter_info->wd_setter) {
	case WDT_INIT:
		return (error_handler(tucb_ptr, ttype, WDT_INIT,
		    			TIMEOUT_ERR, 0, 0, 0));
		break;

	case WDT_OPEN:
		return (error_handler(tucb_ptr, ttype, WDT_OPEN,
		    			TIMEOUT_ERR, 0, 0, 0));
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

  return(0);

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
int process_interrupts(ADAPTER_STRUCT *adapter_info,TUTYPE *tucb_ptr,int ttype)
{
  ushort  i = 0;
  int     rc = 0;

  /**********************************************
   *  Handle Any Interrupts that have occurred
   **********************************************/
  for (i = 0; i < adapter_info->num_interrupts; i++) {

  	switch (adapter_info->int_type[i]) {
		case ARB_CMD:
			if (rc = get_arb_cmd(adapter_info, tucb_ptr, ttype)) {
				return(rc);
			}
			break;

		case MISR_INT:
			if(rc = process_misr_cmd(adapter_info,tucb_ptr,ttype)){
				return(rc);
			}
			break;

		case SRB_RSP:
			if (rc = get_srb_response(adapter_info,tucb_ptr,ttype)){
				return(rc);
			}
			break;

		default:
			return (error_handler(tucb_ptr, ttype,
		    		BAD_INTERRUPT,
		    		ADAP_INT_ERR,
		    		adapter_info->int_type,
		    		0, 0));
			break;
  	} /* end switch */
  }
  adapter_info->num_interrupts = 0;

  return(0);

} /* end process_interrupts */


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
  int       retries, int_level;
  int       rc = 0;
  BOOLEAN   SUCCESS = FALSE;

  for (retries=0; !SUCCESS && (retries < 4); retries++) {
  	/*******************
         *   Reset Adapter
         *******************/
	if (rc = reset_adapter(adapter_info, tucb_ptr, ttype)) {
		return (rc);
	}

	sleep(5);

	/*******************************************************
         *  Set Channel Check Bit in Basic Control Register to
         *  override a logic error
         ********************************************************/
	if (rc = io_wr_swap(adapter_info, IOSHORT, BCtl, 0x2000)) {
		return (error_handler(tucb_ptr, ttype, BCtl_ERR + WRITE_ERR,
			    REGISTER_ERR, BCtl, 0x2000, 0));
	}

	/**********************
         *   Start Up Adapter
         **********************/
	adapter_info->wd_setter = WDT_INIT;
	if (rc = start_up_adapter(adapter_info)) {
		return (error_handler(tucb_ptr, ttype, rc, STARTUP_ERR,0,0,0));
	}

	tucb_ptr->header.adap_flag = TRUE;

	/************************************************************
         * Wait for interrupt to indicate diagnostics have completed
         ************************************************************/
	rc = diag_watch4intr(adapter_info->handle, SRB_RSP, TIMEOUT_LIMIT);

	/**************************************
         * Check for error from diag_watch4intr
         **************************************/
	if (rc) {
		if (retries >= 3) {
			return(process_timeout(adapter_info, tucb_ptr, ttype));
		}
		mps_clean_up(adapter_info);
		tucb_ptr->header.adap_flag = FALSE;
	} else {

		/********************************
        	 *  Get SRB address from LAPWWO
        	 ********************************/
		if (rc = io_rd_swap(adapter_info,IOSHORT,LAPWWO,&srb_address)) {
			return (error_handler(tucb_ptr, ttype, 
					      LAPWWO_ERR + READ_ERR,
			    	              REGISTER_ERR, LAPWWO, 0, 0));
		}

		adapter_info->srb_address = srb_address;

		/**************************
         	*  Process the interrupt
         	**************************/
		if (!(rc = process_interrupts(adapter_info, tucb_ptr, ttype))) {
			SUCCESS = TRUE;
		} else {
			if (retries >= 3) {
				return(rc);
			}

			mps_clean_up(adapter_info);
			tucb_ptr->header.adap_flag = FALSE;
		}
	}


  } /* end retry loop */

  return(0);

} /* end initialize_adapter */


/**************************************************************************** *
*
* FUNCTION NAME =  open_adapter
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
* INVOKED FROM =
*
*****************************************************************************/
int open_adapter (ADAPTER_STRUCT *adapter_info, TUTYPE *tucb_ptr, int ttype)
{
  int  int_level, rc = 0;
  int  pid;

  /*********************************
   * Issue the OPEN adapter command
   *********************************/
  adapter_info->wd_setter = WDT_OPEN;

  /*******************************************
   * Start a new process to initiate OPEN
   *******************************************/
  if ((pid = fork()) == 0) {
  	/* Make sure parent is watching for interrupt before it occurs */
  	sleep(1);
  	mps_open_adapter(adapter_info, ttype);
  	exit(0);
  } else {
  	/************************************************************
     	* Wait for interrupt to indicate open has completed
       	************************************************************/
  	rc=diag_watch4intr(adapter_info->handle, SRB_RSP, TIMEOUT_LIMIT);

  	/**************************************
       	 * Check for error from diag_watch4intr
       	 **************************************/
  	if (rc) {
  		return(process_timeout(adapter_info, tucb_ptr, ttype));
  	}

  	/**************************
       	 *  Process the interrupt
       	 **************************/
  	if (rc = process_interrupts(adapter_info, tucb_ptr, ttype)) {
  		return(rc);
  	}
  } /* end if */

  return(0);

} /* end open_adapter */


/**************************************************************************** *
*
* FUNCTION NAME =  transmit_data
*
* DESCRIPTION   =  This function transmits a packet of data
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
int transmit_data(ADAPTER_STRUCT *adapter_info, TUTYPE *tucb_ptr, int ttype)
{
  int         int_level, rc = 0;
  int         pid;

  /*******************************************
   * Identify action in case time out occurs.
   *******************************************/
  adapter_info->wd_setter = WDT_XMIT;

  /*******************************************
   * Start a new process to initiate transmit
   *******************************************/
  if ((pid = fork()) == 0) {
  	/* Make sure parent is watching for interrupt before it occurs */
  	sleep(1);
  	mps_fastwrt(adapter_info);
  	exit(0);
  } else {
  	/************************************************************
       * Watch for interrupt to indicate transmit has completed
       ************************************************************/
  	rc = diag_watch4intr(adapter_info->handle, MISR_INT, TIMEOUT_LIMIT);

      /**************************************
       * Check for error from diag_watch4intr
       **************************************/
  	if (rc) {
  		return (rc = re_transmit_data(adapter_info, tucb_ptr, ttype)); 
  	}

      /**************************
       *  Process the interrupt
       **************************/
  	if (rc = process_interrupts(adapter_info, tucb_ptr, ttype)) {
  		return(rc);
  	}
  }

  return(0);

} /* end transmit_data */



/**************************************************************************** *
*
* FUNCTION NAME =  re_transmit_data
*
* DESCRIPTION   =  This function transmits a packet of data
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
int re_transmit_data(ADAPTER_STRUCT *adapter_info, TUTYPE *tucb_ptr, int ttype)
{
  int         int_level, rc = 0;
  int         pid;

  /*******************************************
   * Identify action in case time out occurs.
   *******************************************/
  adapter_info->wd_setter = WDT_XMIT;

  /*******************************************
   * Start a new process to initiate transmit
   *******************************************/
  if ((pid = fork()) == 0) {
  	/* Make sure parent is watching for interrupt before it occurs */
  	sleep(1);
  	mps_fastwrt(adapter_info);
  	exit(0);
  } else {
  	/************************************************************
       * Watch for interrupt to indicate transmit has completed
       ************************************************************/
  	rc = diag_watch4intr(adapter_info->handle, MISR_INT, TIMEOUT_LIMIT);

      /**************************************
       * Check for error from diag_watch4intr
       **************************************/
  	if (rc) {
  		return(process_timeout(adapter_info, tucb_ptr, ttype));
  	}

      /**************************
       *  Process the interrupt
       **************************/
  	if (rc = process_interrupts(adapter_info, tucb_ptr, ttype)) {
  		return(rc);
  	}
  }

  return(0);

} /* end re_transmit_data */


/**************************************************************************** *
*
* FUNCTION NAME =  set_strings
*
* DESCRIPTION   =  This function sets the strings for the wrap tests
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
int set_strings(ADAPTER_STRUCT *adapter_info, TUTYPE *tucb_ptr, int ttype)
{
  int         frame_len, i, rc = 0;
  char        *test_string, ch;
  FILE        *fptr;

  /******************************************
   * Check if file specified for test string
   ******************************************/
  if ( (tucb_ptr->header.pattern != NULL) && 
      (strcmp(tucb_ptr->header.pattern, NO_FILE) != 0) ) {
  	if ((fptr = fopen(tucb_ptr->header.pattern, "r")) == NULL) {
  		return (error_handler(tucb_ptr, ttype, FILE_ERR,
			    FILE_ERR, 0, 0, 0));
	}

	/*****************************
         * Check for valid frame size
         *****************************/
	frame_len = tucb_ptr->header.pat_size;
	if ((frame_len < MIN_FRAME_LEN) || (frame_len > MAX_FRAME_LEN)) {
		fclose(fptr);
		return (error_handler(tucb_ptr, ttype, FRAME_LEN_ERR,
			    FRAME_LEN_ERR, 0, 0, 0));
	}

	/*******************
         * Fill test string
         *******************/
	test_string = (char *) malloc(frame_len);
	if (test_string == NULL) {
		fclose(fptr);
		return(ENOMEM);
	}

	ch = fgetc(fptr);
	for (i = 0; ((i < frame_len) && (ch != EOF)); i++) {
		*(test_string + i) = ch;
		ch = fgetc(fptr);
	} /* end for */
	fclose(fptr);

	/*****************************************************
         * Pad string to specified frame length, if necessary
         *****************************************************/
	for ( ;i < frame_len; i++) {
		*(test_string + i) = 'a';
	}

  } else {/* no pattern file specified, use default string */
	test_string = (char *) malloc(strlen(DEFAULT_TEST_STRING));
	strcpy(test_string, DEFAULT_TEST_STRING);
  }

  strcpy(adapter_info->transmit_string, test_string);

  /*******************
   * Free test string
   *******************/
  free(test_string);
  
  strcpy(adapter_info->receive_string, " ");

  return(0);

} /* end set_strings */


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

  /*****************************
   *   Set Up the MPS Adapter  *
   *****************************/
  if (rc1 = set_up_adapter(adapter_info, tucb_ptr, ttype)) {
	return (rc1);
  }

  /***************************
   *  Initialize the Adapter
   ***************************/
  if (rc1 = initialize_adapter(adapter_info, tucb_ptr, ttype)) {
	if (tucb_ptr->header.adap_flag == TRUE) {
		mps_clean_up(adapter_info);
		tucb_ptr->header.adap_flag = FALSE;
	}
	return(rc1);
  }
  /*******************
   * Open the adapter
   *******************/
  if (rc1 = open_adapter(adapter_info, tucb_ptr, ttype)) {
	if (tucb_ptr->header.adap_flag == TRUE) {
		mps_clean_up(adapter_info);
		tucb_ptr->header.adap_flag = FALSE;
	}
	return(rc1);
  }

  /*********************************
   * Set the wrap strings for test
   *********************************/
  if (rc1 = set_strings(adapter_info, tucb_ptr, ttype)) {
	mps_clean_up(adapter_info);
	tucb_ptr->header.adap_flag = FALSE;
	return(rc1);
  }

  /****************************
   * Transmit and receive data
   ****************************/
  if (rc1 = transmit_data(adapter_info, tucb_ptr, ttype)) {
	if (tucb_ptr->header.adap_flag == TRUE) {
		mps_clean_up(adapter_info);
		tucb_ptr->header.adap_flag = FALSE;
	}
	return(rc1);
  }

  /***************************************************
   *  Clean up resources allocated for initialization
   ***************************************************/
  rc2 = mps_clean_up(adapter_info);
  tucb_ptr->header.adap_flag = FALSE;     /* Set flag for HTX signal handler */

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

  /*****************************
   *   Set Up the MPS Adapter  *
   *****************************/
  if (rc1 = set_up_adapter(adapter_info, tucb_ptr, ttype)) {
		return (rc1);
  }

  /***************************
   *  Initialize the Adapter
   ***************************/
  if (rc1 = initialize_adapter(adapter_info, tucb_ptr, ttype)) {
  	if (tucb_ptr->header.adap_flag == TRUE) {
  		mps_clean_up(adapter_info);
  		tucb_ptr->header.adap_flag = FALSE;
  	}
  	return(rc1);
  }

  /*******************
   * Open the adapter
   *******************/
  if (rc1 = open_adapter(adapter_info, tucb_ptr, ttype)) {
  	if (tucb_ptr->header.adap_flag == TRUE) {
  		mps_clean_up(adapter_info);
  		tucb_ptr->header.adap_flag = FALSE;
  	}
  	return(rc1);
  }

  /************************************************************
  *     Watch for interrupt to indicate lobe wire fault
  sleep(20);
  rc1 = diag_watch4intr(adapter_info->handle, ARB_CMD, TIMEOUT_LIMIT);
  ************************************************************/
  rc1 = diag_watch4intr(adapter_info->handle, ARB_CMD, 30);

  if (rc1 == 0) {
  	if (rc1 = process_interrupts(adapter_info, tucb_ptr, ttype)) {
  		mps_clean_up(adapter_info);
  		tucb_ptr->header.adap_flag = FALSE;
  		return(rc1);
  	}
  }

  /*********************************
   * Set the wrap strings for test
   *********************************/
  if (rc1 = set_strings(adapter_info, tucb_ptr, ttype)) {
  	mps_clean_up(adapter_info);
  	tucb_ptr->header.adap_flag = FALSE;
  	return(rc1);
  }

  /****************************
   * Transmit and receive data
   ****************************/
  if (rc1 = transmit_data(adapter_info, tucb_ptr, ttype)) {
  	if (tucb_ptr->header.adap_flag == TRUE) {
  		mps_clean_up(adapter_info);
  		tucb_ptr->header.adap_flag = FALSE;
  	}
  }

  /***************************************************
   *  Clean up resources allocated for initialization
   ***************************************************/
  rc2 = mps_clean_up(adapter_info);
  tucb_ptr->header.adap_flag = FALSE;     /* Set flag for HTX signal handler */

  /**************************************************
   * Determine which, if any, return code to return.
   **************************************************/
  if (rc2) {
  	return (error_handler(tucb_ptr, ttype, rc2, CLEANUP_ERR, 0, 0, 0));
  } else {
  	return(0);
  }

} /* end wrap_test */



