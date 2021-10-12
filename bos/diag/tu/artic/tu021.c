static char sccsid[] = "@(#)48  1.2.1.3  src/bos/diag/tu/artic/tu021.c, tu_artic, bos411, 9428A410j 8/19/93 17:50:36";
/*
 * COMPONENT_NAME:  
 *
 * FUNCTIONS: tu021
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

Function(s) Test Unit 021 CIO PORT B Wrap Test Unit for MP/2 adapter

Module Name :  tu021.c HTX

*****************************************************************************/
#include <stdio.h>
#include "artictst.h"

/*****************************************************************************

tu021

*****************************************************************************/

int tu021 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int rc;
	int type;
	extern int start_diag_tu();

/* If type indicates Multiport/2 with 8Port RS422, do not run test */
	if (icagetadaptype(fdes, &type))
          return(DRV_ERR);
        if (type == MP2_8P422)
	  return(0);

/* Execute the test */
	rc = start_diag_tu(fdes, tucb_ptr,WRAPCIO78B_COM_CODE, WP1_78_ER);

	return(rc);
   }
