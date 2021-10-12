static char sccsid[] = "@(#)21	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kckbrop.c, libKJI, bos411, 9428A410j 6/4/91 12:57:36";
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
 * MODULE NAME:       _Kckbrop
 *
 * DESCRIPTIVE NAME:  OPEN AND FORWARD FOR ALL CANDIDATES OF KANSUJI
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS) : Success
 *                    0x0002 (END_CAND): End of candidate
 *                    0x0004 (NO_CAND) : No candidate
 *                    0x1104 (JTEOVER) : JTE table overflow
 *                    0x1204 (JKJOVER) : JKJ table overflow
 *                    0x1304 (JLEOVER) : JLE table overflow
 *                    0x1404 (FTEOVER) : FTE overflow
 *                    0x1704 (BTEOVER) : BTE overflow
 *                    0x1804 (PTEOVER) : PTE overflow
 *                    0x0108 (WSPOVER) : work space overflows
 *                    0x7fff (UERROR)  : Unpredictable error
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
short   _Kckbrop( z_kcbptr )
struct  KCB     *z_kcbptr;              /* pointer of KCB              */
{
/*----------------------------------------------------------------------*  
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/ 
   extern short           _Kckncv1();   /* Making Next Conv Env. for A/N*/
   extern struct RETNMTCH _Kcnmtch();   /* Select BTE Having Same Seisho*/
   extern struct RETCBKNJ _Kccbknj();   /* Get BTE Seisho               */
   extern short           _Kccinsb();   /* Insert Before     Table Entry*/
/*   extern short           _Kcabro2();*/   /* Set Data for A/N All Cand.   */
   extern short           _Kcobro2();   /* Set Data for A/N All Cand.   */

/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcbte.h"   /* Bunsetsu Table Entry (BTE)                   */
#include   "_Kcsei.h"   /* SEIsho buffer entry (SEI)                    */
#include   "_Kcymm.h"   /* YoMi Map entry (YMM)                         */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/ 
#define Z_OVRFLW    (0x0200 | LOCAL_RC) /* output buffer over flow      */

#define Z_EXIST     (0x0000 | LOCAL_RC) /* exist in candidates          */
#define Z_NOT_EXIST (0x0100 | LOCAL_RC) /* not exist in candidates      */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   short           z_rkncv1;    /* Define Area for Return of _Kcancv1   */
   short           z_rcinsb;    /* Define Area for Return of _Kccinsb   */
   struct RETCBKNJ z_rcbknj;    /* Define Area for Return of _Kccbknj   */
   struct RETNMTCH z_rnmtch;    /* Define Area for Return of _Kcnmtch   */

   short           z_seilen;            /* set seisyo entry length      */
   short           z_countr;            /* loop countr                  */
   short           z_maxsei;            /* set max seisyo length        */
   uschar          z_buf[SEIMAX];       /* work area                    */
   short           z_endflg;            /* end flag                     */

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
   kcbptr1 = z_kcbptr;                  /* initialize of kcbptr1        */
   seiptr1 = kcb.seiaddr;               /* initialize of z_seiptr       */

/*----------------------------------------------------------------------*
 *      CREATE CANDIDATES ON BTE
 *----------------------------------------------------------------------*/
   z_rkncv1 = _Kckncv1( kcbptr1 );

   if ( z_rkncv1 != SUCCESS )
      return( z_rkncv1 );

/*----------------------------------------------------------------------*
 *      CHANGE ORDER
 *----------------------------------------------------------------------*/
   z_seilen = CHPTTOSH( sei.ll  );      /* set seisyo entry length      */

   if( kcb.seill != z_seilen )          /* if a seisho which should be  */
   {                                    /* top of chain                 */
      seiptr1 = (struct SEI *)((uschar *)seiptr1 + z_seilen);
      z_seilen = CHPTTOSH( sei.ll)-2;   /* set seisyo entry length      */
      z_rnmtch = _Kcnmtch( kcbptr1, sei.kj , z_seilen );
      if ( z_rnmtch.rc != Z_EXIST )     /* if specified Seisho exists   */
      {
         if (z_rnmtch.rc == Z_NOT_EXIST)
            return( UERROR );           /* DO NOTHING                   */
         else
            return( z_rnmtch.rc );
      }
                                        /* set top of the chain         */
      z_rcinsb = _Kccinsb( &kcb.bthchh, z_rnmtch.bteptr, NULL );
      if ( z_rcinsb != SUCCESS )
         return( NO_CAND );
   }

/*----------------------------------------------------------------------*
 *      GET NUMBER OF OUTPUT CHARACTERS
 *----------------------------------------------------------------------*/
   z_maxsei = 0;                        /* reset max seisyo length      */

   FOR_FWD( kcb.bthchh , bteptr1 , z_endflg )
   {
      LST_FWD( bteptr1 ,z_endflg );

      z_rcbknj = _Kccbknj( kcbptr1, bteptr1, z_buf , (short)SEIMAX );

      if ( z_rcbknj.rc != SUCCESS)
      {
         if ( z_rcbknj.rc ==  Z_OVRFLW)
            return( UERROR );
         else
            return( z_rcbknj.rc );
      }

      if( ( z_rcbknj.kjno * 2 ) > z_maxsei )
      {
         z_maxsei = z_rcbknj.kjno * 2;  /* reset maxsei length          */
      }
   }

   kcb.maxsei = z_maxsei;               /* set maxsei length to KCB     */

/*----------------------------------------------------------------------*
 *      SET THE NUMBER OF POSITION
 *----------------------------------------------------------------------*/
   seiptr1  = kcb.seiaddr;
   z_seilen = CHPTTOSH( sei.ll) - 2;    /* set seisyo entry length      */

   if( z_seilen == 0 )                  /* if the seisho which should   */
   {                                    /* be top was not specified     */
      kcb.posend = 0;                   /* pos end is top               */
   }
   else                                 /* if the seisho which should   */
   {                                    /* be top was  specified        */
      z_rnmtch = _Kcnmtch( kcbptr1, sei.kj , z_seilen );
      if ( z_rnmtch.rc != Z_EXIST )     /* if specified Seisho exists   */
      {
         if (z_rnmtch.rc == Z_NOT_EXIST)
            return( UERROR );           /* DO NOTHING                   */
         else
            return( z_rnmtch.rc );
      }

      kcb.posend = z_rnmtch.bteno-1;
   }

/*----------------------------------------------------------------------*
 *      CALL SETTING SEISHO ON SEISHO BUFFER AND RETURN
 *----------------------------------------------------------------------*/
   return(_Kcobro2( z_kcbptr, (short)FORWARD ));

}
