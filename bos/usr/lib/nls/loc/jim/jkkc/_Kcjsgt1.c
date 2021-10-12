static char sccsid[] = "@(#)02	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcjsgt1.c, libKJI, bos411, 9428A410j 7/23/92 03:14:33";
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
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kcjsgt1
 *
 * DESCRIPTIVE NAME:  CREATE JTE,JLE AND JKJ.
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS): success
 *                    0x1104 (JTEOVER): JTE table overflow
 *                    0x1204 (JKJOVER): JKJ table overflow
 *                    0x1304 (JLEOVER): JLE table overflow
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

#define  Z_KSKIP	0x1000
/************************************************************************
 *      START OF FUNCTION
 ************************************************************************/
struct RETJSGT1 _Kcjsgt1(z_kcbptr,z_tbltyp,z_offset,z_mode,
                   z_morpos,z_morlen,z_yomipt,z_file)
struct KCB *z_kcbptr;
short  z_tbltyp;                        /*  table type 0:jiritsugo      */
short  z_offset;                        /*             1:long word      */
short  z_mode;                          /*  mode to create a table      */
uschar z_morpos;                        /*                              */
uschar z_morlen;                        /*                              */
short  z_yomipt;                        /*  point to yomi-field in buff */
short  z_file;				/* System File Number		*/
{
/*----------------------------------------------------------------------*
 *      EXTERN DESCRIPTRION
 *----------------------------------------------------------------------*/
   extern struct RETJSTAG _Kcjstag();   /* Get Tag and Dflag            */
   extern struct RETJSKNJ _Kcjsknj();   /* Set Seisho Data on JKJ       */
   extern short           _Kcjsjrt();   /* Set Data on JTE              */
   extern short           _Kcjslng();   /* Set Long Word Data on JLE    */
   extern void            _Kccfree();   /* Free Any Kinds of Table Entry*/

/*----------------------------------------------------------------------*
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcjte.h"   /* Jiritsugo Table Entry (JTE)                  */
#include   "_Kcjtx.h"   /* Jiritsugo Tag eXchange table (JTX)           */
#include   "_Kcmce.h"   /* Mora Code table Entry (MCE)                  */
#include   "_Kcfst.t"   /* JTE & FTE Link Possiblity    		*/

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   struct RETJSGT1 z_ret   ;    /* Define Area for Return of Own        */
   struct RETJSTAG z_rjstag;    /* Define Area for Return of _Kcjstag   */
   struct RETJSKNJ z_rjsknj;    /* Define Area for Return of _Kcjsknj   */
   short           z_rjsjrt;    /* Define Area for Return of _Kcjsjrt   */
   short           z_rjslng;    /* Define Area for Return of _Kcjslng   */

/*----------------------------------------------------------------------*
 *      START OF PROCESS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* establish address'ty to kcb  */

/*----------------------------------------------------------------------*
 *      INITIALIZE RETURN CODE AS DEFAULT
 *----------------------------------------------------------------------*/
   z_ret.rc = SUCCESS;

/*----------------------------------------------------------------------*
 *      GET TYPE AND GRAMMER
 *----------------------------------------------------------------------*/
   if(kcb.env != ENVTAN)
   {
      z_rjstag = _Kcjstag(z_kcbptr,z_offset,z_morpos,z_morlen,z_file);
                                        /*  call _Kcjstag to get hinsi  */
                                        /*  and type                    */
                                        /*  z_offset remain unchanged   */
      if ( z_rjstag.rc != SUCCESS )
      {
            z_ret.rc = z_rjstag.rc;
            return( z_ret );
      }
      z_ret.newoff = z_rjstag.newoff;   /* return new offset of buffer  */

/*----------------------------------------------------------------------*
 *      JTE REDUCTION ROUTINE
 *----------------------------------------------------------------------*/
      if(z_mode != SPECIFIC ) {
       if(( kcb.func != FUNNXTCV ) && ( z_morpos == 0 )) {
         mceptr1 = kcb.mchmce + z_morpos + z_morlen;
         jtxptr1 = kcb.jtxjtx + z_rjstag.hinpos;
         fstptr1 = &fsttbl[mce.code];
         /*-------------------------------------------------------------*
          *      IF IT DOES NOT CONNECT NEXT MORA
          *-------------------------------------------------------------*/
         if(((jtx.pos[2] & fst.pos[2])|
             (jtx.pos[1] & fst.pos[1])|
             (jtx.pos[0] & fst.pos[0])) == 0x00)
         {
             return(z_ret);
         }
       }
      }
   }                                    /* endif not single kanji       */
   else
   {
      z_ret.newoff   = z_offset + 2;
      z_rjstag.flag[0] = NULL;
      z_rjstag.flag[1] = NULL;
      z_rjstag.hinpos  = 0x20;
      z_rjstag.type    = NULL;
   }                                    /* endif single kanji           */

/*----------------------------------------------------------------------*
 *      GET KANJI
 *----------------------------------------------------------------------*/
   z_rjsknj = _Kcjsknj(z_kcbptr,z_offset, z_morlen, z_file );
   if ( z_rjsknj.rc != SUCCESS )
   {
      if( z_rjsknj.rc == Z_KSKIP ) 
	  z_rjsknj.rc = SUCCESS;
      z_ret.rc = z_rjsknj.rc;
      return(z_ret);
   }

/*----------------------------------------------------------------------*
 *      SET DATA ON JTE OR JLE
 *----------------------------------------------------------------------*/
   /*-------------------------------------------------------------------*
    *     REGISTRATE JIRITSU-GO TABLE
    *-------------------------------------------------------------------*/
   if(z_tbltyp == JTB)                  	/* jiritsugo            */
   {
      z_rjsjrt = _Kcjsjrt(z_kcbptr,     	/* call _Kcjsjrt        */
                          z_rjstag.hinpos,	/*  hinsi               */
                          z_rjstag.flag,	/*  dflag               */
                          z_morpos,     	/*  mora position       */
                          z_morlen,     	/*  mora length         */
                          z_rjstag.type,	/*  type                */
                          z_rjsknj.jkjptr);	/*  kjptr               */

      ext_sys_conv = ON;			/* Set Conv Flag	*/
      if ( z_rjsjrt != SUCCESS )
      {
         z_ret.rc = z_rjsjrt;
         return(z_ret);
      }
   }
   /*-------------------------------------------------------------------*
    *     REGISTRATE LONG-WORD TABLE
    *-------------------------------------------------------------------*/
   else
   {
      z_rjslng = _Kcjslng(z_kcbptr,     /* call _Kcjslng                */
                      z_rjstag.hinpos,  /*  hinsi                       */
                      z_rjstag.flag,    /*  dflag                       */
                      z_morpos,         /*  mora position               */
                      z_rjstag.type,    /*  type                        */
                      z_rjsknj.jkjptr,  /*  kjptr                       */
                      z_yomipt);        /* yomi position in buff        */

      if ( z_rjslng != SUCCESS )
      {
         z_ret.rc = z_rjslng;
         return(z_ret);
      }
   }
   return(z_ret);
}
