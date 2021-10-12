/* @(#)28       1.1  src/bos/usr/lpp/kls/dictutil/hut.h, cmdkr, bos411, 9428A410j 5/25/92 14:48:18 */
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hut.h
 *
 * ORIGINS :		27
 *
 * (C) COPYRIGHT International Business Machines Corp.  1991, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************************************************************
 *
 *  Component:    Korean IM User Dictionary Utility
 *
 *  Module:       hut.h
 *
 *  Description:  User Dictionary Maintenance Utility General
 *                Constant Values.
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/

#ifndef _HUT_H_
#define _HUT_H_

/*-------------------------------------------------------------------------*/
/*		Include headers.					   */
/*-------------------------------------------------------------------------*/

#include <nl_types.h>
#include "humacros.h"           /* Hanja Utility Macros.                */
#include "hucommon.h"           /* Hanja Utility Common Define File.    */
#include "huconst.h"
#include "hhccomm.h"
#include "huke.h"               /* Hanja Utility key input.             */

/* standerd err out file        */
#define   ERR_FILE  ("/tmp/..hjdict.err")

/* print out command */
#ifndef PRINT
#define PRINT "sh -c '(/bin/qprt %s)'"
#endif
#define   U_SD_0    ( 0 )    /* switch statement case no. : <0> key only */
#define   U_SD_1    ( 1 )    /* switch statement case no. : <1> key & cadidate */
#define   U_SD_2    ( 2 )    /* switch statement case no. : <2> gerric    */
#define   U_FRONT   ( 1 )    /* current flag : front current              */
#define   U_REVES   ( 2 )    /* current flag : front current              */

#define   U_BAS_IL  (  7  )     /*  il base  value                        */
#define   U_DL_MN   (  3  )     /*  dl minimum value                      */
#define   U_DL_MX   ( 255 )     /*  dl maximum value                      */
#define   U_NAR_V1  (  3  )     /*  nar initial value  ( 1kb )            */
#define   U_NAR_V2  (  52  )     /*  nar initial value  ( 2kb )            */
#define   U_DL_ARA  ( 256 )     /*  dl data area  length                  */
#define   U_ST_CD0  ( 0 )              /* set recovery code  data         */
#define   U_ST_CD1  ( 1 )              /* set recovery code  data         */
#define   U_ST_CD2  ( 2 )              /* set recovery code  data         */
#define   U_ST_CD5  ( 5 )              /* set recovery code  data         */
#define   U_ST_CD6  ( 6 )              /* set recovery code  data         */
#define   U_ST_CD7  ( 7 )              /* set recovery code  data         */
#define   U_BUFFUL  ( 20  )            /* local hangeul data buffer size  */
#define   U_BUFPTR  ( 2  )             /* local pointer  initialize       */
#define   U_ID_STS  ( 2  )             /* index sts                       */
#define   U_IL_HED  ( 5  )             /* index header length             */
#define   U_MN_KEYL  ( 2  )             /* keyl   minimum  length           */
#define   U_MX_KEYL  ( 11 )             /* keyl   maximum  length           */
#define   U_MN_CANDL  ( 2  )             /* candl   minimum  length           */
#define   U_MX_CANDL  ( 41 )             /* candl   maximum  length           */
#define   U_DL_MIN  ( 3  )             /* dl    minimum  length           */
#define   U_HARIV1  ( 0x08 )           /* har   check                     */
#define   U_HARIV2  ( 0x09 )           /* har   check                     */
#define   U_HARIV3  ( 0x0a )           /* har   check                     */
#define   U_BL_HED  ( 2 )              /* bl header position              */
#define   U_NARMIN  ( 3 )              /* nar   minimum  length           */
#define   U_DL_HED  ( 0 )              /* dl head position                */
#define   U_DL_RSV  ( 1 )              /* dl reserve length               */

#define   U_RBN_L1  ( 1 )             /*  rbn skip                        */
#define   U_BLCK_L1  ( 1024 )          /*    1 block size ( 1K )         */
#define   U_BLCK_L2  ( 2048 )          /*    2 block size ( 2K )         */
#define   U_BLCK_L3  ( 3072 )          /*    3 block size ( 3K )         */

#define   U_SEC_C1  ( 0x40 )          /*  second kanji data check low     */
#define   U_SEC_C2  ( 0x7e )          /*  second kanji data check middle */
#define   U_SEC_C3  ( 0x80 )          /*  second kanji data check middle */
#define   U_SEC_C4  ( 0xfc )          /*  second kanji data check high    */

