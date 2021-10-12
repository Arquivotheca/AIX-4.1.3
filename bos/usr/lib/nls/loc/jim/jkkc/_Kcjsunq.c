static char sccsid[] = "@(#)09	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcjsunq.c, libKJI, bos411, 9428A410j 7/23/92 03:15:37";
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
 * MODULE NAME:       _Kcjsunq
 *
 * DESCRIPTIVE NAME:  LOOK UP A WORD ON SYSTEM DICTIONARY ABOUT YOMI
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
short _Kcjsunq(z_kcbptr,z_strpos,z_length,z_mode,z_next,z_file)
struct KCB  *z_kcbptr;                  /* pointer of KCB               */
uschar       z_strpos;                  /* pointer of 1st MCE           */
uschar       z_length;                  /* length of string(mora code)  */
short        z_mode;                    /* flag of the way for looking  */
                                        /*  up words                    */
uschar       z_next[NEXT_CONT_MAX];     /* Next record check            */
short	     z_file;			/* System File Number		*/
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern short           _Kcjsget();   /* Look up about Same Yomi      */

/*----------------------------------------------------------------------*
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcmcb.h"   /* Kkc Control Block (MCB)                      */
#include   "_Kcmce.h"   /* Mora Code table Entry (MCE)                  */
#include   "_Kcsd1.h"   /* System dictionary Data (SD1)                 */
#include   "_Kcsd2.h"   /* System dictionary Data (SD2)                 */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define mcet (*mcetmp)

/*----------------------------------------------------------------------*
*       LOCAL VARIABLES                                                 *
*-----------------------------------------------------------------------*/
   short           z_rjsget;    	/* Define for Return of _Kcjsget*/
   short           z_i;         	/* counter                      */
   short           z_strd ;     	/* start potision of kanji data */
   short           z_endd ;     	/* end potision of kanji data   */
   short           z_strm ;     	/* start position of yomi data  */
   short	   z_save_endd; 	/* Save End offset		*/
   short	   z_entry;		/* Data Entry Counter		*/
   short	   z_count;		/* Counter			*/
   uschar	   z_mora[MORA_MAX];	/* Yomi Mora code		*/
   uschar	   z_mora_buf[MORA_MAX];/* Mora buffer 			*/
   uschar	   z_set_length;	/* End of Kanji data length	*/
   uschar	   z_mora_len;          /* Mora Length                  */
   uschar	   z_miss;		/* Not found pattern		*/

/*----------------------------------------------------------------------*
 *      START OF PROCESS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set pointer of KCB           */
   mceptr1 = kcb.mchmce + (short)z_strpos;
   for( z_i = 0; z_i < z_length ; z_i++ ) {
       mcetmp = kcb.mchmce + (short)z_strpos + z_i;
       z_mora_buf[z_i] = mcet.code;
   }

#ifdef DBG_MSG
   printf("[%x]:ALL MORA = ", z_file );
   for( z_i = 0; z_i < z_length ; z_i++ ) 
	printf("%x ", z_mora_buf[z_i] );
   printf("\n");
#endif

