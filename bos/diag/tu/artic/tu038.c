static char sccsid[] = "@(#)65  1.4  src/bos/diag/tu/artic/tu038.c, tu_artic, bos411, 9428A410j 8/19/93 17:51:41";
/*
 * COMPONENT_NAME:  
 *
 * FUNCTIONS: tu038
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

Function(s) Test Unit 038 SELECT EIB EXT WRAP P1 V35 

Module Name :  tu038.c HTX

*****************************************************************************/
#include <stdio.h>
#include "artictst.h"

/*****************************************************************************

tu038

*****************************************************************************/

int tu038 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int rc;
	extern int start_diag_tu();

	rc = start_diag_tu(fdes, tucb_ptr,SEL_WRAPV35_P1_COM_CODE,SEL_WP1V35_ER);
	return(rc);
   }
