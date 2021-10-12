static char sccsid[] = "@(#)49  1.3.1.3  src/bos/diag/tu/artic/tu022.c, tu_artic, bos411, 9428A410j 8/19/93 17:50:42";
/*
 * COMPONENT_NAME:  
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
 *
 *
 * FUNCTIONS: tu022
 *
 */
/*****************************************************************************

Function(s) Test Unit 022  PORT 0 Wrap Test Unit for MP/2 adapter

Module Name :  tu022.c HTX

*****************************************************************************/
#include <stdio.h>
#include "artictst.h"

/*****************************************************************************

tu022

*****************************************************************************/

int tu022 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        int rc;
        unsigned char pos4, pos5, newpos4, newpos5;
        extern int start_diag_tu();


        if ((tucb_ptr->artic_info.reserved == MP2_8P422) ||
            (tucb_ptr->artic_info.reserved == MP2_6PSYNC))
        {
            /* Read original contents of POS 4 and POS 5 */
            if (icareadpos(fdes, 4, &pos4))
               return(DRV_ERR);

            if (icareadpos(fdes, 5, &pos5))
               return(DRV_ERR);

        /* If EIB indicates Multiport/2 with RS422, set clocking POS registers */
            if (tucb_ptr->artic_info.reserved == MP2_8P422)
            {
                /* Set Clocking bits and clear DCE/DTE bit in both POS registers */
                newpos4 = (pos4 | 0x30) & 0xBF;
                newpos5 = (pos5 | 0x18) & 0xDF;
            }
            else   /* 6 port Synchronous, set clocking appropriately */
            {
                /* Set Clocking bits and clear DCE/DTE bit in both POS registers */
                newpos4 = pos4  & 0x8F;
                newpos5 = pos5 & 0xC7;
            }

            if (icawritepos(fdes, 4, newpos4))
               return(DRV_ERR);

            if (icawritepos(fdes, 5, newpos5))
               return(DRV_ERR);

            /* Execute the test */
            rc = start_diag_tu(fdes, tucb_ptr,WRAP_P0_COM_CODE, WP0_ER);

            if (icawritepos(fdes, 4, pos4))
               return(DRV_ERR);

            if (icawritepos(fdes, 5, pos5))
               return(DRV_ERR);

          }
          else
              /* Execute the test */
              rc = start_diag_tu(fdes, tucb_ptr,WRAP_P0_COM_CODE, WP0_ER);

        return(rc);
   }
