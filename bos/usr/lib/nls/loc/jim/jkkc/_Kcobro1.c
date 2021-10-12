static char sccsid[] = "@(#)57	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcobro1.c, libKJI, bos411, 9428A410j 6/4/91 15:16:28";
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
 * MODULE NAME:       _Kcobro1
 *
 * DESCRIPTIVE NAME:  SELECT ALL CANDIDET, AND MAKE A ENVIRONMENT TO
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS): success
 *                    0x1104 (JTEOVER): JTE overflow
 *                    0x1204 (JKJOVER): JKJ overflow
 *                    0x1304 (JLEOVER): JLE overflow
 *                    0x1404 (FTEOVER): FTE overflow
 *                    0x1704 (BTEOVER): BTE overflow
 *                    0x1804 (PTEOVER): PTE overflow
 *                    0x0108 (WSPOVER): work space overflow
 *                    0x7fff (UERROR):  unpredictable error
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
short   _Kcobro1( z_kcbptr )
struct  KCB     *z_kcbptr;              /* pointer of KCB              */
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern void            _Kcxinia();   /* Initialize Work Area         */
   extern short           _Kcimora();   /* Set 7bit Yomi & Mora Code    */
   extern short           _Kcjproc();   /* JIRITSUGO Process            */
   extern short           _Kcfproc();   /* FUZOKUGO Process             */
   extern struct RETNCUTB _Kcncutb();   /* Delete Duplicate BTE         */
   extern struct RETCBKNJ _Kccbknj();   /* Get BTE Seisho               */
   extern struct RETCGET  _Kccget();    /* Get  Any Kinds of Table Entry*/
   extern short           _Kccinsb();   /* Insert Before     Table Entry*/

/*----------------------------------------------------------------------*
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcgpw.h"   /* General Purpose Workarea (GPW)               */
#include   "_Kcsem.h"   /* SEisho Map entry (SEM)                       */
#include   "_Kcbte.h"   /* Bunsetsu Table Entry (BTE)                   */
#include   "_Kcsei.h"   /* SEIsho buffer entry (SEI)                    */
#include   "_Kcgrm.h"   /* GRammer Map entry (GRM)                      */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define   Z_NEW        4
#define   Z_OVRFLW    (0x0200 | LOCAL_RC)
#define   Z_HR_HIGH    0x82
#define   Z_HR_LOWL    0x9f
#define   Z_HR_LOWH    0xf1

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short           z_rimora;    /* Define Area for Return of _Kcimora   */
   short           z_rjproc;    /* Define Area for Return of _Kcjproc   */
   short           z_rfproc;    /* Define Area for Return of _Kcfproc   */
   struct RETNCUTB z_rncutb;    /* Define Area for Return of _Kcncutb   */
   struct RETCBKNJ z_rcbknj;    /* Define Area for Return of _Kccbknj   */
   struct RETCGET  z_rcget ;    /* Define Area for Return of _Kccget    */
   short           z_rcinsb;    /* Define Area for Return of _Kccinsb   */

   uschar          z_buf[SEIMAX];/* work area                           */
   uschar          z_flg;       /* seisho map save area                 */
   short           z_i;         /* counter                              */
   short           z_fznum;     /* fuzoku-go hyki no.                   */
   short           z_hiraga;    /* hiragana  hyki flag                  */
   short           z_endflg;    /* end flag                             */
   short           z_maxsei;    /* max seisho length                    */

/*----------------------------------------------------------------------*
 *      START OF PROCESS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* initialize of kcbptr1       */
   gpwptr1 = kcb.gpwgpe;                /* initialize of gpwprt1       */
   semptr1 = kcb.semaddr;               /* initialize of semptr1       */
   seiptr1 = kcb.seiaddr;               /* initialize of seiptr1       */
   grmptr1 = kcb.grmaddr;               /* initialize of grmptr1       */

