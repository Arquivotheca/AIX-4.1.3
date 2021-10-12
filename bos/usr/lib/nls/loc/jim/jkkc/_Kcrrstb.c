static char sccsid[] = "@(#)82	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcrrstb.c, libKJI, bos411, 9428A410j 6/4/91 15:21:48";
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
 * MODULE NAME:       _Kcrrstb
 *
 * DESCRIPTIVE NAME:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       void
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
void   _Kcrrstb(z_kcbptr,z_bteptr)

struct KCB   *z_kcbptr;                 /* pointer of KCB               */
struct BTE   *z_bteptr;                 /* pointer of BTE               */
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern short             _Kcbbpen();
   extern void              _Kccfree();

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcpte.h"
#include   "_Kcbte.h"
#include   "_Kcjte.h"
#include   "_Kcjle.h"
#include   "_Kcjkj.h"
#include   "_Kcjtx.h"
#include   "_Kcmce.h"
#include   "_Kcyce.h"
#include   "_Kcype.h"
#include   "_Kcgpw.h"

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   uschar            *z_tmpptr;         /* tempolary  pointer           */
   short             z_num;             /* number of kakutei mora code  */
   short             z_ynum;            /* number of kakutei yomi code  */
   short             z_i;               /* counter                      */
   short             z_pen;             /* penalty                      */
   short             z_endflg;          /* penalty                      */
   short             z_endflg2;         /* penalty                      */

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set base pointer of KCB      */
   bteptr2 = z_bteptr;                  /* set base pointer of BTE      */
   gpwptr1 = kcb.gpwgpe;                /* set base pointer of GPW      */

/*----------------------------------------------------------------------*
 *      GET NUMBER OF MORA & YOMI ( z_num & z_ynum )
 *----------------------------------------------------------------------*/
   if ( bte2.dmyflg != ON )
   {
      z_num = bte2.endp - bte2.stap + 1;/* get length of mora codes     */
      mceptr1 = kcb.mchmce + bte2.endp;

      if ( bte2.stap == 0 )
      {
                                        /* get length of 7bit-yomi      */
         z_ynum = mce.yceaddr - kcb.ychyce + (short)1;
      }
      else
      {
         mceptr2 = kcb.mchmce + bte2.stap - (short)1;
                                        /* get length of 7bit-yomi      */
         z_ynum = mce.yceaddr - mce2.yceaddr;
      }
   }

   else
   {
      z_num = 0;
      z_ynum = 0;
   }

/*----------------------------------------------------------------------*
 *      FREE PTE
 *----------------------------------------------------------------------*/
   /*---------------------   CHECK ALL ELEMENTS   ----------------------*/
   gpw.moraof2p = -1;

   FOR_FWD ( kcb.pthchh, pteptr1, z_endflg )
   {
      LST_FWD ( pteptr1, z_endflg );

      /*--------------   IF THE PATH HAS FIRMED BUNSETU   --------------*/
      if ( ( pte.bteaddr[0] == z_bteptr ) && ( pte.bteaddr[1] != NULL ) )
      {
         z_pen = _Kcbbpen(kcbptr1,(struct BTE *)NULL,bteptr2);
         pte.pen -= ( z_pen + bte2.pen );

         for ( z_i = 0;(z_i < 5)&&(pte.bteaddr[z_i]!=NULL); z_i++ )
            pte.bteaddr[z_i] = pte.bteaddr[z_i+1];
         pte.bteaddr[z_i] = NULL;
         if(gpw.moraof2p < (*pte.bteaddr[0]).endp)
            gpw.moraof2p = (*pte.bteaddr[0]).endp;
         pte.endp -= z_num;
      }

      /*---------   IF THE PATH DOESN'T HAS FIRMED BUNSETU   ----------*/
      else
      {
         for ( z_i = 0; z_i < 6; z_i++ )
         {
            if ( pte.bteaddr[z_i] == NULL )
               break;

            bteptr1 = pte.bteaddr[z_i];
            bte.usage--;
            if(bte.usage == 0)
            {
               if(bte.jteaddr != NULL)
                  (*bte.jteaddr).usage--;
               _Kccfree(&kcb.bthchh,bteptr1);
            }
         }

         pteptr2 = pteptr1;
         CMOVB (kcb.pthchh, pteptr1 );
         _Kccfree(&kcb.pthchh,pteptr2);
      }
   }

   gpw.pthpphin = bte2.hinr;
   jteptr1 = bte2.jteaddr;
   if(jteptr1 != NULL)
   {
      gpw.ppdflag[0] = jte.dflag[0];
      gpw.ppdflag[1] = jte.dflag[1];
      jtxptr1 = kcb.jtxjtx + jte.hinpos;
      gpw.ppsahen  = (jtx.pos[1] & 0x08) >> 3;
   }
   else
   {
      gpw.ppdflag[0] = 0;
      gpw.ppdflag[1] = 0;
      gpw.ppsahen  = 0;
   }

   if ( bte2.dmyflg == ON )
      return;

