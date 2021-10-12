/* @(#)57	1.5.1.1  src/bos/usr/lpp/jls/dictutil/kut.h, cmdKJI, bos411, 9428A410j 7/23/92 01:30:11 */
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: header file
 *
 * ORIGINS: 27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/********************* START OF MODULE SPECIFICATIONS **********************
 *
 * MODULE NAME:         kut.h
 *
 * DESCRIPTIVE NAME:    User Dictionary Maintenance Utility General
 *                      Constant Values.
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
 * FUNCTION:            NA.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Macro.
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        NA.
 *
 * ENTRY POINT:         NA.
 *
 * EXIT-NORMAL:         NA.
 *
 * EXIT-ERROR:          NA.
 *
 * EXTERNAL REFERENCES: NA.
 *
 * TABLES:              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              NA.
 *                      Standard Macro Library.
 *                              NA.
 *
 * CHANGE ACTIVITY:     NA.
 *
 ********************* END OF MODULE SPECIFICATIONS ************************
 */

#ifndef _kj_kudef
#define _kj_kudef

#ifdef  KKCFIX
#include "kurename.h"           /* Kanji Utility External Name Reaname. */
#endif
#include "kumacros.h"           /* Kanji Utility Macros.                */
#include "kucommon.h"           /* Kanji Utility Common Define File.    */
#include "kuke.h"               /* Kanji Utility key input.             */

/* standerd err out file        */
#define   ERR_FILE  ("/tmp/..kjdict.err")

#define   U_SD_0    ( 0 )    /* switch statement case no. : <0> yomi only */
#define   U_SD_1    ( 1 )    /* switch statement case no. : <1> yomi & kj */
#define   U_SD_2    ( 2 )    /* switch statement case no. : <2> gerric    */
#define   U_FRONT   ( 1 )    /* current flag : front current              */
#define   U_REVES   ( 2 )    /* current flag : front current              */

#define   U_BAS_IL  (  7  )     /*  il base  value                        */
#define   U_DL_MN   (  3  )     /*  dl minimum value                      */
#define   U_DL_MX   ( 255 )     /*  dl maximum value                      */
#define   U_NAR_1K  (  8  )     /*  nar initial value  ( 1kb )            */
#define   U_NAR_2K  (  9  )     /*  nar initial value  ( 2kb )            */
#define   U_NAR_3K  ( 10  )     /*  nar initial value  ( 3kb )            */
#define   U_DL_ARA  ( 256 )     /*  dl data area  length                  */
#define   U_ST_CD0  ( 0 )              /* set recovery code  data         */
#define   U_ST_CD1  ( 1 )              /* set recovery code  data         */
#define   U_ST_CD2  ( 2 )              /* set recovery code  data         */
#define   U_ST_CD5  ( 5 )              /* set recovery code  data         */
#define   U_ST_CD6  ( 6 )              /* set recovery code  data         */
#define   U_BUFFUL  ( 20  )            /* local kana data buffer size     */
#define   U_BUFPTR  ( 2  )             /* local pointer  initialize       */
#define   U_ID_STS  ( 2  )             /* index sts                       */
#define   U_IL_HED  ( 5  )             /* index header length             */
#define   U_MN_KNL  ( 2  )             /* knl   minimum  length           */
#define   U_MX_KNL  ( 11 )             /* knl   maximum  length           */
#define   U_MN_KJL  ( 2  )             /* kjl   minimum  length           */
#define   U_MX_KJL  ( 41 )             /* kjl   maximum  length           */
#define   U_DL_MIN  ( 3  )             /* dl    minimum  length           */
#define   U_HARIV1  ( 0x5c )           /* har   check                     */
#define   U_HARIV2  ( 0xb3 )           /* har   check                     */
#define   U_HARIV3  ( 0xff )           /* har   check                     */
#define   U_RL_HED  ( 2 )              /* rl header position              */
#define   U_NARMIN  ( 8 )              /* nar   minimum  length           */
#define   U_DL_HED  ( 0 )              /* dl head position                */
#define   U_DL_RSV  ( 1 )              /* dl reserve length               */

#define   U_RRN_L1  ( 1 )             /*  rrn skip                        */
#define   U_REC_L1  ( 1024 )          /*    1 recorde size ( 1K )         */
#define   U_REC_L2  ( 2048 )          /*    2 recorde size ( 2K )         */
#define   U_REC_L3  ( 3072 )          /*    3 recorde size ( 3K )         */

#define   U_FST_C1  ( 0x81 )          /*  first kanji data check low      */
#define   U_FST_C2  ( 0x9f )          /*  first kanji data check middole  */
#define   U_FST_C3  ( 0xe0 )          /*  first kanji data check middole  */
#define   U_FST_C4  ( 0xfc )          /*  first kanji data check high     */
#define   U_SEC_C1  ( 0x40 )          /*  second kanji data check low     */
#define   U_SEC_C2  ( 0x7e )          /*  second kanji data check middole */
#define   U_SEC_C3  ( 0x80 )          /*  second kanji data check middole */
#define   U_SEC_C4  ( 0xfc )          /*  second kanji data check high    */

