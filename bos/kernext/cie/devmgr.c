static char sccsid[]="@(#)23   1.9  src/bos/kernext/cie/devmgr.c, sysxcie, bos411, 9438B411a 9/20/94 14:17:09";

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 *   init_DEVMGR
 *   uninit_DEVMGR
 *   createDevice
 *   destroyDevice
 *   ACQUIRE_DEVICE
 *   acquireDevice
 *   releaseDevice
 *   acquireChannel
 *   releaseChannel
 *
 * DESCRIPTION:
 *
 *    Device Manager Implementation
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

#include "ciedd.h"
#include "devmgr.h"
#include "dev.h"
#include "chn.h"
#include "chnlist.h"
#include "devtable.h"

#include <sys/ldr.h>
#include <sys/pin.h>
#include <sys/sleep.h>
#include <sys/device.h>
#include <sys/uio.h>

#include "dmalloc.h"

/*---------------------------------------------------------------------------*/
/*                   Initialize the DEVMGR Data Structure                    */
/*---------------------------------------------------------------------------*/

int
   init_DEVMGR(
      void
   )
{
   FUNC_NAME(init_DEVMGR);

   int                       i;
   int                       rc = 0;

   /*------------------------------*/
   /*  Initialize the Trace Table  */
   /*------------------------------*/

   if ((dmgr.traceTable = new_TRACE_TABLE(1024)) == NULL)
      return ENOMEM;

   /*----------------------------------*/
   /*  Initialize the scalar elements  */
   /*----------------------------------*/

   dmgr.chnIndex   = NULL;
   dmgr.devTable   = NULL;
   memset(&dmgr.dataLock,0x00,sizeof(dmgr.dataLock ));

   /*-------------------------------------------------------*/
   /*  Sort the IOCTL dispatch tables by IOCTL Code Number  */
   /*-------------------------------------------------------*/

   for (i=0; (dmgr.devTypeData[i].iodtSize != -1); i++)
   {
      sortIOCTLdefTable(dmgr.devTypeData[i].iodTable,  /* Table Address     */
                        dmgr.devTypeData[i].iodtSize); /* Number of entries */
   }

   /*-------------------------------*/
   /*  Initialize the Device Table  */
   /*-------------------------------*/

   if ((dmgr.devTable = new_DEVTABLE()) == NULL)
      return ENOMEM;

   /*----------------------------------------------------------*/
   /*  Allocate and initialize the Master Data Structure Lock  */
   /*----------------------------------------------------------*/

   lock_alloc(&dmgr.dataLock,
              LOCK_ALLOC_PIN,
              CIO_LOCK_CLASS,
              CIE_LOCK_DATA);

   lock_init(&dmgr.dataLock,TRUE);

   dmgr.initState = CIE_INITIALIZED;

#if defined(DMALLOC)
{
   extern void * arena;
   dmgr.memDebug = arena;
}
#endif

   return 0;
}

/*---------------------------------------------------------------------------*/
/*                  Un-initialize the DEVMGR data structure                  */
/*---------------------------------------------------------------------------*/

void
   uninit_DEVMGR(
      void
   )
{
   free_DEVTABLE(dmgr.devTable);
   dmgr.devTable   = NULL;

   free_TRACE_TABLE(dmgr.traceTable);
   dmgr.traceTable = NULL;

   lock_free(&dmgr.dataLock);

   dmgr.initState = CIE_NOT_INITIALIZED;
}

/*---------------------------------------------------------------------------*/
/*                            Create a new Device                            */
/*---------------------------------------------------------------------------*/

