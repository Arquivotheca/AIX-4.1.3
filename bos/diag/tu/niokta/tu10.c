/* static char sccsid[] = "@(#)46  1.2  src/bos/diag/tu/niokta/tu10.c, tu_adpkbd, bos411, 9431A411a 7/12/94 16:04:26"; */
/*
 * COMPONENT_NAME: tu_adpkbd
 *
 * FUNCTIONS:  	tu10.c 
 *             	Adapter Fuse and Diagnostic Test
 * DESCRIPTION: 
 *		Check tablet and keyboard fuses
 *		Reset POS register
 *		Send NO OP command 
 *		Set adapter to diagnostic mode
 *		Disable keyboard interface 
 *		Write a data byte bit to kbd data line 
 *	        Send diagnostic sense keyboard and UART Pins	
 *		Write a data byte bit to kbd clock line and data line 
 *		Enable keyboard interface 
 *		Reset adapter to diagnostic mode
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
#include <sys/mode.h>
#include <sys/mdio.h>
#include <sys/signal.h>
#include "tu_type.h"

extern int mktu_rc();

int tu10(int fdes, TUTYPE *tucb_ptr)
{
    extern int errno;
    unsigned char cdata;
    unsigned int kbddata;
    ushort data;
    static int rc = SUCCESS;

    /* Initialize variables */
    kbddata = 0;

    /* Reset Keyboard Adapter Hardware */
    if (( rc = Kbd_Reset(fdes)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, LOG_ERR, ADP_HDW_RSET_ERR));

    /* Disable Interrupts */

    cdata = DSBL_KBD_UART_IRQ;
    wr_byte(fdes, &cdata, IOW_CFG_8255);
    usleep(5);

    /* Check the tablet and keyboard fuses */

    rd_byte(fdes, &kbddata, IOR_PB_8255);
    kbddata = kbddata >> 24; 
    
    if (kbddata & TAB_FUSE_GOOD)
        return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, ADP_TAB_FUSE_FAIL));
    
    if (kbddata & KBD_FUSE_GOOD)
        return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, ADP_KEY_FUSE_FAIL));
    
    /* Reset Keyboard Adapter Hardware */

    if (( rc = Kbd_Reset(fdes)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, LOG_ERR, ADP_HDW_RSET_ERR));
    
    /* Send NO_OP command */

    data = NO_OP_CMD; 
    if ((rc = Send_Data(fdes,data, 0, 0)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, LOG_ERR, ADP_NO_OP_ERR));

    /* Set adapter to diagnostic mode by setting SRAM 11 bit 10(=1) */

    data = SET_DIAG_MODE_ON;
    if ((rc = Send_Data(fdes,data, 0, 0)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, LOG_ERR, ADP_DIAG_ON_ERR));

    /* Disable keyboard interface by setting SRAM 11 bit 11(=0)*/

    data = DSBL_KBD_ITF;
    if ((rc = Send_Data(fdes,data, 0, 0)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, LOG_ERR, ADP_DSBL_KBD_ERR));

    /* Write a data byte bit to the keyboard data line using the
     * Diagnostic Write Keyboard Port Pins command
     */

    data = DIAG_WRITE_40;
    if ((rc = Send_Data(fdes,data, 0, 0)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, LOG_ERR, ADP_DIAG_WRT40_ERR));
    
    /* Send Diagnostic Sense Kbd and UART Port Pins command */
    
    /* When the adapter is in diagnostic mode, this command           
     * returns an Interrupt ID of 3 and a byte in IOR_PA_8255
     * The description of this byte is found under the heading
     * of "Diagnostic Sense Keyboard and UART Port Pins". The
     * MSB bit of this byte is 7 and LSB is 0. The bits turned
     * on due to DIAG_WRITE_40 command are 5 and 0. Therefore
     * the returned byte in IOR_PA_8255 would be 0x21.
     */

    data = DIAG_SNSE_KBD_UART;
    Send_Data(fdes,data, 3, 0);
   
    /* Read port A for the ack */

    rc = rd_byte(fdes, &kbddata, IOR_PA_8255);
    kbddata = kbddata >> 24;
    if(!(kbddata == KBD_UART_PIN_ACK))
        return(mktu_rc(fdes,tucb_ptr->header.tu, LOG_ERR, ADP_DIAG_SNSE_ERR));
    
    /* Write a data byte bit to the keyboard clock line and data 
     * line using the Diagnostic Write Keyboard Port Pins command
     */

    data = DIAG_WRITE_C0;
    if ((rc = Send_Data(fdes,data, 0, 0)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, LOG_ERR, ADP_DIAG_WRTC0_ERR));
    
    /* Enable keyboard interface by setting SRAM 11 bit 11(=1)*/

    data = ENBL_KBD_ITF;
    if ((rc = Send_Data(fdes,data, 0, 0)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, LOG_ERR, ADP_ENBL_KBD_ERR));

    /* Reset adapter to diagnostic mode by setting SRAM 11 bit 10(=0) */

    data = SET_DIAG_MDE_OFF;
    if ((rc = Send_Data(fdes,data, 0, 0)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, LOG_ERR, ADP_DIAG_OFF_ERR));
    
    return(rc);
}
