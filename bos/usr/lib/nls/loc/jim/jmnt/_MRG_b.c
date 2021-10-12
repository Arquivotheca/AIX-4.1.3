static char sccsid[] = "@(#)75	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_MRG_b.c, libKJI, bos411, 9428A410j 7/23/92 03:20:07";
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
 * MODULE NAME:         _MRG_b
 *
 * DESCRIPTIVE NAME:    Dictionary registration Kanji input process.
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
 * FUNCTION:            Dictionary registration Kanji input process.
 *                      Dictionary registration process.
 *                      End message set.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        2664 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _MRG_b
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _MRG_b( pt )
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IMSUCC  :Success of Execution.
 *                      IMRGSTI :Information of effective pseudo code input.
 *
 * EXIT-ERROR:          IMRGSTE :Invalid pseudo code input.(Not customized.)
 *                      IMRGIVSE:Invalid string for Dictionary registration.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _MC_rtn :Cursor control routine.
 *                              _MK_rtn :Kana Kanji Conversion process.
 *                              _MM_rtn :Mode switching routine.
 *                              _Mifmst :Input field message set.
 *                              _Mkanasd:DBCS Yomi to Single byte kana.
 *                              _Mregrs :Dictionary registration reset.
 *                              _Mreset :Mode reset process.
 *                              _Kcdctrg:Dictionary registration.
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
 *                              kjsvpt          code            curcol
 *                              lastch          string          type
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              kkcbsvpt        kmpf            curleft
 *                              kjin            kkmode1         msetflg
 *                              regymlen        regyomi         reset
 *
 *   OUTPUT:            Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              kkcflag         kkcrmode        kkmode2
 *                              nextact
 *                      Kana Kanji Control Block(KKCB).
 *                              kanadata        kanalen1        kjdata
 *                              kjlen           rkclen
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              IDENTIFY:Module Identify Create.
 *                      Standard Macro Library.
 *                              NA.
 *
 * CHANGE ACTIVITY:     Tuesday Aug. 23 1988 Satoshi Higuchi
 *                      Modified 3.1.4 Dictionary registration and relation
 *                      parts.
 *                      See Monitor Improvement Specification 3.1.2
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
 *      Dictionary registration Kanji input process.
 *      Dictionary registration process.
 *      End message set.
 */
int     _MRG_b( pt )

