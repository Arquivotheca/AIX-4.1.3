/* @(#)70   1.4  src/bos/usr/lib/methods/cfgcie/cfgutil.h, sysxcie, bos411, 9428A410j 4/1/94 15:28:26 */

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 *   min
 *
 * DESCRIPTION:
 *
 *   Configuration Utility Routines Header File
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

#if !defined(CFGUTIL_H)
#define  CFGUTIL_H

#include <odmi.h>
#include <cf.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <sys/sysconfig.h>
#include <sys/mode.h>
#include <sys/device.h>
#include <sys/errno.h>
#include <cfgtoolsx.h>
#include <ciedds.h>

/*---------------------------------------------------------------------------*/
/*             Misc readability enhancers for cfgtoolsx.h stuff              */
/*---------------------------------------------------------------------------*/

#define  ATTR_DEST_INT        ('i')
#define  ATTR_DEST_STR        ('s')
#define  ATTR_DEST_BOOL       (0xFF)
#define  True  (1)
#define  False (0)
#define  min(x,y) ((x)<(y) ? (x) : (y))

/*---------------------------------------------------------------------------*/
/*                         Implementation Constants                          */
/*---------------------------------------------------------------------------*/

#define  cieName             "ciedd"

/*---------------------------------------------------------------------------*/
/*          Additional internal Config Return Codes (CIE-specific)           */
/*---------------------------------------------------------------------------*/

#define  I_NEWMINOR          (E_LAST_ERROR+1)

/*---------------------------------------------------------------------------*/
/*                   Type Definition for struct attr_list                    */
/*---------------------------------------------------------------------------*/

typedef struct attr_list     attr_list_t;

/*---------------------------------------------------------------------------*/
/*                         Vital Product Data Buffer                         */
/*---------------------------------------------------------------------------*/

typedef struct VPDBUF        VPDBUF;

struct VPDBUF
{
   int                       valid;
   int                       length;
   char                      buffer[1024];
};

/*---------------------------------------------------------------------------*/
/*                         Device Configuration Info                         */
/*---------------------------------------------------------------------------*/

typedef struct DEVINFO       DEVINFO;

struct DEVINFO
{
   const char              * logicalName;
   char                      cdliDDInstance[DDNAMESIZE];
   char                      uniqueType    [UNIQUESIZE];
   char                      location      [LOCSIZE];
   char                      cieDDInstance [DDNAMESIZE];
   CIE_DEV_TYPE              devType;
   CIE_DEV_SUB_TYPE          devSubType;
};

/*---------------------------------------------------------------------------*/
/*                        Dump Option Type Definition                        */
/*---------------------------------------------------------------------------*/

typedef enum DUMPOPT         DUMPOPT;

enum DUMPOPT
{
   dumpMem,                  // Format addresses as memory (seg:off) refs
   dumpRel                   // Format addresses as relative offsets
};

/*---------------------------------------------------------------------------*/
/*                              Hex Memory Dump                              */
/*---------------------------------------------------------------------------*/

int
   memDump(
      char                 * out         ,// IO-Output Buffer
      int                    indent      ,// I -Output Line Indent
      int                    width       ,// I -Output Line Width (src bytes)
      DUMPOPT                opt         ,// I -Dump Options
      void                 * data        ,// I -Source Data
      int                    len          // I -Length of Source
   );

/*---------------------------------------------------------------------------*/
/*          Extract a hex string from its character representation           */
/*---------------------------------------------------------------------------*/

int
   getHex(
      char                 * outbuf      ,
      int                    outlen      ,
      char                 * inbuf
   );

/*---------------------------------------------------------------------------*/
/*             Extract Burned-in Network Address from VPD string             */
/*---------------------------------------------------------------------------*/

int
   getHardwareAddress(
      char                   hdwAddr[6]  ,//  O-Return Buffer
      int                    addrlen     ,// I -Address Length
      const VPDBUF         * vpd          // I -VPD Buffer
   );

/*---------------------------------------------------------------------------*/
/*                    Extract the current network address                    */
/*---------------------------------------------------------------------------*/

int
   getCurAddress(
      const attr_list_t    * alist       ,// I -Attribute List Structure
      char                 * curAddr     ,//  O-Alternate Address
      const char           * hdwAddr     ,// I -Hardwired Address
      int                    addrLen      // I -Address Length
   );

/*---------------------------------------------------------------------------*/
/*              Map the CDLI dvdr Name to its COMIO equivalent               */
/*---------------------------------------------------------------------------*/

int
   cieDDInfo(
      DEVINFO              * devInfo      // IO-Device Configuration Info
   );

/*---------------------------------------------------------------------------*/
/*                 Initialize the Device Dependent Structure                 */
/*---------------------------------------------------------------------------*/

int
   buildDDS(
      DDS                  * dds         ,// IO-Device Dependent Structure
      const DEVINFO        * devInfo     ,// I -Dev Configuration Info
      const attr_list_t    * devAttrList ,// I -Device Attributes
      const VPDBUF         * vpd         ,// I -Vital Product Data
      const VPDBUF         * vpdFDDIx     // I -VPD for FDDI Extension
   );

/*---------------------------------------------------------------------------*/
/*         Get the device number for a logical name and DD Instance          */
/*---------------------------------------------------------------------------*/

int
   getdevno(
      const char           * logicalName ,// I -Device Logical Name
      const char           * DDInstance  ,// I -Dev Dr Instance Name
      int                    generate    ,// I -Generate Flag
      dev_t                * pdevno       //  O-Ptr to devno return area
   );

/*---------------------------------------------------------------------------*/
/*     Find Logical Name in CuDv and return CDLI cdliDDInstance and uniquetype     */
/*---------------------------------------------------------------------------*/

int
   lookupDevice(
      DEVINFO              * devInfo      //  O-Device Configuration Info
   );

/*---------------------------------------------------------------------------*/
/*                               Retrieve VPD                                */
/*---------------------------------------------------------------------------*/

int
   getVPD(
      const char           * ldev        ,// I -Logical Device Name
      VPDBUF               * vpd          //  O-VPD Buffer
   );
#endif
