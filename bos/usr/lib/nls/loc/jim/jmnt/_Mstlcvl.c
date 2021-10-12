static char sccsid[] = "@(#)48	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mstlcvl.c, libKJI, bos411, 9428A410j 7/23/92 03:25:02";
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
 * MODULE NAME:         _Mstlcvl
 *
 * DESCRIPTIVE NAME:    Decide Convetion.
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
 * FUNCTION:            Conversion Data Decide & Learning Update
 *                      Conversion Information.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        3708 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         Module Entry Point Name
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mstlcvl( pt,pos )
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *                      pos     :Settling Position.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      IMSUCC  :Success of Execution.
 *
 * EXIT-ERROR:          Waits Stats Code.
 *                              :KKC Phigical Error.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Internal Subroutines.
 *                              _Mstlcl1:Learning KKC Dictionary.
 *                              _Mstlcl2:Clear Decide Data.
 *                              _Mstlcl3:Overflow Conversion Process.
 *                      Kanji Project Subroutines.
 *                              _Mkanagt:Get Specfied Phrase Information.
 *                              _Msetch :Redraw Input Field Range Set.
 *                              _Mymstl :Decide Backend Process.
 *                              _Kcdctln:Kana/Kanji conversion MRU Learning.
 *                      Standard Library.
 *                              memcmp  :Compare # of Memory.
 *                              memcpy  :Copy # of Byte.
 *                              memset  :Set # of Character.
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
 *                              convlen  convpos  curleft  curright
 *                              kanalen  kanamap  kkcbsvpt
 *                      Trace Block(TRB).
 *                              NA.
 *                      Kana Kanji Control Block(KKCB).
 *                              NA.
 *
 *   OUTPUT:            Kanji Monitor Control Block(KCB).
 *                              hlatst
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              gramap1s kanadata kanalen  kanamap
 *                              kjcvmap  kjdata1s kkcrc
 *                              savelen  savepos  convlen  convpos
 *                              curright savelen  savepos  string
 *                      Trace Block(TRB).
 *                              NA.
 *                      Kana Kanji Control Block(KKCB).
 *                              grammap  grampln  kanadata kanalen1
 *                              kanalen2 kanamap  kjdata   kjlen
 *                              kjmapln
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              ALIGN   :Alignment Data Size.
 *                              CHPTTOSH:Get Short Data From Char Pointer.
 *                              IDENTIFY:Module Identify Create.
 *                              KKCPHYER:KKC Phigcal Error Check.
 *                              SHTOCHPT:Short Data Set Char Pointer
 *                                       In order.
 *
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

static  int _Mstlcl1(); /* Learning Dictionary.                 */
static  int _Mstlcl2(); /* KMISA Interface Var Init for KKCB.   */
static  int _Mstlcl3(); /* Overflow Decide Processing.          */

int     _Mstlcvl( pt,pos )

register KCB *pt;       /* Pointer to Kanji Control Block(KCB).         */
short   pos;            /* Settling Position.                           */