register KCB    *pt;            /* Pointer to Kanji Control Block.      */
{
        register KMISA  *kjsvpt;/* Pointer to Kanji Monitor
                                   Internal Save Area.                 */
        register KKCB   *kkcbsvpt;
                                /* Pointer to Kana Kanji Control Block.*/

        int     _MC_rtn();      /* Cursor control routine.              */
        int     _MK_rtn();      /* Kana Kanji Conversion process.       */
        int     _MM_rtn();      /* Mode switching routine.              */
        int     _Mifmst();      /* Input field message set.             */
        int     _Mkanasd();     /* DBCS Yomi to Single byte kana.       */
        int     _Mregrs();      /* Dictionary registration reset.       */
        int     _Mreset();      /* Mode reset process.                  */

        int     _Kcdctrg();     /* Dictionary registration.             */

        char    *memcpy();      /* Copy characters from memory area
                                   A to B.                              */

        int     retcode;        /* Return code.                         */
        int     rc;             /* Return code.                         */

        int     i;              /* Loop counter.                        */

        int kjlen;              /* Kanji data length.                   */

        char    cnvmd;          /* convert mode                        */
        uchar   *sbstr;         /* single byte string                  */
        short   len;            /* length of single byte string        */
        uchar   dbstr[M_RGYLEN];/* double byte string                  */

        short   err_flg = 0;    /* Not customized.                      */
        short   rst_flg1 = 1;   /* Not string.                          */
        short   rst_flg2 = 0;   /* Not string.                          */



/*********************** Control Block Snap. ****************************/
/*                                                                      */
/*      SNAP AREA:      Kanji Control Block.                            */
/*                      Kanji Monitor Internal Save Area.               */
/*                      Kana Kanji Control Block.                       */
/*                                                                      */
/************************************************************************/

        snap3( SNAP_KCB | SNAP_KMISA | SNAP_KKCB, SNAP_MRG_b, "Start");



        kjsvpt   = pt->kjsvpt;
        kkcbsvpt = kjsvpt->kkcbsvpt;

        /*
         *      Initialize return code.
         */

        retcode = IMSUCC;
        rc      = IMSUCC;



        /* 1.
         *      Check Input code type.
         */

        if ( pt->type != K_INESCF ) {           /* Not pseudo code.     */

            return( retcode );
        };



        /* 2.
         *      Check message set flag.
         *      The message is on display.
         */

        if ( kjsvpt->msetflg != K_MSGDIC ) {

            return( retcode );
        };



        /* 3.
         *      Check Input code.
         */

        switch( pt->code ) {

        case P_ENTER :  /* Input code is ENTER.                         */

        case P_ACTION : /* Input code is ACTION.                        */

        case P_CR :     /* Input code is CR.                            */


            kjsvpt->rkclen = 0; /* Reset Remaining romaji string length.*/

            /* 3.1.1.
             *      Check Kanji string.
             */

            /*
             *      Check cursor position.
             *      More than one character exist in input field.
             */
            if ( kjsvpt->curleft != pt->lastch ) {

                /*
                 *      Check Kanji string.
                 */
                for( i=kjsvpt->curleft ; i<pt->lastch ; i+=C_DBCS ) {

                    /*
                     *      Kanji string is Space.
                     */
                    if (  (pt->string[i]   != C_SPACEH) ||
                          (pt->string[i+1] != C_SPACEL)  ) {

                        rst_flg1 = 0;   /* Set reset No.1 flag.(OFF)    */

                        break;
                    };
                };
            };

            if ( rst_flg1 ) {

                /*
                 *      Check profile parameter kanji input key
                 *      and profile parameter message reset.
                 */

                switch( pt->code ) {

                case P_ENTER :  /* Input code is ENTER.                 */

                    if (  ((kjsvpt->kmpf[0].reset & K_REENT) == K_REENT) ||
                          ((kjsvpt->kmpf[0].kjin  & K_DAENT) == K_DAENT)  ) {

                        rst_flg2 = 1;   /* Set reset No.2 flag.         */
                    } else {

                        err_flg = 1;    /* Set error flag.              */
                    };
                    break;

                case P_ACTION : /* Input code is ACTION.                */

                    if (  ((kjsvpt->kmpf[0].reset & K_REACT) == K_REACT) ||
                          ((kjsvpt->kmpf[0].kjin  & K_DAACT) == K_DAACT)  ) {

                        rst_flg2 = 1;   /* Set reset No.2 flag.         */
                    } else {

                        err_flg = 1;    /* Set error flag.              */
                    };
                    break;

                case P_CR :     /* Input code is CR.                    */

                    if (  ((kjsvpt->kmpf[0].reset & K_RECR) == K_RECR) ||
                          ((kjsvpt->kmpf[0].kjin  & K_DACR) == K_DACR)  ) {

                        rst_flg2 = 1;   /* Set reset No.2 flag.         */
                    } else {

                        err_flg = 1;    /* Set error flag.              */
                    };
                    break;
                };

                if ( rst_flg2 ) {

                    rc = _Mregrs( pt ); /* Dictionary registration reset.*/

                    retcode = IMRGIVSE; /* Set return code.             */
                };

                if ( err_flg ) {

                    rc = _MM_rtn( pt, A_BEEP ); /* Beep.                */

                    retcode = IMRGSTE;  /* Set return code.             */
                };
                break;
            };


            /* 3.1.2.
             *      Kanji input code customized.
             */

            switch( pt->code ) {

            case P_ENTER :      /* Input code is ENTER.                 */

                if ( (kjsvpt->kmpf[0].kjin & K_DAENT) != K_DAENT ) {

                    err_flg = 1;        /* Set error flag.              */
                };
                break;

            case P_ACTION :     /* Input code is ACTION.                */

                if ( (kjsvpt->kmpf[0].kjin & K_DAACT) != K_DAACT ) {

                    err_flg = 1;        /* Set error flag.              */
                };
                break;

            case P_CR :
            /* Input code is CR.                                        */

                if ( (kjsvpt->kmpf[0].kjin & K_DACR) != K_DACR ) {

                    err_flg = 1;        /* Set error flag.              */
                };
                break;
            };

            if ( err_flg ) {

                rc = _MM_rtn( pt, A_BEEP );     /* Beep.                */

                retcode = IMRGSTE;      /* Set return code.             */

                break;
            };


            /* 3.1.3.
             *      Check of Kanji string length.
             */

            /*
             *      Out of Kanji string length.
             */

            /*
             *      String length search.
             */
            kjlen = _Mstrl( pt ) - kjsvpt->curleft;

            if ( kjlen > M_RGIFL ) {

                /*
                 *      Set Error message.
                 */
                rc = _Mifmst( pt, K_MSGOTH, C_FAUL, C_FAUL, C_COL, C_DBCS,
                              sizeof(M_RGKEMG)-1, M_RGKEMG );

                /*
                 *      Reset next function.
                 *          (Dictionary registration Kanji)
                 */
                kjsvpt->nextact &= ~M_RGBON;

                /*
                 *      Set next function.
                 *          (Dictionary registration message reset)
                 */
                kjsvpt->nextact |= M_RMRSON;

                /*
                 *      Previous conversion mode set.
                 */
                kjsvpt->kkmode2 = kjsvpt->kkmode1;

                retcode = IMRGIVSE;     /* Set return code.             */

                break;
            };

/*----------------------------------------------------------------------*/
/*      #(B) Deleted by S,Higuchi on Aug. 23 1988                       */
/*                                                                      */
/*      Below statments sets Kana data length, Kana data, Kanji data    */
/*      length and Kanji data into KKCB before checking Kana data and   */
/*      Kanji data idetify.                                             */
/*                                                                      */
/*           * 3.1.4.                                                   */
/*           *      Dictionary registration.                            */
/*           *                                                          */
/*                                                                      */
/*           *                                                          */
/*           *      Set kana data length.                               */
/*           *                                                          */
/*          kkcbsvpt->kanalen1 = kjsvpt->regymlen;                      */
/*                                                                      */
/*           *                                                          */
/*           *      Set Kana data.                                      */
/*           *                                                          */
/*          memcpy( (char *)kkcbsvpt->kanadata,                         */
/*                  (char *)kjsvpt->regyomi,                            */
/*                  kkcbsvpt->kanalen1);                                */
/*                                                                      */
/*           *                                                          */
/*           *      Set Kanji data length.                              */
/*           *                                                          */
/*          kkcbsvpt->kjlen = kjlen + C_DBCS;                           */
/*                                                                      */
/*           *                                                          */
/*           *      Set Kanji data.                                     */
/*           *                                                          */
/*          SHTOCHPT( kkcbsvpt->kjdata, kkcbsvpt->kjlen );              */
/*          memcpy( (char *)&kkcbsvpt->kjdata[C_DBCS],                  */
/*                  (char *)&pt->string[kjsvpt->curleft],               */
/*                  kkcbsvpt->kjlen-C_DBCS);                            */
/*----------------------------------------------------------------------*/

            err_flg = C_SWOFF;  /* Reset error flag.                    */

            switch ( kjsvpt->regyomi[0] ) {

            case M_NESC :       /* Alphabet/Numeric Yomi string.        */

/*----------------------------------------------------------------------*/
/*      #(B) Deleted by S,Higuchi on Aug. 23 1988                       */
/*              if ( (kkcbsvpt->kanalen1 - C_ANK) == (kjlen / C_DBCS) ){*/
/*----------------------------------------------------------------------*/

		/* #(B) Added by S,Higuchi on Aug. 23 1988              */
		if((kjsvpt->regymlen - C_ANK) == (kjlen / C_DBCS)) {

                    err_flg = C_SWON;

                    cnvmd = M_ALPHCV;

/*----------------------------------------------------------------------*/
/*      #(B) Deleted by S,Higuchi on Aug. 23 1988                       */
/*                  sbstr = (uchar *)kkcbsvpt->kanadata + C_ANK;        */
/*----------------------------------------------------------------------*/

		    /* #(B) Added by S,Higuchi on Aug. 23 1988          */
		    sbstr = (uchar *)kjsvpt->regyomi + C_ANK;

/*----------------------------------------------------------------------*/
/*      #(B) Deleted by S,Higuchi on Aug. 23 1988                       */
/*                  len = kkcbsvpt->kanalen1 - C_ANK;                   */
/*----------------------------------------------------------------------*/

		    /* #(B) Added by S,Higuchi on Aug. 23 1988          */
		    len = kjsvpt->regymlen;

                    rc = _Mkanasd( cnvmd, sbstr, len, dbstr );

                    for ( i=0 ; i<kjlen ; i++ ) {

/*----------------------------------------------------------------------*/
/*      #(B) Deleted by S,Higuchi on Aug. 23 1988                       */
/*                      if ( dbstr[i] != kkcbsvpt->kjdata[i+C_DBCS] ) { */
/*----------------------------------------------------------------------*/

			/* #(B) Added by S,Higuchi on Aug. 23 1988      */
			if(dbstr[i] != pt->string[kjsvpt->curleft + i]) {

                            err_flg = C_SWOFF;
                            break;
                        };
                    };
                };
                break;

            default :

/*----------------------------------------------------------------------*/
/*      #(B) Deleted by S,Higuchi on Aug. 23 1988                       */
/*              if ( kkcbsvpt->kanalen1 == (kjlen / C_DBCS) ) {         */
/*----------------------------------------------------------------------*/

		/* #(B) Added by S,Higuchi Aug. 23 1988                 */
		if(kjsvpt->regymlen == (kjlen / C_DBCS)) {

                    err_flg = C_SWON;

                    cnvmd = M_HIRACV;

/*----------------------------------------------------------------------*/
/*      #(B) Deleted by S,Higuchi on Aug. 23 1988                       */
/*                  sbstr = (uchar *)kkcbsvpt->kanadata;                */
/*----------------------------------------------------------------------*/

		    /* #(B) Added by S,Higuchi on Aug. 23 1988          */
		    sbstr = (uchar *)kjsvpt->regyomi;

/*----------------------------------------------------------------------*/
/*      #(B) Deleted by S,Higuchi on Aug. 23 1988                       */
/*                  len = kkcbsvpt->kanalen1;                           */
/*----------------------------------------------------------------------*/

		    /* #(B) Added by S,Higuchi on Aug. 23 1988          */
		    len = kjsvpt->regymlen;

                    rc = _Mkanasd( cnvmd, sbstr, len, dbstr );

                    for ( i=0 ; i<kjlen ; i++ ) {

/*----------------------------------------------------------------------*/
/*      #(B) Deleted by S,Higuchi on Aug. 23 1988                       */
/*                      if ( dbstr[i] != kkcbsvpt->kjdata[i+C_DBCS] ) { */
/*----------------------------------------------------------------------*/

			/* #(B) Added by S,Higuchi on Aug. 23 1988      */
			if(dbstr[i] != pt->string[kjsvpt->curleft + i]) {

                            err_flg = C_SWOFF;
                            break;
                        };
                    };
                };
                break;
            };

            if ( !err_flg ) {

		/* #(B) Added by S,Higuchi on Aug. 23 1988              */
		/* 3.1.4.
		 *      Dictionary registration.
		 */

		/*
		 *      Set kana data length.
		 */
		 kkcbsvpt->kanalen1 = kjsvpt->regymlen;

		/*
		 *      Set Kana data.
		 */
		 memcpy( (char *)kkcbsvpt->kanadata,
		       (char *)kjsvpt->regyomi,
		       kkcbsvpt->kanalen1);

		/*
		 *      Set Kanji data length.
		 */
		 kkcbsvpt->kjlen = kjlen + C_DBCS;

		/*
		 *      Set Kanji data.
		 */
		 SHTOCHPT( kkcbsvpt->kjdata, kkcbsvpt->kjlen );
		 memcpy( (char *)&kkcbsvpt->kjdata[C_DBCS],
		       (char *)&pt->string[kjsvpt->curleft],
		       kkcbsvpt->kjlen-C_DBCS);

		/* #(B) Added end                                       */

		kjsvpt->kkcflag = M_KKNOP;  /* Set KKC calling flag.    */

                /*
                 *      Dictionary registration.
                 */
                kjsvpt->kkcrc = _Kcdctrg( kkcbsvpt );

                /*
                 *      Check return code.
                 */
                if ( KKCPHYER( kjsvpt->kkcrc ) ) {

                    return( kjsvpt->kkcrc );
                } else {

                    switch( KKCMAJOR( kjsvpt->kkcrc ) ) {

                    case K_KCSUCC :

                        break;

                    case K_KCDCTE :

                        err_flg = C_SWON;
                        break;

                    default :

                        return( kjsvpt->kkcrc );
                    };
                };
            };



            if ( err_flg ) {

                /*
                 *      Set Error message.
                 */
                rc = _Mifmst( pt, K_MSGOTH, C_FAUL, C_FAUL, C_COL, C_DBCS,
                              sizeof(M_RGEMG)-1, M_RGEMG);

                /*
                 *      Reset next function.
                 *          (Dictionary registration Kanji)
                 */
                kjsvpt->nextact &= ~M_RGBON;

                /*
                 *      Set next function.
                 *          (Dictionary registration message reset)
                 */
                kjsvpt->nextact |= M_RMRSON;

                /*
                 *      Previous conversion mode set.
                 */
                kjsvpt->kkmode2 = kjsvpt->kkmode1;

                retcode = IMRGIVSE;     /* Set return code.             */

                break;
            };


            /* 3.1.5.
             *      Set End message.
             */

            /*
             *      Set end message.
             */
            rc = _Mifmst( pt, K_MSGDIC, C_FAUL, C_FAUL, C_COL, C_DBCS,
                          sizeof(M_RGEND)-1, M_RGEND );


            /* 3.1.6.
             *      Dictionary registration flag set.
             */

            /*
             *      Reset next function.
             *          (Dictionary registration Kanji)
             */
            kjsvpt->nextact &= ~M_RGBON;

            /*
             *      Set next function.
             *          (Dictionary registration Message)
             */
            kjsvpt->nextact |= M_RGCON;

            /*
             *      Set Dictionary registration message mode.
             */
            kjsvpt->kkcrmode = K_MEDIC;

            retcode = IMRGSTI;  /* Set return code.                     */

            break;


        /* 3.2.
         *      Input code is RESET.
         */
        case P_RESET :

            kjsvpt->rkclen = 0; /* Reset Remaining romaji string length.*/

            /*
             *      Mode reset process.
             */
            rc = _Mreset( pt, M_ALLRST );

            /*
             *      Check return code.
             */
            if ( rc == IMINSI ) {

                /*
                 *      Check message set flag.
                 *      The message is on display.
                 */
                if ( kjsvpt->msetflg == K_MSGDIC ) {

                    /*
                     *      To fix converted character.
                     */
                    rc = _MK_rtn( pt, A_CNVDEC );

                    /*
                     *      Changed First input mode.
                     */
                    rc = _MM_rtn( pt, A_1STINP );
                };
            };

            retcode = IMRGSTI;  /* Set return code.                     */

            break;


        /* 3.3.
         *      Input code is TAB.
         */
        case P_TAB :

            kjsvpt->rkclen = 0; /* Reset Remaining romaji string length.*/

            rc = _MM_rtn( pt, A_BEEP ); /* Beep.                        */

            retcode = IMRGSTI;  /* Set return code.                     */

            break;


        /* 3.4.
         *      Input code is BACK TAB.
         */
        case P_BTAB :

            kjsvpt->rkclen = 0; /* Reset Remaining romaji string length.*/

            /*
             *      Check cursor position.
             */

            /*
             *      Cursor at the top of input field.
             */
            if ( pt->curcol == kjsvpt->curleft ) {

                rc = _MM_rtn( pt, A_BEEP );     /* Beep.                */
            } else {

                /*
                 *      To fix converted character.
                 */
                rc = _MK_rtn( pt, A_CNVDEC );

                /*
                 *      Changed First input mode.
                 */
                rc = _MM_rtn( pt, A_1STINP );

                /*
                 *      To move cursor at the top of input field.
                 */
                rc = _MC_rtn( pt, A_CIFS );
            };

            retcode = IMRGSTI;  /* Set return code.                     */

            break;


        /* 3.5.
         *      Default.
         */
        default :
            break;
        };



        /* 4.
         *      Return.
         */


/*********************** Control Block Snap. ****************************/
/*                                                                      */
/*      SNAP AREA:      Kanji Control Block.                            */
/*                      Kanji Monitor Internal Save Area.               */
/*                      Kana Kanji Control Block.                       */
/*                                                                      */
/************************************************************************/

        snap3( SNAP_KCB | SNAP_KMISA | SNAP_KKCB, SNAP_MRG_b, "Return" );

        return( retcode );
}
