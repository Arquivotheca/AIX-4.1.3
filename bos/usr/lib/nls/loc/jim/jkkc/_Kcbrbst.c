static char sccsid[] = "@(#)19	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcbrbst.c, libKJI, bos411, 9428A410j 6/4/91 10:14:47";
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
 * MODULE NAME:       _Kcbrbst
 *
 * DESCRIPTIVE NAME:  PROCEED RIGHT CHARACTER WHITCH IS SPECIFIED BY
 *                    KKC USER.
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
#include   "_Kcrcb.h"                   /* Define Return Code Structure */
#include   "_Kcrpn.t"                   /* Right character's Penalty    */
 
/************************************************************************
 *      START OF FUNCTION                                                 
 ************************************************************************/ 
void  _Kcbrbst(z_kcbptr)
struct KCB  *z_kcbptr;                  /* get KCB base address         */
{ 
/*----------------------------------------------------------------------*  
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/ 
   extern void            _Kccfree();   /* Free Any Kinds of Table Entry*/

/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcpte.h"   /* Path Table Entry (PTE)                       */
#include   "_Kcbte.h"   /* Bunsetsu Table Entry (BTE)                   */
#include   "_Kcjte.h"   /* Jiritsugo Table Entry (JTE)                  */
 
/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/ 
#define    Z_MAX_PEN    0x7FFF
#define    Z_DFLG_MRU   0x06

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   short           z_i;         /* loop counter                         */
   struct PTE     *z_minptr;    /* PTE which has minimum pen.           */
   struct PTE     *z_tmpptr;    /* PTE which will be freed              */
   short           z_minpen;    /* minimum penalty                      */
   uschar          z_end;       /* end flag                             */

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
/*----------------------------------------------------------------------*  
 *      PROCEED RIGHT CHARACTER WHITCH IS SPECIFIED BY KKC USER
 *----------------------------------------------------------------------*/ 
   kcbptr1 = z_kcbptr;
   z_minpen = Z_MAX_PEN;
   z_minptr = NULL;
/*----------------------------------------------------------------------*
 *   ADD THE PENALTY OF RIGHT CHRARCTER THE LAST PTE
 *----------------------------------------------------------------------*/
   FOR_BWD(kcb.pthchh,pteptr1,z_end)
   {
      LST_BWD(pteptr1,z_end);

      if( ( kcb.mchacmce - 1 ) > pte.endp )
         break;
      for(z_i = 0 ; z_i < 6 ; z_i++)
         if (pte.bteaddr[z_i] == NULL)
            break;
      bteptr1 = pte.bteaddr[z_i-1];
      pte.pen += rchr_pen[bte.hinr][kcb.charr];

      if (z_minpen >= pte.pen)
      {
         if ( z_minpen ==  pte.pen )
         {
            pteptr2 = z_minptr;
            for ( z_i = 0; z_i < 6; z_i++ )
               if ( pte2.bteaddr[z_i] == NULL )
                  break;
            jteptr2 = (*pte2.bteaddr[z_i-1]).jteaddr;
                                        /* JTE2 has minpen              */
            jteptr1 = bte.jteaddr;      /* JTE1 has current             */

            if(bte.stap == (*pte2.bteaddr[z_i-1]).stap)
            {                           /* J1&J2 has same start pos.    */
               if (jteptr1 != NULL )
               {                        /* J1 is not dummy              */
                  if((jte.dflag[0] & Z_DFLG_MRU)!=Z_DFLG_MRU)
                  {                     /* J1 is not MRU word           */
                     if(jteptr2 == NULL)
                     {                  /* J2 is dmy                    */
                        z_tmpptr = pteptr1;
                        CMOVF(kcb.pthchh,pteptr1);
                        _Kccfree(&kcb.pthchh,z_tmpptr);
                                        /* free pte                     */
                        continue;
                     }
                     else
                     {
                        if(jte.len < jte2.len)
                        {               /* J2 is longer than J1         */
                           z_tmpptr = pteptr1;
                           CMOVF(kcb.pthchh,pteptr1);
                           _Kccfree(&kcb.pthchh,z_tmpptr);
                           continue;
                        }
                     }                  /* end else                     */
                  }
               }
            }
         }
         if(z_minptr != NULL)
         {
            _Kccfree(&kcb.pthchh,z_minptr);
                                        /* free PTE not having minpen   */
         }
         z_minpen = pte.pen;
         z_minptr = pteptr1;
      }
      else
      {
         z_tmpptr = pteptr1;
         CMOVF(kcb.pthchh,pteptr1);
         _Kccfree(&kcb.pthchh,z_tmpptr);
      }
   }                                    /* end of for                   */
}                                       /* end of program               */
