static char sccsid[] = "@(#)81	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Macifst.c, libKJI, bos411, 9428A410j 7/23/92 03:20:30";
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
 * MODULE NAME:         _Macifst
 *
 * DESCRIPTIVE NAME:    Display All candidates in Input field.
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
 * FUNCTION:            Make All candidates message string.
 *                      Display All candidates.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1968 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Macifst
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Macifst( pt )
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IMSUCC  :Success of Execution.
 *
 * EXIT-ERROR:          NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _Maxmst :Auxiliary area No.1 message set.
 *                              _Mifmst :Input field message set.
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
 *   INPUT:             Kanji Monitor Control Block(KCB).
 *                              kjsvpt
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              kkcbsvpt        alcancol        allcanfg
 *                              tankan
 *                      Kana Kanji Controle Block(KKCB).
 *                              candbotm        kjdata          kjmap
 *                              rsnumcnd        totalcan
 *
 *   OUTPUT:            Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              iws3
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              IDENTIFY:Module Identify Create.
 *                              DBCSTOCH:Set DBCS character to pointed area.
 *                              CHPTTOSH:Set Short data to character
 *                                       pointerd area.
 *                      Standard Macro Library.
 *                              NA.
 *
 * CHANGE ACTIVITY:     Sept. 29 1988 Satoshi Higuchi
 *                      Changed some statements to use immediate vale
 *                      that is 100, to M_CANDNM.
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
 *      Make All candidates message string.
 *      Display All candidates.
 */
int     _Macifst( pt )

