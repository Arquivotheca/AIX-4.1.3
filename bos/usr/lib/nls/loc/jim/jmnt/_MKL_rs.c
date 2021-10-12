static char sccsid[] = "@(#)60	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_MKL_rs.c, libKJI, bos411, 9428A410j 7/23/92 03:19:08";
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
 * MODULE NAME:         _MKL_rs
 *
 * DESCRIPTIVE NAME:    Keyboard Lock Reset Routine.
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
 * FUNCTION:            Reset keyboard if the reset key is depressed.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        532 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _MKL_rs
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _MKL_rs( pt )
 *
 *  INPUT:              pt      :Pointer to Kanji Controle Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IMSUCC  :Successful.
 *                      IMKLRSTI:Keybord Reset Information.
 *
 * EXIT-ERROR:          NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _Mreset() : Mode Reset Routine
 *                              _MM_rtn() : Kanji Monitor Mode Switching
 *                                          Routine
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
 *                              kjsvpt  code
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
 *      This module reset keyboard if reset key is depressed.
 */
int  _MKL_rs( pt )

KCB     *pt;            /* Pointer to Kanji Control Block               */
{
        register KMISA  *kjsvpt;
                /* Pointer to Kanji Monitor Internal Save Area.    */

        extern  int     _Mreset();      /* Mode Reset Routine           */
        extern  int     _MM_rtn();      /* Kanji Moniter Mode Switching */
                                        /* Routine                      */

        int     rc;             /* Return Code.                         */

snap3(SNAP_KCB | SNAP_KMISA,SNAP_MKL_rs,"start _MKL_rs");

        /*
         *      Pointer to Kanji Monitor Internal Save Area
         */

        kjsvpt = pt->kjsvpt;

        /* 1. 1
         *      Keybord Reset Process
         */

        switch ( pt->code )
               {
                /*
                 *      Pseudo Code is Katakana/Alphar Numeric
                 *              or Hiragana Shift Key.
                 */
                case P_KATA  :
                case P_ALPHA :
                case P_HIRA  :
                        rc = IMSUCC;            /* Set Successful Return */
                        break;

                /*
                 *      Pseudo Code is Reset Key
                 */
                case P_RESET :
                        /* Call Mode Reset Routine */
                        rc = _Mreset( pt, M_KLRST );

                        rc = IMKLRSTI;          /* Set Reset Information */
                        break;

                /*
                 *      Other key
                 */
                default :
                        /* Call Mode Switching Routine to Beep */
                        rc = _MM_rtn( pt, A_BEEP );

                        rc = IMKLRSTI;          /* Set Information      */
                        break;
        };

        /* 1. 2
         *      Return Code.
         */

snap3(SNAP_KCB | SNAP_KMISA,SNAP_MKL_rs,"return _MKL_rs");

        return( rc );
}
