static char sccsid[]="@(#)14   1.13  src/bos/kernext/cie/chn.c, sysxcie, bos411, 9438B411a 9/20/94 10:15:57";

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 *   new_CHN
 *   free_CHN
 *   openChannel
 *   closeChannel
 *   getChannelStatus
 *   readData
 *   readStatus
 *   queueStatus
 *   writeData
 *   entMulticastAddr
 *   tokFuncAddr
 *   tokGroupAddr
 *   tokSetGroupAddr
 *   tokClearGroupAddr
 *   fddiGroupAddr
 *   fddiClearAllGroups
 *   fddiQueryGroupAddr
 *   entPromiscuousOn
 *   entPromiscuousOff
 *   startSession
 *   haltSession
 *   haltAllSessions
 *
 * DESCRIPTION:
 *
 *    Channel Object Implementation
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

#include <sys/ndd.h>
#include <sys/cdli.h>
#include <sys/comio.h>
#include <sys/device.h>
#include <sys/err_rec.h>
#include <sys/ciouser.h>
#include <sys/tokuser.h>
#include <sys/fddiuser.h>
#include <sys/entuser.h>
#include <sys/mbuf.h>
#include <sys/poll.h>
#include <sys/rtc.h>
#include <stdarg.h>

#include "ciedd.h"
#include "chnlist.h"
#include "dev.h"
#include "chn.h"
#include "ses.h"
#include "addrtbl.h"
#include "mapaddr.h"
#include "mibaddr.h"
#include "nsdmx.h"
#include "status.h"
#include "dmalloc.h"
#include "mbqueue.inl"

/*---------------------------------------------------------------------------*/
/*             Allocate and initialize a Channel Data Structure              */
/*---------------------------------------------------------------------------*/

CHN *
   new_CHN(
      register CIE_DEV_TYPE  devType     ,// I -Device Type Code
      register int           recQueueSize,// I -Receive Queue Size
      register int           staQueueSize // I -Status Queue Size
   )
{
   FUNC_NAME(new_CHN);

   register CHN            * chn;

   /*-----------------------------------------*/
   /*  Allocate a new Channel Data Structure  */
   /*-----------------------------------------*/

   if ((chn=(CHN *)xmalloc(sizeof(CHN),3,pinned_heap)) == NULL)
   {
      XMFAIL(sizeof(CHN));
      return NULL;
   }

   memset(chn,0x00,sizeof(*chn));

   /*--------------------------*/
   /*  Initialize the new CHN  */
   /*--------------------------*/

   strcpy(chn->iCatcher,"CHN");

   chn->dev              =  NULL;
   chn->devno            =  0;
   chn->chan             = -1;
   chn->devflag          =  0;
   chn->sesQueue         =  NULL;
   chn->rdqNotEmpty      =  EVENT_NULL;
   chn->sid              =  0;
   chn->open             =  0;
   chn->lostStatus       =  0;
   chn->rcv_sig		 =  0;

   /*--------------------------------------------*/
   /*  Allocate and initialize the Channel Lock  */
   /*--------------------------------------------*/

   lock_alloc(&chn->lock,
              LOCK_ALLOC_PIN,
              CIO_LOCK_CLASS,
              CIE_LOCK_CHANNEL);

   lock_init(&chn->lock,TRUE);

   /*--------------------------------------------------*/
   /*  Initialize the Received Data and Status Queues  */
   /*--------------------------------------------------*/

   mbqInit(&chn->rdq,recQueueSize);
   mbqInit(&chn->stq,staQueueSize);

   /*------------------------------*/
   /*  Allocate the Session Queue  */
   /*------------------------------*/

   if ((chn->sesQueue = new_QUEUE(10,QUEUE_DYNAMIC)) == NULL)
   {
      TRC_OTHER(chnq,0,0,0);
      lock_free(&chn->lock);
      if (xmfree(chn,pinned_heap) != 0) XFFAIL();
      return NULL;
   }

   return chn;
}

/*---------------------------------------------------------------------------*/
/*                       Free a Channel Data Structure                       */
/*---------------------------------------------------------------------------*/

void
   free_CHN(
      register CHN         * chn          // IO-Channel to be freed
   )
{
   FUNC_NAME(free_CHN);

   if (chn)
   {
      lock_free(&chn->lock);

      /*--------------------------*/
      /*  Free the Session Queue  */
      /*--------------------------*/

      free_QUEUE(chn->sesQueue);

      /*-----------------------------------*/
      /*  Free the Channel Data Structure  */
      /*-----------------------------------*/

      if (xmfree(chn,pinned_heap) != 0) XFFAIL();
   }

}

/*---------------------------------------------------------------------------*/
/*                             Open the Channel                              */
/*---------------------------------------------------------------------------*/

int
   openChannel(
      register DEV         * dev         ,// IO-The Device
      register CHN         * chn         ,// IO-The Channel
      register ulong         devflag      // I -Device Flags
   )
{
   FUNC_NAME(openChannel);

   ns_statuser_t             user;
   int                       rc = 0;

   extern void               ciestat();

   /*-----------------------------------------*/
   /*  If no current opens, allocate the NDD  */
   /*-----------------------------------------*/

   if (dev->nchOpen == 0)
   {
      ndd_t                * nddp;

      if ((rc = ns_alloc(dev->dds.nddName,&nddp)) != 0)
      {
         TRC_OTHER(chnf,rc,0,0);
         return rc;
      }

      dev->nddp      = nddp;
   }

   /*----------------------------------------*/
   /*  Enable receipt of status information  */
   /*----------------------------------------*/

   user.isr      = ciestat;
   user.isr_data = (caddr_t) chn;

   if ((rc = enableStatus(dev->nddp,&chn->sid,&user)) != 0)
   {
      if (dev->nchOpen == 0)
      {
         ns_free(dev->nddp);
         dev->nddp = NULL;
      }

      return rc;
   }

   /*----------------------------*/
   /*  Complete open processing  */
   /*----------------------------*/

   dev->nchOpen++;

   chn->open     = 1;
   chn->devflag  = devflag;

   return 0;
}

