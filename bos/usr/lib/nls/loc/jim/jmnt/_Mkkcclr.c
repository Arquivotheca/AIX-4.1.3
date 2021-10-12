static char sccsid[] = "@(#)24	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mkkcclr.c, libKJI, bos411, 9428A410j 7/23/92 03:23:37";
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
 * MODULE NAME:         _Mkkcclr
 *
 * DESCRIPTIVE NAME:    Reset KMISA variables which are related to KKC.
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
 * FUNCTION:           1. Reset KMISA variables (related to RKC).
 *                     2. Reset KMISA variables (related to all candidates).
 *                     3. Reset nextact process & nextact flag.
 *                     4. Reset KMISA variables (related to KKC flag).
 *                     5. Reset KMISA variables (related to KKC data area).
 *                     6. Set KMISA mode flag.
 *
 * NOTES:              NA.
 *
 *
 *
 * MODULE TYPE:        Procedure
 *
 *  PROCESSOR:         C
 *
 *  MODULE SIZE:        1400 Decimal Bytes.
 *
 *  ATTRIBUTE:         Reentrant
 *
 *  ENTRY POINT:       Module Entry Point Name
 *
 *  PURPOSE:           See Function.
 *
 *  LINKAGE:           _Mkkcclr( pt )
 *
 *  INPUT:             pt :Pointer to Kanji Monitor Control Block.
 *
 *  OUTPUT:            NA
 *
 *  EXIT-NORMAL:       IMSUCC: Successfull.
 *
 *  EXIT-ERROR:        NA
 *
 *  EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:          Internal Subroutines.
 *                     NA.
 *
 *                     Kanji Project Subroutines.
 *                      _Mreset() : Ordinary reset routine.
 *                      _MCN_rs() : No conversion indicator reset routine.
 *                      _Mflyrst(): Reset Flying Conversion Area.
 *
 *                     Standard Liblay.
 *                      memset():Set memory with specified # character.
 *
 *                     Advanced Display Graphics Support Library(GSL).
 *                     NA.
 *
 *  DATA AREAS:        NA.
 *
 *  CONTROL BLOCK:     See Below.
 *
 *   INPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                     NA.
 *
 *                     Extended Information Block(EXT).
 *                     NA.
 *
 *                     Kanji Monitor Control Block(KCB).
 *                     kjsvpt  : Pointer to KMISA.
 *
 *                     Kanji Monitor Internal Save Area(KMISA,FSB).
 *                     rkclen  : RKC Output Length.
 *                     kkmode1 : Current Kanji Monitor Mode.
 *                     nextact : Flag for Next Action Trriger.
 *                     kjcvmax : Length of Kanji Convertion Map.
 *                     savemax : Length of Bacground Save Area.
 *                     kanabmax: Length of Kana Map.
 *                     grammax : Length of Grammer Map.
 *                     kanamax : Length of Kana Data.
 *                     dat1smax: Length of First Convertion Kanji Data.
 *                     map1smax: Length of First Convertion Kanji Map.
 *                     gra1smax: Length of First Convertion Grammer.
 *                     iws1max : Length of Internal Work Area No.1
 *                     iws2max : Length of Internal Work Area No.2
 *                     iws3max : Length of Internal Work Area No.3
 *                     iws4max : Length of Internal Work Area No.4
 *                     Trace Block(TRB).
 *                     NA.
 *
 *   OUTPUT:           DBCS Editor Control Block(DECB,DECB_FLD).
 *                     NA.
 *
 *                     Extended Information Block(EXT).
 *                     NA.
 *
 *                     Kanji Monitor Control Block(KCB).
 *                     NA.
 *
 *                     Kanji Monitor Internal Save Area(KMISA,FSB).
 *
 *
 *                     rkclen    : RKC Output Length.
 *                     rkcchar   : RKC Output Character.
 *                     nextact   : Flag for Next Action Trriger.
 *                     kkmode1   : Current Kanji Monitor Mode.
 *                     convpos   : Effective KKC Start Position.
 *                     convlen   : Effective Length for KKC Conversion.
 *                     cconvpos  : Current KKC Position.
 *                     cconvlen  : Current KKC Lenth.
 *                     savepos   : Current Save Position for Edit Mode.
 *                     savelen   : Current Save Length for Edit Position.
 *                     kanalen   : Kana Length.
 *                     kkcflag   : KKC Mode Flag.
 *                     preccpos  : Previous Conversion Position.
 *                     convimp   : Conversion Impossible Flag.
 *                     kkcrmode  : KKC Mode flag. ( Registration.)
 *                     kjcvmap   : Map for Effective Kanji Conversion range.
 *                     stringsv  : Save Area Pointer for Hidden Character.
 *                     kanamap   : Map for kana.
 *                     grammap   : Map for grammer.
 *                     kanadata  : Kana Data.
 *                     kjdata1s  : Kanji Data. (1st conversion.)
 *                     gramap1s  : Map for grammer ( 1st conversion )
 *                     iws1 - 4  : Work Area.
 *
 *                      ( Variables for All candidates.)
 *                     allcancol : Required number of columns.
 *                     allcanrow : Required number of rows.
 *                     allcstge  : Save Area for Ordinary candidates stages.
 *                     allcstgs  : Save Area for
 *                                  Single Kanji candidates stages.
 *                     alcnmdfg  : All Candidates Mode Flag.
 *
 *                      ( Variables for Flying Conversion.)
 *                     fconvflg  : Flying Conversion flag.
 *
 *                     Trace Block(TRB).
 *                     NA.
 *
 * TABLES:              Table Defines.
 *                      NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              IDENTIFY:Module Identify Create.
 *                              SHTOCHPT:Change short to char. pointer.
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
#include <memory.h>     /* memory header                                */

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

