static char sccsid[] = "@(#)51	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Myomic.c, libKJI, bos411, 9428A410j 7/23/92 03:25:15";
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

/********************* START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:         _Myomic
 *
 * DESCRIPTIVE NAME:    Yomi conversion.
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
 * FUNCTION:            Double bytes code string convert to
 *                              single byte 7 bit yomikana code string.
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1688 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Myomic
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Myomic( dbcs_str,dbcs_len,cvt_area,chmode )
 *
 *  INPUT:              dbcs_str : Pointer of DBCS character srting.
 *                      dbcs_len : the byte length DBCS character string.
 *                      chmode   : Conversion mode.
 *
 *  OUTPUT:             cvt_area : pointer of 7bit code(MKK) strings.
 *                      _Myomic(): the byte length of 7bit code(MKK) string.
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      The byte length of 7bit code(MKK) string.
 *
 * EXIT-ERROR:          Wait State Codes.
 *                      NA.
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
 *      include Kanji Project.
 */
#include "kj.h"         /* Kanji Project Define File.                   */
#include "kcb.h"        /* Kanji controul block structer                */
#include "_Myomic.t"    /* conversion table                             */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-061 COPYRIGHT IBM CORP 1988           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Double bytes code string convert to
 *              single byte 7 bit yomikana code string.
 */
int     _Myomic( dbcs_str,dbcs_len,cvt_area,chmode )

uchar   *dbcs_str;      /* DBCS string area.                            */
int     dbcs_len;       /* DBCS string lengs.                           */
uchar   *cvt_area;      /* MKK 7bit code string area.                   */
uchar   chmode;         /* 7bit code conversion mode.                   */
/*      return       MKK 7bit code string length.(0 - dbcs_len/C_DBCS)  */
{
        int     ret_code;       /* Return Code.                         */

        int     i,j;            /* Loop Counter.                        */
        int     dbcs_ch;        /* DBCS character area.                 */
        uchar   dbcs_hit;       /* MKK code area.                       */
        int     tbl_idx;        /* MKK code conversion table pointer.   */

/************************************************************************
 *      debug process routine.                                          */
CPRINT(START KMYOMIC);

        /*  1.1
         *      Initialize the parameter area.
         */
        /* Return code zero clear.                                      */
        ret_code  =  C_INIT;

        while ( dbcs_len >= C_DBCS )
            {
                /* 1.2.0  Set on dbcs_ch the DBCS character from Input. */
                dbcs_ch  = dbcs_str[0] * C_HIBYTE + dbcs_str[1];

                /* Input character pointer increment two count.         */
                dbcs_str = dbcs_str + C_DBCS;

                /* Input character length decrement two count.          */
                dbcs_len = dbcs_len - C_DBCS;

                /* Conversion code area initiarize.                     */
                dbcs_hit = C_BYTEM;

                /* Conversion code pointer area initiarize.             */
                tbl_idx = -1;

                /* 1.2.1.1 Get the MKK table pointer. (1)
                 *         (hiragana mode,katakana mode and number mode.)
                 */
                if ( (chmode == K_CHHIRA) ||
                     (chmode == K_CHKATA) || (chmode == K_CHNUM )  )
                    {
                        /* Input code is less than C_SPACE ,
                                    the code can not convert.           */
                        if ( dbcs_ch < C_SPACE )
                            {
                                tbl_idx = -1;
                            }

                        /* Input code is more then C_SPACE and
                                               it is less than 0x83fc  */
                        if ( (C_SPACE <= dbcs_ch) && (dbcs_ch <= 0x83fc) )
                            {
                              /* tbl_idx is 0 , 1 or 2.                 */
                              /* Input code high is 0x81 , 0x82 or 0x83 */
                              /* C_HIBYTE is 0x0100                     */
                              /* C_SPACEH is   0x81                     */
                              tbl_idx  = (dbcs_ch / C_HIBYTE) - C_SPACEH;

                              /* Input code low is more than 0x40.      */
                              /* C_BYTEM  is 0xff                       */
                              /* C_SPACEL is 0x40                       */
                              dbcs_hit =
                              kana_tbl[tbl_idx][ (dbcs_ch & C_BYTEM)
                                                          - C_SPACEL ];
                            }

                        /* Input code is more than 0x83fc ,
                                    the code can not convert.           */
                        if ( 0x83fc < dbcs_ch ) {
                                tbl_idx = -1;
                            }

                        /* Input code is nothing on conversion table.   */
                        if ( tbl_idx == -1 ) 
			    {
                                if ( dbcs_ch == C_LIT ) 
				    dbcs_hit = ASQT;
				else if ( dbcs_ch == C_QUOTO )
				    dbcs_hit = ADBQ;
                                else
				    dbcs_hit = C_BYTEM;  /* C_BYTEM : 0xff  */
                            }
                     }

                /* 1.2.1.2 Get the MKK table pointer. (2)
                 *         (alphabet mode.)
                 */
                if ( chmode == K_CHALPH )
                    {
                        /* Input code is less than C_SPACE ,
                                              the code can not convert. */
                        if ( dbcs_ch < C_SPACE )
                            {
                                tbl_idx = -1;
                            }

                        /* Input code is more then C_SPACE and
                                               it is less than 0x82fc   */
                        if ( (C_SPACE <= dbcs_ch) && (dbcs_ch <= 0x82fc) )
                            {
                              /* tbl_idx is 0 or 1.                     */
                              /* Input code high is 0x81 or 0x82.       */
                              /* C_HIBYTE is 0x0100                     */
                              /* C_SPACEH is   0x81                     */
                              tbl_idx = (dbcs_ch / C_HIBYTE) - C_SPACEH;

                              /* Input code low is more than 0x40.      */
                              /* C_BYTEM  is 0xff                       */
                              /* C_SPACEL is 0x40                       */
                              dbcs_hit =
                              alph_tbl[tbl_idx][ (dbcs_ch & C_BYTEM)
                                                          - C_SPACEL ];
                            }

                        /* Input code is more than 0x82fc ,
                                              the code can not convert. */
                        if ( 0x82fc < dbcs_ch )
                            {
                                tbl_idx = -1;
                            }

                        /* Input code is nothing on conversion table.   */
                        if ( tbl_idx == -1 )
                            {
                                dbcs_hit = C_BYTEM;  /* C_BYTEM : 0xff  */

                                /* Input code is single quotation. (')  */
                                if ( dbcs_ch == C_LIT )
                                    dbcs_hit = 0x27;

                                /* Input code is double quotation. (")  */
                                if ( dbcs_ch == C_QUOTO )
                                    dbcs_hit = 0x22;
                            }
                     }

                /*   1.3
                 *      Put the MKK code.
                 */
                /* Check complete conversion.                           */
                if ( dbcs_hit == C_BYTEM )

                        /* input code is can not convert.               */
                        break;

                /* Output conversion code.                              */
                cvt_area[ ret_code ] = dbcs_hit;

                /* Increment output byte pointer(return code).          */
                ret_code = ret_code + C_ADD;    /* C_ADD : 2 bytes      */

            }

        /* Complete conversion input string to output string.           */
/************************************************************************
 *      debug process routine.                                          */
CPRINT(END KMYOMIC);

        /* Return is conversion string length.                          */
        return ( ret_code );
}

