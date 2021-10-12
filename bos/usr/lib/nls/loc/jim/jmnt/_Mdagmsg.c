static char sccsid[] = "@(#)92	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mdagmsg.c, libKJI, bos411, 9428A410j 7/23/92 03:21:17";
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

/********************* START OF MODULE SPECIFICATIONS *********************
 *
 * MODULE NAME:         _Mdagmsg
 *
 * DESCRIPTIVE NAME:    Diagnosis start/stop message.
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
 * FUNCTION:            Set the diagnosis message display data.
 *
 * NOTES:               Starting the function for
 *                               change the distination of diagnosis.
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1132 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mdagmsg
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mdagmsg( pt )
 *
 *  INPUT:              pt      :Pointer to KCB Editor Controle Block.
 *
 *  OUTPUT:             nextact(KMISA)
 *
 * EXIT-NORMAL:         IMDGSUCC :Success of Execution.
 *
 *
 * EXIT-ERROR:          IMDGICW  :Invalid input data.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _Maxmst : aid area parameter set.
 *                              _Mifmst : input area parameter set.
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
 *                              type     Input code type.
 *                              code     Input code.
 *                              maxa1c   Maximum number(byte)
 *                                         of columns of auxiliary.
 *                              axuse1   Auxiliary area No.1. use flag.
 *                              trace    Trace flag.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              nextact  Next action function indicate flag.
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
 *      Check a input data and display amessage of diagnosis start/stop.
 */
_Mdagmsg( pt1 )

