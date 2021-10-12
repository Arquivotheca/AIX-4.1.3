/* @(#)45	1.3.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcmap.h, libKJI, bos411, 9428A410j 7/23/92 03:16:19	*/

/*    
 * COMPONENT_NAME: (libKJI) Japanese Input Method 
 *
 * FUNCTIONS: kana-Kanji-Conversion (KKC) Library
 * 
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when 
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kcmap.h 
 *
 * DESCRIPTIVE NAME:  DEFINE VALUES FOR GLOBAL USE
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       
 *
 ******************** END OF SPECIFICATIONS *****************************/

/*----------------------------------------------------------------------*
 * This file is used to set values to global source variables.         
 *                                                                    
 *     Whenever any KKC control block is to be referenced in a source
 *  program, this file must be also included.                            
 *----------------------------------------------------------------------*/

/************************************************************************
 *      RETURN CODE RETURNED BY KKC               
 ************************************************************************/
/*----------------------------------------------------------------------*
 *      NORMAL RETURN CODE
 *----------------------------------------------------------------------*/
#define   SUCCESS       0x0000          /* success                      */
#define   NOMORE_CAND   0x0002          /* no more next or previous cand*/
#define   END_CAND      0x0002          /* end of all candidates        */
#define   DURING_CNV    0x0006          /* during Chikuji conversion    */
#define   UNREAD_UDCT   0x0008          /* no read user dict            */

/*----------------------------------------------------------------------*
 *      NORMAL RETURN CODE
 *----------------------------------------------------------------------*/
#define   NO_CAND       0x0004          /* no candidate exist           */

#define   NUM_BTE       0x0104          /* number of BTE is more than 6 */

#define   JTEOVER       0x1104          /* JTE overflow                 */
#define   JKJOVER       0x1204          /* JKJ overflow                 */
#define   JLEOVER       0x1304          /* JLE overflow                 */
#define   FTEOVER       0x1404          /* FTE overflow                 */
#define   FWEOVER       0x1504          /* FWE overflow                 */
#define   FKXOVER       0x1604          /* FKX overflow                 */
#define   BTEOVER       0x1704          /* BTE overflow                 */
#define   PTEOVER       0x1804          /* PTE overflow                 */
#define   YCEOVER       0x1904          /* YCE overflow                 */
#define   YPEOVER       0x1a04          /* YPE overflow                 */
#define   MCEOVER       0x1b04          /* MCE overflow                 */

#define   SEIOVER       0x2104          /* SEI buffer overflow          */
#define   SEMOVER       0x2204          /* SEM buffer  overflow         */
#define   YMMOVER       0x2304          /* YMM buffer overflow          */
#define   GRMOVER       0x2404          /* GRM buffer overflow          */

/*----------------------------------------------------------------------*
 *      LOGICAL ERROR
 *----------------------------------------------------------------------*/
#define   WSPOVER       0x0108
#define   ELEM_SIZE     0x0208

/*----------------------------------------------------------------------*
 *      REGISTRATION ERROR
 *----------------------------------------------------------------------*/
#define   EXIST_SYS     0x0110          /* goku exists in system dict.  */
#define   EXIST_USR     0x0210          /* goku exists in user dict.    */
#define   USRDCT_OVER   0x0310          /* no free area in user dict    */
#define   USRREC_OVER   0x0410          /* no free area in the record   */
#define   EQ_YOMI_GOKU  0x0510          /* goku is equal to yomi        */
        
#define   UPDATING      0x0a10          /* user dict is being updated   */
#define   RQ_RECOVER    0x0b10          /* user dict should be recovered*/
#define   MRUBROKEN     0x0c10          /* MRU area should be recovered */

/*----------------------------------------------------------------------*
 *      PHYSICAL ERROR
 *----------------------------------------------------------------------*/
#define   SYS_OPEN      0x0181          /* error of open() with sys dict*/
#define   SYS_CLOSE     0x0281          /* err of close() with sys dict */
#define   SYS_LSEEK     0x0581          /* err of lseek() with sys dict */
#define   SYS_READ      0x0681          /* err of read() with sys dict  */
#define   SYS_WRITE     0x0781          /* err of write() with sys dict */
#define   SYS_LOCKF     0x0881          /* err of lockf() with sys dict */
#define   SYS_SHMAT     0x0981          /* err of shmat() with sys dict */
#define   SYS_FSTAT     0x0a81          /* err of fstat() with sys dict */

#define   USR_OPEN      0x0182          /* err of open() with usr dict  */
#define   USR_CLOSE     0x0282          /* err of close() with usr dict */
#define   USR_LSEEK     0x0582          /* err of lseek() with usr dict */
#define   USR_READ      0x0682          /* err of read() with usr dict  */
#define   USR_WRITE     0x0782          /* err of write() with usr dict */
#define   USR_LOCKF     0x0882          /* err of lockf() with usr dict */
#define   USR_FSTAT     0x0a82          /* err of fstat() with usr dict */
        
