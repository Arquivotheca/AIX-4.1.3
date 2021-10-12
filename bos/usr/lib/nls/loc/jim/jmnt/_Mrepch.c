static char sccsid[] = "@(#)38	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mrepch.c, libKJI, bos411, 9428A410j 7/23/92 03:24:24";
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
 * MODULE NAME:         _Mrepch
 *
 * DESCRIPTIVE NAME:    Character add in replase mode.
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
 * FUNCTION:            Replase Character at Curser position.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        616 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mrepch()
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mrepch(pt, attribute)
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
 *                              _Msetch :Set chenged range for reflesh.
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
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              cconlen
 *                              savelen
 *                              stringsv
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
 *      add and replace character.
 */

int _Mrepch( pt, attrib )
KCB     *pt;            /* Pointer to Kanji Control Block.              */
uchar   attrib;         /* Highlight attribute code.                    */
{
        register KMISA  *kjsvpt;        /* Pointer to KMISA             */

/************************************************************************/
snap3(SNAP_KCB | SNAP_KMISA, SNAP_Mrepch, "_Mrepch start position dump");
/************************************************************************/

        /* 0.
         *      Initialize.
         */

        kjsvpt = pt->kjsvpt;

        /* 1.
         *      Make sure that can add a character hear.
         */

        if (pt->curcol >= kjsvpt->curright) {  /* Cannot input hear */
/***********************************************************************/
snap3(SNAP_KCB | SNAP_KMISA, SNAP_Mrepch,
      "_Mrepch error End ! case: Cannot add a character here.");
/***********************************************************************/
                return(IMFAIL);
        }

        /* 2.
         *      Save hidden character in case of necessity.
         */

        if (attrib == K_HLAT3) {

                /* save hidden character */

                kjsvpt->stringsv[kjsvpt->savelen  ] = pt->string[pt->curcol  ];
                kjsvpt->stringsv[kjsvpt->savelen+1] = pt->string[pt->curcol+1];

                /* Increase saved character length */

                kjsvpt->savelen += C_DBCS;
        }
        else {
                ; /* Take no action */
        }

        /* 3.
         *      Add character to string.
         */

        pt->string[pt->curcol    ] = kjsvpt->chcode[0];
        pt->string[pt->curcol + 1] = kjsvpt->chcode[1];

        /* 4.
         *      Call subroutine -_Msetch- to set changed area.
         */

        _Msetch(pt, pt->curcol, C_DBCS);

        /* 5.
         *      Change Highlight attribute string.
         */

        pt->hlatst[pt->curcol    ] = attrib;
        pt->hlatst[pt->curcol + 1] = attrib;

        /* 6.
         *      if attrib is K_HLAT3(stands for an object of conversion)
         *         increase the length of conversion area.
         */

        if (attrib == K_HLAT3) {
                kjsvpt->convlen  += C_DBCS;
                kjsvpt->cconvlen += C_DBCS;
        }

        /* 7.
         *      Return.
         */

/************************************************************************/
snap3(SNAP_KCB | SNAP_KMISA, SNAP_Mrepch, "_Mrepch End position dump");
/************************************************************************/

        return( IMSUCC );
}
