static char sccsid[] = "@(#)34	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mnxprps.c, libKJI, bos411, 9428A410j 7/23/92 03:24:06";
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
 * MODULE NAME:         _Mnxprps
 *
 * DESCRIPTIVE NAME:    Kana/Kanji Control Block Data Copy to
 *                      Kanji Monitor Internal Save Area.
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
 * FUNCTION:            Kana/Kanji Control Block Data Copy to
 *                      Kanji Monitor Internal Save Area.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1452 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         Module Entry Point Name
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            Module( parm1,parm2,parm3 )
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      IMSUCC  :Success of Execution.
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
 *                              memcpy  :Copy # Character.
 *                              memset  :Set  # of Specified Character.
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
 *                              cconvlen cconvpos convlen  convpos
 *                              grammap  kanalen  kanamap  kjcvmap
 *                              kkcvsvpt
 *                      Trace Block(TRB).
 *                              NA.
 *                      Kana Kanji Control Block(KKCB).
 *                              grammap  grmapln  kanalen1 kanamap
 *                              kjmap    kjmapln
 *
 *   OUTPUT:            Kanji Monitor Control Block(KCB).
 *                              NA.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              grammap  gramapln  kjcvmap
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
 *                              MIN     :Which is Minimum Value.
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

int     _Mnxprps( pt )

register KCB *pt;       /* Pointer to Kanji Control Block(KCB).         */

