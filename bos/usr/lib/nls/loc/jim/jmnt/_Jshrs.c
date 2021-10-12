static char sccsid[] = "@(#)38	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Jshrs.c, libKJI, bos411, 9428A410j 7/23/92 03:17:53";
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
 * MODULE NAME:         _Jshrs
 *
 * DESCRIPTIVE NAME:    Forced shift status reset.
 *
 * COPYRIGHT:           5601-061 COPYRIGHT IBM CORP 1988
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              Kanji Monitor V1.0
 *
 * CLASSIFICATION:      OCO Source Material - IBM Confidential.
 *                      (IBM Confidential-Restricted when aggregated)
 *
 * FUNCTION:            Reset the shift status to default or undefined.
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
 * ENTRY POINT:         _Jshrs
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Jshrs(pt)
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IMSUCC  :Success of Execution.
 *
 * EXIT-ERROR:
 *                      NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              NA.
 *                      Standard Library.
 *                              NA.
 *                      Advanced Display Graphics Support Library(GSL).
 *                              NA.
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      See Below.
 *
 *   INPUT:             Kanji Monitor Control Block(KCB).
 *                              *kjsvpt  : pointer to KMISA.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              _Rkc     : initial value of romaji mode.
 *                              kmact    : Active/Inactive of input field.
 *
 *   OUTPUT:            Kanji Monitor Control Block(KCB).
 *                              shift   : change shift flag.
 *                              shift1  : AN/KANA shift.
 *                              shift2  : RKC shift.
 *
 *                      Kanji Monitor Control Block(KCB).
 *                              NA.
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              IDENTIFY:Module Identify Create.
 *
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
 *   This module ckecks no active field exists and resets shift flags
 *   in KCB.
 *
 */

int _Jshrs(pt)

KCB     *pt ;         /*  pointer to Kanji Control Block        */

{
        KMISA   *kjsvpt;        /*   pointer to kanji moniter ISA.     */
        int     rc ;            /*   return code.                      */

        /* ### */
        snap3( SNAP_KCB | SNAP_KMISA , SNAP_Jshrs , "start" );

        kjsvpt = pt->kjsvpt;    /*  set pointer to kanji moniter ISA. */
        rc = IMSUCC;            /*  set return value.                 */

        /* 1.
         *   check  !  no active field.
         *   reset  !  shift flag of KCB.
         */
                     /*  Active.   */
         if( kjsvpt->kmact == K_IFACT )  {

                rc = KMIFACE;            /*  Set return code   */
                     /*  Inactive. */
         } else {

                /*
                 *      Set shift flags.
                 */

                                       /* Check AN/KANA shift flag in KCB */
                if( pt->shift1 != K_ST1UDF ) {

                                       /*  Set shift flag in KCB  */
                        pt->shift = pt->shift | K_STSFT1;
                };

              /*  Check RKC shift flag in KCB and RKC mode from PROFILE*/
                if (   ( pt->shift2 == K_ST2ROF &&
                         kjsvpt->kmpf[0].rkc == K_ROMON ) ||
                       ( pt->shift2 == K_ST2RON &&
                         kjsvpt->kmpf[0].rkc == K_ROMOFF )   ) {

                                       /*  Set shift flag in KCB  */
                        pt->shift = pt->shift | K_STSFT2;

                };

                /*
                 *      Set Alpha._Num./Katakana/Hiragana shift flag.
                 */

                pt->shift1 = K_ST1UDF;

                /*
                 *      Set RKC shift flag.
                 */

                             /*  Check RKC mode from PROFILE*/
                if( kjsvpt->kmpf[0].rkc == K_ROMON ) {

                                       /*  Set RKC shift flag in KCB  */
                        pt->shift2 = K_ST2RON;

                } else {

                                       /*  Set RKC shift flag in KCB  */
                        pt->shift2 = K_ST2ROF;
                };
         };

        /* 2.
         *   Return Value.
         */

        /* ### */
        snap3( SNAP_KCB | SNAP_KMISA , SNAP_Jshrs , "end");

        return( rc );
}