#define   U_CHK_NL  ( 0x824f )        /*  DBCS Numeric  check Low  code   */
#define   U_CHK_NH  ( 0x8258 )        /*  DBCS Numeric  check High code   */
#define   U_CHK_ALL ( 0x8260 )        /*  DBCS ALPHABET check Low  code  */
#define   U_CHK_ALH ( 0x8279 )        /*  DBCS ALPHABET check High code  */
#define   U_CHK_ASL ( 0x8281 )        /*  DBCS alphabet check Low  code  */
#define   U_CHK_ASH ( 0x829a )        /*  DBCS alphabet check High code  */
#define   U_CHK_HL  ( 0x829f )        /*  DBCS Hiragana check Low  code  */
#define   U_CHK_HH  ( 0x82f1 )        /*  DBCS Hiragana check High code  */
#define   U_CHK_KL  ( 0x8340 )        /*  DBCS Katakana check Low  code  */
#define   U_CHK_KH  ( 0x8396 )        /*  DBCS Katakana check High code  */

#define   E_CHK_NL  ( 0xa3b0 )        /*  EUC Numeric  check Low  code   */
#define   E_CHK_NH  ( 0xa3b9 )        /*  EUC Numeric  check High code   */
#define   E_CHK_ALL ( 0xa3c1 )        /*  EUC ALPHABET check Low  code  */
#define   E_CHK_ALH ( 0xa3da )        /*  EUC ALPHABET check High code  */
#define   E_CHK_ASL ( 0xa3e1 )        /*  EUC alphabet check Low  code  */
#define   E_CHK_ASH ( 0xa3fa )        /*  EUC alphabet check High code  */
#define   E_CHK_HL  ( 0xa4a1 )        /*  EUC Hiragana check Low  code  */
#define   E_CHK_HH  ( 0xa4f3 )        /*  EUC Hiragana check High code  */
#define   E_CHK_KL  ( 0xa5a1 )        /*  EUC Katakana check Low  code  */
#define   E_CHK_KH  ( 0xa5f6 )        /*  EUC Katakana check High code  */

#define   U_KJ_NXT  (  1  )           /*  dl kanji next                   */
#define   U_KJ_SKP  (  2  )           /*  dl kanji read skip              */
#define   U_LO_COD  ( 0x80 )          /*  kanji data convert low data     */
#define   U_HI_COD  ( 0xc0 )          /*  kanji data convert high data    */
#define   U_CD_CNV  ( 0x30 )          /*  kanji data code check data      */

#define   U_ESC_CD  ( 0x1d )         /*  kana data  escape code           */
#define   U_PND_CD  ( 0x1e )         /*  kana data  pond   code           */
#define   U_COD_C1  ( 0x1a )         /*  kana data         minimum  code  */
#define   U_COD_C2  ( 0x74 )         /*  kana data         maximum  code  */
#define   U_COD_C3  ( 0x21 )         /*  kana data         minimum  code  */
#define   U_COD_C4  ( 0x5a )         /*  kana data         maximum  code  */

#define   U_PC_KJ0  ( 0x00 )         /*  pc kanji code are modify         */
#define   U_PC_KJ1  ( 0x10 )         /*  pc kanji code are modify         */
#define   U_PC_KJ2  ( 0x20 )         /*  pc kanji code are modify         */
#define   U_PC_KJ3  ( 0x30 )         /*  pc kanji code are modify         */
#define   U_CHOUON  ( 0x74 )         /*  PC code of chouon                */
#define   U_E3RST   ( 0x7a )         /*  reset code                       */
#define   U_MXYOLN  ( 10   )

#define   U_SLY     ( 0x59 )         /* SBCS Y code     */
#define   U_SSY     ( 0x79 )         /* SBCS y code     */
#define   U_DLY     ( 0x8278 )       /* DBCS Y code     */
#define   U_DSY     ( 0x8299 )       /* DBCS y code     */
#define   U_SLN     ( 0x4e )         /* SDCB N code     */
#define   U_SSN     ( 0x6e )         /* SDCB n code     */
#define   U_DLN     ( 0x826d )       /* SDCB N code     */
#define   U_DSN     ( 0x828e )       /* SDCB n code     */
#define   U_DHNN    ( 0x82f1 )       /* DBCS Hiragana NN code   */
#define   U_DHMI    ( 0x82dd )       /* DBCS Hiragana MI code   */
#define   U_SKNN    ( 0xdd )         /* SBCS Katakana NN code   */
#define   U_SKMI    ( 0xd0 )         /* SBCS Katakana MI code   */
#define   U_DKNN    ( 0x8393 )       /* DBCS Katakana NN code   */
#define   U_DKMI    ( 0x837e )       /* DBCS Katakana MI code   */

