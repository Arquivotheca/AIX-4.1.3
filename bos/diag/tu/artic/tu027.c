static char sccsid[] = "@(#)54  1.2.1.2  src/bos/diag/tu/artic/tu027.c, tu_artic, bos411, 9428A410j 8/19/93 17:51:03";
/*
 * COMPONENT_NAME:  
 *
 * FUNCTIONS: tu027
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

Function(s) Test Unit 027  PORT 5 Wrap Test Unit for MP/2 adapter 

Module Name :  tu027.c HTX

*****************************************************************************/
#include <stdio.h>
#include "artictst.h"

/*****************************************************************************

tu027

*****************************************************************************/

int tu027 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int rc;
	extern int start_diag_tu();

	rc = start_diag_tu(fdes, tucb_ptr,WRAP_P5_COM_CODE, WP5_ER);
	return(rc);
   }
