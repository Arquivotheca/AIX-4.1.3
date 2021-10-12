static char sccsid[]="@(#)24   1.8  src/bos/kernext/cie/devtable.c, sysxcie, bos411, 9428A410j 4/26/94 10:46:13";

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 *   new_DEVTABLE
 *   free_DEVTABLE
 *   mkDevHandle
 *
 * DESCRIPTION:
 *
 *    Device Table
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

#include "ciedd.h"
#include "dev.h"
#include "devtable.h"
#include "dmalloc.h"

/*---------------------------------------------------------------------------*/
/*                   Allocate memory for the Device Table                    */
/*---------------------------------------------------------------------------*/

DEVTABLE *
   new_DEVTABLE(
      void
   )
{
   FUNC_NAME(new_DEVTABLE);

   DEVTABLE                * dt = NULL;

   if ((dt = xmalloc(sizeof(*dt),3,pinned_heap)) != NULL)
   {
      memset((caddr_t)dt,0x00,sizeof(*dt));
      strcpy(dt->iCatcher,"DVT");
   }
   else
      XMFAIL(sizeof(*dt));

   return dt;
}

/*---------------------------------------------------------------------------*/
/*                  Free memory occupied by a Device Table                   */
/*---------------------------------------------------------------------------*/

void
   free_DEVTABLE(
      DEVTABLE             * dt           // IO-Device Table
   )
{
   FUNC_NAME(free_DEVTABLE);

   if (xmfree(dt,pinned_heap) != 0) XFFAIL();
}

/*---------------------------------------------------------------------------*/
/*           Convert a device number to an internal device handle            */
/*---------------------------------------------------------------------------*/

int
   mkDevHandle(
      register DEVTABLE    * dt          ,// I -Device Table
      register dev_t         devno       ,// I -Device Number
      register DEVHANDLE   * dhp          //  O-Ptr to output handle
   )
{
   FUNC_NAME(mkDevHandle);

   register int              i;
   register int              devMajor = major(devno);
   register int              devMinor = minor(devno);
   int                       rc       = 0;

   if (devMinor >= 0 && devMinor < CIE_MAX_DEV)
   {
      /*-----------------------------------------*/
      /*  Find major number in the device table  */
      /*-----------------------------------------*/

      for (i=0; i<dt->majCount && dt->devMajor[i].num!=devMajor; i++);

      if (i < CIE_NUM_MAJ)                 // No Overflow?
      {
         if (i == dt->majCount)            // New major number?
         {
            /*-------------------------------------------------------*/
            /*  Allocate a new major number and init devtable entry  */
            /*-------------------------------------------------------*/

            register int           j;

            dt->majCount++;

            for (j=0; j<CIE_MAX_DEV; j++) dt->devMajor[i].dev[j] = NULL;

            dt->devMajor[i].num      = devMajor;
            dt->devMajor[i].devCount = 0;
         }

         dhp->devIndex = i;
         dhp->devMinor = devMinor;
      }
      else
      {
         rc = ENOSPC;
         TRC_OTHER(mkdo,devno,i,0);
      }
   }
   else
   {
      rc = ENODEV;
      TRC_OTHER(mkdn,devno,0,0);
   }

   return rc;
}
