static char sccsid[] = "@(#)75	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcfstmp.c, libKJI, bos411, 9428A410j 6/4/91 12:49:51";
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
 * MODULE NAME:       _Kcfstmp
 *
 * DESCRIPTIVE NAME:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS): success
 *                    0x1504 (FWEOVER): FWE table overflow
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
struct RETFSTMP _Kcfstmp(z_kcbptr,z_fweptr,z_morpos,z_fno,z_hykptr,z_pen)

struct KCB *z_kcbptr;
struct FWE *z_fweptr;
short      z_morpos;
short      z_fno;
short      z_hykptr;
short      z_pen;
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern struct RETCGET _Kccget();

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcfwe.h"

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   struct RETFSTMP z_ret;
   struct RETCGET z_ret1;

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
  kcbptr1 = z_kcbptr;                   /* establish address'ty to kcb  */

/*----------------------------------------------------------------------*
 *      INITIALIZE
 *----------------------------------------------------------------------*/
   z_ret.rc = 0;

   z_ret1 = _Kccget(&kcb.fwhchh);       /* obtain a new FWE entry       */
   if ( z_ret1.rc == GET_EMPTY )
   {
      z_ret.rc = FWEOVER;
      return(z_ret);
   }
   else if ( ( z_ret1.rc != GET_TOP_MID ) && ( z_ret1.rc != GET_LAST ) )
   {
      z_ret.rc = UERROR;
      return(z_ret);
   }

  fweptr1 = (struct FWE *)z_ret1.cheptr;
                                        /* establish address'ty to FTE  */
  fwe.prnt = z_fweptr;                  /* move parent FWE address      */
  fwe.stap = z_morpos;                  /* move mora pos to FWE         */
  fwe.fno  = z_fno;                     /* move fuzokogo no to FWE      */
  fwe.kjn  = z_hykptr;                  /* move FJK pointer to FWE      */
  fwe.pen  = z_pen;                     /* move penalty  to FWE         */

  z_ret.fweptr = fweptr1;
  z_ret.rc = SUCCESS;
  return(z_ret);
}
