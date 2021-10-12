static char sccsid[] = "@(#)32  1.1  src/bos/diag/tu/mps/mps_tu005.c, tu_mps, bos411, 9437B411a 8/23/94 16:25:22";
/*****************************************************************************
 * COMPONENT_NAME: (tu_mps)  Wildwood LAN adapter test units
 *
 * FUNCTIONS:   int tu005()
 *              int wrap_test()
 *              int open_adapter()
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
* FUNCTION NAME =  tu005()
*
* DESCRIPTION   =  This function initiates the internal wrap test for MPS
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
int tu005 (ADAPTER_STRUCT *adapter_info, TUTYPE *tucb_ptr)
{
  int rc = 0; /* return code */

  rc = wrap_test(adapter_info, tucb_ptr, INT_WRAP_TEST);
  
  return (rc);
}

