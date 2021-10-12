static char sccsid[] = "@(#)25	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mktec.c, libKJI, bos411, 9428A410j 7/23/92 03:23:42";
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
 * MODULE NAME:         _Mktec
 *
 * DESCRIPTIVE NAME:    Kana to alphabet conversion program.
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
 * FUNCTION:            Convert a kana code to
 *                             an alphabet code on same key.
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        640 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mktec
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mktec( code )
 *
 *  INPUT:              code    :Input code from Keyboard
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         alphabet code
 *
 *
 * EXIT-ERROR:          IMKTEERR : nocorrespondence code on table.
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
 *      Kana to alphabet conversion program.
 */
char    _Mktec( code )

uchar    code;       /* Input code from Keyboard.(I)                    */
{
static uchar ktec_tbl[256] =
{
/*  00 -- 0f  */
IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,
IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,

/*  10 -- 1f  */
IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,
IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,

/*  20 -- 2f  */
0x20    ,0x21    ,0x22    ,0x23    ,0x24    ,0x25    ,0x26    ,0x27    ,
0x28    ,0x29    ,0x2a    ,0x2b    ,0x2c    ,0x2d    ,0x2e    ,0x2f    ,

/*  30 -- 3f  */
0x30    ,0x31    ,0x32    ,0x33    ,0x34    ,0x35    ,0x36    ,0x37    ,
0x38    ,0x39    ,0x3a    ,0x3b    ,0x3c    ,0x3d    ,0x3e    ,0x3f    ,

/*  40 -- 4f  */
0x40    ,0x41    ,0x42    ,0x43    ,0x44    ,0x45     ,0x46   ,0x47    ,
0x48    ,0x49    ,0x4a    ,0x4b    ,0x4c    ,0x4d     ,0x4e   ,0x4f    ,


/*  50 -- 5f  */
0x50    ,0x51    ,0x52    ,0x53    ,0x54    ,0x55     ,0x56   ,0x57    ,
0x58    ,0x59    ,0x5a    ,0x5b    ,0x5c    ,0x5d     ,0x5e   ,0x5f    ,

/*  60 -- 6f  */
0x60    ,0x61    ,0x62    ,0x63    ,0x64    ,0x65     ,0x66   ,0x67    ,
0x68    ,0x69    ,0x6a    ,0x6b    ,0x6c    ,0x6d     ,0x6e   ,0x6f    ,

/*  70 -- 7f  */
0x70    ,0x71    ,0x72    ,0x73    ,0x74    ,0x75     ,0x76   ,0x77    ,
0x78    ,0x79    ,0x7a    ,0x7b    ,0x7c    ,0x7d     ,0x7e   ,IMKTEERR,

/*  80 -- 8f  */
IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,
IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,

/*  90 -- 9f  */
IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,
IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,

/*  a0 -- af  */
IMKTEERR,0xa1    ,0xa2    ,0xa3    ,0xa4    ,0xa5    ,0xa6    ,0xa7    ,
0xa8    ,0xa9    ,0xaa    ,0xab    ,0xac    ,0xad    ,0xae    ,0xaf    ,

/*  b0 -- bf  */
0xb0    ,0xb1    ,0x65    ,0xb3    ,0xb4    ,0xb5    ,0x74    ,0x67    ,
0x68    ,0xb9    ,0x62    ,0x78    ,0x64    ,0x72    ,0x70    ,0x63    ,

/*  c0 -- cf  */
0x71    ,0x61    ,0x7a    ,0x77    ,0x73    ,0x75    ,0x69    ,0xc7    ,
0xc8    ,0x6b    ,0x66    ,0x76    ,0xcc    ,0xcd    ,0xce    ,0x6a    ,

/*  d0 -- df  */
0x6e    ,0xd1    ,0xd2    ,0x6d    ,0xd4    ,0xd5    ,0xd6    ,0x6f    ,
0x6c    ,0xd9    ,0xda    ,0xdb    ,0xdc    ,0x79    ,0xde    ,0xdf    ,

/*  e0 -- ef  */
IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,
IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,

/*  f0 -- ff  */
IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,
IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,IMKTEERR,
};

/************************************************************************
 *      debug process routine.                                          */
CPRINT(START-END KMKTEC);

        /* Return conversion code on table.                             */
        return(ktec_tbl[(int)code]);
}
