static char sccsid[] = "@(#)23	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mkjgrst.c, libKJI, bos411, 9428A410j 7/23/92 03:23:32";
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
 * MODULE NAME:         _Mkjgrst
 *
 * DESCRIPTIVE NAME:    Kanji Internal Save Area Data Set Kana/Kanji
 *                      Conversion Block.
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
 * FUNCTION:            1. Currnet Conversion Data Set Kana/Kanji Conversion
 *                         Block.
 *                      2. First Conversion Data Set Kana/Kanji Conversion
 *                         Block,
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1868 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mkjgrst
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mkjgrst( pt,mode )
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *                      mode    :Where Data Set Kana/Kanji Control
 *                               Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      IMSUCC  :Successful of Execution.
 *
 * EXIT-ERROR:          NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Internal Subroutines.
 *                              NA.
 *                      Kanji Project Subroutines.
 *                              _Mkjgrs1:Currnet Convetion Map/Kanji
 *                                       Conversion Block.
 *                              _Mkjgrs2:First Convetion Map Set Kana/Kanji
 *                                       Conversion Block.
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
 *                              kjsvpt
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              cconvlen cconvpos conversn convpos
 *                              gramap1s grammap  kjcvmap  kjdata1s
 *                              kjmap1s  kkcbsvpt
 *                      Trace Block(TRB).
 *                              NA.
 *                      Kana Kanji Control Block(KKCB).
 *                              NA.
 *
 *   OUTPUT:            Kanji Monitor Control Block(KCB).
 *                              NA.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              NA.
 *                      Trace Block(TRB).
 *                              NA.
 *                      Kana Kanji Control Block(KKCB).
 *                              convmode grmapln  grammap  kjdata
 *                              kjlen    kjmap    kjmapln
 *
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              CHPTTOSH:Short Data Set Character Pointer.
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

static void    _Mkjgrs1();/* Current Conversion Data Set KKCB.  */
static void    _Mkjgrs2();/* Save Data Set KKCB.                */

/*
 *      Kanji Conversion Map Set KKCB Interface Variagble.
 */
int     _Mkjgrst( pt,mode )

register KCB *pt;       /* Pointer to Kanji Control Block(KCB).         */
short   mode;           /* KKCB Set Value Select.                       */
{

        register KMISA *kjsvpt; /* Pointer to KMISA.                    */

        int     ret_code;       /* Return Code.                         */

        register int loop;      /* Loop Counter.                        */
        int     phranum;        /* Number of Phrase.                    */

        /*
         *      Debugging Output.
         */
        snap3(SNAP_KMISA|SNAP_KKCB|SNAP_KCB,SNAP_Mkjgrst,"Start");

        /*
         ****************************************************************
         *      0. Set Up General Work Pointer.
         ****************************************************************
         */
        /*
         *      Work Pointer & Variable Initialize.
         */
        kjsvpt  = pt->kjsvpt;   /* Get Pointer to KMISA.                */
        ret_code= IMSUCC;

        /*
         ****************************************************************
         *      1. Select Specfied Function.
         ****************************************************************
         */
        /*
         *      Kanji Map Phrase Number Count before Current Conversion
         *      Position.
         */
        phranum = 0;

        for( loop = 0
            ;loop < (kjsvpt->cconvpos - kjsvpt->convpos )
            ;loop += C_DBCS ) {

                /*
                 *  Continuous or Adjunct Word Check.
                 */
                switch( kjsvpt->kjcvmap[loop] ) {
                case M_KJMNCV:  /* No Conversion Data.                  */
                case M_KJMCTN:  /* Continuous Data.                     */
                case M_KJMJAN:  /* Adjnuct Data.                        */
                        continue;
                };

                /*
                 *  Count Grammer Number.
                 */
                phranum++;
        };

        /*
         *      KMISA Interface Varaible for KKCB which
         *      Variable Set KKCB.
         */
        switch( mode ) {
        case    M_CCDATA:
        /*
         *      Current Conversion Data Set KKCB.
         */
                (void)_Mkjgrs1( pt ,phranum );
                break;
        case    M_1SDATA:
        /*
         *      First Conversion Data Set KKCB.
         */
                (void)_Mkjgrs2( pt ,phranum );
                break;
        default:
        /*
         *      Unknown Keyward Return to Main.
         */
                break;
        };

        /*
         *      Debugging Output.
         */
        snap3(SNAP_KMISA|SNAP_KKCB|SNAP_KCB,SNAP_Mkjgrst,"End  ");

        /*
         ****************************************************************
         *      3. Return to Caller.
         ****************************************************************
         */
        return( ret_code );
}

/*
 *      Current Conversion Data for KKCB Interface
 *      Set KKCB.
 */
