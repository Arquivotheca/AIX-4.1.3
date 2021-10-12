static char sccsid[] = "@(#)02	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcxcmpk.c, libKJI, bos411, 9428A410j 6/4/91 15:28:16";
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
 * MODULE NAME:       _Kcxcmpk
 *
 * DESCRIPTIVE NAME:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:          0 (EQUAL):   equal
 *                      -1 (NOT_EQU): not equal
 *                    7fff (UERROR):  unpredictable error
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
short _Kcxcmpk(z_kcbptr,z_seiptr,z_seilen,z_jteptr,z_strpos,z_length)

struct KCB      *z_kcbptr;              /* pointer of KCB               */
unsigned char   *z_seiptr;              /* offset of seisho             */
                                        /*   from kcb.mdemde            */
short           z_seilen;               /* length of seisho             */
struct JTE      *z_jteptr;              /* pointer of JTE               */
unsigned char   z_strpos;
unsigned char   z_length;
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern short             _Kcc72hr();
   extern short             _Kcc72kn();

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcjte.h"
#include   "_Kcjkj.h"
#include   "_Kcmce.h"
#include   "_Kcyce.h"

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short             z_i;               /* counter                      */
   uschar            z_endflg;
   uschar            z_enc[2];          /* decode area                  */
   uschar            z_kjbuf[2];        /* decode area                  */
   unsigned short    z_wk;

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set base pointer of KCB      */
   jteptr1 = z_jteptr;                  /* set base pointer of JTE      */
   jkjptr1 = jte.jkjaddr;               /* set base pointer of JKJ      */

/*----------------------------------------------------------------------*
 *      COMPARE HIRAGANA OR KATAKANA HYOUKI
 *----------------------------------------------------------------------*/
   if ( jkj.kj[0] == 0x7f )
   {
   /*-------------------------------------------------------------------*
    *   SET THE FIRST & LAST POINTER OF YCE ( yceptr1 & yceptr2 )
    *-------------------------------------------------------------------*/
      if ( z_strpos == 0 )
         yceptr1 = kcb.ychyce;
      else
      {
         mceptr1 = kcb.mchmce + z_strpos - 1;
         yceptr1 = mce.yceaddr + 1;
      }

      mceptr1 = kcb.mchmce + z_strpos - 1 + jte.len;
      yceptr2 = mce.yceaddr;

   /*-------------------------------------------------------------------*
    *   COMPARE LENGTH OF SEISHO
    *-------------------------------------------------------------------*/
      if ( z_seilen != ( ( yceptr2 - yceptr1 + 1 ) * 2 ) )
         return( (short)NOT_EQU );

      for ( z_i = 0; yceptr1 <= yceptr2; yceptr1++, z_i += 2 )
      {
         z_enc[0] = ( ( (z_seiptr[z_i] & 0x20) << 1 )
                             | z_seiptr[z_i] | 0x80 );

         z_enc[1] = z_seiptr[z_i+1];

         z_wk = z_enc[0] * 256 + z_enc[1];

         /*-------------------------------------------------------------*
          *   COMPARE KATAKANA-HYOUKI
          *-------------------------------------------------------------*/
         if ( jkj.kj[1] == 0xff )
         {
            if ( z_wk != (unsigned short)_Kcc72kn(yce.yomi) )
               return( (short)NOT_EQU );
         }
         /*-------------------------------------------------------------*
          *   COMPARE HIRAGANA-HYOUKI
          *-------------------------------------------------------------*/
         else
         {
            if ( z_wk != (unsigned short)_Kcc72hr(yce.yomi) )
               return( (short)NOT_EQU );
         }
      }

      return( (short)EQUAL );
   }

/*----------------------------------------------------------------------*
 *      COMPARE KANJI-HYOUKI
 *----------------------------------------------------------------------*/
   else
   {
      for ( z_i = 0; z_i < z_seilen;
                          z_i += 2, CMOVF(kcb.jkhchh , jkjptr1))
      {
         /*-------------------   encode seisho   -----------------------*/
         if ( z_i != z_seilen - 2 )     /* if not end of seisho         */
            z_enc[0] = ( ( (z_seiptr[z_i] & 0x20) << 1 ) 
                                | z_seiptr[z_i] | 0x80 );

         else                           /* if end of seisho             */
            z_enc[0] = ( ( (z_seiptr[z_i] & 0x20) << 1 ) 
                                | z_seiptr[z_i] & 0x7f );

         z_enc[1] = z_seiptr[z_i+1];

         /*------   compare encoded seisho with kanji in JKJ   ---------*/
         if ( ( z_enc[0] != jkj.kj[0] )   /* if high byte is not equal  */
           || ( z_enc[1] != jkj.kj[1] ) ) /* if low byte is not equal   */
            return( (short)NOT_EQU );
                                          /* if both end                */
         if ( ( (jkj.kj[0] & 0x80) == 0 )  && ( (z_enc[0] & 0x80) == 0 ) )
            return( (short)EQUAL );
                                          /* if one of them end         */
         else if ( ( (jkj.kj[0] & 0x80) == 0 )  || ( (z_enc[0] & 0x80) == 0 ) )
            return( (short)NOT_EQU );
      }
   }
   return( (short)UERROR );
}
