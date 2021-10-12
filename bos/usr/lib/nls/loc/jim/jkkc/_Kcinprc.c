static char sccsid[] = "@(#)90	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcinprc.c, libKJI, bos411, 9428A410j 6/4/91 12:52:50";
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
 * MODULE NAME:       _Kcinprc
 *
 * DESCRIPTIVE NAME:  INPUT  PROCESS
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS):   success
 *                    0x1104(JTEOVER):   JTE table overflow
 *                    0x1404(FTEOVER):   FTE table overflow
 *                    0x1704(BTEOVER):   BTE table overflow
 *                    0x1804(PTEOVER):   PTE table overflow
 *                    0x7fff(UERROR):    unpredictable error
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
short _Kcinprc(z_kcbptr)

struct KCB      *z_kcbptr;              /* get address of KCB           */
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern short  _Kcinchk();            /* input data check routine     */
   extern short  _Kcimora();            /* change 7bit code to mora code*/
   extern short  _Kcilbst();            /* proceed left character       */

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define Z_ADD1   0x01ff                 /* 1 yomi is added              */
#define Z_PART   0x02ff                 /* only the last yomi is changed*/
#define Z_NEW    0x04ff                 /* new environment              */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short    z_rtinchk;                  /* define retern code of inchk  */
   short    z_rtimora;                  /* define retern code of imora  */
   short    z_rtilbst;                  /* define retern code of ilbst  */

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set base address of KCB      */

/*----------------------------------------------------------------------*
 *       CHECK INPUT YOMI & TRANSLATING 7BIT YOMI CODE                  
 *----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*
 *       1. CHECK EXISTING ENVIRONMENT                                
 *----------------------------------------------------------------------*/
   z_rtinchk = _Kcinchk(z_kcbptr);      /* check input yomi code        */

   switch( z_rtinchk )
   {
      case Z_ADD1:                      /* appended one char.           */
      case Z_PART:                      /* changed last one char.       */
      case Z_NEW:                       /* New environment              */
                     break;
                                        /* no change                    */      
      default:       return( z_rtinchk );
   }

/*----------------------------------------------------------------------* 
 *       2. TRANSLATE ALL NEW INPUT PHONETIC INTO MORA CODE             
 *----------------------------------------------------------------------*/
   z_rtimora = _Kcimora( z_kcbptr, z_rtinchk );
                                        /* check input yomi code        */
   if ( z_rtimora != SUCCESS )               
      return( z_rtimora );

/*----------------------------------------------------------------------* 
 *       3. CREATE DUMMY PHRASE NODE                                     
 *----------------------------------------------------------------------*/
   if ( z_rtinchk == Z_NEW )            /* new environment              */
   {
      z_rtilbst = _Kcilbst(z_kcbptr);   /* check input yomi code        */

      if ( z_rtilbst != SUCCESS )
         return(z_rtilbst);          /* then return with ret. code   */
   }

/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/
   return( SUCCESS );
}