int
   createDevice(
      dev_t                  devno       ,// I -Device identifier
      uio_t                * uiop         // I -Device Dependent Structure
   )
{
   FUNC_NAME(createDevice);

   DEV                     * dev;         // Device Instance Block
   DDS                       ddsTemp;     // Device Dependent Structure
   DEVHANDLE                 dh;          // Device Handle
   int                       rc = 0;

   /*-----------------------------------------*/
   /*  Copy DDS from uio to local stack temp  */
   /*-----------------------------------------*/

   if ((rc = uiomove((caddr_t)(&ddsTemp),
                     sizeof(DDS),
                     UIO_WRITE,
                     uiop)) != 0)
   {
      TRC_OTHER(cdvv,rc,&ddsTemp,uiop);
      return rc;
   }

   /*-------------------------------------------------------*/
   /*  Convert device number to device handle and validate  */
   /*-------------------------------------------------------*/

   if ((rc = mkDevHandle(dmgr.devTable,devno,&dh)) != 0)
      return rc;

   /*-------------------------------------------------*/
   /*  Check to see if this device is already active  */
   /*-------------------------------------------------*/

   if ((dev = getDEV(dmgr.devTable,dh)) != NULL)
   {
      /* Oops! Trying to configure same device again */

      TRC_OTHER(cdve,*(ulong *)(&dh),0,0);
      return EEXIST;
   }

   /*--------------------------------*/
   /*  Create Device Instance Block  */
   /*--------------------------------*/

   if ((dev = new_DEV(devno,&ddsTemp)) == NULL)
      return ENOMEM;

   /*-----------------------------------------*/
   /*  Put DEV pointer into the device table  */
   /*-----------------------------------------*/

   if ((rc=setDEV(dmgr.devTable,dh,dev)) != 0)
   {
      TRC_OTHER(cdvs,rc,0,0);
      free_DEV(dev);
      return rc;
   }

   /*------------------------------------------------*/
   /*  Increment device count for this major number  */
   /*------------------------------------------------*/

   if (incrDevCount(dmgr.devTable,dh) == 1)
   {
      /*--------------------------------------------*/
      /*  Add our entry to the device switch table  */
      /*--------------------------------------------*/

      if ((rc = devswadd(devno,dmgr.devswEntry)) != 0)
      {
         TRC_OTHER(cdvx,rc,devno,0);
         decrDevCount(dmgr.devTable,dh);
         clrDEV(dmgr.devTable,dh);
         free_DEV(dev);
         return rc;
      }
   }

   return 0;
}

/*---------------------------------------------------------------------------*/
/*                             Destroy a Device                              */
/*---------------------------------------------------------------------------*/

int
   destroyDevice(
      dev_t                  devno        // I -Device identifier
   )
{
   FUNC_NAME(destroyDevice);

   DEV                     * dev;
   DEVHANDLE                 dh;
   int                       rc = 0;

   /*-------------------------------------------------------*/
   /*  Convert device number to device handle and validate  */
   /*-------------------------------------------------------*/

   if ((rc = mkDevHandle(dmgr.devTable,devno,&dh)) != 0)
      return rc;

   /*----------------------------*/
   /*  Retrieve the DEV pointer  */
   /*----------------------------*/

   if ((dev = getDEV(dmgr.devTable,dh)) == NULL)
   {
      TRC_OTHER(ddvn,devno,0,0);
      return ENODEV;
   }

   /*------------------------------------*/
   /*  Further validation (active, etc)  */
   /*------------------------------------*/

   if (dev->nchOpen != 0)
   {
      TRC_OTHER(ddvb,devno,0,0);
      return EBUSY;
   }

   /*------------------------------------------------*/
   /*  Remove the DEV pointer from the device table  */
   /*------------------------------------------------*/

   clrDEV(dmgr.devTable,dh);

   /*------------------------------------------------*/
   /*  Decrement device count for this major number  */
   /*------------------------------------------------*/

   if (decrDevCount(dmgr.devTable,dh) == 0)
   {
      /*------------------------------------------*/
      /*  Remove our entry from the switch table  */
      /*------------------------------------------*/

      devswdel(devno);
   }

   /*--------------------------------------------*/
   /*  Free storage associated with this device  */
   /*--------------------------------------------*/

   free_DEV(dev);

   return 0;
}

/*---------------------------------------------------------------------------*/
/*                 Defined as a macro to improve performance                 */
/*---------------------------------------------------------------------------*/

