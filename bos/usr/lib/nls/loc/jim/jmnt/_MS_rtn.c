static char sccsid[] = "@(#)79	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_MS_rtn.c, libKJI, bos411, 9428A410j 7/23/92 03:20:22";
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
 * MODULE NAME:         _MS_rtn
 *
 * DESCRIPTIVE NAME:    Shift setting routine
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
 * FUNCTION:            Set shift status according to action code
 *                      in input parameters.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1184 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _MS_rtn
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _MS_rtn( pt, kmatcd )
 *
 *  INPUT:              pt      :Pointer to Kanji Controle Block.
 *                      kmatcd  :KMAT Code (Code 1 (Group 1))
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IMSUCC  :Successful.
 *                      IMKATAW :Warning for Shift1.
 *                               (Katakana Shift Mode Already Set)
 *                      IMALPHW :Warning for Shift1.
 *                               (Alphanumeric Shift Mode Already Set)
 *                      IMHIRAW :Warning for Shift1.
 *                               (Hiragana Shift Mode Already Set)
 *                      IMINSTW :Warning for Insert/Replace.
 *                               (Already Insert Mode)
 *
 * EXIT-ERROR:          NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _Mindset() : Indecator Set Routine.
 *                              _Mreset()  : Mode Reset Routine.
 *                              _MK_rtn()  : KKC Interface Routine.
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
 *                              kjsvpt  shift1  shift2  repins
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              msetflg kkcrmode        insert
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Control Block(KCB).
 *                              shift   shift1  repins
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              actc3
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
 *      This module sets shift status according to
 *      action code in input parameters.
 */
int  _MS_rtn( pt, kmatcd )

