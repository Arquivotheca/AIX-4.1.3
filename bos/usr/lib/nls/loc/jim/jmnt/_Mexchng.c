static char sccsid[] = "@(#)01	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mexchng.c, libKJI, bos411, 9428A410j 7/23/92 03:21:58";
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
 * MODULE NAME:         _Mexchng
 *
 * DESCRIPTIVE NAME:    Replace "Destination String" to "Source String"
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
 * FUNCTION:            Destination String Data Replace Source String Data.
 *                      In Case 'Insert' Mode,Source String Insert Convert-
 *                      ion Position,in Case 'Replace' Mode,Source String
 *                      Data is Replace Destionation String,which is
 *                      shorter or longter, if 'shorter' then Hide Save
 *                      String Data was Restored,otherwise 'longer' then
 *                      Visible Data Area Saved In Hide Save Area.
 *                      ... if Replaec/Insert Operation Was overflowd
 *                      Input Field Length,as soon as stop conversion.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1760 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mexchng
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mexchng( pt,rstr_ptr,rpos,rlen,tpos,tlen )
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *                      rstr_ptr:Pointer to Reference String.
 *                               (Source String).
 *                      rpos    :Parameter 'rstr_ptr' data offset.
 *                      rlen    :Length of Reference String.
 *                               (Source String Length).
 *                      tpos    :Offset of Input Field.
 *                      tlen    :Target String Length.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      IMSUCC  :Success of Execution.
 *                      Other   :_Mlock(),_Mstlcvl() Return Code.
 *
 * EXIT-ERROR:          NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Internal Subroutines.
 *                              NA.
 *                      Kanji Project Subroutines.
 *                              _Mlock  :Lock Keyboard.
 *                              _Mstlcvl:Decide Conversion Data.
 *                              _Mstrl  :Get Input Field String.
 *                      Standard Library.
 *                              memcpy  :Copy Memory # Character(Byte).
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
 *                              lastch   repins   string
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              convlen  convpos  curright savepos
 *                              savelen  stringsv
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Control Block(KCB).
 *                              lastch   string
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              convlen  savelen  stringsv
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
#include <stdio.h>      /* Standar I/O Header.                          */
#include <memory.h>     /* Memory Operation.                            */

/*
 *      include Kanji Project.
 */
#include "kj.h"         /* Kanji Define File.                           */
#include "kcb.h"        /* Kanji Control Structure.                     */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-061 COPYRIGHT IBM CORP 1988           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Exchnge String.
 */
int     _Mexchng( pt,rstr_ptr,rpos,rlen,tpos,tlen )

register KCB *pt;       /* Pointer to Kanji Control Block(KCB).         */
register uchar *rstr_ptr;
                        /* Pointer to Referenced String.                */
