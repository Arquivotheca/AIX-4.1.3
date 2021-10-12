static char sccsid[] = "@(#)50	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_ME_06.c, libKJI, bos411, 9428A410j 7/23/92 03:18:36";
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
 * MODULE NAME:         _ME_06
 *
 * DESCRIPTIVE NAME:    Delete process.
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
 * FUNCTION:            Select the real delete process by environment.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        740 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _ME_06
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _ME_0b(pt)
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IMSUCC  :Success of Execution.
 *
 * EXIT-ERROR:          Wait State Codes.
 *                      IMFAIL  :cursor position error.
 *                      other   :return code from _Mlock() , _Mdelins()
 *                               and _Mdelrep().
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _Mlock  : Management key board rock.
 *                              _Mdelrep: Delete in reolace mode.
 *                              _Mdelins: Delete in insert  mode.
 *                      Standard Library.
 *                              memcpy  : Memory area copy.
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
 *                              kjsvpt  :pointer to Kanji monitor save area.
 *                              curcol  :input field cursor colmn.
 *                              lastch  :input field last character
 *                              repins  :replace/insert mode flag.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              curright:limit of cursor position on right.
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
#include <memory.h>     /* Performs memory operation.                   */

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
 *      Select the real delete process by environment.
 */
int  _ME_06(pt)

KCB     *pt;            /* pointer to KCB.                              */

{
        int     _Mdelrep();     /* Delete in replace mode.              */
        int     _Mdelins();     /* Delete in insert  mode.              */
        int     _Mlock();       /* Management key board rock.           */

        int     ret_code;       /* Return Value.                        */
        int     rc;             /* Return Value.                        */
        short   cvlen;          /* conversion length.                   */
        KMISA   *kjsvpt;        /* pointer to KMIsa.                    */

        CPRINT(#### _ME_06 start ####);
/*##*/  snap3( SNAP_KCB | SNAP_KMISA, SNAP_ME_06 , "start    _ME_06");

        ret_code = IMSUCC;      /* return value initialize              */
        kjsvpt = pt->kjsvpt;    /* set pointer to KMISA.                */

        /* 0.
         *      process loop.
         */
        while( 1 ) {

            /* 1.
             *      cursor can move to this position, but can not
             *      delete here.
             */
            if( pt->curcol == kjsvpt->curright )
                {
                    /*
                     *  cursor position error.
                     *     Call keyboard lock routine and return.
                     */
                    ret_code = _Mlock( pt );
                    ret_code = IMFAIL;
                    break;
                }

            /* 2.
             *      when cursor is kocated afteer end of string,
             *      there are no character to delete.
             */
            if( pt->lastch <= pt->curcol )
                {
                    /*
                     *  there no characters can be deleted.
                     */
                    ret_code = IMFAIL;
                    break;
                }

            /* 3.
             *      save conversion length.
             */
            cvlen = kjsvpt->convlen;

            /* 4.
             *      branch by mode that insert or replace.
             */
            if( pt->repins == K_INS )
                /*
                 *  Input_mode is insert.
                 */
                ret_code = _Mdelins( pt );
              else
                /*
                 *  Input_mode is replace.
                 */
                ret_code = _Mdelrep( pt );

            /* 5.
             *      if an object of conversion disappeared
             *                   by delete process, call fixprocess.
             */
            if( (cvlen != 0) && (kjsvpt->convlen == 0) )
                rc = _MK_rtn(pt , A_REDDEC);

            /*
             *      End of process.
             */
            break;
        }

        /*
         *      Return.
         */
/*##*/  snap3( SNAP_KCB | SNAP_KMISA, SNAP_ME_06 , "return   _ME_06");
        return( ret_code );
}