#define  ACQUIRE_DEVICE(devno,dev)                                \
{                                                                 \
   DEVHANDLE                 dh;                                  \
   int                       rc = 0;                              \
                                                                  \
   /*--------------------------------*/                           \
   /*  Acquire the Global Data Lock  */                           \
   /*--------------------------------*/                           \
                                                                  \
   lock_read(&dmgr.dataLock);                                     \
                                                                  \
   /*-------------------------------------------------------*/    \
   /*  Convert device number to device handle and validate  */    \
   /*-------------------------------------------------------*/    \
                                                                  \
   if ((rc = mkDevHandle(dmgr.devTable,devno,&dh)) != 0)          \
   {                                                              \
      TRC_OTHER(aqdh,rc,0,0);                                     \
      lock_done(&dmgr.dataLock);                                  \
      return rc;                                                  \
   }                                                              \
                                                                  \
   /*----------------------------*/                               \
   /*  Retrieve the DEV pointer  */                               \
   /*----------------------------*/                               \
                                                                  \
   if ((dev = getDEV(dmgr.devTable,dh)) == NULL)                  \
   {                                                              \
      TRC_OTHER(aqdg,rc,0,0);                                     \
      lock_done(&dmgr.dataLock);                                  \
      return ENODEV;                                              \
   }                                                              \
                                                                  \
   /*-------------------*/                                        \
   /*  Lock the device  */                                        \
   /*-------------------*/                                        \
                                                                  \
   simple_lock(&dev->lock);                                       \
}

/*---------------------------------------------------------------------------*/
/*           Acquire temporary 'ownership' of the specified device           */
/*---------------------------------------------------------------------------*/

int
   acquireDevice(
      register dev_t         devno       ,// I -Device Number
      register DEV        ** pdev         //  O-Return area for dev
   )
{
   FUNC_NAME(acquireDevice);

   register DEV            * dev;

   ACQUIRE_DEVICE(devno,dev);

   *pdev = dev;

   return 0;
}

/*---------------------------------------------------------------------------*/
/*                 Release ownership of the specified device                 */
/*---------------------------------------------------------------------------*/

void
   releaseDevice(
      register DEV         * dev          // IO-The device
   )
{
   FUNC_NAME(releaseDevice);

   simple_unlock(&dev->lock);
   lock_done(&dmgr.dataLock);

   return;
}

/*---------------------------------------------------------------------------*/
/*          Acquire temporary 'ownership' of the specified channel           */
/*---------------------------------------------------------------------------*/

int
   acquireChannel(
      register dev_t         devno       ,// I -Device Number
      register chan_t        chan        ,// I -Channel Number
      register DEV        ** pdev        ,//  O-Ptr to dev return
      register CHN        ** pchn         //  O-Ptr to chn return
   )
{
   FUNC_NAME(acquireChannel);

   register DEV            * dev;
   register const CHN_LIST * chl;
   register CHN            * chn;
   int                       rc = 0;

   /*---------------------------------------------*/
   /*  Acquire exclusive ownership of the device  */
   /*---------------------------------------------*/

   ACQUIRE_DEVICE(devno,dev);

   /*---------------------------*/
   /*  Now acquire the channel  */
   /*---------------------------*/

   chl = dev->chnList;

   if ((chn = (chan < chl->size) ? (*chl->table)[chan].chn : NULL) == NULL)
   {
      TRC_OTHER(aqch,devno,chan,0);
      releaseDevice(dev);
      return ECHRNG;
   }

   /*------------------------------------------*/
   /*  Return the channel and device pointers  */
   /*------------------------------------------*/

   *pdev = dev;
   *pchn = chn;

   return 0;
}

/*---------------------------------------------------------------------------*/
/*                Release ownership of the specified channel                 */
/*---------------------------------------------------------------------------*/

void
   releaseChannel(
      register DEV         * dev         ,// IO-The Device
      register CHN         * chn          // IO-The channel
   )
{
   FUNC_NAME(releaseChannel);

   simple_unlock(&dev->lock);
   lock_done(&dmgr.dataLock);
}
