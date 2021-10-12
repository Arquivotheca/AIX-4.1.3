static char sccsid[] = "@(#)43  1.4  src/bos/diag/tu/artic/tu015.c, tu_artic, bos411, 9428A410j 8/19/93 17:50:12";
/*
 * COMPONENT_NAME:  
 *
 * FUNCTIONS: tu015
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
 * NAME: tu015
 *
 * FUNCTION: Test Unit Tu015 - V.24 Wrap Cable Test Unit for C2X adapter
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This function will test the V.24 interface with a wrap plug at
 *          the end of the cable.  The following control lines are wrapped.
 *
 *             Signal   to    Signal
 *             ---------------------
 * 		TXD	to	RXD
 *		RTS	to	CTS
 *		DTR	to 	DSR
 *		LLBT 	to	 TI
 *		RLBT	to	 CI
 *
 *          There is a prerequisite that TU019 be executed before executing
 *          this test unit.
 *
 * RETURNS: The return code from the V.24 Wrap Cable tests.
 *
 */
int tu015 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int rc;
	extern int start_diag_tu();

	rc = start_diag_tu(fdes, tucb_ptr, WRAPV24_COM_CODE, WCV24_ER);
	return(rc);
   }
