static char sccsid[] = "@(#)63	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_MK_b4b.c, libKJI, bos411, 9428A410j 7/23/92 03:19:19";
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
 * MODULE NAME:         _MK_b4b
 *
 * DESCRIPTIVE NAME:    Hiragana / Katakana Conversion (Mode 2)
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
 * FUNCTION:            Convert katakana in the string into hiragana.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        644 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _MK_b4b
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _MK_b4b( pt )
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
 *                              _Mhrktsw() : Hiragana/Katakana Conversion
 *                                           Routine.
 *                              _Msetch()  : Set Changed Position and
 *                                           Changed Length for Display.
 *                      Standard Library.
 *                              memset()
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
 *                              cconvpos        cconvlen
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Controle Block(KCB).
 *                              hlatst
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              actc3   kanadata        kanalen
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
 *      This mode dose,
 *              1. Call Hiragana/Katakana Conversion Routine.
 *              2. Reset Hiragana Attribute and
 *                 Mode to Continuous Input Mode.
 */
int  _MK_b4b( pt )

KCB     *pt;            /* Pointer to Kanji Control Block               */
{
        extern  int     _Mhrktsw();     /* Hiragana/Katakana Conversion */
                                        /* Routine                      */
        extern  int     _Msetch();      /* Set Changed Position and     */
                                        /* Change Length for Display    */

        register        KMISA   *kjsvpt;        /* Pointer to KMISA     */

        uchar   *hlatst;        /* Highlight Attribute                  */

        short   pos;            /* Chenged Position                     */
        short   len;            /* Chenged Length                       */

        int     rc;             /* Return Code.                         */

snap3(SNAP_KCB | SNAP_KMISA,SNAP_MK_b4b,"start _MK_b4b");

        /*
         * Pointer to Kanji Monitor Internal Save Area
         */

        kjsvpt = pt->kjsvpt;


        /* 1. 1
         *      Call Hiragana/Katakana Conversion Routine
         */

        rc = _Mhrktsw( pt );

        /* 1. 2
         *      Set Highlight Attribute (K_HLAT3)
         */

        hlatst  = pt->hlatst;           /* Work Highlight Attr. Pointer */
        hlatst += kjsvpt->cconvpos;     /* Move Work Pointer             */

        /* Set Highlight Attribute to Reverse and Underline */
        memset( hlatst, K_HLAT3, kjsvpt->cconvlen );

        /* 1. 3
         *      Set Hiragana/Katakana Mode
         */

        kjsvpt->hkmode = K_HKTEN;

        /* 1. 4
         *      Set KMAT Action Code 3
         *              for Continuous Input Mode
         */

         kjsvpt->actc3 = A_CONINP ;

        /* 1. 5
         *      Set Changed Position and Changed Length
         */

        pos = kjsvpt->cconvpos;         /* Changed Postion              */
        len = kjsvpt->cconvlen;         /* Changed Length               */

        /*
         *      Set Conversion Position and Length
         */
        rc = _Msetch( pt, pos, len );

        /* 1. 6
         *      Clear Hiragana / Katakana Map
         */
        memset(kjsvpt->kanadata,'\0',kjsvpt->kanalen+1);

        kjsvpt->kanalen = 0;                    /* Map Length Reset     */

        /* 1. 7
         *      Return Code.
         */

snap3(SNAP_KCB | SNAP_KMISA,SNAP_MK_b4b,"return _MK_b4b");

        return( rc );
}
