static char sccsid[]="@(#)93   1.1  src/bos/usr/fvcioem/ddt/tok/memdump.c, fvcioem, bos411, 9428A410j 4/26/94 13:54:09";

/*
 *
 * COMPONENT_NAME: (sysxcie_tc) COMIO Emulator Test Cases
 *
 * FUNCTIONS:
 *
 *   memdump
 *
 * DESCRIPTION:
 *
 *   Format a dump of memory
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

#include "memdump.h"
#include <stdio.h>

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
   )
{
   int                       totLen = 0; // Total formatted output length
   char                    * s;          // Ptr to data
   char                    * e;          // Ptr to end of data
   char                    * lo;         // s rounded down to width
   char                    * hi;         // e rounded up to width

   char                    * p;          // Scan pointer
   char                      buffer[160];// Work buffer

   static char               hexChar[16]  = "0123456789ABCDEF";

   static char               charSet[256] =
   {
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
   };

   width  = ((width > 16) ? 32 : ((width > 8) ? 16 : 8));
   indent = (indent > 30) ? 30 : indent;

   s  = data;
   e  = s + len;

   if (opt == dumpMem)
   {
      lo = (char *)(((unsigned long)s)         & ~((unsigned long)width-1));
      hi = (char *)(((unsigned long)(e+width)) & ~((unsigned long)width-1));
   }
   else
   {
      lo = s;
      hi = lo + ((unsigned long)(len+width-1) & ~((unsigned long)width-1));
   }

   p = lo;

   while(p < hi)
   {
      int                    i;
      char                 * dx = buffer+indent;
      char                 * dc;

      memset(buffer,' ',sizeof(buffer));

      if (opt == dumpMem)
      {
         dx += sprintf(dx,"%8.8x  ",p);
      }
      else
      {
         const int           width  = (len & 0xFFFF0000) ? 8 : 4;
         unsigned int        curOff = p-lo;

         dx += sprintf(dx,"%*.*x  ",width,width,curOff);
      }

      dc = dx + 2*width + width/4 + 2;

      for(i=0; i<width; i++)
      {
         if (p < s || p >= e)
         {
            *dx++ = ' ';
            *dx++ = ' ';
            *dc++ = ' ';
         }
         else
         {
            *dx++ = hexChar[*p>>4 & 0x0F];
            *dx++ = hexChar[*p    & 0x0F];
            *dc++ = (char)(charSet[*p] ? *p : '.');
         }

         p++;

         dx += (i%4 == 3);
      }

      *++dx = '<';
      *dc++ = '>';
      *dc++ = '\n';
      *dc   = 0;

      strcpy(out,buffer);

      out    += (dc-buffer);
      totLen += (dc-buffer);
   }

   return totLen;
}
