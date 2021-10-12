/* static char sccsid[] = "@(#)78  1.7  src/bos/diag/tu/mouse/mou_io.c, tu_mouse, bos41J, 9515A_all 4/5/95 16:02:59"; */
/*
 *
 * COMPONENT_NAME: tu_mouse
 *
 * FUNCTIONS:   rdata, non_block, re_block, wr_byte,
 *              rd_byte, wr_2byte, wr_word, rd_word,
 *              clean, init_dev, rd_iocc, wr_iocc,
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
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <sys/devinfo.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/signal.h>
#include <sys/mode.h>
#include <sys/mdio.h>

/***************************************************************************/
/* NOTE: This function is called by Hardware exerciser (HTX),Manufacturing */
/*       application and Diagnostic application to invoke a test unit (TU) */
/*                                                                         */
/*       If the mfg mode in the tu control block (tucb) is set to be       */
/*       invoked by HTX then TU program will look at variables in tu       */

/*       uses the predefined values.                                       */
/*                                                                         */
/***************************************************************************/

#include "tu_type.h"

extern int	pos_slot;

/***************************************************************************

clean

exit function 

***************************************************************************/

void clean()
{
  uchar cdata;
  int fdes;

  exit(0);

} 

/*************************************************************************

sendack

Writes a command to the mouse device 

**************************************************************************/

int sendack(fdes, command)
int fdes;
ushort command;
{
  int count = 0;
  int rc = SUCCESS; 
  unsigned int msedata;


/* Send command to mouse device */

  wr_2byte(fdes, &command, MOUSE_DATA_TX_REG);

/* Read mouse 32 bit register for acknowledgement of data */

  for (count = 0; count < REPEAT_COUNT; count++)
     {
        rc = rd_word(fdes, &msedata, MOUSE_RX_STAT_REG);
        if ((msedata & 0x0f000000) == 0x09000000)
              break;
        usleep(1 * 1000);
     }

  if (count == REPEAT_COUNT)
          return(DEV_POLLING_ERR);
  return(rc);
}

/***********************************************************************

rdata 

Reads data from mouse device status register (get mouse status) 

************************************************************************/

int rdata (fdes, msedata)
int fdes;
unsigned int *msedata;
{
  int count = 0;
  int rc = SUCCESS;
  unsigned int mtdata;
  unsigned char cdata;

 /* Read mouse 32 bit register for mouse data */

  for (count = 0; count < REPEAT_COUNT; count++)
     {
        rc = rd_word(fdes, &mtdata, MOUSE_RX_STAT_REG);
        if ((mtdata & 0x0f000000) == 0x09000000)
             break;
        usleep(1 * 1000);
     }

  if (count == REPEAT_COUNT)
           return(DEV_POLLING_ERR);
  *msedata = mtdata;
  return(rc);
}

/***********************************************************************

non_block

Sets mouse device in non-block mode

************************************************************************/

