/* static char sccsid[] = "@(#)23  1.3  src/bos/diag/tu/tablet/tu60.c, tu_tab, bos411, 9428A410j 5/12/94 15:13:40"; */
/*
 * COMPONENT_NAME: tu_tab
 *
 * FUNCTIONS:  tu60.c
 *             Input Device Switch Test
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

int tu60(int fdes, TUTYPE *tucb_ptr)
{
  unsigned char cdata;
  static int rc = SUCCESS;
  int  mrc; /* Message return code */
  long msg_num = msgtab[4];  
  
/*
 * =====> Tablet Input Device Switch Test        <=====
 * =====> User Interactive Test                  <=====
 * =====> Tablet 5083 model 21 and model 22 Only <=====
 * This test will request that user place input device in active area to test
 * switches. The operator will be requested to press the stylus or cursor button
 * The operator will be asked if the light in the small center square at the top 
 * of the Tablet 'ON' and 'OFF' when releasing the button. If tablet does not 
 * behave correspondingly, an error will be reported.
 */

  mrc = putmsg(tucb_ptr, msg_num, TM_23A);
  
  if (mrc != YES)    /* User should just hit <Enter> or ESC */
    {
      if (mrc < 0)
	return(rc);
      else if (mrc == CANCEL_KEY_ENTERED || mrc == EXIT_KEY_ENTERED)
	return(mrc);
      else
        return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_FL_PRES_SWTCH));
    }

  msg_num = msgtab[5];
  mrc = putmsg(tucb_ptr, msg_num, TM_24);
  
  if (mrc != YES)    /* User should just hit <Enter> or ESC */
    {
      if (mrc < 0)
	return(rc);
      else if (mrc == CANCEL_KEY_ENTERED || mrc == EXIT_KEY_ENTERED)
	return(mrc);
      else
        return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_FL_REL_SWTCH));
    }

  return(rc);
}
