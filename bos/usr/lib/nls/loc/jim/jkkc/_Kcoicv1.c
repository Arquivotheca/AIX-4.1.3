static char sccsid[] = "@(#)65	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcoicv1.c, libKJI, bos411, 9428A410j 6/4/91 15:18:16";
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
 * MODULE NAME:       _Kcoicv1
 *
 * DESCRIPTIVE NAME:  INITIAL CONVERSION
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS):    success
 *                    0x0004 (NO_CAND):    no candidate exist
 *                    0x0006 (DURING_CNV): no candidate exist
 *                    0x1104 (JTEOVER):    JTE table overflow
 *                    0x1204 (JKJOVER):    JKJ overflow
 *                    0x1304 (JLEOVER):    JLE overflow
 *                    0x1404 (FTEOVER):    FTE table overflow
 *                    0x1704 (BTEOVER):    BTE table overflow
 *                    0x1804 (PTEOVER):    PTE table overflow
 *                    0x2104 (SEIOVER):    seisho buffer overflow
 *                    0x2204 (SEMOVER):    seisho map buffer overflow
 *                    0x2304 (YMMOVER):    ymi map buffer overflow
 *                    0x2404 (YMMOVER):    grammar map buffer overflow
 *                    0x7fff (UERROR):     unpredictable error
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
short _Kcoicv1(z_kcbptr)

struct KCB      *z_kcbptr;              /* get address of KCB           */
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern short           _Kcinprc();   /* input      process           */
   extern short           _Kcjproc();   /* jiritsu-go process           */
   extern short           _Kcfproc();   /* fuzoku-go  process           */
   extern short           _Kcrproc();   /* result     process           */
   extern void            _Kccpurg();

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcyce.h"
#include   "_Kcmce.h"
#include   "_Kcgpw.h"
#include   "_Kcsei.h"
#include   "_Kcsem.h"
#include   "_Kcgrm.h"
#include   "_Kcymm.h"
#include   "_Kcpte.h"
#include   "_Kcbte.h"

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define   Z_NOT_FIRM   -1
#define   Z_DCTSTP      1               /* abusolute conversion         */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short             z_mode;
   short             z_retry;           /* declare retry flag           */
   short             z_dctctl;          /* declare dict. control var.   */
   short             z_i     ;          /* declare loop counter         */
   short             z_rinprc;          /* input      process           */
   short             z_rjproc;          /* jiritsu-go process           */
   short             z_rfproc;          /* fuzoku-go  process           */
   short             z_rrproc;          /* result     process           */
   short             z_intkak;

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set base address of KCB      */
   gpwptr1 = kcb.gpwgpe;                /* set base address of KCB      */

/*----------------------------------------------------------------------*
 *      SET ENVIRONMENT OF CONVERSION
 *----------------------------------------------------------------------*/
   kcb.env  = ENVNONE;                  /* set ENV. initial conversion  */

   if ( gpw.pkakutei == PKAKALL)        /* set hinshi of left char      */
      gpw.pthpphin = 0;

/*----------------------------------------------------------------------*
 *      PROCESS OF INPUT YOMI
 *----------------------------------------------------------------------*/
   z_rinprc = _Kcinprc(z_kcbptr);

   if ( z_rinprc != SUCCESS )
      return(z_rinprc);                 /* then return with ret. code   */

/*----------------------------------------------------------------------*
 *      INITIALIZE
 *----------------------------------------------------------------------*/
   z_retry  = OFF;                      /* reset retry flag             */
   z_mode = ORD;

   gpw.pkakutei = PKAKNON;

/*----------------------------------------------------------------------*
 *      START CONVERSION PROCESS
 *----------------------------------------------------------------------*/
   for ( z_dctctl = 0; z_dctctl < kcb.mchacmce; z_dctctl += Z_DCTSTP )
   {
      /*---------------   JIRITSU-GO PROCESS   -------------------------*/
      z_rjproc = _Kcjproc( z_kcbptr, z_dctctl, z_mode );
                                        /* create jiritsu-go table entry*/
      switch( z_rjproc )
      {
         case SUCCESS: break;
         default:      return(z_rjproc);
      }

      /*---------------   FUZOKU-GO PROCESS   --------------------------*/
      gpw.kakuflg = OFF;

      z_rfproc = _Kcfproc( z_kcbptr, z_dctctl, z_mode );
                                        /* create fuzoku-go table entry */
      switch(z_rfproc)
      {
         case SUCCESS: break;
         default:      return(z_rfproc);
      }

      /*------------------   RESULT PROCESS   --------------------------*/
      z_rrproc = _Kcrproc( z_kcbptr, z_dctctl, z_mode );

      switch(z_rrproc)
      {
         /*----------------------   SUCCESS   --------------------------*/
         case SUCCESS:
            z_dctctl = -1;
            z_intkak = ON;
         case Z_NOT_FIRM:               /* success but not kakutei      */
            break; 

         /*-----------------   IF NO PATH, RETRY   ---------------------*/
         case NO_CAND:                  /* NO PATH                      */
            if( gpw.tbflag == ON )      /* if tanbunsetsu mode          */
            {
               gpw.tbflag = OFF;        /* reset tunbunsetsu flag       */
               gpw.pendpos = -2;        /* reset fuzokugo analayzing pos*/
            }

            else if ( ( ( kcb.ymill2 > 0 ) && ( kcb.ymill1 > kcb.ymill2 ) ) 
                        /* if 1st phrase length was specified  and      */
                   && ( z_retry == OFF )
                   && ( gpw.pkakutei != PKAKPRT ) )
            {
               z_retry = ON;            /* set retry flag               */

               gpw.pkakutei = PKAKALL;  /* set prev. kakutei            */
               kcb.ymill1 = kcb.ymill2; /* retry shorter yomi length    */
               kcb.charr  = LR_UNDEFINED;
               z_rinprc = _Kcinprc(z_kcbptr);
               if ( z_rinprc != SUCCESS )
                  return(z_rinprc);     /* then return with ret. code   */

            }

            else
               return( NO_CAND );

            z_dctctl = -1;
	    z_mode = ORD;
            continue;

         /*---------------------   FATAL ERROR   -----------------------*/
         default:
            return(z_rrproc);
      }

      if (gpw.pkakutei == PKAKALL)
         break;

      if( ( ( kcb.charr != 0x00 ) ||    /* if specified right char or   */
          ( kcb.cnvx == ON ) ) &&       /* conversion flag is kakutei   */
          (z_dctctl == kcb.mchacmce -1 ) )/* all mora was looked up     */
      {
         z_mode = ABS;                  /* set kakutei mode of YES      */
         z_dctctl = -1;
         continue;
      }
   } /* end loop */

/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/
   if (gpw.pkakutei != PKAKNON)
      return( SUCCESS );
   else
      return( DURING_CNV );
}
