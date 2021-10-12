static char sccsid[] = "@(#)94	1.2  src/bos/diag/tu/ktat/tu006.c, tu_ktat, bos41J, 9519A_all 5/3/95 14:57:52";
/*
 *   COMPONENT_NAME: tu_ktat
 *
 *   FUNCTIONS: tu006
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

#include <stdio.h>
#include <sys/diagex.h>
#include "kent_defs.h"
#include "kent_tu_type.h"
#include <sys/errno.h>


#define TU6_FAILED 0x06000000

tu006()
{
  int rc, wrap_mode;

  wrap_mode = t_r_len_mode4;
  if(rc = tu_wrap(wrap_mode, aui_port))
  {
    rc = rc | TU6_FAILED;
    return(rc);
  } 
  return(0);
} /*  end tu006  */


