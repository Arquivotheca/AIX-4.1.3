static char sccsid[] = "@(#)58	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_ME_0e.c, libKJI, bos411, 9428A410j 7/23/92 03:19:01";
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
 * MODULE NAME:         _ME_0e
 *
 * DESCRIPTIVE NAME:    Back space key process when Kanji No. input.
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
 * FUNCTION:            Check display at input field or auxiliary field.
 *                      Check cursor position.
 *                      Delete a character before cursor.
 *                      Shift character string at right side of cursor
 *                                      to left.
 *                      Set value to KCB , KMISA.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        928 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _ME_0e
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _ME_0e(pt)
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      IMSUCC  :Success of Execution.
 *
 * EXIT-ERROR:          Wait State Codes.
 *                      other   :return code from _Msetch() , _Mmvch()
 *                               and _MM_rtn().
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _MM_rtn :Move conversion.
 *                              _Msetch :Set up convert position for display
 *                              _Mmvch  :Move character in a dimension.
 *                      Standard Liblary.
 *                              NA.
 *                      Advanced Display Graphics Support Liblary(GSL).
 *                              NA.
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      See Below.
 *
 *   INPUT:             DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              kjsvpt  :pointer to KMISA.
 *                              lastch  :input field last char position.
 *                              curcol  :input field cursor column.
 *                              cura1c  :position of aux area1.
 *                              ax1     :aux. area display position.
 *                              string  :pointer to character string.
 *                      Kanji Monitor Control Block(KCB).
 *                              knjnumfg:Kanji No. input area flag.
 *                              curleft :limit of cursor position on left.
 *                              curright:limit of cursor position on right.
 *                              chpsa1  :aux. area for conversion character
 *                                       display point.
 *                              ax1lastc:auxiliary area No1. last character
 *                                       position.
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
 *                              lastch  :input field last char position.
 *                              cura1c  :position of aux. area.
 *                              curcol  :input field cursor column.
 *                              chpsa1  :aux. area for conversion character
 *                                       display point.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              curright:limit of cursor position on right.
 *                              ax1lastc:auxiliary area No1. last character
 *                                       position.
 *                      Trace Block(TRB).
 *                              NA.
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Liblary.
 *                              IDENTIFY:Module Identify Create.
 *                      Standard Macro Liblary.
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
int  _ME_0e(pt)

KCB     *pt;            /* pointer to KCB.                              */

