static char sccsid[] = "@(#)97	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcjmdct.c, libKJI, bos411, 9428A410j 6/4/91 12:54:05";
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
 * MODULE NAME:       _Kcjmdct
 *
 * DESCRIPTIVE NAME:  LOOKING UP A WORD IN MRU
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS): success
 *                    0x1104 (JTEOVER): JTE table overflow
 *                    0x1204 (JKJOVER): JKJ table overflow
 *                    0x1304 (JLEOVER): JLE table overflow
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
short _Kcjmdct(z_kcbptr,z_strpos,z_length)
struct KCB    *z_kcbptr;                /* pointer of KCB               */
unsigned char z_strpos;                 /* offset of 1st MCE            */
unsigned char z_length;                 /* length of string(mora code)  */
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern struct RETXCMPY _Kcxcmpy();   /* Compare Mora With 7 bit yomi */
   extern short           _Kccinsb();   /* Insert Before     Table Entry*/
   extern short           _Kcxcmpk();   /* Compare Seisho With JKJ      */
   extern void            _Kccfree();   /* Free Any Kinds of Table Entry*/

/*----------------------------------------------------------------------*
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcmce.h"   /* Mora Code table Entry (MCE)                  */
#include   "_Kcyce.h"   /* Yomi Code table Entry (YCE)                  */
#include   "_Kcjte.h"   /* Jiritsugo Table Entry (JTE)                  */
#include   "_Kcjkj.h"   /* Jiritsugo KanJi hyoki table entry (JKJ)      */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define    Z_FREQ6     0x06             /* frequency 6                  */
#define    Z_FLG_MASK  0xf8             /* dflag mask                   */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   struct RETXCMPY z_rxcmpy;    /* Define Area for Return of _Kcxcmpy   */
   short           z_rxcmpk;    /* Define Area for Return of _Kcxcmpk   */
   short           z_rcinsb;    /* Define Area for Return of _Kccinsb   */

   struct  JTE     *z_jteptr;   /* temp JTE pointer                     */
   short           z_endflgj;   /* end flag for JTE                     */
   short           z_endflgj2;  /* end flag for JTE                     */
   short           z_endflgk;   /* end flag for JKJ                     */

   uschar          *z_strptr;   /* pointer of mru                       */
   short           z_mrulen;    /* length of mru                        */
   short           z_ylen;      /* length of data yomi in MRU           */
   short           z_slen;      /* length of data seisho in MRU         */

/*----------------------------------------------------------------------*
 *      START OF PROCESS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;

/*----------------------------------------------------------------------*
 *      GET LENGTH OF MRU BUFFER
 *----------------------------------------------------------------------*/
   z_strptr = kcb.mdemde + 2;           /* set start ptr of entry       */
   z_mrulen = (short)GMRULEN(kcb.mdemde);
                                        /* Low Byte High byte replaced  */
                                        /* Search JTE which has same YOMI*/
   FOR_FWD(kcb.jthchh , jteptr1 ,z_endflgj)
   {
      LST_FWD(jteptr1,z_endflgj);

      if((jte.stap == z_strpos)&&
         (jte.len > 0))
      {
            break;
      }
   }
   if (z_endflgj == ON)
      return( SUCCESS );

