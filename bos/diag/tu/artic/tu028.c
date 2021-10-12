static char sccsid[] = "@(#)55  1.2.1.2  src/bos/diag/tu/artic/tu028.c, tu_artic, bos411, 9428A410j 8/19/93 17:51:07";
/*
 * COMPONENT_NAME:  
 *
 * FUNCTIONS: tu028
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

Function(s) Test Unit 028  PORT 6 Wrap Test Unit for MP/2 adapter 

Module Name :  tu028.c HTX

*****************************************************************************/
#include <stdio.h>
#include "artictst.h"

/*****************************************************************************

tu028

*****************************************************************************/

int tu028 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int rc;
	extern int start_diag_tu();

	rc = start_diag_tu(fdes, tucb_ptr,WRAP_P6_COM_CODE, WP6_ER);
	return(rc);
   }
