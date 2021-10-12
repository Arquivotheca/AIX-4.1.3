static char sccsid[] = "@(#)61  1.4  src/bos/diag/tu/artic/tu034.c, tu_artic, bos411, 9428A410j 8/19/93 17:51:25";
/*
 * COMPONENT_NAME:  
 *
 * FUNCTIONS: tu034
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

Function(s) Test Unit 034 SELECT EIB EXT WRAP P0 RS422 

Module Name :  tu034.c HTX

*****************************************************************************/
#include <stdio.h>
#include "artictst.h"

/*****************************************************************************

tu034

*****************************************************************************/

int tu034 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int rc;
	extern int start_diag_tu();

	rc = start_diag_tu(fdes, tucb_ptr,SEL_WRAPRS422_P0_COM_CODE,SEL_WP0422_ER);
	return(rc);
   }
