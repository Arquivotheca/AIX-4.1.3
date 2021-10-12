static char sccsid[]="@(#)14   1.4  src/bos/kernext/cie/nsdebug.c, sysxcie, bos411, 9428A410j 4/18/94 16:21:48";

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 *   dbg_ns_alloc
 *   dbg_ns_free
 *   dbg_ns_add_filter
 *   dbg_ns_del_filter
 *   dbg_ns_add_status
 *   dbg_ns_del_status
 *   dbg_ndd_output
 *   dbg_ndd_ctl
 *
 * DESCRIPTION:
 *
 *   NS Wrapper for Debugging
 *
 * ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <sys/cdli.h>
#include <sys/mbuf.h>
#include <sys/ndd.h>

#include "ciedd.h"
#include "nsdebug.h"

#undef  ns_alloc
#undef  ns_free
#undef  ns_add_filter
#undef  ns_del_filter
#undef  ns_add_status
#undef  ns_del_status

/*---------------------------------------------------------------------------*/
/*                        Debug wrapper for ns_alloc                         */
/*---------------------------------------------------------------------------*/

int
   dbg_ns_alloc(
      char                 * nddname     ,
      ndd_t               ** nddpp
   )
{
   register int              rc;

   dbgout("===> ns_alloc %s -> %x",nddname,nddpp);
   rc = ns_alloc(nddname,nddpp);
   dbgout("<=== rc=%d nddp=%x",rc,*nddpp);

   TRC_OTHER(NSAL,rc,nddname,nddpp);

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                         Debug wrapper for ns_free                         */
/*---------------------------------------------------------------------------*/

void
   dbg_ns_free(
      ndd_t                * nddp
   )
{
   register int              rc;

   dbgout("===> ns_free nddp=%x",nddp);
   ns_free(nddp);
   dbgout("<=== ns_free done");

   TRC_OTHER(NSFR,rc,nddp,0);
}

/*---------------------------------------------------------------------------*/
/*                      Debug wrapper for ns_add_filter                      */
/*---------------------------------------------------------------------------*/

int
   dbg_ns_add_filter(
      ndd_t                * nddp        ,
      caddr_t                filter      ,
      int                    len         ,
      struct ns_user       * user
   )
{
   register int              rc;
   char                      buf[1024];

   dbgout("===> ns_add_filter nddp=%x",nddp);

   dbgout("     filter=%x length=%d",filter,len);

   if (filter && len > 0)
   {
      memDump(buf,5,16,dumpRel,filter,min(len,128));
      dbgout(buf);
   }

   dbgout("     user=%x",user);
   memDump(buf,5,16,dumpRel,user,sizeof(struct ns_user));
   dbgout(buf);

   rc = ns_add_filter(nddp,filter,len,user);

   dbgout("<=== rc=%d",rc);

   TRC_OTHER(NSAF,rc,nddp,user);
   TRC_OTHER(NSAG,filter,len,0);

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                      Debug wrapper for ns_del_filter                      */
/*---------------------------------------------------------------------------*/

int
   dbg_ns_del_filter(
      ndd_t                * nddp        ,
      caddr_t                filter      ,
      int                    len
   )
{
   register int              rc;
   char                      buf[1024];

   dbgout("===> ns_del_filter nddp=%x",nddp);

   dbgout("     filter=%x length=%d",filter,len);

   if (filter && len > 0)
   {
      memDump(buf,5,16,dumpRel,filter,min(len,128));
      dbgout(buf);
   }

   rc = ns_del_filter(nddp,filter,len);

   dbgout("<=== rc=%d",rc);

   TRC_OTHER(NSDF,rc,nddp,0);
   TRC_OTHER(NSDG,filter,len,0);

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                      Debug wrapper for ns_add_status                      */
/*---------------------------------------------------------------------------*/

int
   dbg_ns_add_status(
      ndd_t                * nddp        ,
      caddr_t                statfilter  ,
      int                    len         ,
      struct ns_statuser   * statuser
   )
{
   register int              rc;
   char                      buf[1024];

   dbgout("===> ns_add_status nddp=%x",nddp);

   dbgout("     filter=%x length=%d",statfilter,len);

   if (statfilter && len > 0)
   {
      memDump(buf,5,16,dumpRel,statfilter,min(len,128));
      dbgout(buf);
   }

   dbgout("     statuser=%x",statuser);
   memDump(buf,5,16,dumpRel,statuser,sizeof(struct ns_statuser));
   dbgout(buf);

   rc = ns_add_status(nddp,statfilter,len,statuser);

   dbgout("<=== rc=%d sid=%x",rc,((ns_com_status_t *)(statfilter))->sid);

   TRC_OTHER(NSAS,rc,nddp,statuser);
   TRC_OTHER(NSAT,statfilter,len,0);

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                      Debug wrapper for ns_del_status                      */
/*---------------------------------------------------------------------------*/

int
   dbg_ns_del_status(
      ndd_t                * nddp        ,
      caddr_t                statfilter  ,
      int                    len
   )
{
   register int              rc;
   char                      buf[1024];

   dbgout("===> ns_del_status nddp=%x",nddp);

   dbgout("     filter=%x length=%d",statfilter,len);

   if (statfilter && len > 0)
   {
      memDump(buf,5,16,dumpRel,statfilter,min(len,128));
      dbgout(buf);
   }

   rc = ns_del_status(nddp,statfilter,len);

   dbgout("<=== rc=%d",rc);

   TRC_OTHER(NSDS,rc,nddp,0);
   TRC_OTHER(MSDT,statfilter,len,0);

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                       Debug wrapper for ndd_output                        */
/*---------------------------------------------------------------------------*/

int
   dbg_ndd_output(
      ndd_t                * nddp        ,
      struct mbuf          * m
   )
{
   register int              rc;
   char                      buf[1024];

/*{
char buf[1024];
memDump(buf,5,16,dumpRel,m,128);
dbgout(buf);
}*/
   rc = (nddp->ndd_output)(nddp,m);

   TRC_OTHER(NDDO,rc,nddp,m);

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                         Debug wrapper for ndd_ctl                         */
/*---------------------------------------------------------------------------*/

int
   dbg_ndd_ctl(
      ndd_t                * nddp        ,
      int                    cmd         ,
      caddr_t                arg         ,
      int                    len
   )
{
   register int              rc = 0;
   char                      buf[1024];

   dbgout("===> ndd_ctl nddp=%x cmd=%d",nddp,cmd);
   dbgout("     arg=%x length=%d",arg,len);

   if (arg != NULL && len > 0)
   {
      memDump(buf,5,16,dumpRel,arg,min(len,128));
      dbgout(buf);
   }

   rc = (nddp->ndd_ctl)(nddp,cmd,arg,len);

   dbgout("<=== rc=%d",rc);

   TRC_OTHER(NDCT,rc,nddp,0);
   TRC_OTHER(NDCU,cmd,arg,len);

   return rc;
}
