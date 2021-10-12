/* @(#)94   1.1  src/bos/usr/fvcioem/ddt/tok/memdump.h, fvcioem, bos411, 9428A410j 4/26/94 13:54:11 */

/*
 *
 * COMPONENT_NAME: (sysxcie_tc) COMIO Emulator Test Cases
 *
 * FUNCTIONS:
 *
 *    memDump
 *
 * DESCRIPTION:
 *
 *    Memory Dump Formatting Routine
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

/*---------------------------------------------------------------------------*/
/*                               Dump Options                                */
/*---------------------------------------------------------------------------*/

typedef enum DUMPOPT         DUMPOPT;

enum DUMPOPT
{
   dumpMem,                  // Format addresses as memory (seg:off) refs
   dumpRel                   // Format addresses as relative offsets
};

/*---------------------------------------------------------------------------*/
/*                              Hex Memory Dump                              */
/*---------------------------------------------------------------------------*/

int
   memDump(
      char                 * out         ,// IO-Output Buffer
      int                    indent      ,// I -Output Line Indent
      int                    width       ,// I -Output Line Width (src bytes)
      DUMPOPT                opt         ,// I -Dump Options
      void                 * data        ,// I -Source Data
      int                    len          // I -Length of Source
   );