#define   FZK_OPEN      0x0184          /* err of open() with fzk dict  */
#define   FZK_CLOSE     0x0284          /* err of close() with fzk dict */
#define   FZK_LSEEK     0x0584          /* err of lseek() with fzk dict */
#define   FZK_READ      0x0684          /* err of read() with fzk dict  */
#define   FZK_WRITE     0x0784          /* err of write() with fzk dict */
#define   FZK_LOCKF     0x0884          /* err of lockf() with fzk dict */
#define   FZK_FSTAT     0x0a84          /* err of fstat() with fzk dict */

#define   SYS_ERROR     0x81            /* error of system dictionary   */
#define   USR_ERROR     0x82            /* error of user dictionary     */
#define   FZK_ERROR     0x84            /* error of fuzoku-go dict      */
#define   REG_ERROR     0x10            /* error of registration        */

#define   OPEN_ERROR    0x01            /* error of open()              */
#define   CLOSE_ERROR   0x02            /* error of close()             */
#define   LSEEK_ERROR   0x05            /* error of lseek()             */
#define   READ_ERROR    0x06            /* error of read()              */
#define   WRITE_ERROR   0x07            /* error of write()             */
#define   LOCKF_ERROR   0x08            /* error of lockf()             */
#define   SHMAT_ERROR   0x09            /* error of shmat()             */
#define   FSTAT_ERROR   0x0a            /* error of fstat()             */
#define   USED_ERROR    0x0a            /* updating user dictionary     */
#define   RECOVER_ERROR 0x0b            /* request of recovery          */

/*----------------------------------------------------------------------*
 *      MEMORY ALLOCATION ERROR
 *----------------------------------------------------------------------*/
#define   MEM_MALLOC    0x0388          /* err of malloc()              */

/*----------------------------------------------------------------------*
 *      CHECK SYSTEM DICTIONARIES
 *----------------------------------------------------------------------*/
#define   SYS_INCRR     0x1081          /* System dictionary incorrected*/
#define   USR_INCRR     0x1082          /* Usr    dictionary incorrected*/
#define   FZK_INCRR     0x1084          /* FZK    dictionary incorrected*/

/*----------------------------------------------------------------------*
 *      UNPREDICTABLE ERROR
 *----------------------------------------------------------------------*/
#define  UERROR         0x7FFF          /* this code is fatal error     */


/************************************************************************
 *      CODE FOR GRAMMER FROM TRL               
 ************************************************************************/
/*----------------------------------------------------------------------*
 *      JDTYPE       
 *----------------------------------------------------------------------*/
#define   TP_IPPAN         1            /* general word                   */
#define   TP_SETTO         2            /* prefix                         */
#define   TP_OSETTO        3            /* prefix 'o'                     */
#define   TP_GSETTO        4            /* prefix 'go'                    */
#define   TP_SETSUBI       5            /* suffix                         */
#define   TP_SUUCHI        6            /* numeric                        */
#define   TP_NSETTO        7            /* prefix for numeric             */
#define   TP_JOSUSHI       8            /* suffix for numeric             */
#define   TP_KOYUU         9            /* proper noun                    */
#define   TP_PSETTO       10            /* prefix for proper noun         */
#define   TP_PSETSUBI     11            /* suffix for proper noun         */
#define   TP_TOUTEN       12            /* ","  Touten                    */
#define   TP_KUTEN        13            /* "."  Kuten                     */
#define   TP_CONVKEY      14            /* Conversion key                 */
#define   TP_USERDCT      29            /* word in user dictionary        */
#define   TP_DUMMY_STP    31            /* word in user dictionary        */
#define   TP_DUMMY_END    42            /* word in user dictionary        */

/*------------------------------------------------------------------------*
 *      RIGHT HINSI                 
 *------------------------------------------------------------------------*/
#define E_MEISHI        1
#define E_RENYOU        2
#define E_MEIREN        3
#define E_SHUUSHI       4
#define E_RENTAI        5
#define E_SHUREN        6
#define E_FYOU          7
#define E_FTAI          8
#define E_FNEU          9
#define E_OTHER        10
#define E_KUGIRI       11
#define E_CONVKEY      12
#define E_SETTO        13
#define E_NSETTO       14
#define E_PSETTO       15
#define E_OSETTO       16
#define E_GSETTO       17
#define E_SETSUBI      18
#define E_SUUCHI       19
#define E_KOYUU        20
#define E_TOUTEN       21
#define E_KUTEN        22

/*----------------------------------------------------------------------*
 *  LEFT HINSI                                                            
 *----------------------------------------------------------------------*/
#define S_MEISHI        1
#define S_MEIYOU        2
#define S_YOUGEN        3
#define S_FUKUSHI       4
#define S_OTHER         5
#define S_TOUTEN        6
#define S_KUTEN         7
#define S_CONVKEY       8
#define S_SETTO         9
#define S_SETSUBI      10
#define S_JOSUSHI      11
#define S_PSETSUBI     12
#define S_SUUCHI       13
#define S_KOYUU        14

/*----------------------------------------------------------------------*
 *      GRAMMER           
 *----------------------------------------------------------------------*/
