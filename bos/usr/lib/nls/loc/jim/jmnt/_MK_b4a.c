static char sccsid[] = "@(#)62	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_MK_b4a.c, libKJI, bos411, 9428A410j 7/23/92 03:19:16";
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
 * MODULE NAME:         _MK_b4a
 *
 * DESCRIPTIVE NAME:    Hiragana / Katakana Conversion (Mode 1)
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
 * FUNCTION:            Convert Hiragana in the string into Katagana.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        868 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _MK_b4a
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _MK_b4a( pt )
 *
 *  INPUT:              pt      :Pointer to Kanji Controle Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         NA.
 *
 * EXIT-ERROR:          NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _Mgetchm() : Get Character Mode
 *                              _Mhrktsw() : Hiragana / Katakana
 *                                           Conversion Routine.
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
 *                              kjsvpt  string
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              cconvpos        convlen
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
 *                              kanadata       kanalen
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
#include "kcb.h"        /* Kanji Control Block Structure.               */


/*
 *      Copyright Identify.
 */
static char *cprt1="5601-061 COPYRIGHT IBM CORP 1988           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      This module does,
 *              1.Make a map for Hiragana / Katakana conversion.
 *              2.Call Hiragana / Katakana conversion routine.
 */
int  _MK_b4a( pt )

KCB     *pt;            /* Pointer to Kanji Control Block               */
{
        extern  uchar   _Mgetchm();     /* Get Character Mode           */
        extern  int     _Mhrktsw();     /* Hiragana/Katakana Conversion */
                                        /* Routine                      */

        register        KMISA   *kjsvpt;        /* Pointer to KMISA     */

        uchar   *string;        /* Work String Pointer                  */
        uchar   *mdpt;          /* Conversion Flag                      */
        uchar   *mpt;           /* Map Pointer                          */
        ushort  pos;            /* Conversion Position                  */
        int     alphlen;        /* Alph/Nume/Symbol Counter             */
        char    chr_typ;        /* Characte Type (_Mgetchm Return Code) */
        int     i;              /* Loop Counter                         */

        int     rc;             /* Return Code.                         */

snap3(SNAP_KCB | SNAP_KMISA,SNAP_MK_b4a,"start _MK_b4a");

        /*
         *      Pointer to Kanji Monitor Internal Save Area
         */

        kjsvpt = pt->kjsvpt;

        /*
         *      Set String Work Pointer
         */

        string = pt->string;

        /* 1. 1
         *      Make Hiragana/Katakana Map
         */

        mdpt    = kjsvpt->kanadata; /* Set Map Pointer                  */
        *mdpt   = M_NOHRKT;         /* Set Map Mode Flag                */
        pos     = kjsvpt->cconvpos; /* Set String Position              */
        mpt     = mdpt + 1;         /* Move Data Pointer                */
        alphlen = 0;                /* Initialize Alph / Num Counter    */
        kjsvpt->kanalen = 0 ;       /* Initialize Hira/Kana Map Length  */

        /*
         *      Hiragana / Katakana Map Making Process
         */

        for ( i = 0; i < kjsvpt->convlen / C_DBCS; i++ , pos += C_DBCS )
            {
                /*
                 *      Check Katakana Not to be Converted (High Byte)
                 */
                if ( *(string + pos) == 0x83 )
                   {
                        /*
                         *      Check Katakana Not to be Converted (Low Byte)
                         */
                        if ( *(string + pos + 1) == 0x94 ||
                             *(string + pos + 1) == 0x95 ||
                             *(string + pos + 1) == 0x96 )
                           {
                                /* Set Alpha Numeric Symbol Code */
                                *mpt++ = M_HKALPH;

                                /* Add Alpha Numeric Symbol Count */
                                alphlen++;
                                continue;
                           };
                   };

                /*
                 *      Check Hiragana Not to be Converted
                 */
                if ( *(string + pos    ) == 0x81 &&
                     *(string + pos + 1) == 0x5b )
                   {
                        /* Set Alpha/Numeric Symbol Code    */
                        *mpt++ = M_HKALPH;

                        /* Add Alpha/Numeric Symbol Counter */
                        alphlen++;
                        continue;
                   };

                /*
                 *      Get Character Type
                 */
                chr_typ = _Mgetchm( string, pos , kjsvpt );

                /*
                 *      Set Character Type Data into The Map
                 */
                switch ( chr_typ )
                       {
                        /*
                         *      Alpha/Numeric, Symbol, Blank
                         */
                        case K_CHALPH:
                        case K_CHNUM:
                        case K_CHKIGO:
                        case K_CHBLNK:

                            /* Set Alpha/Numeric, Symbol, Blank Code    */
                                *mpt++ = M_HKALPH;

                            /* Add Alpha/Numeric, Symbol, Blank Counter */
                                alphlen++;
                                break;

                        /*
                         *      Hiragana
                         */
                        case K_CHHIRA:
                                /* Set Hiragana Code */
                                *mpt++ = M_HKHIRA;

                                /* Set Hiragana/Katakana/Alpha/Symbol Flag */
                                *mdpt = M_HRKTMX;

                                /* Set Data Lenght */
                                kjsvpt->kanalen += alphlen + 1;

                                /* Clear Alpha/Numeric/Symbol Counter */
                                alphlen = 0;
                                break;

                        /*
                         *      Katakana
                         */
                        case K_CHKATA:
                                /* Set Katakana Code */
                                *mpt++ = M_HKKATA;

                                /*
                                 *      Check Map Mode Flag
                                 */
                                if ( *mdpt == M_NOHRKT )
                                        /* Set Katakana/Alpha/Symbol Flag */
                                        *mdpt = M_KTALL;

                                /* Set map Lenght */
                                kjsvpt->kanalen += alphlen + 1;

                                /* Clear Alpha/Numeric/Symbol Counter */
                                alphlen = 0;
                                break;
                       };
            };

        /* 1. 2
         *      Call Hiragana/Katakana Conversion Routine
         */

        rc = _Mhrktsw( pt );

        /* 1. 3
         *      Set Hiragana/Katakana Conversion Mode
         */

        kjsvpt->hkmode = K_HKKAN;

        /* 1. 4
         *      Return Code.
         */

snap3(SNAP_KCB | SNAP_KMISA,SNAP_MK_b4a,"return _MK_b4a");

        return( rc );
}