#define   U_HENE    ( 68   )
#define   U_FONE    ( 93   )
#define   U_P_UPM   ( 1    )         /* process mode of kudicuph          */
#define   U_P_ACWT  ( 2    )         /* process mode of kudicuph          */
#define   U_P_SEAR  ( 4    )         /* process mode of kudicuph          */
#define   U_P_IPCH  ( 5    )         /* process mode of kudicuph          */
#define   U_P_YOCH  ( 6    )         /* process mode of kudicuph          */
#define   U_P_GOCH  ( 7    )         /* process mode of kudicuph          */
#define   U_P_UPDT  ( 8    )         /* process mode of kudicuph          */
#define   U_P_CUMV  ( 9    )         /* process mode of kudicuph          */
#define   U_P_DEL   ( 10   )         /* process mode of kudicuph          */
#define   U_P_CAN   ( 11   )         /* process mode of kudicuph          */
#define   U_P_FIX   ( 12   )         /* process mode of kudicuph          */
#define   U_P_MSG   ( 13   )         /* process mode of kudicuph          */
#define   U_P_END   ( 14   )         /* process mode of kudicuph          */
#define   U_P_EMP   ( 15   )         /* process mode of kudicuph          */

/*
 *      process mode of kuipfld
 */
#define   T_GOKU    (   0  )         /* mode of inout field             */
#define   T_YOMI    (   1  )         /* mode of inout field             */
#define   T_FILE    (   2  )         /* mode of inout field             */

/*
 *      User Dictionary Message ID.
 */
#define U_AMSGN  (      1     )/* Initial.                                */
#define U_BMSGN  (      2     )/* End of Update.                          */
#define U_CMSGN  (      3     )/* End of File Recovery.                   */
#define U_DMSGN  (      4     )/* Need File Recovery.                     */
#define U_EMSGN  (      5     )/* Not Need File Recovery.                 */
#define U_FMSGN  (      6     )/* Access Error.                           */
#define U_GMSGN  (      7     )/* Invalid Key.                            */
#define U_HMSGN  (      8     )/* No Data.                                */
#define U_IMSGN  (      9     )/* Delete Data.                            */
#define U_JMSGN  (     10     )/* Input DBCS String for Yomi.             */
#define U_KMSGN  (     11     )/* Input Yomi and DBCS String.             */
#define U_LMSGN  (     12     )/* Input Yomi.                             */
#define U_MMSGN  (     13     )/* Complete Dictionary Registration.       */
#define U_NMSGN  (     14     )/* Already Exist in User Dictionary.       */
#define U_OMSGN  (     15     )/* Already Exist in System Dictionary.     */
#define U_PMSGN  (     16     )/* Invalid Character in Yomi.              */
#define U_QMSGN  (     17     )/* Overflow User Dictionary Registration   */
                               /* Area.                                   */
#define U_RMSGN  (     18     )/* No More Registration This Yomi.         */
#define U_SMSGN  (     19     )/* Same Data in Yomi and DBCS String,      */
                               /* Not Registration.                       */
#define U_TMSGN  (     20     )/* Query Delete Data in User Dictionary.   */
#define U_UMSGN  (     21     )/* Exit No Uppdate.                        */
#define U_VMSGN  (     22     )/* Input Yomi After Action.                */
#define U_WMSGN  (     23     )/* Not Regist this Yomi.                   */
#define U_XMSGN  (     24     )/* Too Long Length of Yomi.                */
#define U_YMSGN  (     25     )/* Same Data in Yomi and DBCS String,      */
                               /* Not Update.                             */
#define U_ZMSGN  (     26     )/* Failier of Recover Dictionary.          */
#define U_AAMSGN (     27     )/* No Data in User Dictionary,can not      */
                               /* Update.                                 */
#define U_ABMSGN (     28     )/* Already Exist Data in User Dictionary.  */
#define U_ACMSGN (     29     )/* "Hiragana" and Alphabet Contains in Yomi*/
#define U_ADMSGN (     30     )/* Query Delete Data in User Dictionary    */
                               /* ... In Case of Same Data in System      */
                               /* Dictionary.                             */
#define U_AEMSGN (     31     )/* Now Searching... Please Wait a Moment   */
#define U_AFMSGN (     32     )/* Now Updating...  Please Wait a Moment   */
#define U_AGMSGN (     33     )/* Access Error. User Dictionary.          */
#define U_AHMSGN (     34     )/* Access Error. System Dictionary.        */
#define U_AIMSGN (     35     )/* Access Error. Profile.                  */
#define U_AJMSGN (     36     )/* Too Long Yomi Length (EISUU)            */
#define U_AKMSGN (     37     )/* Now Updating Another One                */
#define U_ALMSGN (     38     )/* Do You Want to Update ? y/n             */
#define U_AMMSGN (     39     )/* Cancel Update.                          */
#define U_ANMSGN (     40     )/* Dictionary data to be empty.            */
#define U_AOMSGN (     41     )/* Please enterd F3 or F12 keys.           */
#define U_APMSGN (     42     )/* no data after recovery.                 */
#define U_AQMSGN (     43     )/* Recovering...                           */

