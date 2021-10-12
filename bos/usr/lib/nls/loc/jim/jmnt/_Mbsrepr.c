static char sccsid[] = "@(#)88	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mbsrepr.c, libKJI, bos411, 9428A410j 7/23/92 03:20:56";
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
 * MODULE NAME:         _Mbsrepr
 *
 * DESCRIPTIVE NAME:    Backspace - restore - in replace mode.
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
 * FUNCTION:            Erase character just before the curser, and
 *                      Restore saved character.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        876 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mbsrepr()
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mbsrepr(pt)
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      IMSUCC  :Success of Execution.
 *
 * EXIT-ERROR:          Return Codes Returned to Caller.
 *                      IMFAIL  :Failure of Execution.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _Msetch :Set chenged range for reflesh.
 *                              _Mmvch  :Move character in string
 *                                       and clear rest area.
 *                      Standard Library.
 *                              NA.
 *                      Advanced Display Graphics Support Library(GSL).
 *                              NA.
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      See Below.
 *
 *   INPUT:             DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Controle Block(KCB).
 *                              curcol
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              convpos
 *                              convlen
 *                              stringsv
 *                              savepos
 *                              savelen
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Controle Block(KCB).
 *                              string
 *                              hlatst
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              kjcvmap
 *                              stringsv
 *                              savelen
 *                              convlen
 *                              cconvlen
 *                      Trace Block(TRB).
 *                              NA.
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
 *      Erase character just before the curser, and
 *      Restore saved character.
 */

int _Mbsrepr( pt )
KCB     *pt;            /* Pointer to Kanji Control Block.              */
{
        register KMISA  *kjsvpt;        /* Pointer to KMISA             */
        uchar           clrstr[2];      /* Clear data array             */
        int             _Msetch();      /* see EXTERNAL REFERENCES      */
        int             _Mmvch();       /* see EXTERNAL REFERENCES      */

/************************************************************************/
snap3(SNAP_KCB | SNAP_KMISA, SNAP_Mbsrepr, "_Mbsrepn start position dump");
/************************************************************************/

        /* 0.
         *      Initialize.
         */

        kjsvpt = pt->kjsvpt;    /* set the pointer address to KMISA     */

        /* 1.
         *      Move character string forward from curser.
         */

        _Mmvch( pt->string, pt->curcol,
               (kjsvpt->convpos + kjsvpt->convlen - pt->curcol),
                M_FORWD, C_DBCS, FALSE, (char *)NULL, NULL, NULL);

        /* 2.
         *      Move highlight attribute code forward from curser.
         */

        clrstr[0] = (uchar)K_HLAT0;     /* Define clear data */
        if ( _Mmvch( pt->hlatst, pt->curcol,
                    (kjsvpt->convpos + kjsvpt->convlen - pt->curcol),
                     M_FORWD, C_DBCS, TRUE, clrstr, 0, 1             )
                                                              != IMSUCC ) {

                /* Try to move attribute.
                   If cannot move attribute data because of curser position,
                     set clear data at last character of conversion string  */

                pt->hlatst[kjsvpt->convpos + kjsvpt->convlen - C_DBCS    ] =
                pt->hlatst[kjsvpt->convpos + kjsvpt->convlen - C_DBCS + 1] =
                                                                      K_HLAT0;
        }

        /* 3.
         *      Move Kanji Conversion map data forward from curser.
         */

        clrstr[0] = (uchar)M_KJMNCV;
        clrstr[1] = (uchar)M_KSNCNV;
        _Mmvch( kjsvpt->kjcvmap, (pt->curcol - kjsvpt->convpos),
               (kjsvpt->convpos + kjsvpt->convlen - pt->curcol),
                M_FORWD, C_DBCS, TRUE, clrstr, 0, 2             );

        /* 4.
         *      Restore saved character from save string.
         */

        pt->string[kjsvpt->savepos + kjsvpt->savelen - C_DBCS    ] =
                kjsvpt->stringsv[kjsvpt->savelen - C_DBCS   ];
        pt->string[kjsvpt->savepos + kjsvpt->savelen - C_DBCS + 1] =
                kjsvpt->stringsv[kjsvpt->savelen - C_DBCS +1];

        /* 5.
         *      Decreace saved character length.
         */

        kjsvpt->savelen -= C_DBCS;

        /* 6.
         *      Set changed position and length for display.
         */

        _Msetch( pt, (pt->curcol - C_DBCS),
                (kjsvpt->convpos + kjsvpt->convlen - (pt->curcol - C_DBCS)));

        /* 7.
         *      Decreace conversion length.
         */

        kjsvpt->convlen  -= C_DBCS;

        /* 8.
         *      Check current conversion string length and curser position,
         *      then decrease current curser position or current curser length.
         */

        if (kjsvpt->cconvlen != 0) {    /* there are current
                                              conversion characters */
                if (pt->curcol == kjsvpt->cconvpos) {

                        /* The curser was located in top of
                                current conversion characters. */

                        kjsvpt->cconvpos -= C_DBCS;

                                /* Move all string of current
                                        conversion string. */
                                /* So change top position of current
                                        conversion string. */
                }
                else {

                        /* The curser was located in the middle of
                                current conversion characters.    */

                        kjsvpt->cconvlen -= C_DBCS;

                                /* Move forward from curser */
                                /* So change string length of current
                                        conversion length */
                }
        }

        /* 9.
         *      Return.
         */

/************************************************************************/
snap3(SNAP_KCB | SNAP_KMISA, SNAP_Mbsrepr, "_Mbsrepr End position dump");
/************************************************************************/

        return(IMSUCC);
}
