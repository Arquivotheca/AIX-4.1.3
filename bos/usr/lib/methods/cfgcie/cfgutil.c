static char sccsid[]="@(#)69   1.4  src/bos/usr/lib/methods/cfgcie/cfgutil.c, sysxcie, bos411, 9428A410j 4/1/94 15:28:11";

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 *   vpdItemLength
 *   memDump
 *   cvtBool
 *   getValAtt
 *   getHex
 *   getHardwareAddress
 *   getCurAddress
 *   cieDDInfo
 *   buildDDS
 *   getdevno
 *   lookupDevice
 *   getVPD
 *
 * DESCRIPTION:
 *
 *   Configuration Utility Routines
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

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

#include "cfgutil.h"

/*---------------------------------------------------------------------------*/
/*                     Device Driver Name Map Structure                      */
/*---------------------------------------------------------------------------*/

typedef struct DVDRMAP       DVDRMAP;

struct DVDRMAP
{
   char                    * cdliDDInstance;// CDLI DD Instance Name from ODM
   char                    * cieDDInstance; // Corresponding COMIO DD Instance
   CIE_DEV_TYPE              devType;       // COMIO Device Type Code
   CIE_DEV_SUB_TYPE          devSubType;    // COMIO Device SubType Code
};

/*---------------------------------------------------------------------------*/
/*             Returns the length of a VPD item pointed to by p              */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

unsigned int
   vpdItemLength(
      const char *           p            // I -Ptr to start of item
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  vpdItemLength(p)    (( (unsigned int) ((p)[3]) ) << 1)


/*---------------------------------------------------------------------------*/
/*                               Message Table                               */
/*---------------------------------------------------------------------------*/

typedef struct EMESSAGE      EMESSAGE;

struct EMESSAGE
{
   int                       msgNum;      // Message Number
   const char              * msgText;     // Message Text
};

/*---------------------------------------------------------------------------*/
/*                     Write an error message to stderr                      */
/*---------------------------------------------------------------------------*/

void
   emsg(
      int                    msgNum      ,// I -The message Number
      ...                                 // I -Substitution Text
   )
{
   static EMESSAGE           msgTbl[] =
   {
      { 0,"Unknown Error code %d in COMIO Emulator"                        },
      { 1,"Unable to initialize ODM"                                       },
      { 2,"Unable to acquire the ODM lock"                                 },
      { 3,"Device attribute '%s' is missing or invalid"                    },
      { 4,"Adapter address in CuAt VPD is not valid"                       },
      { 5,"Could not find adapter address in CuAt VPD"                     },
      { 7,"Unable to open the %x class in the Configuration Database"      },
      { 8,"%s entry for %s missing from Configuration Database"            },
      {20,"Unable to create the '%s' special file"                         },
      {21,"Unable to obtain major number for %s"                           },
      {22,"Unable to obtain minor number for %s"                           },
      {40,"Unable to load the COMIO Device Driver"                         },
      {41,"COMIO Emulation for device %s is already configured"            },
      {42,"Unable to configure COMIO Emulation for %s - errno=%d"          },
      {43,"Device %s with driver %s is not supported by the COMIO Emulator"},
      {44,"Device %s is busy and cannot be unconfigured"                   },
      {45,"Unable to unconfigure COMIO Emulation for %s - errno=%d"        },
      {46,"Unable to unload the COMIO Device Driver"                       },
      {47,"Device %s was not previously configured for COMIO Emulation"    }
   };

   va_list                   argp;

   int                       lo;
   int                       hi;
   int                       i;
   int                       found;

   va_start(argp,msgNum);

   lo    = 0;
   hi    = sizeof(msgTbl)/sizeof(EMESSAGE);

   found = 0;

   while(i=(hi+lo)/2, !found)
   {
      if (msgNum > msgTbl[i].msgNum)
         lo = i;
      else if (msgNum < msgTbl[i].msgNum)
         hi = i;
      else
         found = 1;
   }

   if (found)
      vfprintf(stderr,msgTbl[i].msgText,argp);
   else
      fprintf(stderr,msgTbl[0].msgText,i);

   putc('\n',stderr);
}

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
   )
{
   int                       totLen = 0; // Total formatted output length
   char                    * s;          // Ptr to data
   char                    * e;          // Ptr to end of data
   char                    * lo;         // s rounded down to width
   char                    * hi;         // e rounded up to width

   char                    * p;          // Scan pointer
   char                      buffer[160];// Work buffer

   static char             * hexChar="0123456789ABCDEF";


   width  = ((width > 16) ? 32 : ((width > 8) ? 16 : 8));
   indent = min(indent,30);

   s  = data;
   e  = s + len;

   if (opt == dumpMem)
   {
      lo = (char *)(((unsigned long)s)         & ~((unsigned long)width-1));
      hi = (char *)(((unsigned long)(e+width)) & ~((unsigned long)width-1));
   }
   else
   {
      lo = s;
      hi = lo + ((unsigned long)(len+width-1) & ~((unsigned long)width-1));
   }

   p = lo;

   while(p < hi)
   {
      int                    i;
      char                 * dx = buffer+indent;
      char                 * dc;

      memset(buffer,' ',sizeof(buffer));

      if (opt == dumpMem)
         dx += sprintf(dx,"%Fp  ",p);
      else
         dx += sprintf(dx,"%4.4hX  ",(p-lo));

      dc = dx + 2*width + width/4 + 2;

      for(i=0; i<width; i++)
      {
         if (p < s || p >= e)
         {
            *dx++ = ' ';
            *dx++ = ' ';
            *dc++ = ' ';
         }
         else
         {
            *dx++ = hexChar[*p>>4 & 0x0F];
            *dx++ = hexChar[*p    & 0x0F];
            *dc++ = (char)((isascii(*p) && isprint(*p)) ? *p : '.');
         }

         p++;

         dx += (i%4 == 3);
      }

      *++dx = '<';
      *dc++ = '>';
      *dc++ = '\n';
      *dc   = 0;

      strcpy(out,buffer);

      out    += (dc-buffer);
      totLen += (dc-buffer);
   }

   return totLen;
}

