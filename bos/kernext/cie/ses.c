static char sccsid[]="@(#)31   1.7  src/bos/kernext/cie/ses.c, sysxcie, bos411, 9428A410j 4/18/94 16:22:05";

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 *   initSesBlock
 *   new_SES
 *   destroy_SES
 *   tokSetFuncAddr
 *   tokClearFuncAddr
 *
 * DESCRIPTION:
 *
 *    Session Object Implementation
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

#include "dev.h"
#include "chn.h"
#include "ses.h"
#include "nsdmx.h"
#include "dmalloc.h"

/*---------------------------------------------------------------------------*/
/*                        Initialize a Session Block                         */
/*---------------------------------------------------------------------------*/

void
   initSesBlock(
      register SES         * ses         ,//  O-Ptr to output Session Block
      register CIE_DEV_TYPE  devType     ,// I -Device Type Code
      register ushort        netidLength ,// I -Length of net id (1 or 2)
      register netid_t       netid        // I -Network ID
   )
{
   FUNC_NAME(initSesBlock);

   /*--------------*/
   /*  Initialize  */
   /*--------------*/

   memset(ses,0x00,sizeof(*ses));

   strcpy(ses->iCatcher,"SES");

   switch(devType)
   {
      case CIE_DEV_FDDI:

         if (netid == FDDI_SMT_NETID)
            ses->type = SES_FDDI_SMT;
         else if (netid < 0x0100)
            ses->type = SES_DSAP;
         else
         {
            TRC_OTHER(isfu,devType,netidLength,netid);
            ses->type = SES_UNKNOWN;
         }

         netidLength = 0;

         break;

      case CIE_DEV_TOK:

         if (netid == TOK_MAC_FRAME_NETID)
            ses->type = SES_TOK_MAC;
         else if (netid < 0x0100)
            ses->type = SES_DSAP;
         else
         {
            TRC_OTHER(istu,devType,netidLength,netid);
            ses->type = SES_UNKNOWN;
         }

         netidLength = 0;

         break;

      case CIE_DEV_ENT:

         if (netidLength == 2)
            ses->type = SES_ETHERTYPE;
         else if (netidLength == 1)
            ses->type = SES_DSAP;
         else
         {
            TRC_OTHER(iseu,devType,netidLength,netid);
            ses->type = SES_UNKNOWN;
         }

         break;

      default:
         TRC_OTHER(isuu,devType,netidLength,netid);
         ses->type = SES_UNKNOWN;
   }

   ses->netidLength = netidLength;
   ses->netid       = netid;
}

/*---------------------------------------------------------------------------*/
/*                  Create a new Session Block (struct SES)                  */
/*---------------------------------------------------------------------------*/

SES *
   new_SES(
      register CIE_DEV_TYPE  devType     ,// I -Device Type Code
      register ushort        netidLength ,// I -Length of net id (1 or 2)
      register netid_t       netid        // I -Network ID
   )
{
   FUNC_NAME(new_SES);

   SES                     * ses;

   /*----------------------------------------*/
   /*  Allocate memory for the sesion block  */
   /*----------------------------------------*/

   if ((ses = xmalloc(sizeof(SES),2,pinned_heap)) == NULL)
      XMFAIL(sizeof(SES));
   else
      initSesBlock(ses,devType,netidLength,netid);

   return ses;
}

/*---------------------------------------------------------------------------*/
/*                          Destroy a session block                          */
/*---------------------------------------------------------------------------*/

void
   destroy_SES(
      register SES         * ses          // IO-The session block
   )
{
   FUNC_NAME(destroy_SES);

   if (xmfree(ses,pinned_heap) != 0) XFFAIL();
}

/*---------------------------------------------------------------------------*/
/*           Enable a Token Ring functional address for a session            */
/*---------------------------------------------------------------------------*/

int
   tokSetFuncAddr(
      register SES         * ses         ,// IO-Session
      register const ndd_t * nddp        ,// I -NDD
      register unsigned int  addrBits     // I -Functional Address Bits
   )
{
   int                       rc = 0;

   char                      funcAddr[6] = { 0xC0,0x00,0x00,0x00,0x00,0x00 };

   /*-----------------------------------------------------*/
   /*  Put functional address bits into complete address  */
   /*-----------------------------------------------------*/

   /*
      The following statement masks out any functional address bits
      already owned by this session.  This is because cdli provides
      a bitwise reference count for EACH bit in the functional
      address.  If a user were to set the same bit twice for the
      same session, cdli would increment the reference count
      twice and then when we terminated it would be decremented
      only once.  To avoid having to keep our own reference counters
      we just won't set a given bit more than once per session.
   */

   *(unsigned int *)(&funcAddr[2]) = (addrBits & ~ses->ds.tok.funcAddr);

   /*----------------------------------------------*/
   /*  Invoke ndd_ctl to place address on adapter  */
   /*----------------------------------------------*/

   rc = NDD_CTL(nddp,NDD_ENABLE_ADDRESS,(caddr_t)&funcAddr,6);

   /*--------------------------------------------------------------*/
   /*  If result was OK, update session's functional address bits  */
   /*--------------------------------------------------------------*/

   if (rc == 0)
      ses->ds.tok.funcAddr |= addrBits;
   else
      TRC_OTHER(tsfa,rc,ses,addrBits);

   return rc;
}

/*---------------------------------------------------------------------------*/
/*           Disable a Token Ring functional address for a session           */
/*---------------------------------------------------------------------------*/

int
   tokClearFuncAddr(
      register SES         * ses         ,// IO-Session
      register const ndd_t * nddp        ,// I -NDD
      register unsigned int  addrBits     // I -Functional Address Bits
   )
{
   int                       rc = 0;

   char                      funcAddr[6] = { 0xC0,0x00,0x00,0x00,0x00,0x00 };

   /*-----------------------------------------------------*/
   /*  Put functional address bits into complete address  */
   /*-----------------------------------------------------*/

   /*
      The following code prevents the user from inadvertently
      turning off functional address bits he doesn't own.  This
      is necessary because cdli will blindly decrement its
      reference counters for a given bit regardless of whether
      or not the user doing the disabling is the one who originally
      enabled the bit.  Thus we must mask the address to only the
      bits currently owned by this session.
   */

   *(unsigned int *)(&funcAddr[2]) = (addrBits & ses->ds.tok.funcAddr);

   /*----------------------------------------------*/
   /*  Invoke ndd_ctl to place address on adapter  */
   /*----------------------------------------------*/

   rc = NDD_CTL(nddp,NDD_DISABLE_ADDRESS,(caddr_t)&funcAddr,6);

   /*--------------------------------------------------------------*/
   /*  If result was OK, update session's functional address bits  */
   /*--------------------------------------------------------------*/

   if (rc == 0)
      ses->ds.tok.funcAddr &= ~addrBits;
   else
      TRC_OTHER(tcfa,rc,ses,addrBits);

   return rc;
}
