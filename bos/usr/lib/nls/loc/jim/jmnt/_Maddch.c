static char sccsid[] = "@(#)82  1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Maddch.c, libKJI, bos411, 9428A410j 7/23/92 03:20:35";
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
 * MODULE NAME:         _Maddch
 *
 * DESCRIPTIVE NAME:    Add character for display.
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
 * FUNCTION:            Add character behind already exist character string.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        608 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Maddch
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Maddch(pt,attrib)
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *                      attrib  :Highlight attribute code.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      IMSUCC  :Success of Execution.
 *
 * EXIT-ERROR:          Wait State Codes.
 *                      IMFAIL  :adding character is impossible
 *                               or  return code from _Msetch().
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _Msetch :Set up covert position for display.
 *                      Standard Library.
 *                              memcpy  :Memory area copy.
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
 *                              kjsvpt  :pointer to KMISA.
 *                              curcol  :cursor position.
 *                              cnvsts  :convert mode.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              curright:limit of cursor position on right.
 *                              chcode  :character code.
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Control Block(KCB).
 *                              string  :pointer to input character string.
 *                              hlatst  :pointer to input character string's
 *                                       attribute.
 *                              lastch  :character string last position.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              convlen :conversion length.
 *                              cconvlen:current conversion length.
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
#include <memory.h>     /* System memory operation uty.                 */

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
int  _Maddch(pt,attrib)

KCB     *pt;            /* pointer to KCB.                              */
char    attrib;         /* highlight attribute code.                    */

{

        int     _Msetch();      /* set up convert position for display. */

        KMISA   *kjsvpt;        /* pointer to KMISA.                    */
        int      ret_code;      /* return value.                        */
        int      wi1;           /* work integer value.                  */

        CPRINT(#### _Maddch start ####);
/*##*/  snap3( SNAP_KCB | SNAP_KMISA, SNAP_Maddch , "strat _Maddch");

        ret_code = IMSUCC;      /* initialize return code.              */
        kjsvpt = pt->kjsvpt;    /* set pointer to KMISA.                */


        /* 0.
         *      process loop.
         */
        while( 1 )  {

            /* 1.
             *      be added character at this position ?
             */
            if( kjsvpt->curright <= pt->curcol )
                {
                    ret_code = IMFAIL;
                    break;
                }

            /* 2.
             *      chcode => string.
             */
            memcpy( &(pt->string[pt->curcol]) , kjsvpt->chcode , C_DBCS );

            /* 3.
             *      change 'convert character start point'
             *      change 'convert character length'
             */
            ret_code = _Msetch( pt , pt->curcol , C_DBCS );
            if( ret_code != IMSUCC )  break;

            /* 4.
             *      attrib(parameter) =>
             *                       hlatst(attribute of input character)
             */
            for(wi1 = 0 ; wi1 < C_DBCS ; wi1 ++ )
                {
                    pt->hlatst[pt->curcol + wi1] = attrib;
                }

            /* 5.
             *      last character position update.
             */
            pt->lastch = pt->curcol + C_DBCS;

            /* 6.
             *      update conversion length and current conversion length
             *                           if modo indicated conversion.
             */
            if( pt->cnvsts == K_CONVGO )
                {
                    kjsvpt->convlen  += C_DBCS;
                    kjsvpt->cconvlen += C_DBCS;
                }

            break;
        }


        /* 7.
         *      return.
         */
/*##*/  snap3( SNAP_KCB | SNAP_KMISA, SNAP_Maddch , "return _Maddch");
        return( ret_code );

}
