static char sccsid[] = "@(#)24  1.1  src/bos/diag/tu/ppckbd/tu20.c, tu_ppckbd, bos41J, 9520A_all 5/6/95 14:32:53";
/*
 *   COMPONENT_NAME: tu_ppckbd
 *
 *   FUNCTIONS: tu20
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "tu_type.h"

int tu20(int fdes, TUTYPE *tucb_ptr)
{
  extern int errno;
  unsigned char cdata, kbddata, byte1, byte2;
  char data;
  static int rc = SUCCESS;

  /* Initialize variables */

  kbddata = 0;

  /* open Device Driver in Diagnostics mode */

  if ((rc = open_kbd(tucb_ptr)) != SUCCESS)
     {
        close(kbdtufd);
        return(mktu_rc(fdes,tucb_ptr->header.tu, LOG_ERR, FAILED_DD_OPEN));
     }
  if((rc=set_sem(WAIT_FOR_ONE)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));

  /* Send Keyboard ID command */

  data = KBD_ID_CMD;
  send_data(fdes, data);
  usleep(500);

  /* Now read two kbd id bytes, the second byte must follow the
   * completion of the first byte by no more than 500 micro-seconds
   */

  read_data(fdes, &byte1);
  if ((byte1 != KBD_1_101_ID) && (byte1 != KBD_1_106_ID))
        return(mktu_rc(tucb_ptr->header.tu, KBD_ERR, INVALID_ID));

  usleep(500);
  read_data(fdes, &byte2);
  if ((byte2 != KBD_2_101_ID) && (byte2 != KBD_2_106_ID))
        return(mktu_rc(tucb_ptr->header.tu, KBD_ERR, INVALID_ID));

  /* Send keyboard Echo command */

  data =  KBD_ECHO_CMD;
  send_data(fdes, data);

  /* Verify the echoed data */

  read_data(fdes, &kbddata);
  if (kbddata != ECHO_DATA)
        return(mktu_rc(tucb_ptr->header.tu, KBD_ERR, KBD_ECHO_ERR));

  /* Keyboard Scan Code command */

  data = KBD_SCAN_CDE_CMD;
  send_data(fdes, data);

  /* Select scan code set 3 */

  data = KBD_SCAN_SET3_CMD;
  send_data(fdes, data);

  /* Verify the selected scan code test */
  /* Send Scan set query command */
  /* The Query is F0 00 , send first 0xF0, and then 0x00 */

  data = KBD_SCAN_CDE_CMD;
  send_data(fdes, data);
  data =  KBD_SCAN_QRY_CMD;
  send_data(fdes, data);
  read_data(fdes, &kbddata);
  if (kbddata != SCAN_SET3)
        return(mktu_rc(tucb_ptr->header.tu, KBD_ERR, SCAN_SET3_ERR));

  /* Restore keyboard to Scan code 2 */
  /* Keyboard Scan Code command */

  data = KBD_SCAN_CDE_CMD;
  send_data(fdes, data);

  /* Select scan code set 2 */

  data = KBD_SCAN_SET2_CMD;
  send_data(fdes, data);

  /* Restore keyboard initial conditions */

  if ((rc = kbd_restore(fdes)) != SUCCESS )
        return (mktu_rc(tucb_ptr->header.tu, KBD_ERR, KBD_RESTORE_ERR));

  if ((rc = kbd_reset(fdes)) != SUCCESS )
        return (mktu_rc(tucb_ptr->header.tu, KBD_ERR, KBD_RESET_ERR));

  if((rc=rel_sem()) != SUCCESS)
        return(rc);

  if (kbdtufd != FAILURE)
  	close(kbdtufd);
  return(rc);
}

