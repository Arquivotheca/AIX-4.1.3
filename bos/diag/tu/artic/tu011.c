static char sccsid[] = "@(#)39  1.4  src/bos/diag/tu/artic/tu011.c, tu_artic, bos411, 9428A410j 8/19/93 17:49:53";
/*
 * COMPONENT_NAME:  
 *
 * FUNCTIONS: tu011
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

Function(s) Test Unit 011 - X.21 Wrap Test Unit for C2X adapter

Module Name :  tu011.c HTX

*****************************************************************************/
#include <stdio.h>
#include "artictst.h"

/*****************************************************************************

tu011

*****************************************************************************/

int tu011 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int rc;
	extern int start_diag_tu();

	rc = start_diag_tu(fdes, tucb_ptr, WRAPX21_COM_CODE, WX21_ER);
	return(rc);
   }
