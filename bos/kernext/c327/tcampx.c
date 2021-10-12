static char sccsid[] = "@(#)54	1.16  src/bos/kernext/c327/tcampx.c, sysxc327, bos411, 9428A410j 10/31/90 14:36:03";
/*
 * COMPONENT_NAME: (SYSXC327) c327 tca device driver entry points
 *
 * FUNCTIONS:    getLaNum(), initDdi(), mhatoi(), tcampx()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

/***********************************************************************/
/*  IBM CONFIDENTIAL                                                   */
/*  Copyright International Business Machines Corp. 1985, 1988         */
/*  Unpublished Work                                                   */
/*  All Rights Reserved                                                */
/*  Licensed Material -- Program Property of IBM                       */
/*                                                                     */
/*  Use, Duplication or Disclosure by the Government is subject to     */
/*  restrictions as set forth in paragraph (b)(3)(B) of the Rights in  */
/*  Technical Data and Computer Software clause in DAR 7-104.9(a).     */
/***********************************************************************/

/*
** INCLUDE FILES
*/
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/errno.h>
#include <sys/intr.h>
#include <sys/io3270.h>
#include "c327dd.h"
#include "dftnsDcl.h"
#include "tcadefs.h"
#include "tcadecls.h"
#include "tcaexterns.h"
/*
** static function prototypes
*/
static int getLaNum(int);
static char mhatoi(char);
static void initDdi(int);
/*PAGE*/
/*******************************************************************
**
** Function Name:       tcampx 
**
** Description:
**obtain session number from file name extension
**
** Inputs:      dev             device minor number
**
** Outputs:     0               operation was successful
**              -1              operation not successful
**
** Externals  
** Referenced 
**
** Externals 
** Modified 
**
*******************************************************************/
tcampx (dev_t devt, int *laNum, char *channame)
{
   int   dev;

   dev = minor(devt);

   C327TRACE4("MpxS", dev, *laNum, *channame);
   C327TRACE4("Mpx1", *(channame+1), *(channame+2), *(channame+3));

   /* ------------------------------------------------------ */
   /* make sure the device c327 has been added to the system */
   /* remember we start at session 1, not 0                  */
   /* ------------------------------------------------------ */
   if (!tca_data[dev].INIT_COMPLETE)
      initDdi(dev);

   if (channame == NULL){  /* channel needs to be deallocated */
      *laNum = 0;  /* closing time */
      C327TRACE1("Mmx4");
      return( SUCCESS );  /* exit tcampx */
   }

   if (*channame == 'P'){
      if ( (*(channame+1) != '/') ||
           ( (printerAddr = (mhatoi(*(channame+2)) * 16) +
         mhatoi(*(channame+3)) ) > 0x1F) ){
         C327TRACE2("Bmx0",printerAddr);
         return(ENXIO);                /* exit tcampx */
      }
      tca_sess_type = SESS_TYPE_PRINTER;
      C327UNPTRACE2("Mmx0",printerAddr);
      *laNum = getLaNum(dev);  /* make up good laNum */
   }
   else{
      tca_sess_type = SESS_TYPE_TERMINAL;
      if (*channame >= '0'){
         *laNum = (mhatoi(*channame) * 16) + mhatoi(*(channame+1));
      }
      else{
         if (*channame == 0) {
            *laNum = getLaNum(dev);
         } else{
            C327TRACE1("Bmx2");
            return(ENODEV);/* exit tcampx */
         }
      }
   }

   if ( (*laNum == 0) || (*laNum > MAX_SESSIONS) ){
      C327TRACE2("Bmx1", *laNum);
      return(ENXIO);/* exit tcampx */
   }

   C327TRACE3("MpxE", *laNum, tca_sess_type);
   return( SUCCESS );/* exit tcampx */
}
/*PAGE*/
/**************************************************************************
** getLaNum:
**      Get next free link address number
***************************************************************************/
static int getLaNum(int dev)
{
   int  session;

   for (session=1;session <= MAX_SESSIONS ;session++){
      if (tca_data[dev].mlnk_ptrs[session] == NULL )
         break;
   }
   return(session);
}
/*PAGE*/
/***************************************************************************
** mhatoi:
**      convert character from '0'-'9',A-F,a-f to numeric equivalent
*
**************************************************************************/
static char mhatoi(char n)
{

   if(n >= '0' && n <= '9') /* try for an ascii number */
      n -= '0';
   else {                          /* try for an ascii character */
      n |= 0x20; /* set for lower case conversion */
      if(n >= 'a' && n <= 'f'){
         n += 10;        /* offset from ascii numbers */
         n -= 'a';       /* numbers range from 10 to 15 */
      } else
         n = 0;          /* invalid convert input */
   }
   return(n);
}

/***************************************************************************
**Function:
***************************************************************************/
static void initDdi (int dev)
{
   int     session;

   tca_data[dev].lower_link_address = 1;
   tca_data[dev].upper_link_address = MAX_SESSIONS;

   for(session = 1; session <= MAX_SESSIONS; session++)
      tca_data[dev].mlnk_ptrs[session] = (linkAddr *)NULL;

   /* state variable set upon successful completion of init routine */
   tca_data[dev].INIT_COMPLETE = SET;
}