#define U_ARMSGN (     44     )/* Please input file name.                 */
#define U_ASMSGN (     45     )/* this file will be updata.     */
#define U_ATMSGN (     46     )/* this file is not opened.                */
#define U_AUMSGN (     47     )/* Combineing...                           */
#define U_AVMSGN (     48     )/* End of combine.                         */
#define U_AWMSGN (     49     )/* Adding dictionary is not exist.         */
#define U_AXMSGN (     50     )/* not appoint equal name.                 */
#define U_AYMSGN (     51     )/* thid file is not user dictionary.       */
#define U_AZMSGN (     52     )/* End of write udict to file    	*/
#define U_BAMSGN (     53     )/* End of print out.     		*/
#define U_BBMSGN (     54     )/* Can not make temporary file for print */
#define U_BCMSGN (     55     )/* print error.  			*/
#define U_BDMSGN (     56     )/* this file is needed Recovery. 	*/
#define U_BEMSGN (     57     )/* this file is used.    		*/
#define U_BFMSGN (     58     )/* this file name is invalid.    	*/
#define U_BGMSGN (     59     )/* file comb success, but data delete    */
#define U_ADDING (     60     )/* Adding a entry 			*/
#define U_UPDATING (   61     )/* Updating entries 			*/
#define U_WRERROR (    62     )/* writing error 			*/
#define U_WRFORCE (    63     )/* force to write  			*/

#define U_FILEE  (   -1       )/* file io error                           */

#define U_STATUS (   0x1c02   )/* user dictionary file status code offset */


#define U_SREST  (     1      )/* save rstore                             */
#define U_HEIGHT (    20      )/* character height                        */
#define U_XSIZE  (   720      )/* crt x size                              */
#define U_YSIZE  (   512      )/* crt y size                              */
#define U_SEPA   (     2      )/* separate count                          */
#define U_XPIC   (     1      )/* width pixels entry number               */
#define U_YPIC   (     2      )/* height pixels entry number              */
#define U_QDSP   (    32      )/* gsqdsp entry count                      */
#define U_HFIELD (    10      )/* field count                             */

#define U_REGN   (  0x8231    )/* "1" (registration)                      */
#define U_UPDN   (  0x8232    )/* "2" (update)                            */
#define U_FOMN   (  0x8233    )/* "3" (ichiran)                           */
#define U_SETN   (  0x8234    )/* "4" (conbnation)                        */
#define U_RCVN   (  0x8235    )/* "5" (recovery)                          */
#define U_ENDN   (  0x8239    )/* "9" (end)                               */
#define U_NOP    (  0x8230    )/* no operation                            */

#define U_REPMOD (     3      )/* line mode                               */

#define U_FNMAX  (    30      )/* user dictionary file name max length    */
#define U_TRYLOK (    10      )/* retry times of file's lock              */

#define U_CLR    (      1     )/* Function No.1                           */
#define U_TRM    (     12     )/* Function No.12                          */
#define U_FKEY   (      2     )/* GSL Event Type.2                        */

#define U_INDLN  (      4     )/* Length of Indicator.                    */
#define U_YMFLD  (     20     )/* Maximum Length of Yomi String.          */
#define U_KJFLD  (     40     )/* Maximum Length of DBCS String.          */
#define U_MSFLD  (     64     )/* Maximum Length of Field Size.           */
#define U_MAXCOL (     54     )/* Maximum Colomn size of display          */
#define U_LSTCD  (     'q'    )/* GSL Type 2 Event Last Character.        */
                               /* Control Sequence.                       */
#define U_YSTLNX ( U_XP*10    )/* Yomi Field Underline Start X Offset.    */
#define U_YEDLNX ( U_XP*22    )/* Yomi Field Underline End   X Offset.    */
#define U_YLINEY ( U_YP*17-8  )/* Yomi Field Underline Start X Offset.    */

#define U_KSTLNX ( U_XP*10    )/* Yomi Field Underline Start X Offset.    */
#define U_KEDLNX ( U_XP*32    )/* Yomi Field Underline End   X Offset.    */
#define U_KLINEY ( U_YP*14-8  )/* Yomi Field Underline Start X Offset.    */
#define U_YMAX   (    10      )/* Maximum Length of Yomi(Unit By DBCS).   */

#define U_UDIL   (  0x1c00    )/* User Dictionary Index Length.           */
#define U_INITIL (      5     )/* Initial Value in Index Length.          */

/*      Module :  kuhtdc
 *      Signle Byte Data Code Interchange for DBCS Data Code Conversion
 *      Mode.
 */
#define U_SFTANK (     1      )/* DBCS Alphabet/Numeric Mode.             */
#define U_SFTKAT (     2      )/* DBCS Katakana Mode.                     */
#define U_SFTHIR (     3      )/* DBCS Hiragana Mode.                     */

/*
 *      DBCS Character Type.
 */
#define U_CHHIRA (      1     )/* Hiragana Include long lowel character.  */
#define U_CHKATA (      2     )/* Katakana.                               */
#define U_CHALPH (      3     )/* Alphabet.                               */
#define U_CHNUM  (      4     )/* Numeric.                                */
#define U_CHBLNK (      5     )/* Blank.                                  */
#define U_CHKIGO (      6     )/* Others & PC Kanji Code.                 */

#define U_CONVOF (      0     )/* Kana/Kanji Conversion OFF.              */
#define U_CONVON (      1     )/* Kana/Kanji Conversion ON.               */
#define U_MIXMOF (      0     )/* Field is DBCS Mode.                     */
#define U_MIXMON (      1     )/* Field is MIX  Mode.                     */
#define U_REP    (      0     )/* Field Editing Mode is Replace.          */
#define U_INS    (      1     )/* Field Editing Mode is Insert.           */

