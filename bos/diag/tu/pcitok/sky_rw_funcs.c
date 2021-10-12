/*
 *   COMPONENT_NAME: tu_pcitok
 *
 *   FUNCTIONS: config_reg_read
 *		config_reg_write
 *		io_rd
 *		io_rd_swap
 *		io_wr
 *		io_wr_swap
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

/*****************************************************************************
* This file contains functions to access the configuration area of Skyline,
* and the I/O area.  
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

/* local header files */
#include "skytu_type.h"

/*****************************************************************************
*
* FUNCTION NAME =  config_reg_write
*
* DESCRIPTION   =  This function writes the given value to the given
*                  Configuration register.
*
* INPUT   =    reg_offset     - Configuration register
*              value          - value to be written
*
* RETURN-NORMAL =  0
* RETURN-ERROR =   error code
* INVOKED FROM = pos_chk
*
*****************************************************************************/
int config_reg_write (reg_offset, value, dds)
  ulong reg_offset;
  uchar value;
  mps_dds dds;
{
  int rc;
  char bus[32];

  struct mdio md;
  int md_data;
  int mdd_fd; 
  sprintf(bus,"%s%s","/dev/",dds.par_name);
  if((mdd_fd = open(bus, O_RDWR)) < 0)
  {
#ifdef DEBUG 
      DEBUG_1("MDD_open failed, rc=%d\n", rc); 
#endif
      rc = MDD_OPEN_FAILED;
      return(rc);
  }
  md.md_addr = reg_offset;
  md.md_data = (char *)&value;
  md.md_size = 1;
  md.md_incr = MV_BYTE;
  md.md_sla = dds.slot_num;

#ifdef DEBUG 
  DEBUG_2("WRITE md_addr = 0x%08x, value = 0x%08x\n", md.md_addr,*md.md_data);
#endif

  rc = ioctl(mdd_fd, MIOPCFPUT, &md);
  if (rc = close(mdd_fd))
  {
#ifdef DEBUG 
	DEBUG_1("mdd close failed, rc = 0x%08x\n", rc);
#endif
	rc = MDD_CLOSE_FAILED;
  }

  return(rc);

}  /* End config_reg_write */

/*****************************************************************************
*
* FUNCTION NAME =  config_reg_read
*
* DESCRIPTION   =  This function reads the given Configuration register.
*
* INPUT   =    reg_offset     - Configuration register
*              value          - location to place value read
*
* RETURN-NORMAL =  0
* RETURN-ERROR =   error code
* INVOKED FROM = pos_chk
*
*****************************************************************************/
int config_reg_read (reg_offset, value, dds)
  ulong reg_offset;
  uchar *value;
  mps_dds dds;
{
  int rc;
  char bus[32];

  struct mdio md;
  int md_data;
  int mdd_fd;  
  sprintf(bus,"%s%s","/dev/",dds.par_name);
  if((mdd_fd = open(bus, O_RDWR)) < 0)
  {
#ifdef DEBUG 
      DEBUG_1("MDD_open failed, rc=%d\n", rc);  
#endif
      rc = MDD_OPEN_FAILED;
      return(rc);
  }

  md.md_addr = reg_offset;
  md.md_data = value;
  md.md_size = 1;
  md.md_incr = MV_BYTE;
  md.md_sla = dds.slot_num;
  rc = ioctl(mdd_fd, MIOPCFGET, &md);

#ifdef DEBUG 
  DEBUG_2("READ md_addr = 0x%08x, value = 0x%08x\n", md.md_addr,*value);
#endif
  if (rc = close(mdd_fd))
  {
#ifdef DEBUG 
	DEBUG_1("mdd close failed, rc = 0x%08x\n", rc);
#endif
	rc = MDD_CLOSE_FAILED;
  }

  return(rc);

}  /* End config_reg_read */


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
  	DEBUG_2("IO_READ: addr = 0x%08x,value = 0x%08x\n",io_addr,*return_val);
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

  DEBUG_2("IO_WRITE: address = 0x%08x,0x%08x\n",io_addr,value); 
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
  ulong	 	*return_val)
{
  ulong  data;
  uint    rc = 0;

  rc = diag_io_read(adapter_info->handle, type, io_addr,
	(void *)&data, NULL, PROCLEV);

  if (rc) {
	return(rc);
  } else {
	switch(type) {
		case IOSHORT: 
			*return_val = SWAP16((ushort) (data >> 16));
			*return_val = *return_val << 16;
  	DEBUG_2("IO_READ_S: address = 0x%08x,0x%08x\n",io_addr,(*return_val) >> 16);
			break;

		case IOLONG:  
			*return_val = SWAP32(data);
  	DEBUG_2("IO_READ_S: address = 0x%08x,0x%08x\n",io_addr,*return_val);
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
  ulong 	 value)
{
  ulong       rc;

  DEBUG_2("IO_WRITE_S: address = 0x%08x,0x%08x\n",io_addr,value);
  switch(type) {
	case IOSHORT:  
		value = SWAP16((ushort)value);
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







