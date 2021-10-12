static char sccsid[] = "@(#)23  1.1  src/bos/diag/tu/ppckbd/tu10.c, tu_ppckbd, bos41J, 9520A_all 5/6/95 14:32:46";
/*
 *   COMPONENT_NAME: tu_ppckbd
 *
 *   FUNCTIONS: tu10
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

tu10(int fdes, TUTYPE *tucb_ptr)
{
  int rc = SUCCESS;
  unsigned char   cdata, kbd_data = 0;
  extern int errno;

  /* open Device Driver in Diagnostics mode */

  if ((rc = open_kbd(tucb_ptr)) != SUCCESS)
     {
        close(kbdtufd);
        return(mktu_rc(fdes,tucb_ptr->header.tu, LOG_ERR, FAILED_DD_OPEN));
     }

  if((rc=set_sem(WAIT_FOR_ONE)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));

  if ((rc = kbd_reset(fdes)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, KBD_ERR, KBD_RESET_ERR));

  if ((rc = led_on(fdes)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, KBD_ERR, KBD_LED_ON_ERR));
  sleep(2);
  if ((rc = led_off(fdes)) != SUCCESS)
        return (mktu_rc(tucb_ptr->header.tu, KBD_ERR, KBD_LED_OFF_ERR));

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
