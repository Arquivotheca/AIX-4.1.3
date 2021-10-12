static char sccsid[] = "@(#)78	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_MR_rtn.c, libKJI, bos411, 9428A410j 7/23/92 03:20:17";
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
 * MODULE NAME:         _MR_rtn
 *
 * DESCRIPTIVE NAME:    Romaji to Kana conversion interface.
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
 * FUNCTION:            Clear length of remeining romaji string.
 *                      Kana to Alphabet conversion processing.
 *                      Romaji to Kana conversion processing.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1288 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _MR_rtn
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _MR_rtn( pt, rkcmode )
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *                      rkcmode :Romaji to Kana conversion mode.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IMSUCC  :Success of Execution.
 *
 * EXIT-ERROR:          IMIVCODE:Invalid pseudo code error.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _MD_rtn :Code process routine.
 *                              _Mktec  :Kana to Alphabet conversion.
 *                              _Rkc    :Romaji to Kana conversion.
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
 *   INPUT:             Kanji Monitor Controle Block(KCB).
 *                              kjsvpt          code            shift1
 *                              type
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              rkcchar         rkclen
 *
 *   OUTPUT:            Kanji Monitor Controle Block(KCB).
 *                              type
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              chcode          chcodlen        pscode
 *                              rkcchar         rkclen
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              IDENTIFY:Module Identify Create.
 *                              DBCSTOCH:Set DBCS character to pointed area.
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
 *      Clear length of remeining romaji string.
 *      Single byte kana code to Single byte alphabet code conversion.
 *      Romaji to Double byte kana code conversion.
 */
int     _MR_rtn( pt, rkcmode )

