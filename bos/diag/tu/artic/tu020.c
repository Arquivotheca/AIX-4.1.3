static char sccsid[] = "@(#)47  1.2.1.3  src/bos/diag/tu/artic/tu020.c, tu_artic, bos411, 9428A410j 8/19/93 17:50:31";
/*
 * COMPONENT_NAME:  
 *
 * FUNCTIONS: tu020
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

Function(s) Test Unit 020 CIO PORT A Wrap Test Unit for MP/2 adapter

Module Name :  tu020.c HTX

*****************************************************************************/
#include <stdio.h>
#include "artictst.h"

/*****************************************************************************

tu020

*****************************************************************************/

int tu020 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int rc;
	int type;
	extern int start_diag_tu();

/* If adapter_type indicates Multiport/2 with 8Port RS422, do not run test */
	if (icagetadaptype(fdes, &type))
	   return(DRV_ERR);

        if (type == MP2_8P422)
           return(0);

/* Execute the test */
	rc = start_diag_tu(fdes, tucb_ptr,WRAPCIO78A_COM_CODE, WP0_78_ER);

	return(rc);
   }
