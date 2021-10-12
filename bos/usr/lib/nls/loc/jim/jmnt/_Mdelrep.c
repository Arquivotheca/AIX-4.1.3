static char sccsid[] = "@(#)97	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mdelrep.c, libKJI, bos411, 9428A410j 7/23/92 03:21:42";
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
 * MODULE NAME:         _Mdelrep
 *
 * DESCRIPTIVE NAME:    Delete in insert mode.
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
 * FUNCTION:            Delete character just curser, and
 *                      move string from immidiately after curser forward.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1328 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mdelrep()
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mdelrep(pt)
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
 *                              memset  :fill in a constant value in memeory.
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
 *                              cconvpos
 *                              cconvlen
 *                              kjcvmap
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

int _Mdelrep( pt )
KCB     *pt;            /* Pointer to Kanji Control Block.              */
{
        register KMISA  *kjsvpt;        /* Pointer to KMISA             */
        uchar           clrstr[2];      /* Clear data array             */
        int             csv;            /* Conversion status value      */
        short           start;          /* Conversion unit start positi */
        short           end;            /* Conversion unit end position */
        short           nochng;         /* No change flag               */
        int             _Msetch();      /* see EXTERNAL REFERENCES      */
        int             _Mmvch();       /* see EXTERNAL REFERENCES      */
        char            *memset();      /* see EXTERNAL REFERENCES      */

/************************************************************************/
snap3(SNAP_KCB | SNAP_KMISA, SNAP_Mdelrep, "_Mdelrep start position dump");
/************************************************************************/

        /* 0.
         *      Initialize.
         */

        kjsvpt = pt->kjsvpt;    /* Set pointer address to KMISA */

        /* 1.
         *      If there aren't conversion characters, call _Mdelins.
         */

        if (kjsvpt->convlen == 0) {
/************************************************************************/
int trash;
trash = _Mdelins(pt);
snap3(SNAP_KCB | SNAP_KMISA, SNAP_Mdelrep,
      "_Mdelrep end position dump. case: there were no conversion string");
return(trash);
/************************************************************************/
                /* return(_Mdelins(pt)); */ /* if there isn't conversion
                                                 string, call delete subroutine
                                                 in insert mode */
                }

        /* 2.
         *      Set Blank code at the curser posistion.
         *                      and
         *      Move character string forward from immidiately after curser.
         */

        pt->string[pt->curcol    ] = (uchar)C_SPACEH;
        pt->string[pt->curcol + 1] = (uchar)C_SPACEL;

        clrstr[0] = (uchar)C_SPACEH;
        clrstr[1] = (uchar)C_SPACEL;
        _Mmvch( pt->string, pt->curcol + C_DBCS,
               (kjsvpt->convpos + kjsvpt->convlen - (pt->curcol + C_DBCS)),
                M_FORWD, C_DBCS, TRUE, clrstr, 0, C_DBCS);

        /* 3.
         *      Set clear attribute code at the curser posistion.
         *                      and
         *      Move hilight attribute string forward from
         *              immidiately after curser.
         */

        pt->hlatst[pt->curcol    ] =
        pt->hlatst[pt->curcol + 1] = K_HLAT0;

        clrstr[0] = (uchar)K_HLAT0;
        _Mmvch( pt->hlatst, (pt->curcol + C_DBCS),
               (kjsvpt->convpos + kjsvpt->convlen - (pt->curcol + C_DBCS)),
                M_FORWD, C_DBCS, TRUE, clrstr, 0, 1             );

        /* 4.
         *      Set clear kanji conversion map code at the curser posistion.
         *                      and
         *      Move kjmap string forward from immidiately after curser.
         */

        clrstr[0] = (uchar)M_KJMNCV;
        clrstr[1] = (uchar)M_KSNCNV;
        _Mmvch( kjsvpt->kjcvmap, (pt->curcol + C_DBCS - kjsvpt->convpos),
               (kjsvpt->convpos + kjsvpt->convlen - (pt->curcol - C_DBCS)),
                M_FORWD, C_DBCS, TRUE, clrstr, 0, 2             );

        /* 5.
         *      Decrease current conversion character length.
         */

        kjsvpt->cconvlen -= C_DBCS;

        /* 6.
         *      Decrease current conversion character length.
         */

        kjsvpt->convlen -= C_DBCS;

        /* 7.
         *      Check kjcvmap then change highlight attribute
         */

        if ((pt->curcol <  kjsvpt->convpos) ||
            (pt->curcol >= kjsvpt->convpos + kjsvpt->convlen)) {

                /* The curser is not in conversion string */

                csv = M_CUROUT;
        }
        else {

                /* The curser is in conversion string */

                csv = kjsvpt->kjcvmap[pt->curcol - kjsvpt->convpos + 1];
        }

        switch(csv) {
            case M_CUROUT: /* The curser is not in conversion string        */
            case M_KSCNUM: /* This character is not an object of conversion */

                break; /* Take no action */

            case M_KSNCNV: /* This character is not converted yet */

                pt->hlatst[pt->curcol    ] =
                pt->hlatst[pt->curcol + 1] = (uchar)K_HLAT3;

                    /* Set Hightlight attribute of Reverse and underline */

                kjsvpt->cconvlen += C_DBCS;

                    /* Increase current conversion length */

                break;

            case M_KSCNVK: /* In conversion (KANJI/SEISHO)         */
            case M_KSCNSK: /* In conversion (KANJI/SEISHO/TANKAN)  */
            case M_KSCNVY: /* In conversion (YOMI)                 */
            case M_KSCNSY: /* In conversion (YOMI/TANKAN)          */

                if (kjsvpt->cconvlen == 0) {

                    /* If the current conversion length comes to 0,
                                         try to change highlight attribute */

                    _Mckbk(pt, pt->curcol, &start, &end, &nochng);

                        /* Check Conversion status and
                                  get Hilighting block position and length */

                    if (nochng == C_SWOFF) { /* It is necessaly to
                                                          change attribute */

                        memset(pt->hlatst + start, (char)K_HLAT3,
                                                   (int)(end - start + 1));

                            /* Set highlight attribute: Reverse and Underline */

                        kjsvpt->cconvpos = start;

                            /* Set new current conversion string position */

                        kjsvpt->cconvlen = end - start + 1;

                            /* Set new current conversion string length */

                    }
                }

                break;

            default:

                break;

        } /* END OF SWITCH */

        /* 8
         *   Restore saved character or decrease last character position.
         */

        if ((kjsvpt->savelen != 0) &&                   /* Make sure that   */
            ((kjsvpt->savepos + kjsvpt->savelen) >      /* it have to restore */
             (kjsvpt->convpos + kjsvpt->convlen)  ) ) { /* saved character  */

                /* Restore a saved character */

                pt->string[kjsvpt->convpos + kjsvpt->convlen    ] =
                        kjsvpt->stringsv[kjsvpt->savelen - C_DBCS    ];
                pt->string[kjsvpt->convpos + kjsvpt->convlen + 1] =
                        kjsvpt->stringsv[kjsvpt->savelen - C_DBCS + 1];

                /* Decreace saved character length */

                kjsvpt->savelen -= C_DBCS;

        }
        else {  /* It is no necessity to restore */

                /* Decrease last character position (character length)  */

                pt->lastch -= C_DBCS;

        }

        /* 9.
         *      Set changed position and length for display.
         */

        _Msetch( pt, pt->curcol,
                 (kjsvpt->convpos + kjsvpt->convlen + C_DBCS) - pt->curcol);

        /* 10.
         *      Return.
         */

/************************************************************************/
snap3(SNAP_KCB | SNAP_KMISA, SNAP_Mdelrep, "_Mdelrep End position dump");
/************************************************************************/

        return(IMSUCC);
}
