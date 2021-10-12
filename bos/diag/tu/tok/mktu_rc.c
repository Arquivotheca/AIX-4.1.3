static char sccsid[] = "src/bos/diag/tu/tok/mktu_rc.c, tu_tok, bos411, 9428A410j 6/20/91 12:41:52";
/*
 * COMPONENT_NAME: (TU_TOK) Token Test Unit
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
/*****************************************************************************

Function(s) Make Test Unit Return Code

Module Name :  mktu_rc.c
SCCS ID     :  1.5

Current Date:  6/20/91, 10:14:45
Newest Delta:  1/19/90, 16:49:13

Function takes test unit number (tu_no), error type (err_type),
and error code (error_code) to form a special unique return code.

*****************************************************************************/
#include "toktst.h"	/* among others, contains defs. for error types */

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
