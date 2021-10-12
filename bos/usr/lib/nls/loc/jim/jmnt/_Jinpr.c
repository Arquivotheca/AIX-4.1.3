static char sccsid[] = "@(#)35	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Jinpr.c, libKJI, bos411, 9428A410j 7/23/92 03:17:35";
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
 * MODULE NAME:         _Jinpr
 *
 * DESCRIPTIVE NAME:    Kanji Input Code Processing.
 *
 * COPYRIGHT:           5601-061 COPYRIGHT IBM CORP 1988
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              Kanji Monitor V1.0
 *
 * CLASSIFICATION:      OCO Source Material - IBM Confidential.
 *                      (IBM Confidential-Restricted when aggregated)
 *
 * FUNCTION:            Check input field.
 *                      Trigger flag reset.
 *                      Next function process.
 *                      Input code process.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        3300 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Jinpr
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Jinpr( pt )
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IMSUCC  :Success of Execution.
 *
 * EXIT-ERROR:          KMINPRE :DBCS input field is not active.
 *                      KMSGLCHE:Unsupproted shift mode.
 *                      KKMALOCE:Memory allocation error.
 *                      KKSYDCOE:System dictionary open error.
 *                      KKFZDCOE:Adjunct dictionary open error.
 *                      KKUSDCOE:User dictionary open error.
 *                      KKFATALE:KKC  Fatal Error Occure.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _MCN_rs :Unconvertible indicator reset.
 *                              _MD_rtn :Code process routine.
 *                              _MKL_rs :Keyboard lock reset.
 *                              _MK_rtn :Kana Kanji Conversion process.
 *                              _MMSG_rs:Message reset.
 *                              _MRG_a  :Dictionary registration YOMI input.
 *                              _MRG_b  :Dictionary registration KANJI input.
 *                              _MRM_rs :Dictionary registration message
 *                                       reset.
 *                              _MR_rtn :Romaji to Kana interface.
 *                              _Mdagend:Diagnosis end.
 *                              _Mdagmsg:Diagnosis Start/Stop message set.
 *                              _Mdagst :Diagnosis start.
 *                              _Mdaha  :Dakuten/Handakuten letter process.
 *                              _Mhtdc  :Hankaku letter(PC code) to PC Kanji
 *                                       code conversion.
 *                              _Mkkcclr:Kana Kanji Conversion interface
 *                                       area clear.
 *                              _Msetch :Set changed position and changed
 *                                       length.
 *                              _Mflypro:Flying Conversion Processing.
 *                              _Tracep :Trace the data.
 *                                       (argument point to index of data)
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
 *                              kjsvpt          code            curcol
 *                              shift1          shift2          shift3
 *                              string          type
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              kkcbsvpt        kmpf            convpos
 *                              convlen         curleft         kana
 *                              kblock          kjcvmap         kkcrc
 *                              kmact           msetflg         nextact
 *                              pfkreset        convtype
 *
 *   OUTPUT:            Kanji Monitor Control Block(KCB).
 *                              beep            chlen           chlna1
 *                              curcol          discrd          kbdlok
 *                              shift           string
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              chcode          chcodlen        kkcrc
 *                              psocde
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              IDENTIFY:Module Identify Create.
 *                              KKCPHYER:Kana Kanji Conversion Phigical
 *                                       Error Code Chcek.
 *                              KKCMAJOR:Kana Kanji Conversion Major Error
 *                                       Code Check.
 *                      Standard Macro Library.
 *                              NA.
 *
 * CHANGE ACTIVITY:     Friday Aug. 26 1988 Satoshi Higuchi
 *                      Added a statement.
 *                      If success Dakuten, Handakuten process,
 *                      force to change input mode to first input.
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
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Make sure that input field is active field.
 *      Reset the trigger flag of Kanji Control Block.
 *      Call all kind of subroutine specified by next function.
 *      Execute pre-processing.
 */
int     _Jinpr( pt )