#define  TAIGENKA1   11                 /* taigen-ka 1-dan doshi        */
#define  TAIGENKASA  13                 /* taigen-ka sa-hen doushi      */
#define  NOUN        19                 /* noun                         */
/*       PREFIX      23 */              /* prefix                       */
/*       SUFFIX      24 */              /* suffix                       */
#define  PRE_NUM     25                 /* prefix for numeric           */
#define  SUF_NUM     26                 /* suffix for numeric           */
#define  PRONOUN     27                 /* proper noun                  */
#define  PRE_PRO     28                 /* prefix for proper            */
#define  SUF_PRO     29                 /* suffix for proper            */
#define  TEINEIO     30                 /* teinei no O                  */
#define  TEINEIGO    43                 /* teinei no GO                 */
#define  ALPHA       31                 /* alphabet                     */
#define  KATAKANA    32                 /* katakana                     */
#define  OPERATOR    35                 /* sign used by calculation     */
#define  MONESIGN    36                 /* sign for money               */
#define  UNITSIGN    37                 /* sign for unit                */
#define  NUMERAL     42                 /* numeral                      */
#define  PROSEI      40                 /* numeral                      */
#define  PRONA       41                 /* numeral                      */

/*----------------------------------------------------------------------*/
/*      CODE OF LEFT WORD & RIGHT WORD                                  */
/*----------------------------------------------------------------------*/
#define  LR_UNDEFINED         0
#define  LR_ALPHABET          1
#define  LR_KATAKANA          2
#define  LR_KUTEN             3
#define  LR_TOUTEN            4
#define  LR_KIGO1             5
#define  LR_KIGO2             6
#define  LR_KIGO3             7
#define  LR_KIGO4             8
#define  LR_LEFTPAREN         9
#define  LR_RIGHTPAREN       10
#define  LR_DELIMITER        11
#define  LR_NUMERIC          12
#define  LR_ABBRIVIATION     16

/*----------------------------------------------------------------------*/
/*   DFLAG BIT ALLOCATIONS                                              */
/*----------------------------------------------------------------------*/
/* ippango byte 1 */
#define  O_OK          0x40
#define  GO_OK         0x20
#define  SETTO_OK      0x10
#define  SETSUBI_OK    0x08
#define  FREQ          0x07

/* ippango byte 2 */
#define  NUMBER        0x08
#define  PRE_GOKI_REQ  0x04
#define  SUC_GOKI_REQ  0x02
#define  COMPOUND_NOT  0x01

/* setsuji byte 1 */
#define  POS_SAHEN     0x20
#define  POS_KEIDOU    0x10
#define  POS_FUKUSHI   0x08

/* setsuji byte 2 */
#define  SAHEN_REQ     0x40
#define  KEIDOU_REQ    0x20
#define  MEISHI_REQ    0x10
#define  DUP_OK        0x08
#define  INTERNAL_NOT  0x04
#define  ALMIGHTY      0x02

/* koyuu-meishi, its setsuji byte 1 */
#define  PRONOUN_SEI   0x40
#define  PRONOUN_MEI   0x20
#define  PRO_CHIMEI    0x10
#define  PRO_CHIMEIXX  0x08

/* koyuu-meishi, its setsuji byte 2 */
#define  HOUJINMEI     0x01

/* josushi byte 1 */
#define  KURAIDORI     0x10

/*----------------------------------------------------------------------*/
/*   HINR-HINL PENALTY'S DEPENDENCY TO DFLG                             */
/*----------------------------------------------------------------------*/
#define  CW  51
#define  PG  52
#define  GS  53
#define  OY  54
#define  GY  55
#define  GP  56
#define  SG  57
#define  PP  58
#define  SS  59
#define  SP  60
#define  NPG 72
#define  NGS 73
#define  NPS 74 /* TORI 7/26/87 */
#define  NSS 79
#define  KPG 82
#define  KGS 83
#define  KCW 84
#define   GN 85
#define  NPN 86

/*----------------------------------------------------------------------*
 *  ATTORIBUTE FOR SEISHO DATA (SEISHO ZOKUSEI CODE)  
 *----------------------------------------------------------------------*/
#define  ALPHANUM      0x0f             /* alphanumeric                 */
#define  ALPHAKAN      0x0e             /* alphanumeric & kansuji       */
#define  RYAKUSHO      0x10             /* single kanji                 */
#define  KANSUUJI      0x11             /* numeric expressioned by knji */
#define  JRT           0x12             /* jiritu-go                    */
#define  FZK           0x13             /* fizoku-go                    */
#define  FZK_HOMO      0x14             /* fuzoku-go which has homonym  */
#define  ALLFZK        0x15             /* all fuzoku-go                */
#define  ALLFZK_HOMO   0x16             /* all fuzoku-go homonym        */
#define  PREFIX        0x17             /* prefix                       */
#define  SUFFIX        0x18             /* suffix                       */
#define  NUM_PREFIX    0x19             /* prefix for numeric           */
#define  NUM_SUFFIX    0x1a             /* suffix for numeric           */
#define  PRO_PREFIX    0x1b             /* prefix for pronoun           */
#define  PRO_SUFFIX    0x1c             /* suffix for pronoun           */
#define  CONTINUE      0x01             /* continuous                   */

