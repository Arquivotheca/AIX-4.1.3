static char sccsid[]="@(#)26   1.7  src/bos/kernext/cie/filter.c, sysxcie, bos411, 9428A410j 4/18/94 16:21:42";

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 *   mk_DSAP_filter
 *   mk_ethertype_filter
 *   mk_FDDI_SMT_filter
 *   mk_TOK_MAC_filter
 *
 * DESCRIPTION:
 *
 *    NS Demuxer Filter Tracking
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

#include <sys/types.h>
#include <stddef.h>
#include <errno.h>
#include <sys/comio.h>
#include <sys/cdli.h>
#include <sys/fddi_demux.h>
#include <sys/tok_demux.h>
#include <sys/malloc.h>

#include "devtype.h"
#include "trccap.h"
#include "filter.h"
#include "dmalloc.h"

/*---------------------------------------------------------------------------*/
/*                            Build a DSAP filter                            */
/*---------------------------------------------------------------------------*/

void
   mk_DSAP_filter(
      ns_8022_t            * f           ,//  O-Ptr to return area
      netid_t                netid        // I -Net ID
   )
{
   FUNC_NAME(mk_DSAP_filter);

   memset(f,0x00,sizeof(*f));

   f->filtertype = NS_8022_LLC_DSAP;
   f->dsap       = (u_char)(netid & 0x00FF);

}

/*---------------------------------------------------------------------------*/
/*                         Build an ethertype filter                         */
/*---------------------------------------------------------------------------*/

void
   mk_ethertype_filter(
      ns_8022_t            * f           ,//  O-Ptr to return area
      netid_t                netid        // I -Net ID
   )
{
   FUNC_NAME(mk_DSAP_filter);

   memset(f,0x00,sizeof(*f));

   f->filtertype = NS_ETHERTYPE;
   f->ethertype  = netid;

}

/*---------------------------------------------------------------------------*/
/*   Build a standard ns_8022 filter from devType, netid and netid length    */
/*---------------------------------------------------------------------------*/

void
   mk_FDDI_SMT_filter(
      fddi_dmx_filter_t    * f           ,//  O-Ptr to return area
      ushort                 ftype        // I -Filter Type Code
   )
{
   FUNC_NAME(mk_FDDI_SMT_filter);

   memset(f,0x00,sizeof(*f));

   f->filter.filtertype = ftype;

}

/*---------------------------------------------------------------------------*/
/*   Build a standard ns_8022 filter from devType, netid and netid length    */
/*---------------------------------------------------------------------------*/

void
   mk_TOK_MAC_filter(
      ns_8022_t            * f            //  O-Ptr to return area
   )
{
   FUNC_NAME(mk_TOK_MAC_filter);

   memset(f,0x00,sizeof(*f));

   f->filtertype = TOK_DEMUX_MAC;

}
