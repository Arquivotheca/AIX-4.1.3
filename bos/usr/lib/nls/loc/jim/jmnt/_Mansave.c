static char sccsid[] = "@(#)83	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mansave.c, libKJI, bos411, 9428A410j 7/23/92 03:20:38";
/*
 * COMPONENT_NAME :	Japanese Input Method - Kanji Monitor
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/********************* START OF MODULE SPECIFICATIONS **********************
 *
 * MODULE NAME:         _Mansave
 *
 * DESCRIPTIVE NAME:    Save management fly conversion.
 *
 * COPYRIGHT:           5601-061 COPYRIGHT IBM CORP 1988
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              Kanji Monitor V1.0
 *
 * CLASSIFICATION:      OCO Source Material - IBM Confidential.
 *                      (IBM Confidential - Restricted when aggregated)
 *
 * FUNCTION:            Save save area in KMISA characters string
 *                      of non-conversion object by fly conversion.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        @@@@Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mansave
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mansave( pt, pos, len )
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *                      pos     :Position of fly conversion.
 *                      len     :Length of fly conversion.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         NA.
 *
 * EXIT-ERROR:          NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              NA.
 *                      Standard Library.
 *                              memcpy  :Copy characters from memory area
 *                                       A to B.
 *                      Advanced Display Graphics Support Library(GSL).
 *                              NA.
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      See Below.
 *
 *   INPUT:             DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Control Block(KCB).
 *                              string
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              NA.
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Control Block(KCB).
 *                              NA.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              fconvpos kjdataf kjmapf
 *                      Trace Block(TRB).
 *                              NA.
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              IDENTIFY:Module Identify Create.
 *                      Standard Macro Library.
 *                              NA.
 *
 * CHANGE ACTIVITY:     Sept. 20 1988 Added by Satoshi Higuchi
 *                      Overflow of the kjdataf etc. support.
 *                      Added logics.
 *
 ********************* END OF MODULE SPECIFICATIONS ************************
 */

/*
 *      include Standard.
 */
#include <stdio.h>      /* Standard I/O Package.                        */
#include <memory.h>     /* Perform Memory Operations.                   */

/*
 *      include Kanji Project.
 */
#include "kj.h"         /* Kanji Project Define File.                   */
#include "kcb.h"        /* Kanji Control Block Structure.               */


/*
 *      Copyright Identify.
 */
static char *cprt1="5601-061 COPYRIGHT IBM CORP 1988           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Input character on editorial mode.
 */
int     _Mansave( pt, pos, len )

register KCB    *pt;            /* Pointer to Kanji Control Block.      */
short           pos;            /* Position of fly conversion.          */
short           len;            /* Length of fly conversion.            */
{
        register KMISA  *kjsvpt;/* Pointer to Kanji Monitor
                                   Internal Save Area.                  */
        register KKCB   *kkcbsvpt;
                                /* Pointer to Kana Kanji Control Block. */

        short   length;         /* Map Length.                          */
        short   kjmappos;       /* kjmapf pos.                          */
        short   kjdatpos;       /* kjdataf pos.                         */
/*======================================================================*/
/*  #(B) Sept. 20 1988 S,Higuchi                                        */
/*      Added source.                                                   */
/*      short length1;                                                  */
/*      short length2;                                                  */
/*======================================================================*/
	short           length1;/* length of work area                  */
	short           length2;/* length of work area                  */
        uchar   stringp = 0; 	/* Character data 'NULL'.               */
        int j;                  /* loop counter.                        */

        kjsvpt   = pt->kjsvpt;          /* Get Pointer to Kanji Monitor */
					/* Internal Save Area.          */
/*======================================================================*/
/* #(B) Sept 20 1988 S,Higuchi                                          */
/*      Added source.                                                   */
/*      if(kjsvpt->fcvovfg != C_SWON) {                                 */
/*          length1 = CHPTTOSH(kjsvpt->kjdataf);                        */
/*          if(kjsvpt->datafmax < length1+len) {                        */
/*              kjsvpt->fcvovfg = C_SWON;                               */
/*              return(IMSUCC);                                         */
/*          }                                                           */
/*          length1 = CHPTTOSH(kjsvpt->kjmapf);                         */
/*          if(kjsvpt->mapfmax < length1+len/C_DBCS) {                  */
/*              kjsvpt->fcvovfg = C_SWON;                               */
/*              return(IMSUCC);                                         */
/*          }                                                           */
/*======================================================================*/
	if(kjsvpt->fcvovfg != C_SWON) {
	    length1 = CHPTTOSH(kjsvpt->kjdataf);
	    if(kjsvpt->datafmax < length1+len) {
		kjsvpt->fcvovfg = C_SWON;
		return(IMSUCC);
	    }
	    length1 = CHPTTOSH(kjsvpt->kjmapf);
	    if(kjsvpt->mapfmax < length1+len/C_DBCS) {
		kjsvpt->fcvovfg = C_SWON;
		return(IMSUCC);
	    }
	   /*  1.
	    *   Save length of characters and Map to Save Area in KMISA.
	    *   Length of characters is fly conversion position.
	    */

	   /*
	    *      Get Previous Kanji Map Length.
	    */
	    length   = CHPTTOSH(kjsvpt->kjmapf);
	   /*
	    *      KMISA Previous Kanji Map Search for Specified
	    *      number of 'Current Kanji Converstion Map Phrase Number'.
	    */
	    kjmappos = length - C_DBCS;
	    kjdatpos = kjmappos * C_DBCS;

	   /*
	    *      KKCB Data Copy To KMISA.
	    */

	    for ( j = 0; j < len / C_DBCS; j++ )  {

	       (void)memcpy( (char *)&kjsvpt->kjmapf[kjmappos+j+2],
			     (char *)&stringp,1 );

	    };

	   /*
	    *      KMISA Saved Kanji Map Length Set.
	    */
	    length = CHPTTOSH( kjsvpt->kjmapf ) + len / C_DBCS;
	    SHTOCHPT(kjsvpt->kjmapf,length);

	   /*
	    *      KMISA Saved Kanji Data Set.
	    */
	    (void)memcpy( (char *)&kjsvpt->kjdataf[kjdatpos+2],
			  (char *)&pt->string[pos],len );
	   /*
	    *      KMISA Saved Kanji Data Length Set.
	    */
	    length = CHPTTOSH( kjsvpt->kjdataf) + len;
	    SHTOCHPT( kjsvpt->kjdataf,length);

	   /*  2.
	    *   Set following position of fly conversion.
	    */
	    kjsvpt->fconvpos += len;
/*======================================================================*/
/* #(B) Sept. 20 1988 S,Higuchi                                         */
/*      Added source.                                                   */
/*      }                                                               */
/*======================================================================*/
	}       /* end of if(kjsvpt->fcvovfg != C_SWON) {               */

       /*  3.
        *   Return.
        */

        return;
}
