static char sccsid[] = "@(#)17	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mkanagt.c, libKJI, bos411, 9428A410j 7/23/92 03:23:05";
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
 * MODULE NAME:         _Mkanagt
 *
 * DESCRIPTIVE NAME:    Get Kana data.
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
 * FUNCTION:            Get Kana data position.
 *                      Get Kana data length.
 *                      Get Jiritsugo length.
 *                      Get Goki length.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1064 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mkanagt
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mkanagt( pt, pos,
 *                                kanapos, kanalen, jiritsu, maplen );
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *                      pos     :Conversion position.
 *
 *  OUTPUT:             kanapos :Position of Kana data.
 *                      kanalen :Length of Kana data.
 *                      jiritsu :Length of Jiritsugo.
 *                      maplen  :Length of Goki.
 *
 * EXIT-NORMAL:         IMSUCC  :Success of Execution.
 *
 * EXIT-ERROR:          NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              NA.
 *                      Standard Library.
 *                              NA.
 *                      Advanced Display Graphics Support Library(GSL).
 *                              NA.
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      See Below.
 *
 *   INPUT:             Kanji Monitor Control Block(KCB).
 *                              kjsvpt
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              convlen         kanalen         kanamap
 *                              kjcvmap
 *
 *   OUTPUT:                    NA.
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              IDENTIFY:Module Identify Create.
 *                      Standard Macro Library.
 *                              NA.
 *
 * CHANGE ACTIVITY:     NA.
 *
 ********************* END OF MODULE SPECIFICATIONS ************************
 */

/*
 *      include Standard.
 */
#include <stdio.h>      /* Standard I/O Package.                        */

/*
 *      include Kanji Project.
 */
#include "kj.h"         /* Kanji Project Define File.                   */
#include "kcb.h"        /* Kanji Control Block Structure.               */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-061 COPYRIGHT IBM CORP 1988           ";
static char *cprt3="Licensed Mateliars-Property of IBM  ";

/*
 *      Get Kana data position.
 *      Get Kana data length.
 *      Get Jiritsugo length.
 *      Get Goki length.
 */
int     _Mkanagt( pt, pos, kanapos, kanalen, jiritsu, maplen )

