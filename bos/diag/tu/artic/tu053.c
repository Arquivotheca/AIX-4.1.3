static char sccsid[] = "@(#)80  1.2.1.2  src/bos/diag/tu/artic/tu053.c, tu_artic, bos411, 9428A410j 8/19/93 17:48:17";
/*
 * COMPONENT_NAME:  
 *
 * FUNCTIONS: tu053
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*****************************************************************************

Function(s) Test Unit 053  PORT 0 Wrap Test Unit for Portmaster adapter 

Module Name :  tu053.c HTX

*****************************************************************************/
#include <stdio.h>
#include "artictst.h"

/*****************************************************************************

tu053

*****************************************************************************/

int tu053 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int rc;
	extern int start_diag_tu();

	rc = start_diag_tu(fdes, tucb_ptr,WRAP_P0_COM_PCODE, WP0_ER);
	return(rc);
   }
