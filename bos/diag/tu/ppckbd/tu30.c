static char sccsid[] = "@(#)25  1.1  src/bos/diag/tu/ppckbd/tu30.c, tu_ppckbd, bos41J, 9520A_all 5/6/95 14:32:57";
/*
 *   COMPONENT_NAME: tu_ppckbd
 *
 *   FUNCTIONS: tu30
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

extern int mktu_rc();

int tu30(int fdes, TUTYPE *tucb_ptr)
{
  extern int errno;
  unsigned char cdata;
  unsigned char data;

  static int rc = SUCCESS;

#ifdef DIAGNOSTICS
  int  mrc; /* Message return code */

  mtp = &msgtab[0];    /* Initialize pointer into message table */
  mrc = putmsg(tucb_ptr, mtp); /* keyboard_explain msg */
  if (mrc != 0) 
	return(mrc);
  mrc = putmsg(tucb_ptr, ++mtp); /* keyboard_no_enter msg (same w/out yes/no) */
  if (mrc != 0) 
        return(mrc);
#endif

  /* open Device Driver in Diagnostics mode */

  if ((rc = open_kbd(tucb_ptr)) != SUCCESS)
     {
        close(kbdtufd);
        return(mktu_rc(fdes,tucb_ptr->header.tu, LOG_ERR, FAILED_DD_OPEN));
     }

  if((rc=set_sem(WAIT_FOR_ONE)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));

#ifdef nodiag

        detrace(0, "\n =====> KEYBOARD INDICATORS TEST         <=====");
        detrace(0, "\n =====> TEST UNIT 30                     <=====");
        detrace(0, "\n =====> THIS IS AN USER INTERACTIVE TEST <=====\n\n\n");
        detrace(0,"\nThe LED indicators on the keyboard attached to");
        detrace(0,"\nthe system keyboard adapter should now be on.");
        detrace(0,"\nfor 3 seconds. \n");

#endif

  /* Send Restore Indicator command */

  data = RST_MDE_IND_CMD;
  send_data(fdes, data);

  /* Send Indicators ON command */

  data =  SET_IND_ON_CMD;
  send_data(fdes, data);
  sleep(3);

  /* Send Restore Indicator command */

  data = RST_MDE_IND_CMD;
  send_data(fdes, data);

  /* Send Indicators OFF command */

  data =  SET_IND_OFF_CMD;
  send_data(fdes, data);

  /* Restore keyboard initial conditions */

  if ((rc = kbd_reset(fdes)) != SUCCESS )
        return (mktu_rc(tucb_ptr->header.tu, KBD_ERR, KBD_RESET_ERR));

  if((rc=rel_sem()) != SUCCESS)
        return(rc);

  if (kbdtufd != FAILURE)
  	close(kbdtufd);

#ifdef nodiag
   printf("\n Did the keyboard LED indicators work? (y/n)");
   cdata=NULL;
   scanf("%c", &cdata);
   if (cdata == 'n' || cdata == 'N')
        return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, FAIL_IND_TEST));
#endif

#ifdef DIAGNOSTICS

  mrc = putmsg(tucb_ptr, ++mtp); /* LEDs on? yes/no msg */
				
  if (mrc != YES)    /* User should just hit <Enter> or ESC */
  {
    if (mrc < 0)
         return(rc);
    else if (mrc == CANCEL_KEY_ENTERED || mrc == EXIT_KEY_ENTERED)
      return(mrc);
    else
      return(mktu_rc(tucb_ptr->header.tu, SYS_ERR, DEV_IND_ON_ERR));
  }
  mrc = putmsg(tucb_ptr, ++mtp); /* LEDs off yes/no msg */

  if (mrc != YES)    /* User should just hit <Enter> or ESC */
  {
    if (mrc < 0)
         return(rc);
    else if (mrc == CANCEL_KEY_ENTERED || mrc == EXIT_KEY_ENTERED)
      return(mrc);
    else
      return(mktu_rc(tucb_ptr->header.tu, SYS_ERR, DEV_FAIL_IND_TEST));
  }

#endif

  return(rc);
}
