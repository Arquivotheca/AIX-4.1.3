static char sccsid[] = "@(#)40	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Jtrak.c, libKJI, bos411, 9428A410j 7/23/92 03:17:59";
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
 * MODULE NAME:         _Jtrak
 *
 * DESCRIPTIVE NAME:    Kanji status tracking.
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
 * FUNCTION:            1. Aralize the input data,
 *                      2. Set the shift state internally if it
 *                         is a shift key.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        732 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Jtrak
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Jtrak(pt)
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IMSUCC :Success of Execution.
 *
 * EXIT-ERROR:          KMIFACE  : Invalid DBCS input field length.
 *                      KMNPSCDE : Invalid pseudo code.
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
 *   INPUT:             DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Control Block(KCB).
 *                              *kjsvpt : pointer to KMISA.
 *                              type    : Input code type.
 *                              code    : Input code.
 *                              shift1  : AN/Kana shift.
 *                              shift2  : RKC shift.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              kmact   : Active/Inactive of input field.
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Control Block(KCB).
 *                              discrd  : discard flag.
 *                              shift   : change shift flag.
 *                              shift1  : AN/Kana shift.
 *                              shift2  : RKC shift.
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
 *   This module dose,
 *       1.Check input field is active.
 *       2.Check type and code set in KCB.
 *       3.Set shift flag.
 */

int _Jtrak(pt)
KCB     *pt ;         /*  pointer to Kanji Control Block        */
{
        KMISA   *kjsvpt;         /*   pointer to kanji moniter ISA      */
        int     rc    ;          /*   return code                       */
        uchar   grpcode;         /*   Group Code                        */
        uchar   actcode;         /*   Action Code                       */
        int     j;               /*   loop counter                      */

        /*  {code(KCB), Group Code, Action Code}  */
        static uchar kmat[20][3] = {
            {P_KATA,  A_GRPSFT, A_SFTKAT}, {P_ALPHA,  A_GRPSFT, A_SFTALP},
            {P_HIRA,  A_GRPSFT, A_SFTHIR}, {P_RKC,    A_GRPSFT, A_SFTRKC},
            {P_CONV,  A_GRPKKC, A_NOP   }, {P_NCONV,  A_GRPKKC, A_NOP   },
            {P_ACAND, A_GRPKKC, A_NOP   }, {P_REG,    A_GRPKKC, A_NOP   },
            {P_KANJI, A_GRPKKC, A_NOP   }, {P_CNVMSW, A_GRPKKC, A_NOP   },
            {P_DIAG,  A_GRPDIA, A_NOP   }, {P_PCAND,  A_GRPKKC, A_NOP   },
            {P_REVOD, A_NOP,    A_NOP   }, {P_REVOE,  A_NOP,    A_NOP   },
            {P_REVOF, A_NOP,    A_NOP   }, {P_REV10,  A_NOP,    A_NOP   },
            {P_REV11, A_NOP,    A_NOP   }, {P_PARPHY, A_GRPKKC, A_NOP   },
            {P_REV13, A_NOP,    A_NOP   }, {P_REV14,  A_NOP,    A_NOP   } };


        /* ### */
        snap3( SNAP_KCB | SNAP_KMISA , SNAP_Jtrak , "start" );

        kjsvpt = pt->kjsvpt;  /*  set pointer to kanji moniter ISA  */
        rc = IMSUCC;          /*  set return value                  */

        /* 1.
         *   prepare atin.
         */

        /* 1.1.
         *   check  !  field active flag in KMISA.
         */

                               /* If Active field.   */
        if ( kjsvpt->kmact == K_IFACT )  {

                rc = KMIFACE;  /* Set return code  */

                /* ### */
                snap3( SNAP_KCB | SNAP_KMISA , SNAP_Jtrak , "error");

                return( rc );
        };

        /* 1.2.
         *   check  !  Input code type in KCB.
         */

                              /* Not pseudo code.   */
        if ( pt->type != K_INESCF )  {

                pt->discrd = K_DISOFF;   /*  Reset discard flag in KCB  */

                rc = KMNPSCDE;           /*  Set return code    */

                /* ### */
                snap3( SNAP_KCB | SNAP_KMISA , SNAP_Jtrak , "error");

                return( rc );
        };

        /* 1.3.
         *   check  !  Input code in KCB.
         */

                            /* Not DBCS code.     */
        if ( pt->code > P_REV14 )  {

                pt->discrd = K_DISOFF;  /*  Reset discard flag in KCB  */

                /* ### */
                snap3( SNAP_KCB | SNAP_KMISA , SNAP_Jtrak , "error");

                return( rc );
                            /* DBCS code.     */
        } else {

                pt->discrd = K_DISON;   /*  Reset discard flag in KCB  */
        };

        /* 2.
         *   Set shift state.
         */

                     /* Reset shift flag.    */
        pt->shift = K_STNOT;

        /* 2.1.
         *   get  !  Action Code ,Group Code .
         */

        for ( j=0 ; j<20 ; j++ )  {

                             /*  Check input code in KCB  */
                if ( pt->code == kmat[j][0] )  {

                        grpcode = kmat[j][1];   /*  group code   */
                        actcode = kmat[j][2];   /*  action code  */
                };
        };

        /* 2.2.
         *   Set shift flags.
         */

                          /*  Check Group Code  */
        if ( grpcode != A_GRPSFT ) {

                /* ### */
                snap3( SNAP_KCB | SNAP_KMISA , SNAP_Jtrak , "error");

                return( rc );
        };
          /*  Set shift flags in accordance with Action Code  */
        switch( actcode )  {

                case A_SFTKAT :   /*  Case Katakana shift key  */

                                  /*  Set change shift flag in KCB  */
                        pt->shift  = pt->shift | K_STSFT1;

                                  /*  Set AN/Kana shift flag in KCB  */
                        pt->shift1 = K_ST1KAT;
                        break;

                case A_SFTALP :   /*  Case Alpha / Num shift key */

                                  /*  Set change shift flag in KCB  */
                        pt->shift  = pt->shift | K_STSFT1;

                                  /*  Set AN/Kana shift flag in KCB  */
                        pt->shift1 = K_ST1AN;
                        break;

                case A_SFTHIR :   /*  Case Hiragana shift key */

                                  /*  Set change shift flag in KCB  */
                        pt->shift  = pt->shift | K_STSFT1;

                                  /*  Set AN/Kana shift flag in KCB  */
                        pt->shift1 = K_ST1HIR;
                        break;

                case A_SFTRKC :   /*  Case Romaji-Kana Conversion shift  */

                                  /*  Set change shift flag in KCB  */
                        pt->shift  = K_STSFT2;

                                  /*  Check RKC shift flag in KCB  */
                        if ( pt->shift2 == K_ST2RON )

                                  /*  Set RKC shift flag in KCB  */
                                pt->shift2 = K_ST2ROF;
                        else
                                  /*  Set RKC shift flag in KCB  */
                                pt->shift2 = K_ST2RON;

                        break;

                default :         /*  Default case       */
                        break;
        };

        /* 3.
         *   Return Value.
         */

        /* ### */
        snap3( SNAP_KCB | SNAP_KMISA , SNAP_Jtrak , "end");

        return( rc );
}
