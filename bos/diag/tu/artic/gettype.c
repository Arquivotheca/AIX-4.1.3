static char sccsid[] = "@(#)02        1.2  src/bos/diag/tu/artic/gettype.c, tu_artic, bos41J, 9508A 2/15/95 17:50:16";
/* COMPONENT_NAME:
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
 * FUNCTIONS: gettype
 *
 */

#include <stdio.h>
#include <artictst.h>

/*
 * NAME: gettype
 *
 * FUNCTION: function to allow user to determine eib type
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) User passes file descriptor.
 *
 * RETURNS: Integer value indicating eib type as follows:
 *    0 = X.25            100 = 4P/MP         300 = C1X
 *    1 = MP/2 4P 232     101 = PM 6P V.35    400 = GALE 4P/MP
 *    2 = MP/2 6P SYNC    102 = PM 6P X.21    401 = GALE 6P V.35
 *    3 = MP/2 8P 232     103 = PM 8P 232     402 = GALE 6P X.21
 *    4 = MP/2 8P 422     104 = PM 8P 422     403 = GALE 8P 232
 *    5 = MP/2 COMBO      200 = SP-5          404 = GALE 8P 422
 *
 */
int gettype (fdes)
   int fdes;
   {
        int type;
        if (icagetadaptype(fdes, &type))
           return(-1);
        else
           return(type);
   }