int non_block(fdes)
int fdes;
{
  unsigned char cdata;
  unsigned int mouse;
  ushort data;
  int rc = SUCCESS;
  int count = 0;

  /* send disable interrupts command    */

  cdata = ADP_CMD_DISABLE_INT;
  wr_byte(fdes, &cdata, MOUSE_ADP_CMD_REG);

  usleep(25 * 1000);

 /*   read in any remaining mouse data */

  for (count = 0; count < READ_COUNT; count++)
        rd_word(fdes, &mouse, MOUSE_RX_STAT_REG);

  /* read mouse adapter status register */

  rd_byte(fdes, &cdata, MOUSE_ADP_STAT_REG);

  if (cdata & STAT_INT_ENABLED)
     {
    /* send disable interrupts command    */

        cdata = ADP_CMD_DISABLE_INT;
        wr_byte(fdes, &cdata, MOUSE_ADP_CMD_REG);
        usleep(25 * 1000);

   /* read mouse adapter status register */

        rd_byte(fdes, &cdata, MOUSE_ADP_STAT_REG);
     }

  if (cdata & STAT_INT_ENABLED)
     {
#ifdef debugg
        detrace(0, " Unable to disable interrupts\n");
#endif
        return(ADP_BLOCK_DIS_ERR);
     }

  /*   disable block mode    */

        data = ADP_CMD_DISABLE_BLK_MODE;
	wr_2byte(fdes, &data, MOUSE_DATA_TX_REG);
	rd_byte(fdes, &cdata, MOUSE_ADP_STAT_REG);

        /*   if command received but adapter receiving mouse data   */
  
        if (cdata & STAT_BLK_CMD_RECD_WAIT)
            {
               usleep(1200);        
	       rd_byte(fdes, &cdata, MOUSE_ADP_STAT_REG);
	       if (cdata & 0x02)
                 {
		    return(ADP_NONBLK_ERR);
		 }
 	    }
 

 /*   read in any remaining mouse data */

  for (count = 0; count < READ_COUNT; count++)
        rd_word(fdes, &mouse, MOUSE_RX_STAT_REG);

/* disable mouse  */

  data = MOUSE_DISABLE_COMMAND;
  rc = wr_2byte(fdes, &data, MOUSE_DATA_TX_REG);
  usleep(25 * 1000); 

  return(rc);
}   /* non_block() */

/*********************************************************************

re_block

Sets mouse device to block mode    

**********************************************************************/

int re_block(fdes)
int fdes;
{
  unsigned char cdata;
  unsigned int mouse;
  int rc = SUCCESS;
  ushort data;

/* set mouse stream mode */

  data = SET_STREAM_MODE;
  wr_2byte(fdes, &data, MOUSE_DATA_TX_REG);

  /* read ACK data */

  rdata(fdes, &mouse);

 /*   enable  block mode */

  data = ADP_CMD_ENABLE_BLK_MODE;
  wr_2byte(fdes, &data, MOUSE_DATA_TX_REG);

/* read mouse adap status reg  */

  rd_byte(fdes, &cdata, MOUSE_ADP_STAT_REG);

/* if command not received  */

  if (!(cdata & STAT_BLK_MODE_ENABLED))
     {
/* resend the command */

        data = ADP_CMD_ENABLE_BLK_MODE;
        wr_2byte(fdes, &data, MOUSE_DATA_TX_REG);
        usleep(25 * 1000);

/* read mouse adap status reg   */

        rd_byte(fdes, &cdata, MOUSE_ADP_STAT_REG);
     }

/*   if cmd NOT rcvd & block mode     */

  if (!(cdata & 0x02))
     {
#ifdef debugg
        detrace(0,"Unable to set adap block mode\n");
#endif
        return(FAILURE);
     }

/* enable mouse */

  data = ENABLE_MOUSE;
  rc = wr_2byte(fdes, &data, MOUSE_DATA_TX_REG);

  return(rc);
} /* re_block() */

/******************************************************************

init_dev

Resets mouse device hardware and enables interrupts

*******************************************************************/

int init_dev(fdes)
int fdes;
{
  unsigned char cdata;
  int rc = SUCCESS;
 
  /* set semaphore to lock pos register 2 */

  if((rc=set_sem(WAIT_FOR_ONE)) != SUCCESS)
     return(rc); 

  /* Reset mouse hardware */

  rd_iocc(fdes, &cdata, RESET_REG);  /* read POS register two */
  cdata |= 0x80;           /* set mouse reset bit */
  wr_iocc(fdes, &cdata, RESET_REG);
  cdata &= 0x7f;           /* reset register to previous state */
  wr_iocc(fdes, &cdata, RESET_REG);

  /* release semaphore to unlock pos register 2 */

  if((rc=rel_sem()) != SUCCESS)
     return(rc);

  cdata = ADP_CMD_DISABLE_INT; 
  rc = wr_byte(fdes, &cdata, MOUSE_ADP_CMD_REG);
  usleep(25 * 1000);
  re_block(fdes);

  if (rc != SUCCESS)
     return(DEV_INIT_ERR);
  return(rc);
}