KCB     *pt;            /* Pointer to Kanji Control Block               */
uchar   kmatcd;         /* KMAT Action Code                             */
{
        register        KMISA   *kjsvpt;        /* Pointer to KMISA     */

        extern  int     _Mindset();     /* Indecator Set Routine.       */
        extern  int     _Mreset();      /* Mode Reset Routine.          */
        extern  int     _MK_rtn();      /* KKC Interface Routine.       */

        short   mode;           /* Prameter for _Mindset Call           */

        int     rc;             /* Return Code.                         */

snap3(SNAP_KCB | SNAP_KMISA,SNAP_MS_rtn,"start _MS_rtn");

        /*
         *      Pointer to Kanji Monitor Internal Save Area
         */

        kjsvpt = pt->kjsvpt;

        /*
         *      Set Successful Return Code
         */

        rc = IMSUCC;

        /* 1. 1
         *      Set Shift Status and Shift Indicator to Action Code
         */

        switch ( kmatcd )
               {

                /* 1. 1. 1
                 *      Set Katakana Shift
                 */

                case A_SFTKAT:
                        /*
                         *      Check Katakana Shift
                         */
                        if ( pt->shift1 == K_ST1KAT )
                           {
                                /*
                                 * Warning for Katakana
                                 *      Shift Mode Already Set
                                 */
                                rc = IMKATAW;
                                break;
                           };

                        if ( ( pt->shift1 != K_ST1HIR ) &&
                             ( pt->shift2 == K_ST2RON ) &&
                             ( pt->repins == K_REP    ) )

                                /* Change Left and Right Indicator */
                                mode = M_INDB;
                        else
                                /* Change Right Indicator */
                                mode = M_INDR;

                        /* Set Katakana Shift Mode */
                        pt->shift1 = K_ST1KAT;

                        /* Set Shift Change Flag */
                        pt->shift = pt->shift | K_STSFT1;

                        /*
                         *      Call ndicator Set Routine
                         */
                        rc = _Mindset( pt, mode );

                        break;

                /* 1. 1. 2
                 *      Set Alpha/Numeric Shift
                 */

                case A_SFTALP:
                        /*
                         *      Check Alpha Numeric Shift
                         */
                        if ( pt->shift1 == K_ST1AN )
                           {
                                /*
                                 * Warning for Alpha Numeric
                                 *      Shift Mode Already Set
                                 */
                                 rc = IMALPHW;
                                break;
                           };

                        if ( ( pt->shift1 != K_ST1UDF ) &&
                             ( pt->shift2 == K_ST2RON ) &&
                             ( pt->repins == K_REP    ) )
                                /* Change Left and Right Indicator */
                                mode = M_INDB;
                        else
                                /* Change Right Indicator */
                                mode = M_INDR;

                        /* Set Alpha Numeric Shift Mode */
                        pt->shift1 = K_ST1AN;

                        /* Set Shift Change Flag */
                        pt->shift = pt->shift | K_STSFT1;

                        /*
                         *      Call Indicator Set Routine
                         */
                        rc = _Mindset( pt, mode );

                        break;

                /* 1. 1. 3
                 *      Hiragana Shift Set
                 */

                case A_SFTHIR:
                        /*
                         *      Check Hiragana Shift
                         */
                        if ( pt->shift1 == K_ST1HIR )
                           {
                                /*
                                 * Warning for Hiragana
                                 *      Shift Mode Already Set
                                 */
                                rc = IMHIRAW;
                                break;
                           };

                        if ( ( pt->shift1 != K_ST1KAT ) &&
                             ( pt->shift2 == K_ST2RON ) &&
                             ( pt->repins == K_REP    ) )

                                /* Change Left and Right Indicator */
                                mode = M_INDB;
                        else
                                /* Change Right Indicator */
                                mode = M_INDR;

                        /* Set Hiragana Shift Mode */
                        pt->shift1 = K_ST1HIR;

                        /* Set Shift Change Flag */
                        pt->shift = pt->shift | K_STSFT1;

                        /*
                         *      Call Indicator Set Routine
                         */
                        rc = _Mindset( pt, mode );

                        break;

                /* 1. 1. 4
                 *      Set RKC Shift
                 */
                case A_SFTRKC:
                        /* Set Shift Changed Flag */
                        pt->shift = K_STSFT2;

                        /*
                         *      Check Romaji Kana Conversion Flag
                         */
                        if ( pt->shift2 == K_ST2RON )
                                /* Reset Romaj Kana Conv. Flag (OFF) */
                                pt->shift2 = K_ST2ROF;
                        else
                                /* Set Romaj Kana Conv. Flag (ON) */
                                pt->shift2 = K_ST2RON;

                        if ( ((pt->shift1 == K_ST1KAT) ||
                              (pt->shift1 == K_ST1HIR)) &&
                              (pt->repins == K_REP) )
                           {
                                /*
                                 *      Call Indicator Set Routine
                                 *              (Change Left Indicator)
                                 */
                                rc = _Mindset( pt, M_INDL );
                           };

                        break;

                /* 1. 1. 5
                 *      Reset Key
                 */

                case A_RESET:
                        /*
                         *      Reset Appropriate Mode Status
                         */
                        rc = _Mreset( pt, M_ALLRST );

                        /*
                         *      _Mrest Information Check
                         */
                        if ( rc != IMINSI )
                                break;

                        if ( (kjsvpt->msetflg == 0x00) ||
                             ((kjsvpt->kkcrmode == K_KADIC) &&
                              (kjsvpt->msetflg == K_MSGDIC)) )
                           {
                                /*
                                 *      Call KKC Interface Routine
                                 *              (Fix Conversion)
                                 */
                                rc = _MK_rtn( pt, A_CNVDEC );

                                /* Set First Kana Input Mode */
                                kjsvpt->actc3 = A_1STINP;
                           };

                        break;

                /* 1. 1. 6
                 *      Insert Key
                 */

                case A_INSERT:
                        /*
                         *      Check Insert Key (Profile)
                         */
                        if ( kjsvpt->kmpf[0].insert == K_INSERT )
                           {
                                /*
                                 *      Check Current Mode (KCB)
                                 */
                                if ( pt->repins == K_REP )
                                        /* Set Insert Mode (KCB) */
                                        pt->repins = K_INS;
                                else
                                        /* Set Replace Mode (KCB) */
                                        pt->repins = K_REP;
                           }
                        else
                           {
                                /*
                                 *      Check Current Mode (KCB)
                                 */
                                if ( pt->repins == K_REP )
                                        /* Set Insert Mode (KCB) */
                                        pt->repins = K_INS;
                                else
                                   {
                                        /* Warning (Already Insert Mode) */
                                        rc = IMINSTW;
                                        break;
                                   };
                           };

                        if ( (kjsvpt->msetflg == 0x00) ||
                             ((kjsvpt->kkcrmode == K_KADIC) &&
                              (kjsvpt->msetflg == K_MSGDIC)) )
                           {
                                /*
                                 *      Call KKC Interface Routine
                                 *              (Fix Conversion)
                                 */
                                rc = _MK_rtn( pt, A_CNVDEC );

                                /* Set First Kana Input Mode */
                                kjsvpt->actc3 = A_1STINP;
                           };

                        /*
                         *      Call Indicator Set Routine
                         *              (Change Left Indicator)
                         */
                        rc = _Mindset( pt, M_INDL );

                        break;
               };

        /* 1. 2
         *      Return Code.
         */

snap3(SNAP_KCB | SNAP_KMISA,SNAP_MS_rtn,"return _MS_rtn");

        return( rc );
}