{
        int     _Mmvch();       /* Move conversion.                     */
        int     _MM_rtn();      /* Set up convert position for display. */
        int     _Msetch();      /* Move character in a dimension.       */

        int     ret_code;       /* Return Value.                        */
        KMISA   *kjsvpt;        /* pointer to KMISA.                    */

        /* parameter to kmmmvch().      */
        char    *str;           /* character string.                    */
        int     pos;            /* start position (top).                */
        int     len;            /* move length (byte).                  */
        int     dir;            /* move direction (forward or backward).*/
        int     dist;           /* move distance (byte).                */
        int     clear;          /* clear flag.                          */
        char    clrstr[2];      /* clear string.                        */
        int     clrpos;         /* clear data start position in clrstr. */
        int     clrlen;         /* clear data length.                   */

        /* parameter to _Msetch().      */
        short   position;       /* starting position of changed char.   */
        short   length;         /* length of changed character.         */


        CPRINT(#### _ME_0e ####);
/*##*/  snap3( SNAP_KCB | SNAP_KMISA, SNAP_ME_0e , "strat   _ME_0e");

        ret_code = IMSUCC;      /* return value initialize              */
        kjsvpt = pt->kjsvpt;

        /* 1.
         *      check !  aux. area or input area.
         */
        if( kjsvpt->knjnumfg == C_INPFLD )
            {
                /* 2.
                 *      input field displied.
                 */
                /* 2. 1
                 *      check !  now cursol position.
                 */
                 if(    (kjsvpt->curleft == pt->curcol)
                     || (kjsvpt->curleft == pt->lastch)    )
                    {
                        ret_code = _MM_rtn( pt , A_BEEP );
/*##*/  snap3( SNAP_KCB | SNAP_KMISA, SNAP_ME_0e , "return.2.1 _ME_0e");
                        return( ret_code );
                    }

                /* 2. 2
                 *      delete a character immediately left cursor
                 *                       and move character string immedi.
                 */
                /* set parameter for _Mmvch. (see declare)              */
                pos      = pt->curcol;
                len      = pt->lastch - pt->curcol;
                if ( len <= 0 ) {
                        len = C_DBCS;
                        DBCSTOCH(&pt->string[pt->lastch],C_SPACE);
                };
                dir      = M_FORWD;
                dist     = C_DBCS;
                clear    = TRUE;
                DBCSTOCH(clrstr,C_SPACE);
                clrpos   = 0;
                clrlen   = C_DBCS;
                ret_code = _Mmvch( pt->string , pos , len , dir , dist ,
                                   clear , clrstr , clrpos , clrlen );

                /* 2. 3
                 *      set KCB & KMISA.
                 */
                /* set parameter for _Msetch. (see declare)             */
                position         = pt->curcol - C_DBCS;
                length           = pt->lastch - pt->curcol + C_DBCS;
                /* set up conversion position.                          */
                ret_code         = _Msetch( pt , position , length );

                /* set input field last character position.             */
                pt->lastch       = pt->lastch - C_DBCS;

                /* set cursor move right corner.                        */
                kjsvpt->curright = pt->lastch ;

            }
        else
            {
                /* 3.
                 *      aux. field displied.
                 */
                /* 3. 1
                 *      check !  now cursol position.
                 */



/* #(B)  1987.11.30. Flying Conversion Change */
                 if(    (kjsvpt->curleft == pt->cura1c)
                     || (kjsvpt->curleft == kjsvpt->ax1lastc)    )
                    {
                        /* let beep.                                    */
                        ret_code = _MM_rtn( pt , A_BEEP );
/*##*/  snap3( SNAP_KCB | SNAP_KMISA, SNAP_ME_0e , "return.3.1 _ME_0e");
                        return( ret_code );
                    }
/* #(E)  1987.11.30. Flying Conversion Change */



                /* 3. 2
                 *      delete a character immediately left cursor
                 *                       and move character string immdei.
                 */
                /* parameter for _Mmvch set. (see declare)              */
                pos      = pt->cura1c;



/* #(B)  1987.11.30. Flying Conversion Change */
                len      = kjsvpt->ax1lastc - pt->cura1c;
/* #(E)  1987.11.30. Flying Conversion Change */



                if ( len <= 0 ) {
                        len = C_DBCS;



/* #(B)  1987.11.30. Flying Conversion Change */
                        DBCSTOCH(&pt->aux1[kjsvpt->ax1lastc],C_SPACE);
/* #(E)  1987.11.30. Flying Conversion Change */



                };
                dir      = M_FORWD;
                dist     = C_DBCS;
                clear    = TRUE;
                DBCSTOCH(clrstr,C_SPACE);
                clrpos   = 0;
                clrlen   = C_DBCS;

                ret_code = _Mmvch( pt->aux1 , pos , len , dir , dist ,
                                   clear , clrstr , clrpos , clrlen );

                /* 3. 3
                 *      set KCB & KMISA.
                 */
                /* set update character position of auxiliary area 1    */
                pt->chpsa1       = pt->cura1c - C_DBCS;



/* #(B)  1987.11.30. Flying Conversion Change */

                /* set update character length of auxiliary area 1      */
                pt->chlna1       = kjsvpt->ax1lastc - pt->cura1c + C_DBCS;

                /* set auxiliary area No.1 last character position.     */
                kjsvpt->ax1lastc  = kjsvpt->ax1lastc - C_DBCS;

                /* set cursor move right corner.                        */
                kjsvpt->curright = kjsvpt->ax1lastc ;

/* #(E)  1987.11.30. Flying Conversion Change */



            }

        /*
         *      Return.
         */
/*##*/  snap3( SNAP_KCB | SNAP_KMISA, SNAP_ME_0e , "return     _ME_0e");
        return( ret_code );
}
