/* @(#)02  1.6.1.1  src/bos/usr/lib/nls/loc/imt/tfep/taiwan.h, libtw, bos411, 9431A411a 7/26/94 17:16:09 */
/*
 *   COMPONENT_NAME: LIBTW
 *
 *   FUNCTIONS: CreateAux
 *              DestroyAux
 *              DrawAux
 *              DrawIndicator
 *              DrawText
 *              HideAux
 *              HideIndicator
 *              HideText
 *              MakeBeep
 *              StartText
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <iconv.h>
#include "im.h"
#include "imP.h"
#include "imerrno.h"            /* input method error header file */
#include "ted.h"
#include "tedinit.h"

/**********************************************/
/*      IMIndicatorInfo.unique declaration    */
/**********************************************/
#define IND_INITIALIZE  0x00
#define IND_ALPHA_NUM   0x00
#define IND_PHONETIC    0x01
#define IND_TSANG_JYE   0x02
#define IND_INTER_CODE  0x03
#define IND_CAPS_LOCK   0x04    /*         |<-- Msg -->|<-- IM --->| */
#define IND_BLANK       0x08    /* |---|---|---|---|---|---|---|---| */
#define IND_NO_WORD     0x10    /* | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | */
#define IND_ERROR_MSG   0x18    /* |---|---|---|---|---|---|---|---| */
#define IND_RADICAL     0x20
#define IM_MASK         0x07
#define MSG_MASK        0x38

#define IND_DEFAULT_LEN 80

/* ----------  Define The EUC  Temporarily define --*/

/* For IMShortForm Indicator                            */

/*#define S_ALPHA_NUM_EUC     "\323\301\305\306 "               @big5 */
/*#define S_CHINESE_EUC       "\304\343\305\306 "               @big5 */
/*#define S_HALF_SIZE_EUC     "\306\244\312\260 "               @big5 */
/*#define S_FULL_SIZE_EUC     "\307\300\312\260 "               @big5 */
/*#define S_PHONETIC_EUC      "\316\303\323\366 "               @big5 */
/*#define S_TSANG_JYE_EUC     "\324\277\357\356 "               @big5 */
/*#define S_INTERNAL_CODE_EUC "\304\371\356\243 "               @big5 */
/*#define S_CAPS_LOCK_EUC     "\264\274\353\321 "               @big5 */
/*#define S_BLANK_EUC         "\245\355\245\355"                @big5 */
/*#define S_ERROR_MSG_EUC     "\362\343\353\250 "       */
/*#define S_ERROR_MSG_EUC     "\321\272\362\343\365\357"        @big5 */
/*#define S_NO_WORD_EUC       "\260\364\307\363 "               @big5 */
/*#define SB_BLANK            "\122"                            @big5 */
/*#define IND_SHORT_FORM      ( sizeof(S_ALPHA_NUM_EUC) - 1           \
                            + sizeof(S_HALF_SIZE_EUC) - 1           \
                            + sizeof(S_PHONETIC_EUC)  - 1           \
                            + sizeof(SB_BLANK) - 1 + 35             )  @big5 */


/* For IMLongForm indicator                             */

/*#define ALPHA_NUM_EUC     "\323\301\305\306\245\355"      @big5 */
/*#define CHINESE_EUC       "\304\343\305\306\245\355"      @big5 */
/*#define HALF_SIZE_EUC     "\306\244\312\260\245\355"      @big5 */
/*#define FULL_SIZE_EUC     "\307\300\312\260\245\355"      @big5 */
/*#define PHONETIC_EUC      "\316\303\323\366\245\355"      @big5 */
/*#define TSANG_JYE_EUC     "\324\277\357\356\245\355"      @big5 */
/*#define INTERNAL_CODE_EUC "\304\371\356\243\245\355"      @big5 */
/*#define CAPS_LOCK_EUC     "\264\274\353\321\245\355"      @big5 */
/*#define BLANK_EUC         "\245\355\245\355"              @big5 */
/*#define ERROR_MSG_EUC     "\362\343\353\250\245\355"  */
/*#define ERROR_MSG_EUC     "\321\272\362\343\365\357\245\355" @big5 */
/*#define NO_WORD_EUC       "\260\364\307\363\245\355"         @big5 */
/*#define RADICAL_EUC       "Radi"      */
/*#define DB_BLANK          "\245\355"                         @big5 */
/*#define IND_LONG_FORM     ( sizeof(ALPHA_NUM_EUC) - 1           \
                          + sizeof(HALF_SIZE_EUC) - 1           \
                          + sizeof(PHONETIC_EUC)  - 1           \
                          + sizeof(DB_BLANK) - 1 + 56)       @big5 */

