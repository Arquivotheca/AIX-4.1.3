/* @(#)05	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcjsknj.c, libKJI, bos411, 9428A410j 7/23/92 03:15:28	*/
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
 * MODULE NAME:       _Kcjsknj
 *
 * DESCRIPTIVE NAME:  GETS NEW JKJ AND SETS THE KANJI AT THE POSITION OF
 * 		      OFFSET.
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS): success
 *                    0x1204 (JKJOVER): JKJ table overflow
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
#include   "_Kcrcb.h"                   /* Define Return Code Structure */


/************************************************************************
 *      START OF FUNCTION
 ************************************************************************/
struct RETJSKNJ _Kcjsknj(z_kcbptr,z_offset, z_morlen,z_file )

struct  KCB   *z_kcbptr;
short   z_offset;
uschar  z_morlen;
short   z_file;
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern struct RETCGET _Kccget();

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcmcb.h"                   /* Kkc Control Block (MCB)      */
#include   "_Kcjkj.h"
#include   "_Kcctb.h"

#define   Z_KSKIP	0x1000		/* Kanji Skip Mode		*/
/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
	struct RETJSKNJ	  z_ret;
	struct RETCGET 	  z_hyoki;
	struct JKJ 	  *z_jkjp1;     /* work add pointer of jkj entry*/
	uschar *z_bufptr; 		/* address to point dict buf    */
	short  z_newoff;                /* return value of new offset   */
	uschar z_upper;                 /* work area for the upper byte */
	uschar z_lower;                 /* work area for the lower byte */

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* establish address'ty to mcb  */
                                        /*                              */
/*----------------------------------------------------------------------*
 *      SET KANJI BUFFER ADDERSS             
 *----------------------------------------------------------------------*/
  if( z_morlen == 1 ) {
      mcbptr1 = (struct MCB *)kcb.myarea;
      z_bufptr=mcb.dsyseg[z_file]+mcb.mulsys[z_file].record_size+z_offset; 
  }
  else
      z_bufptr = kcb.sdesde + z_offset; /* points kanji in buffer       */

/*----------------------------------------------------------------------*
 *      CHECK SKIP KANJI DATA
 *----------------------------------------------------------------------*/

  if( *z_bufptr == 0x80 ) {
#ifdef NOSKIP
      if( kcb.func == FUNCONV ) {	/* INITIAL CONVERSION 0x00	*/
	  *z_bufptr++;
          for(;((*z_bufptr) & 0x80) == 0;z_bufptr +=2);
	  z_bufptr += 2;
          if( z_morlen == 1 ) {
      	      z_newoff = z_bufptr + 2 - 
		 ( mcb.dsyseg[z_file] + mcb.mulsys[z_file].record_size ); 
  	  }
  	  else
              z_newoff = z_bufptr + 2 - kcb.sdesde;
  	  z_ret.newoff = z_newoff;      /* set new_offset               */
          z_ret.rc = Z_KSKIP;           /* set normal return code       */
  	  return(z_ret);
      }
      else
#endif
	  *z_bufptr++;			/* Skip 0x80 data		*/
  }

/*----------------------------------------------------------------------*
 *      GET A JKJ ENTRY BY _Kccget                  
 *----------------------------------------------------------------------*/
   z_hyoki = _Kccget(&kcb.jkhchh);      /* get a JKJ entry by _Kccget   */

   if ( z_hyoki.rc == GET_EMPTY )
   {
      z_ret.rc = JKJOVER;
      return(z_ret);
   }
   else if ( ( z_hyoki.rc != GET_TOP_MID ) && ( z_hyoki.rc != GET_LAST ) )
   {
      z_ret.rc = UERROR;
      return(z_ret);
   }

/*----------------------------------------------------------------------*
 *      ESTABLISH ADDRESSABILITY TO JKJ ENTRY
 *----------------------------------------------------------------------*/
  z_ret.jkjptr = (struct JKJ *)z_hyoki.cheptr;
                                        /* set the 1st jkj kanji pointer*/
                                        /*    in return code            */

/*----------------------------------------------------------------------*
 *      LOOP WHILE MAB OFF
 *----------------------------------------------------------------------*/
