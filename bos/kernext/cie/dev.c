static char sccsid[]="@(#)22   1.9  src/bos/kernext/cie/dev.c, sysxcie, bos41B, 412_41B_sync 12/2/94 15:15:56";

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 *   devInitENT
 *   devInitFDDI
 *   new_DEV
 *   free_DEV
 *   attachChannelToDevice
 *   detachChannelFromDevice
 *   createChannel
 *   destroyChannel
 *   describeDevice
 *   queryStatistics
 *   tokQueryVPD
 *   tokQueryTokenRingInfo
 *   entQueryVPD
 *
 * DESCRIPTION:
 *
 *    Device Object Implementaiton
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
#include <sys/ioctl.h>
#include <sys/adspace.h>
#include <sys/devinfo.h>
#include <sys/comio.h>
#include <sys/ciouser.h>
#include <sys/err_rec.h>
#include <sys/tokuser.h>
#include <sys/entuser.h>
#include <sys/fddiuser.h>
#include <sys/ndd.h>
#include <sys/cdli.h>
#include <sys/cdli_tokuser.h>
#include <sys/cdli_entuser.h>
#include <sys/cdli_fddiuser.h>
#include <sys/tok_demux.h>
#include <sys/eth_demux.h>
#include <sys/tokenring_mibs.h>

#define  fddi_hdr cfddi_dmxhdr
#include <sys/fddi_demux.h>
#undef   fddi_hdr

typedef struct tok_dmx_stats tok_dmx_stats_t;
typedef struct eth_dmx_stats eth_dmx_stats_t;
typedef struct {
        ent_ndd_stats_t ent_ndd;
        union {
                en3com_stats_t en3com;
                ient_stats_t ient;
        } ent_dev;
} ent_stable_t;

#include "chnlist.h"
#include "dev.h"
#include "chn.h"
#include "addrtbl.h"
#include "statistics.h"
#include "mapaddr.h"
#include "dmalloc.h"

/*---------------------------------------------------------------------------*/
/*             Initialize Ethernet Device-Type-Specific Section              */
/*---------------------------------------------------------------------------*/

static
int
   devInitENT(
      register DEV         * dev          // IO-Device
   )
{
   /*----------------------------------------*/
   /*  Allocate the MultiCast Address Table  */
   /*----------------------------------------*/

   dev->ds.ent.multiCastTable = new_ADDRTABLE(ENT_MAX_MULTI,ent_NADR_LENGTH);

   return (dev->ds.ent.multiCastTable == NULL);
}

/*---------------------------------------------------------------------------*/
/*               Initialize FDDI Device-Type-Specific Section                */
/*---------------------------------------------------------------------------*/

static
int
   devInitFDDI(
      register DEV         * dev          // IO-Device
   )
{
   int                       fail = 0;

   /*------------------------------------*/
   /*  Allocate the Group Address Table  */
   /*------------------------------------*/

   dev->ds.fddi.groupTable = new_ADDRTABLE(FDDI_MAX_ADDRS,FDDI_NADR_LENGTH);

   if (dev->ds.fddi.groupTable == NULL)
      fail = 1;
   else
   {
      /*-------------------------------------------------*/
      /*  Decide which SMT Filters this device will use  */
      /*-------------------------------------------------*/

      dev->ds.fddi.smtDef.numFilters = 0;

      if (dev->dds.ds.fddi.passSMT)
         dev->ds.fddi.smtDef.filter[dev->ds.fddi.smtDef.numFilters++] =
            FDDI_DEMUX_SMT;

      if (dev->dds.ds.fddi.passNSA)
         dev->ds.fddi.smtDef.filter[dev->ds.fddi.smtDef.numFilters++] =
            FDDI_DEMUX_SMT_NSA;

      if (dev->dds.ds.fddi.passBeacon)
         dev->ds.fddi.smtDef.filter[dev->ds.fddi.smtDef.numFilters++] =
            FDDI_DEMUX_MAC;
   }

   return fail;
}
/*---------------------------------------------------------------------------*/
/*                 Create a new Device Instance Block object                 */
/*---------------------------------------------------------------------------*/