/*----------------------------------------------------------------------*
 *  ATTORIBUTE FOR GRAMMER DATA (GRAMMER CODE)                          
 *----------------------------------------------------------------------*/
#define  GR_JRT          0x12           /* jiritsu-go                   */
#define  GR_PREFIX       0x17           /* prefix                       */
#define  GR_SUFFIX       0x18           /* suffix                       */
#define  GR_NUM_PREFIX   0x19           /* prefix for numeric           */
#define  GR_NUM_SUFFIX   0x1a           /* suffix for numeric           */
#define  GR_PRO_PREFIX   0x1b           /* prefix for pronoun           */
#define  GR_PRO_SUFFIX   0x1c           /* suffix for pronoun           */
#define  GR_ALPHA        0x00           /* alphabet/alphanumeric        */
#define  GR_RYAKUSHO     0x00           /* ryakusho                     */
#define  GR_ALPHAKAN     0x00           /* alphabet & kansuji           */

/*----------------------------------------------------------------------*
 *   FUNCTIION FOR OTHER CONVERSION                                     
 *----------------------------------------------------------------------*/
#define  NEXTCNV   1                    /* next conversion              */
#define  PREVCNV   2                    /* previous cwconversion        */
#define  FRSTCNV   3                    /* first candidate              */
#define  LASTCNV   4                    /* last candidate               */

#define  ALLOP     0                    /* open & forward for brows cnv */
#define  ALLFW     1                    /* forward for brows cnv        */
#define  ALLBW     2                    /* backward for brows cnv       */

#define  ESC_ALPHA 0x1d                 /* escape for alphanumeric      */

/************************************************************************
 *      VALUES FOR GLOBAL USE
 ************************************************************************/
/*----------------------------------------------------------------------*
 *      ID ( System ID(2bytes) + Version No.(2bytes) 
 *                    + Release No.(2bytes) + Maintenance No.(2bytes) )
 *----------------------------------------------------------------------*/
#define   AIXID     0x01020000          /* for AIX                      */
#define   S5080ID   0x02020000          /* for 5080 system              */

/*----------------------------------------------------------------------*
 *      MAX SIZE OF TABLE
 *----------------------------------------------------------------------*/
#define  MAXWSP    0xbb25               /* maxsize of working pool      */

#define  YMIMAX    256                  /* yomi                         */
#define  SEIMAX    512                  /* Seisho                       */
#define  SEMMAX    512                  /* Seisho Map                   */
#define  YMMMAX     34                  /* Yomi Map                     */
#define  GRMMAX /* 128  */ 256          /* Grammer Map                  */
#define  MAXJTX /* 128  */ 256          /* Jiritsu-go Tag eXchenge      */
#define  MAXFST /* 128  */ 256          /* LINK POSSIBLITY              */
#define  MAXFKX /* 200  */ 100          /* Fuzoku-go Kanji eXhenge      */
#define  MAXJTE /* 600  */ 300          /* Jiritsu-go                   */
#define  MAXJLE /* 400  */ 200          /* Long Word                    */
#define  MAXJKJ /* 1000 */ 500          /* Kanji Hyouki                 */
#define  MAXFTE /* 300  */ 150          /* Fuzoku-go                    */
#define  MAXFWE /* 300  */ 150          /* Fuzoku work                  */
#define  MAXBTE /* 800  */ 400          /* Bunsetsu                     */
#define  MAXPTE /* 400  */ 200          /* Path                         */
#define  MAXYCE    256                  /* Yomi                         */
#define  MAXYPE    256                  /* Prev Yomi                    */
#define  MAXMCE    256                  /* Mora                         */

/*----------------------------------------------------------------------*
 *   Privious KAKutei flag                                              
 *----------------------------------------------------------------------*/
#define  PKAKNON 0
#define  PKAKALL 1
#define  PKAKPRT 2

/*------------------------------------------------------------------------*
 *  DICTIONARY                                                        
 *------------------------------------------------------------------------*/
#define  SX        0                    /* index of system dictionary     */
#define  SD        1                    /* data of system dictionary      */
#define  TX        2                    /* index of dictionary for tankan */
#define  TD        3                    /* data of dictionary for tankan  */
#define  UX        4                    /* index of user dictionary       */
#define  UD        5                    /* data of user dictionary        */
#define  MRU       6                    /* MRU(Most Recentry Used) data   */
#define  FD        7                    /* fuzoku-go dictionary           */

#define  SHARE     0
#define  EXCLUSIVE 1

#define  ENVSYS    "JSYSDICT"
#define  ENVUSR    "JUSRDICT"
#define  ENVFZK    "JADJDICT"

#define  SYSDICT   "sysdict"
#define  USRDICT   "usrdict"
#define  FZKDICT   "adjdict"
#define  DCTPATH   "/usr/lpp/jpnls/dict"

