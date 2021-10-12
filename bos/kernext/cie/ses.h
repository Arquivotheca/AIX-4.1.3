/* @(#)55   1.6  src/bos/kernext/cie/ses.h, sysxcie, bos411, 9428A410j 4/1/94 15:51:27 */

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
 *    Session Object External Interface
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

#if !defined(SES_H)
#define  SES_H

#include <sys/ndd.h>
#include <sys/comio.h>
#include <sys/ciouser.h>
#include <sys/err_rec.h>
#include <sys/entuser.h>
#include <sys/fddiuser.h>
#include <sys/tokuser.h>

#include "sestype.h"

/*---------------------------------------------------------------------------*/
/*                          Session Block Structure                          */
/*---------------------------------------------------------------------------*/

struct SES
{
   char                      iCatcher[4]; // Eye Catcher
   SES_TYPE                  type;
   ushort                    netidLength;
   netid_t                   netid;

   /*--------------------------------*/
   /*  Device-Specific Session Data  */
   /*--------------------------------*/

   union
   {
      /*--------*/
      /*  FDDI  */
      /*--------*/

      struct
      {
         int                 SMT_correlator[FDDI_SMT_MAX_FILTERS];
                                                // Correlators for SMT Filters
      } fddi;

      /*--------------*/
      /*  Token Ring  */
      /*--------------*/

      struct
      {
         unsigned int        funcAddr;          // Functional Address Bits
      } tok;

      /*------------*/
      /*  Ethernet  */
      /*------------*/

      struct
      {
         int                 unused;            // Nothing at present
      } ent;

   } ds;
};

/*---------------------------------------------------------------------------*/
/*                        Initialize a Session Block                         */
/*---------------------------------------------------------------------------*/

void
   initSesBlock(
      register SES         * ses         ,//  O-Ptr to output Session Block
      register CIE_DEV_TYPE  devType     ,// I -Device Type Code
      register ushort        netidLength ,// I -Length of net id (1 or 2)
      register netid_t       netid        // I -Network ID
   );

/*---------------------------------------------------------------------------*/
/*                  Create a new Session Block (struct SES)                  */
/*---------------------------------------------------------------------------*/

SES *
   new_SES(
      register CIE_DEV_TYPE  devType     ,// I -Device Type Code
      register ushort        netidLength ,// I -Length of net id (1 or 2)
      register netid_t       netid        // I -Network ID
   );

/*---------------------------------------------------------------------------*/
/*                          Destroy a session block                          */
/*---------------------------------------------------------------------------*/

void
   destroy_SES(
      register SES         * ses          // IO-The session block
   );

/*---------------------------------------------------------------------------*/
/*           Enable a Token Ring functional address for a session            */
/*---------------------------------------------------------------------------*/

int
   tokSetFuncAddr(
      register SES         * ses         ,// IO-Session
      register const ndd_t * nddp        ,// I -NDD
      register unsigned int  addrBits     // I -Functional Address Bits
   );

/*---------------------------------------------------------------------------*/
/*           Disable a Token Ring functional address for a session           */
/*---------------------------------------------------------------------------*/

int
   tokClearFuncAddr(
      register SES         * ses         ,// IO-Session
      register const ndd_t * nddp        ,// I -NDD
      register unsigned int  addrBits     // I -Functional Address Bits
   );

#endif
