static char sccsid[] = "@(#)12	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Midecid.c, libKJI, bos411, 9428A410j 7/23/92 03:22:40";
/*
 * COMPONENT_NAME :	Japanese Input Method - Kanji Monitor
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/********************* START OF MODULE SPECIFICATIONS **********************
 *
 * MODULE NAME:         _Midecid
 *
 * DESCRIPTIVE NAME:    Primary decide management fly conversion.
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
 * FUNCTION:            When input character , call KKC with fly mode
 *                      and to go inside conversion.
 *                      Save decided character string to area of KMISA.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:            Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Midecid
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Midecid ( pt, pos, len, mode )
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *                      pos     :Position of fly conversion.
 *                      len     :Length of fly conversion.
 *                      mode    :Conversion character mode.
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
 *                              _Myomic :DBCS Yomi to 7bit yomi.
 *                              _Mlfrtc :DBCS Character Class Get.
 *                              _Kcconv :Yomi Convert DBCS String.
 *                              WatchUdict: Watch/Load user dict.
 *                      Standard Library.
 *                              NA.
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
 *                              kjmapf fcnverfg curleft kanalenf kanamapf
 *                              conversn
 *                      Trace Block(TRB).
 *                              NA.
 *                      Kana Kanji Control Brock(KKCB).
 *                              kanamap kanalen1 kanadata kjmapln kjmap
 *                              kjdata glmapln
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Control Block(KCB).
 *                              NA.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              fcnverfg fconvpos kkcflag kkcrc convnum
 *                              kjdataf kjmapf gramapf kanamapf kanadatf
 *                              kanalenf
 *                      Trace Block(TRB).
 *                              NA.
 *                      Kana Kanji Control Brock(KKCB).
 *                              kanadata kanalen1 kanalen2 leftchar
 *                              rightchr extconv intconv convmode
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              IDENTIFY:Module Identify Create.
 *                              CHPTTOSH:Char Pointer Data convert to short.
 *                              MIN     :Each of Lower Number Get.
 *                      Standard Macro Library.
 *                              NA.
 *
 * CHANGE ACTIVITY:     Sept. 20 1988 Added by Satoshi Higuchi
 *                      Overflow of the kjdataf etc. support.
 *                      Added logics.
 *
 *                      06/09/92 call WatchUdict() for loading user dict.
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
int     _Midecid( pt, pos, len, mode )
register KCB    *pt;            /* Pointer to Kanji Control Block.      */
short           pos;            /* Position of fly conversion.          */
short           len;            /* Length of fly conversion.            */
uchar           mode;           /* Character mode.                      */
{
        int     _Myomic();      /* DBCS String Convert to 7bit code.    */
        uchar   _Mlfrtc();      /* DBCS Character Class Code.           */
        int     _Kcconv();      /* Conversion Word.                     */
	short	WatchUdict();	/* Watch/Load User dictionary		*/
        char    *memcpy();      /* Copy # of Character.                 */

        register KMISA  *kjsvpt;/* Pointer to Kanji Monitor
                                   Internal Save Area.                  */
        register KKCB   *kkcbsvpt;
                                /* Pointer to Kana Kanji Control Block. */

        uchar           *yomistr;
                                /* String is fly conversion.            */

        uchar           dbcs_let[2];
                                /* DBCS character                       */

        register int    loop;   /* Loop Counter.                        */
        uchar           *srcpos;/* Source Bit Pattern Byte Pointer.     */
        uchar           *dstpos;/* Destination Bit Pattern Byte Pointer.*/
        int             kanabyte;/* Kanamap Byte Number.                */
        int             kanabits;/* Kanamap Byte BIt Position.          */
        uchar           wrkpat; /* Bit Operation Work.                  */
        uchar           srcpat; /* BIt Operation Source Byte Work.      */
        uchar           dstpat; /* Bit Operation Destination Byte Work. */
        int             actbit; /* Useful Bit Pattern Number.           */
        int             rembit; /* I can Use Bit Number.                */
        int             srcbit; /* Source Bit Pattern Byte Bit Position.*/
        int             dstbit; /* Destination Bit Pattern Byte Bit Pos.*/
        int             movlen; /* Character or Bits Move Length.       */
        short           length;  /* Map Length.                         */
        short           clength; /* Character Length.                   */
        short           grampos; /* Grammer Map Index.                  */
        short           kjmappos;/* kjmap1s pos.                        */
        short           kjdatpos;/* kjdata1s pos.                       */
        short           kjlen;  /* String data pos.                     */
        char            lpsw;   /* Loop switch flag                     */
        uchar   stringp = 0x00; /* Character data 'NULL'.               */
        int j;                  /* loop counter.                        */
/*======================================================================*/
/*  #(B) Sept. 20 1988 S,Higuchi                                        */
/*      Added source.                                                   */
/*      short length1;                                                  */
/*      short length2;                                                  */
/*======================================================================*/
	short           length1;/* length of work area                  */
	short           length2;/* length of work area                  */

        kjsvpt   = pt->kjsvpt;          /* Get Pointer to Kanji Monitor */
                                        /* Internal Save Area.          */
        kkcbsvpt = kjsvpt->kkcbsvpt;    /* Get Pointer to Kana/Kanji    */
                                        /* Control Block.               */


        lpsw  = C_SWOFF;   /*  Intialize loop switch flag  */
/*======================================================================*/
/* #(B) Sept. 20 1988 S,Higuchi                                         */
/*      Added source.                                                   */
/*      if(kjsvpt->fcvovfg != C_SWON) {                                 */
/*======================================================================*/
	if(kjsvpt->fcvovfg != C_SWON) {

	    while ( lpsw == C_SWOFF )  {

		/*  1.
		 *   Check fly conversion flag in KMISA
		 */
		if ( kjsvpt->fcnverfg == C_SWON ) {

/* #(B) 1988.02.02. Flying Conversion Change.   */
		    /* Set KKC routine return code.
		       Not Found Candidates.        */
		    kjsvpt->kkcrc = K_KCNFCA;

		    break;
		};
		/*  2.
		 *   Call _Myomic.
		 *   This routine Double bytes code string convert to
		 *                    single byte 7 bit yomikana code string.
		 */

		/*  Set pointer to position of conversion comparison.  */
		yomistr = (pt->string) + ((int)pos);

		/*  Set length of yomistr.                             */
		clength = len - C_DBCS;

		/*  Set length of Kana data and string of Kana data.   */
		kkcbsvpt->kanalen1 =
		kkcbsvpt->kanalen2 = _Myomic( yomistr, clength
					  , kkcbsvpt->kanadata, mode );

		/*  3.
		 *   Set value to valeable in KKCB.
		 */

		/*  Case conversion position is not equal
		    fly conversion position.                                */
		if ( kjsvpt->curleft != pos )  {

		    /*
		     *  Check character mode.
		     *  And set mode to class of the left charcter in KKCB.
		     */
		    kkcbsvpt->leftchar = _Mlfrtc( pt->string + pos - 2 );

		/*  Case conversion position is equal
		    fly conversion position.                                */
		}  else  {

		    /*  Set Hiragana mode to class of the left character
			in KKCB.                                            */
		    kkcbsvpt->leftchar = M_HIRA;

		};

		/*
		 *  Check character mode.
		 *  And set mode to class of the right charcter in KKCB.
		 */
		kkcbsvpt->rightchr = _Mlfrtc( pt->string + pos + len - 2 );

		/*  Set to External Conversion flag in KKCB.  */
		/*  All Kana data have to be converted.       */
		kkcbsvpt->extconv = K_EXTALL;

		/*  Set to Internal Conversion flag in KKCB.  */
		/*  Each Converted data should be returned respectively.  */
		kkcbsvpt->intconv = K_INTEAC;

		/*
		 *  Set to conversion mode in KKCB from conversion mode
		 *  in KMPROFILE.
		 */
		kkcbsvpt->convmode = kjsvpt->kmpf[0].conversn;

		/*  4.
		 *   Call _Kcconv.
		 *   This routine is Kana/Kanji Conversion .
		 */
        	if ( kjsvpt->kmpf[0].udload )
                    (void)WatchUdict( kkcbsvpt );

		/*  Set flag for KKC Calling in KMISA.  */
		kjsvpt->kkcflag = M_KKNOP;

		/*  Call KKC and set return code.   */
		kjsvpt->kkcrc   = _Kcconv( kkcbsvpt );


		/*  5.
		 *   Check return code of KKC.
		 */

		/*   Case Normal return code. */
		switch ( kjsvpt->kkcrc ) {

		case K_KCFLYC:
		case K_KCSUCC:

                /*   Change return code.             */
                kjsvpt->kkcrc = K_KCSUCC;

                /*   Conversion Counter Increment.   */
                kjsvpt->convnum++;
                break;
		default :
		    break;
		};

		/*   Set loop end switch             */
		lpsw = C_SWON;

	    };

	    /*  6.
	     *   Change the text of KMISA.
	     */

	    /*  Case success return code of KKC.        */
	    if ( kjsvpt->kkcrc == K_KCSUCC )  {
/*======================================================================*/
/* #(B) Sept. 20 1988 S,Higuchi                                         */
/*      Added source.                                                   */
/*      length1 = CHPTTOSH(kjsvpt->kjdataf);                            */
/*      length2 = CHPTTOSH(kkcbsvpt->kjdata);                           */
/*      if(kjsvpt->datafmax < length1+(length2-C_DBCS+C_DBCS)) {        */
/*          kjsvpt->fcvovfg = C_SWON;                                   */
/*          return(IMSUCC);                                             */
/*      }                                                               */
/*      length1 = CHPTTOSH(kjsvpt->kjmapf);                             */
/*      length2 = CHPTTOSH(kkcbsvpt->kjmap);                            */
/*      if(kjsvpt->mapfmax < length1+(length2-C_DBCS+C_ANK)) {          */
/*          kjsvpt->fcvovfg = C_SWON;                                   */
/*          return(IMSUCC);                                             */
/*      }                                                               */
/*      length1 = kjsvpt->gramapf[0];                                   */
/*      length2 = kkcbsvpt->grammap[0];                                 */
/*      if(kjsvpt->grafmax < length1+(length2-C_ANK)) {                 */
/*          kjsvpt->fcvovfg = C_SWON;                                   */
/*          return(IMSUCC);                                             */
/*      }                                                               */
/*======================================================================*/
		length1 = CHPTTOSH(kjsvpt->kjdataf);
		length2 = CHPTTOSH(kkcbsvpt->kjdata);
		if(kjsvpt->datafmax < length1+(length2-C_DBCS+C_DBCS)) {
		    kjsvpt->fcvovfg = C_SWON;
		    return(IMSUCC);
		}
		length1 = CHPTTOSH(kjsvpt->kjmapf);
		length2 = CHPTTOSH(kkcbsvpt->kjmap);
		if(kjsvpt->mapfmax < length1+(length2-C_DBCS+C_ANK)) {
		    kjsvpt->fcvovfg = C_SWON;
		    return(IMSUCC);
		}
		length1 = kjsvpt->gramapf[0];
		length2 = kkcbsvpt->grammap[0];
		if(kjsvpt->grafmax < length1+(length2-C_ANK)) {
		    kjsvpt->fcvovfg = C_SWON;
		    return(IMSUCC);
		}
		/*
		 *      Copy KKCB Kana Map to KMISA Kana Map.
		 */
		/*
		 *      KKCB Kana Map Data Area Address & Bit Position Get.
		 */
		srcpos   = &kkcbsvpt->kanamap[1];
		srcbit   = 0;

		/*
		 *      Make Kana data Length.
		 */
		kanabits = kjsvpt->kanalenf;
		kanabyte = (kanabits/C_BITBYT);

		/*
		 *      KMISA Kana Map Data Area Address & Bit Position Get.
		 */
		dstpos   = &kjsvpt->kanamapf[kanabyte+1];
		dstbit   = kanabits % C_BITBYT;

		/*
		 *      KKCB Kana Map Length Get.
		 */
		loop     = kkcbsvpt->kanalen1;

		/*
		 *      KKCB Kana Map Copy to KMISA Kana Map.
		 */
		while( loop > 0 ) {
		    /*
		     *      Available Bit Get.
		     */
		    rembit = C_BITBYT - dstbit;
		    actbit = MIN( loop ,rembit );
		    actbit = MIN( actbit, C_BITBYT - srcbit );

		    /*
		     *      KKCB Source Pattern Get.
		     *
		     *                      +----- actbit
		     *                    < |  >
		     *                   |    |
		     *      +------------+------+
		     *      |ZZZXXXXXXXXXYYYYYYA|
		     *      +---+--------+------+
		     *          ^        ^< |  >|
		     *          |        |  +--- rembit
		     *          |        +------ dstbit;
		     *          +--------------- srcbit;
		     *
		     *              |
		     *              V
		     *      +--------------------+
		     *      |XXXXXXXXXYYYYYYA0000|
		     *      +--------------------+
		     *              |
		     *              V
		     *      +--------------------+
		     *      |000000000000XXXXXXXX|
		     *      +--------------------+
		     *              |
		     *              V
		     *      +--------------------+
		     *      |000000000000XXXXXXXX|
		     *      +--------------------+
		     *              |
		     *              V
		     *      +--------------------+
		     *      |00000000000XXXXXXXX0|
		     *      +--------------------+
		     *              |
		     *              V
		     *      +--------------------+
		     *      |000000000000XXXXXXX0|
		     *      +--------------------+
		     */
		    srcpat = *srcpos;
		    srcpat <<= srcbit;
		    srcpat >>= dstbit;
		    srcpat >>= (rembit - actbit);
		    srcpat <<= (rembit - actbit);

		    /*
		     *      KMISA Target Pattern Get.
		     *
		     *                      +----- actbit
		     *                    < |  >
		     *                   |    |
		     *      +------------+------+
		     *      |ZZZXXXXXXXXXYYYYYYA|
		     *      +---+--------+------+
		     *          ^        ^< |  >|
		     *          |        |  +--- rembit
		     *          |        +------ dstbit
		     *          +--------------- srcbit
		     *
		     *              |       +----------------|
		     *              V                        V
		     *      +-------------------+   +-------------------+
		     *      |0000000ZZZXXXXXXXXX|   |A00000000000XXXXXXX|
		     *      +-------------------+   +-------------------+
		     *              |                        |
		     *              V                        V
		     *      +-------------------+   +-------------------+
		     *      |ZZZXXXXXXXXX0000000|   |000000000000000000A|
		     *      +-------------------+   +-------------------+
		     *              |                        |
		     *              V------------------------+
		     *      +-------------------+
		     *      |ZZZXXXXXXXXX000000A|
		     *      +-------------------+
		     */
		    wrkpat = dstpat = *dstpos;
		    dstpat >>= rembit;
		    dstpat <<= rembit;
		    wrkpat <<= dstbit + actbit;
		    wrkpat >>= dstbit + actbit;
		    dstpat |= wrkpat;

		    /*
		     *      New Pattern Sets.
		     */
		    *dstpos = srcpat | dstpat;
		    srcbit += actbit;
		    dstbit += actbit;
		    loop   -= actbit;

		    /*
		     *      Increment Source Bit Position.
		     */
		    if( srcbit >= C_BITBYT ) {
			    srcpos++;
			    srcbit -= C_BITBYT;
		    };

		    /*
		     *      Increment Destination Bit Position.
		     */
		    if( dstbit >= C_BITBYT ) {
			    dstpos++;
			    dstbit -= C_BITBYT;
		    };
		};

		/*
		 *      Kana map Length Update.
		 */
		kjsvpt->kanamapf[0] = 1 +
                    ALIGN( kjsvpt->kanalenf + kkcbsvpt->kanalen1,C_BITBYT)
                    / C_BITBYT;

		/*
		 *      Copy New Kana data.
		 */
		(void)memcpy( (char *)&kjsvpt->kanadatf[kjsvpt->kanalenf],
			      (char *)kkcbsvpt->kanadata,kkcbsvpt->kanalen1 );

		/*
		 *      Kana data Length Adjust.
		 */
		kjsvpt->kanalenf += kkcbsvpt->kanalen1;

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
		 *      KKCB Kanji Map Length Data Length Get.
		 */
		movlen   = kkcbsvpt->kjmapln - C_DBCS;

		/*
		 *      KKCB Data Copy To KMISA.
		 */

		/*  Hiragana/Katakana mode character.           */
		(void)memcpy( (char *)&kjsvpt->kjmapf[kjmappos+2],
			      (char *)&kkcbsvpt->kjmap[2],movlen );

		/*  Other than Hiragana/Katakana mode character.           */
		(void)memcpy( (char *)&kjsvpt->kjmapf[kjmappos+movlen+2],
			      (char *)&stringp,1 );

		/*
		 *      KMISA Saved Kanji Map Length Set.
		 */
		length = CHPTTOSH( kjsvpt->kjmapf ) + movlen + 1;
		SHTOCHPT(kjsvpt->kjmapf,length);

		/*
		 *      KMISA Saved Kanji Data Set.
		 */

		/*  Hiragana/Katakana mode character.           */
		(void)memcpy( (char *)&kjsvpt->kjdataf[kjdatpos+2],
			      (char *)&kkcbsvpt->kjdata[2],
			      movlen*C_DBCS);

		/*  Other than Hiragana/Katakana mode character.           */
		kjlen = kjdatpos + movlen * C_DBCS + 2;
		(void)memcpy( (char *)&kjsvpt->kjdataf[kjlen],
			      (char *)&pt->string[pos+len-2],2 );

		/*
		 *      KMISA Saved Kanji Data Length Set.
		 */
		length = CHPTTOSH( kjsvpt->kjdataf) + (movlen + 1) * C_DBCS;
		SHTOCHPT( kjsvpt->kjdataf,length);

		/*
		 *      Copy Saved Grammer Map.
		 */
		grampos = kjsvpt->gramapf[0];
		movlen = kkcbsvpt->grmapln-1;
		(void)memcpy( (char *)&kjsvpt->gramapf[grampos],
			      (char *)&kkcbsvpt->grammap[1],movlen );
		/*
		 *      Adjust Saved Grammer Map Length.
		 */
		kjsvpt->gramapf[0] += movlen;

		/*  Case error return code of KKC.              */
	    } else {
/*======================================================================*/
/* #(B) Sept 20 1988 S,Higuchi                                          */
/*      Added source.                                                   */
/*      length1 = CHPTTOSH(kjsvpt->kjdataf);                            */
/*      if(kjsvpt->datafmax < length1+len) {                            */
/*          kjsvpt->fcvovfg = C_SWON;                                   */
/*          return(IMSUCC);                                             */
/*      }                                                               */
/*      length1 = CHPTTOSH(kjsvpt->kjmapf);                             */
/*      if(kjsvpt->mapfmax < length1+len/C_DBCS) {                      */
/*          kjsvpt->fcvovfg = C_SWON;                                   */
/*          return(IMSUCC);                                             */
/*      }                                                               */
/*======================================================================*/
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

		    (void)memcpy( (char *)&kjsvpt->kjmapf[kjmappos+2],
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

	    };

	    /*  7.
	     *   Set following position of fly conversion.
	     */
	    kjsvpt->fconvpos += len;

	    /*  8.
	     *   Reset fly conversion flag in KMISA.
	     */
	    kjsvpt->fcnverfg = C_SWOFF;
/*======================================================================*/
/* #(B) Sept. 20 1988 S,Higuchi                                         */
/*      Added source.                                                   */
/*      }                                                               */
/*======================================================================*/
	}       /* end of if(kjsvpt->fcvovfg != C_SWON) {               */
	/*  9.
	 *    Return.
	 */

	return;
}
