static char sccsid[] = "@(#)83	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcrsfzk.c, libKJI, bos411, 9428A410j 6/4/91 15:22:05";
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
 * MODULE NAME:       _Kcrsfzk
 *
 * DESCRIPTIVE NAME:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS): success
 *                    0x2104 (SEIOVER): seisho buffer overflow
 *                    0x2204 (SEMOVER): seisho map buffer overflow
 *                    0x2304 (YMMOVER): yomi map buffer overflow
 *                    0x2404 (GRMOVER): grammar map buffer overflow
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
short _Kcrsfzk(z_kcbptr,z_bteptr,z_seiptr,z_semptr,z_grmptr,z_ymmptr)

struct  KCB   *z_kcbptr;
struct  BTE   *z_bteptr;
struct  SEI   *z_seiptr;
struct  SEM   *z_semptr;
struct  GRM   *z_grmptr;
struct  YMM   *z_ymmptr;
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern short           _Kcc72hr();
   extern struct RETCFKNJ _Kccfknj();


/*----------------------------------------------------------------------*
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcbte.h"
#include   "_Kcsei.h"
#include   "_Kcsem.h"
#include   "_Kcgrm.h"
#include   "_Kcymm.h"
#include   "_Kcyce.h"
#include   "_Kcmce.h"
#include   "_Kcjte.h"
#include   "_Kcfkj.h"
#include   "_Kcgpw.h"

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define Z_OVRFLW   0x02ff

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
struct RETCFKNJ z_ret1;
char            *z_outptr;
short           z_outlen;
short           z_wkattr;
unsigned short  z_wkshr;
short           z_morpos;
short           z_jmorps;
short           z_seill;
short           z_semll;
short           z_i1;
short           z_ymno;
short           z_ymll;
short           z_kjpos;

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* establish address'ty to KCB  */
   bteptr1 = z_bteptr;                  /* establish address'ty to BTE  */
   gpwptr1 = kcb.gpwgpe;                /* establish address'ty to GPW  */
   jteptr1 = bte.jteaddr;

/*----------------------------------------------------------------------*
 *      IF FUZOKU-GO DOESN'T EXIST IN THE BTE
 *----------------------------------------------------------------------*/
   if ( ( bte.dmyflg == ON ) ||
        ( (jteptr1 != NULL) && (( jte.stap + jte.len ) > bte.endp) ) )
      return( SUCCESS );

/*----------------------------------------------------------------------*
 *      GET START-POSITION OF FUZOKU-GO IN MCE TABLE
 *                                      & SET SEISHO MAP ( 0x13, 0x15 )
 *----------------------------------------------------------------------*/
 /*----------   IF JIRITSU-GO EXISTS IN THE BTE   ----------------------*/
   if ( ( bte.jteaddr != NULL )
                && ( ( jte.len > 0 )
                    || ( kcb.env == ENVNEXT ) || ( kcb.env == ENVZEN ) ) )
   {                                    /* skip jiritsugo               */
      z_wkattr = FZK;                   /*  0x13                        */
      z_morpos = jte.stap + jte.len;    /* points next after jiritsugo  */
      z_jmorps = z_morpos;              /* save start pointd            */
   }
 /*----------   IF JIRITSU-GO DOESN'T EXIST IN THE BTE   ---------------*/
   else                                 /* fuzukugo only                */
   {
      z_wkattr = ALLFZK;                /*  0x15                        */
      z_morpos = bte.stap;
      z_jmorps = -1;                    /* initialize jiritsugo wk      */
   }

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS OF OUTPUT BUFFERS
 *----------------------------------------------------------------------*/
   seiptr1 = z_seiptr;
   semptr1 = z_semptr;
   grmptr1 = z_grmptr;
   ymmptr1 = z_ymmptr;

