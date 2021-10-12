static char sccsid[] = "@(#)49	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_ME_05.c, libKJI, bos411, 9428A410j 7/23/92 03:18:33";
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
 * MODULE NAME:         _ME_05
 *
 * DESCRIPTIVE NAME:    Erase Input field process.
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
 * FUNCTION:            Clear all data in field.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        496 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _ME_05
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _ME_05(pt)
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
 *                              _Msetch :set up convert position for display
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
 *                      Kanji Monitor Control Block(KCB).
 *                              kjsvpt  :pointer to KMISA.
 *                              lastch  :character stringth last position.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              curleft :limit of cursor position on left.
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Control Block(KCB).
 *                              string  :pointer to character string.
 *                              lastch  :character stringth last position.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              NA.
 *                      Trace Block(TRB).
 *                              NA.

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
 *      Clear all data in field.
 */
int  _ME_05(pt)

KCB     *pt;            /* pointer to KCB.                              */

{
        int     _Msetch();      /* @@@@                                 */

        register int     wi1;     /* work integer value.                */
        register KMISA   *kjsvpt; /* pointer to KMISA.                  */


/*##*/  snap3( SNAP_KCB | SNAP_KMISA, SNAP_ME_05 , "start    _ME_05");

        kjsvpt = pt->kjsvpt;      /* set pointer to KMISA.              */

        /* 1.
         *      set blank from top of input field
         *                ro   end of strinh.
         */
        for( wi1 = kjsvpt->curleft ; wi1 < pt->lastch ; wi1 += C_DBCS)
            {
                pt->string[wi1     ] = C_SPACEH;
                pt->string[wi1 + 1 ] = C_SPACEL;
            }

        /* 2.
         *      set start position of change and change length
         *                        to KCB.
         */
        _Msetch(pt, kjsvpt->curleft, pt->lastch - kjsvpt->curleft);

        /* 3.
         *      set character length to Zero.
         */
        pt->lastch = kjsvpt->curleft;

        /* 4.
         *      Return.
         */
/*##*/  snap3( SNAP_KCB | SNAP_KMISA, SNAP_ME_05 , "return _ME_05");
        return( IMSUCC );
}