KCB     *pt1;      /* Pointer to KCB                                    */
{
        int     _Maxmst();      /* Aid area parameter set.              */
        int     _Mifmst();      /* Input area parameter set.            */
KCB     *pt;
KMISA   *kjsvpt;  /* Pointer to KMISA                                   */

        int     rc;             /* Return Code.(internal function)      */
        int     i,j;            /* Loop Counter.                        */

        /* the parameter of _Maxmst function or of _Mfmrst function.    */
        uchar   msg_typ;        /* the message type for it display.      */
        short   area_num;       /* the aid area number for it stored.(=1)*/
        short   scr_col;        /* width length.                         */
        short   scr_row;        /* line number.                          */
        short   csr_col;        /* cursor horizontal position.           */
        short   csr_row;        /* cursor vertical position.             */
        short   csr_lft;        /* cursor horizontal motiom size.        */
        short   csr_rgt;        /* cursor vertical motion size.          */
        short   str_len;        /* display character number. (byte count)*/
        uchar   *str_ptr;       /* display character source pointer.(DBCS*/

        /* trase start message.                                          */
static  uchar   trsmsg[32] = {0x81,0x96,0x83,0x67,0x83,0x8c,0x81,0x5b,
                              0x83,0x58,0x8a,0x4a,0x8e,0x6e,0x81,0x48,
                              0x81,0x69,0x82,0x99,0x81,0x5e,0x82,0x8e,
                              0x81,0x6a,0x81,0x96,0x00,0x00,0x00,0x00};
        /* trace end message.                                            */
static  uchar   tremsg[32] = {0x81,0x96,0x83,0x67,0x83,0x8c,0x81,0x5b,
                              0x83,0x58,0x8f,0x49,0x97,0xb9,0x81,0x48,
                              0x81,0x69,0x82,0x99,0x81,0x5e,0x82,0x8e,
                              0x81,0x6a,0x81,0x96,0x00,0x00,0x00,0x00};
/*************************************************************************
 *      debug process routine.                                           */
snap3(SNAP_KCB  ,SNAP_Mdagend,"START KMDAGEND");

        /*  1.1
         *      Get KCB pointer.
         */
        pt = pt1;

        /*  1.2
         *      Get KMISA pointer
         */
        kjsvpt = pt->kjsvpt;

        /*  1.3
         *     Check the key input data.(1. type check)
         */
        if (pt->type != K_INESCF)
            {
                /* nextact bit reset and error return.                   */
                kjsvpt->nextact = kjsvpt->nextact & ~M_DAGMON;
/*************************************************************************
 *      debug process routine.                                           */
snap3(SNAP_KCB  ,SNAP_Mdagend,"ERROR 1.3 KMDAGEND");

                return( IMDGIVK );
            }

        /*  1.4
         *     Check the key input data.(2. code check)
         */
        if (pt->code == P_CURU)
            /*  1.5.0  Trace start request.                             */
            {
                if (pt->trace == K_TALL)
                    {
                        /* 1.5.0.1 trace not start                      */

                        /* nextact bit reset and error return.          */
                        kjsvpt->nextact = kjsvpt->nextact & ~M_DAGMON;
/*************************************************************************
 *      debug process routine.                                           */
snap3(SNAP_KCB  ,SNAP_Mdagend,"ERROR 1.5.0 KMDAGMSG");

                        return( IMDGIVK );
                    }

                /* 1.5.1 confirmation massage output for trace start.   */
                if ( (pt->maxa1c >= M_MSSLNC) &&
                     (pt->maxa1r >= M_MSSLNR) && (pt->axuse1 == K_ANOUSE) )
                    {
                        /* put out the massage to aid area on display.  */

                        /* the message type for it display.             */
                        msg_typ   = K_MSGOTH;
                        /* the aid area number for it stored.(=1)       */
                        area_num  = C_AUX1;
                        /* width length.                                */
                        scr_col   = M_DGMSCL;
                        /* line number.                                 */
                        scr_row   = M_DGMSRL;
                        /* cursor horizontal position.                  */
                        csr_col   = M_DGMSCX;
                        /* cursor vertical position.                    */
                        csr_row   = M_DGMSCY;
                        /* cursor horizontal motiom size.               */
                        csr_lft   = M_DGSCL;
                        /* cursor vertical motion size.                 */
                        csr_rgt   = M_DGSCR;
                        /* display character number. (byte count)       */
                        str_len   = 28;
                        /* display character source pointer.(DBCS)      */
                        str_ptr   = trsmsg;   /* trace start massage.   */

                        rc = _Maxmst( pt,msg_typ,area_num,scr_col,scr_row,
                        csr_col,csr_row,csr_lft,csr_rgt,str_len,str_ptr );
                        /* rc = IMSUCC:success,IMTRUSTW:IMIVMSGE:error  */
                    }
                else
                    {
                        /*  1.5.3
                         *     put out the massage to input area on display.
                         */

                        /* the message type for it display.              */
                        msg_typ   = K_MSGOTH;
                        /* width length.                                 */
                        csr_col   = M_DGMSCX;
                        /* line number.                                  */
                        csr_row   = M_DGMSCY;
                        /* cursor horizontal motiom size.                */
                        csr_lft   = M_DGSCL;
                        /* cursor vertical motion size.                  */
                        csr_rgt   = M_DGSCR;
                        /* display character number. (byte count)        */
                        str_len   = 28;
                        /* display character source pointer.(DBCS)       */
                        str_ptr   = trsmsg;

                        rc = _Mifmst( pt,msg_typ,csr_col,csr_row,csr_lft
                                                ,csr_rgt,str_len,str_ptr );
                        /* rc = IMSUCC:success,IMTRUSTW:IMIVMSGE:error  */

                    }

                /* 1.6
                 *      Trace start process continue return.
                 */

                /* nextact:next function entry bit set on.(to _Mdagst)  */
                kjsvpt->nextact = (kjsvpt->nextact | M_DAGSON);

                /* nextact:next function entry bit reset.(to _Mdagmsg)  */
                kjsvpt->nextact = (kjsvpt->nextact & ~M_DAGMON);

/*************************************************************************
 *      debug process routine.                                           */
snap3(SNAP_KCB  ,SNAP_Mdagend,"END 1.6 KMDAGEND");

                return( IMDGVK );

            }


        if (pt->code == P_CURD)
            /*  1.7
             *     Trace stop process.
             */
            {
                if (pt->trace != K_TALL)
                    {
                        /* 1.7.0.1 trace not start                      */

                        /* nextact bit reset and error return.          */
                        kjsvpt->nextact = kjsvpt->nextact & ~M_DAGMON;
/*************************************************************************
 *      debug process routine.                                           */
snap3(SNAP_KCB  ,SNAP_Mdagend,"ERROR 1.7.0.1 KMDAGEND");

                        return( IMDGIVK );
                    }

                /* 1.7.1 confirmation massage output for trace end.     */
                if ( (pt->maxa1c >= M_MSSLNC) &&
                     (pt->maxa1r >= M_MSSLNR) && (pt->axuse1 == K_ANOUSE) )
                    {
                        /* put out the massage to aid area on display.  */

                        /* the message type for it display.             */
                        msg_typ   = K_MSGOTH;
                        /* the aid area number for it stored.(=1)       */
                        area_num  = C_AUX1;
                        /* width length.                                */
                         scr_col   = M_DGMECL;
                        /* line number.                                 */
                        scr_row   = M_DGMERL;
                        /* cursor horizontal position.                  */
                        csr_col   = M_DGMECX;
                        /* cursor vertical position.                    */
                        csr_row   = M_DGMECY;
                        /* cursor horizontal motiom size.               */
                        csr_lft   = M_DGECL;
                        /* cursor vertical motion size.                 */
                        csr_rgt   = M_DGECR;
                        /* display character number. (byte count)       */
                        str_len   = 28;
                        /* display character source pointer.(DBCS)      */
                        str_ptr   = tremsg;   /* trace end massage.     */

                        rc = _Maxmst(pt,msg_typ,area_num,scr_col,scr_row,
                        csr_col,csr_row,csr_lft,csr_rgt,str_len,str_ptr );
                        /* rc = IMSUCC:success,IMTRUSTW:IMIVMSGE:error  */

                    }

                else

                    {
                        /*  1.7.3
                         * put out the massage to input area on display. */

                        /* the message type for it display.              */
                        msg_typ   = K_MSGOTH;
                        /* width length.                                 */
                        csr_col   = M_DGMECX;
                        /* line number.                                  */
                        csr_row   = M_DGMECY;
                        /* cursor horizontal motiom size.                */
                        csr_lft   = M_DGECL;
                        /* cursor vertical motion size.                  */
                        csr_rgt   = M_DGECR;
                        /* display character number. (byte count)        */
                        str_len   = 28;
                        /* display character source pointer.(DBCS)       */
                        str_ptr   = tremsg;

                        rc = _Mifmst( pt,msg_typ,csr_col,csr_row,csr_lft
                                                ,csr_rgt,str_len,str_ptr );
                        /* rc = IMSUCC:success,IMTRUSTW:IMIVMSGE:error  */

                    }

                /* 1.8
                 *      Trace end process continue return.
                 */

                /* nextact : next entry bit set on. ( to _Mdagend)      */
                kjsvpt->nextact = (kjsvpt->nextact | M_DAGEON);

                /* nextact : next entry bit reset. ( to _Mdagmsg)       */
                kjsvpt->nextact = (kjsvpt->nextact & ~M_DAGMON);

/*************************************************************************
 *      debug process routine.                                           */
snap3(SNAP_KCB  ,SNAP_Mdagend,"END 1.8 KMDAGEND");

                return( IMDGVK );

            }
        /*      1.4.2
         *
         */

        /* Key input is not P_CURU and not P_CURD.                      */
        /* nextact : next entry bit off. ( to _Mdagmsg)                 */
        kjsvpt->nextact = (kjsvpt->nextact & ~M_DAGMON);
/*************************************************************************
 *      debug process routine.                                           */
snap3(SNAP_KCB  ,SNAP_Mdagend,"ERROR 1.4.2 KMDAGEND");

        return( IMDGIVK );
}

