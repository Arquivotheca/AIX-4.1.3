/* static char sccsid[] = "@(#)62  1.1.1.2  src/bos/diag/tu/mouse/tu50.c, tu_mouse, bos411, 9431A411a 7/13/94 08:47:40"; */
/*
 * COMPONENT_NAME: tu_mouse
 *
 * FUNCTIONS: tu50
 *
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

#include <sys/signal.h>
#include <sys/mode.h>
#include <sys/mdio.h>


/***************************************************************************/
/* NOTE: This function is called by Hardware exerciser (HTX),Manufacturing */
/*       application and Diagnostic application to invoke a test unit (TU) */
/*                                                                         */
/*       If the mfg mode in the tu control block (tucb) is set to be       */
/*       invoked by HTX then TU program will look at variables in tu       */
/*       control block for values from the rule file. Else, TU program     */
/*       uses the predefined values.                                       */
/*                                                                         */
/***************************************************************************/

#include "tu_type.h"
#include "mou_io.h"
extern int mktu_rc();

int tu50(fdes,tucb_ptr)
  int fdes;
  TUTYPE *tucb_ptr;
{
  extern int errno;
  int count;
  unsigned char cdata;
  ushort data;
  int i, rc = SUCCESS, restrc = SUCCESS;
  union regs {
                unsigned int lreg;
                unsigned char reg[4];
             } mouse;

#ifdef debugg
  detrace(0, "\n\ntu05:   Begins...\n");
#endif

/* Initialize variables */

  mouse.lreg = 0;
  count = 0;

  /* initialize mouse device */

  if ((rc = init_dev(fdes)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, rc));


  /* if mouse interrupts disabled */
  /* Disable mouse adapter block mode */

  if ((rc = non_block(fdes)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));

  /* Read from mouse receive/stat reg - Make sure data buffer is empty */

  for (count = 0; count < READ_COUNT ; count++)
        rd_word(fdes, &mouse.lreg, MOUSE_RX_STAT_REG);

  /* If mouse device is disabled */
  /* Send 'read device type' command */

  data = READ_DEV_TYP_COMMAND;
  if ((rc = sendack(fdes, data)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_READ_DEV_TYP_ERR));

  /* Read from mouse 32 bit register */

  if ((rc = rdata(fdes, &mouse.lreg)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));

  /* Verify that '0x00' value returned */

  if (mouse.reg[1] != 0x00)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_READ_DEV_TYP_ERR));

  /* Send 'read data' command */

  data = READ_DATA_COMMAND;
  if ((rc = sendack(fdes, data)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_RD_DATA_REPORT_ERR));

  /* Read from mouse 32 bit register */
  /* Read byte1 */

  if ((rc = rdata(fdes, &mouse.lreg)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_BYTE1_ERR));

  /* Check that bit 3 of byte1 is on */

  if (!(mouse.reg[1] & 0x08))
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_RD_DATA_REPORT_ERR));

  /* Read byte2 */

  if ((rc = rdata(fdes, &mouse.lreg)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_BYTE2_ERR));

  /* Read byte3 */

  if ((rc = rdata(fdes, &mouse.lreg)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_BYTE3_ERR));

  /* Set mouse adapter back to block mode and enable mouse interrupts */

    if ((rc = re_block(fdes)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, ADP_BLK_MOD_ERR));

  return(rc);
}
