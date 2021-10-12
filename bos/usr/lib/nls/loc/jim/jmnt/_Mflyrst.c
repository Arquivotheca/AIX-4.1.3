static char sccsid[] = "@(#)04	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mflyrst.c, libKJI, bos411, 9428A410j 7/23/92 03:22:13";
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
 * MODULE NAME:         _Mflyrst
 *
 * DESCRIPTIVE NAME:    Reset Flying Conversion Area.
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
 * FUNCTION:            Reset Flying Conversion Area
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:         700 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mflyrst
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mflyrst( pt )
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
 *                              NA.
 *                      Standard Library.
 *                              memcpy  :Copy characters from memory area
 *                                       A to B.
 *                      Advanced Display Graphics Support Library(GSL).
 *                              NA.
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      See Below.
 *
 *   INPUT:             Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              kjsvpt
 *
 *   OUTPUT:            Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              kjdataf         kjmapf          gramapf
 *                              kanamapf        kanadatf        kanalenf
 *                              fconvpos        fcnverfg        convnum
 *
 * TABLES:              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              IDENTIFY:Module Identify Create.
 *                              SHTOCHPT:Set short data to character
 *                                       pointerd area.
 *                      Standard Macro Library.
 *                              NA.
 *
 * CHANGE ACTIVITY:     Sept. 20 1988 Added by Satoshi Higuchi
 *                      Overflow of the kjdataf etc. support.
 *                      Added to set overflow flag to clear.
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

int     _Mflyrst( pt )

register KCB    *pt;            /* Pointer to Kanji Control Block.      */
{
        register KMISA  *kjsvpt;/* Pointer to Kanji Monitor
                                   Internal Save Area.                  */

        short   loop;           /* Loop Start Counter.                  */
        short   length;         /* Data Length.                         */


        kjsvpt = pt->kjsvpt;



        /*
         *      Flying Conversion Kanji Data Area Reset.
         */
        length = CHPTTOSH( kjsvpt->kjdataf );

        for ( loop = C_DBCS ; loop < length ; loop++ ) {

            kjsvpt->kjdataf[loop] = 0x00;
        };

        SHTOCHPT( kjsvpt->kjdataf, C_DBCS );



        /*
         *      Flying Conversion Kanji Map Area Reset.
         */
        length = CHPTTOSH( kjsvpt->kjmapf );

        for ( loop = C_DBCS ; loop < length ; loop++ ) {

            kjsvpt->kjmapf[loop] = 0x00;
        };

        SHTOCHPT( kjsvpt->kjmapf, C_DBCS );



        /*
         *      Flying Conversion Grammer Map Area Reset.
         */
        length = (short)kjsvpt->gramapf[0];

        for ( loop = C_ANK ; loop < length ; loop++ ) {

            kjsvpt->gramapf[loop] = 0x00;
        };

        kjsvpt->gramapf[0] = C_ANK;



        /*
         *      Flying Conversion Kana Map Area Reset.
         */
        length = (short)kjsvpt->kanamapf[0];

        for ( loop = C_ANK ; loop < length ; loop++ ) {

            kjsvpt->kanamapf[loop] = 0x00;
        };

        kjsvpt->kanamapf[0] = C_ANK;



        /*
         *      Flaying Conversion Kana Data Area Reset.
         */
        length = kjsvpt->kanalenf;

        for ( loop = 0 ; loop < length ; loop++ ) {

            kjsvpt->kanadatf[loop] = 0x00;
        };



        kjsvpt->kanalenf = 0; /* Flying Conversion Kana Data Length Reset. */
        kjsvpt->fconvpos = 0; /* Flying Conversion Position Reset.         */
        kjsvpt->convnum  = 0; /* Flying Conversion Counter Reset.          */
        kjsvpt->fcnverfg = 0; /* Flying Conversion Error Flag Reset.       */
/*======================================================================*/
/* #(B) Sept. 20 1988 S,Higuchi                                         */
/*      Added source.                                                   */
/*      kjsvpt->fcvovfg = C_SWOFF;                                      */
/*======================================================================*/
	kjsvpt->fcvovfg = C_SWOFF;
}
