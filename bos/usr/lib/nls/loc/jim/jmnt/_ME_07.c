static char sccsid[] = "@(#)51	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_ME_07.c, libKJI, bos411, 9428A410j 7/23/92 03:18:39";
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
 * MODULE NAME:         _ME_07
 *
 * DESCRIPTIVE NAME:    Editorial character Input process.
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
 * FUNCTION:            Input character on editorial mode.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        480 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _ME_07
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _ME_07(pt)
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IMSUCC  :Success of Execution.
 *
 * EXIT-ERROR:          other   :return code from _Mecho().
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _Mecho  : Character input routine.
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
 *                              NA.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              NA.
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Control Block(KCB).
 *                              NA.
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
 *      Input character on editorial mode.
 */
int  _ME_07(pt)

KCB     *pt;            /* pointer to KCB.                              */

{
        KMISA  *kjsvpt;         /* Pointer to KMISA                     */
        int     _Mecho();       /* character input routine.             */
        int     rc;             /* return code.                         */

        CPRINT(#### _ME_07 start ####);
       snap3 ( SNAP_KCB | SNAP_KMISA , SNAP_ME_07 , "start" );

        /* 0.
         *      Set pointer to KMISA
         */
        kjsvpt = pt->kjsvpt;

        /* 1.
         *      Check curser position.
         */
        if (pt->curcol >= (kjsvpt->convpos + kjsvpt->convlen)) {

                /* 2.a
                 *      Call _Mecho at normal mode.
                 */
                rc = _Mecho ( pt , K_HLAT3 , M_NORMAL );

        }
        else {

                /* 3.b
                 *      Call _Mecho at edit mode.
                 */

                rc = _Mecho ( pt , K_HLAT3 , M_EDITIN );

        }

        CPRINT(#### _ME_07 return ####);

       snap3 ( SNAP_KCB | SNAP_KMISA , SNAP_ME_07 , "end" );

        return( rc );

}
