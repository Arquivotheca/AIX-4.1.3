static char sccsid[] = "@(#)27	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kccfknj.c, libKJI, bos411, 9428A410j 6/4/91 10:16:55";
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
 * MODULE NAME:       _Kccfknj
 *
 * DESCRIPTIVE NAME:  SET FUZOKUGO HYOKI ON SPECIFIED OUTPUT BUFFER
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS)    : Success
 *                    0x02ff(Z_OVRFLW)   : Output Buffer Overflow
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
struct RETCFKNJ _Kccfknj(z_kcbptr,z_fkjpos,z_morpos,z_outptr,z_outlen)

struct  KCB   *z_kcbptr;
short         z_fkjpos;
short         z_morpos;
uschar       *z_outptr;
short         z_outlen;
{
/*----------------------------------------------------------------------*  
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/ 
   extern short           _Kcc72hr();   /* Exchange YOMI to DBCS Hira.  */

/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcjte.h"   /* Jiritsugo Table Entry (JTE)                  */
#include   "_Kcfkj.h"   /* Fuzokugo KanJi hyoki table (FKJ)             */
#include   "_Kcmce.h"   /* Mora Code table Entry (MCE)                  */
#include   "_Kcyce.h"   /* Yomi Code table Entry (YCE)                  */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/ 
#define Z_OVRFLW  (0x0200 | LOCAL_RC)

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   struct RETCFKNJ z_ret   ;    /* Define Area for Return of _Kccfknj   */

   short           z_kjno;
   short           z_mrno;
   short           z_i1;
   short           z_ycesta;
   short           z_yceend;
   short           z_ymino;
   unsigned short  z_wkshr;

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
/*----------------------------------------------------------------------*  
 *      IF KANJI-HYOKI NOT EXIST
 *----------------------------------------------------------------------*/ 
   if ( z_fkjpos == NULL )
   {
      z_ret.rc = SUCCESS;
      z_ret.kjno = 0;
      z_ret.mrno = 0;
      z_ret.homoni = 0;
      return( z_ret );
   }

   kcbptr1 = z_kcbptr;                  /* establish address'ty to KCB  */

   if ( z_fkjpos >= 0 )
      fkjptr1 = kcb.fkjfkj + z_fkjpos;
   else
      fkjptr1 = kcb.fkjfkj - z_fkjpos;

   z_ret.rc = SUCCESS;                  /* initialize return code       */
   z_ret.homoni = 1;                    /* homonim exists               */
/*----------------------------------------------------------------------*  
 *      IF KANJI-HYOKI NOT EXIST
 *----------------------------------------------------------------------*/ 
   z_mrno = (fkj.hdr.hdr2 & 0xf0 ) >> 4;/* set no of mora               */
   z_kjno = fkj.hdr.hdr2 & 0x0f;        /* no of kanji                  */

   z_ret.mrno = z_mrno;                 /* set no of mora on return code*/

   /*-------------------------------------------------------------------*
    *   KANJI HYOUKI
    *-------------------------------------------------------------------*/
   if( z_fkjpos > 0 )
   {
      if( z_kjno * 2 > z_outlen)
      {
         z_ret.rc = Z_OVRFLW;
         return(z_ret);
      }
      fkjptr1 ++ ;                      /* advance FKJ pointer to next  */

      for( z_i1 = 0 ; z_i1 < z_kjno ; z_i1 ++ )
      {                                 /* move kanji hyoki to output   */
                                        /* area                         */
         *z_outptr = fkj.elm.kj[0];
                                        /* move 1st byte                */
         *(z_outptr + 1) = fkj.elm.kj[1];
                                        /* move 2nd byte                */
         z_outptr = z_outptr + 2 ;      /* increase buff ptr            */
         fkjptr1 ++ ;                   /* advance FKE pointer to next  */
      }

      z_ret.kjno = z_kjno;
   }

   /*-------------------------------------------------------------------*
    *   HIRAGANA HYOUKI
    *-------------------------------------------------------------------*/
   else
   {
      if( z_morpos == 0 )               /* check if 1st mora            */
      {
         z_ycesta = 0;                  /* start from begining          */
      }
      else
      {                                 /* the next to the end of previ-*/
                                        /* ous yomi position            */
         mceptr1 = kcb.mchmce + z_morpos - 1;
         z_ycesta = (short)( mce.yceaddr - kcb.ychyce + 1 );
      }                                   /* save the last postition      */

      mceptr1 = kcb.mchmce + z_morpos + z_mrno - 1;
      z_yceend = (short)( mce.yceaddr - kcb.ychyce );
      z_ymino = z_yceend - z_ycesta + 1;
                                        /* save yomi length             */

      if( z_outlen < ( z_ymino * 2 ) )
      {                                 /* may excess output area       */
         z_ret.rc = Z_OVRFLW;           /* overflow indication          */
         return(z_ret);                 /* return to caller             */
      }

      yceptr1 = kcb.ychyce + z_ycesta;
      yceptr2 = kcb.ychyce + z_yceend;

      for ( ; yceptr1 <= yceptr2; yceptr1++ )
      {                                 /* convert yomi(1 byte) to      */
                                        /* dbcs(2 bytes) code           */
         z_wkshr = _Kcc72hr( yce.yomi );
         SHTOCHPT(z_outptr , z_wkshr);  /* move char on the output buff.*/

         z_outptr  = z_outptr + 2 ;     /* advance ptr                  */
      }

      z_ret.kjno = z_ymino;
   }

   return(z_ret);
}
