static char sccsid[] = "@(#)05	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mfmrst.c, libKJI, bos411, 9428A410j 7/23/92 03:22:18";
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
 * MODULE NAME:         _Mfmrst
 *
 * DESCRIPTIVE NAME:    Recover Field Save Information & Release
 *                      Resource.
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
 * FUNCTION:            1. Recover Input Field Information.
 *                      2. Release Resource.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        804 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mfmrst
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mfmrst( pt,msg_type )
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *                      msg_type:Release Message_type.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      IMSUCC  :Success of Execution.
 *                      IMNOSAVW:No Field Save Informatin.
 *
 * EXIT-ERROR:          Waits State Code.
 *                      IMIVMSGE:Invalid Message Type.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Internal Subroutines.
 *                              NA.
 *                      Kanji Project Subroutines.
 *                              _Msetch :Field Redraw Range Set.
 *                      Standard Library.
 *                              memcpy  :Copy # of Character.
 *                              memset  :Set  # of Specified Character.
 *                      Advanced Display Graphics Support Library(GSL).
 *                              NA.
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      See Below.
 *
 *   INPUT:             Kanji Monitor Control Block(KCB).
 *                              NA.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              auxflg1  auxflg2  auxflg3  auxflg4
 *                              ifsaved  ifsaveo  kjsvpt
 *                              FSB Memger.
 *                              curcol   curleft  curright currow
 *                              string   hlatst   length
 *                      Trace Block(TRB).
 *                              NA.
 *                      Kana Kanji Control Block(KKCB).
 *                              NA.
 *
 *   OUTPUT:            Kanji Monitor Control Block(KCB).
 *                              axuse1   axuse2   axuse3   axuse4
 *                              curcol   currow   hlatst   string
 *                              lastch
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              auxflg1  auxflg2  auxflg3  auxflg4
 *                              curleft  curright msetflg
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
#include "kcb.h"        /* Kanji Control Block.                         */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-061 COPYRIGHT IBM CORP 1988           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Field Save Block Data Reocover.
 */
int     _Mfmrst( pt,msg_type )

register KCB *pt;       /* Pointer to Kanji Control Block(KCB).         */
uchar   msg_type;       /* Display Message Type.                        */

