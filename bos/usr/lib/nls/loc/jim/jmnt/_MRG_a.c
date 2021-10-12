static char sccsid[] = "@(#)74	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_MRG_a.c, libKJI, bos411, 9428A410j 7/23/92 03:20:03";
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
 * MODULE NAME:         _MRG_a
 *
 * DESCRIPTIVE NAME:    Dictionary registration Yomi input process.
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
 * FUNCTION:            Dictionary registration Yomi input process.
 *                      Dictionary registration Kanji input initialize.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        2120 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _MRG_a
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _MRG_a( pt )
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
 *                              _MM_rtn :Mode switching routine.
 *                              _Mgetchm:Get the character mode.
 *                              _Mifmst :Input field message set.
 *                              _Mregrs :Dictionary registration reset.
 *                              _Mreset :Mode reset.
 *                              _Mstrl  :Search string length.
 *                              _Myomic :7 bit yomi(kana) conversion.
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
 *                              kjsvpt          code            curcol
 *                              indlen          lastch          string
 *                              type
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              kmpf            curleft         kjin
 *                              kkmode1         nextact         realcol
 *                              reset           rkclen
 *
 *   OUTPUT:            Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              kkcrmode        kkmode2         nextact
 *                              regymlen        regyomi
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              IDENTIFY:Module Identify Create.
 *                      Standard Macro Library.
 *                              NA.
 *
 * CHANGE ACTIVITY:     Sept. 28 1988 Satoshi Higuchi
 *                      First, this source wrote array size immediate.
 *                      Changed immediate to defined name that use #define.
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
 *      Dictionary registration Yomi input process.
 *      Dictionary registration Kanji input initialize.
 */
int     _MRG_a( pt )