#define U_KNLLEN (      1     )/* KNL length                              */
#define U_KJLLEN (      1     )/* KNL length                              */
#define U_ILLEN  (      2     )/* IL  length                              */
#define U_STSLEN (      1     )/* STS length                              */
#define U_HARLEN (      1     )/* HAR length                              */
#define U_RRNLEN (      1     )/* RRN length                              */
#define U_NARLEN (      1     )/* NAR length.                             */
#define U_DLLEN  (      1     )/* DL  length                              */
#define U_RLLEN  (      2     )/* RL  length                              */
#define U_RSVLEN (      1     )/* RSV length                              */
#define U_UDCRSV (   0x00     )/* Reserve Area Data                       */
#define U_NOKAN  (      0     )/* Kana Data none                          */
#define U_NOKNJ  (      1     )/* Kana Data Exist & Kanji Data none       */
#define U_KANKNJ (      2     )/* Kana Data Exist & Kanji Data Exist      */
#define U_MODINQ (      0     )/*                                         */
#define U_DADDLF (      1     )/* ADD Mode for LEFT                       */
#define U_DADDRT (      2     )/* ADD Mode for RIGHT                      */
#define U_DADDNW (      3     )/* ADD Mode for NEW                        */
#define U_MINNAR (      8     )/*                                         */
#define U_BASNAR (      7     )/*                                         */
#define U_BACKWD (      1     )/* Direction of character movement.        */
#define U_FORWD  (      0     )/* Direction of character movement.        */
#define U_SPRF   (      50    )/* Persentage of Dictionary Spilit.        */
                               /* (Fraction).                             */
#define U_SPRD   (     100    )/* Persentage of Dictionary Spilit.        */
                               /* (Divisor).                              */

#define U_7PCCL  (    0x80    )/* 7bit-> PC Convertion Code.              */
#define U_7PCCU  (    0xc0    )/* 7bit-> PC Convertion Code.              */
#define U_7PCCC  (    0x1f    )/* 7bit-> PC Convertion Code.              */

#define U_KAN_MX (    255     )/* Kana  Data Max length                   */
#define U_KNJ_MX (    254     )/* Kana  Data Max length                   */
#define U_OMIT_C (   0x1e     )/* Omit Process check Data                 */
#define U_HAR_V1 (   0x39     )/* User Dictionary size HAR Data 1         */
#define U_HAR_V2 (   0x5b     )/* User Dictionary size HAR Data 2         */
#define U_HAR_V3 (   0x5d     )/* User Dictionary size HAR Data 3         */
#define U_HAR_V4 (   0xb2     )/* User Dictionary size HAR Data 4         */
#define U_HAR_V5 (   0xb4     )/* User Dictionary size HAR Data 5         */
#define U_HAR_V6 (   0xfe     )/* User Dictionary size HAR Data 6         */

#define U_MAXKNL (      40    )/* Yomigana Maximum Length                 */
#define U_MAXKJL (      10    )/* Kanji Maximum Length                    */
#define U_HSHDTL (       3    )/* 1 Hasshu Data Length                    */
#define U_SHSPOS (    128     )/* Position of System Dictionary Hash.     */
#define U_SDVERL (      2     )/* Length of System Dictionary Vertion     */
                               /* Number.                                 */
#define U_SDBKLN (   2048     )/* System Dictionary Blocking Size.        */
#define U_HASSHU (U_SDBKLN - U_SHSPOS - U_SDVERL)
                               /* Hasshu Data Length                      */
#define U_NOUSE  (  0xffffff  )/* Hasshu Data No Use                      */

#define U_B_YOMI (      20    )/* yomi length in buffer                   */
#define U_B_KAN  (      40    )/* kanji len. in buffer                    */
/*
 *      Dictionary Data Buffer Status Code.
 */
#define U_S_INIT (      0     )/* Initial Status.                         */
#define U_S_YOMD (      1     )/* Yomi Delete Status.                     */
#define U_S_KNJD (      2     )/* Kanji Delete Status.                    */
#define U_S_YOMA (      3     )/* Yomi Add Status.                        */
#define U_S_KNJU (      4     )/* Kanji Update Status.                    */
#define U_S_ADEL (      5     )/* Add(3) Delete Status.                   */

#define U_REC_L  (   1024L    )/*  1 recode size (1K)                     */
#define U_MRU_A  ( 7L*U_REC_L )/*  MRU area    7K                         */
#define U_INDX_A ( 3L*U_REC_L )/*  INDEX area  3K                         */
#define U_MRU_T  (      0     )/*  Top of MRU area                        */
#define U_INDX_T (    U_MRU_A )/*  Top of INDEX area                      */

#define U_KN_L   (      20    )/*  yomi length (7bit)                     */
#define U_KJ_L   (      40    )/*  kanji length (7bit)                    */
#define U_MSB_M  (      0x80  )/*  msb mask pattern                       */
#define U_MSB_O  (      0x3f  )/*  msb(2bits) off patt.                   */
#define U_M_OFF  (      0     )/*  msb off                                */
#define U_1KJ_L  (      2     )/*  1 kanji length                         */
#define U_EQUAL  (      0     )/*  memcmp equal code                      */

#define U_REGIST (      1     )/* mode (registration)                     */
#define U_ENT_P  (      2     )/* 1st kanji data point                    */
#define U_NAR_P  (      4     )/* nar position in index                   */
#define U_IND_P  (      5     )/* 1st entry in index                      */
#define U_MRU_P  (      2     )/* 1st entry in mru                        */

