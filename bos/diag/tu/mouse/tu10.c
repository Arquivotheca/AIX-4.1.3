/* static char sccsid[] = "@(#)58  1.1.1.2  src/bos/diag/tu/mouse/tu10.c, tu_mouse, bos411, 9431A411a 7/13/94 08:47:26"; */
/*
 * COMPONENT_NAME: tu_mouse
 *
 * FUNCTIONS: tu10
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

int tu10(fdes,tucb_ptr)
  int fdes;
  TUTYPE *tucb_ptr;
{
  extern int errno;
  int count;
  unsigned char byte1, byte2, byte3, cdata;
  ushort data;
  static int rc = SUCCESS, restrc = SUCCESS;
  union regs {
                unsigned int lreg;
                unsigned char reg[4];
             } mouse;

#ifdef debugg
  detrace(0, "\n\ntu01:   Begins...\n");
#endif

/* Initialize variables */

  mouse.lreg = 0;
  count = 0;
  byte1 = byte2 = byte3 = 0;

  /* initialize mouse device */

  if ((rc = init_dev(fdes)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, rc));

  /* Disable mouse adapter block mode */

  if ((rc = non_block(fdes)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu,LOG_ERR, rc));

  /* Read from mouse receive/stat reg - Make sure data buffer is empty */


  for (count = 0; count < READ_COUNT; count++)
        rd_word(fdes, &mouse.lreg, MOUSE_RX_STAT_REG);

 /* If mouse device disabled */
 /* Send mouse status request command */

  data = STATUS_RQST_COMMAND;
  if ((rc = sendack(fdes, data)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));

    /* Read mouse 32 bit register to make sure mouse is disabled */
    /* Read 1st byte */
 
  rc = rdata(fdes, &mouse.lreg);
  byte1 = mouse.reg[1];
  if ((mouse.lreg & 0x00200000) != 0x00000000)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_DISABL_ERR));

    /* Read 2nd byte */

  rc = rdata(fdes, &mouse.lreg);
  byte2 = mouse.reg[1];

    /* Read 3rd byte */

  rc = rdata(fdes, &mouse.lreg);
  byte3 = mouse.reg[1];

  /* If mouse device truly disabled */
  /* Send 'reset - ff' command to mouse device */

  data = MOUSE_RESET_COMMAND;
  if ((rc = sendack(fdes, data)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_RSET_ERR));

  /* Allow 500ms delay to check for BAT completion code */

  usleep (500 * 1000);

  /* If successful at resetting mouse device */
  /* Read mouse 32 bit register for BAT completion code */

  rdata(fdes, &mouse.lreg);

      /* Check for mouse BAT completion code */

  if (mouse.reg[1] != MOUSE_COMPLETION_CODE)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_RSET_ERR));

     /* Read mouse 32 bit register for I.D.code */

   rc = rdata(fdes, &mouse.lreg);
          /* for mouse I.D. code */

  if (mouse.reg[1] != MOUSE_ID_CODE)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_IDCODE_ERR));

    /* Confirm mouse reset status */
    /* Send mouse status request command */

  if ((rc = sendack(fdes, STATUS_RQST_COMMAND)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, rc));

     /* Read mouse 32 bit register for mouse reset status */
     /* Read 1st byte */

  rc = rdata(fdes, &mouse.lreg);

  if ((mouse.reg[1] & SET_MOU_REMOTE_MODE) != 0x00)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_SET_REMOTE_ERR));

     /* Read 2nd byte */

  rc = rdata(fdes, &mouse.lreg);

  if (mouse.reg[1] != 0x02)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));

     /* Read 3rd byte */

  rc = rdata(fdes, &mouse.lreg);

  if (mouse.reg[1] != 0x64)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_RD_STAT_ERR));

   /* Restore mouse device original settings */
   /* Set original sampling rate */

  data = SET_SAMPLING_RATE;
  if ((rc = sendack(fdes, data)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_SET_SAMP_ERR));

    /* Send second byte(the sampling rate data) of the command */

  cdata = byte3;
  if ((rc = wr_byte(fdes, &cdata, MOUSE_DATA_TX_REG)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_BYTE3_ERR));

    /* Send command to read from mouse tx reg */

  cdata = MOUSE_DEV_CMD;

  if ((rc = wr_byte(fdes, &cdata, MOUSE_ADP_CMD_REG)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_CMD_REG_ERR));

    /* Read acknowledge response */

  if ((rc = rdata(fdes,&mouse.lreg)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));

    /* Set mouse to remote mode */

  data = SET_MOU_REMOTE_MODE;
  if ((restrc = sendack(fdes, data)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_SET_REMOTE_ERR));

  if ((byte1 & 0x10) == 0x10)
     {
      /* Set scaling 2:1 */

        if ((restrc = sendack(fdes, SET_SCALING_TWO_TO_ONE)) != SUCCESS)
              return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_SET_SCALE_ERR));
     }

    /* Set original resolution */

  if ((rc = sendack(fdes, SET_RESOLUTIONS)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_SET_RESOLN_ERR));

   /* Send second byte(the resolution data) of 'set resolution' command */

  cdata = byte2;
  if ((rc = wr_byte(fdes, &cdata, MOUSE_DATA_TX_REG)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_BYTE2_ERR));

    /* Send command to read from mouse tx reg */

  cdata = MOUSE_DEV_CMD;
  if ((rc = wr_byte(fdes, &cdata, MOUSE_ADP_CMD_REG)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_CMD_REG_ERR));

    /* Read acknowledge response */

  if ((rc = rdata(fdes,&mouse.lreg)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));

   /* Enable mouse */

  if ((restrc = sendack(fdes, ENABLE_MOUSE)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_ENABL_ERR));

    /* Confirm mouse enable status */
    /* Send mouse status request command */

  if ((restrc = sendack(fdes, STATUS_RQST_COMMAND)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, restrc));

    /* Read mouse 32 bit register to make sure mouse is enabled */
    /* and mouse original settings on.  */
    /* Read 1st byte */

  rc = rdata(fdes, &mouse.lreg);

     /* If mouse is not enabled */

  if (!(mouse.reg[1] & 0x20))
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_ENABL_ERR));

     /* If original mouse scaling not restored */

  if ((mouse.reg[1] & 0x10) != (byte1 & 0x10))
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_SET_SCALE_ERR));

    /* Read 2nd byte */

  rc = rdata(fdes, &mouse.lreg);

     /* If original mouse resolution not restored */

  if (mouse.reg[1] != byte2)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_SET_RESOLN_ERR));

    /* Read 3rd byte */

  rc = rdata(fdes, &mouse.lreg);

     /* If original mouse sampling rate not restored */

  if (mouse.reg[1] != byte3)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_SET_SAMP_ERR));

    /* Restore mouse to original stream mode */

  if ((restrc = sendack(fdes, SET_STREAM_MODE)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_SET_STREAM_ERR));

   /* Set mouse adapter back to block mode */

    if ((rc = re_block(fdes)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, ADP_BLK_MOD_ERR));

  return(rc);
}
