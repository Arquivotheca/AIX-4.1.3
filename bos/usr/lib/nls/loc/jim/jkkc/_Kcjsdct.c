static char sccsid[] = "@(#)00	1.2.1.3  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcjsdct.c, libKJI, bos411, 9428A410j 5/28/93 07:57:50";
/*
 * COMPONENT_NAME: (libKJI) Japanese Input Method (JIM)
 *
 * FUNCTIONS: Kana-Kanji-Conversion (KKC) Library
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kcjsdct
 *
 * DESCRIPTIVE NAME:  LOOKING UP ON SYSTEM DICTIONARY.
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS): success
 *                    0x1104 (JTEOVER): JTE table overflow
 *                    0x1204 (JKJOVER): JKJ table overflow
 *                    0x1304 (JLEOVER): JLE table overflow
 *                    0x7fff(UERROR):     unpredictable error
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
short _Kcjsdct(z_kcbptr,z_strpos,z_length,z_flag,z_file)

struct KCB    *z_kcbptr;                /* pointer of KCB               */
unsigned char z_strpos;                 /* offset of 1st MCE            */
unsigned char z_length;                 /* length of string(mora code)  */
short         z_flag;                   /* flag for the way of looking  */
                                        /*  up the words                */
short	      z_file;			/* System File Number		*/
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern short             _Kcjshsh();
   extern short             _Kcjsunq();
   extern short             _Kczread();

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcmce.h"

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define   Z_SDCT_MAX   320              /* NOT FIXED                    */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short        z_rc;                   /* return code save area        */
   short        z_ret;                  /* rerative record number       */
   short	z_i;		  
   uschar     	z_next[NEXT_CONT_MAX];  /* Next Record check		*/

/*----------------------------------------------------------------------*
 *       READ THE RECORD CORRESPONDED TO RRN FROM TARGET FILE
 *----------------------------------------------------------------------*/
   z_rc = _Kczread( z_kcbptr, SX, (short)0 , EXCLUSIVE, z_file );
   if ( z_rc != SUCCESS )
      return( z_rc );

/*----------------------------------------------------------------------*
 *       SERACH INDEX TO GET RRN (Rerative Record Number) 
 *----------------------------------------------------------------------*/
   z_ret = _Kcjshsh( z_kcbptr, z_strpos, z_length, z_next, z_file );
   if ( z_ret == UERROR )
      return( SUCCESS );

 for( z_i = 0; z_i < 8 ; z_i++ ) {
    /*------------------------------------------------------------------*
     *       READ THE RECORD CORRESPONDED TO RRN FROM TARGET FILE
     *------------------------------------------------------------------*/
    z_rc = _Kczread( z_kcbptr, SD, z_ret + z_i , EXCLUSIVE, z_file );

    if ( z_rc != SUCCESS )
      return( z_rc );

    /*------------------------------------------------------------------*
     *       SERACH BUFFER TO GET JIRITSU-GO
     *------------------------------------------------------------------*/
    z_rc = _Kcjsunq( z_kcbptr, z_strpos, z_length, z_flag, z_next, z_file );

    if( z_rc != SUCCESS )
            return( z_rc );

    if(( z_length == 1 ) || 
       ( _Kcjscnt(z_kcbptr,z_strpos,z_length,z_flag,z_next,z_file)) == NEXT_END ) 
          break;
    else 
          z_next[NEXT_CONT_CHECK] = NEXT_CONT;
  }
/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/
   return( SUCCESS );
}