/**************************************/
/*   Used by TIMProcess.c             */
/**************************************/

#define TEXT_ON         1
#define TEXT_OFF        0
#define TedProcError    IMError

/**************************************/
/*       ENV variable definition      */
/**************************************/

#define ENVVAR             "TIMPROFILE"
#define PHUSERDICT         "TIMPHUSRDICT"
#define TJUSERDICT         "TIMTJUSRDICT"
#define PHSYSDICT          "TIMPHSYSDICT"
#define TJSYSDICT          "TIMTJSYSDICT"
#define LEARNFILE          "TIMLEARNINGFILE"     /* V4.1        */
#define KEYLAYOUT          "PHKBD"               /* @big5       */

/***********************************************/
/*  dictionary name default value defintion    */
/***********************************************/

#define DEFAULTPHUSERDICT  "/usr/lpp/tls/dict/phusrdict"
#define DEFAULTTJUSERDICT  "/usr/lpp/tls/dict/tjusrdict"
#define DEFAULTPHSYSDICT   "/usr/lpp/tls/dict/phsysdict"
#define DEFAULTTJSYSDICT   "/usr/lpp/tls/dict/tjsysdict"
#define DEFAULTPROFILE     "/usr/lpp/tls/defaults/timrc"
#define FILENAME1          ".timrc"
#define FILENAME2          "phusrdict"
#define FILENAME3          "tjusrdict"
#define FILENAME4          "phsysdict"
#define FILENAME5          "tjsysdict"
#define FILENAME6          ".learnfile"     /* V4.1     */

#define MAX_CHAR_NO        20
#define SPACE              0x20
#define PATHFIND           3

/**************************************/
/*     dictionary type defintion      */
/**************************************/

#define PH_USER_DICT       1
#define TJ_USER_DICT       PH_USER_DICT + 1
#define PH_SYS_DICT        TJ_USER_DICT + 1
#define TJ_SYS_DICT        PH_SYS_DICT + 1

/*****************************************/
/*  profile structure string definition  */
/*****************************************/

#define INITCHAR           "initchar"
#define INITSIZE           "initsize"
#define INITBEEP           "initbeep"
#define INITSELECTION      "initselection" /* V4.1 */
#define INITLEARN          "initlearn"     /* V4.1 */
#define INITCODESET_1      "big5"          /* V4.1 */
#define INITKEYLAYOUT      "initphkbd"     /* @big5 */

#define TIM_INIT_AN        "alpha_numerical"
#define TIM_INIT_TJ        "tsang_jye"
#define TIM_INIT_PH        "phonetic"
#define TIM_INIT_HALF      "half"
#define TIM_INIT_FULL      "full"
#define TIM_INIT_BEEP_ON   "beep_on"
#define TIM_INIT_BEEP_OFF  "beep_off"
#define TIM_INIT_SELECTION_ON  "selection_on"  /* V4.1 */
#define TIM_INIT_SELECTION_OFF "selection_off" /* V4.1 */
#define TIM_INIT_LEARN_ON  "learn_on"          /* V4.1 */
#define TIM_INIT_LEARN_OF  "learn_off"         /* V4.1 */
#define TIM_INIT_BIG5      "big5"              /* V4.1 */
#define TIM_INIT_NON_IBM_LAYOUT "da-chien"      /*@big5 */
#define TIM_INIT_ET_LAYOUT      "eten"           /*@et   */

/*************************************/
/*     read/write only definition    */
/*************************************/

#define READ_ONLY          0x04
#define WRITE_ONLY         0x02

/******************************************/
/*   indicator buffer string max length   */
/******************************************/

#define TIM_INDSTRMAXLEN   110

/*******************************/
/*     aux buffer max size     */
/*******************************/

#define TIM_AUXCOLMAX      80
#define TIM_AUXROWMAX      14

/*******************************/
/*     aux buffer format       */
/*******************************/

#define SHORTAUX           0
#define LONGAUX            1

/*****************************/
/*     error definition      */
/*****************************/

#define TIMDictError       1
#define TIMEDError         2

/******************  Global Variables   *****************************/

char   *profile_ptr;     /* pointer to one allocated profile memory */
char   *profile_offset;  /* set initial offset for further used     */
int    profile_size;     /* the size of profile                     */

/******************************************************/
/*             Dictionary data structure              */
/******************************************************/
typedef struct{
        char    *PH_usr_dict;   /* path to PH_Usr_Dict       */
        char    *TJ_usr_dict;   /* path to TJ_Usr_Dict       */
        char    *PH_sys_dict;   /* path to PH_Sys_Dict       */
        char    *TJ_sys_dict;   /* path to TJ_Sys_Dict       */
}DictName;