{
        int     memcmp();       /* Compare Memory.                      */
        char    *memcpy();      /* Copy # of Character.                 */

        register KMISA *kjsvpt; /* Pointer to KMISA.                    */
        int     ret_code;       /* Return Code.                         */
        short   indexpos;       /* kjcvmap Index.                       */
        short   incindex;       /* kjcvmap Index Increment Value.       */
        short   kjdatlen;       /* kjdata1s length.                     */
        int     lastpos;        /* kjcvmap Last Charcter Position.      */
        uchar   *str1;          /* Kanji Word Position for kjcvmap.     */
        uchar   *str2;          /* Kanji Word Position for kjdata1s.    */
        short   str1len;        /* Length of str1.                      */
        short   str2len;        /* Length of str2.                      */
        register int loop;      /* Loop Counter.                        */

        /*
         *      Debugging Output.
         */
        snap3(SNAP_KCB|SNAP_KMISA,SNAP_Mstlcvl,"Start");

        /*
         ****************************************************************
         *      0. Set Up General Work Pointer.
         ****************************************************************
         */
        /*
         *      Work Pointer & Variable Initialize.
         */
        kjsvpt  = pt->kjsvpt;
        ret_code= IMSUCC;

        /*
         ****************************************************************
         *      1. Overflow Conversion?
         ****************************************************************
         */
        if( pos > kjsvpt->curright ) {
                ret_code = _Mstlcl3( pt );

                /*
                 *      Debugging Output.
                 */
                snap3(SNAP_KCB|SNAP_KMISA,SNAP_Mstlcvl,"End");
                return( ret_code );
        };

        /*
         *      Get Current Conversion First Position.
         */
        indexpos = kjsvpt->convpos;

        /*
         ****************************************************************
         *      2. Get Decide Conversion Data Last Character Position.
         ****************************************************************
         */
        /*
         *      Get Current Conversion Last Position.
         */
        lastpos  = kjsvpt->convlen;

        /*
         ****************************************************************
         *      3. Decide Conversion Data From Current Conversion First
         *      Data to Specified Character Position.
         ****************************************************************
         */
        while( indexpos < pos && ret_code == IMSUCC ) {
                /*
                 *      Kanji Conversion Map Map Data Check.
                 */
                switch( kjsvpt->kjcvmap[0] ) {
                /*
                 *      Numeric Conversion don't Learning.
                 */
                case M_KJMKNM:
                        ret_code = _Mstlcl2(kjsvpt,&incindex);
                        break;
                /*
                 *      Not Conversion Data.
                 */
                case M_KJMNCV:
                        (void)memcpy((char *)kjsvpt->kjcvmap,
                                     (char *)&kjsvpt->kjcvmap[C_DBCS],
                                     lastpos- C_DBCS);
                        kjsvpt->kjcvmap[lastpos-C_DBCS  ]='\0';
                        kjsvpt->kjcvmap[lastpos-C_DBCS+1]='\0';
                        incindex  = C_DBCS;
                        break;
                /*
                 *      Alphanumeric Convertion Data is Learning.
                 */
                case M_KJMALN:
                /*
                 *      Kanji Conversion Data is Learning.
                 */
                default:
                        /*
                         *      Not Decide Conversion?
                         */
                        if( kjsvpt->kjcvmap[1]!=M_KSCNVK ) {
                            ret_code = _Mstlcl2(kjsvpt,&incindex);
                            break;
                        };

                        /*
                         *      Word Data Get.
                         */
                        str1 = &pt->string[indexpos];
                        str2 = &kjsvpt->kjdata1s[2];

                        /*
                         *      Get One Phrase Length And String.
                         *      From Current Conversion Data.
                         */
                        str1len = C_DBCS;
                        for( loop = str1len
                            ;loop <( pos  - kjsvpt->convpos)
                            ;loop += C_DBCS,str1len += C_DBCS ) {

                                /*
                                 *  Continuous Adjunct Word Check.
                                 */
                                switch( kjsvpt->kjcvmap[loop] ) {
                                case M_KJMCTN:  /* Continuous Data.     */
                                case M_KJMJAN:  /* Adjunct Data.        */
                                        continue;
                                };

                                /*
                                 *  Other Word Start.
                                 */
                                break;
                        };

                        /*
                         *      Get One Phrase Length & String
                         *      From First Conversion Data.
                         */
                        str2len = C_DBCS;
                        kjdatlen= CHPTTOSH(kjsvpt->kjmap1s);
                        for( loop = 2 + str2len/C_DBCS
                            ;loop < kjdatlen
                            ;loop ++ ,str2len+= C_DBCS) {

                                /*
                                 *  Continuous Adjunct Word Check.
                                 */
                                switch( kjsvpt->kjmap1s[loop] ) {
                                case M_KJMCTN:  /* Continuous Data.     */
                                case M_KJMJAN:  /* Adjunct Data.        */
                                        continue;
                                };

                                /*
                                 *  Other Word Start.
                                 */
                                break;
                        };

                        /*
                         *      Learing Word?
                         */
                        if(     (str1len!=str2len)
                             || (memcmp((char *)str1,
                                        (char *)str2,
                                        str1len)!=0 ) ) {
                                /*
                                 *      Learning Word.
                                 */
                                ret_code = _Mstlcl1( pt,indexpos );

                                if( ret_code != IMSUCC )
                                        break;
                        };

                        /*
                         *      Decide Conversion.
                         */
                        ret_code = _Mstlcl2(kjsvpt,&incindex);
                        break;
                };

                /*
                 *      Increment Kanji Conversion Map Index.
                 */
                indexpos += incindex;
        };

        /*
         ****************************************************************
         *      4. Conversion Decide Backend Processing.
         ****************************************************************
         */
        /*
         *      Decide Conversion Data.
         */
        if( ret_code == IMSUCC )
                ret_code = _Mymstl( pt,pos );
        else
                (void)_Mymstl( pt,pos );
        /*
         *      Debugging Output.
         */
        snap3(SNAP_KCB|SNAP_KMISA,SNAP_Mstlcvl,"End");

        /*
         ****************************************************************
         *      5. Return to Caller.
         ****************************************************************
         */
        return( ret_code );
}

