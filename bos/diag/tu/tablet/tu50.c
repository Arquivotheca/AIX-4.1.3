/* static char sccsid[] = "@(#)22  1.4  src/bos/diag/tu/tablet/tu50.c, tu_tab, bos411, 9428A410j 5/12/94 15:13:37"; */
/*
 * COMPONENT_NAME: tu_tab
 *
 * FUNCTIONS:  tu50.c
 *             Input Device LED Test
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

int tu50(int fdes, TUTYPE *tucb_ptr)
{
  unsigned char cdata;
  static int rc = SUCCESS;
  
  int  mrc; /* Message return code */
  long msg_num = msgtab[2];
  
/*
 * =====> Tablet LED Test                        <=====
 * =====> User Interactive Test                  <=====
 * =====> Tablet 5083 model 21 and model 22 Only <=====
 * The objective of this test is to check if the tablet is able to recognize 
 * an input device. The oeprator will be asked to place the input device on
 * the rightmost square at the top of the Tablet, at this time the tablet will
 * turn the LED located under the small center square at the top of the tablet.
 * This test will fail is the LED does not turn ON, and turn OFF when input device
 * is withdrawn.
 */ 


  mrc = putmsg(tucb_ptr, msg_num, TM_22);
  
  if (mrc != YES)    /* User should just hit <Enter> or ESC */
    {
      if (mrc < 0)
	return(rc);
      else if (mrc == CANCEL_KEY_ENTERED || mrc == EXIT_KEY_ENTERED)
	return(mrc);
      else
        return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_FAIL_INACT));
    }

  msg_num = msgtab[3];
  mrc = putmsg(tucb_ptr, msg_num, TM_23);
  
  if (mrc != YES)    /* User should just hit <Enter> or ESC */
    {
      if (mrc < 0)
	return(rc);
      else if (mrc == CANCEL_KEY_ENTERED || mrc == EXIT_KEY_ENTERED)
	return(mrc);
      else
        return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_FAIL_ACT));
    }

  return(rc);
}
