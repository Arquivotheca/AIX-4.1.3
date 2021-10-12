static char sccsid[] = "@(#)53	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_ME_0a.c, libKJI, bos411, 9428A410j 7/23/92 03:18:46";
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
 * MODULE NAME:         _ME_0a
 *
 * DESCRIPTIVE NAME:    All candidates Input number process.
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
 * FUNCTION:            Get Input number.
 *                      Search string of Input number.
 *                      Kana Kanji Control Block data copy to
 *                          Kanji Monitor Internal Save Area.
 *                      Reset All candidates mode.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        2800 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _ME_0a
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _ME_0a( pt )
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IMSUCC  :Success of Execution.
 *
 * EXIT-ERROR:          IMNTNUME:Input code is not number.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _MM_rtn :Mode control routine.
 *                              _Mfmrst :Field message reset.
 *                              _Mkanagt:Get kana data.
 *                              _Mnumgt :Get number character.
 *                              _Mnxprps:Kana Kanji Control Block data copy
 *                                       to Kanji Monitor Internal Save Area.
 *                              _Mrsstrh:Next/Pre_candidates conversion.
 *                              _Mexchng:Replace "Destination String" to
 *                                       "Source string".
 *                              _Msetch :Set up convert position for display.
 *                              _Mflyrst:Reset Flying Conversion Area.
 *                      Standard Library.
 *                              memcpy  :Copy characters from memory area
 *                                       A to B.
 *                              memset  :Set character to memory area.
 *                      Advanced Display Graphics Support Library(GSL).
 *                              NA.
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      See Below.
 *
 *   INPUT:             Kanji Monitor Control Block(KCB).
 *                              kjsvpt          code            kbdlok
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              kkcbsvpt        cconvlen        cconvpos
 *                              convlen         convpos         kanalen
 *                              kanamap         kjcvmap         kkmode2
 *                              tankan
 *                      Kana Kanji Control Block(KKCB).
 *                              grammap         kanamap         kjdata
 *                              kjmap           rsnumcnd
 *
 *   OUTPUT:            Kanji Monitor Control Block(KCB).
 *                              curcol          hlatst
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              actc3           kanamap         kjcvmap
 *                              kkmode1         stringsv        grammap
 *                              kanadata        kjdata1s        kjmap1s
 *                              gramap1s        convpos         convlen
 *                              cconvpos        cconvlen        savepos
 *                              savelen         kanalen         preccpos
 *                      Kana Kanji Control Block(KKCB).
 *                              grammap         grmapln         kanamap
 *                              kjdata          kjlen           kjmap
 *                              kjmapln         knmapln         iws1
 *                              iws2
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              IDENTIFY:Module Identify Create.
 *                              CHPTTOSH:Short data set Character pointer.
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
#include <memory.h>     /* Performs Memory Operations.                  */

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
 *      Get Input number.
 *      Search string of Input number.
 *      Kana Kanji Control Block data copy to
 *          Kanji Monitor Internal Save Area.
 *      Reset All candidates mode.
 */
int     _ME_0a( pt )

