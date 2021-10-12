static char sccsid[] = "@(#)40	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mrscvym.c, libKJI, bos411, 9428A410j 7/23/92 03:24:32";
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
 * MODULE NAME:         _Mrscvym
 *
 * DESCRIPTIVE NAME:    One Phrase Translatre 7Bit Yomi String To
 *                      DBCS Yomi String.
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
 * FUNCTION:            1. One Phrase 7Bit Yomi Code Get.
 *                      2. 7Bit Yomi Code Convert to DBCS String.
 *                      3. DBCS String Set Kana/Kanji Control Blcok.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1060 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mrscvym
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mrscvym( pt )
 *
 *  INPUT:              pt      :Pointer to Kanji Control Blck.
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
 *                              _Mkanagt:Get one Phrase Kana Information.
 *                              _Mkanasd:7Bit yomi Code Convert to DBCS.
 *                      Standard Library.
 *                              memcpy  :Copy # of Characters.
 *                              memset  :Set  # of Specified Characters.
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
 *                              cconvpos convlen  convpos  kjcvmap
 *                              kkcbsvpt
 *                      Trace Block(TRB).
 *                              NA.
 *                      Kana Kanji Control Block(KKCB).
 *                              NA.
 *
 *   OUTPUT:            Kanji Monitor Control Block(KCB).
 *                              NA.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              kjcvmap
 *                      Trace Block(TRB).
 *                              NA.
 *                      Kana Kanji Control Block(KKCB).
 *                              kanadata kjdata   kjlen    kkcrc
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              IDENTIFY:Module Identify Create.
 *                              SHTOCHPT:Short Set to Char Pointer.
 *                      Standard Macro Library.
 *                              NA.
 *
 * CHANGE ACTIVITY:     Sept. 29 1988 Satoshi Higuchi
 *                      Added logic about Yomi length shorter than Kanji
 *                      and do all candidates, drop adjunction
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
 *      One Phrase 7Bit Yomi Code Set DBCS String.
 */
int     _Mrscvym( pt )

register KCB *pt;       /* Pointer to Kanji Control Block(KCB).         */

