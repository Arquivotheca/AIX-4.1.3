static char sccsid[] = "@(#)40  1.4  src/bos/diag/tu/artic/tu012.c, tu_artic, bos411, 9428A410j 8/19/93 17:49:57";
/*
 * COMPONENT_NAME:  
 *
 * FUNCTIONS: tu012
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
 * NAME: tu012
 *
 * FUNCTION: Test Unit 012 - V.24 Wrap Test Unit for C2X adapter
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This function will test the V.24 interface with a wrap plug at
 *          the base of the card.  The following control lines are wrapped:
 *
 *            Signal    Pin     to      Pin   Signal
 *            --------------------------------------	
 *		TXD	 2	to	 3	RXD
 *		RTS	 4	to	 5	CTS
 *		DTR	20	to	 6	DSR
 *		LLBT	18	to	25	TI
 *		RLBT	21	to	22	CI
 *
 *          There is a prerequisite that TU019 be executed before executing
 *          this test unit.
 *
 * RETURNS: The return code from the V.24 Wrap tests.
 *
 */
int tu012 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int rc;
	extern int start_diag_tu();

	rc = start_diag_tu(fdes, tucb_ptr, WRAPV24_COM_CODE, WV24_ER);
	return(rc);
   }