/*----------------------------------------------------------------------*
 *      LOOK UP WORDS FROM MRU BUFFER
 *----------------------------------------------------------------------*/
   while ( z_strptr < ( kcb.mdemde + z_mrulen ) )
   {
      /*----------------------   initialisation   ----------------------*/
      z_ylen = (short)*z_strptr;        /* set length of yomi in MRU    */
      z_slen = *(z_strptr + z_ylen);    /* set length of seisho in MRU  */


      if ( z_ylen == 0 )
         break;

      /*---------   compare 7bit yomi code with yomi in MRU   ----------*/
      z_rxcmpy = _Kcxcmpy(z_kcbptr,z_strptr+(uschar)1,z_ylen-(short)1,
                                                z_strpos,z_length);

      if ( z_rxcmpy.res == EQ_SHORT )
      {
                                        /* Search JTE which has same KJ */
         jteptr2 = jteptr1;
         FOR_FWD_MID(kcb.jthchh , jteptr2 ,z_endflgj)
         {
            LST_FWD(jteptr2,z_endflgj);

            if((jte2.stap != z_strpos)||
               (jte2.len  != z_rxcmpy.len))
               continue;

            z_rxcmpk = _Kcxcmpk(z_kcbptr,(uschar *)(z_strptr+z_ylen+1),
                                      z_slen-(short)1,jteptr2,
                                      z_strpos,z_length);
            if ( z_rxcmpk == EQUAL )    /* if KJs are equal             */
            {
               jte2.dflag[0] &= Z_FLG_MASK;
               jte2.dflag[0] |= Z_FREQ6 ;
/*jte2.dflag[0] = ((jte.dflag[0] & FREQ)|(jte2.dflag[0] & Z_FLG_MASK));*/

               z_jteptr = jteptr2;
               if(jteptr1 != jteptr2 )
                  CMOVB(kcb.jthchh,jteptr2);

               z_rcinsb=_Kccinsb(&kcb.jthchh,z_jteptr,jteptr1);
               if (z_rcinsb != SUCCESS)
                  return( z_rcinsb );

               CMOVF(kcb.jthchh,z_jteptr);
               jteptr1 = z_jteptr;
            }
         }
      }
      z_strptr += ( z_ylen + z_slen );  /* point the next target word   */
   }

/*----------------------------------------------------------------------*
 *      JTE REDUCTION ROUTINE
 *----------------------------------------------------------------------*/
   /*-------------------------------------------------------------*
    *      IF IT IS SAME AS JTE CREATED AT LAST TIME
    *-------------------------------------------------------------*/
   if(kcb.env == ENVNONE)
   {
      FOR_FWD(kcb.jthchh,jteptr1 ,z_endflgj)
      {
         LST_FWD(jteptr1,z_endflgj);

         if((jte.stap == z_strpos)&&(z_endflgj!=ON))
         {
            jteptr2 = jteptr1;
            CMOVF(kcb.jthchh,jteptr2);
            FOR_FWD_MID(kcb.jthchh,jteptr2 ,z_endflgj2)
            {
               LST_FWD(jteptr2,z_endflgj2);
               if((jte.hinpos == jte2.hinpos)&&
                                           /* jrt-go hin. pos table tag    */
                  (jte.stap == jte2.stap)&&/* jrt-go start mora pointer    */
                  (jte.len  == jte2.len )&&/* jrt-go mora length           */
                  (jte.dtype == jte2.dtype)&&/*jrt-go type                 */
                  ((jte.dflag[0]&0xF8)==(jte2.dflag[0]&0xF8))&&
                                           /* jrt-go flag                  */
                  (jte.dflag[1] == jte2.dflag[1]))/* jrt-go flag           */
               {
                  if ( jte2.jkjaddr != 0 )
                  {
                     jkjptr1 = jte2.jkjaddr;

                     FOR_FWD_MID(kcb.jkhchh,jkjptr1,z_endflgk)
                     {
                        if ((jkj.kj[0] & 0x80) == 0x00 )
                           z_endflgk = ON;
                                        /* set flag on if end of hyouki */
                        jkjptr2  = jkjptr1;
                                        /* get last jiritu hyouki ptr   */
                        CMOVB(kcb.jkhchh,jkjptr1);
                                        /* free jiritu-hyoki pointer    */
                        _Kccfree(&kcb.jkhchh,jkjptr2);

                     }
                  }
                  z_jteptr = jteptr2;
                  CMOVB(kcb.jthchh,jteptr2);
                  _Kccfree(&kcb.jthchh,z_jteptr);
               }
            }
         }
      }
   }
/*----------------------------------------------------------------------*
 *      RETURN NORMAL
 *----------------------------------------------------------------------*/
   return( SUCCESS );
}
