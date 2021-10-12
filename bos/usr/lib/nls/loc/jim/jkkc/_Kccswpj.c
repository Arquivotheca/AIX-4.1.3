static char sccsid[] = "@(#)46	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kccswpj.c, libKJI, bos411, 9428A410j 6/4/91 10:22:21";
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
 * MODULE NAME:       _Kccswpj
 *
 * DESCRIPTIVE NAME:  DELETE DISUSED ELEMENTS OF JIRITSU-GO TABLE
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       the number of deleated JTE entries
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
short _Kccswpj(z_kcbptr,z_mpos)

struct KCB   *z_kcbptr;                 /* pointer of KCB               */
short        z_mpos;                    /* mora position                */
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern void              _Kccfree();

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcjte.h"
#include   "_Kcjkj.h"

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   struct JKJ        *z_jkjtmp;         /* temporaly JKJ pointer        */
   struct JTE        *z_jtetmp;         /* temporaly JTE pointer        */
   short             z_rc;              /* return code from _Kccfree    */
   short             z_num;             /* number of freed PTEs         */
   short             z_i;               /* counter                      */
   short             z_endflg;          /* flag                         */
   uschar            z_endflg1;         /* end flag for loop            */


/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set base pointer of KCB      */

/*----------------------------------------------------------------------*
 *       FREE JTE WHOSE USAGE COUNTER IS 0
 *----------------------------------------------------------------------*/
   /*-------------------------   INITIALISE   --------------------------*/
   z_num = 0;                           /* init cnt of deleted elements */

   if( (CMOVT(kcb.jthchh,jteptr1)) == NULL)
      return(z_num);

   /*--------------------------   FREE JTEs   --------------------------*/
   FOR_FWD(kcb.jthchh,jteptr1,z_endflg1)/* check all elems of jiritu-go */
   {
      LST_FWD(jteptr1,z_endflg1);
      /*-----  IF USAGE CNT IS 0 AND END POS IS LEFT OF MORA POS   -----*/
      if ( ( jte.usage == 0 ) && ( ( jte.stap + jte.len - 1 ) < z_mpos ) )
      {
         /*------------------   DELETE KANJI-HYOUKI   ------------------*/
         if ( jte.jkjaddr != NULL )
         {
            jkjptr1 = jte.jkjaddr;      /* set 1st jiritu-go hyouki ptr */
            z_endflg = OFF;             /* init end-flag                */

            FOR_FWD_MID(kcb.jkhchh,jkjptr1,z_endflg)
            {
               if ((jkj.kj[0] & 0x80) == 0x00 )
                  z_endflg = ON;        /* set flag on if end of hyouki */
               z_jkjtmp = jkjptr1;
               CMOVB(kcb.jkhchh,jkjptr1);/* get last jiritu hyouki ptr  */
                                        /* free jiritu-hyoki pointer    */
               _Kccfree(&kcb.jkhchh,z_jkjtmp);

            }
         }
                                        /* free disused elem            */
         z_jtetmp = jteptr1;
         CMOVB(kcb.jthchh,jteptr1);
         _Kccfree(&kcb.jthchh,z_jtetmp);

         z_num++;                       /* include cnt of deleted elems */
      }
   }
          
/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/
   return (z_num);
}
