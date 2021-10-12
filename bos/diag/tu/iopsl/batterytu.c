static char sccsid[] = "@(#)29  1.2  src/bos/diag/tu/iopsl/batterytu.c, tu_iopsl, bos411, 9428A410j 3/6/92 16:56:21";
/*
 * COMPONENT NAME : TU_IOPSL
 *
 * FUNCTIONS      : battery_tu
 *
 * ORIGINS        : 27
 *
 * IBM CONFIDENTIAL --
 *
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
*/

#include <sys/types.h>
#include <stdio.h>

#include <diag/atu.h>
#include "misc.h"
#include "salioaddr.h"

#define GOOD_BATTERY 0x80

/* This function checks if the battery is good */
int battery_tu(int fd, TUCB *pt)
{
  int rc;
  uchar_t reg_d;

  rc = SUCCESS;
  rc = get_tod_data(fd, (ulong_t)REG_D, &reg_d);

  if(rc == SUCCESS)
    if((reg_d & GOOD_BATTERY) != GOOD_BATTERY)
      rc = BATTERY_ERROR;

  return(rc);
}
