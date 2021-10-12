static char sccsid[] = "@(#)14  1.2  src/bos/diag/tu/siosl/kbda/slih/kbd_slih.c, tu_siosl, bos411, 9428A410j 12/17/93 12:19:59";
/*
 *   COMPONENT_NAME: TU_SIOSL
 *
 *   FUNCTIONS: kbd_interrupt_handler
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/intr.h>
#include <stdio.h>
#include <sys/diagex.h>   /* include system file when available */
#include "salioaddr.h"

int kbd_interrupt_handler(diag_struc_t *handle, uchar *data_ptr)
{
   uchar *rx_buffer_status = data_ptr;
   uchar cmd_status;
   uint offset;

   (void)diag_io_read(handle,IOCHAR,(caddr_t)KBD_STAT_CMD_REG, (ulong *)&cmd_status,NULL,INTRKMEM);

    if ((cmd_status) & KBD_RX_BUF_FULL) {
      if (rx_buffer_status[0] >= 4)
      {
	 offset = rx_buffer_status[0] = 1;
      }
      else {
	 offset = rx_buffer_status[0] = rx_buffer_status[0] + 1;
      }

      (void)diag_io_read(handle,IOCHAR,(caddr_t)KBD_DATA_REG,
			 (ulong *)&rx_buffer_status[offset],NULL,INTRKMEM);

      return(INTR_SUCC);

    } else {
      return(INTR_FAIL);
    } /* endif */

}
