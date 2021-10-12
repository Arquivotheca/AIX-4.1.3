static char sccsid[] = "@(#)85	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcilbst.c, libKJI, bos411, 9428A410j 6/4/91 12:51:59";
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
 * MODULE NAME:       _Kcilbst
 *
 * DESCRIPTIVE NAME:  PROCEED LEFT  CHARACTER WHITCH IS SPECIFIED BY
 *                    KKC USER.
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS): success
 *                    0x1104 (JTEOVER): JTE table overflow
 *                    0x1404 (FTEOVER): FTE overflow
 *                    0x1704 (BTEOVER): BTE overflow
 *                    0x1804 (PTEOVER): PTE overflow
 *                    0x7fff (UERROR) : unpredictable error
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
short _Kcilbst( z_kcbptr )
struct KCB  *z_kcbptr;                  /* get KCB base address         */
{ 
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern short           _Kcjsjrt();   /* Set Data on JTE              */
   extern short           _Kcbproc();   /* Create BTE from JTE & FTE    */
   extern void            _Kcfinit();   /* Initialze FUZOKUGO-TABLE     */

/*----------------------------------------------------------------------*
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcbte.h"   /* Bunsetsu Table Entry (BTE)                   */
#include   "_Kcgpw.h"   /* General Purpose Workarea (GPW)               */
 
/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define    Z_HIN_NSK   29               /* hinpos which is noun sahen   */
                                        /* keiyou-doushi                */
#define    Z_HIN_PRE   106              /* hinpos which is prefix       */
#define    Z_HIN_SK    82               /* hinpos which is sahen keiyou */

#define    Z_FREQ5     0x05             /* frequency 5                  */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short           z_rjsjrt;    /* Define Area for Return of _Kcjsjrt   */
   short           z_rbproc;    /* Define Area for Return of _Kcbproc   */

   uschar          z_hinpos;    /* set jrt-go hinshi                    */
   uschar          z_flag[2];   /* set jrt-go flag                      */
   uschar          z_morap;     /* set jrt-go start mora pointer        */
   uschar          z_moraln;    /* set jrt-go mora length               */
   uschar          z_type;      /* set jrt-go type                      */
   short           z_usage;     /* set count of usage                   */
   struct  JKJ    *z_kjptr;     /* set jrt-go kanji pointer             */
   short           z_endflg;    /* end flag                             */

/*----------------------------------------------------------------------*
 *      START OF PROCESS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;
   gpwptr1 = kcb.gpwgpe;

   z_hinpos  = Z_HIN_NSK;               /* set meishi, sahen, keiyo-dous*/
   z_flag[0] = FREQ;                    /* set default flag             */
   z_flag[1] = 0;                       /* set default flag             */
   z_morap   = 0;                       /* set default start mora ptr   */
   z_moraln  = 0;                       /* set default mora length      */
   z_type    = TP_IPPAN;                /* set default type             */
   z_kjptr   = NULL;                    /* set default kanji pointer    */
   z_usage   = 0;                       /* reset usage count            */

