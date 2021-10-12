static char sccsid[] = "@(#)34  1.1  src/bos/diag/tu/mps/mps_tu007.c, tu_mps, bos411, 9437B411a 8/23/94 16:25:26";
/*****************************************************************************
 * COMPONENT_NAME: (tu_mps)  Wildwood LAN adapter test units
 *
 * FUNCTIONS:   int tu007()
 *              int wrap_test()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *****************************************************************************/

/*** header files ***/
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mdio.h>
#include <sys/sleep.h>
#include <sys/errno.h>
#include <sys/sysconfig.h>
#include <cf.h>

/* local header files */
#include "mpstu_type.h"
#include "mps_regs.h"


/**************************************************************************** *
*
* FUNCTION NAME =  tu007()
*
* DESCRIPTION   =  This function initiates the network wrap test for MPS
*
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*              tucb_ptr       - ptr to test unit control block
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   non-zero
*
* INVOKED FROM = exectu
*
*****************************************************************************/
int tu007 (ADAPTER_STRUCT *adapter_info, TUTYPE *tucb_ptr)
{
  int rc = 0; /* return code */

  rc = network_test(adapter_info, tucb_ptr, NETWORK_TEST);

  return (rc);
}

