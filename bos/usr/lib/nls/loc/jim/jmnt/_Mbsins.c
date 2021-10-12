static char sccsid[] = "@(#)86	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mbsins.c, libKJI, bos411, 9428A410j 7/23/92 03:20:49";
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
 * MODULE NAME:         _Mbsins
 *
 * DESCRIPTIVE NAME:    Backspace in insert mode.
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
 *                      move string from curser forward.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        804 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mbsins()
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mbsins(pt)
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
 *                              _Mmvch  :Move character in same string and
 *                                       set clear data source area that
 *                                       wasn't replaced by movement of data.
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
 *                              lastch
 *                              string
 *                              hlatst
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              convpos
 *                              convlen
 *                              cconvlen
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
 *                              lastch
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              convlen
 *                              cconvlen
 *                              kjcvmap
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
 *      move string from curser forward.
 */

int _Mbsins( pt )
KCB     *pt;            /* Pointer to Kanji Control Block.              */
{
        register KMISA  *kjsvpt;        /* Pointer to KMISA             */
        uchar           clrstr[2];      /* Clear data array             */
        int             _Msetch();      /* see EXTERNAL REFERENCES      */
        int             _Mmvch();       /* see EXTERNAL REFERENCES      */

/************************************************************************/
snap3(SNAP_KCB | SNAP_KMISA, SNAP_Mbsins, "_Mbsins() start dump");
/************************************************************************/

        /* 0.
         *      Initialize.
         */

        kjsvpt = pt->kjsvpt;    /* Set the address of KMISA */

        /* 1.
         *      Set Blank code just before the curser.
         */

        pt->string[pt->curcol - C_DBCS    ] = C_SPACEH; /* Set Higher   */
        pt->string[pt->curcol - C_DBCS + 1] = C_SPACEL; /* Set Lower    */

        /* 2.
         *      Move character string forward from curser.
         */

        clrstr[0] = (uchar)C_SPACEH;    /* Set clear data */
        clrstr[1] = (uchar)C_SPACEL;    /* Set clear data */
        _Mmvch( pt->string, pt->curcol, (pt->lastch - pt->curcol),
                M_FORWD, C_DBCS, TRUE, clrstr, 0, C_DBCS);
                /* Move Character string and crear remainig part */

        /* 3.
         *      if there is conversion strings, move hilight attribute,
         *              and kanji convesion map, and decreace conversion
         *                      length and current conversion length.
         */

        if (kjsvpt->convlen != 0) {  /* Tere is conversion strings */

                /* 3.a
                 *      Crear highlight attribute just before the curser.
                 */

                pt->hlatst[pt->curcol - C_DBCS    ] =
                pt->hlatst[pt->curcol - C_DBCS + 1] = K_HLAT0;

                /* 3.b
                 *      Move forward C_DBCS(=2) byte highlight attribute
                 *      block from curser to end of conversion character
                 */

                clrstr[0] = (uchar)K_HLAT0;     /* Set crear data */
                _Mmvch( pt->hlatst, pt->curcol,
                       (kjsvpt->convpos + kjsvpt->convlen - pt->curcol),
                        M_FORWD, C_DBCS, TRUE, clrstr, 0, 1             );

                /* 3.c
                 *      Move forward kjcvmap data from curser position.
                 */

                clrstr[0] = (uchar)M_KJMNCV;    /* Set crear data */
                clrstr[1] = (uchar)M_KSNCNV;    /* Set crear data */
                _Mmvch( kjsvpt->kjcvmap, (pt->curcol - kjsvpt->convpos),
                       (kjsvpt->convpos + kjsvpt->convlen - pt->curcol),
                        M_FORWD, C_DBCS, TRUE, clrstr, 0, 2             );

                /* 3.d
                 *      If there are current conversion characters,
                 *        must change conversion data and parameters.
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

                /* 3.e
                 *      Decrease convesion character length.
                 */

                kjsvpt->convlen -= C_DBCS;
        }

        /* 4.
         *      Set changed position and length for display.
         */

        _Msetch( pt, (pt->curcol - C_DBCS),
                (pt->lastch - (pt->curcol - C_DBCS)) );

        /* 5.
         *      Decreace last character position.
         */

        pt->lastch -= C_DBCS;

        /* 6.
         *      Return.
         */

/************************************************************************/
snap3(SNAP_KCB | SNAP_KMISA, SNAP_Mbsins, "_Mbsins() normal end");
/************************************************************************/

        return(IMSUCC);
}
