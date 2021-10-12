static char sccsid[] = "@(#)25	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kckcanr.c, libKJI, bos411, 9428A410j 7/23/92 03:15:58";
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
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kckcanr
 *
 * DESCRIPTIVE NAME:  MAKE ROMAN NUMERIC CANDIDATES
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS):    success
 *                    0x1104 (JTEOVER):    JTE table overflow
 *                    0x1204 (JKJOVER):    JKJ table overflow
 *                    0x7fff (UERROR) :    unpredictable error
 *
 ******************** END OF SPECIFICATIONS *****************************/
                                                                          
/*----------------------------------------------------------------------*
 *       COPYRIGHT
 *----------------------------------------------------------------------*/
#ifdef _AIX
static char *cprt1="5601-061 COPYRIGHT IBM CORP 1989           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";
#endif

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
short  _Kckcanr( z_kcbptr, z_strpos, z_length )
struct  KCB  *z_kcbptr;
uschar        z_strpos;                 /* pointer of 1st MCE           */
uschar	      z_length;
{
/*----------------------------------------------------------------------*  
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/ 
   extern   struct RETCGET   _Kccget();
   extern   short            _Kcjsjrt();

/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcjkj.h"   /* JKJ  Code table Entry (JKJ)                  */
#include   "_Kcmce.h"   /* Mora Code table Entry (MCE)  		*/

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/ 
#define   Z_HIN_DUMMY  127

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   short            z_i;
   short            z_rjsjrt;
   struct RETCGET   z_rcget;
   uschar           z_flag[2];
   uschar           z_kj[2][2];

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
   kcbptr1 = z_kcbptr;                  /* establish address   to kcb   */

/*----------------------------------------------------------------------*
 *      INITIALIZE
 *----------------------------------------------------------------------*/
   z_kj[0][0] = z_kj[0][1] = z_kj[1][0] = z_kj[1][1] = NULL;
   z_flag[0] = NUM_FLAG0;
   z_flag[1] = NUM_FLAG1;

/*----------------------------------------------------------------------*
 *      SET & GET JKJ
 *----------------------------------------------------------------------*/
   /*----------------------  GET JKJ ENTRY  -------------------------*/
   mceptr1 = kcb.mchmce + z_strpos;
   if((( z_length + 1 ) == 2 ) && 
      ((mce.code >= NUM_MORA_1) && (mce.code <= NUM_MORA_9))) {
      z_kj[0][0] = z_kj[1][0] = (ROMA_H_PCCODE & 0x7f);
      z_kj[0][1] = ROMAL_L_PCCODE + mce.code - NUM_MORA_1;
      z_kj[1][1] = ROMAS_L_PCCODE + mce.code - NUM_MORA_1;
   }
   else {
      if((( z_length + 1 ) == 3 ) && ( mce.code == NUM_MORA_1)) {
	 mceptr1++;
         if(mce.code == NUM_MORA_0) {
            z_kj[0][0] = z_kj[1][0] = (ROMA_H_PCCODE & 0x7f);
            z_kj[0][1] = ROMAL_L_PCCODE + 9;
            z_kj[1][1] = ROMAS_L_PCCODE + 9;
         }
      }
   }

   if(z_kj[0][0] != NULL ) {            /* make roman numeric candidates*/
      for( z_i = 0; z_i < 2; z_i ++ ) {
         z_rcget = _Kccget(&kcb.jkhchh);
         if(z_rcget.rc == GET_EMPTY) {
            return( JKJOVER );
         }
         jkjptr1 = (struct JKJ *)z_rcget.cheptr;
         jkj.kj[0] = z_kj[z_i][0];
         jkj.kj[1] = z_kj[z_i][1];
         z_rjsjrt = _Kcjsjrt( z_kcbptr,         /* pointer of KCB       */
                             NUM_HINSI,         /* hinshi               */
                                z_flag,         /* dflag                */
                              z_strpos,         /* mora position        */
                              z_length,         /* mora length          */
                              NUM_TYPE,         /* type                 */
                              jkjptr1 );        /* pointer of first JKJ */

         if ( z_rjsjrt != SUCCESS )
            return( z_rjsjrt );
      }
   }
/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/
   return( SUCCESS );
}
