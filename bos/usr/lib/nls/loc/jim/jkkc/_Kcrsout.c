static char sccsid[] = "@(#)85	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcrsout.c, libKJI, bos411, 9428A410j 6/4/91 15:22:30";
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
 * MODULE NAME:       _Kcrsout
 *
 * DESCRIPTIVE NAME:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS): success
 *                    0x2104 (SEIOVER): seisho buffer overflow
 *                    0x2204 (SEMOVER): seisho map buffer overflow
 *                    0x2304 (YMMOVER): yomi map buffer overflow
 *                    0x2404 (GRMOVER): grammar map buffer overflow
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


/************************************************************************
 *      START OF FUNCTION
 ************************************************************************/
short _Kcrsout(z_kcbptr,z_bteptr,z_seiptr,z_semptr,z_grmptr,z_ymmptr)

struct  KCB   *z_kcbptr;
struct  BTE   *z_bteptr;
struct  SEI   *z_seiptr;
struct  SEM   *z_semptr;
struct  GRM   *z_grmptr;
struct  YMM   *z_ymmptr;
{
/*----------------------------------------------------------------------* 
 *       EXTERNAL REFERENCE    
 *----------------------------------------------------------------------*/
   extern   short   _Kcrsjrt();
   extern   short   _Kcrsfzk();

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcbte.h"
#include   "_Kcsei.h"
#include   "_Kcsem.h"
#include   "_Kcgrm.h"
#include   "_Kcymm.h"

/*----------------------------------------------------------------------* 
 *       DEFINITION OF LOCAL VARIABLES    
 *----------------------------------------------------------------------*/
   short   z_rc;                        /* return code                  */

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;

/*----------------------------------------------------------------------* 
 *      SET JIRITSU-GO IN OUTPUT BUFFER
 *----------------------------------------------------------------------*/
   z_rc = _Kcrsjrt( z_kcbptr, z_bteptr,
                                 z_seiptr, z_semptr, z_grmptr, z_ymmptr );
   if ( z_rc != SUCCESS )
      return(z_rc);

   /*-------------------------------------------------------------------*
    *   CHECK ENVIRONMENT
    *-------------------------------------------------------------------*/
     if ( ( kcb.env == ENVTAN )
       || ( kcb.env == ENVALPH )
       || ( kcb.env == ENVALKN )
       || ( kcb.env == ENVKANSU ) )
        return((short)SUCCESS);

/*----------------------------------------------------------------------* 
 *      SET FUZOKU-GO IN OUTPUT BUFFER
 *----------------------------------------------------------------------*/
   z_rc = _Kcrsfzk( z_kcbptr, z_bteptr,
                                 z_seiptr, z_semptr, z_grmptr, z_ymmptr );
   return( z_rc );
}
