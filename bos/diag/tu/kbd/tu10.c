/* static char sccsid[] = "@(#)98  1.3  src/bos/diag/tu/kbd/tu10.c, tu_kbd, bos411, 9433A411a 7/13/94 09:35:51"; */
/*
 * COMPONENT_NAME: tu_kbd 
 *
 * FUNCTIONS:  	tu10.c 
 *             	
 * DESCRIPTION: Open Device Driver 
 *              Keyboard Reset Test 
 *		Reset POS register
 *		Send Reset Command
 *		Get BAT completion
 *		Set Speaker to high volume
 *		Send Higher byte of Set 30 Hz Frequency
 *		Send Lower byte of Set 30 Hz Frequency
 *		Send Higher byte of Set 2 Sec Duration 
 *		Send Lower byte of Set 2 Sec Duration 
 *		Wait for 4 secs for the completion of the speaker tone
 *	        Close Device Driver	
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

#include "tu_type.h"

extern int mktu_rc();

int tu10(int fdes, TUTYPE *tucb_ptr)
{
    extern int errno;
    unsigned char cdata, kbddata;
    ushort data;
    static int rc = SUCCESS;
    
    /* open Device Driver in Diagnostics mode */
    rc = open_kbd(tucb_ptr);

    /* Initialize variables */
    kbddata = 0;
    
    /* Reset Keyboard Adapter Hardware */
    if (( rc = Kbd_Reset(fdes)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, ADP_HDW_RSET_ERR));
 
    /* Send Reset command */
    data = KBD_RSET_CMD;
    if ((rc = send_kbd(fdes, data)) != SUCCESS)
        return(mktu_rc(fdes, tucb_ptr->header.tu, KBD_ERR, DEV_RESET_ERR));
    
    /* Read port A for BAT comp. code */
    if ((rc = get_data(fdes,0x08,KBD_CMP_CDE,&kbddata)) != SUCCESS)
        return(mktu_rc(fdes, tucb_ptr->header.tu, KBD_ERR, ADP_PA_RST_ERR));
    
    /* Set speaker to high volume */
    /* Speaker is set to high volume by Initialize speaker volume cmd */

    data = SPK_HVOL_CMD;
    rc = Send_Data(fdes, data,0 ,KBD_CMD_ACCP);
    
    /* Set speaker to 30 Hz frequency */
    
    /* This is a two byte command and the value of both the bytes are    
     * calculated using the table in the sub-heading of Adapter Speaker
     * Control. 
     * A sample calculation is given below    
     * The high byte is called Cx and for 30Hz frequency it is defined as
     * decimal 64 which 0x40. It is tagged along with 0x08 of the Set
     * Speaker Frequency command.
     * The low byte is is called Ct and is calculated as (6000/30) - 9.31
     * where 30 is the given frequency. The decimal value is 191 and the
     * hex value is 0xBF which is tagged along with 0x09 of the Set Speaker
     * Frequency command */
    
    /* Send high order byte */

    data = SPK_30HZ_HBYT_CMD;
    rc = Send_Data(fdes, data, 0, KBD_CMD_ACCP);
    
    /* Send low order byte */

    data = SPK_30HZ_LBYT_CMD;
    rc = Send_Data(fdes, data, 0, KBD_CMD_ACCP);
    
    /* To set speaker duration for 2 secs */
    /* This a two byte command and is as follows.
     * The first byte which is the high byte is tagged along with 0x07 
     * which is the Set Speaker Duration command  
     * The second byte which is the low byte is tagged along with 0x02
     * which is Write to Speaker Command since this is specified in the
     * last two lines of Speaker Duration Control write up. */
    
    /* The high order byte */

    data = SPK_2SEC_DUR_HBYT_CMD;
    rc = Send_Data(fdes, data, 0, KBD_CMD_ACCP);
    
    /* Send low order byte */

    data = SPK_2SEC_DUR_LBYT_CMD;
    rc = Send_Data(fdes, data, 0, SPK_CMD_ACCP);
    
    /* A delay of 4 secs to take into account the duration of the
     * speaker tone. Without this delay the speaker is executing
     * the tone and the adapter is processing the next command.
     * Due to this the buffer is filled with the ack from the
     * next command where as the ack of the speaker tone is still
     * due. This delay will wait for the ack of the speaker tone
     * before the next command is executed. */
    usleep(4000 * 1000);
    
    close(kbdtufd);
    return(rc);
}