/*---------------------------------------------------------------------------*/
/*                Convert a yes/no string to Boolean (short)                 */
/*---------------------------------------------------------------------------*/

static
short
   cvtBool(
      register const char  * sRep         // I -String rep of boolean (yes/no)
   )
{
   register int              len = strlen(sRep);
   register int              val = -1;

   if (len == 2 && strcasecmp(sRep,"no") == 0)
      val = 0;
   else if (len == 3 && strcasecmp(sRep,"yes") == 0)
      val = 1;

   return val;
}

/*---------------------------------------------------------------------------*/
/*                    Front-end to getatt from cfgtools.x                    */
/*                 Adds handling for boolean types (yes/no)                  */
/*---------------------------------------------------------------------------*/

static
int
   getValAtt(
      const attr_list_t    * alist       ,// I -Attribute List Structure
      const char           * aname       ,// I -Attribute Name
      void                 * dest        ,//  O-Ptr to destination
      char                   type         // I -Destination Type Code
   )
{
   int                       dummy;       // Dummy int for getatt 'bc' parm
   int                       rc;

   if (type == ATTR_DEST_BOOL)
   {
      /*------------------------------------------------*/
      /*  Handle boolean output conversion from yes/no  */
      /*------------------------------------------------*/

      char                   sTemp[8];    // Temp to hold attrib's string value
      short                  bTemp;       // Temp to hold attrib's bool value

      if ((rc = getatt(alist,aname,sTemp,ATTR_DEST_STR,&dummy)) == 0)
      {
         if ((bTemp = cvtBool(sTemp)) >= 0 )
            *(unsigned char *)dest = bTemp;
         else
            rc = E_BADATTR;
      }
   }
   else
   {
      /*-------------------------------------*/
      /*  Let getatt handle all other types  */
      /*-------------------------------------*/

      rc = getatt(alist,aname,dest,type,&dummy);
   }

   if (rc)
      emsg(3,aname);

   return (rc != 0);
}

