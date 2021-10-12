static char sccsid[]="@(#)68   1.7  src/bos/usr/lib/methods/cfgcie/cfgcie.c, sysxcie, bos41J, 9521B_all 5/25/95 07:46:39";

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 *   main
 *
 * DESCRIPTION:
 *
 *   Configuration Method
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

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#include "cfgutil.h"

void
   printDevInfo(
      DEVINFO              * d,
      char                 * msg
   )
{

   printf("Device Configuration Info - %s\n",msg);
   printf("  Logical Name  '%s'\n",(d->logicalName ? d->logicalName : ""));
   printf("  CDLI Instance '%s'\n",d->cdliDDInstance);
   printf("  Unique Type   '%s'\n",d->uniqueType);
   printf("  Location      '%s'\n",d->location);
   printf("  CIE Instance  '%s'\n",d->cieDDInstance);
   printf("  DevType        %d \n",d->devType);
   printf("  DevSubType     %d \n",d->devSubType);
}

/*---------------------------------------------------------------------------*/
/*                               Main Routine                                */
/*---------------------------------------------------------------------------*/

int
   main(
      int                    argc        ,
      char                 * argv[]
   )
{
   int                       rc;
   int                       i;
   int                       argError    = 0;
   int                       iplPhase    = RUNTIME_CFG;

   DEVINFO                   devInfo;

   attr_list_t             * devAttrList;
   int                       devAttrCount;
   VPDBUF                    vpd;         // VPD Buffer
   VPDBUF                    vpdFDDIx;    // VPD Buffer for FDDI Extension

   dev_t                     devno;       // Device Number (Major/Minor)

   struct DDS                dds;         // Device-Dependent Structure
   struct cfg_dd             cfgdd;       // Parameter Block for sysconfig

   int                       newMinor = 0;// We allocated new minor number
   int                       odmLockID;

   int                       c;

   long			     create_flags;

   /*-----------------------------------*/
   /*  Validate Command Line Arguments  */
   /*-----------------------------------*/

   while ((c = getopt(argc,argv,"l:12")) != EOF)
   {
      switch(c)
      {
         case 'l':           // Logical Name
            if (devInfo.logicalName != NULL &&strcmp(devInfo.logicalName,optarg)!=0)
               argError = 1;
            else
               devInfo.logicalName = (const char *)optarg;
            break;

         case '1':
            if (iplPhase == RUNTIME_CFG)
               iplPhase = PHASE1;
            else
               argError = 1;
            break;

         case '2':
            if (iplPhase == RUNTIME_CFG)
               iplPhase = PHASE2;
            else
               argError = 1;
            break;

         default:
            argError = 1;
            break;
      }
   }

   if (argError)                     return E_ARGS;
   if (devInfo.logicalName == NULL)  return E_LNAME;

   /*---------------------------------*/
   /*  Data Structure Initialization  */
   /*---------------------------------*/

   bzero(devInfo ,sizeof(devInfo ));
   bzero(vpd     ,sizeof(vpd     ));
   bzero(vpdFDDIx,sizeof(vpdFDDIx));
   bzero(dds     ,sizeof(dds     ));
   bzero(cfgdd   ,sizeof(cfgdd   ));

   /*------------------*/
   /*  Initialize ODM  */
   /*------------------*/

   if (odm_initialize() != 0)
   {
      emsg(1);
      return E_ODMINIT;
   }


   /*-----------------------------------*/
   /*  Lock the Configuration Database  */
   /*-----------------------------------*/

   /*if ((odmLockID = odm_lock("/etc/objrepos/config_lock",ODM_WAIT)) == -1)
   {
      odm_terminate();
      emsg(2);
      return E_ODMLOCK;
   }*/

   /*-------------------------------------------*/
   /*  Lookup Device Configuration Info in ODM  */
   /*-------------------------------------------*/

   if ((rc = lookupDevice(&devInfo)) != 0)
      goto quit;

   /*---------------------------------------------------*/
   /*  Map the CDLI ddInstance to its COMIO equivalent  */
   /*---------------------------------------------------*/

   if ((rc = cieDDInfo(&devInfo)) != 0)
      goto quit;

   /*-------------------*/
   /*  Read Device VPD  */
   /*-------------------*/

   if ((rc = getVPD(devInfo.logicalName,&vpd)) != 0)
      goto quit;

   /*-----------------------------------------*/
   /*  For FDDI, get extension card VPD also  */
   /*-----------------------------------------*/

   if (devInfo.devType == CIE_DEV_FDDI)
   {
      char                   FDDIxLogicalName[DDNAMESIZE];

      switch (rc = getFDDIxLogicalName(&devInfo,FDDIxLogicalName))
      {
         case 0:

            /*----------------------------------------*/
            /*  Retrieve VPD for FDDI Extension Card  */
            /*----------------------------------------*/

            if ((getVPD(FDDIxLogicalName,&vpdFDDIx)) != 0)
               goto quit;
            break;

         case E_NODEPENDENT:

            /*--------------------------------------*/
            /*  Did not find a FDDI Extension Card  */
            /*--------------------------------------*/
            break;

         default:

            /*---------------------------------------------------*/
            /*  Error generating logical name - Cannot continue  */
            /*---------------------------------------------------*/

            goto quit;
      }
   }

   /*--------------------------*/
   /*  Load Device Attributes  */
   /*--------------------------*/

   if ((devAttrList = get_attr_list(devInfo.logicalName,
                                    devInfo.uniqueType,
                                    &devAttrCount,
                                    20)
       ) == NULL)
   {
      rc = E_INVATTR;
      goto quit;
   }

   /*----------------------------------------*/
   /*  Build the Device Dependent Structure  */
   /*----------------------------------------*/

   if ((rc = buildDDS(&dds,
                      &devInfo,
                      devAttrList,
                      &vpd,
                      &vpdFDDIx)
       ) != 0)
      goto quit;

   /*------------------------------*/
   /*  Generate the Device Number  */
   /*------------------------------*/

   switch(rc = getdevno(devInfo.logicalName,
                        devInfo.cieDDInstance,
                        1,
                        &devno))
   {
      case 0:
         break;

      case I_NEWMINOR:
         newMinor = 1;
         break;

      default:
         goto quit;
   }

   /*---------------------------*/
   /*  Create the special file  */
   /*---------------------------*/

   create_flags = ( S_IFMPX | S_IFCHR | S_IRUSR | S_IWUSR | \
                    S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH );
   if ((rc = mk_sp_file(devno,devInfo.logicalName,create_flags)) != 0)
   {
      if (newMinor) reldevno(devInfo.logicalName,True);
      emsg(20,devInfo.logicalName);
      rc = E_MKSPECIAL;
      goto quit;
   }

   /*---------------------------------*/
   /*  Load the Device Driver Module  */
   /*---------------------------------*/

   if ((cfgdd.kmid = loadext(cieName,True,False)) == NULL)
   {
      emsg(40);
      if (newMinor) reldevno(devInfo.logicalName,True);
      rc = E_LOADEXT;
      goto quit;
   }

   /*--------------------------------------------------------*/
   /*  Set up the cfgdd structure for the call to sysconfig  */
   /*--------------------------------------------------------*/

   cfgdd.devno  = devno;
   cfgdd.cmd    = CFG_INIT;
   cfgdd.ddsptr = (caddr_t)&dds;
   cfgdd.ddslen = sizeof(dds);

   /*--------------------------------*/
   /*  Initialize the device driver  */
   /*--------------------------------*/

   if ((rc = sysconfig(SYS_CFGDD,
                       (void *)&cfgdd,
                       sizeof(cfgdd))) != 0)
   {
      switch(errno)
      {
         case EEXIST:
	    /*
	     This message is being put out to standard out and that causes
	     problems for cfgmgr who reads everything from stdout and
	     thinks they device names to be configured
            */
            /*emsg(41,devInfo.logicalName);*/
	    /* Please note that this change of returning 0 instead of E_A...
	       is done mainly to accomodate tok,ent,fddi and ient drivers
	       They are returning error when I return E_A... and consequently
	       the cfgtok, cfgent, ... methods return and error instead
	       of returning OK, on finding that you are trying to configure
	       and already configured device. So instead of changing all
	       three config methods, we decided to change this one cfgmethods
	    */
	    rc = 0;
            /*rc = E_ALREADYDEF;*/
            goto quit;

         default:
            emsg(42,devInfo.logicalName,errno);
            loadext(cieName,False,False);
            if (newMinor) reldevno(devInfo.logicalName,True);
            rc = E_CFGINIT;
            goto quit;
      }
   }

   /*------------------------*/
   /*  Close ODM and return  */
   /*------------------------*/

quit:
   /*odm_unlock(odmLockID);*/
   odm_terminate();

   return rc;
}
