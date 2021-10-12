/* @(#)51   1.6  src/bos/kernext/cie/mapaddr.h, sysxcie, bos411, 9428A410j 4/1/94 15:50:11 */

/* 
 * 
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 * 
 * FUNCTIONS:
 * 
 *   mapNetAddress
 * 
 * DESCRIPTION:
 * 
 *    Network Address Mapping
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

#if !defined(MAPADDR_H)
#define  MAPADDR_H

#include "devtype.h"

/*---------------------------------------------------------------------------*/
/*                Enumeration for Address mapping directions                 */
/*---------------------------------------------------------------------------*/

typedef enum MAP_ADDR_DIR    MAP_ADDR_DIR;

enum  MAP_ADDR_DIR
{
   CDLI_TO_CIO,
   CIO_TO_CDLI
};

/*---------------------------------------------------------------------------*/
/*            Map a network address between CIO and CDLI formats             */
/*---------------------------------------------------------------------------*/

int
   mapNetAddress(
      CIE_DEV_TYPE           devType     ,// I -Device Type Code
      MAP_ADDR_DIR           direction   ,// I -Mapping Direction
      ushort                 cioLen      ,// I -Length of COMIO-format address
      char                 * cioAddr     ,// I -Ptr to COMIO-format address
      ushort                 cdliLen     ,// I -Length of CDLI-format address
      char                 * cdliAddr     //  O-Ptr to CDLI-format address
   );

#endif
