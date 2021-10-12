static char sccsid[] = "@(#)65	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_MK_c1f.c, libKJI, bos411, 9428A410j 7/23/92 03:19:30";
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
 * MODULE NAME:         _MK_c1f
 *
 * DESCRIPTIVE NAME:    Input DBCS String Primary Flying Conversion to DBCS.
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
 * FUNCTION:            DBCS String Flying Convet to Primary Candidates.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        5100 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         Module Entry Point Name
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _MK_c1f( pt )
 *
 *  INPUT:              pt      :Pointer to Kanji Monitor Control Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      IMSUCC  :Successful of Execution.
 *
 * EXIT-ERROR:          Wait State Codes.
 *                      Other   :Return Code by _Kcconv(Phigical Error Code).
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Internal Subroutines.
 *                              NA.
 *                      Kanji Project Subroutines.
 *                              NA.
 *                      Standard Library.
 *                              memcpy  :Specified # Char Copy.
 *                              memset  :Specified Char Padding.
 *                      Advanced Display Graphics Support Library(GSL).
 *                              NA.
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      See Below.
 *
 *   INPUT:             Kanji Monitor Control Block(KCB).
 *                              NA.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              NA.
 *                      Trace Brock(TRB).
 *                              NA.
 *                      Kana Kanji Contror Brock(KKCB).
 *                              NA.
 *
 *   OUTPUT:            Kanji Monitor Control Brock(KCB).
 *                              NA.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              NA.
 *                      Trace Brock(TRB).
 *                              NA.
 *                      Kana Kanji Control Brock(KKCB).
 *                              NA.
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              ALIGN   :Adjust Alignment Number.
 *                              CHPTTOSH:Char Pointer Data convert to short.
 *                              IDENTIFY:Module Identify Create.
 *                              KKCPHYER:KKC Phigical Error Check.
 *                              MAX     :Each of Bigger Number Get.
 *                              MIN     :Each of Lower Number Get.
 *                              SHTOCHPT:short Data Set Char Pointer Area.
 *                      Standard Macro Library.
 *                              NA.
 *
 * CHANGE ACTIVITY:     Sept. 21 1988 Added by Satoshi Higuchi
 *                      Overflow of the kjdataf etc. support.
 *
 *                      06/09/92 call WatchUdict() for loading user dict.
 *
 ********************* END OF MODULE SPECIFICATIONS ************************
 */

/*
 *      include Standard.
 */
#include <stdio.h>      /* Standar I/O Header.                          */
#include <memory.h>     /* Memory Operation.                            */
/*
 *      include Kanji Project.
 */
#include "kj.h"         /* Kanji Define File.                           */
#include "kcb.h"        /* Kanji Monitor Control Block.                 */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-061 COPYRIGHT IBM CORP 1988           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

static int	_MK_c13f();
static int	_MK_c14f();

int     _MK_c1f( pt )

