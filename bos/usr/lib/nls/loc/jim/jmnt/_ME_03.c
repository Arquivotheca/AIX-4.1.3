static char sccsid[] = "@(#)47	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_ME_03.c, libKJI, bos411, 9428A410j 7/23/92 03:18:27";
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
 * MODULE NAME:         _ME_03
 *
 * DESCRIPTIVE NAME:    Backspace key process.
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
 * FUNCTION:            Select the real backspace process by rnvironment.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        848 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _ME_03
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _ME_03(pt)
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IMSUCC  :Success of Execution.
 *
 * EXIT-ERROR:          IMFAUL  :cursor is located at top of input field.
 *                               cursor is located after string.
 *                      other   :return code of _Mbsins() , _Mbsrepr() ,
 *                               _Mbsrepn().
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _MM_rtn : Mode switching routine.
 *                              _Mbsrepr: Backspace - restore -
 *                                                     in replace mode.
 *                              _Mbsrepn: Backspace - no restore -
 *                                                     in replace mode.
 *                              _Mbsins : Backspace in insert mode.
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
 *                              curcol  :cursor position.
 *                              lastch  :last character position.
 *                              repins  :replace/insert flag.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              curleft :limit of cursor position on left.
 *                              savelen :length of saved sring.
 *                              savepos :start position of saving.
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
 *      Select the real backspace process by rnvironment.
 */
int  _ME_03(pt)

KCB     *pt;            /* pointer to KCB.                              */

{
        int     _Mbsins();      /* Mode conversion.                     */
        int     _Mbsrepn();     /* Backspace - restore -                */
                                /*             in replace mode.         */
        int     _Mbsrepr();     /* Backspace - no restore -             */
                                /*             in replace mode.         */
        int     _MM_rtn();      /* Backspace in insert mode.            */

        uchar   inum[C_DBCS];   /*                                      */
        uchar   onum[C_DBCS];   /*                                      */
        KMISA   *kjsvpt;        /* pointer to KMISA.                    */
        int     ret_code;       /* return value.                        */
        uchar   *strptr;        /*                                      */
        int     brkflg;         /* brake flag                           */
        int     wi1;            /* work variavle integer.               */
        short   cvlen;          /* conversion length escape area.       */

        CPRINT(#### _ME_03 start ####);
/*##*/  snap3( SNAP_KCB | SNAP_KMISA , SNAP_ME_03 , "strat _ME_03");

        ret_code = IMSUCC;      /* initialize return code.              */
        kjsvpt = pt->kjsvpt;    /* set pointer to KMISA.                */

        /* 1.
         *      make sure that backspace ruotin can be executed.
         */
        /* 1. a
         *      cursor is located at top of input field.
         */
        if( pt->curcol == kjsvpt->curleft )
            {
                /* 1. a1
                 *      cursor positon is head of character stringth.
                 *      Beep then return.
                 */
                _MM_rtn( pt , A_BEEP );
/*##*/  snap3( SNAP_KCB | SNAP_KMISA, SNAP_ME_03 , "return.1.a1 _ME_03");
                return( IMFAIL );
             }
        /* 1. b
         *      cursor is located after string.
         */
        if( pt->curcol > pt->lastch )
            {
                /* 1. b1
                 *      return immendiately.
                 */
/*##*/  snap3( SNAP_KCB | SNAP_KMISA, SNAP_ME_03 , "return.1.b1 _ME_03");
                return( IMFAIL );
             }

        /* 2.
         *      escape conversion length.
         */
        cvlen = kjsvpt->convlen;

        /* 3.
         *      check !
         *      insert mode  or  input mode.
         */
        if ( pt->repins == K_INS )
            {
                /* 3. a1
                 *    backword process in insert mode.
                 */
                ret_code = _Mbsins( pt );
            }
        else
            {
                /* 3. b1
                 *    check !
                 *      Need to return it
                 *       that is keeped character when inputmode is replace.
                 *
                 *      1. saved character are more than Zero.
                 *         and a character which is an object of conversion.
                 *      2. last position of saved character equal
                 *                       last position of character which
                 *                       is an object of conversion.
                 */
                if(    ( kjsvpt->savelen != 0 )
                    && ( kjsvpt->convlen != 0 )
                    && ( (kjsvpt->savepos + kjsvpt->savelen) ==
                         (kjsvpt->convpos + kjsvpt->convlen)    )       ) {
                         /* 3. b1b
                         *      backword process in replace mode.
                         */
                        ret_code = _Mbsrepr( pt );
                    }
                else
                    {
                        /* 3. b1a
                         *       backword process in replace mode.
                         */
                        ret_code = _Mbsrepn( pt );
                    }
            }

        /* 4.
         *   if an object of conversion disappeared by backspace,
         *   call fix process.
         */
        if( (cvlen != 0) && (kjsvpt->convlen == 0) )  {
                ret_code = _MK_rtn(pt , A_REDDEC);
        }

        /* 5.
         *      return.
         */
/*##*/  snap3( SNAP_KCB | SNAP_KMISA, SNAP_ME_03 , "return _ME_03");
        return( ret_code );

}
