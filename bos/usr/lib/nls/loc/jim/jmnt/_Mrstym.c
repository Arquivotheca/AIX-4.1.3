static char sccsid[] = "@(#)42	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mrstym.c, libKJI, bos411, 9428A410j 7/23/92 03:24:41";
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
 * MODULE NAME:         _Mrstym
 *
 * DESCRIPTIVE NAME:    DBCS Kanji String to DBCS Yomi String.
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
 * FUNCTION:            DBCS Kanji String Convert to DBCS Yomi String
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        2680 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         Module Entry Point Name
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mrstym( pt,pos )
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *                      pos     :DBCS Kanji Code Last Position.
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
 *                              _MK_rtn :Conversion Mode Set.
 *                              _Mexchng:Replace String.
 *                              _Mkanasd:DBCS Yomi to Single Byte Kana.
 *                              _Msetch :Redraw Range Set.
 *                      Standard Library.
 *                              memcpy  :Copy # of Character.
 *                              memset  :Set # of Specified Char.
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
 *                              convlen  convpos  curleft  kjcvmap
 *                      Trace Block(TRB).
 *                              NA.
 *                      Kana Kanji Control BLock(KKCB).
 *                              NA.
 *
 *   OUTPUT:            Kanji Monitor Control Block(KCB).
 *                              hlatst
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              cconvlen cconvpos curcol   gramap1s
 *                              kanadata kanalen  kjcvmap  kjdata1s
 *                      Trace Block(TRB).
 *                              NA.
 *                      Kana Kanji Control BLock(KKCB).
 *                              NA.
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              ALIGN   :Aligment Number.
 *                              CHPTTOSH:Get Short Data From Char Pointer.
 *                              IDENTIFY:Module Identify Create.
 *                              MIN     :Which is Mininum Value Return.
 *                              SHTOCHPT:Set Shot Data To Char Pointer.
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
 *      DBCS Kanji Code Convert to DBCS Yomi Code.
 */
int     _Mrstym( pt,pos )

register KCB *pt;       /* Pointer to Kanji Control Block(KCB).         */
short   pos;            /* Yomi string negative conversion.             */
                        /* from KCB(string) convpos to pos.             */

