static char sccsid[] = "@(#)87	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mbsrepn.c, libKJI, bos411, 9428A410j 7/23/92 03:20:53";
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
 * MODULE NAME:         _Mbsrepn
 *
 * DESCRIPTIVE NAME:    Backspace - No restore - in replace mode.
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
 * FUNCTION:            if curser is located at fixed character,
 *                              erase character just before the curser.
 *                      if curser is located at conversion character,
 *                              call _Mbsins();.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        568 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mbsrepn()
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mbsrepn(pt)
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
 *                              _Mbsins :Backspace in insert mode.
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
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              convlen
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Controle Block(KCB).
 *                              string
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
 *      if curser is located at fixed character,
 *              erase character just before the curser.
 *      if curser is located at convert character,
 *              call _Mbsins();.
 */
int _Mbsrepn( pt )
KCB     *pt;            /* Pointer to Kanji Control Block.              */
{
        register KMISA  *kjsvpt;        /* Pointer to KMISA             */
        int             _Mbsins();      /* see EXTERNAL REFERENCES      */

/************************************************************************/
snap3(SNAP_KCB | SNAP_KMISA, SNAP_Mbsrepn, "_Mbsrepn start position dump");
/************************************************************************/

        /* 0.
         *      Initialize.
         */

        kjsvpt = pt->kjsvpt;    /* Set pointer address to KMISA */

        /* 1.
         *      Check a existance of conversion string.
         */

        if (kjsvpt->convlen != 0) {  /* There is the conversion strings */
/************************************************************************/
int trash;
trash = _Mbsins(pt);
snap3(SNAP_KCB | SNAP_KMISA, SNAP_Mbsrepn,
      "_Mbsrepn end position dump. case: there were sevral conversion strings");
return(trash);
/************************************************************************/

                /* return(_Mbsins(pt)); */ /* Call Back Space in insert mode */
        }

        /* 2.
         *      Set Blank code just before the curser.
         */

        pt->string[pt->curcol - C_DBCS    ] = (uchar)C_SPACEH;
        pt->string[pt->curcol - C_DBCS + 1] = (uchar)C_SPACEL;

        /* 3.
         *      Set changed position and length for display.
         */

        _Msetch( pt, (pt->curcol - C_DBCS), C_DBCS );

        /* 4.
         *      Return.
         */

/************************************************************************/
snap3(SNAP_KCB | SNAP_KMISA, SNAP_Mbsrepn, "_Mbsrepn End position dump");
/************************************************************************/

        return(IMSUCC);
}