short   rpos;           /* Referenced Position of String.               */
short   rlen;           /* Referenced Length of String.                 */
short   tpos;           /* Target Position of KCB String.               */
short   tlen;           /* Traget Length of KCB String.                 */
{
        int     _Mlock();       /* Keyboard Lock.                       */
        int     _Mstlcvl();     /* Decide Conversion.                   */
        int     _Mstrl();       /* Get Input Field Length.              */
        char    *memcpy();      /* Memory Copy Operation.               */
        short   exdifl;         /* Exchange Length Work Variable.       */

        int ret_code;           /* Return Value.                        */

        register uchar *string; /* Pointer to string(KCB).              */
        register uchar *stringsv;
                                /* Pointer to stringsv(KMISA).          */
        register KMISA *kjsvpt; /* Pointer to Kanji Monitor Internal    */
                                /* Save Area.                           */
        int     refmax;         /* Reference Max Position.              */
        int     tarmax;         /* Target Max Position.                 */
        int     convmax;        /* Conversion Max Position.             */
        int     savemax;        /* Save String Max Position.            */
        int     dstpos;         /* Move Destination Offset.             */
        int     movelen;        /* String Move Length.                  */
        int     nconvlen;       /* New Convlen.                         */
        int     nconvmax;       /* New Conversion Max Position.         */
        int     nsavelen;       /* New Save String Position.            */
        int     nlastch;        /* String Lastcharacter Position.       */
        int     nchlen;         /* Change String Length.                */
        char    woutflg;        /* String Overflow.                     */
        int     woutl;          /* Overflow Length.                     */
        int     ovflen;         /* Overflow Total Length.               */
        int     spacelen;       /* Last Character DBCS Length,          */
        register int loop;      /* Loop Work Variable.                  */

        /*
         *      Debugging Output.
         */
        snap3(SNAP_KCB|SNAP_KMISA,SNAP_Mexchng,"Start");

        /*
         ****************************************************************
         *      0. Set Up General Work Pointer.
         ****************************************************************
         */
        /*
         *      Initialize Work Pointer & Variable.
         */
        kjsvpt  = pt->kjsvpt;   /* Get Pointer to KMISA.                */
        string  = pt->string;   /* Get Pointer to Input Field.          */
        stringsv= kjsvpt->stringsv;/* Get Pointer to String Save Area.  */
        ret_code= IMSUCC;       /* Return Code Init.                    */

        /*
         ****************************************************************
         *      1. Initialize Work Variable Preprocess.
         ****************************************************************
         */
        /*
         *      Overflow Conversion Flag OFF.
         */
        woutflg = C_SWOFF;

        /*
         *      Overflow Total Length
         */
        ovflen  = 0;

        /*
         *      Input Field Replace 'Replace String' when
         *      it's string Last Positin Get.
         */
        refmax  = tpos + rlen;

        /*
         *      Input Field Previous 'String' when
         *      its' string Last Position Get.
         */
        tarmax  = tpos + tlen;

        /*
         *      Get Conversion Last Position On Input Field.
         */
        convmax = kjsvpt->convpos + kjsvpt->convlen;

        /*
         *      Get Background Save String Last Position.
         */
        savemax = kjsvpt->savepos + kjsvpt->savelen;

        /*
         *      Exchange Character Size Calculate.
         */
        exdifl   = rlen - tlen;

        /*
         *      Get Length of New Conversion String Length that
         *      Efficiency of Modify Length.
         */
        nconvlen = kjsvpt->convlen + exdifl;

        /*
         *      Get New Conversion Max Position On Input Field.
         */
        nconvmax = kjsvpt->convpos + nconvlen;

        /*
         *      New Save Length.
         */
        nsavelen = kjsvpt->savelen;

        /*
         ****************************************************************
         *      2. Replace/Insert Mode Function Check.
         ****************************************************************
         */
        /*
         *      Replace/Insert Processing.
         */
        switch( pt->repins ) {
        case K_INS:
                /*
                 ********************************************************
                 *      1-1. 1 Get Last Character Position &
                 *      Redraw Range.
                 ********************************************************
                 */
                /*
                 *      Last Character Position Update.
                 */
                nlastch  = pt->lastch + exdifl;
                /*
                 *      Check Last Character Position( Override Right
                 *      Margine).
                 */
                woutl = nlastch - kjsvpt->curright;
                if( woutl > 0 ) {
                        /*
                         *      Get Input Field Last DBCS Space Number.
                         */
                        spacelen   = _Mstrl( pt );
                        spacelen   = pt->lastch - MAX( spacelen,convmax );

                        /*
                         *      Get Not Useful DBCS Space Number.
                         */
                        woutl      = MIN( woutl,spacelen );

                        /*
                         *      Move Last Character Position.
                         */
                        pt->lastch -= woutl;
                        nlastch    -= woutl;
                };
                /*
                 *      Redraw String Length Get.
                 */
                if( tlen != rlen )
                        nchlen = MAX(pt->lastch,nlastch) - tpos;
                else
                        nchlen = tlen;

                /*
                 ********************************************************
                 *      1-1. 2. Check Override Right Margine.
                 ********************************************************
                 */
                /*
                 *      If Insert Replace Character Field is Overflow
                 *      then truncate String.
                 */
                if( nlastch > kjsvpt->curright ) {
                        /*
                         *      Overflow Length Truncate.
                         */
                        ovflen = woutl = nlastch - kjsvpt->curright;

                        /*
                         *      Last Position move Cursor Right
                         *      Margin.
                         */
                        nlastch -= woutl;

                        /*
                         *      Redraw Length Not Count out of
                         *      Right Margin.
                         */
                        nchlen  -= woutl;

                        /*
                         *      Conversion Last Position Overflow
                         *      Check & Truncate.
                         */
                        nconvlen -= woutl;
                        nconvmax -= woutl;

                        /*
                         *      Reference String Override Last Character
                         *      Position.
                         */
                        if( refmax > nlastch ) {
                                /*
                                 *      Reference String Truncate Length.
                                 */
                                woutl = refmax - nlastch;

                                /*
                                 *      Relace String Length Truncate.
                                 */
                                rlen   -= woutl;

                                /*
                                 *      Replace String Max Position
                                 *      Substract.
                                 */
                                refmax -= woutl;

                                /*
                                 *      Exchnage Difference Length
                                 *      Not Count out of Right Margin.
                                 */
                                exdifl  -= woutl;

                                /*
                                 *      Current Available Data Was
                                 *      Truncate.
                                 */
                                pt->lastch = tarmax;
                        } else {
                                /*
                                 *      Current Available Data Was
                                 *      Truncate.
                                 */
                                pt->lastch -= woutl;
                        };

                        /*
                         *      Overflow Converstion Flag ON.
                         */
                        woutflg  = C_SWON;
                };

                /*
                 ********************************************************
                 *      1-1. 3. Move Source String Left/Right/None.
                 ********************************************************
                 */
                /*
                 *      Move String.
                 *
                 *      exdifl <  0               exdifl > 0
                 *
                 *      +---------------+       +---------------+
                 *      |         ABCDEF|       |         ABCDEF|
                 *      +---------------+       +---------------+
                 *               /    /                         |
                 *      +---------------+       +---------------+
                 *      |       ABCDEF__|       |         ABABCD|EF
                 *      +---------------+       +---------------+
                 *                    ^                   ^      ^
                 *              Padding Space.            |      Trauncete.
                 *              (DBCS Space Code).        No Modify.
                 *
                 *      Above is exdifl = -2    Above is exdifl = 2
                 */
                /*
                 *      KCB Target String After Moved Position &
                 *      KCB Target String Available length Get From KCB
                 *      Target String Last Positin To Last Positon.
                 */

                /*
                 *      Get Replace String Last Position.
                 */
                dstpos  = tarmax     + exdifl;

                /*
                 *      Get Remain String Length from Last Position
                 *      to Source String Last Position.
                 */

                movelen = pt->lastch - tarmax;
                if( exdifl < 0 ) {

                        /*
                         *      DBCS String Shift Left.
                         */
                        (void)memcpy( (char *)&string[dstpos],
                                      (char *)&string[tarmax],
                                      movelen);

                        /*
                         *      Tail Padding DBCS Space Code.
                         */
                        for(  loop = dstpos + movelen
                             ;loop < pt->lastch
                             ;loop+= C_DBCS ) {
                                string[loop  ]= C_SPACEH;
                                string[loop+1]= C_SPACEL;
                        };
                } else if( exdifl > 0 ) {
                        /*
                         *      DBCS String Shift Right.
                         */
                        for(  loop = tarmax + movelen - 1
                             ;loop >= tarmax
                             ;loop-- )  {
                                string[loop + exdifl]
                                         = string[loop];
                        };
                };


                /*
                 ********************************************************
                 *      1-1. 4  Copy Destination String to Target String.
                 ********************************************************
                 */
                /*
                 *      Copy Reference String to Target String.
                 */
                (void)memcpy((char *)&string[tpos],
                             (char *)&rstr_ptr[rpos],
                            rlen);
                /*
                 ********************************************************
                 *      1-1. 5. Break Switch and Continue Process.
                 ********************************************************
                 */
                break;
        case K_REP:
                /*
                 ********************************************************
                 *      1-2. 1 Get Last Character Position &
                 *      Redraw Range.
                 ********************************************************
                 */
                /*
                 *      Replace String Override Last Position?
                 */
                nlastch = MAX( nconvmax , savemax );
                if( nlastch >= convmax )
                        nlastch = MAX( nlastch , pt->lastch );

                /*
                 *      Display Modify Range Set.
                 */
                if( tlen != rlen )
                        nchlen = MAX(convmax,nconvmax) - tpos;
                else
                        nchlen = tlen;

                /*
                 ********************************************************
                 *      1-2. 2  Move String.
                 ********************************************************
                 */
                /*
                 *      In Case Replace String Length shorter than
                 *      Target String Length.
                 */
                if( exdifl < 0 ) {
                        /*
                         *************************************************
                         *      1-2. 2.2 Target String Shift.
                         *************************************************
                         */
                        /*
                         *      Long String Shift Replace
                         *      Short String.
                         */
                        movelen = convmax - tarmax;
                        if( movelen > 0 ) {
                                (void)memcpy(
                                        (char *)&string[tarmax + exdifl],
                                        (char *)&string[tarmax],
                                        movelen);
                        };
                        /*
                         *************************************************
                         *      1-2. 2.3 Background Save String Area
                         *      Padding.
                         *************************************************
                         */
                        /*
                         *      Background Save String
                         *      does not Save then
                         *      Padding Kanji Space Code.
                         */
                        for(  loop = MAX(convmax + exdifl,savemax)
                             ;loop < convmax
                             ;loop+= C_DBCS ) {
                                 string[loop  ] = C_SPACEH;
                                 string[loop+1] = C_SPACEL;
                        };

                        /*
                         *************************************************
                         *      1-2. 2.3 Background Save String Area
                         *      Padding.
                         *************************************************
                         */
                        /*
                         *      Background Save String
                         *      was saved then
                         *      Display String Shorter Than
                         *      Before Display Size,
                         *      Restore Save String in
                         *      Foreground Display Area.
                         */
                        movelen = kjsvpt->savelen - nconvlen;
                        if( movelen > 0 ) {
                                (void)memcpy(
                                        (char *)&string[nconvmax],
                                        (char *)&stringsv[nconvlen],
                                        movelen);
                                nsavelen -= movelen;
                        };
                /*
                 *      In Case Replace String Length Greater Than
                 *      Target String.
                 */
                } else if( exdifl > 0 ) {
                        /*
                         *************************************************
                         *      1-3. 3.1 Check Override Right Margine
                         *************************************************
                         */
                        /*
                         *      Current String is shorter than
                         *      Replace String.
                         */
                        woutl = nlastch - kjsvpt->curright;
                        if( woutl > 0 ) {
                                /*
                                 *      Overflow Length Set.
                                 */
                                ovflen = woutl;

                                /*
                                 *      Replace Character Area Truncate.
                                 */
                                rlen    -= woutl;

                                /*
                                 *      Replace String Length Substract.
                                 */
                                refmax  -= woutl;

                                /*
                                 *      Exchange Differance Length Substract
                                 */
                                exdifl  -= woutl;

                                /*
                                 *      Regresh Range Move Left.
                                 */
                                nchlen  -= woutl;
                                /*
                                 *      Last Position Move Left.
                                 */
                                nlastch -= woutl;

                                /*
                                 *      Input Field Overflow Truncate.
                                 */
                                nconvmax -= woutl;
                                nconvlen -= woutl;

                                /*
                                 *      Overflow Decide.
                                 */
                                woutflg  = C_SWON;
                        };

                        /*
                         *************************************************
                         *      1-3. 3.2 If Replace Mode then Current
                         *      Replace Area String Save in Background
                         *      Save Area.
                         *************************************************
                         */
                        /*
                         *      Already Save KCB String Data.
                         */
                        if( nconvlen > kjsvpt->savelen ) {
                                /*
                                 *      Save String Data to
                                 *      KMISA String Save Area.
                                 */
                                movelen = pt->lastch - convmax;
                                if( movelen > 0 ) {
                                        movelen = MIN( movelen,
                                                nconvlen-kjsvpt->convlen);

                                        /*
                                         *      Current Display String
                                         *      Save Buckground Save Area.
                                         */
                                        (void)memcpy(
                                            (char *)&stringsv[kjsvpt->savelen],
                                            (char *)&string[convmax],
                                            movelen);

                                        /*
                                         *      Bacground Save Legnth
                                         *      Add.
                                         */
                                        nsavelen += movelen;
                                };
                        };

                        /*
                         *************************************************
                         *      1-3. 3.3 Current Display String Shift
                         *************************************************
                         */
                        /*
                         *      Current Conversion String Shift Right.
                         */
                        movelen = convmax - tarmax;
                        for(  loop = tarmax + movelen -1
                             ;loop >=tarmax
                             ;loop-- ) {
                                string[loop + exdifl] = string[loop];
                        };
                /*
                 *      In Case Replace String is equal to Target String.
                 */
                } else {
                        ;
                };
                /*
                 ********************************************************
                 *      1-1. 2.1  Copy Destination String to Target String.
                 *      1-2. 3.4        Same of Above.
                 ********************************************************
                 */
                 /*
                 *      Replace Reference String to Target String.
                 */
                (void)memcpy((char *)&string[tpos],
                             (char *)&rstr_ptr[rpos],
                             rlen);
                /*
                 ********************************************************
                 *      1-2. 2.5  Break Switch and Continue Process.
                 *      1-2. 3.5        Same of Above.
                 ********************************************************
                 */
                break;
        };

        /*
         ***************************************************************
         *      1-3. 1. Update Kanji Control Block Information.
         ***************************************************************
         */
        /*
         *      Set New Last Character Posotion.
         */
        pt->lastch       = nlastch;

        /*
         *      Conversion String Length Update.
         */
        kjsvpt->convlen += exdifl + ovflen;

        /*
         *      Conversion Position Last Posotion Update.
         */
        convmax         += exdifl + ovflen;

        /*
         *      Input Field Hide Display String Length Update.
         */
        kjsvpt->savelen  = nsavelen;

        /*
         *      String Refresh Range Sets.
         */
        (void)_Msetch( pt,tpos,nchlen );

        /*
         ***************************************************************
         *      1-3. 2. Overflow Convertin Processing.
         ***************************************************************
         */
        /*
         *      Overflow Conversion Backend Processing.
         */
        if( woutflg == C_SWON ) {
                /*
                 *      Overflow Conversion Occure.
                 */
                ret_code = _Mstlcvl( pt,kjsvpt->curright + C_ANK );

                /*
                 *      Keyboard Lock Indicator & Lock Keyboard.
                 */
                if( ret_code == IMSUCC )
                        ret_code = _Mlock( pt );
        };

        /*
         *      Debugging Output.
         */
        snap3(SNAP_KCB|SNAP_KMISA,SNAP_Mexchng,"End");

        /*
         ***************************************************************
         *      1-3. 3. Return to Caller.
         ***************************************************************
         */
        return( ret_code );
}
