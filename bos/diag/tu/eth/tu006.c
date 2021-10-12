static char sccsid[] = "src/bos/diag/tu/eth/tu006.c, tu_eth, bos411, 9428A410j 6/19/91 14:57:01";
/*
 * COMPONENT_NAME: (TU_ETH) Ethernet Test Unit
 *
 * FUNCTIONS: tu006
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************

Function(s) Test Unit 006 - Internal Wrap Test

Module Name :  tu006.c
SCCS ID     :  1.8

Current Date:  6/13/91, 13:11:27
Newest Delta:  1/19/90, 16:30:32

*****************************************************************************/
#include <stdio.h>
#include "ethtst.h"

#ifdef debugg
extern void detrace();
#endif

/*****************************************************************************

tu006

*****************************************************************************/

int tu006 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	extern int wrap();

	tucb_ptr->eth_htx_s.wrap_type = INTERNAL;
	tucb_ptr->eth_htx_s.fairness = DISABLE;
	tucb_ptr->eth_htx_s.parity = DISABLE;
	return(wrap(fdes, tucb_ptr));
   }
