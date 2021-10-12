/* @(#)85   1.1  src/bos/usr/fvcioem/ddt/common/ddtlog.h, fvcioem, bos411, 9428A410j 4/26/94 13:53:51 */

/*
 *
 * COMPONENT_NAME: (sysxcie_tc) COMIO Emulator Test Tools
 *
 * FUNCTIONS:
 *
 *   initLog
 *   printLog
 *   fprintLog
 *
 * DESCRIPTION:
 *
 *    DDT stdout/stderr logging routines
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#if !defined(DDTLOG_H)
#define  DDTLOG_H

void
   initLog(
      int                    argc        ,
      char                 * argv[]
   );

int
   printLog(
      char                 * fmt         ,
      ...
   );

int
   fprintLog(
      FILE                 * f           ,
      char                 * fmt         ,
      ...
   );

#define printf     printLog
#define fprintf    fprintLog

#endif
