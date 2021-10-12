static char sccsid[] = "@(#)67	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_MK_c3.c, libKJI, bos411, 9428A410j 7/23/92 03:19:35";
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
 * MODULE NAME:         _MK_c3
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
 *                      Display Next all candidates
 *                      in Auxiliary area No.1 or Input field.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        2500 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _MK_c3
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _MK_c3( pt )
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
 *                              _Mkjgrst:Set Kana/Kanji data.
 *                              _Msglfw :Singel Kanji candidates Forward.
 *                              _Kcallfw:All candidates Forward.
 *                              _Kcallop:All candidates Open and Forward.
 *                              _Kcsglfw:Single kanji Forward.
 *                              _Kcsglop:Single kanji Open and Forward.
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
 *                              allcanfg        allcstge        allcstgs
 *                              tankan          alcnmdfg
 *                      Kana Kanji Control Block(KKCB).
 *                              candbotm        grammap         grmapln
 *                              kanalen1        kjdata          kjlen
 *                              kjmap           kjmapln         totalcan
 *
 *   OUTPUT:            Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              allcstgs        kkcflag         kkcrc
 *                              iws1            tankan
 *                      Kana Kanji Control Block(KKCB).
 *                              kanalen2        grammap         grmapln
 *                              kjdata          kjlen           kjmap
 *                              kjmapln         reqcnt
 *
 * TABLES:              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              IDENTIFY:Module Identify Create.
 *                              SHTOCHPT:Set short data to character
 *                                       pointerd area.
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
 *      Get request number.
 *      Call Kana Kanji Conversion routine.
 *      Display Next all candidates in Auxiliary area No.1 or Input field.
 */
int     _MK_c3( pt )

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
        int     _Mkjgrst();     /* Set Kana/Kanji data.                 */
        int     _Msglfw();      /* Single Kanji candidates Forward.     */

        int     _Kcallfw();     /* All candidates Forward.              */
        int     _Kcallop();     /* All candidates Open and Forward.     */
        int     _Kcsglfw();     /* Single kanji Forward.                */
        int     _Kcsglop();     /* Single kanji Open and Forward.       */

        char    *memcpy();      /* Copy characters from memory area
                                   A to B.                              */

        int     rc;             /* Return code.                         */

        short   i;              /* Loop counter.                        */

        short   rst_flg;        /* All candidates reset flag.           */

        short   allcstgs;       /* Singel kanji first stage.            */

        short   inplen;         /* Length input filed.                  */
        short   sgllen;         /* Length Single kanji candidates.      */

        short   totalcan;       /* Total candidates number.             */

        short   chknum = 0;     /* All candidates stage number.         */



/*********************** Control Block Snap. ****************************/
/*                                                                      */
/*      SNAP AREA:      Kanji Monitor Internal Save Area.               */
/*                      Kana Kanji Control Block.                       */
/*                                                                      */
/************************************************************************/

        snap3( SNAP_KMISA | SNAP_KKCB, SNAP_MK_c3, "Start" );



        kjsvpt   = pt->kjsvpt;
        kkcbsvpt = kjsvpt->kkcbsvpt;

        /*
         *      Initialize return code.
         */
        rc = IMSUCC;

        rst_flg = C_SWOFF;      /* Set erset flag.                      */



/* #(B) 1987.11.30. Flying Conversion Add */

        /*
         *      Check All candidates mode flag.
         */
        if ( kjsvpt->alcnmdfg ) {       /* Single Kanji Candidates Mode.*/

            /*
             *      Single Kanji Candidates Forward.
             */
            rc = _Msglfw( pt );

            return( IMSUCC );
        };