/*************************************************/
/*          TIM Profile data structure           */
/*************************************************/
typedef struct{
        int           initchar;      /* PH/TJ/AN             */
        int           initsize;      /* half/full size       */
        int           initbeep;      /* Beep sound is ON/OFF */
        int           initselection; /* ListBox is On/Off    */
        int           initlearn;     /* Learning is On/Off   */
        int           initcodeset;   /* CodeSet will be used @big5 */
        iconv_t       iconv_flag;    /* opening iconv flag   @big5 */
        int           initkeyboard;  /* IBM or non-IBM layout @big5 */
        DictName      dictname;      /* Ph/Tj sys/usr dict   */
        char          *learnfile;    /* V410 path to learning file */
}TimProfile;

/********************************************************/
/*             data structure for word tree             */
/********************************************************/
typedef struct element{
        struct element  *next;
        char            *leftword;
        char            *rightword;
}list;

/************************************************/
/*            TIM ED data structure             */
/************************************************/
typedef struct {
        int     TedID;        /* TIM ED identification                */
        char    *echobufs;    /* echo buffer string for text area     */
        char    *echobufa;    /* echo buffer attribute for text area  */
        char    *fixbuf;      /* buffer for final result (EUC)        */
        int     echobufsize;  /* echo buffer size                     */
        char    **auxbufs;    /* pointer to aux buf string address    */
        char    **auxbufa;    /* pointer to aux buf attribute address */
        AuxSize auxsize;      /* auxiliary area size                  */
}TIMed,*TIMED;

/**********************************************/
/*          TIM-FEP data structure            */
/**********************************************/
typedef struct _TIMFEP{
        IMFepCommon     common;         /* IMFEP common portion */
        int             timver;         /* TIM version          */
        IMKeymap        *keymap;        /* TIM keymap           */
}TIMfep,*TIMFEP;

/************************************************/
/*           TIM Object data structure          */
/************************************************/
typedef struct _TIMobject{
        IMObjectCommon  imobject;       /* IM common information        */
        IMTextInfo      textinfo;       /* TIM text information         */
        IMAuxInfo       auxinfo;        /* TIM auxiliary information    */
        IMSTR           string;         /* TIM string for GetString     */
        IMSTR           indstr;         /* TIM indicator string         */
        IMIndicatorInfo indinfo;        /* TIM indicator string         */
        IMQueryState    querystate;     /* TIM query state              */
        caddr_t         auxID;          /* Auxiliary ID created by CB   */
        caddr_t         convert_str;    /* final EUC(Convert key press) */
        int             auxIDflag;      /* TRUE : Aux area is created   */
        int             auxstate;       /* Auxiliary area state         */
        int             textstate;      /* the state of text area       */
        int             state_aux_text; /* return value of IMProcess */
        IMBuffer        output;         /* output buffer             */
        TIMed           tedinfo;        /* TIM ed information           */
}TIMobj,*TIMOBJ;

#define MakeBeep(ImObj)         (*(ImObj->imobject.cb->beep))(ImObj,BEEP_ON,ImObj->imobject.udata)
#define DrawIndicator(ImObj)    (*(ImObj->imobject.cb->indicatordraw))(ImObj,&(ImObj->indinfo),ImObj->imobject.udata)
#define HideIndicator(ImObj)    (*(ImObj->imobject.cb->indicatorhide))(ImObj,ImObj->imobject.udata)
#define DestroyAux(ImObj)       (*(ImObj->imobject.cb->auxdestroy))(ImObj,ImObj->auxID,ImObj->imobject.udata)
#define HideAux(ImObj)          (*(ImObj->imobject.cb->auxhide))(ImObj,ImObj->auxID,ImObj->imobject.udata)
#define CreateAux(ImObj)        (*(ImObj->imobject.cb->auxcreate))(ImObj,&(ImObj->auxID),ImObj->imobject.udata)
#define DrawAux(ImObj)          (*(ImObj->imobject.cb->auxdraw))(ImObj,ImObj->auxID,&(ImObj->auxinfo),ImObj->imobject.udata)
#define StartText(ImObj)        (*(ImObj->imobject.cb->textstart))(ImObj,&offset,ImObj->imobject.udata)
#define DrawText(ImObj)         (*(ImObj->imobject.cb->textdraw))(ImObj,&(ImObj->textinfo),ImObj->imobject.udata)
#define HideText(ImObj)         (*(ImObj->imobject.cb->texthide))(ImObj,ImObj->imobject.udata)

#define TIM_VALID_BITS          (ShiftMask|LockMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask)

uint    erase_flag;        /* erase previous buffer   */

#define TITLE_LEN       70
#define TITLE_STR       70