/*---------------------------------------------------------------------------*/
/*          Extract a hex string from its character representation           */
/*---------------------------------------------------------------------------*/

int
   getHex(
      char                 * outbuf      ,
      int                    outlen      ,
      char                 * inbuf
   )
{
   int                       l = strlen(inbuf);
   int                       nibble = (l%2);
   char                    * p = outbuf;

   *p = 0;

   while (p<outbuf+outlen && *inbuf)
   {
      register char c = tolower(*inbuf++);

      if (isascii(c) && isxdigit(c))
      {
         int                 v = (c & 0x0F) + ((c & 0xC0) ? 9 : 0);

         switch(nibble)
         {
            case 0:
               *p = v << 4;
               break;
            case 1:
               *(p++) |= v;
         }
      }
      else
         break;

      nibble = !nibble;
   }

   *p = 0;

   return(*inbuf != 0 || p < outbuf+outlen);
}

/*---------------------------------------------------------------------------*/
/*             Extract Burned-in Network Address from VPD string             */
/*---------------------------------------------------------------------------*/

int
   getHardwareAddress(
      char                   hdwAddr[6]  ,//  O-Return Buffer
      int                    addrlen     ,// I -Address Length
      const VPDBUF         * vpd          // I -VPD Buffer
   )
{
   const unsigned char     * p = (const unsigned char *)(vpd->buffer+7);
   const unsigned char     * e = p + vpd->length;

   while(p < e)
   {
      if (memcmp(p,"*NA",3) == 0)
      {
         if(addrlen == vpdItemLength(p)-4)
         {
            memcpy(hdwAddr,&p[4],addrlen);
            return 0;
         }
         else
         {
            emsg(4);
            return -1;
         }
      }

      p += vpdItemLength(p);
   }

   emsg(5);
   return -1;
}

/*---------------------------------------------------------------------------*/
/*                    Extract the current network address                    */
/*---------------------------------------------------------------------------*/

int
   getCurAddress(
      const attr_list_t    * alist       ,// I -Attribute List Structure
      char                 * curAddr     ,//  O-Alternate Address
      const char           * hdwAddr     ,// I -Hardwired Address
      int                    addrLen      // I -Address Length
   )
{
   short                     useAltAddr;
   char                      altAddr[30];

   if (getValAtt(alist,"use_alt_addr",&useAltAddr,ATTR_DEST_BOOL) != 0)
      return 1;

   if (!useAltAddr)
      memcpy(curAddr,hdwAddr,addrLen);
   else
   {
      if (getValAtt(alist,"alt_addr",altAddr,ATTR_DEST_STR) != 0)
         return 1;

      if (altAddr[0] != '0' || tolower(altAddr[1]) != 'x')
      {
         emsg(3,"alt_addr");
         return 1;
      }

      if (getHex(curAddr,addrLen,altAddr+2) != 0)
      {
         emsg(3,"alt_addr");
         return 1;
      }
   }

   return 0;
}

/*---------------------------------------------------------------------------*/
/*              Map the CDLI dvdr Name to its COMIO equivalent               */
/*---------------------------------------------------------------------------*/

