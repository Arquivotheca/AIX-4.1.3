/*
 *   COMPONENT_NAME: tu_pcitok
 *
 *   FUNCTIONS: mps_recv
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
* INVOKED FROM = process_misr_cmd
*
*****************************************************************************/
int mps_recv(ADAPTER_STRUCT *adapter_info)
{
  volatile rx_list_t  recvlist;
  int                 i, rc = 0, len;

  DEBUG_0("Entering mps_recv\n");

  /* check the status of the buffer */

  recvlist.recv_status = SWAP32(adapter_info->recv_list[0]->recv_status);
  recvlist.fr_len      = SWAP16(adapter_info->recv_list[0]->fr_len);

  DEBUG_1("Frame length = %d\n",recvlist.fr_len);
  DEBUG_1("Receive status = 0x%08x\n",recvlist.recv_status);
  if (!(recvlist.recv_status & BUFPRO)) {
	return(RX_DESC_ERR);
  }

  /*******************
    * Get receive data.  Do not include header, adapter changes it during wrap,
    * or CRC.
    *******************/
  len = SWAP16(adapter_info->recv_list[0]->data_len) - 20;

  if ((len < 0) | (len > MAX_BUF_LEN)) {
	return(RX_LEN_ERR);
  }

  /*
   * Copys data from d_master'ed mbuf 
   */
  bcopy(adapter_info->recv_buf[0]+16, adapter_info->receive_string, len);
  adapter_info->receive_string[len] = '\0';

  if (adapter_info->opened)
  {
  	rc=strcmp(adapter_info->transmit_string+16,adapter_info->receive_string);
  	if (rc) {
		DEBUG_0("Expected buffer\n")
#ifdef DEBUG
		hexdump(adapter_info->transmit_string,
  			strlen(adapter_info->transmit_string));
		DEBUG_0("Received buffer\n")
		hexdump(adapter_info->receive_string,len);
#endif
		return(DATA_MISCOMPARE);
  	}
  }
  DEBUG_0("\nExiting mps_recv\n");
  return(rc);

} /* end mps_recv */
