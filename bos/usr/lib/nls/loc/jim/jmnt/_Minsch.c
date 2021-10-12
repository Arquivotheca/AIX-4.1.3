static char sccsid[] = "@(#)15	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Minsch.c, libKJI, bos411, 9428A410j 7/23/92 03:22:55";
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
 * MODULE NAME:         _Minsch
 *
 * DESCRIPTIVE NAME:    Character add in insert mode.
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
 * FUNCTION:            Insert Character at Curser position.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1268 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Minsch()
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Minsch(pt, attrib, mode)
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *                      attrib  :Highilight attribute code.
 *                      mode    :Input mode.(Normal inpur ot Edit input)
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
 *                              _MK_rtn :Call _MK_rtn to be finalized.
 *                              _Mlock  :lock a keyboard.
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
 *                              lastch
 *                              string
 *                              hlatst
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              chcode
 *                              charcont
 *                              convpos
 *                              convlen
 *                              cconvpos
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
 *      insert character.
 */

int _Minsch( pt, attrib )
KCB     *pt;            /* Pointer to Kanji Control Block.              */
uchar   attrib;         /* Highlight attribute code.                    */
{
        register KMISA  *kjsvpt;        /* Pointer to KMISA             */
        uchar           clrstr[2];      /* clear pattern string         */

/************************************************************************/
snap3(SNAP_KCB | SNAP_KMISA, SNAP_Minsch, "_Minsch start position dump");
/************************************************************************/

        /* 0.
         *      Initialize.
         */

        kjsvpt = pt->kjsvpt;  /* Set pointer address to KMISA */

        /* 1.
         *      Make sure that can insert a character here.
         */

        if (pt->lastch >= kjsvpt->curright) {   /* cannot insert here
                                                   because of field full. */

            if ((pt->string[pt->lastch - C_DBCS    ] == C_SPACEH) &&
                (pt->string[pt->lastch - C_DBCS + 1] == C_SPACEL)   ) {
                                                /* Be sure that last character
                                                     is a PC Kanji Code blank */

                if ((kjsvpt->convlen != 0) &&
                    (pt->lastch == kjsvpt->convpos + kjsvpt->convlen)) {
                                                /* Be sure that last character
                                                     is a conversion chracter */

                    kjsvpt->convlen -= C_DBCS; /* Decrease conversion length */

                    kjsvpt->kjcvmap[kjsvpt->convlen    ] = M_KJMNCV;/*Clear  */
                    kjsvpt->kjcvmap[kjsvpt->convlen + 1] = M_KSNCNV;/*kjcvmap*/


                    if ((kjsvpt->cconvlen != 0) &&
                        (pt->lastch ==
                                   (kjsvpt->cconvpos + kjsvpt->cconvlen))) {

                        /* If last character is current conversion character*/
                        /*   decrease current conversion character length   */

                        kjsvpt->cconvlen -= C_DBCS;

                    }
                }

                pt->lastch -= C_DBCS; /* Decrease last character position */

            }
            else { /* the last character is not PC Kanji Code Blank */

                if (kjsvpt->charcont == C_SWON) { /*compulsive insert flag ON*/

                    if ((kjsvpt->convlen != 0) &&
                        (pt->lastch == (kjsvpt->convpos + kjsvpt->convlen))){

                                            /* Be sure that last character
                                                   is conversion character. */

                        if (kjsvpt->kjcvmap[kjsvpt->convlen - C_DBCS + 1] !=
                                                                   M_KSNCNV) {

                                /* if last character is in conversion,
                                           call _MK_rtn to make certain */

                            _MK_rtn(pt, A_CNVDEC);

                                /* and suppress a curser move */

                            kjsvpt->actc2 = A_NOP;

/***********************************************************************/
snap3(SNAP_KCB | SNAP_KMISA, SNAP_Minsch,
"_Minsch error End ! case: input field is full and compulsive insert flag ON, \
but last character was converted one.");
/***********************************************************************/

                            return(IMFAIL);

                        }
                        else {  /* last character is not in conversion. */

                            /* Clear Highlight attribute */

                            pt->hlatst[pt->lastch - C_DBCS    ] =
                            pt->hlatst[pt->lastch - C_DBCS + 1] = K_HLAT0;

                            /* Decrease conversion character length */

                            kjsvpt->convlen -= C_DBCS;


                            if ((kjsvpt->cconvlen != 0) &&
                                (kjsvpt->cconvpos + kjsvpt->cconvlen ==
                                                              pt->lastch)) {

                                /* If the last character is current
                                      conversion character,
                                         Decrease current character length */

                                kjsvpt->cconvlen -= C_DBCS;

                            }
                        }
                    }

                    /* Change last character to PC Kanji code Blank */

                    pt->string[pt->lastch - C_DBCS    ] = C_SPACEH;
                    pt->string[pt->lastch - C_DBCS + 1] = C_SPACEL;

                    /* Decrease last character position (character length) */

                    pt->lastch -= C_DBCS;

                    /* Set keyboard lock flag */

                    _Mlock(pt);
                }
                else {  /* compulsive insert flag OFF */

                    /* Set keyboard lock flag */

                    _Mlock(pt);
/***********************************************************************/
snap3(SNAP_KCB | SNAP_KMISA, SNAP_Minsch,
"_Minsch error End ! case: input field is full but compulsive insert flag OFF");
/***********************************************************************/
                    return(IMFAIL);
                }
            }
        }

        /* 2.
         *      Move backward after curser position character.
         */

        _Mmvch( pt->string, pt->curcol, (pt->lastch - pt->curcol), M_BACKWD,
                C_DBCS, FALSE, (char *)NULL, NULL, NULL);

        /* 3.
         *      write input character to curser position.
         */

        pt->string[pt->curcol    ] = kjsvpt->chcode[0];
        pt->string[pt->curcol + 1] = kjsvpt->chcode[1];

        /* 4.
         *      incriment last character position.
         */

        pt->lastch += C_DBCS;

        /* 5.
         *      Move backward after curser position Highlight string.
         */

        if (kjsvpt->convlen != 0) {  /* There is conversion characters */

                /* Move backward after curser position Highlight string. */

                _Mmvch( pt->hlatst, pt->curcol,
                        (kjsvpt->convpos + kjsvpt->convlen - pt->curcol),
                        M_BACKWD, C_DBCS, FALSE, (char *)NULL, NULL, NULL);

        }

        /* 6.
         *      write input attribute to curser position.
         */

        pt->hlatst[pt->curcol] = pt->hlatst[pt->curcol + 1] = attrib;

        /* 7.
         *      Check attribute and convert srtring,
         *                      and
         *      Move backward after curser position Kanji conversion map string.
         *                      and
         *      write input kanji conversion map code to curser position.
         */

        if (attrib == K_HLAT3) { /* if the catacter input from keyboard is  */
                                 /*     to be converted chracter, change    */
                                 /*     a data if needed.                   */

            if (kjsvpt->convlen != 0) {

                /* if the conversion character is already exist.
                        move the kjcvmap data after from just cuser */

                clrstr[0] = (uchar)M_KJMNCV;
                clrstr[1] = (uchar)M_KSNCNV;
                _Mmvch( kjsvpt->kjcvmap, pt->curcol - kjsvpt->convpos,
                        (kjsvpt->convpos + kjsvpt->convlen - pt->curcol),
                        M_BACKWD, C_DBCS, TRUE, clrstr, 0, 2);

            }

            /* Increase conversion character length */

            kjsvpt->convlen  += C_DBCS;

            /* Increase current conversion character length */

            kjsvpt->cconvlen += C_DBCS;

        }

        /* 8.
         *      set chenged position to display.
         */

        _Msetch(pt, pt->curcol, (pt->lastch - pt->curcol));

        /* 9.
         *      Return.
         */

/************************************************************************/
snap3(SNAP_KCB | SNAP_KMISA, SNAP_Minsch, "_Minsch End position dump");
/************************************************************************/

        return( IMSUCC );
}
