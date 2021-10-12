/* @(#)92	1.2  src/bos/usr/bin/localedef/err.h, cmdnls, bos411, 9428A410j 6/1/91 14:41:50 */
/*
 * COMPONENT_NAME: (CMDLOC) Locale Database Commands
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

#include "localedef_msg.h"

#define MALLOC(t,n)   ((t *)safe_malloc(sizeof(t)*(n),__FILE__,__LINE__))
void error(int,...);
void diag_error(int, ...);

#define INTERNAL_ERROR   error(ERR_INTERNAL, __FILE__, __LINE__)
