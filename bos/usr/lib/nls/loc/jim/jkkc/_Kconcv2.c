static char sccsid[] = "@(#)68	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kconcv2.c, libKJI, bos411, 9428A410j 6/4/91 15:18:48";
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
 * MODULE NAME:       _Kconcv2
 *
 * DESCRIPTIVE NAME:  SET CANDIDATE ON OUTPUT BUFFER FOR NEXT CONVERSION
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS): success
 *                    0x0002 (NOMORE_CAND): No more candidate
 *                    0x0004 (NO_CAND): No candidate
 *                    0x2104 (SEIOVER): seisho buffer overflow
 *                    0x2204 (SEMOVER): seisho map buffer overflow
 *                    0x2304 (YMMOVER): yomi map buffer overflow
 *                    0x2404 (GRMOVER): grammar map buffer overflow
 *                    0x0108 (WSPOVER): work space overflows
 *                    0x7fff (UERROR) :  unpredictable error
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
short _Kconcv2(z_kcbptr,z_func)

struct KCB    *z_kcbptr;                /* pointer of KCB               */
short         z_func;                   /* function for other candidate */
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern struct RETNMTCH _Kcnmtch();   /* Select BTE Having Same Seisho*/
   extern void            _Kcrinit();   /* Clear Output Buffers         */
   extern short           _Kcrsout();   /* Set BTE data on Output Buffer*/

/*----------------------------------------------------------------------*
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcjte.h"   /* Jiritsugo Table Entry (JTE)                  */
#include   "_Kcbte.h"   /* Bunsetsu Table Entry (BTE)                   */
#include   "_Kcsei.h"   /* SEIsho buffer entry (SEI)                    */
#include   "_Kcsem.h"   /* SEisho Map entry (SEM)                       */
#include   "_Kcgrm.h"   /* GRammer Map entry (GRM)                      */
#include   "_Kcymm.h"   /* YoMi Map entry (YMM)                         */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define   Z_EXIST       0x00ff
#define   Z_NOT_EXIST   0x01ff

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short           z_ret   ;    /* Define Area for Return of Own        */
   struct RETNMTCH z_rnmtch;    /* Define Area for Return of _Kcnmtch   */
   short           z_rrsout;    /* Define Area for Return of _Kcrsout   */

   short           z_length;    /* length of seisho                   */

/*----------------------------------------------------------------------*
 *      START OF PROCESS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set base pointer of KCB      */

/*----------------------------------------------------------------------*
 *       GET NEXT BTE
 *----------------------------------------------------------------------*/
   z_ret = (short)SUCCESS;              /* set return code normal       */

   switch ( z_func )
   {
      /*---------------------   NEXT CONVERSION   ----------------------*/
      case NEXTCNV:
         seiptr1 = kcb.seiaddr;         /* set base pointer of SEI      */
                                        /* get length of seisho         */
         z_length = CHPTTOSH( sei.ll ) - 2;
                                        /* get next BTE                 */
         z_rnmtch = _Kcnmtch(z_kcbptr,&sei.kj[0][0],z_length);
                                        /* compare with input seisho    */
         if ( z_rnmtch.rc != Z_EXIST )
         {
            if ( z_rnmtch.rc == Z_NOT_EXIST)
                return( UERROR );
            else
                return( z_rnmtch.rc );
         }

         bteptr1 = z_rnmtch.bteptr;
         CMOVF(kcb.bthchh , bteptr1 );  /* get next seisho              */

         if ( bte.top == ON )           /* if no more candidate         */
            z_ret = NOMORE_CAND;

         break;

      /*--------------------   PREVIOUS CONVERSION   --------------------*/
      case PREVCNV:
         seiptr1 = kcb.seiaddr;         /* set base pointer of SEI      */
                                        /* get length of seisho         */
         z_length = CHPTTOSH( sei.ll ) - 2;
                                        /* get next BTE                 */
         z_rnmtch = _Kcnmtch(z_kcbptr,&sei.kj[0][0],z_length);
                                        /* compare with input seisho    */
         if ( z_rnmtch.rc != Z_EXIST )
         {
            if ( z_rnmtch.rc == Z_NOT_EXIST)
                return( UERROR );
            else
                return( z_rnmtch.rc );
         }

         bteptr1 = z_rnmtch.bteptr;
         CMOVB(kcb.bthchh , bteptr1);    /* get next candidate           */

         if ( bte.last == ON )           /* if no more candidate         */
            z_ret = NOMORE_CAND;

         break;

      /*-----------------   RETURN TO FIRST CANDIDATE   -----------------*/
      case FRSTCNV:
         CMOVT( kcb.bthchh , bteptr1 ); /* get the top of candidates    */

         if (bteptr1 != NULL )
            z_ret = SUCCESS;
         else
            z_ret = NO_CAND;

         break;

      /*-----------------   RETURN TO LAST CANDIDATE   ------------------*/
      case LASTCNV:
         CMOVL( kcb.bthchh , bteptr1 ); /* get the last of candidates    */

         if (bteptr1 != NULL )
            z_ret = SUCCESS;
         else
            z_ret = NO_CAND;

         break;
   }

/*----------------------------------------------------------------------*
 *      SET RESULT OF CONVERSION
 *----------------------------------------------------------------------*/
   seiptr1 = kcb.seiaddr;
   semptr1 = kcb.semaddr;
   grmptr1 = kcb.grmaddr;
   ymmptr1 = kcb.ymmaddr;

   kcb.seill = 0;
   kcb.semll = 0;
   kcb.grmll = 0;
   kcb.ymmll = 0;

   _Kcrinit(z_kcbptr);

   z_rrsout = _Kcrsout(z_kcbptr,bteptr1, kcb.seiaddr, kcb.semaddr,
                                   kcb.grmaddr, kcb.ymmaddr);

   if ( z_rrsout != SUCCESS )
      return( UERROR );

   kcb.seill = (short)CHPTTOSH(sei.ll);
   kcb.semll = (short)CHPTTOSH(sem.ll);
   kcb.grmll = (short)grm.ll;
   kcb.ymmll = (short)ymm.ll;

/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/
   return(z_ret);
}
