static char sccsid[] = "@(#)45	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_ME_01.c, libKJI, bos411, 9428A410j 7/23/92 03:18:21";
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
 * MODULE NAME:         _ME_01
 *
 * DESCRIPTIVE NAME:    process of first'YOMI' character input.
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
 * FUNCTION:            Judge a character to be an object of conversion by
 *                          caharacter's mode and profile.
 *                      Add a character (Insert/Replace)
 *
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        800 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _ME_01
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _ME_01(pt)
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IMSUCC  :return code from _Mecho():Success of adding
 *                                                         character
 *
 *
 * EXIT-ERROR:          IMFAIL  :return code from _Mecho():Failure of adding
 *                                                         character
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _Mecho  :Character Input routine.
 *                              _Mgetchm:Get character mode.
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
 *                              conv    :conversion field flag.
 *                              chcode  :character code.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              kkmode1 :convert conbertion mode.
 *                              kmpf    :Kanji Monitor Profile save area.
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Control Block(KCB).
 *                              cnvsts  :convert skip flag.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              chmod   :character mode.
 *                              savelen :saved character length.
 *                              convpos :start position of an object of
 *                                                         conversion.
 *                              convlen :length of an object of conversion.
 *                              cconvpos:start position of current object
 *                                                      of conversion.
 *                              cconvlen:length of current object of
 *                                                   conversion
 *                              fconvpos:Start Position of Flying Conversion.
 *                              act3    :KMAT action code 3.
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

/*      Judge a character to be an object of conversion by
 *          caharacter's mode and profile.
 *      Add a character (Insert/Replace)
 */
int  _ME_01(pt)

KCB     *pt;            /* pointer to KCB.                              */

{
        int     _Mecho();       /* Character Input routine.             */
        uchar   _Mgetchm();     /* Kanji monitor get character mode.    */

        int     ret_code;       /* Return Value.                        */
        int     convflg;        /* conversion flag.                     */
        uchar   mode;           /* return code from _Mgetchm.           */
        KMISA   *kjsvpt;        /* pointer to KMISA.                    */

        ret_code = IMSUCC;      /* return value initialize              */
        kjsvpt = pt->kjsvpt;    /* set pointer to KMISA.                */

        CPRINT(#### _ME_01 start ####);
/*##*/  snap3( SNAP_KCB | SNAP_KMISA, SNAP_ME_01 , "start _ME_01");

        /* 1.
         *      Judge. a character to be an oblect of conversion.
         */
        /* initialize converton flag(OFF).                              */
        convflg = FALSE;

        /* check the prohibition flag of conversion.                    */
        if( (pt->conv != K_KKCOFF) && (kjsvpt->kkmode1 != A_DCREGA) )
            {
                /* get caharacter mode.                                 */
                mode = _Mgetchm( kjsvpt->chcode , (ushort)0 , kjsvpt );

/*####*/ PRINT( mode , d );
                /*
                 *      Judge. a character to be an oblect of conversion.
                 *          by character mode of Input.
                 */
                switch(mode) {
                    case( K_CHHIRA ) :
                        if (CHPTTOSH(kjsvpt->chcode) != 0x815B)
                            convflg = TRUE ;
                        break ;
                    case( K_CHKATA ) :
                        if( kjsvpt->kmpf[0].katakana == K_KANAON )
                                convflg = TRUE;
                        break;
                    case( K_CHALPH ) :
                        if( kjsvpt->kmpf[0].alphanum == K_ALPON )
                                convflg = TRUE;
                        break;
                    case( K_CHNUM  ) :
                        if( kjsvpt->kmpf[0].alphanum == K_ALPON )
                            convflg = TRUE;
                        break;
                    default :
                        break;
                }
            }
        /* 2.
         *      branch by judeg.
         */
        if( convflg )
            {
                /* In case of an object of conversion                   */
                kjsvpt->chmode   = mode;

                /* initialize variable for conversion.                  */
                /*     set cnvert status of KCB.                        */
                pt->cnvsts       = K_CONVGO;

                /*     set save point of input field of yomi.           */
                /*     set conversion position of KMISA.                */
                /*     set current conversion position of KMISA.        */
                /*     set flying conversion position of KMISA.         */


/* #(B) 1987.12.04. Flying Conversion Change */
                kjsvpt->savepos  =
                kjsvpt->convpos  =
                kjsvpt->cconvpos =
                kjsvpt->fconvpos = pt->curcol;
/* #(E) 1987.12.04. Flying Conversion Change */



                /*     set string save length of KMISA.                 */
                /*     set convertin length of KMISA.                   */
                /*     set current conversion length of KMISA.          */
                kjsvpt->savelen  =
                kjsvpt->convlen  =
                kjsvpt->cconvlen = 0;

                /* set KMAT action code (continueing input mode).       */
                kjsvpt->actc3    = A_CONINP;

                /* add a character.                                     */
                ret_code = _Mecho( pt , K_HLAT3 , M_NORMAL );
            }
        else
            {
                /* In case of fixed character.                          */
                if(kjsvpt->kkmode1 != A_DCREGA)
                    {
                        kjsvpt->actc3    = A_1STINP;
                    }

                /* add a character.                                     */
                ret_code = _Mecho( pt , K_HLAT0 , M_NORMAL );
            }

        /* 3.
         *      Return.
         */
/*##*/  snap3( SNAP_KCB | SNAP_KMISA, SNAP_ME_01 , "return _ME_01");
        return( ret_code );
}
