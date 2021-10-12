static char sccsid[] = "@(#)71	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcfproc.c, libKJI, bos411, 9428A410j 6/4/91 12:49:06";
/*
 * COMPONENT_NAME: (libKJI) Japanese Input Method (JIM)
 * 
 * FUNCTIONS: Kana-Kanji-Conversion (KKC) Library
 *
 * ORIGINS: 27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when 
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kcfproc
 *
 * DESCRIPTIVE NAME:  CHECK AND LOOK UP FUZOKUGO DICTIONARY
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS) : Success
 *                    0x1404(FTEOVER) : FTE overflow
 *                    0x1704(BTEOVER) : BTE overflow
 *                    0x1804(PTEOVER) : PTE overflow
 *                    0x7FFF(UERROR ) : Fatal error
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------*
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#if defined(_AGC) || defined(_GSW)
#include   "_Kcfnm.h"                   /* Define Names file            */
#endif
#include   "_Kcmap.h"                   /* Define Constant file         */
#include   "_Kcrcb.h"                   /* Define Return Code Structure */

 
/************************************************************************
 *      START OF FUNCTION
 ************************************************************************/
short  _Kcfproc(z_kcbptr,z_dctctl,z_mode)
struct KCB      *z_kcbptr;              /* get address of KCB           */
short  z_dctctl;                        /* dict. looking up cntrl info. */
short  z_mode;                          /* dict. looking up mode        */
  
{ 
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern short           _Kcfchek();   /* Judge Object of Looking up   */
   extern void            _Kcfinit();   /* Initialze FUZOKUGO-TABLE     */
   extern short           _Kcffdct();   /* Construct FTE                */
   extern short           _Kcbproc();   /* Create BTE from JTE & FTE    */

/*----------------------------------------------------------------------*
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcgpw.h"   /* General Purpose Workarea (GPW)               */
 
/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short           z_rfchek;    /* Define Area for Return of _Kcfchek   */
   short           z_rffdct;    /* Define Area for Return of _Kcffdct   */
   short           z_rbproc;    /* Define Area for Return of _Kcbproc   */

   short           z_stpos ;    /* define start                         */
   short           z_anapos;    /* define analasys position             */

/*----------------------------------------------------------------------*
 *      START OF PROCESS
 *----------------------------------------------------------------------*/
   /*-------------------------------------------------------------------*  
    *      SET BASE ADDRESS
    *-------------------------------------------------------------------*/ 
   kcbptr1 = z_kcbptr;                  /* set base address of KCB      */ 
   gpwptr1 = kcb.gpwgpe;                /* set base address of MCE      */

   /*-------------------------------------------------------------------*  
    *       CALL CHECK ROUTINE                                           
    *-------------------------------------------------------------------*/ 
   z_rfchek = _Kcfchek(z_kcbptr,z_dctctl,z_mode);
                                        /* check looking up target      */

   /*-------------------------------------------------------------------*  
   *       SET START POSIOTION                                          *  
   *--------------------------------------------------------------------*/ 
   if ((z_mode == ABS)&&                /* if abusolute conversion and  */
       (gpw.tbflag == ON))              /* tabuntsu flag is ON          */

   {
      z_stpos = z_rfchek - 2;           /* set start pos                */
      gpw.pendpos = z_rfchek;           /* save endpos as previous endpo*/
   }
   else
   {
      z_stpos = gpw.pendpos;            /* set start pos as previous end*/
   }

   /*-------------------------------------------------------------------*  
    *       ANALIZE LOOP                                                   
    *-------------------------------------------------------------------*/ 
   for (z_anapos=z_stpos+1;             /* set init analasys position   */
        z_anapos<z_rfchek;
        z_anapos++)
                                        /* count up                     */
   {
      if (z_anapos < 0)                 /* if anapos is negative        */
         continue;                      /* not look up                  */

      _Kcfinit(z_kcbptr);               /* initialaze fzk table         */

      gpw.pendpos = z_anapos;           /* set start pos as previous end*/

      z_rffdct = _Kcffdct(z_kcbptr,z_anapos);
                                        /* look up fuzokugo dictionary  */
                                        /* function value is num of fzk */

      /*----------------------------------------------------------------*  
       *       BUNSETSU PROCESS                                       
       *----------------------------------------------------------------*/ 
      z_rbproc =_Kcbproc(z_kcbptr,(short)z_anapos,
                         (uschar)z_dctctl);
                                        /* call bunsetsu process        */
      if ( z_rbproc != SUCCESS )
         return( z_rbproc );         /* return with error code       */

      if(gpw.kakuflg == ON)
         break;
   }                                    /* end of for loop              */

   /*-------------------------------------------------------------------*  
    *       END PROCESS                                                    
    *-------------------------------------------------------------------*/ 
   return( SUCCESS );                   /* return normal                */
}                                       /* end of program               */
