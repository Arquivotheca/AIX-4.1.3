/* @(#)49   1.6  src/bos/kernext/cie/filter.h, sysxcie, bos411, 9428A410j 4/1/94 15:49:59 */

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

#if !defined(FILTER_H)
#define      FILTER_H

/*---------------------------------------------------------------------------*/
/*                            Build a DSAP filter                            */
/*---------------------------------------------------------------------------*/

void
   mk_DSAP_filter(
      ns_8022_t            * f           ,//  O-Ptr to return area
      netid_t                netid        // I -Net ID
   );

/*---------------------------------------------------------------------------*/
/*                         Build an ethertype filter                         */
/*---------------------------------------------------------------------------*/

void
   mk_ethertype_filter(
      ns_8022_t            * f           ,//  O-Ptr to return area
      netid_t                netid        // I -Net ID
   );

/*---------------------------------------------------------------------------*/
/*   Build a standard ns_8022 filter from devType, netid and netid length    */
/*---------------------------------------------------------------------------*/

void
   mk_FDDI_SMT_filter(
      fddi_dmx_filter_t    * f           ,//  O-Ptr to return area
      ushort                 ftype        // I -Filter Type Code
   );

/*---------------------------------------------------------------------------*/
/*   Build a standard ns_8022 filter from devType, netid and netid length    */
/*---------------------------------------------------------------------------*/

void
   mk_TOK_MAC_filter(
      ns_8022_t            * f            //  O-Ptr to return area
   );

#endif
