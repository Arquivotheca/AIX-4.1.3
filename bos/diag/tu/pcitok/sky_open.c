/*
 *   COMPONENT_NAME: tu_pcitok
 *
 *   FUNCTIONS: mps_open_adapter
 *		open_adapter
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

  DEBUG_0("open adapter\n");
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
	rc = diag_watch4intr(adapter_info->handle, SRB_RSP, TIMEOUT_LIMIT);

  	/**************************************
       	 * Check for error from diag_watch4intr
       	 **************************************/
  	if (rc) {
		DEBUG_0("Interrupt after open adapter not received\n");
  		return(process_timeout(adapter_info, tucb_ptr, ttype, SRB_RSP & ARB_CMD));
		return(rc);
  	}

  	/**************************
       	 *  Process the interrupt
       	 **************************/
  	if (rc = process_interrupts(adapter_info, tucb_ptr, ttype, SRB_RSP)) {
  		return(rc);
  	}
  } /* end if */

  DEBUG_0("return from open_adapter\n");
  return(0);

} /* end open_adapter */
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
* INVOKED FROM = open_adapter
*
*****************************************************************************/

int mps_open_adapter(ADAPTER_STRUCT *adapter_info,int open_option)
{
  struct {
	ushort   cmd;
	uchar    reserved5[5];
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

  /********************
  *  Set open options
  ********************/
  bzero(parm, sizeof(o_parm));
  o_parm.cmd = SWAP16(OPEN_ADAPTER);

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

  if (rc = io_wr_swap(adapter_info,IOSHORT, LAPE, 0x00)) {
	return (REGISTER_ERR + WRITE_ERR); 
  }

  if (rc = io_wr_swap(adapter_info,IOSHORT, LAPA, adapter_info->srb_address)) {
	return (REGISTER_ERR + WRITE_ERR); 
  }

  for (i = 0; i < sizeof(o_parm)/2; i++) {

  	if (rc = io_wr(adapter_info,IOSHORT, LAPDInc, *(parm+i))) {
	return (REGISTER_ERR + WRITE_ERR); 
	}
  }
  if (rc = io_wr_swap(adapter_info,IOSHORT, LISR_SUM, 0x20)) {
	return (REGISTER_ERR + WRITE_ERR); 
  }
}
