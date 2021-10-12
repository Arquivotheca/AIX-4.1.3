static char sccsid[] = "@(#)46	1.4.1.1  src/bos/usr/lpp/jls/dictutil/kuhkfc.c, cmdKJI, bos411, 9428A410j 7/23/92 01:27:39";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kuhkfc
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/********************* START OF MODULE SPECIFICATIONS **********************
 *
 * MODULE NAME:         kuhkfc
 *
 * DESCRIPTIVE NAME:    Hiragana or Kantakana Check Routine
 *
 * COPYRIGHT:           5756-030 COPYRIGHT IBM CORP 1991
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              User Dictionary Maintenance for AIX 3.2
 *
 * CLASSIFICATION:      OCO Source Material - IBM Confidential.
 *                      (IBM Confidential-Restricted when aggregated)
 *
 * FUNCTION:            Hiragana or Katakana Check Routine
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        672 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kuhkfc
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            kuhkfc( kanadat, kanalen, kjdata, kjlen,
 *                                                hkdata, hklen )
 *
 *  INPUT:              kanadata:Kana (7 bit) Data String.
 *                      kanalen :Kana (7 bit) Length.
 *                      kjdata  :Kanji Data String.
 *                      kjlen   :Kanji String Length.
 *
 *  OUTPUT:             hkdata  :Hiragana/Katakana Conversion Data
 *                      hklen   :Hiragana/Katakana Length
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      U_DIVHK :Unsuccessful Conversion(Hiragana/Katakana)
 *                      U_DHIRA :Successful Conversion(Hiragana)
 *                      U_DKATA :Successful Conversion(Katakana)
 *
 * EXIT-ERROR:          Wait State Codes.
 *                      NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              kudicymc() : Convert 7 Bits Code from
 *                                                         Kanji Code.
 *                      Standard Library.
 *                              strncmp
 *                      Advanced Display Graphics Support Library(GSL).
 *                              NA.
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      NA.
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
#include "kut.h"        /* Kanji Utility Define File.                   */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-125 COPYRIGHT IBM CORP 1989           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Please Descripte This Module.
 */
int  kuhkfc( kanadata, kanalen, kjdata, kjlen, hkdata, hklen )

uchar   *kanadata;      /* Yomigana String                              */
short   kanalen;        /* Yomigana String Length                       */
uchar   *kjdata;        /* Kanji String (PC Kanji Code)                 */
short   kjlen;          /* Kanji String Length                          */
uchar   *hkdata;        /* Hiragana/Katakana Conversion Code            */
short   *hklen;         /* Hiragana/Katakana Conversion Length          */
{
        extern  void    kudicymc();     /* PC Kanji Code Conversion     */

        uchar   conv_dt[U_KJ_L];        /* 7 Bits Code Data Area        */
        short   conv_len;               /* 7 Bits Code Length           */
        short   mflag;                  /* 7 Bits Code Conversion Flag  */

        int     rt_cd;                  /* strncmp Return Code          */

        int     rc;                     /* Return Code                  */

        /*
         *      Unsuccssful Return Code Set
         */

        rc = U_DIVHK;

        /*
         *      Input Parameter Initialize
         */

        *hkdata = 0x00;
        *(hkdata + 1) = 0x00;

        *hklen = 0;

        /*
         *      Data Length Check (kanalen or kjlen)
         */

        if ( kanalen <= 0 || kjlen   <= 0 )
           {
                return( rc );
           };

        /*
         *      Convert 7 Bits Code from PC Kanji Code
         */

        (void)kudicymc( kjdata, kjlen, conv_dt, &conv_len, &mflag );

        switch ( mflag )
          {
            /*
             *  Hiragana Code (kudicymc Return Code)
             */
            case 0 :

            /*
             *  Katakana Code (kudicymc Return Code)
             */
            case 2 :

                /*
                 *      Kanalen and Conv_len Length Check
                 */

                if ( kanalen != conv_len )
                   {
                        break;
                   };

                /*
                 *      Kanadata and Conv_dt Matching Check
                 */

                rt_cd = strncmp( kanadata, conv_dt, conv_len );

                /*
                 *      strncmp Return Code Check
                 */

                if ( rt_cd != 0 )
                   {
                        break;
                   };

                /*
                 *      Hiragana / Katakana Length Set
                 */

                *hklen = 2;

                /*
                 *      Hiragana / Katakana Check
                 */

                if ( mflag == 0 )
                   {
                        /*
                         *      Hiragana Code Set
                         */
                        *hkdata = 0xff;
                        *(hkdata + 1) = 0xfe;

                        /*
                         *      Hiragana Return Code Set
                         */
                        rc = U_DHIRA;
                   }
                else
                   {
                        /*
                         *      Katakana Code Set
                         */
                        *hkdata = 0xff;
                        *(hkdata + 1) = 0xff;

                        /*
                         *      Katakana Return Code Set
                         */
                        rc = U_DKATA;
                    };
                break;

            default :
                break;
          };

        /*
         *      Return Value
         */

        return( rc );
}