/******************************************************************

rd_byte

Reads a byte from mouse using machine Device Driver

*******************************************************************/

int rd_byte( int fdes, unsigned char *pdata,unsigned int addr)
{
  MACH_DD_IO iob;
  int rc;

  iob.md_data = pdata;
  iob.md_incr = MV_BYTE;
  iob.md_size = sizeof(*pdata);
  iob.md_addr = addr;

  rc = ioctl(fdes, MIOBUSGET, &iob);
  return (rc);
}

/******************************************************************

rd_word

Reads a word from mouse device using machine Device Driver

********************************************************************/

int rd_word(int fdes, unsigned int *pldata,unsigned int addr)
{
  MACH_DD_IO iob;
  int rc;

  iob.md_data = (char *)pldata;
  iob.md_incr = MV_BYTE;
  iob.md_size = sizeof(unsigned int);
  iob.md_addr = addr;

  rc = ioctl(fdes, MIOBUSGET, &iob);
  return (rc);
}

/********************************************************************

wr_byte

Writes a byte to mouse device using machine Device Driver

*********************************************************************/

int wr_byte(int fdes, unsigned char *pdata,unsigned int addr)
{
  MACH_DD_IO iob;
  int rc;

  iob.md_data = pdata;
  iob.md_incr = MV_BYTE;
  iob.md_size = sizeof(*pdata);
  iob.md_addr = addr;

  rc = ioctl(fdes, MIOBUSPUT, &iob);
  return (rc);
}

/********************************************************************

wr_word

Writes a word to mouse device using machine Device Driver

*********************************************************************/

int wr_word(int fdes, unsigned int *pldata,unsigned int addr)
{
  MACH_DD_IO iob;
  int rc;

  iob.md_data = (char *)pldata;
  iob.md_incr = MV_WORD;
  iob.md_size = sizeof(*pldata);
  iob.md_addr = addr;

  rc = ioctl(fdes, MIOBUSPUT, &iob);
  return (rc);
}

/**********************************************************************

wr_2byte

This function uses the machine device driver to write two bytes from
pdata to the specified address 

***********************************************************************/

int wr_2byte(fdes, pdata, addr)
int fdes;
ushort *pdata;
unsigned int addr;
{
  MACH_DD_IO iob;
  int rc;

  iob.md_data = (char *)pdata;
  iob.md_incr = MV_BYTE;
  iob.md_size = sizeof(*pdata);
  iob.md_addr = addr;

  rc = ioctl(fdes, MIOBUSPUT, &iob);
  return (rc);
}

/******************************************************************

rd_iocc


This function is used to read POS registers using machine Device
Driver
 
*******************************************************************/ 

int rd_iocc(fdes, pdata, addr)
int fdes;
char *pdata;
unsigned int addr;
{
   MACH_DD_IO iob;
   int rc;

   iob.md_data = pdata;
   iob.md_incr = MV_BYTE;
   iob.md_size = sizeof(*pdata);
   iob.md_addr = POSREG(addr, pos_slot); /* IOCC address */

   rc = ioctl(fdes, MIOCCGET, &iob);
   return (rc);
}

/*******************************************************************

wr_iocc

This function is used to write to POS registers using machine Device
Driver
 
********************************************************************/

int wr_iocc(fdes, pdata, addr)
int fdes;
char *pdata;
unsigned int addr;
{
   MACH_DD_IO iob;
   int rc;

   iob.md_data = pdata;
   iob.md_incr = MV_BYTE;
   iob.md_size = sizeof(*pdata);
   iob.md_addr = POSREG(addr, pos_slot); /* IOCC address */

   rc = ioctl(fdes, MIOCCPUT, &iob);
   return (rc);
}