register KCB    *pt;            /* Pointer to Kanji Control Block.      */
{
        register KMISA  *kjsvpt;/* Pointer to Kanji Monitor
                                   Internal Save Area.                  */
        register KKCB   *kkcbsvpt;
                                /* Pointer to Kana Kanji Control Block. */

        int     _MM_rtn();      /* Mode conversion.                     */
        int     _Mkanagt();     /* Get Kana data.                       */
        int     _Mfmrst();      /* Field message reset.                 */
        int     _Mnumgt();      /* Get number.                          */
        int     _Mnxprps();     /* Kana Kanji Control Block data copy to
                                   Kanji Monitor Internal Save Area.    */
        int     _Mrsstrh();     /* Next/Pre_candidate conversion.       */

        char    *memcpy();      /* Memory copy.                         */
        char    *memset();      /* Memory set.                          */

        int     rc;             /* Return code.                         */

        int     i;              /* Loop counter.                        */

        static uchar    numtbl[10][2] = { 0x82 , 0x4f , 0x82 , 0x50 ,
                                          0x82 , 0x51 , 0x82 , 0x52,
                                          0x82 , 0x53 , 0x82 , 0x54,
                                          0x82 , 0x55 , 0x82 , 0x56,
                                          0x82 , 0x57 , 0x82 , 0x58};
                                /* Double byte number code table.       */

        short   bcount;         /* Kana map check position counter.     */
        short   mcount;         /* Shift counter.                       */

        uchar   onum[C_DBCS];   /* Output Double byte code.(Number)     */

        short   inpnum;         /* Input number.                        */

        uchar   *kjdata;        /* Kanji data string.                   */
        short   kjpos;          /* Kanji data position.                 */
        uchar   *kjmap;         /* Kanji map string.                    */
        short   kjmapps;        /* Kanji map position.                  */
        uchar   *grammap;       /* Grammer map string.                  */
        short   grmapps;        /* Grammer map position.                */
        uchar   *kanamap;       /* Kana map string.                     */
        short   knmapps;        /* Kana map position.                   */

        uchar   *kjcvmap;       /* Kanji conversion map string.         */
        short   kjcvmpps;       /* kanji conversion map position.       */
        short   convpos;        /* Convesion position.                  */
        short   convlen;        /* Conversion length.                   */
        short   cconvpos;       /* Current conversion position.         */
        short   cconvlen;       /* Current conversion length.           */

        short   movpos;         /* Kanji conversion map move position.  */
        short   movlen;         /* Kanji conversion map move length.    */

        short   kanapos;        /* Kana data position.                  */
        short   kanalen;        /* Kana data length.                    */
        short   jiritsu;        /* Jiritsugo length.                    */
        short   maplen;         /* Goki length.                         */

        short   length;         /* Decid Conversion Length.             */

/*********************** Control Block Snap. ****************************/
/*                                                                      */
/*      SNAP AREA:      Kanji Control Block.                            */
/*                      Kanji Monitor Internal Save Area.               */
/*                      Kana Kanji Control Block.                       */
/*                                                                      */
/************************************************************************/

        snap3( SNAP_KCB | SNAP_KMISA | SNAP_KKCB, SNAP_ME_0a, "Start" );



        kjsvpt   = pt->kjsvpt;
        kkcbsvpt = kjsvpt->kkcbsvpt;



        /* 1.
         *      Check Input number.
         */


        /* 1.1.
         *      Get number string.
         */
        if ( _Mnumgt( &pt->code, onum ) == IMNTNUME ) {

            rc = _MM_rtn( pt, A_BEEP ); /* Beep.                        */

            return( IMNTNUME );
        };

        /* 1.2.
         *      Check input number.
         */
        for ( i=0 ; i<10 ; i++ ) {

            if (  (onum[0] == numtbl[i][0]) &&
                  (onum[1] == numtbl[i][1])  ) {

                if ( i < kkcbsvpt->rsnumcnd ) {

                    inpnum = i; /* Input number set.                    */

                    break;
                } else {

                    rc = _MM_rtn( pt, A_BEEP ); /* Beep.                */

                    return( IMNTNUME );
                };
            };
        };



        /* 2.
         *      Search string of Input number.
         */

        kjdata = kkcbsvpt->kjdata;      /* Set Kanji data string.       */

        kjpos = 0;      /* Initialize Kanji data position.              */

        for ( i=0 ; i<kkcbsvpt->rsnumcnd ; i++ ) {

            if ( i == inpnum ) {

                /*
                 *      Set Kanji data length.
                 */
                kkcbsvpt->kjlen = CHPTTOSH( &kjdata[kjpos] );

                break;
            };

            /*
             *      Kanji data position increase.
             */
            kjpos += CHPTTOSH( &kjdata[kjpos] );
        };



/* #(B) 1987.12.17. Flying Conversion Add */
        if ( kjsvpt->alcnmdfg ) {

            convpos  = kjsvpt->convpos;
            convlen  = kjsvpt->convlen;
            cconvpos = kjsvpt->cconvpos;
            cconvlen = kjsvpt->cconvlen;

            _Mfmrst( pt, K_MSGOTH );

            _Mexchng( pt, kjdata, kjpos + C_DBCS, C_DBCS,
                      cconvpos, cconvlen );

            if (  (convpos + convlen) > (cconvpos + cconvlen)  ) {

                pt->curcol = cconvpos;
            } else {

                pt->curcol = cconvpos + C_DBCS;
            };


            /*
             *      Flying Conversion Area Reset.
             */

            _Mflyrst( pt );
            kjsvpt->fconvflg = 0;

            /*
             *      KKC Control Block Parameter Reset.
             */

            memset( (char *)kjsvpt->kjcvmap, J_NULL, convlen );

            memset( (char *)kjsvpt->stringsv, J_NULL, convlen );

            length = kjsvpt->kanamap[0];
            memset( (char *)kjsvpt->kanamap, J_NULL, length );
            kjsvpt->kanamap[0] = 0x01;

            length = kjsvpt->grammap[0];
            memset( (char *)kjsvpt->grammap, J_NULL, length );
            kjsvpt->grammap[0] = 0x01;

            memset( (char *)kjsvpt->kanadata, J_NULL, kjsvpt->kanalen );

            length = CHPTTOSH( kjsvpt->kjdata1s );
            memset( (char *)kjsvpt->kjdata1s, J_NULL, length );
            SHTOCHPT( kjsvpt->kjdata1s, C_DBCS );

            length = CHPTTOSH( kjsvpt->kjmap1s );
            memset( (char *)kjsvpt->kjmap1s, J_NULL, length );
            SHTOCHPT( kjsvpt->kjmap1s, C_DBCS );

            length = kjsvpt->gramap1s[0];
            memset( (char *)kjsvpt->gramap1s, J_NULL, length );
            kjsvpt->gramap1s[0] = 0x01;

            kjsvpt->convpos  =
            kjsvpt->convlen  =
            kjsvpt->cconvpos =
            kjsvpt->cconvlen =
            kjsvpt->savepos  =
            kjsvpt->savelen  =
            kjsvpt->kanalen  =
            kjsvpt->preccpos = 0;


            memset( (char *)&pt->hlatst[convpos], K_HLAT0, convlen );

            _Msetch( pt, convpos, convlen );

            kjsvpt->actc3 = A_1STINP;

            return( IMSUCC );
        };
/* #(E) 1987.12.17. Flying Conversion Add */



        /* 3.
         *      Set     Kanji data string
         *          and Kanji map
         *          and Kana map
         *          and Grramer map
         *          and Kanjo conversion code map.
         */
        switch ( kjsvpt->tankan ) {

            case C_SWON :       /* Tankan flag ON.                      */

                /*
                 *      Set kanji data string.
                 */
                memcpy( (char *)kkcbsvpt->kjdata,
                        (char *)&kjdata[kjpos],
                        kkcbsvpt->kjlen );

                /*
                 *      Set conversion string strat position.
                 */
                convpos  = kjsvpt->convpos;

                /*
                 *      Set conevrsion string length.
                 */
                convlen  = kjsvpt->convlen;

                /*
                 *      Set current conversion string strat position.
                 */
                cconvpos = kjsvpt->cconvpos;

                /*
                 *      Set current conversion string length.
                 */
                cconvlen = kjsvpt->cconvlen;

                /*
                 *      Set Kanji conversion map position.
                 */
                kjcvmpps = cconvpos - convpos;

                /*
                 *      Get Kana data.
                 */
                rc = _Mkanagt( pt, kjcvmpps,
                               &kanapos, &kanalen, &jiritsu, &maplen );

                if ( jiritsu && (kanalen != jiritsu) ) {

                    bcount = 1; /* Set Kana map check position.         */

                    mcount = 7; /* Set shift counter.                   */

                    /*
                     *      Operate Kana data from first position
                     *          to last position.
                     */
                    for ( i=0 ; i<kjsvpt->kanalen ; i++ ) {

                        /*
                         *      Set Kana map flag.
                         */
                        kjsvpt->iws2[i] =
                            (kjsvpt->kanamap[bcount] >> mcount) & C_SB01;

                        if ( mcount ) {

                            mcount--;   /* Secrease shift counter.      */
                        } else {

                            mcount = 7; /* Reset shift counter.         */
                            bcount++;   /* Increase Kana map position.  */
                        };
                    };



                    for ( i=kanapos+1 ; i<(kanapos + kanalen) ; i++ ) {

                        kjsvpt->iws2[i] = 0x00; /* Reset Kana map flag. */
                    };

                    for ( i=0 ; i<kjsvpt->kanabmax ; i++ ) {

                        /*
                         *      Clear kana map save area.
                         */
                        kjsvpt->iws1[i] = 0x00;
                    };

                    bcount = 1; /* Set Kana map check position.         */

                    mcount = 7; /* Set shift counter.                   */

                    for ( i=0 ; i<kjsvpt->kanalen ; i++ ) {

                        kjsvpt->iws1[bcount] |= (kjsvpt->iws2[i] << mcount);

                        if ( mcount ) {

                            mcount--;   /* Decrease shift counter.      */

                        } else {

                            /*
                             *      Set Kana map.
                             */
                            kjsvpt->kanamap[bcount] &= kjsvpt->iws1[bcount];

                            mcount = 7; /* Reset shift counter.         */

                            bcount++;   /* Increase Kana map position.  */
                        };
                    };

                    if ( mcount ) {

                        /*
                         *      Set kana map.
                         */
                        kjsvpt->kanamap[bcount] &= kjsvpt->iws1[bcount];
                    };
                };


                /*
                 *      Set Kanji conversion map.
                 */
                kjcvmap  = kjsvpt->kjcvmap;

                kjsvpt->kjcvmap[kjcvmpps+C_ANK] = M_KSCNSK;

                /*
                 *      Check current conversion string length.
                 */
                if ( cconvlen > C_DBCS ) {

                    /*
                     *      Set Kanji conversion map move position.
                     */
                    movpos = kjcvmpps + C_DBCS;

                    /*
                     *      Set Kanji conversion map move length.
                     */
                    movlen = (convpos + convlen) - (cconvpos + cconvlen);

                    /*
                     *      Check Kanji conversion map move length.
                     */
                    if ( movlen ) {

                        /*
                         *      Move Kanji conversion map.
                         */
                        memcpy( (char *)&kjsvpt->kjcvmap[movpos],
                                (char *)&kjcvmap[kjcvmpps+cconvlen],
                                movlen);

                        /*
                         *      Set Kanji conversion map.(NULL)
                         */
                        memset( (char *)&kjsvpt->kjcvmap[movpos+movlen],
                                NULL, cconvlen-C_DBCS);
                    } else {

                        /*
                         *      Set Kanji conversion map.(NULL)
                         */
                        memset( (char *)&kjsvpt->kjcvmap[movpos],
                                NULL, cconvlen-C_DBCS);
                    };
                };
                break;


            case C_SWOFF :      /* Tankan flag OFF.                     */

                /*
                 *      Set kanji data string.
                 */
                memcpy( (char *)kkcbsvpt->kjdata,
                        (char *)&kjdata[kjpos],
                        kkcbsvpt->kjlen);

                kjmap   = kkcbsvpt->kjmap;      /* Set Kanji map.       */

                kjmapps = 0;    /* Initialize Kanji map data length.    */

                grammap = kkcbsvpt->grammap;    /* Set Grammer map.     */

                grmapps = 0;    /* Initialize Grammer map data length.  */

                kanamap = kkcbsvpt->kanamap;    /* Set Kana map.        */

                knmapps = 0;    /* Initialize Kana map data length.     */

                for ( i=0 ; i<kkcbsvpt->rsnumcnd ; i++ ) {

                    if ( i == inpnum ) {

                        /*
                         *      Set Kanji map length.
                         */
                        kkcbsvpt->kjmapln = CHPTTOSH( &kjmap[kjmapps] );

                        /*
                         *      Set Kanji map string.
                         */
                        memcpy( (char *)kkcbsvpt->kjmap,
                                (char *)&kjmap[kjmapps],
                                kkcbsvpt->kjmapln);

                        /*
                         *      Set Grammer map length.
                         */
                        kkcbsvpt->grmapln = (short)grammap[grmapps];

                        /*
                         *      Set Grammer map string.
                         */
                        memcpy( (char *)kkcbsvpt->grammap,
                                (char *)&grammap[grmapps],
                                kkcbsvpt->grmapln);

                        /*
                         *      Set Kana map length.
                         */
                        kkcbsvpt->knmapln = (short)kanamap[knmapps];

                        /*
                         *      Set Kana map string.
                         */
                        memcpy( (char *)kkcbsvpt->kanamap,
                                (char *)&kanamap[knmapps],
                                kkcbsvpt->knmapln);

                        break;
                    };

                    /*
                     *      Set Kanji map position.
                     */
                    kjmapps += CHPTTOSH( &kjmap[kjmapps] );

                    /*
                     *      Set Grammer map position.
                     */
                    grmapps += (short)grammap[grmapps];

                    /*
                     *      Set Kana map position.
                     */
                    knmapps += (short)kanamap[knmapps];
                };

                /*
                 *      Copy Kana Kanji Control Block data in
                 *      Kanji Monitor Internal Save Area.
                 */
                rc = _Mnxprps( pt );

                break;
        };



        /* 4.
         *      Field message reset.
         */
        rc = _Mfmrst( pt, K_MSGOTH );



        /* 5.
         *      Next/Pre_candidate conevrsion.
         */
        rc = _Mrsstrh(pt);



        /* 6.
         *      Chenge mode.
         */

        /* 6.1.
         *      Set action code No.3.
         */
        kjsvpt->actc3 = kjsvpt->kkmode2;


        /* 6.2.
         *      Check keyboard lock flag.
         */
        if ( pt->kbdlok == K_KBLON ) {

            kjsvpt->kkmode1 = kjsvpt->kkmode2;
        };



        /* 7.
         *      Return.
         */



/*********************** Control Block Snap. ****************************/
/*                                                                      */
/*      SNAP AREA:      Kanji Control Block.                            */
/*                      Kanji Monitor Internal Save Area.               */
/*                      Kana Kanji Control Block.                       */
/*                                                                      */
/************************************************************************/

        snap3(SNAP_KCB | SNAP_KMISA | SNAP_KKCB, SNAP_ME_0a, "Return");



        return( IMSUCC );
}
