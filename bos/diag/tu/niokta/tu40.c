/* static char sccsid[] = "@(#)44  1.2  src/bos/diag/tu/niokta/tu40.c, tu_adpkbd, bos411, 9431A411a 7/19/94 15:34:26"; */
/*
 * COMPONENT_NAME: tu_adpkbd
 *
 * FUNCTIONS:  tu40.c
 *             External Wrap Test
 *
 * DESCRIPTION: Disable Kbd and UART IRQ
 *              Reset POS register
 *              Initialize the tablet
 *              Send Wrap Mode Command
 *              Send data to test wrap mode
 *              Verify the wraped data
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <sys/devinfo.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mode.h>
#include <sys/mdio.h>
#include <sys/signal.h>
#include "tu_type.h"

extern int mktu_rc();

int tu40(fdes, tucb_ptr)
int fdes;
TUTYPE *tucb_ptr;
{
  unsigned char cdata;
  ushort data;
  unsigned int kbddata;
  static int rc = SUCCESS, count;
  int i,j,k;

  /* 
   * Baud rate of 300, 600, 1200, 2400, 4800, 9600, 24000 
   * Assuming an OSC value of 9.216 MHz.   
   * register shold be loaded with the following values respectively 
   * 0x60, 0xB0, 0xD8, 0xEC, 0xF6, 0xFB, 0xFE 
   */

  int b_rate = 0xFB;
  int t_code = 0xFE;

  /* Initialize variables */
  kbddata = 0;

  /* Reset Tablet Adapter Hardware */

  if ((rc = Kbd_Reset(fdes)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, KBD_ERR,ADP_HDW_RSET_ERR));

  /* Initialize UART starts */
  /* Enable the UART by addressing SRAM at address 12 and bit 4=(1)*/

  data = ENBL_UART_ITF;
  if ((rc = Send_Data(fdes,data, 0, 0)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_ENBL_IF_ERR));

  /* Blocking of received UART bytes is set to inactive by setting
   * SRAM 10 bit 5(=0) */

  data = UART_BLK_INACT;
  if ((rc = Send_Data(fdes,data, 0, 0)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_BLK_INACT_ERR));

  /* Initialize UART Framing command */

  data = INIT_UART_FRM_E; /* Set frame to EVEN parity */
  if ((rc = Send_Data(fdes,data, 0, 0)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, KBD_ERR, ADP_FRM_O_ERR));

  /* Initialize UART ends */

  data = b_rate << 8;
  data |= SET_BAUD_CMD;

  /* Set baud rate */

  if ((rc = Send_Data(fdes,data, 0, 0)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_WRAP_ERR));

  data = t_code << 8;
  data |= WRITE_2_TABLET;
  Send_Data(fdes,data, 0, 0);
  sleep(1);
  rd_byte(fdes, &kbddata, IOR_PA_8255);
  kbddata = kbddata >> 24;
  if (kbddata != t_code)
       return(mktu_rc(tucb_ptr->header.tu, KBD_ERR, ADP_WRP_MDE_ERR));

  /* Reset Tablet Adapter Hardware */

  if ((rc = Kbd_Reset(fdes)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, KBD_ERR, ADP_HDW_RSET_ERR));

  return(rc);
}
