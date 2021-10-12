static char sccsid[] = "@(#)74	1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcjscnt.c, libKJI, bos411, 9428A410j 7/23/92 00:14:59";
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
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kcjscnt
 *
 * DESCRIPTIVE NAME:  CHECK NEXT BLOCK IN SYSTEM DICTIONARY.
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       NEXT_CONT : Read Next Block.
 *		      NEXT_END  : Not Read Next Block.
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------*
 *       COPYRIGHT
 *----------------------------------------------------------------------*/
#ifdef _AIX
static char *cprt1="5601-061 COPYRIGHT IBM CORP 1988,1989      ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";
#endif

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
short _Kcjscnt(z_kcbptr,z_strpos,z_length,z_mode,z_next,z_file)
struct KCB  *z_kcbptr;                  /* pointer of KCB               */
uschar       z_strpos;                  /* pointer of 1st MCE           */
uschar       z_length;                  /* length of string(mora code)  */
short        z_mode;                    /* flag of the way for looking  */
                                        /*  up words                    */
uschar       z_next[NEXT_CONT_MAX];     /* Next record check            */
short	     z_file;			/* System File Number		*/
{
/*----------------------------------------------------------------------*
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcmcb.h"   /* Kkc Control Block (MCB)                      */
#include   "_Kcmce.h"   /* Mora Code table Entry (MCE)                  */
#include   "_Kcsd1.h"   /* System dictionary Data (SD1)                 */

/*----------------------------------------------------------------------*
*       LOCAL VARIABLES                                                 *
*-----------------------------------------------------------------------*/
   short	z_ret;                  /* Return val                   */

/*----------------------------------------------------------------------*
 *      START OF PROCESS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set pointer of KCB           */
   mcbptr1 = (struct MCB *)kcb.myarea;
   mceptr1 = kcb.mchmce + (short)z_strpos;
   sdcptr1 = (struct SDC *)( kcb.sdesde + mcb.mulsys[z_file].record_size + 2);

   sdmptr1 = (struct SDM *)((uschar *)sdcptr1 + 
	     (uschar)_get_offset( sdc.count ));

   if(( z_next[0] == sdm.code[0] ) && ( z_next[1] == sdm.code[1] ))
       z_ret = NEXT_CONT;
   else
       z_ret = NEXT_END;
   return( z_ret );
}
