static char sccsid[] = "@(#)35  1.2.1.2  src/bos/diag/tu/artic/tu006.c, tu_artic, bos411, 9428A410j 8/19/93 17:49:35";
/*
 * COMPONENT_NAME:  
 *
 * FUNCTIONS: tu006
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
 * NAME: tu006
 *
 * FUNCTION: Gate Array Test Unit
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This function will initiate the Gate Array (SSTIC) test unit.  This
 *          is a more thorough test than the POST test.  This test will
 *          generate even and odd parity errors, test the Watch Dog Timer and
 *          check to see if the Watch Dog Timer can be disabled.  There is a 
 *          prerequisite that TU019 be executed before executing this test
 *          unit.
 *
 * RETURNS: The return code from the Gate Array test.
 *
 */
int tu006 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        extern unsigned char cardnum;
	extern int start_diag_tu();

	return(start_diag_tu(fdes, tucb_ptr, GATEARRAY_COM_CODE, GATE_ER));
   }