#define  DFGSIZE   ( 1024 / 2 )
#define  MDESIZE   ( 1024 * 7 )
#define  UXESIZE   ( 1024 * 1 )
#define  UDESIZE   ( 1024 * 1 )
#define  LC_HAR    ( MDESIZE + 3 )
#define  LC_NAR    ( LC_HAR +1 )

#define  SDESIZE   ( 2048     )         /* system dictionary block size */

#define  ENVLIM    80

/*------------------------------------------------------------------------*
 *  TABLE                                                                 
 *------------------------------------------------------------------------*/
#define  JTB       0                    /* Jiritsu-go Table               */
#define  LTB       1                    /* Long word Table                */

/*------------------------------------------------------------------------*
 *  LOOK UP WORDS FROM DICTIONARIES                                       
 *------------------------------------------------------------------------*/
#define  GENERIC   0
#define  SPECIFIC  1

#define  JD_YET       0                 /* not ready for looking up       */
#define  JD_READY     1                 /* ready for looking up           */
#define  JD_LONG      2                 /* candidates exist               */
                                        /*      in long word table        */
#define  JD_COMPLETE  9                 /* looking up words is completed  */

/*----------------------------------------------------------------------*
 *      DIRECTION OF NEXT & PREVIOUS CONVERSION
 *----------------------------------------------------------------------*/
#define  FORWARD     1                  /* serch forward            tk  */
#define  BACKWARD    2                  /* serch backward           tk  */

/*----------------------------------------------------------------------*
 *      TYPE OF CONVERSION
 *----------------------------------------------------------------------*/
#define  ORD         1                  /* ordinary change              */
#define  NXT         3                  /* next     change              */
#define  ABS         9                  /* absolute change              */

/*----------------------------------------------------------------------*
 *      MEANING OF FUZOKUGO FLAG
 *----------------------------------------------------------------------*/
                                        /* fzkflg                       */
#define    F_FLG_NOEXT      8           /* fzk hyoki not exist          */
#define    F_FLG_TWO        4           /* One/Two fzk hyoki exist      */
#define    F_FLG_USE2       2           /* 2nd hyoki is used            */
#define    F_FLG_USE1       1           /* 1st hyoki is used            */

/*----------------------------------------------------------------------*
 *      Numeric Conversion
 *----------------------------------------------------------------------*/
#define   MAXINT   25                   /* limitation of kuraidori      */

#define   ROMA_H_PCCODE     0xfa        /* roma suji high byte          */
#define   ROMAS_L_PCCODE    0x40        /* roma suji low byte for kkckxx*/
#define   ROMAL_L_PCCODE    0x4a        /* roma suji low byte for kkckxx*/

#define   PERIOD_PCCODE     0x73        /* PC code of period            */
#define   PERIOD_H_PCCODE   0x81        /* kanma high byte for kkckxx   */
#define   PERIOD_L_PCCODE   0x45        /* kanma low byte for kkckxx    */

#define   COMMA_PCCODE      0x72        /* Pc code of comma             */
#define   COMMA_H_PCCODE    0x81        /* point high byte for kkckxx   */
#define   COMMA_L_PCCODE    0x41        /* point low byte for kkckxx    */

#define   ZERO_PCCODE       0x75        /* PC code of 0                 */
#define   NINE_PCCODE       0x7e        /* PC code of 9                 */

/*----------------------------------------------------------------------*
 *  Chain Table Management
 *----------------------------------------------------------------------*/
#define   LOCAL_RC         0x00ff

/*----------------------------------------------------------------------*
 *    FOR CCFPTH
 *----------------------------------------------------------------------*/
#define   FPTH_FOUND    ( 0x0000 | LOCAL_RC )
#define   FPTH_NOT_FND  ( 0x0100 | LOCAL_RC )

/*----------------------------------------------------------------------*
 *    FOR CGET
 *----------------------------------------------------------------------*/
#define   GET_TOP_MID   ( 0x0000 | LOCAL_RC )
#define   GET_LAST      ( 0x0100 | LOCAL_RC )
#define   GET_EMPTY     ( 0x0200 | LOCAL_RC )

/*----------------------------------------------------------------------*
 *    FOR CMOVx
 *----------------------------------------------------------------------*/
#define   MOV_MID       ( 0x0000 | LOCAL_RC )
#define   MOV_TOP       ( 0x0100 | LOCAL_RC )
#define   MOV_LAST      ( 0x0200 | LOCAL_RC )
#define   MOV_EMPTY     ( 0x0400 | LOCAL_RC )

/*----------------------------------------------------------------------*
 *      FOR ALPHABETIC/ALPHANUMERIC AND KANSUJI CONVERSION
 *----------------------------------------------------------------------*/
#define   ALPHMASK      0x01            /* alpha/alphanum conv ON       */
#define   KANMASK       0x02            /* kansuji conv ON              */

/*----------------------------------------------------------------------*
 *      7BIT YOMI CODE 
 *----------------------------------------------------------------------*/
