static char sccsid[] = "@(#)42	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_MC_rtn.c, libKJI, bos411, 9428A410j 7/23/92 03:18:07";
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
 * MODULE NAME:         _MC_rtn
 *
 * DESCRIPTIVE NAME:    Cursor control routine.
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
 * FUNCTION:            Control cursor movement according to the
 *                      action code of input parameter.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1656 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _MC_rtn
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _MC_rtn( pt, kmactcd2 )
 *
 *  INPUT:              pt      :Pointer to Kanji Controle Block.
 *                      kmactcd2:Action Code from Kanji Monitor Action Table
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         KMSUCC  :Successful.
 *
 * EXIT-ERROR:          NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _Mcrmv()   : Move Cursor
 *                              _Maxcrmv() : Move Cursor in Auxiliary Area
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
 *                      Kanji Monitor Controle Block(KCB).
 *                              kjsvpt  axuse1  cura1c  cura1r  curcol
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              curleft curright        actc3   kkmode1
 *                              convpos cconvpos        cconvlen
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Controle Block(KCB).
 *                              curcol
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              actc3
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
 *      This module calls subroutines to move the cursor in input field or
 *      auxiliary area, according to the action code and current status.
 */
int  _MC_rtn( pt, kmatcd2 )

KCB     *pt;            /* Pointer to Kanji Control Block               */
uchar   kmatcd2;        /* Action Code from KMAT                        */
{
        register        KMISA   *kjsvpt;        /* Pointer to KMISA     */

        extern  int     _Mcrmv();       /* Cursor Move Process          */
        extern  int     _Maxcrmv();     /* Aux. Area Cursor Move Process*/

        char    axcrflg;        /* Auxiliary Area Cursor Flag           */
        short   nxtcol;         /* Next Cursor Column                   */
        short   nxtrow;         /* Next Cursor Row                      */
        int     rc;             /* Return Code.                         */

snap3(SNAP_KCB | SNAP_KMISA,SNAP_MC_rtn,"start _MC_rtn");

        /*
         *      Pointer to Kanji Monitor Internal Save Area
         */

        kjsvpt = pt->kjsvpt;

        /* 1. 1. 1
         *      Set Default Return Code
         */

        rc = KMSUCC;

        /* 1. 2
         *      Check Cursor Status
         */

        if ( pt->axuse1 == C_SWON &&
             pt->cura1c != C_FAUL &&
             pt->cura1r != C_FAUL )
                /* Cursor in Aux. Area to be Moved */
                axcrflg = C_SWON;
        else
                /* Cursor in Input Field to be Moved */
                axcrflg = C_SWOFF;

        /* 1. 3
         *      Cursor Control (KMAT Action Code 2)
         */

        switch ( kmatcd2 )
               {
                /* 1. 3-1
                 *      No Operation
                 */
                case A_NOP:
                        break;

                /* 1. 3-2
                 *      Move Cursor Rightword (1 Character)
                 */
                case A_CRSC:
                        /*
                         *      Aux. Area Cursor Flag
                         */

                        if ( axcrflg == C_SWOFF )
                           {
                                /*
                                 *      Move Cursor in Input Field
                                 */

                                /* Move Cursor Rightword (2 Bytes) */
                                nxtcol = pt->curcol + C_DBCS;

                                /* Call Cursor Moving Routine */
                                rc = _Mcrmv( pt, nxtcol );
                           }
                        else
                           {
                                /*
                                 *      Move Cursor in Auxiliary Area
                                 */

                                /* Move Aux. Cursor Rightword (2 Bytes) */
                                nxtcol = pt->cura1c + C_DBCS;
                                /* Not Move in Row */
                                nxtrow = pt->cura1r;

                                /*
                                 * Call Aux. Area Cursor Moving Routine
                                 */
                                rc = _Maxcrmv( pt, nxtcol, nxtrow );
                           };
                        break;

                /* 1. 3-3
                 *      Move Cursor Leftword (1 Character)
                 */
                case A_CLSC:
                        /*
                         *      Aux. Area Cursor Flag
                         */

                        if ( axcrflg == C_SWOFF )
                           {
                                /*
                                 *      Move Cursor in Input Field
                                 */

                                /* Move Cursor Leftword (2 Bytes) */
                                nxtcol = pt->curcol - C_DBCS;

                                /* Call Cursor Moving Routine */
                                rc = _Mcrmv( pt, nxtcol );
                           }
                        else
                           {
                                /*
                                 *      Move Cursor in Auxiliary Area
                                 */

                                /* Move Aux. Cursor Leftword (2 Bytes) */
                                nxtcol = pt->cura1c - C_DBCS;
                                /* Not Move in Row */
                                nxtrow = pt->cura1r;

                                /*
                                 * Call Aux. Area Cursor Moving Routine
                                 */
                                rc = _Maxcrmv( pt, nxtcol, nxtrow );
                           };
                        break;

                /* 1. 3-4
                 *      Move Cursor Upp (1 Character)
                 */
                case A_CUSC:
                        break;

                /* 1. 3-5
                 *      Move Cursor Low (1 Character)
                 */
                case A_CDSC:
                        break;

                /* 1. 3-6
                 *      Move Cursor Rightword (2 Character)
                 */
                case A_CRDC:
                        /*
                         *      Aux. Area Cursor Flag
                         */

                        if ( axcrflg == C_SWOFF )
                           {
                                /*
                                 *      Move Cursor in Input Field
                                 */

                                /* Move Cursor Rightword (4 Bytes) */
                                nxtcol = pt->curcol + C_DCHR;

                                /* Call Cursor Moving Routine */
                                rc = _Mcrmv( pt, nxtcol );

                                /*
                                 * Check Overflow by Cursor Movement
                                 */

                                if ( rc == KMCROTW )
                                   {
                                       /* Move Cursor Leftword (2 Bytes) */
                                        nxtcol = nxtcol - C_DBCS;

                                       /*
                                        * Call Cursor Moving Routine
                                        */
                                        rc = _Mcrmv( pt, nxtcol );
                                   };
                           }
                        else
                           {
                                /*
                                 *      Move Cursor in Auxiliary Area
                                 */

                                /* Move Aux. Cursor Rightword (4 Bytes) */
                                nxtcol = pt->cura1c + C_DCHR;
                                /* Not Move in Row */
                                nxtrow = pt->cura1r;

                                /*
                                 * Call Aux. Area Cursor Moving Routine
                                 */
                                rc = _Maxcrmv( pt, nxtcol, nxtrow );

                                /*
                                 * Check Overflow by Cursor Movement
                                 */

                                if ( rc == KMCROTW )
                                   {
                                        /*
                                         * Move Aux. Cursor Leftword
                                         *             (2 Bytes)
                                         */
                                        nxtcol = nxtcol - C_DBCS;

                                        /*
                                         * Call Aux. Area
                                         *      Cursor Moving Routine
                                         */
                                        rc = _Maxcrmv( pt, nxtcol ,nxtrow );
                                   };

                           };
                        break;

                /* 1. 3-7
                 *      Move Cursor Leftword (2 Character)
                 */
                case A_CLDC:
                        /*
                         *      Aux. Area Cursor Flag
                         */

                        if ( axcrflg == C_SWOFF )
                           {
                                /*
                                 *      Move Cursor in Input Field
                                 */

                                /* Move Cursor Leftword (4 Bytes) */
                                nxtcol = pt->curcol - C_DCHR;

                                /* Call Cursor Moving Routine */
                                rc = _Mcrmv( pt, nxtcol );

                                /*
                                 * Check Overflow by Cursor Movement
                                 */

                                if ( rc == KMCROTW )
                                   {
                                       /* Move Cursor Rightword (2 Bytes) */
                                        nxtcol = nxtcol + C_DBCS;

                                       /*
                                        * Call Cursor Moving Routine
                                        */
                                        rc = _Mcrmv( pt, nxtcol ,nxtrow );
                                   };
                           }
                        else
                           {
                                /*
                                 *      Move Cursor in Auxiliary Area
                                 */

                                /* Move Aux. Cursor Leftword (4 Bytes) */
                                nxtcol = pt->cura1c - C_DCHR;
                                /* Not Move in Row */
                                nxtrow = pt->cura1r;

                                /*
                                 * Call Aux. Area Cursor Moving Routine
                                 */
                                rc = _Maxcrmv( pt, nxtcol, nxtrow );

                                /*
                                 * Check Overflow by Cursor Movement
                                 */

                                if ( rc == KMCROTW )
                                   {
                                        /*
                                         * Move Aux. Cursor Rightword
                                         *             (2 Bytes)
                                         */
                                        nxtcol = nxtcol + C_DBCS;

                                        /*
                                         * Call Aux. Area
                                         *      Cursor Moving Routine
                                         */
                                        rc = _Maxcrmv( pt, nxtcol ,nxtrow );
                                   };
                           };
                        break;

                /* 1. 3-8
                 *      Move Cursor to Starting Position in Input Field
                 *      (If cursor position is alreading at
                 *       the input field top, return to application)
                 */
                case A_CIFS:
                        /*
                         *      Aux. Area Cursor Flag
                         */

                        if ( axcrflg == C_SWOFF )
                           {
                                /*
                                 *      Move Cursor in Input Field
                                 */

                                if ( pt->curcol > kjsvpt->curleft )
                                   {
                                        /* Move Cursor to Starting Pos. */
                                        nxtcol = kjsvpt->curleft;

                                        /* Call Cursor Moving Routine */
                                        rc = _Mcrmv( pt, nxtcol );
                                   }
                                else
                                   {
                                        /*
                                         * Set Action Code to Routine
                                         *      to The Application
                                         */
                                        kjsvpt->actc3 = kjsvpt->actc3 |
                                                              A_DICOFF;
                                   };
                           }
                        else
                           {
                                /*
                                 *      Move Cursor in Auxiliary Area
                                 */

                                /* Move Cursor to Start Pos. in Aux. Area */
                                nxtrow = pt->cura1r;

                                if ( pt->cura1c > kjsvpt->curleft )
                                   {
                                        /*
                                         * Move Cursor to Start Position
                                         *      in Aux. Area
                                         */
                                        nxtcol = kjsvpt->curleft;
                                        nxtrow = pt->cura1r;

                                        /*
                                         * Call Aux. Area
                                         *      Cursor Moving Routine
                                         */
                                        rc = _Maxcrmv( pt, nxtcol, nxtrow );
                                   }
                                else
                                   {
                                        /*
                                         * Set Action Code to Routine
                                         *      to The Application
                                         */
                                        kjsvpt->actc3 = kjsvpt->actc3 |
                                                              A_DICOFF;
                                   };
                           };
                        break;

                /* 1. 3-9
                 *      Move Cursor to The End Position of
                 *              Current Conversion Area
                 */
                case A_CCVN:
                        /* Set Next Cursor Position */
                        nxtcol = kjsvpt->cconvpos + kjsvpt->cconvlen;

                        /* Call Cursor Moving Routine */
                        rc = _Mcrmv( pt, nxtcol );
                        break;

                /* 1. 3-10
                 *      Move Cursor to The Hind Most Position of
                 *              Current Conversion Area
                 */
                case A_CCVE:
                        /* Set Next Cursor Position */
                        nxtcol = kjsvpt->cconvpos +
                                 kjsvpt->cconvlen - C_DBCS;

                        /* Call Cursor Moving Routine */
                        rc = _Mcrmv( pt, nxtcol );
                        break;

                /* 1. 3-11
                 *      Move Cursor to The Start Position of Yomi
                 */
                case A_CYMS:
                        /* Set Next Cursor Position */
                        pt->curcol = kjsvpt->convpos;
                        break;

                /* 1. 3-12
                 *      Move Cursor Rightword (1 Character)
                 *              (No Attribute Changing)
                 */
                case A_CRNA:
                        /*
                         *      Aux. Area Cursor Flag
                         */

                        if ( axcrflg == C_SWOFF )
                           {
                                /*
                                 *      Move Cursor in Input Field
                                 */

                                if ( (pt->curcol < kjsvpt->curleft ||
                                      pt->curcol > kjsvpt->curright ) &&
                                     (kjsvpt->kkmode1 != A_KNJNOM ) )
                                   {
                                        /*
                                         *      Set Warning for Invalid
                                         *              Cursor Position
                                         */
                                        rc = KMCROTW;
                                   }
                                else
                                   {
                                        /*
                                         *      Set Cursor Movement Range
                                         */

                                        if ( pt->curcol + C_DBCS <=
                                                   kjsvpt->curright )
                                                /*
                                                 * Move Cursor Rightword
                                                 *              (2 byte)
                                                 */
                                                pt->curcol += C_DBCS;
                                   };
                           }
                        else
                           {
                                /*
                                 *      Move Cursor in Auxiliary Area
                                 */

                                if ( (pt->cura1c < kjsvpt->curleft ||
                                      pt->cura1c > kjsvpt->curright) &&
                                     (kjsvpt->kkmode1 != A_KNJNOM) )
                                   {
                                        /*
                                         *      Set Warning for Invalid
                                         *              Cursor Position
                                         */
                                        rc = KMCROTW;
                                   }
                                else
                                   {
                                        /*
                                         *      Check Cursor Movement
                                         *              in Aux.Area
                                         */

                                        if ( pt->cura1c + C_DBCS <=
                                                   kjsvpt->curright )
                                                /*
                                                 * Move Cursor Rightword
                                                 *      in Aux. Area (2 byte)
                                                 */
                                                pt->cura1c += C_DBCS;
                                   };
                           };
                        break;

                /* 1. 3-13
                 *      Move Cursor Leftword (1 Character)
                 *              (No Attribute Changing)
                 */
                case A_CLNA:
                        /*
                         *      Aux. Area Cursor Flag
                         */

                        if ( axcrflg == C_SWOFF )
                           {
                                /*
                                 *      Move Cursor in Input Field
                                 */

                                if ( (pt->curcol < kjsvpt->curleft ||
                                      pt->curcol > kjsvpt->curright) &&
                                     (kjsvpt->kkmode1 != A_KNJNOM) )
                                   {
                                        /*
                                         *      Set Warning for Invalid
                                         *              Cursor Position
                                         */
                                        rc = KMCROTW;
                                   }
                                else
                                   {
                                        /*
                                         *      Set Cursor Movement Range
                                         */
                                        if ( pt->curcol - C_DBCS >=
                                                    kjsvpt->curleft )
                                                /*
                                                 * Move Cursor Leftword
                                                 *              (2 byte)
                                                 */
                                                pt->curcol -= C_DBCS;
                                   };

                           }
                        else
                           {
                                /*
                                 *      Move Cursor in Auxiliary Area
                                 */

                                if ( (pt->cura1c < kjsvpt->curleft ||
                                      pt->cura1c > kjsvpt->curright) &&
                                     (kjsvpt->kkmode1 != A_KNJNOM) )
                                   {
                                        /*
                                         *      Set Warning for Invalid
                                         *              Cursor Position
                                         */
                                         rc = KMCROTW;
                                   }
                                else
                                   {
                                        /*
                                         *      Check Cursor Movement
                                         *              in Aux.Area
                                         */

                                        if ( pt->cura1c - C_DBCS >=
                                                    kjsvpt->curleft )
                                                /*
                                                 * Move Cursor Leftword
                                                 *      in Aux. Area (2 byte)
                                                 */
                                                pt->cura1c -= C_DBCS;
                                   };

                           };
                        break;

                /* 1. 3-14
                 *      Yomi Start Cursor Move
                 */
                case A_CSTP:
                        /*
                         *      Move Cursor to Starting Position
                         *              in Input Field
                         */

                        if ( axcrflg == C_SWOFF )
                           {
                                /*
                                 *      Move Cursor in Input Field
                                 */

                                /* Move Cursor to Starting Position */
                                pt->curcol = kjsvpt->curleft;
                           }
                        else
                           {
                                /*
                                 *      Move Cursor in Auxiliary Area
                                 */

                                /* Move Cursor to Start Pos. in Aux. Area */
                                pt->cura1c = kjsvpt->curleft;
                           };
                        break;

                    default:
                        break;
               };

        /* 1. 4
         *      Return Code.
         */

snap3(SNAP_KCB | SNAP_KMISA,SNAP_MC_rtn,"return _MC_rtn");

        return( rc );
}