{
        int     _Msetch();      /* Display String Refresh Range Set.    */

        char    *memcpy();      /* Memory Copy Operation.               */
        char    *memset();      /* Memory Set Operation.                */

        register KMISA *kjsvpt; /* Pointer to Kanji Monitor Internal    */
                                /* Save Area.                           */
        register FSB   *fsb;    /* Pointer to Active FSB.               */

        int ret_code;           /* Return Value.                        */

        /*
         *      Debugging Output.
         */
        snap3(SNAP_KCB|SNAP_KMISA,SNAP_Mfmrst,"Start");

        /*
         ****************************************************************
         *      0. Set Up General Work Pointer.
         ****************************************************************
         */
        /*
         *      Initialize Work Pointer & Intitial Value.
         */
        kjsvpt  = pt->kjsvpt;   /* Get Pointer to KMISA.                */
        ret_code= IMSUCC;       /* Defult Return Value Succesful.       */

        /*
         *      Collection Return Position.
         */
        do {
                /*
                 ********************************************************
                 *      1. Already Display Information Save?
                 ********************************************************
                 */
                /*
                 *      Allreaddy Save Display Information.
                 *      ,if not Save Display Information
                 *      then msetflg(KMISA) do't set specified message type
                 *      data bits.
                 */
                if( !(kjsvpt->msetflg & msg_type ) ) {
                        /*
                         *      No Save Information.
                         */
                        ret_code = IMNOSAVW;
                        break;
                };

                /*
                 *      Message Type Check and Save Field Save Address Get.
                 */
                switch( msg_type ) {
                /*
                 *      General Message.
                 */
                case K_MSGOTH:
                        fsb = kjsvpt->ifsaveo;
                        break;
                /*
                 *      Dictionary Registration Message.
                 */
                case K_MSGDIC:
                        fsb = kjsvpt->ifsaved;
                        break;
                /*
                 *      Unknown Message.
                 */
                default:
                        ret_code = IMIVMSGE;
                        continue;
                };

                /*
                 ********************************************************
                 *      2. Field Save Information Restore to Kanji
                 *      Control Block & Kanji Monitor Internal Save
                 *      Area.
                 ********************************************************
                 */
                /*
                 *      Recover Input Field Data.
                 */
                if( fsb->length != 0 ) {
                        /*
                         *      Save String Set KCB Display String Area.
                         */
                        (void)memcpy((char *)pt->string,(char *)fsb->string,
                                      fsb->length);

                        /*
                         *      Save String Highlighting Attribute Set KCB
                         *      Highlighting Attribute.
                         */
                        (void)memcpy((char *)pt->hlatst,(char *)fsb->hlatst,
                                     fsb->length);

                        /*
                         *      Refresh String Range Set.
                         */
                         (void)_Msetch( pt,C_COL,fsb->length );
                };

                /*
                 *      Save Cursor Position Recover.
                 */
                /*
                 *      Recover Input Field Cursor Col.
                 */
                pt->curcol = fsb->curcol;
                /*
                 *      Recover Input Field Cursor Row.
                 */
                pt->currow = fsb->currow;

                /*
                 *      Cursor Movemnt Range Sets.
                 */
                /*
                 *      Recover Input Field Left Margin.
                 */
                kjsvpt->curleft = fsb->curleft;
                /*
                 *      Recover Input Field Right Margin.
                 */
                kjsvpt->curright= fsb->curright;

                /*
                 *      Recover Input Field Last Character Position.
                 */
                pt->lastch      = fsb->lastch;

                /*
                 *      Reset Message ID.
                 */
                kjsvpt->msetflg &= ~msg_type;

                /*
                 ********************************************************
                 *      3. Auxiliary Area Lock Flag Claer & Initialize
                 *      Auxiliay Area.
                 ********************************************************
                 */
                /*
                 *      Auxiliary Area Use Flag Claer.
                 */
                /*
                 *      Message is Use Auxiliary Area No.1
                 */
                       if( kjsvpt->auxflg1 == msg_type ) {
                                /*
                                 *      Release Auxiliary Area Use Flag.
                                 */
                                kjsvpt->auxflg1 = K_ANOUSE;
                                pt->axuse1      = K_ANOUSE;

                                /*
                                 *      Cursor Position Invisible.
                                 */
                                pt->cura1c      = C_FAUL;
                                pt->cura1r      = C_FAUL;
                /*
                 *      Message is Use Auxiliary Area No.2
                 */
                } else if( kjsvpt->auxflg2 == msg_type ) {
                                /*
                                 *      Release Auxiliary Area Use Flag.
                                 */
                                kjsvpt->auxflg2 = K_ANOUSE;
                                pt->axuse2      = K_ANOUSE;

                                /*
                                 *      Cursor Position Invisible.
                                 */
                                pt->cura2c      = C_FAUL;
                                pt->cura2r      = C_FAUL;
                /*
                 *      Message is Use Auxiliary Area No.3
                 */
                } else if( kjsvpt->auxflg3 == msg_type ) {
                                /*
                                 *      Release Auxiliary Area Use Flag.
                                 */
                                kjsvpt->auxflg3 = K_ANOUSE;
                                pt->axuse3      = K_ANOUSE;

                                /*
                                 *      Cursor Position Invisible.
                                 */
                                pt->cura3r      = C_FAUL;
                                pt->cura3c      = C_FAUL;
                /*
                 *      Message is Use Auxiliary Area No.4
                 */
                } else if( kjsvpt->auxflg4 == msg_type ) {
                                /*
                                 *      Release Auxiliary Area Use Flag.
                                 */
                                kjsvpt->auxflg4 = K_ANOUSE;
                                pt->axuse4      = K_ANOUSE;

                                /*
                                 *      Cursor Position Invisible.
                                 */
                                pt->cura4c      = C_FAUL;
                                pt->cura4r      = C_FAUL;
                };

                /*
                 ********************************************************
                 *      4. Completer of Execution.
                 *      Auxiliay Area.
                 ********************************************************
                 */
                /*
                 *      Complete of Execution.
                 */
                ret_code = IMSUCC;

        } while( NILCOND );

        /*
         *      Debugging Output.
         */
        snap3(SNAP_KCB|SNAP_KMISA,SNAP_Mfmrst,"End");

        /*
         ****************************************************************
         *      5. Return to Caller.
         ****************************************************************
         */
        return( ret_code );
}

