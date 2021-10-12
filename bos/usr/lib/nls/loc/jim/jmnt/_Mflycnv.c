static char sccsid[] = "@(#)02	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mflycnv.c, libKJI, bos411, 9428A410j 7/23/92 03:22:04";
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
 * MODULE NAME:         _Mflycnv
 *
 * DESCRIPTIVE NAME:    fly conversion.
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
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        @@@ Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mflycnv
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mflycnv ( pt, pos, len, mode )
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
 *                              fcnverfg curleft conversn
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
 *                              fcnverfg kkcflag kkcrc
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
 *                      Standard Macro Library.
 *                              NA.
 *
 * CHANGE ACTIVITY:     06/09/92 call WatchUdict() for loading user dict.
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
int     _Mflycnv( pt, pos, len, mode )

register KCB    *pt;            /* Pointer to Kanji Control Block.      */
short           pos;            /* Position of fly conversion.          */
short           len;            /* Length of fly conversion.            */
uchar           mode;           /* Character mode.                      */
{
        int     _Myomic();      /* DBCS String Convert to 7bit code.    */
        uchar   _Mlfrtc();      /* DBCS Character Class Code.           */
        int     _Kcconv();      /* Conversion Word.                     */
	short 	WatchUdict();	/* Watch/Load User dictionary		*/
        register KMISA  *kjsvpt;/* Pointer to Kanji Monitor
                                   Internal Save Area.                  */
        register KKCB   *kkcbsvpt;
                                /* Pointer to Kana Kanji Control Block. */

        uchar           *yomistr;
                                /* String is fly conversion.            */


        kjsvpt   = pt->kjsvpt;          /* Get Pointer to Kanji Monitor */
                                        /* Internal Save Area.          */
        kkcbsvpt = kjsvpt->kkcbsvpt;    /* Get Pointer to Kana/Kanji    */
                                        /* Control Block.               */


        for ( ;; )  {

        /*  1.
         *   Check of fly conversion flag in KMISA
         */
           if ( kjsvpt->fcnverfg == C_SWON )  break;

        /*  2.
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

        /*  3.
         *   Set value to valeable in KKCB.
         */

        /*  Case conversion position is not equal fly conversion position. */
           if ( kjsvpt->curleft != pos )  {

        /*
         *  Check character mode.
         *  And set mode to class of the left charcter in KKCB.
         */
              kkcbsvpt->leftchar = _Mlfrtc( pt->string + pos - 2 );

        /*  Case conversion position is equal fly conversion position. */
           }  else  {

        /*  Set Hiragana mode to class of the left character in KKCB.  */
              kkcbsvpt->leftchar = M_HIRA;

           };

        /*  Set Hiragana mode to right character class in KKCB.  */
           kkcbsvpt->rightchr = M_HIRA;

        /*  Set to External Conversion flag in KKCB.  */
        /*  Not all Kana data have to be converted.       */
           kkcbsvpt->extconv = K_EXTNAL;

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
           if ( kjsvpt->kkcrc == K_KCFLYC )  {

              kjsvpt->kkcrc = K_KCSUCC;

           };

        /*   Case Candidate not found. */
           if ( kjsvpt->kkcrc == K_KCNFCA )  {

        /*  Set to fly conversion flag.    */
              kjsvpt->fcnverfg = C_SWON;

           };


        /*  6.
         *   Return.
         */
           break;

        };   /*  End for loop  */

        return;
}
