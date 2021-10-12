static char sccsid[] = "@(#)81	1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Knumset.c, libKJI, bos411, 9428A410j 7/23/92 00:34:27";
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
 * MODULE NAME:       _Knumset
 *
 * DESCRIPTIVE NAME:  MAKE TABLE OF CANDIDATES FOR KANSUJI CONVERSION
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS):    success
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
#include   "_Kckcb.h"   		/* Kkc Control Block (KCB)      */
#include   "_Kcmap.h"                   /* Define Constant file         */
#include   "_Kcrcb.h"                   /* Define Return Code Structure */
#include   "_Kcmce.h"   		/* Mora Code table Entry (MCE)  */
#include   "_Kcchh.h"   		/* CHain Header map (CHH)       */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/ 
#define   Z_END        0x01ff
#define   Z_CONTINUE   0x02ff
#define   Z_HIN_DUMMY  127


/************************************************************************
 *      START OF FUNCTION                                                 
 ************************************************************************/ 
short  _Knumset( z_kcbptr, z_strpos, z_moralen, z_setlen )
struct KCB      *z_kcbptr;
uschar           z_strpos;              /* Pointer of 1st MCE           */
uschar           z_moralen;             /* length of string(mora code)  */
uschar           z_setlen;              /* length of string(mora code)  */
{
/*----------------------------------------------------------------------*  
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/ 
   extern   short   _Kckcanr();
   extern   short   _Kckcan1();
   extern   short   _Kckcan2();
   extern   short   _Kckcan3();

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
	short	 z_rc;
	uschar	 z_numlen;

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
	kcbptr1 = z_kcbptr;             /* initialize of kcbptr1        */

	for( z_numlen = 0; z_numlen < z_moralen; z_numlen++ ) {
	    mceptr1 = kcb.mchmce + (short)z_strpos + (short)z_numlen;
	    if(( mce.code >= NUM_MORA_0 ) && ( mce.code <= NUM_MORA_9 ))
		continue;
	    else
		break;
	}
	if( z_numlen == z_setlen )
	    return( OFF );
 
/*----------------------------------------------------------------------*
 *      SIMPLE KANSUJI 
 *----------------------------------------------------------------------*/
	z_rc = _Kckcan1( z_kcbptr, z_strpos, z_numlen );
	if ( z_rc == Z_END )
	    return( SUCCESS );
	else
	{
	    if ( z_rc != Z_CONTINUE)
	    return( z_rc );
	}
/*----------------------------------------------------------------------*
 *      SIMPLE KANSUJI WITH UNIT
 *----------------------------------------------------------------------*/
	z_rc = _Kckcan2( z_kcbptr, z_strpos, z_numlen );
	if ( z_rc  != SUCCESS)
            return( z_rc );
/*----------------------------------------------------------------------*
 *      COMPLEX KANSUJI 
 *----------------------------------------------------------------------*/
	z_rc = _Kckcan3( z_kcbptr, z_strpos, z_numlen );
	if ( z_rc  != SUCCESS)
            return( z_rc );
/*----------------------------------------------------------------------*
 *      ROMAN NUMERIC
 *----------------------------------------------------------------------*/
	z_rc = _Kckcanr( z_kcbptr, z_strpos, z_numlen );
/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/
        return( z_rc );
}

/*----------------------------------------------------------------------*
 *      STATIC TABLES
 *----------------------------------------------------------------------*/
static uschar kjn1[2][2] = {
    {0x82,0x60},{0x82,0x81}		/* A , a			*/
};
/************************************************************************
 *      START OF FUNCTION                                                 
 ************************************************************************/ 
