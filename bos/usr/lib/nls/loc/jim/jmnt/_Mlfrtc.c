static char sccsid[] = "@(#)27	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mlfrtc.c, libKJI, bos411, 9428A410j 7/23/92 03:23:50";
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
 * MODULE NAME:         _Mlfrtc
 *
 * DESCRIPTIVE NAME:    Get the character class code for
 *                                        Leftchar/Rightchar conversion.
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
 * FUNCTION:            Convert a DBCS code
 *                                     to the character class code.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1232 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mlfrtc
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mlfrtc( dbcs_let )
 *
 *  INPUT:              dbcs_let : pointer to the DBCS code.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         The chalacter class.
 *
 *
 * EXIT-ERROR:          NA.
 *
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              NA.
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
 *                              NA.
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
 *                              NA.
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
 *      Include Kanji Project.
 */
#include "kj.h"         /* Kanji Project Define File.                   */
#include "kcb.h"        /* Kanji Control Block Structure.               */
#include "_Mlfrtc.t"    /* Class code conversion table.                 */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-061 COPYRIGHT IBM CORP 1988           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Get the character class code for
 *                        Leftchar/Rightchar conversion.
 */
uchar   _Mlfrtc( dbcs_let )

uchar   *dbcs_let;         /* DBCS character pointer. (2 bytes)         */
{
        uchar   dbcs_typ;       /* Return Code.                         */
        int     dbcs_ch;        /* DBCS code or ANK code buffer         */
        int     i;              /* Loop counter.                        */

/************************************************************************
 *      debug process routine.                                          */
CPRINT(START KMLFRTC);

        /*  1.1
         *      Input ANK code or input DBCS code.
         */
        if (dbcs_let[0] == 0x00)
            /* Input ANK code set.    */
            {
                dbcs_ch = dbcs_let[1];
            }
        else
            /* Input DBCS code set.   */
            {
                dbcs_ch = (dbcs_let[0] * C_HIBYTE) + dbcs_let[1];
            };

        /*  1.2
         *      Get the character class code from the conversion table.
         */
        /* ANK class code get the conversion table.                     */
        if ( (0 <= dbcs_ch) && (dbcs_ch < C_HIBYTE) )
            {
                dbcs_typ = anktbl[ dbcs_ch ];
/*************************************************************************
 *      debug process routine.                                          */
CPRINT(END 1.2 KMLFRTC);

                /* Return ANK class code.                               */
                return( dbcs_typ);
            };

        /* DBCS class code get the conversion table [0].                */
        if ( (C_SPACE <= dbcs_ch) && (dbcs_ch <= 0x81fc) )
            {
                dbcs_typ = dbcstbl[0][ dbcs_ch - C_SPACE ];
/************************************************************************
 *      debug process routine.                                          */
CPRINT(END 1.2 KMLFRTC);

                /* Return DBCS class code.                              */
                return( dbcs_typ);
            };

        /* DBCS class code get the conversion table [1].                */
        if ( (0x8240 <= dbcs_ch) && (dbcs_ch <= 0x82fc) )
            {
                dbcs_typ = dbcstbl[1][ dbcs_ch - 0x8240 ];
/*************************************************************************
 *      debug process routine.                                           */
CPRINT(END 1.2 KMLFRTC);

                /* Return DBCS class code.                              */
                return( dbcs_typ);
            };

        /*  1.3
         *      Get the character class code from the extent input code.
         */
        /* Greek or alphabet class code get.                            */
        if ( (0x839f <= dbcs_ch) && (dbcs_ch <= 0x8491) )
            {
                dbcs_typ = M_ALPHA; /* Greek or alphabet class code set.*/
/************************************************************************
 *      debug process routine.                                          */
CPRINT(END 1.3 KMLFRTC);

                /* Return greek or alphabet class code.                 */
                return( dbcs_typ);
            };

        /* Katakana class code get.                                     */
        if ( (0x8340 <= dbcs_ch) && (dbcs_ch <= 0x8396) )
            {
                dbcs_typ = M_KANA; /* katakana class code set.          */
/************************************************************************
 *      debug process routine.                                          */
CPRINT(END 1.3 KMLFRTC);

                /* Return katakana class code.                          */
                return( dbcs_typ);
            };

        /* Ruled line class code get.                                   */
        if ( (0x849f <= dbcs_ch) && (dbcs_ch <= 0x84be) )
            {
                dbcs_typ = M_SYM4;      /* ruled line class code.       */
/************************************************************************
 *      debug process routine.                                          */
CPRINT(END 1.3 KMLFRTC);

                /* Return ruled line class code.                        */
                return( dbcs_typ);
            };

        /* single quotation(') or double quotation(") class code get.   */
        if ( (dbcs_ch == C_LIT) || (dbcs_ch == C_QUOTO) )
            {
                dbcs_typ = M_DELM;      /* ' "  class code set.         */
/************************************************************************
 *      debug process routine.                                          */
CPRINT(END 1.3 KMLFRTC);

                /* Return (') or (") class code.                        */
                return( dbcs_typ);
            };

        /* IBM expansion class code get.                                */
        if ( (0xfa40 <= dbcs_ch) && (dbcs_ch <= 0xfa5a) )
            {
                dbcs_typ = M_SYM4;      /* IBM expansion class code set.*/
/************************************************************************
 *      debug process routine.                                          */
CPRINT(END 1.3 KMLFRTC);

                /* Retrun IBM expansion class code.                     */
                return( dbcs_typ);
            };

        /* Kanji class code set.                                        */
        dbcs_typ = M_HIRA;              /* kanji class code set.        */
/************************************************************************
 *      debug process routine.                                          */
CPRINT(END 1.3 KMLFRTC);

        /* Return kanji class code.                                     */
        return ( dbcs_typ );
}
