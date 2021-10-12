static char sccsid[] = "@(#)55	1.1  exectu.c, bos320 4/22/91 16:38:17";
/*
 * COMPONENT_NAME:  SOL Test Unit
 *
 * FUNCTIONS: exectu
 *
 * ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************

Function(s) Exec TU for Diagnostics

*****************************************************************************/
#include <stdio.h>
#include "soltst.h"

int exectu (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        register i, loop, tu;
        static int rc;
        int k;

        extern int tu_001();
        extern int tu_002();
        extern int tu_003();
        extern int tu_004();


        tu = tucb_ptr -> header.tu;
        loop = tucb_ptr -> header.loop;

        for (i = 0; i < loop; i++){
          switch (tu) {
            case 1:
              rc = tu_001(fdes, tucb_ptr);
              break;
            case 2:
              rc = tu_002(fdes, tucb_ptr);
              break;
            case 3:
              rc = tu_003(fdes, tucb_ptr);
              break;
            case 4:
              rc = tu_004(fdes, tucb_ptr);
              break;
            default:
              return(SOL_ILLEGAL_TU_ERR);
          } /* endswitch */

          if (rc) {    /* break for loop if bad return code */
            tucb_ptr -> header.r1 = i;  /* return loop number failed on */
            break;
          } /* endif */
        } /* endfor */

   return(rc);

}
