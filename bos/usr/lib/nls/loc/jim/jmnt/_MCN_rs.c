static char sccsid[] = "@(#)41	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_MCN_rs.c, libKJI, bos411, 9428A410j 7/23/92 03:18:03";
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
 * MODULE NAME:         _MCN_rs
 *
 * DESCRIPTIVE NAME:    Reset unsuccessful indicator.
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
 * FUNCTION:            Reset indicator from unsuccessful conversion status.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        476 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _MCN_rs
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _MCN_rs( pt )
 *
 *  INPUT:              pt      :Pointer to Kanji Controle Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         NA.
 *
 * EXIT-ERROR:          NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _Mindset() : Indicator Set Routine.
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
 *                              kjsvpt
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              NA.
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Controle Block(KCB).
 *                              NA.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              convimp nextact
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
 *      This module resets indicator from unsuccessful conversion status.
 */
int  _MCN_rs( pt )

KCB     *pt;            /* Pointer to Kanji Control Block               */
{
        register        KMISA   *kjsvpt;        /* Pointer to KMISA     */

        extern  int     _Mindset();     /* Indicator Set Routine        */

        int     rc;             /* Return Code.                         */

snap3(SNAP_KCB | SNAP_KMISA,SNAP_MCN_rs,"start _MCN_rs");

        /*
         *      Pointer to Kanji Montor Internal Save Area
         */

        kjsvpt = pt->kjsvpt;

        /* 1. 1
         *      Reset Conversion Impossible Flag (OFF)
         */

        kjsvpt->convimp = K_CVIPOF;

        /* 1. 2
         *      Reset Next Function (OFF)
         */

        kjsvpt->nextact = kjsvpt->nextact & ~M_CNRSON;

        /* 1. 3
         *      Reset Left Indicator
         */

        rc = _Mindset( pt, M_INDL );

        /* 1. 4
         *      Return Code.
         */

snap3(SNAP_KCB | SNAP_KMISA,SNAP_MCN_rs,"return _MCN_rs");

        return( rc );
}
