static char sccsid[] = "@(#)96  1.2  src/bos/diag/tu/kbd/mktu_rc.c, tu_kbd, bos411, 9428A410j 5/10/94 14:20:30";
/*
 * COMPONENT_NAME: tu_kbd
 *
 * FUNCTIONS: mktu_rc
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************

Function(s) Make Test Unit Return Code

Function takes test unit number (tu_no), error type (err_type),
and error code (error_code) to form a special unique return code.

*****************************************************************************/
#include "tu_type.h"	/* among others, contains defs. for error types */

int mktu_rc (fdes,tu_no, err_type, error_code) 
   int fdes;
   int tu_no;
   int err_type;
   int error_code;
   {
	static int rc;
        unsigned char cdata;

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