int
   cieDDInfo(
      DEVINFO              * devInfo      // IO-Device Configuration Info
   )
{
   static DVDRMAP            dvdrMap[] =
   {
   /*---------------------------------------------------------------*/
   /*   CDLI       COMIO Dev    COMIO          COMIO                */
   /*   DvDr       Instance     Device         Device               */
   /*   Name       Name         Type Code      Sub Type Code        */
   /*---------------------------------------------------------------*/
      {"tokdd"    ,"cietok"    ,CIE_DEV_TOK   ,CIE_DEV_SUB_NULL     },
      {"entdd"    ,"cieent"    ,CIE_DEV_ENT   ,CIE_DEV_SUB_3COM     },
      {"ethdd"    ,"cieent"    ,CIE_DEV_ENT   ,CIE_DEV_SUB_IENT     },
      {"fddidd"   ,"ciefddi"   ,CIE_DEV_FDDI  ,CIE_DEV_SUB_NULL     },
      { NULL      , NULL       ,CIE_DEV_NULL  ,CIE_DEV_SUB_NULL     }
   };

   int                       i;
   int                       instLen;
   int                       rc;          // Return Code

   /*-----------------------------------------------*/
   /*  Search Device Driver name table for a match  */
   /*-----------------------------------------------*/

   instLen = strlen(devInfo->cdliDDInstance);

   for (i=0; dvdrMap[i].cdliDDInstance != NULL; i++)
   {
      register int           len = strlen(dvdrMap[i].cdliDDInstance);

      if
      (
         len == instLen                            &&
         strcmp(dvdrMap[i].cdliDDInstance,
                devInfo->cdliDDInstance,len) == 0
      )
         break;
   }

   /*------------------------------------------------*/
   /*  If no match, we don't know how to emulate it  */
   /*------------------------------------------------*/

   if (dvdrMap[i].devType == CIE_DEV_NULL)
   {
      emsg(43,devInfo->logicalName,devInfo->cdliDDInstance);
      return E_WRONGDEVICE;
   }

   /*----------------------------------------------------------------------*/
   /*  Extract Device Type Code and COMIO Driver Instance Name from table  */
   /*----------------------------------------------------------------------*/

   strcpy(devInfo->cieDDInstance,dvdrMap[i].cieDDInstance);
   devInfo->devType    = dvdrMap[i].devType;
   devInfo->devSubType = dvdrMap[i].devSubType;

   return 0;
}

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
   )
{
   int                       err = 0;     // Error Flags
   int                       rc  = 0;

   bzero(dds,sizeof(*dds));

   dds->devType     = devInfo->devType;
   dds->devSubType  = devInfo->devSubType;

   strcpy(dds->devName,devInfo->logicalName);
   strcpy(dds->nddName,devInfo->logicalName);

   /*------------------------------------*/
   /*  Extract "interesting" attributes  */
   /*------------------------------------*/

   switch(devInfo->devType)
   {
      case CIE_DEV_ENT:
      {
         err |= getHardwareAddress(dds->ds.tok.hdwAdptAddr,
                                   TOK_NADR_LENGTH,
                                   vpd);

         err |= getCurAddress(devAttrList,
                              dds->ds.tok.curAdptAddr,
                              dds->ds.tok.hdwAdptAddr,
                              TOK_NADR_LENGTH);

         err |= getValAtt(devAttrList,
                          "rec_que_size",
                          &dds->recQueueSize,
                          ATTR_DEST_INT);

         err |= getValAtt(devAttrList,
                          "sta_que_size",
                          &dds->staQueueSize,
                          ATTR_DEST_INT);

         err |= getValAtt(devAttrList,
                          "net_id_offset",
                          &dds->ds.ent.netIdOffset,
                          ATTR_DEST_INT);

         err |= getValAtt(devAttrList,
                         "type_field_off",
                         &dds->ds.ent.eTypeOffset,
                         ATTR_DEST_INT);
         break;
      }

      case CIE_DEV_TOK:
      {
         int                 ringSpeed;

         err |= getHardwareAddress(dds->ds.ent.hdwAdptAddr,
                                   ent_NADR_LENGTH,
                                   vpd);

         err |= getCurAddress(devAttrList,
                              dds->ds.ent.curAdptAddr,
                              dds->ds.ent.hdwAdptAddr,
                              ent_NADR_LENGTH);

         err |= getValAtt(devAttrList,
                          "rec_que_size",
                          &dds->recQueueSize,
                          ATTR_DEST_INT);

         err |= getValAtt(devAttrList,
                          "sta_que_size",
                          &dds->staQueueSize,
                          ATTR_DEST_INT);

         err |= getValAtt(devAttrList,
                          "ring_speed",
                          &ringSpeed,
                          ATTR_DEST_INT);
         dds->ds.tok.ringSpeed = ((ringSpeed == 16) ? TOK_16M : TOK_4M);

         break;
      }

      case CIE_DEV_FDDI:
      {
         err |= getHardwareAddress(dds->ds.fddi.hdwAdptAddr,
                                   FDDI_NADR_LENGTH,
                                   vpd);

         err |= getCurAddress(devAttrList,
                              dds->ds.fddi.curAdptAddr,
                              dds->ds.fddi.hdwAdptAddr,
                              FDDI_NADR_LENGTH);

         err |= getValAtt(devAttrList,
                          "rcv_que_size",
                          &dds->recQueueSize,
                          ATTR_DEST_INT);

         err |= getValAtt(devAttrList,
                          "stat_que_size",
                          &dds->staQueueSize,
                          ATTR_DEST_INT);

         err |= getValAtt(devAttrList,
                          "pass_bcon",
                          &dds->ds.fddi.passBeacon,
                          ATTR_DEST_BOOL);

         err |= getValAtt(devAttrList,
                          "pass_nsa",
                          &dds->ds.fddi.passNSA,
                          ATTR_DEST_BOOL);

         err |= getValAtt(devAttrList,
                          "pass_smt",
                          &dds->ds.fddi.passSMT,
                          ATTR_DEST_BOOL);

         break;
      }
   }

   /*-----------------------------------------------*/
   /*  If attributes had errors, clean up and exit  */
   /*-----------------------------------------------*/

   if (err) return E_BADATTR;

   /*-------------------*/
   /*  Copy VPD to DDS  */
   /*-------------------*/

   switch(devInfo->devType)
   {
      case CIE_DEV_TOK:

         dds->ds.tok.vpd.status  = vpd->valid
                                      ? TOK_VPD_VALID
                                      : TOK_VPD_NOT_READ;

         dds->ds.tok.vpd.l_vpd   = min(vpd->length,
                                       sizeof(dds->ds.tok.vpd.vpd));

         memcpy(dds->ds.tok.vpd.vpd,
                vpd->buffer,
                dds->ds.tok.vpd.l_vpd);
         break;

      case CIE_DEV_ENT:

         dds->ds.ent.vpd.status  = vpd->valid
                                      ? VPD_VALID
                                      : VPD_NOT_READ;

         dds->ds.ent.vpd.length  = min(vpd->length,
                                       sizeof(dds->ds.ent.vpd.vpd));

         memcpy(dds->ds.ent.vpd.vpd,
                vpd->buffer,
                dds->ds.ent.vpd.length);
         break;

      case CIE_DEV_FDDI:

         /*-----------------------*/
         /*  Primary Adapter VPD  */
         /*-----------------------*/

         dds->ds.fddi.vpd.status = vpd->valid
                                      ? FDDI_VPD_VALID
                                      : FDDI_VPD_NOT_READ;

         dds->ds.fddi.vpd.l_vpd  = min(vpd->length,
                                       sizeof(dds->ds.fddi.vpd.vpd));

         memcpy(dds->ds.fddi.vpd.vpd,
                vpd->buffer,
                dds->ds.fddi.vpd.l_vpd);

         /*-------------------------*/
         /*  Extension Adapter VPD  */
         /*-------------------------*/

         dds->ds.fddi.vpd.xc_status = vpdFDDIx->valid
                                      ? FDDI_VPD_VALID
                                      : FDDI_VPD_NOT_READ;

         dds->ds.fddi.vpd.l_xcvpd   = min(vpdFDDIx->length,
                                          sizeof(dds->ds.fddi.vpd.xcvpd));

         memcpy(dds->ds.fddi.vpd.xcvpd,
                vpdFDDIx->buffer,
                dds->ds.fddi.vpd.l_xcvpd);
         break;
   }

   return 0;
}

