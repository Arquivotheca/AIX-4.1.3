/* @(#)47   1.6  src/bos/kernext/cie/devtype.h, sysxcie, bos411, 9428A410j 4/1/94 15:49:19 */

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 * DESCRIPTION:
 *
 *    COMIO Emulator Device Type Codes
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

#if !defined(DEVTYPE_H)
#define  DEVTYPE_H

/*---------------------------------------------------------------------------*/
/*                    COMIO Emulator * Device Type Codes                     */
/*---------------------------------------------------------------------------*/

typedef enum CIE_DEV_TYPE    CIE_DEV_TYPE;

enum CIE_DEV_TYPE
{
   CIE_DEV_NULL      = -1,                    // Device Type not set
   CIE_DEV_TOK       =  0,                    // Token Ring
   CIE_DEV_FDDI      =  1,                    // FDDI
   CIE_DEV_ENT       =  2,                    // Ethernet (all types)
   CIE_DEV_MAX       =  3
};

/*---------------------------------------------------------------------------*/
/*                           Device SubType Codes                            */
/*---------------------------------------------------------------------------*/

typedef enum CIE_DEV_SUB_TYPE CIE_DEV_SUB_TYPE;

enum CIE_DEV_SUB_TYPE
{
   CIE_DEV_SUB_NULL        = -1,             // No subtype needed
   CIE_DEV_SUB_NA          =  0,             // No subtype needed
   CIE_DEV_SUB_3COM        =  1,             // 3COM Ethernet
   CIE_DEV_SUB_IENT        =  2              // Integrated Ethernet
};

#endif
