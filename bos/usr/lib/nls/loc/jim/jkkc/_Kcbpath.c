static char sccsid[] = "@(#)16	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcbpath.c, libKJI, bos411, 9428A410j 6/4/91 10:13:08";
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
 * MODULE NAME:       _Kcbpath
 *
 * DESCRIPTIVE NAME:  AFTER CREATING BUNSETSU,CREATE NEW PATH OR ADD PATH
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
struct RETBPATH _Kcbpath(z_kcbptr,z_bteptr,z_bendp,z_minpen)
struct  KCB *z_kcbptr;                  /* get KCB base address         */
struct  BTE *z_bteptr;                  /* get BTE address from caller  */
short        z_bendp;                   /* set end position of bunsetsu */
struct {
          short        pen;
          struct PTE   *pteptr;
       }   z_minpen[];          /* buffer of minimum penalties  */

{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern short           _Kcbnewp();   /* Create New PTE for New BTE   */
   extern short           _Kcbaddp();   /* Add New BTE to Path          */
   extern short           _Kcbcutp();   /* Cut Large Penalty path       */

/*----------------------------------------------------------------------*
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcbte.h"   /* Bunsetsu Table Entry (BTE)                   */
#include   "_Kcjte.h"   /* Jiritsugo Table Entry (JTE)                  */
#include   "_Kcgpw.h"   /* General Purpose Workarea (GPW)               */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short           z_rbnewp;    /* Define Area for Return of _Kcbnewp   */
   short           z_rbaddp;    /* Define Area for Return of _Kcbaddp   */
   short           z_rbcutp;    /* Define Area for Return of _Kcbcutp   */
   struct RETBPATH z_ret;       /* Define Area for Return of Own        */

/*----------------------------------------------------------------------*
 *      CREATE NEW PATH OR ADD PATH
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set base address of KCB      */
   bteptr1 = z_bteptr;                  /* set base address of BTE      */
   jteptr1 = bte.jteaddr;               /* set base address of JTE      */
   gpwptr1 = kcb.gpwgpe;                /* set base addrees of GPW      */

   if (((gpw.leftflg== OFF) && (bte.stap == 0)) ||
                                       /* if left char was not specified*/
                                       /* and start position is top of  */
                                       /* mora                          */
       ((gpw.leftflg == ON) && (bte.stap == 0) && ((jteptr1 == NULL) ||
                                                   (jte.len == 0))))
                                       /* if left char was specified    */
                                       /* and dummy it's dummy bunsetsu */
   {
      z_rbnewp = _Kcbnewp(z_kcbptr,z_bteptr,z_minpen);
                                        /* create new path              */
      if ( z_rbnewp != SUCCESS )
      {
         z_ret.rc = z_rbnewp;
         return(z_ret);
      }
   }
   else
   {
      if (bte.endp == z_bendp)          /*check if it's correct bunsetsu*/
      {
         z_rbaddp = _Kcbaddp(z_kcbptr,z_bteptr,z_minpen);
                                        /* add the bunsetsu in path entry*/
         if ( z_rbaddp != SUCCESS )
         {
            z_ret.rc = z_rbaddp;
            return(z_ret);
         }
      }                                 /*  end if                      */
   }
   if (z_bendp >= 0)
   {
      z_rbcutp = _Kcbcutp(z_kcbptr,z_bendp);
                                        /* cut unusable path            */
      if ( z_rbcutp != SUCCESS )
      {
         z_ret.rc = z_rbcutp;
         return(z_ret);
      }
   }
   z_ret.rc = SUCCESS;
   return(z_ret);
}