#define   U_CHK_NL  ( 0xa3b0 )
#define   U_CHK_NH  ( 0xa3b9 )
#define   U_CHK_HL  ( 0xb0a1 )
#define   U_CHK_HH  ( 0xc8fe )

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


#define   U_E3RST   ( 0x7a )         /*  reset code                       */
#define   U_MXYOLN  ( 10   )

#define   U_SLY     ( 0x59 )         /* SBCS Y code     */
#define   U_SSY     ( 0x79 )         /* SBCS y code     */
#define   U_DLY     ( 0xa3d9 )       /* DBCS Y code     */
#define   U_DSY     ( 0xa3f9 )       /* DBCS y code     */
#define   U_SLN     ( 0x4e )         /* SBCS N code     */
#define   U_SSN     ( 0x6e )         /* SBCS n code     */
#define   U_DLN     ( 0xa3ce )       /* DBCS N code     */
#define   U_DSN     ( 0xa3ee )       /* DBCS n code     */
#define   U_DHNN    ( 0x82f1 )       /* DBCS Hiragana NN code   */
#define   U_DHMI    ( 0x82dd )       /* DBCS Hiragana MI code   */
#define   U_SKNN    ( 0xdd )         /* SBCS Katakana NN code   */
#define   U_SKMI    ( 0xd0 )         /* SBCS Katakana MI code   */
#define   U_DKNN    ( 0x8393 )       /* DBCS Katakana NN code   */
#define   U_DKMI    ( 0x837e )       /* DBCS Katakana MI code   */

#define   U_HENE    ( 68   )
#define   U_FONE    ( 93   )
#define   U_P_UPM   ( 1    )         /* process mode of hudicuph          */
#define   U_P_ACWT  ( 2    )         /* process mode of hudicuph          */
#define   U_P_SEAR  ( 4    )         /* process mode of hudicuph          */
#define   U_P_IPCH  ( 5    )         /* process mode of hudicuph          */
#define   U_P_KEYCH ( 6    )         /* process mode of hudicuph          */
#define   U_P_CANCH ( 7    )         /* process mode of hudicuph          */
#define   U_P_UPDT  ( 8    )         /* process mode of hudicuph          */
#define   U_P_CUMV  ( 9    )         /* process mode of hudicuph          */
#define   U_P_DEL   ( 10   )         /* process mode of hudicuph          */
#define   U_P_CAN   ( 11   )         /* process mode of hudicuph          */
#define   U_P_FIX   ( 12   )         /* process mode of hudicuph          */
#define   U_P_MSG   ( 13   )         /* process mode of hudicuph          */
#define   U_P_END   ( 14   )         /* process mode of hudicuph          */
#define   U_P_EMP   ( 15   )         /* process mode of hudicuph          */

/*
 *      process mode of huipfld
 */
#define   T_CAND    (   0  )         /* mode of inout field             */
#define   T_KEY    (   1  )         /* mode of inout field             */
#define   T_FILE    (   2  )         /* mode of inout field             */

/*
 *      User Dictionary Message ID.
 */
