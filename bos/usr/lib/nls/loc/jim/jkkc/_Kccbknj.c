static char sccsid[] = "@(#)26	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kccbknj.c, libKJI, bos411, 9428A410j 6/4/91 10:16:32";
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
 * MODULE NAME:       _Kccbknj
 *
 * DESCRIPTIVE NAME:  SET BUNSETSU HYOKI ON SPECIFIED OUTPUT BUFFER
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS)    : Success
 *                    0x02ff(Z_OVRFLW)   : Output Buffer Overflow
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
struct RETCBKNJ _Kccbknj(z_kcbptr,z_bteptr,z_outptr,z_outlen)

struct  KCB   *z_kcbptr;
struct  BTE   *z_bteptr;
uschar  *z_outptr;
short   z_outlen;
{
/*----------------------------------------------------------------------*  
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/ 
   extern struct RETCJKNJ _Kccjknj();   /* Get Jiritsugo Seisho         */
   extern struct RETCFKNJ _Kccfknj();   /* Get FUZOKUGO Seisho          */
   extern short           _Kcc72hr();   /* Exchange YOMI to DBCS Hira.  */

/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcbte.h"   /* Bunsetsu Table Entry (BTE)                   */
#include   "_Kcjte.h"   /* Jiritsugo Table Entry (JTE)                  */
#include   "_Kcjkj.h"   /* Jiritsugo KanJi hyoki table entry (JKJ)      */
#include   "_Kcfkj.h"   /* Fuzokugo KanJi hyoki table (FKJ)             */
#include   "_Kcfkx.h"   /* Fuzokugo Kj hyoki eXchange table entry (FKX) */
#include   "_Kcmce.h"   /* Mora Code table Entry (MCE)                  */
#include   "_Kcyce.h"   /* Yomi Code table Entry (YCE)                  */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/ 
#define Z_OVRFLW  (0x0200 | LOCAL_RC)   /* output buffer over flow      */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   struct RETCBKNJ z_ret;       /* Define Area for Return of _Kccbknj   */

   struct RETCJKNJ z_rcjknj;    /* Define Area for Return of _Kccjknj   */
   struct RETCFKNJ z_rcfknj;    /* Define Area for Return of _Kccfknj   */
   short           z_morpos;
   short           z_kjno;
   unsigned short  z_wkshr;
   short           z_i;

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
   kcbptr1 = z_kcbptr;
   bteptr1 = z_bteptr;
   jteptr1 = bte.jteaddr;

   z_ret.rc = SUCCESS;                 /* initialize return code       */
   z_ret.kjno = 0;                     /* initialize return code       */
/*----------------------------------------------------------------------*  
 *      PROCESS UNTIL LAST MORA
 *----------------------------------------------------------------------*/ 
   for( z_morpos = bte.stap; z_morpos <= bte.endp; )
   {
   /*-------------------------------------------------------------------*  
    *      IF JIRITSU-GO EXIST
    *-------------------------------------------------------------------*/ 
      if( (bte.jteaddr != NULL) && (jte.stap == z_morpos)
                                && ( jte.jkjaddr != NULL ) )
      {
      /*----------------------------------------------------------------*  
       *      GET JIRITSU-GO HYOKI
       *----------------------------------------------------------------*/ 
         z_rcjknj = _Kccjknj(kcbptr1,jteptr1,z_outptr,z_outlen);
         if ( z_rcjknj.rc == Z_OVRFLW )
         {
            z_ret.rc = Z_OVRFLW;
            return(z_ret);
         }
         else if ( z_rcjknj.rc != SUCCESS )
         {
            z_ret.rc = UERROR;
            return(z_ret);
         }

         if ( z_rcjknj.kjno == 0 )
         {
            bte.jteaddr = NULL;
         }
         else
         {
            z_outptr += z_rcjknj.kjno * 2;
            z_outlen += z_rcjknj.kjno * 2;

            z_ret.kjno = z_rcjknj.kjno; /* save kanji no to hv been proc*/
                                        /* advance mora position        */
            z_morpos = z_morpos + z_rcjknj.mrno;
         }
      }
      else
      /*----------------------------------------------------------------*  
       *      IF FUZOKU-GO EXIST
       *----------------------------------------------------------------*/ 
      {
         if( ( bte.kjf1 == z_morpos ) 
                    && ( bte.fzkflg & F_FLG_USE1)
                    && ( bte.dmyflg == OFF ) )
         {
            /*----------------------------------------------------------*
             *      GET 1ST  FUZOKU-GO HYOKI
             *----------------------------------------------------------*/
            z_rcfknj = _Kccfknj(kcbptr1,bte.kjh1,z_morpos,z_outptr,z_outlen);
            if ( z_rcfknj.rc == Z_OVRFLW )
            {
                  z_ret.rc = Z_OVRFLW;
                  return(z_ret);
            }
            else if ( z_rcfknj.rc != SUCCESS )
            {
                  z_ret.rc = UERROR;
                  return(z_ret);
            }

            z_outptr += z_rcfknj.kjno * 2;
            z_outlen += z_rcfknj.kjno * 2;

            z_ret.kjno = z_ret.kjno + z_rcfknj.kjno;
                                        /* _Kccfknj returns no of kj    */
                                        /* increase no of kanji         */
            z_morpos = z_morpos + z_rcfknj.mrno;
                                        /* advance mora position        */
         }

         else if ( ( bte.kjf2 == z_morpos )
                         && ( bte.fzkflg & F_FLG_USE2 )
                         && ( bte.dmyflg == OFF ) )
         {
            /*----------------------------------------------------------*
             *      GET 2ST  FUZOKU-GO HYOKI
             *----------------------------------------------------------*/
            z_rcfknj = _Kccfknj(kcbptr1,bte.kjh2,z_morpos,z_outptr,z_outlen);
            if ( z_rcfknj.rc == Z_OVRFLW )
            {
                  z_ret.rc = Z_OVRFLW;
                  return(z_ret);
            }
            else if ( z_rcfknj.rc != SUCCESS )
            {
                  z_ret.rc = UERROR;
                  return(z_ret);
            }

            z_outptr += z_rcfknj.kjno * 2;
            z_outlen += z_rcfknj.kjno * 2;

            z_ret.kjno = z_ret.kjno + z_rcfknj.kjno;
                                        /* _Kccfknj returns no of kj    */
                                        /* increase no of kanji         */
            z_morpos = z_morpos + z_rcfknj.mrno;
                                        /* advance mora position        */
         }
         else
         {                              /* convert 7 bit yomi to dbcs   */
            if ( z_morpos != 0 )
            {
               mceptr1 = kcb.mchmce + z_morpos - 1;
               yceptr1 = mce.yceaddr + 1;
            }
            else
               yceptr1 = kcb.ychyce;

            mceptr1 = kcb.mchmce + z_morpos;

            for ( z_i = 0; yceptr1 <= mce.yceaddr; yceptr1++, z_i += 2)
            {
               z_wkshr = _Kcc72hr( yce.yomi );
               SHTOCHPT(z_outptr+z_i , z_wkshr);/* move char on output  */
               z_ret.kjno++;
            }
            z_outptr += z_i;
            z_outlen += z_i;
            z_morpos++;
         }
      }
   }

   return(z_ret);
}
