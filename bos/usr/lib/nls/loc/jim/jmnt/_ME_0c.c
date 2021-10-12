static char sccsid[] = "@(#)56	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_ME_0c.c, libKJI, bos411, 9428A410j 7/23/92 03:18:53";
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
 * MODULE NAME:         _ME_0c
 *
 * DESCRIPTIVE NAME:    Delete process when Kanji No. input.
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
 *                      Delete a character at cursor.
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
 *  MODULE SIZE:        932 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _ME_0c
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _ME_0c(pt)
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      IMSUCC  :Success of Execution.
 *
 * EXIT-ERROR:          Wait State Codes.
 *                      other   :return code of _Mmvch() , _Msetch()
 *                               and _MM_rtn().
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _Mmvch  : Move character in a dimension.
 *                              _Msetch : Set up convert position
 *                                                      for dsiplay.
 *                              _MM_rtn : Move conversion.
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
 *                              lastch  :input field last char position.
 *                              curcol  :input field cursor column.
 *                              string  :pointer to input character string.
 *                              ax1     :aux. area display position.
 *                              cura1c  :column No. of aux. area 1.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              knjnumfg:Kanji No. input area flag.
 *                              curright:limit of cursor position on right.
 *                              curleft :limit of cursor position on left.
 *                              ax1lastc:auxiliary area No.1 last character
 *                                       position.
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Control Block(KCB).
 *                              lastch  :input field last char position.
 *                              chlna1  :conversion char length of aux. 1.
 *                              chpsa1  :aux. area for conversion character
 *                                       display point.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              curright:limit of cursor position on right.
 *                              ax1lastc:auxiliary area No.1 last character
 *                                       position.
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
int  _ME_0c(pt)

KCB     *pt;            /* pointer to KCB.                              */

{
        int     _Mmvch();       /* Move character in a dimension.       */
        int     _Msetch();      /* Set up convert position              */
                                /*                for dsiplay.          */
        int     _MM_rtn();      /* Move conversion.                     */

        int     ret_code;       /* Return Value.                        */
        KMISA   *kjsvpt;        /* pointer to KMIsa.                    */

        /* parameter to _Mmvch().       */
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

        CPRINT(#### _ME_0c ####);
/*##*/  snap3( SNAP_KCB | SNAP_KMISA, SNAP_ME_0c , "start _ME_0c");

        ret_code = IMSUCC;      /* return value initialize              */
        kjsvpt = pt->kjsvpt;    /* set pointer to KMISA.                */

        if( kjsvpt->knjnumfg == C_INPFLD )
            {
                /* 2.
                 *      input field displied.
                 */
                /* 2. 1
                 *      check !  now cursol position.
                 */
                 if(    (kjsvpt->curright <= pt->curcol)
                     || (kjsvpt->curleft  == pt->lastch)    )
                    {
                        /* let beep.                                    */
                        ret_code = _MM_rtn( pt , A_BEEP );
/*##*/  snap3( SNAP_KCB | SNAP_KMISA, SNAP_ME_0c , "return.3.1 _ME_0c");
                        return( ret_code );
                    }

                /* 2. 2
                 *      delete a character immediately left cursor
                 *          and move character string immedi.
                 */
                /* parameter for _Mmvch set. (see declare)              */
                pos      = pt->curcol + C_DBCS;
                len      = pt->lastch - pt->curcol - C_DBCS;
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
                 *      set parameter fo r_Msetch.
                 */
                /* set parameter for _Msetch. (see declare)             */
                position         = pt->curcol;
                length           = pt->lastch - pt->curcol;

                /* set up convert position.                             */
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
                 if(    (kjsvpt->curright <= pt->cura1c)
                     || (kjsvpt->curleft  == kjsvpt->ax1lastc)    )
                    {
                        /* let beep.                                    */
                        ret_code = _MM_rtn( pt , A_BEEP );
/*##*/  snap3( SNAP_KCB | SNAP_KMISA, SNAP_ME_0c , "return.3.1 _ME_0c");
                        return( ret_code );
                    }
/* #(E)  1987.11.30. Flying Conversion Change */



                /* 3. 2
                 *      delete a character immediately left cursor
                 *                     and move character string immedi.
                 */
                /* parameter for _Mmvch set. (see declare)              */
                pos      = pt->cura1c + C_DBCS;



/* #(B)  1987.11.30. Flying Conversion Change */
                len      = kjsvpt->ax1lastc - pt->cura1c - C_DBCS;
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
                pt->chpsa1       = pt->cura1c;



/* #(B)  1987.11.30. Flying Conversion Change */

                /* set update character length of auxiliary area 1      */
                pt->chlna1       = kjsvpt->ax1lastc - pt->chpsa1;

                /* set auxiliary area No.1 last character position.     */
                kjsvpt->ax1lastc = kjsvpt->ax1lastc - C_DBCS;

                /* set cursor move right corner.                        */
                kjsvpt->curright = kjsvpt->ax1lastc ;

/* #(E)  1987.11.30. Flying Conversion Change */



            }

        /*
         *      Return.
         */
/*##*/  snap3( SNAP_KCB | SNAP_KMISA, SNAP_ME_0c , "return  _ME_0c");
        return( ret_code );
}
