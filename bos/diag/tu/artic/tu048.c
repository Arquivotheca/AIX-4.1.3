static char sccsid[] = "@(#)75  1.5  src/bos/diag/tu/artic/tu048.c, tu_artic, bos411, 9428A410j 3/8/94 14:57:29";
/*
 * COMPONENT_NAME:  
 *
 * FUNCTIONS: tu048
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

Function(s) Test Unit 048 SELECT EIB CABLE WRAP P2 RS232 

Module Name :  tu048.c HTX

*****************************************************************************/
#include <stdio.h>
#include "artictst.h"

/*****************************************************************************

tu048

*****************************************************************************/

int tu048 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int rc;
	extern int start_diag_tu();

	rc = start_diag_tu(fdes, tucb_ptr,SEL_CABLERS232_P2_COM_CODE,SEL_WP2232_ER);
	return(rc);
   }
