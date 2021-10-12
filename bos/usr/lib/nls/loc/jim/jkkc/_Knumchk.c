static char sccsid[] = "@(#)80	1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Knumchk.c, libKJI, bos411, 9428A410j 7/23/92 00:31:08";
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
 * MODULE NAME:       _Knumchk
 *
 * DESCRIPTIVE NAME:  CHECH OF KANSUJI
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x01 (ON)  :  include numeric data.
 *		      0x00 (OFF) :  not include numeric data.
 *
 ******************** END OF SPECIFICATIONS *****************************/
                                                                          
/*----------------------------------------------------------------------*  
 *      INCLUDE FILES                                                     
 *----------------------------------------------------------------------*/ 
#if defined(_AGC) || defined(_GSW)
#include   "_Kcfnm.h"                   /* Define Names file            */
#endif

/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kckcb.h"   		/* Kkc Control Block (KCB)      */
#include   "_Kcmap.h"                   /* Define Constant file         */
#include   "_Kcrcb.h"                   /* Define Return Code Structure */
#include   "_Kcmce.h"   		/* Mora Code table Entry (MCE)  */

static	char	z_codebuf[3][2] = {
	{ NUM_MORA_0, NUM_MORA_9 },	/* Mora Code 0-9		*/
	{ NUM_YOMI_0, NUM_YOMI_9 },	/* Yomi Code 0-9		*/
	{ NUM_CHAR_0, NUM_CHAR_9 },	/* Char Code 0-9		*/
};

static	char	z_codebuf2[2][4] = {
    M_E_ALPHA_a,M_E_ALPHA_z,M_E_ALPHA_A,M_E_ALPHA_Z    /* Mora Code 0-9	*/
};

/************************************************************************
 *      START OF FUNCTION                                                 
 ************************************************************************/ 
_Knumchk( z_kcbptr, z_strpos, z_length, z_type  )
struct KCB      *z_kcbptr;
uschar           z_strpos;              /* Pointer of 1st MCE           */
uschar           z_length;              /* length of string(mora code)  */
uschar		 z_type;		/* Code Type Mode		*/
{
/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   	short    z_i, z_ret, z_flag, z_code, z_mode;

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
	kcbptr1 = z_kcbptr;             /* initialize of kcbptr1        */
	mceptr1 = kcb.mchmce + (short)z_strpos;

	if( z_type & NUM_ONLY ) z_mode = NUM_ONLY;
	else 			z_mode = NUM_INCL;
	z_code = z_type & 0x0F;
	if(( mce.code < z_codebuf[z_code][0] ) || 
	   ( mce.code > z_codebuf[z_code][1] ))
	    return( OFF );
	for( z_i = 0, z_flag = 0, z_ret = 0; z_i < z_length; z_i++ ) {
	    mceptr1 = kcb.mchmce + (short)z_strpos + z_i;
	    if(( mce.code >= z_codebuf[z_code][0] ) && 
	       ( mce.code <= z_codebuf[z_code][1] ))
		    z_flag |= NUM_DATA;
	    else
		    z_flag |= NUM_ELSE;
	}
	if( z_mode == NUM_ONLY ) {
	    if( z_flag == NUM_DATA ) z_ret = ON;
	    else 		     z_ret = OFF;
	}
	else {
	    if( z_flag & NUM_DATA )  z_ret = ON;
	    else 		     z_ret = OFF;
	}
	return( z_ret );
}

/************************************************************************
 *      START OF FUNCTION                                                 
 ************************************************************************/ 
_Kalpchk( z_kcbptr, z_strpos, z_length, z_type  )
struct KCB      *z_kcbptr;
uschar           z_strpos;              /* Pointer of 1st MCE           */
uschar           z_length;              /* length of string(mora code)  */
uschar		 z_type;		/* Code Type Mode		*/
{
/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   	short    z_i, z_ret, z_flag, z_code, z_mode;

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
	kcbptr1 = z_kcbptr;             /* initialize of kcbptr1        */
	mceptr1 = kcb.mchmce + (short)z_strpos;

	if(( ext_sys_conv ) || ( ext_usr_conv )) return( OFF );
	if( z_type & ALP_ONLY ) z_mode = ALP_ONLY;
	else 			z_mode = ALP_INCL;
	z_code = z_type & 0x0F;
	if(((mce.code >= z_codebuf2[z_code][0]) && (mce.code <= z_codebuf2[z_code][1])) || 
	   ((mce.code >= z_codebuf2[z_code][2]) && (mce.code <= z_codebuf2[z_code][3])))
	   ;
	else
	    return( OFF );
	for( z_i = 0, z_flag = 0, z_ret = 0; z_i < z_length; z_i++ ) {
	    mceptr1 = kcb.mchmce + (short)z_strpos + z_i;
	    if(((mce.code >= z_codebuf2[z_code][0]) && (mce.code <= z_codebuf2[z_code][1])) ||
	       ((mce.code >= z_codebuf2[z_code][2]) && (mce.code <= z_codebuf2[z_code][3])))
		    z_flag |= ALP_DATA;
	    else
		    z_flag |= ALP_ELSE;
	}
	if( z_mode == ALP_ONLY ) {
	    if( z_flag == ALP_DATA ) z_ret = ON;
	    else 		     z_ret = OFF;
	}
	else {
	    if( z_flag & ALP_DATA )  z_ret = ON;
	    else 		     z_ret = OFF;
	}
	return( z_ret );
}