   switch(kcb.charl)                    /* Branch by left character     */
   {
      case  LR_UNDEFINED     :          /* Undefind                     */
         z_type = TP_CONVKEY;
         z_hinpos = Z_HIN_PRE;
         break;
         
      case  LR_KUTEN         : 
         z_type = TP_KUTEN;
         z_hinpos = Z_HIN_PRE;
         break;
         
      case  LR_TOUTEN        : 
         z_type = TP_TOUTEN;
         z_hinpos = Z_HIN_PRE;
         break;
         
      case  LR_KIGO1         : 
         z_hinpos = Z_HIN_NSK;
         z_flag[0] = NULL;
         break;
         
      case  LR_KIGO2         : 
         z_type = TP_NSETTO;
         z_hinpos = Z_HIN_PRE;

         z_rjsjrt =
           _Kcjsjrt(z_kcbptr,z_hinpos,z_flag,z_morap,z_moraln,z_type,z_kjptr);
                                        /* create dummy jiritsugo entry */
         if ( z_rjsjrt != SUCCESS )
            return(z_rjsjrt);           /* return with error code       */

         z_type = TP_SUUCHI;
         z_flag[0] = NULL;
         z_hinpos = Z_HIN_NSK;  /* set meishi, sahen, keiyo-dousi       */

         break;
         
      case  LR_KIGO3         : 
         z_type = TP_JOSUSHI;
         break;

      case  LR_ALPHABET      : 
         z_flag[0] = (SETTO_OK | SETSUBI_OK | FREQ );

         z_rjsjrt =
           _Kcjsjrt(z_kcbptr,z_hinpos,z_flag,z_morap,z_moraln,z_type,z_kjptr);
                                        /* create dummy jiritsugo entry */
         if ( z_rjsjrt != SUCCESS )
            return(z_rjsjrt);           /* return with error code       */

       /*z_type = TP_KOYUU;                                           */
       /*z_flag[0] = (PRONOUN_SEI | PRONOUN_MEI | PRO_CHIMEI | FREQ); */

       /*z_rjsjrt =                                                           */
       /*  _Kcjsjrt(z_kcbptr,z_hinpos,z_flag,z_morap,z_moraln,z_type,z_kjptr);*/
                                        /* create dummy jiritsugo entry */
       /*if ( z_rjsjrt != SUCCESS ) */
       /*   return(z_rjsjrt);       */  /* return with error code       */

         z_type     = TP_IPPAN;         /* set jrt-go type              */
         z_hinpos   = Z_HIN_SK;         /* sahen_meishi_ka , keiyou_dous*/
         z_flag[0]  = (SETTO_OK | SETSUBI_OK | Z_FREQ5 );

         break;

      case  LR_KATAKANA      : 
         z_hinpos = Z_HIN_NSK;
/*       z_rjsjrt =
           _Kcjsjrt(z_kcbptr,z_hinpos,z_flag,z_morap,z_moraln,z_type,z_kjptr);*/
                                        /* create dummy jiritsugo entry */
     /*  if ( z_rjsjrt != SUCCESS )
            return(z_rjsjrt);*/         /* return with error code       */

    /*   z_type = TP_KOYUU;
         z_flag[0] = (PRONOUN_SEI | PRONOUN_MEI | PRO_CHIMEI | FREQ);*/

         break;

      case  LR_NUMERIC       :
         z_type = TP_SUUCHI;
         break;

      case  LR_RIGHTPAREN    : 
      case  LR_ABBRIVIATION  : 
         z_hinpos = Z_HIN_NSK;
         z_flag[0] = FREQ;
         break;

      case  LR_KIGO4         : 
      case  LR_LEFTPAREN     : 
      case  LR_DELIMITER     : 
      default:
         z_hinpos = Z_HIN_PRE;
         break;
   }
   z_rjsjrt =
       _Kcjsjrt(z_kcbptr,z_hinpos,z_flag,z_morap,z_moraln,z_type,z_kjptr);
                                        /* create dummy jiritsugo entry */
   if ( z_rjsjrt != SUCCESS )
      return(z_rjsjrt);                 /* return with error code       */

/*----------------------------------------------------------------------*  
 *       SET UP DUMMY BTE                                                  
 *----------------------------------------------------------------------*/ 
   _Kcfinit(z_kcbptr);                  /* initialaze fzk table         */


   z_rbproc = _Kcbproc( z_kcbptr,(short)(-1),(uschar)0);
                                        /* create dummy bunsetsu  entry */
   if ( z_rbproc != SUCCESS )
      return(z_rbproc);              /* return with error code       */

/*----------------------------------------------------------------------*  
 *       SET UP DUMMY BTE
 *----------------------------------------------------------------------*/ 
   if ( CACTV( kcb.bthchh ) == 0 )
      return( SUCCESS );

   FOR_FWD(kcb.bthchh,bteptr1,z_endflg) /* loop for all BTE             */
   {
      LST_FWD(bteptr1,z_endflg);

      bte.dmyflg = ON;                  /* dummy flag on                */
   }

   gpw.leftflg = ON;                    /* set left flag                */

   return(SUCCESS);                     /* normal end                   */
}                                       /* end of program               */
