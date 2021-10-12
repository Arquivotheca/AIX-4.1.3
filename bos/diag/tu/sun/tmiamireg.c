static char sccsid[] = "@(#)42	1.1  src/bos/diag/tu/sun/tmiamireg.c, tu_sunrise, bos411, 9437A411a 3/28/94 17:46:51";
/*
 *   COMPONENT_NAME: tu_sunrise
 *
 *   FUNCTIONS: TestMiamiRegs
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************

Function(s) - Miami Register Test

Module Name :  tmiamireg.c

Test writes/reads/compares values to check all bit positions
in writable Miami registers.

*****************************************************************************/
#include <stdio.h>
#include <errno.h>
#include "suntu.h"
#include "error.h"
#include "sun_tu_type.h"

int TestMiamiRegs ()
{
   unsigned int wWriteVal, wReadVal;
   int rc, maxpatterns;
   unsigned int i;
   static unsigned int tval[] = {  0x55555555, 12345678,
                                   0xffffffff, 0xaaaaaaaa, 0x0 };

   maxpatterns = sizeof(tval)/sizeof(tval[0]); /* number of test pattern */

   for (i = 0x00; i < maxpatterns; i++)
   {
     /********** Test the BCR1 Miami register **********/
     wWriteVal = tval[i] & 0xfffff;
     if (rc = pio_write (MIbaseaddr+_BCR1, wWriteVal, 1))
     {
       LOG_REGISTER_ERROR(ERRORMIAMI_WRITE, rc, wWriteVal, 0, _BCR1);
     }
     if (rc = pio_read (MIbaseaddr+_BCR1, &wReadVal, 1))
     {
       LOG_REGISTER_ERROR(ERRORMIAMI_READ, rc, 0, wReadVal, _BCR1);
     }
     /*  Is what you read what you wrote? */
     wReadVal = wReadVal & 0x1fffff;
     if (wReadVal != wWriteVal)
     {
       DEBUG_2("_BCR1 register ERROR Expected %04x Actual %04x \n",wWriteVal,wReadVal);
       LOG_REGISTER_ERROR(ERRORMIAMI_REGCMP, 0, wWriteVal, wReadVal, _BCR1);
     }

     /********** Test the CAR1 Miami register **********/
     wWriteVal = tval[i];
     if (rc = pio_write (MIbaseaddr+_CAR1, wWriteVal, 1))
     {
       LOG_REGISTER_ERROR(ERRORMIAMI_WRITE, rc, wWriteVal, 0, _CAR1);
     }
     if (rc = pio_read (MIbaseaddr+_CAR1, &wReadVal, 1))
     {
       LOG_REGISTER_ERROR(ERRORMIAMI_READ, rc, 0, wReadVal, _CAR1);
     }
     /*  Is what you read what you wrote? */
     if (wReadVal != wWriteVal)
     {
       LOG_REGISTER_ERROR(ERRORMIAMI_REGCMP, 0, wWriteVal, wReadVal, _CAR1);
     }

     /********** Test the SAR1 Miami register **********/
     wWriteVal = tval[i];
     if (rc = pio_write (MIbaseaddr+_SAR1, wWriteVal, 1))
     {
       LOG_REGISTER_ERROR(ERRORMIAMI_WRITE, rc, wWriteVal, 0, _SAR1);
     }
     if (rc = pio_read (MIbaseaddr+_SAR1, &wReadVal, 1))
     {
       LOG_REGISTER_ERROR(ERRORMIAMI_READ, rc, 0, wReadVal, _SAR1);
     }
     /*  Is what you read what you wrote? */
     if (wReadVal != wWriteVal)
     {
       LOG_REGISTER_ERROR(ERRORMIAMI_REGCMP, 0, wWriteVal, wReadVal, _SAR1);
     }

     /********** Test the CCR1 Miami register **********/
     wWriteVal = tval[i];
     wWriteVal = tval[i] & 0x7fe;   /* mask out the DMA Start bit */
     if (rc = pio_write (MIbaseaddr+_CCR1, wWriteVal, 1))
     {
       LOG_REGISTER_ERROR(ERRORMIAMI_WRITE, rc, wWriteVal, 0, _CCR1);
     }
     if (rc = pio_read (MIbaseaddr+_CCR1, &wReadVal, 1))
     {
       LOG_REGISTER_ERROR(ERRORMIAMI_READ, rc, 0, wReadVal, _CCR1);
     }
     /*  Is what you read what you wrote? */
     wReadVal = wReadVal & 0x7fe;
     if (wReadVal != wWriteVal)
     {
       LOG_REGISTER_ERROR(ERRORMIAMI_REGCMP, 0, wWriteVal, wReadVal, _CCR1);
     }

   } /* End of for */

   return(OK);
}