#define   Y_VU          0x1a
#define   Y_XKA         0x1b
#define   Y_XKE         0x1c
#define   Y_ESC         0x1d
#define   Y_POND        0x1e
#define   Y_XA          0x1f
#define   Y_A           0x20
#define   Y_XI          0x21
#define   Y_I           0x22
#define   Y_XU          0x23
#define   Y_U           0x24
#define   Y_XE          0x25
#define   Y_E           0x26
#define   Y_XO          0x27
#define   Y_O           0x28
#define   Y_KA          0x29
#define   Y_GA          0x2a
#define   Y_KI          0x2b
#define   Y_GI          0x2c
#define   Y_KU          0x2d
#define   Y_GU          0x2e
#define   Y_KE          0x2f
#define   Y_GE          0x30
#define   Y_KO          0x31
#define   Y_GO          0x32
#define   Y_SA          0x33
#define   Y_ZA          0x34
#define   Y_SHI         0x35
#define   Y_JI          0x36
#define   Y_SU          0x37
#define   Y_ZU          0x38
#define   Y_SE          0x39
#define   Y_ZE          0x3a
#define   Y_SO          0x3b
#define   Y_ZO          0x3c
#define   Y_TA          0x3d
#define   Y_DA          0x3e
#define   Y_CHI         0x3f
#define   Y_DI          0x40
#define   Y_XTSU        0x41
#define   Y_TSU         0x42
#define   Y_DU          0x43
#define   Y_TE          0x44
#define   Y_DE          0x45
#define   Y_TO          0x46
#define   Y_DO          0x47
#define   Y_NA          0x48
#define   Y_NI          0x49
#define   Y_NU          0x4a
#define   Y_NE          0x4b
#define   Y_NO          0x4c
#define   Y_HA          0x4d
#define   Y_BA          0x4e
#define   Y_PA          0x4f
#define   Y_HI          0x50
#define   Y_BI          0x51
#define   Y_PI          0x52
#define   Y_FU          0x53
#define   Y_BU          0x54
#define   Y_PU          0x55
#define   Y_HE          0x56
#define   Y_BE          0x57
#define   Y_PE          0x58
#define   Y_HO          0x59
#define   Y_BO          0x5a
#define   Y_PO          0x5b
#define   Y_MA          0x5c
#define   Y_MI          0x5d
#define   Y_MU          0x5e
#define   Y_ME          0x5f
#define   Y_MO          0x60
#define   Y_XYA         0x61
#define   Y_YA          0x62
#define   Y_XYU         0x63
#define   Y_YU          0x64
#define   Y_XYO         0x65
#define   Y_YO          0x66
#define   Y_RA          0x67
#define   Y_RI          0x68
#define   Y_RU          0x69
#define   Y_RE          0x6a
#define   Y_RO          0x6b
#define   Y_XWA         0x6c
#define   Y_WA          0x6d
#define   Y_WI          0x6e
#define   Y_WE          0x6f
#define   Y_WO          0x70
#define   Y_NN          0x71
#define   Y_DKT         0x72
#define   Y_CNM         0x72
#define   Y_HDK         0x73
#define   Y_CHO         0x74
#define   Y_0           0x75
#define   Y_1           0x76
#define   Y_2           0x77
#define   Y_3           0x78
#define   Y_4           0x79
#define   Y_5           0x7a
#define   Y_6           0x7b
#define   Y_7           0x7c
#define   Y_8           0x7d
#define   Y_9           0x7e
#define   Y_A_CNM       0x2C
#define   Y_A_PRD       0x2E
#define   Y_A_0         0x30
#define   Y_A_1         0x31
#define   Y_A_2         0x32
#define   Y_A_3         0x33
#define   Y_A_4         0x34
#define   Y_A_5         0x35
#define   Y_A_6         0x36
#define   Y_A_7         0x37
#define   Y_A_8         0x38
#define   Y_A_9         0x39

/*----------------------------------------------------------------------*
 *      MORA CODE	V3 Format
 *----------------------------------------------------------------------*/
#define   M_AADM    	0x01    /* '!' exciamation mark.                */
#define   M_ADBQ    	0x02    /* '"' double quotation.                */
#define   M_ASHP    	0x03    /* '#' sharp.                           */
#define   M_APCT    	0x05    /* '%' percent.                         */
#define   M_AAMP    	0x06    /* '&' ampersand.                       */
#define   M_ASQT    	0x07    /*  '  single quotation.                */
#define   M_ALPR    	0x08    /* '(' left parentheses.                */
#define   M_ARPR    	0x09    /* ')' right parentheses.               */
#define   M_AAST    	0x0a    /* '*' asterisk.                        */
#define   M_APLS    	0x0b    /* '+' plus.                            */
#define   M_ACOM    	0x0c    /* ',' comma.                           */
#define   M_AMIN    	0x0d    /* '-' minus sign.                      */
#define   M_APER    	0x0e    /* '.' period.                          */
#define   M_AFSL    	0x0f    /* '/' forward slash.                   */
#define   M_ACOL    	0x10    /* ':' colon                            */
#define   M_ASCL    	0x11    /* ';' semicolon.                       */
#define   M_ALAB    	0x12    /* '<' left angle bracket.              */
#define   M_AEQU    	0x13    /* '=' equal sign.                      */
#define   M_ARAB    	0x14    /* '>' right angle bracket.             */
#define   M_AQUE    	0x15    /* '?' question mark.                   */
#define   M_AATM    	0x16    /* '@' at mark.                         */
#define   M_A           0x20
#define   M_I           0x22
#define   M_U           0x24
#define   M_E           0x26
#define   M_O           0x28