DEV *
   new_DEV(
      register dev_t         devno       ,// I -Device Number
      register const DDS   * dds          // I -DDS Temporary in kernel mem
   )
{
   FUNC_NAME(new_DEV);

   register DEV            * dev;
   register CHN_LIST       * chnList;
   int                       fail = 0;

   if (dds == NULL)
   {
      TRC_OTHER(ndds,devno,0,0);
      return EFAULT;
   }

   /*------------------------*/
   /*  Validate Device Type  */
   /*------------------------*/

   if (dds->devType <= CIE_DEV_NULL || dds->devType >= CIE_DEV_MAX)
   {
      TRC_OTHER(ndvd,dds->devType,0,0);
      return ENODEV;
   }

   /*-------------------------------*/
   /*  Allocate memory for the DEV  */
   /*-------------------------------*/

   if ((dev = xmalloc(sizeof(DEV),2,pinned_heap)) == NULL)
   {
      XMFAIL(sizeof(DEV));
      return NULL;
   }

   memset(dev,0x00,sizeof(*dev));

   /*----------------------*/
   /*  Initialize the DEV  */
   /*----------------------*/

   strcpy(dev->iCatcher,"DEV");

   dev->chnList           = NULL;
   dev->nddp              = NULL;
   dev->iodTable          = NULL;
   dev->iodtSize          = 0;
   dev->devno             = devno;
   dev->nchOpen           = 0;
   dev->numSes            = 0;

   memcpy(&dev->dds,dds,sizeof(DDS));

   /*------------------------------*/
   /*  Initialize the device lock  */
   /*------------------------------*/

   lock_alloc(&dev->lock,
              LOCK_ALLOC_PIN,
              CIO_LOCK_CLASS,
              CIE_LOCK_DEVICE);

   simple_lock_init(&dev->lock);

   /*----------------------------------------*/
   /*  Allocate memory for the Channel List  */
   /*----------------------------------------*/

   if ((dev->chnList = new_CHN_LIST()) == NULL)
   {
      if (xmfree(dev,pinned_heap) != 0) XFFAIL();
      return NULL;
   }

   /*---------------------------------------------------------*/
   /*  Get IOCTL Dispatch Table address and size from anchor  */
   /*---------------------------------------------------------*/

   dev->iodTable = dmgr.devTypeData[dev->dds.devType].iodTable;
   dev->iodtSize = dmgr.devTypeData[dev->dds.devType].iodtSize;

   /*--------------------------------------*/
   /*  Initialize Device-Specific Section  */
   /*--------------------------------------*/

   switch(dds->devType)
   {
      case CIE_DEV_ENT:

         fail = devInitENT(dev);
         break;

      case CIE_DEV_FDDI:

         fail = devInitFDDI(dev);
         break;

      case CIE_DEV_TOK:

         fail = 0;
         break;
   }

   if (fail)
   {
      free_CHN_LIST(dev->chnList);
      if (xmfree(dev,pinned_heap) != 0) XFFAIL();
      return NULL;
   }

   return dev;
}

/*---------------------------------------------------------------------------*/
/*                    Free the storage occupied by a DEV                     */
/*---------------------------------------------------------------------------*/

void
   free_DEV(
      register DEV         * dev          // IO-DEV to be freed
   )
{
   FUNC_NAME(free_DEV);

   if (dev)
   {
      switch(dev->dds.devType)
      {
         case CIE_DEV_ENT:
            free_ADDRTABLE(dev->ds.ent.multiCastTable);
            break;

         case CIE_DEV_FDDI:
            free_ADDRTABLE(dev->ds.fddi.groupTable);
            break;

         case CIE_DEV_TOK:
            break;
      }

      free_CHN_LIST(dev->chnList);

      lock_free(&dev->lock);

      if (xmfree(dev,pinned_heap) != 0) XFFAIL();
   }

}

/*---------------------------------------------------------------------------*/
/*               Attach a Channel Structure to a Channel List                */
/*---------------------------------------------------------------------------*/

