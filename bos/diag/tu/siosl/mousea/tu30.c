static char sccsid[] = "@(#)75  1.2  src/bos/diag/tu/siosl/mousea/tu30.c, cmddiag, bos41J, 9515A_all 4/7/95 11:05:16";
/*
 *   COMPONENT_NAME: TU_SIOSL
 *
 *   FUNCTIONS: tu30m
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

/* Function tu30m - TU 30 - Performs mouse adapter wrap data test.  See
   related documentation for return code information.
*/


/* For this TU, the mouse device MUST be unattached for it to properly run */

int tu30m(long fd, struct TUCB *tucb_ptr)
{
   unsigned char data;
   unsigned int ldata;
   ushort wrapdata;
   int index = 0, status, rc = SUCCESS;
   char msg[80];
   union regs
   {
     unsigned int lreg;
     unsigned char reg[4];
   } mouse;

   /* Data patterns to be wrapped */
   ushort datapattern[] = { 0xaa01, 0x5501, 0xff01 };

 /* Read from mouse receive/stat reg - Make sure data buffer is empty */
   rd_wordm(fd, &mouse.lreg, MOUSE_RX_STAT_REG);
   rd_wordm(fd, &mouse.lreg, MOUSE_RX_STAT_REG);
   rd_wordm(fd, &mouse.lreg, MOUSE_RX_STAT_REG);
   rc = rd_wordm(fd, &mouse.lreg, MOUSE_RX_STAT_REG);

 if (rc == SUCCESS) {
   rd_bytem(fd, &data, MOUSE_STAT_REG);

   /* First disable interrupts, if necessary */
   if (data & MOUSE_STAT_INTERRUPT_ENABLED) {

     /* send disable interrupts command    */
     data = MOUSE_CMD_DISABLE_INTERRUPT;
     rc = wr_bytem(fd, &data, MOUSE_CMD_REG);
     usleep(25000);
   }
 }

 /* Let's test wrapping the data */

 if (rc == SUCCESS) {

  for (index = 0; index < (sizeof(datapattern))/2; index++) {

   /* Send round of data */
    if (rc == SUCCESS) {
     wrapdata = datapattern[index];
     /* Load data and wrap command into mouse data tx register */
     rc = wr_2bytem(fd, &wrapdata, MOUSE_DATA_TX_REG);
     usleep(25000);
    }

   if (rc == SUCCESS)
   {
   /* Read mouse receive status register for wrapped data  */
   /* Wait 1 sec for data to be wrapped */
    mouse.lreg = 0;
    sleep(1);
    rc = rd_wordm(fd, &mouse.lreg, MOUSE_RX_STAT_REG);
   }

   if (rc == SUCCESS)
   {
     if (mouse.reg[0] & MOUSE_STAT_RX_DATA_ERROR)
     {
        sprintf(msg,"Error in receive mouse data status reg = :%2X, index = %d\n",mouse.reg[0],index);
        PRINTERR(msg);
        rc = -1;
     }
     else if (mouse.reg[0] & MOUSE_STAT_RX_REG1_FULL)
     {

         /* Compare data to see if they are matched */
         if (mouse.reg[1] != (datapattern[index]>>8))
         {
           sprintf(msg,"Txdata is not matched, Rxdata=%2X, index = %d\n",mouse.reg[1],index);

           PRINTERR(msg);
           rc = MOUSE_WRAP_ERROR;
         }
     }
     else
     {
       PRINTERR("No data is received for diagnostic wrap test\n");
       sprintf(msg,"mouse.lreg = %8x for iteration %d\n",mouse.lreg,index);
       PRINTERR(msg);
       if (mach_info.mach_model == FIREB_MODEL)
           rd_posm(fd, &data, SIO_CTL_G30REG);
       else
           rd_posm(fd, &data, SIO_CONTROL_REG);
       sprintf(msg,"Read from POS2:%2X\n",data);
       PRINTERR(msg);
       rc = -1;
     }
   }

  } /* for loop */

 } /* rc = SUCCESS */

 return (rc);
}


