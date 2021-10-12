static char sccsid[] = "@(#)03	1.2.1.2  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcjshsh.c, libKJI, bos411, 9428A410j 3/14/94 01:28:04";
/*
 * COMPONENT_NAME: (libKJI) Japanese Input Method (JIM)
 * 
 * FUNCTIONS: Kana-Kanji-Conversion (KKC) Library
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kcjshsh
 *
 * DESCRIPTIVE NAME:  RRN (Relative Record Number) comparing each hush
 *		      data with MCE.
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       rrn (relative record number)
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
short   _Kcjshsh(z_kcbptr,z_strpos,z_length,z_next,z_file)

struct KCB      *z_kcbptr;              /* pointer of KCB               */
unsigned char   z_strpos;               /* offset of 1st MCE            */
unsigned char   z_length;               /* length of MCE(yomi data)     */
unsigned char   z_next[NEXT_CONT_MAX];  /* Next record check            */
short		z_file;			/* System File Number		*/
{
/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcmce.h"
#include   "_Kcsxe.h"
#include   "_Kcmcb.h"			/* Monitor control block (MCB)  */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short             z_i, z_j;          /* counter                      */
   short	     z_record;		/* Mora Record Number		*/
   short	     z_mora_all;	/* mora buffer length counter	*/
   short	     z_mora_byte;	/* mora byte count           	*/
   unsigned char     z_mora[MAX_MORA_INDEX]; 
   unsigned char     *z_dict;		/* Dict mora code		*/

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set pointer of KCB           */
   mcbptr1 = ( struct MCB *)kcb.myarea; /* Get MCB pointer		*/
   mceptr1 = kcb.mchmce + z_strpos;

   if( z_length == 1 ) {
       z_next[0]=0; z_next[1]=0; z_next[NEXT_CONT_CHECK]=NEXT_END;
       return( mcb.mulsys[z_file].mono_sr );
   }

   z_mora_byte = mcb.mulsys[z_file].poly_ml * 2; 
   for( z_i = 0; z_i < z_mora_byte ; z_i++ ) 
       z_mora[z_i] = (*mceptr1++).code;

   z_next[0]=z_mora[0]; z_next[1]=z_mora[1]; z_next[NEXT_CONT_CHECK]=0;

/*----------------------------------------------------------------------*
 *       SERACH INDEX TO RRN (Rerative Recoed Number)  
 *----------------------------------------------------------------------*/
   z_mora_all = ( mcb.mulsys[z_file].poly_l / 
		( mcb.mulsys[z_file].poly_ml * 2 )) + 1 ;
   z_record  = mcb.mulsys[z_file].poly_sr;
   for ( z_i = z_record; z_i <= z_mora_all; z_i++ ) {
      if( z_i == z_record )
	  z_dict = (uschar *)&kcb.sxesxe[0]; 
      else 
	  z_dict += (uschar)z_mora_byte;

      /*---------------   compare mora code with index   ---------------*/
      for( z_j = 0; z_j < z_mora_byte; z_j++ ) {
          if( *( z_dict + z_j ) >= z_mora[z_j] ) {
	      if( *( z_dict + z_j ) == z_mora[z_j] ) {
	          if( *( z_dict + z_j + 1 ) >= z_mora[z_j+1] ) {
	      	      return( z_i );
		  }
	  	  else break;
	      }
	      else return( z_i );
	  }
	  else break;
      }
   }
   return( UERROR );
}
