/* @(#)58   1.8  src/bos/kernext/cie/trccap.h, sysxcie, bos411, 9428A410j 4/21/94 09:15:12 */

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 *   NETADDR1
 *   NETADDR2
 *   XMFAIL
 *   XFFAIL
 *   TRC_XMIT
 *   TRC_RECV
 *   TRC_OTHER
 *   TRC_DEBUG
 *   FUNC_NAME
 *   new_TRACE_TABLE
 *   free_TRACE_TABLE
 *   cieTrace
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

#if !defined(TRCCAP_H)
#define  TRCCAP_H

#include <sys/lock_def.h>
#include <sys/trcmacros.h>

/*---------------------------------------------------------------------------*/
/*                                Trace Hooks                                */
/*---------------------------------------------------------------------------*/

#define  HKWD_CIOEM_RECV     0x33D00000
#define  HKWD_CIOEM_XMIT     0x33E00000
#define  HKWD_CIOEM_OTHER    0x33F00000

/*---------------------------------------------------------------------------*/
/*  Macros to extract portions of a net address into a word                  */
/*  for inclusion in trace statements.                                       */
/*                                                                           */
/*    NETADDR1 extracts the first 4 bytes into a fullword                    */
/*    NETADDR2 extracts the last 2 bytes << 16 into a word                   */
/*---------------------------------------------------------------------------*/

#define  NETADDR1(x)  (*(ulong *)(x))
#define  NETADDR2(x)  (((ulong)(((ushort *)(x))[2])) << 16)

/*---------------------------------------------------------------------------*/
/*                      Trace macro for xmalloc failure                      */
/*---------------------------------------------------------------------------*/

#define  XMFAIL(bytes)         \
(                              \
   cieTrace                    \
   (                           \
      dmgr.traceTable,         \
      *(ulong *)"xmal",        \
      (ulong)(bytes),          \
      (ulong)(__FILE__),       \
      (ulong)(__LINE__)        \
   ),                          \
   TRCHKGT                     \
   (                           \
      HKWD_CIOEM_OTHER,        \
      *(ulong *)"xmal",        \
      (ulong)(bytes),          \
      (ulong)(__FILE__),       \
      (ulong)(__LINE__),       \
      (ulong)(0)               \
   )                           \
)

/*---------------------------------------------------------------------------*/
/*                     Trace macro for xmfree() failure                      */
/*---------------------------------------------------------------------------*/

#define  XFFAIL()              \
(                              \
   cieTrace                    \
   (                           \
      dmgr.traceTable,         \
      *(ulong *)"xmfr",        \
      (ulong)(0),              \
      (ulong)(__FILE__),       \
      (ulong)(__LINE__)        \
   ),                          \
   TRCHKGT                     \
   (                           \
      HKWD_CIOEM_OTHER,        \
      *(ulong *)"xmfr",        \
      (ulong)(0),              \
      (ulong)(__FILE__),       \
      (ulong)(__LINE__),       \
      (ulong)(0)               \
   )                           \
)

/*---------------------------------------------------------------------------*/
/*                       Trace macro for Transmit Path                       */
/*---------------------------------------------------------------------------*/