#define U_LSEEKE (      -1    )/*  lseek fails code                       */
#define U_READ_E (      -1    )/*  read fails code                        */
#define U_WRITEE (      -1    )/*  write fails code                       */

#define U_TPOSI  (       2    )/*  MRU data top                           */
#define U_ERR    (      -1    )/*  error code                             */
#define U_KNL    (       1    )/*  kana length                            */
#define U_KJL    (       1    )/*  kanji length                           */
#define U_MLEN   (       2    )/*  MRU length size                        */
#define U_FTOP   (       0    )/*  file top position                      */
#define U_ETOP   (       0    )/*  entry top position                     */
#define U_NORMAL (       0    )/*  normal return code                     */
#define U_STEPNO (       4    )/*  step number                            */
#define U_DMY    (       5    )/*  initial value dummy                    */
#define U_SHORT  (       2    )/*  short size                             */
#define U_CONV   (       0x3f )/*  convert bit                            */
#define U_STEP0  (       0    )/*  step number 0                          */
#define U_STEP1  (       1    )/*  step number 1                          */
#define U_STEP2  (       2    )/*  step number 2                          */
#define U_STEP3  (       3    )/*  step number 3                          */
#define U_STEP4  (       4    )/*  step number 4                          */
/*
#if defined(U_MAX)
#undef U_MAX
#endif
#define U_MAX    (       50   )--  insert data max                        --
*/
#define U_WORK   (       40   )/*  work bufer size                        */
#define U_MRUMAX (    0x1c00  )/*  MRU area max                           */

#define U_MINFLD (      1    )/* minimam filed No.                        */

#define U_DISP   (      0     )/* initial display process.      */
#define U_NEXTP  (      1     )/* next page process.    */
#define U_BEFORP (      2     )/* before page process.  */
#define U_RESET  (      11    )/* reset mode of kudicupm        */
#define U_REDRW  (      12    )/* redrow mode of kudicupm       */
#define U_REVER  (      13    )/* redrow & reverse mode of kudicupm     */

#define U_TNMMAX (     14     )/* user dictionary temporary file name     */
                               /* length max                              */
#define U_TNMOFF (     10     )/* user dictionary temporary file name     */
                               /* length offset                           */

#define U_YOMIF  (      0     )/* read field on display map.              */
#define U_GOKUF  (      1     )/* goku field on display map.              */
#define U_NODIS  (      -1    )/* no use field on display map.            */

#define U_XYOMI  (      5     )/* X axis of yomigana field.     */
#define U_XGOKU  (      11    )/* X axis of Goki field. */
#define U_XYAJI  (      7     )/* X axis of yajirusi.   */
#define U_HIGHT  (      27    )/* character hight.                        */
#define U_SDISP  (      439   )/* display area start Y axis.              */
#define U_YOMLEN (      20    )/* length of yomigana.   */
#define U_GOKLEN (      40    )/* lenght of goki.       */
#define U_YOMFLD ( U_YOMLEN+1 )/* length of yomigana field.     */
#define U_GOKFLD ( U_GOKLEN+1 )/* lenght of goki field. */
#define U_YAJLEN (      2     )/* length of yajirusi.   */

#define U_UPM_EL (     10     )/* erase left   x axis                     */
#define U_UPM_ER (    710     )/* erase right  x axis                     */
#define U_UPM_EB (     94     )/* erase bottom y axis                     */

#define U_ECHO   (      1     )/* echo mode (kuevwt)                      */
#define U_NECHO  (      0     )/* non echo mode (kuevwt)                  */

#define U_YESCD  (    0xf1    )/* return code (yes)                       */
#define U_NOCD   (    0xf2    )/* return code (no)                        */

/*
 *      GSL gsgtat_ Interface Parameter.
 */
#define U_5081DA (  0x0408    )/*         IBM5080 Display Id.           */
#define U_ALIGNH (      1     )/* Horizonal Alignment Normal.             */
#define U_ALIGNV (      1     )/* Vertical  Alignment Normal.             */
#define U_ATUNCH ( 0x80000000 )/* Attribute is unchanged.                 */
#define U_ATUNCI (     -1     )/* Attribute is unchanged.                 */
#define U_BASELN (      0     )/* 0 Degree or left to right in viwer's    */
                               /* terms.                                  */
#define U_EXPAN  ( 0x00000100 )/* No Text Expantion.                      */
#define U_FONT   ((char *)-1  )/* Unchanged font parameter.               */
#define U_FONTID (     -1     )/* Unchanged text font.                    */
#define U_PRE    (      1     )/* Character preceision.                   */
#define U_SPAC   (      0     )/* No Text Spacing.                        */
#define U_UPVCTX (      0     )/* Upper vector of text string(X-component)*/
#define U_UPVCTY (      1     )/* Upper vector of text string(Y-component)*/

