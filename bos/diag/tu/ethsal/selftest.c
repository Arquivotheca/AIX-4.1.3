static char sccsid[] = "@(#)54  1.1.1.2  src/bos/diag/tu/ethsal/selftest.c, tu_ethsal, bos411, 9428A410j 10/20/93 14:14:45";
/*
 *   COMPONENT_NAME: tu_ethsal
 *
 *
 *   FUNCTIONS: selftest_tu
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/comio.h>
#include <sys/ciouser.h>
#include <sys/devinfo.h>
#include <sys/entuser.h>

#include "exectu.h"
#include "tu_type.h"

/*
 * NAME : selftest_tu
 *
 * DESCRIPTION :
 *
 * An ENT_SELFTEST ioctl is issued to the device
 * driver to perform the 82596 selftest.
 *
 *
 * INPUT :
 *
 *   None.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   Error code or SUCCESS.
 *
*/

int selftest_tu(void)
{
  int rc, halt_rc;

  struct
  {
    ulong_t rom;
    ulong_t test;
  } result;

  rc = halt_rc = SUCCESS;
  rc = start_ether();
  result.test = 0;

  if(rc == SUCCESS)
  {
    if(ioctl(get_fd(ETHS_FD), ENT_SELFTEST, &result) < 0)
    {
      set_tu_errno();
      incr_stat((uchar_t) BAD_OTHERS, 1);
      rc = ENT_SELFTEST_ERR;
    }
  }

  incr_stat((uchar_t) GOOD_OTHERS, 1);

  if(result.test != 0)
    rc = SELFTEST_ERR;

  halt_rc = halt_ether();

  if(rc == SUCCESS)
    rc = halt_rc;

  return(rc);
}