register KCB    *pt;            /* Pointer to Kanji Control Block.      */
{
        register KMISA  *kjsvpt;/* Pointer to Kana Kanji Monitor
                                   Internal Save Area.                  */

        int     _MC_rtn();      /* Cursor control routine.              */
        int     _MM_rtn();      /* Mode switching routine.              */
        uchar   _Mgetchm();     /* Get the character mode.              */
        int     _Mifmst();      /* Input field message set.             */
        int     _Mregrs();      /* Dictionary registration reset.       */
        int     _Mreset();      /* Mode reset.                          */
        int     _Mstrl();       /* Search string length.                */
        int     _Myomic();      /* 7 bit yomi(kana) conversion.         */

        char    *memcpy();      /* Copy characters from memory area
                                   A to B.                              */

        int     retcode;        /* Return code.                         */
        int     rc;             /* Return code.                         */

        int     i;              /* Loop counter.                        */

        short   err_flg = 0;    /* Not customized.                      */
        short   rst_flg = 1;    /* Not string..                         */

        uchar   rtnmode;        /* Return character mode(_Mgetchm).     */

        int     rtnlen;         /* Return string length(_Mstrl,_Myomic).*/

        uchar   *yomistr;       /* DBCS string pointer.                 */
        short   yomilen;        /* Length of yomistr.                   */
/*======================================================================*/
/* #(B) Sept. 28 1988 Satoshi Higuchi                                   */
/* Changed source.                                                      */
/* Old source.                                                          */
/*      uchar   chgyomi[12];                                            */
/* New source.                                                          */
/*      uchar   chgyomi[M_RGYMA];                                       */
/*======================================================================*/
	uchar   chgyomi[M_RGYMA];/* 7bit changed string area.           */

        uchar   mode;           /* 7bit conversion mode.                */

        short   msglen;         /* Length of Kanji prompt message.      */
        short   rightpos;       /* Kanji input field right end position.*/



/*********************** Control Block Snap. ****************************/
/*                                                                      */
/*      SNAP AREA:      Kanji Control Block.                            */
/*                      Kanji Monitor Internal Save Area.               */
/*                                                                      */
/************************************************************************/

        snap3( SNAP_KCB | SNAP_KMISA, SNAP_MRG_a, "Start");



        kjsvpt  = pt->kjsvpt;

        /*
         *      Initialize return code.
         */

        retcode = IMSUCC;
        rc      = IMSUCC;



        /* 1.
         *      Check Iput code type.
         */

        if ( pt->type != K_INESCF ) {   /* Not pseudo code.             */

            return( retcode );
        };



        /* 2.
         *      Check Input code.
         */

        switch( pt->code ) {


        /* 2.1.
         *      Input code is Enter,ACTION or CR.
         */

        case P_ENTER :  /* Input code is ENTER.                         */

        case P_ACTION : /* Input code is ACTION.                        */

        case P_CR :     /* Input code is CR.                            */


            kjsvpt->rkclen = 0; /* Reset Remaining romaji string length.*/

            /* 2.1.1.
             *      Check Yomi string.
             */

            /*
             *      Check cursor position.
             *      More than one character exist in input field.
             */
            if ( kjsvpt->curleft != pt->lastch ) {

                /*
                 *      Check Yomi string.
                 */
                for( i=kjsvpt->curleft ; i<pt->lastch ; i+=C_DBCS ) {

                    /*
                     *      Yomi string is Space.
                     */
                    if (  (pt->string[i]   != C_SPACEH) ||
                          (pt->string[i+1] != C_SPACEL)  ) {

                        rst_flg = 0;    /* Set reset flag.(OFF)         */

                        break;
                    };
                };
            };

            if ( rst_flg ) {

                /*
                 *      Check profile parameter kanji input key
                 *      and profile parameter message reset.
                 */

                switch( pt->code ) {

                case P_ENTER :  /* Input code is ENTER.                 */

                    if (  ((kjsvpt->kmpf[0].reset & K_REENT) != K_REENT) &&
                          ((kjsvpt->kmpf[0].kjin  & K_DAENT) != K_DAENT)  ) {

                        err_flg = 1;    /* Set error flag.              */
                    };
                    break;

                case P_ACTION : /* Input code is ACTION.                */

                    if (  ((kjsvpt->kmpf[0].reset & K_REACT) != K_REACT) &&
                          ((kjsvpt->kmpf[0].kjin  & K_DAACT) != K_DAACT)  ) {

                        err_flg = 1;    /* Set error flag.              */
                    };
                    break;

                case P_CR :     /* Input code is CR.                    */

                    if (  ((kjsvpt->kmpf[0].reset & K_RECR) != K_RECR) &&
                          ((kjsvpt->kmpf[0].kjin  & K_DACR) != K_DACR)  ) {

                        err_flg = 1;    /* Set error flag.              */
                    };
                    break;
                };

                if ( err_flg ) {

                    rc = _MM_rtn( pt, A_BEEP ); /* Beep.                */

                    retcode = IMRGSTE;  /* Set return code.             */

                } else {

                    rc = _Mregrs( pt ); /* Dictionary registration reset*/

                    retcode = IMRGIVSE; /* Set return code.             */
                };
                break;
            };


            /* 2.1.2.
             *      Check the kanji input stop code was customized.
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

            case P_CR :         /* Input code is CR.                    */

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


            /* 2.1.3.
             *      7bit code conversion.
             */

            rtnlen  = _Mstrl( pt );     /* String length search.        */

            yomilen = rtnlen - kjsvpt->curleft; /* Set Yomi length.     */

            /*
             *      Get character mode.
             */
            rtnmode = _Mgetchm( pt->string, kjsvpt->curleft , kjsvpt );

            /*
             *      Check character mode.
             */
            switch( rtnmode ) {

            case K_CHKATA :     /* Katakana mode.                       */

            case K_CHHIRA :     /* Hiragana mode.                       */

                mode = rtnmode; /* Set character mode.                  */

                for ( i=C_DBCS ; i<yomilen ; i+=C_DBCS ) {

                    /*
                     *      Get character mode.
                     */
                    rtnmode = _Mgetchm( pt->string, kjsvpt->curleft+i , kjsvpt );

                    if ( rtnmode == K_CHNUM ) { /* Number mode.         */

                        mode = K_CHALPH;        /* Set character mode.  */

                        break;
                    };
                };
                break;

            case K_CHNUM :      /* Number mode.                         */

                mode = K_CHALPH;        /* Set character mode.          */

                break;

            default :

                mode = rtnmode;         /* Set character mode.          */

                break;
            };

            /*
             *      Set Yomi string.
             */
            yomistr = pt->string + kjsvpt->curleft;

            /*
             *      7 bit yomikana conversion process.
             */
            rtnlen  = _Myomic( yomistr, yomilen, chgyomi, mode );

            /*
             *      Check 7 bit yomi code and length.
             */
            if (  (chgyomi[0] == 0x74)          ||
                  (rtnlen < (yomilen / C_DBCS)) ||
                  ( (rtnlen > ((M_RGYLEN / C_DBCS) - C_ANK)) &&
                    (mode == K_CHALPH)                        )  ) {

                /*
                 *      Set Error message.
                 */
                rc = _Mifmst( pt, K_MSGOTH, C_FAUL, C_FAUL, C_COL, C_DBCS,
                              sizeof(M_RGEMSG)-1, M_RGEMSG );

                /*
                 *      Reset next finction.
                 *          (Dictionary registration Yomi)
                 */
                kjsvpt->nextact &= ~M_RGAON;

                /*
                 *      Set next function.
                 *          (Dictionary registration message reset)
                 */
                kjsvpt->nextact |= M_RMRSON;

                /*
                 *      Previous conversion mode set.
                 */
                kjsvpt->kkmode2 = kjsvpt->kkmode1;

                retcode = IMRGIVSE;     /* set return code.             */

                break;
            } else {

                if ( mode == K_CHALPH ) {       /* Alpha/Numeric mode.  */

                    kjsvpt->regyomi[0] = M_NESC;/* Set Yomi string.(ESC)*/

                    /*
                     *      Set Yomi string.
                     */
                    memcpy( (char *)&kjsvpt->regyomi[1],
                            (char *)chgyomi,
                            rtnlen);

                    /*
                     *      Set Yomi string length.
                     */
                    kjsvpt->regymlen = rtnlen + C_ANK;
                } else {

                    /*
                     *      Set Yomi string.
                     */
                    memcpy( (char *)kjsvpt->regyomi,
                            (char *)chgyomi,
                            rtnlen);

                    /*
                     *      Set Yomi string length.
                     */
                    kjsvpt->regymlen = rtnlen;
                };
            };


            /* 2.1.4.
             *      Kanji prompt message set.
             */

            /*
             *      Set message length.
             */
            msglen = sizeof(M_RGKMSG) - 1;

            /*
             *      Set cursor right position.
             */
            rightpos = kjsvpt->realcol - pt->indlen;

            /*
             *      Set Kanji Prompt message.
             */
            rc = _Mifmst( pt, K_MSGDIC, msglen, C_ROW, msglen, rightpos,
                          msglen, M_RGKMSG );


            /* 2.1.5.
             *      Dictionary registration Kanji input flag set.
             */

            /*
             *      Reset next function.
             *          (Dictionary registration Yomi)
             */
            kjsvpt->nextact &= ~M_RGAON;

            /*
             *      Set next function.
             *          (Dictionary registration Kanji)
             */
            kjsvpt->nextact |= M_RGBON;

            /*
             *      Set Dictionary registration Kanji mode.
             */
            kjsvpt->kkcrmode = K_KADIC;


            /* 2.1.6.
             *      First input mode.
             */

            rc = _MM_rtn( pt, A_1STINP );       /* First input mode.    */

            retcode = IMRGSTI;  /* Set return code.                     */

            break;



        /* 2.2.
         *      Input code is RESET.
         */
        case P_RESET :  /* Input code is RESET.                         */

            kjsvpt->rkclen = 0; /* Reset Remaining romaji string length.*/

            /*
             *      Mode cancel process.
             */
            rc = _Mreset( pt, M_ALLRST );

            retcode = IMRGSTI;  /* Set return code.                     */

            break;



        /* 2.3.
         *      Input code is TAB.
         */
        case P_TAB :

            kjsvpt->rkclen = 0; /* Reset Remaining romaji string length.*/

            rc = _MM_rtn( pt, A_BEEP ); /* Beep.                        */

            retcode = IMRGSTI;  /* Set return code.                     */

            break;



        /* 2.4.
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
                 *      To move cursor at the top of input field.
                 */
                rc = _MC_rtn( pt, A_CIFS );
            };

            retcode = IMRGSTI;  /* Set return code.                     */

            break;



        /* 2.5.
         *      Default.
         */
        default :
            break;
        };



        /* 3.
         *      Return.
         */



/*********************** Control Block Snap. ****************************/
/*                                                                      */
/*      SNAP AREA:      Kanji Control Block.                            */
/*                      Kanji Monitor Internal Save Area.               */
/*                                                                      */
/************************************************************************/

        snap3( SNAP_KCB | SNAP_KMISA, SNAP_MRG_a, "Return");

        return( retcode );
}
