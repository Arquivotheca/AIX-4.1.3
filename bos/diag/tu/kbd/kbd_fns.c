/* static char sccsid[] = "@(#)90  1.2  src/bos/diag/tu/kbd/kbd_fns.c, tu_kbd, bos411, 9428A410j 5/11/94 13:47:01"; */
/*
 * COMPONENT_NAME: tu_kbd
 *
 * FUNCTIONS:  Kbd_Reset, Send_Data, send_kbd, get_data 
 *              
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
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <sys/devinfo.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <cur00.h>
#include <cur02.h> 

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
    
/***************************************************************************/
/* FUNCTION NAME: Kbd_Reset

   DESCRIPTION: Reset the keyboard bit in the POS register
                Configure the 8255
                Restore the original setting of the POS register

   NOTES:       This function resets the tablet adapter and also
                configures the 8255 chip.

***************************************************************************/

int Kbd_Reset(fdes)
int fdes;
{

        int rc = SUCCESS;
        unsigned char init_pos_val,new_pos_val,kbddata;
        int wait_time;

        /* Lock the POS Register with a semaphore */

        wait_time = WAIT_FOR_ONE;
        if ((rc = set_sem(wait_time)) != SUCCESS)
              return(rc);

        rc |= rd_iocc(fdes,&init_pos_val,RESET_REG);
        new_pos_val = init_pos_val | RST_KBD_8051;
        rc |= wr_iocc(fdes,&new_pos_val,RESET_REG);
        usleep(10);
        kbddata = CFG_8255_CMD;
        rc |= wr_byte(fdes,&kbddata, IOW_CFG_8255);
        usleep(10);
        rc |= wr_iocc(fdes,&init_pos_val,RESET_REG);

        /* Release the POS Register's semaphore */

        if ((rc = rel_sem()) != SUCCESS)
              return(rc);

        /* get BAT looking for self test performed interrupt  */

        if ((rc = get_data(fdes, ID_BAT_INT, ADP_COMP_CODE, &kbddata)) != SUCCESS)
	   return(rc);

        /* tell 8051 that we want kbd ACK's  
        /* set SRAM 10 mode bit = 0 to report receipt of keyboard acknowledgement
        /* byte with informational interrupt 
         */

        if((rc = Send_Data(fdes, KBD_ACK_BYT_SET_CMD, 0, 0)) != SUCCESS)
          return(rc);

        return(rc);
}

/***************************************************************************
 * FUNCTION NAME: send_kbd() 
 *
 * DESCRIPTION:  This function sends a command to the keyboard and waits for 
 *               ACK response unless the echo command is sent or SRAM writes
 *               which will not produce an ACK response. 
 ***************************************************************************
 */

int send_kbd(int fdes, ushort command)
{
   int rc =FAILURE;
   uchar data;
 
   if (!Send_Data(fdes,command, 0, 0)) { /* send Command */
      if (command == KBD_ECHO_CMD)
      { /* echo does not return ACK */ 
           rc = SUCCESS;
      }
      else {
         if (!get_data(fdes, 0, KBD_ACK, &data))   /* wait for ACK */
           rc = SUCCESS;
      }
   }
   return(rc);
}  

/*****************************************************************************
 * FUNCTION NAME: Send_Data()
 *
 * DESCRIPTION:  This function check port C ofr output buffer empty (register 
 *               0x0056. It will poll fro a maximum of 30 milliseconds for bit
 *               7 to be set to one.
 ***************************************************************************** 
 */

int Send_Data(int fdes, ushort command, uchar itype, uchar idata)
{
   int count;
   unsigned char kbddata;
   int rc;

   /* read Port C check for output buffer empty */
   /* (max loop time 30ms then return failure) */

   for (count = 0; count < REPEAT_COUNT; count++) {
      if((rc = rd_byte(fdes, &kbddata, IOR_PC_8255)) != SUCCESS)
            return(FAILURE);
      if (kbddata & OUTPUT_BUFF_EMPTY) break;
      usleep(1000);
   }
  
   if (count == REPEAT_COUNT) return(FAILURE);

   /* Send command to device */

   if((rc = wr_word(fdes, &command, IOW_PA_8255)) != SUCCESS)
         return(FAILURE);

   /* get response */

   return(get_data(fdes, itype, idata, &kbddata));
}

 
/****************************************************************************
 * FUNCTION NAME: get_data()
 *
 * DESCRIPTION:  This function checks if the input buffer of register 0x0056.
 *               It polls for bit 5 of register 0x0056 to be set to one.
 **************************************************************************** 
 */

int get_data(int fdes, int itype, int idata, unsigned char *porta)

{
   int count;
   unsigned char portc;
   int rc = FAILURE;

   /* max time to loop is about 600ms then return FAILURE */
   /* note: 600ms will be rather large for most cases     */
   /* however, will cover all conditions...               */

   count = 600;

   for(;;){

   /* wait for input buffer full  */
   do{
     usleep(1000);
     rc = rd_byte(fdes, &portc, IOR_PC_8255);
   } while (!rc && (count--) && !(portc & INPUT_BUFF_FULL));

   if (rc || (count < 0))
      {
        rc = FAILURE;
        break;
      }
   /* read data from port a  */
   if ((rc = rd_byte(fdes, porta, IOR_PA_8255)) != SUCCESS)
         break;

   /* delay just to make sure strobe clear (9-10us, wait 30 to make sure */
   usleep(30);

   /* verify we got expected data */
   /* check for same type interrupt */
   if (itype == (portc & ID_ERR_INT)) {
                     /* any bit match if unsolicited status */
       if (((!itype && (idata & *porta & OUTPUT_BUFF_EMPTY))
             && (*porta & idata & INVALID_CMD_ACK)) ||
                     /* any keyboard response if waiting for keyboard data */
          ((itype == 1)  ||
                     /* all other cases, data must match */
          (idata == *porta))) {
             rc = SUCCESS;
             break;
       }
    }
    else {
                    /* looking for specific keyboard data */
       if ((itype == ID_INFO_INT) && ((portc & ID_ERR_INT) == 1) &&
             (*porta == idata)) {
          rc = SUCCESS;
          break;
       }
    }
  }
  return(rc);
}
