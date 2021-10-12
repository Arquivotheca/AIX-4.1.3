static char sccsid[] = "@(#)77	1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcopdcu2.c, libKJI, bos411, 9428A410j 7/23/92 00:22:53";
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
 * MODULE NAME:       _Kcopdcu2
 *
 * DESCRIPTIVE NAME:  SET USER DICTIONARY FILE POINTER TO MCB
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS):    success
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
#include   "_Kcuxe.h"                   /* User dictionary index        */
#include   "dict.h"                     /* dictionary information       */

/************************************************************************
 *      SET THE CURRENT UDICTINFO
 ************************************************************************/
static          UDICTINFO       udictp;
void            *SetCurrentUDICTINFO(UDICTINFO up)  { udictp = up; }
UDICTINFO       GetCurrentUDICTINFO()               { return(udictp); }


/************************************************************************
 *      START OF FUNCTION
 ************************************************************************/
short   _Kcopdcu2( z_mcbptr, z_usrnm )

struct  MCB     *z_mcbptr;              /* pointer of MCB               */
char            *z_usrnm;
{

/*----------------------------------------------------------------------* 
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   char         *strcpy();              /* define strcpy()              */

/*----------------------------------------------------------------------* 
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short        z_length;               /* set length of path name      */

/*----------------------------------------------------------------------* 
 *      SET BASE POINTER
 *----------------------------------------------------------------------*/
   mcbptr1 = z_mcbptr;                  /* establish address'th to mcb  */

/*----------------------------------------------------------------------* 
 *      SET INFORMATION OF USER DICTIONARY
 *----------------------------------------------------------------------*/
   mcb.dusnm[ 0 ] = 0x00;               /* reset the path name area     */

   if ( ( z_length = strlen( udictp->udictname ) ) >= ENVLIM )
   {
      strncpy( mcb.dusnm , udictp->udictname , ( ENVLIM - 1 ) );
      mcb.dusnm[ENVLIM-1] = '\0';
      mcb.dusnmll = ENVLIM - 1;         /* set length of path name      */
   }
   else
   {
                                        /* set path name                */
      strcpy( mcb.dusnm , udictp->udictname );
      mcb.dusnmll = z_length;           /* set length of path name      */
   }

/*----------------------------------------------------------------------*
 *      ALLOCATE MEMORY FOR "DICTIONARY BUFFER"
 *----------------------------------------------------------------------*/
  RefreshMcbUdict( &mcb, udictp );

/*----------------------------------------------------------------------* 
 *      RETURN
 *----------------------------------------------------------------------*/
   return( SUCCESS );
}

