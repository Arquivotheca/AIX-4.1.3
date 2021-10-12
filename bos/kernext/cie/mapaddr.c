static char sccsid[]="@(#)28   1.7  src/bos/kernext/cie/mapaddr.c, sysxcie, bos411, 9428A410j 4/18/94 16:21:45";

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 *   min
 *   max
 *   tokFGtoCDLI
 *   findAddrMap
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

#include <stddef.h>
#include <string.h>
#include <sys/errno.h>

#include "ciedd.h"
#include "mapaddr.h"
#include "trccap.h"
#include "dmalloc.h"

#define  min(x,y)            (((x) <= (y)) ? (x) : (y))
#define  max(x,y)            (((x) >= (y)) ? (x) : (y))

/*---------------------------------------------------------------------------*/
/*               Type Definition for Address Mapping Function                */
/*---------------------------------------------------------------------------*/

typedef int                  MAP_ADDR_FUNC(
                                ushort      srcLen,
                                char      * src,
                                ushort      dstLen,
                                char      * dst
                             );

/*---------------------------------------------------------------------------*/
/*                        Address Mapping Definition                         */
/*---------------------------------------------------------------------------*/

typedef struct ADDR_MAP      ADDR_MAP;

struct ADDR_MAP
{
   CIE_DEV_TYPE              devType;     // Device Type Code
   ushort                    cioLen;      // Length of CIO-format Address
   ushort                    cdliLen;     // Length of CDLI-format Address
   MAP_ADDR_FUNC           * cioToCdli;   // Mapping Function CIO  -> CDLI
   MAP_ADDR_FUNC           * cdliToCio;   // Mapping Function CDLI -> CIO
};

/*---------------------------------------------------------------------------*/
/*  Map a COMIO Token Ring 4-byte (Func/Group) Address to CDLI 6-byte format */
/*---------------------------------------------------------------------------*/

static
int
   tokFGtoCDLI(
      ushort                 cioLen      ,// I -CIO-format length
      char                 * cioAddr     ,// I -CIO-format address
      ushort                 cdliLen     ,// I -CDLI-format length
      char                 * cdliAddr     //  O-CDLI-format address
   )
{
   cdliAddr[0] = 0xC0;
   cdliAddr[1] = 0x00;

   memcpy(cdliAddr+2,cioAddr,4);

   return 0;
}

/*---------------------------------------------------------------------------*/
/*           Search an address mapping table for a matching entry            */
/*---------------------------------------------------------------------------*/

static
ADDR_MAP *
   findAddrMap(
      const ADDR_MAP       * map         ,// I -The address mapping table
      CIE_DEV_TYPE           devType     ,// I -Device Type Code
      ushort                 cioLen      ,// I -COMIO-format addr length
      ushort                 cdliLen      // I -CDLI=format addr length
   )
{
   const ADDR_MAP          * a;

   for (a=map; a->devType != CIE_DEV_NULL; a++)
   {
      if(a->devType==devType && a->cioLen==cioLen && a->cdliLen==cdliLen)
         return a;
   }

   return NULL;
}

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
   )
{
   FUNC_NAME(mapNetAddress);

   static ADDR_MAP           addrMap[] =
   {
      {CIE_DEV_TOK  , 4  , 6  , tokFGtoCDLI  , NULL        },
      {CIE_DEV_TOK  , 6  , 6  , NULL         , NULL        },
      {CIE_DEV_ENT  , 6  , 6  , NULL         , NULL        },
      {CIE_DEV_FDDI , 6  , 6  , NULL         , NULL        },
      {CIE_DEV_NULL , 0  , 0  , NULL         , NULL        }
   };

   ADDR_MAP                * a;
   int                       rc    = 0;
   int                       clen  = min(cdliLen,cioLen);
   int                       dCIO  = max(0,cioLen-cdliLen);
   int                       dCDLI = max(0,cdliLen-cioLen);

   if ((a = findAddrMap(addrMap,devType,cioLen,cdliLen)) == NULL)
   {
      TRC_OTHER(mapx,devType,cioLen,cdliLen);
      rc = EINVAL;
   }
   else
   {
      if (direction == CIO_TO_CDLI)
      {
         if (a->cioToCdli == NULL)
            memcpy(cdliAddr+dCDLI,cioAddr+dCIO,  clen);
         else
            rc = (a->cioToCdli)(cioLen,cioAddr,cdliLen,cdliAddr);
      }
      else
      {
         if (a->cdliToCio == NULL)
            memcpy(cioAddr+dCIO,  cdliAddr+dCDLI,clen);
         else
            rc = (a->cdliToCio)(cdliLen,cdliAddr,cioLen,cioAddr);
      }

      if (rc) TRC_OTHER(mapy,devType,cioLen,cdliLen);
   }

   return rc;
}
