static char sccsid[] = "@(#)41	1.3.1.1 src/bos/usr/lib/nls/loc/jim/jkkc/_Kclmrua.c, libKJI, bos411, 9428A410j 7/23/92 03:16:14";
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
 * MODULE NAME:       _Kclmrua
 *
 * DESCRIPTIVE NAME:  insert of MRU area
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       VOID
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------*
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#if defined(_AGC) || defined(_GSW)
#include   "_Kcfnm.h"                   /* Define Names file            */
#endif
#include   "_Kcmap.h"                   /* Define Constant file         */

/************************************************************************
 *      START OF FUNCTION
 ************************************************************************/
void _Kclmrua(udpnt, kanadt, kanaln, kanjidt, kanjiln)

uschar  *udpnt   ;                      /* user dictionary buffer ptr.  */
uschar  *kanadt  ;                      /* kana data                    */
uschar   kanaln  ;                      /* kana length                  */
uschar  *kanjidt ;                      /* kanji data                   */
uschar   kanjiln ;                      /* kanji length                 */

{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern void            _Kclmvch();   /* Move Character in a Dimemsion*/

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   int      step_no          ;          /* step number                  */
   short    mru_len          ;          /* MRU length                   */
   char    *mru_dat          ;          /* MRU data area                */
   int      pnt              ;          /* copy position                */
   int      total_len        ;          /* total length                 */
   int      l                ;          /* loop counter                 */
   uschar  *dummy1           ;          /* dummy buffer                 */
   uschar   kanjiwk[U_WORK]  ;          /* kanji data work buffer       */
   usint    ptr1, ptr2, ptr3 ;          /* MRU area check pointer       */

/*----------------------------------------------------------------------*
 *      START OF PROCESS
 *----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*
 *      If error ocured , break out
 *----------------------------------------------------------------------*/
   for( step_no = NULL ; step_no <= U_STEPNO ; step_no++)
   {
      switch(step_no)                   /* select process by step number*/
      {
         case U_STEP0 :                 /* MRU data read                */
                                        /* calculate entry total length */
            total_len = kanaln + kanjiln + U_KNL + U_KJL ;
            mru_len = GMRULEN( udpnt ) ;/* get MRU length               */

            if(mru_len == NULL)         /* if initial MRU               */
            {
                   mru_len =  U_MLEN ;  /* plus MRL itself              */
            }
            break ;

         case U_STEP1 :                 /* MRU area check               */
                                        /* allocate MRU data area       */
            mru_dat = malloc(U_MRUMAX); /* MRU data copy                */
            memcpy(mru_dat, udpnt, mru_len) ;
            if((total_len + mru_len) > U_MRUMAX)
            {
               for(ptr1 = sizeof(short);/* check loop                   */
                   ptr1 < (U_MRUMAX - total_len);)
               {
                  ptr3 = ptr1 ;         /* copy before point            */
                  ptr1 += *(mru_dat + ptr1) ;   /* get KNL              */
                  ptr1 += *(mru_dat + ptr1) ;   /* get KNL + KJL        */
               }
               mru_len = ptr3 ;         /* delete the oldest KNL        */
            }
            break ;

         case U_STEP2 :                 /* kanji data convert           */
                                        /* insert data copy             */
            memcpy(kanjiwk, kanjidt, (int)kanjiln) ;

            for(l = NULL; l < (kanjiln - sizeof(short));
                l += U_SHORT)           /* kanji data convert           */
            {
               *(kanjiwk + l) &= U_CONV ;/* top bits off                */
            }
            break ;

         case U_STEP3 :                 /* entry insert                 */
            _Kclmvch(mru_dat,           /* area move                    */
                   (int)U_TPOSI,
                   (int)(mru_len - sizeof(short)),
                   (int)U_BACKWD,
                   total_len,
                   (int)FALSE,
                   dummy1,
                   NULL,
                   NULL ) ;
            mru_len += total_len ;      /* plus MRU length              */
            pnt = U_FTOP ;              /* set top position             */

            SMRULEN(mru_dat, mru_len) ; /* set MRU length               */
            pnt += U_MLEN ;             /* position up                  */
                                        /* set kana length              */
            *(mru_dat + pnt) = kanaln + sizeof(uschar) ;
            pnt++ ;                     /* position  1 up               */
                                        /* copy kana data               */
            memcpy((mru_dat + pnt), kanadt, (int)kanaln) ;
            pnt += kanaln ;             /* position up                  */
                                        /* set kanji length             */
            *(mru_dat + pnt) = kanjiln + sizeof(uschar) ;
            pnt++ ;                     /* position 1 up                */
                                        /* copy kanji data              */
            memcpy((mru_dat + pnt), kanjiwk, (int)kanjiln) ;
            break ;

         case U_STEP4 :                 /* MRU data write               */
                                        /* MRU data copy                */
            memcpy(udpnt, mru_dat, (int)mru_len) ;
            free(mru_dat) ;             /* free allocated area          */
            break ;
      }

   }
 return ;                               /* return                       */
}
