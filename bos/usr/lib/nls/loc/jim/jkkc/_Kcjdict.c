static char sccsid[] = "@(#)92	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcjdict.c, libKJI, bos411, 9428A410j 7/23/92 03:10:56";
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
 * MODULE NAME:       _Kcjdict
 *
 * DESCRIPTIVE NAME:  LOOK UP A WORD IN DICTIONARIES
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS): success
 *                    0x1104 (JTEOVER): JTE table overflow
 *                    0x1204 (JKJOVER): JKJ table overflow
 *                    0x1304 (JLEOVER): JLE table overflow
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
short _Kcjdict(z_kcbptr,z_stpos,z_endpos)
struct KCB      *z_kcbptr;              /* get address of KCB           */
uschar z_stpos;                         /* start position of target     */
uschar z_endpos;                        /* end position of target       */
{ 
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern short           _Kcjldct();   /* Looking up Long Words        */
   extern short           _Kcjudct();   /* Looking up on User Dict.     */
   extern short           _Kcjsdct();   /* Looking up on System Dict.   */
   extern short           _Kcjmdct();   /* Looking up MRU Entry         */

/*----------------------------------------------------------------------*
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcmce.h"   /* Mora Code table Entry (MCE)                  */
#include   "_Kcmcb.h"   /* Monitor control block (MCB)                  */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short           z_rjldct;    /* Define Area for Return of _Kcjldct   */
   short           z_rjudct;    /* Define Area for Return of _Kcjudct   */
   short           z_rjsdct;    /* Define Area for Return of _Kcjsdct   */
   short           z_rjmdct;    /* Define Area for Return of _Kcjmdct   */
   uschar          z_length;    /* define mora length of target         */
   short           z_file;      /* System File Number                   */

/*----------------------------------------------------------------------*  
 *       JIRITSUGO DICTIONARY LOOK UP                                      
 *----------------------------------------------------------------------*/ 
   kcbptr1 = z_kcbptr;                  /* set base address of KCB      */ 
   mceptr1 = kcb.mchmce+z_stpos;        /* set start mora pointer       */
   mcbptr1 = ( struct MCB *)kcb.myarea; /* Get MCB pointer              */

/*----------------------------------------------------------------------*  
 *       HIRAGANA MORA, NORMAL YOMI PROCESS                                
 *----------------------------------------------------------------------*/ 
   z_length = z_endpos - z_stpos+1;     /* set length of target         */
   if (mce.jdok == JD_LONG )            /* if long word endtry          */
   {
      /*----------------------------------------------------------------*  
      *       LONG WORD DICT LOOK UP
      *-----------------------------------------------------------------*/ 
      z_rjldct=_Kcjldct(z_kcbptr,z_stpos,z_length);
                                        /* look up in long word dict.   */
      if ( z_rjldct != SUCCESS )
         return(z_rjldct);              /* default fatal error          */
   }
   else                                 /* not long word                */
   {
      /*-------------------------------------------------------------*  
      *       LOOK UP WORD IN USER DICTIONARY                        *  
      *--------------------------------------------------------------*/ 
      z_rjudct = _Kcjudct(z_kcbptr,z_stpos,z_length);
                                        /* look up in user dictionary   */
      if ( z_rjudct != SUCCESS )
         return(z_rjudct);              /* default fatal error          */

      /*----------------------------------------------------------------*  
      *       LOOK UP WORD IN SYSTEM DICTIONARY                         *  
      *-----------------------------------------------------------------*/ 
      ext_sys_conv = OFF;               /* Initializ mora2 setflag      */
      for( z_file = 0; z_file < mcb.mul_file; z_file++ ) {
				      	/* erase kcb.env != ENVNEXT chk */
	  if( !_Kchkex( z_kcbptr, z_stpos, z_length, z_file ))
	      continue;
          z_rjsdct = _Kcjsdct(z_kcbptr,z_stpos,z_length,(short)GENERIC,z_file);
                                        /* look up in system dictionary */
          if ( z_rjsdct != SUCCESS )
             return(z_rjsdct);          /* default fatal error          */
      }
      if(( !ext_sys_conv ) || ( kcb.env == ENVNEXT ) || ( kcb.env == ENVZEN )) {
	  if( _Knumchk( z_kcbptr, z_stpos, z_length, NUM_MORA|NUM_INCL ))
	      _Knumset( z_kcbptr, z_stpos, z_length, 0 );
	  else if( _Kalpchk( z_kcbptr, z_stpos, z_length, ALP_MORA|ALP_INCL ))
	      _Kalpset( z_kcbptr, z_stpos, z_length, 0 );
      }
   }                                    /* end of else(jdok == 2)       */
   /*-------------------------------------------------------------------*
    *       LOOK UP WORD IN MRU
    *-------------------------------------------------------------------*/
   z_rjmdct = _Kcjmdct(z_kcbptr,z_stpos,z_length);
                                        /* look up in MRU               */
   if ( z_rjmdct != SUCCESS )
      return(z_rjmdct);                 /* default fatal error          */

   return(SUCCESS);
}                                       /* end of program               */