/*---------------------------------------------------------------------*
 *      CALL KKCINCHK() & KKCIMORA()
 *---------------------------------------------------------------------*/
   _Kcxinia( z_kcbptr );

   gpw.pmode = kcb.mode;
   kcb.ymill2 = kcb.ymill1;             /* Only One bunsetsu            */

   z_rimora = _Kcimora(z_kcbptr,(short)Z_NEW);
   if ( z_rimora != SUCCESS )
      return(UERROR);

   gpw.tbflag = ON;                     /* set single flag ON          */

   if ( ( sem.ll[0] == 0x00 ) && ( sem.ll[1] == 0x02 ) )
   {
      z_flg = sem.code[2];
      grmptr1 = (struct GRM *)((uschar *)grmptr1 + 1);
      seiptr1 = (struct SEI *)((uschar *)seiptr1 + 2);
   }
   else
      z_flg = sem.code[0];

   switch( z_flg )
   {
      /*----------------------------------------------------------------*/
      /*        FUZOKU-GO ONLY WITHOUT HOMONYM                          */
      /*----------------------------------------------------------------*/
      case ALLFZK :
         /*----------   candidates having fuzoku-go only   -------------*/   
         z_rcget = _Kccget(&kcb.bthchh);
         if(z_rcget.rc == GET_EMPTY)
         {
            return( BTEOVER );
         }
         else
         {
            if((z_rcget.rc != GET_TOP_MID )&&
               (z_rcget.rc != GET_LAST    ))
               return( UERROR );
         }

         bteptr2 = (struct BTE *)z_rcget.cheptr;
         bte2.hinl = 0;
         bte2.hinr = 0;
         bte2.stap = 0;
         bte2.endp = kcb.mchacmce -1 ;
         bte2.jteaddr = NULL;
         bte2.kjf1 = NULL;
         bte2.kjh1 = NULL;
         bte2.kjf2 = NULL;
         bte2.kjh2 = NULL;
         bte2.usage = 0;
         bte2.pen = 0;
         bte2.dmyflg = OFF;
         bte2.fzkflg = F_FLG_NOEXT;

         /*-------   candidates having jiritsu-go & fuzoku-go   --------*/   
         kcb.charl = LR_UNDEFINED;
                                        /* create JTE                   */
         z_rjproc = _Kcjproc( z_kcbptr, kcb.mchacmce-1, (short)ABS );
         if ( z_rjproc != SUCCESS )
            return( z_rjproc );
                                        /* create FTE & BTE             */
         z_rfproc = _Kcfproc( z_kcbptr, kcb.mchacmce-1, (short)ABS );

         if( z_rfproc  != SUCCESS)
            return( z_rfproc );
                                        /* delete indentify BTE         */
         z_rncutb = _Kcncutb( z_kcbptr, (short)0 );
         if ( z_rncutb.rc != SUCCESS )
            return(z_rncutb.rc);

         if ( z_rncutb.delno < 0 )
            return((short)UERROR);

         break;

      /*----------------------------------------------------------------*/
      /*        FUZOKU-GO ONLY WITH HOMONYM                             */
      /*----------------------------------------------------------------*/
      case FZK_HOMO    :
      case ALLFZK_HOMO :
         /*----------   candidates having fuzoku-go only   -------------*/
         /*-------   get fuzoku-go number from input date       --------*/
         if( grm.ll == 2 )           /* if grammer map length = 2   */
         {
            z_fznum = (short)grm.byte[0];/* set FUZOKUGO number       */
         }
         else if( grm.ll == 3 )      /* if grammer map lingth = 3   */
         {
            z_fznum = (short)(grm.byte[0] & 0x7f);
            z_fznum <<= 8;
            z_fznum += (short)grm.byte[1];
         }                              /* caluculate & set FUZOKUGO No*/
         else
         {
            return( (short)UERROR );    /* grammer map isn't corect    */
         }

         /*-------   get kanji hyoki ON or OFF                  --------*/
         z_hiraga = ON;                 /* set seisyo code are HIRAGANA*/

         for ( z_i = 0; z_i < ( CHPTTOSH(sei.ll)- 2 ) / 2; z_i++ )
         {
            if( sei.kj[ z_i ][ 0 ] != Z_HR_HIGH )
            {
               z_hiraga = OFF;          /* there is KANJI              */
               break;
            }
            else
            {
               if((sei.kj[z_i][1]< Z_HR_LOWL)||(sei.kj[z_i][1]>Z_HR_LOWH))
               {
                  z_hiraga = OFF;          /* there is KANJI              */
                  break;
               }
            }
         }
         if( z_hiraga == ON )           /* all seisyo code are HIRAGANA*/
            z_fznum = z_fznum *(-1);

         /*-------   get same fuzoku-go as input data           --------*/
         z_rcget = _Kccget(&kcb.bthchh);
         if(z_rcget.rc == GET_EMPTY)
         {
            return( BTEOVER );
         }
         else
         {
            if((z_rcget.rc != GET_TOP_MID )&&
               (z_rcget.rc != GET_LAST    ))
               return( UERROR );
         }

         bteptr1 = (struct BTE *)z_rcget.cheptr;
         bte.hinl = 0;
         bte.hinr = 0;
         bte.stap = 0;
         bte.endp = kcb.mchacmce -1 ;
         bte.jteaddr = NULL;
         bte.kjf1 = 0;
         bte.kjh1 = z_fznum;
         bte.kjf2 = NULL;
         bte.kjh2 = NULL;
         bte.usage = 0;
         bte.pen = 0;
         bte.dmyflg = OFF;

         /*-------   get the other hyoki                        --------*/
         z_rcget = _Kccget(&kcb.bthchh);
         if(z_rcget.rc == GET_EMPTY)
         {
            return( BTEOVER );
         }
         else
         {
            if((z_rcget.rc != GET_TOP_MID )&&
               (z_rcget.rc != GET_LAST    ))
               return( UERROR );
         }

         bteptr2 = (struct BTE *)z_rcget.cheptr;
         bte2.hinl = 0;
         bte2.hinr = 0;
         bte2.stap = 0;
         bte2.endp = kcb.mchacmce -1 ;
         bte2.jteaddr = NULL;
         bte2.kjf1 = 0;
         bte2.kjh1 = bte.kjh1 * (-1);
         bte2.kjf2 = NULL;
         bte2.kjh2 = NULL;
         bte2.usage = 0;
         bte2.pen = 0;
         bte2.dmyflg = OFF;

         if ( z_fznum >= 0)
         {
            bte.fzkflg  = F_FLG_USE1;
            bte2.fzkflg = NULL;
         }
         else
         {
            bte.fzkflg  = NULL;
            bte2.fzkflg = F_FLG_USE1;
         }
         /*-------   candidates having jiritsu-go & fuzoku-go   --------*/
         kcb.charl = LR_UNDEFINED;
                                        /* create JTE                   */
         z_rjproc = _Kcjproc( z_kcbptr, kcb.mchacmce-1, (short)ABS );
         if ( z_rjproc  != SUCCESS )
            return( z_rjproc );
                                        /* create FTE & BTE             */
         z_rfproc = _Kcfproc( z_kcbptr, kcb.mchacmce-1, (short)ABS );

         if ( z_rfproc  != SUCCESS )
            return( z_rfproc );
                                        /* delete indentify BTE         */
         z_rncutb = _Kcncutb( z_kcbptr, (short)0 );
         if (z_rncutb.rc != SUCCESS)
            return(z_rncutb.rc);

         if ( z_rncutb.delno < 0 )
            return((short)UERROR);

         break;

      /*----------------------------------------------------------------*/
      /*        JIRITSU-GO                                              */
      /*----------------------------------------------------------------*/
      case JRT        :
      case PREFIX     :
      case SUFFIX     :
      case NUM_PREFIX :
      case NUM_SUFFIX :
      case PRO_PREFIX :
      case PRO_SUFFIX :
         kcb.charl = LR_UNDEFINED;
                                        /* create JTE                   */
         z_rjproc = _Kcjproc( z_kcbptr, kcb.mchacmce-1, (short)ABS );
         if( z_rjproc  != SUCCESS )
            return( z_rjproc );
                                        /* create FTE & BTE             */
         z_rfproc = _Kcfproc( z_kcbptr, kcb.mchacmce-1, (short)ABS );

         if ( z_rfproc  != SUCCESS )
            return( z_rfproc );
                                        /* delete indentify BTE         */
         z_rncutb = _Kcncutb( z_kcbptr, (short)1 );
         if(z_rncutb.rc != SUCCESS)
            return(z_rncutb.rc);

         FOR_FWD( kcb.bthchh , bteptr1 , z_endflg )
         {
            LST_FWD( bteptr1 ,z_endflg );

            if((bte.jteaddr == NULL )&&(bte.fzkflg ^ 0x8))
            {
               z_rcget = _Kccget(&kcb.bthchh);
               if(z_rcget.rc == GET_EMPTY)
               {
                  return( BTEOVER );
               }
               else
               {
                  if((z_rcget.rc != GET_TOP_MID )&&
                     (z_rcget.rc != GET_LAST    ))
                     return( UERROR );
               }

               bteptr2 = (struct BTE *)z_rcget.cheptr;
               bte2.hinl = 0;
               bte2.hinr = 0;
               bte2.stap = 0;
               bte2.endp = kcb.mchacmce -1 ;
               bte2.jteaddr = NULL;
               bte2.kjf1 = 0;
               bte2.kjh1 = bte.kjh1;
               bte.kjh1 *= (-1);
               bte2.kjf2 = NULL;
               bte2.kjh2 = NULL;
               bte2.usage = 0;
               bte2.pen = 0;
               bte2.dmyflg = OFF;
               if ( bte.kjh1 >= 0)
               {
                  bte.fzkflg  = F_FLG_USE1;
                  bte2.fzkflg = NULL;
               }
               else
               {
                  bte.fzkflg  = NULL;
                  bte2.fzkflg = F_FLG_USE1;
               }
               z_rcinsb =_Kccinsb(&kcb.bthchh,bteptr2,bteptr1);
               if ( z_rcinsb != 0 )
                  return( UERROR );
            }
         }

         if ( z_rncutb.delno < 0 )
            return( UERROR );

         break;

      /*----------------------------------------------------------------*
       *        DEFAULT
       *----------------------------------------------------------------*/
      default   :
         return( UERROR );              /* sem.code[] isn't correct     */
   }

   kcb.totcand = CACTV( kcb.bthchh );
                                        /* get number of all candidates */

/*----------------------------------------------------------------------*
 *      GET NUMBER OF OUTPUT CHARACTERS
 *----------------------------------------------------------------------*/
   z_maxsei = 0;                        /* reset max seisyo length     */

   FOR_FWD( kcb.bthchh , bteptr1 , z_endflg )
   {
      LST_FWD( bteptr1 ,z_endflg );

      z_rcbknj = _Kccbknj( kcbptr1, bteptr1, z_buf , (short)SEIMAX );

      if( z_rcbknj.rc != SUCCESS)
      {
         if( z_rcbknj.rc == Z_OVRFLW)
            return( UERROR );
         else
            return( z_rcbknj.rc );
      }

      if( ( z_rcbknj.kjno * 2 ) > z_maxsei )
      {
         z_maxsei = z_rcbknj.kjno * 2;  /* reset maxsei length          */
      }
   }

   kcb.maxsei = z_maxsei;               /* set maxsei length to KCB    */

/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/

   return( SUCCESS );                       /* return to caller            */

}
