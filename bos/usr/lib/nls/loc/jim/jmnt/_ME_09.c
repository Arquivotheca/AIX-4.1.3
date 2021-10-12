static char sccsid[] = "@(#)52	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_ME_09.c, libKJI, bos411, 9428A410j 7/23/92 03:18:42";
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
 * MODULE NAME:         _ME_09
 *
 * DESCRIPTIVE NAME:    Backspace process on editing mode.
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
 * FUNCTION:            Backspace process on editing mode.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        972 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _ME_09
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _ME_09(pt)
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IMSUCC  :Success of Execution.
 *
 * EXIT-ERROR:          other   :return code from _ME_03() , _MK_rtn()
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _MK_rtn : KKC interface routine.
 *                              _ME_03  : Backword key process.
 *                              _Mckbk  : query Goki block that contains
 *                                        the cursor.
 *                      Standard Library.
 *                              memset  : Memory area copy.
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
 *                              curcol  :input field cursor column.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              convpos :conversion position.
 *                              convlen :conversion length.
 *                              kjcvmap :Kanji and conversion map.
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
 *                              actc3   :KMAT Action Code 3.
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
#include <memory.h>     /* system momory access uty.                    */

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
 *      Backspace process on editing mode.
 */
int  _ME_09(pt)

KCB     *pt;            /* pointer to KCB.                              */

{
        int     _MK_rtn();      /* KKC interface routine.               */
        int     _ME_03();       /* Backspace key process.               */
        int     _Mckbk();       /* Set change object limits.            */

        int     ret_code;       /* Return Value.                        */
        int     fixflg;         /* fixation flag.                       */
        int     fixroc;         /* fixed conversion or fixed read flag. */
        int     wi1;            /* integer work variable.               */
        KMISA   *kjsvpt;        /* pointer to KMIsa.                    */
        short   start;          /* parameter to _Mckbk                  */
        short   end;            /* parameter to _Mckbk                  */
        short   nochng;         /* parameter to _Mckbk                  */


        CPRINT(#### _ME_09 start ####);
/*##*/  snap3( SNAP_KCB | SNAP_KMISA, SNAP_ME_09 , "start    _ME_09");

        ret_code = IMSUCC;      /* return value initialize              */
        kjsvpt = pt->kjsvpt;    /* work pointer set.                    */

        /* 1.
         *      check ! make sure uhat it is necessary to fix.
         *              case(fixflg==C_SWON) it is necessary to fix.
         *              case(fixflg!=C_SWON) it is no necessary  to fix.
         */
        fixflg = C_SWOFF;
        while(TRUE) {
                if( pt->curcol == kjsvpt->convpos )
                    {
                        fixflg = C_SWON;
                        break;
                    }
                wi1 = pt->curcol - kjsvpt->convpos - 1;
                if( kjsvpt->kjcvmap[wi1] != (char)M_KSNCNV )
                    {
                        fixflg = C_SWON;
                        break;
                    }
                break;
        }


        if(fixflg == C_SWON)
            {
                /* 1. a
                 *  it is necessary to fix.
                 */
                /* check !                                              */
                /*       fix of converted KANJI or YOMI.                */
                fixroc = C_SWOFF;

                for( wi1 = 1 ; wi1 < kjsvpt->convlen ; wi1 = wi1 + 2 )
                    {
                        if( kjsvpt->kjcvmap[wi1] != (char)M_KSNCNV )
                            {
                                /* converted KANJI.                     */
                                fixroc = C_SWON;
                                break;
                            }
                    }

                if( fixroc == C_SWOFF ) {
                    /* 1. a. (1)
                     *  fixation of Yomi process.
                     */
                    /* do post fixing process.                          */
                    ret_code = _MK_rtn( pt ,A_REDDEC );

                    /* backward process.                                */
                    ret_code = _ME_03( pt );
                    /* set KMAT ation code( first YOMI character input )*/
                    kjsvpt->actc3 = A_1STINP;

              } else {
                    /* 1. a. (2)
                     *  fixation converted KANJI.
                     */
                    ret_code = _MK_rtn( pt , A_CNVDEC );
                    /* 1. a. (2)
                     *  backward process.
                     */
                    ret_code = _ME_03( pt );

                    /* set KMAT ation code( first YOMI character input )*/
                    kjsvpt->actc3 = A_1STINP;

                }
            }
        else
            {
                /* 1. b1
                 *  it is no necessary to fix.
                 */
                /*
                 *  backward process.
                 */
                ret_code = _ME_03( pt );

                /* 1. b2
                 *
                 *    when there is an object of conversion and
                 *      isn't current object of conversion , make new.
                 *
                 */
                if(
                   ( (kjsvpt->convlen != 0) && (kjsvpt->cconvlen ==0) )
                 &&
                   (    (pt->curcol >= kjsvpt->convpos)
                     && (pt->curcol < kjsvpt->convpos + kjsvpt->convlen)   )
                )
                    {
                        /* get range of 'GOKI' whitch contains          */
                        /*                                the cursor.   */
                        ret_code = _Mckbk( pt , pt->curcol
                                              , &start , &end , &nochng );
                        /* check a nochange flag.                       */
                        if( nochng == C_SWOFF )
                            {
                                /* need revers process.                 */
                                /* set attribute code which means       */
                                /*             reverse and underline.   */
                                memset( (char *)(pt->hlatst + start) ,
                                                   (int)K_HLAT3 ,
                                                   (int)(end - start + 1) );

                                /* set a start position and length of   */
                                /*    current object of conversion area.*/
                                kjsvpt->cconvpos = start;
                                kjsvpt->cconvlen = end - start + 1;
                                ret_code = _Msetch( pt , start ,
                                                (short)(end - start + 1) );
                            }
                    }

            }

        /* 4.
         *      Return.
         */
/*##*/  snap3( SNAP_KCB | SNAP_KMISA, SNAP_ME_09 , "return   _ME_09");
        return( ret_code );
}
