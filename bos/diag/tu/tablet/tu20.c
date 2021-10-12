/* static char sccsid[] = "@(#)19  1.2  src/bos/diag/tu/tablet/tu20.c, tu_tab, bos411, 9428A410j 5/21/94 08:08:19"; */
/*
 * COMPONENT_NAME: tu_tab
 *
 * FUNCTIONS:  tu20.c
 *             Set Wrap and Reset Wrap Mode Test
 *
 * DESCRIPTION: Reset POS register
 *              Initialize the tablet
 *              Send Wrap Mode Command
 *              Send data to test wrap mode
 *              Verify the echoed data
 *              Send Reset Wrap Mode Command
 *              Send data to test for reset wrap mode
 *              Verify for Device Error code
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
  unsigned char cdata;
  ushort data;
  
  static int rc = SUCCESS, count;
  unsigned char tabdata;
  
  /* Open Device Driver in Diagnostics Mode */
  rc = open_kbd(tucb_ptr);

  /* Initialize variables */
  tabdata = 0;
  
  /* Reset Tablet Adapter Hardware */
  if (( rc = Tab_Reset(fdes)) != SUCCESS)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, ADP_HDW_RSET_ERR));

  /* Initialize the tablet */
  if ((rc = Tab_Init(fdes)) != SUCCESS)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, ADP_FRM_O_ERR));

  /* Set wrap mode command */
  data = TAB_SET_WRAP_MODE_CMD;
  if ((rc = Send_Data(fdes, data, 0, 0)) != SUCCESS)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_SET_WRAP_MDE_ERR));

  /* Send data to test wrap mode */
  data = TAB_WRAP_MODE_TEST_DATA;
  if ((rc = Send_Data(fdes, data, 0,0)) != SUCCESS)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_WRP_MDE_DATA_ERR));

  rc = get_data(fdes, 0, 0, &tabdata);
  
  if (tabdata != TAB_WRAP_TEST_DATA)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_WRP_MDE_ERR));
  
  /* Reset wrap mode command */
  data = TAB_RSET_WRAP_MODE_CMD;
  if ((rc = Send_Data(fdes, data, 0, 0)) != SUCCESS)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_RST_WRP_MDE_ERR));

  /* To test if tablet is still in wrap mode by sending data*/
  data = TAB_WRAP_MODE_TEST_DATA;
  if ((rc = Send_Data(fdes, data, 0, 0)) != SUCCESS)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_RST_WRP_MDE_DTA_ERR));

  rc = get_data(fdes, 0, 0, &tabdata); 

  if (tabdata != UART_Q_TIMEOUT)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_RST_WRP_MDE_ERR));
  
  close(kbdtufd);
  return(rc);
}
