static char sccsid[] = "@(#)73	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcfsfzk.c, libKJI, bos411, 9428A410j 6/4/91 12:49:25";
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
 * MODULE NAME:       _Kcfsfzk
 *
 * DESCRIPTIVE NAME:  SET DATA ON FTE
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 ( SUCCESS ):  success
 *                    0x1404 ( FTEOVER ):  FTE table overflow
 *                    0x1604 ( FKXOVER ):  FKX overflow
 *                    0x7fff ( UERROR ):   unpredictable error
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
short _Kcfsfzk(z_kcbptr,z_fweptr)

struct KCB *z_kcbptr;
struct FWE *z_fweptr;
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern struct RETCGET  _Kccget ();   /* Get  Any Kinds of Table Entry*/
   extern struct RETFHYKI _Kcfhyki();   /* Get FUZOKUGO Hoyki           */
   extern void            _Kccfree();   /* Free Any Kinds of Table Entry*/

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcfae.h"   /* Fuzokugo Attribute table Entry (FAE)         */
#include   "_Kcfle.h"   /* Fuzokugo Linkage table Entry (FLE) format    */
#include   "_Kcfwe.h"   /* Fuzokugo Work table Entry (FWE)              */
#include   "_Kcfte.h"   /* Fuzokugo Table Entry (FTE)                   */
#include   "_Kcfkx.h"   /* Fuzokugo Kj hyoki eXchange table entry (FKX) */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   struct RETCGET  z_rcget ;    /* Define Area for Return of _Kccget    */
   struct RETFHYKI z_rfhyki;    /* Define Area for Return of _Kcfhyki   */
   uschar          z_joshi;
   uschar          z_endflg;

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;
   fweptr1 = z_fweptr;

/*----------------------------------------------------------------------*
 *      INITIALIZE LOCAL VARIABLE
 *----------------------------------------------------------------------*/
   z_joshi = NULL;                       /* initialize joshi parm        */

/*----------------------------------------------------------------------*
 *      GET FTE
 *----------------------------------------------------------------------*/
   z_rcget = _Kccget( &kcb.fthchh );

   if ( z_rcget.rc == GET_EMPTY )
      return( FTEOVER );
   else if ( ( z_rcget.rc != GET_TOP_MID ) && ( z_rcget.rc != GET_LAST ) )
      return( UERROR ); 

   fteptr1 = (struct FTE *)z_rcget.cheptr;

/*----------------------------------------------------------------------*
 *      SET  STAP
 *----------------------------------------------------------------------*/
   fte.stap = fwe.stap;                  /* move mora pos to FTE         */

/*----------------------------------------------------------------------*
 *      SET  SETSU[0], SETSU[1], SETSU[2]
 *----------------------------------------------------------------------*/
   faeptr1 = kcb.faefae + fwe.fno;      /* point correpn'd FAE entry    */
   fleptr1 = kcb.flefle + fae.flendx;   /* point corrent FLE entry      */

   fte.setsu[0] = HIGH(fle.lkvt[0]);    /* move the begining 24 bits    */
   fte.setsu[1] = LOW(fle.lkvt[0]);     /* (3 bytes) of lkvt(16 bytes)  */
   fte.setsu[2] = HIGH(fle.lkvt[1]);    /* to FTE                       */

   if(LOW(fle.lkvt[1]) & 0x80)          /* check the 25th bit of lkvt   */
   {
      fte.setsu[2] |= 0x03;             /* turn on the 23rd and 24th    */
      z_joshi = 'T';                    /* bunsetsu-to                  */
   }

/*----------------------------------------------------------------------*
 *      SET  HINL
 *----------------------------------------------------------------------*/
   fte.hinl = fwe.fno;                   /* move fuzokugo no to FTE      */

/*----------------------------------------------------------------------*
 *      SET  FHTX
 *----------------------------------------------------------------------*/
   z_rfhyki  = _Kcfhyki( z_kcbptr, fweptr1, z_joshi);
                                        /* call to process FKX          */
                                        /* returns FKX pointer,if not   */
                                        /* NULL                         */
   if ( z_rfhyki.rc != SUCCESS )
      return(z_rfhyki.rc);

   if ( z_rfhyki.fkxptr == NULL )       /* no FKJ                       */
   {
      fte.fhtx = -1  ;
   }                                    /* if no FKJ,no ptr to FKX      */
   else
   {
      fkxptr1  = z_rfhyki.fkxptr;
      fte.fhtx = z_rfhyki.fkxptr - kcb.fkxfkx;
   }

/*----------------------------------------------------------------------*
 *      SET  PEN, HINR
 *----------------------------------------------------------------------*/
   fte.pen = fwe.pen;                   /* add FWE penalty to FTE       */
   fte.hinr = fae.hin;                  /* move hinshi code(uschar) to  */
                                        /* FTE (short)                  */
   if( ( fte.hinr >= 75 ) && ( fte.hinr < 90 ) )
      z_joshi = 'J';

/*----------------------------------------------------------------------*
 *      ASLONG AS FWE PARENT POINTER EXIST
 *----------------------------------------------------------------------*/
                                        /* check next FWE  entry        */
                                        /* the last is dummy FWE  entry */
                                        /* no need to process           */
   fweptr1 = (struct FWE *)fwe.prnt;    /* point parent FWE             */
                                        /* process current FWE entry    */
   while( fwe.prnt != NULL )
   {
      faeptr1 = kcb.faefae + fwe.fno;   /* point correpn'd FAE entry    */

      z_rfhyki  = _Kcfhyki(z_kcbptr,fweptr1,z_joshi);
                                        /* call to process FKX          */
                                        /* returns FKX pointer,if not   */
                                        /* NULL                         */
                                        /* null --> -1                  */
      if ( z_rfhyki.fkxptr != NULL )
      {
          fkxptr1  = z_rfhyki.fkxptr;

          if(fte.fhtx == -1)
             fte.fhtx = z_rfhyki.fkxptr - kcb.fkxfkx;
      }

      fte.pen += fwe.pen;               /* add FWE penalty to FTE       */
      fte.hinr = fae.hin;               /* move hinshi code(uschar) to  */
                                        /* FTE (short)                  */
      if( ( fte.hinr >= 75 ) && ( fte.hinr < 90 ) )
         z_joshi = 'J';

      fweptr1 = (struct FWE *)fwe.prnt; /* point parent FWE             */
                                        /* process current FWE entry    */
   }

   if ( fte.fhtx != -1   )
      fkx.flag &= 0x7f ;                /* ^  FKXCONT : 0x80            */

/*----------------------------------------------------------------------*
 *      PROONING
 *----------------------------------------------------------------------*/
   fteptr2 = fteptr1;
   CMOVB(kcb.fthchh, fteptr2 );
   if (fteptr2 == fteptr1)              /* if FTE is only 1             */
      return( SUCCESS );

   FOR_BWD_MID(kcb.fthchh,fteptr2,z_endflg)
   {
      LST_BWD(fteptr2,z_endflg);

      if ( ( fte.stap == fte2.stap )
        && ( fte.fhtx == -1 )
        && ( fte.hinr == fte2.hinr )
        && ( fte.hinl == fte2.hinl ) )
      {
         if (fte2.pen > fte.pen)
            fte2.pen = fte.pen;

         _Kccfree(&kcb.fthchh,fteptr1);
         break;
      }
   }

   return( SUCCESS );
}
