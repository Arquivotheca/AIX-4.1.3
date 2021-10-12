static char sccsid[] = "@(#)43	1.1  src/bos/diag/tu/fddi/mktu_rc.c, tu_fddi, bos411, 9428A410j 7/9/91 12:39:58";
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: mktu_rc
 *     
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*****************************************************************************

Function(s) Make Test Unit Return Code

Module Name :  mktu_rc.c
SCCS ID     :  1.5

Current Date:  5/23/90, 11:17:46
Newest Delta:  1/19/90, 16:25:14

Function takes test unit number (tu_no), error type (err_type),
and error code (error_code) to form a special unique return code.

*****************************************************************************/
#include "fdditst.h"	/* among others, contains defs. for error types */

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
