static char sccsid[] = "@(#)82  1.2.1.2  src/bos/diag/tu/artic/tu055.c, tu_artic, bos411, 9428A410j 8/19/93 17:48:28";
/*
 * COMPONENT_NAME:  
 *
 * FUNCTIONS: tu055
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

Function(s) Test Unit 055  PORT 2 Wrap Test Unit for Portmaster adapter 

Module Name :  tu055.c HTX

*****************************************************************************/
#include <stdio.h>
#include "artictst.h"

/*****************************************************************************

tu055

*****************************************************************************/

int tu055 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int rc;
	extern int start_diag_tu();

	rc = start_diag_tu(fdes, tucb_ptr,WRAP_P2_COM_PCODE, WP2_ER);
	return(rc);
   }
