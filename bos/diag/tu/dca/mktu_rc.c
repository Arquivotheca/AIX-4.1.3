static char sccsid[] = "src/bos/diag/tu/dca/mktu_rc.c, tu_tca, bos411, 9428A410j 6/19/91 15:30:55";
/*
 * COMPONENT_NAME: (TU_TCA) TCA/DCA Test Unit
 *
 * FUNCTIONS: mktu_rc
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "tcatst.h"	/* among others, contains defs. for error types */

/*
 * NAME: mktu_rc
 *
 * FUNCTION: Make Test Unit Return Code
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) Function takes test unit number (tu_no), error type (err_type),
            and error code (error_code) to form a special unique return code.
 *
 * RETURNS: The Return code that was deciphered
 *
 */
int mktu_rc (tu_no, err_type, error_code)
   int tu_no;
   int err_type;
   int error_code;
   {
	static int rc;

	rc = 0;
	if ((tu_no < 0x00) || (tu_no > 0x7F))
		return(-1);
	if ((err_type < 0x00) || (err_type > 0xFF))
		return(-2);
	if ((error_code < 0x0000) || (error_code > 0xFFFF))
		return(-3);
	
	rc = tu_no << 24;
	rc |= err_type << 16;
	rc |= error_code;

	return(rc);
   }
