static char sccsid[] = "@(#)53	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcncutb.c, libKJI, bos411, 9428A410j 6/4/91 15:15:39";
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
 * MODULE NAME:       _Kcncutb
 *
 * DESCRIPTIVE NAME:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS): success
 *                    0x0108 (WSPOVER): work space overflow
 *                    0x7fff (UERROR):  unpredictable error
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
struct RETNCUTB   _Kcncutb(z_kcbptr, z_mode)

struct  KCB  *z_kcbptr;
short        z_mode;
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern  void              _Kccfree();
   extern  struct RETCBKNJ   _Kccbknj();

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcbte.h"

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define   Z_OVER   0x02ff

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   struct RETNCUTB   z_ret;
   struct RETCBKNJ   z_rcbknj;
   struct BTE        *z_bteptr;
   short             z_endfg1;
   short             z_endfg2;
   short             z_i1;
   short             z_wklen;
   char              z_wkout[SEIMAX];
   short             z_match;
   short             z_wklen2;
   char              z_wkout2[SEIMAX];
   short             z_cmplen;

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* establish address'ty to KCB  */

/*----------------------------------------------------------------------*
 *      INITIALIZE
 *----------------------------------------------------------------------*/
   z_wklen   = SEIMAX;                  /* initialize work area length  */
   z_wklen2  = SEIMAX;                  /* initialize work area length  */

   z_ret.rc  = SUCCESS;                 /* initialize rc no of BTE delet*/
   z_ret.delno = 0;

/*----------------------------------------------------------------------*
 *      CHECK WHEATHER BTE ENTRY EXIST OR NOT
 *----------------------------------------------------------------------*/
   if (CACTV(kcb.bthchh) == 0 )
      return(z_ret);

/*----------------------------------------------------------------------*
 *      PROCESS FOR ALL BTE ENTRIES
 *----------------------------------------------------------------------*/
   FOR_FWD ( kcb.bthchh, bteptr1, z_endfg1 )
   {
      LST_FWD ( bteptr1, z_endfg1 );

   /*-------------------------------------------------------------------*
    *   IF DUMMY FLAG OF THE BTE IS "ON", DELEAT THE BTE
    *-------------------------------------------------------------------*/
      if ( bte.dmyflg == ON )
      {
         bteptr2 = bteptr1;
         CMOVB(kcb.bthchh, bteptr1 );
         _Kccfree(&kcb.bthchh,bteptr2);
         z_ret.delno++;                 /* increase no of deleted BTE   */
         continue;
      }

   /*-------------------------------------------------------------------*
    *   DELEAT UNNECESSALLY BTE
    *
    *   When environment is either ENVNEXT or ENVZEN,
    *   deleat the BTE which is established by only Fuzoku-go
    *   and whose start position of yomi is not 0
    *   and deleat the BTE whose JTE pointer is NULL.
    *-------------------------------------------------------------------*/
      if ((( bte.stap != 0 ) || (( bte.fzkflg & 0x04 ) == 0x04 )
        ||((z_mode == 1)&&(bte.jteaddr == NULL)&&(bte.fzkflg==0x8)))
        && ( ( kcb.env == ENVNEXT ) || ( kcb.env == ENVZEN ) ) )
             /*  || ( ( ( z_mode == 3 ) || ( z_mode == 2 ) )
                                          && ( bte.fzkflg == 0x08 ) )
               || ( ( ( z_mode == 1 ) || ( z_mode == 3 ) ) */
      {
         bteptr2 = bteptr1;
         CMOVB(kcb.bthchh, bteptr1 );
         _Kccfree(&kcb.bthchh,bteptr2);
         z_ret.delno++;                 /* increase no of deleted BTE   */
         continue;
      }

   /*-------------------------------------------------------------------*
    *   GET KANJI HYOKI OF BTE
    *-------------------------------------------------------------------*/
      z_rcbknj = _Kccbknj(z_kcbptr,bteptr1,z_wkout,z_wklen);
                                        /* call _Kccbknj to build       */
                                        /* hyoki in work area           */
      if ( z_rcbknj.rc != SUCCESS )
      {
         z_ret.rc = UERROR;
         return(z_ret);
      }

      z_cmplen = z_rcbknj.kjno;         /* save the length to compare   */

   /*-------------------------------------------------------------------*
    *   COMPARE TARGET BTE WITH NEXT BTE's
    *-------------------------------------------------------------------*/
      if (z_endfg1 == ON) break;
      bteptr2 = bteptr1;
      CMOVF(kcb.bthchh,bteptr2);

      FOR_FWD_MID ( kcb.bthchh, bteptr2, z_endfg2 )
      {
         LST_FWD ( bteptr2, z_endfg2 );

      /*----------------------------------------------------------------*
       *  AVOID TO COMPARE WITH DUMMY BTE
       *        CHECK START & END POSITION OF YOMI
       *----------------------------------------------------------------*/
         if ( ( bte2.dmyflg == ON )     /* check dummy fg of BTE        */
           || ( ( bte2.stap != bte.stap ) || ( bte2.endp != bte.endp ) ) )
            continue;

      /*----------------------------------------------------------------*
       *        CHECK KANJI HYOKI
       *----------------------------------------------------------------*/
         z_rcbknj = _Kccbknj(z_kcbptr,bteptr2,z_wkout2,z_wklen2);
         if ( z_rcbknj.rc != SUCCESS )
         {
            z_ret.rc = UERROR;
            return(z_ret);
         }

      /*----------------------------------------------------------------*
       *        CHECK THE LENGTH OF KANJI HYOKI
       *----------------------------------------------------------------*/
         if ( z_cmplen != z_rcbknj.kjno )
            continue;

         for(z_i1 = 0, z_match = 1; z_i1 < (z_cmplen * 2); z_i1 ++)
         {
            if( *(z_wkout + z_i1) != *(z_wkout2 + z_i1) )
            {
               z_match = 0;             /* no match                     */
               break;                   /* unnecessary to compare       */
            }
         }

         if( z_match == 1 )
         {
           z_bteptr = bteptr2;
           CMOVB(kcb.bthchh, bteptr2 );
            _Kccfree(&kcb.bthchh,z_bteptr);
            z_ret.delno++;              /* increase no of deleted BTE   */
         }

      }/* end for */
   }/* end for*/

   z_ret.rc = 0;
   z_ret.delno = z_ret.delno;
   return(z_ret);
}
