/* static char sccsid[] = "@(#)99  1.3  src/bos/diag/tu/kbd/tu20.c, tu_kbd, bos411, 9433A411a 7/13/94 09:37:04"; */
/*
 * COMPONENT_NAME: tu_kbd 
 * FUNCTIONS:   tu20.c	   
 *              Keyboard Commands Test
 *
 * DESCRIPTION:	Reset POS register
 *		Send Keyboard ID command
 *		Send Keyboard Layout command
 *		Send keyboard Echo command
 *		Send Keyboard Scan code command
 *		Send Keyboard Scan Code Set 1 command
 *		Send Scan Set Query command
 *		Restore keyboard to Scan Code 3
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

int tu20(int fdes, TUTYPE *tucb_ptr)
{
    extern int errno;
    unsigned char cdata, kbddata, byte1, byte2;
    ushort data;
    int keyboard;
    
    static int rc = SUCCESS;

    /* open Device Driver in Diagnostics mode */
    rc = open_kbd(tucb_ptr);

    /* Initialize variables */

    kbddata = 0;
    keyboard = 0;  /* 106 keyboard */
    
    /* Reset Keyboard Adapter Hardware */
    if (( rc = Kbd_Reset(fdes)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, ADP_HDW_RSET_ERR));
    
    /* Send Keyboard ID command */
    data = KBD_ID_CMD;
    if ((rc = send_kbd(fdes, data)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_ID_ERR));
    
    /* Read port A for keyboard ID */
    if ((rc = get_data(fdes,1 , 0, &kbddata)) != SUCCESS) 
        return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, ADP_PA_ID_ERR));
    
    byte1 = kbddata;
    if ((byte1 != KBD_1_101_ID) && (byte1 != KBD_1_106_ID))
        return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_ID_BYT1_ERR));
    
    /* Second Byte */
    /* Read port A for keyboard ID */
    if ((rc = get_data(fdes,1 , 0, &kbddata)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, ADP_PA_ID_ERR));
    
    byte2 = kbddata;
    if ((byte2 != KBD_2_101_ID) && (byte2 != KBD_2_106_ID))
        return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_ID_BYT2_ERR));

    /* Keyboard Layout ID */
    if (byte1 == KBD_1_101_ID && byte2 == KBD_2_101_ID)
      { 
         keyboard = 1;
    /* Send keyboard layout command */
         data =  KBD_LAY_ID_CMD;
         if ((rc = send_kbd(fdes, data)) != SUCCESS)
             return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_LAY_ID_ERR));
    
    /* To read and verify the keyboard layout ID bytes */
    /* First Byte */
    /* Read port A for keyboard layout ID */

         if ((rc = get_data(fdes,1 , 0, &kbddata)) != SUCCESS)
             return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, ADP_PA_LID_ERR));
    
         byte1 = kbddata;

         if ((byte1 != KBD_LAY_101) && (byte1 != KBD_LAY_102))
           return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_LID_BYT1_ERR));
    
    /* Second Byte */
    /* Read port A for keyboard ID */

         if ((rc = get_data(fdes,1 , 0, &kbddata)) != SUCCESS)
            return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, ADP_PA_LID_ERR));
    
         if (kbddata != KBD_2ND_LAY_ID)
           return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_LID_BYT2_ERR));
      }

    /* Send keyboard Echo command */
    data =  KBD_ECHO_CMD;
    if ((rc = send_kbd(fdes, data)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_ECHO_ERR));
    
    /* Verify the echoed data */
    /* Read port A for echoed data */

    if ((rc = get_data(fdes, 8, ECHO_DATA, &kbddata)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, ADP_PA_ECH_ERR));
    
    if (keyboard == 1) 
       {
    /* Keyboard Scan Code command */
          data = KBD_SCAN_CDE_CMD;
          if ((rc = send_kbd(fdes, data)) != SUCCESS)
              return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_SCAN_ERR));
    
    /* Select scan code set 1 */
          data = KBD_SCAN_SET1_CMD;
          if ((rc = send_kbd(fdes, data)) != SUCCESS)
             return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_SCAN_SET1_ERR));
    
    /* Verify the selected scan code test */
    /* Keyboard Scan Code command */
          data = KBD_SCAN_CDE_CMD;
          if ((rc = send_kbd(fdes, data)) != SUCCESS)
              return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_SCAN_ERR));

    /* Send Scan set query command */
          data =  KBD_SCAN_QRY_CMD;
          if ((rc = send_kbd(fdes, data)) != SUCCESS)
             return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_SCN_QRY_ERR));
    
    /* Read port A for echoed data */
          if ((rc = get_data(fdes,1 , 0, &kbddata)) != SUCCESS)
              return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, ADP_PA_QRY_ERR));
    
          if (kbddata != SCAN_SET1)
             return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_SCAN_SET1_ERR));

    /* Restore keyboard to Scan code 3 */
    /* Keyboard Scan Code command */
          data = KBD_SCAN_CDE_CMD;
          if ((rc = send_kbd(fdes, data)) != SUCCESS)
              return(fdes,mktu_rc(tucb_ptr->header.tu, KBD_ERR, DEV_SCAN_ERR));
    
    /* Select scan code set 3 */
          data = KBD_SCAN_SET3_CMD;
          if ((rc = send_kbd(fdes, data)) != SUCCESS)
             return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_SCAN_SET3_ERR));
    
       }
    
    close(kbdtufd);
    return(rc);
}