register KCB    *pt;            /* Pointer to Kanji Control Block       */
{
        register KMISA  *kjsvpt;/* Pointer to Kana Kanji Monitor
                                   Internal Save Area.                  */
        register KKCB   *kkcbsvpt;
                                /* Pointer to Kana Kanji Control Block. */

        int     _Mifmst();      /* Input field message data set.        */
        int     _Maxmst();      /* Auxiliary area No.1 message data set.*/

        char    *memcpy();      /* Memory copy.                         */

        int     rc;             /* Return code.                         */

        int     i,j;            /* Loop counter.                        */

        static uchar    chnumtbl[10][2] = { 0x82 , 0x4f , 0x82 , 0x50 ,
                                            0x82 , 0x51 , 0x82 , 0x52 ,
                                            0x82 , 0x53 , 0x82 , 0x54 ,
                                            0x82 , 0x55 , 0x82 , 0x56 ,
                                            0x82 , 0x57 , 0x82 , 0x58 };
                                /* Number character table.              */

        short   restnum;        /* Rest number.                         */
        short   ranknum;        /* Rest number( rank ).                 */
        short   rankflg;        /* Rest number wright flag.             */

        short   length;         /* Message string length.               */

        short   setpos;         /* Current String Position              */

        short   kjdatpos;       /* Kanji data position.                 */
        short   kjdatlen;       /* Kanji data length.                   */
        short   kjmappos;       /* Kanji map position.                  */
        short   kjmaplen;       /* Kanji map length.                    */
        short   jiritsu;        /* Jiritsugo length.                    */



/*********************** Control Block Snap. ****************************/
/*                                                                      */
/*      SNAP AREA:      Kanji Monitor Internal Save Area.               */
/*                      Kana Kanji Control Block.                       */
/*                                                                      */
/************************************************************************/

        snap3( SNAP_KMISA | SNAP_KKCB, SNAP_Macifst, "Start");



        kjsvpt   = pt->kjsvpt;
        kkcbsvpt = kjsvpt->kkcbsvpt;



        /* 1.
         *      Set All candidates to Message string area.
         */

        /*
         *      Set message string length.
         */
        length = kjsvpt->alcancol + sizeof(M_ACIFMG) - 1;

        setpos = 0;     /* Initialize message setting position.         */


        /* 1.1.
         *      Set Kanji data.
         */

        /*
         *      Tankan flag check.
         */
        if ( kjsvpt->tankan == C_SWOFF ) {      /* Tankan flag OFF.     */

            kjdatpos = 0;       /* Initialize Kanji data position.      */

            kjmappos = 0;       /* Initialize Kanji data map position.  */

            /*
             *      Set All candidates Kanji data.
             */
            for ( i=0 ; i<kkcbsvpt->rsnumcnd ; i++ ) {

                /*
                 *      Set All candidates serect number.
                 */
                kjsvpt->iws3[setpos]   = chnumtbl[i][0];
                kjsvpt->iws3[setpos+1] = chnumtbl[i][1];

                setpos += C_DBCS;       /* Message position increase.   */

                /*
                 *      Set Double byte '.' character of PC kanji code.
                 */
                DBCSTOCH( &kjsvpt->iws3[setpos], 0x8144 );

                setpos += C_DBCS;       /* Message position increase.   */

                /*
                 *      Get Kanji data length.
                 */
                kjdatlen = CHPTTOSH( &kkcbsvpt->kjdata[kjdatpos] );

                /*
                 *      Get Kanji map length.
                 */
                kjmaplen = CHPTTOSH( &kkcbsvpt->kjmap[kjmappos] );

                /*
                 *      Get Jiritsugo length.
                 */
                jiritsu = 0;    /* Initialize Jiritsugo length.         */

                for ( j=C_DBCS ; j<kjmaplen ; j++ ) {

                    /*
                     *      Adjust string.
                     */
                    if ( kkcbsvpt->kjmap[kjmappos+j] == M_KJMJAN ) {

                        break;
                    };

                    jiritsu++;  /* Jiritsugo length increase.           */
                };

                jiritsu *= C_DBCS;      /* Set Jiritugo length.         */

                /*
                 *      Set message string( Kanji data ).
                 */
                memcpy( (char *)&kjsvpt->iws3[setpos],
                        (char *)&kkcbsvpt->kjdata[kjdatpos+C_DBCS],
                        jiritsu);

                kjdatpos += kjdatlen;   /* Kanji data position increase.*/

                kjmappos += kjmaplen;   /* Kanji map position increase. */

                setpos += jiritsu;      /* Message position increase.   */

                /*
                 *      Set space character of PC kanji code.
                 */
                DBCSTOCH( &kjsvpt->iws3[setpos], C_SPACE );

                setpos += C_DBCS;       /* message position increase.   */
            };
        } else {                                /* Tankan flag ON.      */

            kjdatpos = 0;       /* Initialize Kanji data position.      */

            /*
             *      Set All candidates Kanji data.
             */
            for ( i=0 ; i<kkcbsvpt->rsnumcnd ; i++ ) {

                /*
                 *      Set All candidates serect number.
                 */
                kjsvpt->iws3[setpos]   = chnumtbl[i][0];
                kjsvpt->iws3[setpos+1] = chnumtbl[i][1];

                setpos += C_DBCS;       /* Message position increase.   */

                /*
                 *      Set Double byte '.' character of PC kanji code.
                 */
                DBCSTOCH( &kjsvpt->iws3[setpos], 0x8144 );

                setpos += C_DBCS;       /* Message position increase.   */

                kjdatpos += C_DBCS;     /* Kanji data position increase.*/

                /*
                 *      Set message string( Kanji data ).
                 */
                kjsvpt->iws3[setpos]   = kkcbsvpt->kjdata[kjdatpos];
                kjsvpt->iws3[setpos+1] = kkcbsvpt->kjdata[kjdatpos+1];

                kjdatpos += C_DBCS;     /* Kanji data position increase.*/

                setpos += C_DBCS;       /* Message position increase.   */

                /*
                 *      Set space character of PC kanji code.
                 */
                DBCSTOCH( &kjsvpt->iws3[setpos], C_SPACE );

                setpos += C_DBCS;       /* Message position increase.   */
            };
        };


        /* 1.2.
         *      Set space character of PC kanji code.
         */

        if ( setpos < kjsvpt->alcancol ) {

            for ( i=setpos ; i<kjsvpt->alcancol ; i+=C_DBCS ) {

                /*
                 *      Set space character of PC kanji code.
                 */
                DBCSTOCH( &kjsvpt->iws3[i], C_SPACE );
            };
        };



        /* 2.
         *      Set message string.
         */

        setpos = kjsvpt->alcancol;      /* Set message position.        */

        memcpy( (char *)&kjsvpt->iws3[setpos],
                M_ACIFMG,
                sizeof(M_ACIFMG)-1);    /* Set message string.          */

        setpos += C_DBCS;       /* Message position increase.           */


        /* 2.1.
         *      Tankan flag check.
         */
        if ( kjsvpt->tankan == C_SWOFF ) {      /* Tankan flag OFF.     */

            /*
             *      Clear of Tankna message.
             */
            DBCSTOCH( &kjsvpt->iws3[setpos], C_SPACE );
        };

        setpos += C_DCHR;       /* Message position increase.           */


        /* 2.2.
         *      Set rest number.
         */

        rankflg = C_SWOFF;      /* Initialize rank wrighting flag.      */

        /*
         *      Get rest number.
         */
        restnum = kkcbsvpt->totalcan - kkcbsvpt->candbotm;

        /*
         *      Set rest number ( Hundred rank ).
         */
	ranknum = (restnum - restnum % M_CANDNM ) / M_CANDNM;

        if ( ranknum ) {

            /*
             *      Set number letter of PC kanji code.
             */
            kjsvpt->iws3[setpos]       = chnumtbl[ranknum][0];
            kjsvpt->iws3[setpos+C_ANK] = chnumtbl[ranknum][1];

            rankflg = C_SWON;   /* Set rank wrightting flag.            */
        };

        setpos += C_DBCS;       /* Message position increase.           */

	restnum = restnum - ranknum * M_CANDNM; /* Get rest number.     */

        /*
         *      Set rest number ( Ten tank ).
         */
        ranknum = ( restnum - restnum % 10 ) / 10;

        if ( rankflg || ranknum ) {

            /*
             *      Set number letter of PC kanji code.
             */
            kjsvpt->iws3[setpos]       = chnumtbl[ranknum][0];
            kjsvpt->iws3[setpos+C_ANK] = chnumtbl[ranknum][1];
        };

        setpos += C_DBCS;       /* Message position increase.           */

        /*
         *      Get rest number ( One tank ).
         */
        ranknum = restnum - ranknum * 10;

        /*
         *      Set number letter of PC kanji code.
         */
        kjsvpt->iws3[setpos]       = chnumtbl[ranknum][0];
        kjsvpt->iws3[setpos+C_ANK] = chnumtbl[ranknum][1];



        /* 3.
         *      Display All candidates.
         */
        switch( kjsvpt->allcanfg ) {

        /* 3.1.
         *      Display All candidates in
         *          Auxiliary area No.1 with single-row.
         */
        case M_ACAX1S :

            /*
             *      Set Auxiliary area No.1 message.
             */
            rc = _Maxmst( pt, K_MSGOTH, C_AUX1, length, M_ACAX1R,
                          C_FAUL, C_FAUL, C_COL, C_DBCS,
                          length, kjsvpt->iws3 );

            break;

        /*
         *      Display All candidates in Input field.
         */
        case M_ACIF :

            /*
             *      Set Input field message.
             */
            rc = _Mifmst( pt, K_MSGOTH, C_FAUL, C_FAUL, C_COL, C_DBCS,
                          length, kjsvpt->iws3 );

            break;
        };


        /* 4.
         *      Return.
         */



/*********************** Control Block Snap. ****************************/
/*                                                                      */
/*      SNAP AREA:      Kanji Monitor Internal Save Area.               */
/*                      Kana Kanji Control Block.                       */
/*                                                                      */
/************************************************************************/

        snap3( SNAP_KMISA | SNAP_KKCB, SNAP_Macifst, "Return");

        return( IMSUCC );
}
