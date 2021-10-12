static char sccsid[] = "@(#)28	1.1  src/bos/diag/tu/sol/tu001.c, tu_sol, bos411, 9428A410j 4/23/91 10:48:04";
/****************************************************************************

   COMPONENT_NAME: SOL Test Unit

   FUNCTIONS: tu_001  (Buffer Access Mode Test Unit)

   ORIGINS: 27


   (C) COPYRIGHT International Business Machines Corp. 1991
   All Rights Reserved
   Licensed Materials - Property of IBM
   US Government Users Restricted Rights - Use, duplication or
   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.


*****************************************************************************/


#include <stdio.h>
#include "soltst.h"

int
tu_001(fdes, tucb_ptr)
int fdes;
TUTYPE *tucb_ptr;
{

        int                             result, counter;
        uchar                           rc;
        char                            choice;
        struct sol_buffer_access        buffer_access_struct;
#define BUFFER_LENGTH   512
        char                            write_array[BUFFER_LENGTH];
        char                            read_array[BUFFER_LENGTH];

        bzero(&buffer_access_struct,sizeof(buffer_access_struct));

        /* fill in write buffer and fill read with all 0xff */
        for (counter = 0; counter < BUFFER_LENGTH; counter++) {
                    write_array[counter] = counter;
                    read_array[counter] = 0xff;
        } /* endfor */

        /* set-up struct for write operation  */

        buffer_access_struct.flag = SOL_WRITE;
        buffer_access_struct.bufptr = write_array;
        buffer_access_struct.buflen = BUFFER_LENGTH;

        result = ioctl(fdes,SOL_BUFFER_ACCESS,&buffer_access_struct);
        if (result != 0) {                      /* if error */
                if (errno == EIO) {
                      tucb_ptr -> diag.sla_error = buffer_access_struct.result;
                      return(SOL_BUF_ACCESS_WRITE_FAILED);
                } else {
                      tucb_ptr -> diag.sla_error = errno;
                      return(SOL_PROGRAMMING_ERROR);
                } /* endif */

        }  /* endif */
        /* set-up struct for read operation  */

        bzero(&buffer_access_struct,sizeof(buffer_access_struct));

        buffer_access_struct.flag = SOL_READ;
        buffer_access_struct.bufptr = read_array;
        buffer_access_struct.buflen = BUFFER_LENGTH;

        result = ioctl(fdes,SOL_BUFFER_ACCESS,&buffer_access_struct);
        if (result != 0) {                      /* if error */
                if (errno == EIO) {
                      tucb_ptr -> diag.sla_error = buffer_access_struct.result;
                      return(SOL_BUF_ACCESS_READ_FAILED);

                } else {
                      tucb_ptr -> diag.sla_error = errno;
                      return(SOL_PROGRAMMING_ERROR);
                } /* endif */

        }  /* endif */
        /* compare read/write buffers  */

        if (strncmp(write_array, read_array, BUFFER_LENGTH)) {
          return(SOL_BUF_ACCESS_CMP_FAILED);
        } /* endif */

        return(SOL_SUCCESS);
}

