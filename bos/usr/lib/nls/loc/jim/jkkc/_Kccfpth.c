static char sccsid[] = "@(#)28	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kccfpth.c, libKJI, bos411, 9428A410j 6/4/91 10:17:16";
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
 * MODULE NAME:       _Kccfpth
 *
 * DESCRIPTIVE NAME:  FIND OUT A PATH WHICH HAS LOWER A PENALTY
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       (0x00FF)FPTH_FOUND   : Found
 *                    (0x01FF)FPTH_NOT_FND : Not Found
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
struct RETCFPTH _Kccfpth(z_kcbptr,z_pthend,z_mode)

struct KCB *z_kcbptr;
short      z_pthend;
short      z_mode;
{
/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcpte.h"   /* Path Table Entry (PTE)                       */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/ 
#define    Z_MAX_PEN    0x7fff
#define    Z_MOD_EQ     0
#define    Z_MOD_LNG    1
#define    Z_MOD_LG4    2

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   struct RETCFPTH z_ret;       /* Define Area for Return of Own        */
   short  z_minpen;
   short  z_wkpen;
   short  z_end ;

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
   kcbptr1 = z_kcbptr;                  /* establish address'ty to kcb  */
                                        /*                              */
/*----------------------------------------------------------------------*  
 *      INITIALIZE LOCAL VARIABLE
 *----------------------------------------------------------------------*/ 
   z_ret.rc = FPTH_NOT_FND;
   z_ret.pen = Z_MAX_PEN;
   z_ret.pteptr = NULL;
   z_minpen = Z_MAX_PEN ;
   if(z_pthend < 0)
      return(z_ret);

/*----------------------------------------------------------------------*  
 *      FIND THE LOWEST PENALTY UNTIL THE LAST PTE
 *----------------------------------------------------------------------*/ 
   FOR_BWD(kcb.pthchh,pteptr1,z_end)
   {
      LST_BWD(pteptr1,z_end);
      if(z_pthend > pte.endp)           /* test                         */
         break;
                                        /* establish address'ty to PTE  */
      if((z_pthend == pte.endp)         /* path ends at mora end        */
        ||((z_pthend < pte.endp) && (z_mode >= Z_MOD_LNG)))
      {
         z_wkpen = pte.pen * 10 / (pte.endp + 1);

         if(z_wkpen > z_minpen)         /* smaller than minmum penalty  */
         {
            continue  ;
         }
         else
         {
           if((z_wkpen == z_minpen)&&   /* smaller than minmum penalty  */
             ((*z_ret.pteptr).endp > pte.endp))
              continue;
         }
         z_minpen = z_wkpen;
         z_ret.pteptr = pteptr1;
         z_ret.rc = FPTH_FOUND;
      }
   }                                    /* end of for                   */

/*----------------------------------------------------------------------*  
 *      IF NOT ARRIVED 4 BUNSETSU THEN NOT FOUND
 *----------------------------------------------------------------------*/ 
   if ((z_ret.pteptr != NULL)&&(z_mode == Z_MOD_LG4))
   {
      pteptr1 = z_ret.pteptr;
      if (pte.bteaddr[4] == NULL)
         z_ret.rc = FPTH_NOT_FND;
   }
   z_ret.pen = z_minpen;
   return(z_ret);                       /* return to caller             */
}

