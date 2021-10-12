static char sccsid[] = "@(#)15	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcbnewp.c, libKJI, bos411, 9428A410j 6/4/91 10:12:40";
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
 * MODULE NAME:       _Kcbnewp
 *
 * DESCRIPTIVE NAME:  CREATE NEW PATH
 *
 * FUNCTION:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS)    : Success
 *                    0x1804(PTEOVER)    : PTE overflow
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
short _Kcbnewp(z_kcbptr,z_bteptr,z_minp)

struct KCB   *z_kcbptr;                 /* pointer of KCB               */
struct BTE   *z_bteptr;                 /* pointer of BTE               */
struct {
       short        pen;
       struct PTE   *pteptr;
       } z_minp[];
{
/*----------------------------------------------------------------------*  
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/ 
   extern struct RETCGET  _Kccget();    /* Get  Any Kinds of Table Entry*/
   extern struct RETCSWPP _Kccswpp();   /* Deleta Disused PTE           */
   extern short           _Kcbbpen();   /* Culculate Penalty BTE to BTE */
   extern void            _Kccfree();   /* Free Any Kinds of Table Entry*/

/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcjte.h"   /* Jiritsugo Table Entry (JTE)                  */
#include   "_Kcbte.h"   /* Bunsetsu Table Entry (BTE)                   */
#include   "_Kcpte.h"   /* Path Table Entry (PTE)                       */
#include   "_Kcyce.h"   /* Yomi Code table Entry (YCE)                  */
#include   "_Kcmce.h"   /* Mora Code table Entry (MCE)                  */
#include   "_Kcgpw.h"   /* General Purpose Workarea (GPW)               */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   struct RETCGET  z_rcget ;    /* Define Area for Return of _Kccget    */
   struct RETCSWPP z_rcswpp;    /* Define Area for Return of _Kccswpp   */
   short           z_i;                 /* counter                      */
   short           z_pen;               /* penalty of connection        */
   short           z_stap;              /* start pos of yomi            */
   short           z_endp;              /* end pos of yomi              */
   short           z_pos;               /* hinshi                       */
   short           z_act;               /* active flag                  */

/*----------------------------------------------------------------------*
 *      START PROCESS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set base pointer of KCB      */
   gpwptr1 = kcb.gpwgpe;                /* set base pointer of GPW      */
   bteptr1 = z_bteptr;                  /* set base pointer of BTE      */

/*----------------------------------------------------------------------*
 *       RETURN IF INCONSISTENCY OCCURS ABOUT CUT POINT
 *----------------------------------------------------------------------*/
   if ( bte.stap == 0 )                 /* get start posision of the BTE*/
      z_stap = 0;

   else
   {
      mceptr1 = kcb.mchmce + bte.stap - 1;
      z_stap = ( mce.yceaddr + 1 ) - kcb.ychyce;
   }
   if(bte.endp >= 0)                    /* get end posision of the BTE  */
   {
      mceptr1 = kcb.mchmce + bte.endp;
      z_endp = mce.yceaddr - kcb.ychyce;
   }
   else
      z_endp = 0;

   if ( ( z_stap + 1 <= kcb.ymill2 ) && ( z_endp + 1 > kcb.ymill2 ) )
      return((short)SUCCESS);

   z_pen = _Kcbbpen(kcbptr1,(struct BTE *)NULL,bteptr1);
   z_pen += bte.pen;                    /* calculate penalty            */

   if ( ( bte.hinr & 0x0080 ) != 0 )
      z_pos = bte.hinr & 0x007f;
   else
      z_pos = 0;

   z_act = CSTTS( z_minp[z_pos].pteptr );

   if ( ( z_pen <= ( z_minp[z_pos].pen + 3 ) )
          || ( z_act != ON ) )
   {
      if ( ( z_minp[z_pos].pteptr != NULL ) 
        && ( z_act == ON )
        && ( ( z_minp[z_pos].pen - 3 ) > z_pen ) )
      {
         pteptr2 = z_minp[z_pos].pteptr;
         for ( z_i = 0; z_i < 6; z_i++ )
         {
            if ( pte2.bteaddr[z_i] == (struct BTE *)NULL )
               break;
            bteptr2 = pte2.bteaddr[z_i];
            bte2.usage--;
         }
         _Kccfree(&kcb.pthchh,z_minp[z_pos].pteptr);
      }

/*----------------------------------------------------------------------*
 *      GET PTE ENTRY
 *----------------------------------------------------------------------*/
      z_rcget = _Kccget(&kcb.pthchh);

      if ( z_rcget.rc == GET_EMPTY )
      {
         z_rcswpp = _Kccswpp(kcbptr1,NULL);
                                        /* Retry to get PTE             */
         z_rcget = _Kccget(&kcb.pthchh);
         if ( z_rcget.rc == GET_EMPTY )
            return( PTEOVER );
         else if ( ( z_rcget.rc != GET_TOP_MID ) 
                && ( z_rcget.rc != GET_LAST ) )
            return(z_rcget.rc);
      }
      else if ( ( z_rcget.rc != GET_TOP_MID ) 
             && ( z_rcget.rc != GET_LAST ) )
         return(z_rcget.rc);

/*----------------------------------------------------------------------*
 *       SET NEW PTE
 *----------------------------------------------------------------------*/
      pteptr1                           /* set base pointer of PTE      */
           = (struct PTE *)z_rcget.cheptr;

      if ( ( z_pen < z_minp[z_pos].pen ) || ( z_act != ON ) )
      {
         z_minp[z_pos].pteptr = pteptr1;
         z_minp[z_pos].pen = z_pen;
      }

      pte.bteaddr[0] = bteptr1;        /* set pointer of BTE in path   */
      for ( z_i = 1; z_i < 6; z_i++ )
         pte.bteaddr[z_i] = NULL;

      if(gpw.moraof2p < bte.endp)
         gpw.moraof2p = bte.endp;       /* save end pos if num of       */
                                        /* 1st bunsetu node is minimum  */
      pte.endp = bte.endp;              /* set end of path              */

/*----------------------------------------------------------------------*
 *      GET PENALTY OF CONNEXCTION
 *----------------------------------------------------------------------*/
      pte.pen = z_pen;                  /* calculate penalty            */
      bte.usage++;                      /* incliment usage counter      */
   }

/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/
   return ( SUCCESS );
}
