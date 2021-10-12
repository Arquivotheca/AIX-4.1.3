static char sccsid[] = "@(#)61	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcobrop.c, libKJI, bos411, 9428A410j 6/4/91 15:17:31";
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
 * MODULE NAME:       _Kcobrop
 *
 * DESCRIPTIVE NAME:  SELECT ALL CANDIDET ONLY ORDINARY CODE
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS) : success
 *                    0x0002 (END_CAND): End of candidate
 *                    0x0004 (NO_CAND) : No candidate
 *                    0x1104 (JTEOVER) : JTE overflow
 *                    0x1204 (JKJOVER) : JKJ overflow
 *                    0x1304 (JLEOVER) : JLE overflow
 *                    0x1404 (FTEOVER) : FTE overflow
 *                    0x1704 (BTEOVER) : BTE overflow
 *                    0x1804 (PTEOVER) : PTE overflow
 *                    0x0108 (WSPOVER) : work space overflow
 *                    0x7fff (UERROR)  : unpredictable error
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
short _Kcobrop( z_kcbptr )
struct KCB      *z_kcbptr;              /* pointer of MCB              */
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern short           _Kcobro1();   /* Making All Cand. Environment */
   extern short           _Kcobro2();   /* Set Candidates on Seisho Buf.*/
   extern short           _Kccinsb();   /* Insert Before     Table Entry*/
   extern struct RETNMTCH _Kcnmtch();   /* Select BTE Having Same Seisho*/

/*----------------------------------------------------------------------*
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcsei.h"   /* SEIsho buffer entry (SEI)                    */
#include   "_Kcgpw.h"   /* General Purpose Workarea (GPW)               */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/ 
#define Z_EXIST     (0x0000 | LOCAL_RC) /* exist in candidates          */
#define Z_NOT_EXIST (0x0100 | LOCAL_RC) /* not exist in candidates      */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short           z_robro1;    /* Define Area for Return of _Kcobro1   */
   short           z_robro2;    /* Define Area for Return of _Kcobro2   */
   short           z_rcinsb;    /* Define Area for Return of _Kccinsb   */
   struct RETNMTCH z_rnmtch;    /* Define Area for Return of _Kcnmtch   */

   short             z_len1;
   short             z_len2;

/*----------------------------------------------------------------------*
 *      START OF PROCESS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set base pointer of KCB      */

   /*---------------   set environment brows conversion   --------------*/
   kcb.env = ENVZEN;
   kcb.charl = LR_UNDEFINED;

/*----------------------------------------------------------------------*
 *      MAKE LIST OF CANDIDATES(BTE)
 *----------------------------------------------------------------------*/
   z_robro1 = _Kcobro1( z_kcbptr );

   gpwptr1 = kcb.gpwgpe;
   gpw.pkakutei = PKAKALL;

   if (z_robro1 != SUCCESS)
   {
      return(UERROR);
   }

   seiptr1 = kcb.seiaddr;               /* set base pointer of SEI      */

                                        /* get length of seisho         */
                                        /*      of next conversion      */
   z_len1 = CHPTTOSH(sei.ll) - 2;

   /*-----------   rotate BTE IF SEISHO OF INIT CNV EXIST   ------------*/
   if ( ( kcb.seill - 2 != z_len1 ) && ( kcb.seill != 0 ) )
   {
                                        /* set base pointer of seisho   */
                                        /*      of initial conversion   */
      seiptr1 = kcb.seiaddr; 

                                        /* get length of seisho         */
                                        /*      of initial conversion   */
      z_len2 = (short)sei.kj[z_len1/2][0] * (short)256
                           + (short)sei.kj[z_len1/2][1] - (short)2;

      z_rnmtch = _Kcnmtch(z_kcbptr,&sei.kj[z_len1/2+1][0],z_len2);

      if( z_rnmtch.rc == Z_EXIST )
      {
         z_rcinsb=_Kccinsb(&kcb.bthchh,z_rnmtch.bteptr,NULL);
         switch(z_rcinsb)
         {
            case SUCCESS:
               break;
            default:
               return( z_rcinsb );
         }
      }
      else
      {
         if( z_rnmtch.rc != Z_NOT_EXIST )
            return( z_rnmtch.rc );
      }


   }
   /*-------------------------------------------------------------------*/

   seiptr1 = kcb.seiaddr;               /* set base pointer of seisho   */

   if (( kcb.seill == 0 ) || ( CHPTTOSH(sei.ll) == 2))
   {
      kcb.posend = 0;
   }
   else
   {
      z_rnmtch = _Kcnmtch(z_kcbptr, sei.kj ,z_len1);
      if( z_rnmtch.rc == Z_EXIST )
      {
         kcb.posend = z_rnmtch.bteno - 1;
      }
      else
      {
         if( z_rnmtch.rc == Z_NOT_EXIST )
            kcb.posend = 0;
         else
            return( z_rnmtch.rc );
      }
   }

   z_robro2 = _Kcobro2( z_kcbptr, (short)ALLFW );
   if (z_robro2 != SUCCESS)
      return( z_robro2 );

/*---------------------------------------------------------------------*
 *      RETURN
 *---------------------------------------------------------------------*/
   return( SUCCESS );
}
