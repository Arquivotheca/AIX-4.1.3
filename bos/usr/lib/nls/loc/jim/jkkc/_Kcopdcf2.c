static char sccsid[] = "@(#)76	1.2  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcopdcf2.c, libKJI, bos411, 9428A410j 8/19/92 20:35:11";
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
 * MODULE NAME:       _Kcopdcf2
 *
 * DESCRIPTIVE NAME:  SET FUZOKUGO GAKUSYU FILE POINTER TO MCB
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS):  success
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------* 
 *      INCLUDE FILES                                                    
 *----------------------------------------------------------------------*/
#if defined(_AGC) || defined(_GSW)
#include   "_Kcfnm.h"                   /* Define Names file            */
#endif
#include   "_Kcmap.h"                   /* Define Constant file         */
#include   "_Kcmcb.h"                   /* Monitor Control Block (MCB)  */


#include   "dict.h"                     /* dictionary information       */
/************************************************************************
 *      SET THE CURRENT FDICTINFO
 ************************************************************************/
static          FDICTINFO       fdictp;
void            *SetCurrentFDICTINFO(FDICTINFO fp)  { fdictp = fp; }
FDICTINFO       GetCurrentFDICTINFO()               { return(fdictp); }


/************************************************************************
 *      START OF FUNCTION
 ************************************************************************/
short   _Kcopdcf2( z_mcbptr, z_fzknm )

struct  MCB     *z_mcbptr;              /* pointer of MCB               */
char            *z_fzknm;
{
/*----------------------------------------------------------------------*
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   char    *strcpy();                   /* define strcpy()              */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short        z_length;               /* set length of path name      */

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   mcbptr1 = z_mcbptr;                  /* establish address'th to mcb  */

/*----------------------------------------------------------------------*
 *      SET INFORMATION IF FZOKU-GO DICTIONARY
 *----------------------------------------------------------------------*/
   mcb.dfznm[ 0 ] = 0x00;               /* reset the path name area     */

   if ( ( z_length = strlen( fdictp->fdictname ) ) >= ENVLIM )
   {
      strncpy( mcb.dfznm , fdictp->fdictname , ENVLIM-1 );
      mcb.dfznm[ENVLIM-1] = '\0';
      mcb.dfznmll = ENVLIM - 1;         /* set length of path name      */
   }
   else
   {
      strcpy( mcb.dfznm , fdictp->fdictname );    /* set path name                */
      mcb.dfznmll = z_length;           /* set length of path name      */
   }

/*----------------------------------------------------------------------*
 *      ALLOCATE MEMORY FOR "DICTIONARY BUFFER"
 *----------------------------------------------------------------------*/
   mcb.dfzfd = fdictp->dfzfd;
   mcb.dfgdfg = fdictp->dfgdfg;
   mcb.kcbaddr->dfgdfg = fdictp->dfgdfg;

/*----------------------------------------------------------------------*
 *      RETUEN
 *----------------------------------------------------------------------*/
   return( SUCCESS );
}
