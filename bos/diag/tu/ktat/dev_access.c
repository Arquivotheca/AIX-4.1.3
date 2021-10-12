static char sccsid[] = "@(#)84	1.2  src/bos/diag/tu/ktat/dev_access.c, tu_ktat, bos41J, 9519A_all 5/3/95 15:01:36";
/*
 *   COMPONENT_NAME: tu_ktat
 *
 *   FUNCTIONS: config_reg_read
 *		config_reg_write
 *		dma_finish
 *		dma_setup
 *		io_read
 *		io_write
 *		wait_4_intr
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
#include <sys/dma.h>

/* local header files */

extern diagex_dds_t dds;
extern diag_struc_t *diagex_hdl;
extern int mdd_fd;


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
int config_reg_write (reg_offset, value)
  ulong reg_offset;
  int *value;
{
  int rc;

  struct mdio md;
  int md_data;
/*  int mdd_fd; */

  md.md_addr = reg_offset;
  md.md_data = (char *)&value;
  md.md_size = 1;
  md.md_incr = MV_WORD;
  md.md_sla = dds.slot_num;
/*
  printf("WRITE md_addr = 0x%08x\n\n", md.md_addr);
  printf("WRITE value_address = 0x%08x\n", &value);
  printf("WRITE md_data_address = 0x%08x\n", md.md_data);
  printf("WRITE (md) value = 0x%08x\n", value);
*/
/*
  printf("md_data = 0x%08x\n", md.md_data);
  printf("md_size = 0x%08x\n", md.md_size);
  printf("md_incr = 0x%08x\n", md.md_incr);
  printf("md_sla = 0x%08x\n", md.md_sla);
*/
  rc = ioctl(mdd_fd, MIOPCFPUT, &md);
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
int config_reg_read (reg_offset, value)
  ulong reg_offset;
  int *value;
{
  int rc;

  struct mdio md;
  int md_data;
/*  int mdd_fd;  */

  md.md_addr = reg_offset;
  md.md_data = (char *)value;
  md.md_size = 1;
  md.md_incr = MV_WORD;
  md.md_sla = dds.slot_num;
/*
  printf("READ md_addr = 0x%08x\n", md.md_addr);
  printf("READ value as address = 0x%08x\n", value);
  printf("READ md.data_address  = 0x%08x\n\n", md.md_data);
*/  
  rc = ioctl(mdd_fd, MIOPCFGET, &md);

/*  printf("READ (md) value = 0x%08x\n",*value); */
  return(rc);

}  /* End config_reg_read */


/**************************************************************************** *
*
* FUNCTION NAME =  io_read
*
* DESCRIPTION   =  This function reads a value from the given I/O register.
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex_hdl info
*              type           - type of transfer: byte, word, or long
*              addr           - I/O register
*              value          - value read
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   error code
*
* INVOKED FROM = io_test
*
*****************************************************************************/

int io_read (type, addr, value)
  int 		 type; 
  ulong 	 addr; 
  int            *value;
  {
    int    rc = 0;
    int    data;
    int    junk;

/* printf("address to read (io_read)= 0x%08x\n", addr);  */
    rc = diag_io_read(diagex_hdl, type, addr, &data,(void *) NULL, PROCLEV);

    if (rc) {
      return(rc);
    }
    else {
     *value = data;
/* printf("data read (io_read)= 0x%08x\n", data); */

     return(rc);
    }

  }  /* End io_read */


/**************************************************************************** *
*
* FUNCTION NAME =  io_write
*
* DESCRIPTION   =  This function writes the given value to the given
*                  I/O register.
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex_hdl info
*              type           - type of transfer: byte, word, or long
*              addr           - I/O register
*              value          - value to be written
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   error code
*
* INVOKED FROM = io_test
*
*****************************************************************************/
int io_write (type, addr, value)
  int 		 type; 
  ulong 	 addr; 
  int            value;
  {
    int    rc = 0;
    int    junk;
/*
printf("address to write (io_write) = 0x%08x\n", addr);
printf("data to write (io_write) = 0x%08x\n", value);
*/  
  rc = diag_io_write(diagex_hdl, type, addr, value,(void *) NULL, PROCLEV);
    return(rc);
  }  /* End io_write */


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

/*
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

}*/  /* End io_rd_swap */


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
/*
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

} */  /* End io_wr_swap */

int dma_setup(buf, addr, flag, count)
  char  *buf;
  ulong *addr;
  int   flag; 
  int   count;
{
  int rc;
  rc = diag_map_page(diagex_hdl, flag, buf, addr, count);
  return(rc);
}  

int dma_finish(addr)
  ulong *addr;
{
  int rc;
  rc = diag_unmap_page(diagex_hdl, addr);
  return(rc);
}

int wait_4_intr(int mask, int timeout)
{
  int rc;

  rc = diag_watch4intr(diagex_hdl, mask, timeout);
  return(rc);
}



