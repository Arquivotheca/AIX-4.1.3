static char sccsid[] = "@(#)72  1.5  src/bos/diag/tu/artic/tu045.c, tu_artic, bos411, 9428A410j 3/8/94 14:55:21";
/*
 * COMPONENT_NAME:   
 *
 * FUNCTIONS: tu045
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

Function(s) Test Unit 045 SELECT EIB CABLE WRAP P0 V35 

Module Name :  tu045.c HTX

*****************************************************************************/
#include <stdio.h>
#include "artictst.h"

/*****************************************************************************

tu045

*****************************************************************************/

int tu045 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int rc;
	extern int start_diag_tu();

	rc = start_diag_tu(fdes, tucb_ptr,SEL_CABLEV35_P0_COM_CODE,SEL_WP0V35_ER);
	return(rc);
   }
