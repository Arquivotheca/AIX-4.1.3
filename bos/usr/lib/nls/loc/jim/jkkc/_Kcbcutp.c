static char sccsid[] = "@(#)13	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcbcutp.c, libKJI, bos411, 9428A410j 6/4/91 10:11:39";
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
 * MODULE NAME:       _Kcbcutp
 *
 * DESCRIPTIVE NAME:  CUT PATH WHICH HAS LARGE PENALTY
 *
 * FUNCTION:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS)    : Success
 *                    0x7FFF(UERROR )    : Fatal error
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
short _Kcbcutp(z_kcbptr,z_endpos)

struct KCB      *z_kcbptr;              /* pointer of KCB               */
short           z_endpos;               /* offset of mora end           */
{
/*----------------------------------------------------------------------*  
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/ 
   extern struct RETCFPTH _Kccfpth();   /* Sellect Least Penalty Path   */
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
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   struct RETCFPTH z_rcfpth;    /* Define Area for Return of _Kccfpth   */
   short           z_i;                 /* counter                      */
   struct PTE     *z_tmpptr;
   uschar          z_endflg;

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
   kcbptr1 = z_kcbptr;                  /* set base pointer of KCB      */

   if ( CACTV( kcb.pthchh ) > 0 )       /* if path entry exists         */
   {
                                        /* get smallest penalty of PTEs */
      z_rcfpth = _Kccfpth(z_kcbptr,z_endpos,(short)0);

      if ( z_rcfpth.rc == FPTH_NOT_FND )
         return( SUCCESS );
      else if ( z_rcfpth.rc != FPTH_FOUND )
         return( UERROR );
 
      FOR_FWD(kcb.pthchh,pteptr1,z_endflg)/* inqure all path entries    */
      {
         LST_FWD(pteptr1,z_endflg);

         if ( pte.endp == z_endpos )    /* if endp is equal to end of   */
         {                              /*   bunsetu                    */

                                        /* if penalty is larger than    */
                                        /*   border                     */
            pteptr2 = z_rcfpth.pteptr;
            if (pte.pen > pte2.pen + 30 )
            {                           /* free                         */
               for ( z_i = 0; z_i < 5; z_i++ )
               {
                  if ( pte.bteaddr[z_i] != NULL )
                  {
                     bteptr1 = pte.bteaddr[z_i];
                     bte.usage--;       /* reduce count of usage        */
                                        /*   of all BTE                 */
                     if((bte.usage == 0)&&(bte.jteaddr!=NULL))
                     {                  /* reduce count of usage of JTE */
                        jteptr1 = bte.jteaddr;
                        jte.usage--;
                        _Kccfree(&kcb.bthchh,bteptr1);
                     }
                  }
               }
                                        /* free PTE                     */
               z_tmpptr = pteptr1;
               CMOVB(kcb.pthchh,pteptr1);
               _Kccfree(&kcb.pthchh,z_tmpptr);
            }
         }
      }
   }

   return( SUCCESS );
}
