static char sccsid[] = "@(#)63  1.4  src/bos/diag/tu/artic/tu036.c, tu_artic, bos411, 9428A410j 8/19/93 17:51:33";
/*
 * COMPONENT_NAME:  
 *
 * FUNCTIONS: tu036
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

Function(s) Test Unit 036 SELECT EIB EXT WRAP P0 V35 

Module Name :  tu036.c HTX

*****************************************************************************/
#include <stdio.h>
#include "artictst.h"

/*****************************************************************************

tu036

*****************************************************************************/

int tu036 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int rc;
	extern int start_diag_tu();

	rc = start_diag_tu(fdes, tucb_ptr,SEL_WRAPV35_P0_COM_CODE,SEL_WP0V35_ER);
	return(rc);
   }
