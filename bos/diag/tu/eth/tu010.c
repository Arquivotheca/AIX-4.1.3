static char sccsid[] = "src/bos/diag/tu/eth/tu010.c, tu_eth, bos411, 9428A410j 6/19/91 15:00:50";
/*
 * COMPONENT_NAME: (TU_ETH) Ethernet Test Unit
 *
 * FUNCTIONS: tu010
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

Function(s) Test Unit 010 - External Wrap Test (BNC Connector) with Fairness

Module Name :  tu010.c
SCCS ID     :  1.4

Current Date:  6/13/91, 13:11:29
Newest Delta:  3/22/91, 12:59:40

*****************************************************************************/
#include <stdio.h>
#include "ethtst.h"

#ifdef debugg
extern void detrace();
#endif

/*****************************************************************************

tu010

*****************************************************************************/

int tu010 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	extern int wrap();
	extern int mktu_rc();

	/* Verify that the DIX Connector has been removed	*/
	/* for the BNC Wrap test				*/
	/*
	tucb_ptr->eth_htx_s.wrap_type = EXTERNAL_DIX;
	tucb_ptr->eth_htx_s.fairness = DISABLE;
	tucb_ptr->eth_htx_s.parity = DISABLE;
	if ((wrap(fdes, tucb_ptr) & 0x0000ffff) != TXBD_NOK_ERR)
		return(mktu_rc(tucb_ptr->header.tu,LOG_ERR,DIX_ON_BNC));
	*/

	tucb_ptr->eth_htx_s.wrap_type = EXTERNAL_BNC;
	tucb_ptr->eth_htx_s.fairness = ENABLE;
	tucb_ptr->eth_htx_s.parity = DISABLE;
	return(wrap(fdes, tucb_ptr));
   }