/*---------------------------------------------------------------------------*/
/*         Get the device number for a logical name and DD Instance          */
/*---------------------------------------------------------------------------*/

int
   getdevno(
      const char           * logicalName ,// I -Device Logical Name
      const char           * DDInstance  ,// I -Dev Dr Instance Name
      int                    generate    ,// I -Generate Flag
      dev_t                * pdevno       //  O-Ptr to devno return area
   )
{
   long                    * mptr;        // Ptr to minor number(s)
   long                      mcount;      // Count of Minor Numbers
   long                      devMajor;    // Device Major Number
   long                      devMinor;    // Device Minor Number
   int                       rc = 0;

   /*------------------------*/
   /*  Get the major number  */
   /*------------------------*/

   if ((devMajor = genmajor(DDInstance)) == -1)
   {
      emsg(21,logicalName);
      rc = E_MAJORNO;
   }
   else
   {
      /*------------------------*/
      /*  Get the minor number  */
      /*------------------------*/

      mptr = getminor(devMajor,&mcount,logicalName);

      /*-------------------------------------------------------------------*/
      /*  If minor doesn't exist and we're allowed to generate one, do it  */
      /*-------------------------------------------------------------------*/

      if (mptr == NULL && generate)
      {
         mptr = genminor(logicalName,devMajor,-1,1,1,1);
         rc   = I_NEWMINOR;
      }

      /*--------------------*/
      /*  Check results...  */
      /*--------------------*/

      if (mptr == NULL)
      {
         rc = E_MINORNO;
         emsg(22,logicalName);
      }
      else
      {
         devMinor = *mptr;
         *pdevno  = makedev(devMajor,devMinor);
      }
   }

   return rc;
}

