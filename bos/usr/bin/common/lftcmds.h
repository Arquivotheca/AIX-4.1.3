/* @(#)07	1.1  src/bos/usr/bin/common/lftcmds.h, cmdlft, bos411, 9428A410j 10/18/93 10:00:18 */
/*
 *   COMPONENT_NAME: CMDLFT
 *
 * FUNCTIONS:  none
 *
 *   ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*------------
  Include files
  ------------*/
#include <nl_types.h>
#include <ctype.h>

/*------------
  Message facilities
  ------------*/
nl_catd catd;
#include "lftcmds_msg.h"
#include "lftcmds_cat.h"

#define MSGSTR(setnum, msgnum, string) catgets(catd, setnum, msgnum, string)

/*------------
  Global data values
  ------------*/
char program[24];               /* program name */

