/* static char sccsid[] = "@(#)61  1.1.1.2  src/bos/diag/tu/mouse/tu40.c, tu_mouse, bos411, 9431A411a 7/13/94 08:47:36"; */
/*
 * COMPONENT_NAME: tu_mouse
 *
 * FUNCTIONS: tu40
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

int tu40(fdes,tucb_ptr)
  int fdes;
  TUTYPE  *tucb_ptr;
{
  extern int errno;
  int count;
  unsigned char byte1, byte2, byte3, cdata;
  ushort data;
  int i, rc = SUCCESS, restrc = SUCCESS;
  union regs {
                unsigned int lreg;
                unsigned char reg[4];
             } mouse;

#ifdef debugg
  detrace(0, "\n\ntu04:   Begins...\n");
#endif

/* Initialize variables */

  mouse.lreg = 0;
  count = 0;
  byte1 = byte2 = byte3 = 0;

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

    /* If mouse device disabled */
    /* Send mouse status request command */

  data = STATUS_RQST_COMMAND;
  if ((rc = sendack(fdes, data)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_RQST_STAT_ERR));

  /* Save original mouse settings */
  /* Read mouse 32 bit register to make sure mouse is disabled */
  /* Read 1st byte */

  if ((rc = rdata(fdes, &mouse.lreg)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_BYTE1_ERR));

  byte1 = mouse.reg[1];

  /* Read 2nd byte */

  if ((rc = rdata(fdes, &mouse.lreg)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_BYTE2_ERR));

  byte2 = mouse.reg[1];

  /* Read 3rd byte */

  if ((rc = rdata(fdes, &mouse.lreg)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_BYTE3_ERR));

  byte3 = mouse.reg[1];

  /* Send 'set default' command */

  data = SET_DEFAULT;
  if ((rc = sendack(fdes, data)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_DEFAULT_ERR));

  /* Get status report to confirm default settings */
  /* Send mouse status request command */

  data = STATUS_RQST_COMMAND;
  if ((rc = sendack(fdes, data)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_RQST_STAT_ERR));

  /* Read mouse 32 bit register */
  /* Read 1st byte */

  if ((rc = rdata(fdes, &mouse.lreg)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_BYTE1_ERR));

  if ((mouse.reg[1] & 0x70) != 0x00)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_DEFAULT_ERR));

      /* Read 2nd byte */

  if ((rc = rdata(fdes, &mouse.lreg)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_BYTE2_ERR));

  if (mouse.reg[1] != 0x02)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_DEFAULT_ERR));

      /* Read 3rd byte */

  if ((rc = rdata(fdes, &mouse.lreg)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_BYTE3_ERR));

  if (mouse.reg[1] != 0x64)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_DEFAULT_ERR));

     /* Restore original mouse settings */

  if ((byte1 & 0x10) == 0x10)
     {
      /* Set scaling back to 2:1 */

        data = SET_SCALING_TWO_TO_ONE;
        if ((rc = sendack(fdes, data)) != SUCCESS)
              return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_SCALING_SET_ERR));
     }

     /* Restore original resolution */

  data = SET_RESOLUTIONS;
  if ((rc = sendack(fdes, data)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_RESOLUTION_RATE_ERR));

      /* Send saved resolution data */

  cdata = byte2;
  if ((rc = wr_byte(fdes, &cdata, MOUSE_DATA_TX_REG)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_DATA_TX_REG_ERR));

  cdata = MOUSE_DEV_CMD;
  if ((rc = wr_byte(fdes, &cdata, MOUSE_ADP_CMD_REG)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_CMD_REG_ERR));

     /* Read ack */

  if ((rc = rdata(fdes, &mouse.lreg)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_ADP_ERR));

     /* Restore original sampling rate */

  data = SET_SAMPLING_RATE;
  if ((rc = sendack(fdes, data)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_SAMP_RATE_ERR));

      /* Send saved sampling rate data */

  cdata = byte3;
  if ((rc = wr_byte(fdes, &cdata, MOUSE_DATA_TX_REG)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_DATA_TX_REG_ERR));

  cdata = MOUSE_DEV_CMD;
  if ((rc = wr_byte(fdes, &cdata, MOUSE_ADP_CMD_REG)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_CMD_REG_ERR));

     /* Read ack */

  if ((rc = rdata(fdes, &mouse.lreg)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_ADP_ERR));


    /* Confirm default mouse settings */
    /* Set mouse adapter back to block mode and enable mouse */

    if ((rc = re_block(fdes)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, ADP_BLK_MOD_ERR));

  return(rc);
}
