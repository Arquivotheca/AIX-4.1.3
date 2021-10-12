static char sccsid[] = "@(#)93	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kc0jlrn.c, libKJI, bos411, 9428A410j 6/4/91 10:04:45";
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
 * MODULE NAME:       _Kc0jlrn
 *
 * DESCRIPTIVE NAME:  STUDY OF JIRITUGO'S HYOKI
 *
 * FUNCTION:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       VOID
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------*  
 *      INCLUDE FILES                                                     
 *----------------------------------------------------------------------*/ 
#if defined(_AGC) || defined(_GSW)
#include   "_Kcfnm.h"                   /* Define Names file            */
#endif
#include   "_Kcmap.h"                   /* Define Constant file         */

/************************************************************************
 *      START OF FUNCTION                                                 
 ************************************************************************/ 
void   _Kc0jlrn( z_kcbptr )
struct  KCB     *z_kcbptr;              /* pointer of KCB              */
{
/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcymi.h"   /* YoMI buffer (YMI)                            */
#include   "_Kcsei.h"   /* SEIsho buffer entry (SEI)                    */
#include   "_Kcsem.h"   /* SEisho Map entry (SEM)                       */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/ 
#define   Z_WKSIZE   256                /* Tempolary Work area size     */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   short        z_mrulen;               /* set MRU length               */
   short        z_countr;               /* counter                      */
   uschar       z_work[Z_WKSIZE];       /* work area                    */
   uschar      *z_wrkptr;               /* pointer of work area         */
   uschar      *z_ymiptr;               /* internal yomi buffer pointer */
   uschar      *z_seiptr;               /* internal seisyo buffer point */
   uschar      *z_semptr;               /* internal seisyo map pointer  */

   uschar      *z_mrupt1;               /* internal MRU buffer pointer  */
   uschar      *z_mrupt2;               /* internal MRU buffer pointer  */

   short        z_wrklen;               /* work area length             */
   short        z_entlen;               /* each entry length            */
   uschar       z_cmpare;               /* compare entry flag           */
   uschar       *z_mrulim;              /* MRU limit                    */


/*----------------------------------------------------------------------*  
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/ 
   kcbptr1  = z_kcbptr;                 /* initialize of kcbptr1        */
   ymiptr1  = kcb.ymiaddr;              /* initialize of ymiptr1        */
   seiptr1  = kcb.seiaddr;              /* initialize of seiptr1        */
   semptr1  = kcb.semaddr;              /* initialize of semptr1        */

/*----------------------------------------------------------------------*
 *      SET TARGET YOMI & SEISHO IN WORK BUFFER
 *
 *          +----+---------------+----+------------------+
 *          | YL |     YOMI      | SL |     SEISHO       |
 *          +----+---------------+----+------------------+
 *
 *          |----|               |----|
 *           1 byte               1 byte
 *
 *          |--------------------|-----------------------|
 *                  YL                    SL
 *----------------------------------------------------------------------*/
   z_work[ 0 ] = ( kcb.ymill2 + 1 );    /* set YOMI length in work area */

   /*---------  ESCAPE-CODE(0x0E) IS INSERTED THE TOP OF YOMI  ---------*/
   if( ( sem.code[0] == ALPHANUM ) || ( sem.code[0] == ALPHAKAN ) )
   {                                    /* for alphanumeric             */
      z_work[ 0 ]++ ;                   /* yomi length is longer 1 byte */
      z_work[ 1 ] = ESC_ALPHA;          /* The alphanumeric escape      */
      z_countr = 2;
   }
   else
      z_countr = 1;

   /*--------------------  SET YOMI IN WORK BUFFER  --------------------*/
   z_ymiptr = ymi.yomi;                 /* initialize of z_ymiptr       */

   for( ; z_countr < z_work[0] ; z_countr++ )
   {
      z_work[ z_countr ] = *z_ymiptr++; /* copy YOMI code in work area  */
   }

   /*-------------------  SET SEISHO IN WORK BUFFER  -------------------*/
   z_seiptr = ( uschar *) sei.kj;       /* initialize of z_seiptr       */
   z_semptr = sem.code;                 /* initialize of z_semptr       */

   z_wrkptr = &( z_work[ z_countr ] );  /* initialize of z_wrkptr       */
   z_wrkptr++;                          /* pass 1 byte for sei length   */
   z_work[ z_countr ] = 1;              /* SEISYO length + 1            */

