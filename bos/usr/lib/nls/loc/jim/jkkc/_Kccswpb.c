static char sccsid[] = "@(#)45	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kccswpb.c, libKJI, bos411, 9428A410j 6/4/91 10:22:02";
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
 * MODULE NAME:       _Kccswpb
 *
 * DESCRIPTIVE NAME:  DELETE DISUSED ELEMENTS OF BUNSETSU TABLE
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       number of freed entries 
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
short _Kccswpb(z_kcbptr,z_mpos)

struct KCB   *z_kcbptr;                 /* pointer of KCB               */
short        z_mpos;                    /* mora position                */
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern short             _Kccfree();

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcjte.h"
#include   "_Kcbte.h"

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   struct BTE        *z_tmpptr;         /* temporaly BTE                */
   short             z_num;             /* number of freed PTEs         */
   uschar            z_endflg ;         /* end flag                     */

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set base pointer of KCB      */

/*----------------------------------------------------------------------*
 *       FREE PTE WHICH HAS MORE THAN 5 BUNSETU-NODES
 *----------------------------------------------------------------------*/
   /*-------------------------   INITIALISE   --------------------------*/
   z_num = 0;                           /* init cnt of deleted elements */

   if( (CMOVT(kcb.bthchh,bteptr1)) == NULL)
      return(z_num);

   /*--------------------------   FREE BTEs   --------------------------*/
   FOR_FWD(kcb.bthchh,bteptr1,z_endflg)
   {
      LST_FWD(bteptr1,z_endflg);
      /*----   IF USAGE CNT IS 0 AND END POS IS LEFT OF MORA POS   -----*/
      if ( ( bte.usage == 0 ) && ( bte.endp < z_mpos ) )
      {
         jteptr1 = bte.jteaddr;
         if (jteptr1 != NULL)
            jte.usage--;                /* dec usage cnt                */

                                        /* free disused BTE             */
         z_tmpptr = bteptr1;
         CMOVB(kcb.bthchh,bteptr1);
         _Kccfree(&kcb.bthchh,z_tmpptr);

         z_num++;                       /* inc cnt of deleted elements  */
      }
   }
}