{
        int     memcmp();       /* Compare Memory.                      */
        char    *memcpy();      /* Copy # of Character.                 */
        char    *memset();      /* Set Memory.                          */

        register KMISA *kjsvpt; /* Pointer to KMISA.                    */
        register int loop;      /* Loop Counter.                        */
        int     loopend;        /* Loop Terminate Value.                */
        int     movlen;         /* Length of Data Move.                 */
        int     kjmp1sln;       /* kjmap1s Move Length.                 */
        int     kjcvofs;        /* kjcvmap Offset.                      */
        register uchar *kjcvmap;/* kjcvmap Real Address.                */

        short   indexpos;       /* KCB String Index.                    */
        short   incindex;       /* indexpos increment value.            */
        short   incpos;         /* pos increment value.                 */
        short   convlen;        /* Current Convlen.                     */
        uchar   hlat;           /* Cursor Position Hilighting Attribute.*/

        uchar   wordtype;       /* Conversion Word Type.                */
        uchar   convstat;       /* Conversion Status.                   */

        uchar   sbdbcvflg;      /* Sigle byte/Double Byte Conversion    */
                                /* Flag.                                */
        uchar   convflg;        /* Conversion Information Active Flag.  */

        uchar   cnvmd;          /* 7bit yomi code convert to DBCS 2byte */
                                /* Conversion Mode.                     */
        short   len1;           /* Number of DBCS Character Bytes.      */
        short   sblen;          /* Length of 7bit yomi code.            */
        uchar   *sbyomi;        /* Pointer to 7bit yomi code.           */
        uchar   dbyomi[M_RGYLEN*2];
                                /* Pointer to DBCS yomi code.           */
        uchar  *bitpat;         /* Pointer to kanamap.                  */
        uchar   bitmsk;         /* kanamap bitpattern Check.            */
        int     rotbyt;         /* kanamap Next Byte Fetch Cycle.       */
        int     rembit;         /* Remain Bit Number.                   */
        uchar  *srcpos;         /* Pointer to Source kanamap.           */
        uchar  *dstpos;         /* Pointer to Destination kanamap.      */
        uchar   srcpat;         /* Source kanamap Bit Mask.             */
        uchar   dstpat;         /* Dsestination kanamap Bit Mask.       */
        uchar   wrkpat;         /* Work Bit Mask.                       */
        int     srcbit;         /* Source Active Bit Possitin.          */
        int     dstbit;         /* Destination Active Bit Position.     */
        int     actbit;         /* Real Useful Bit Number.              */
        int     sconvlen;       /* Save Current Convlen.                */
        int     adjnum;         /* Number of Adjunct.                   */

        /*
         *      Debugging Output.
         */
        snap3(SNAP_KCB|SNAP_KMISA,SNAP_Mrstym,"Start");

        /*
         ****************************************************************
         *      0. Set Up General Work Pointer.
         ****************************************************************
         */
        kjsvpt  = pt->kjsvpt;

        /*
         ****************************************************************
         *      1. Initial Value Set.
         ****************************************************************
         */
        /*
         *      Work Pointer & Variable Initialize.
         */
        indexpos= kjsvpt->convpos;
        sconvlen= kjsvpt->convlen;

        /*
         ****************************************************************
         *      2. DBCS 'Kanji Converted String' Recover to DBCS 'Yomi'
         *      From Conversion Position To Specified Postion.
         ****************************************************************
         */
        /*
         *      Conversion String(KCB) Recover to Yomi Code.
         */
        while( indexpos < pos ) {
                /*
                 *      Check Word Type.
                 */
                /*
                 *      Get Kanji Conversion Map Positon.
                 */
                kjcvofs  = indexpos - kjsvpt->convpos;
                kjcvmap  = &kjsvpt->kjcvmap[kjcvofs];

                wordtype = kjcvmap[0];  /* Current Conversion Map Map   */
                                        /* Data.                        */
                convstat = kjcvmap[1];  /* Current Conversion Map       */
                                        /* Conversion Status Data.      */

                /*
                 *      Get Remain Kanji Conversion Data.
                 */
                convlen  = kjsvpt->convlen - kjcvofs;

                sbdbcvflg= C_SWOFF;     /* 7Bit Yomi Code Not Available.*/
                convflg  = C_SWOFF;     /* DBCS Yomi/Kana Map/Kana Data */
                                        /* Not Available.               */

                /*
                 *      Analize Current Conversion Status.
                 */
                switch( convstat ) {
                /*
                 *      No Conversion Data Processing.
                 */
                case M_KSNCNV:  /* No Conversion Data.                  */
                        /*
                         *      Increment Next Position(do my length).
                         */
                        incindex = C_DBCS;

                        /*
                         *      Do my Length Diffrent After DBCS Yomi?
                         *      This Data No Conversion Data,so don't
                         *      Change Length.
                         */
                        incpos   = 0;

                        /*
                         *      Kanji Conversion Map Length DBCS.
                         */
                        len1     = C_DBCS;

                        /*
                         *      Kanji Conversion Map is allready
                         *      not available.
                         */
                        sblen    = 0;
                        break;
                /*
                 *      Not Conversion Data Processing.
                 */
                case M_KSCNUM:  /* Cannot Conversion Data.              */
                        /*
                         *      Increment Next Position(do my length).
                         */
                        incindex = C_DBCS;

                        /*
                         *      Do My Length Not Change.
                         */
                        incpos   = 0;

                        /*
                         *      Set Kanji Conversion Map Length.
                         */
                        len1     = C_DBCS;

                        /*
                         *      Kanji Conversion Map & Conversion
                         *      Map Claer 'ONE DBCS'.
                         */
                        sblen    = len1 / C_DBCS;
                        break;
                /*
                 *      Kanji Connversion or Signal Kanji Conversion
                 *      Data Processing.
                 */
                case M_KSCNVK:  /* Converting DBCS.                     */
                case M_KSCNSK:  /* Converting DBCS(Word).               */
                        /*
                         *      Single Byte/Double Byte Conversion Enable.
                         *      7Bit Yomi Data Avail.
                         */
                        sbdbcvflg = C_SWON;
                /*
                 *      Current Conversion Yomi Data Processing.
                 */
                case M_KSCNVY:  /* Converting Yomi DBCS.                */
                case M_KSCNSY:  /* Converting Yomi DBCS(Word).          */
                        /*
                         *      DBCS String/Kana Map/Kana Data was
                         *      Available.
                         */
                        convflg   = C_SWON;

                        /*
                         *      Get Current Converted Phrase Data,
                         *      and Length.
                         */

                        /*
                         *      Current Conversion Position DBCS String
                         *      Length Set.
                         */
                        len1    = C_DBCS;

                        /*
                         *      Initialize Counter for Adjunct.
                         */
                        adjnum  = 0;

                        /*
                         *      Search for Next Phrase Start
                         *      or Reached Conversion Data Not
                         *      Available.
                         */
                        for( loop=len1; loop<convlen ; loop += C_DBCS) {
                                /*
                                 *      Kanji Conversion Map Map
                                 *      Status Check.
                                 */
                                switch( kjcvmap[loop] ) {
                                case M_KJMJAN:  /* Adjunct Data.        */
                                        adjnum++;
                                case M_KJMCTN:  /* Continuous Data.     */
                                        len1 += C_DBCS;
                                        continue;
                                };
                                /*
                                 *      Other Data Type Available
                                 *      Next Phrase Start Position.
                                 */
                                break;
                        };

                        /*
                         *      Search Kanamap Bit On Next Position
                         *      ,with count word length.
                         */
                        /*
                         *      7Bit Yomi Data Counter Init.
                         */
                        sblen   = 0;

                        /*
                         *      Number of Bit is Current Phrase
                         *      and Next Phrase First Bit.
                         *      Word + Adjnuct Number + Next Word.
                         */
                        adjnum  = 1 + adjnum + 1;

                        /*
                         *      Get Kana Map First Data Address &
                         *      Available Bit Number.
                         */
                        bitpat  = &kjsvpt->kanamap[1];
                        rotbyt  = C_BITBYT;
                        bitmsk  = *bitpat++;

                        for( loop = kjsvpt->kanalen ;loop>0 ;loop-- ) {
                                /*
                                 *      Check MSB Bit On.
                                 */
                                if( bitmsk & (1<<(C_BITBYT-1)) ) {
                                        /*
                                         *      Next Phrase Arrive?
                                         */
                                        if( --adjnum<= 0 )
                                              break;
                                };

                                /*
                                 *      7bit Yomi Code Count Up.
                                 */
                                sblen++;

                                /*
                                 *      Get Next Kana Map Bit Pattern.
                                 */
                                /*
                                 *      Available Bit None?
                                 */
                                if( --rotbyt<= 0 ) {
                                        /*
                                         *      Get New Data And Reinit
                                         *      Available Bit Counter.
                                         */
                                        bitmsk = *bitpat++;
                                        rotbyt = C_BITBYT;
                                } else {
                                        /*
                                         *      Logical Shift Left One Bit.
                                         */
                                        bitmsk <<= 1;
                                };
                        };

                        /*
                         *      Kanji Conversion Map Conversion map
                         *      Increment Value Get.
                         */
                        incindex = C_DBCS * sblen;

                        /*
                         *      Conversion last postion incremnt Value.
                         */
                        if( sbdbcvflg == C_SWON )
                                incpos   = (sblen * C_DBCS) - len1;
                        else
                                incpos   = 0;
                        break;
                };

                /*
                 *      7bit yomi code convert to DBCS 2byte Code.
                 */
                if( sbdbcvflg == C_SWON ) {
                        /*
                         *      Get Pointer to 7bit yomi code.
                         */
                        sbyomi = (uchar *)kjsvpt->kanadata;

                        /*
                         *      Kanji to Yomi Data Conversion Type.
                         */
                        switch( wordtype ) {
                        /*
                         *      Kanji Numeric Code.
                         */
                        case M_KJMKNM:
                                cnvmd = M_NUMCV;
                                break;
                        /*
                         *      Kanji Alphabetic Character Code.
                         */
                        case M_KJMALN:
                                cnvmd = M_ALPHCV;
                                break;
                        /*
                         *      Other of Above Type Code.
                         */
                        default:
                                cnvmd = M_HIRACV;
                                break;
                        };

                        /*
                         *      Kanji Code Code to DBCS Yomi Code.
                         */
                        (void)_Mkanasd(cnvmd,sbyomi,sblen,dbyomi);

                        /*
                         *      KCB String Data Reform.
                         */
                        (void)_Mexchng(pt,
                                       dbyomi,0,sblen*C_DBCS,indexpos,len1);
                        /*
                         *      Overflow Conversion?
                         */
                        if( kjsvpt->convlen == 0 )
                                break;
                };

                /*
                 *      Clear Kanji Conversion Map.
                 */
                if( incindex > 0 ) {
                        /*
                         *      Move Length Size.
                         */
                        if( sblen!=0 )
                                movlen = sblen * C_DBCS - len1;
                        else
                                movlen = 0;

                        if( movlen < 0 ) {
                                /*
                                 *      Current Kanji Conversion Map
                                 *      Shift Left.
                                 */
                                (void)memcpy( (char *)&kjcvmap[len1+movlen],
                                              (char *)&kjcvmap[len1],
                                               convlen - len1);
                                (void)memset((char *)&kjcvmap[
                                                        convlen+movlen],
                                                '\0',-movlen);
                        } else if( movlen > 0 ) {
                                /*
                                 *      Current Kanji Convetion Map
                                 *      Shift Right.
                                 */
                                loop    = convlen - 1;
                                while( loop >= len1 ) {
                                        kjcvmap[loop+movlen] = kjcvmap[loop];
                                        loop--;
                                };
                        };

                        /*
                         *      Current Kanji Conversion Status Clear.
                         */
                        (void)memset((char *)&kjcvmap[0],'\0',sblen*C_DBCS);
                };

                /*
                 *      Conversion Information Active.
                 */
                if( convflg == C_SWON) {
                        /*
                         *      First Conversion Grammer Map Shift Left.
                         */
                        /*
                         *      Get Grammer Length.
                         */
                        if( kjsvpt->gramap1s[1]&(1<<(C_BITBYT-1)) )
                                movlen = 2;
                        else
                                movlen = 1;

                        /*
                         *      First Conversion Grammer Map Shift Left.
                         */
                        loop = kjsvpt->gramap1s[0] - 1;
                        (void)memcpy((char *)&kjsvpt->gramap1s[1],
                              (char *)&kjsvpt->gramap1s[1+movlen],
                              loop - movlen);
                        /*
                         *      Padding NULL After Shift Left Last Position.
                         */
                        (void)memset((char *)&kjsvpt->gramap1s[
                              1 + loop - movlen],
                              '\0',
                              movlen);
                        /*
                         *      Update First Conversion Grammer Map.
                         */
                        kjsvpt->gramap1s[0] -= movlen;

                        /*
                         *      First Conversion Kanji Data & Kanji Conv-
                         *      rtion Map Shift Left.
                         */

                        /*
                         *      Get First Conversion One Phrase Length.
                         */
                        kjmp1sln= 1;
                        loopend= CHPTTOSH( kjsvpt->kjmap1s );
                        for(loop=2+kjmp1sln;loop <loopend;loop++) {
                                switch( kjsvpt->kjmap1s[loop] ) {
                                case M_KJMCTN:  /* Contiuous Data.      */
                                case M_KJMJAN:  /* Adjunct Data.        */
                                        kjmp1sln++;
                                        continue;
                                };
                                break;
                        };

                        /*
                         *      First Conversion Kanji Map Shift Left.
                         */
                        (void)memcpy( (char *)&kjsvpt->kjmap1s[2],
                                      (char *)&kjsvpt->kjmap1s[2+kjmp1sln],
                                      loopend - 2 - kjmp1sln);
                        (void)memset( (char *)&kjsvpt->kjmap1s[
                                                loopend - kjmp1sln],
                                      '\0',
                                      kjmp1sln);

                        /*
                         *      Set First Kanji Convetion Map Length.
                         */
                        loop = loopend - kjmp1sln;
                        SHTOCHPT(kjsvpt->kjmap1s,loop);

                        /*
                         *      First Conversion Kanji Data Shift Left.
                         */
                        loopend = CHPTTOSH( kjsvpt->kjdata1s ) - 2;
                        kjmp1sln *= C_DBCS;
                        (void)memcpy( (char *)&kjsvpt->kjdata1s[2],
                                (char *)&kjsvpt->kjdata1s[2+kjmp1sln],
                                loopend - kjmp1sln);
                        (void)memset( (char *)&kjsvpt->kjdata1s[
                                                2 + loopend - kjmp1sln],
                                      '\0',
                                      kjmp1sln);

                        /*
                         *      Set First Kanji Conversion Data Length.
                         */
                        loop = 2 + loopend  - kjmp1sln;
                        SHTOCHPT(kjsvpt->kjdata1s,loop);

                        /*
                         *      Kana Map shift left 'sblen'bits.
                         */
                        /*
                         *      Get Kana Map Next Phrase Address &
                         *      Bit Position.
                         */
                        srcpos= &kjsvpt->kanamap[1 + (sblen/C_BITBYT)];
                        srcbit= sblen % C_BITBYT;

                        /*
                         *     Get Next Kana Map Phrase Address &
                         *     Bit Position.
                         */
                        dstpos= &kjsvpt->kanamap[1];
                        dstbit= 0;

                        loop =kjsvpt->kanalen;
                        while( loop > 0 ) {
                                /*
                                 *      Get Available Bit Number Minimum
                                 */
                                rembit = C_BITBYT - dstbit;
                                actbit = MIN(loop,rembit);
                                actbit = MIN(actbit,C_BITBYT - srcbit);

                                /*
                                 *      Load Shift Data.
                                 */
                                srcpat = *srcpos;
                                srcpat <<= srcbit;
                                srcpat >>= dstbit;
                                srcpat >>= rembit - actbit;
                                srcpat <<= rembit - actbit;

                                /*
                                 *      Destination Pattern Get.
                                 */
                                wrkpat = dstpat = *dstpos;
                                wrkpat = dstpat;
                                dstpat >>= rembit;
                                dstpat <<= rembit;
                                wrkpat <<= dstbit + actbit;
                                wrkpat >>= dstbit + actbit;
                                dstpat |= wrkpat;

                                /*
                                 *      Set Pattern.
                                 */
                                *dstpos = dstpat | srcpat;

                                /*
                                 *      Bit Possition Update.
                                 */
                                srcbit += actbit;
                                dstbit += actbit;
                                loop   -= actbit;

                                if( srcbit >= C_BITBYT ) {
                                        srcpos++;
                                        srcbit -= C_BITBYT;
                                };

                                if( dstbit >= C_BITBYT ) {
                                        dstpos++;
                                        dstbit -= C_BITBYT;
                                };
                        };

                        /*
                         *      Set Kana Map Length.
                         */
                        kjsvpt->kanamap[0] = 1 +
                                ALIGN(kjsvpt->kanalen - sblen,C_BITBYT)
                                / C_BITBYT;

                        /*
                         *      Kana Data shift left 'sblen'bytes.
                         */
                        loop = kjsvpt->kanalen;
                        (void)memcpy( (char *)kjsvpt->kanadata,
                                      (char *)&kjsvpt->kanadata[sblen],
                                      loop - sblen);
                        (void)memset( (char *)&kjsvpt->kanadata[
                                      loop - sblen],
                                      '\0',
                                      sblen);

                        /*
                         *      Set Kana Data Length.
                         */
                        kjsvpt->kanalen -= sblen;

                        /*
                         *      Current Grammer Map shift left.
                         */
                        if( kjsvpt->grammap[1] & (1<<(C_BITBYT-1)) )
                                movlen = 2;
                        else
                                movlen = 1;

                        /*
                         *      Current Grammer Map Shift Left.
                         */
                        loop = kjsvpt->grammap[0] - 1;
                        (void)memcpy((char *)&kjsvpt->grammap[1],
                                (char *)&kjsvpt->grammap[1+movlen],
                                loop - movlen);
                        (void)memset((char *)&kjsvpt->grammap[
                                1 + loop - movlen],
                                '\0',
                                movlen);
                        /*
                         *      Set New Grammer Map Length.
                         */
                        kjsvpt->grammap[0] -= movlen;

                };

                /*
                 *      Debuggging Dump.
                 */
                snap3(SNAP_KMISA,SNAP_Mrstym,"Move KMISA");

                /*
                 *      Get Next Kanji Conversion Map Position.
                 */
                indexpos += incindex;

                /*
                 *      Get Last Kanji Convettion Map Position Update.
                 */
                pos      += incpos;
        };

        /*
         ****************************************************************
         *      3. Now Overflow Conversion Status?
         ****************************************************************
         */
        if( kjsvpt->convlen == 0 ) {
                /*
                 *      Cursor Posoition Reset.
                 */
                pt->curcol = kjsvpt->cconvpos;

                /*
                 *      Current Conversion Region Reset.
                 */
                kjsvpt->cconvlen = 0;
                kjsvpt->convpos  = kjsvpt->curleft;

                /*
                 *      Reset Primary Input Mode.
                 */
                (void)_MK_rtn( pt,A_1STINP);
        } else {
                /*
                 ********************************************************
                 *      4. Normal Yomi Conversion.
                 ********************************************************
                 */
                /*
                 *      Current Active Conversion Position Reset.
                 */
                kjsvpt->cconvpos = kjsvpt->convpos;

                /*
                 *      Yomi Data Hilighting(Reverse for cursor & Underline
                 *      for Yomi Data).
                 */
                switch( kjsvpt->kjcvmap[1] ) {
                case M_KSNCNV:  /* No Convetion Data.                   */
                case M_KSCNVK:  /* Converting DBCS.                     */
                case M_KSCNSK:  /* Converting DBCS(Word).               */
                case M_KSCNVY:  /* Converting Yomi.                     */
                case M_KSCNSY:  /* Converting Yomi(Word).               */
                        /*
                         *      Hilighting Reverse & Underline.
                         */
                        hlat    = K_HLAT3;
                        /*
                         *      Current Conversion Length Set.
                         */
                        kjsvpt->cconvlen = C_DBCS;
                        break;
                case M_KSCNUM:
                default      :
                        /*
                         *      Underline.
                         */
                        hlat    = K_HLAT2;

                        /*
                         *      Current Conversion Legnth Set.
                         */
                        kjsvpt->cconvlen = 0;
                        break;
                };
                (void)memset((char *)&pt->hlatst[kjsvpt->convpos],
                                hlat,C_DBCS);
                (void)memset((char *)&pt->hlatst[kjsvpt->convpos+C_DBCS],
                                K_HLAT2,
                                kjsvpt->convlen - C_DBCS);

                if( sconvlen > kjsvpt->convlen ) {
                        (void)memset(
                                (char *)&pt->hlatst[
                                        kjsvpt->convpos + kjsvpt->convlen],
                                K_HLAT0,
                                sconvlen - kjsvpt->convlen);
                };

                /*
                 ********************************************************
                 *      5. Set Redraw Area.
                 ********************************************************
                 */
                (void)_Msetch( pt,kjsvpt->convpos,kjsvpt->convlen);
        };

        /*
         *      Debugging Output.
         */
        snap3(SNAP_KCB|SNAP_KMISA,SNAP_Mrstym,"Start");

        /*
         ****************************************************************
         *      6. Return to Caller.
         ****************************************************************
         */
        return( IMSUCC );
}

