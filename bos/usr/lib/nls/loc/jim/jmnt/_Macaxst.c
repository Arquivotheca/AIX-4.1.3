static char sccsid[] = "@(#)80	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Macaxst.c, libKJI, bos411, 9428A410j 7/23/92 03:20:26";
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
 * MODULE NAME:         _Macaxst
 *
 * DESCRIPTIVE NAME:    Display All candidates in Auxiliary area No.1.
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
 *                      Diaplay All candidates.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1916 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Macaxst
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Macaxst( pt )
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
 *                              kkcbsvpt        alcancol        alcanrow
 *                              tankan
 *                      Kana Kanji Controle Block(KKCB).
 *                              candbotm        kjdata          rsnumcnd
 *                              totalcan
 *
 *   OUTPUT:            Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              iws4
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
 * CHANGE ACTIVITY:     Bug Fix.
 *                      1988.02.23. : Change Kanji data Copy length.
 *
 *                      Sept. 29 1988 Satoshi Higuchi
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
int     _Macaxst( pt )

register KCB    *pt;    /* Pointer to Kanji Control Block.              */
{
        register KMISA  *kjsvpt;/* Pointer to Kanji Monitor
                                   Internal Save Area.                  */
        register KKCB   *kkcbsvpt;
                                /* Pointer to Kana Kanji Control Block. */

        int     _Maxmst();      /* Auxiliary area No.1 message data set.*/

        char    *memcpy();      /* Copy characters from memory area
                                   A to B.                              */

        int     rc;             /* Return code.                         */

        int     i,j;            /* Loop counter.                        */

        short   first;          /* Loop first position.                 */
        short   loop;           /* Loop last position.                  */

        static uchar    chnumtbl[10][2] = { 0x82 , 0x4f , 0x82 , 0x50 ,
                                            0x82 , 0x51 , 0x82 , 0x52 ,
                                            0x82 , 0x53 , 0x82 , 0x54 ,
                                            0x82 , 0x55 , 0x82 , 0x56 ,
                                            0x82 , 0x57 , 0x82 , 0x58 };
                                /* Number character table.              */

        short   restnum;        /* Rest number.                         */
        short   ranknum;        /* Rest number( rank ).                 */
        short   rankflg;        /* Rest number wright flag.             */

        short   length;         /* Length of message data.              */

        short   toppos;         /* Row top position.                    */

        short   setpos;         /* message setting position.            */

        short   msglen;         /* All candidates message length.       */

        short   kjdatpos;       /* Position of Kanji data.              */
        short   kjdatlen;       /* Length of Kanji data.                */
/************************************************************************/
/* #(B) 1988.02.23. Bug Fix.                                            */
/*      Add Reason.                                                     */
/*          short   datcplen;                                           */
/************************************************************************/
        short   datcplen;       /* Kanji Data Copy Length.              */


/*********************** Control Block Snap. ****************************/
/*                                                                      */
/*      SNAP AREA:      Kanji Monitor Internal Save Area.               */
/*                      Kana Kanji Control Block.                       */
/*                                                                      */
/************************************************************************/

        snap3( SNAP_KMISA | SNAP_KKCB, SNAP_Macaxst, "Start");



        kjsvpt   = pt->kjsvpt;
        kkcbsvpt = kjsvpt->kkcbsvpt;



        /* 1.
         *      Set All candidates to Message string area.
         */

        /*
         *      Set message string length.
         */
        length = kjsvpt->alcancol * (kkcbsvpt->rsnumcnd + M_ACMGFD);


        /* 1.1.
         *      Set message No.1 string.
         */

        toppos = 0;     /* Set row top position.                        */

        msglen = sizeof(M_ACAXM1) - 1;  /* Set message No1. length.     */

        /* 1.1.1.
         *      Set message No.1 string.
         */
        memcpy( (char *)&kjsvpt->iws4[toppos], M_ACAXM1, msglen);


        /* 1.1.2.
         *      Clear of Tankan message.
         */
        if ( kjsvpt->tankan == C_SWOFF ) {      /* Tankan flag OFF.     */

            /*
             *      Set space character of PC kanji code.
             */
            DBCSTOCH( &kjsvpt->iws4[C_DBCS], C_SPACE );
            DBCSTOCH( &kjsvpt->iws4[C_DCHR], C_SPACE );
        };


        /* 1.1.3.
         *      Set rest number.
         */

        setpos = msglen - M_KNJNOL;     /* Set message position.        */

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
            kjsvpt->iws4[setpos]       = chnumtbl[ranknum][0];
            kjsvpt->iws4[setpos+C_ANK] = chnumtbl[ranknum][1];

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
            kjsvpt->iws4[setpos]       = chnumtbl[ranknum][0];
            kjsvpt->iws4[setpos+C_ANK] = chnumtbl[ranknum][1];
        };

        setpos += C_DBCS;       /* Message position increase.           */

        /*
         *      Get rest number ( One tank ).
         */
        ranknum = restnum - ranknum * 10;

        /*
         *      Set number letter of PC kanji code.
         */
        kjsvpt->iws4[setpos]       = chnumtbl[ranknum][0];
        kjsvpt->iws4[setpos+C_ANK] = chnumtbl[ranknum][1];


        /* 1.1.4.
         *      Set space character of PC kanji code.
         */

        first = toppos + msglen;        /* Set loop first position.     */

        loop = toppos + kjsvpt->alcancol;   /* Set loop last position.  */

        for ( i=first ; i<loop ; i+=C_DBCS ) {

            /*
             *      Set space character of PC kanji code.
             */
            DBCSTOCH( &kjsvpt->iws4[i], C_SPACE );
        };

        toppos += kjsvpt->alcancol;     /* Set row top position.        */


        /* 1.2.
         *      Set message No.2 string.
         */

        msglen = sizeof(M_ACAXM2) - 1;  /* Set message No.2 length.     */


        /* 1.2.1.
         *      Set message No.2 string.
         */
        memcpy( (char *)&kjsvpt->iws4[toppos], M_ACAXM2, msglen);


        /* 1.2.2.
         *      Set space character of PC kanji code.
         */

        first = toppos + msglen;        /* Set loop first position.     */

        loop = toppos + kjsvpt->alcancol;   /* Set loop last position.  */

        for ( i=first ; i<loop ; i+=C_DBCS ) {

            /*
             *      Set space character of PC kanji code.
             */
            DBCSTOCH( &kjsvpt->iws4[i], C_SPACE );
        };

        toppos += kjsvpt->alcancol;     /* Set row top position.        */



        /* 2.
         *      Set Kanji data.
         */

        kjdatpos = 0;   /* Initialized Kanji data position.             */

        for ( i=0 ; i<kkcbsvpt->rsnumcnd ; i++ ) {


            /* 2.1.
             *      Set All candidates select number letter of PC kanji code.
             */
            setpos = toppos;    /* Set message position.                */

            for ( j=0 ; j<M_KNJNOL ; j+=C_DBCS ) {

                /*
                 *      Set space character of PC kanji code.
                 */
                DBCSTOCH( &kjsvpt->iws4[setpos+j], C_SPACE );
            };

            setpos += C_DBCS;   /* Message position increase.           */

            /*
             *      Set All candidates select number.
             */
            kjsvpt->iws4[setpos]   = chnumtbl[i][0];
            kjsvpt->iws4[setpos+1] = chnumtbl[i][1];

            setpos += C_DCHR;   /* Message position increase.           */

            /* 2.2.
             *      Set Kanji data length.
             */
            kjdatlen = CHPTTOSH( &kkcbsvpt->kjdata[kjdatpos] ) - C_DBCS;

            kjdatpos += C_DBCS; /* Kanji data position increase.        */

/************************************************************************/
/* #(B) 1988.02.23. Bug Fix.                                            */
/*      Add Reason.                                                     */
/*          datcplen = MIN( kjdatlen, (kjsvpt->alcancol - M_KNJNOL) );  */
/************************************************************************/

            datcplen = MIN( kjdatlen, (kjsvpt->alcancol - M_KNJNOL) );

            /*
             *      Set Kanji data.
             */
/************************************************************************/
/* #(B) 1988.02.23. Bug Fix.                                            */
/*      Modify Reason.                                                  */
/*          memcpy( (char *)&kjsvpt->iws4[setpos],                      */
/*                  (char *)&kkcbsvpt->kjdata[kjdatpos],                */
/*                  kjdatlen);                                          */
/*      Change Reason.                                                  */
/*          memcpy( (char *)&kjsvpt->iws4[setpos],                      */
/*                  (char *)&kkcbsvpt->kjdata[kjdatpos],                */
/*                  datcplen);                                          */
/************************************************************************/

            memcpy( (char *)&kjsvpt->iws4[setpos],
                    (char *)&kkcbsvpt->kjdata[kjdatpos],
                    datcplen);

            kjdatpos += kjdatlen;       /* Kanji data position increase.*/

            /*
             *      Set loop first position.
             */
/************************************************************************/
/* #(B) 1988.02.23. Bug Fix.                                            */
/*      Modify Reason.                                                  */
/*          first = toppos + M_KNJNOL + kjdatlen;                       */
/*      Change Reason.                                                  */
/*          first = toppos + M_KNJNOL + datcplen;                       */
/************************************************************************/

            first = toppos + M_KNJNOL + datcplen;

            /*
             *      Set loop last position.
             */
            loop = toppos + kjsvpt->alcancol;

            for ( j=first ; j<loop ; j+=C_DBCS ) {

                /*
                 *      Set space character of PC kanji code.
                 */
                DBCSTOCH( &kjsvpt->iws4[j], C_SPACE );
            };

            toppos += kjsvpt->alcancol; /* Set row top position.        */
        };


        /* 2.3.
         *      Space padding.
         */

        /*
         *      Set loop last position.
         */
        loop = kjsvpt->alcancol * kjsvpt->alcanrow - length + toppos;

        for ( i=toppos ; i<loop ; i+=C_DBCS ) {

            /*
             *      Set space character of PC kanji code.
             */
            DBCSTOCH( &kjsvpt->iws4[i], C_SPACE );
        };



        /* 3.
         *      Display All candidates in Auxiliary area No.1 .
         */

        /*
         *      Auxiliary area No.1 message set of
         *      All candidates data string.
         */
        rc = _Maxmst( pt, K_MSGOTH, C_AUX1,
                      kjsvpt->alcancol, kjsvpt->alcanrow,
                      C_FAUL, C_FAUL, C_COL, C_DBCS,
                      length, kjsvpt->iws4 );



        /* 4.
         *      Return.
         */



/*********************** Control Block Snap. ****************************/
/*                                                                      */
/*      SNAP AREA:      Kanji Monitor Internal Save Area.               */
/*                      Kana Kanji Control Block.                       */
/*                                                                      */
/************************************************************************/

        snap3( SNAP_KMISA | SNAP_KKCB, SNAP_Macaxst, "Return");

        return( IMSUCC );
}
