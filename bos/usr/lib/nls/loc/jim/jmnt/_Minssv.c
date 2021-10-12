static char sccsid[] = "@(#)16	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Minssv.c, libKJI, bos411, 9428A410j 7/23/92 03:22:59";
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
 * MODULE NAME:         _Minssv
 *
 * DESCRIPTIVE NAME:    insert character in conversion area in insert mode.
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
 * FUNCTION:            Insert Character in conversion area and
 *                      save hidden character.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1540 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Minssv()
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Minssv(pt, attrib)
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *                      attrib  :Highilight attribute code.
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
 *                              stringsv
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
 *                              lastch
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              convlen
 *                              cconvlen
 *                              stringsv
 *                              savelen
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
 *      insert character and save hidden character.
 */

int _Minssv( pt, attrib )
KCB     *pt;            /* Pointer to Kanji Control Block.              */
uchar   attrib;         /* Highlight attribute code.                    */
{
        register KMISA  *kjsvpt;        /* Pointer to KMISA             */
        uchar           clrstr[2];      /* clear pattern string         */

/************************************************************************/
snap3(SNAP_KCB | SNAP_KMISA, SNAP_Minssv, "_Minssv start position dump");
/************************************************************************/

        /* 0.
         *      Initialize.
         */

        kjsvpt = pt->kjsvpt;  /* Set pointer address to KMISA */

        /* 1.
         *      Make sure that can insert a character here.
         */

        if ((kjsvpt->convlen != 0) &&
            ((kjsvpt->curright - (kjsvpt->convpos + kjsvpt->convlen))
                                                               < C_DBCS)) {
                           /* cannot insert here because of field full. */

            if ((pt->string[pt->lastch - C_DBCS    ] == C_SPACEH) &&
                (pt->string[pt->lastch - C_DBCS + 1] == C_SPACEL)   ) {
                                                /* Be sure that last character
                                                     is a PC Kanji Code blank */

                /* Clear highlight attribute */

                pt->hlatst[pt->lastch - C_DBCS    ] = K_HLAT0;
                pt->hlatst[pt->lastch - C_DBCS + 1] = K_HLAT0;

                /* Decrease conversion length */

                kjsvpt->convlen -= C_DBCS;

                /* Crear kjcvmap */

                kjsvpt-> kjcvmap[kjsvpt->convlen    ] =M_KJMNCV;
                kjsvpt-> kjcvmap[kjsvpt->convlen + 1] =M_KSNCNV;
                if ((kjsvpt->cconvlen != 0) &&
                    (pt->lastch == (kjsvpt->cconvpos + kjsvpt->cconvlen))) {

                        /* If lastcharacter is current conversion character,*/
                        /*   decrease current conversion character length   */

                        kjsvpt->cconvlen -= C_DBCS;

                }
                if ((kjsvpt->savelen != 0) &&
                    (pt->lastch == (kjsvpt->savepos + kjsvpt->savelen))) {

                    /* If the saved character is existing at last charcter */
                    /*    Restore saved character to field                 */

                    pt->string[pt->lastch - C_DBCS    ] =
                        kjsvpt->stringsv[kjsvpt->savelen - C_DBCS];
                    pt->string[pt->lastch - C_DBCS + 1] =
                        kjsvpt->stringsv[kjsvpt->savelen - C_DBCS + 1];

                    /* Decrease saved character length */

                    kjsvpt->savelen -= C_DBCS;

                }
                else {

                    /* If the saved character isn't existing */
                    /*    Decrease last character position.  */

                    pt->lastch -= C_DBCS;

                }

            }
            else { /* last character is not PC Kanji Code Blank */

                if (kjsvpt->charcont == C_SWON) { /*compulsive insert flag ON*/

                    if (kjsvpt->kjcvmap[kjsvpt->convlen - C_DBCS + 1]
                                                               != M_KSNCNV) {

                                        /* and last character is not Yomi.
                                                        (is in conversion) */

                        /* Call _MK_rtn to make cirtain of conversion */

                        _MK_rtn(pt, A_CNVDEC);

                        /* Set keyboard lock flag */

                        _Mlock(pt);

                        /* Suppress a curser move */

                        kjsvpt->actc2 = A_NOP;

/***********************************************************************/
snap3(SNAP_KCB | SNAP_KMISA, SNAP_Minssv,
"_Minssv error End ! case: input field is full and compulsive insert flag ON, \
but last character was converted one.");
/***********************************************************************/
                        return(IMFAIL);
                    }
                    else { /* Last character is YOMI (is not in conversion) */

                        /* Clear highlight Attribute */

                        pt->hlatst[pt->lastch - C_DBCS    ] =
                        pt->hlatst[pt->lastch - C_DBCS + 1] = K_HLAT0;

                        /* Decrease conversion character length */

                        kjsvpt->convlen -= C_DBCS;

                        if ((kjsvpt->cconvlen != 0) && /*convelen can be ZERO*/
                            ((kjsvpt->convpos  + kjsvpt->convlen + C_DBCS) ==
                             (kjsvpt->cconvpos + kjsvpt->cconvlen        ))) {

                                /* If the last character is current
                                       conversion character, decrease
                                           current conversion length    */

                            kjsvpt->cconvlen -= C_DBCS;

                        }

                        if((kjsvpt->savelen != 0) &&
                           (pt->lastch == (kjsvpt->savepos + kjsvpt->savelen))){

                                /* If the saved character is existing at
                                    last chracter, restore saved character */

                            /* Restore saved character */

                            pt->string[pt->lastch - C_DBCS    ] =
                                kjsvpt->stringsv[kjsvpt->savelen - C_DBCS];
                            pt->string[pt->lastch - C_DBCS + 1] =
                                kjsvpt->stringsv[kjsvpt->savelen - C_DBCS + 1];

                            /* Decrease saved character length */

                            kjsvpt->savelen -= C_DBCS;
                        }
                        else { /* the saved character is not exist */

                            /* Clear highlight attribute */

                            pt->hlatst[pt->lastch - C_DBCS    ] = C_SPACEH;
                            pt->hlatst[pt->lastch - C_DBCS + 1] = C_SPACEL;

                            /* Decrease last character position */

                            pt->lastch -= C_DBCS;

                        }

                        /* Set keyboard lock flag */

                        _Mlock(pt);
                    }
                }
                else {  /* compulsive insert flag OFF */

                    /* Set keyboard lock flag */

                    _Mlock(pt);
/***********************************************************************/
snap3(SNAP_KCB | SNAP_KMISA, SNAP_Minssv,
"_Minssv error End ! case: input field is full but compulsive insert flag OFF");
/***********************************************************************/
                    return(IMFAIL);
                }
            }
        }

        /* 2.
         *      if there are hidden characters, save hidden characters.
         *      else incliment last character position.
         */

        if (pt->lastch > (kjsvpt->convpos + kjsvpt->convlen)) {

            /* (if convlen equal 0, it is no problem.) */
            /* There is a character will be hidden. */

            /* Save hidden character */

            kjsvpt->stringsv[kjsvpt->savelen] =
                pt->string[kjsvpt->convpos + kjsvpt->convlen];
            kjsvpt->stringsv[kjsvpt->savelen + 1] =
                pt->string[kjsvpt->convpos + kjsvpt->convlen + 1];

            /* Increase saved character length */

            kjsvpt->savelen += C_DBCS;
        }
        else { /* There is not a character will be hidden. */

            /* Increase last character position */

            pt->lastch += C_DBCS;

        }

        /* 3.
         *      Move backward after curser position character.
         */

        _Mmvch( pt->string, pt->curcol,
               (kjsvpt->convpos + kjsvpt->convlen - pt->curcol),
                M_BACKWD, C_DBCS, NULL, (char *)NULL, NULL, NULL);

        /* 4.
         *      write input character to curser position.
         */

        pt->string[pt->curcol    ] = kjsvpt->chcode[0];
        pt->string[pt->curcol + 1] = kjsvpt->chcode[1];

        /* 5.
         *      Move backward after curser position Highlight string.
         */

        _Mmvch( pt->hlatst, pt->curcol,
               (kjsvpt->convpos + kjsvpt->convlen - pt->curcol),
                M_BACKWD, C_DBCS, TRUE, clrstr, 0, 1);

        /* 6.
         *      write input attribute to curser position.
         */

        pt->hlatst[pt->curcol] = pt->hlatst[pt->curcol + 1] = attrib;

        /* 7.
         *      Move backward after curser position Kanji conversion map string.
         *                      and
         *      write input kanji conversion map code to curser position.
         */

        clrstr[0] = (uchar)M_KJMNCV;
        clrstr[1] = (uchar)M_KSNCNV;
        _Mmvch( kjsvpt->kjcvmap, pt->curcol - kjsvpt->convpos,
               (kjsvpt->convpos + kjsvpt->convlen - pt->curcol),
                M_BACKWD, C_DBCS, TRUE, clrstr, 0, 2);

                                /* If _Mmvch failed, it is no problem */

        /* 6.
         *      Incliment conversion character length,
         *      and current conversion length.
         */

        kjsvpt->convlen  += C_DBCS;
        kjsvpt->cconvlen += C_DBCS;

        /* 7.
         *      set chenged position to display.
         */

        _Msetch(pt, pt->curcol,
                ((kjsvpt->convpos + kjsvpt->convlen) - pt->curcol));

        /* 8.
         *      Return.
         */

/************************************************************************/
snap3(SNAP_KCB | SNAP_KMISA, SNAP_Minssv, "_Minssv End position dump");
/************************************************************************/

        return( IMSUCC );
}
