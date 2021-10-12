/* static char sccsid[] = "@(#)21  1.5  src/bos/diag/tu/tablet/tu40.c, tu_tab, bos411, 9428A410j 5/27/94 15:38:37"; */
/*
 * COMPONENT_NAME: tu_tab
 *
 * FUNCTIONS:  tu40.c
 *             Tablet LED Test
 *
 * DESCRIPTION: Reset POS regsiter
 *              Initialize the tablet
 *              Enable the Tablet
 *              Send Read Current Status/Coord. command
 *              Check for LED blinking
 *              Disable the tablet
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

#ifdef DIAGNOSTICS
#include "dtablet_msg.h"
#endif

extern int mktu_rc();

int tu40(int fdes, TUTYPE *tucb_ptr)
{
  unsigned char cdata;
  ushort data;
  static int rc = SUCCESS;
  unsigned char tabdata;
  int count;
  
#ifdef DIAGNOSTICS
  int  mrc; /* Message return code */
  long msg_num = msgtab[0];
#endif

 /* Initialize variables */
  tabdata = 0;

#ifdef DIAGNOSTICS

  mrc = dsply_test_msg(tucb_ptr, msg_num, TM_10);
  if (mrc == CANCEL_KEY_ENTERED || mrc == EXIT_KEY_ENTERED)
       return(mrc);
#endif

  /* Open Device Driver in Diagnostics Mode */
  rc = open_kbd(tucb_ptr);

  /* Reset Tablet Adapter Hardware */
  if (( rc = Tab_Reset(fdes)) != SUCCESS)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, ADP_HDW_RSET_ERR));
  
  /* Initialize the tablet */
  if ((rc = Tab_Init(fdes)) != SUCCESS)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, ADP_FRM_O_ERR));
  
/*
 * =====> Tablet LED Test                        <=====
 * =====> User Interactive Test                  <=====
 * =====> Tablet 5083 model 21 and model 22 Only <===== 
 * Operator will be requested to Move the Input Device off the surface of the
 * Tablet inside the large square and to check if the light in the small
 * leftmost square is BLINKING  
 */

  data = TAB_ENBL_CMD;
  if ((rc = Send_Data(fdes, data, 0, 0)) != SUCCESS)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_ENBL_ERR));

  /* Read current status/coordinate command */
  data = TAB_RD_STAT_CMD;
  for(count =0; count < 10;count++)
     {
        rc = Send_Data(fdes, data, 0, 0);
        usleep(100 * 1000);
     }
  
  /* Ask user if tablet indicator LED was flickering */
  
  close(kbdtufd);

#ifdef DIAGNOSTICS
  msg_num = msgtab[1];
  mrc = putmsg(tucb_ptr, msg_num, TM_11);
  
  if (mrc != YES)    /* User should just hit <Enter> or ESC */
    {
      if (mrc < 0)
	return(rc);
      else if (mrc == CANCEL_KEY_ENTERED || mrc == EXIT_KEY_ENTERED)
	return(mrc);
      else
        return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_FAIL_LED_TEST));
    }
#endif

  rc = open_kbd(tucb_ptr); 
  
  /* Disable tablet command */
  data = TAB_DSBL_CMD;
  if (( rc = Send_Data(fdes, data, 0, 0)) != SUCCESS)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_DSBL_ERR));
  
  close(kbdtufd); 
  return(rc);
}