/*
 *      First Conversion Data is different Current Conversion
 *      Data. We Tray to Learning.
 */
static int      _Mstlcl1( pt,indexpos )

register KCB *pt;       /* Pointer to KCB.                              */
short   indexpos;       /* Kanji Conversion Map Index Posotion.         */
{
        int      _Kcdctln();    /* Learning KKC MRU.                    */
        register KMISA *kjsvpt; /* Pointer to KMISA.                    */
        register KKCB *kkcbsvpt;/* Pointer to KKCB.                     */
        int     ret_code;       /* Return Code.                         */

        register int loop;      /* Loop Variable.                       */
        short   kanalen1;       /* length of kanadata length.           */
        short   kanalen2;       /* length of processing kanadata.       */
        short   kanapos;        /* Position of kanadata.                */
        short   kjcvlen;        /* Length of kjcvmap.                   */
        short   grampln;        /* Grammer Map Length.                  */

        /*
         ****************************************************************
         *      1-1. 0. Set Up General Work Pointer.
         ****************************************************************
         */
        /*
         *      Pointer Initialize.
         */
        kjsvpt   = pt->kjsvpt;          /* Get Pointer to KMISA.        */
        kkcbsvpt = kjsvpt->kkcbsvpt;    /* Get Pointer to KKCB.         */

        /*
         ****************************************************************
         *      1-1. 1. Kana/Kanji Control Block Interface Parameter
         *      Sets.
         ****************************************************************
         */
        /*
         *      Kanji Conversion Map Phrase Information Get.
         */
        (void)_Mkanagt(pt,0,&kanapos,&kanalen1,&kanalen2,&kjcvlen);

        /*
         *      KKCB Interface Parameter Sets.
         */
        kkcbsvpt->kjlen     = kjcvlen + 2;
        SHTOCHPT(kkcbsvpt->kjdata,kkcbsvpt->kjlen);
        (void)memcpy( (char *)&kkcbsvpt->kjdata[2]
                     ,(char *)&pt->string[indexpos],kjcvlen);

        /*
         *     KKCB Kana Data Set.
         */
        (void)memcpy((char *)kkcbsvpt->kanadata,
                     (char *)&kjsvpt->kanadata[kanapos],kanalen1);

        /*
         *      KKCB Kana Data Length Set.
         */
        kkcbsvpt->kanalen1 = kanalen1;

        /*
         *      KKCB Kana Data Length Only Word.
         */
        kkcbsvpt->kanalen2 = kanalen2;

        /*
         *      KKCB Kanji Map Set.
         */
        kkcbsvpt->kjmapln   = kjcvlen/C_DBCS + 2;
        SHTOCHPT(kkcbsvpt->kjmap,kkcbsvpt->kjmapln);
        for( loop = 0 ; loop < kjcvlen ; loop += C_DBCS ) {
                kkcbsvpt->kjmap[2 + loop/C_DBCS]
                        = kjsvpt->kjcvmap[loop];
        };
        /*
         *      KKCB Grammer Set,if Grammer Map MSB set data is 2Byte
         *      Otherwise 1byte.
         */
        if( kjsvpt->grammap[1] & (1<<(C_BITBYT-1)) )
                grampln = 2;
        else
                grampln = 1;

        /*
         *      KKCB Grammer Map Length Set.
         */
        kkcbsvpt->grmapln   = grampln + 1;
        kkcbsvpt->grammap[0]= kkcbsvpt->grmapln;

        /*
         *      KKCB Grammer Map Set.
         */
        (void)memcpy( (char *)&kkcbsvpt->grammap[1]
                     ,(char *)&kjsvpt->grammap[1],grampln);

        /*
         ****************************************************************
         *      1-1. 2. Call Dictionary Registration at Kana/Kanji
         *      Conversion Subroutins.
         ****************************************************************
         */
        /*
         *      KKCB Learning Dictionary Call.
         */
        snap3(SNAP_KKCB,SNAP_USER(599),"KKC Learning Start");
        kjsvpt->kkcrc = ret_code = _Kcdctln( kjsvpt->kkcbsvpt);
        snap3(SNAP_KKCB,SNAP_USER(599),"KKC Learning END");

        /*
         *      KKC Phigical Error Anzlize.
         */
        if( !KKCPHYER(ret_code) )
                ret_code = IMSUCC;

        /*
         ****************************************************************
         *      1-1. 3. Set Kana/Kanji Conversion Routine Close Status.
         ****************************************************************
         */
        /*
         *      KKC Use Flag On.
         */
        kjsvpt->kkcflag = M_KKNOP;

        /*
         ****************************************************************
         *      1-1. 4. Return to Caller.
         ****************************************************************
         */
        return( ret_code );
}

