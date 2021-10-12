static char sccsid[] = "@(#)73	1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kchkex.c, libKJI, bos411, 9428A410j 7/23/92 00:11:54";
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
 * MODULE NAME:       _Kchkex
 *
 * DESCRIPTIVE NAME:  CHECK OF CANDIDATE IN SYSTEM DICTIONARY'S INFORMATION.
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x01 (ON) : EXIST OF CANDIDATE.  
 * 		      0x00 (OFF): NO EXIST 
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
short _Kchkex(z_kcbptr,z_stpos,z_length,z_file)
struct KCB    *z_kcbptr;                /* pointer of KCB               */
uschar 	      z_stpos;                  /* offset of 1st MCE            */
uschar        z_length;                 /* length of string(mora code)  */
short	      z_file;			/* System File Number		*/
{
/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcmce.h"
#include   "_Kcmcb.h"                   /* Monitor control block (MCB)  */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES 
 *----------------------------------------------------------------------*/
	short	 z_ix, z_mx, z_ret;
	usshort	 z_short_mora;
	uschar	 z_mora0;
/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
        kcbptr1 = z_kcbptr;             /* set pointer of KCB           */
	mcbptr1 = ( struct MCB *)kcb.myarea; /* Get MCB pointer         */
        mceptr1 = kcb.mchmce + z_stpos;

	if( z_length == 1 ) {
	    if(((*mceptr1).code >= mcb.mulsys[z_file].dict_mono_lkey ) &&
	       ((*mceptr1).code <= mcb.mulsys[z_file].dict_mono_hkey )) {
		z_ix = (*mceptr1).code / 8;
		z_mx = (*mceptr1).code % 8;
		if(mcb.mulsys[z_file].dict_mono_ex[z_ix]&(1<<(7-z_mx)))
		    z_ret = ON;
		else
		    z_ret = OFF;
	    }
	    else
		    z_ret = OFF;
	}
	else {
	    z_mora0 = (*mceptr1++).code;
	    z_short_mora = ((z_mora0<<8)&0xFF00) + ((*mceptr1).code&0xFF);
	    if(( z_short_mora >= (mcb.mulsys[z_file].dict_poly_lkey&0xFFFF )) &&
	       ( z_short_mora <= (mcb.mulsys[z_file].dict_poly_hkey&0xFFFF ))) {
		z_ix = z_mora0 / 8;
		z_mx = z_mora0 % 8;
		if(mcb.mulsys[z_file].dict_poly_ex[z_ix]&(1<<(7-z_mx)))
		    z_ret = ON;
		else
		    z_ret = OFF;
	    }
	    else
		    z_ret = OFF;
	}
	return( z_ret );
}
