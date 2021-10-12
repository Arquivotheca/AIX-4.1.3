static char sccsid[] = "@(#)37	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mregrs.c, libKJI, bos411, 9428A410j 7/23/92 03:24:20";
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
 * MODULE NAME:         _Mregrs
 *
 * DESCRIPTIVE NAME:    Reset dictionary registration.
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
 * FUNCTION:            Reset dictionary registration and return to
 *                      first Yomi input mode.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        596 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mregrs
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mregrs(pt)
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IMSUCC :Success of Execution.
 *
 * EXIT-ERROR:          NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _Mfmrst() : restore saved field
 *                              _MM_rtn() : mode switching routine
 *                              _MK_rtn() : Kana Kanji conversion interface
 *                                          routine
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
 *                              *kjsvpt  : pointer to KMISA
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              nextact  : next function
 *                              kkcrmode : dictionary registration mode
 *                              msetflg  : message set flag
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
 *                              kkcrmode : dictionary registration mode
 *                              nextact  : next function
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
 *  This module does,
 *      1.Reset dictionary registration flag OFF.
 *      2.Reset message in input field.
 *      3.Return to first yomi input mode.
 */

int _Mregrs(pt)
KCB     *pt ;         /*  pointer to Kanji Control Block        */
{
        KMISA   *kjsvpt;         /*   pointer to kanji moniter ISA      */
        extern  int     _Mfmrst(); /* restore saved field               */
        extern  int     _MM_rtn(); /* mode switching routine            */
        extern  int     _MK_rtn(); /* Kana Kanji conversion interface
                                      routine                           */

/* ### */
        CPRINT(======== start _Mregrs ===========);
snap3( SNAP_KCB | SNAP_KMISA , SNAP_Mregrs , "start _Mregrs" );

        kjsvpt = pt->kjsvpt;  /*  set pointer to kanji moniter ISA  */

        /* 1.
         *   reset  !  dictionary registration mode flag.
         */
        kjsvpt->kkcrmode = K_NODIC;

        /* 2.
         *   reset  !  next function.
         */
        kjsvpt->nextact = kjsvpt->nextact & ~(M_RGAON | M_RGBON | M_RGCON);

        /* 3.
         *      Conversion fixing.
         *
         */
                        /*  Check conversion length  */
        if ( kjsvpt->convlen > 0 ) {

                _MK_rtn( pt, A_CNVDEC ); /* KKC Interface Routine to fix */
        };

        /* 4.
         *   reset  !  meseage in input field.
         */
        _Mfmrst( pt, K_MSGDIC );

        /* 5.
         *   return to first yomi input mode.
         */
        _MM_rtn( pt, A_1STINP );

        /* 6.
         *  Return.
         */

/* ### */
snap3( SNAP_KCB | SNAP_KMISA , SNAP_Mregrs , "end ---No.1 ---- _Mregrs " );

         return( IMSUCC );
}
