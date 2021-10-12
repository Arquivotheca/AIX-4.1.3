static char sccsid[]="@(#)29   1.7  src/bos/kernext/cie/nsdmx.c, sysxcie, bos411, 9428A410j 4/18/94 16:21:54";

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 *   enable_FDDI_SMT
 *   disable_FDDI_SMT
 *   enable_TOK_MAC
 *   disable_TOK_MAC
 *   enable_ethertype
 *   disable_ethertype
 *   enable_DSAP
 *   disable_DSAP
 *   enableStatus
 *   disableStatus
 *   enableSessionFilters
 *   disableSessionFilters
 *
 * DESCRIPTION:
 *
 *    NS Interface Routines
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

#include <sys/comio.h>
#include <sys/ciouser.h>
#include <sys/err_rec.h>
#include <sys/entuser.h>
#include <sys/fddiuser.h>
#include <sys/tokuser.h>
#include <sys/cdli.h>
#include <sys/ndd.h>
#include <sys/device.h>
#include <sys/comio.h>
#include <sys/mbuf.h>
#include <stdarg.h>
#include <sys/tok_demux.h>
#include <sys/eth_demux.h>

#define  fddi_hdr cfddi_dmxhdr
#include <sys/fddi_demux.h>
#undef   fddi_hdr

#include "dev.h"
#include "chn.h"
#include "ses.h"
#include "filter.h"
#include "nsdmx.h"
#include "dmalloc.h"

/*---------------------------------------------------------------------------*/
/*                      Enable the SMT Net ID for FDDI                       */
/*---------------------------------------------------------------------------*/