register KCB    *pt;            /* Pointer to Kanji Control Block.      */
{
        register KMISA  *kjsvpt;/* Pointer to Kanji Monitor
                                   Internal Save Area.                  */
        register KKCB   *kkcbsvpt;
                                /* Pointer to Kana Kanji Control Block. */

        int     _MCN_rs();      /* Unconvertible indicator reset.       */
        int     _MD_rtn();      /* Code process routine.                */
        int     _MKL_rs();      /* Keyboard lock reset.                 */
        int     _MK_rtn();      /* Kana Kanji Conversion process.       */
        int     _MMSG_rs();     /* Message reset.                       */
        int     _MRG_a();       /* Dictionary registration YOMI input.  */
        int     _MRG_b();       /* Dictionary registration KANJI input. */
        int     _MRM_rs();      /* Dictionary registration message reset*/
        int     _MR_rtn();      /* Romaji to Kana interface.            */
        int     _Mdagend();     /* Diagnosis end.                       */
        int     _Mdagmsg();     /* Diagnosis Start/Stop message set.    */
        int     _Mdagst();      /* Diagnosis start.                     */
        int     _Mdaha();       /* Dakuten/Handakuten letter process.   */
        int     _Mhtdc();       /* Hankaku letter(PC code) to PC Kanji
                                   code conversion.                     */
        int     _Mkkcclr();     /* Kana Kanji Conversion interface area
                                   clear.                               */
        int     _Msetch();      /* Set changed position and changed
                                   length.                              */
        int     _Mflypro();     /* Flying Conversion Processing.        */

        int     _Tracep();      /* Trace the data.
                                   (argument point to index of data)    */

        int     ret_cod;        /* Return code.                         */
        int     rc;             /* Return code.                         */
        int     tra_rc;         /* Return code. (trace function)        */

        short   pro_flg;        /* Processing flag.                     */

        ulong   nextact;        /* Next function.                       */

        short   curcol;         /* Current cursor position.             */

        short   strpos;         /* Chenged character position.          */
        short   convrimt;       /* Rimits of conversion string.         */
        short   chkpos;         /* Check position of conversion mode.   */

        uchar   kanain[4];      /* Input string of Dakuten processing.  */
        uchar   kanaout[4];     /* Output string of Dakuten processing. */

        short   cmode;          /* Conversion mode.                     */
        uchar   dbcode[4];      /* Output double byte string.           */


/************************************************************************
 *      Trace data structure.
 ************************************************************************/

/* Trace pointer data structuer.                                        */

struct  {
                short length;
                short index;
                char  *ddatap;
        } trp;



/*********************** Control Block Snap. ****************************/
/*                                                                      */
/*      SNAP AREA:      Kanji Control Block.                            */
/*                      Kanji Monitor Internal Save Area.               */
/*                                                                      */
/************************************************************************/

        snap3( SNAP_KCB | SNAP_KMISA, SNAP_Jinpr, "Start" );



        kjsvpt   = pt->kjsvpt;
        kkcbsvpt = kjsvpt->kkcbsvpt;

        /*
         *      Initialize return code.
         */

        ret_cod = IMSUCC;
        rc      = IMSUCC;
        tra_rc  = IMSUCC;

        pro_flg = C_SWON;       /* Initialize processing flag.          */



        /****************************************************************/
        /*                      Trace dump start.                       */
        /****************************************************************/

        if ( pt->trace == K_TALL ) {

            /*
             *      Kanji Control Block trace output.
             */
            trp.ddatap = (char *)pt;
            trp.length = pt->length + 4;
            trp.index  = 0x8001;
            tra_rc     = _Tracep( (TRB *)pt->tracep, (uchar *)&trp );

            /*
             *      Kanji Monitor Internal Save Area trace output.
             */
            trp.ddatap = (char *)kjsvpt;
            trp.length = kjsvpt->length + 4;
            trp.index  = 0x8002;
            tra_rc     = _Tracep( (TRB *)pt->tracep, (uchar *)&trp );

            /*
             *      Kana Kanji Control Block trace output.
             */
            trp.ddatap = (char *)kkcbsvpt;
            trp.length = kkcbsvpt->length+ 4;
            trp.index  = 0x8003;
            tra_rc     = _Tracep( (TRB *)pt->tracep, (uchar *)&trp );
        }
        /****************************************************************/
        /*                      Trace dump end.                         */
        /****************************************************************/



        /* 0.
         *      Check input type & code.
         */
        if ( pt->type == K_INESCF && pt->code > P_CLR ) {

            return( ret_cod );
        };



        /* 1.
         *      Check Input field.
         */

        if ( kjsvpt->kmact == K_IFINAC ) {      /* Inactive field.      */

            pro_flg = C_SWOFF;  /* Processing flag reset.               */

            ret_cod = KMINPRE;  /* Set of error return code.            */

        } else {        /* Active field.                                */



            /* 2.
             *      Trigger reset.
             */

            pt->shift  = K_STNOT;       /* Shift change flag reset.     */

            pt->chlen  = 0;             /* Changed Input field string
                                           length reset.                */

            pt->chlna1 = 0;             /* Changed Auxiliary area No.1
                                           string length reset.         */

            pt->beep   = K_BEEPOF;      /* Beep flag reset.             */

            pt->discrd = K_DISON;       /* Discord flag reset.          */

            kjsvpt->kkcrc = K_KCSUCC;   /* Clear KKC Error Code.        */

            curcol = pt->curcol;        /* Set Current cursor position. */


            /* 3.
             *      Next function Process.
             */

            nextact = kjsvpt->nextact;  /* Next function set.           */

            if (  (nextact & M_DAGMON)  &&  pro_flg  ) {

                /*
                 *      Daiagnosis message processing.
                 */

                if ( _Mdagmsg( pt ) != IMDGIVK ) {

                    pro_flg = C_SWOFF;  /* Processing flag reset.       */
                };
            };

            if (  (nextact & M_DAGSON)  &&  pro_flg  ) {

                /*
                 *      Daiagnosis start processing.
                 */

                if ( _Mdagst( pt ) != IMDGIVK ) {

                    pro_flg = C_SWOFF;  /* Processing flag reset.       */



/* #(B) 1988.01.12. Flying Conversion Add */
                    /*
                     *      Check Input Code Type.
                     *              Input Code Type is Pseudo Code.
                     */

                    if ( pt->type == K_INESCF ) {

                        /*
                         *      Check Input Code.
                         */
                        switch ( pt->code ) {

                        case P_KATA :   /* Katakana shift.              */
                        case P_ALPHA:   /* Alpha/Num shift.             */
                        case P_HIRA :   /* Hiragana shift.              */
                        case P_RKC  :   /* RKC.                         */

                            /*
                             *      Set Pseudo Code.
                             */
                            kjsvpt->pscode = pt->code;

                            rc = _MD_rtn( pt ); /* Code process.        */

                            break;
                        };
                    };
/* #(E) 1988.01.12. Flying Conversion Add */
                };
            };

            if (  (nextact & M_DAGEON)  &&  pro_flg  ) {

                /*
                 *      Daiagnosis end processing.
                 */

                if ( _Mdagend( pt ) != IMDGIVK ) {

                    pro_flg = C_SWOFF;  /* Processing flag reset.       */



/* #(B) 1988.01.12. Flying Conversion Add */
                    /*
                     *      Check Input Code Type.
                     *              Input Code Type is Pseudo Code.
                     */

                    if ( pt->type == K_INESCF ) {

                        /*
                         *      Check Input Code.
                         */
                        switch ( pt->code ) {

                        case P_KATA :   /* Katakana shift.              */
                        case P_ALPHA:   /* Alpha/Num shift.             */
                        case P_HIRA :   /* Hiragana shift.              */
                        case P_RKC  :   /* RKC.                         */

                            /*
                             *      Set Pseudo Code.
                             */
                            kjsvpt->pscode = pt->code;

                            rc = _MD_rtn( pt ); /* Code process.        */

                            break;
                        };
                    };
/* #(E) 1988.01.12. Flying Conversion Add */
                };
            };

            if (  (nextact & M_KLRSON)  &&  pro_flg  ) {

                /*
                 *      Keyborad lock reset processing.
                 */

                if ( _MKL_rs( pt ) != IMSUCC ) {

                    pro_flg = C_SWOFF;  /* Processing flag reset.       */
                };
            };

            if (  (nextact & M_MGRSON)  &&  pro_flg  ) {

                /*
                 *      Other type message reset processing.
                 */

                if ( _MMSG_rs( pt, K_MSGOTH ) != IMSUCC ) {

                    pro_flg = C_SWOFF;  /* Processing flag reset.       */
                };
            };

            if (  (nextact & M_RGCON)  &&  pro_flg  ) {

                /*
                 *      Dictionary registration end message reset process.
                 */

                if ( _MMSG_rs( pt, K_MSGDIC ) != IMSUCC ) {

                    pro_flg = C_SWOFF;  /* Processing flag reset.       */
                };
            };

            if (  (nextact & M_RGAON)  &&  pro_flg  ) {

                /*
                 *      Dictionary registration Yomi input process.
                 */

                if ( _MRG_a( pt ) != IMSUCC ) {

                    pro_flg = C_SWOFF;  /* Processing flag reset.       */
                };
            };

            if (  (nextact & M_RGBON)  &&  pro_flg  ) {

                /*
                 *      Dictionary registration Kanji input process.
                 */

                if ( _MRG_b( pt ) != IMSUCC ) {

                    pro_flg = C_SWOFF;  /* Processing flag reset.       */
                };
            };

            if (  (nextact & M_CNRSON)  &&  pro_flg  ) {

                /*
                 *      Unconvertible indicator reset processing.
                 */

                if ( _MCN_rs( pt ) != IMSUCC ) {

                    pro_flg = C_SWOFF;  /* Processing flag reset.       */
                };
            };

            if (  (nextact & M_RMRSON)  &&  pro_flg  ) {

                /*
                 *      Dictionary registration error message reset process.
                 */

                if ( _MRM_rs( pt ) != IMSUCC ) {

                    pro_flg = C_SWOFF;  /* Processing flag reset.       */
                };
            };



            /* 4.
             *      Check Input code type.
             */

            if ( pro_flg ) {


                switch ( pt->type ) {

                /* 4.1.
                 *      Input code type is character code.
                 */
                case K_INASCI :


                    /* 4.1.1.
                     *      Clear pseudo code.
                     */
                    kjsvpt->pscode = 0x00;


                    /* 4.1.2.
                     *      Check shift NO.3
                     */

                    /*
                     *      Single byte character mode.
                     */
                    if ( pt->shift3 == K_ST3SIN ) {

                        ret_cod = KMSGLCHE;     /* Set return code.     */

                        break;
                    };


                    /* 4.1.3.
                     *      Dakuten,Han_dakuten conversion processing.
                     */

                    /*
                     *      Input code is Dakuten or Han_dakuten.
                     */
                    if (  (pt->code == 0xde) ||
                          (pt->code == 0xdf)  ) {

                        /*
                         *      Check cursor position.
                         */
                        if ( pt->curcol == kjsvpt->curleft ) {

                            /*
                             *      Dakuten,Han_dakuten conversion process.
                             */
                            rc = _MR_rtn( pt, M_RKCON );

                            break;
                        };

                        /*
                         *      Set Dakuten,Han_dakuten
                         *      string conversion position.
                         */
                        strpos = pt->curcol - C_DBCS;

                        /*
                         *      Set Dakuten,Han_dakuten conversion string.
                         */
                        kanain[0] = pt->string[strpos];
                        kanain[1] = pt->string[strpos+1];
                        kanain[2] = pt->code;
                        kanain[3] = 0;

                        /*
                         *      Dakuten,Han_dakuten conversion processing.
                         */
                        rc = _Mdaha( kanain, kanaout );

                        /*
                         *      Check return code.
                         *      Return code is Dakuten,Han_dakuten
                         *      conversion processing.
                         */
                        if ( rc == IMSUCC ) {

                            /*
                             *      Set String data.
                             */
                            pt->string[strpos]   = kanaout[0];
                            pt->string[strpos+1] = kanaout[1];

                            /*
                             *      Set changed character code.
                             */
                            kjsvpt->chcode[0] = kanaout[0];
                            kjsvpt->chcode[1] = kanaout[1];

                            /*
                             *      Set changed character code length.
                             */
                            kjsvpt->chcodlen = C_DBCS;

                            /*
                             *      Clear Remeining romaji string length.
                             */
                            rc = _MR_rtn( pt, M_RKCOFF );

                            /*
                             *      Set Rimits position of object of
                             *      conversion.
                             */
                            convrimt = kjsvpt->convpos + kjsvpt->convlen;

                            /*
                             *      Check cursor position.
                             */
                            if (  (strpos >= kjsvpt->convpos) &&
                                  (strpos < convrimt)          ) {

                                /*
                                 *      Set conversion mode check position.
                                 */
                                chkpos = strpos - kjsvpt->convpos + 1;

                                /*
                                 *      Check conversion mode.
                                 */
                                if ( kjsvpt->kjcvmap[chkpos] != M_KSNCNV ) {

                                    /*
                                     *      To fix converted character.
                                     */
                                    rc = _MK_rtn( pt , A_CNVDEC );
/*------------------------------------------------------------------------------*/
/*      #(B) Added by S,Higuchi on Aug. 26 1988                                 */
/*      If success Dakute, Handakuten process at _Mdara(), force to change      */
/*      input mode to first inp at _MM_rtn().                                   */
/*                                                                              */
/*      (void)_MM_rtn(pt,A_1STINP);                                             */
/*------------------------------------------------------------------------------*/
				    (void)_MM_rtn(pt,A_1STINP);
                                };



/* #(B) 1987.12.23. Flying Conversion Add */
                                /*
                                 *      Flying Conversion Processing.
                                 */
                                _Mflypro( pt );
/* #(E) 1987.12.23. Flying Conversin Add */



                            } else if ( kjsvpt->convlen > 0 ) {

                                /*
                                 *      To fix converted character.
                                 */
                                rc = _MK_rtn( pt, A_CNVDEC );
/*------------------------------------------------------------------------------*/
/*      #(B) Added by S,Higuchi on Aug. 26 1988                                 */
/*      If success Dakute, Handakuten process at _Mdara(), force to change      */
/*      input mode to first inp at _MM_rtn().                                   */
/*                                                                              */
/*      (void)_MM_rtn(pt,A_1STINP);                                             */
/*------------------------------------------------------------------------------*/
				(void)_MM_rtn(pt,A_1STINP);
                            };

                            /*
                             *      Set changed character position.
                             */
                            rc = _Msetch( pt, strpos, C_DBCS );
                        } else {

                            /*
                             *      Dakuten,Han_dakuten conversion
                             *      processing.
                             */
                            rc = _MR_rtn( pt, M_RKCON );
                        };
                        break;
                    };


                    /* 4.1.4.
                     *      Romaji to Kana conversion processing.
                     */

                    /*
                     *      Romaji to Kana conversion shift is
                     *          Romaji to Kana conversion mode.
                     *      Alphabet/Numeric/Katakana/Hiragana shift is
                     *          Katakana mode or Hiragana mode.
                     *      Display not message.
                     */
                    if (  (pt->shift2 == K_ST2RON)                      &&
                          ( (pt->shift1 == K_ST1KAT) ||
                            (pt->shift1 == K_ST1HIR)  )                 &&
                          ((kjsvpt->msetflg & K_MSGOTH) != K_MSGOTH)  )  {
                        /*
                         *      Check Input character code.
                         */

                        /*
                         *      Blank code.
                         */
                        if ( pt->code == C_BLANK ) {

                            /*
                             *      Space conversion processing.
                             */
                            rc = _MR_rtn( pt, M_RKCOFF );
                        } else {

                            /*
                             *      Romaji to Kana conversion processing.
                             */
                            rc = _MR_rtn( pt, M_RKCON );
                        };
                        break;
                    };


                    /* 4.1.5.
                     *      Single byte to Double byte conversion processing.
                     */

                    /*
                     *      Check Alpha/Kana shift mode.
                     */

                    /*
                     *      Alphabet/Numeric mode.
                     */
                    if ( pt->shift1 == K_ST1AN ) {

                            /*
                             *      Set Character change mode.
                             *          (Alphabet/Numeric)
                             */
                            cmode = M_SFTANK;

                    /*
                     *      Katakana mode.
                     */
                    } else if ( pt->shift1 == K_ST1KAT ) {

                            /*
                             *      Set Character change mode.
                             *          (Katakana)
                             */
                            cmode = M_SFTKAT;

                    /*
                     *      Hiragana mode.
                     */
                    } else if ( pt->shift1 == K_ST1HIR ) {

                            /*
                             *      Set Character change mode.
                             *          (Hiragana)
                             */
                            cmode = M_SFTHIR;

                    /*
                     *      First mode is Katakana mode.
                     */
                    } else if ( kjsvpt->kmpf[0].kana == K_KATA ) {

                            /*
                             *      Set Character change mode.
                             *          (Katakana)
                             */
                            cmode = M_SFTKAT;

                    /*
                     *      First mode is Hiragana mode.
                     */
                    } else {

                            /*
                             *      Set Character change mode.
                             *          (Hiragana)
                             */
                            cmode = M_SFTHIR;
                    };

                    /*
                     *      Single byte code to Double byte code conversion.
                     */
                    if ( _Mhtdc( cmode, pt->code, dbcode ) == IMSUCC ) {

                        /*
                         *      Set chabged character code.
                         */
                        kjsvpt->chcode[0] = dbcode[0];
                        kjsvpt->chcode[1] = dbcode[1];

                        /*
                         *      Set chenged character code length.
                         */
                        kjsvpt->chcodlen = C_DBCS;

                        rc = _MD_rtn( pt );     /* Code process.        */

                    };
                    break;


                /* 4.2.
                 *      Input Code Type is Pseudo Code.
                 */
                case K_INESCF :

                    /*
                     *      Romaji to Kana conversion shift is
                     *          Romaji to Kana conversion mode.
                     *      Alphabet/Numeric/Katakana/Hiragana shift is
                     *          Katakana mode or Hiragana mdoe.
                     */
                    if (  (pt->shift2 == K_ST2RON)     &&
                          ( (pt->shift1 == K_ST1KAT) ||
                            (pt->shift1 == K_ST1HIR)    )  ) {

                        /*
                         *      Clear Remeining romaji string length.
                         */
                        if ( _MR_rtn( pt, M_RKCOFF ) != IMSUCC ) {

                            break;
                        };
                    };

                    kjsvpt->pscode = pt->code;  /* Set pseudo code.     */


                    /*
                     *      Chekc pseuco code and Keyboard lock flag.
                     */

                    /*
                     *      Input code is Function code.
                     */
                    if ( kjsvpt->pscode == P_CLR ) {

                        /*
                         *      Check profire parameter function key reset
                         *      and message set flag.
                         */
                        if (  (kjsvpt->kmpf[0].pfkreset == K_PFKON)     &&
                              (kjsvpt->msetflg & (K_MSGOTH | K_MSGDIC))  ) {

                                /*
                                 *      Kanji Monitor Internal Save Area
                                 *      variables reset.
                                 */
                                rc = _Mkkcclr( pt );
                            };

                        pt->discrd = K_DISOFF;  /* Discord flag reset.  */

                    /*
                     *      Keyboard lock flag OFF.
                     */
                    } else if ( pt->kbdlok == K_KBLOFF ) {

                        rc = _MD_rtn( pt );     /* Code process.        */

                    /*
                     *      Keyboard lock flag ON.
                     *      Input code is Katakana shift
                     *                 or Alpha/Num shift
                     *                 or Hiragana shift.
                     */
                    } else if (  (kjsvpt->pscode == P_KATA)  ||
                                 (kjsvpt->pscode == P_ALPHA) ||
                                 (kjsvpt->pscode == P_HIRA)   ) {

                        rc = _MD_rtn( pt );     /* Code process.        */
                    };
                    break;


                /* 4.3.
                 *      Input Code Type is Binary Data.
                 */
                case K_INBIN :

                    /*
                     *      Romaji to Kana conversion shift is
                     *          Romaji to Kana conversion mode.
                     *      Alphabet/Numeric/Katakana/Hiragana shift is
                     *          Katakana mode or Hiragana mdoe.
                     */
                    if (  (pt->shift2 == K_ST2RON)     &&
                          ( (pt->shift1 == K_ST1KAT) ||
                            (pt->shift1 == K_ST1HIR)    )  ) {

                        /*
                         *      Clear Remeining romaji string length.
                         */
                        rc = _MR_rtn( pt, M_RKCOFF );
                    };
                    break;
                };
            };
        };



        if( KKCPHYER( kjsvpt->kkcrc ) ) {
            /*
             *      Check Kana Kanji Conversion return code.
             *      Kana Kanji Conversion return code is
             *      Phigical error code.
             */
            switch( KKCMAJOR( kjsvpt->kkcrc ) ) {

            /*
             *      Physical error on System Dictionary.
             */
            case K_KCSYPE :

                ret_cod = KKSYDCOE; /* Set return code.                 */
                break;

            /*
             *      Physical error on User Dictionary.
             */
            case K_KCUSPE :
                ret_cod = KKUSDCOE; /* Set return code.                 */
                break;

            /*
             *      Physical error on Adjunct Dictionary.
             */
            case K_KCFZPE :
                ret_cod = KKFZDCOE; /* Set return code.                 */
                break;

            /*
             *      Memory allocation error.
             */
            case K_KCMALE :
                ret_cod = KKMALOCE; /* Set return code.                 */
                break;

            /*
             *      If Unknown Error Accept then 'KKC Return Code'
             *      Return to Application.
             */
            default :
                ret_cod = KKFATALE; /* Set return code.                 */
                break;
            };
        } else {

            switch( KKCMAJOR( kjsvpt->kkcrc ) ) {

                /*
                 *      Normal.
                 */
                case K_KCSUCC :

                /*
                 *      Nop more candidate (page end).
                 */
                case K_KCNMCA :

                /*
                 *      Candidates not found.
                 */
                case K_KCNFCA :

                /*
                 *      Dictionary registration error.
                 */
                case K_KCDCTE :

                    ret_cod = IMSUCC;   /* Set return code.             */
                    break;

                /*
                 *      Logical error.
                 */
                case K_KCLOGE :

                /*
                 *      If Unknown Error Accept then 'KKC Return Code'
                 *      Return to Application.
                 */
                default :

                    pt->curcol = curcol;/* Set current cursor position. */

                    ret_cod = KKFATALE; /* Set return code.             */

                    break;
            };
        };



        /*
         *      Check Profile Keyboard lock flag.
         */
        if ( kjsvpt->kmpf[0].kblock == K_KBLOFF ) {

                pt->kbdlok = K_KBLOFF;  /* Reset keyboard lock flag.    */
        };



        /****************************************************************/
        /*                      Trace dump start.                       */
        /****************************************************************/

        /*
         *      Kanji Control Block trace output.
         */
        trp.ddatap = (char *)pt;
        trp.length = pt->length + 4;
        trp.index  = 0x0004;
        tra_rc     = _Tracep( (TRB *)pt->tracep, (uchar *)&trp );

        /*
         *      Kanji Monitor Internal Save Area trace output.
         */
        trp.ddatap = (char *)kjsvpt;
        if ( pt->trace == K_TALL )
            trp.length = kjsvpt->length + 4;
        else
            trp.length = sizeof(KMISA) + 4;
        trp.index  = 0x0005;
        tra_rc     = _Tracep( (TRB *)pt->tracep, (uchar *)&trp );

        /*
         *      Kana Kanji Control Block trace output.
         */
        trp.ddatap = (char *)kkcbsvpt;
        trp.length = kkcbsvpt->length + 4;
        trp.index  = 0x0006;
        tra_rc     = _Tracep( (TRB *)pt->tracep, (uchar *)&trp );

        /****************************************************************/
        /*                      Trace dump end.                         */
        /****************************************************************/



        /* 5.
         *      Return.
         */

/*********************** Control Block Snap. ****************************/
/*                                                                      */
/*      SNAP AREA:      Kanji Control Block.                            */
/*                      Kanji Monitor Internal Save Area.               */
/*                                                                      */
/************************************************************************/

        snap3( SNAP_KCB | SNAP_KMISA, SNAP_Jinpr, "Return");


        return( ret_cod );
}