register KCB    *pt;            /* Pointer to Kanji Control Block.      */
uchar   rkcmode;                /* Romaji to Kana conversion mode.      */
{
        register KMISA  *kjsvpt;/* Pointer to Kanji Monitor
                                   Internal Save Area.                  */

        int     _MD_rtn();      /* Code process routine.                */
        char    _Mktec();       /* Kana to Alphabet conversion.         */
        int     _Rkc();          /* Romaji to Kana conversion.           */

        char    *memcpy();      /* Copy characters from memory area
                                   A to B.                              */

        int     retcode;        /* Return code.                         */
        int     rc;             /* Return code.                         */

        char    btype;          /* Save to character type.              */

        uchar   retch;          /* Return of character code by _Mktec.  */

        short   mode;           /* Romaji to Kana conversion mode.      */
        uchar   romaji[6];      /* Input string.                        */
        short   rlen = 0;       /* Input string length.                 */
        uchar   kana[6];        /* Return string.                       */
        short   klen = 0;       /* Return string length.                */
        uchar   rest[6];        /* Remaining string.                    */
        short   rslen = 0;      /* Remaining string length.             */



/*********************** Control Block Snap. ****************************/
/*                                                                      */
/*      SNAP AREA:      Kanji Control Block.                            */
/*                      Kanji Monitor Internal Save Area.               */
/*                                                                      */
/************************************************************************/

        snap3( SNAP_KCB | SNAP_KMISA, SNAP_MR_rtn, "start");



        kjsvpt = pt->kjsvpt;

        /*
         *      Initialize return code.
         */

        retcode = IMSUCC;
        rc      = IMSUCC;



        /* 1.
         *      Check Input parameter(Romaji to Kana convert mode).
         */

        switch ( rkcmode ) {


        /* 1.1.
         *      Clear length of remaining romaji string mode.
         */

        case M_RKCOFF :


            /* 1.1.1.
             *      Input code type is pseudo code.
             *      Input code is Convert or No convert.
             *      Remaining romaji string is 'n'.
             *      Remaining romaji string is One character.
             */
            if (  (pt->type == K_INESCF)       &&
                  ( (pt->code == P_CONV)  ||
                    (pt->code == P_NCONV)  )   &&
                  (kjsvpt->rkcchar[0] == 'n')  &&
                  (kjsvpt->rkclen == 1)         )  {

                if ( pt->shift1 == K_ST1KAT ) { /* Katakana shift.      */

                    /*
                     *      Set changed character code.
                     */
                    DBCSTOCH( &kjsvpt->chcode[0], 0x8393 );

                } else {                        /* Hiragana shift.      */

                    /*
                     *      Set changed character code.
                     */
                    DBCSTOCH( &kjsvpt->chcode[0], 0x82f1 );
                };

                /*
                 *      Set changed character code length.
                 */
                kjsvpt->chcodlen = C_DBCS;

                btype = pt->type;       /* Save character code type.    */

                pt->type = K_INASCI;    /* Set character code type.     */

                kjsvpt->pscode = 0x00;  /* Clear pseudo code.           */

                rc = _MD_rtn( pt );     /* Code process.                */

                pt->type = btype;       /* Set character code type.     */

            /* 1.1.2.
             *      Input code type is character code.
             *      Input code is Blank code.
             */
            } else if (  (pt->type == K_INASCI)  &&
                         (pt->code == C_BLANK)    ) {

                /*
                 *      Remaining romaji string is 'n'.
                 *      Remaining romaji string is One character.
                 */
                if (  (kjsvpt->rkcchar[0] == 'n')  &&
                      (kjsvpt->rkclen == 1)         ) {

                    if ( pt->shift1 == K_ST1KAT ) { /* Katakana shift.  */

                        /*
                         *      Set changed character code.
                         */
                        DBCSTOCH( &kjsvpt->chcode[0], 0x8393 );

                    } else {                        /* Hiragana shift.  */

                        /*
                         *      Set changed character code.
                         */
                        DBCSTOCH( &kjsvpt->chcode[0], 0x82f1 );
                    };

                    /*
                     *      Set changed character code.
                     */
                    DBCSTOCH( &kjsvpt->chcode[C_DBCS], C_SPACE );

                    /*
                     *      Set changed character code length.
                     */
                    kjsvpt->chcodlen = C_DCHR;

                    rc = _MD_rtn(pt);   /* Code process.                */

                } else {
                /*
                 *      Remaining romaji string is not 'n'.
                 *      Remaining romaji string is not One character.
                 */

                    /*
                     *      Set changed character code.
                     */
                    DBCSTOCH( &kjsvpt->chcode[0], C_SPACE );

                    /*
                     *      Set changed character code length.
                     */
                    kjsvpt->chcodlen = C_DBCS;

                    rc = _MD_rtn( pt ); /* Code process.                */
                };

            /*
             *      Input code type is pseudo code.
             *      Input code is Back Space.
             *      remaining romaji string not Zero.
             */
            } else if (  (pt->type == K_INESCF) &&
                         (pt->code == P_BSPACE) &&
                         kjsvpt->rkclen          ) {

                retcode = IMIVCODE;     /* Set of error return code.    */
            };

            /*
             *      Clear of length of remaining romaji string.
             */
            kjsvpt->rkclen = 0;

            break;



        /* 1.2.
         *      Romaji to Kana conversion mode.
         */

        case M_RKCON :


            /* 1.2.1.
             *      Single byte kana code to Single byte Alphabet code
             *      conversion processing.(both of letter share same key)
             */
            if ( (retch = _Mktec( pt->code )) != IMKTEERR ) {

                /*
                 *      Remaining romaji string.
                 */
                if ( kjsvpt->rkclen ) {

                    /*
                     *      Set convert string.
                     */
                    memcpy( (char *)romaji,
                            (char *)kjsvpt->rkcchar,
                            kjsvpt->rkclen);
                    romaji[kjsvpt->rkclen] = retch;

                } else {

                    romaji[0] = retch;  /* Set Convert string.          */
                };

                rlen = kjsvpt->rkclen + 1;      /* Convert length set.  */

                if ( pt->shift1 == K_ST1KAT ) { /* Katakana shift.      */

                    mode = M_DBKATA;    /* Convert mode set.            */

                } else {                        /* Hiragana shift.      */

                    mode = M_DBHIRA;    /* Convert mode set.            */
                };


                /* 1.2.2.
                 *      Romaji to Kana conversion processing.
                 */
                rc = _Rkc( mode, romaji, rlen, kana, &klen, rest, &rslen);

                /*
                 *      Initialize changed character length.
                 */
                kjsvpt->chcodlen = 0;

                /*
                 *      Initialize length of remaining string.
                 */
                kjsvpt->rkclen = 0;


                if ( klen > 0 ) {       /* Convert successful.          */

                    /*
                     *      Set changed character.
                     */
                    memcpy( (char *)kjsvpt->chcode,
                            (char *)kana,
                            klen);

                    /*
                     *      Set changed character length.
                     */
                    kjsvpt->chcodlen = klen;

                    if ( rslen > 0 ) {  /* Remains convert character.   */

                        /*
                         *      Set remaining romaji string.
                         */
                        memcpy( (char *)kjsvpt->rkcchar,
                                (char *)rest,
                                rslen);

                        /*
                         *      Set length of remaining string.
                         */
                        kjsvpt->rkclen = rslen;
                    };

                    rc = _MD_rtn( pt );         /* Code process.        */

                } else if ( rslen > 0 ) {
                /*
                 *      Remains convert character.
                 */

                    /*
                     *      Set remaining romaji string.
                     */
                    memcpy( (char *)kjsvpt->rkcchar,
                            (char *)rest,
                            rslen);

                    /*
                     *      Set length of remaining string.
                     */
                    kjsvpt->rkclen = rslen;
                };
            };
            break;
        };



        /* 2.
         *      Return.
         */



/*********************** Control Block Snap. ****************************/
/*                                                                      */
/*      SNAP AREA:      Kanji Control Block.                            */
/*                      Kanji Monitor Internal Save Area.               */
/*                                                                      */
/************************************************************************/

        snap3( SNAP_KCB | SNAP_KMISA, SNAP_MR_rtn, "Return");

        return( retcode );
}