{
        char    *memcpy();      /* Copy # of Character.                 */
        char    *memset();      /* Set Memory.                          */

        register KMISA *kjsvpt; /* Pointer to KMISA.                    */
        KKCB    *kkcbsvpt;      /* Pointer to KKCB.                     */
        register int loop;      /* Loop Counter.                        */
        int     movlen;         /* Length of Data Move.                 */
        int     kjcvofs;        /* Kanji Conversion Map Offset.         */
        int     convmax;        /* Kanji Conversion Map Length.         */
        register uchar *kjcvmap;/* Kanji Conversion Map Access Base     */
                                /* Pointer.                             */
        int     ret_code;       /* Return Code.                         */

        uchar   cnvmd;          /* 7bit yomi code convert to DBCS 2byte */
                                /* Conversion Mode.                     */
        short   strlen;         /* Number of DBCS Character Bytes.      */
        short   cvlen;          /* Number of DBCS Word Bytes.           */
        short   sbyomiln;       /* Length of 7bit yomi code.            */
        short   sbyomipos;      /* Position of 7bit yomi code.          */
        short   phranum;        /* Number of Phrase in yomi code.       */
/*======================================================================*/
/* #(B) Sept. 29 1988 Satoshi Higuchi                                   */
/* Added source.                                                        */
/*      short   adjnum;                                                 */
/*======================================================================*/
	short   adjnum;         /* Number of adjuction at Yomi          */
        uchar   *sbyomi;        /* Pointer to 7bit yomi code.           */

        uchar   *str;           /* Pointer to DBCS yomi code.           */

        /*
         *      Debugging Output.
         */
        snap3(SNAP_KCB|SNAP_KMISA,SNAP_Mrscvym,"Start");

        /*
         ****************************************************************
         *      0. Set Up General Work Pointer.
         ****************************************************************
         */
        /*
         *      Work Pointer & Variable Initialize.
         */
        kjsvpt  = pt->kjsvpt;           /* Pointer to KMISA.            */
        kkcbsvpt= kjsvpt->kkcbsvpt;     /* Pointer to KKCB.             */
        ret_code= IMSUCC;               /* Default Return Code Set.     */
        str     = &kkcbsvpt->kjdata[2]; /* Kana/Kanji Control Block     */
                                        /* DBCS String Address.         */

        /*
         ****************************************************************
         *      1. Get 7bit yomi code for MKK.
         ****************************************************************
         */
        /*
         *      Kanji Conversion Map Releted Position Get.
         */
        kjcvofs  = kjsvpt->cconvpos - kjsvpt->convpos;

        /*
         *      Get Kanji Conversion Map Base Address.
         */
        kjcvmap  = &kjsvpt->kjcvmap[kjcvofs];

        /*
         *      Get Kanji Conversion Map Length.
         */
        convmax  = kjsvpt->convlen - kjcvofs;

        /*
         *      Get 7Bit Yomi Data Length & Address.
         */
        (void)_Mkanagt(pt,kjcvofs,&sbyomipos,&sbyomiln,&phranum,&cvlen);

        /*
         *      Get 7Bit Yomi Data Address.
         */
        sbyomi = &kjsvpt->kanadata[sbyomipos];

        /*
         *      Get 7Bit Yomi Length.
         */
        strlen = sbyomiln * C_DBCS;
/*======================================================================*/
/* #(B) Sept. 29 1988 Satoshi Higuchi                                   */
/* Added sorce.                                                         */
/*      adjnum = (sbyomiln - phranum) * C_DBCS;                         */
/*======================================================================*/
	/*
	 *      Get Adjunct length that join Jiritugo
	 */
	adjnum = (sbyomiln - phranum) * C_DBCS;

        /*
         ****************************************************************
         *      2. 7bit yomi code convert to DBCS Code.
         ****************************************************************
         */
        /*
         *      Kanji to Yomi Data Conversion Type.
         */
        switch( kjcvmap[0] ) {
        /*
         *      DBCS Numeric Code.
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
        (void)_Mkanasd( cnvmd,sbyomi,sbyomiln,str );

        /*
         ****************************************************************
         *      3. Kanji Monitor Internal Save Area Kanji Conversion
         *      Map Set.
         ****************************************************************
         */
        /*
         *      Move Length Get.
         */
        movlen = strlen - cvlen;

        /*
         *      KMISA Kanji Conversion Map Conversion Status Sets.
         */
        for( loop = 0 ; loop < cvlen ; loop += C_DBCS ) {
                kjcvmap[loop+1] =
                        (kjcvmap[loop+1]==M_KSCNSK) ? M_KSCNSY:M_KSCNVY;
        };

        if( movlen < 0 ) {
                /*
                 *      Kanji Conversion Map.
                 */
/*======================================================================*/
/* #(B) Sept. 29 1988 Satoshi Higuchi                                   */
/* Changed source.                                                      */
/* Old source.                                                          */
/*      (void)memcpy( (char *)&kjcvmap[cvlen+movlen],                   */
/*                    (char *)&kjcvmap[cvlen],                          */
/*                    convmax + movlen);                                */
/* New source.                                                          */
/*      (void)memcpy((char *)&kjcvmap[strlen-adjnum],                   */
/*                   (char *)&kjcvmap[cvlen-adjnum],                    */
/*                   convmax + movlen);                                 */
/*======================================================================*/
		(void)memcpy((char *)&kjcvmap[strlen-adjnum],
			     (char *)&kjcvmap[cvlen-adjnum],
			     convmax + movlen);

                (void)memset( (char *)&kjcvmap[convmax+movlen],
                              '\0',-movlen);
        } else if( movlen > 0 ) {
                /*
                 *      Kanji Conversion Map shift right.
                 */
                for( loop=convmax-1;loop >= cvlen; loop-- )
                        kjcvmap[loop+movlen] = kjcvmap[loop];

                /*
                 *      Initialize Data.
                 */
                for( loop=cvlen ; loop<(movlen+cvlen) ;loop += C_DBCS ) {
                        kjcvmap[loop  ] = M_KJMCTN;
                        kjcvmap[loop+1] = kjcvmap[cvlen-1];
                };
        };

        /*
         ****************************************************************
         *      4. Kana/Kanji Control Block Error Status Clear.
         ****************************************************************
         */
        /*
         *      KKC Return Code Initialize.
         */
        kjsvpt->kkcrc  = K_KCSUCC;

        /*
         ****************************************************************
         *      5. Kana/Kanji Control Block DBCS String Copy to
         *      Kaji Monitor Internal Save Area First Conversion
         *      DBCS String Area.
         ****************************************************************
         */
        /*
         *      KKCB Kanji Data & Kanji Length Set.
         */
        /*
         *      Set KKCB Kanji Data.
         */
        (void)memcpy((char *)&kkcbsvpt->kjdata[2],(char *)str,strlen);

        /*
         *      Set KKCB Kanji Length.
         */
        kkcbsvpt->kjlen = strlen + 2;
        SHTOCHPT( kkcbsvpt->kjdata,kkcbsvpt->kjlen );

        /*
         *      Debugging Output.
         */
        snap3(SNAP_KCB|SNAP_KMISA,SNAP_Mrscvym,"End");

        /*
         ****************************************************************
         *      6. Return to Caller.
         ****************************************************************
         */
        return( ret_code );
}