int
   attachChannelToDevice(
      register DEV         * dev         ,// IO-Device
      register CHN         * chn          // IO-Channel to be attached
   )
{
   FUNC_NAME(attachChannelToDevice);

   register CHN_LIST       * chl;        // Ptr to channel list
   register CHN_LIST_ENTRY * e;          // Ptr to channel list entry
   register int              chan;       // Channel Number
   register int              rc = 0;

   chl = dev->chnList;

   /*-----------------------------------------------------*/
   /*  Take the available next channel off the free list  */
   /*-----------------------------------------------------*/

   if ((chan = chl->free) == chl->size)
   {
      /*---------------------------------------------------*/
      /*  No more entries on free list -- expand the list  */
      /*---------------------------------------------------*/

      if ((rc = expand_CHN_LIST(chl,8)) != 0) return rc;

      chan = chl->free;
   }

   /*--------------------------------------------------*/
   /*  Remove new entry from free list and initialize  */
   /*--------------------------------------------------*/

   e = &((*chl->table)[chan]);

   chl->free   = e->nextFree;
   e->nextFree = -1;

   e->chn      = chn;
   chn->dev    = dev;
   chn->chan   = chan;
   chn->devno  = dev->devno;

   chl->used++;

   return 0;
}

/*---------------------------------------------------------------------------*/
/*              Detach a Channel Structure from a Channel List               */
/*---------------------------------------------------------------------------*/

void
   detachChannelFromDevice(
      register DEV         * dev         ,// IO-Device
      register CHN         * chn          // I0-Channel to be detached
   )
{
   FUNC_NAME(detachChannelFromDevice);

   register CHN_LIST       * chl;        // Ptr to channel list
   register CHN_LIST_ENTRY * e;          // Ptr to channel list entry

   chl = dev->chnList;

   /*-----------------------------------------------------*/
   /*  Make temp pointer to CHN_LIST_ENTRY for this chan  */
   /*-----------------------------------------------------*/

   e = &((*chl->table)[chn->chan]);

   /*--------------------------------------------------*/
   /*  Return the Channel List Entry to the free list  */
   /*--------------------------------------------------*/

   e->nextFree = chl->free;
   chl->free   = chn->chan;
   e->chn      = NULL;

   chl->used--;

   /*-----------------------------------------------*/
   /*  Set channel number in CHN to "null" channel  */
   /*-----------------------------------------------*/

   chn->chan   = -1;
   chn->devno  = 0;
   chn->dev    = NULL;
}

/*---------------------------------------------------------------------------*/
/*                           Create a new channel                            */
/*---------------------------------------------------------------------------*/

int
   createChannel(
      register DEV         * dev         ,// IO-The Device
      chan_t               * chanp        //  O-Output channel pointer
   )
{
   FUNC_NAME(createChannel);

   register CHN            * chn;
   int                       rc = 0;

   /*---------------------------------------*/
   /*  Validate receive/status queue sizes  */
   /*---------------------------------------*/

   if (dev->dds.recQueueSize <= 2)
   {
      TRC_OTHER(qszr,dev->dds.recQueueSize,0,0);
      return EINVAL;
   }

   if (dev->dds.staQueueSize <= 2)
   {
      TRC_OTHER(qszs,dev->dds.recQueueSize,0,0);
      return EINVAL;
   }

   /*-----------------------------------------*/
   /*  Allocate memory for the CHN Structure  */
   /*-----------------------------------------*/

   if ((chn = new_CHN(dev->dds.devType,          // Device Type Code
                      dev->dds.recQueueSize,     // Rcv Queue Size
                      dev->dds.staQueueSize)     // Status Queue Size
       ) == NULL)
      return ENOMEM;

   /*---------------------------------------*/
   /*  Attach the CHN structure to the DEV  */
   /*---------------------------------------*/

   if ((rc = attachChannelToDevice(dev,chn)) != 0)
   {
      TRC_OTHER(cchx,rc,0,0);
      free_CHN(chn);
      return rc;
   }

   *chanp = chn->chan;

   return 0;
}

/*---------------------------------------------------------------------------*/
/*                             Destroy a channel                             */
/*---------------------------------------------------------------------------*/

