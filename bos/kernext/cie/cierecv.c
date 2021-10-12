static char sccsid[]="@(#)19   1.9  src/bos/kernext/cie/cierecv.c, sysxcie, bos411, 9438B411a 9/19/94 18:10:26";

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 *   cierecv
 *
 * DESCRIPTION:
 *
 *    COMIO Emulator Received-Data ISR
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
#include "dev.h"
#include "chn.h"
#include "ses.h"

#include <sys/cdli.h>
#include <sys/ndd.h>
#include <sys/mbuf.h>
#include <sys/poll.h>

#include "nsdmx.h"
#include "addrtbl.h"
#include "dmalloc.h"
#include "mbqueue.inl"

/*---------------------------------------------------------------------------*/
/*              Determine if an ethernet address is multi-cast               */
/*---------------------------------------------------------------------------*/

static
int
   isMulticast(
      const char           * p            // I -Ethernet address
   )
{
   return ((*p & MULTI_BIT_MASK) == MULTI_BIT_MASK);
}

/*---------------------------------------------------------------------------*/
/*       Check to see if an address is equal to the broadcast address        */
/*---------------------------------------------------------------------------*/

static
int
   isBroadcast(
      const char           * p           ,// I -Net Address
      int                    l            // I -Address Length
   )
{
   int                       i;
   int                       bcast = 1;

   for (i=0; i<l && bcast; i++) bcast = (*p++ == 0xFF);

   return bcast;
}

/*---------------------------------------------------------------------------*/
/*        Check to see if an address is an active multicast for a chn        */
/*---------------------------------------------------------------------------*/

static
int
   activeMultiCastAddr(
      const char           * addr        ,// I -Address to be checked
      int                    len         ,// I -Address length
      const ADDRTABLE      * tbl         ,// I -Multicast Address Table
      const int              ref[]        // I -Channel MC Addr Reference
   )
{
   int                       i;
   int                       match = 0;

   for (i=0; i<tbl->tSize && !match; i++)
      match = ((ref[i] > 0) && (memcmp(ePtr(tbl,i)->addr,addr,len) == 0));

   return match;
}

/*---------------------------------------------------------------------------*/
/*              Handle a received frame on the interrupt level               */
/*---------------------------------------------------------------------------*/

void
   cierecv(
      ndd_t                * ndd         ,// I -NDD Pointer
      mbuf_t               * m           ,// I -Mbuf chain containing data
      caddr_t                macp        ,// I -Ptr to start of packet in m
      CHN                  * chn          // I -isr_data (channel address)
   )
{
   FUNC_NAME(cierecv);

   DEV                       * dev;       // Device

   int                       discard = 0; // Packet is to be discarded
   int                       pSave;       // Interrupt Priority Save
   int                       rc      = 0;

   /*----------------------*/
   /*  Validate arguments  */
   /*----------------------*/

   if (chn == NULL)
   {
      TRC_RECV(intn,chn,ndd,m);
      return;
   }

   if (memcmp(chn->iCatcher,"CHN\0",4) != 0)
   {
      TRC_RECV(intc,chn,ndd,m);
      return;
   }

   if (m->m_nextpkt != NULL)
   {
      TRC_RECV(indm,chn,ndd,m);
      return;
   }

   /*-----------------------------------------------------*/
   /*  Get pointer to the device object for this channel  */
   /*-----------------------------------------------------*/

   dev = chn->dev;

#if defined(DMALLOC)
{
   /*-------------------------------------------------------*/
   /*  Register the received mbuf chain in the debug table  */
   /*-------------------------------------------------------*/

   mbuf_t                  * p;
   mbuf_t                  * q;

   for (p=m; p; p=p->m_nextpkt)
      for (q=p; q; q=q->m_next)
         d_save(q,q->m_len,MBUF,__FILE__,__FUNC__,__LINE__);
}
#endif

   /*-------------------------------------------------------------*/
   /*  If we are in the process of closing this channel, discard  */
   /*-------------------------------------------------------------*/

   if (!chn->open)
   {
      TRC_RECV(into,chn,ndd,m);
      m_freem(m);
      return;
   }

   /*--------------------*/
   /*  Lock the channel  */
   /*--------------------*/

   lockChannel(pSave,chn);

   /*-------------------------------------------------------------------*/
   /*  Special handling for Ethernet when the device is in promiscuous  */
   /*  mode but the receiving channel was not the one requesting        */
   /*  promiscuous mode -- we have to filter out some packets           */
   /*-------------------------------------------------------------------*/

   discard = 0;

   if (dev->dds.devType    == CIE_DEV_ENT &&
       dev->ds.ent.proMode >  0           &&
       chn->ds.ent.proMode == 0             )
   {
      char *                  destAddr = mtod(m,char *);

      if (isMulticast(destAddr))
      {
         /*-------------------------------------------*/
         /*  Packet is either broadcast or multicast  */
         /*-------------------------------------------*/

         if (isBroadcast(destAddr,ent_NADR_LENGTH))
            /* accept all broadcast packets */;
         else if
         (
            activeMultiCastAddr(destAddr,
                                ent_NADR_LENGTH,
                                dev->ds.ent.multiCastTable,
                                chn->ds.ent.multiCastRef)
         )
            /* accept a packet matching an active multicast address */;
         else
            discard = 1;
      }
      else
      {
         /*---------------------------------------------------------------*/
         /*  Packet is not broad/multi-cast - Match to current adpt addr  */
         /*---------------------------------------------------------------*/

         discard = (memcmp(destAddr,dev->ds.ent.curAddr,ent_NADR_LENGTH) != 0);
      }
   }

   /*---------------------------------------------------------*/
   /*  Enqueue the mbuf chain on the channel's receive queue  */
   /*---------------------------------------------------------*/

   if (discard)
   {
      /*-------------------------------------*/
      /*  Act as if we didn't see the frame  */
      /*-------------------------------------*/

      TRC_RECV(RCVX,0,0,0);

      m_freem(m);
   }
   else if (mbqQueue(&chn->rdq,m) != NULL)
   {
      /*------------------------------------------------*/
      /*  If anybody's blocked on a read, wake them up  */
      /*------------------------------------------------*/

      if (chn->rdqNotEmpty != EVENT_NULL)
         e_wakeup(&(chn->rdqNotEmpty));

      /*------------------------------------------------*/
      /*  If anybody's needs a signal, send it  */
      /*------------------------------------------------*/

      if (chn->rcv_sig)
         pidsig(chn->proc_id,SIGIO);

      /*-------------------------------------------------------------*/
      /*  Do a selnotify in case anyone's waiting on select(POLLIN)  */
      /*-------------------------------------------------------------*/

      selnotify(chn->devno,chn->chan,POLLIN);

      /*----------------------------------------*/
      /*  Record receive-queue high water mark  */
      /*----------------------------------------*/

      setHWM(dev->stats.rdqMax,mbqHWM(&chn->rdq));
   }
   else
   {
      /*-------------------------------------------*/
      /*  Could not queue the packet - discard it  */
      /*-------------------------------------------*/

      TRC_RECV(intq,0,0,0);
      fetch_and_add(&dev->stats.rdqOvfl,1);
      m_freem(m);
   }

   /*----------------------*/
   /*  Unlock the Channel  */
   /*----------------------*/

   unlockChannel(pSave,chn);

   return;
}
