static char sccsid[] = "@(#)23	1.1  src/bos/diag/tu/pcitok/sky_tu007.c, tu_pcitok, bos41J, 9512A_all 3/21/95 17:01:33";
/*
 *   COMPONENT_NAME: tu_pcitok
 *
 *   FUNCTIONS: tu007
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************
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
#include "skytu_type.h"
#include "sky_regs.h"


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