static void _Mkjgrs1( pt,phranum )

register KCB *pt;       /* Pointer to Kanji Control Block(KCB).         */
int     phranum;        /* Number of Phrase Before Current Conversion   */
                        /* Position.                                    */
{
        register KMISA *kjsvpt; /* Pointer to KMISA.                    */
        register KKCB  *kkcbsvpt;
                                /* Pointer to KCCB.                     */

        register int loop;      /* Loop Counter.                        */
        int     kjcvpos;        /* Kanji Conversion Map Postion.        */
        int     grampos;        /* Current Grammer Map Position.        */
        int     grmapln;        /* Grammer Map Length.                  */

        /*
         ****************************************************************
         *      1-1. 0. Set Up General Work Pointer.
         ****************************************************************
         */
        /*
         *      Work Pointer & Variable Initialize.
         */
        kjsvpt  = pt->kjsvpt;   /* Get Pointer to KMISA.                */
        kkcbsvpt= kjsvpt->kkcbsvpt;
                                /* Get Pointer to KKCB.                 */

        /*
         ****************************************************************
         *      1-1. 1. Set Kana/Kanji Conversion Mode.
         ****************************************************************
         */
        /*
         *      KMISA kjsvpt Real position  get.
         */
        kjcvpos = kjsvpt->cconvpos - kjsvpt->convpos;

        /*
         *      KKCB Convesrion Mode Set.
         */
        switch( kjsvpt->kjcvmap[kjcvpos] ) {
        /*
         *      DBCS Numeric Code Conversion.
         */
        case M_KJMKNM:
                kkcbsvpt->convmode = K_KANCON;
                break;
        /*
         *      Alpahnumeric Conversion.
         */
        case M_KJMALN:
                kkcbsvpt->convmode = K_ALPCON;
                break;
        /*
         *      Abbrivate Conversion.
         */
        case M_KJMABB:
                kkcbsvpt->convmode = K_RYACON;
                break;
        /*
         *      Conversion Mode Depend on Profile.
         */
        default:
                kkcbsvpt->convmode = kjsvpt->kmpf[0].conversn;
                break;
        };

        /*
         ****************************************************************
         *      1-1. 2. Kanji Control Block Data Set to Kana/Kanji
         *      Control Block.
         *      KCB: string ----> KKCB kjdata,kjlen
         ****************************************************************
         */
        /*
         *      KMISA Kanji Conversion Map Set KKCB.
         */
        (void)memcpy( (char *)&kkcbsvpt->kjdata[2],
                      (char *)&pt->string[kjsvpt->cconvpos],
                      kjsvpt->cconvlen);
        /*
         *      Set Length of KKCB Kanji Conversion Map.
         */
        kkcbsvpt->kjlen = kjsvpt->cconvlen + 2;
        SHTOCHPT( kkcbsvpt->kjdata,kkcbsvpt->kjlen );

        /*
         ****************************************************************
         *      1-1. 3. Kanji Monitor Internal Save Area Data Set to
         *      Kana/Kanji Control Block.
         *      KMISA: kjcvmap ---> KKCB kjmap,kjmapln.
         ****************************************************************
         */
        /*
         *      Set KMISA Kanji Conversion Map Map Data to
         *      KKCB Kanji Conversion Map.
         */
        kkcbsvpt->kjmap[2] = kjsvpt->kjcvmap[kjcvpos];

        /*
         *      KKCB Kanji Conversion Map Length Set.
         */
        kkcbsvpt->kjmapln = 1 + 2;
        SHTOCHPT( kkcbsvpt->kjmap,kkcbsvpt->kjmapln );

        /*
         ****************************************************************
         *      1-1. 4. Kanji Monitor Internal Save Area Data Set to
         *      Kana/Kanji Control Block.
         *      KMISA: grammap ---> KKCB grammap,grmpln.
         ****************************************************************
         */
        /*
         *      Get Position of Current Conversion Grammer Map.
         */
        grampos = 1;
        for( loop = 1; loop <=phranum ; loop++ ) {
                /*
                 *      Grammer Map Data Length Check.
                 */
                if( kjsvpt->grammap[grampos] & (1<<(C_BITBYT-1)) )
                        grampos += 2;
                else
                        grampos++;
        };

        /*
         *      Get Length of Current Conversion Grammer Map.
         */
        if( kjsvpt->grammap[ grampos ] & (1<<(C_BITBYT-1)) )
                grmapln = 2;
        else
                grmapln = 1;

        /*
         *      Set KMISA Current Conversion Position Grammer to
         *      KKCB Grammer Map.
         */
        (void)memcpy( (char *)&kkcbsvpt->grammap[1],
                      (char *)&kjsvpt->grammap[grampos],
                      grmapln);
        /*
         *      Set Length of KKCB Grammer Map.
         */
        kkcbsvpt->grammap[0] = grmapln + 1;
        kkcbsvpt->grmapln = grmapln + 1;
        /*
         ****************************************************************
         *      1-1. 5. Return to Main.
         ****************************************************************
         */
        return;
}