   do
   {
      *z_wrkptr = *z_seiptr;            /* set first byte to work area  */
      *z_wrkptr &= U_MSB_O;             /* reset 2 bits of MSB          */
      z_wrkptr++;                       /* point next byte in work area */
      z_seiptr++;

      *z_wrkptr = *z_seiptr;            /* set second byte to work area */
      z_wrkptr++;                       /* point next byte in work area */
      z_seiptr++;

      z_work[ z_countr ] += 2;          /* add length.  2 bytes         */

      z_semptr++;                       /* ptr of next Seisho map       */
   }
   while( ( *z_semptr == CONTINUE ) &&
          ( z_work[ z_countr ] + 1 < CHPTTOSH( sei.ll ) ) );

   z_wrkptr -= 2;
   *z_wrkptr |= U_MSB_M;

/*----------------------------------------------------------------------*
 *      SET YOMI & SEISHO ( IN WORK BUFFER ) IN MRU
 *----------------------------------------------------------------------*/
   z_mrulen = ( short )GETSHORT(kcb.mdemde);
   if( z_mrulen == NULL )
       z_mrulen = 2;
                                         /* set used MRU length         */
   z_wrklen = z_work[0] + z_work[ z_countr ];
                                        /* set used work area length    */
   z_mrulim = kcb.mdemde + ( U_MRU_A - z_wrklen );
                                        /* calculate MRU limit          */
   z_mrupt1 = kcb.mdemde + 2;           /* initialize of z_mruptr       */

   /*------------  SEARCH PLACE TO SET TARGET YOMI & SESHO  ------------*/
   while( 1 )
   {
      z_entlen = *z_mrupt1;             /* set mru entry yomi length    */
      z_entlen += *(z_mrupt1+z_entlen); /* set mru entry length         */

      /*---------------------  SEARCH SAME ENTRY  ----------------------*/
      if ( z_entlen != z_wrklen )
         z_cmpare = OFF;
      else
      {
         z_cmpare = ON;                    /* set compare flag ON          */

         for( z_countr = 0; z_countr < z_entlen; z_countr++ )
         {
            if( *(z_mrupt1+z_countr) != z_work[ z_countr ])
            {
               z_cmpare = OFF;          /* entry isn't same work area   */
               break;
            }
         }
      }

      /*--------------------  IF SAME ENTRY EXISTS  --------------------*/
      if( z_cmpare == ON )              /* there is same entry in MRU   */
      {
         z_mrupt2 = z_mrupt1 + z_entlen - 1;
                                        /* set mrupt2 as copy address   */
         break;
      }

      z_mrupt1 += z_entlen;             /* set pointer of next entry    */

      /*-------------------  SHORTAGE OF MRU SPACE  --------------------*/
      if( z_mrupt1 > z_mrulim )
      {                                 /* MRU over the mrulimit        */
         z_mrupt2 = z_mrupt1 + z_wrklen - z_entlen - 1;
                                        /* set mrupt2 as copy address   */
         z_mrulen += ( z_wrklen - z_entlen );
         break;                         /* set new used MRU length      */
      }

      /*-------------------------  END OF MRU  -------------------------*/
      else if( z_mrupt1 - kcb.mdemde >= z_mrulen )
      {                                 /* last MRU entry               */
         z_mrupt2 = z_mrupt1 + z_wrklen - 1;
                                        /* set mrupt2 as copy address   */
         z_mrulen += z_wrklen;
         break;                         /* set new used MRU length      */
      }
   }

   /*---------------------  SHIFT CONTENTS OF MRU  ---------------------*/
   for( z_mrupt1 = (z_mrupt2 - z_wrklen);
                     z_mrupt2 >= (kcb.mdemde + 2); z_mrupt1--, z_mrupt2-- )
      *z_mrupt2 = *z_mrupt1;

   /*--------------------  SET WORK BUFFER IN MRU  ---------------------*/
   z_mrupt2 = kcb.mdemde + 2;           /* indicate MRU first address   */

   for( z_countr = 0; z_countr < z_wrklen ; z_countr++, z_mrupt2++ )
      *z_mrupt2 = z_work[ z_countr ];

   /*------------------------  SET MRU LENGTH  -------------------------*/
   z_mrupt1 = kcb.mdemde;
   *(z_mrupt1++) = z_mrulen % 256;
   *z_mrupt1 = z_mrulen / 256;

/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/
   return;
}

