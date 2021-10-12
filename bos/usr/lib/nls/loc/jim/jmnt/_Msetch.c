static char sccsid[] = "@(#)44	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Msetch.c, libKJI, bos411, 9428A410j 7/23/92 03:24:46";
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
 * MODULE NAME:         _Msetch
 *
 * DESCRIPTIVE NAME:    Set up convert position for display.
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
 * FUNCTION:            Set up convert position for display.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        516 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Msetch
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Msetch(pt,position,length)
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *                      position:Starting point of changed character.
 *                      length  :Length of changed character.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      IMSUCC  :Success of Execution.
 *
 * EXIT-ERROR:          Wait State Codes.
 *                      IMFAIL  :Fail of Execution because input parameter
 *                               error.
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
 *                              kjsvpt  :pointer to input character string.
 *                              chpos   :conversion character position.
 *                              chlen   :conversion character length.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              realcol :max use culomn.
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Control Block(KCB).
 *                              chpos   :convert character position.
 *                              chlen   :convert character length.
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
 *      Please Descripte This Module.
 */
int  _Msetch(pt,position,length)

KCB     *pt;            /* pointer to KCB.                              */
short   position;       /* starting position of changed character.      */
short   length;         /* length of changed character.                 */

{
        KMISA   *kjsvpt;        /* pointer to KMISA.                    */
        int      ret_code;      /* return value.                        */
        short    lastpos1;      /* convert ended point at this time.    */
        short    lastpos2;      /* convert ended point at last time.    */
        short    ws1;           /* work short variable.                 */

        CPRINT(#### _Msetch start ####);
/*##*/  snap3( SNAP_KCB | SNAP_KMISA, SNAP_Msetch , "strat _Msetch");

        ret_code = IMSUCC;      /* set return code.                     */
        kjsvpt = pt->kjsvpt;    /* work pointer set.                    */

        /* 0.
         *      process loop.
         */
        while( 1 )  {

            /* 1.
             *      check !  input parameter.
             */
            if(    ( position < 0 ) || ( kjsvpt->realcol < position )
                || ( length < 1 )
                || ( kjsvpt->realcol - position < length )     )
                {
                    /* 1. a
                     *      input parameter error.
                     */
                    ret_code = IMFAIL;
                    break;
                 }

            /* 2.
             *      check !  alread convert.
             */
            if( pt->chlen > 0 )
                {
                    /* 1. b
                     *      convert character is aleady exist.
                     */
                    lastpos1 = position + length;
                    lastpos2 = pt->chpos + pt->chlen;

                    /* 1. b2
                     *      set conversion start point
                     *                  to update character start point.
                     */
                    pt->chpos = (position < pt->chpos)?position:pt->chpos;

                    /* 1. b3
                     *      set conversion end point.
                     */
                    ws1 = (lastpos1 > lastpos2)?lastpos1:lastpos2;

                    /* 1. b4
                     *      set conversion length.
                     *                  to update character length.
                     */
                    pt->chlen = ws1 - pt->chpos;

                    break;
                }
            else
                {
                    /* 1. a
                     *      convert character is nothing.
                     *         update character start point.
                     *         update character start point.
                     */
                    pt->chpos = position;
                    pt->chlen = length;

                    break;
                }
        }

        /* 3.
         *      return.
         */
/*##*/  snap3( SNAP_KCB | SNAP_KMISA, SNAP_Msetch , "return _Msetch");
        return( ret_code );

}
