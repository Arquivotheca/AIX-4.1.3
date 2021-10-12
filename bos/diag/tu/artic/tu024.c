static char sccsid[] = "@(#)51  1.3.1.2  src/bos/diag/tu/artic/tu024.c, tu_artic, bos411, 9428A410j 8/19/93 17:50:51";
/*
 * COMPONENT_NAME:  
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
 *
 *
 * FUNCTIONS: tu024
 *
 */
/*****************************************************************************

Function(s) Test Unit 024  PORT 2 Wrap Test Unit for MP/2 adapter 

Module Name :  tu024.c HTX

*****************************************************************************/
#include <stdio.h>
#include "artictst.h"

/*****************************************************************************

tu024

*****************************************************************************/

int tu024 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int rc;
	extern int start_diag_tu();
 
	rc = start_diag_tu(fdes, tucb_ptr,WRAP_P2_COM_CODE, WP2_ER);
	return(rc);
   }
