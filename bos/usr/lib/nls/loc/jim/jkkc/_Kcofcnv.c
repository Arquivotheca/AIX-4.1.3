static char sccsid[] = "@(#)62	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcofcnv.c, libKJI, bos411, 9428A410j 6/4/91 15:17:44";
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
 * MODULE NAME:       _Kcofcnv
 *
 * DESCRIPTIVE NAME:  SELSECT FIRST CANDIDATE
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS): success
 *                    0x0002 (NOMORE_CAND): No more candidate
 *                    0x0004 (NO_CAND): No candidate
 *                    0x0108 (WSPOVER): work space overflows
 *                    0x2104 (SEIOVER): seisho buffer overflow
 *                    0x2204 (SEMOVER): seisho map buffer overflow
 *                    0x2304 (YMMOVER): yomi map buffer overflow
 *                    0x2404 (GRMOVER): grammar map buffer overflow
 *                    0x7fff (UERROR) :  unpredictable error
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------* 
 *      INCLUDE FILES                                                    
 *----------------------------------------------------------------------*/
#if defined(_AGC) || defined(_GSW)
#include   "_Kcfnm.h"                   /* Define Names file            */
#endif
#include   "_Kcmap.h"                   /* Define Constant file         */


/************************************************************************
 *      START OF FUNCTION
 ************************************************************************/
short   _Kcofcnv( z_kcbptr )

struct  KCB     *z_kcbptr;              /* pointer of KCB               */
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern  short   _Kconcv2();

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short        z_rtcode;

/*----------------------------------------------------------------------*
 *      SELECT CANDIDATES
 *----------------------------------------------------------------------*/
   z_rtcode = _Kconcv2( z_kcbptr, (short)FRSTCNV );

/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/
   return( z_rtcode );
}