#define U_AMSGN  (      1     )/* Initial loading.                       */
#define U_BMSGN  (      2     )/* The dictionary file is updated.        */
#define U_CMSGN  (      3     )/* All data is collect.                   */
#define U_DMSGN  (      4     )/* The dictionary file needs recovering.  */
#define U_EMSGN  (      5     )/* Nothing is required for recovery.      */
#define U_FMSGN  (      6     )/* Can not access user dictionary.        */
#define U_GMSGN  (      7     )/* Invalid key has been pressed.          */
#define U_HMSGN  (      8     )/* There is no data in user dictionary.   */
#define U_IMSGN  (      9     )/* Some data has been deleted.            */
#define U_JMSGN  (     10     )/* Input Candidate. */
#define U_KMSGN  (     11     )/* Input Key and its Candidates.    */
#define U_LMSGN  (     12     )/* Input Key.                             */
#define U_MMSGN  (     13     )/* The registration has been completed.   */
#define U_NMSGN  (     14     )/* Already Exists in User Dictionary.     */
#define U_OMSGN  (     15     )/* Already Exists in System Dictionary.   */
#define U_PMSGN  (     16     )/* Invalid characters exist in Key.       */
#define U_QMSGN  (     17     )/* The dictionary file size exceeds the limit */
#define U_RMSGN  (     18     )/* Number of candidates exceeds the limit.    */
#define U_SMSGN  (     19     )/* The Key and the Candidate should not be the same */
#define U_TMSGN  (     20     )/* The data will be deleted. Press Enter to continue */
#define U_UMSGN  (     21     )/* Update was cancelled. press Enter to go main menu */
#define U_VMSGN  (     22     )/* Enter after Key Input.       */
#define U_WMSGN  (     23     )/* Not Regist this Key.          */
#define U_XMSGN  (     24     )/* Length of the Key exceeds the limit(10 char).     */
#define U_YMSGN  (     25     )/* The Key and the Candidate should not be the same */
#define U_ZMSGN  (     26     )/* Failed in dictionary file recovery!!!.            */
#define U_AAMSGN (     27     )/* There is no data in user dictionary, so you can't update      */
#define U_ABMSGN (     28     )/* Already Exists Data in User Dictionary.  */
#define U_ACMSGN (     29     )/* The Key must consists of only hangul or only number */ 
#define U_ADMSGN (     30     )/* The data will be deleted. Press Enter to continue */
#define U_AEMSGN (     31     )/* Now Searching... Please Wait a Moment   */
#define U_AFMSGN (     32     )/* Now Updating...  Please Wait a Moment   */
#define U_AGMSGN (     33     )/* Can not access user dictionary.          */
#define U_AHMSGN (     34     )/* Can not access system dictionary.        */
#define U_AIMSGN (     35     )/* can not access profile.                  */
#define U_AJMSGN (     36     )/* Length of the Key exceeds the limit(10 char)   */
#define U_AKMSGN (     37     )/* The dictionary is now in use        */
#define U_ALMSGN (     38     )/* Your update will not be applied.  Enter = Start F12 = Exit    */
#define U_AMMSGN (     39     )/* Update has been cancelled.               */
#define U_ANMSGN (     40     )/* No data left in dictionary file.            */
#define U_AOMSGN (     41     )/* Please press F3 or F12 key.           */
#define U_APMSGN (     42     )/* No data could be recovered.                 */
#define U_AQMSGN (     43     )/* Recovering...                           */
#define U_ARMSGN (     44     )/* Please input file name.                 */
#define U_ASMSGN (     45     )/* File name already exists. Enter = Do  F12 = Cancel */
#define U_ATMSGN (     46     )/* File access permission denied.                */
#define U_AUMSGN (     47     )/* Combining...                           */
#define U_AVMSGN (     48     )/* The file combination was completed.     */
#define U_AWMSGN (     49     )/* Additional dictionary is not exist.         */
#define U_AXMSGN (     50     )/* It is not allowed to combine the same files.  */
#define U_AYMSGN (     51     )/* second file is not user dictionary.       */
#define U_AZMSGN (     52     )/* Finished writing user dictionary to the above file    */
#define U_BAMSGN (     53     )/* Finished printing user dictionary.     */
#define U_BBMSGN (     54     )/* File access permission denied */
#define U_BCMSGN (     55     )/* Printing Error occurred.  */
#define U_BDMSGN (     56     )/* The dictionary file needs recovering. */
#define U_BEMSGN (     57     )/* New dictionary file is now in use.    */
#define U_BFMSGN (     58     )/* this file name is invalid.    */
#define U_BGMSGN (     59     )/* File combination successed, but some data deleted */
#define U_BHMSGN (     60     )/* Length of the Candidate exceeds the limit(10 char) */
#define U_BIMSGN (     61     )/* Additional dictionary and new dictionary should not be same */
#define U_BJMSGN (     62     )/* Total length of candidates of A Key exceeds the limit */
#define U_DERRMSGN (   63     )/* display size err */
#define U_TITLEMSGN (  64     )/* menu title name  for display */
#define U_SELMSGN (    65     )/* Move cursor to desired item and press Enter */
#define U_FOOTMSGN (   66     )/* Enter = Do */
#define U_MNU1MSGN (   67     )/* 1. Registration */ 
#define U_MNU2MSGN (   68     )/* 2. Update(replacement, deletion) */
#define U_MNU3MSGN (   69     )/* 3. List */
#define U_MNU4MSGN (   70     )/* 4. Combination */
#define U_MNU5MSGN (   71     )/* 5. Recovery */
#define U_MNU6MSGN (   72     )/* 9. Exit */
#define U_ERRFSMSGN (  73     )/* File Seek Error */
#define U_ERRSWMSGN (  74     )/* Status Write Error */
#define U_ERRFWMSGN (  75     )/* File Write Error */
#define U_USRFNMSGN (  76     )/* (dictionary : user dict name */
#define U_F11MSGN (    77     )/* ** User Dictionary Registration ** */
#define U_F13MSGN (    78     )/* Enter after key and candidate input */
#define U_F14MSGN (    79     )/* key : [                    ] */
#define U_F15MSGN (    80     )/* candidate : [                    ] */
#define U_F17MSGN (    81     )/* Enter = Register */
#define U_F18MSGN (    82     )/* F3 = End */
#define U_F19MSGN (    83     )/* F5 = Clear */
#define U_F1MSGN (     84     )/* ** User Dictionary Combination ** */
#define U_F2MSGN (     85     )/* This function combines two user dictionaries */
#define U_F3MSGN (     86     )/* Enter the file names */
#define U_F4MSGN (     87     )/* Original User Dictionary */
#define U_F6MSGN (     88     )/* Additional User Dictionary */
#define U_F8MSGN (     89     )/* New User Dictionary */
#define U_F10MSGN (    90     )/* Enter = Combine  F3 = End */
#define U_UHDMSGN (    91     )/* ** User Dictionary Update ** */
#define U_UF1MSGN (    92     )/* F2 = Delete */
#define U_UF2MSGN (    93     )/* F3 = End */
#define U_UF3MSGN (    94     )/* F7 = Previous */
#define U_UF4MSGN (    95     )/* F8 = Next */
#define U_UF5MSGN (    96     )/* F9 = Search */
#define U_UF6MSGN (    97     )/* F12 = Cancel */
#define U_LHDMSGN (    98     )/* ** User Dictionary List ** */
#define U_LNM1MSGN (   99     )/*  1. Screen   */
#define U_LNM2MSGN (   100    )/*  2. Printer  */
#define U_LNM3MSGN (   101    )/*  3. File     */
#define U_LTX1MSGN (    102    )/* This function lists content of dictionary */
#define U_LTX2MSGN (    103    )/* Move cursor to desired item and press Enter */
#define U_HD3MSGN (     104    )/* ** User Dictionary Display ** */