/*---------------------------------------------------------------------------*/
/*                             Close the channel                             */
/*---------------------------------------------------------------------------*/

int
   closeChannel(
      register DEV         * dev         ,// IO-The Device
      register CHN         * chn          // IO-The Channel
   )
{
   FUNC_NAME(closeChannel);

   mbuf_t                  * m;
   int                       rc = 0;

   /*--------------------------------------------*/
   /*  Make sure this channel is currently open  */
   /*--------------------------------------------*/

   if (!chn->open)
   {
      TRC_OTHER(chng,chn,0,0);
      return ENXIO;
   }

   chn->open = 0;

   /*-----------------------------------------*/
   /*  Disable receipt of status information  */
   /*-----------------------------------------*/

   disableStatus(dev->nddp,&chn->sid);

   /*-------------------------------------------------------------*/
   /*  Disable Group/Multicast addresses enabled on this channel  */
   /*-------------------------------------------------------------*/

   switch(dev->dds.devType)
   {
      case CIE_DEV_TOK:

         if (chn->ds.tok.groupActive)
            tokClearGroupAddr(dev,chn,dev->nddp,dev->ds.tok.groupAddr);

         break;

      case CIE_DEV_FDDI:

         disableAllGroups(dev->nddp,
                          dev->ds.fddi.groupTable,
                          chn->ds.fddi.groupRef);
         break;

      case CIE_DEV_ENT:

         disableAllGroups(dev->nddp,
                          dev->ds.ent.multiCastTable,
                          chn->ds.ent.multiCastRef);
         break;

      default:

         TRC_OTHER(cchd,dev->dds.devType,0,0);
         break;
   }

   /*---------------------------------*/
   /*  Terminate all active sessions  */
   /*---------------------------------*/

   haltAllSessions(dev,chn);

   /*-----------------------------*/
   /*  Purge the Read Data Queue  */
   /*-----------------------------*/

   while((m = mbqDeQueue(&chn->rdq)) != NULL) m_freem(m);

   /*--------------------------*/
   /*  Purge the Status Queue  */
   /*--------------------------*/

   while((m = mbqDeQueue(&chn->stq)) != NULL) m_freem(m);

   /*----------------------------------------------*/
   /*  If this is the last close, dealloc the NDD  */
   /*----------------------------------------------*/

   if (--dev->nchOpen == 0)
   {
      ns_free(dev->nddp);
      dev->nddp = NULL;
   }

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                           Return Channel Status                           */
/*---------------------------------------------------------------------------*/

int
   getChannelStatus(
      register DEV            * dev      ,// IO-Device
      register CHN            * chn      ,// IO-Channel
      register cio_stat_blk_t * arg      ,// IO-Status Block Return Area
      register ulong            devflag   // I -Device Flags from Open
   )
{
   FUNC_NAME(getChannelStatus);

   cio_stat_blk_t            stat;
   int                       rc  = 0;
   int                       xrc = 0;

   /*---------------------------------*/
   /*  Dequeue the next status block  */
   /*---------------------------------*/

   readStatus(dev->dds.devType,chn,&stat);

   /*-------------------------*/
   /*  Copy it to user space  */
   /*-------------------------*/

   if ((xrc = copyout((caddr_t)&stat,(caddr_t)arg,sizeof(*arg))) != 0)
   {
      TRC_OTHER(chni,xrc,&stat,arg);
      return EFAULT;
   }

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                Wait for and dequeue the next input buffer                 */
/*---------------------------------------------------------------------------*/

int
   readData(
      register DEV         * dev         ,// IO-The Device
      register CHN         * chn         ,// IO-The Channel
      register mbuf_t     ** pm           //  O-Output mbuf pointer
   )
{
   FUNC_NAME(readData);
   int                       pSave;       // Interrupt Priority Save Area

   mbuf_t                  * m;

   /*-------------------------------------*/
   /*  Initialize output pointer to NULL  */
   /*-------------------------------------*/

   *pm = NULL;

   /*----------------------------------------------*/
   /*  First see if there's anything in the queue  */
   /*----------------------------------------------*/

   lockChannel(pSave,chn);

   while((m = mbqDeQueue(&chn->rdq)) == NULL)
   {
      int                    wakeupCode;

      unlockChannel(pSave,chn);

      /*-------------------------------------------------------*/
      /*  If not, and DNDELAY was specified, just return zero  */
      /*-------------------------------------------------------*/

      if (chn->devflag & DNDELAY) return 0;

      TRC_RECV(RWIT,dev,chn,0);

      /*-----------------------------------------------*/
      /*  Sleep until something appears on this queue  */
      /*-----------------------------------------------*/

      wakeupCode = e_sleep_thread(
                       &chn->rdqNotEmpty,          // Event List Anchor Address
                       &dev->lock,                 // Lock to be released
                       LOCK_SIMPLE|INTERRUPTIBLE); // Lock flags

      /*---------------------------------------------------------------------*/
      /*  Check reason for waking up (note: dev->lock has been reacquired)   */
      /*---------------------------------------------------------------------*/

      if (wakeupCode == THREAD_INTERRUPTED)
      {
         TRC_RECV(RINT,dev,chn,0);
         return EINTR;
      }

      lockChannel(pSave,chn);
   }

   unlockChannel(pSave,chn);

   *pm = m;

   return 0;
}

/*---------------------------------------------------------------------------*/
/*                 Dequeue and return the next status block                  */
/*---------------------------------------------------------------------------*/

void
   readStatus(
      register CIE_DEV_TYPE     devType     ,// I -Device Type Code
      register CHN            * chn         ,// IO-The Channel
      register cio_stat_blk_t * stat         //  O-Output status buffer
   )
{
   FUNC_NAME(readStatus);

   STATBLK                 * s = NULL;
   int                       pSave;       // Interrupt Priority Save Area
   int                       i;

   lockChannel(pSave,chn);

   if (chn->lostStatus)
   {
      chn->lostStatus = 0;

      unlockChannel(pSave,chn);

      TRC_RECV(LSDQ,chn,0,0);

      s = mkLostStatusStatus();
   }
   else
   {
      mbuf_t               * m;

      m = (mbqDeQueue(&chn->stq));

      unlockChannel(pSave,chn);

      s = (m ? mtod(m,STATBLK *) : mkNullBlkStatus());
   }

   /*-----------------------------------------------------*/
   /*  Map the status block to COMIO format if necessary  */
   /*-----------------------------------------------------*/

   if (s->format == STAT_CDLI)
      mapAsyncStatus(devType,stat,&s->stat.ndd);
   else
      *stat = s->stat.cio;

   /*-------------------------*/
   /*  Free the Status Block  */
   /*-------------------------*/

   free_STATBLK(s);
}

/*---------------------------------------------------------------------------*/
/*                     Queue a status block to a channel                     */
/*---------------------------------------------------------------------------*/

void
   queueStatus(
      register CHN           * chn         ,// IO-The channel
      register const STATBLK * s            // I -The status block
   )
{
   int                       pSave;       // Interrupt Priority Save
   register DEV            * dev;

   /*--------------------------------------------------------*/
   /*  Queue the status block on the channel's status queue  */
   /*--------------------------------------------------------*/

   dev = chn->dev;

   if (mbqQueue(&chn->stq,dtom(s)) != NULL)
   {
      setHWM(dev->stats.stqMax,mbqHWM(&chn->stq));

      selnotify(chn->devno,chn->chan,POLLPRI);
   }
   else
   {
      /*----------------------------------------------------------------------*/
      /*  The queue was full - record that fact and release the status block  */
      /*----------------------------------------------------------------------*/

      TRC_OTHER(LOST,dev,chn,s);
      fetch_and_add(&dev->stats.stqOvfl,1);
      chn->lostStatus = 1;

      free_STATBLK(s);
   }
}

/*---------------------------------------------------------------------------*/
/*                       Write a data block to the NDD                       */
/*---------------------------------------------------------------------------*/

void
   writeData_timeout(
      register struct trb  * trb          // Timer request block
   )
{
   e_clear_wait((tid_t)trb->func_data, THREAD_TIMED_OUT);
}

int
   writeData(
      register DEV         * dev         ,// IO-The device
      register CHN         * chn         ,// IO-The channel
      register mbuf_t      * m            // I -The mbuf
   )
{
   FUNC_NAME(writeData);

   register int              rc   = 0;
   register int              done = 0;
   int                       curPrty;


#if defined(DMALLOC)
{
   /*----------------------------------------------------------------*/
   /*  Tentatively mark this mbuf chain as released because, if the  */
   /*  write is successful, we will no longer own it                 */
   /*----------------------------------------------------------------*/

   mbuf_t                  * p;
   mbuf_t                  * q;

   for (p=m; p; p=p->m_nextpkt)
      for (q=p; q; q=q->m_next)
         d_release(q,MBUF);
}
#endif
   while(!done)
   {
      switch(rc = NDD_OUTPUT(dev->nddp,m))
      {
         /*---------------------------------------------------------*/
         /*  Write successful (at least the NDD accepted the mbuf)  */
         /*---------------------------------------------------------*/

         case 0:
         {
            done = 1;
            break;
         }

         /*--------------------------------------------*/
         /*  EAGAIN: We'll have to wait and try again  */
         /*--------------------------------------------*/

         case EAGAIN:
         {
            TRC_XMIT(XAGN,dev,chn,m);

            /*--------------------------*/
            /*  Check for DNDELAY flag  */
            /*--------------------------*/

            if (chn->devflag & DNDELAY)
            {
               /*--------------------------------------------------------*/
               /*  User doesn't want blocking write, just return EAGAIN  */
               /*--------------------------------------------------------*/

               done = 1;
            }
            else
            {
               /*----------------------------------------*/
               /*  Blocking write... retry after 100 ms  */
               /*----------------------------------------*/

               struct trb *trb;

               /*-----------------------*/
               /*  Interruptible delay  */
               /*-----------------------*/
               trb = talloc();
               /*CIE_ASSERT(trb != NULL);*/
               TICKS_TO_TIME(trb->timeout.it_value, HZ/10);
               trb->flags      =  T_INCINTERVAL;
               trb->func       =  (void (*)(void *))writeData_timeout;
               trb->eventlist  =  EVENT_NULL;
               trb->func_data  =  (uint)thread_self();
               trb->ipri       =  INTTIMER;
               e_assert_wait(&trb->eventlist, INTERRUPTIBLE);
               simple_unlock(&dev->lock);
               tstart(trb);
               rc = e_block_thread();
               while (tstop(trb));
               tfree(trb);
               simple_lock(&dev->lock);

               /*---------------------------*/
               /*  If interrupted bail out  */
               /*---------------------------*/

               if (rc == THREAD_INTERRUPTED)
               {
                  TRC_XMIT(XINT,dev,chn,m);
                  done = 1;
                  rc   = EINTR;
               }
            }

            break;
         }

         /*-------------------------*/
         /*  Any other return code  */
         /*-------------------------*/

         default:
         {
            TRC_XMIT(XERR,rc,chn,m);
            done = 1;
            break;
         }
      }
   }

   /*----------------------------------------------------------------*/
   /*  If the write was unsuccessful we have to free the mbuf chain  */
   /*----------------------------------------------------------------*/

   if (rc != 0)
   {
#if defined(DMALLOC)
{
   /*-----------------------------------------------------*/
   /*  We have to put the mbuf chain back into the debug  */
   /*  table so that d_freem() can find it                */
   /*-----------------------------------------------------*/

   mbuf_t                  * p;
   mbuf_t                  * q;
   for (p=m; p; p=p->m_nextpkt)
      for (q=p; q; q=q->m_next)
         d_save(q,q->m_len,MBUF,__FILE__,__FUNC__,__LINE__);
}
#endif

      m_freem(m);
   }

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                Handle Ethernet Multicast Address Requests                 */
/*---------------------------------------------------------------------------*/

int
   entMulticastAddr(
      register DEV         * dev         ,// I -Device
      register CHN         * chn         ,// I -Channel
      register void        * argp        ,// IO-Parameter block
      register ulong         devflag      // I -Device Flags from open
   )
{
   FUNC_NAME(entMulticastAddr);

   ent_set_multi_t           arg;        // Local copy of parameter block
   int                       pSave;       // Interrupt Priority Save Area
   int                       rc  = 0;
   int                       xrc = 0;

   /*---------------------------------------------*/
   /*  Copy the parameter block to local storage  */
   /*---------------------------------------------*/

   if ((xrc = copyin((caddr_t)argp,(caddr_t)&arg,sizeof(arg))) != 0)
   {
      TRC_OTHER(emaa,xrc,chn,argp);
      return EFAULT;
   }

   /*--------------------------------*/
   /*  Processing depends on opcode  */
   /*--------------------------------*/

   switch(arg.opcode)
   {
      /*-----------------------*/
      /*  Add a group address  */
      /*-----------------------*/

      case ENT_ADD:
      {
         rc = enableGroup(dev->nddp,
                          dev->ds.ent.multiCastTable,
                          chn->ds.ent.multiCastRef,
                          arg.multi_addr);
         break;
      }

      /*--------------------------*/
      /*  Remove a group address  */
      /*--------------------------*/

      case ENT_DEL:
      {
         rc = disableGroup(dev->nddp,
                           dev->ds.ent.multiCastTable,
                           chn->ds.ent.multiCastRef,
                           arg.multi_addr);
         break;
      }

      /*-------------------*/
      /*  Invalid command  */
      /*-------------------*/

      default:
      {
         TRC_OTHER(emac,arg.opcode,chn,0);
         rc = EFAULT;
         break;
      }
   }

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                 Set/Clear a Token Ring Functional Address                 */
/*---------------------------------------------------------------------------*/

int
   tokFuncAddr(
      register DEV         * dev         ,// I -Device
      register CHN         * chn         ,// I -Channel
      register void        * argp        ,// IO-Parameter Block
      register ulong         devflag      // I -Device Flags from open
   )
{
   FUNC_NAME(tokFuncAddr);

   SES                     * ses;
   CURSOR                    c;
   tok_func_addr_t           arg;
   int                       pSave;       // Interrupt Priority Save Area
   int                       xrc = 0;
   int                       rc  = 0;

   /*-----------------------------------------*/
   /*  Copy parameter block to local storage  */
   /*-----------------------------------------*/

   if ((xrc = copyin((caddr_t)argp,(caddr_t)&arg,sizeof(arg))) != 0)
   {
      TRC_OTHER(tfaa,xrc,chn,argp);
      return EFAULT;
   }

   /*------------------------------------------*/
   /*  Make sure the channel has been started  */
   /*------------------------------------------*/

   if (qEmpty(chn->sesQueue))
   {
      TRC_OTHER(tfan,chn,0,0);
      arg.status = CIO_NOT_STARTED;
      rc         = ENOMSG;
   }
   else
   {
      /*-------------------------------------------*/
      /*  Find a DSAP session with matching netid  */
      /*-------------------------------------------*/

      for (
         ses  = qFirst(chn->sesQueue,c,SES);
         ses != NULL && !(ses->type == SES_DSAP && ses->netid == arg.netid);
         ses  = qNext(chn->sesQueue,c,SES)
      );

      if (ses == NULL)
      {
         TRC_OTHER(tfas,chn,arg.netid,0);
         arg.status = CIO_NETID_INV;
         rc         = ENOMSG;
      }
      else
      {
         /*------------------------------------------------*/
         /*  Perform the requested operation (add/delete)  */
         /*------------------------------------------------*/

         arg.status = CIO_OK;
         rc         = 0;

         arg.func_addr &= 0x7FFFFFFF; // Ensure functional/group bit is off

         switch(arg.opcode)
         {
            case TOK_ADD:
               rc = tokSetFuncAddr(ses,dev->nddp,arg.func_addr);
               break;

            case TOK_DEL:
               rc = tokClearFuncAddr(ses,dev->nddp,arg.func_addr);
               break;

            default:
               TRC_OTHER(tfac,chn,arg.opcode,0);
               arg.status = CIO_INV_CMD;
               rc         = ENOMSG;
               break;
         }
      }
   }

   /*------------------------------------------------*/
   /*  Return updated parameter block to user space  */
   /*------------------------------------------------*/

   if ((xrc = copyout((caddr_t)&arg,(caddr_t)argp,sizeof(arg))) != 0)
   {
      TRC_OTHER(tfab,xrc,&arg,argp);
      return EFAULT;
   }

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                      Set a Token Ring Group Address                       */
/*---------------------------------------------------------------------------*/

int
   tokGroupAddr(
      register DEV         * dev         ,// I -Device
      register CHN         * chn         ,// I -Channel
      register void        * argp        ,// IO-Parameter Block
      register ulong         devflag      // I -Device Flags from open
   )
{
   FUNC_NAME(tokGroupAddr);

   tok_group_addr_t          arg;
   int                       pSave;       // Interrupt Priority Save Area
   int                       rc  = 0;
   int                       xrc = 0;

   /*-----------------------------------------*/
   /*  Copy Parameter Block to local storage  */
   /*-----------------------------------------*/

   if ((xrc = copyin((caddr_t)argp,(caddr_t)&arg,sizeof(arg))) != 0)
   {
      TRC_OTHER(tgaa,xrc,argp,&arg);
      return EFAULT;
   }

   arg.status = CIO_OK;

   /*----------------------------------*/
   /*  Force the Group Address bit on  */
   /*----------------------------------*/

   arg.group_addr |= 0x80000000;

   /*--------------------------------*/
   /*  Processing depends on opcode  */
   /*--------------------------------*/

   switch(arg.opcode)
   {
      case TOK_ADD:
      {
         if (dev->ds.tok.groupActive && dev->ds.tok.groupOwner!=chn->chan)
         {
            TRC_OTHER(tgad,0,0,0);
            arg.status = TOK_NO_GROUP;
         }
         else
            rc = tokSetGroupAddr(dev,chn,dev->nddp,arg.group_addr);

         break;
      }

      case TOK_DEL:
      {
         if (!(dev->ds.tok.groupActive && dev->ds.tok.groupOwner==chn->chan))
         {
            TRC_OTHER(tgax,0,0,0);
            arg.status = TOK_NO_GROUP;
         }
         else
            rc = tokClearGroupAddr(dev,chn,dev->nddp,arg.group_addr);

         break;
      }

      default:
      {
         TRC_OTHER(tgac,arg.opcode,0,0);
         arg.status = CIO_INV_CMD;
         break;
      }
   }

   /*------------------------------------------------*/
   /*  Return updated parameter block to user space  */
   /*------------------------------------------------*/

   if (rc == 0)
   {
      if ((xrc = copyout((caddr_t)&arg,(caddr_t)argp,sizeof(arg))) != 0)
      {
         TRC_OTHER(tgab,xrc,&arg,argp);
         return EFAULT;
      }
   }

   return (rc ? rc : (arg.status ? ENOMSG : 0));
}

/*---------------------------------------------------------------------------*/
/*                      Set a Token Ring Group Address                       */
/*---------------------------------------------------------------------------*/

int
   tokSetGroupAddr(
      register DEV         * dev         ,// IO-Device
      register CHN         * chn         ,// IO-Channel
      register ndd_t       * nddp        ,// IO-NDD
      register unsigned int  groupAddr    // I -Group Address (lsw)
   )
{
   FUNC_NAME(tokSetGroupAddr);

   char                      addr[6] = { 0xC0,0x00,0x00,0x00,0x00,0x00 };
   int                       pSave;       // Interrupt Priority Save Area
   int                       rc;

   *(unsigned int *)(&addr[2]) = groupAddr;

   rc = NDD_CTL(nddp,NDD_ENABLE_ADDRESS,(caddr_t)&addr,6);

   if (rc == 0)
   {
      dev->ds.tok.groupAddr   = groupAddr;
      dev->ds.tok.groupOwner  = chn->chan;
      dev->ds.tok.groupActive = 1;
      chn->ds.tok.groupActive = 1;
   }
   else
      TRC_OTHER(tsga,rc,chn,groupAddr);

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                     Clear a Token Ring Group Address                      */
/*---------------------------------------------------------------------------*/

int
   tokClearGroupAddr(
      register DEV         * dev         ,// IO-Device
      register CHN         * chn         ,// IO-Channel
      register ndd_t       * nddp        ,// IO-NDD
      register unsigned int  groupAddr    // I -Group Address (lsw)
   )
{
   FUNC_NAME(tokClearGroupAddr);

   char                      addr[6] = { 0xC0,0x00,0x00,0x00,0x00,0x00 };
   int                       pSave;       // Interrupt Priority Save Area
   int                       rc;

   *(unsigned int *)(&addr[2]) = groupAddr;

   rc = NDD_CTL(nddp,NDD_DISABLE_ADDRESS,(caddr_t)&addr,6);

   if (rc == 0)
   {
      dev->ds.tok.groupAddr   = 0;
      dev->ds.tok.groupOwner  = 0;
      dev->ds.tok.groupActive = 0;
      chn->ds.tok.groupActive = 0;
   }
   else
      TRC_OTHER(tcga,rc,chn,groupAddr);

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                       Set a Group Address for FDDI                        */
/*---------------------------------------------------------------------------*/

int
   fddiGroupAddr(
      register DEV         * dev         ,// I -Device
      register CHN         * chn         ,// I -Channel
      register void        * argp        ,// IO-Parameter Block
      register ulong         devflag      // I -Device Flags from open
   )
{
   FUNC_NAME(fddiGroupAddr);

   fddi_set_addr_t           arg;        // Local copy of parameter block
   int                       rc  = 0;
   int                       xrc = 0;
   ADDRTABLE               * g;

   /*---------------------------------------------*/
   /*  Copy the parameter block to local storage  */
   /*---------------------------------------------*/

   if ((xrc = copyin((caddr_t)argp,(caddr_t)&arg,sizeof(arg))) != 0)
   {
      TRC_OTHER(fgaa,xrc,argp,&arg);
      return EFAULT;
   }

   arg.status = 0;

   /*--------------------------------*/
   /*  Processing depends on opcode  */
   /*--------------------------------*/

   switch(arg.opcode)
   {
      /*-----------------------*/
      /*  Add a group address  */
      /*-----------------------*/

      case FDDI_ADD:
      {
         rc = enableGroup(dev->nddp,
                          dev->ds.fddi.groupTable,
                          chn->ds.fddi.groupRef,
                          arg.addr);
         break;
      }

      /*--------------------------*/
      /*  Remove a group address  */
      /*--------------------------*/

      case FDDI_DEL:
      {
         rc = disableGroup(dev->nddp,
                           dev->ds.fddi.groupTable,
                           chn->ds.fddi.groupRef,
                           arg.addr);

         if (rc == ENOCONNECT)
         {
            arg.status = FDDI_NO_ADDR;
            rc         = 0;
         }

         break;
      }

      /*-------------------*/
      /*  Invalid command  */
      /*-------------------*/

      default:
      {
         TRC_OTHER(fgac,arg.opcode,0,0);
         arg.status = CIO_INV_CMD;
         break;
      }
   }

   if (rc == 0)
   {
      if ((xrc = copyout((caddr_t)&arg,(caddr_t)argp,sizeof(arg))) != 0)
      {
         TRC_OTHER(fgab,xrc,&arg,argp);
         return EFAULT;
      }
   }

   return (rc ? rc : (arg.status ? ENOMSG : 0));
}

/*---------------------------------------------------------------------------*/
/*              Disable all fddi group addresses for a channel               */
/*---------------------------------------------------------------------------*/

int
   fddiClearAllGroups(
      register DEV         * dev         ,// IO-Device
      register CHN         * chn          // IO-Channel
   )
{
   FUNC_NAME(fddiClearAllGroups);

   int                       index;
   int                       rc = 0;

   rc = disableAllGroups(dev->nddp,
                         dev->ds.fddi.groupTable,
                         chn->ds.fddi.groupRef);

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                      Query Group Addresses for FDDI                       */
/*---------------------------------------------------------------------------*/

int
   fddiQueryGroupAddr(
      register DEV         * dev         ,// I -Device
      register CHN         * chn         ,// I -Channel
      register void        * argp        ,// IO-Parameter Block
      register ulong         devflag      // I -Device Flags from open
   )
{
   FUNC_NAME(fddiQueryGroupAddr);

#define  ADDR_BUF_SIZE       4096

   fddi_query_addr_t         arg;
   char                      addrBuf[ADDR_BUF_SIZE];

   int                       rc  = 0;
   int                       xrc = 0;

   /*---------------------------------------*/
   /*  Make sure we have a started channel  */
   /*---------------------------------------*/

   if (qEmpty(chn->sesQueue))
   {
      TRC_OTHER(fqgs,0,0,0);
      arg.status = CIO_NOT_STARTED;
      goto quit;
   }

   /*----------------------------------------------------*/
   /*  Initialize local copy of parameter block to zero  */
   /*----------------------------------------------------*/

   memset(&arg,0x00,sizeof arg);

   /*---------------------------------------*/
   /*  Retrieve the address list from CDLI  */
   /*---------------------------------------*/

   rc = NDD_CTL(
           dev->nddp,                    // NDD Pointer
           NDD_MIB_ADDR,                 // IOCTL Function
           &addrBuf,                     // -> output buffer
           ADDR_BUF_SIZE);               // Size of output buffer

   /*-----------------------------------------------------*/
   /*  If the IOCTL was successful, map the addresses to  */
   /*  the COMIO IOCTL parameter block format             */
   /*-----------------------------------------------------*/

   if (rc == 0)
   {
      ndd_mib_addr_elem_t  * cdliAddr;
      int                    i;

      for (
            i = 0, cdliAddr = first_mib_addr(&addrBuf);
            i < mib_addr_count(&addrBuf);
            i++, cdliAddr = next_mib_addr(cdliAddr)
          )
      {
         /*--------------------------------*/
         /*  Process only valid addresses  */
         /*--------------------------------*/

         if (cdliAddr->status != NDD_MIB_INVALID)
         {
            rc = mapNetAddress(
                  CIE_DEV_FDDI,              // Device Type
                  CDLI_TO_CIO,               // Mapping Direction
                  sizeof(arg.addrs[0]),      // COMIO Address Length
                  arg.addrs[arg.addr_cnt],   // COMIO Address
                  cdliAddr->addresslen,      // CDLI Address Length
                  cdliAddr->address);        // CDLI Address

            if (rc == 0) arg.addr_cnt++;
         }
      }
   }
   else
      TRC_OTHER(fqgx,rc,0,0);

quit:
   if ((xrc = copyout((caddr_t)&arg,(caddr_t)argp,sizeof(arg))) != 0)
   {
      TRC_OTHER(fqgb,xrc,&arg,argp);
      return EFAULT;
   }

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                     Turn on Ethernet Promiscuous Mode                     */
/*---------------------------------------------------------------------------*/

int
   entPromiscuousOn(
      register DEV         * dev         ,// I -Device
      register CHN         * chn         ,// I -Channel
      register void        * argp        ,// IO-Parameter Block
      register ulong         devflag      // I -Device Flags from open
   )
{
   FUNC_NAME(entPromiscuousOn);

   int                       rc = 0;

   /*-------------------------------------------------------*/
   /*  Enable Promiscuous Mode only if not already enabled  */
   /*-------------------------------------------------------*/

   if (dev->ds.ent.proMode == 0)
   {
      rc = NDD_CTL(dev->nddp,NDD_PROMISCUOUS_ON,NULL,0);
      if (rc) TRC_OTHER(epnx,rc,dev,chn);
   }

   /*-----------------------------------------*/
   /*  Update reference counts if successful  */
   /*-----------------------------------------*/

   if (rc == 0)
   {
      dev->ds.ent.proMode++;
      chn->ds.ent.proMode++;
   }

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                    Turn off Ethernet Promiscuous Mode                     */
/*---------------------------------------------------------------------------*/

int
   entPromiscuousOff(
      register DEV         * dev         ,// I -Device
      register CHN         * chn         ,// I -Channel
      register void        * argp        ,// IO-Parameter Block
      register ulong         devflag      // I -Device Flags from open
   )
{
   FUNC_NAME(entPromiscuousOff);

   int                       rc = 0;

   /*---------------------------------------------------------*/
   /*  Disable Promiscuous Mode only if reference count == 1  */
   /*---------------------------------------------------------*/

   if (chn->ds.ent.proMode == 0)
   {
      TRC_OTHER(epfo,dev,chn,0);
      rc = EINVAL;
   }
   else if (chn->ds.ent.proMode == 1)
   {
      rc = NDD_CTL(dev->nddp,NDD_PROMISCUOUS_OFF,NULL,0);
      if (rc) TRC_OTHER(epfx,rc,dev,chn);
   }
   else
      /* Not Yet */;

   if (rc == 0)
   {
      dev->ds.ent.proMode--;
      chn->ds.ent.proMode--;
   }

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                      Start a session on the channel                       */
/*---------------------------------------------------------------------------*/

int
   startSession(
      register DEV            * dev         ,// IO-Device Block
      register CHN            * chn         ,// IO-Channel Block
      register cio_sess_blk_t * uarg        ,// IO-IOCTL Command Parameters
      register ulong            devflag      // I -Device Flags from Open
   )
{
   FUNC_NAME(startSession);

   register SES            * ses;
   int                       rc  = 0;
   int                       xrc = 0;
   int                       pSave;       // Interrupt Priority Save Area

   cio_sess_blk_t            arg;

   ns_user_t                 user;
   extern void               cierecv();

   /*-----------------------------------------*/
   /*  Copy parameter block to local storage  */
   /*-----------------------------------------*/

   if ((xrc = copyin((caddr_t)uarg,(caddr_t)&arg,sizeof(arg))) != 0)
   {
      TRC_OTHER(sssa,xrc,uarg,&arg);
      return EFAULT;
   }

   /*---------------------------------*/
   /*  Initialize return status code  */
   /*---------------------------------*/

   arg.status = 0;

   /*------------------------------------*/
   /*  Build the NS 'ns_user' structure  */
   /*------------------------------------*/

   user.isr        = cierecv;
   user.isr_data   = (caddr_t) chn;
   user.protoq     = NULL;
   user.pkt_format = NS_INCLUDE_MAC;
   user.netisr     = 0;
   user.ifp        = NULL;

   /*----------------------------------------------------*/
   /*  Create the new session and add it to the channel  */
   /*----------------------------------------------------*/

   if ((ses = new_SES(dev->dds.devType,arg.length,arg.netid)) == NULL)
      return ENOMEM;

   lockChannel(pSave,chn);
   qPush(chn->sesQueue,ses);
   unlockChannel(pSave,chn);

   /*-----------------------------------------------*/
   /*  Enable the appropriate ns demuxer filter(s)  */
   /*-----------------------------------------------*/

   rc = enableSessionFilters(dev,chn,ses,&user);

   /*--------------------------------------*/
   /*  Check results of filter enablement  */
   /*--------------------------------------*/

   if (rc == 0)
   {
      STATBLK              * s;

      /*---------------------------------------------------*/
      /*  Generate a START_DONE status block and queue it  */
      /*---------------------------------------------------*/

      s = mkStartDoneStatus(dev,arg.netid);

      lockChannel(pSave,chn);
      queueStatus(chn,s);
      unlockChannel(pSave,chn);

      /*--------------------------------------------------*/
      /*  Update the high-session counter for the device  */
      /*--------------------------------------------------*/

      dev->numSes++;

      setHWM(dev->stats.sesMax,dev->numSes);
   }
   else
   {
      TRC_OTHER(sssx,rc,chn,arg.netid);

      /*-------------------------------------*/
      /*  Remove the session and destroy it  */
      /*-------------------------------------*/

      lockChannel(pSave,chn);
      qPop(chn->sesQueue,void);
      unlockChannel(pSave,chn);

      destroy_SES(ses);

      /*------------------------------------------------------------*/
      /*  Fill in status and convert return code to expected value  */
      /*------------------------------------------------------------*/

      switch(rc)
      {
         case EEXIST:
         case EADDRINUSE:
         case EALREADY:
         {
            arg.status = CIO_NETID_DUP;
            rc         = EADDRINUSE;
            break;
         }

         case EINVAL:
         {
            arg.status = CIO_NETID_INV;
            rc         = EINVAL;
            break;
         }

         default:
         {
            arg.status = 0;
            break;
         }
      }
   }

   /*------------------------------------------------*/
   /*  Return updated parameter block to user space  */
   /*------------------------------------------------*/

   if ((xrc = copyout((caddr_t)&arg,(caddr_t)uarg,sizeof(*uarg))) != 0)
   {
      TRC_OTHER(sssb,xrc,&arg,uarg);
      return EFAULT;
   }

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                       Halt a session on the channel                       */
/*---------------------------------------------------------------------------*/

int
   haltSession(
      register DEV            * dev         ,// IO-Device Block
      register CHN            * chn         ,// IO-Channel Block
      register cio_sess_blk_t * uarg        ,// IO-IOCTL Command Parameters
      register ulong            devflag      // I -Device Flags from Open
   )
{
   FUNC_NAME(haltSession);

   int                       rc  = 0;
   int                       xrc = 0;

   cio_sess_blk_t            arg;

   /*-----------------------------------------*/
   /*  Copy parameter block to local storage  */
   /*-----------------------------------------*/

   if ((xrc = copyin((caddr_t)uarg,(caddr_t)&arg,sizeof(arg))) != 0)
   {
      TRC_OTHER(hssa,xrc,uarg,&arg);
      return EFAULT;
   }

   /*------------------------------------------------*/
   /*  Make sure there is at least one open session  */
   /*------------------------------------------------*/

   if (!qEmpty(chn->sesQueue))
   {
      register SES         * ses;
      register QUEUE       * sq = chn->sesQueue;
      CURSOR                 c;
      SES                    key;

      /*--------------------------------------------------------*/
      /*  Create a dummy session block to search session queue  */
      /*--------------------------------------------------------*/

      initSesBlock(&key,dev->dds.devType,arg.length,arg.netid);

      /*---------------------------------------------------*/
      /*  Search the session queue for a matching session  */
      /*---------------------------------------------------*/

      for(ses = qFirst(sq,c,SES); ses; ses = qNext(sq,c,SES))
      {
         if (memcmp((caddr_t)ses,(caddr_t)&key,offsetof(SES,ds)) == 0) break;
      }

      /*----------------------------*/
      /*  Did we find the session?  */
      /*----------------------------*/

      if (ses)
      {
         STATBLK           * s;
         int                 pSave;       // Interrupt Priority Save Area

         /*----------------------------------------------------*/
         /*  Disable the filters associated with this session  */
         /*----------------------------------------------------*/

         disableSessionFilters(dev,chn,ses);

         /*---------------------------------------------*/
         /*  Disable functional address for token ring  */
         /*---------------------------------------------*/

         if (dev->dds.devType == CIE_DEV_TOK && ses->ds.tok.funcAddr != 0)
            tokClearFuncAddr(ses,dev->nddp,ses->ds.tok.funcAddr);

         /*--------------------------------------*/
         /*  Generate a start_done status block  */
         /*--------------------------------------*/

         s = mkHaltDoneStatus(dev,arg.netid);

         /*-------------------------------------------------*/
         /*  Remove the SES and queue the HALT_DONE status  */
         /*-------------------------------------------------*/

         lockChannel(pSave,chn);

         qDelete(sq,c);
         queueStatus(chn,s);

         unlockChannel(pSave,chn);

         /*-------------------------*/
         /*  Destroy the SES block  */
         /*-------------------------*/

         destroy_SES(ses);

         dev->numSes--;
      }
      else
      {
         TRC_OTHER(hssn,chn,arg.netid,0);
         rc = EINVAL;
      }
   }
   else
   {
      TRC_OTHER(hsse,0,0,0);
      rc = EINVAL;
   }

   /*------------------------------------------------*/
   /*  Return updated parameter block to user space  */
   /*------------------------------------------------*/

   if ((xrc = copyout((caddr_t)&arg,(caddr_t)uarg,sizeof(*uarg))) != 0)
   {
      TRC_OTHER(hssb,xrc,&arg,uarg);
      return EFAULT;
   }

   arg.status = (rc==0 ? 0 : CIO_NETID_INV);

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                       Terminate all active sessions                       */
/*---------------------------------------------------------------------------*/

void
   haltAllSessions(
      DEV                  * dev         ,// IO-Device
      CHN                  * chn          // IO-Channel
   )
{
   FUNC_NAME(haltAllSessions);

   int                       pSave;       // Interrupt Priority Save Area
   QUEUE                   * q;
   SES                     * ses;

   lockChannel(pSave,chn);

   /*------------------------------------------------*/
   /*  Pop sessions off the queue and halt each one  */
   /*------------------------------------------------*/

   while((ses = qPop(chn->sesQueue,SES)) != NULL)
   {
      unlockChannel(pSave,chn);

      /*-------------------------------*/
      /*  Disable all session filters  */
      /*-------------------------------*/

      disableSessionFilters(dev,chn,ses);

      /*---------------------------------------------*/
      /*  Disable functional address for token ring  */
      /*---------------------------------------------*/

      if (dev->dds.devType == CIE_DEV_TOK && ses->ds.tok.funcAddr != 0)
         tokClearFuncAddr(ses,dev->nddp,ses->ds.tok.funcAddr);

      /*-----------------------*/
      /*  Destroy the session  */
      /*-----------------------*/

      destroy_SES(ses);

      lockChannel(pSave,chn);
   }

   unlockChannel(pSave,chn);
}
/*---------------------------------------------------------------------------*/
/*                     Turn on Ethernet Signal Support                     */
/*---------------------------------------------------------------------------*/

int
   entSignalSupport(
      register DEV         * dev         ,// I -Device
      register CHN         * chn         ,// I -Channel
      register void        * argp        ,// IO-Parameter Block
      register ulong         devflag      // I -Device Flags from open
   )
{
   FUNC_NAME(entSignalSupport);

   chn->rcv_sig++;
   chn->proc_id = getpid();

}