/*---------------------------------------------------------------------------*/
/*     Find Logical Name in CuDv and return CDLI cdliDDInstance and uniquetype     */
/*---------------------------------------------------------------------------*/

int
   lookupDevice(
      DEVINFO              * devInfo      //  O-Device Configuration Info
   )
{
   struct Class            * pCuDv = NULL;
   struct CuDv               iCuDv;

   char                      odmSearchArg[80];
   int                       rc = 0;

   /*-------------*/
   /*  Open CuDv  */
   /*-------------*/

   if ((int)(pCuDv = odm_open_class(CuDv_CLASS)) == -1)
   {
      emsg(7,"CuDv");
      return E_ODMOPEN;
   }

   /*-------------------------------------------*/
   /*  Find device with specified logical name  */
   /*-------------------------------------------*/

   sprintf(odmSearchArg,"name='%s'",devInfo->logicalName);

   if ((rc = (int)odm_get_first(pCuDv,odmSearchArg,&iCuDv)) <= 0)
   {
      odm_close_class(CuDv_CLASS);
      emsg(8,"CuDv",devInfo->logicalName);
      return (rc==0) ? E_NOCuDv : E_ODMGET;
   }

   /*-------------------------------------------------*/
   /*  Return the CDLI cdliDDInstance and uniqueType  */
   /*-------------------------------------------------*/

   strcpy(devInfo->cdliDDInstance,iCuDv.ddins        );
   strcpy(devInfo->uniqueType,    iCuDv.PdDvLn_Lvalue);
   strcpy(devInfo->location,      iCuDv.location     );

   odm_close_class(CuDv_CLASS);

   return 0;
}

/*---------------------------------------------------------------------------*/
/*                               Retrieve VPD                                */
/*---------------------------------------------------------------------------*/