#define U_FILEE  (    -1      )/* file io error                           */
#define U_TRYLOK (     5      )/* retry times of file's lock              */

#define U_MAXCOL (     65     )/* Maximum Colomn size of display          */
                               /* Control Sequence.                       */

#define U_MODINQ (      0     )/*                                         */
#define U_DADDLF (      1     )/* ADD Mode for LEFT                       */
#define U_DADDRT (      2     )/* ADD Mode for RIGHT                      */
#define U_DADDNW (      3     )/* ADD Mode for NEW                        */
#define U_BACKWD (      1     )/* Direction of character movement.        */
#define U_FORWD  (      0     )/* Direction of character movement.        */
#define U_SPRF   (      50    )/* Persentage of Dictionary Spilit.        */
                               /* (Fraction).                             */
#define U_SPRD   (     100    )/* Persentage of Dictionary Spilit.        */

#define U_HAR_V1 (   0x06     )/* User Dictionary size HAR Data 1         */
#define U_HAR_V2 (   0x34     )/* User Dictionary size HAR Data 2         */

#define U_B_KEY (      20    )/* Key length in buffer                   */
#define U_B_CAND (      40    )/* Candidate len. in buffer                    */
/*
 *      Dictionary Data Buffer Status Code.
 */
#define U_S_INIT (      0     )/* Initial Status.                         */
#define U_S_KEYD (      1     )/* Key Delete Status.            */
#define U_S_CAND (      2     )/* Candidate Delete Status.                    */
#define U_S_KEYA (      3     )/* Key Add Status.               */
#define U_S_CANU (      4     )/* Candidate Update Status.                    */
#define U_S_ADEL (      5     )/* Add(3) Delete Status.                   */

#define U_REGIST (      1     )/* mode (registration)                     */

#define U_TPOSI  (       2    )/*  MRU data top                           */
#define U_ERR    (      -1    )/*  error code                             */
#define U_STEPNO (       4    )/*  step number                            */
#define U_STEP0  (       0    )/*  step number 0                          */
#define U_STEP1  (       1    )/*  step number 1                          */
#define U_STEP2  (       2    )/*  step number 2                          */
#define U_STEP3  (       3    )/*  step number 3                          */
#define U_STEP4  (       4    )/*  step number 4                          */

