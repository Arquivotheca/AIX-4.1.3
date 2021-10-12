/* static char sccsid[] = "@(#)24  1.4  src/bos/diag/tu/tablet/tu70.c, tu_tab, bos411, 9428A410j 5/12/94 15:13:42"; */
/*
 * COMPONENT_NAME: tu_tab
 *
 * FUNCTIONS:  tu70.c
 *             Disable and Enable test
 *
 * DESCRIPTION: Reset POS register
 *              Initialize the tablet
 *              Set Sampling Speed
 *              Set Resolution
 *              Enable the tablet
 *              Check if LED is blinking
 *              Disable the tablet
 *              Check if LED is blinking
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

int tu70(int fdes, TUTYPE *tucb_ptr)
{
  unsigned char cdata, cdata1;
  ushort data;
  static int rc = SUCCESS;
  unsigned char tabdata;
  
#ifdef DIAGNOSTICS
  int  mrc; /* Message return code */
  long msg_num = msgtab[6];  
#endif

  /* Open Device Driver in Diagnostics Mode */
  rc = open_kbd(tucb_ptr);
  
  /* Initialize variables */
  
  tabdata = 0; 

/* 
 * =====> Enable and Disable Tablet Test         <=====
 * =====> User Interactive Test                  <=====
 * =====> Tablet 5083 model 21 and model 22 Only <=====
 */
  
  /* Reset Tablet Adapter Hardware */
  if ((rc = Tab_Reset(fdes)) != SUCCESS)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, ADP_HDW_RSET_ERR));
  
  /* Initialize the tablet */
  if ((rc = Tab_Init(fdes)) != SUCCESS)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, ADP_FRM_O_ERR));
  
  /* Set tablet to incremental mode command */
  data = SET_DATA_MODE_CMD;
  if ((rc = Send_Data(fdes,data, 0, 0)) != SUCCESS)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_INCR_MDE_ERR));
  
  /* Set sampling speed to 50 coord.pairs/sec command */
  data = SET_SAMP_SPEED_CMD;
  if ((rc = Send_Data(fdes,data, 0, 0)) != SUCCESS)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_SAMP_SPD_ERR));
  
  /* the yy value of the command */
  data = SAMP_50_SPEED_CMD;
  if ((rc = Send_Data(fdes,data, 0, 0)) != SUCCESS)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_SMP_SPD_YY_ERR));
  
  /* Set resolution to 10 lines/inch command */
  data = SET_RESOLN_CMD84;
  if ((rc = Send_Data(fdes,data, 0, 0)) != SUCCESS)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_RESL_CMD84_ERR));
  
  /* The aa value of the command */
  data = SET_RESOLN_02_MSB;
  if ((rc = Send_Data(fdes,data, 0, 0)) != SUCCESS)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_RESL_MSB_ERR));
  
  /* The second byte of the command */
  data = SET_RESOLN_CMD85;
  if ((rc = Send_Data(fdes,data, 0, 0)) != SUCCESS)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_RESL_CMD85_ERR));
  
  /* The bb value of the command */
  data = SET_RESOLN_LSB;
  if ((rc = Send_Data(fdes,data, 0, 0)) != SUCCESS)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_RESL_LSB_ERR));
  
  /* Enable tablet command */
  data = TAB_ENBL_CMD;
  if ((rc = Send_Data(fdes,data, 0, 0)) != SUCCESS)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_ENBL_ERR));
  
  /* Prompt user to test tablet enable status 
   * Ask user to move input device and press the switches, making sure
   * indicators function correctly 
   * Ask: Is the light in the small leftmost square at the top of the Tablet
   *      'ON' when the stylus/cursor in motion?
   */

  close(kbdtufd);

#ifdef DIAGNOSTICS
  mrc = putmsg(tucb_ptr, msg_num, TM_37);
  
  if (mrc != YES)    /* User should just hit <Enter> or ESC */
    {
      if (mrc < 0)
	return(rc);
      else if (mrc == CANCEL_KEY_ENTERED || mrc == EXIT_KEY_ENTERED)
	return(mrc);
      else
        return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_FL_ENBL_TEST));
    }
#endif

  rc = open_kbd(tucb_ptr);
  
  /* Prompt user to test tablet disable status */
  /* Disable tablet command */
  data = TAB_DSBL_CMD;
  if ((rc = Send_Data(fdes,data, 0, 0)) != SUCCESS)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_DSBL_ERR));
  
  /* Ask user to move input device and press the switches, making sure
   * indicators function correctly 
   * Ask: Is the light in the small leftmost square at the top of
   *      the Tablet 'OFF' when the Stylus/cursor in motion?
   */

  close(kbdtufd);  

#ifdef DIAGNOSTICS
  msg_num = msgtab[7];
  mrc = putmsg(tucb_ptr, msg_num, TM_38);
  
  if (mrc != YES)    /* User should just hit <Enter> or ESC */
    {
      if (mrc < 0)
	return(rc);
      else if (mrc == CANCEL_KEY_ENTERED || mrc == EXIT_KEY_ENTERED)
	return(mrc);
      else
        return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_FL_DSBL_TEST));
    }
#endif

  return(rc);
}

