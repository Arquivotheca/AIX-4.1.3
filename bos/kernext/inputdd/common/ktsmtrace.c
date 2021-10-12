static char sccsid[] = "@(#)04   1.1  src/bos/kernext/inputdd/common/ktsmtrace.c, inputdd, bos411, 9428A410j 10/24/93 14:44:15";
/*
 * COMPONENT_NAME: (INPUTDD) Keyboard/Tablet/Sound/Mouse DD - ktsmtrace.c
 *
 * FUNCTIONS: None
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
 */

#include "ktsmtrace.h"

/*

  defines DEBUG level tracing for driver, does not generate code if
  ship level (ie: GS_DEBUG_TRACE is not defined)

*/

#ifdef GS_DEBUG_TRACE
   GS_TRC_GLB(0,TRC_PRIORITY);
   GS_TRC_MODULE(inputdd, 0);
#endif