register KCB    *pt;            /* Pointer to Kanji Control Block.      */
{

        register KMISA  *kjsvpt;/* Pointer to Kanji Monitor
                                   Internal Save Area.                  */
        register KKCB   *kkcbsvpt;
                                /* Pointer to Kana Kanji Control Block. */

        short   loop1st;        /* Loop Start Counter.                  */
        short   loopend;        /* Loop End Counter.                    */
        short   loop1;          /* Loop First Position.                 */
        short   loop2;          /* Loop First Position.                 */

        short   fpos;           /* Flying Conversion Position.          */
        short   flen;           /* Flying Conversion Length.            */

        uchar   cnvmodfg;       /* Conversion Mode Flag.                */
        uchar   modchgfg;       /* Character Mode Change Flag.          */
        uchar   convflag;       /* Conversion Flag.                     */

        uchar   cmode;          /* Current Character Mode.              */
        uchar   fmode;          /* Character Mode.                      */

        short   length;         /* String Length.                       */
/*======================================================================*/
/* #(B) Sept. 21 1988 S,Higuchi                                         */
/* Added source.                                                        */
/*     int   rc = 0;                                                    */
/*     short length1;                                                   */
/*     short length2;                                                   */
/*======================================================================*/
	int     rc = 0;         /* return code                          */
	short   length1;        /* length of the work area              */
	short   length2;        /* length of the work area              */


        kjsvpt   = pt->kjsvpt;          /* Get Pointer to Kanji Monitor */
                                        /* Internal Save Area.          */
        kkcbsvpt = kjsvpt->kkcbsvpt;    /* Get Pointer to Kana/Kanji    */
                                        /* Control Block.               */


        /*
         ****************************************************************
         *      1. Check Conversion Limit.
         ****************************************************************
         */
        /*
         *      Conversion Length Check.
         */
        if(    ((kjsvpt->cconvlen/C_DBCS)>kkcbsvpt->kanamax )
            || (kjsvpt->cconvlen == 0 )                         ) {
                /*
                 *      Cannot Convrsion.
                 */
                _MK_c14f( pt );
                return( IMSUCC );
        };



        /*
         *      Set Flying Conversion Flag.
         */
        kjsvpt->fconvflg = C_SWON;



        /*
         *      Initialize Flying Conversion Valiables.
         */
        cnvmodfg = C_SWOFF;
        modchgfg = C_SWOFF;
        convflag = C_SWOFF;



        /*
         *      Flying Conversion.
         */

        loop1st = kjsvpt->fconvpos;
        loopend = kjsvpt->cconvpos + kjsvpt->cconvlen;

        if ( loop1st != loopend ) {


            for ( loop1 = loop1st ; loop1 < loopend ; ) {

                cnvmodfg = C_SWOFF;     /* Reset Conversion Mode.       */
                modchgfg = C_SWOFF;     /* Reset Mode Change Flag.      */

                fpos = loop1;

                flen = C_DBCS;

                /*
                 *      Get First Position Character Mode.
                 */
                cmode = _Mgetchm( pt->string , fpos , kjsvpt );

                if (   ( cmode == K_CHHIRA )  ||
                       ( (cmode == K_CHKATA) &&
                         (kjsvpt->kmpf[0].katakana == K_KANAON) )   ) {

                    cnvmodfg = C_SWOFF; /* Set Conversion Mode.         */
                } else {

                    cnvmodfg = C_SWON;  /* Set Conversion Mode.         */
                };

                for ( loop2 = (fpos + C_DBCS) ;
                      loop2 < loopend ;
                      loop2 += C_DBCS ) {

                    /*
                     *      Get Current Character Mode.
                     */
                    fmode = _Mgetchm( pt->string , loop2 , kjsvpt );

                    /*
                     *      Check Character Mode.
                     */
                    if (   ( cmode == K_CHHIRA )  ||
                           ( (cmode == K_CHKATA) &&
                             (kjsvpt->kmpf[0].katakana == K_KANAON) )   ) {

                        /*
                         *      CHeck Character Mode.
                         */
                        switch( fmode ) {

                        case K_CHHIRA:  /* Hiragana.    */

                            flen += C_DBCS;/* Increase Fly Convet length. */
                            modchgfg = C_SWOFF;/* Set Mode Change Flag.   */
                            break;

                         case K_CHKATA:  /* Katakana.    */

                            /*
                             *      Check Profile.
                             */
                            if ( kjsvpt->kmpf[0].katakana == K_KANAON ) {

                                modchgfg = C_SWOFF;/* Set Mode Change Flag.*/
                            } else {

                                modchgfg = C_SWON;/* Set Mode Change Flag. */
                            };

                            flen += C_DBCS;/* Increase Fly Convet length. */
                            break;

                        default:

                            flen += C_DBCS;/* Increase Fly Convet length. */
                            modchgfg = C_SWON;
                            break;
                        };

                    } else {

                        switch( fmode ) {

                        case K_CHHIRA:

                            modchgfg = C_SWON;/* Set Mode Change Flag.*/
                            break;

                        case K_CHKATA:

                            if ( kjsvpt->kmpf[0].katakana == K_KANAON ) {

                               modchgfg = C_SWON;/* Set Mode Change Flag.*/
                            } else {

                               flen += C_DBCS;/* Increase Fly Convet length.*/
                               modchgfg = C_SWOFF;/* Set Mode Change Flag.*/
                            };
                            break;

                        default:

                            flen += C_DBCS;/* Increase Fly Convet length. */
                            modchgfg = C_SWOFF;/* Set Mode Change Flag.*/
                            break;
                        };
                    };

                    if ( modchgfg ) {

                        if ( cnvmodfg ) {

                            _Mansave( pt, fpos, flen );
/*======================================================================*/
/* #(B) Sept. 21 1988 S,Higuchi                                         */
/* Added source.                                                        */
/*      if(kjsvpt->fcvovfg == C_SWON)                                   */
/*          return(IMFCVOVF);                                           */
/*======================================================================*/
			    if(kjsvpt->fcvovfg == C_SWON)
				return(IMFCVOVF);

                            convflag = C_SWOFF; /* Set Conversion Flag. */
                        } else {

                            _Midecid( pt, fpos, flen, cmode );
/*======================================================================*/
/* #(B) Sept. 21 1988 S,Higuchi                                         */
/* Added source.                                                        */
/*      if(kjsvpt->fcvovfg == C_SWON)                                   */
/*          return(IMFCVOVF);                                           */
/*======================================================================*/
			    if(kjsvpt->fcvovfg == C_SWON)
				return(IMFCVOVF);

                            convflag = C_SWOFF; /* Set Conversion Flag. */
                        };

                        break;
                    };
                };

                loop1 += flen;
            };

            if ( !modchgfg ) {

                if ( cnvmodfg ) {

                    _Mansave( pt, fpos, flen );
/*======================================================================*/
/* #(B) Sept. 21 1988 S,Higuchi                                         */
/* Added source.                                                        */
/*      if(kjsvpt->fcvovfg == C_SWON)                                   */
/*          return(IMFCVOVF);                                           */
/*======================================================================*/
		    if(kjsvpt->fcvovfg == C_SWON)
			return(IMFCVOVF);

                    convflag = C_SWOFF;         /* Set Conversion Flag. */
                } else {

                    convflag = C_SWON;          /* Set Conversion Flag. */
                };
            };
        } else {

            convflag = C_SWOFF;                 /* Set Conversion Flag. */
        };



        if ( convflag ) {
/*======================================================================*/
/* #(B) Sept. 21 1988 S,Higuchi                                         */
/* Changed source.                                                      */
/* Old source.                                                          */
/*      _MK_c11f( pt, fpos, flen, cmode );                              */
/*                                                                      */
/* New source.                                                          */
/*      rc = _MK_c11f(pt,fpos,flen,cmode);                              */
/*      if(rc == IMFCVOVF)                                              */
/*          return(rc);                                                 */
/*======================================================================*/
	    rc = _MK_c11f(pt,fpos,flen,cmode);
	    if(rc == IMFCVOVF)
		return(rc);
        };

        if ( kjsvpt->convnum ) {
/*======================================================================*/
/* #(B) Sept. 21 1988 S, Higuchi                                        */
/* Added source.                                                        */
/*      if(kjsvpt->fcvovfg == C_SWOFF) {                                */
/*          length1 = CHPTTOSH(kjsvpt->kjdata1s);                       */
/*          length2 = CHPTTOSH(kjsvpt->kjdataf);                        */
/*          if(kjsvpt->dat1smax < length1+(length2-C_DBCS))             */
/*              return(IMFCVOVF);                                       */
/*                                                                      */
/*          length1 = CHPTTOSH(kjsvpt->kjmap1s);                        */
/*          length2 = CHPTTOSH(kjsvpt->kjmapf);                         */
/*          if(kjsvpt->map1smax < length1+(length2-C_DBCS))             */
/*              return(IMFCVOVF);                                       */
/*                                                                      */
/*          length1 = kjsvpt->gramap1s[0];                              */
/*          length2 = kjsvpt->gramapf[0];                               */
/*          if(kjsvpt->gra1smax < length1+(length2-C_ANK))              */
/*              return(IMFCVOVF);                                       */
/*      }                                                               */
/*======================================================================*/
	    if(kjsvpt->fcvovfg == C_SWOFF) {
		length1 = CHPTTOSH(kjsvpt->kjdata1s);
		length2 = CHPTTOSH(kjsvpt->kjdataf);
		if(kjsvpt->dat1smax < length1+(length2-C_DBCS))
		    return(IMFCVOVF);

		length1 = CHPTTOSH(kjsvpt->kjmap1s);
		length2 = CHPTTOSH(kjsvpt->kjmapf);
		if(kjsvpt->map1smax < length1+(length2-C_DBCS))
		    return(IMFCVOVF);

		length1 = kjsvpt->gramap1s[0];
		length2 = kjsvpt->gramapf[0];
		if(kjsvpt->gra1smax < length1+(length2-C_ANK))
		    return(IMFCVOVF);
	    }

	    _MK_c12f( pt );

            length = CHPTTOSH( kjsvpt->kjdataf ) - C_DBCS;

            memcpy( (char *)kjsvpt->iws1,
                    (char *)&kjsvpt->kjdataf[C_DBCS],
                    length );

            _MK_c13f( pt, kjsvpt->iws1, length );

        } else {

            _MK_c14f( pt );
        };

        return( IMSUCC );
}


