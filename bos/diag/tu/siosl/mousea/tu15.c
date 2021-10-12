static char sccsid[] = "@(#)72  1.2  src/bos/diag/tu/siosl/mousea/tu15.c, cmddiag, bos41J, 9515A_all 4/7/95 11:05:10";
/*
 *   COMPONENT_NAME: TU_SIOSL
 *
 *   FUNCTIONS: tu15m
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

/* Function tu15m - TU 15 - Tests native POS register contents for RSC/RBW
   products ONLY.  See related documentation for return code information.
*/ 

int tu15m(long fd, struct TUCB *tucb_ptr)
{
   unsigned char data,rd_value;
   unsigned int ldata; 
   int count = 0,status, rc = SUCCESS;
   int test_byte;
   char msg[80];

     /* Test for SIO ID (POS 0 and 1) */

     if (mach_info.mach_model == FIREB_MODEL)
     {
         rc = rd_posm(fd, &data, SIO_ID0_G30REG);
	 test_byte = 0xf8;
     } 
     else
     {
         rc = rd_posm(fd, &data, SIO_ID0_REG);
	 test_byte = 0xfe;
     }

     if (data != test_byte)
     {
        sprintf(msg,"Error detected in SIO ID read, POS 0=%2X\n",data);
        PRINTERR(msg);
        rc = MOUSE_LOGIC_ERROR;
     }
     else
     {
       /* Test for SIO ID (POS 0 and 1) */

       if (mach_info.mach_model == FIREB_MODEL)
          rc = rd_posm(fd, &data, SIO_ID1_G30REG);
       else
          rc = rd_posm(fd, &data, SIO_ID1_REG);

       if (data != 0xf6)
       {
         sprintf(msg,"Error detected in SIO ID read, POS 1=%2X\n",data);
         PRINTERR(msg);
         rc = MOUSE_LOGIC_ERROR;
       }
     }

return(rc);
}


