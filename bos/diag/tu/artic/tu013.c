static char sccsid[] = "@(#)41  1.4  src/bos/diag/tu/artic/tu013.c, tu_artic, bos411, 9428A410j 8/19/93 17:50:02";
/*
 * COMPONENT_NAME:  
 *
 * FUNCTIONS: tu013
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
 * NAME: tu013
 *
 * FUNCTION: Test Unit tu013 - V.35 Wrap Test Unit for C2X adapter
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This function will test the V.35 interface with a wrap plug at the
 *          base of the card.  The following control lines will be wrapped:
 *
 *             Signal   Pin     to      Pin    Signal 	
 *             ---------------------------------------
 *		TXD(A)	 P	to	 R	RXD(A)
 *		TXD(B)	 S	to	 T	RXD(B)
 *		RTS	 C	to	 D	CTS
 *		DTR	 H	to	 E	DSR
 *
 *          There is a prerequisite the TU019 be executed before executing
 *          this test unit.
 *
 * RETURNS: The return code from the V.35 Wrap tests.
 *
 */
int tu013 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int rc;
	extern int start_diag_tu();

	rc = start_diag_tu(fdes, tucb_ptr, WRAPV35_COM_CODE, WV35_ER);
	if (rc)
		return(rc);
	rc = start_diag_tu(fdes, tucb_ptr, WRAPCIOV35_COM_CODE, WV35_ER);
	return(rc);
   }
