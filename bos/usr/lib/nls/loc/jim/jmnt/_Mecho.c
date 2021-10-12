static char sccsid[] = "@(#)00	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mecho.c, libKJI, bos411, 9428A410j 7/23/92 03:21:52";
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
 * MODULE NAME:         _Mecho
 *
 * DESCRIPTIVE NAME:    Character Input routine
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
 * FUNCTION:            Call character adding subroutne in condition then.
 *                      And change chracter attribute if need be.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1424 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mecho()
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            Module( parm1,parm2,parm3 )
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *                      attrib  :Highlight attribute code.
 *                      mode    :Character Input mode. (Normal or Edit)
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
 *                              _Maddch :add a Character on condition that
 *                                       curser locate after last character.
 *                              _Minsch :add a Character in condition of
 *                                       insert mode.
 *                              _Mrepch :add a character in condition of
 *                                       replace mode.
 *                              _Minssv :add a character on condition that
 *                                       mode is replace and curser locate
 *                                       in YOMI part.
 *                              _Mlock  :Keyboard lock process set routine.
 *                              _Mckbk  :Get the range of GOKI
 *                              _Msetch :set changed range.
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
 *                              hlatst  :Attribute strings.
 *                              curcol  :Curser Position.
 *                              lastch  :the position after last character.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              kjcvmap :Kanji Conversion Status map.
 *                              convpos :Start position of Conversion range.
 *                              convlen :Length of Conversion range.
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Controle Block(KCB).
 *                              hlatst  :Highlight attribute string.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              NA.
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
 *      Please Descripte This Module.
 */

