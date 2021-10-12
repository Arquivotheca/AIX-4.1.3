static char sccsid[] = "@(#)81	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcrproc.c, libKJI, bos411, 9428A410j 6/4/91 15:21:37";
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
 * MODULE NAME:       _Kcrproc
 *
 * DESCRIPTIVE NAME:  DECISION AND SET    RESULT
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS):    success
 *                    0x0004 (NO_CAND):    no candidate exist
 *
 *                    0x2104 (SEIOVER):    seisho buffer overflow
 *                    0x2204 (SEMOVER):    seisho map buffer overflow
 *                    0x2304 (YMMOVER):    ymi map buffer overflow
 *                    0x2404 (YMMOVER):    grammar map buffer overflow
 *
 *                        -1 (Z_NOT_FIRM): not firm up
 *                    0x7fff (UERROR):     unpredictable error
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
short   _Kcrproc(z_kcbptr, z_dctctl, z_rmode)

struct KCB      *z_kcbptr;              /* get address of KCB           */
short  z_dctctl;                        /* dict. looking up cntrl info. */
short  z_rmode;                         /* kakutai mode                 */
{ 
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern struct RETCFPTH _Kccfpth();   /* get least penalry path       */
   extern short           _Kcrchek();   /* result decision check routine*/
   extern void            _Kcbrbst();   /* creating right jishu for dumy*/
   extern void            _Kcrinit();   /* intialize all output buffer  */
   extern short           _Kcrsout();   /* set result output buffer     */
   extern short           _Kcrrstb();   /* after care of comfirm        */
   extern short           _Kcjproc();   /* jiritsu-go process           */

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcsei.h" 
#include   "_Kcsem.h" 
#include   "_Kcgrm.h" 
#include   "_Kcymm.h" 
#include   "_Kcbte.h" 
#include   "_Kcpte.h" 
#include   "_Kcmce.h" 
#include   "_Kcyce.h" 
#include   "_Kcgpw.h" 
 
/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define   Z_NOT_FIRM       -1

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short               z_i;             /* declare loop counter         */
   short               z_bmlen;         /* declare create bunsetsu len  */
   short               z_mode;          /* declare mode                 */
   short               z_pthend;        /* declare path end             */

   struct   RETCFPTH   z_rcfpth;        /* get least penalry path       */
   short               z_rrsout;        /* set result output buffer     */
   short               z_rrrstb;        /* after care of confirming     */
   short               z_rrchek;        /* return of rchek              */

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set base address of KCB      */ 
   gpwptr1 = kcb.gpwgpe;                /* set base address of GPW      */ 
   seiptr1=kcb.seiaddr;                /* seisho output area pointer    */
   semptr1=kcb.semaddr;                /* seisho map output area pointer*/
   grmptr1=kcb.grmaddr;                /*grammer map output area pointer*/
   ymmptr1=kcb.ymmaddr;                /* yomi map output area pointer  */

/*----------------------------------------------------------------------*
 *      FIRM UP
 *----------------------------------------------------------------------*/
   if( z_rmode  ==  ABS )               /* if do firm up                */
   {
      /*---------------   PROCESS OF RIGHT CHARACTER   -----------------*/
      if(z_dctctl == kcb.mchacmce-1)
      {
         _Kcbrbst(z_kcbptr);            /* add penalty of right char.   */

         z_pthend = kcb.mchacmce-1 ;    /* set end point of path moralen*/
         z_mode = 0;                    /* set mode                     */
      }
      /*----------------------------------------------------------------*/
      else
      {
         z_rrchek = _Kcrchek(z_kcbptr,z_dctctl,z_rmode);

         if ( z_rrchek == Z_NOT_FIRM )
            return( Z_NOT_FIRM );
         else
            z_pthend = z_rrchek;        /* set end point of path moralen*/

         if (gpw.kakuflg == ON)
           z_mode = 1;
         else
           z_mode = 2;       
      }
   }

/*----------------------------------------------------------------------*
 *      NOT FIRM UP
 *----------------------------------------------------------------------*/
   else
   {
      z_rrchek = _Kcrchek(z_kcbptr,z_dctctl,z_rmode);

      if ( z_rrchek == Z_NOT_FIRM )
         return( Z_NOT_FIRM );
      else
         z_pthend = z_rrchek;           /* set end point of path moralen*/

      z_mode = 1;                       /* set mode                     */
   }

/*----------------------------------------------------------------------*
 *      GET THE PTE WHICH HAS LEAST PENALTY
 *----------------------------------------------------------------------*/
   z_rcfpth = _Kccfpth(z_kcbptr,z_pthend,z_mode);

   if (z_rcfpth.rc != FPTH_FOUND)
   {
      if ( z_rcfpth.rc == FPTH_NOT_FND )
      {
         if ( ( z_rmode  == ABS )
                     && ( z_dctctl != kcb.mchacmce - 1 ) )
            return( Z_NOT_FIRM );
         else
            return( NO_CAND );
      }
      else
         return( UERROR );
   }

/*----------------------------------------------------------------------*
 *      SET RESULT TO OUTPUT BUFFER
 *----------------------------------------------------------------------*/
   pteptr1 = z_rcfpth.pteptr;           /* set PTE pointer              */

   if ( ( z_rmode == ABS ) && ( z_dctctl == kcb.mchacmce - 1 ) )
   {
      /*---------   FIRM UP ALL BUNSETSU OF CONFIRMED PATH   -----------*/
      for( z_i = 0; ( pte.bteaddr[z_i] != NULL ) && ( z_i < 6 ); z_i++)
      {
         bteptr1 = pte.bteaddr[z_i];

         if(bte.dmyflg != ON)
         {
            z_rrsout =                  /* create bunsetsu result       */
               _Kcrsout(z_kcbptr,       /* post KCB pointer             */
                        bteptr1,        /* post BTE pointer             */
                        kcb.seiaddr,    /* post SEI pointer             */
                        kcb.semaddr,    /* post SEM pointer             */
                        kcb.grmaddr,    /* post GRM pointer             */
                        kcb.ymmaddr);   /* post YMM pointer             */

            if (z_rrsout != SUCCESS)    /* error handling               */
               return( z_rrsout );
         }
                                        /* set kakutei yomi length      */
         kcb.seill = sei.ll[0]*256 + sei.ll[1];
         kcb.semll = sem.ll[0]*256 + sem.ll[1];
         kcb.ymmll = ymm.ll;
         kcb.grmll = grm.ll;

         mceptr1 = kcb.mchmce + kcb.mchacmce-1;
         kcb.ymill2 =
             (short)(mce.yceaddr - kcb.ychyce) + gpw.accfirm + 1;
         gpw.pkakutei = PKAKALL;
      }
   }

   else
   {
      /*---------   FIRM UP ALL BUNSETSU OF CONFIRMED PATH   -----------*/
      bteptr1 = pte.bteaddr[0];

      if(bte.dmyflg != ON)
      {
         z_rrsout =                     /* create bunsetsu result       */
            _Kcrsout(z_kcbptr,          /* post KCB pointer             */
                     bteptr1,           /* post BTE pointer             */
                     kcb.seiaddr,       /* post SEI pointer             */
                     kcb.semaddr,       /* post SEM pointer             */
                     kcb.grmaddr,       /* post GRM pointer             */
                     kcb.ymmaddr);      /* post YMM pointer             */

         if (z_rrsout != SUCCESS)
            return( z_rrsout );

         kcb.seill = sei.ll[0]*256 + sei.ll[1];
         kcb.semll = sem.ll[0]*256 + sem.ll[1];
         kcb.ymmll = ymm.ll;
         kcb.grmll = grm.ll;
         mceptr1 = kcb.mchmce + bte.endp;
         if ((kcb.cnvx == OFF)&&(kcb.cnvi == OFF))
         {                     /* case of on the fly           */
            kcb.ymill2 =
              (short)(mce.yceaddr - kcb.ychyce) + gpw.accfirm + 1;
            gpw.pkakutei = PKAKPRT;
         }
         else                  /* case of tsukuji input        */
            gpw.pkakutei = PKAKNON;
      }
      else
      {
         gpw.pkakutei = PKAKNON;
         gpw.leftflg  = OFF;
      }

      /*---------   AFTER CARE OF CONFIRMING   -------------------------*/
      bteptr1 = pte.bteaddr[0];         /* set confirm bunsetsu address */

      _Kcrrstb(z_kcbptr,bteptr1);
   }                                    /* end else of not confirm      */

/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/
   return( SUCCESS );
}                                       /* end of program               */