int _Mkkcclr(pt)

KCB    *pt;       /* Pointer to Kanji Control Block  */

 {
        int     ret_code;   /* Return Code.                               */
        KMISA   *kjsvpt;    /* Pointer to KMISA                           */
        char    *memset();  /* Memory Set Function                        */
        int     _Mreset();  /* Key Board Reset subroutine                 */
        int     _MCN_rs();  /* None Conversion indicator reset subroutine */
        int     _Mstlcvl(); /* Decide Conversion.                         */
        int     _Mflyrst(); /* Reset Flying Conversion Area.              */
        int     i,j;        /* Loop Counter                               */

        /*
         *    1. Set Default Return Code.
         */

        ret_code = IMSUCC;

        /*
         *    2. Set KMISA pointer.
         */

        kjsvpt = pt->kjsvpt;

        /*
         *    3. Reset RKC variables in KMISA.
         */

        if ( kjsvpt->rkclen > 0 ){
        kjsvpt->rkclen = 0;
        memset(kjsvpt->rkcchar,J_NULL,sizeof(kjsvpt->rkcchar));
        };

        /*
         *    4. All Candidates Mode Reset.
         */

        if (kjsvpt->kkmode1 == A_ALCADM){

         kjsvpt->alcancol = 0;
         kjsvpt->alcanrow = 0;
         kjsvpt->alcnmdfg = 0;

         for (i = 0 ; i < sizeof(kjsvpt->allcstge) / 2 ; i++){
         kjsvpt->allcstge[i]=0;
         };
         for (j = 0 ; j < sizeof(kjsvpt->allcstgs) / 2 ; j++){
         kjsvpt->allcstgs[j]=0;
         };
        };

        /*
         *    5. Nextact Processes  Reset.
         */

        if (kjsvpt->nextact & M_KLRSON) {
          _Mreset(pt,M_KLRST);
        };

        if (kjsvpt->nextact & M_CNRSON){
          _MCN_rs(pt);
        };

        if (kjsvpt->msetflg & K_MSGOTH){
          _Mreset(pt,M_MSGRST);
        };

        if (kjsvpt->msetflg & K_MSGDIC){
          _Mreset(pt,M_RGRST);
        };

        if (kjsvpt->nextact > 0){
        kjsvpt->nextact = 0;
        };
                if ((pt->cnvsts == K_CONVGO) && (kjsvpt->convlen > 0))
                   {
                        (void)_Mstlcvl( pt, kjsvpt->convpos +
                                            kjsvpt->convlen );

                   };

        /*
         * 6. KKC Control Block Parameter Reset.
         */

        kjsvpt->convpos = 0;
        kjsvpt->convlen = 0;
        kjsvpt->cconvpos = 0;
        kjsvpt->cconvlen = 0;
        kjsvpt->savepos = 0;
        kjsvpt->savelen = 0;
        kjsvpt->kanalen = 0;
        kjsvpt->kkcflag = C_SWOFF;
        kjsvpt->preccpos = 0;



/* #(B) 1987.12.08. Flying Conversion Add */
        /*
         *      Flying Conversion Area Reset.
         */
        _Mflyrst( pt );
        kjsvpt->fconvflg = 0;
/* #(E) 1987.12.08. Flying Conversion Add */


        memset(kjsvpt->kjcvmap,J_NULL,kjsvpt->kjcvmax);

        memset(kjsvpt->stringsv,J_NULL,kjsvpt->savemax);

        memset(kjsvpt->kanamap,J_NULL,kjsvpt->kanabmax);
        kjsvpt->kanamap[0] = 0x01;

        memset(kjsvpt->grammap,J_NULL,kjsvpt->grammax);
        kjsvpt->grammap[0] = 0x01;

        memset(kjsvpt->kanadata,J_NULL,kjsvpt->kanamax);

        memset(kjsvpt->kjdata1s, J_NULL ,kjsvpt->dat1smax);
        SHTOCHPT(kjsvpt->kjdata1s,2);

        memset(kjsvpt->kjmap1s, J_NULL , kjsvpt->map1smax);
        SHTOCHPT(kjsvpt->kjmap1s,2);

        memset(kjsvpt->gramap1s,J_NULL,kjsvpt->gra1smax);
        kjsvpt->gramap1s[0] = 0x01;

        memset(kjsvpt->iws1,J_NULL,kjsvpt->iws1max);

        memset(kjsvpt->iws2,J_NULL,kjsvpt->iws2max);

        memset(kjsvpt->iws3,J_NULL,kjsvpt->iws3max);

        memset(kjsvpt->iws4,J_NULL,kjsvpt->iws4max);

        /*
         * 7. Mode Change Reset.
         */

        kjsvpt->kkmode1 = A_1STINP;
        kjsvpt->kkmode2 = A_1STINP;

        /*
         *      Return to Kanji Monitor.
         */

        return( ret_code );
}