{
        char    *memcpy();      /* Copy # of Character.                 */
        char    *memset();      /* Set  # of Specified Character.       */
        register KMISA *kjsvpt; /* Pointer to KMISA.                    */
        register KKCB  *kkcbsvpt;
                                /* Pointer to KMISA.                    */

        int     ret_code;       /* Return Code.                         */
        register int loop;      /* Loop Counter.                        */
                int workcnt;    /* Work Counter.                        */
        register uchar *kjcvmap;/* Kanji Convertion Map Address.        */
        int     kjcvpos;        /* Kanji Convertion MapWrite Position.  */
        int     convmax;        /* Kanji Convertion Map max Position.   */
        int     grampos;        /* Gramap  Write Position.              */
        int     phranum;        /* Number of Phrase.                    */
        int     adjnum;         /* Number of Adjunct.                   */
        int     movlen;         /* Number of Move Character.            */
        int     gramisa;        /* KMISA Grammer Map Word Length.       */
        int     gramkkcb;       /* KKCB  Grammer Map Word Length.       */
        int     rotbyt;         /* Rotete Byte Number Count.            */
        int     totwrd;         /* Total Word and Adjunct.              */
        int     kanabit;        /* KMISA Kana Map Word Bit Map Nummber. */
        uchar   *dstpos;        /* KMISA Kana Map Bit Map Byte Position.*/
        uchar   *srcpos;        /* KKCB  Kana Map Bit Map Byte Position.*/
        int     dstbit;         /* KMISA Kana Map Bit Map Bit Posotion. */
        int     srcbit;         /* KKCB  Kana Map Bit Map Bit Posotion. */
        int     actbit;         /* Bit Map Request Bit Number.          */
        int     rembit;         /* Bit Map Reamain Bits Number.         */
        uchar   srcpat;         /* KMISA Kana Map Bit Map Operation Work*/
        uchar   dstpat;         /* KMISA Kana Map Bit Map Operation Work*/
        uchar   wrkpat;         /* KMISA Kana Map Bit Map Operation Work*/

        /*
         *      Debugging Output.
         */
        snap3(SNAP_KMISA|SNAP_KKCB,SNAP_Mnxprps,"Start");

        /*
         *      Collection for Exit Point.
         */
        do {
                /*
                 *********************************************************
                 *      0. Set Up General Work Pointer & Variable.
                 *********************************************************
                 */
                /*
                 *      Work Pointer & Variable Initialize.
                 */
                kjsvpt  = pt->kjsvpt;           /* Get Pointer to KMISA.*/
                kkcbsvpt= kjsvpt->kkcbsvpt;     /* Get Pointer to KKCB. */
                ret_code= IMSUCC;               /* Set Default Return.  */

                /*
                 *********************************************************
                 *      1. Kana/Kanji Control Block Reflect to Kanji
                 *      Monitor Internal Save Area.
                 *      KKCB:Kanji Conversion Map
                 *                ---> KMISA:Kanji Conversion Map
                 *********************************************************
                 */
                /*
                 *      KKCB Data Not Available.
                 */
                if( kkcbsvpt->kjmapln <=  2 ) {
                        ret_code = IMKJCVLE;
                        break;
                };

                /*
                 *      KMISA Interface Varaible for KKCB which
                 *      modified by KKC call,so it's efficiency of
                 *      each Interface Vraibele that Reflect to KMISA
                 *      Interface Variable.
                 */

                /*
                 *      Count Phrase & Adujnct.
                 */
                phranum = 0;
                adjnum  = 0;
                kjcvpos = kjsvpt->cconvpos - kjsvpt->convpos;
                for( loop = 0
                    ;loop < kjcvpos
                    ;loop += C_DBCS ) {

                        /*
                         *  Continuous Adjunct Word Check.
                         */
                        switch( kjsvpt->kjcvmap[loop] ){
                        case M_KJMNCV:  /* No Convetion Data.   */
                        case M_KJMCTN:  /* Continuous Data.     */
                                continue;
                        case M_KJMJAN:  /* Adjnuct Data.        */
                                adjnum++;
                                continue;
                        };

                        /*
                         *  Count Phrase Number.
                         */
                        phranum++;
                };

                /*
                 *      KKCB Kanji Map Reflect to
                 *      KMISA Kanji Conversion Map
                 */
                /*
                 *      KMISA Knaji Conversion Map Position Get.
                 */
                kjcvmap = &kjsvpt->kjcvmap[kjcvpos];

                /*
                 *      Current Conversion Data Max Position.(KMISA)
                 */
                convmax = kjsvpt->convlen - kjcvpos;

                /*
                 *      Get Diffrent Number of Effect Length.
                 */
                movlen  =    (kkcbsvpt->kjmapln - 2)*C_DBCS
                           - kjsvpt->cconvlen;

                /*
                 *      Check Maximum Length of Kanji Conversion
                 *      Map.
                 */
                if( (kjsvpt->convlen + movlen)>kjsvpt->kjcvmax ) {
                        ret_code = IMKJCVOE;
                        break;
                };

                if( movlen > 0 ) {
                        /*
                         *      Previous Conversion Length less than or
                         *      Equqal Current Conversion Length.
                         *      Insert Different Number of Character
                         *      & Replace Current Conversion Data.
                         */
                        for( loop=convmax-1;loop>=kjsvpt->cconvlen;loop--)
                                kjcvmap[loop+movlen ] = kjcvmap[ loop ];

                        /*
                         *      Insert Area Conversion Status anbigious
                         *      status.
                         *      It's status set 'Now Conversion'.
                         */
                        for( loop = 0 ; loop < movlen ; loop += C_DBCS )
                                kjcvmap[kjsvpt->cconvlen+loop+1]= M_KSCNVK;
                } else if( movlen < 0 ) {
                        /*
                         *      Current Conversion Length less equual than
                         *      Previous Conversion Length.
                         *      Delete Different Number of Character
                         *      & Replace Current Conversion Data.
                         */
                        (void)memcpy(
                            (char *)&kjcvmap[kjsvpt->cconvlen + movlen],
                            (char *)&kjcvmap[kjsvpt->cconvlen],
                            convmax - kjsvpt->cconvlen);

                        (void)memset( (char *)&kjcvmap[convmax + movlen],
                                      '\0',
                                      -movlen);
                };

                /*
                 *      KKCB New Conversion Data Copy to KMISA.
                 */
                for( loop = 2; loop < kkcbsvpt->kjmapln ; loop++ ) {
                        workcnt = ( loop - 2 ) * C_DBCS;
                        kjcvmap[workcnt  ] = kkcbsvpt->kjmap[loop];
                        kjcvmap[workcnt+1] = M_KSCNVK;
                };
                /*
                 *      KKCB Grammer Map  Reflect to KMISA grammap.
                 */

                /*
                 *********************************************************
                 *      2. Kana/Kanji Control Block Reflect to Kanji
                 *      Monitor Internal Save Area.
                 *      KKCB:Grammer Map---> KMISA:Grammer Map
                 *********************************************************
                 */
                /*
                 *      Get Current Grammer Map Position.
                 */
                grampos = 1;
                for( loop = 1; loop <=phranum ; loop++ ) {
                        /*
                         *      Grammer Data Length Check.
                         */
                        if( kjsvpt->grammap[grampos] & (1<<(C_BITBYT-1)) )
                                grampos += 2;
                        else
                                grampos ++;
                };

                /*
                 *      Grammer Map Current Grammer Length Get.
                 */
                if( kjsvpt->grammap[grampos] & (1<<(C_BITBYT-1)) )
                        gramisa = 2;
                else
                        gramisa = 1;

                /*
                 *      Get KKCB Grammer Map Length.
                 */
                gramkkcb = kkcbsvpt->grmapln - 1;

                /*
                 *      Previous Grammer Length not equal Current
                 *      Grammer Length Reflect New Value Set KMISA.
                 */
                if( gramisa == gramkkcb ) {
                        (void)memcpy((char *)&kjsvpt->grammap[grampos],
                                     (char *)&kkcbsvpt->grammap[1],
                                     gramisa);
                } else {
                        /*
                         *      Grammar map longer before conversion,
                         *      then insert else replace and move left
                         *      least character.
                         */
                        movlen = gramkkcb - gramisa;
                        if( movlen>0 ) {
                                /*
                                 *      Move Right
                                 */
                                for( loop = kjsvpt->grammap[0]-1
                                    ;loop >=(grampos + gramisa)
                                    ; ) {
                                        kjsvpt->grammap[ loop + movlen ]
                                                = kjsvpt->grammap[ loop ];
                                        loop--;
                                };
                        } else {
                                /*
                                 *      Move Left Grammer Map.
                                 */
                                (void)memcpy(
                                  (char *)&kjsvpt->grammap[grampos],
                                  (char *)&kjsvpt->grammap[grampos-movlen],
                                  kjsvpt->grammap[0] -1 + movlen);
                        };

                        /*
                         *      Copy New Grammer Map.
                         */
                        (void)memcpy( (char *)&kjsvpt->grammap[grampos],
                                      (char *)&kkcbsvpt->grammap[1],
                                      gramkkcb);

                        /*
                         *      Grammap Length Adjust.
                         */
                        kjsvpt->grammap[0] += movlen;

                };

                /*
                 *********************************************************
                 *      3. Kana/Kanji Control Block Reflect to Kanji
                 *      Monitor Internal Save Area.
                 *      KKCB:Kana Map---> KMISA:Kana Map
                 *********************************************************
                 */
                /*
                 *      Kana Data Length Init Zero.
                 */
                kanabit= 0;

                /*
                 *      Kana Map First Data Address Copy to Working Var.
                 */
                dstpos = &kjsvpt->kanamap[1];   /* Get Pointer to Kana  */
                                                /* Map.                 */
                wrkpat = *dstpos++;             /* Get Pointer to       */
                                                /* Current Bit Pattern. */
                rotbyt = C_BITBYT;              /* Bit Available Number.*/

                /*
                 *      Skip Number of Bit On.
                 */
                totwrd = phranum + adjnum + 1;

                /*
                 *      Search Bit On Position Kana Map Length(bits).
                 */
                loop  = kjsvpt->kanalen;
                while( loop> 0 ) {
                        /*
                         *      Check MSB Bit On..
                         */
                        if( wrkpat & (1<<(C_BITBYT-1)) ) {
                                /*
                                 *      Phrase End?
                                 */
                                if( --totwrd<= 0 )
                                      break;
                        };

                        wrkpat <<= 1;
                        /*
                         *      Kanadata Length Make.
                         */
                        loop--;
                        kanabit++;

                        /*
                         *      Next kanamap Data Get.
                         */
                        if( --rotbyt<=0 ) {
                                wrkpat = *dstpos++;
                                rotbyt = C_BITBYT;
                        };
                };

                srcbit   = 0;                   /* KKCB Kana Map Data   */
                                                /* Available            */
                                                /* Position Get.        */
                srcpos   = &kkcbsvpt->kanamap[1];
                                                /* Get Pointer to KKCB  */
                                                /* Kana Map Data Adress */
                dstbit   = kanabit % C_BITBYT;  /* Get KKCB Kana Map    */
                                                /* Data Available Pos-  */
                                                /* ision.               */
                dstpos   = &kjsvpt->kanamap[1 + (kanabit/C_BITBYT)];
                                                /* Get KMISA Kana Map   */
                                                /* Data Area.           */

                /*
                 *      Replace KKCB Kanamap to KMISA.
                 */
                loop     = kkcbsvpt->kanalen1;
                while( loop > 0 ) {
                        /*
                         *      Activate Bit Number Get.
                         */
                        rembit = C_BITBYT - dstbit;
                        actbit = MIN(loop,C_BITBYT - srcbit);
                        actbit = MIN(rembit,actbit);

                        /*
                         *      KKCB Source Pattern Get.
                         *
                         *                      +----- actbit
                         *                    < |  >
                         *                   |    |
                         *      +------------+------+
                         *      |ZZZXXXXXXXXXYYYYYYA|
                         *      +---+--------+------+
                         *          ^        ^< |  >|
                         *          |        |  +--- rembit
                         *          |        +------ dstbit;
                         *          +--------------- srcbit;
                         *
                         *              |
                         *              V
                         *      +--------------------+
                         *      |XXXXXXXXXYYYYYYA0000|
                         *      +--------------------+
                         *              |
                         *              V
                         *      +--------------------+
                         *      |000000000000XXXXXXXX|
                         *      +--------------------+
                         *              |
                         *              V
                         *      +--------------------+
                         *      |000000000000XXXXXXXX|
                         *      +--------------------+
                         *              |
                         *              V
                         *      +--------------------+
                         *      |00000000000XXXXXXXX0|
                         *      +--------------------+
                         *              |
                         *              V
                         *      +--------------------+
                         *      |000000000000XXXXXXX0|
                         *      +--------------------+
                         */
                        srcpat = *srcpos;
                        srcpat <<= srcbit;
                        srcpat >>= dstbit;
                        srcpat >>=(rembit-actbit);
                        srcpat <<=(rembit-actbit);

                        /*
                         *    KMISA Target Pattern Get.
                         *
                         *                    +----- actbit
                         *                  < |  >
                         *                 |    |
                         *    +------------+------+
                         *    |ZZZXXXXXXXXXYYYYYYA|
                         *    +---+--------+------+
                         *        ^        ^< |  >|
                         *        |        |  +--- rembit
                         *        |        +------ dstbit
                         *        +--------------- srcbit
                         *
                         *            |       +----------------|
                         *            V                        V
                         *    +-------------------+   +-------------------+
                         *    |0000000ZZZXXXXXXXXX|   |A00000000000XXXXXXX|
                         *    +-------------------+   +-------------------+
                         *            |                        |
                         *            V                        V
                         *    +-------------------+   +-------------------+
                         *    |ZZZXXXXXXXXX0000000|   |000000000000000000A|
                         *    +-------------------+   +-------------------+
                         *            |                        |
                         *            V------------------------+
                         *    +-------------------+
                         *    |ZZZXXXXXXXXX000000A|
                         *    +-------------------+
                         */
                        wrkpat = dstpat = *dstpos;
                        dstpat >>= rembit;
                        dstpat <<= rembit;
                        wrkpat <<= (actbit+dstbit);
                        wrkpat >>= (actbit+dstbit);
                        dstpat |= wrkpat;

                        /*
                         *      New Bit Pattern Write.
                         */
                        *dstpos = dstpat | srcpat;

                        dstbit += actbit;
                        srcbit += actbit;
                        loop   -= actbit;

                        /*
                         *      Increment Source Bit Position.
                         */
                        if( dstbit >= C_BITBYT ) {
                                dstbit -= C_BITBYT;
                                dstpos++;
                        };

                        /*
                         *      Increment Destination Bit Position.
                         */
                        if( srcbit>= C_BITBYT ) {
                                srcbit -= C_BITBYT;
                                srcpos++;
                        };
                };

        } while( NILCOND );
        /*
         *      Debugging Output.
         */
        snap3(SNAP_KMISA|SNAP_KKCB,SNAP_Mnxprps,"End");

        /*
         ****************************************************************
         *      4. Return to Caller.
         ****************************************************************
         */
        return( ret_code );
}