/*
 *      Decide Conversion Data and which Effective Data Clear.
 */
static int _Mstlcl2( kjsvpt,incindex )

register KMISA *kjsvpt; /* Pointer to KMISA.                            */
short   *incindex;      /* kjcvmap Increment Value.                     */

{
        char    *memset();      /* Set Memory Specified Character.      */
        char    *memcpy();      /* Copy # of Character.                 */

        register uchar *mapptr; /* Working Pointer.                     */
        register int    loop;   /* Loop Counter.                        */
        int     loop2;          /* Loop Counter.                        */

        int     grampln;        /* Grammer Map Length.                  */

        int     si;             /* Number of Shift Left kjcvmap.        */
        int     s1;             /* Number of Shift Left kjmap1s.        */
        int     a1;             /* Number of Shift Left kjmap1s.        */
        int     b1;             /* Number of Shift Left kanamap and     */
                                /* kanadata.                            */
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

        /*
         ****************************************************************
         *      1-2. 0. Set Up General Work Pointer.
         ****************************************************************
         */
        /*
         *      kjcvmap Shift Working Value Initialize.
         */
        mapptr  = &kjsvpt->kjcvmap[2];  /* Pointer Assign kjcvmap.      */
        si      = C_DBCS;               /* Shift Number Initialize.     */
        a1      = 0;                    /* Adjunct Counter Initialize.  */
        loop    = kjsvpt->convlen - C_DBCS;
                                        /* Maximum Loop Count.          */

        /*
         ****************************************************************
         *      1-2. 1. Update Releted Kana/Kanji Control Block Info-
         *      mation.
         ****************************************************************
         */
        /*
         *      Search Next Phrase.
         */
        while( loop>0 ) {
                /*
                 *      Next Phrase Map?
                 */
                switch( *mapptr ) {
                /*
                 *      Adjunct Char?
                 */
                case M_KJMJAN:
                        a1++;
                /*
                 *      Continuous Char?
                 */
                case M_KJMCTN:
                        /*
                         *      Phrase Length Inrement.
                         */
                        si    += C_DBCS;

                        /*
                         *      Get Next Map Address.
                         */
                        mapptr+= C_DBCS;

                        /*
                         *      Decrement Loop Couter.
                         */
                        loop  -= C_DBCS;
                        continue;
                };

                /*
                 *      Reached Next Phrase,Exit Loop.
                 */
                break;
        };

        /*
         *      Kanji Map Move Left & Tail Padding '\0'
         */
        (void)memcpy( (char *)kjsvpt->kjcvmap,
                (char *)&kjsvpt->kjcvmap[si],
                loop);
        (void)memset( (char *)&kjsvpt->kjcvmap[loop],
                '\0',
                si);

        /*
         *      Previous Kanji Map Shift Working Value Initialize.
         */
        mapptr  = &kjsvpt->kjmap1s[2+C_ANK];
        s1      = 1;
        loop    = CHPTTOSH(kjsvpt->kjmap1s)- 2 - s1;

        /*
         *      Search Next Phrase?
         */
        while( loop > 0 ) {
                /*
                 *      Next Phrase Map?
                 */
                switch( *mapptr ) {
                /*
                 *      Adjunct or Continuous Map?
                 */
                case M_KJMJAN:
                case M_KJMCTN:
                        s1++;
                        mapptr++;
                        loop--;
                        continue;
                };
                break;
        };

        /*
         *      Previous Kanji Map Move Left & Tail Padding '\0'
         */
        (void)memcpy( (char *)&kjsvpt->kjmap1s[2],
                      (char *)&kjsvpt->kjmap1s[2+s1],
                       loop);
        (void)memset( (char *)&kjsvpt->kjmap1s[2+loop],
                      '\0',
                       s1);
        SHTOCHPT(kjsvpt->kjmap1s,loop+2);

        /*
         *      Previous Kanji Data Shift Left (si*C_DBCS)byte
         *      and Tail Position Padding '\0'.
         */
        (void)memcpy( (char *)&kjsvpt->kjdata1s[2],
                (char *)&kjsvpt->kjdata1s[2+s1*C_DBCS],
                loop*C_DBCS);
        (void)memset((char *)&kjsvpt->kjdata1s[2+loop*C_DBCS],
                '\0',
                s1*C_DBCS);
        SHTOCHPT(kjsvpt->kjdata1s,loop*C_DBCS+2);

        /*
         *      Previous Grammer Map Shift Left One Word.
         */
        if( kjsvpt->gramap1s[1] & (1<<(C_BITBYT-1)) )
                grampln = 2;
        else
                grampln = 1;

        (void)memcpy( (char *)&kjsvpt->gramap1s[1],
                      (char *)&kjsvpt->gramap1s[1+grampln],
                      (int)kjsvpt->gramap1s[0] - grampln );
        kjsvpt->gramap1s[0] -= grampln;

        /*
         *      Set Kana Map Bit On Number.
         *      First   1 For Phrase Bit.
         *      Second a1 is Adjunct Bit.
         *      Third   1 For Next Bit.
         */
        loop2 = 1 + a1 + 1;

        /*
         *      Number of Kana data Length Initialize.
         */
        b1    = 0;

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
                        if( --loop2<= 0 )
                              break;
                };

                /*
                 *      7bit Yomi Code Count Up.
                 */
                b1++;

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
         *      Kana Map Shift Left b1 Bit.
         */
        /*
         *      Get Kana Map Next Phrase Address &
         *      Bit Position.
         */
        srcpos= &kjsvpt->kanamap[1 + (b1/C_BITBYT)];
        srcbit= b1 % C_BITBYT;

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
                ALIGN(kjsvpt->kanalen - b1,C_BITBYT)
                / C_BITBYT;

        /*
         *      Set New Kana Data Length.
         */
        kjsvpt->kanalen -= b1;

        /*
         *      Shift Left Kana Data.
         */
        (void)memcpy( (char *)&kjsvpt->kanadata[0],
                      (char *)&kjsvpt->kanadata[b1],
                      kjsvpt->kanalen);
        (void)memset( (char *)&kjsvpt->kanadata[kjsvpt->kanalen],'\0',b1);

        /*
         *      Grammer Map Shift Left One Word.
         */
        if( kjsvpt->grammap[1] & (1<<(C_BITBYT-1)) )
                grampln = 2;
        else
                grampln = 1;

        (void)memcpy( (char *)&kjsvpt->grammap[1],
                      (char *)&kjsvpt->grammap[1+grampln],
                      (int)kjsvpt->grammap[0] - grampln );
        kjsvpt->grammap[0] -= grampln;

        /*
         ****************************************************************
         *      1-2. 2. Set Next Conversion Data Position.
         ****************************************************************
         */
        /*
         *      Index Position Increment.
         */
        *incindex = si;

        /*
         ****************************************************************
         *      1-2. 3. Return to Caller.
         ****************************************************************
         */
        return( IMSUCC );
}


