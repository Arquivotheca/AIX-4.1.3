/* @(#)56   1.6  src/bos/kernext/cie/statistics.h, sysxcie, bos411, 9428A410j 4/1/94 15:51:59 */

/* 
 * 
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 * 
 * FUNCTIONS:
 * 
 *   mapTokMonStats
 *   mapFddiStats
 *   mapEnt3ComStats
 *   mapEntIentStats
 * 
 * DESCRIPTION:
 * 
 *    Device Statistics
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

#if !defined(MAPSTATS_H)
#define  MAPSTATS_H

#include <sys/ndd.h>
#include <sys/cdli.h>
#include <sys/cdli_tokuser.h>
#include <sys/cdli_entuser.h>
#include <sys/cdli_fddiuser.h>
#include <sys/fddi_demux.h>
#include <sys/tok_demux.h>
#include <sys/eth_demux.h>

/*---------------------------------------------------------------------------*/
/*               Convert Token Ring Status from CDLI to COMIO                */
/*---------------------------------------------------------------------------*/

void
   mapTokMonStats(
      register       tok_query_stats_t * t3    ,//  O-COMIO TR Statistics
      register const DEVSTATS          * ds    ,// I -CIOEM Device Statistics
      register const mon_all_stats_t   * t4    ,// I -CDLI TR Statistics
      register const nd_dmxstats_t     * dmx   ,// I -CDLI Generic Demux Stats
      register const tok_dmx_stats_t   * dmxTok // I -CDLI TR Demux Stats
   );

/*---------------------------------------------------------------------------*/
/*                Convert FDDI Statistics from CDLI to COMIO                 */
/*---------------------------------------------------------------------------*/

void
   mapFddiStats(
      register       fddi_query_stats_t * f3     ,//  O-COMIO FDDI Statistics
      register const DEVSTATS           * ds     ,// I -CIOEM Device Statistics
      register const fddi_ndd_stats_t   * f4     ,// I -CDLI FDDI Statistics
      register const nd_dmxstats_t      * dmx    ,// I -CDLI Generic Demux Stats
      register const fddi_dmx_stats_t   * dmxFddi // I -CDLI FDDI Demux Stats
   );

/*---------------------------------------------------------------------------*/
/*            Convert Ethernet 3Com Statistics from CDLI to COMIO            */
/*---------------------------------------------------------------------------*/

void
   mapEnt3ComStats(
      register       ent_query_stats_t  * e3    ,//  O-COMIO 3Com Ent Statistics
      register const DEVSTATS           * ds    ,// I -CIOEM Device Statistics
      register const en3com_all_stats_t * e4    ,// I -CDLI 3Com Statistics
      register const nd_dmxstats_t      * dmx   ,// I -CDLI Generic Demux Stats
      register const eth_dmx_stats_t    * dmxEnt // I -CDLI 3Com Demux Stats
   );

/*---------------------------------------------------------------------------*/
/*         Convert Integrated Ethernet Statistics from CDLI to COMIO         */
/*---------------------------------------------------------------------------*/

void
   mapEntIentStats(
      register       ent_query_stats_t * e3    ,//  O-COMIO Ient Statistics
      register const DEVSTATS          * ds    ,// I -Emulator Device Statistics
      register const ient_all_stats_t  * e4    ,// I -CDLI Ient Statistics
      register const nd_dmxstats_t     * dmx   ,// I -CDLI Generic Demux Stats
      register const eth_dmx_stats_t   * dmxEnt // I -CDLI Ient Demux Stats
   );

#endif
