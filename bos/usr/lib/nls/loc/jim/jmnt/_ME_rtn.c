static char sccsid[] = "@(#)59	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_ME_rtn.c, libKJI, bos411, 9428A410j 7/23/92 03:19:05";
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
 * MODULE NAME:         _ME_rtn
 *
 * DESCRIPTIVE NAME:    Character Edit Processing Main Routine.
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
 * FUNCTION:            Branch each edit function by Action-code, for example,
 *                      character add, delete, backspace and so on.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1328 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _ME_rtn
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _ME_rtn( pt, action )
 *
 *  INPUT:              pt      : Pointer to Kanji Control Block. (KCB)
 *                      action  : Action Code.
 *
 *  OUTPUT:             None
 *
 * EXIT-NORMAL:         IMSUCC : Success of Execution.
 *
 * EXIT-ERROR:          IMFAIL : Invalid Input or Failure of Execution.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _ME_01()        First Character Input.
 *                              _ME_02()        Continuous Character Input.
 *                              _ME_03()        Back Space.
 *                              _ME_04()        Erase EOF.
 *                              _ME_05()        Erase Input.
 *                              _ME_06()        Delete.
 *                              _ME_07()        Editorial Character Input.
 *                              _ME_09()        Editorial Mode Back Space.
 *                              _ME_0a()        Selection Number Input in
 *                                               All Candidate mode.
 *                              _ME_0b()        Knaji Number Input.
 *                              _ME_0c()        Delete in Kanji Number
 *                                               Input mode.
 *                              _ME_0d()        Conversion mode switch number
 *                                              input.
 *                              _ME_0e()        Back Space in Kanji Number
 *                                               Input mode.
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
 *      Control routine for all editorial subroutine.
 */

int _ME_rtn( pt, action )
KCB     *pt;            /*  Pointer to Kanji Control Block              */
uchar   action;         /*  Action code from KMAT                       */
{
        int     rc;             /* Return Code.                         */
        int     _ME_01();       /* see EXTERNAL REFERENCES              */
        int     _ME_02();       /* see EXTERNAL REFERENCES              */
        int     _ME_03();       /* see EXTERNAL REFERENCES              */
        int     _ME_04();       /* see EXTERNAL REFERENCES              */
        int     _ME_05();       /* see EXTERNAL REFERENCES              */
        int     _ME_06();       /* see EXTERNAL REFERENCES              */
        int     _ME_07();       /* see EXTERNAL REFERENCES              */
        int     _ME_09();       /* see EXTERNAL REFERENCES              */
        int     _ME_0a();       /* see EXTERNAL REFERENCES              */
        int     _ME_0b();       /* see EXTERNAL REFERENCES              */
        int     _ME_0c();       /* see EXTERNAL REFERENCES              */
        int     _ME_0d();       /* see EXTERNAL REFERENCES              */
        int     _ME_0e();       /* see EXTERNAL REFERENCES              */

/************************************************************************/
snap3(SNAP_KCB | SNAP_KMISA, SNAP_ME_rtn, "Start _ME_rtn");
/************************************************************************/

        /* 1.
         *      Branch by Lower 4bit code of Action code
         */
        switch(action & C_LHBYTE) {
            case A_NOP:
                rc = IMSUCC;     /* Take no action and return imidiately. */
                break;
            case A_1STCHR:
                rc = _ME_01(pt); /* Call first character input routine. */
                break;
            case A_CONCHR:
                rc = _ME_02(pt); /* Call continuous character input routine */
                break;
            case A_BCURDL:
                rc = _ME_03(pt); /* Call Back Space routine. */
                break;
            case A_RCURDL:
                rc = _ME_04(pt); /* Call Erase EOF routine. */
                break;
            case A_IFDEL:
                rc = _ME_05(pt); /* Call Erase Input routine. */
                break;
            case A_CUPDEL:
                rc = _ME_06(pt); /* Call Delete routine. */
                break;
            case A_CHREDT:
                rc = _ME_07(pt); /* Call Editorial Character Input routine. */
                break;
            case A_BCKSPC:
                rc = _ME_09(pt); /* Call Editorial Mode Back Space routine. */
                break;
            case A_ALCINP:
                rc = _ME_0a(pt); /* Call Selection Number Input routine in
                                     All Candidate mode. */
                break;
            case A_KJNINP:
                rc = _ME_0b(pt); /* Call Knaji Number Input routine. */
                break;
            case A_KJNDEL:
                rc = _ME_0c(pt); /* Call Delete routine in Kanji Number
                                                                Input mode. */
                break;
            case A_CMSINP:
                rc = _ME_0d(pt); /* Call conversion mode switch number
                                                             input routine. */
                break;
            case A_KJNBSP:
                rc = _ME_0e(pt); /* Call Back Space routine in Kanji Number
                                                                Input mode. */
                break;
            default:
                rc = IMFAIL;  /* Invalid action code input. */
        }
/************************************************************************/
snap3(SNAP_KCB | SNAP_KMISA, SNAP_ME_rtn, "Normal return _ME_rtn");
/************************************************************************/
        return( rc ); /* Return to caller */
}