/*
 *      KMISA Overflow Decide Conversion Process.
 */

static int _Mstlcl3( pt )

register KCB *pt;       /* Pointer to KCB.                              */

{
        register KMISA *kjsvpt; /* Pointer to KMISA.                    */
        short   length;         /* Length Counter.                      */

        /*
         ****************************************************************
         *      1-3. 0. Set Up General Work Pointer.
         ****************************************************************
         */
        /*
         *      Pointer Initialize.
         */
        kjsvpt  = pt->kjsvpt;

        /*
         ****************************************************************
         *      1-3. 1. Initiailize Kana/Kanji Control Block Interface
         *      Variables.
         ****************************************************************
         */
        /*
         *      KMISA for KKCB Interface Variable Initialize.
         *
         *      Kanji Conversion Map,First Conversion Map
         *      First Conversion Kanji Data,Kana Map,
         *      Kana Data,Grammer Map,First Conversion Grammer Map
         *      All Data Area Initialize.
         */
        (void)memset((char *)kjsvpt->kjcvmap,'\0',kjsvpt->convlen);

        length = CHPTTOSH( kjsvpt->kjmap1s );
        if( length > 0 )
                (void)memset((char *)kjsvpt->kjmap1s,'\0',length);
        SHTOCHPT(kjsvpt->kjmap1s,2);

        length = CHPTTOSH( kjsvpt->kjdata1s );
        if( length > 0 )
                (void)memset((char *)kjsvpt->kjdata1s,'\0',length);
        SHTOCHPT(kjsvpt->kjdata1s,2);

        length = (short)kjsvpt->gramap1s[0];
        if( length > 0 )
                (void)memset((char *)kjsvpt->gramap1s,'\0',length);
        kjsvpt->gramap1s[0] = 1;

        (void)memset((char *)kjsvpt->kanamap,'\0',sizeof(kjsvpt->kanamap));
        kjsvpt->kanamap[0] = 1;

        if( kjsvpt->kanalen>0 )
                (void)memset((char *)kjsvpt->kanadata,'\0',kjsvpt->kanalen);
        kjsvpt->kanalen = 0;

        length = (short)kjsvpt->grammap[0];
        if( length > 0 )
                (void)memset((char *)kjsvpt->grammap,'\0',length);
        kjsvpt->grammap[0] = 1;

        /*
         ****************************************************************
         *      1-3. 2. Set Hiighlingting Attribute.
         ****************************************************************
         */
        /*
         *      Highlighting Attribute Set Normal.
         */
        length = kjsvpt->curright - kjsvpt->convpos;
        if( length > 0 )  {
                (void)memset((char *)&pt->hlatst[kjsvpt->convpos],
                     K_HLAT0,length);

                /*
                 *      Set Modify Display Area Set.
                 */
                (void)_Msetch( pt,kjsvpt->convpos,length);
        };

        /*
         ****************************************************************
         *      1-3. 3. Background Save Area & Conversion Area Reset.
         ****************************************************************
         */
        /*
         *      Background Save String Number & Position Reset.
         */
        kjsvpt->savepos = kjsvpt->curleft;
        kjsvpt->savelen = 0;

        /*
         *      Conversion String Number & Position Reset.
         */
        kjsvpt->convpos = kjsvpt->curleft;
        kjsvpt->convlen = 0;

        /*
         ****************************************************************
         *      1-3. 4. Return to Caller.
         ****************************************************************
         */
        return( IMSUCC );
}
