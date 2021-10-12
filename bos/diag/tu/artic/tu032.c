static char sccsid[] = "@(#)59  1.4  src/bos/diag/tu/artic/tu032.c, tu_artic, bos411, 9428A410j 8/19/93 17:51:18";
/*
 * COMPONENT_NAME:  
 *
 * FUNCTIONS: tu032
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

#include <stdio.h>
#include "artictst.h"

/*
 * NAME: tu032
 *
 * FUNCTION: Portmaster 4 port selectable (SELECT EIB RELAY TEST) 
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 *
 */
int tu032 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	extern int start_diag_tu();

	return(start_diag_tu(fdes, tucb_ptr,SEL_RELAY_COM_CODE,SEL_RELAY_ER));
   }
