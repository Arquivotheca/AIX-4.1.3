static char sccsid[]="@(#)20   1.7  src/bos/kernext/cie/ciestat.c, sysxcie, bos411, 9428A410j 4/18/94 16:21:00";

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 *   ciestat
 *
 * DESCRIPTION:
 *
 *    COMIO Emulator Reveived-Status ISR
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
#include "status.h"

#include <sys/ndd.h>
#include <sys/cdli.h>
#include <sys/mbuf.h>
#include <sys/poll.h>

#include "nsdmx.h"

/*---------------------------------------------------------------------------*/
/*              Receive a status block on the interrupt thread               */
/*---------------------------------------------------------------------------*/

void
   ciestat(
      struct ndd           * ndd         ,// I -NDD Pointer
      const ndd_statblk_t  * stat        ,// I -Status Block
      CHN                  * chn          // I -isr_data (channel address)
   )
{
   STATBLK                 * s;

   int                       pSave;       // Interrupt Priority Save
   int                       rc = 0;

   /*----------------------*/
   /*  Validate arguments  */
   /*----------------------*/

   if (chn == NULL)
   {
      TRC_OTHER(insa,chn,ndd,stat);
      return;
   }

   if (memcmp(chn->iCatcher,"CHN\0",4) != 0)
   {
      TRC_RECV(insc,chn,ndd,stat);
      return;
   }

   /*-------------------------------------------------------------*/
   /*  If we are in the process of closing this channel, discard  */
   /*-------------------------------------------------------------*/

   if (!chn->open)
   {
      TRC_RECV(inso,chn,ndd,stat);
      return;
   }

   /*--------------------*/
   /*  Lock the channel  */
   /*--------------------*/

   lockChannel(pSave,chn);

   /*------------------------------------------------------------*/
   /*  Encapsulate the status block and queue it to the channel  */
   /*------------------------------------------------------------*/

   if ((s = mkCDLIStatus(stat)) == NULL)
   {
      TRC_OTHER(insn,0,0,0);
      chn->lostStatus = 1;
   }
   else
      queueStatus(chn,s);

   /*----------------------*/
   /*  Unlock the channel  */
   /*----------------------*/

   unlockChannel(pSave,chn);

   return;
}
