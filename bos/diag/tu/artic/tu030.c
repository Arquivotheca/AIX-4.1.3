static char sccsid[] = "@(#)57  1.2.1.2  src/bos/diag/tu/artic/tu030.c, tu_artic, bos411, 9428A410j 8/19/93 17:51:14";
/*
 * COMPONENT_NAME:  
 *
 * FUNCTIONS: tu030
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include "artictst.h"

/*
 * NAME: tu030
 *
 * FUNCTION:  Multiport/2 Counter A Test
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * RETURNS: The return code from the Counter A test.
 *
 */
int tu030 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        int rc;
        unsigned char pos4, pos5, newpos4, newpos5;
        extern int start_diag_tu();


        /* Read original contents of POS 4 and POS 5 */
        if (icareadpos(fdes, 4, &pos4))
           return(DRV_ERR);

        if (icareadpos(fdes, 5, &pos5))
           return(DRV_ERR);

         /* Set Clocking bits and clear DCE/DTE bit in both POS registers */
        newpos4 = (pos4 | 0x30) & 0xBF;
        newpos5 = (pos5 | 0x18) & 0xDF;

        if (icawritepos(fdes, 4, newpos4))
           return(DRV_ERR);

        if (icawritepos(fdes, 5, newpos5))
           return(DRV_ERR);

        rc = start_diag_tu(fdes, tucb_ptr, COUNTERA_COM_CODE,COUNTA_ER);

        if (icawritepos(fdes, 4, pos4))
           return(DRV_ERR);

        if (icawritepos(fdes, 5, pos5))
           return(DRV_ERR);

        return(rc);
   }
