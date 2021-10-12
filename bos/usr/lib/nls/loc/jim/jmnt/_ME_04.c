static char sccsid[] = "@(#)48	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_ME_04.c, libKJI, bos411, 9428A410j 7/23/92 03:18:30";
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
 * MODULE NAME:         _ME_04
 *
 * DESCRIPTIVE NAME:    Erase EOF process.
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
 * FUNCTION:            Erase character behind cursor.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        512 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _ME_04
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _ME_04(pt)
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IMSUCC  :Success of Execution.
 *
 * EXIT-ERROR:          other   :return code from _Msetch().
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _Msetch :set up convert position for display
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
 *                              curcol  :input field cursor colmn.
 *                              lastch  :input field last character
 *                              string  :pointer to input character string.
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
 *                              lastch  :input field last character
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
 *      Please Descripte This Module.
 */
int  _ME_04(pt)

register KCB     *pt;    /* pointer to KCB.                             */

{
        int     _Msetch();      /* set up convert position for display. */

        register int     wi1;   /* work integer value.                  */

/*##*/  snap3( SNAP_KCB | SNAP_KMISA, SNAP_ME_04 , "strat _ME_04");


        /* 1.
         *      check !  cursor position.
         */
        if( pt->lastch <= pt->curcol )
            {
/*##*/  snap3( SNAP_KCB | SNAP_KMISA, SNAP_ME_04 , "return.1 _ME_04");
                /* cursor position is located after string.             */
                return( IMFAIL );
            }
        else
            {
                /* 1. b1
                 *      set blank from current cursor position
                 *                to last position of character string.
                 */
                for( wi1 = pt->curcol ; wi1 < pt->lastch ; wi1 += C_DBCS)
                    {
                        pt->string[wi1    ] = C_SPACEH;
                        pt->string[wi1 + 1] = C_SPACEL;
                    }

                /* 1. b2
                 *      set start position change and changelength
                 *          to KCB.
                 */
                _Msetch( pt, pt->curcol, pt->lastch - pt->curcol );

                /* 1. b3
                 *      set string length.
                 */
                pt->lastch = pt->curcol;
            }

        /* 1. b4
         *      return.
         */
/*##*/  snap3( SNAP_KCB | SNAP_KMISA, SNAP_ME_04 , "return _ME_04");
        return( IMSUCC );

}
