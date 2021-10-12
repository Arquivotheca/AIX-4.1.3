static char sccsid[]="@(#)84   1.1  src/bos/usr/fvcioem/ddt/common/ddtlog.c, fvcioem, bos411, 9428A410j 4/26/94 13:53:49";

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

#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

static int                   firstTime = 1;
static FILE                * logFile = NULL;

void
   initLog(
      int                    argc        ,
      char                 * argv[]
   )
{
   if (argc < 2)
      printf("Log file not specified on command line\n");
   else
   {
      if ((logFile = fopen(argv[1],"w")) == NULL)
         perror("Unable to open log file");
   }
   return;
}

int
   printLog(
      char                 * fmt         ,
      ...
   )
{
   va_list                   arglist;


   va_start(arglist,fmt);

   if (logFile) vfprintf(logFile,fmt,arglist);

   return vprintf(fmt,arglist);
}

int
   fprintLog(
      FILE                 * f           ,
      char                 * fmt         ,
      ...
   )
{
   va_list                   arglist;

   va_start(arglist,fmt);

   if (logFile) vfprintf(logFile,fmt,arglist);

   return vfprintf(f,fmt,arglist);
}
