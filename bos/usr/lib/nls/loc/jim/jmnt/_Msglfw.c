static char sccsid[] = "@(#)46	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Msglfw.c, libKJI, bos411, 9428A410j 7/23/92 03:24:53";
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
 * MODULE NAME:         _Msglfw
 *
 * DESCRIPTIVE NAME:    Next Single Candidates.
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
 *                      Display Next Single candidates
 *                      in Auxiliary area No.1 or Input field.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:         900 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Msglfw
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Msglfw( pt )
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
 *                              _Kcsglfw:Single kanji Forward.
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
 *                              kkcbsvpt        allcanfg        allcstgs
 *                      Kana Kanji Control Block(KKCB).
 *                              candbotm        totalcan
 *
 *   OUTPUT:            Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              allcstgs        kkcflag         kkcrc
 *                              iws1
 *                      Kana Kanji Control Block(KKCB).
 *                              reqcnt
 *
 * TABLES:              NA.
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
 *      Display Next Single candidates in Auxiliary area No.1 or Input field.
 */
int     _Msglfw( pt )

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
        int     _Kcsglfw();     /* Single kanji Forward.                */

        int     rc;             /* Return code.                         */

        short   i;              /* Loop counter.                        */

        short   rst_flg;        /* All candidates reset flag.           */

        short   totalcan;       /* Total candidates number.             */



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

        if ( kjsvpt->allcstgs[0] == kkcbsvpt->totalcan ) {

            return( IMSUCC );
        };

        /* 1.1.
         *      Single kanji candidates.
         */

        /* 1.1.1.
         *      Last candidates of Single kanji candidates.
         */
        if ( kkcbsvpt->totalcan == kkcbsvpt->candbotm ) {


            /*
             *      Set Single kanji candidates request number.
             */
            kkcbsvpt->reqcnt = kjsvpt->allcstgs[0];

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
        return( IMSUCC );
}
