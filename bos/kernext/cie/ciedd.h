/* @(#)40   1.7  src/bos/kernext/cie/ciedd.h, sysxcie, bos411, 9428A410j 4/18/94 16:20:52 */

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 *   arraysize
 *   min
 *   max
 *
 * DESCRIPTION:
 *
 *    COMIO Emulator Global Declarations
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

#if !defined(CIEDD_H)
#define      CIEDD_H

typedef struct mbuf          mbuf_t;
typedef struct uio           uio_t;
typedef struct devsw         devsw_t;

/*---------------------------------------------------------------------------*/
/*                         Standard System Includes                          */
/*---------------------------------------------------------------------------*/

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/mbuf.h>

#include <stddef.h>
#include <fcntl.h>
#include <string.h>
#include <sys/m_param.h>
#include <sys/intr.h>
#include <sys/time.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/limits.h>
#include <sys/sleep.h>

#include <sys/malloc.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#include <sys/atomic_op.h>

/*---------------------------------------------------------------------------*/
/*                         Implementation Constants                          */
/*---------------------------------------------------------------------------*/

#define  CIE_DV_DR_NAME               "ciedd"
#define  CIE_MAX_NETID                    257
#define  CHN_INDEX_SIZE                   256
#define  ENT_MAX_MULTI                     10
#define  LOCK_INIT                          0
#define  FDDI_SMT_MAX_FILTERS               3

/*---------------------------------------------------------------------------*/
/*                      Complex Lock "Occurrence" Codes                      */
/*---------------------------------------------------------------------------*/

#define  CIE_LOCK_DATA       1
#define  CIE_LOCK_DEVICE     2
#define  CIE_LOCK_CHANNEL    3
#define  CIE_LOCK_QUEUE      4
#define  CIE_LOCK_TRACE      5
#define  CIE_LOCK_DEBUG      6

/*---------------------------------------------------------------------------*/
/*                          Global Type Definitions                          */
/*---------------------------------------------------------------------------*/

typedef enum   CIE_INIT_STATE  CIE_INIT_STATE;  // Anchor Initialization State

typedef struct DEVTABLE        DEVTABLE;        // Device Table
typedef volatile struct DEV    DEV;             // Device Instance
typedef volatile struct CHN    CHN;             // Channel Instance
typedef volatile struct SES    SES;             // Session Block

typedef struct CHN_INDEX       CHN_INDEX;       // Channel Index
typedef struct CHN_INDEX_ENTRY CHN_INDEX_ENTRY; // Channel Index Entry
typedef struct CHN_LIST        CHN_LIST;        // Channel List
typedef struct CHN_LIST_ENTRY  CHN_LIST_ENTRY;  // Channel List Entry

typedef struct DEVHANDLE       DEVHANDLE;       // Device Handle

typedef int                    EVENT_WORD;      // e_sleep/e_wakeup event word

/*---------------------------------------------------------------------------*/
/*                     Standard COMIO Emulator Includes                      */
/*---------------------------------------------------------------------------*/

#include "trccap.h"

#include "devtype.h"
#include "devconst.h"
#include "cieutil.h"
#include "nsdebug.h"

/*---------------------------------------------------------------------------*/
/*         Enumeration type for COMIO Emulator Initialization States         */
/*---------------------------------------------------------------------------*/

enum CIE_INIT_STATE
{
   CIE_NOT_INITIALIZED = 0,
   CIE_INITIALIZED     = 1
};

/*---------------------------------------------------------------------------*/
/*                   COMIO Emulator Data Structure Anchor                    */
/*---------------------------------------------------------------------------*/

typedef struct DEVMGR        DEVMGR;      // Device Manager

struct DEVMGR
{
   const char                iCatcher[4]; // Eye Catcher
   const char              * compTime;    // Compile Timestamp
   TRACE_TABLE             * traceTable;  // Ptr to Trace Table
   void                    * memDebug;    // Ptr to dmalloc tables
   const DEV_TYPE_DEF      * devTypeData; // Ptr to Device Type Static Data
   const devsw_t           * devswEntry;  // Ptr to Device Switch Table Entry
   CIE_INIT_STATE            initState;   // Initialization State
   DEVTABLE                * devTable;    // Device Table
   CHN_INDEX               * chnIndex;    // Channel Index
   Complex_lock              dataLock;    // Master Data Lock
};

/*---------------------------------------------------------------------------*/
/*                         Static DEVMGR Definition                          */
/*---------------------------------------------------------------------------*/

extern volatile DEVMGR       dmgr;

/*---------------------------------------------------------------------------*/
/*                             Array-size Macro                              */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

size_t
   arraysize(
      <arrayname>            x            // I -The name of the array
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  arraysize(x)        (sizeof(x)/sizeof((x)[0]))

/*---------------------------------------------------------------------------*/
/*                                 min Macro                                 */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

int
   min(
      int                    x           ,// I -First operand
      int                    x            // I -Second operand
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  min(x,y)            (((x) <= (y)) ? (x) : (y))

/*---------------------------------------------------------------------------*/
/*                                 max Macro                                 */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

int
   max(
      int                    x           ,// I -First operand
      int                    x            // I -Second operand
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  max(x,y)            (((x) >= (y)) ? (x) : (y))

#endif