int _Mecho( pt, attrib, mode)
KCB     *pt;            /* Pointer to Kanji Control Block.              */
uchar   attrib;         /* Highlight attribute code.                    */
uchar   mode;           /* Chartacter input mode.                       */
{
    register KMISA  *kjsvpt;        /* Pointer to KMISA                 */
    register int    i;              /* Loop counter                     */
    int             cvst1;          /* hold conversion status value     */
    int             cvst2;          /* hold conversion status value     */
    short           start, end;     /* GOKI start and end position      */
    short           nochng;         /* NO change flag                   */
    int             chst, chend;    /* chenge posithion counter         */
    int             skipflg;        /* Execution skip flag              */
    int             ret_code;       /* Return Code.                     */

    int             _Maddch();      /* See MODULE SPECIFICATIONS        */
    int             _Minsch();      /* See MODULE SPECIFICATIONS        */
    int             _Mrepch();      /* See MODULE SPECIFICATIONS        */
    int             _Minssv();      /* See MODULE SPECIFICATIONS        */
    int             _Msetch();      /* See MODULE SPECIFICATIONS        */
    int             _Mlock();       /* See MODULE SPECIFICATIONS        */
    int             _Mckbk();       /* See MODULE SPECIFICATIONS        */

/************************************************************************/
snap3(SNAP_KCB | SNAP_KMISA, SNAP_Mecho, "_Mecho start position dump");
/************************************************************************/

    /* 0.
     *      Initial value set
     */

    kjsvpt  = pt->kjsvpt; /* Set pointer to KMISA */

    skipflg = FALSE;

    /* 1.
     *      Compare curser position and last character position.
     */

    if (pt->curcol >= pt->lastch) { /* Curser locate after last character. */
        if ((ret_code = _Maddch(pt, attrib)) == IMFAIL) { /* Call Character*/
                                                          /* add routine   */

            /* If it failed adding a character, set a keyboard lock flag   */

            _Mlock(pt);
/***********************************************************************/
snap3(SNAP_KCB | SNAP_KMISA, SNAP_Mecho,
      "_Mecho error End ! case: _Maddch returns IMFAIL");
/***********************************************************************/
            return(ret_code);
        }
    }
    else {                              /* Cuser locate before last char. */
        if (pt->repins == K_INS) {      /* insert mode */

            /* Call character input routne   */

            if ((ret_code = _Minsch(pt, attrib)) == IMFAIL) {

                /* If it failed adding a character, set a keyboard lock flag  */

                _Mlock(pt);
/***********************************************************************/
snap3(SNAP_KCB | SNAP_KMISA, SNAP_Mecho,
      "_Mecho error End ! case: _Minsch returns IMFAIL");
/***********************************************************************/
                return(ret_code);
            }
        }
        else {                          /* replace mode */
            if (mode == M_NORMAL) {

                /* Call character replace routine */

                if ((ret_code = _Mrepch(pt, attrib)) == IMFAIL) {

                    /* If it failed adding a character, */
                    /* set a keyboard lock flag         */

                    _Mlock(pt);
/***********************************************************************/
snap3(SNAP_KCB | SNAP_KMISA, SNAP_Mecho,
      "_Mecho error End ! case: _Mrepch returns IMFAIL");
/***********************************************************************/
                    return(ret_code);
                }
            }
            else {  /* mode is EDITORIAL mode */

                /* Call character insert in conversion character area */

                if ((ret_code = _Minssv(pt, attrib)) == IMFAIL) {

                    /* If it failed adding a character, */
                    /* set a keyboard lock flag         */

                     _Mlock(pt);
/***********************************************************************/
snap3(SNAP_KCB | SNAP_KMISA, SNAP_Mecho,
      "_Mecho error End ! case: _Minssv returns IMFAIL");
/***********************************************************************/
                     return(ret_code);
                }
            }
        }
    }

    /* 2.
     *      Attribute change routine
     */

    if (kjsvpt->convlen != 0) { /* Thjere are convesion characters. */

        /* 2.a
         *      Get conversion status part value
         *      at immdiately after curser.
         */

        /* Check curser position */

        if ((pt->curcol + C_DBCS <  kjsvpt->convpos) ||
            (pt->curcol + C_DBCS >= kjsvpt->convpos + kjsvpt->convlen)) {

            /* The curser is not in conversion string */

            cvst1 = M_CUROUT;

        }
        else {

            /* The curser is in conversion string */

            /* Get Conversion Status */

            cvst1 = kjsvpt->kjcvmap[pt->curcol + C_DBCS - kjsvpt->convpos + 1];

        }

        /* 2.b
         *      Get conversion status part value
         *      at just befor cueser.
         */

        if ((pt->curcol - C_DBCS <  kjsvpt->convpos) ||
            (pt->curcol - C_DBCS >= kjsvpt->convpos + kjsvpt->convlen)) {

            /* The curser is not in conversion string */

            cvst2 = M_CUROUT;
        }
        else {

            /* The curser is in conversion string */

            /* Get Conversion Status */

            cvst2 = kjsvpt->kjcvmap[pt->curcol - C_DBCS - kjsvpt->convpos + 1];
        }

        /* 2.c
         *      Case of cvst2 indicates YOMI.
         *
         *      The character just before curser is YOMI.
         */

        if (cvst2 == M_KSNCNV) {

            /*
             *      Change Attribute in GOKI range just after cuser.
             */

            switch(cvst1) {
                case M_KSCNVK:      /* in conversion (KANJI)        */
                case M_KSCNSK:      /* in conversion (KANJI/TANKAN) */
                case M_KSCNVY:      /* in conversion (YOMI)         */
                case M_KSCNSY:      /* in conversion (YOMI/TANKAN)  */

                    /* Get position and length of GOKI */

                    _Mckbk(pt, pt->curcol + C_DBCS, &start, &end, &nochng);

                    chst  = -1; /* Chenge trace valiable clear */
                    chend = -1; /* Chenge trace valiable clear */

                    for (i = start; i <= end; i++) {

                        /*
                         * Check attribute. If attribute is K_HLAT3
                         * (Reverse and underline), change to K_HLAT2
                         * (underline).
                         */

                        if (pt->hlatst[i] == K_HLAT3) {

                            /* Change attribute data to UNDERLINE only */

                            pt->hlatst[i] = K_HLAT2;

                            if (chst == -1) { /* Still not changed */

                               chst = chend = i; /* Set start position */

                            }
                            else {  /* already change strated */

                               chend = i;   /* renew end position */

                            }
                        }
                    }

                    _Msetch(pt, chst, chend - chst + 1); /* Set refresh area */

                    break;

                case M_KSNCNV:      /* Not converted YOMI           */

                    if (kjsvpt->kjcvmap[pt->curcol - kjsvpt->convpos + 1]
                                                               == M_KSNCNV) {

                                       /*If the character at curser position  */
                        skipflg = TRUE;/*is YOMI, skip next execution block   */
                                       /*that Changing attribute before cueser*/

                    }

                    break;

                case M_KSCNUM:      /* Out of the object of conversion */
                case M_CUROUT:      /* curser locate out of conversion range */
                default:
                        /* take no action */
                    break;
            }


            if (!skipflg) {

                /*
                 *      Change attribute in GOKI range before cueser.
                 *
                 */

                /* Get position and length of GOKI */

                _Mckbk(pt, pt->curcol, &start, &end, &nochng);

                chst  = -1; /* Chenge trace valiable clear */
                chend = -1; /* Chenge trace valiable clear */

                for (i = start; i <= end; i++) {

                    /*
                     * Check attribute. If attribute is K_HLAT2 (underline),
                     * change to K_HLAT3 (Reverse and underline).
                     */

                    if (pt->hlatst[i] == K_HLAT2) {

                        /* Change attribute data to Reverse and Underline */

                        pt->hlatst[i] = K_HLAT3;

                        if (chst == -1) {  /* Still not changed */

                           chst = chend = i; /* Set start position */

                        }
                        else { /* already change strated */

                           chend = i; /* renew end position */

                        }
                    }
                } /* end of for loop */

                _Msetch(pt, chst, chend - chst + 1); /* Set refresh area */


                /*  Set current conversion character area. */

                kjsvpt->cconvpos = start;
                kjsvpt->cconvlen = end - start + 1;

            } /* Skip Block END */
        }
        else {

            ;   /* Take no action */

        }
    }
/************************************************************************/
snap3(SNAP_KCB | SNAP_KMISA, SNAP_Mecho, "_Mecho End position dump");
/************************************************************************/
    return(ret_code);
}
