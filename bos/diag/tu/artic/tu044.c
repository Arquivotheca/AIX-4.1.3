static char sccsid[] = "@(#)71  1.5  src/bos/diag/tu/artic/tu044.c, tu_artic, bos411, 9428A410j 3/8/94 14:54:31";
/*
 * COMPONENT_NAME:  
 *
 * FUNCTIONS: tu044
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

Function(s) Test Unit 044 SELECT EIB CABLE WRAP P0 X21 

Module Name :  tu044.c HTX

*****************************************************************************/
#include <stdio.h>
#include "artictst.h"

/*****************************************************************************

tu044

*****************************************************************************/

int tu044 (fdes, tucb_ptr) 
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int rc;
	extern int start_diag_tu();

	rc = start_diag_tu(fdes, tucb_ptr,SEL_CABLEX21_P0_COM_CODE,SEL_WP0X21_ER);
	return(rc);
   }
