/* @(#)79   1.3  src/bos/kernext/cie/trccap.inl, sysxcie, bos411, 9428A410j 4/19/94 14:00:24 */

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 *   cieTrace
 *
 * DESCRIPTION:
 *
 *   Trace Table (inline functions)
 *
 * ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#if defined(INLINE)

#include <stddef.h>
#include <sys/errno.h>
#include <sys/malloc.h>
#include <sys/m_intr.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#include <net/spl.h>

#define inline  static

#else

#define inline

#endif

/*---------------------------------------------------------------------------*/
/*                           Record a Trace Entry                            */
/*---------------------------------------------------------------------------*/

inline
void
   cieTrace(
      register TRACE_TABLE * tt          ,// IO-Trace Table
      register ulong         traceID     ,// I -Trace ID
      register ulong         arg1        ,// I -First Trace Data Word
      register ulong         arg2        ,// I -Second Trace Data Word
      register ulong         arg3         // I -Third Trace Data Word
   )
{
   if (tt)
   {
      register TRACE_ENTRY * te;
      register int           pSave;

      pSave = disable_lock(PL_IMP,&tt->lock);

      te = tt->next;

      /*--------------------------------*/
      /*  Allocate a Trace Table Entry  */
      /*--------------------------------*/

      te->traceID = traceID;
      te->arg1    = arg1;
      te->arg2    = arg2;
      te->arg3    = arg3;

      tt->next = (++te < tt->last) ? te : &(tt->table[0]);

      te->traceID = 0x21212121;             // !!!!

      unlock_enable(pSave,&tt->lock);
   }
}
