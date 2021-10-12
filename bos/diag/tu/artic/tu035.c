static char sccsid[] = "@(#)62  1.4  src/bos/diag/tu/artic/tu035.c, tu_artic, bos411, 9428A410j 8/19/93 17:51:30";
/*
 * COMPONENT_NAME:  
 *
 * FUNCTIONS: tu035
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

Function(s) Test Unit 035 SELECT EIB EXT WRAP P0 X21 

Module Name :  tu035.c HTX

*****************************************************************************/
#include <stdio.h>
#include "artictst.h"

/*****************************************************************************

tu035

*****************************************************************************/

int tu035 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int rc;
	extern int start_diag_tu();

	rc = start_diag_tu(fdes, tucb_ptr,SEL_WRAPX21_P0_COM_CODE,SEL_WP0X21_ER);
	return(rc);
   }
