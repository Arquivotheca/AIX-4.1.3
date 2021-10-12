static char sccsid[] = "src/bos/diag/tu/eth/tu007.c, tu_eth, bos411, 9428A410j 6/19/91 14:58:07";
/*
 * COMPONENT_NAME: (TU_ETH) Ethernet Test Unit
 *
 * FUNCTIONS: tu007
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

Function(s) Test Unit 007 - External Wrap Test (DIX Connector)

Module Name :  tu007.c
SCCS ID     :  1.13

Current Date:  6/13/91, 13:11:27
Newest Delta:  1/19/90, 16:30:44

*****************************************************************************/
#include <stdio.h>
#include "ethtst.h"

#ifdef debugg
extern void detrace();
#endif

/*****************************************************************************

tu007

*****************************************************************************/

int tu007 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	extern int wrap();

	tucb_ptr->eth_htx_s.wrap_type = EXTERNAL_DIX;
	tucb_ptr->eth_htx_s.fairness = DISABLE;
	tucb_ptr->eth_htx_s.parity = DISABLE;
	return(wrap(fdes, tucb_ptr));
   }
