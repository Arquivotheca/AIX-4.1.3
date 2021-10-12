static char sccsid[] = "@(#)73  1.2  src/bos/diag/tu/siosl/mousea/tu20.c, cmddiag, bos411, 9428A410j 4/7/94 12:20:04";
/*
 *   COMPONENT_NAME: TU_SIOSL
 *
 *   FUNCTIONS: tu20m
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <sys/sem.h>
#include <sys/devinfo.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mode.h>
#include <sys/mdio.h>
#include <diag/atu.h>
#include "tu_type.h"


/* Function tu20m - TU 20 - Tests fuse/polyswitch status for either RSC/RBW 
   or RS1/RS2 products.  See related documentation for return code information.
*/

int tu20m(long fd, struct TUCB *tucb_ptr)
{
   unsigned char data, bad_switch_status;
   unsigned int ldata;
   int status, rc = SUCCESS;
   char msg[80];
 
   /* mach_info.mach_type is initialized in get_machine_model_tum function,
      which is called from the exectu function */

   if ( (mach_info.mach_type == 0x01) || (mach_info.mach_type == 0x00) || 
	(mach_info.mach_type == 0x04) )
   {
     /* Grab the semaphore before accessing 8255, reg. 0x55, since the adapter
	may be in reset state. */
     rc = set_semm(1);

     if (rc != SUCCESS) {
       PRINTERR("Error grabbing semaphore in TU 20\n");
       return(-1);
     }

      /* Check the mouse adapter fuse for RS1/RS2 products */

      rd_bytem(fd, &data, IOR_PB_8255);

      if (data & MOU_FUSE_BAD) {
         sprintf(msg,"Fuse status is bad, contents of data = :%2X\n",data);
         PRINTERR(msg);
         rc = FUSE_BAD_ERROR;
      }

     /* Now release the semaphore after accessing 8255 adapter reg. 0x55 */
      rel_semm();

   }

   else    /* This section only applies to RSC/RBW products */
   {

      /* Check the ktsm adapters' common fuse/polyswitch status for RSC/RBW 
         products */

      /* Read tablet modem status register */
      rc = rd_bytem(fd, &data, TABLET_MODEM_ST_REG);

      if (rc == SUCCESS)
      {
        /* The "bad_switch_status" variable is the kbd/mse/tab bad polyswitch
            status mask */

        /* The following fuse/switch status mask is determined in 
	   get_machine_model_tum function which is called in exectu function */
          bad_switch_status = mach_info.switch_stat_mask;

        /* Check for bad fuse/switch status */
          if (data & bad_switch_status)
          {
           sprintf(msg,"Fuse or polyswitch status is bad, tablet modem status reg = :%2X\n",data);
           PRINTERR(msg);
           rc = FUSE_BAD_ERROR;
          }

      }
   }

   return (rc);
}

