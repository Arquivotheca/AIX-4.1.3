static char sccsid[] = "@(#)14	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcjugt1.c, libKJI, bos411, 9428A410j 6/4/91 12:56:21";
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
 * MODULE NAME:       _Kcjugt1
 *
 * DESCRIPTIVE NAME:
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

/************************************************************************
 *      START OF FUNCTION
 ************************************************************************/
struct RETJUGT1 _Kcjugt1(z_kcbptr,z_tbltyp,z_offset,
                   z_morpos,z_morlen,z_yomipt,z_yomiln)

struct KCB *z_kcbptr;
short  z_tbltyp;                        /*  table type 0:jiritsugo      */
                                        /*             1:long word      */
short  z_offset;                        /*  offset from kcb.udeude      */
uschar z_morpos;                        /*                              */
uschar z_morlen;                        /*                              */
short  z_yomipt;                        /*  point to yomi-field in buff */
short  z_yomiln;                        /*  point to yomi-field in buff */
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern struct RETJUKNJ _Kcjuknj();   /* Set Seisho Data on JKJ       */
   extern short           _Kcjsjrt();   /* Set Data on JTE              */
   extern short           _Kcjulng();   /* Set Long Word Data on JLE    */

/*----------------------------------------------------------------------*
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcjkj.h"   /* Jiritsugo KanJi hyoki table entry (JKJ)      */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   struct RETJUGT1 z_ret ;      /* Define Area for Return of Own        */
   struct RETJUKNJ z_rjuknj;    /* Define Area for Return of _Kcjuknj   */
   short           z_rjsjrt;    /* Define Area for Return of _Kcjsjrt   */
   short           z_rjulng;    /* Define Area for Return of _Kcjulng   */
   short           z_off2;      /* Pointer of KANJI                     */
   struct JKJ     *z_jkjptr;    /* JKJ pointer                          */
   uschar          z_hinpos;    /* hinshi position                      */
   uschar          z_flag[2];   /* dflag                                */
   uschar          z_type;      /* type                                 */

/*----------------------------------------------------------------------*
 *      START OF PROCESS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* establish address'ty to kcb  */

/*----------------------------------------------------------------------*
 *      INITIALIZE RETURN CODE AS DEFAULT
 *----------------------------------------------------------------------*/
   z_ret.rc = SUCCESS;                  /* set return code              */
   z_hinpos  = 29;                      /*  hinsi                       */
   z_flag[0] = 0x05;                    /*  dflag                       */
   z_flag[1] = 0x00;                    /*  dflag                       */
   z_type    = TP_IPPAN;                /*  type                        */

/*----------------------------------------------------------------------*
 *      GET KANJI
 *----------------------------------------------------------------------*/
   z_off2  = z_offset + 1;              /* advance by 1 to point seisho */
   z_rjuknj = _Kcjuknj(z_kcbptr,z_off2);
                                        /* call _Kcjuknj                */
   if(z_rjuknj.rc != SUCCESS)
   {
      z_ret.rc = z_rjuknj.rc;
      return(z_ret);
   }

   z_jkjptr = z_rjuknj.jkjptr;

/*----------------------------------------------------------------------*
 *      SET DATA ON JTE OR JLE
 *----------------------------------------------------------------------*/
   if(z_tbltyp == JTB)                  /* jiritsugo                    */
   {
      z_rjsjrt = _Kcjsjrt(z_kcbptr,     /* call _Kcjsjrt                */
                        z_hinpos,       /*  hinsi                       */
                        z_flag,         /*  dflag                       */
                        z_morpos,       /*  mora position               */
                        z_morlen,       /*  mora length                 */
                        z_type,         /*  type                        */
                        z_jkjptr);      /*  kjptr                       */
      if(z_rjsjrt != SUCCESS)
      {
         z_ret.rc = z_rjsjrt;
         return(z_ret);
      }
   }
   else
   {
      z_rjulng = _Kcjulng(z_kcbptr,     /* call _Kcjslng                */
                       z_hinpos,        /* hinsi                        */
                       z_flag,          /* dflag                        */
                       z_morpos,        /* mora position                */
                       z_type,          /* type                         */
                       z_jkjptr,        /* kanji pointer                */
                       z_yomipt,        /* yomi position in buff        */
                       z_yomiln);       /* yomi length                  */

      if(z_rjulng != SUCCESS)
      {
         z_ret.rc = z_rjulng;
         return(z_ret);
      }
   }

   z_ret.newoff = z_rjuknj.newoff;      /* return new offset of buffer  */
   return(z_ret);
}
