static char sccsid[] = "src/bos/diag/tu/dca/sdelay.c, tu_tca, bos411, 9428A410j 8/7/92 10:57:28";
/*
 * COMPONENT_NAME: (TCATU) TCA/DCA Test Unit
 *
 * FUNCTIONS: short_delay
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>

#ifdef debugg
extern void detrace();
#endif

/*
 * NAME: short_delay
 *
 * FUNCTION: Provide a short time delay
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This procedure allow for a short (1 millisecond) time delay.
 *
 * RETURNS: NONE
 *
 */
void short_delay (unsigned char msec)
   {

        usleep(msec * 1000); 
   }