int
   getVPD(
      const char           * ldev        ,// I -Logical Device Name
      VPDBUF               * vpd          //  O-VPD Buffer
   )
{
   struct Class            * pCuVPD;
   struct CuVPD              iCuVPD;
   char                      crit[100];
   char                    * v;
   int                       vl;

   int                       rc = 0;

   vpd->valid     = 0;
   vpd->length    = 0;
   vpd->buffer[0] = 0x00;

   /*------------------------*/
   /*  Open the CuVPD Class  */
   /*------------------------*/

   pCuVPD = odm_open_class(CuVPD_CLASS);

   if ((int)pCuVPD == -1)
   {
      emsg(7,"CuVPD");
      return E_ODMOPEN;
   }

   /*-----------------------------*/
   /*  Build the search criteria  */
   /*-----------------------------*/

   sprintf(crit,"name=%s",ldev);

   /*-------------------*/
   /*  Find the device  */
   /*-------------------*/

   if ((rc = (int)odm_get_first(pCuVPD,crit,&iCuVPD)) == -1)
   {
      odm_close_class(pCuVPD);
      emsg(8,"CuVPD",ldev);
      return (rc==0) ? E_VPD : E_ODMGET;
   }

   /*------------------------------------------*/
   /*  Parse the VPD string to get its length  */
   /*------------------------------------------*/

   v = iCuVPD.vpd;

   while(*v == '*')
   {
      unsigned int           l;
      l = ((unsigned int)(unsigned char)(v[3])) << 1;
      v += l;
   }

   vl = v - iCuVPD.vpd;

   /*---------------------------------------------------------*/
   /*  Build a standard VPD Buffer in the user's return area  */
   /*---------------------------------------------------------*/

   memcpy(vpd->buffer,"VPD",3);
   *((ushort *)(vpd->buffer+3)) = (vl+1) >> 1;
   *((ushort *)(vpd->buffer+5)) = 0;
   memcpy(vpd->buffer+7,iCuVPD.vpd,vl);

   vpd->length = vl;
   vpd->valid = 1;

   /*-------------------------*/
   /*  Close the CuVPD Class  */
   /*-------------------------*/

   odm_close_class(pCuVPD);

   return 0;
}

/*---------------------------------------------------------------------------*/
/*         Generate the expected location of the FDDI extension card         */
/*---------------------------------------------------------------------------*/

static
int
   genFDDIxLocation(
      char                 * location     // IO-Slot Location ID
   )
{
   int                       len = strlen(location);
   char                    * e   = location + len;
   int                       rc  = -1;

   if (len > 4 && isdigit(e[-1]) && isdigit(e[-2]))
   {
      char                 * p = e;

      /*---------------------------------------------------------------*/
      /*  Given a numeric ASCII string with possibly some non-numeric  */
      /*  leading bytes, the following statement decrements the        */
      /*  numeric value by 1.  The only assumption is that the         */
      /*  digit character codes are contiguous in the code page.       */
      /*---------------------------------------------------------------*/

      while(p>location && isdigit(*--p) && !isdigit(--*p)) *p = '9';

      /*---------------------------------------------*/
      /*  Check for the following conditions:        */
      /*      Not all numeric (p > location)         */
      /*      At least one trailing digit (p < e-1)  */
      /*      Initial numeric value was not zero     */
      /*---------------------------------------------*/

      rc = (p > location && p < e && isdigit(*p)) ? 0 : -1;
   }

   return rc;
}

/*---------------------------------------------------------------------------*/
/*             Retrieve the FDDI extension Logical Name from ODM             */
/*---------------------------------------------------------------------------*/

int
   getFDDIxLogicalName(
      const DEVINFO        * devInfo     ,// I -Device Configuration Info
      char                 * logicalName  //  O-VPD Output Buffer
   )
{
   struct Class            * pCuDv = NULL;
   struct CuDv               iCuDv;

   char                      odmSearchArg[80];
   int                       rc = 0;

   char                      xLocation[LOCSIZE];

   logicalName[0] = 0;

   /*-------------*/
   /*  Open CuDv  */
   /*-------------*/

   if ((int)(pCuDv = odm_open_class(CuDv_CLASS)) == -1)
   {
      emsg(7,"CuVPD");
      return E_ODMOPEN;
   }

   /*------------------------------------------------------*/
   /*  Generate expected location for FDDI extension card  */
   /*------------------------------------------------------*/

   strcpy(xLocation,devInfo->location);

   if (genFDDIxLocation(xLocation) != 0) return E_NODEPENDENT;

   /*------------------------------------*/
   /*  Find the device at that location  */
   /*------------------------------------*/

   sprintf(odmSearchArg,"location='%s'",xLocation);

   if ((int)odm_get_first(pCuDv,odmSearchArg,&iCuDv) != -1)
   {
      if (strcmp(iCuDv.parent,devInfo->logicalName) == 0)
      {
         rc = 0;
         strcpy(logicalName,iCuDv.name);
      }
      else
         rc = E_NODEPENDENT;
   }
   else
      rc = E_NODEPENDENT;

   odm_close_class(CuDv_CLASS);

   return rc;
}
