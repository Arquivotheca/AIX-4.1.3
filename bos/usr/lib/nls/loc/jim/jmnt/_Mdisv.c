static char sccsid[] = "@(#)99	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mdisv.c, libKJI, bos411, 9428A410j 7/23/92 03:21:47";
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
 * MODULE NAME:         _Mdisv
 *
 * DESCRIPTIVE NAME:    Save Display Information.
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
 * FUNCTION:            Input Field Data & Cursor Move Data Save.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        568 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mdisv
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mdisv( pt,msg_type )
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *                      msg_type:Save Message Type.
 *                      sav_inf :Save Information Conditon.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      IMSUCC  :Success of Execution.
 *                      IMIVMSGE:Invalid Message Type.
 *
 * EXIT-ERROR:          NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Internal Subroutines.
 *                              NA.
 *                      Kanji Project Subroutines.
 *                              NA.
 *                      Standard Library.
 *                              memcpy  :Copy # of Character.
 *                      Advanced Display Graphics Support Library(GSL).
 *                              NA.
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      See Below.
 *
 *   INPUT:             Kanji Monitor Control Block(KCB).
 *                              hlatst   indlen   kjsvpt  string
 *                              lastch
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              curleft  curright ifsaved  ifsaveo
 *                              realcol
 *                      Trace Block(TRB).
 *                              NA.
 *                      Kana Kanji Control Block(KKCB).
 *                              NA.
 *
 *   OUTPUT:            Kanji Monitor Control Block(KCB).
 *                              NA.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              FSB Area Member.
 *                              curcol   curleft  curright currow
 *                              lastch   length   hlatst   string
 *                      Trace Block(TRB).
 *                              NA.
 *                      Kana Kanji Control Block(KKCB).
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
#include <stdio.h>      /* Standar I/O Header.                          */
#include <memory.h>     /* Memory Operation.                            */

/*
 *      include Kanji Project.
 */
#include "kj.h"         /* Kanji Define File.                           */
#include "kcb.h"        /* Kanji Monitor Control Block.                 */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-061 COPYRIGHT IBM CORP 1988           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Display Information Specified Message ID Save Area,
 *      Each Save Condition.
 */
int     _Mdisv( pt,msg_type,sav_inf )

register KCB *pt;       /* Pointer to Kanji Control Block(KCB).         */
uchar   msg_type;       /* Display Message Type.                        */
uchar   sav_inf;        /* Save Information Flag.                       */

{
        char    *memcpy();      /* Memory Copy Operation.               */

        register KMISA *kjsvpt; /* Pointer to Kanji Monitor Internal    */
                                /* Save Area.                           */
        register FSB   *fsb;    /* Pointer to Field Save Area.          */
        int     ret_code;       /* Return Value.                        */

        /*
         *      Debugging Dump.
         */
        snap3(SNAP_KCB|SNAP_KMISA|SNAP_FSB,SNAP_Mdisv,"Start");

        /*
         ****************************************************************
         *      0. Set Up General Work Pointer.
         ****************************************************************
         */
        /*
         *      Initialize Work Pointer & Intitial Value.
         */
        kjsvpt  = pt->kjsvpt;
        ret_code= IMSUCC;

        /*
         *      Collection for Return.
         */
        do {
                /*
                 ********************************************************
                 *      1. Message Type Analize.
                 ********************************************************
                */
                /*
                 *      FSB Address Get.
                 */
                switch( msg_type ) {
                /*
                 *      General Message Output Type.
                 */
                case K_MSGOTH:
                        fsb = kjsvpt->ifsaveo;
                        break;
                /*
                 *      Dictionary Registration Message Output Type.
                 */
                case K_MSGDIC:
                        fsb = kjsvpt->ifsaved;
                        break;
                /*
                 *      Unkonw Message Output Type.
                 */
                default:
                        fsb = NULL;
                        break;
                };

                /*
                 *      Message Not Availabele.
                 */
                if( fsb == NULL ) {
                        ret_code = IMIVMSGE;
                        break;
                };

                /*
                 ********************************************************
                 *      2. Display Information Save.
                 ********************************************************
                */
                /*
                 *      Input Field Information Save?
                 */
                if( (sav_inf & M_NSVIF)!=0 ) {
                        fsb->length = 0;
                } else {
                        /*
                         *      Input FIeld String & Hilighting Save.
                         */
                        /*
                         *      Field Data Length Save.
                         */
                        fsb->length = kjsvpt->realcol - pt->indlen;

                        /*
                         *      Save Input Field String.
                         */
                        (void)memcpy((char *)fsb->string,
                                     (char *)pt->string,fsb->length);

                        /*
                         *      Save Input Field Hilighting.
                         */
                        (void)memcpy((char *)fsb->hlatst,
                                     (char *)pt->hlatst,fsb->length);
                };

                /*
                 *      Cursor Position Save.
                 */
                fsb->curcol = pt->curcol;
                fsb->currow = pt->currow;

                /*
                 *      Cursor Move Range Save.
                 */
                fsb->curleft = kjsvpt->curleft;
                fsb->curright= kjsvpt->curright;

                /*
                 *      Last Character Position Save.
                 */
                fsb->lastch = pt->lastch;

        } while( NILCOND );

        /*
         *      Debugging Dump.
         */
        snap3(SNAP_KCB|SNAP_KMISA|SNAP_FSB,SNAP_Mdisv,"End");

        /*
         ****************************************************************
         *      3. Return to Caller.
         ****************************************************************
         */
        return( ret_code );
}