#define  TRC_XMIT(tag,a,b,c)   \
(                              \
   cieTrace                    \
   (                           \
      dmgr.traceTable,         \
      *(ulong *)(#tag),        \
      (ulong)(a),              \
      (ulong)(b),              \
      (ulong)(c)               \
   ),                          \
   TRCHKGT                     \
   (                           \
      HKWD_CIOEM_XMIT,         \
      *(ulong *)(#tag),        \
      (ulong)(a),              \
      (ulong)(b),              \
      (ulong)(c),              \
      (ulong)(0)               \
   )                           \
)

/*---------------------------------------------------------------------------*/
/*                       Trace macro for Receive Path                        */
/*---------------------------------------------------------------------------*/

#define  TRC_RECV(tag,a,b,c)   \
(                              \
   cieTrace                    \
   (                           \
      dmgr.traceTable,         \
      *(ulong *)(#tag),        \
      (ulong)(a),              \
      (ulong)(b),              \
      (ulong)(c)               \
   ),                          \
   TRCHKGT                     \
   (                           \
      HKWD_CIOEM_RECV,         \
      *(ulong *)(#tag),        \
      (ulong)(a),              \
      (ulong)(b),              \
      (ulong)(c),              \
      (ulong)(0)               \
   )                           \
)

/*---------------------------------------------------------------------------*/
/*                       Trace macro for other events                        */
/*---------------------------------------------------------------------------*/

#define  TRC_OTHER(tag,a,b,c)  \
(                              \
   cieTrace                    \
   (                           \
      dmgr.traceTable,         \
      *(ulong *)(#tag),        \
      (ulong)(a),              \
      (ulong)(b),              \
      (ulong)(c)               \
   ),                          \
   TRCHKGT                     \
   (                           \
      HKWD_CIOEM_OTHER,        \
      *(ulong *)(#tag),        \
      (ulong)(a),              \
      (ulong)(b),              \
      (ulong)(c),              \
      (ulong)(0)               \
   )                           \
)

/*---------------------------------------------------------------------------*/
/*                             Debug Trace macro                             */
/*---------------------------------------------------------------------------*/

#if defined(DEBUG)

#define  TRC_DEBUG(tag,a,b,c)  \
(                              \
   cieTrace                    \
   (                           \
      dmgr.traceTable,         \
      *(ulong *)(#tag),        \
      (ulong)(a),              \
      (ulong)(b),              \
      (ulong)(c)               \
   ),                          \
   cieTrace                    \
   (                           \
      dmgr.traceTable,         \
      *(ulong *)(#tag),        \
      (ulong)(__FILE__),       \
      (ulong)(__FUNC__),       \
      (ulong)(__LINE__)        \
   )                           \
)

#else

#define  TRC_DEBUG(tag,a,b,c)

#endif

/*---------------------------------------------------------------------------*/
/*                          Trace Table Definition                           */
/*---------------------------------------------------------------------------*/

typedef struct TRACE_TABLE   TRACE_TABLE;
typedef struct TRACE_ENTRY   TRACE_ENTRY;

struct TRACE_ENTRY
{
   ulong                     traceID;     // Trace ID
   ulong                     arg1;        // Trace Data Word 1
   ulong                     arg2;        // Trace Data Word 2
   ulong                     arg3;        // Trace Data Word 3
};

struct TRACE_TABLE
{
   char                      iCatcher[16]; // Eye Catcher
   TRACE_ENTRY             * last;         // Ptr to last trace entry + 1
   TRACE_ENTRY             * next;         // Ptr to next trace entry
   Simple_lock               lock;         // Trace Table Lock
   ulong                     res1;         // Reserved
   TRACE_ENTRY               table[1];     // Trace Table
};

/*---------------------------------------------------------------------------*/
/*           Generate the current function name as a static string           */
/*---------------------------------------------------------------------------*/

#define  FUNC_NAME(x) static char * __FUNC__ =#x

/*---------------------------------------------------------------------------*/
/*                        Allocate a new Trace Table                         */
/*---------------------------------------------------------------------------*/

TRACE_TABLE *
   new_TRACE_TABLE(
      register int           tSize        // I -Size of trace table
   );

/*---------------------------------------------------------------------------*/
/*                           Delete a Trace Table                            */
/*---------------------------------------------------------------------------*/

void
   free_TRACE_TABLE(
      register TRACE_TABLE * tt           // IO-Trace Table
   );

#if defined(INLINE)

#include "trccap.inl"

#else

/*---------------------------------------------------------------------------*/
/*                           Record a Trace Entry                            */
/*---------------------------------------------------------------------------*/

void
   cieTrace(
      register TRACE_TABLE * tt          ,// IO-Trace Table
      register ulong         traceID     ,// I -Trace ID
      register ulong         arg1        ,// I -First Trace Data Word
      register ulong         arg2        ,// I -Second Trace Data Word
      register ulong         arg3         // I -Third Trace Data Word
   );

#endif

#endif
