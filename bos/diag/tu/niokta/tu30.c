/* static char sccsid[] = "@(#)49  1.2  src/bos/diag/tu/niokta/tu30.c, tu_adpkbd, bos411, 9431A411a 7/19/94 15:34:11"; */
/*
 * COMPONENT_NAME: tu_adpkbd
 *
 * FUNCTIONS:  	tu30.c 
 *             	Adapter Diagnostics Wrap Test
 * DESCRIPTION: Disable Kbd and UART IRQ
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

int tu30(int fdes, TUTYPE *tucb_ptr)
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
    
    /* Enable the UART by addressing SRAM address 12 bit 4(=1) */

    data = ENBL_UART_ITF;
    if ((rc = Send_Data(fdes,data, 0, 0)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, LOG_ERR, ADP_ENBL_IF_ERR));
    
    /* Blocking of received UART byte is set to inactive by setting 
     * SRAM 10 bit 5(=0)
     */

    data = UART_BLK_INACT;
    if ((rc = Send_Data(fdes,data, 0, 0)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, LOG_ERR, ADP_BLK_INACT_ERR));

    /* The adapter port signals can be diagnostically sensed using
     * the WRITE UART-CONTROL command 0x23 as mentioned in the
     * "Diagnostic Wraps" write up. Any data byte is tagged along
     * with 0x23 and the ack is verified for Diagnostic Wrap
     */ 

    data = DIAG_WRAP_DATA_55;
    Send_Data(fdes,data, 0, 0);
    
    /* To read and verify the wrapped data
     * Read Port C for input buffer and interrupt ID code
     */
    
    /* Read Port A for the wrap data */

    usleep(100 * 1000);
    rd_byte(fdes, &kbddata, IOR_PA_8255);
    kbddata = kbddata >> 24;
    
    if(!(kbddata == WRAP_DATA_55))
        return(mktu_rc(fdes,tucb_ptr->header.tu, LOG_ERR, ADP_WRP_55_ERR));
    
    /* The adapter port signals can be diagnostically sensed using
     * the WRITE UART-CONTROL command 0x23 as mentioned in the
     * "Diagnostic Wraps" write up. Any data byte is tagged along
     * with 0x23 and the ack is verified for Diagnostic Wrap
     */ 

    data = DIAG_WRAP_DATA_9A;
    Send_Data(fdes,data, 0, 0);
    
    /* To read and verify the wrapped data,
     * Read Port C for input buffer and interrupt ID code
     */
    
    /* Read Port A for the wrap data */

    usleep(100 * 1000);
    rd_byte(fdes, &kbddata, IOR_PA_8255);
    kbddata = kbddata >> 24;   

    if(!(kbddata == WRAP_DATA_9A))
        return(mktu_rc(fdes,tucb_ptr->header.tu, LOG_ERR, ADP_WRP_9A_ERR));

    /* Blocking of received UART byte is set to active by setting 
     * SRAM 10 bit 5(=1)
     */

    data = UART_BLK_ACT;
    if ((rc = Send_Data(fdes,data, 0, 0)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, LOG_ERR, ADP_BLK_ACT_ERR));
    
    /* Disable the UART by addressing SRAM address 12 bit 4(=1) */

    data = DSBL_UART_ITF;
    if ((rc = Send_Data(fdes,data, 0, 0)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, LOG_ERR, ADP_DSBL_IF_ERR));

    return(rc);
}