/*----------------------------------------------------------------------*
 *      FREE BTE
 *----------------------------------------------------------------------*/
   /*---------------------   CHECK ALL ELEMENTS   ----------------------*/
   FOR_FWD ( kcb.bthchh, bteptr1, z_endflg )
   {
      LST_FWD ( bteptr1, z_endflg );

      /*--------   IF START POS IS RIGHT OF FIRMED MORA POS    ---------*/
      if ( bte.stap > bte2.endp  )
      {
         bte.stap -= z_num;
         bte.endp -= z_num;

         if ( bte.kjf1 >= z_num )
            bte.kjf1 -= z_num;
         
         if ( bte.kjf2 >= z_num )
            bte.kjf2 -= z_num;
      }
      /*-------   IF START POS ISN'T RIGHT OF FIRMED MORA POS   --------*/
      else
      {
         z_tmpptr = (uschar *)bteptr1;
         CMOVB ( kcb.bthchh,bteptr1 );
         _Kccfree(&kcb.bthchh,z_tmpptr);
      }
   }
          
/*----------------------------------------------------------------------*
 *      FREE JTE
 *----------------------------------------------------------------------*/
   /*---------------------   CHECK ALL ELEMENTS   ----------------------*/
   FOR_FWD ( kcb.jthchh, jteptr1, z_endflg )
   {
      LST_FWD ( jteptr1, z_endflg );

      /*----------   IF JIRITU-GO IS LEFT OF FIRMED BUNSETU   ----------*/
      if ( jte.stap > bte2.endp )
      {
         jte.stap -= z_num;
      }
      /*--------   IF JIRITU-GO ISN'T LEFT OF FIRMED BUNSETU   ---------*/
      else
      {
         if ( jte.jkjaddr != 0 )
         {
            jkjptr1 = jte.jkjaddr;

            FOR_FWD_MID ( kcb.jkhchh, jkjptr1, z_endflg2 )
            {
               if ( (jkj.kj[0] & 0x80) == 0x00 )
                  z_endflg2 = ON;

               jkjptr2 = jkjptr1;
               CMOVB ( kcb.jkhchh,jkjptr1 );
               _Kccfree( &kcb.jkhchh, jkjptr2 );
            }
         }

         jteptr2 = jteptr1;
         CMOVB ( kcb.jthchh,jteptr1 );
         _Kccfree(&kcb.jthchh,jteptr2);
      }
   }
          
/*----------------------------------------------------------------------*
 *      FREE JLE
 *----------------------------------------------------------------------*/
   /*---------------------   CHECK ALL ELEMENTS   ----------------------*/
   FOR_FWD ( kcb.jlhchh, jleptr1, z_endflg )
   {
      LST_FWD ( jleptr1, z_endflg );

      /*----------   IF JIRITU-GO IS LEFT OF FIRMED BUNSETU   ----------*/
      if ( jle.stap > bte2.endp ) {
         jle.stap -= z_num;
      }

      /*--------   IF JIRITU-GO ISN'T LEFT OF FIRMED BUNSETU   ---------*/
      else
      {
         if ( jle.jkjaddr != 0 )
         {
            jkjptr1 = jle.jkjaddr;

            FOR_FWD_MID ( kcb.jkhchh, jkjptr1, z_endflg2 )
            {
               if ( (jkj.kj[0] & 0x80) == 0x00 )
                  z_endflg2 = ON;

               jkjptr2 = jkjptr1;
               CMOVB ( kcb.jkhchh,jkjptr1 );
               _Kccfree( &kcb.jkhchh, jkjptr2 );
            }
         }

         jleptr2 = jleptr1;
         CMOVB ( kcb.jlhchh,jleptr1 );
         _Kccfree(&kcb.jlhchh,jleptr2);
      }
   }

/*----------------------------------------------------------------------*
 *      SHIFT MCE
 *----------------------------------------------------------------------*/
   mceptr1 = kcb.mchmce + bte2.endp + 1;

   for ( z_i = 0; z_i < (kcb.mchacmce - z_num); z_i++ )
   {
      *(kcb.mchmce+z_i) = *(mceptr1+z_i);
      mceptr2 = kcb.mchmce+z_i ;
      mce2.yceaddr -= z_ynum;
   }

   for (; z_i < kcb.mchacmce; z_i++)
   {
      mceptr2 = kcb.mchmce + z_i ;
      mce2.code = 0 ;
      mce2.jdok = 0 ;
      mce2.yceaddr = NULL ;
      mce2.maxln   = 0 ;
   } ;

   kcb.mchacmce -= z_num;

   gpwptr1 = kcb.gpwgpe;
   gpw.leftflg = OFF;                   /* reset left word              */
   gpw.pendpos -= z_num;

/*----------------------------------------------------------------------*
 *      SHIFT YCE & YPE
 *----------------------------------------------------------------------*/
   yceptr1 = kcb.ychyce + z_ynum;
   ypeptr1 = kcb.yphype + gpw.accfirm + z_ynum;

   for ( z_i = 0; z_i < (kcb.ychacyce - z_ynum); z_i++ )
   {
      *(kcb.ychyce+z_i) = *(yceptr1+z_i);
      if ( gpw.pkakutei == PKAKPRT)     /* if partial kakutei           */
         *(kcb.yphype+gpw.accfirm+z_i) = *(ypeptr1+z_i);
   }

   kcb.ychacyce -= z_ynum;
   if ( gpw.pkakutei == PKAKPRT)
      kcb.yphacype -= z_ynum;

   gpw.accfirm += z_ynum;
}
