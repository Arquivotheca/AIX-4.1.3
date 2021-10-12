static char sccsid[] = "@(#)76  1.5  src/bos/diag/tu/artic/tu049.c, tu_artic, bos411, 9428A410j 3/8/94 14:58:29";
/*
 * COMPONENT_NAME:  
 *
 * FUNCTIONS: tu049
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

Function(s) Test Unit 049 SELECT EIB CABLE WRAP P2 RS422 

Module Name :  tu049.c HTX

*****************************************************************************/
#include <stdio.h>
#include "artictst.h"

/*****************************************************************************

tu049

*****************************************************************************/

int tu049 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int rc;
	extern int start_diag_tu();

	rc = start_diag_tu(fdes, tucb_ptr,SEL_CABLERS422_P2_COM_CODE,SEL_WP2422_ER);
	return(rc);
   }