_MK_c11f( pt, pos, len, mode )

register KCB    *pt;            /* Pointer to Kanji Control Block.      */
short           pos;
short           len;
uchar           mode;
{
        int     _Myomic();      /* DBCS String Convert to 7bit code.    */
        uchar   _Mlfrtc();      /* DBCS Character Class Code.           */
        int     _Kcconv();      /* Conversion Word.                     */
        short   WatchUdict();  /* Watch/Load User dictionary		*/
        register KMISA  *kjsvpt;/* Pointer to Kanji Monitor
                                   Internal Save Area.                  */
        register KKCB   *kkcbsvpt;
                                /* Pointer to Kana Kanji Control Block. */

        uchar   *yomistr;       /* String is fly conversion.            */

        register int    loop;   /* Loop Counter.                        */
        uchar           *srcpos;/* Source Bit Pattern Byte Pointer.     */
        uchar           *dstpos;/* Destination Bit Pattern Byte Pointer.*/
        int             kanabyte;/* Kanamap Byte Number.                */
        int             kanabits;/* Kanamap Byte BIt Position.          */
        uchar           wrkpat; /* Bit Operation Work.                  */
        uchar           srcpat; /* BIt Operation Source Byte Work.      */
        uchar           dstpat; /* Bit Operation Destination Byte Work. */
        int             rembit; /* I can Use Bit Number.                */
        int             srcbit; /* Source Bit Pattern Byte Bit Position.*/
        int             actbit; /* Useful Bit Pattern Number.           */
        int             dstbit; /* Destination Bit Pattern Byte Bit Pos.*/
        int             movlen; /* Character or Bits Move Length.       */
        short           length;  /* Map Length.                         */
        short           grampos; /* Grammer Map Index.                  */
        short           kjmappos;/* kjmap1s pos.                        */
        short           kjdatpos;/* kjdata1s pos.                       */
        short           kjlen;  /* String data pos.                     */
        uchar   stringp = NULL; /* Character data 'NULL'.               */
/*======================================================================*/
/* #(B) Sept. 21 1988 S,Higuchi                                         */
/* Added source.                                                        */
/*     short length1;                                                   */
/*     short length2;                                                   */
/*======================================================================*/
	short           length1;/* length of the work area              */
	short           length2;/* length of the work area              */

        kjsvpt   = pt->kjsvpt;          /* Get Pointer to Kanji Monitor */
                                        /* Internal Save Area.          */
        kkcbsvpt = kjsvpt->kkcbsvpt;    /* Get Pointer to Kana/Kanji    */
                                        /* Control Block.               */


        /*  1.
         *   Call _Myomic.
         *   This routine Double bytes code string convert to
         *                    single byte 7 bit yomikana code string.
         */

        /*  Set pointer to position of conversion comparison.  */
        yomistr = (pt->string) + ((int)pos);


        /*  Set length of Kana data and string of Kana data.   */
        kkcbsvpt->kanalen1 =
        kkcbsvpt->kanalen2 = _Myomic( yomistr, len,
                                      kkcbsvpt->kanadata, mode );



        /*  2.
         *   Set value to valeable in KKCB.
         */


        /*  2.1.
         *   Set Left Character mode.
         */

        if ( kjsvpt->curleft != pos )  {

            /*
             *  Check character mode.
             *  And set mode to class of the left charcter in KKCB.
             */
            kkcbsvpt->leftchar = _Mlfrtc( pt->string + pos - 2 );

        } else  {

            /*
             *  Set Hiragana mode to class of the left character in KKCB.
             */
            kkcbsvpt->leftchar = M_HIRA;

        };


        /* 2.2.
         *  Set Right Character Mode.
         */
        if ( pt->repins == K_INS ) {

            /*
             *  Check character mode.
             *  And set mode to class of the left charcter in KKCB.
             */
            kkcbsvpt->rightchr = _Mlfrtc( pt->string + pos + len );

        } else {

            /*
             *  Set Hiragana mode to right character class in KKCB.
             */
            kkcbsvpt->rightchr = M_HIRA;

        };


        /* 2.3.
         *  Set to External Conversion flag in KKCB.
         *  All kana data have to be converted.
         */
        kkcbsvpt->extconv = K_EXTALL;


        /* 2.4.
         *  Set to Internal Conversion flag in KKCB.
         *  Each Converted data should be returned respectively.
         */
        kkcbsvpt->intconv = K_INTEAC;


        /* 2.5.
         *  Set to conversion mode in KKCB from conversion mode
         *  in KMPROFILE.
         */
        kkcbsvpt->convmode = kjsvpt->kmpf[0].conversn;



        /*  3.
         *   Call _Kcconv.
         *   This routine is Kana/Kanji Conversion .
         */
        if ( kjsvpt->kmpf[0].udload )
                (void)WatchUdict( kkcbsvpt );

        /* 3.1.
         *  Set flag for KKC Calling in KMISA.
         */
        kjsvpt->kkcflag = M_KKNOP;

        /* 3.2.
         *  Call KKC and set return code.
         */
        kjsvpt->kkcrc = _Kcconv( kkcbsvpt );

        /* 4.
         *  Check return code of KKC.
         */

        if( KKCPHYER(kjsvpt->kkcrc) )
            return( kjsvpt->kkcrc );

        if( kjsvpt->kkcrc == K_KCSUCC ) {
/*======================================================================*/
/* #(B) Sept. 21 1988 S,Higuchi                                         */
/* Added source.                                                        */
/*      length1 = CHPTTOSH(kjsvpt->kjdataf);                            */
/*      length2 = CHPTTOSH(kkcbsvpt->kjdata);                           */
/*      if(kjsvpt->datafmax < length1+(length2-C_DBCS))                 */
/*          return(IMFCVOVF);                                           */
/*                                                                      */
/*      length1 = CHPTTOSH(kjsvpt->kjmapf);                             */
/*      length2 = CHPTTOSH(kkcbsvpt->kjmap);                            */
/*      if(kjsvpt->mapfmax < length1+(length2-C_DBCS))                  */
/*          return(IMFCVOVF);                                           */
/*                                                                      */
/*      length1 = kjsvpt->gramapf[0];                                   */
/*      length2 = kkcbsvpt->grammap[0];                                 */
/*      if(kjsvpt->grafmax < length1+(length2-C_ANK))                   */
/*          return(IMFCVOVF);                                           */
/*======================================================================*/
	    length1 = CHPTTOSH(kjsvpt->kjdataf);
	    length2 = CHPTTOSH(kkcbsvpt->kjdata);
	    if(kjsvpt->datafmax < length1+(length2-C_DBCS))
		return(IMFCVOVF);

	    length1 = CHPTTOSH(kjsvpt->kjmapf);
	    length2 = CHPTTOSH(kkcbsvpt->kjmap);
	    if(kjsvpt->mapfmax < length1+(length2-C_DBCS))
		return(IMFCVOVF);

	    length1 = kjsvpt->gramapf[0];
	    length2 = kkcbsvpt->grammap[0];
	    if(kjsvpt->grafmax < length1+(length2-C_ANK))
		return(IMFCVOVF);

            /*
             *      Conversion Number Increase.
             */
            kjsvpt->convnum++;

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
            /* (void)memcpy( (char *)&kjsvpt->kjmapf[kjmappos+movlen+2],
                          (char *)&stringp,1 ); */

            /*
             *      KMISA Saved Kanji Map Length Set.
             */
            length = CHPTTOSH( kjsvpt->kjmapf ) + movlen;
            SHTOCHPT(kjsvpt->kjmapf,length);

            /*
             *      KMISA Saved Kanji Data Set.
             */

            /*  Hiragana/Katakana mode character.           */
            (void)memcpy( (char *)&kjsvpt->kjdataf[kjdatpos+2],
                          (char *)&kkcbsvpt->kjdata[2],
                          movlen*C_DBCS);

            /*  Other than Hiragana/Katakana mode character.           */
            /* kjlen = kjdatpos + movlen * C_DBCS + 2;
            (void)memcpy( (char *)&kjsvpt->kjdataf[kjlen],
                          (char *)&pt->string[pos+len-2],2 ); */

            /*
             *      KMISA Saved Kanji Data Length Set.
             */
            length = CHPTTOSH( kjsvpt->kjdataf) + movlen * C_DBCS;
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
/* #(B) Sept. 21 1988 S,Higuchi                                         */
/* Added source.                                                        */
/*      length1 = CHPTTOSH(kjsvpt->kjdataf);                            */
/*      if(kjsvpt->datafmax < length1+len)                              */
/*          return(IMFCVOVF);                                           */
/*                                                                      */
/*      length1 = CHPTTOSH(kjsvpt->kjmapf);                             */
/*      if(kjsvpt->mapfmax < length1+len/C_DBCS)                        */
/*          return(IMFCVOVF);                                           */
/*======================================================================*/
	    length1 = CHPTTOSH(kjsvpt->kjdataf);
	    if(kjsvpt->datafmax < length1+len)
		return(IMFCVOVF);

	    length1 = CHPTTOSH(kjsvpt->kjmapf);
	    if(kjsvpt->mapfmax < length1+len/C_DBCS)
		return(IMFCVOVF);

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
            (void)memset( (char *)&kjsvpt->kjmapf[kjmappos+2],
                          (char *)NULL,len / C_DBCS );

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
/*======================================================================*/
/* #(B) Sept. 21 1988 S,Higuchi                                         */
/* changed source.                                                      */
/* Old source.                                                          */
/*      return;                                                         */
/*                                                                      */
/* New source.                                                          */
/*      return(IMSUCC);                                                 */
/*======================================================================*/
	return(IMSUCC);
}

_MK_c12f( pt )

register KCB    *pt;            /* Pointer to Kanji Control Block.      */
{
        register KMISA *kjsvpt; /* Pointer to KMISA.                    */

        short   loop;           /* First Loop Position.                 */
        short   length;         /* Character Length.                    */
        short   pos;            /* Character Position.                  */
        uchar   mode;           /* Character Mode.                      */
        uchar   yomimap;        /* Yomi Map.                            */

	kjsvpt   = pt->kjsvpt;          /* Get Pointer to Kanji Monitor */
					/* Internal Save Area.          */

        /*
         *      Set Kana Data Map.
         */
        length = kjsvpt->kanamapf[0];

        for ( loop = 0 ; loop < length ; loop++ ) {
            kjsvpt->kanamap[loop] = kjsvpt->kanamapf[loop];
        };


        /*
         *      Set Kana Data.
         */
        for ( loop = 0 ; loop < kjsvpt->kanalenf ; loop++ ) {
            kjsvpt->kanadata[loop] = kjsvpt->kanadatf[loop];
        };

        /*
         *      Set Kana Data Length.
         */
        kjsvpt->kanalen = kjsvpt->kanalenf;

        /*
         *      Set Grammer Map.
         *      Set First Grammer Map.
         */
        length = kjsvpt->gramapf[0];
        for ( loop = 0 ; loop < length ; loop++ ) {
            kjsvpt->grammap[loop]  = kjsvpt->gramapf[loop];
            kjsvpt->gramap1s[loop] = kjsvpt->gramapf[loop];
        };

        /*
         *      Set First Kanji Data.
         *      Set First Kanji Map.
         */
        pos = 0;
        length = CHPTTOSH( kjsvpt->kjmapf ) - C_DBCS;
        for ( loop = 0 ; loop < length ; loop++ ) {

            if ( kjsvpt->kjmapf[loop + C_DBCS] == M_KSNCNV ) {
                continue;
            };

            kjsvpt->kjmap1s[pos + C_DBCS] = kjsvpt->kjmapf[loop + C_DBCS];

            kjsvpt->kjdata1s[pos * C_DBCS + C_DBCS] =
                kjsvpt->kjdataf[loop * C_DBCS + C_DBCS];
            kjsvpt->kjdata1s[pos * C_DBCS + C_DBCS + C_ANK] =
                kjsvpt->kjdataf[loop * C_DBCS + C_DBCS + C_ANK];
            pos++;
        };

        SHTOCHPT( kjsvpt->kjmap1s , pos + C_DBCS );
        SHTOCHPT( kjsvpt->kjdata1s, pos * C_DBCS + C_DBCS );


        /*
         *      Set Kanji Conversion Map.
         */
        for ( loop = 0, pos = 0 ; loop < length ; loop++, pos += C_DBCS ) {

            /*
             *  Set Kanji Map.
             */
            kjsvpt->kjcvmap[pos] = kjsvpt->kjmapf[loop + C_DBCS];

            /*
             *  Set Conversion Status Map.
             */

            if ( kjsvpt->kjmapf[loop + C_DBCS] == M_KSNCNV ) {

                /*
                 *      DBCS Character Type Get.
                 */
                mode = _Mgetchm( kjsvpt->kjdataf, loop * C_DBCS + C_DBCS , kjsvpt );

                /*
                 *      Analize Character Type.
                 */
                switch( mode ) {
                /*
                 *      DBCS Hiragana.
                 */
                case K_CHHIRA:
                    yomimap = M_YMKANA;
                    break;
                /*
                 *      DBCS Katakana.
                 */
                case K_CHKATA:
                    if( kjsvpt->kmpf[0].katakana == K_KANAON ) {
                        yomimap = M_YMKANA;
                    } else {
                        yomimap = M_YMIMPO;
                    };
                        break;
                /*
                 *      DBCS Alphabetic.
                 */
                case K_CHALPH:
                    if( kjsvpt->kmpf[0].alphanum == K_ALPON ) {
                        yomimap = M_YMALNM;
                    } else {
                        yomimap = M_YMIMPO;
                    };
                        break;
                /*
                 *      DBCS Numeric.
                 */
                case K_CHNUM:
                    if(  (kjsvpt->kmpf[0].alphanum == K_ALPOFF) &&
                         (kjsvpt->kmpf[0].kanjinum == K_KJNMOF)  ) {
                        yomimap = M_YMIMPO;
                    } else {
                        yomimap = M_YMALNM;
                    };
                    break;

                /*
                 *      DBCS Other Type.
                 */
                default:
                    yomimap = M_YMIMPO;
                    break;
                };

                if ( yomimap == M_YMIMPO ) {

                    kjsvpt->kjcvmap[pos + C_ANK] = M_KSCNUM;

                } else {

                    kjsvpt->kjcvmap[pos + C_ANK] = M_KSNCNV;
                };
            } else {

                kjsvpt->kjcvmap[pos + C_ANK] = M_KSCNVK;
            };
        };
}


/*
 *      Conversion Backend Process.
 */
static  int     _MK_c13f( pt,ostr,ostrlen)

register KCB *pt;       /* Pointer to KCB.                              */
uchar   *ostr;          /* Pointer to Converted String Save Area.       */
short   ostrlen;        /* Length of ostr.                              */

{
        int     _Mckbk();       /* Get Phrase Area Range.               */
        int     _MM_rtn();      /* Mode Change.                         */
        char    *memset();      /* Set Memory.                          */

        register KMISA *kjsvpt; /* Pointer to KMISA.                    */

        short   convmax;        /* KCB String convmax Position.         */
        short   atpos;          /* Highlighting Attribute Position.     */
        short   ist;            /* Next Phrase Start Offset.            */
        short   ied;            /* Next Phrase End   Offset.            */
        short   flag;           /* Phrase Attribute Set Flags.          */
        short   mlastch;        /* Last Character Position.             */
        /*
         ****************************************************************
         *      1-4. 0. Set Up General Work Pointer.
         ****************************************************************
         */
        kjsvpt  = pt->kjsvpt;   /* Get Pointer to Kanji Monitor Internal*/
                                /* Save Area.                           */
        /*
         ****************************************************************
         *      1-4. 1. Yomi DBCS String Area Replace DBCS Converted
         *      'Kanji' String.
         ****************************************************************
         */
        mlastch = pt->lastch;   /* Save Current Last Character Position */
        /*
         *      Display String Update.
         */
        _Mexchng(pt,ostr,0,ostrlen,kjsvpt->cconvpos,kjsvpt->cconvlen);
        mlastch = MAX(mlastch,pt->lastch);
                                /* Get Maximim Last Character Position  */
                                /* ,which is Previous or Current?       */
        /*
         ****************************************************************
         *      1-4. 2. Set Hilighting Attribute.
         *              Set Cursor Move Region.
         *              Set Convertion Mode.
         ****************************************************************
         */
        /*
         *      HIghlighting Attribute Sets.
         */
        if( kjsvpt->convlen == 0 ) {
                /*
                 ************************************************
                 *      1-4. 2-1. Overflow Conversion Data
                 *      Not Available Conversion Data.
                 ************************************************
                 */
                /*
                 *      Cursor Move Current Converting Data First Column.
                 */
                pt->curcol  = kjsvpt->cconvpos;

                /*
                 *      Current Conversion Data Nothing Now.
                 */
                kjsvpt->cconvlen= 0;
                kjsvpt->cconvpos= kjsvpt->curleft;

                /*
                 *      Change Current Mode to Primary Input Mode.
                 */
                (void)_MM_rtn(pt,A_1STINP);
        } else {
                /*
                 ************************************************
                 *      1-4. 2-2. Conversion Data Available.
                 ************************************************
                 */
                /*
                 *      Conversion Result Reflect Display Area.
                 */
                /*
                 *      Cursor Posotion Move Converted String Last Position.
                 */
                pt->curcol += ostrlen - kjsvpt->cconvlen;

                /*
                 *      Get Phrase Start Position & End Position.
                 */
                atpos = kjsvpt->cconvpos + ostrlen - C_DBCS;
                (void)_Mckbk(pt,atpos,&ist,&ied,&flag);

                /*
                 *      Current Phrase Attribute Set.
                 */
                convmax = kjsvpt->convpos + kjsvpt->convlen;
                if( flag == C_SWON ) {
                        /*
                         *      Cursor Display Area is Not
                         *      Conversional DBCS String.
                         */
                        (void)memset(
                                (char *)&pt->hlatst[kjsvpt->cconvpos],
                                K_HLAT2,
                                convmax - kjsvpt->cconvpos);

                        (void)memset(
                                (char *)&pt->hlatst[convmax],
                                K_HLAT0,
                                mlastch - convmax);
                        /*
                         *      Converted Length Set Zero.
                         */
                        kjsvpt->cconvlen = 0;
                } else {
                        /*
                         *      ied offset convert to position.
                         */
                        ied += C_POS;
                        /*
                         *      Before Current Converted String Attribute
                         *      Set.
                         */
                        (void)memset(
                                (char *)&pt->hlatst[kjsvpt->cconvpos],
                                K_HLAT2,
                                ist - kjsvpt->cconvpos);

                        /*
                         *      Current Conveted Attribute Set.
                         */
                        (void)memset(
                                (char *)&pt->hlatst[ist],
                                K_HLAT3,
                                ied - ist);

                        /*
                         *      After Current Convertd Attribute Set.
                         */
                        (void)memset(
                                (char *)&pt->hlatst[ied],
                                K_HLAT2,
                                convmax - ied);

                        /*
                         *      If After DBCS Convertd String Length
                         *      less than Before it then Previous
                         *      Hilighting Area Need Reset
                         *      Non Attribute.
                         */
                        (void)memset(
                                (char *)&pt->hlatst[convmax],
                                K_HLAT0,
                                mlastch - convmax);

                        /*
                         *      Current Conversion Posotion Sets.
                         */
                        kjsvpt->cconvpos = ist;
                        kjsvpt->cconvlen = ied - ist;

                };

                /*
                 *      Backend Conversion Status Set.
                 */
                if( pt->curcol == (kjsvpt->convpos + kjsvpt->convlen) ) {
                        /*
                         *      Cursor Position Conversion Last Position
                         *      when Which String is Non Converted
                         *      String Set Editing Mode A
                         *      ,Otherwiase Cotinuous Conversion Mode.
                         */
                        if(
                            kjsvpt->kjcvmap[pt->curcol-kjsvpt->convpos
                                        - C_DBCS +1] == M_KSCNVK )
                                kjsvpt->actc3 = A_CNVMOD;
                        else
                                kjsvpt->actc3 = A_EDTMOA;
                } else
                        kjsvpt->actc3 = A_EDTMOA;
        };

        /*
         ****************************************************************
         *      1-4. 3. Return to Caller.
         ****************************************************************
         */
        return( IMSUCC );
}


/*
 *      Cannot Conversional Word Processing.
 */
static  int     _MK_c14f( pt )

register KCB *pt;       /* Pointer to KCB.                              */

{
        int     _Msetch();      /* Display Refresh String Area Set.     */
        int     _Mindset();     /* Indicator Status Set.                */
        char    *memset();      /* Set Memory.                          */
        uchar   hlat;           /* HIlighting Attribute.                */

        register KMISA *kjsvpt; /* Pointer to KMISA.                    */

        /*
         ****************************************************************
         *      1-5. 0. Set Up General Work Pointer.
         ****************************************************************
         */
        kjsvpt  = pt->kjsvpt;   /* Get Pointer to Kanji Monitor Internal*/
                                /* Save Block.                          */
        /*
         *      Collection Exit Point.
         */
        do {
                /*
                 *******************************************************
                 *      1-5  1. Caution Bell On.
                 *******************************************************
                 */
                /*
                 *      Editor Action Code Sets.
                 */
                kjsvpt->actc3 = A_BEEP | A_EDTMOA;

                /*
                 *******************************************************
                 *      1-5  2. Cannot Conversion Indicator On.
                 *******************************************************
                 */
                /*
                 *      Set Cannot Conversion Indicator.
                 */
                kjsvpt->convimp = C_SWON;
                (void)_Mindset(pt,M_INDL);
                /*
                 *      Set Next Action Code(Draw Cannnot Conversion
                 *      Indicator).
                 */
                kjsvpt->nextact |= M_CNRSON;

                /*
                 *******************************************************
                 *      1-5  3. Highlinghting Attribute Set.
                 *******************************************************
                 */
                /*
                 *      Attribute Data Not Active?
                 */
                if( kjsvpt->cconvlen < C_DBCS )
                        break;

                /*
                 *      Display Refresh Area Sets.
                 */
                (void)_Msetch(pt,kjsvpt->cconvpos,kjsvpt->cconvlen);

                /*
                 *      Cursor Positon Set.
                 */
                pt->curcol = kjsvpt->cconvpos;

                /*
                 *      Hilighting Attribute Sets.
                 */
                if( kjsvpt->cconvlen > C_DBCS ) {
                        (void)memset(
                                (char *)&pt->hlatst[kjsvpt->cconvpos+C_DBCS],
                                K_HLAT2,
                                kjsvpt->cconvlen - C_DBCS);
                };

                /*
                 *      Conversion Actve Length & Hilighting Set.
                 */
                switch(
                    kjsvpt->kjcvmap[kjsvpt->cconvpos-kjsvpt->convpos+1]) {
                case M_KSNCNV:  /* Conversion DBCS String.              */
                case M_KSCNVK:  /* Conversion DBCS(Single Word).        */
                case M_KSCNSK:  /* Converting Yomi DBCS.                */
                case M_KSCNVY:  /* Converting Yomi DBCS(Word).          */
                        hlat    = K_HLAT3;
                        kjsvpt->cconvlen = C_DBCS;
                        break;
                case M_KSCNUM:  /* Numberic or Symbol Character.        */
                default      :
                        hlat    = K_HLAT2;
                        kjsvpt->cconvlen = 0;
                        break;
                };

                /*
                 *      Cursor Highlighting Attribute Set.
                 */
                (void)memset((char *)&pt->hlatst[kjsvpt->cconvpos],
                              hlat,C_DBCS);

        } while( NILCOND );

        /*
         ****************************************************************
         *      1-5. 4. Return to Caller.
         ****************************************************************
         */
        return( IMSUCC );
}
