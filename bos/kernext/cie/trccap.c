static char sccsid[]="@(#)34   1.8  src/bos/kernext/cie/trccap.c, sysxcie, bos411, 9428A410j 6/14/94 11:34:26";

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 *   new_TRACE_TABLE
 *   free_TRACE_TABLE
 *
 * DESCRIPTION:
 *
 *    Trace Capture Routines
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

#include <sys/errno.h>
#include <sys/malloc.h>
#include <sys/m_intr.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>

#include "ciedd.h"
#include "trccap.h"
#include "dmalloc.h"
#include "dev.h"

/*---------------------------------------------------------------------------*/
/*                        Allocate a new Trace Table                         */
/*---------------------------------------------------------------------------*/

TRACE_TABLE *
   new_TRACE_TABLE(
      register int           tSize        // I -Size of trace table
   )
{
   FUNC_NAME(new_TRACE_TABLE);

   int                       priSave;
   int                       tBytes;
   TRACE_TABLE             * tt;

   tBytes = offsetof(TRACE_TABLE,table) + tSize * sizeof(TRACE_ENTRY);

   if ((tt = xmalloc(tBytes,4,pinned_heap)) == NULL) return ENOMEM;

   memset(tt,0x00,tBytes);

   strcpy(tt->iCatcher,"CIEDDTRACE");

   tt->next = &(tt->table[0]);
   tt->last = &(tt->table[tSize]);

   lock_alloc(&tt->lock,
              LOCK_ALLOC_PIN,
              CIO_LOCK_CLASS,
              CIE_LOCK_TRACE);

   simple_lock_init(&tt->lock);

   tt->table[0].traceID = 0x21212121;         // !!!!

   /*dbgout("Trace table allocated at %x",tt);*/

   return tt;
}

/*---------------------------------------------------------------------------*/
/*                           Delete a Trace Table                            */
/*---------------------------------------------------------------------------*/

void
   free_TRACE_TABLE(
      register TRACE_TABLE * tt           // IO-Trace Table
   )
{
   FUNC_NAME(free_TRACE_TABLE);

   register int              priSave;

   if (tt)
   {
      lock_free(&tt->lock);
      if (xmfree(tt,pinned_heap) != 0) XFFAIL();
   }
}

#if !defined(INLINE)
#include "trccap.inl"
#endif
