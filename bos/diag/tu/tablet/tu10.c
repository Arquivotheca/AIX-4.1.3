/* static char sccsid[] = "@(#)18  1.2  src/bos/diag/tu/tablet/tu10.c, tu_tab, bos411, 9428A410j 5/12/94 15:13:28"; */
/*
 * COMPONENT_NAME: tu_tab
 *
 * FUNCTIONS:  tu10.c
 *             Reset, Read Configuration, and Read Status/Coord. Test
 *
 * DESCRIPTION: Reset POS register
 *              Initialize the tablet
 *              Send Tablet Reset Command
 *              Send Read Configuration Command
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

int tu10(int fdes, TUTYPE *tucb_ptr)
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
  if ((rc = Tab_Reset(fdes)) != SUCCESS)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, ADP_HDW_RSET_ERR));

  /* Initialize the tablet */
  if ((rc = Tab_Init(fdes)) != SUCCESS)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, ADP_FRM_O_ERR));

  /* Reset command */
  data = TAB_RESET_CMD;
  if ((rc = Send_Data(fdes, data, 0, 0)) != SUCCESS)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_RESET_ERR));
  
  usleep(500 * 1000);

  /* Read configuration command  */
  data = TAB_RD_CFG_CMD;
  if ((rc = Send_Data(fdes, data, 0, 0)) != SUCCESS)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_RD_CFG_ERR));

  /* There will only one byte of information from the
   * READ CONFIGURATION COMMAND. It will indicate whether a TABLET
   * is connected or a CURSOR PAD is connected. CURSOR PAD is the one
   * with 6.1 x 6.1 IN. surface area and the TABLET is one with
   * 11.5 x 11.5 IN. surface in area.
   */

  rc = get_data(fdes, 0, 0, &tabdata);

  /* Byte to be compared for specified tablet and cursor pad are attached */
   if ((tabdata != KNOWN_TABLET)
        && (tabdata != KNOWN_CURSOR_PAD))
        return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_UNKNOWN_ERR)); 

  /* Read current status/coordinate command */

  if ((rc = tab_status(fdes)) != SUCCESS)
    return(mktu_rc(tucb_ptr->header.tu, TAB_ERR, DEV_RD_ST_ERR));

  close(kbdtufd);
  return(rc);
}
