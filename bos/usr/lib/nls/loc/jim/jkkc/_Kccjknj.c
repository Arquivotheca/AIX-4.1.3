static char sccsid[] = "@(#)36	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kccjknj.c, libKJI, bos411, 9428A410j 6/4/91 10:19:17";
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
 * MODULE NAME:       _Kccjknj
 *
 * DESCRIPTIVE NAME:  SET JIRITSUGO HYOKI ON SPECIFIED OUTPUT BUFFER
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
struct RETCJKNJ _Kccjknj(z_kcbptr,z_jteptr,z_outptr,z_outlen)

struct  KCB   *z_kcbptr;
struct  JTE   *z_jteptr;
uschar  *z_outptr;
short   z_outlen;
{
/*----------------------------------------------------------------------*  
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/ 
   extern short           _Kcc72kn();   /* Exchange YOMI to DBCS Kata.  */
   extern short           _Kcc72hr();   /* Exchange YOMI to DBCS Hira.  */

/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcjte.h"   /* Jiritsugo Table Entry (JTE)                  */
#include   "_Kcjkj.h"   /* Jiritsugo KanJi hyoki table entry (JKJ)      */
#include   "_Kcmce.h"   /* Mora Code table Entry (MCE)                  */
#include   "_Kcyce.h"   /* Yomi Code table Entry (YCE)                  */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/ 
#define Z_OVRFLW  (0x0200 | LOCAL_RC)   /* output buffer over flow      */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   struct RETCJKNJ z_ret   ;    /* Define Area for Return of _Kccjknj   */
   short           z_kjno;      /* Define Area for Kanji Number         */
   short           z_mrno;      /* Define Area for Mora  Number         */
   struct YCE     *z_i1;        /* Define YCE pointer                   */
   unsigned short  z_wkshr;

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
   kcbptr1 = z_kcbptr;                  /* establish address'ty to KCB  */
   jteptr1 = z_jteptr;                  /* establish address'ty to JTE  */
                                        /*                              */
   z_ret.rc = SUCCESS;                  /* initialize return code       */
   z_kjno = 0;                          /* initialize no of kj          */
   z_mrno = 0;                          /* initialize no of mora        */

/*----------------------------------------------------------------------*  
 *      IF KANJI-HYOKI EXIST
 *----------------------------------------------------------------------*/ 

   if(jte.jkjaddr > 0)                  /* if kanji-hyki exists         */
   {
   /*-------------------------------------------------------------------*  
    *      ESTABLISH ADDRESSABILITY TO JKJ ENTRY
    *-------------------------------------------------------------------*/ 
      z_mrno = jte.len;                 /* initialize no of mora        */
      jkjptr1 = jte.jkjaddr;            /* establish address'ty to JKJ  */

      if(jkj.kj[0] == 0x7f)
      {
         if( jte.stap == 0 )            /* check if 1st mora            */
         {
            mceptr1 = kcb.mchmce;
            yceptr1 = kcb.ychyce;       /* start from begining          */
         }
         else
         {                              /* the next to the end of previ-*/
                                        /* ous yomi position            */
            mceptr1 = kcb.mchmce + jte.stap -1;
            yceptr1 = mce.yceaddr + 1;
         }                              /* save the last postition      */
         mceptr1 = kcb.mchmce + jte.stap + jte.len - 1;
         yceptr2 = mce.yceaddr;

         z_kjno = yceptr2 - yceptr1 + 1;
                                        /* save yomi length             */
         if( z_outlen < ( z_kjno * 2 ) )
         {                              /* may excess output area       */
            z_ret.rc = Z_OVRFLW;        /* overflow indication          */
            return(z_ret);              /* return to caller             */
         }


         /*-------------------------------------------------------------*  
          *      GET DBCS CODE FROM YOMI CODE
          *-------------------------------------------------------------*/ 
         for(z_i1 = yceptr1;z_i1 < yceptr2 + 1; z_i1++,z_outptr+=2)
         {
                                       /* convert yomi(1 byte) to      */
                                       /* dbcs(2 bytes) code           */
            switch(jkj.kj[1])
            {
               case 0xff:              /* yomi to Katakana             */
                  z_wkshr = _Kcc72kn(z_i1->yomi);
                  break;
               case 0xfe:              /* yomi to Hirakana             */
                  z_wkshr = _Kcc72hr(z_i1->yomi);
                  break;
            }
            SHTOCHPT(z_outptr,z_wkshr);/* move char on the output buff.*/
         }
      }
      else
      {
         /*-------------------------------------------------------------*  
          *      GET DBCS CODE FROM JKJ
          *-------------------------------------------------------------*/ 
         for(;jkj.kj[0]&0x80;z_outptr+=2,z_kjno++,
                            CMOVF(kcb.jkhchh,jkjptr1))
                                        /* set JKJ ptr                  */
         {
                                        /* advance one entry            */
            if( z_kjno + 2 > z_outlen ) /* check if output area has     */
            {                           /* enough space                 */
               z_ret.rc = Z_OVRFLW;
               return(z_ret);
            }

            *z_outptr = jkj.kj[0];      /* move 1st byte                */
            *(z_outptr+1) = jkj.kj[1];  /* move 2nd byte                */
         }                              /* end of for                   */

         *z_outptr = jkj.kj[0] | 0x80;  /* move 1st byte & set MSB on   */
         *(z_outptr+1) = jkj.kj[1];     /* move 2nd byte                */
         z_kjno++;
      }
   }
   z_ret.kjno = z_kjno;
   z_ret.mrno = z_mrno;
   return(z_ret);
}
