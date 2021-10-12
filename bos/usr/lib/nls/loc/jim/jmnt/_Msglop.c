static char sccsid[] = "@(#)47	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Msglop.c, libKJI, bos411, 9428A410j 7/23/92 03:24:57";
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
 * MODULE NAME:         _Msglop
 *
 * DESCRIPTIVE NAME:    Next All Candidates.
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
 * FUNCTION:            Get request number.
 *                      Call Kana Kanji Conversion routine.
 *                      Display Single candidates
 *                      in Auxiliary area No.1 or Input field.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1500 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Msglop
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Msglop( pt )
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
 *                              _Macaxst:Display All candidates
 *                                       in Auxiliary area No.1.
 *                              _Macifst:Display All candidates
 *                                       in Input field.
 *                              _Kcsglop:Single kanji Open and Forward.
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
 *                              kkcbsvpt        alcancol        alcanrow
 *                              allcanfg        allcstgs
 *                      Kana Kanji Control Block(KKCB).
 *                              kanalen1        totalcan
 *
 *   OUTPUT:            Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              allcstgs        kkcflag         kkcrc
 *                              tankan
 *                      Kana Kanji Control Block(KKCB).
 *                              kanalen2        reqcnt
 *
 * TABLES:              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              IDENTIFY:Module Identify Create.
 *                      Standard Macro Library.
 *                              NA.
 *
 * CHANGE ACTIVITY:     Tuesday Aug. 23 1988 Satoshi Higuchi
 *                      Added to check KATAKANA profile ON or OFF
 *                      after K_CHKATA of case.
 *                      See problem collection sheet P-4
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
 *      Get request number.
 *      Call Kana Kanji Conversion routine.
 *      Display Single candidates in Auxiliary area No.1 or Input field.
 */
int     _Msglop( pt )

