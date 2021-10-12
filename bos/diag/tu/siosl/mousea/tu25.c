static char sccsid[] = "@(#)74  1.1  src/bos/diag/tu/siosl/mousea/tu25.c, cmddiag, bos411, 9428A410j 12/17/93 10:45:52";
/*
 *   COMPONENT_NAME: TU_SIOSL
 *
 *   FUNCTIONS: tu25m
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


/* Function tu25m - TU 25 - Puts mouse adapter in non-block mode.  See
   related documentation for return code information.
*/


/* For this TU, the mouse device MUST be attached to run this test properly,
   otherwise, unpredictable adapter/device behavior may result. */

int tu25m(long fd,struct TUCB *tucb_ptr)
{
  unsigned char data;
  unsigned char cdata;
  ushort udata;
  int i, rc = SUCCESS,count = 0;
  unsigned int ldata;
  char msg[80],done,receive;
  union regs
   {
     unsigned int lreg;
     unsigned char reg[4];
   } mouse;


 /* Check to see if mouse is in block mode */
 if ((rc = rd_bytem(fd, &data, MOUSE_STAT_REG)) != SUCCESS)
    PRINTERR("Unsuccessful read of one byte from MOUSE_STAT_REG\n");

 if (rc == SUCCESS)
   /************************************************************************/
   /* Check for block mode enabled, if true then disable it              */
   /************************************************************************/
   {
     if (data & MOUSE_STAT_BLK_MODE_ENABLED)
     {
       rc = rd_bytem(fd, &data, MOUSE_STAT_REG);

       /* First disable interrupts, if necessary */

       if (rc == SUCCESS) {

         if (data & MOUSE_STAT_INTERRUPT_ENABLED) {

           /* send disable interrupts command    */
           cdata = MOUSE_CMD_DISABLE_INTERRUPT;
           rc = wr_bytem(fd, &cdata, MOUSE_CMD_REG);
           usleep(25000);
         }
       }

       if (rc == SUCCESS) {

        /* Disable block mode */
           data = MOUSE_CMD_DISABLE_BLK_MODE;
           rc = wr_bytem(fd, &data, MOUSE_CMD_REG);
           usleep(25000);

        /* check the block disable received status of the adapter  */
          if (rc == SUCCESS) {
            rc = rd_bytem(fd, &data, MOUSE_STAT_REG);
          }

         if (rc == SUCCESS) {
           if (!(data & 0x04))
           {
                                       /*      resend the command            */
            data = MOUSE_CMD_DISABLE_BLK_MODE;
            wr_bytem(fd, &data, MOUSE_CMD_REG);
            usleep(25000);

                                       /*      read mouse adap status reg    */
            rc = rd_bytem(fd, &data, MOUSE_STAT_REG);
           }
         }
               /*  In the case that bits 1 and or 2 are set,  */
               /* will wait 1.2 ms and read again, just in case both are set */
         if (rc == SUCCESS) {

          if (data & 0x06)
          {
            usleep(1200);                    /*      wait for 1.2 ms         */
                                         /*      read mouse adap status reg    *
/
            rc = rd_bytem(fd, &data, MOUSE_STAT_REG);
                                         /*      if command not rcvd after busy*
/
           if (rc == SUCCESS) {
            if (data & 0x02)
            {
              PRINTERR("Error -> Mouse block mode was not disabled\n");
              rc = MOUSE_NON_BLOCK_ERROR;
            }
           }
          }

         if (rc == SUCCESS)
         {
                              /*   read in any remaining mouse data */
          rd_wordm(fd, &mouse.lreg, MOUSE_RX_STAT_REG);
          rd_wordm(fd, &mouse.lreg, MOUSE_RX_STAT_REG);
          rd_wordm(fd, &mouse.lreg, MOUSE_RX_STAT_REG);
          rd_wordm(fd, &mouse.lreg, MOUSE_RX_STAT_REG);

                                 /* disable mouse                      */
          udata = 0xf500;
          rc = wr_2bytem(fd, &udata, MOUSE_DATA_TX_REG);
          usleep(25000);
         /* Read mouse 32 bit register for acknowledgement of data */

          if (rc == SUCCESS)
          {
            for (count = 0; count < REPEAT_COUNT+100; count++)
            {
             rc = rd_wordm(fd, &mouse.lreg, MOUSE_RX_STAT_REG);
             if ((rc == SUCCESS) && ((mouse.lreg & 0x0f000000) == 0x09000000))
              break;
            }
             /* Check for mouse device ACK */
            if (rc == SUCCESS) {
              if ((mouse.lreg & 0x00FF0000) != 0x00FA0000) {
                PRINTERR("Error -> Mouse device not disabled\n");
                rc = MOUSE_NON_BLOCK_ERROR;
              }
            }

          } /* if rc = SUCCESS */
        } /* if rc = SUCCESS */
       } /* if rc = SUCCESS */

      } /* if rc = SUCCESS */
     } /* if block mode enabled */
   } /* if rc = SUCCESS */

return(rc);
}

