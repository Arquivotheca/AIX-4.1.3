/* static char sccsid[] = "@(#)20  1.2  src/bos/diag/tu/tablet/tu30.c, tu_tab, bos411, 9428A410j 5/21/94 08:07:29"; */
/*
 * COMPONENT_NAME: tu_tab
 *
 * FUNCTIONS:  tu30.c
 *             Set Conversion and Resolution Test
 *
 * DESCRIPTION: Reset POS register
 *              Initialize the tablet
 *              Set Conversion to Metric
 *              Set Resolution to 200 LPCM
 *              Send Read Current Status/Coord. command
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

int tu30(int fdes, TUTYPE *tucb_ptr)
{
  unsigned char cdata;
  ushort data;
  static int rc = SUCCESS;
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
  
  /* Set conversion command */
  data = TAB_SET_CONVER_CMD;
  if ((rc = Send_Data(fdes, data, 0, 0)) != SUCCESS)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_SET_CONVER_ERR));

  /* Set conversion to metric command */
  data = TAB_SET_METRIC_CMD;
  if ((rc = Send_Data(fdes, data, 0, 0)) != SUCCESS)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_SET_METRIC_ERR));

  /* SET RESOLUTION IS A TWO 2-BYTE COMMAND */
  /* First Byte */
  /* Set resolution command */
  data = SET_RESOLN_CMD84;
  if ((rc = Send_Data(fdes, data, 0, 0)) != SUCCESS)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_RESLN_BYTE1_ERR));

  /* Set resolution MSB of 200 LPCM(scale factor) command */
  data = SET_RESOLN_MSB_CMD65;
  if ((rc = Send_Data(fdes, data, 0, 0)) != SUCCESS)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_RESLN_MSB_ERR));
  
  /* Second Byte */
  /* Set resolution command */
  data = SET_RESOLN_CMD85;
  if ((rc = Send_Data(fdes, data, 0, 0)) != SUCCESS)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_RESLN_BYTE2_ERR));

  /* Set resolution LSB of 200 LPCM(scale factor) command */
  data = SET_RESOLN_LSB_CMD99;
  if ((rc = Send_Data(fdes, data, 0, 0)) != SUCCESS)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_RESLN_LSB_ERR));

  /* Read current status/coordinate command */
  if ((rc = tab_status(fdes)) != SUCCESS)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_RD_ST_ERR));
  
  close(kbdtufd);
  return(rc);
}
