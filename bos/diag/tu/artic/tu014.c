static char sccsid[] = "@(#)42  1.4  src/bos/diag/tu/artic/tu014.c, tu_artic, bos411, 9428A410j 8/19/93 17:50:08";
/*
 * COMPONENT_NAME:  
 *
 * FUNCTIONS: tu014
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
 * NAME: tu014
 *
 * FUNCTION:  Test Unit tu014 - X.21 Wrap Cable Test Unit for C2X adapter
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This function will test the X.21 interface with a wrap plug at
 *          the end of a cable.  The following control lines are wrapped:
 *
 *             Signal   to     Signal
 *             ----------------------
 *		T(B)	to	R(B)
 *		T(A)	to 	R(A)
 *		C(B)	to	I(B)
 *		C(A)	to 	I(A)
 *
 *          There is a prerequisite the TU019 be executed before executing
 *          this test unit.
 *
 * RETURNS: The return code from the X.21 Wrap Cable tests.
 *
 */
int tu014 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int rc;
	extern int start_diag_tu();

	rc = start_diag_tu(fdes, tucb_ptr, WRAPX21_COM_CODE, WCX21_ER);
	return(rc);
   }
