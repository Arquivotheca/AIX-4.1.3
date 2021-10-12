static char sccsid[] = "@(#)47	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kccswpp.c, libKJI, bos411, 9428A410j 6/4/91 10:22:43";
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
 * MODULE NAME:       _Kccswpp
 *
 * DESCRIPTIVE NAME:  DELETE DISUSED ELEMENTS OF PATH TABLE
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       NOT ERROR CODE
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
struct RETCSWPP _Kccswpp(z_kcbptr,z_pteptr)

struct KCB   *z_kcbptr;                 /* pointer of KCB               */
struct PTE   *z_pteptr;                 /* pointer of PTE               */

{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern void            _Kccfree();   /* Free Any Kinds of Table Entry*/
   extern struct RETCFPTH _Kccfpth();   /* Sellect Least Penalty Path   */

/*----------------------------------------------------------------------*
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcpte.h"   /* Path Table Entry (PTE)                       */
#include   "_Kcbte.h"   /* Bunsetsu Table Entry (BTE)                   */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define    Z_MOD_LG4    2
#define    Z_MAX_PEN    0x7fff

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   struct RETCSWPP z_ret   ;    /* Define Area for Return of Own        */
   struct RETCFPTH z_rcfpth;    /* Define Area for Return of _Kccfpth   */
   struct PTE     *z_tmpptr;            /* tempolary PTE pointer        */
   short           z_avpen;             /* averase penalty of PTE       */
   short           z_i;                 /* counter                      */
   short           z_bon;               /* boundry of penalty           */
   uschar          z_endflg;            /* end flag                     */
   uschar          z_endflg2;           /* end flag 2                   */

/*----------------------------------------------------------------------*
 *      START OF PROCESS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set base pointer of KCB      */

/*----------------------------------------------------------------------*
 *      FREE PTE WHICH HAS MORE THAN 4 BUNSETU-NODES
 *----------------------------------------------------------------------*/
   /*-------------------------   INITIALISE   --------------------------*/
   z_ret.num = 0;                       /* init cnt of deleted elements */
   z_ret.pteptr = z_pteptr;             /* init return of pteptr        */

   CMOVT(kcb.pthchh,pteptr1);

   if ( pteptr1 == NULL )               /* return if no element exist   */
      return(z_ret);

   /*--------------------------   FREE PTEs   --------------------------*/
   FOR_FWD(kcb.pthchh,pteptr1,z_endflg)
   {
      LST_FWD(pteptr1,z_endflg);
      /*----------   IF PTE HAS MORE THAN 4 BUNSETSU-NODES   -----------*/
      if ( pte.bteaddr[4] != NULL )
      {
         /*--------------   FREE ELEMENT BUNSETSU-TABLE   --------------*/
         for (z_i = 0;(pte.bteaddr[z_i]!=NULL)&&(z_i<6); z_i++ )
         {
            bteptr1 = pte.bteaddr[z_i];
            bte.usage--;                /* dec usage cnt of bunsetu elem*/
         }
         if(z_pteptr == pteptr1)
         {
            z_pteptr = pteptr1;
            z_pteptr = CMOVB(kcb.pthchh,z_pteptr);
         }
         z_tmpptr = pteptr1;
         CMOVB(kcb.pthchh,pteptr1);
         _Kccfree(&kcb.pthchh,z_tmpptr);/* free unused PTE              */
         z_ret.num++;                   /* incliment cnt of deleted elem*/
      }
   }
          
/*----------------------------------------------------------------------*
 *       FREE PTE WHICH HAS LARGER PENALTY THAN ANOTHER LIKELY PTE
 *----------------------------------------------------------------------*/
   /*-------------------------------------------------------------------*
    *       OUTER LOOP
    *-------------------------------------------------------------------*/
   FOR_FWD(kcb.pthchh,pteptr2,z_endflg)
   {
      LST_FWD(pteptr2,z_endflg);
      pteptr1 = pteptr2;
      CMOVF(kcb.pthchh,pteptr1);
      if( pteptr1 == pteptr2)           /* return if no element exist   */
         return(z_ret);
      /*----------------------------------------------------------------*
       *       INNER LOOP
       *----------------------------------------------------------------*/
      FOR_FWD_MID(kcb.pthchh,pteptr1,z_endflg2)
      {
         LST_FWD(pteptr1,z_endflg2);

         if ( (pte.pen > pte2.pen) && (pte.endp == pte2.endp) &&
           (pte.bteaddr[1] != NULL) && (pte.bteaddr[1] == pte2.bteaddr[1]) )
         {
            for (z_i = 0;(pte.bteaddr[z_i]!=NULL)&&(z_i<6); z_i++ )
            {
               bteptr1 = pte.bteaddr[z_i];
               bte.usage--;             /* dec usage cnt of bunsetu elem*/
            }
            if(z_pteptr == pteptr1)
            {
               z_pteptr = pteptr1;
               z_pteptr = CMOVB(kcb.pthchh,z_pteptr);
            }

            z_tmpptr = pteptr1;
            CMOVB(kcb.pthchh,pteptr1);
            _Kccfree(&kcb.pthchh,z_tmpptr);/* free unused PTE           */

            z_ret.num++;                 /*incliment cnt of deleted elem*/
         }
      }
   }
          
/*----------------------------------------------------------------------*
 *       FREE PTE WHICH HAS VERY LARGE PENALTY
 *----------------------------------------------------------------------*/
   if ( z_ret.num == 0 )
   {
      z_rcfpth = _Kccfpth(z_kcbptr,(short)1,(short)Z_MOD_LG4);
                                        /* get smallest penalty of PTEs */
      z_bon = (z_rcfpth.pen * 4 / 3) + 2;/* get boundary of penalty     */

      /*------------------------   FREE PTEs   -------------------------*/
      FOR_FWD(kcb.pthchh,pteptr1,z_endflg)
      {
         LST_FWD(pteptr1,z_endflg);

         if ( pte.endp >= 0 )
            z_avpen = pte.pen *10 / (pte.endp + 1);
         else
            z_avpen = Z_MAX_PEN;

         if ( ( z_avpen > z_bon ) && ( pte.bteaddr[1] != NULL ) )
         {
            for (z_i = 0;(pte.bteaddr[z_i]!=NULL)&&(z_i<6); z_i++ )
            {
               bteptr1 = pte.bteaddr[z_i];
               bte.usage--;             /* dec usage cnt of bunsetu elem*/
            }
            if(z_pteptr == pteptr1)
            {
               z_pteptr = pteptr1;
               z_pteptr = CMOVB(kcb.pthchh,z_pteptr);
            }

            z_tmpptr = pteptr1;
            CMOVB(kcb.pthchh,pteptr1);
            _Kccfree(&kcb.pthchh,z_tmpptr);/* free unused PTE           */
            z_ret.num++;                /*increment cnt of deleted elems*/
         }
      }
   }
          
/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/
   z_ret.pteptr = z_pteptr;
   return(z_ret);
}
