

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
 *   UnConfigure Method
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

   DEVINFO                   devInfo;

   dev_t                     devno;       // Device Number (Major/Minor)

   struct DDS                dds;         // Device-Dependent Structure
   struct cfg_dd             cfgdd;       // Parameter Block for sysconfig

   int                       odmLockID;
   int                       c;

   /*-----------------------------------*/
   /*  Validate Command Line Arguments  */
   /*-----------------------------------*/

   while ((c = getopt(argc,argv,"l:")) != EOF)
   {
      switch(c)
      {
         case 'l':           // Logical Name
            if (devInfo.logicalName!=NULL && strcmp(devInfo.logicalName,optarg)!=0)
               argError = 1;
            else
               devInfo.logicalName = (const char *)optarg;
            break;

         default:
            argError = 1;
            break;
      }
   }

   if (argError)                     return E_ARGS;
   if (devInfo.logicalName == NULL)  return E_LNAME;

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

   /*------------------------------*/
   /*  Generate the Device Number  */
   /*------------------------------*/

   if ((rc = getdevno(devInfo.logicalName,
                      devInfo.cieDDInstance,
                      0,
                      &devno)
       ) != 0)
      goto quit;

   /*------------------------------------------------*/
   /*  Verify that the driver is actually in memory  */
   /*------------------------------------------------*/

   if (loadext(cieName,False,True) != NULL)
   {
      /*--------------------------------------------------------*/
      /*  Set up the cfgdd structure for the call to sysconfig  */
      /*--------------------------------------------------------*/

      cfgdd.kmid   = 0;
      cfgdd.devno  = devno;
      cfgdd.cmd    = CFG_TERM;
      cfgdd.ddsptr = NULL;
      cfgdd.ddslen = 0;

      /*-------------------------------*/
      /*  Terminate the device driver  */
      /*-------------------------------*/

      if ((rc = sysconfig(SYS_CFGDD,(void *)&cfgdd,sizeof(cfgdd))) != 0)
      {
         switch(errno)
         {
            case EBUSY:
               rc = E_BUSY;
               emsg(44,devInfo.logicalName);
               goto quit;

            case ENODEV:
               rc = E_WRONGDEVICE;
               emsg(47,devInfo.logicalName);
               goto quit;

            default:
               emsg(45,devInfo.logicalName,errno);
         }
      }

      /*-----------------------------------*/
      /*  Unload the Device Driver Module  */
      /*-----------------------------------*/

      if (loadext(cieName,False,False) == -1)
         emsg(46,devInfo.logicalName);
   }

   /*-----------------------------*/
   /*  Release the device number  */
   /*-----------------------------*/

   reldevno(devInfo.logicalName,True);

   /*------------------------*/
   /*  Close ODM and return  */
   /*------------------------*/

quit:
   /*odm_unlock(odmLockID);*/
   odm_terminate();

   return rc;
}
