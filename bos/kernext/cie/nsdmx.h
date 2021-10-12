/* @(#)53   1.6  src/bos/kernext/cie/nsdmx.h, sysxcie, bos411, 9428A410j 4/1/94 15:50:51 */

/* 
 * 
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 * 
 * FUNCTIONS:
 * 
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

#if !defined(NSDMX_H)
#define      NSDMX_H

/*---------------------------------------------------------------------------*/
/*                           Enable Status Receipt                           */
/*---------------------------------------------------------------------------*/

int
   enableStatus(
      register ndd_t                * nddp        ,// IO-The NDD
      register uint                 * psid        ,//  O-Ptr to sid
      register const ns_statuser_t  * user         // I -User Structure
   );

/*---------------------------------------------------------------------------*/
/*                          Disable Status Receipt                           */
/*---------------------------------------------------------------------------*/

int
   disableStatus(
      register ndd_t       * nddp        ,// IO-The NDD
      register unsigned int* psid         //  O-Ptr to sid
   );

/*---------------------------------------------------------------------------*/
/*              Enable the ndd demuxer filter(s) for a Session               */
/*---------------------------------------------------------------------------*/

int
   enableSessionFilters(
      register DEV         * dev         ,// IO-Device
      register CHN         * chn         ,// IO-Channel
      register SES         * ses         ,// IO-Session Block
      register ns_user_t   * user         // IO-User Definition
   );

/*---------------------------------------------------------------------------*/
/*              Disable the ndd demuxer filter(s) for a Session              */
/*---------------------------------------------------------------------------*/

int
   disableSessionFilters(
      register DEV         * dev         ,// IO-Device
      register CHN         * chn         ,// IO-Channel
      register SES         * ses          // IO-Session Block
   );

#endif