register KCB    *pt;            /* Pointer to Kanji Control Block.      */
{
        register KMISA  *kjsvpt;/* Pointer to Kanji Monitor
                                   Internal Save Area.                  */
        register KKCB   *kkcbsvpt;
                                /* Pointer to Kana Kanji Control Block. */

        int     _Macaxst();     /* Display All candidates
                                   in Auxiliary area No.1.              */
        int     _Macifst();     /* Display All candidates
                                   in Input field.                      */

        int     _Kcsglop();     /* Single kanji Open and Forward.       */

        int     rc;             /* Return code.                         */

        short   rst_flg;        /* All candidates reset flag.           */

        short   allcstgs;       /* Singel kanji first stage.            */

        short   inplen;         /* Length input filed.                  */
        short   sgllen;         /* Length Single kanji candidates.      */

        short   totalcan;       /* Total candidates number.             */

        uchar   *yomistr;       /* DBCS string pointer.                 */
        short   yomilen;        /* Length of yomistr.                   */
        uchar   mode;           /* 7bit conversion mode.                */
        uchar   mode2;          /* 7bit conversion mode.                */

        short   loop;           /* Loop Counter.                        */



        kjsvpt   = pt->kjsvpt;
        kkcbsvpt = kjsvpt->kkcbsvpt;

        /*
         *      Initialize return code.
         */
        rc = IMSUCC;

        rst_flg = C_SWOFF;      /* Set erset flag.                      */



        /* 1.
         *      Call Kana Kanji Conversion routine.
         */


        /* 1.1.
         *      Single all candidates.
         */


        /*
         *      Set Yomi String.
         */
        yomistr = pt->string + kjsvpt->cconvpos;

        /*
         *      Set Yomi String Length.
         */
        yomilen = kjsvpt->cconvlen;

        /*
         *      Get Character Mode.
         */
        mode = _Mgetchm( pt->string, kjsvpt->cconvpos , kjsvpt );

        switch( mode ) {

        case K_CHKATA : /* Katakana mode.                               */
/*----------------------------------------------------------------------*/
/*      #(B) Added by S,Higuchi on Aug. 23 1988                         */
/*      Below statements checks KATAKANA ON or OFF in profile           */
/*----------------------------------------------------------------------*/
	    if(kjsvpt->kmpf[0].katakana == K_KANAOF) {
		kjsvpt->actc3 = kjsvpt->kkmode2 | A_BEEP;
		return(IMSUCC);
	    }   /* end of added statements */

        case K_CHHIRA : /* Hiragana mode.                               */

            for ( loop = C_DBCS ; loop < yomilen ; loop += C_DBCS ) {

                /*
                 *      Get character mode.
                 */
                mode2 = _Mgetchm( pt->string, kjsvpt->cconvpos+loop , kjsvpt );

                switch( mode2 ) {

                case K_CHKATA : /* Katakana mode.                       */
/*----------------------------------------------------------------------*/
/*      #(B) Added by S,Higuchi on Aug. 23 1988                         */
/*      Below statements checks KATAKANA ON or OFF in profile           */
/*----------------------------------------------------------------------*/
		    if(kjsvpt->kmpf[0].katakana == K_KANAOF) {
			kjsvpt->actc3 = kjsvpt->kkmode2 | A_BEEP;
			return(IMSUCC);
		    }   /* end of added statements */

                case K_CHHIRA : /* Hiragana mode.                       */

                    break;

                default :

                    kjsvpt->actc3 = kjsvpt->kkmode2 | A_BEEP;

                    return( IMSUCC );

                };
            };
            break;

        default :

            kjsvpt->actc3 = kjsvpt->kkmode2 | A_BEEP;

            return( IMSUCC );

        };


        /*
         *      Set Kana data length.
         */
        kkcbsvpt->kanalen1 =
        kkcbsvpt->kanalen2 =
                _Myomic( yomistr, yomilen, kkcbsvpt->kanadata, mode );


        /*
         *      Check Kana data length.
         */
        if ( kkcbsvpt->kanalen1 != (yomilen / C_DBCS) ) {

            kjsvpt->actc3 = kjsvpt->kkmode2 | A_BEEP;

            return( IMSUCC );
        };



        /*
         *      Check All candidates flag.
         *      Calculeted Single kanji candidates
         *          first request number stage.
         */
        switch ( kjsvpt->allcanfg ) {

            /*
             *      Auxiliary area No.1 with multi-rows.
             */
            case M_ACAX1A :

                /*
                 *      Calculeted Single kanji candidates
                 *          first request number stage.
                 */
                allcstgs = kjsvpt->alcanrow - M_ACMGFD;

                break;


            /*
             *      Auxiliary area No.1 with single-row.
             */
            case M_ACAX1S :
            /*
             *      Input field.
             */
            case M_ACIF :

                /*
                 *      Set Input field length.
                 */
                inplen = kjsvpt->alcancol + C_DBCS;

                /*
                 *      Set Single kanji candidates length.
                 */
                sgllen = M_KNJNOL + C_DBCS;

                /*
                 *      Calculeted Single kanji candidates
                 *          first request stage.
                 */
                allcstgs = (inplen - (inplen % sgllen)) / sgllen;

                if ( allcstgs > M_DFLTRC ) {

                    allcstgs = M_DFLTRC;
                };
                break;
        };

        /*
         *      Set Single kanji candidates request number.
         */
        kkcbsvpt->reqcnt = allcstgs;

        /*
         *      Set Kana Kanji Conversion calling flag.
         */
        kjsvpt->kkcflag = M_KKNOP;

        /*
         *      Singel kanji candidates Open.
         */
        kjsvpt->kkcrc = _Kcsglop( kkcbsvpt );



        /*
         *      Check return code.
         *      Calculeted Single kanji candidates
         *          request number stage.
         */
        switch( kjsvpt->kkcrc ) {


        case K_KCSUCC : /* Single kanji success.            */
        case K_KCNMCA : /* Single kanji page end.           */

            /*
             *      Set Tankan flag.
             */
            kjsvpt->tankan = C_SWON;


            /*
             *      Set Total candidates number.
             */
            totalcan = kkcbsvpt->totalcan;


            /*
             *      Calculeted Single kanji candidates
             *          request number stage.
             */

            /*
             *      Check Total candidates number.
             */
            if ( totalcan <= allcstgs ) {

                /*
                 *      Set Single kanji candidates stage.
                 *          (First request number)
                 */
                kjsvpt->allcstgs[0] = totalcan;

            } else {

                /*
                 *      Set Single kanji candidates stage.
                 *          (First request number)
                 */
                kjsvpt->allcstgs[0] = allcstgs;

                /*
                 *      Set Single kanji candidates stage.
                 *          (Last request number)
                 */
                kjsvpt->allcstgs[1] = totalcan % allcstgs;
            };
            break;


        case K_KCNFCA :     /* Candidates not found.        */
        default :

            /*
             *      Set All candidate reset flag.
             */
            rst_flg = C_SWON;

            break;
        };



        /*
         *      Check reset flag.
         */
        if ( rst_flg ) {

            if ( kjsvpt->kkcrc == K_KCNFCA ) {

                /*
                 *      Field message reset.
                 */
                rc = _Mfmrst( pt, K_MSGOTH );

                /*
                 *      Set action code No.3.
                 */
                kjsvpt->actc3 = kjsvpt->kkmode2 | A_BEEP;

                /*
                 *      Cannot Conversion Indicator On.
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

            } else {

                /*
                 *      Set action code No.3.
                 */
                kjsvpt->actc3 = kjsvpt->kkmode2;
            };

            return( IMSUCC );
        };



        /* 2.
         *      Display.
         *      Check All candidates flag.
         */
        switch( kjsvpt->allcanfg ) {

        /*
         *      Display Next all candidates in Auxiliary area No.1
         *          with multi-rows.
         */
        case M_ACAX1A :

            /*
             *      Set Auxiliary area No.1 message.
             */
            rc = _Macaxst( pt );

            break;

        /*
         *      Display Next all candidates in Auxiliary area No.1
         *          with single-rows.
         */
        case M_ACAX1S :
        /*
         *      Display Next all candidates in Input field.
         */
        case M_ACIF :

            /*
             *      Set Input field message.
             */
            rc = _Macifst( pt );

            break;
        };

        /* 3.
         *      Return.
         */
        return( IMSUCC );
}
