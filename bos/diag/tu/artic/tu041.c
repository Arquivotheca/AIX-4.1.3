static char sccsid[] = "@(#)68  1.4  src/bos/diag/tu/artic/tu041.c, tu_artic, bos411, 9428A410j 8/19/93 17:51:53";
/*
 * COMPONENT_NAME:  
 *
 * FUNCTIONS: tu041
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

Function(s) Test Unit 041 SELECT EIB EXT WRAP P3 RS232 

Module Name :  tu041.c HTX

*****************************************************************************/
#include <stdio.h>
#include "artictst.h"

/*****************************************************************************

tu041

*****************************************************************************/

int tu041 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int rc;
	extern int start_diag_tu();

	rc = start_diag_tu(fdes, tucb_ptr,SEL_WRAPRS232_P3_COM_CODE,SEL_WP3232_ER);
	return(rc);
   }
