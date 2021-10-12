/* @(#)17  1.2  src/bos/kernext/dlc/qlc/qlcerrlog.h, sysxdlcq, bos411, 9428A410j 11/2/93 09:34:04 */
#ifndef _H_QLCERRLOG
#define _H_QLCERRLOG
/*
 * COMPONENT_NAME: (SYSXDLCQ) X.25 QLLC module
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*****************************************************************************/
/* ANSI prototype for errlogger function.                                    */
/*****************************************************************************/

extern void errlogger(
  unsigned       error_id,
  char          *resource_name,
  unsigned int   user_ls_correlator,
  char          *calling_address,
  char          *called_address);


#endif
