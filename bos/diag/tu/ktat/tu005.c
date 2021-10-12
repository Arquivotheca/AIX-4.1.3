static char sccsid[] = "@(#)93	1.2  src/bos/diag/tu/ktat/tu005.c, tu_ktat, bos41J, 9519A_all 5/3/95 14:57:34";
/*
 *   COMPONENT_NAME: tu_ktat
 *
 *   FUNCTIONS: tu005
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


#define TU5_FAILED 0x05000000

tu005()
{
  int rc, wrap_mode;

  wrap_mode = t_r_len_mode3;
  if(rc = tu_wrap(wrap_mode, aui_port))
  {
    rc = rc | TU5_FAILED;
    return(rc);
  } 
  return(0);
} /*  end tu005  */


