static char sccsid[] = "@(#)56  1.2.1.2  src/bos/diag/tu/artic/tu029.c, tu_artic, bos411, 9428A410j 8/19/93 17:51:11";
/*
 * COMPONENT_NAME:  
 *
 * FUNCTIONS: tu029
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

Function(s) Test Unit 029  PORT 7 Wrap Test Unit for MP/2 adapter 

Module Name :  tu029.c HTX

*****************************************************************************/
#include <stdio.h>
#include "artictst.h"

/*****************************************************************************

tu029

*****************************************************************************/

int tu029 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int rc;
	extern int start_diag_tu();

	rc = start_diag_tu(fdes, tucb_ptr,WRAP_P7_COM_CODE, WP7_ER);
	return(rc);
   }