#define   M_KA          0x29
#define   M_KI          0x2B 
#define   M_KU          0x2D
#define   M_KE          0x2F
#define   M_KO          0x31

#define   M_GA          0x2A
#define   M_GI          0x2C
#define   M_GU          0x2E
#define   M_GE          0x30
#define   M_GO          0x32

#define   M_SA          0x33
#define   M_SI          0x35
#define   M_SU          0x37
#define   M_SE          0x39
#define   M_SO          0x3B

#define   M_ZA          0x34
#define   M_JI          0x36
#define   M_ZU          0x38
#define   M_ZE          0x3A
#define   M_ZO          0x3C

#define   M_TA          0x3D
#define   M_CHI         0x3F
#define   M_TSU         0x42
#define   M_TE          0x44
#define   M_TO          0x46

#define   M_DA          0x3E
#define   M_DE          0x45
#define   M_DO          0x47

#define   M_NA          0x48
#define   M_NI          0x49
#define   M_NU          0x4A
#define   M_NE          0x4B
#define   M_NO          0x4C

#define   M_HA          0x4D
#define   M_HI          0x50
#define   M_FU          0x53
#define   M_HE          0x56
#define   M_HO          0x59

#define   M_PA          0x4F
#define   M_PI          0x52
#define   M_PU          0x55
#define   M_PE          0x58
#define   M_PO          0x5B

#define   M_BA          0x4E
#define   M_BI          0x51
#define   M_BU          0x54
#define   M_BE          0x57
#define   M_BO          0x5A

#define   M_MA          0x5C
#define   M_MI          0x5D
#define   M_MU          0x5E
#define   M_ME          0x5F
#define   M_MO          0x60

#define   M_YA          0x62
#define   M_YU          0x64
#define   M_YO          0x66

#define   M_RA          0x67
#define   M_RI          0x68
#define   M_RU          0x69
#define   M_RE          0x6A
#define   M_RO          0x6B

#define   M_WA          0x6D
#define   M_NN          0x71

#define   M_KYA         0x90
#define   M_KYU         0x91
#define   M_KYO         0x92

#define   M_GYA         0x93
#define   M_GYU         0x94
#define   M_GYO         0x95

#define   M_SYA         0x96
#define   M_SYU         0x97
#define   M_SYO         0x98

#define   M_JA          0x99
#define   M_JU          0x9A
#define   M_JO          0x9B

#define   M_CYA         0x9C
#define   M_CYU         0x9D
#define   M_CYO         0x9E

#define   M_NYA         0xA2
#define   M_NYU         0xA3
#define   M_NYO         0xA4

#define   M_HYA         0xA5
#define   M_HYU         0xA6
#define   M_HYO         0xA7

#define   M_PYA         0xAB
#define   M_PYU         0xAC
#define   M_PYO         0xAD

#define   M_BYA         0xA8
#define   M_BYU         0xA9
#define   M_BYO         0xAA

#define   M_MYA         0xAE
#define   M_MYU         0xAF
#define   M_MYO         0xB0

#define   M_RYA         0xB1
#define   M_RYU         0xB2
#define   M_RYO         0xB3

#define   M_XTSU        0x41
#define   M_WO          0x70
#define   M_CHO         0x74
#define   M_VU          0xB4

#define   M_XA          0x1F
#define   M_XI          0x21
#define   M_XU          0x23
#define   M_XE          0x25
#define   M_XO          0x27

#define   M_XYA         0x61
#define   M_XYU         0x63
#define   M_XYO         0x65

/*************************** 
#define   M_XKA         0x00
#define   M_XKE         0x00
***************************/

/*----------------------------------------------------------------------*
 *  Miscellaneous                                                         
 *----------------------------------------------------------------------*/
#define  vary      1000                 /* max ele for varying array    */
#define  VARY      1000                 /*     "                        */

#define  uschar    unsigned char
#define  usshort   unsigned short
#define  usint     unsigned int
#define  unsign    unsigned

#define  NULL      0                    /*   "                          */
#define  ON        1                    /* status of flag               */
#define  OFF       0                    /*   "                          */
#define  YES       1                    /* condition                    */
#define  NO        0                    /*   "                          */
#define  UNDEF     0                    /* Undefined                    */
#define  TRUE      1                    /*                              */
#define  FALSE     0                    /*                              */

#define  EQUAL     0
#define  EQ_SHORT  1                    /* result of comparison         */
#define  NOT_EQU   -1                   /* result of comparison         */
#define  LONG      2                    /* result of comparison         */

