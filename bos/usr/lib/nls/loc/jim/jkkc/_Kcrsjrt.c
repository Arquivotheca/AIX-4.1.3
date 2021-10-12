static char sccsid[] = "@(#)84	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcrsjrt.c, libKJI, bos411, 9428A410j 6/4/91 15:22:16";
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
 * MODULE NAME:       _Kcrsjrt
 *
 * DESCRIPTIVE NAME:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS):   success
 *                    0x2104 (SEIOVER):   seisho buffer overflow
 *                    0x2204 (SEMOVER):   seisho map buffer overflow
 *                    0x2304 (YMMOVER):   yomi map buffer overflow
 *                    0x2404 (GRMOVER):   grammar map buffer overflow
 *                    0x7fff (UERROR):    unpredictale error
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
short _Kcrsjrt(z_kcbptr,z_bteptr,z_seiptr,z_semptr,z_grmptr,z_ymmptr)

struct  KCB   *z_kcbptr;
struct  BTE   *z_bteptr;
struct  SEI   *z_seiptr;
struct  SEM   *z_semptr;
struct  GRM   *z_grmptr;
struct  YMM   *z_ymmptr;
{
/*----------------------------------------------------------------------* 
 *       EXTERNAL DEFINITION
 *----------------------------------------------------------------------*/
   extern   short             _Kcrgeta();
   extern   struct RETCJKNJ   _Kccjknj();

/*----------------------------------------------------------------------* 
 *       INCLUDE FILES
 *----------------------------------------------------------------------*/
#include       "_Kcchh.h"
#include       "_Kckcb.h"
#include       "_Kcbte.h"
#include       "_Kcsei.h"
#include       "_Kcsem.h"
#include       "_Kcgrm.h"
#include       "_Kcymm.h"
#include       "_Kcjte.h"
#include       "_Kcyce.h"
#include       "_Kcmce.h"
#include       "_Kcgpw.h"

/*----------------------------------------------------------------------* 
 *       DEFINITION OF CONSTANTS
 *----------------------------------------------------------------------*/
#define   Z_OVRFLW   0x02ff

/*----------------------------------------------------------------------* 
 *       DEFINITION OF VARIABLES
 *----------------------------------------------------------------------*/
   short z_ret;
   struct RETCJKNJ z_ret1;
   char  *z_outptr;
   short z_outlen;
   short z_ymmll;
   short z_ymmymi;
   short z_i1;
   short z_seill;
   short z_semll;

/*----------------------------------------------------------------------* 
 *       SET BASE POINTERS
 *----------------------------------------------------------------------*/
    kcbptr1 = z_kcbptr;                 /* establish address'ty to KCB  */
    bteptr1 = z_bteptr;                 /* establish address'ty to BTE  */
    seiptr1 = z_seiptr;                 /* establish address'ty to SEI  */
    semptr1 = z_semptr;                 /* establish address'ty to SEM  */
    grmptr1 = z_grmptr;                 /* establish address'ty to GRM  */
    ymmptr1 = z_ymmptr;                 /* establish address'ty to YMM  */
    gpwptr1 = kcb.gpwgpe;               /* establish address'ty to GPW  */
                     
    jteptr1 = bte.jteaddr;              /* establish address'ty to JTE  */

/*----------------------------------------------------------------------* 
 *       NO JIRITSU-GO EXIST
 *----------------------------------------------------------------------*/
   if ( ( bte.dmyflg == ON )            /* dummy Bunsetsu               */ 
     || ( bte.jteaddr == NULL )         /* no jiritsu-go exist          */
     || ( jte.len == 0 ) )              /* dummy jiritsu-go             *
                                         *   for right and left char    */
   {              
      return( SUCCESS );        
   }

/*----------------------------------------------------------------------*
 *      CALCULATE ADDRESS OF OUTPUT
 *----------------------------------------------------------------------*/
   z_seill  = (sei.ll[0] * 256 ) + sei.ll[1];
   z_outptr = (char *)seiptr1 + z_seill;
                                        /* count up pointer             */
                                        /* seiptr points top            */
                                        /* sei.ll show offset from top  */
                                        /* to free area                 */
   z_outlen = kcb.seimaxll - z_seill;
                                        /* set length of output         */
                                        /* max - current                */

/*----------------------------------------------------------------------*
 *      SET KANJI HYOUKI OF JIRITSU-GO TO SEISHO BUFFER
 *----------------------------------------------------------------------*/
   z_ret1 = _Kccjknj(kcbptr1,bte.jteaddr,z_outptr,z_outlen);

   if (z_ret1.rc != SUCCESS )
   {
      if (z_ret1.rc == Z_OVRFLW )
         return( SEIOVER );
      else
         return( UERROR );
   }

