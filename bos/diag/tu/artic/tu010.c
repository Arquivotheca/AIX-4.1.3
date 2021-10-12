static char sccsid[] = "@(#)38  1.4  src/bos/diag/tu/artic/tu010.c, tu_artic, bos411, 9428A410j 8/19/93 17:49:48";
/*
 * COMPONENT_NAME:  
 *
 * FUNCTIONS: tu010
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

/*****************************************************************************

Function(s) Test Unit 010 - X.21, V.24, V.35 Wrap Test Unit for C2X adapters

Module Name :  tu010.c HTX

*****************************************************************************/
#include <stdio.h>
#include "artictst.h"

/*****************************************************************************

tu010

*****************************************************************************/

int tu010 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int rc;
	extern int start_diag_tu();

	rc = start_diag_tu(fdes, tucb_ptr, WRAPX21_COM_CODE, WX21_37_ER);
	if (rc)
		return(rc);
	rc = start_diag_tu(fdes, tucb_ptr, WRAPV24_COM_CODE, WV24_37_ER);
	if (rc)
		return(rc);
	rc = start_diag_tu(fdes, tucb_ptr, WRAPV35_COM_CODE, WV35_37_ER);
	if (rc)
		return(rc);
	rc = start_diag_tu(fdes, tucb_ptr, WRAPCIOV35_COM_CODE, WV35_37_ER);
	return(rc);
   }
