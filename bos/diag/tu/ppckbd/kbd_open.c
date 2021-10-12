static char sccsid[] = "@(#)20  1.1  src/bos/diag/tu/ppckbd/kbd_open.c, tu_ppckbd, bos41J, 9520A_all 5/6/95 14:32:02";
/*
 * COMPONENT_NAME: tu_ppckbd
 *
 * FUNCTIONS:   kbd_open
 *
 * DESCRIPTION: Open Device Driver in diagnostics mode
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include "tu_type.h"
#include <varargs.h>


open_kbd(TUTYPE *tucb_ptr)
{
    uint arg;

       /* in case that the kbd test code is executed using a serial terminal
          and the systems does not have a console keyboard "unusual" */

       if(tucb_ptr->tuenv.kbd_fd == NULL)
          return(SUCCESS);
 
       if ((kbdtufd = open(tucb_ptr->tuenv.kbd_fd, O_RDWR)) < 0)
           return(FAILURE);

       arg = KSDENABLE;
       if (ioctl(kbdtufd, KSDIAGMODE, &arg) == SUCCESS)
           return(SUCCESS);
     
       return(FAILURE);
}