int
   destroyChannel(
      register DEV         * dev         ,// IO-The device
      register chan_t      * chanp        // I -The channel number
   )
{
   FUNC_NAME(destroyChannel);

   register CHN_LIST       * chl;
   register CHN            * chn;
   register chan_t           chan = *chanp;

   int                       rc = 0;

   /*----------------------*/
   /*  Locate the channel  */
   /*----------------------*/

   chl = dev->chnList;

   if ((chn = (chan < chl->size) ? (*chl->table)[chan].chn : NULL) == NULL)
   {
      TRC_OTHER(dchx,dev,chan,0);
      return ECHRNG;
   }

   /*--------------------------------------*/
   /*  Detach the channel from the Device  */
   /*--------------------------------------*/

   detachChannelFromDevice(dev,chn);

   /*-----------------------*/
   /*  Destroy the channel  */
   /*-----------------------*/

   free_CHN(chn);

   return 0;
}

/*---------------------------------------------------------------------------*/
/*                     ioctl(IOCINFO) - Describe Device                      */
/*---------------------------------------------------------------------------*/

int
   describeDevice(
      register DEV         * dev         ,// I -Device
      register CHN         * chn         ,// I -Channel
      register void        * info        ,//  O-User's devinfo buffer
      register ulong         devflag      // I -Device Flags from open
   )
{
   FUNC_NAME(describeDevice);

   struct devinfo            temp;
   int                       rc  = 0;
   int                       xrc = 0;

   memset(&temp,0x00,sizeof(temp));

   switch(dev->dds.devType)
   {
      case CIE_DEV_TOK:
      {
         temp.devtype             = DD_NET_DH;
         temp.flags               = 0;
         temp.devsubtype          = DD_TR;

         temp.un.token.rdto       = 0;
         temp.un.token.broad_wrap = ((dev->nddp->ndd_flags&NDD_SIMPLEX)==0);

         memcpy(temp.un.token.net_addr,
                dev->dds.ds.tok.curAdptAddr,
                min(sizeof(temp.un.token.net_addr),
                    sizeof(dev->dds.ds.tok.curAdptAddr)));

         memcpy(temp.un.token.haddr,
                dev->dds.ds.tok.hdwAdptAddr,
                min(sizeof(temp.un.token.haddr),
                    sizeof(dev->dds.ds.tok.hdwAdptAddr)));

         temp.un.token.speed = dev->dds.ds.tok.ringSpeed;

         break;
      }

      case CIE_DEV_ENT:
      {
         temp.devtype             = DD_NET_DH;
         temp.flags               = 0;
         temp.devsubtype          = DD_EN;

         temp.un.ethernet.rdto       = 0;
         temp.un.ethernet.broad_wrap = ((dev->nddp->ndd_flags&NDD_SIMPLEX)==0);

         memcpy(temp.un.ethernet.net_addr,
                dev->dds.ds.ent.curAdptAddr,
                min(sizeof(temp.un.ethernet.net_addr),
                    sizeof(dev->dds.ds.ent.curAdptAddr)));

         memcpy(temp.un.ethernet.haddr,
                dev->dds.ds.ent.hdwAdptAddr,
                min(sizeof(temp.un.ethernet.haddr),
                    sizeof(dev->dds.ds.ent.hdwAdptAddr)));

         break;
      }

      case CIE_DEV_FDDI:
      {
         temp.devtype             = DD_NET_DH;
         temp.flags               = 0;
         temp.devsubtype          = DD_FDDI;

         temp.un.fddi.rdto       = 0;
         temp.un.fddi.broad_wrap = ((dev->nddp->ndd_flags&NDD_SIMPLEX)==0);

         memcpy(temp.un.fddi.netaddr,
                dev->dds.ds.fddi.curAdptAddr,
                min(sizeof(temp.un.fddi.netaddr),
                    sizeof(dev->dds.ds.fddi.curAdptAddr)));

         memcpy(temp.un.fddi.haddr,
                dev->dds.ds.fddi.hdwAdptAddr,
                min(sizeof(temp.un.fddi.haddr),
                    sizeof(dev->dds.ds.fddi.hdwAdptAddr)));

         temp.un.fddi.attach_class = ((dev->nddp->ndd_flags&CFDDI_NDD_DAC)==0);

         break;
      }
   }

   if ((xrc = copyout((caddr_t)&temp,(caddr_t)info,sizeof(struct devinfo))) != 0)
   {
      TRC_OTHER(ddvb,0,&temp,info);
      return EFAULT;
   }

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                    Query Statistics - ioctl(CIO_QUERY)                    */
/*---------------------------------------------------------------------------*/

int
   queryStatistics(
      register DEV         * dev         ,// I -Device
      register CHN         * chn         ,// I -Channel
      register void        * argp        ,//  O-Paramter Block
      register ulong         devflag      // I -Device Flags from open
   )
{
   FUNC_NAME(queryStatistics);

   cio_query_blk_t           qp;
   int                       rc  = 0;
   int                       xrc = 0;

   if ((xrc = copyin((caddr_t)argp,(caddr_t)&qp,sizeof(qp))) != 0)
   {
      TRC_OTHER(qsta,xrc,argp,&qp);
      return EFAULT;
   }

   qp.status = CIO_OK;

   switch(dev->dds.devType)
   {
      case CIE_DEV_TOK:
      {
         tok_query_stats_t   cioTokStats;
         mon_all_stats_t     tokStats;
         int                 l;

         /*------------------------------------------------------*/
         /*  Retrieve Statistics from the Network Device Driver  */
         /*------------------------------------------------------*/

         rc = NDD_CTL(
                 dev->nddp,
                 NDD_GET_ALL_STATS,
                 &tokStats,
                 sizeof(tokStats));

         if (rc == 0)
         {
            /*--------------------------------------------------------*/
            /*  Map the returned statistics into a COMIO stats block  */
            /*--------------------------------------------------------*/

            mapTokMonStats(
               &cioTokStats,
               &dev->stats,
               &tokStats,
               &(dev->nddp->ndd_demuxer->nd_dmxstats),
               (tok_dmx_stats_t *)&(dev->nddp->ndd_demuxer->nd_specstats));

            if ((l = qp.buflen) < sizeof(cioTokStats))
               qp.status = CIO_BUF_OVFLW;
            else
               l = sizeof(cioTokStats);

            if ((xrc = copyout((caddr_t)&cioTokStats,(caddr_t)qp.bufptr,l)) != 0)
            {
               TRC_OTHER(qstu,xrc,&cioTokStats,qp.bufptr);
               rc = EFAULT;
            }
         }
         else
            TRC_OTHER(qstt,rc,chn,0);

         break;
      }

      case CIE_DEV_FDDI:
      {
         fddi_query_stats_t  cioFddiStats;
         fddi_ndd_stats_t    fddiStats;
         int                 l;

         /*------------------------------------------------------*/
         /*  Retrieve Statistics from the Network Device Driver  */
         /*------------------------------------------------------*/

         rc = NDD_CTL(
                 dev->nddp,
                 NDD_GET_STATS,
                 &fddiStats,
                 sizeof(fddiStats));

         if (rc == 0)
         {
            /*--------------------------------------------------------*/
            /*  Map the returned statistics into a COMIO stats block  */
            /*--------------------------------------------------------*/

            mapFddiStats(
               &cioFddiStats,
               &dev->stats,
               &fddiStats,
               &(dev->nddp->ndd_demuxer->nd_dmxstats),
               (fddi_dmx_stats_t *)&(dev->nddp->ndd_demuxer->nd_specstats));

            if ((l = qp.buflen) < sizeof(cioFddiStats))
               qp.status = CIO_BUF_OVFLW;
            else
               l = sizeof(cioFddiStats);

            if (copyout((caddr_t)&cioFddiStats,(caddr_t)qp.bufptr,l) != 0)
            {
               TRC_OTHER(qstg,xrc,&cioFddiStats,qp.bufptr);
               rc = EFAULT;
            }
         }
         else
            TRC_OTHER(qstf,rc,chn,0);

         break;
      }

      case CIE_DEV_ENT:
      {
         ent_query_stats_t   cioEntStats;
	 ent_stable_t stable;         /* shared statistic table */
         int                 l;

         /*------------------------------------------------------*/
         /*  Retrieve Statistics from the Network Device Driver  */
         /*------------------------------------------------------*/

         rc = NDD_CTL(
                 dev->nddp,
                 NDD_GET_STATS,
                 &stable,
                 sizeof(ent_ndd_stats_t));
	 if (rc) {
		TRC_OTHER(qste,rc,chn,0);
		break;
	 }

	 if (stable.ent_ndd.ent_ent_genstats.device_type == ENT_3COM) {
           rc = NDD_CTL(
                 dev->nddp,
                 NDD_GET_ALL_STATS,
                 &stable,
                 sizeof(en3com_all_stats_t));

           if (rc) {
             TRC_OTHER(qste,rc,chn,0);
	     break;
	   }
           /*--------------------------------------------------------*/
           /*  Map the returned statistics into a COMIO stats block  */
           /*--------------------------------------------------------*/

           mapEnt3ComStats(
               &cioEntStats,
               &dev->stats,
               &stable,
               &(dev->nddp->ndd_demuxer->nd_dmxstats),
               (eth_dmx_stats_t *)&(dev->nddp->ndd_demuxer->nd_specstats));

         }
         else {    /* this is for Integrated Ethernet */
            /*--------------------------------------------------------*/
            /*  Map the returned statistics into a COMIO stats block  */
            /*--------------------------------------------------------*/

             mapEntIentStats(
               &cioEntStats,
               &dev->stats,
               &stable,
               &(dev->nddp->ndd_demuxer->nd_dmxstats),
               (eth_dmx_stats_t *)&(dev->nddp->ndd_demuxer->nd_specstats));
	 }
         if ((l = qp.buflen) < sizeof(cioEntStats))
               qp.status = CIO_BUF_OVFLW;
         else
               l = sizeof(cioEntStats);

         if (copyout((caddr_t)&cioEntStats,(caddr_t)qp.bufptr,l) != 0) {
               TRC_OTHER(qsth,xrc,&cioEntStats,qp.bufptr);
               rc = EFAULT;
         }

         if ((l = qp.buflen) < sizeof(cioEntStats))
               qp.status = CIO_BUF_OVFLW;
         else
               l = sizeof(cioEntStats);

         if (copyout((caddr_t)&cioEntStats,(caddr_t)qp.bufptr,l) != 0) {
               TRC_OTHER(qsth,xrc,&cioEntStats,qp.bufptr);
               rc = EFAULT;
         }

         break;
      }

      default:
      {
         TRC_OTHER(qstd,dev->dds.devType,0,0);
         rc = ENODEV;
         break;
      }
   }

   /*-------------------------------------------------------------------*/
   /*  If all is well, write back the query block and execute clearall  */
   /*-------------------------------------------------------------------*/

   if (rc == 0)
   {
      if ((xrc = copyout((caddr_t)&qp,(caddr_t)argp,sizeof(qp))) != 0)
      {
         TRC_OTHER(qstb,xrc,&qp,argp);
         rc = EFAULT;
      }
      else
      {
         if (qp.clearall == CIO_QUERY_CLEAR)
         {
            /*---------------------------------------*/
            /*  Clear The Emulator-Maintained Stats  */
            /*---------------------------------------*/

            dev->stats.sesMax  = 0;
            dev->stats.rdqMax  = 0;
            dev->stats.rdqOvfl = 0;
            dev->stats.stqMax  = 0;
            dev->stats.stqOvfl = 0;

            /*-----------------------------------*/
            /*  Clear the CDLI-maintained Stats  */
            /*-----------------------------------*/

            rc = NDD_CTL(dev->nddp,NDD_CLEAR_STATS,NULL,0);

            if (rc) TRC_OTHER(qstz,rc,dev,0);
         }
      }
   }

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                          Retrieve Token Ring VPD                          */
/*---------------------------------------------------------------------------*/

int
   tokQueryVPD(
      register DEV         * dev         ,// I -Device
      register CHN         * chn         ,// I -Channel
      register void        * vpd         ,//  O-User's VPD Buffer
      register ulong         devflag      // I -Device Flags from open
   )
{
   FUNC_NAME(tokQueryVPD);

   int                       xrc = 0;

   if ((xrc = copyout((caddr_t)&dev->dds.ds.tok.vpd,
               (caddr_t)vpd,
               sizeof(tok_vpd_t))) != 0)
   {
      TRC_OTHER(tqvb,xrc,&dev->dds.ds.tok.vpd,vpd);
      return EFAULT;
   }

   return 0;
}

/*---------------------------------------------------------------------------*/
/*              Retrieve Token-Ring-Specific Device Information              */
/*---------------------------------------------------------------------------*/

int
   tokQueryTokenRingInfo(
      register DEV         * dev         ,// I -Device
      register CHN         * chn         ,// I -Channel
      register void        * argp        ,// IO-User's tok_q_ring_info_t buf
      register ulong         devflag      // I -Device Flags from open
   )
{
   FUNC_NAME(tokQueryTokenRingInfo);

   tok_q_ring_info_t         q;           // Query Block
   tok_ring_info_t           ringInfo;    // Token Ring Info Block
   token_ring_all_mib_t      trMIB;       // SNMP MIB for Token Ring

   int                       rc  = 0;
   int                       xrc = 0;

   /*-------------------------------------------*/
   /*  Acquire a copy of Query Parameter Block  */
   /*-------------------------------------------*/

   if ((xrc = copyin((caddr_t)argp,(caddr_t)&q,sizeof(q))) != 0)
   {
      TRC_OTHER(tria,xrc,argp,&q);
      return EFAULT;
   }

   /*-------------------------------------------*/
   /*  Make sure the output pointer isn't null  */
   /*-------------------------------------------*/

   if (q.p_info == NULL)
   {
      TRC_OTHER(tric,&q,0,0);
      return EFAULT;
   }

   /*------------------------------------------*/
   /*  Initialize the output buffer temp area  */
   /*------------------------------------------*/

   memset(&ringInfo,0x00,sizeof(ringInfo));

   /*----------------------------------------------------*/
   /*  Set the status code in the query parameter block  */
   /*----------------------------------------------------*/

   q.status = (qEmpty(chn->sesQueue) ? CIO_NOT_STARTED : 0);

   /*-----------------------------*/
   /*  Copy the current address   */
   /*-----------------------------*/

   mapNetAddress(CIE_DEV_TOK,
                 CDLI_TO_CIO,
                 sizeof(ringInfo.adap_phys_addr),
                 &ringInfo.adap_phys_addr,
                 sizeof(dev->ds.tok.curAddr),
                 &dev->ds.tok.curAddr);

   /*------------------------------------------*/
   /*  Get the NAUN address from the SNMP MIB  */
   /*------------------------------------------*/

   if ((rc = NDD_CTL(dev->nddp,NDD_MIB_GET,&trMIB,sizeof(trMIB))) == 0)
   {
      mapNetAddress(CIE_DEV_TOK,
                    CDLI_TO_CIO,
                    sizeof(ringInfo.upstream_node_addr),
                    &ringInfo.upstream_node_addr,
                    sizeof(trMIB.Token_ring_mib.Dot5Entry.upstream),
                    &trMIB.Token_ring_mib.Dot5Entry.upstream);
   }
   else
      TRC_OTHER(trix,rc,dev,0);

   /*--------------------------------------------------------------*/
   /*  Copy the info buffer temp to its destination in user-space  */
   /*--------------------------------------------------------------*/

   if ((xrc = copyout((caddr_t)&ringInfo,
                     q.p_info,
                     min(q.l_buf,sizeof(ringInfo)))) != 0)
   {
      TRC_OTHER(trib,xrc,&ringInfo,q.p_info);
      return EFAULT;
   }

   /*---------------------------------------------------------*/
   /*  Copy the updated parameter block back into user-space  */
   /*---------------------------------------------------------*/

   if ((xrc = copyout((caddr_t)&q,(caddr_t)argp,sizeof(q))) != 0)
   {
      TRC_OTHER(trid,xrc,&q,argp);
      return EFAULT;
   }

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                           Retrieve Ethernet VPD                           */
/*---------------------------------------------------------------------------*/

int
   entQueryVPD(
      register DEV         * dev         ,// I -Device
      register CHN         * chn         ,// I -Channel
      register void        * vpd         ,//  O-User's VPD Buffer
      register ulong         devflag      // I -Device Flags from open
   )
{
   FUNC_NAME(entQueryVPD);

   int                       rc  = 0;
   int                       xrc = 0;

   if ((xrc = copyout((caddr_t)&dev->dds.ds.ent.vpd,
               (caddr_t)vpd,
               sizeof(ccc_vpd_blk_t))) != 0)
   {
      TRC_OTHER(eqvb,xrc,&dev->dds.ds.ent.vpd,vpd);
      return EFAULT;
   }

   return rc;
}