   z_seill = z_seill + ( z_ret1.kjno * 2 );
                                        /* _Kccjnknj returns no of kj   */
                                        /* add no of kanji to LL        */
   sei.ll[0] = z_seill / 256;
   sei.ll[1] = z_seill % 256;

/*----------------------------------------------------------------------*
 *      RETURN NORMALY IF ENVIRONMENT IS SINGLE KNJI CONVERSION
 *----------------------------------------------------------------------*/
   if ( kcb.env == ENVTAN )
      return( SUCCESS );

/*----------------------------------------------------------------------*
 *      SET SEISHO MAP OF JIRITSU-GO TO SEISHO MAP BUFFER
 *----------------------------------------------------------------------*/
   /*-----------  check the size of seisho map   -----------------------*/
   z_semll  = sem.ll[0] * 256 + sem.ll[1];

   if ( (z_semll + z_ret1.kjno) > kcb.semmaxll )
      return( SEMOVER );

   /*-----------  determine seisho attribute code ----------------------*/
   if ( kcb.env == ENVALPH )            /* alphabetic/alphanumeric conv */
      sem.code[ z_semll -2 ] = ALPHANUM;
   else if ( kcb.env == ENVALKN )       /* alpha/alphanum & kansuji cnv */
      sem.code[ z_semll -2 ] = ALPHAKAN;
   else if ( kcb.env == ENVKANSU )      /* kansuji cnv                  */
      sem.code[ z_semll -2 ] = KANSUUJI;
   else
      sem.code[ (z_semll - 2) ] = _Kcrgeta( jte.dtype );
                                        /* set 1st attribute            */
                                        /* set continuation mark such as*/
                                        /* att.01.01...01.att           */
                                        /* until nxt kj                 */
   for( z_i1 = 0; z_i1 < ( z_ret1.kjno - 1 ) ; z_i1 ++ )
      sem.code[ (z_semll -1 + z_i1) ] = CONTINUE;

   z_semll = z_semll +  z_ret1.kjno ;
                                        /* add no of kanji to LL        */
   sem.ll[0] = z_semll / 256;
   sem.ll[1] = z_semll % 256;

/*----------------------------------------------------------------------*
 *      SET GRAMMAR MAP OF JIRITSU-GO TO GRAMMAR MAP BUFFER
 *----------------------------------------------------------------------*/
   /*-----------  check the size of grammar map   ----------------------*/
   if ( ((char *)grmptr1 + 2) > ((char *)kcb.grmaddr + kcb.grmmaxll) )
      return( GRMOVER );

   /*-----------  set grammar code in grammar buffer   -----------------*/
   if ( kcb.env == ENVALPH )            /* alphabetic/alphanumeric conv */
      grm.byte[ grm.ll - 1 ] = GR_ALPHA;
   else if ( kcb.env == ENVALKN )       /* alpha/alphanum & kansuji cnv */
      grm.byte[ grm.ll - 1 ] = GR_ALPHAKAN;
   else
      grm.byte[ grm.ll - 1 ] = bte.hinl;

   grm.ll ++;                           /* increase by one              */

/*----------------------------------------------------------------------*
 *      SET YOMI MAP OF JIRITSU-GO TO YOMI MAP BUFFER
 *----------------------------------------------------------------------*/
   /*-------------  check the size of yomi map   -----------------------*/
   mceptr1 = kcb.mchmce + jte.stap-1;   /* set start mora address       */
   mceptr2 = kcb.mchmce + jte.stap + jte.len-1;
                                        /* set end   mora address       */
   if (jte.stap != 0)
      z_ymmymi = (short)(mce.yceaddr - kcb.ychyce) +  gpw.accfirm + 1;
   else
      z_ymmymi =  gpw.accfirm;
                                        /* set start mora number        */
   z_ymmll  = (short)((mce2.yceaddr - kcb.ychyce + gpw.accfirm)/8 + 1);
                                        /* set yomi map length          */
   if( ((uschar *)ymmptr1 + z_ymmll + 1) >
       ((uschar *)kcb.ymmaddr + kcb.ymmmaxll ) )
      return( YMMOVER );

   /*--------------  set yomi code in grammar buffer   -----------------*/
   ymm.byte[ ( z_ymmymi / 8 ) ] |= ( 0x80 >> (z_ymmymi % 8));
   ymm.ll = z_ymmll+1;
                                        /* if length of yomi is less    */
                                        /* than 8,1 byte is enough.     */
                                        /* z_ymmymi / 8 is 0.           */
                                        /* ex.if z_ymmymi is 11(dec),   */
                                        /*  bit pattern(0x0030)         */
                                        /*  12345678 12345678           */
                                        /*  00000000 00100000           */
   return( SUCCESS );
}
