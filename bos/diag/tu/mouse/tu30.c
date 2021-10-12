/* static char sccsid[] = "@(#)60  1.1.1.2  src/bos/diag/tu/mouse/tu30.c, tu_mouse, bos411, 9431A411a 7/13/94 08:47:32"; */
/*
 * COMPONENT_NAME: tu_mouse
 *
 * FUNCTIONS: tu30
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

int tu30(fdes,tucb_ptr)
int fdes;
TUTYPE   *tucb_ptr;
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
  detrace(0, "\n\ntu03:   Begins...\n");
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
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));

  /* Read from mouse receive/stat reg - Make sure data buffer is empty */


  for (count = 0; count < READ_COUNT; count++)
        rd_word(fdes, &mouse.lreg, MOUSE_RX_STAT_REG);


  /* If mouse device is disabled */
  /* Send mouse status request command */

  data = STATUS_RQST_COMMAND;
  if ((rc = sendack(fdes, data)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));

   /* Save original mouse settings */
   /* Read mouse 32 bit register to make sure mouse is disabled */
   /* Read 1st byte */

  rc = rdata(fdes, &mouse.lreg);
  byte1 = mouse.reg[1];

    /* Read 2nd byte */


  rc = rdata(fdes, &mouse.lreg);
  byte2 = mouse.reg[1];

    /* Read 3rd byte */

  rc = rdata(fdes, &mouse.lreg);
  byte3 = mouse.reg[1];

   /* Test setting sampling rate, resolution, and scaling for mouse */
   /* Send 'set sampling rate' command to mouse */

  data = SET_SAMPLING_RATE;
  if ((rc = sendack(fdes, data)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_SAMP_RATE_ERR));

      /* Send sampling data amount to mouse - 10 reports/sec */

  data = 0x0a00;
  if ((rc = sendack(fdes, data)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_ADP_ERR));

     /* If sampling rate set returned an ack, set resolution */
     /* Send 'set resolution' command to mouse */

  data = SET_RESOLUTIONS;
  if ((rc = sendack(fdes, data)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_RESOLUTION_RATE_ERR));

       /* Send resolution setting to mouse - 1 count/mm */

  data = 0x0000;
  if ((rc = sendack(fdes, data)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_ADP_ERR));

     /* If sending resolution data to mouse returns an ack, set mouse
        scaling to 2:1  */

  data = SET_SCALING_TWO_TO_ONE;
  if ((rc = sendack(fdes, data)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_SCALING_SET_ERR));

     /* Obtain mouse status report to check settings */
     /* Send mouse status request command */

  data = STATUS_RQST_COMMAND;
  if ((rc = sendack(fdes, data)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_RQST_STAT_ERR));

      /* If successfully sends mouse status request */
      /* Read mouse 32 bit register */
      /* Read 1st byte */


  rdata(fdes, &mouse.lreg);

       /* Check for 2:1 scaling */

  if ((mouse.reg[1] & 0x10) != 0x10)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_SCALING_SET_ERR));

        /* Read 2nd byte */

  rdata(fdes, &mouse.lreg);

         /* Check for 1 count/mm resolution */

  if (mouse.reg[1] != 0x00)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_RESOLUTION_RATE_ERR));

       /* Read 3rd byte */

   rdata(fdes, &mouse.lreg);

         /* Check for 10 reports/sec sampling rate */

  if (mouse.reg[1] != 0x0a)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_SAMP_RATE_ERR));

     /* If successful at sending status request */
     /* Obtain mouse status to check settings */
     /* Restore original mouse settings */
     /* Reset scaling */

  data = RESET_SCALING;
  if ((rc = sendack(fdes, data)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_SCALING_RSET_ERR));

     /* Restore original resolution */

  data = SET_RESOLUTIONS;
  if ((rc = sendack(fdes, data)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_SET_RESOLN_ERR));

      /* Send saved resolution data */

  cdata = byte2;

  if ((rc = wr_byte(fdes, &cdata, MOUSE_DATA_TX_REG)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_BYTE2_ERR));

  cdata = MOUSE_DEV_CMD;
  if ((rc = wr_byte(fdes, &cdata, MOUSE_ADP_CMD_REG)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_CMD_REG_ERR));

     /* Read ack */

  if((rc = rdata(fdes, &mouse.lreg)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_RESOLUTION_RATE_ERR));

     /* Restore original sampling rate */

  data = SET_SAMPLING_RATE;

  if ((rc = sendack(fdes, data)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_SET_SAMP_ERR));

      /* Send saved sampling rate data */

  cdata = byte3;

  if ((rc = wr_byte(fdes, &cdata, MOUSE_DATA_TX_REG)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_BYTE3_ERR));

  cdata = MOUSE_DEV_CMD;

  if ((rc = wr_byte(fdes, &cdata, MOUSE_ADP_CMD_REG)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_CMD_REG_ERR));

     /* Read ack */

  if((rc = rdata(fdes, &mouse.lreg)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_SET_SAMP_ERR));

     /* Confirm restored mouse settings by sending a status request */

     /* Send mouse status request command */

  data = STATUS_RQST_COMMAND;
  if ((restrc = sendack(fdes, data)) != SUCCESS)
     {
        if (rc == SUCCESS)
              return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_RQST_STAT_ERR));
     }

    /* Read mouse 32 bit register to make sure */
    /* original mouse settings on.  */

    /* Read 1st byte */

  if((rc = rdata(fdes, &mouse.lreg)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_BYTE1_ERR));

     /* If original mouse scaling not restored */

  if ((mouse.reg[1] & 0x10) !=  0x00)
     {
        if (rc == SUCCESS)
              return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_SCALING_RSET_ERR));
     }

    /* Read 2nd byte */

  if((rc = rdata(fdes, &mouse.lreg)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_BYTE2_ERR));

     /* If original mouse resolution not restored */

  if (mouse.reg[1] != byte2)
     {
        if (rc == SUCCESS)
              return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_RESOLUTION_RATE_ERR));
     }

    /* Read 3rd byte */

  if((rc = rdata(fdes, &mouse.lreg)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_BYTE3_ERR));

     /* If original mouse sampling rate not restored */

  if (mouse.reg[1] != byte3)
     {
        if (rc == SUCCESS)
              return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_SAMP_RATE_ERR));
     }

   /* Test setting sampling rate, resolution, and scaling for mouse */
   /* Set mouse adapter back to block mode and enable mouse */

    if ((rc = re_block(fdes)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, ADP_BLK_MOD_ERR));

return(rc);
}
