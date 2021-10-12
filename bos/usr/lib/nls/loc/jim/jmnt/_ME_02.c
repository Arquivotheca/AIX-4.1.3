static char sccsid[] = "@(#)46	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_ME_02.c, libKJI, bos411, 9428A410j 7/23/92 03:18:24";
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
 * MODULE NAME:         _ME_02
 *
 * DESCRIPTIVE NAME:    continuous character input of object of conversion.
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
 * FUNCTION:            Judge an object of conversion to be fixed
 *                        by character's mode.
 *                       case 'continue' :add a character.
 *                       case 'fix'      :fix an object of aonversion
 *                                        then do a first 'YOMI'
 *                                        character input.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        912 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _ME_02
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _ME_02(pt)
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IMSUCC  :return code from _Mecho(),_MK_rtn() and
 *                               _ME_rtn.
 *
 * EXIT-ERROR:          IMFAIL :return code from _Mecho(),_MK_rtn() and
 *                               _ME_rtn.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _ME_rtn : Character edit processing
 *                                                            main routine.
 *                              _MK_rtn : KKC interface routine.
 *                              _Mecho  : Character Input routine.
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
 *                              kjsvpt  :pointer to KMISA.
 *                              chcode  :pointe to Inputed character buffer.
 *                              chmode  :character mode.
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
 *      Judge an object of conversion to be fixed
 *       by character's mode.
 *      case 'continue' :add a character.
 *      case 'fix'      :fix an object of aonversion
 *                       then do a first 'YOMI'
 *                       character input.
 */
int  _ME_02(pt)

KCB     *pt;            /* pointer to KCB.                              */

{
        int     _MK_rtn();      /* KKC interface routine.               */
        int     _ME_rtn();      /* Character edit processing main.      */
        int     _Mecho();       /* Character input routine.             */

        int     ret_code;       /* Return Value.                        */
        int     wi1;            /* work integer value.                  */
        int     wi2;            /* work integer value.                  */
        KMISA   *kjsvpt;        /* pointer to KMISA.                    */

                                /* fixed dimension.                     */
        static int     fixdim[7][7] = {
                                         { 0 , 0 , 0 , 0 , 0 , 0 , 0 } ,
                                         { 0 , 0 , 0 , 0 , 0 , 0 , 0 } ,
                                         { 1 , 1 , 0 , 0 , 0 , 1 , 1 } ,
                                         { 1 , 1 , 0 , 0 , 0 , 1 , 1 } ,
                                         { 1 , 1 , 1 , 1 , 1 , 1 , 1 } ,
                                         { 1 , 1 , 1 , 1 , 1 , 1 , 1 } ,
                                         { 1 , 1 , 1 , 1 , 1 , 1 , 1 }
                                       };

        CPRINT(#### _ME_02 ####);
/*##*/  snap3( SNAP_KCB | SNAP_KMISA, SNAP_ME_02 , "return _ME_02");

        /* set pointer to KMISA.                                        */
        kjsvpt = pt->kjsvpt;

        /* 1.
         *      Judge an object of conversion to be fixed.
         */
        /* get caharacter mode.                                         */
        wi1 = _Mgetchm( kjsvpt->chcode , 0 , kjsvpt );

        if( fixdim[kjsvpt->chmode - 1][wi1 - 1] == 0)

                /* 1. a
                 *      not fix ( continuous input ).
                 */
                /* continue character input.                            */
                ret_code = _Mecho( pt , K_HLAT3 , M_NORMAL );
        else
            {
                /* 1. b
                 *      fix ( fix and first 'YOMI' input ).
                 */
                /* do post fixing process.                              */
                _MK_rtn( pt , A_REDDEC );

                /* first object character input status.                 */
                ret_code = _ME_rtn( pt , A_1STCHR );
            }

        /* 3.
         *      Return.
         */
/*##*/  snap3( SNAP_KCB | SNAP_KMISA, SNAP_ME_02 , "return _ME_02");
        return( ret_code );
}