static
int
   enable_FDDI_SMT(
      register ndd_t              * nddp ,// IO-NDD
      register SES                * ses  ,// IO-Session
      register ns_user_t          * user ,// IO-ns_user structure
      register const FDDI_SMT_DEF * fdef  // I -SMT Filter Definition
   )
{
   FUNC_NAME(enable_FDDI_SMT);

   int                       i;
   int                       rc   = 0;

   /*-------------------------------------------------------------------*/
   /*  Enable all Demuxer filters necessary to emulate the old process  */
   /*-------------------------------------------------------------------*/

   for (i=0; rc == 0 && i < fdef->numFilters; i++)
   {
      fddi_dmx_filter_t      f;

      /*---------------------*/
      /*  Create the filter  */
      /*---------------------*/

      mk_FDDI_SMT_filter(&f,fdef->filter[i]);

      /*-------------------------*/
      /*  Add it to the demuxer  */
      /*-------------------------*/

      rc = ns_add_filter(nddp,(caddr_t)&f,sizeof(f),user);

      if (rc == 0)
         ses->ds.fddi.SMT_correlator[i] = f.id;
      else
         TRC_OTHER(fesx,rc,ses,fdef);
   }

   /*---------------------*/
   /*  Check return code  */
   /*---------------------*/

   if (rc != 0)
   {
      /*-----------------------------*/
      /*  Clean up before returning  */
      /*-----------------------------*/

      disable_FDDI_SMT(nddp,ses,fdef);
   }

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                      Disable the SMT Net ID for FDDI                      */
/*---------------------------------------------------------------------------*/

static
int
   disable_FDDI_SMT(
      register ndd_t              * nddp ,// IO-NDD
      register SES                * ses  ,// IO-Session
      register const FDDI_SMT_DEF * fdef  // I -SMT Filter Definition
   )
{
   FUNC_NAME(disable_FDDI_SMT);

   int                       i;
   int                       rc   = 0;

   /*--------------------------------------------------------------------*/
   /*  Disable all Demuxer filters necessary to emulate the old process  */
   /*--------------------------------------------------------------------*/

   for (i=0; i < fdef->numFilters; i++)
   {
      fddi_dmx_filter_t      f;

      /*---------------------*/
      /*  Create the filter  */
      /*---------------------*/

      mk_FDDI_SMT_filter(&f,fdef->filter[i]);

      f.id = ses->ds.fddi.SMT_correlator[i];

      /*------------------------------*/
      /*  Remove it from the demuxer  */
      /*------------------------------*/

      rc = ns_del_filter(nddp,(caddr_t)&f,sizeof(f));

      if (rc) TRC_OTHER(fdsx,rc,ses,fdef);
   }

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                   Enable the MAC Net ID for Token Ring                    */
/*---------------------------------------------------------------------------*/

static
int
   enable_TOK_MAC(
      register ndd_t       * nddp        ,// IO-NDD
      register SES         * ses         ,// IO-Session
      register ns_user_t   * user         // IO-ns_user structure
   )
{
   FUNC_NAME(enable_TOK_MAC);

   ns_8022_t                 f;
   int                       rc = 0;

   /*-----------------------------*/
   /*  Create a MAC Frame Filter  */
   /*-----------------------------*/

   mk_TOK_MAC_filter(&f);

   /*-------------------------*/
   /*  Add it to the demuxer  */
   /*-------------------------*/

   rc = ns_add_filter(nddp,(caddr_t)&f,sizeof(f),user);

   if (rc) TRC_OTHER(femx,rc,ses,0);

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                   Disable the MAC Net ID for Token Ring                   */
/*---------------------------------------------------------------------------*/

static
int
   disable_TOK_MAC(
      register ndd_t       * nddp        ,// IO-NDD
      register SES         * ses          // IO-Session
   )
{
   FUNC_NAME(disable_TOK_MAC);

   ns_8022_t                 f;
   int                       rc = 0;

   /*-----------------------------*/
   /*  Create a MAC Frame Filter  */
   /*-----------------------------*/

   mk_TOK_MAC_filter(&f);

   /*------------------------------*/
   /*  Remove it from the demuxer  */
   /*------------------------------*/

   rc = ns_del_filter(nddp,(caddr_t)&f,sizeof(f));

   if (rc) TRC_OTHER(fdmx,rc,ses,0);

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                        Enable an Ethertype Filter                         */
/*---------------------------------------------------------------------------*/

static
int
   enable_ethertype(
      register ndd_t       * nddp        ,// IO-NDD
      register SES         * ses         ,// IO-Session
      register ns_user_t   * user         // IO-ns_user structure
   )
{
   FUNC_NAME(enable_ethertype);

   ns_8022_t                 f;
   int                       rc = 0;

   /*------------------------------*/
   /*  Create an Ethertype filter  */
   /*------------------------------*/

   mk_ethertype_filter(&f,ses->netid);

   /*-------------------------*/
   /*  Add it to the demuxer  */
   /*-------------------------*/

   rc = ns_add_filter(nddp,(caddr_t)&f,sizeof(f),user);

   if (rc) TRC_OTHER(feex,rc,ses,0);

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                       Disable an Ethertype Filter                         */
/*---------------------------------------------------------------------------*/

static
int
   disable_ethertype(
      register ndd_t       * nddp        ,// IO-NDD
      register SES         * ses          // IO-Session
   )
{
   FUNC_NAME(disable_ethertype);

   ns_8022_t                 f;
   int                       rc = 0;

   /*------------------------------*/
   /*  Create an Ethertype Filter  */
   /*------------------------------*/

   mk_ethertype_filter(&f,ses->netid);

   /*------------------------------*/
   /*  Remove it from the demuxer  */
   /*------------------------------*/

   rc = ns_del_filter(nddp,(caddr_t)&f,sizeof(f));

   if (rc) TRC_OTHER(fdex,rc,ses,0);

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                           Enable a DSAP Filter                            */
/*---------------------------------------------------------------------------*/

static
int
   enable_DSAP(
      register ndd_t       * nddp        ,// IO-NDD
      register SES         * ses         ,// IO-Session
      register ns_user_t   * user         // IO-ns_user structure
   )
{
   FUNC_NAME(enable_DSAP);

   ns_8022_t                 f;
   int                       rc = 0;

   /*------------------------------*/
   /*  Create an Ethertype filter  */
   /*------------------------------*/

   mk_DSAP_filter(&f,ses->netid);

   /*-------------------------*/
   /*  Add it to the demuxer  */
   /*-------------------------*/

   rc = ns_add_filter(nddp,(caddr_t)&f,sizeof(f),user);

   if (rc) TRC_OTHER(fedx,rc,ses,0);

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                           Disable a DSAP Filter                           */
/*---------------------------------------------------------------------------*/

static
int
   disable_DSAP(
      register ndd_t       * nddp        ,// IO-NDD
      register SES         * ses          // IO-Session
   )
{
   FUNC_NAME(disable_DSAP);

   ns_8022_t                 f;
   int                       rc = 0;

   /*------------------------------*/
   /*  Create an Ethertype Filter  */
   /*------------------------------*/

   mk_DSAP_filter(&f,ses->netid);

   /*------------------------------*/
   /*  Remove it from the demuxer  */
   /*------------------------------*/

   rc = ns_del_filter(nddp,(caddr_t)&f,sizeof(f));

   if (rc) TRC_OTHER(fddx,rc,ses,0);

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                           Enable Status Receipt                           */
/*---------------------------------------------------------------------------*/

int
   enableStatus(
      register ndd_t                * nddp        ,// IO-The NDD
      register uint                 * psid        ,//  O-Ptr to sid
      register const ns_statuser_t  * user         // I -User Structure
   )
{
   FUNC_NAME(enableStatus);

   #define  statusFilterCode (NDD_HARD_FAIL   | \
                              NDD_LIMBO_ENTER | \
                              NDD_LIMBO_EXIT  | \
                              NDD_STATUS)

   ns_com_status_t           filter;
   int                       rc = 0;

   /*------------------------------*/
   /*  Initialize the filter data  */
   /*------------------------------*/

   filter.filtertype = NS_STATUS_MASK;
   filter.mask       = statusFilterCode;
   filter.sid        = 0;

   /*--------------------------------------------------*/
   /*  Enable status receipt for all applicable codes  */
   /*--------------------------------------------------*/

   rc = ns_add_status(nddp,&filter,sizeof(filter),user);

   /*--------------------------------------------------------------*/
   /*  If something failed undo enable; otherwise save correlator  */
   /*--------------------------------------------------------------*/

   if (rc)
   {
      TRC_OTHER(feax,rc,nddp,&filter);
      ns_del_status(nddp,&filter,sizeof(filter));
   }
   else
      *psid = filter.sid;

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                          Disable Status Receipt                           */
/*---------------------------------------------------------------------------*/

int
   disableStatus(
      register ndd_t       * nddp        ,// IO-The NDD
      register unsigned int* psid         //  O-Ptr to sid
   )
{
   FUNC_NAME(disableStatus);

   ns_com_status_t           filter;
   int                       rc = 0;

   /*------------------------------*/
   /*  Initialize the filter data  */
   /*------------------------------*/

   filter.filtertype = NS_STATUS_MASK;
   filter.mask       = statusFilterCode;
   filter.sid        = *psid;

   /*---------------------------------------------------*/
   /*  Disable status receipt for all applicable codes  */
   /*---------------------------------------------------*/

   rc = ns_del_status(nddp,&filter,sizeof(filter));

   if (rc) TRC_OTHER(fdax,rc,nddp,&filter);

   *psid = 0;

   return rc;
}

/*---------------------------------------------------------------------------*/
/*              Enable the ndd demuxer filter(s) for a Session               */
/*---------------------------------------------------------------------------*/

int
   enableSessionFilters(
      register DEV         * dev         ,// IO-Device
      register CHN         * chn         ,// IO-Channel
      register SES         * ses         ,// IO-Session Block
      register ns_user_t   * user         // IO-User Definition
   )
{
   FUNC_NAME(enableSessionFilters);

   int                       rc;

   switch(ses->type)
   {
      /*--------------------*/
      /*  FDDI SMT Session  */
      /*--------------------*/

      case SES_FDDI_SMT:
      {
         if (dev->ds.fddi.smtActive)
         {
            TRC_OTHER(fesu,dev,ses->type,0);
            rc = EADDRINUSE;
         }
         else
         {
            if ((rc = enable_FDDI_SMT(dev->nddp,
                                      ses,
                                      user,
                                      &dev->ds.fddi.smtDef)) == 0)
            {
               dev->ds.fddi.smtActive = 1;
               chn->ds.fddi.smtActive = 1;
            }
         }

         break;
      }

      /*--------------------------*/
      /*  Token Ring MAC Session  */
      /*--------------------------*/

      case SES_TOK_MAC:
      {
         if (dev->ds.tok.macActive)
         {
            TRC_OTHER(femu,dev,ses->type,0);
            rc = EADDRINUSE;
         }
         else
         {
            if ((rc = enable_TOK_MAC(dev->nddp,ses,user)) == 0)
            {
               dev->ds.tok.macActive = 1;
               chn->ds.tok.macActive = 1;
            }
         }

         break;
      }

      /*------------------------------*/
      /*  Ethernet Ethertype session  */
      /*------------------------------*/

      case SES_ETHERTYPE:
      {
         rc = enable_ethertype(dev->nddp,ses,user);
         break;
      }

      /*--------------------*/
      /*  Any DSAP session  */
      /*--------------------*/

      case SES_DSAP:
      {
         rc = enable_DSAP(dev->nddp,ses,user);
         break;
      }

      /*---------------------------------------------------------*/
      /*  Configuration of device-type/netid-length was invalid  */
      /*---------------------------------------------------------*/

      case SES_UNKNOWN:
      {
         TRC_OTHER(feux,dev,0,0);
         rc = EINVAL;
         break;
      }

      /*------------------------------------------*/
      /*  This should theoretically never happen  */
      /*------------------------------------------*/

      default:
      {
         TRC_OTHER(feix,dev,0,0);
         break;
      }
   }

   return rc;
}

/*---------------------------------------------------------------------------*/
/*              Disable the ndd demuxer filter(s) for a Session              */
/*---------------------------------------------------------------------------*/

int
   disableSessionFilters(
      register DEV         * dev         ,// IO-Device
      register CHN         * chn         ,// IO-Channel
      register SES         * ses          // IO-Session Block
   )
{
   FUNC_NAME(disableSessionFilters);

   int                       rc;

   switch(ses->type)
   {
      /*--------------------*/
      /*  FDDI SMT Session  */
      /*--------------------*/

      case SES_FDDI_SMT:
      {
         if (!dev->ds.fddi.smtActive)
         {
            TRC_OTHER(fdsu,dev,chn,ses);
            rc = EFAULT;
         }
         else
         {
            if ((rc = disable_FDDI_SMT(dev->nddp,
                                       ses,
                                       &dev->ds.fddi.smtDef)) == 0)
            {
               dev->ds.fddi.smtActive = 0;
               chn->ds.fddi.smtActive = 0;
            }
         }

         break;
      }

      /*--------------------------*/
      /*  Token Ring MAC Session  */
      /*--------------------------*/

      case SES_TOK_MAC:
      {
         if (!dev->ds.tok.macActive)
         {
            TRC_OTHER(fdmu,dev,chn,ses);
            rc = EFAULT;
         }
         else
         {
            if ((rc = disable_TOK_MAC(dev->nddp,ses)) == 0)
            {
               dev->ds.tok.macActive = 0;
               chn->ds.tok.macActive = 0;
            }
         }

         break;
      }

      /*------------------------------*/
      /*  Ethernet Ethertype session  */
      /*------------------------------*/

      case SES_ETHERTYPE:
      {
         rc = disable_ethertype(dev->nddp,ses);
         break;
      }

      /*--------------------*/
      /*  Any DSAP session  */
      /*--------------------*/

      case SES_DSAP:
      {
         rc = disable_DSAP(dev->nddp,ses);
         break;
      }

      /*---------------------------------------------------------*/
      /*  Configuration of device-type/netid-length was invalid  */
      /*---------------------------------------------------------*/

      case SES_UNKNOWN:
      {
         TRC_OTHER(fdux,dev,chn,ses);
         rc = EINVAL;
         break;
      }

      /*------------------------------------------*/
      /*  This should theoretically never happen  */
      /*------------------------------------------*/

      default:
      {
         TRC_OTHER(fdix,dev,chn,ses);
         break;
      }
   }

   return rc;
}