#define  ELM_EMPTY -1                   /* the chain is empty           */

/*----------------------------------------------------------------------*
 *      User Dicitionary Addition
 *----------------------------------------------------------------------*/
#define U_REC_L1  ( 1024 )          /*    1 recorde size ( 1K )         */
#define U_REC_L2  ( 2048 )          /*    2 recorde size ( 2K )         */
#define U_REC_L3  ( 3072 )          /*    3 recorde size ( 3K )         */

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

#define U_BACKWD (      1     )/* Direction of character movement.        */
#define U_FORWD  (      0     )/* Direction of character movement.        */

#define U_DADDLF (      1     )/* ADD Mode for LEFT                       */
#define U_DADDRT (      2     )/* ADD Mode for RIGHT                      */
#define U_DADDNW (      3     )/* ADD Mode for NEW                        */
#define U_BASNAR (      7     )/*                                         */

#define U_KAN_MX (    255     )/* Kana  Data Max length                   */
#define U_KNJ_MX (    254     )/* Kana  Data Max length                   */
#define U_OMIT_C (   0x1e     )/* Omit Process check Data                 */
#define U_HAR_V1 (   0x39     )/* User Dictionary size HAR Data 1         */
#define U_HAR_V2 (   0x5b     )/* User Dictionary size HAR Data 2         */
#define U_HAR_V3 (   0x5d     )/* User Dictionary size HAR Data 3         */
#define U_HAR_V4 (   0xb2     )/* User Dictionary size HAR Data 4         */
#define U_HAR_V5 (   0xb4     )/* User Dictionary size HAR Data 5         */
#define U_HAR_V6 (   0xfe     )/* User Dictionary size HAR Data 6         */

#define U_REC_L  (   1024L    )/*  1 recode size (1K)                     */
#define U_MRU_A  ( 7L*U_REC_L )/*  MRU area    7K                         */
#define U_INDX_A ( 3L*U_REC_L )/*  INDEX area  3K                         */

#define U_FON    (      1     )/* Logical Value On.                       */
#define U_FOF    (      0     )/* Logical Value Off.                      */

#define U_IL_HED  ( 5  )             /* index header length             */

#define U_1KJ_L  (      2     )/*  1 kanji length                         */

#define U_MSB_M  (      0x80  )/*  msb mask pattern                       */
#define U_MSB_O  (      0x3f  )/*  msb(2bits) off patt.                   */

#define U_NOKAN  (  0x00ff    )/* Kana Data none                          */
#define U_NOKNJ  (  0x01ff    )/* Kana Data Exist & Kanji Data none       */
#define U_KANKNJ (  0x02ff    )/* Kana Data Exist & Kanji Data Exist      */


#define U_STEPNO (       4    )/*  step number                            */
#define U_STEP0  (       0    )/*  step number 0                          */
#define U_STEP1  (       1    )/*  step number 1                          */
#define U_STEP2  (       2    )/*  step number 2                          */
#define U_STEP3  (       3    )/*  step number 3                          */
#define U_STEP4  (       4    )/*  step number 4                          */

#define U_KNL    (       1    )/*  kana length                            */
#define U_KJL    (       1    )/*  kanji length                           */

#define U_WORK   (       40   )/*  work bufer size                        */
#define U_MRUMAX (    0x1c00  )/*  MRU area max                           */

#define U_SHORT  (       2    )/*  short size                             */
#define U_CONV   (       0x3f )/*  convert bit                            */

#define U_TPOSI  (       2    )/*  MRU data top                           */

#define U_UDCRSV (   0x00     )/* Reserve Area Data                       */

#define U_MODINQ (      0     )/* Inquiry mode                          */
#define U_MODREG (      1     )/* Registration mode                     */

#define U_SPRF   (      50    )/* Persentage of Dictionary Spilit.        */
                               /* (Fraction).                             */
#define U_SPRD   (     100    )/* Persentage of Dictionary Spilit.        */
                               /* (Divisor).                              */

#define U_MLEN   (       2    )/*  MRU length size                        */
#define U_FTOP   (       0    )/*  file top position                      */

/*----------------------------------------------------------------------*
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcmac.h"           /* Kanji Utility Macros.                */
#include   "_Kcwsp.h"           /* Definition of internal table         */
#include   "_Kcdct.h"           /* Definition of dictiobary buffer      */
#include   "_Kctbl.h"           /* Definition of const. table structure */
#include   "_Kcctb.h"           /* Definition of Control Block( KCB )   */

#include   "_Knum.h"
/*----------------------------------------------------------------------*
 *      DEFINE SYSTEM DICTIONARY
 *----------------------------------------------------------------------*/
#define    RECORD_CONT   0x00	/* Record continue mode			*/
#define    RECORD_END    0x01	/* Record NO continue mode		*/
#define    REC_CONT	 16	/* Skip Record Number			*/
#define    MORA_MAX	 256 	/* Max of Mora count			*/
#define    KANJI_MAX	 512 	/* Max of KANJI Entry			*/
#define    DFLAG_MAX	 0x4	/* Max of Dflag 			*/
