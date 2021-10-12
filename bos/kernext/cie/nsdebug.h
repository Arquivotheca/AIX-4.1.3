/* @(#)15   1.2  src/bos/kernext/cie/nsdebug.h, sysxcie, bos411, 9428A410j 4/1/94 15:50:36 */

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 *   NDD_OUTPUT
 *   NDD_CTL
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

#if !defined(NSDEBUG_H)
#define  NSDEBUG_H

#if defined(NSDEBUG)

#include <sys/cdli.h>
#include <sys/mbuf.h>
#include <sys/ndd.h>

#define  ns_alloc            dbg_ns_alloc
#define  ns_free             dbg_ns_free
#define  ns_add_filter       dbg_ns_add_filter
#define  ns_del_filter       dbg_ns_del_filter
#define  ns_add_status       dbg_ns_add_status
#define  ns_del_status       dbg_ns_del_status

#define  NDD_OUTPUT(n,m)     dbg_ndd_output((n),(m))
#define  NDD_CTL(n,c,a,l)    dbg_ndd_ctl((n),(c),(a),(l))

/*---------------------------------------------------------------------------*/
/*                        Debug wrapper for ns_alloc                         */
/*---------------------------------------------------------------------------*/

int
   dbg_ns_alloc(
      char                 * nddname     ,
      ndd_t               ** nddpp
   );

/*---------------------------------------------------------------------------*/
/*                         Debug wrapper for ns_free                         */
/*---------------------------------------------------------------------------*/

void
   dbg_ns_free(
      ndd_t                * nddp
   );

/*---------------------------------------------------------------------------*/
/*                      Debug wrapper for ns_add_filter                      */
/*---------------------------------------------------------------------------*/

int
   dbg_ns_add_filter(
      ndd_t                * nddp        ,
      caddr_t                filter      ,
      int                    len         ,
      struct ns_user       * user
   );

/*---------------------------------------------------------------------------*/
/*                      Debug wrapper for ns_del_filter                      */
/*---------------------------------------------------------------------------*/

int
   dbg_ns_del_filter(
      ndd_t                * nddp        ,
      caddr_t                filter      ,
      int                    len
   );

/*---------------------------------------------------------------------------*/
/*                      Debug wrapper for ns_add_status                      */
/*---------------------------------------------------------------------------*/

int
   dbg_ns_add_status(
      ndd_t                * nddp        ,
      caddr_t                statfilter  ,
      int                    len         ,
      struct ns_statuser   * statuser
   );

/*---------------------------------------------------------------------------*/
/*                      Debug wrapper for ns_del_status                      */
/*---------------------------------------------------------------------------*/

int
   dbg_ns_del_status(
      ndd_t                * nddp        ,
      caddr_t                statfilter  ,
      int                    len
   );

/*---------------------------------------------------------------------------*/
/*                       Debug wrapper for ndd_output                        */
/*---------------------------------------------------------------------------*/

int
   dbg_ndd_output(
      ndd_t                * nddp        ,
      struct mbuf          * m
   );

/*---------------------------------------------------------------------------*/
/*                         Debug wrapper for ndd_ctl                         */
/*---------------------------------------------------------------------------*/

int
   dbg_ndd_ctl(
      ndd_t                * nddp        ,
      int                    cmd         ,
      caddr_t                arg         ,
      int                    len
   );

#else

#define  NDD_OUTPUT(n,m)     ((n->ndd_output)((n),(m)))
#define  NDD_CTL(n,c,a,l)    ((n->ndd_ctl)((n),(c),(a),(l)))

#endif

#endif
