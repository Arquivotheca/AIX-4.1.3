/* static char sccsid[] = "@(#)12  1.3  src/bos/diag/tu/tablet/tab_fns.c, tu_tab, bos411, 9428A410j 5/21/94 08:08:45"; */
/*
 *
 * COMPONENT_NAME: tu_tab
 *
 * FUNCTIONS:   Send_Data, Tab_Init, tab_status 
 *              Tab_Reset, get_data
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

/* Debug */
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

/************************************************************************
 FUNCTION NAME: tab_status
  
   DESCRIPTION: Activate the UART block
                Send Read Curents Status/Coord. command
                Read Port C for block transfer Interrupt ID number
                Read Port A for number of bytes
                Compare Status Byte with VALID_BYTES pattern
                Read the other bytes of block transfer
                Inactivate the UART Block

   NOTES:       This function reads and compares the bytes when
                the adapter initiates a block transfer.
                tablet (bit 1) and always 1 (bit 7) are checked.
***********************************************************************/

int tab_status(int fdes)
{
    int rc = SUCCESS;
    ushort data;
    unsigned char tabdata;
    unsigned char bytecount;
    unsigned char byte1;
    int count;

    /* initialize variables */
    bytecount = 0;
    byte1 = 0;

    /* Blocking of received UART bytes is set to active by setting SRAM
     * 10 bit 5(=1). This is done so that the reported bytes are buffered
     * and blocked according to the blocking factor which is 6. */

    data = UART_BLK_ACT;
    if (( rc = Send_Data(fdes, data, 0, 0)) != SUCCESS)
        return(FAILURE);

    /* Send read current status/coord command to save the
     * current status and coordinates */

    data = TAB_RD_STAT_CMD;
    if ((rc = Send_Data(fdes, data, 0, 0)) != SUCCESS)
        return(FAILURE);

    /* Poll Port A for data to be compared
     * Read first byte which contains how many bytes of info are sent by
     * TAB_READ_CURR_STAT command. It should contain the value 6.
     * bytecount contains the byte count sent by the read status command */

    usleep(100 * 1000);
    rc = rd_byte(fdes, &tabdata, IOR_PA_8255);
    bytecount = tabdata;
    if (bytecount != 6)
        return(FAILURE);

    /* Read tabdata which is the beginning of saving current status */
 
    usleep(100 * 1000);
    rc = rd_byte(fdes, &tabdata, IOR_PA_8255);

    /* Save the first byte for comparison */

    byte1 = tabdata;
    if ((VALID_BYTES & byte1) != VALID_BYTES)
        return(FAILURE);

    /* extract remaining bytes from status read block, to purge the
     * tablet status buffer 
     * extract byte from status block  
     */

    for(count = 0; count < bytecount-1; ++count)
       {
        usleep(100 * 1000);
        rc = rd_byte(fdes, &tabdata, IOR_PA_8255);
       }

    /* Blocking of received UART bytes is set to inactive by setting
     * SRAM 10 bit 5 (= 0). This is done so that the a byte received
     * from the device is passed on to the system. */

    data = UART_BLK_INACT;
    if (( rc = Send_Data(fdes, data, 0, 0)) != SUCCESS)
        return(FAILURE);

    return(rc);
}


/***************************************************************************/
/* FUNCTION NAME: Tab_Init

   DESCRIPTION: Enables UART interface
   Disables tablet
   Inactivate the UART block
   Initialize UART Framing

   NOTES:       This function initializes the tablet by addressing
   the bits in SRAM. This is done to make the tablet
   functional after the keyboard and UART IRQ is
   disabled. Any change in the above sequence of
   commands might cause errors in the TU CODE
   running
****************************************************************************/

int Tab_Init(int fdes)
{
    int rc = SUCCESS;
    unsigned char cdata, tabdata;
    ushort data;

    /* Enable the UART by addressing SRAM at address 12 and bit 4=(1)*/
    data = ENBL_UART_IF;
    if ((rc = Send_Data(fdes, data, 0, 0)) != SUCCESS)
        return(FAILURE);

    /* Disable tablet command */
    data = TAB_DSBL_CMD;
    if ((rc = Send_Data(fdes, data, 0, 0)) != SUCCESS)
        return(FAILURE);

    /* Blocking of received UART bytes is set to inactive by setting
     * SRAM 10 bit 5(=0) */
    data = UART_BLK_INACT;
    if ((rc = Send_Data(fdes, data, 0, 0)) != SUCCESS)
        return(FAILURE);

    /* Initialize UART Framing command */
    /* Framing is set to odd parity and a blocking factor of 6 */
    data = INIT_UART_FRM_O;
    if ((rc = Send_Data(fdes, data, 0, 0)) != SUCCESS)
        return(FAILURE);

    return(rc);
}

/***************************************************************************/
/* FUNCTION NAME: Tab_Reset

   DESCRIPTION: Reset the keyboard bit in the POS register
                Configure the 8255
                Restore the original setting of the POS register

   NOTES:       This function resets the tablet adapter and also
                configures the 8255 chip.

***************************************************************************/

int Tab_Reset(fdes)
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
        /* set SRAM 10 mode bit = 0 to report receipt of keyboard adapter
        /* acknowledgement byte with informational interrupt
         */

        if((rc = Send_Data(fdes, KBD_ACK_BYT_SET_CMD, 0, 0)) != SUCCESS)
          return(rc);

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

   usleep(20 * 1000);

   /* read Port C check for output buffer empty */
   /* (max loop time 30ms then return failure) */
   /* Poll register 0x0056 bit 7 =1 */

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

   /* max time to loop is about 600ms then return FAILURE  */
   /* note: 600ms will be rather large for most cases      */
   /* however, will cover all conditions...                */
   /* Poll register 0x0056 bit 5 =1 for port C buffer full */  

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

   /* register 0x0055 (port B) bit 3 =1 indicates strobe in          */
   /* Tablet hardware specification indicates to look for low signal */
   /* on bit 3 i.e ZERO, due to some timing problems it is difficult */
   /* to read bit 3 after the Input Buffer Full in port C is set to  */
   /* ONE. Therefore Port B is read when bit 3 is ONE which means    */
   /* that the data line is unlatched. Strobe must clear within 10   */ 
   /* micro seconds, therefore instead of checking prot for strobe   */ 
   /* a delay just to make sure strobe clear (9-10us, wait 30 to     */
   /*  make sure is used */

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
                    /* looking for specific tablet data */
       if ((itype == ID_INFO_INT) && ((portc & ID_ERR_INT) == 1) &&
             (*porta == idata)) {
          rc = SUCCESS;
          break;
       }
    }
  }
  return(rc);
}
