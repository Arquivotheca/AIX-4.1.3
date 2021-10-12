/* @(#)74   1.6  src/bos/kernext/cie/sestype.h, sysxcie, bos411, 9428A410j 4/1/94 15:51:45 */

/* 
 * 
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 * 
 * FUNCTIONS:
 * 
 * DESCRIPTION:
 * 
 *    Session Type Codes
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

#if !defined(SESTYPE_H)
#define  SESTYPE_H

/*---------------------------------------------------------------------------*/
/*                            Session Type Codes                             */
/*---------------------------------------------------------------------------*/

typedef enum   SES_TYPE        SES_TYPE;  // Session Type Code

enum SES_TYPE
{
   SES_UNKNOWN,                           // Unknown Session Type
   SES_FDDI_SMT,                          // FDDI SMT Session
   SES_TOK_MAC,                           // Token Ring MAC Session
   SES_ETHERTYPE,                         // Ethernet Ethertype Session
   SES_DSAP                               // DSAP Session (any media)
};

#endif