register KCB    *pt;    /* Pointer to Kanji Control Block               */
short   pos;            /* Conversion position.                         */
short   *kanapos;       /* Position of Kana data.                       */
short   *kanalen;       /* Length of Kana data.                         */
short   *jiritsu;       /* Length of Jiritsugo.                         */
short   *maplen;        /* Length of Goki.                              */
{
        register KMISA  *kjsvpt;/* Pointer to Kana Kanji Monitor
                                   Internal Save Area.                  */

        int     i;              /* Loop Counter                         */

        short   posend = 0;     /* Kana data end position.              */
        short   kjnum1 = 1;     /* Conv. mode counter.                  */
        short   kjnum2 = 0;     /* Conv. mode counter.                  */
        short   jannum1 = 0;    /* Adjust mode counter.                 */
        short   jannum2 = 0;    /* Adjust mode counter.                 */
        short   jan = 0;        /* Adjust mode counter.                 */
        short   pos1 = 0;       /* Kana data start position.            */
        short   pos2 = 0;       /* Kana data end position.              */
        short   mcount;         /* Shift conter.                        */
        short   bcount;         /* Kana map check position counter.     */



/*********************** Control Block Snap. ****************************/
/*                                                                      */
/*      SNAP AREA:      Kanji Monitor Internal Save Area.               */
/*                                                                      */
/************************************************************************/

        snap3( SNAP_KMISA, SNAP_Mkanagt, "Start-Return");



        kjsvpt = pt->kjsvpt;


        /* 1.
         *      Get kjcvmap position
         */


        /* 1.1.
         *      Not conversion mode processing
         */

        /*
         *      Check Kanji map code.
         */
        if ( kjsvpt->kjcvmap[pos] == M_KJMNCV ) {       /* Yomi.        */

            /*
             *      To Last position of Kanji convert map From Input position.
             */
            for ( i=pos ; i<kjsvpt->convlen ; i+=C_DBCS ) {

                /*
                 *      Check of Kanji map code.
                 */
                if ( kjsvpt->kjcvmap[i] != M_KJMNCV ) { /* Yomi.        */

                    pos2 = i;   /* Next Goki position set.              */

                    break;
                };
            };

            /*
             *      Check next Goki position.
             */
            if ( !pos2 ) {

                pos2 = kjsvpt->convlen; /* Next Goki position set.      */
            };

            *kanapos = 0;       /* Kana data position set.              */

            *kanalen = 0;       /* Kaba data length set.                */

            *jiritsu = 0;       /* Jiritsugo length set.                */

            *maplen  = pos2 - pos;      /* Goki length set.             */

            return( IMSUCC );
        };


        /* 1.2.
         *      Conversion mode processing
         */


        /* 1.2.1.
         *      Get Goki.
         */

        /*
         *      To Input position From First position Kanji convert map.
         */
        for ( i=0 ; i<pos ; i+=C_DBCS ) {


            /*
             *      Check Kanji map code.
             */
            switch( kjsvpt->kjcvmap[i] ) {

            case M_KJMNCV :     /* Yomi.                                */

            case M_KJMCTN :     /* Continuation.                        */

                break;

            default :

                kjnum1++;       /* Kanji data number increase.          */

                break;
            };
        };

        /*
         *      Check Kanji map code To Input position
         *          From Last position of Kanji convert map.
         */
        for ( i=pos+C_DBCS ; i<kjsvpt->convlen ; i+=C_DBCS ) {


            /*
             *      Check Kanji map code.
             */
            switch( kjsvpt->kjcvmap[i] ) {

            case M_KJMJAN :     /* Adjust.                              */

                jan = C_SWON;   /* Adjust flag ON.                      */

                jannum1++;      /* Adjust number increase.              */

                break;

            case M_KJMCTN :     /* Continuation.                        */

                if ( jan ) {    /* Adjust flag ON.                      */

                    jannum1++;  /* Adjust number increase.              */
                };
                break;

            default :

                posend = i;     /* End position set.                    */

                break;
            };

            if ( posend ) {
                break;
            };
        };

        /*
         *      Check End position.
         */
        if ( !posend ) {

            posend = kjsvpt->convlen;   /* End position set.            */
        };


        /* 1.2.2.
         *      Get Kana data processing.
         */
        mcount = 7;     /* First rigth shift count set.                 */

        bcount = 1;     /* First Kana map position set.                 */

        /*
         *      Check kana map From last position To first position.
         */
        for ( i=0 ; i<kjsvpt->kanalen ; i++ ) {

            /*
             *      Check Kana map.
             */
            if ( ((kjsvpt->kanamap[bcount] >> mcount) & C_SB01) != 0x00 ) {

                kjnum2++;       /* Kanji data number increase.          */

                if ( kjnum2 == kjnum1 ) {       /* Kanji data number1
                                                   equal
                                                   Kanji data number2.  */

                    pos1 = i;   /* Start position set.                  */

                    break;
                };
            };

            if ( mcount ) {

                mcount--;       /* Shift count decrease.                */
            } else {

                mcount = 7;     /* First rigth shift count set.         */

                bcount++;       /* Kana map position increase.          */
            };
        };

        /*
         *      Check kana map From last position To first position.
         */
        for ( i=pos1 ; i<kjsvpt->kanalen ; i++ ) {

            /*
             *      Check Kana map.
             */
            if ( ((kjsvpt->kanamap[bcount] >> mcount) & C_SB01) != 0x00 ) {

                jannum2++;      /* Adjust number increase.              */

                if ( jannum2 == (jan + 2) ) {   /* Adjust number1
                                                   equal
                                                   Adjust number2.      */

                    pos2 = i;   /* End position set.                    */

                    break;
                };
            };

            if ( mcount ) {

                mcount--;       /* Shift count decrease.                */

            } else {

                mcount = 7;     /* First rigth shift count set.         */

                bcount++;       /* Kana map position increase.          */
            };
        };

        /*
         *      Check end position.
         */
        if ( !pos2 ) {

            pos2 = kjsvpt->kanalen;     /* End position set.            */
        };



        /* 2.
         *      Set of output parameter.
         */
        *kanapos = pos1;        /* Set Kana data position.              */

        *kanalen = pos2 - pos1; /* Set Kana data length.                */

        *jiritsu = *kanalen - jannum1;  /* Set Jiritsugo length.        */

        if( *kanalen == *jiritsu ) {    /* Kana data length
                                           equal
                                           Jiritsugo length.            */

            /*
             *      Check Kanji map code.
             */
            if ( (kjsvpt->kjcvmap[pos] == M_KJMJAH) ||
                 (kjsvpt->kjcvmap[pos] == M_KJMAAN) ||
                 (kjsvpt->kjcvmap[pos] == M_KJMAAH) ) {
                /* Kanji convert map is Adjust.
                   Kanji convert map is Abbreviation/Alphabet.
                   Kanji convert map is All adjust.                     */

                *jiritsu = 0;   /* Clear Jiritsugo length.              */
            };
        };

        *maplen = posend - pos; /* Set Goki length.                     */



        /* 3.
         *      Return.
         */

        return( IMSUCC );
}