/*
 *      All Candidate Mode Allways Set First Conversion
 *      Kanji Conversion Status Set KKCB,so
 *      this Routine First Conversion map in KMISA Set
 *      KKCB Effective Member.
 */
static void _Mkjgrs2( pt,phranum )

register KCB *pt;       /* Pointer to Kanji Control Block(KCB).         */
int     phranum;        /* Phrase Number of Before Current Conversion   */
                        /* Position.                                    */
{
        register KMISA *kjsvpt; /* Pointer to KMISA.                    */
        register KKCB  *kkcbsvpt;
                                /* Pointer to KKCB.                     */

        register int loop;      /* Loop Counter.                        */
        int     loop2;          /* Loop Counter.                        */
        int     loopend;        /* Loop End.                            */
        int     kjmppos;        /* Kanji Conversion Map                 */
        int     kjmplen;        /* Knaji Map Length.                    */
        int     kjcvpos;        /* Kanji Conversion Map Position.       */
        int     grampos;        /* Grammer Map Position.                */
        int     cphranum;       /* Conversion Phrase Number.            */
        int     grmapln;        /* Grammer Map length.                  */

        /*
         ****************************************************************
         *      1-2. 0. Set Up General Work Pointer.
         ****************************************************************
         */
        /*
         *      Work Pointer & Variable Initialize.
         */
        kjsvpt  = pt->kjsvpt;   /* Get Pointer to KMISA.                */
        kkcbsvpt= kjsvpt->kkcbsvpt;
                                /* Get Pointer to KKCB.                 */

        /*
         ****************************************************************
         *      1-2. 1. Set Kana/Kanji Conversion Mode.
         ****************************************************************
         */
        /*
         *      Phrase Number Counter Set.
         */
        loop2  = 0;

        /*
         *      KMISA kjmap1s Real position  get.
         */
        loopend = CHPTTOSH( kjsvpt->kjmap1s ) -2;

        /*
         *      Phrase String Positin Init.
         */
        kjmppos = 0;

        /*
         *      Skip Specified Number of Phrase Word &
         *      Which Position Get.
         *                               Get This Position.
         *                               V
         *      +-------------------------------+
         *      |Phrase |Phrase |Phrase |Phrase |
         *      +-------------------------------+
         *         |               |
         *         +---------------+
         *           Word Number
         *
         */
        for( loop=0 ; loop<loopend ; loop++) {

                /*
                 *      Skip Non Availabele Data.
                 */
                switch( kjsvpt->kjmap1s[2+loop] ) {
                case M_KJMCTN:  /* Continuous Data.     */
                case M_KJMJAN:  /* Adjnuct Datg.        */
                        continue;
                };

                /*
                 *      Phrase Counter Increment.
                 */
                loop2++;

                /*
                 *      Phrase Counter End?
                 */
                if( loop2 > phranum ) {
                        /*
                         *      Current Phrase Start Position Set.
                         */
                        kjmppos = loop;
                        break;
                };

        };

        /*
         *      KKCB Convesrion Mode Set.
         */
        switch( kjsvpt->kjmap1s[2+kjmppos] ) {
        /*
         *      DBCS Numeric Code Conversion.
          */
        case M_KJMKNM:
                kkcbsvpt->convmode = K_KANCON;
                break;
        /*
         *      Alphanumeric Conversion.
         */
        case M_KJMALN:
                kkcbsvpt->convmode = K_ALPCON;
                break;
        /*
         *      Abbriviate Conversion.
         */
        case M_KJMABB:
                kkcbsvpt->convmode = K_RYACON;
                break;
        /*
         *      Depend On Profile Initial Variable.
         */
        default:
                kkcbsvpt->convmode = kjsvpt->kmpf[0].conversn;
                break;
        };

        /*
         *      KMISA First Conversion data Set KKCB Effect member.
         */
        kjcvpos = kjsvpt->cconvpos - kjsvpt->convpos;
        loopend = kjsvpt->cconvlen + kjcvpos;

        /*
         *      Counter Gremmer Number Before Current Conversion
         *      Position.
         */
        cphranum = 0;
        for( loop = kjcvpos ; loop < loopend ; loop += C_DBCS ) {

                /*
                 *  Continuous or Adjunct Check.
                 */
                switch( kjsvpt->kjcvmap[loop]) {
                case M_KJMNCV:  /* No Conversion Data.  */
                case M_KJMCTN:  /* Contiuous Data.      */
                case M_KJMJAN:  /* Adjnuct Data.        */
                        continue;
                };

                /*
                 *  Count Grammer Number.
                 */
                cphranum++;
        };

        /*
         *      Get Length of Previous Kanji Map.
         */
        loopend = CHPTTOSH( kjsvpt->kjmap1s ) - 2;

        /*
         *      Current Conversion Phrase Length.
         */
        kjmplen = 0;

        /*
         *      Counter for Phrase Number.
         */
        loop2   = 0;

        /*
         *      Previous Conversion Kanji Map Search for Specified
         *      Phrase Count ,Phrase Position.
         *
         *
         *      I want to length(Bytes) From A to B.
         *
         *      A.                     B.
         *      V                      V
         *      +-------------------------------+
         *      |Phrase |Phrase |Phrase |Phrase |
         *      +-------------------------------+
         *         |               |
         *         +---------------+
         *           Grammer Number
         *
         */
        for( loop = kjmppos ; loop < loopend ; loop++ ) {

                /*
                 *      One Phrase Last Position Get.
                 */
                switch( kjsvpt->kjmap1s[2+loop] ) {
                case M_KJMCTN:  /* Continuous Data.     */
                case M_KJMJAN:  /* Adjunct Data.        */
                        /*
                         *      Phrase Length Increment.
                         */
                        kjmplen++;
                        continue;
                };

                /*
                 *      Increment Phrase Count.
                 */
                loop2++;

                /*
                 *      Last Phrase Character?
                 */
                if( loop2 > cphranum )
                        break;

                /*
                 *      Phrase Length Increment.
                 */
                kjmplen++;

        };

        /*
         ****************************************************************
         *      1-2. 2. Kanji Monitor Internal Save Area Data  Set to
         *      Kana/Kanji Control Block.
         *      KMISA: kjdata1s ----> KKCB kjdata,kjlen
         ****************************************************************
         */
        /*
         *      Set First Conversion Data Set KKCB.
         */
        (void)memcpy( (char *)&kkcbsvpt->kjdata[2],
                      (char *)&kjsvpt->kjdata1s[2+kjmppos*C_DBCS],
                      kjmplen*C_DBCS);

        /*
         *      Set KKCB Kanji Data Length.
         */
        kkcbsvpt->kjlen = kjmplen*C_DBCS + 2;
        SHTOCHPT( kkcbsvpt->kjdata,kkcbsvpt->kjlen );

        /*
         ****************************************************************
         *      1-2. 3. Kanji Monitor Internal Save Area Data Set to
         *      Kana/Kanji Control Block.
         *      KMISA: kjcvmap ---> KKCB kjmap,kjmapln.
         ****************************************************************
         */
        /*
         *      Set First Conversion Map Length Set KKCB.
         */
        kkcbsvpt->kjmap[2] = kjsvpt->kjmap1s[2+kjmppos];
        kkcbsvpt->kjmapln = 2 + 1;
        SHTOCHPT( kkcbsvpt->kjmap,kkcbsvpt->kjmapln );

        /*
         ****************************************************************
         *      1-2. 4. Kanji Monitor Internal Save Area Data Set to
         *      Kana/Kanji Control Block.
         *      KMISA: grammap ---> KKCB grammap,grmpln.
         ****************************************************************
         */
        /*
         *      Counter Grammer Number Until Current Conversion
         *      Position from First Conversion Grammer Map.
         */
        grampos = 1;
        for( loop = 1; loop <=phranum ; loop++ ) {
                /*
                 *      Grammer Data Length Check.
                 */
                if( kjsvpt->gramap1s[grampos] & (1<<(C_BITBYT-1)) )
                        grampos += 2;
                else
                        grampos++;
        };

        /*
         *      Grammer Data Length Get.
         */
        if( kjsvpt->gramap1s[grampos ] & (1<<(C_BITBYT-1)) )
                grmapln = 2;
        else
                grmapln = 1;

        /*
         *      Set First Conversion Grammer Map Set KKCB.
         */
        (void)memcpy( (char *)&kkcbsvpt->grammap[1],
                      (char *)&kjsvpt->gramap1s[ grampos ],
                      grmapln);

        /*
         *      Set KKCB Grammer Map Length.
         */
        kkcbsvpt->grammap[0] = grmapln + 1;
        kkcbsvpt->grmapln = grmapln + 1;

        /*
         ****************************************************************
         *      1-2. 5. Return to Main.
         ****************************************************************
         */
        return;
}


