/* static char sccsid[] = "@(#)47  1.2  src/bos/diag/tu/niokta/tu20.c, tu_adpkbd, bos411, 9431A411a 7/12/94 16:04:32"; */
/*
 * COMPONENT_NAME: tu_adpkbd
 *
 * FUNCTIONS:  	tu20.c 
 *             	Read SRAM and Invalid Commands Test
 * DESCRIPTION: Disable Kbd and UART IRQ
 *		Reset POS register
 *		Read SRAM 0x0D 
 *		Read SRAM 0x00 
 *	        Send Invalid command 	
 *	        Send Invalid scan count command 	
 *	        Send Invalid mode command 	
 *	        Send Invalid extended byte command 	
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


int tu20(int fdes, TUTYPE *tucb_ptr)
{
    extern int errno;
    unsigned char cdata;
    unsigned int kbddata;
    ushort data;
    static int rc = SUCCESS;

    /* Initialize variables */
    kbddata = 0;

    /* Reset Keyboard Adapter Hardware */

    if ((rc = Kbd_Reset(fdes)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, LOG_ERR, ADP_HDW_RSET_ERR));

    /* Read Shared RAM address 0x0D for the keystroke initiate
     * system attention ack
     */

    data = RD_SRAM_0D;
    Send_Data(fdes,data, 0, 0);
    
    /* Read Port A for the ack */

    rd_byte(fdes, &kbddata, IOR_PA_8255);
    kbddata = kbddata >> 24;
    if(!(kbddata == RD_SRAM_0D_ACK))
        return(mktu_rc(fdes,tucb_ptr->header.tu, LOG_ERR, ADP_RD_SRAM_0D_ERR));

    /* Read Shared RAM address 0x00 for the keystroke ack byte */ 

    data = RD_SRAM_00;
    Send_Data(fdes,data, 0, 0);
    
    /* Read Port A for the ack */

    rd_byte(fdes, &kbddata, IOR_PA_8255);
    kbddata = kbddata >> 24;   
    if(!(kbddata == RD_SRAM_00_ACK))
        return(mktu_rc(fdes,tucb_ptr->header.tu, LOG_ERR, ADP_RD_SRAM_00_ERR));
    
    /* Send an invalid command to the adapter */ 

    data = INVALID_CMD;
    Send_Data(fdes,data, 0, 0);
    
    /* Read Port A for the ack */

    rd_byte(fdes, &kbddata, IOR_PA_8255);
    kbddata = kbddata >> 24;   
    if (!(kbddata == INVALID_CMD_ACK))
       return(mktu_rc(fdes,tucb_ptr->header.tu, LOG_ERR, ADP_INVALID_CMD_ERR));
    
    /* Send an invalid Set Scan Count command to the adapter */ 

    data = INVALID_SCN_CNT_CMD;
    Send_Data(fdes,data, 0, 0);
    
    /* Read Port A for the ack */

    rd_byte(fdes, &kbddata, IOR_PA_8255);
    kbddata = kbddata >> 24;
    if (!(kbddata == INVALID_SCN_ACK))
      return(mktu_rc(fdes,tucb_ptr->header.tu, LOG_ERR, ADP_INVL_SCN_CNT_ERR));

    /* Send an invalid Mode command to the adapter */ 

    data = INVALID_MDE_CMD;
    Send_Data(fdes,data, 0, 0);
    
    /* Read Port A for the ack */

    rd_byte(fdes, &kbddata, IOR_PA_8255);
    kbddata = kbddata >> 24;   
    if(!(kbddata == INVALID_MDE_ACK))
       return(mktu_rc(fdes,tucb_ptr->header.tu, LOG_ERR, ADP_INVALID_MDE_ERR));

    /* Send an invalid extended command to the adapter */ 

    data = INVALID_EXT_CMD;
    Send_Data(fdes,data, 0, 0);
    
    /* Read Port A for the ack */

    rd_byte(fdes, &kbddata, IOR_PA_8255);
    kbddata = kbddata >> 24;   
    if(!(kbddata == INVALID_CMD_ACK))
       return(mktu_rc(fdes,tucb_ptr->header.tu, LOG_ERR, ADP_INVALID_CMD_ERR));
    
    return(rc);
}
