static char sccsid[] = "@(#)91	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mdagend.c, libKJI, bos411, 9428A410j 7/23/92 03:21:12";
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

/********************* START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:         _Mdagend
 *
 * DESCRIPTIVE NAME:    Dagnosis end
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
 * FUNCTION:            Stop a diagnosis, if instructed.
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
 * ENTRY POINT:         _Mdagend
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mdagend( pt )
 *
 *  INPUT:              pt      :Pointer to KCB Editor Controle Block.
 *
 *  OUTPUT:             nextact(KMISA)
 *
 * EXIT-NORMAL:         IMDGVK   :Success of Execution.
 *
 * EXIT-ERROR:          NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _MM_rtn   Mode chenge routine.
 *                              _Tracef    memory trace data file control.
 *                              _Mfmrst   message reset.
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
 *                              type     Input code type.
 *                              code     Input code.
 *                              tracep   Pointer to Trace Area.(TRB)
 *                              trace    Trace flag.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              nextact  Next action function indicate flag.
 *                              msetflg  Message Display Status Flag.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              NA.
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Controle Block(KCB).
 *                              NA.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              nextact  Next action function indicate flag.
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
#include "kcb.h"        /* Kanji controul block structer KCB            */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-061 COPYRIGHT IBM CORP 1988           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Stop a diagnosis , if instructed.
 */
_Mdagend( pt1 )

KCB     *pt1;      /* Pointer to KCB                                    */
{
        int     _Tracef();       /* Memory trace data file control.      */
        int     _MM_rtn();      /* Mode change routine.(beep output)    */
        int     _Mifmst();      /* Message reset.                       */
        KCB     *pt;            /* Pointer to KCB                       */
        KMISA   *kjsvpt;        /* Pointer to KMISA                     */
        TRB     *troutp;        /* Pointer to TRB                       */

        int     rc;             /* Return Code.(internal function)      */
        uchar   code;           /* key input code (KCB)                 */
        uchar   m_code;         /* beep put out mode code (_MM_rtn)     */
        short   opcode;         /* trace operation code. open or close  */
        uchar   msg_typ;        /* reset the display message (_Mfmrst)  */
/*************************************************************************
 *      debug process routine.                                           */
snap3(SNAP_EXT  ,SNAP_Mdagend,"START KMDAGEND");
snap3(SNAP_KMISA,SNAP_Mdagend,"START KMDAGEND");

        /*  1.1
         *      Get KCB pointer.
         */
        pt = pt1;

        /*  1.2
         *      Get KMISA pointer
         */
        kjsvpt = pt->kjsvpt;

        /*  1.3
         *     Check the key of input data.(1. type check)
         */

        /*  1.3 (1) Check type and code.                                */
        code = pt->code;

        if    (
                   (
                       pt->type != K_INASCI
                   )
                ||
                   (
                      (code != C_1CDSY ) && (code != C_1CDLY ) &&
                      (code != C_1CDKNN) && (code != C_1CDSN ) &&
                      (code != C_1CDLN ) && (code != C_1CDKMI)
                   )
              )

            /* 1.3. (1).(a) put out beep.                           */
            {



/* #(B) 1988.01.12. Flying Conversion Change */
                if ( pt->type == K_INESCF ) {

                    /*
                     *      Check Input Code.
                     */
                    switch ( pt->code ) {

                    case P_KATA :   /* Katakana shift.              */
                    case P_ALPHA:   /* Alpha/Num shift.             */
                    case P_HIRA :   /* Hiragana shift.              */
                    case P_RKC  :   /* RKC.                         */

                        break;

                    default :
                        m_code = A_BEEP;
                        _MM_rtn( pt,m_code );
                        break;
                    };
                };
/* #(E) 1988.01.12. Flying Conversion Change */



/*************************************************************************
 *      debug process routine.                                           */
snap3(SNAP_EXT  ,SNAP_Mdagend,"_MM_rtn ERROR KMDAGEND");
snap3(SNAP_KMISA,SNAP_Mdagend,"_MM_rtn ERROR KMDAGEND");

                /* 1.3. (1).(b) input invalid data return.               */
                return( IMDGVK );
            }

        /*  1.3 (2) Check code. (type == K_INASCI)                      */
        if ((code == C_1CDSY) || (code == C_1CDLY) || (code == C_1CDKNN))
            {
                /* 1.3 (2).(a) change the trace on memory.              */
                /* Set the trace data output pointer on KCB.            */
                troutp = pt->tracep;

                /* the operation code is on memory change code.         */
                opcode = C_TRFCL;

                rc     = _Tracef( troutp,opcode );

                /* 1.3 (2).(b)  set trace flag.(change disk to memory)  */
                pt->trace = K_TLIMIT;
            }

        /*  1.4  nextact : next entry bit off. ( to _Mdagmsg)           */
                kjsvpt->nextact = (kjsvpt->nextact & ~M_DAGEON);

        /*  1.5  reset output message.                                  */
        msg_typ = K_MSGOTH;
        rc = _Mfmrst( pt,msg_typ );

/*************************************************************************
 *      debug process routine.                                           */
snap3(SNAP_EXT  ,SNAP_Mdagend,"END KMDAGEND");
snap3(SNAP_KMISA,SNAP_Mdagend,"END KMDAGEND");

        /*  1.6  Complete stop a diagnosis data writeing.                */
        return( IMDGVK );
}

