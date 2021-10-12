static char sccsid[] = "@(#)71  1.1  src/bos/diag/tu/siosl/mousea/tu10.c, cmddiag, bos411, 9428A410j 12/17/93 10:45:24";
/*
 *   COMPONENT_NAME: TU_SIOSL
 *
 *   FUNCTIONS: tu10m
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <sys/sem.h>
#include <sys/devinfo.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mode.h>
#include <sys/mdio.h>
#include <diag/atu.h>
#include "tu_type.h"


/* Function tu10m - TU 10 - Tests mouse adapter interrupt logic.  See
   related documentation for return code information.
*/ 

int tu10m(long fd, struct TUCB *tucb_ptr)
{
   unsigned char data,rd_value;
   unsigned int ldata; 
   int count = 0,status, rc = SUCCESS;
   char msg[80];

   /* Check to see if mouse is in block mode */
   rc = rd_bytem(fd, &data, MOUSE_STAT_REG);
   if (rc == SUCCESS)
   {
       if (data & MOUSE_STAT_BLK_MODE_ENABLED)
       {
          data = MOUSE_CMD_DISABLE_BLK_MODE;
          rc = wr_bytem(fd, &data, MOUSE_CMD_REG);
       }
   }

   if (rc == SUCCESS)    /* Test the interrupt logic of the mouse */
   {
      data = MOUSE_CMD_ENABLE_INTERRUPT;
      /* Issue command to mouse adapter    */
      rc = wr_bytem(fd, &data, MOUSE_CMD_REG);
   }
   if (rc == SUCCESS)
   {
     /* Read the status register to verify that interrupt is enabled */
     rc = rd_bytem(fd, &data, MOUSE_STAT_REG);
   }
   if (rc == SUCCESS)
   {
     /* Check for interrupt enabled */
     if ((data & MOUSE_STAT_INTERRUPT_ENABLED) != MOUSE_STAT_INTERRUPT_ENABLED)
     {
        sprintf(msg,"Unable to enable mouse interrupts, status:%2X\n",data);
        PRINTERR(msg);
        rc = MOUSE_LOGIC_ERROR;
     }
     else
     {
        data = MOUSE_CMD_DISABLE_INTERRUPT;
        /* Disable mouse interrupt */
        rc = wr_bytem(fd, &data, MOUSE_CMD_REG);
     }
   }
   if (rc == SUCCESS)
   {
     /* Read the status register to verify that interrupt is disabled */
     rc = rd_bytem(fd, &data, MOUSE_STAT_REG);
   }
   if (rc == SUCCESS)
   {
     /* Check for interrupt disabled */
     if ((data & MOUSE_STAT_INTERRUPT_ENABLED) != 0)
     {
        sprintf(msg,"Unable to disable mouse interrupts, status:%2X\n",data);
        PRINTERR(msg);
        rc = MOUSE_LOGIC_ERROR;
     }
   }
return(rc);
}

