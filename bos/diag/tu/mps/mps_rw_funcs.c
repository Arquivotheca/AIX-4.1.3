static char sccsid[] = "@(#)25  1.1  src/bos/diag/tu/mps/mps_rw_funcs.c, tu_mps, bos411, 9437B411a 8/23/94 16:25:08";
/*****************************************************************************
 * COMPONENT_NAME: (tu_mps)  Wildwood LAN adapter test units
 *
 * FUNCTIONS: pos_rd
 *            pos_wr
 *            io_rd
 *            io_wr
 *            io_rd_swap
 *            io_wr_swap
 *
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
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
#include <sys/types.h>
#include <sys/mdio.h>
#include <sys/errno.h>
#include <sys/watchdog.h>
#include <sys/diagex.h>

/* local header files */
#include "mps_macros.h"
#include "mpstools.h"


/**************************************************************************** *
*
* FUNCTION NAME =  pos_wr
*
* DESCRIPTION   =  This function writes the given value to the given
*                  POS register.
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*              pos_addr       - POS register
*              value          - value to be written
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   error code
*
* INVOKED FROM = pos_chk
*
*****************************************************************************/
int pos_wr (ADAPTER_STRUCT *adapter_info, ulong pos_addr, uchar value)
{
	ulong rc;

	rc = diag_pos_write(adapter_info->handle, pos_addr, value,
	    NULL, PROCLEV);
	return(rc);

}  /* End pos_wr */


/**************************************************************************** *
*
* FUNCTION NAME =  pos_rd
*
* DESCRIPTION   =  This function reads a value from the given POS register
*                  'save area'.
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*              pos_addr       - POS register
*              value          - value read
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   error code
*
* INVOKED FROM = pos_chk
*
*****************************************************************************/
int pos_rd (ADAPTER_STRUCT *adapter_info, ulong pos_addr, uchar *return_val)
{
  uchar  value;
  uint   rc = 0;

  rc = diag_pos_read(adapter_info->handle, pos_addr, &value, NULL ,PROCLEV);

  if (rc) {
	return(rc);            /* read failed */
  } else {
  	*return_val = value;
  	return(rc);            /* read passed */
  }

}  /* End pos_rd */


/**************************************************************************** *
*
* FUNCTION NAME =  io_rd
*
* DESCRIPTION   =  This function reads a value from the given I/O register.
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*              type           - type of transfer: byte, word, or long
*              io_addr        - I/O register
*              value          - value read
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   error code
*
* INVOKED FROM = io_test
*
*****************************************************************************/

int io_rd (
  ADAPTER_STRUCT *adapter_info, 
  int 		 type, 
  ulong 	 io_addr, 
  ushort 	 *return_val)
{
  uint    rc = 0;
  ushort  data;

  rc = diag_io_read(adapter_info->handle, type, io_addr, &data, NULL, PROCLEV);

  if (rc) {
	return(rc);
  } else {
	*return_val = data;
	return(rc);
  }

}  /* End io_rd */


/**************************************************************************** *
*
* FUNCTION NAME =  io_wr
*
* DESCRIPTION   =  This function writes the given value to the given
*                  I/O register.
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*              type           - type of transfer: byte, word, or long
*              io_addr        - I/O register
*              value          - value to be written
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   error code
*
* INVOKED FROM = io_test
*
*****************************************************************************/
int io_wr (ADAPTER_STRUCT *adapter_info, int type, ulong io_addr, ushort value)
{
  uint   rc = 0;

  rc = diag_io_write(adapter_info->handle, type, io_addr, value, NULL, PROCLEV);
  return(rc);

}  /* End io_wr */


/**************************************************************************** *
*
* FUNCTION NAME =  io_rd_swap
*
* DESCRIPTION   =  This function reads a value from the given I/O register
*                  and then returns the byte-swapped value.
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*              type           - type of transfer: byte, word, or long
*              io_addr        - I/O register
*              value          - value read
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   error code
*
* INVOKED FROM = io_test
*
*****************************************************************************/
int io_rd_swap (
  ADAPTER_STRUCT *adapter_info, 
  int 		 type, 
  ulong 	 io_addr, 
  ushort	 *return_val)
{
  ushort  data;
  uint    rc = 0;

  rc = diag_io_read(adapter_info->handle, type, io_addr,
	    &data, NULL, PROCLEV);

  if (rc) {
	return(rc);
  } else {
	switch(type) {
		case IOSHORT: 
			*return_val = SWAP16(data);
			break;

		case IOLONG:  
			*return_val = SWAP32(data);
			break;

		default:      
			*return_val = data;
			break;
	}

	return(rc);
  }

}  /* End io_rd_swap */

/**************************************************************************** *
*
* FUNCTION NAME =  io_wr_swap
*
* DESCRIPTION   =  This function performs a byte swap on the given
*                  value before writing it out to the given I/O register.
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*              type           - type of transfer: byte, word, or long
*              io_addr        - I/O register
*              value          - value to be written
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   error code
*
* INVOKED FROM = io_test
*
*****************************************************************************/
int io_wr_swap (
  ADAPTER_STRUCT *adapter_info, 
  int 		 type, 
  ulong 	 io_addr, 
  ushort 	 value)
{
  ulong       rc;

  switch(type) {
	case IOSHORT:  
		value = SWAP16(value);
		break;

	case IOLONG:   
		value = SWAP32(value);
		break;

	default:      
		break;
  }

  rc = diag_io_write(adapter_info->handle, type, io_addr, value, NULL, PROCLEV);

  return(rc);

}  /* End io_wr_swap */