#define IUBIAS   (    -6500   )/* Kanji Utility Internal Error Code.      */
#define IUSUCC   (      0     )/* Suucessful of Execution.                */
#define IUFAIL   ( IUBIAS-1   )/* Fail.                                   */
#define IUHTDCAN ( IUBIAS-2   )/* Invalid input parameter.                */
#define IUHTDERR ( IUBIAS-3   )/* Unsuccessful.                           */
#define IURECOV  ( IUBIAS-4   )/* recovery status                         */
#define INOMSGFL ( IUBIAS-5   )/* no message file                         */
#define IUUPDAT  ( IUBIAS-6   )/* now updating                            */

#define UDBIAS   (    -6000   )/* Kanji Utiliay Global Error Code.        */
#define UDSUCC   (      0     )/* Successful of Exction.                  */
#define UDCALOCE ( UDBIAS-1   )/*  calloc error                           */
#define UDLSEEKE ( UDBIAS-2   )/*  lseek error                            */
#define UDWRITEE ( UDBIAS-3   )/*  write error                            */
#define UDREADE  ( UDBIAS-4   )/*  read error                             */
#define UDIVHARE ( UDBIAS-5   )/* invarid HAR value return code           */
#define UDIVPARE ( UDBIAS-6   )/* invarid parameter kudicadp              */
#define UDDCEXTE ( UDBIAS-7   )/* kanji is exist in user dictionary       */
#define UDDCFULE ( UDBIAS-8   )/* data add space is none                  */
#define UDOVFDLE ( UDBIAS-9   )/* dl size is over                         */
#define UDNODTE  ( UDBIAS-10  )/* User Dictionary No Data.                */
#define UDRIMPE  ( UDBIAS-11  )/* Recovery impossible.                    */
#define UDRNVDW  ( UDBIAS-12  )/* Recovery Complete.                      */
#define UDRDPDW  ( UDBIAS-13  )/* Recovery Complete.                      */
#define UDDISPE  ( UDBIAS-14  )/* Display size error.                     */

#define UTSYDCOE ( KKSYDCOE   )/* System Dictionary Open Error.           */
#define UTUSDCOE ( KKUSDCOE   )/* User Dictionary Open Error.             */

#define U_DIVHK  (    -1      )/* Unsuccessful(Hiragana,Katakana).        */
#define U_DHIRA  (     1      )/* Hiragana Flag.                          */
#define U_DKATA  (     2      )/* Katakana Flag.                          */

#define U_EVDATA (     13     )/*           event data count              */
#define U_YOMILN (     24     )/*           yomi data length              */
#define U_EMSG   (     78     )/* Maximum Length of Error Message.        */
#define U_AFIELD (     13     )/*           field count                   */

#define U_XP     (     18     )/* Character Box Width. (Double Character) */
#define U_XPB    (      9     )/* Character Box Width. (Single Character) */
#define U_YP     (     20     )/* Character Box Height.                   */

#define U_F17LN  (     78     )/* Maximum Length of Field Message.        */

#define U_TEXT   (      7     )/* Foreground Color Index.                 */
#define U_BACK   (      0     )/* Background Color Index.                 */

#define U_NZERO  (     ~0     )/* None Zero Value.                        */

#define U_ENDID  (    9999    )/* Last Field ID.                          */

#define U_FON    (      1     )/* Logical Value On.                       */
#define U_FOF    (      0     )/* Logical Value Off.                      */
#define U_CHLOW  (      1     )/* Lower Character Position.               */

/*
 *      GSL Interface Event.
 */
#define U_KTYPE  (      0     )/* DBCS Event Type.                        */
#define U_FTYPE  (      2     )/* Control  Sequence Event Type.           */

#define U_EKEY   (      0     )/* Position of Event Reason Code.          */
#define U_ETYPE  (      4     )/* ######                                  */
#define U_KLEN   (      1     )/* Event Buffer Length.                    */

#define U_X114   ( 10 * U_XP  )/* line start x point                      */
#define U_X214   ( 30 * U_XP  )/* line end   x point                      */
#define U_Y114   (17*U_YP - 2 )/* line start y point                      */
#define U_Y214   (17*U_YP - 2 )/* line end   y point                      */

#define U_X116   ( 10* U_XP   )/* line start x point                      */
#define U_X216   ( 50 * U_XP  )/* line end   x point                      */
#define U_Y116   (14*U_YP - 2 )/* line start y point                      */
#define U_Y216   (14*U_YP - 2 )/* line end   y point                      */

/*
 *      Dictionary String Operation Mode.
 */
#define U_HIRA    (      0    )/* Hiragana Mode.                          */
#define U_CAPON   (      1    )/* Upper Case Alphanumeric Mode.           */
#define U_KATA    (      2    )/* Katakana Mode.                          */
#define U_CAPOFF  (      3    )/* Lower Case Alphanumeric Mode.           */
#define U_HEMIX   (      4    )/* Mix Mode.                               */
#define U_INVALD  (      5    )/* Invalid Mode.                           */

/*------------------------------------------------------------------------*
 * The maximum number of multiple system dictionary
 *------------------------------------------------------------------------*/
#define	MAX_SYSDICT	16

/*
 *      User Dictionary Control Structure.
 */