#ifdef DBG_MSG
	printf("	KANJI = ");
#endif

  for(;((*z_bufptr) & 0x80) == 0;z_bufptr +=2) {
     z_jkjp1  = (struct JKJ *)z_hyoki.cheptr;
     z_upper = *z_bufptr;               /* point the upper byte         */

#ifdef KANJI_CHECK
     if( _chk_code( z_upper|0x80 ) )
         (*z_jkjp1).kj[0] = ((KANJI_RM & 0x7F) | (z_upper & 0x0F) | 0x80);
     else
         (*z_jkjp1).kj[0] = (( z_upper & 0x20 ) << 1) | z_upper | 0x80 ;
#else
     (*z_jkjp1).kj[0] = (( z_upper & 0x20 ) << 1) | z_upper | 0x80 ;
#endif

     z_lower = *(z_bufptr + 1);         /* point the lower byte         */
     (*z_jkjp1).kj[1] = z_lower;

#ifdef DBG_MSG
    printf("%c%c (%02x%02x) ", 
    (*z_jkjp1).kj[0]|0x80,(*z_jkjp1).kj[1],(*z_jkjp1).kj[0],(*z_jkjp1).kj[1]);
#endif
                                        /* move the lower to JKJ        */
     z_hyoki = _Kccget(&kcb.jkhchh);    /* get a JKJ entry by _Kccget   */
                                        /* for the nxt entry            */
      if ( z_hyoki.rc == GET_EMPTY )
      {
         z_ret.rc = JKJOVER;
         return(z_ret);
      }
      else if ( ( z_hyoki.rc != GET_TOP_MID ) && ( z_hyoki.rc != GET_LAST ) )
      {
         z_ret.rc = UERROR;
         return(z_ret);
     }
  }

/*----------------------------------------------------------------------*
 *      MSB ON ( THE LAST DBCS )
 *----------------------------------------------------------------------*/
  z_jkjp1  = (struct JKJ *)z_hyoki.cheptr;
                                        /* set the jkj kanji pointer    */
                                        /* into a work pointer          */
  z_upper = *z_bufptr;                  /* point the upper byte         */

#ifdef KANJI_CHECK
  if( z_upper == 0xBF )
      (*z_jkjp1).kj[0] = (( z_upper | 0x40 ) & 0x7F );
  else if( _chk_code( z_upper ) )
      (*z_jkjp1).kj[0] = (( KANJI_RM & 0x7F ) | ( z_upper & 0x0F ));
  else 
      (*z_jkjp1).kj[0] = (((( z_upper & 0x20 ) << 1) | z_upper ) & 0x7F );
#else
  if( z_upper == 0xBF )
      (*z_jkjp1).kj[0] = (( z_upper | 0x40 ) & 0x7F );
  else 
      (*z_jkjp1).kj[0] = (((( z_upper & 0x20 ) << 1) | z_upper ) & 0x7F );
#endif

  z_lower = *(z_bufptr + 1);            /* point the lower byte         */
  (*z_jkjp1).kj[1] = z_lower;

#ifdef DBG_MSG
    printf("%c%c (%02x%02x)  ", 
    (*z_jkjp1).kj[0]|0x80,(*z_jkjp1).kj[1],(*z_jkjp1).kj[0],(*z_jkjp1).kj[1]);
#endif

                                        /* move the lower to JKJ        */
  if( z_morlen == 1 ) {
      z_newoff = z_bufptr + 2 - 
		 ( mcb.dsyseg[z_file] + mcb.mulsys[z_file].record_size ); 
  }
  else
      z_newoff = z_bufptr + 2 - kcb.sdesde;
                                        /* get new offset past kanji_hyk*/

  z_ret.newoff = z_newoff;              /* set new_offset               */
  z_ret.rc = SUCCESS;                   /* set normal return code       */

  return(z_ret);
}

_chk_code( z_code )
uschar z_code;
{
    if((( z_code >> 4 ) & 0x0F ) == ( KANJI_RM >> 4 )) 
	return( ON );
    else
	return( OFF );
}