#define U_DISP   (      0     )/* initial display process.      */
#define U_NEXTP  (      1     )/* next page process.    */
#define U_BEFORP (      2     )/* before page process.  */
#define U_RESET  (      11    )/* reset mode of hudicupm        */
#define U_REDRW  (      12    )/* redraw mode of hudicupm       */
#define U_REVER  (      13    )/* redraw & reverse mode of hudicupm     */

#define U_TNMOFF (     10     )/* user dictionary temporary file name     */
                               /* length offset                           */
#define U_TNMMAX (     24     )/* user dictionary temporary file name     */
                               /* length max = U_TNMOFF + strlen(".tmp")  */

#define U_KEYF  (      0     )/* read field on display map.              */
#define U_CANDF  (      1     )/* goku field on display map.              */
#define U_NODIS  (      -1    )/* no use field on display map.            */

/*------------------------------------------------------------------------*/
/*		These Macro uesed at hudicupm().			  */
/*------------------------------------------------------------------------*/

#define U_XKEY  (      5     )/* X axis of Key field.     */
#define U_XCAND  (      11    )/* X axis of Goki field. */
#define U_XARR  (      7     )/* X axis of arrow.   */
#define U_HIGHT  (      27    )/* character hight.                        */
#define U_SDISP  (      439   )/* display area start Y axis.              */
#define U_KEYLEN (      20    )/* length of Key.   */
#define U_CANLEN (      40    )/* lenght of goki.       */
#define U_KEYFLD ( U_KEYLEN+1 )/* length of Key field.     */
#define U_CANFLD ( U_CANLEN+1 )/* lenght of goki field. */
#define U_ARRLEN (      2     )/* length of arrow.   */

#define U_ECHO   (      1     )/* echo mode (huevwt)                      */
#define U_NECHO  (      0     )/* non echo mode (huevwt)                  */
#define U_EMSG   (      78    )/* Maximum Length of Error Message.        */

#define IUBIAS   (    -6500   )/* Dictionary Utility Internal Error Code. */
#define IUSUCC   (      0     )/* Suucessful of Execution.                */
#define IUFAIL   ( IUBIAS-1   )/* Fail.                                   */
#define IUHTDCAN ( IUBIAS-2   )/* Invalid input parameter.                */
#define IUHTDERR ( IUBIAS-3   )/* Unsuccessful.                           */
#define IURECOV  ( IUBIAS-4   )/* recovery status                         */
#define INOMSGFL ( IUBIAS-5   )/* no message file                         */
#define IUUPDAT  ( IUBIAS-6   )/* now updating                            */

/*------------------------------------------------------------------------*/
/*		These Macro uesed at hudicrcv().			  */
/*------------------------------------------------------------------------*/

#define UDBIAS   (    -6000   )/* Dictionary Utiliay Global Error Code.   */
#define UDSUCC   (      0     )/* Successful of Exction.                  */
#define UDRNEED  (      1     )/* Recovery is needed.                     */
#define UDCALOCE ( UDBIAS-1   )/*  calloc error                           */
#define UDLSEEKE ( UDBIAS-2   )/*  lseek error                            */
#define UDWRITEE ( UDBIAS-3   )/*  write error                            */
#define UDREADE  ( UDBIAS-4   )/*  read error                             */
#define UDIVHARE ( UDBIAS-5   )/* invalid HAR value return code           */
#define UDIVPARE ( UDBIAS-6   )/* invalid parameter hudicadp              */
#define UDDCEXTE ( UDBIAS-7   )/* Candidate is exist in user dictionary       */
#define UDDCFULE ( UDBIAS-8   )/* data add space is none                  */
#define UDOVFDLE ( UDBIAS-9   )/* dl size is over                         */
#define UDNODTE  ( UDBIAS-10  )/* User Dictionary No Data.                */
#define UDRIMPE  ( UDBIAS-11  )/* Recovery impossible.                    */
#define UDRNVDW  ( UDBIAS-12  )/* Recovery Complete.                      */
#define UDRDPDW  ( UDBIAS-13  )/* Recovery Complete.                      */
#define UDDISPE  ( UDBIAS-14  )/* Display size error.                     */
#define UDCANDE  ( UDBIAS-15  )/* candidate size overflow.                */

#define UTSYDCOE ( KKSYDCOE   )/* System Dictionary Open Error.           */
#define UTUSDCOE ( KKUSDCOE   )/* User Dictionary Open Error.             */

/*------------------------------------------------------------------------*/
/*		This Macro uesed at hudisply().			          */
/*------------------------------------------------------------------------*/
#define U_ENDID  (    9999    )/* Last Field ID.                          */

