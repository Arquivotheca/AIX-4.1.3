/* static char sccsid[] = "@(#)59  1.1.1.2  src/bos/diag/tu/mouse/tu20.c, tu_mouse, bos411, 9431A411a 7/13/94 08:47:29"; */
/*
 * COMPONENT_NAME: tu_mouse
 *
 * FUNCTIONS: tu20
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

int tu20(fdes,tucb_ptr)
  int fdes;
  TUTYPE  *tucb_ptr;
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
  static unsigned char datapattern[3] = { 0x55, 0xaa, 'H' };

#ifdef debugg
  detrace(0, "\n\ntu02:   Begins...\n");
#endif

/* Initialize variables */

  mouse.lreg = 0;
  count = 0;

  /* initialize mouse device */

  if ((rc = init_dev(fdes)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, rc));


  /* Disable mouse adapter block mode */

  if ((rc = non_block(fdes)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));

  /* Read from mouse receive/stat reg - Make sure data buffer is empty */

  for (count = 0; count < READ_COUNT ; count++)
        rd_word(fdes, &mouse.lreg, MOUSE_RX_STAT_REG);

  /* If mouse device is disabled */
  /* Put mouse device in wrap mode */

  data = SET_WRAP_MODE;
  if ((rc = sendack(fdes, data)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));

   /* If mouse successfully ACK Wrap Mode Command */
   /* Send data patterns to mouse device */

  for (i = 0; i < 3; i++)
     {
        cdata = datapattern[i];
        if ((rc = wr_byte(fdes, &cdata, MOUSE_DATA_TX_REG)) != SUCCESS)
              return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_DATA_TX_REG_ERR));
        cdata = MOUSE_DEV_CMD;
        if ((rc = wr_byte(fdes, &cdata, MOUSE_ADP_CMD_REG)) != SUCCESS)
              return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_CMD_REG_ERR));

   /* Read back wrapped data */

        rdata(fdes, &mouse.lreg);
        if (mouse.reg[1] != datapattern[i])
              return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_WRAP_ERR));
     }
      /* Reset wrap mode */

  cdata = RESET_WRAP_MODE;
  if ((rc = wr_byte(fdes, &cdata, MOUSE_DATA_TX_REG)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_DATA_TX_REG_ERR));

  cdata = MOUSE_DEV_CMD;
  if ((rc = wr_byte(fdes, &cdata, MOUSE_ADP_CMD_REG)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_CMD_REG_ERR));

      /* Read back ack */

  rdata(fdes,&mouse.lreg);

  if (mouse.reg[1] != MOUSE_ACK_COMMAND)
     {
#ifdef debugg
detrace(1,"Error in mouse reset wrap mode test, mouse.lreg = %8x\n",mouse.lreg);
#endif
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_RSET_WRP_ERR));
     }

    /* Set/Reset Wrap mode test */
    /* Mouse error handling test - Send one byte of data (not a command)
       to mouse */
    /* Arbitrary non-command data */

  data = 0x5500;
  if ((rc = wr_2byte(fdes, &data, MOUSE_DATA_TX_REG)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_DATA_TX_REG_ERR));

  rdata(fdes,&mouse.lreg);

     /* If mouse resend command returned */

  if (mouse.reg[1] == 0xFE)
     {

/* Send more arbitrary non-command mouse data */

        data = 0xaa00;
        if ((rc = wr_2byte(fdes, &data, MOUSE_DATA_TX_REG)) != SUCCESS)
              return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_DATA_TX_REG_ERR));

        if ((rc = rdata(fdes,&mouse.lreg)) != SUCCESS)
              return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));

       /* If mouse error code not returned */

        if (mouse.reg[1] != 0xFC)
           {
#ifdef debugg
detrace(1,"Error in mouse error handling test, byte = %2x\n",mouse.reg[1]);
#endif

              return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_HANDLING_ERR));
           }
     }

    /* Mouse error handling test */
    /* Mouse resend test */
    /* Send resend command to mouse */

  data = RESEND_COMMAND;
  if ((rc = wr_2byte(fdes, &data, MOUSE_DATA_TX_REG)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ADP_DATA_TX_REG_ERR));

  rdata(fdes,&mouse.lreg);

     /* If previous output not returned */

  if (mouse.reg[1] != 0xFC)
     {
#ifdef debugg
      detrace(1,"Error in mouse resend test, byte = %2x\n",mouse.reg[1]);
#endif
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, DEV_RESEND_ERR));
     }

   /* Mouse resend test */
   /* Set mouse adapter back to block mode */

    if ((rc = re_block(fdes)) != SUCCESS)
        return(mktu_rc(tucb_ptr->header.tu, MOU_ERR, ADP_BLK_MOD_ERR));

return(rc);
}