typedef struct udcs UDCS;
struct  udcs {
        UDCS    *pr_pos;        /* Pointer to Previous UDCS.              */
        UDCS    *nx_pos;        /* Pointer to Next     UDCS.              */
        uchar   status;         /* Status Information.                    */
        uchar   yomilen;        /*   yomi length                          */
	uchar   yomi[U_YOMFLD]; /*   yomi       */
        uchar   pos;            /*   position                             */
        uchar   kanlen;         /*   kanji length                         */
	uchar   kan[U_GOKFLD];  /*   kanji      */
        long    ycaddflg;       /* add flag when Yomi changed.            */
        long    rsv3;           /* **** RESERVED FOR FUTURE USE ****      */
        long    rsv4;           /* **** RESERVED FOR FUTURE USE ****      */
        long    rsv5;           /* **** RESERVED FOR FUTURE USE ****      */
        long    rsv6;           /* **** RESERVED FOR FUTURE USE ****      */
        long    rsv7;           /* **** RESERVED FOR FUTURE USE ****      */
        long    rsv8;           /* **** RESERVED FOR FUTURE USE ****      */
};
/*
 *      User Dictionary Display Buffer.
 */
typedef struct udfield UDMFLD;
struct udfield {
	short   rsv4;           /* ** RESERVED MODE ** from Field No.   */
        short   fstat;          /* Field Status.                          */
        UDCS    *dbufpt;        /* Pointer to User Dictionary Buffer.     */
	long    rsv5;           /* ** RESERVED MODE ** from chclr.      */
	long    rsv6;           /* ** RESERVED MODE ** from bkclr.      */
        long    rsv1;           /* **** RESERVED FOR FUTURE USE ****      */
        long    rsv2;           /* **** RESERVED FOR FUTURE USE ****      */
        long    rsv3;           /* **** RESERVED FOR FUTURE USE ****      */
};
typedef struct udms UDMS;
struct  udms {
        short   prestat;        /* Previous Display Status.               */
        short   rsv1;           /* **** RESERVED FOR FUTURE USE ****      */
        UDMFLD  *fld;           /* Field Status.                          */
        short   poststat;       /* Post Display Status.                   */
        short   rsv2;           /* **** RESERVED FOR FUTURE USE ****      */
};

/*
 *      Error Message Handling.
 */
typedef struct uecb UECB;
struct uecb {
        short   id;             /* Error Message ID.                      */
        uchar   msg[U_EMSG];    /* Error Message.                         */
};

/*
 *      System Dicionary Access Pointer.(Dictionary is Memory Mapped File).
 */
typedef struct sdcb SDCB;
struct  sdcb  {
	off_t   st_size;                /* File size in bytes */
        uchar   *dcptr; /* Pointer to Dcitionary Base Address.            */
        uchar   *rdptr; /* Pointer to Current Read  Position.             */
        uchar   *wtptr; /* Pointer to Current Write Position.             */
};

/*
 *      User Dicionary Access Pointer.(Dictionary is Memory Mapped File).
 */
typedef struct udcb UDCB;
struct  udcb  {
        uchar   *dcptr; /* Pointer to Dcitionary Base Address.          */
        uchar   *rdptr; /* Pointer to Current Read  Position.           */
        uchar   *wtptr; /* Pointer to Current Write Position.           */
        UECB    *erptr; /* Pointer to Error Information BLock.          */
	uchar   *rsv1;  /* Pointer to DBCS Editor Control  Block.       */
        uchar   *udfname;
                        /* Pointer to User Dictionary File Name.        */
                        /* (Terminate By NULL Character)                */
	uchar   *secbuf;
                        /* Auxiliary Buffer for User Dictionary.        */
	uchar   *thdbuf;
                        /* Backup Buffer for User Dictionary.        	*/
        long    updflg; /* Update Flag.                                 */
        long    ufilsz; /* User Dictionary File Size (byte)             */
        ushort  uurmf;  /* Update Uty Return Mode Flag.                 */
        ushort  ymax;   /* Maximum Number of Coordinate Y.              */
        ushort  xoff;   /* Screen X offset.                             */
        ushort  yoff;   /* Screen Y offset.                             */
	int     orgfd;  /* user ductionary file discripter      	*/
	uchar   *tmpname; /* user dictionary temporally name. ( *.bak ) */
	int     tmpfd;  /* user ductionary file discripter ( *.bak )   	*/
};
/*
 *      Caution. SDCB,UDCB Strucrure is Conatins Common Information
 *      Area.  ( Same Mean Below. )
 *
 *      struct dcb {
 *              uchar   *dcptr; * Pointer to Dcitionary Base Address.
 *              uchar   *rdptr; * Pointer to Current Read  Position.
 *              uchar   *wtptr; * Pointer to Current Write Position.
 *              union {
 *                       *
 *                       *      User Dicaionary Special Information.
 *                       *
 *                      struct {
 *                              UDCB    *erptr; * Pointer to Error Info.
 *                              char    *dp;    * Pointer to DBCS Editor
 *                              char    *udfname;
 *                                              * Pointer to User Dic.
 *                              ushort  xoff;   * Screen X offset.
 *                              ushort  yoff;   * Screen Y offset.
 *                      } user;
 *              };
 *      };
 *
 *
 */
/*
 *      Input Field of display data.
 */
typedef struct  difld   DIFLD;
struct  difld {
	char yofld[U_YOMFLD];
	char gofld[U_GOKFLD];
};
#endif