/*----------------------------------------------------------------------*
 *      SEARCH SOME CANDIDATES FROM SYSTEM DCTIONARY
 *----------------------------------------------------------------------*/
 if( z_next[NEXT_CONT_CHECK] != NEXT_CONT )  {
   /*--------------------   compare 1st mora code   --------------------*/
   if((z_mode==GENERIC)||((z_mode==SPECIFIC)&&(z_length==1))) {
      mcbptr1 = (struct MCB *)kcb.myarea;   
      if( mcb.mulsys[z_file].mono_l != 0 ) {
        sdcptr1 = (struct SDC *)( mcb.dsyseg[z_file] + 
				  mcb.mulsys[z_file].record_size + 2 );
        /*--------------------------------------------------------------*
         *      SET DATA ENTRY COUNTER                        
         *--------------------------------------------------------------*/
	sdmptr1 = (struct SDM *)((uschar *)sdcptr1 + 
		  (uschar)_get_offset( sdc.count ));
	z_entry = _get_entry( sdc.count );
        for( z_count = 0; z_count < z_entry ; z_count++ ) {
	    if( z_mora_buf[0] == sdm.code[0] ) {
		sdaptr1 = (struct SDA *)((uschar *)sdmptr1 + z_entry * 2 );
		z_strd  = ((short)( sda.addr[1] & 0x3F) * 256 +
			   (short)( sda.addr[0]) + 1 );
		z_strm  = z_strd - 1;
		sdlptr1 = (struct SDL *)(mcb.dsyseg[z_file] + 
					 mcb.mulsys[z_file].record_size + 
			   		 z_strd - 1);
		z_endd  = (short)( z_strd + _get_entry( sdl.length ) - 2 );
		if( !sdl.length[0] ) z_strd += 2;
#ifdef DBG_MSG
		printf("	######## 1:MORA = %x ########\n", z_mora_buf[0] );
#endif
                if((z_rjsget=_Kcjsget(z_kcbptr,(short)JTB,z_strd, z_mode,
			 z_endd,z_strpos,(short)1,z_strm,z_file)) != SUCCESS )
               	    return(z_rjsget);
		if( z_mode == SPECIFIC ) {
		    mce.jdok = JD_COMPLETE;
		    return( SUCCESS );
		}
		break;
	    }
	    else {
	        if( z_mora_buf[0] <= sdm.code[0] ) 
		    break;
		else
		    sdmptr1++;
	    }
	}
      }
    }
  }
    /*----------------- End  compare 1st mora code   -------------------*/
    if( z_length == 1 ) {
        mce.jdok = JD_COMPLETE;
        return( SUCCESS );
    }
    else {
        mcbptr1 = (struct MCB *)kcb.myarea;   
        sdcptr1 = (struct SDC *)( kcb.sdesde + 2);
	sdmptr1 = (struct SDM *)((uschar *)sdcptr1 + 
		  (uschar)_get_offset( sdc.count ));
	z_entry = _get_entry( sdc.count );
    }
    /*------------------------------------------------------------------*
     *       CHECK Pre Mora2 Data
     *------------------------------------------------------------------*/
    if( mcb.mulsys[z_file].poly_l != 0 ) {
      /*------------------   compare mora code   -----------------------*/
      z_mora[0] = z_mora_buf[0]; z_mora[1] = z_mora_buf[1];
      z_set_length = 0; z_mora_len = 0; z_miss = 0; 

      for( z_count = 0; z_count < z_entry ; z_count++ ) {
        if( z_mora[0] == sdm.code[0] ) {
	  if( z_mora[1] == sdm.code[1] ) {
	    sdaptr1 = (struct SDA *)((uschar *)sdmptr1 + z_entry * 2 );
	    z_strd  = ((short)( sda.addr[1] & 0x3F) * 256 +
		       (short)( sda.addr[0]) + 1 );
	    z_strm  = z_strd - 1;
	    sdlptr1 = (struct SDL *)(kcb.sdesde + z_strd - 1);
	    if( !z_set_length ) z_mora_len = 2;
	    else                z_mora_len++; 
	    if(( sda.addr[1] & 0x80 ) == 0x80 ) {   /* Check Data Area 	*/ 
	        z_endd = (short)( z_strd + _get_entry( sdl.length ) - 2 );
		if( !sdl.length[0] ) z_strd += 2;
#ifdef DBG_MSG
		printf("	######## 2:MORA = %x, %x ########\n", z_mora[0] ,z_mora[1] );
#endif
                if((z_rjsget=_Kcjsget(z_kcbptr,(short)JTB,z_strd, z_mode,
		    z_endd,z_strpos,(short)z_mora_len+z_miss,z_strm,z_file)) != 
		    SUCCESS ) {
               	      return(z_rjsget);
		}
	    }
	    else
	        z_endd  = (short)((uschar *)sdlptr1 - kcb.sdesde - 1);
	    if( !z_set_length ) z_set_length += 2;
	    else                z_set_length += 1;
	    if(( z_set_length + z_miss ) >= z_length ) break;
	    if(( z_set_length + z_miss )  % 2 ) {
		z_mora[1] = z_mora_buf[z_set_length+z_miss];
        	sdcptr1 = (struct SDC *)(kcb.sdesde + z_save_endd + 1);
	        sdmptr1 = (struct SDM *)((uschar *)sdcptr1 + 
			  _get_offset( sdc.count ));
		z_entry = _get_entry( sdc.count );
		z_count = -1;
	    }
	    else {
	        if(( sda.addr[1] & 0x40 ) != 0x40 ) /* Check Mora Area */ 
		    break;  
		z_mora[0] = z_mora_buf[z_set_length+z_miss];
		z_mora[1] = 0x00;
        	sdcptr1  = (struct SDC *)(kcb.sdesde + z_endd + 1);
		z_save_endd = z_endd;
	        sdmptr1 = (struct SDM *)((uschar *)sdcptr1 + 
			  _get_offset( sdc.count ));
		z_entry = _get_entry( sdc.count );
		z_count = -1;
		continue;
	    }
	  }
	  else {
	      if((z_mora[0] <= sdm.code[0]) && (z_mora[1] <= sdm.code[1])) {
	          if((( z_set_length + z_miss ) % 2 ) || ( z_set_length == 0 ))
		      break;
		  else {
	 	    sdmptr1 = (struct SDM *)((uschar *)sdcptr1 + 
		            (uschar)_get_offset( sdc.count ));
		    z_count = -1;
		    z_mora[1] = z_mora_buf[z_set_length+z_miss+1];
		    z_miss++;
		    continue;
		  }
	      }
	      else
		  sdmptr1++;
	    }
	}
	else {
	    if((z_mora[0] <= sdm.code[0]) && (z_mora[1] <= sdm.code[1])) 
		break;
	    else
		sdmptr1++;
	}
      }
   }

/*----------------------------------------------------------------------*
 *       RETURN NORMAL
 *----------------------------------------------------------------------*/
   if( mce.jdok == JD_READY )           /* set jdok COMPLETE            */
       mce.jdok =  JD_COMPLETE;         /* if the word is not found     */
   return( SUCCESS );
}

_get_offset( buf )
uschar	buf[];
{
	uschar	z_ret;
	
    	if( buf[0] ) z_ret = 1;
    	else         z_ret = 3;
	return( z_ret );
}

_get_entry( buf )
uschar	buf[];
{
	short 	z_entry;

    	if( buf[0] ) {
	    z_entry = (short)buf[0];
	}
    	else {
	    z_entry = (short)( buf[1] ) + 
	              (short)( buf[2] * 256 );
	}
	return( z_entry );
}