/* #(E) 1987.11.30. Flying Conversion Add */



        /* 1.
         *      Call Kana Kanji Conversion routine.
         */

        /*
         *      Check Tankan flag.
         */
        switch( kjsvpt->tankan ) {


        /* 1.1.
         *      Single kanji candidates.
         */
        case C_SWON :

            /* 1.1.1.
             *      Last candidates of Single kanji candidates.
             */
            if ( kkcbsvpt->totalcan == kkcbsvpt->candbotm ) {

                /*
                 *      Kanji Monitor Internal Save Area data copy to
                 *              Kana Kanji Control Block.
                 */
                rc = _Mkjgrst( pt, M_1SDATA );

                /*
                 *      Double byte shift of Kanji data.
                 */
                memcpy( (char *)kjsvpt->iws1,
                        (char *)kkcbsvpt->kjdata,
                        kkcbsvpt->kjlen);
                SHTOCHPT( kkcbsvpt->kjdata, C_DBCS );
                memcpy( (char *)&kkcbsvpt->kjdata[C_DBCS],
                        (char *)kjsvpt->iws1,
                        kkcbsvpt->kjlen);

                /*
                 *      Kanji data length increase.
                 */
                kkcbsvpt->kjlen += C_DBCS;

                /*
                 *      Double byte shift of Kanji map.
                 */
                memcpy( (char *)kjsvpt->iws1,
                        (char *)kkcbsvpt->kjmap,
                        kkcbsvpt->kjmapln);
                SHTOCHPT( kkcbsvpt->kjmap, C_DBCS );
                memcpy( (char *)&kkcbsvpt->kjmap[C_DBCS],
                        (char *)kjsvpt->iws1,
                        kkcbsvpt->kjmapln);

                /*
                 *      Kanji map length increase.
                 */
                kkcbsvpt->kjmapln += C_DBCS;

                /*
                 *      Single byte shift of Grammer map.
                 */
                memcpy( (char *)kjsvpt->iws1,
                        (char *)kkcbsvpt->grammap,
                        kkcbsvpt->grmapln);
                kkcbsvpt->grammap[0] = C_ANK;
                memcpy( (char *)&kkcbsvpt->grammap[C_ANK],
                        (char *)kjsvpt->iws1,
                        kkcbsvpt->grmapln);

                /*
                 *      Grammer map length increase.
                 */
                kkcbsvpt->grmapln += C_ANK;

                /*
                 *      Set Kana data length No.2.
                 */
                kkcbsvpt->kanalen2 = kkcbsvpt->kanalen1;

                /*
                 *      Set All candidates request number.
                 */
                kkcbsvpt->reqcnt = kjsvpt->allcstge[0];

                /*
                 *      Set Tankan flag.
                 */
                kjsvpt->tankan = C_SWOFF;

                /*
                 *      Set Kana Kanji Conversion calling flag.
                 */
                kjsvpt->kkcflag = M_KKNOP;

                /*
                 *      All candidates Open.
                 */
                kjsvpt->kkcrc = _Kcallop( kkcbsvpt );

                /*
                 *      Check reteurn code.
                 */
                if (  (kjsvpt->kkcrc != K_KCSUCC) &&
                      (kjsvpt->kkcrc != K_KCNMCA)  ) {

                    rst_flg = C_SWON;   /* Set All candidate reset flag.*/
                };
            } else {

                /*
                 *      Check previous Single kanji candidates position.
                 *      Set Single kanji candidates request number.
                 */
                if (   (kkcbsvpt->totalcan - kkcbsvpt->candbotm)
                     <  kjsvpt->allcstgs[0]                       ) {

                    /*
                     *      Set Single kanji candidates request number.
                     */
                    kkcbsvpt->reqcnt = kjsvpt->allcstgs[1];

                } else {

                    /*
                     *      Set Single kanji candidates request number.
                     */
                    kkcbsvpt->reqcnt = kjsvpt->allcstgs[0];
                };

                /*
                 *      Set Kana Kanji Conversion calling flag.
                 */
                kjsvpt->kkcflag = M_KKNOP;

                /*
                 *      Single Kanji candidates Forward.
                 */
                kjsvpt->kkcrc = _Kcsglfw( kkcbsvpt );

                /*
                 *      Check reteurn code.
                 */
                if (  (kjsvpt->kkcrc != K_KCSUCC) &&
                      (kjsvpt->kkcrc != K_KCNMCA)  ) {

                    rst_flg = C_SWON;   /* Set All candidate reset flag.*/
                };
            };
            break;



        /* 1.2.
         *      All candidates.
         */
        case C_SWOFF :

            /* 1.2.1.
             *      Last candidates of Single kanji candidates.
             */
            if ( kkcbsvpt->totalcan == kkcbsvpt->candbotm ) {

                /*
                 *      Check Single kanji candidates stage.
                 *          (First request number)
                 *      Single kanji candidate not open.
                 */
                if ( kjsvpt->allcstgs[0] == -1 ) {

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

                        /*
                         *      Set Single kanji candidates stage.
                         *          (First request number)
                         *      Not Single kanji candidates.
                         */
                        kjsvpt->allcstgs[0] = 0;

                        /*
                         *      Copy Kanji Monitor Internal Save Area data
                         *          to Kana Kanji Control Block.
                         */
                        rc = _Mkjgrst( pt, M_1SDATA );

                        /*
                         *      Shift Kanji data by Double byte.
                         */
                        memcpy( (char *)kjsvpt->iws1,
                                (char *)kkcbsvpt->kjdata,
                                kkcbsvpt->kjlen);
                        SHTOCHPT( kkcbsvpt->kjdata, C_DBCS );
                        memcpy( (char *)&kkcbsvpt->kjdata[C_DBCS],
                                (char *)kjsvpt->iws1,
                                kkcbsvpt->kjlen);

                        /*
                         *      Kanji data length increase.
                         */
                        kkcbsvpt->kjlen += C_DBCS;

                        /*
                         *      Shift of Kanji map data by Double byte.
                         */
                        memcpy( (char *)kjsvpt->iws1,
                                (char *)kkcbsvpt->kjmap,
                                kkcbsvpt->kjmapln);
                        SHTOCHPT( kkcbsvpt->kjmap, C_DBCS );
                        memcpy( (char *)&kkcbsvpt->kjmap[C_DBCS],
                                (char *)kjsvpt->iws1,
                                kkcbsvpt->kjmapln);

                        /*
                         *      Kanji map length increase.
                         */
                        kkcbsvpt->kjmapln += C_DBCS;

                        /*
                         *      Shift Grammer map data by Single byte.
                         */
                        memcpy( (char *)kjsvpt->iws1,
                                (char *)kkcbsvpt->grammap,
                                kkcbsvpt->grmapln);
                        kkcbsvpt->grammap[0] = C_ANK;
                        memcpy( (char *)&kkcbsvpt->grammap[C_ANK],
                                (char *)kjsvpt->iws1,
                                kkcbsvpt->grmapln);

                        /*
                         *      Grammer map length increase.
                         */
                        kkcbsvpt->grmapln += C_ANK;

                        /*
                         *      Set Kana data length No.2.
                         */
                        kkcbsvpt->kanalen2 = kkcbsvpt->kanalen1;

                        /*
                         *      Set All candidates request number.
                         */
                        kkcbsvpt->reqcnt = kjsvpt->allcstge[0];

                        /*
                         *      Set Kana Kanji Conversion calling flag.
                         */
                        kjsvpt->kkcflag = M_KKNOP;

                        /*
                         *      All candidates Open.
                         */
                        kjsvpt->kkcrc = _Kcallop( kkcbsvpt );

                        /*
                         *      Check reteurn code.
                         */
                        if (  (kjsvpt->kkcrc != K_KCSUCC) &&
                              (kjsvpt->kkcrc != K_KCNMCA)  ) {

                            /*
                             *      Set All candidate reset flag.
                             */
                            rst_flg = C_SWON;

                        /*
                         *      Next All candidates not found.
                         */
                        } else if ( !kjsvpt->allcstge[1] ) {

                            return( IMSUCC );
                        };
                        break;

                    default :

                        /*
                         *      Set All candidate reset flag.
                         */
                        rst_flg = C_SWON;

                        break;
                    };


                /*
                 *      Single kanji candidates not found.
                 */
                } else if ( !kjsvpt->allcstgs[0] ) {

                    /*
                     *      Next All candidates not found.
                     */
                    if ( !kjsvpt->allcstge[1] ) {

                        return( IMSUCC );
                    } else {

                        /*
                         *      Set All candidates request number.
                         */
                        kkcbsvpt->reqcnt = kjsvpt->allcstge[0];

                        /*
                         *      Set Kana Kanji Conversion calling flag.
                         */
                        kjsvpt->kkcflag = M_KKNOP;

                        /*
                         *      All candidates Forward.
                         */
                        kjsvpt->kkcrc = _Kcallfw( kkcbsvpt );

                        /*
                         *      Check reteurn code.
                         */
                        if (  (kjsvpt->kkcrc != K_KCSUCC) &&
                              (kjsvpt->kkcrc != K_KCNMCA)  ) {

                            /*
                             *      Set All candidate reset flag.
                             */
                            rst_flg = C_SWON;
                        };
                    };


                /*
                 *      Single kanji candidates Open.
                 */
                } else {

                    /*
                     *      Set Single kanji candidates request number.
                     */
                    kkcbsvpt->reqcnt = kjsvpt->allcstgs[0];

                    /*
                     *      Set Tankan flag.
                     */
                    kjsvpt->tankan = C_SWON;

                    /*
                     *      Set Kana Kanji Conversion calling flag.
                     */
                    kjsvpt->kkcflag = M_KKNOP;

                    /*
                     *      Single kanji candidates Open.
                     */
                    kjsvpt->kkcrc = _Kcsglop( kkcbsvpt );

                    /*
                     *      Check reteurn code.
                     */
                    if (  (kjsvpt->kkcrc != K_KCSUCC) &&
                          (kjsvpt->kkcrc != K_KCNMCA)  ) {

                        /*
                         *      Set All candidate reset flag.
                         */
                        rst_flg = C_SWON;
                    };
                };

            } else {

                for ( i=0 ; ; i++ ) {

                    /*
                     *      All candidates number check point increase.
                     */
                    chknum += kjsvpt->allcstge[i];

                    if ( chknum >= kkcbsvpt->candbotm ) {

                        /*
                         *      Set All candidates request number.
                         */
                        kkcbsvpt->reqcnt = kjsvpt->allcstge[i+1];

                        break;
                    };
                };

                /*
                 *      Set Kana Kanji Conversion calling flag.
                 */
                kjsvpt->kkcflag = M_KKNOP;

                /*
                 *      All candidates Forward.
                 */
                kjsvpt->kkcrc = _Kcallfw( kkcbsvpt );

                /*
                 *      Check reteurn code.
                 */
                if (  (kjsvpt->kkcrc != K_KCSUCC) &&
                      (kjsvpt->kkcrc != K_KCNMCA)  ) {

                    /*
                     *      Set All candidate reset flag.
                     */
                    rst_flg = C_SWON;
                };
            };
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



/*********************** Control Block Snap. ****************************/
/*                                                                      */
/*      SNAP AREA:      Kanji Monitor Internal Save Area.               */
/*                      Kana Kanji Control Block.                       */
/*                                                                      */
/************************************************************************/

        snap3( SNAP_KMISA | SNAP_KKCB, SNAP_MK_c3, "Return");

        return( IMSUCC );
}
