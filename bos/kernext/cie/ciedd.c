static char sccsid[]="@(#)18   1.12  src/bos/kernext/cie/ciedd.c, sysxcie, bos411, 9438B411a 9/20/94 14:16:39";

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 *   cieconfig
 *   ciempx
 *   cieopen
 *   cieclose
 *   cieread
 *   ciewrite
 *   cieselect
 *   cieioctl
 *
 * DESCRIPTION:
 *
 *    COMIO Emulator Filesystem Interface
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

#include <sys/cdli.h>
#include <sys/ndd.h>
#include <sys/comio.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/ciouser.h>
#include <sys/err_rec.h>
#include <sys/poll.h>
#include <sys/uio.h>
#include <sys/mbuf.h>

#include "devmgr.h"
#include "devtable.h"
#include "dev.h"
#include "chn.h"
#include "cieutil.h"
#include "nsdmx.h"
#include "status.h"
#include "dmalloc.h"
#include "mbqueue.inl"

/*---------------------------------------------------------------------------*/
/*                     Device Configuration Entry Point                      */
/*---------------------------------------------------------------------------*/

int
   cieconfig(
      dev_t                  devno,       // I - Device Number
      int                    cmd,         // I - Command Code
      uio_t                * uiop         // I - Ptr to uio containing dds
   )
{
   FUNC_NAME(cieconfig);

   static volatile int       configLock = 0; // Serializes config code

   int                       oldValue   = 0;
   int                       rc         = 0;
   pid_t                     curpid     = getpid();

   TRC_OTHER(CFGA,devno,cmd,uiop);

   /*-----------------------------------*/
   /*  Acquire Configuration Code Lock  */
   /*-----------------------------------*/

   while(!compare_and_swap(&configLock,&oldValue,curpid))
   {
      delay(HZ/4);
      oldValue = 0;
   }

   /*-------------------------------------------------------*/
   /*  Initialize the COMIO Emulator if first time through  */
   /*-------------------------------------------------------*/

   if (dmgr.initState != CIE_INITIALIZED)
   {
      TRC_OTHER(INIT,0,0,0);

      /*---------------------------------------*/
      /*  Initialize the debug logging system  */
      /*---------------------------------------*/

      dbginit();

      /*dbgout("\n\n\nCOMIO Emulator Initialization");
      dbgout(dmgr.compTime);*/

      /*------------------------------------------------------------*/
      /*  Pin our code and static data so we won't get page faults  */
      /*------------------------------------------------------------*/

      if ((rc = pincode(cieconfig)) != 0)
      {
         TRC_OTHER(pin!,rc,0,0);
         /*dbgout("Could not pin the COMIO emulator - rc=%d",rc);*/
         return rc;
      }

#if defined(DMALLOC)
      /*--------------------------------------*/
      /*  Initialize debug memory allocation  */
      /*--------------------------------------*/

      dmallocInit(4096,16384);
#endif

      /*-----------------------------------------------*/
      /*  Initialize the Global Anchor Data Structure  */
      /*-----------------------------------------------*/

      if ((rc = init_DEVMGR()) != 0)
      {
         TRC_OTHER(ini!,rc,0,0);
         /*dbgout("COMIO Emulator initialization failed - rc=%d",rc);*/
#if defined(DMALLOC)
         dmallocReport();
         dmallocTerm();
#endif
         unpincode(cieconfig);
         dbgterm();
         return rc;
      }
   }

   /*-------------------------------------*/
   /*  Execute The Configuration Command  */
   /*-------------------------------------*/

   switch(cmd)
   {
      /*-----------------------*/
      /*  Initialize a Device  */
      /*-----------------------*/

      case CFG_INIT:
      {
         TRC_OTHER(CFGI,devno,cmd,uiop);

         if (uiop == NULL)
         {
            TRC_OTHER(cfgu,0,0,0);
            rc = EFAULT;
         }
         else if (uiop->uio_resid != sizeof(DDS))
         {
            TRC_OTHER(cfgv,uiop->uio_resid,sizeof(DDS),0);
            rc = EFAULT;
         }
         else
         {
            lock_write(&dmgr.dataLock);
            rc = createDevice(devno,uiop);
            lock_done(&dmgr.dataLock);
         }

         break;
      }

      /*----------------------*/
      /*  Terminate a Device  */
      /*----------------------*/

      case CFG_TERM:
      {
         TRC_OTHER(CFGT,devno,cmd,0);

         lock_write(&dmgr.dataLock);
         rc = destroyDevice(devno);
         lock_done(&dmgr.dataLock);

         break;
      }

      /*----------------------------*/
      /*  Query Vital Product Data  */
      /*----------------------------*/

      case CFG_QVPD:
      {
         TRC_OTHER(CFGV,devno,cmd,0);

         if (uiop == NULL)
         {
            rc = EFAULT;
            TRC_OTHER(qvpu,0,0,0);
         }
         else
         {
            DEV            * dev;

            if ((rc = acquireDevice(devno,&dev)) == 0)
            {
               switch(dev->dds.devType)
               {
                  case CIE_DEV_TOK:
                     rc = uiomove(&dev->dds.ds.tok.vpd,
                                  sizeof(tok_vpd_t),
                                  UIO_READ,
                                  uiop);
                     break;

                  case CIE_DEV_ENT:
                     rc = uiomove(&dev->dds.ds.ent.vpd,
                                  sizeof(ccc_vpd_blk_t),
                                  UIO_READ,
                                  uiop);
                     break;

                  case CIE_DEV_FDDI:
                     rc = uiomove(&dev->dds.ds.fddi.vpd,
                                  sizeof(fddi_vpd_t),
                                  UIO_READ,
                                  uiop);
                     break;
               }

               if (rc) TRC_OTHER(qvpd,rc,dev,0);

               releaseDevice(dev);
            }
            else
               TRC_OTHER(qvpz,rc,0,0);
         }

         break;
      }

      /*-------------------------------*/
      /*  Unknown Config Command Code  */
      /*-------------------------------*/

      default:
      {
         TRC_OTHER(cfg!,cmd,0,0);
         rc = EINVAL;
         break;
      }
   }

   /*----------------------------------------*/
   /*  Last person out turns off the lights  */
   /*----------------------------------------*/

   if (dmgr.devTable->devCount == 0)
   {
      TRC_OTHER(TERM,0,0,0);

      /*----------------------------*/
      /*  Reset the Device Manager  */
      /*----------------------------*/

      uninit_DEVMGR();

#if defined(DMALLOC)
      /*----------------------------------------*/
      /*  Check for memory remaining allocated  */
      /*----------------------------------------*/

      dmallocReport();
      dmallocTerm();
#endif

      /*-------------------*/
      /*  Unpin ourselves  */
      /*-------------------*/

      unpincode(cieconfig);

      dbgterm();
   }

   /*-------------------------------------*/
   /*  Release Configuration Code "Lock"  */
   /*-------------------------------------*/

   configLock = 0;

   TRC_OTHER(CFGZ,0,0,0);

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                 Multiplex Channel Assignment Entry Point                  */
/*---------------------------------------------------------------------------*/

int
   ciempx(
      dev_t                  devno       ,// I -Device Major/Minor Number
      chan_t               * chanp       ,// IO-Ptr to Channel Number
      const char           * channame     // I -Channel Name
   )
{
   FUNC_NAME(ciempx);

   DEV                     * dev;
   CHN                     * chn;

   int                       rc = 0;

   TRC_OTHER(MPXA,devno,chanp,channame);

   if (chanp == NULL) return EFAULT;

   /*--------------------------------*/
   /*  Find the DEV for this device  */
   /*--------------------------------*/

   if ((rc = acquireDevice(devno,&dev)) != 0)
   {
      TRC_OTHER(mpxa,rc,0,0);
      return rc;
   }

   /*------------------------------------------------------------*/
   /*  Either create or destroy the channel, depending on parms  */
   /*------------------------------------------------------------*/

   if (channame != NULL)
   {
      rc = createChannel(dev,chanp);
      if (rc) TRC_OTHER(mpxc,rc,0,0);
   }
   else
   {
      rc = destroyChannel(dev,chanp);
      if (rc) TRC_OTHER(mpxd,rc,0,0);
   }

   /*-----------------*/
   /*  Release locks  */
   /*-----------------*/

   releaseDevice(dev);

   TRC_OTHER(MPXZ,0,0,0);

   return 0;
}

/*---------------------------------------------------------------------------*/
/*                          Device Open Entry Point                          */
/*---------------------------------------------------------------------------*/

int
   cieopen(
      dev_t                  devno       ,// I -Device major/minor number
      ulong                  devflag     ,// I -Device Flags
      chan_t                 chan        ,// I -Channel Number
      void                 * ext          // IO-Open Extension
   )
{
   FUNC_NAME(cieopen);

   DEV                     * dev;
   CHN                     * chn;

   int                       rc = 0;

   TRC_OTHER(OPNA,devno,devflag,chan);

   if ((devflag & DKERNEL) || (ext != NULL))
   {
      TRC_OTHER(opnk,ext,0,0);
      return EACCES;
   }

   /*------------------------------------*/
   /*  Locate the Channel Control Block  */
   /*------------------------------------*/

   if ((rc = acquireChannel(devno,chan,&dev,&chn)) != 0)
   {
      TRC_OTHER(opna,rc,0,0);
      return rc;
   }

   /*--------------------*/
   /*  Open the channel  */
   /*--------------------*/

   rc = openChannel(dev,chn,devflag);

   if (rc) TRC_OTHER(opnc,rc,0,0);

   /*----------------------*/
   /*  Release the device  */
   /*----------------------*/

   releaseChannel(dev,chn);

   TRC_OTHER(OPNZ,0,0,0);

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                         Device Close Entry Point                          */
/*---------------------------------------------------------------------------*/

int
   cieclose(
      dev_t                  devno       ,// I -Device major/minor number
      chan_t                 chan        ,// I -Channel Number
      void                 * ext          // IO-Open Extension
   )
{
   FUNC_NAME(cieclose);

   DEV                     * dev;
   CHN                     * chn;

   int                       rc = 0;

   TRC_OTHER(CLSA,devno,chan,ext);

   if (ext != NULL)
   {
      TRC_OTHER(clsk,0,0,0);
      return EACCES;
   }

   /*------------------------------------*/
   /*  Locate the Channel Control Block  */
   /*------------------------------------*/

   if ((rc = acquireChannel(devno,chan,&dev,&chn)) != 0)
   {
      TRC_OTHER(clsa,rc,0,0);
      return rc;
   }

   /*---------------------*/
   /*  Close the channel  */
   /*---------------------*/

   rc = closeChannel(dev,chn);

   if (rc) TRC_OTHER(clsc,rc,0,0);

   /*----------------------*/
   /*  Release the device  */
   /*----------------------*/

   releaseChannel(dev,chn);

   TRC_OTHER(CLSZ,0,0,0);

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                          Device Read Entry Point                          */
/*---------------------------------------------------------------------------*/

int
   cieread(
      dev_t                  devno       ,// I -Device Major/Minor Number
      uio_t                * uiop        ,// IO-Data to be written
      chan_t                 chan        ,// I -Channel number
      cio_read_ext_t       * ext          // IO-Read Extension
   )
{
   FUNC_NAME(cieread);

   DEV                     * dev;         // Device
   CHN                     * chn;         // Channel
   cio_read_ext_t            lclExt;      // Local copy of extension block
   mbuf_t                  * m;           // mbuf pointer
   int                       rc = 0;

   TRC_RECV(RCVA,devno,uiop,chan);

   if (uiop == NULL)
   {
      TRC_RECV(rcvu,0,0,0);
      return EFAULT;
   }

   /*------------------------------------*/
   /*  Initialize local extension block  */
   /*------------------------------------*/

   memset((caddr_t)&lclExt,0,sizeof(lclExt));

   /*------------------------------------*/
   /*  Locate the Channel Control Block  */
   /*------------------------------------*/

   if ((rc = acquireChannel(devno,chan,&dev,&chn)) != 0)
   {
      TRC_RECV(rcva,rc,0,0);
      return rc;
   }

   /*--------------------------------------------------------------------*/
   /*  If there is at least one session open, read the next input block  */
   /*--------------------------------------------------------------------*/

   if (!qEmpty(chn->sesQueue))
   {
      if ((rc = readData(dev,chn,&m)) == 0)
      {
         if (m == NULL)
            TRC_RECV(RCVN,0,0,0);
         else
         {
            /*---------------------------------*/
            /*  Copy the data to the uio area  */
            /*---------------------------------*/

            rc = mbufToUio(dev->dds.devType,uiop,&m);

            if (rc) TRC_RECV(rcvv,rc,0,0);

            if (ext && rc==EMSGSIZE)
            {
               TRC_RECV(rcvo,0,0,0);
               lclExt.status = CIO_BUF_OVFLW;
               rc            = 0;
            }
         }
      }
      else
         TRC_RECV(RCVR,rc,0,0);
   }
   else
   {
      TRC_RECV(rcvs,0,0,0);
      lclExt.status = CIO_NOT_STARTED;
      rc = EIO;
   }

   /*-----------------------*/
   /*  Release the Channel  */
   /*-----------------------*/

   releaseChannel(dev,chn);

   /*--------------------------------------*/
   /*  Return extension to user if needed  */
   /*--------------------------------------*/

   if (ext)
   {
      int                    xrc;

      if ((xrc = copyout((caddr_t)&lclExt,(caddr_t)ext,sizeof(ext))) != 0)
      {
         TRC_RECV(rcvb,xrc,0,0);
         return EFAULT;
      }
   }

   TRC_RECV(RCVZ,0,0,0);

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                         Device Write Entry Point                          */
/*---------------------------------------------------------------------------*/

int
   ciewrite(
      dev_t                  devno       ,// I -Device Major/Minor Number
      uio_t                * uiop        ,// IO-Data to be written
      chan_t                 chan        ,// I -Channel number
      cio_write_ext_t      * ext          // IO-Write Extension
   )
{
   FUNC_NAME(ciewrite);

   DEV                     * dev;         // Device
   CHN                     * chn;         // Channel
   cio_write_ext_t           lclExt;      // Local copy of extension
   mbuf_t                  * m;           // mbuf pointer
   int                       rc  = 0;
   int                       xrc = 0;

   TRC_XMIT(XMTA,devno,uiop,chan);

   if (uiop == NULL)
   {
      TRC_XMIT(xmtu,0,0,0);
      return EFAULT;
   }

   /*------------------------------------*/
   /*  Initialize local extension block  */
   /*------------------------------------*/

   if (ext)
   {
      if ((xrc = copyin((caddr_t)ext,(caddr_t)&lclExt,sizeof(lclExt))) != 0)
      {
         TRC_XMIT(xmta,xrc,0,0);
         return EFAULT;
      }
   }
   else
      memset(&lclExt,0,sizeof(lclExt));

   /*------------------------------------*/
   /*  Locate the Channel Control Block  */
   /*------------------------------------*/

   if ((rc = acquireChannel(devno,chan,&dev,&chn)) != 0)
   {
      TRC_XMIT(xmtc,rc,0,0);
      return rc;
   }

   /*------------------------------------------------*/
   /*  Make sure there is at least one open session  */
   /*------------------------------------------------*/

   if (qEmpty(chn->sesQueue))
   {
      lclExt.status = CIO_NOT_STARTED;
      rc = EIO;
      TRC_XMIT(xmts,0,0,0);
   }
   else
   {
      /*----------------------------------------------------------*/
      /*  Build an mbuf chain containing the contents of the uio  */
      /*----------------------------------------------------------*/

      if ((rc = uioToMbuf(&m,uiop)) == 0)
      {
         /*------------------------*/
         /*  Write the data block  */
         /*------------------------*/

         if ((rc = writeData(dev,chn,m)) == 0)
         {
            /*-------------------------------------------------*/
            /*  Queue Transmit Done Acknowledgement if needed  */
            /*-------------------------------------------------*/

            if (ext->flag & CIO_ACK_TX_DONE)
            {
               STATBLK     * s;
               int           pSave;       // Interrupt Priority Save Area

               TRC_XMIT(XMTD,0,0,0);

               s = mkTXDoneStatus(CIO_OK,ext->write_id);

               lockChannel(pSave,chn);
               queueStatus(chn,s);
               unlockChannel(pSave,chn);
            }
         }
         else
            TRC_XMIT(xmtw,rc,m,0);
      }
      else
         TRC_XMIT(xmtv,rc,0,0);
   }

   /*----------------------*/
   /*  Release the Channel */
   /*----------------------*/

   releaseChannel(dev,chn);

   /*--------------------------------------------*/
   /*  Return the updated extension to the user  */
   /*--------------------------------------------*/

   if (ext)
   {
      int                    xrc;

      if ((xrc = copyout((caddr_t)&lclExt,(caddr_t)ext,sizeof(lclExt))) != 0)
      {
         TRC_XMIT(xmtb,rc,0,0);
         return EFAULT;
      }
   }

   TRC_XMIT(XMTZ,rc,0,0);

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                            Select Entry Point                             */
/*---------------------------------------------------------------------------*/

int
   cieselect(
      dev_t                  devno       ,// I -Device Major/Minor Number
      ushort                 events      ,// I -Event mask to be tested
      ushort               * revents     ,// O -Events that actually occurred
      chan_t                 chan         // I -Channel number
   )
{
   FUNC_NAME(cieselect);

   DEV                     * dev;
   CHN                     * chn;
   register mbuf_t         * m;

   int                       pSave;       //Interrupt Priority Save
   int                       rc = 0;

   TRC_OTHER(SELA,devno,events,chan);

   if (revents == NULL)
   {
      TRC_OTHER(selr,0,0,0);
      return EFAULT;
   }

   /*------------------------------------*/
   /*  Locate the Channel Control Block  */
   /*------------------------------------*/

   if ((rc = acquireChannel(devno,chan,&dev,&chn)) != 0)
   {
      TRC_OTHER(sela,rc,0,0);
      return rc;
   }

   /*-----------*/
   /*  Lock it  */
   /*-----------*/

   lockChannel(pSave,chn);

   /*---------------------------------*/
   /*  Check for data in input queue  */
   /*---------------------------------*/

   *revents |= !mbqEmpty(&chn->rdq) ? (events & POLLIN)  : 0;

   /*------------------------------------*/
   /*  Check for status in status queue  */
   /*------------------------------------*/

   *revents |= !mbqEmpty(&chn->stq) ? (events & POLLPRI) : 0;

   /*-----------------------------*/
   /*  Output is always possible  */
   /*-----------------------------*/

   *revents |= (events & POLLOUT);

   /*----------------------*/
   /*  Unlock the channel  */
   /*----------------------*/

   unlockChannel(pSave,chn);

   /*----------------------*/
   /*  Release the device  */
   /*----------------------*/

   releaseChannel(dev,chn);

   TRC_OTHER(SELZ,0,*revents,0);

   return 0;
}

/*---------------------------------------------------------------------------*/
/*                             IOCTL Entry Point                             */
/*---------------------------------------------------------------------------*/

int
   cieioctl(
      dev_t                  devno       ,// I -Device Number
      int                    cmd         ,// I -IOCTL Command Code
      void                 * arg         ,// IO-IOCTL Paramater Block
      ulong                  devflag     ,// I -Device Flags from OPEN
      int                    chan        ,// I -Channel Number
      void                 * ext          // IO-IOCTL Extension Block
   )
{
   FUNC_NAME(cieioctl);

   DEV                     * dev;
   CHN                     * chn;

   IODT_ENTRY                searchKey;
   IODT_ENTRY              * iodte;

   int                       rc = 0;

   TRC_OTHER(IOCA,devno,cmd,arg);
   TRC_OTHER(IOCB,devflag,chan,ext);

   if (arg == NULL)
   {
      TRC_OTHER(iocg,0,0,0);
      return EFAULT;
   }

   /*------------------------------------*/
   /*  Locate the Channel Control Block  */
   /*------------------------------------*/

   if ((rc = acquireChannel(devno,chan,&dev,&chn)) != 0)
   {
      TRC_OTHER(ioca,rc,0,0);
      return rc;
   }

   /*---------------------------------------------*/
   /*  Find the IOD_TABLE entry for this command  */
   /*---------------------------------------------*/

   searchKey.cmd = cmd;

   iodte = searchIODTable(
              cmd,
              dev->iodTable,
              dev->iodtSize
           );

   if (iodte == NULL)
   {
      TRC_OTHER(iocc,0,0,0);
      rc = EINVAL;
   }
   else if(iodte->handler == NULL)
   {
      TRC_OTHER(IOCN,0,0,0);
      rc = ENOSYS;
   }
   else
   {
      /*-----------------------------------------------*/
      /*  Call the appropriate IOCTL handler function  */
      /*-----------------------------------------------*/

      rc = (iodte->handler)(
                 dev,
                 chn,
                 arg,
                 devflag
           );

      if (rc) TRC_OTHER(ioch,rc,0,0);
   }

   /*-----------------------*/
   /*  Release the channel  */
   /*-----------------------*/

   releaseChannel(dev,chn);

   TRC_OTHER(IOCZ,0,0,0);

   return rc;
}
