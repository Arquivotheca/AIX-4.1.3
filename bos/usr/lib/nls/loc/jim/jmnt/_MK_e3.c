static char sccsid[] = "@(#)68	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_MK_e3.c, libKJI, bos411, 9428A410j 7/23/92 03:19:39";
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
 * MODULE NAME:         _MK_e3
 *
 * DESCRIPTIVE NAME:    Previous All candidates.
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
 *                      Display Previous All candidates
 *                      in Auxiliary area No.1 or Input field.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1200 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _MK_e3
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _MK_e3( pt )
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
 *                              _Msglbw :Single Kanji candidates backward.
 *                              _Kcallbw:All candidates Backward.
 *                              _Kcsglbw:Single kanji Backward.
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
 *                              kkcbsvpt        allcanfg        allcstge
 *                              allcstgs        kkmode2         tankan
 *                              alcnmdfg
 *                      Kana Kanji Control Block(KKCB).
 *                              candtop         totalcan
 *
 *   OUTPUT:            Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              actc3           kkcflag         kkcrc
 *                      Kana Kanji Control Block(KKCB).
 *                              reqcnt
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
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Get request number.
 *      Call Kana Kanji Conversion routine.
 *      Display Previous All candidates
 *          in Auxiliary area No.1 or Input field.
 */
int     _MK_e3( pt )

register KCB    *pt;            /* Pointer to Kanji Control Block.      */
{
        register KMISA  *kjsvpt;/* Pointer to Kanji Monitor
                                   Internal Save Area.                 */
        register KKCB   *kkcbsvpt;
                                /* Pointer to Kana Kanji Control Block. */

        int     _Macaxst();     /* Display All candidates
                                   in Auxiliary area No.1.              */
        int     _Macifst();     /* Display All candidates
                                   in Input field.                      */
        int     _Msglbw();      /* Single Kanji candidates backward.    */

        int     _Kcallbw();     /* All candidates Backward.             */
        int     _Kcsglbw();     /* Single kanji Backward.               */

        int     rc;             /* Return code.                         */

        short   rst_flg;        /* All candidates reset flag.           */

        short   i;              /* Loop counter                         */

        short   chknum = 0;     /* All candidates stage number.         */



/*********************** Control Block Snap. ****************************/
/*                                                                      */
/*      SNAP AREA:      Kanji Monitor Internal Save Area.               */
/*                      Kana Kanji Control Block.                       */
/*                                                                      */
/************************************************************************/

        snap3( SNAP_KMISA | SNAP_KKCB, SNAP_MK_e3, "Start");



        kjsvpt   = pt->kjsvpt;
        kkcbsvpt = kjsvpt->kkcbsvpt;

        /*
         *      Initialize return code.
         */
        rc    = IMSUCC;

        rst_flg = C_SWOFF;      /* SEt All candidates reset flag.       */



/* #(B) 1987.11.30. Flying Conversion Add */

        /*
         *      Check All Candidates Mode Flag.
         */
        if ( kjsvpt->alcnmdfg ) {       /* Single Kanji Candidates Mode.*/

            /*
             *      Single Kanji Candidates Backward.
             */
            rc = _Msglbw( pt );

            return( IMSUCC );
        };
/* #(E) 1987.11.30. Flying Conversion Add */



        /* 1.
         *      Calling Kana Kanji Conversion routine.
         */

        /*
         *      Check Tankan flag.
         */
        switch( kjsvpt->tankan ) {

        /* 1.1.
         *      Single kanji candidates.
         */
        case C_SWON :

            /*
             *      Check Total candidates of Single kanji candidates.
             */
            if ( kjsvpt->allcstgs[0] == kkcbsvpt->totalcan ) {

                return( IMSUCC );
            };


            /*
             *      Check Display position of Single kanji candidates.
             */
            if (  (kkcbsvpt->candtop == 1) &&
                   kjsvpt->allcstgs[1]      ) {

                /*
                 *      Set Single kanji candidates request number.
                 *          (Last candidates)
                 */
                kkcbsvpt->reqcnt = kjsvpt->allcstgs[1];

            } else {

                /*
                 *      Set Single kanji candidates request number.
                 *          (First candidates)
                 */
                kkcbsvpt->reqcnt = kjsvpt->allcstgs[0];
            };

            /*
             *      Set Kana Kanji Conversion calling flag.
             */
            kjsvpt->kkcflag = M_KKNOP;

            /*
             *  Single kanji candidates Backward.
             */
            kjsvpt->kkcrc = _Kcsglbw( kkcbsvpt );

            /*
             *      Check reteurn code.
             */
            if (  (kjsvpt->kkcrc != K_KCSUCC) &&
                  (kjsvpt->kkcrc != K_KCNMCA)  ) {

                rst_flg = C_SWON;   /* Set All candidate reset flag.*/
            };

            break;


        /*
         *      All candidates.
         */
        case C_SWOFF :

            /*
             *      Check Total candidates of All candidates.
             */
            if ( kjsvpt->allcstge[0] == kkcbsvpt->totalcan ) {

                return( IMSUCC );
            };

            /*
             *      Check Display position of All candidates.
             */
            if ( kkcbsvpt->candtop == 1 ) {

                /*
                 *      Check Last candidates.
                 */
                for ( i=0 ; ; i++ ) {

                    /*
                     *      Set All candidates position.
                     */
                    chknum += kjsvpt->allcstge[i];

                    /*
                     *      Check All candidates position.
                     */
                    if ( chknum == kkcbsvpt->totalcan ) {

                        /*
                         *      Set All candidates request number.
                         */
                        kkcbsvpt->reqcnt = kjsvpt->allcstge[i];

                        break;
                    };
                };
            } else {

                for ( i=0 ; ; i++ ) {

                    /*
                     *      Set All candidates position.
                     */
                    chknum += kjsvpt->allcstge[i];

                    /*
                     *      Check All candidates position.
                     */
                    if ( chknum >= kkcbsvpt->candbotm ) {

                        /*
                         *      Set All candidates request number.
                         */
                        kkcbsvpt->reqcnt = kjsvpt->allcstge[i-1];

                        break;
                    };
                };
            };

            /*
             *      Set Kana Kanji Conversion calling flag.
             */
            kjsvpt->kkcflag = M_KKNOP;

            /*
             *      All candidates Backward.
             */
            kjsvpt->kkcrc = _Kcallbw( kkcbsvpt );

            /*
             *      Check reteurn code.
             */
            if (  (kjsvpt->kkcrc != K_KCSUCC) &&
                  (kjsvpt->kkcrc != K_KCNMCA)  ) {

                rst_flg = C_SWON;   /* Set All candidate reset flag.*/
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
                kjsvpt->actc3 = kjsvpt->kkmode2 | A_BEEP;
            };

            return( IMSUCC );
        };



        /* 2.
         *      Display.
         *      Check All candidates flag.
         */
        switch( kjsvpt->allcanfg ) {

        /*
         *      Display All candidates in Auxiliary area No.1
         *          with multi-rows.
         */
        case M_ACAX1A :

            /*
             *      Set Auxiliary area No.1 message.
             */
            rc = _Macaxst( pt );

            break;

        /*
         *      Display All candidates in Auxiliary area No.1
         *          with single-row.
         */
        case M_ACAX1S :
        /*
         *      Display All candidates in Input field.
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

        snap3( SNAP_KMISA | SNAP_KKCB, SNAP_MK_e3, "Return");

        return( IMSUCC );
}