#define U_FON    (      1     )/* Logical Value On.                       */
#define U_FOF    (      0     )/* Logical Value Off.                      */

/*------------------------------------------------------------------------*/
/*       	These Macro used at hudickc().				  */
/*------------------------------------------------------------------------*/
#define U_HANGEUL    (      0    )/* Hangeul Mode.                        */
#define U_NUMERIC    (      1    )/* Numeric Mode.                        */
#define U_HNMIX      (      2    )/* Hangeul and Numeric Mix Mode.        */
#define U_INVALD     (      3    )/* Invalid Mode.                        */

/*------------------------------------------------------------------------*/
/*      User Dictionary Control Structure.			   	  */
/*------------------------------------------------------------------------*/
typedef struct udcs UDCS;
struct  udcs {
        UDCS    *pr_pos;        /* Pointer to Previous UDCS.              */
        UDCS    *nx_pos;        /* Pointer to Next     UDCS.              */
        uchar   status;         /* Status Information.                    */
        short   keylen;         /*   hangeul length                       */
	uchar   key[U_KEY_MX];  /*   hangeul       		 	  */
        short   candlen;        /*   hangeul length                       */
	uchar   cand[U_CAN_MX]; /*   hangeul       			  */
        short   pos;            /*   position                             */
        long    kcaddflg;       /* add flag when Key changed.             */
};

/*------------------------------------------------------------------------*/
/*      User Dictionary Display Buffer.				 	  */
/*------------------------------------------------------------------------*/
typedef struct udfield UDMFLD;
struct udfield {
        short   fstat;          /* Field Status.                          */
        UDCS    *dbufpt;        /* Pointer to User Dictionary Buffer.     */
};
typedef struct udms UDMS;
struct  udms {
        short   prestat;        /* Previous Display Status.               */
        UDMFLD  *fld;           /* Field Status.                          */
        short   poststat;       /* Post Display Status.                   */
};

/*------------------------------------------------------------------------*/
/*      Error Message Handling.						  */
/*------------------------------------------------------------------------*/
typedef struct uecb UECB;
struct uecb {
        short   id;             /* Error Message ID.                      */
        uchar   msg[U_EMSG];    /* Error Message.                         */
};

/*-------------------------------------------------------------------------*/
/*      System Dicionary Access Pointer.(Dictionary is Memory Mapped File).*/
/*-------------------------------------------------------------------------*/
typedef struct sdcb SDCB;
struct  sdcb  {
        uchar   *dcptr; /* Pointer to Dcitionary Base Address.            */
        uchar   *rdptr; /* Pointer to Current Read  Position.             */
        uchar   *wtptr; /* Pointer to Current Write Position.             */
};

/*-------------------------------------------------------------------------*/
/*      User Dicionary Access Pointer.(Dictionary is Memory Mapped File).  */
/*-------------------------------------------------------------------------*/
typedef struct udcb UDCB;
struct  udcb  {
        uchar   *dcptr; /* Pointer to Dcitionary Base Address.            */
        uchar   *rdptr; /* Pointer to Current Read  Position.             */
        uchar   *wtptr; /* Pointer to Current Write Position.             */
        nl_catd msg_catd ;  /* Message catalog descriptor.                */
        uchar   *udfname;
                        /* Pointer to User Dictionary File Name.          */
                        /* (Terminate By NULL Character)                  */
	uchar   *secbuf;
                        /* Auxiliary Buffer for User Dictionary.          */
        long    updflg; /* Update Flag.                                   */
        long    ufilsz; /* User Dictionary File Size (byte)               */
        ushort  uurmf;  /* Update Uty Return Mode Flag.                   */
        ushort  ymax;   /* Maximum Number of Coordinate Y.                */
        ushort  xoff;   /* Screen X offset.                               */
        ushort  yoff;   /* Screen Y offset.                               */
	int     orgfd;  /* user ductionary file discripter      */
	uchar   *tmpname; /* user dictionary temporally name.             */
};

/*-------------------------------------------------------------------------*/
/*      Input Field of display data.					   */
/*-------------------------------------------------------------------------*/
typedef struct  difld   DIFLD;
struct  difld {
	char keyfld[U_KEY_MX];
	char candfld[U_CAN_MX];
};

#endif
/*-------------------------------------------------------------------------*/
/*			End of _HUT_H_					   */
/*-------------------------------------------------------------------------*/