/*----------------------------------------------------------------------*
 *      GET LENGTH OF YOMI MAP, SEISHO AND SEISHO MAP IN EACH BUFFER
 *----------------------------------------------------------------------*/
   z_seill = (sei.ll[0] * 256 ) + sei.ll[1];
   z_semll = (sem.ll[0] * 256 ) + sem.ll[1];

   if ( z_morpos == 0 )
   {
      z_ymll = gpw.accfirm;
   }
   else
   {
      mceptr1 = kcb.mchmce + z_morpos - 1;
      z_ymll = mce.yceaddr - kcb.ychyce + gpw.accfirm + 1;
   }

   z_outptr = (char *)z_seiptr + z_seill;
   z_outlen
        = kcb.seimaxll - ( (char *)z_seiptr - (char *)kcb.seiaddr ) - 2;

   while ( z_morpos <= bte.endp )
   {
   /*----------------------------------------------------------------------*
    *   FUZOKU-GO IS WRITTEN BY KANJI
    *----------------------------------------------------------------------*/
      if ( ( (bte.kjf1 == z_morpos) && (bte.fzkflg != F_FLG_NOEXT) )
        || ( (bte.kjf2 == z_morpos)
                && (bte.fzkflg <= (F_FLG_TWO | F_FLG_USE2 | F_FLG_USE1))
                && (bte.fzkflg >= F_FLG_TWO) ) )
      {
      /*----------------------------------------------------------------*
       *        SET KANJI HYOKI IN OUTPUT BUFFER
       *----------------------------------------------------------------*/
         if ( bte.kjf1 == z_morpos )
            z_ret1 = _Kccfknj(kcbptr1,bte.kjh1,z_morpos,z_outptr,z_outlen);
         else
            z_ret1 = _Kccfknj(kcbptr1,bte.kjh2,z_morpos,z_outptr,z_outlen);

      /*----------------------------------------------------------------*
       *        CHECK LENGTH OF REMAINED OUTPUT BUFFER
       *----------------------------------------------------------------*/
         /*-------------------  check seisho buffer  -------------------*/
         if ( z_ret1.rc != SUCCESS )
         {
            if ( z_ret1.rc == Z_OVRFLW)
               return( SEIOVER );
            else
               return( UERROR );
         }

         /*-----------------  check seisho map buffer  -----------------*/
         if ( ( z_semll + z_ret1.kjno * 2 ) > kcb.semmaxll  )
            return( SEMOVER );

         /*-----------------  check grammer map buffer  ----------------*/
         if ( grm.ll + 1 > kcb.grmmaxll )
             return( GRMOVER );

         /*------------------  check yomi map buffer  ------------------*/
         mceptr1 = kcb.mchmce + z_morpos + z_ret1.mrno-1;

         if ( z_morpos == 0 )
         {
            z_ymno = mce.yceaddr - kcb.ychyce + 1;
         }
         else
         {
            mceptr2 = kcb.mchmce + z_morpos - 1;
            z_ymno = mce.yceaddr - mce2.yceaddr;
         }

         if( ymm.ll + ( z_ymno / 8 ) > kcb.ymmmaxll )
            return( YMMOVER );

         z_outptr += ( z_ret1.kjno * 2 );
         z_outlen -= ( z_ret1.kjno * 2 );

         z_seill += ( z_ret1.kjno * 2 );
         sei.ll[0] = z_seill / 256;
         sei.ll[1] = z_seill % 256;

      /*----------------------------------------------------------------*
       *        SET SEISHO MAP IN OUTPUT BUFFER
       *----------------------------------------------------------------*/
                                        /* set 1st attribute            */
         sem.code[z_semll - 2] = z_wkattr + z_ret1.homoni;
                                        /* set continuation mark such as*/
                                        /* att.01.01...01.att           */
                                        /* until nxt kj                 */
         for(z_i1 = 1; z_i1 < z_ret1.kjno ; z_i1 ++ )
            sem.code[z_semll -2 + z_i1] = 0x01;
                                        /* add no of kanji to LL        */
                                        /* advance mora position        */

          z_semll += z_ret1.kjno ;
          sem.ll[0] = z_semll / 256;
          sem.ll[1] = z_semll % 256;

      /*----------------------------------------------------------------*
       *        SET GRAMMAR MAP IN OUTPUT BUFFER
       *----------------------------------------------------------------*/
         if ( bte.kjf1 == z_morpos )
            z_kjpos = bte.kjh1;
         else
            z_kjpos = bte.kjh2;

         if ( z_kjpos < 0 )
            z_kjpos *= (-1);

         if ( z_kjpos >=128 )
         {
            grm.byte[grm.ll - 1] = ( z_kjpos / 256 ) | 0x80;
            grm.byte[grm.ll] = z_kjpos % 256;
            grm.ll += 2;
         }
         else
         {
            grm.byte[grm.ll - 1] = z_kjpos;
            grm.ll++;
         }

      /*----------------------------------------------------------------*
       *        SET YOMI MAP IN OUTPUT BUFFER
       *----------------------------------------------------------------*/
         ymm.byte[z_ymll/8] |= ( 0x80 >> ( z_ymll % 8 ) );

         z_ymll += z_ymno;
         ymm.ll = ( z_ymll - 1 ) / 8 + 2;

      /*----------------------------------------------------------------*
       *        PROCEED MORA POSITION
       *----------------------------------------------------------------*/
         z_morpos += z_ret1.mrno;
      }

   /*-------------------------------------------------------------------*
    *   FUZOKU-GO IS WRITTEN BY HIRAGANA
    *-------------------------------------------------------------------*/
      else
      {
      /*----------------------------------------------------------------*
       *        GET NUMBER OF YOMI
       *----------------------------------------------------------------*/
         if ( z_morpos == 0 )
            yceptr1 = kcb.ychyce;
         else
         {
            mceptr2 = kcb.mchmce + z_morpos - 1;
            yceptr1 = mce2.yceaddr + 1;
         }

         mceptr1 = kcb.mchmce + z_morpos;

         z_ymno = mce.yceaddr - yceptr1 + 1;

      /*----------------------------------------------------------------*
       *        CHECK LENGTH OF REMAINED OUTPUT BUFFER
       *----------------------------------------------------------------*/
         /*-------------------  check seisho buffer  -------------------*/
         if ( z_seill + ( z_ymno * 2 ) > kcb.seimaxll )
               return( SEIOVER );

         /*-----------------  check seisho map buffer  -----------------*/
         if ( z_semll + ( z_ymno * 2 ) > kcb.semmaxll  )
            return( SEMOVER );

         if ( ( ( z_jmorps == z_morpos ) && ( z_wkattr == FZK ) )
           || ( ( z_wkattr == ALLFZK ) && ( z_morpos == bte.stap ) ) )
         {
            /*---------------  check grammer map buffer  ---------------*/
            if ( grm.ll + 1 > kcb.grmmaxll )
               return( GRMOVER );

            /*----------------  check yomi map buffer  -----------------*/
            if( ymm.ll + ( z_ymno / 8 ) > kcb.ymmmaxll )
               return( YMMOVER );
         }

      /*----------------------------------------------------------------*
       *        SET HIRAGANA HYOKI IN OUTPUT BUFFER
       *----------------------------------------------------------------*/
         for ( ; yceptr1 <= mce.yceaddr; yceptr1++ )
         {
            z_wkshr = _Kcc72hr( yce.yomi );

            *z_outptr = z_wkshr / 256;
            *(z_outptr + 1) = z_wkshr % 256;
            z_outptr += 2;
            z_outlen -= 2;
         }

         z_seill += ( z_ymno * 2 );
         sei.ll[0] = z_seill / 256;
         sei.ll[1] = z_seill % 256;

      /*----------------------------------------------------------------*
       *        SET SEISHO MAP IN OUTPUT BUFFER
       *----------------------------------------------------------------*/
         if ( ( ( z_jmorps == z_morpos ) && ( z_wkattr == FZK ) )
           || ( ( z_wkattr == ALLFZK ) && ( z_morpos == bte.stap ) ) )
            sem.code[z_semll - 2] = z_wkattr;
         else
            sem.code[z_semll -2] = 0x01;

         for(z_i1 = 1; z_i1 < z_ymno ; z_i1 ++ )
            sem.code[z_semll -2 + z_i1] = 0x01;

          z_semll += z_ymno;
          sem.ll[0] = z_semll / 256;
          sem.ll[1] = z_semll % 256;

      /*----------------------------------------------------------------*
       *        SET GRAMMAR MAP IN OUTPUT BUFFER
       *----------------------------------------------------------------*/
         if ( (( z_wkattr == ALLFZK ) && ( z_morpos == bte.stap ))||
             (( z_wkattr == FZK) && ( jte.stap == 0 ) &&
              (jte.len == 0) && ( z_morpos == bte.stap )))
         {
            grm.byte[grm.ll - 1] = 0x00;
            grm.ll++;
         }

      /*----------------------------------------------------------------*
       *        SET YOMI MAP IN OUTPUT BUFFER
       *----------------------------------------------------------------*/
         if ( ( ( z_jmorps == z_morpos ) && ( z_wkattr == FZK ) )
           || ( ( z_wkattr == ALLFZK ) && ( z_morpos == bte.stap ) ) )
            ymm.byte[z_ymll/8] |= ( 0x80 >> ( z_ymll  % 8 ) );

         z_ymll += z_ymno;
         ymm.ll = ( z_ymll - 1 ) / 8 + 2;

      /*----------------------------------------------------------------*
       *        PROCEED MORA POSITION
       *----------------------------------------------------------------*/
         z_morpos++;
      }
   }    /* end of while */

   return( SUCCESS );
}
