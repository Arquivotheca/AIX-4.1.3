static char sccsid[] = "@(#)39	1.3  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcliccs.c, libKJI, bos411, 9428A410j 5/18/93 05:28:41";
/*
 * COMPONENT_NAME: (libKJI) Japanese Input Method (JIM)
 *
 * FUNCTIONS: Kana-Kanji-Conversion (KKC) Library
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kcliccs
 *
 * DESCRIPTIVE NAME:  System Dictionary Check
 *
 * INPUT: 
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS)     : success
 *                    0x0110(EXIST_SYS)   : goku exists in system dict.
 *                    0x0510(EQ_YOMI_GOKU): goku is equal to yomi
 *                    0x7FFF(UERROR )    : Fatal error
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
short _Kcliccs(z_kcbptr)

struct KCB      *z_kcbptr;              /* get address of KCB           */
{ 
/*----------------------------------------------------------------------*  
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/ 
   extern void            _Kcxinia();   /* Initialize Work Area         */
   extern short           _Kcc72hr();   /* Exchange YOMI to DBCS Hira.  */
   extern short           _Kcinprc();   /* Input Process                */
   extern short           _Kcjsjrt();   /* Set Data on JTE              */
   extern short           _Kcbproc();   /* Create BTE from JTE & FTE    */
   extern struct RETNMTCH _Kcnmtch();   /* Select BTE Having Same Seisho*/

/*----------------------------------------------------------------------*  
*       INCLUDE FILES                                                   *  
*-----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcsei.h"   /* SEIsho buffer entry (SEI)                    */
#include   "_Kcymi.h"   /* YoMI buffer (YMI)                            */
#include   "_Kcgpw.h"   /* General Purpose Workarea (GPW)               */
#include   "_Kcmcb.h"   /* Monitor control block (MCB)                  */
 
/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/ 
#define   Z_NEW     4                   /* New environment              */
#define   Z_EXIST  (0x0000 | LOCAL_RC)  /* return of _Kcnmtch           */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   short           z_rc72hr;    /* Define Area for Return of _Kcc72hr   */
   short           z_rjsdct;    /* Define Area for Return of _Kcjsdct   */
   short           z_rimora;    /* Define Area for Return of _Kcimora   */
   short           z_rbproc;    /* Define Area for Return of _Kcbproc   */
   struct RETNMTCH z_rnmtch;    /* Define Area for Return of _Kcnmtch   */
   uschar     z_length;                 /* define mora length of target */
   short      z_seilen;                 /* define seisho length         */
   short      z_i;                      /* define loop counter          */
   short           z_file;      /* System File Number                   */

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
   kcbptr1 = z_kcbptr;                  /* set base address of KCB      */ 
   gpwptr1 = kcb.gpwgpe;                /* set base address of GPW      */ 
   mcbptr1 = ( struct MCB *)kcb.myarea; /* Get MCB pointer              */
   _Kcxinia( z_kcbptr );                /* initialize Internal Word Area*/

   gpw.pmode = kcb.mode;                /* save mode                    */
   kcb.env  = ENVDICT;                  /* Save Environment             */
   kcb.ymill2 = kcb.ymill1;             /* Only One bunsetsu            */

/*----------------------------------------------------------------------*  
 *      CHECK YOMI & SEISHO
 *----------------------------------------------------------------------*/ 
   ymiptr1 = kcb.ymiaddr;               /* set yomi address             */
   seiptr1 = kcb.seiaddr;               /* set yomi address             */
   if((CHPTTOSH(sei.ll)-2)/2 == kcb.ymill1)
                                        /* compare seisho and yomi len  */
   {
      for( z_i = 0; z_i < kcb.ymill1 ; z_i++ )
      {
         if((short)_Kcc72hr(ymi.yomi[z_i]) != (short)CHPTTOSH(sei.kj[z_i]))
         {
            break;
         }
      }
      if(z_i == kcb.ymill1)
         return( EQ_YOMI_GOKU );
   }

/*----------------------------------------------------------------------  
 *      TRANSLATE ALL NEW INPUT PHONETIC INTO MORA CODE 
 *----------------------------------------------------------------------*/ 

   z_rimora = _Kcimora(z_kcbptr,(short)Z_NEW);
                                        /* check input yomi code        */
   if (z_rimora != SUCCESS )            /* error handling               */
      return( UERROR );              /* then return with ret. code   */

   gpw.tbflag = ON;                     /* set single flag ON           */

/*----------------------------------------------------------------------*  
 *       HIRAGANA MORA, NORMAL YOMI PROCESS                              
 *----------------------------------------------------------------------*/ 
   z_length = kcb.mchacmce ;            /* set length of target         */

/*----------------------------------------------------------------------*  
 *       LOOK UP WORD IN SYSTEM DICTIONARY                               
 *----------------------------------------------------------------------*/ 
   for( z_file = 0; z_file < mcb.mul_file; z_file++ ) {
      z_rjsdct = _Kcjsdct(z_kcbptr,0,z_length,(short)SPECIFIC,z_file);
                                        /* look up in system dictionary */
      if ( z_rjsdct != SUCCESS )        /* error handling               */
         continue; 

      if( CACTV( kcb.jthchh ) != 0)     /* if Entry was found           */
         break;
   }
   if( z_file >= mcb.mul_file)          /* not found                    */
      return(SUCCESS);                  /* return                       */

/*----------------------------------------------------------------------*  
 *       MAKE BTE                                                        
 *----------------------------------------------------------------------*/ 
   z_rbproc =
         _Kcbproc(z_kcbptr,(short)(kcb.mchacmce-1),(uschar)0);
   if (z_rbproc != SUCCESS )
      return( UERROR );

/*----------------------------------------------------------------------*  
 *       CHECK IF SEISHO IS UNIQUE                                        
 *----------------------------------------------------------------------*/ 
   seiptr1 = kcb.seiaddr;               /* set seisho address           */
   z_rnmtch =
      _Kcnmtch( kcbptr1,(uschar *)seiptr1+2,(short)(CHPTTOSH(sei.ll)-2));
                                        /* call nmtch to find a same    */
   if( z_rnmtch.rc == Z_EXIST )         /* Already exist in Sys. Dict.  */
   {
      return( EXIST_SYS );              /* Return with error code       */
   }

   return(SUCCESS);
}                                       /* end of program               */