short  _Kalpset( z_kcbptr, z_strpos, z_moralen, z_setlen )
struct KCB      *z_kcbptr;
uschar           z_strpos;              /* Pointer of 1st MCE           */
uschar           z_moralen;             /* length of string(mora code)  */
uschar           z_setlen;              /* length of string(mora code)  */
{
/*----------------------------------------------------------------------*  
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/ 
   extern   struct RETCGET   _Kccget();
   extern   short            _Kcjsjrt();

/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcjkj.h"   /* JKJ  Code table Entry (JKJ)                  */
#include   "_Kcymi.h"   /* Yomi Control Block (YMI)      		*/
#include   "_Kcmcb.h"   /* KKC  Control Block (MCB)                     */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   uschar	    z_alplen;
   short            z_i;
   short            z_rc;
   short            z_rjsjrt;
   struct RETCGET   z_rcget;
   short            z_endflg;
   uschar           z_flag[2];
   struct JKJ       *z_jkjtmp;

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* initialize of kcbptr1        */

   for( z_alplen = 0; z_alplen < z_moralen; z_alplen++ ) {
        mceptr1 = kcb.mchmce + (short)z_strpos + (short)z_alplen;
	if((( mce.code >= M_E_ALPHA_a ) && ( mce.code <= M_E_ALPHA_z)) ||
	   (( mce.code >= M_E_ALPHA_A ) && ( mce.code <= M_E_ALPHA_Z)))
	     continue;
	else
	    break;
   }
   if( z_alplen == z_setlen )
   	return( OFF );
 
/*----------------------------------------------------------------------*
 *      SIMPLE ALPHANUMERIC CONV
 *----------------------------------------------------------------------*/
   z_rc = Z_CONTINUE;

   mcbptr1 = (struct MCB *)kcb.myarea;
   ymiptr1 = mcb.ymiaddr;
/*----------------------------------------------------------------------*
 *      GET & SET JKJ
 *----------------------------------------------------------------------*/
   for( z_i = 0, mceptr1 = kcb.mchmce + z_strpos ; 
	z_i < z_alplen; z_i++, mceptr1++ ) {
      /*----------------------  GET JKJ ENTRY  -------------------------*/
      z_rcget = _Kccget(&kcb.jkhchh);
      if(z_rcget.rc == GET_EMPTY) {
         return( JKJOVER );
      }
      else {
         if((z_rcget.rc != GET_TOP_MID )&&
            (z_rcget.rc != GET_LAST    ))
            return( UERROR );
      }

      jkjptr1 = (struct JKJ *)z_rcget.cheptr;

      if ( z_i == 0 )                   /* save 1st pointer of JKJ      */
         jkjptr2 = jkjptr1;

      /*----------------------  SET JKJ  -------------------------------*/
      if(( mce.code >= M_E_ALPHA_A ) && ( mce.code <= M_E_ALPHA_Z)) { 
         jkj.kj[0] = 0x82;
         if(( ymi.yomi[z_strpos+z_i] >= M_E_ALPHA_A ) && 
	    ( ymi.yomi[z_strpos+z_i] <= M_E_ALPHA_Z )) 
            jkj.kj[1] = kjn1[0][1] + ( mce.code - M_E_ALPHA_A );
	 else 
            jkj.kj[1] = kjn1[1][1] + ( mce.code - M_E_ALPHA_A );
      }
      else {                     	/* INVALID CODE                 */
            z_rc = Z_END;
            FOR_BWD_MID( kcb.jkhchh, jkjptr1, z_endflg) {
               z_jkjtmp = jkjptr1;
               CMOVF(kcb.jkhchh,jkjptr1);
               _Kccfree( &kcb.jkhchh, z_jkjtmp );
               if ( z_jkjtmp == jkjptr2 )
                  z_endflg = ON;
            }
            return( z_rc );
      }
   }
   jkj.kj[0] &= 0x7F;                   /* last JKJ entry               */

/*----------------------------------------------------------------------*
 *      GET & SET JTE
 *----------------------------------------------------------------------*/

   z_flag[0] = NUM_FLAG0;
   z_flag[1] = NUM_FLAG1;

   z_rjsjrt = _Kcjsjrt( z_kcbptr,               /* pointer of KCB       */
                       NUM_HINSI,               /* hinshi               */
                          z_flag,               /* dflag                */
                        z_strpos,               /* mora position        */
                        z_alplen,               /* mora length          */
                        NUM_TYPE,               /* type                 */
                        jkjptr2 );              /* pointer of first JKJ */

/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/
    return( z_rjsjrt );
}
