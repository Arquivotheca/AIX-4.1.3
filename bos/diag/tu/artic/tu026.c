static char sccsid[] = "@(#)53  1.2.1.2  src/bos/diag/tu/artic/tu026.c, tu_artic, bos411, 9428A410j 8/19/93 17:50:59";
/*
 * COMPONENT_NAME:  
 *
 * FUNCTIONS: tu026
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

Function(s) Test Unit 026  PORT 4 Wrap Test Unit for MP/2 adapter 

Module Name :  tu026.c HTX

*****************************************************************************/
#include <stdio.h>
#include "artictst.h"

/*****************************************************************************

tu026

*****************************************************************************/

int tu026 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int rc;
	extern int start_diag_tu();

	rc = start_diag_tu(fdes, tucb_ptr,WRAP_P4_COM_CODE, WP4_ER);
	return(rc);
   }
