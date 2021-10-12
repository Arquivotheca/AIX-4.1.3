/* @(#)40	1.1  src/bos/usr/lib/nls/loc/ZH.im/chinese.h, ils-zh_CN, bos41B, 9504A 12/19/94 14:38:11  */
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: CreateAux
 *		DestroyAux
 *		DrawAux
 *		DrawIndicator
 *		DrawText
 *		HideAux
 *		HideIndicator
 *		HideText
 *		MakeBeep
 *		StartText
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include "im.h"
#include "imP.h"
#include "imerrno.h"            /* input method error header file */
#include "zhed.h"
#include "zhedinit.h"

/**********************************************/
/*      IMIndicatorInfo.unique declaration    */
/**********************************************/
#define IND_INITIALIZE        0x00
#define IND_ALPHA_NUM         0x00
#define IND_TSANG_JYE         0x01
#define IND_PINYIN            0x02
#define IND_ENGLISH_CHINESE   0x03
#define IND_ABC               0x04
#define IND_USER_DEFINED      0x05
#define IND_CAPS_LOCK         0x06    /*         |<-- Msg -->|<-- IM --->| */
#define IND_BLANK             0x07    /* |---|---|---|---|---|---|---|---| */
#define IND_NO_WORD           0x10    /* | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | */
#define IND_ERROR_MSG         0x18    /* |---|---|---|---|---|---|---|---| */
#define IND_RADICAL           0x20
#define IM_MASK               0x07
#define MSG_MASK              0x38

#define IND_DEFAULT_LEN 80

/* ----------  Define The UTF  Temporarily define --*/

/* For IMShortForm Indicator                            */

#define S_ALPHA_NUM_UTF          " ASCII"
#define S_CHINESE_UTF            " 中文 "
#define S_HALF_SIZE_UTF          " 半角 "
#define S_FULL_SIZE_UTF          " 全角 "
#define S_TSANG_JYE_UTF          " 仓頡 "
#define S_PINYIN_UTF             " 拼音 "
#define S_LEGEND_UTF             " 联想 "
#define S_ENGLISH_CHINESE_UTF    " 英中 "
#define S_ABC_UTF                " 智能ABC"
#define S_USER_DEFINED_UTF       " 用户 "
#define S_CAPS_LOCK_UTF          " 大写 "
#define S_BLANK_UTF              "       "
#define S_ERROR_MSG_UTF          " 按错键 "
#define S_NO_WORD_UTF            " 无字 "
#define SB_BLANK                 "     "
#define IND_SHORT_FORM           ( sizeof(S_ALPHA_NUM_UTF) - 1           \
                                 + sizeof(S_HALF_SIZE_UTF) - 1           \
                                 + sizeof(S_PINYIN_UTF)    - 1           \
                                 + sizeof(SB_BLANK) - 1 + 57             )


/* For IMLongForm indicator                             */

#define ALPHA_NUM_UTF          " 英文     "
#define CHINESE_UTF            " 中文     "
#define HALF_SIZE_UTF          " 半角     "
#define FULL_SIZE_UTF          " 全角     "
#define TSANG_JYE_UTF          " 仓頡     "
#define PINYIN_UTF             " 拼音     "
#define LEGEND_UTF             " 联想     "
#define ENGLISH_CHINESE_UTF    " 英中     "
#define ABC_UTF                " 智能ABC  " 
#define USER_DEFINED_UTF       " 用户     " 
#define CAPS_LOCK_UTF          " 大写     "
#define BLANK_UTF              "         "
#define DB_BLANK               "         "
#define ERROR_MSG_UTF          " 按错键     "
#define NO_WORD_UTF            " 无字     "
#define IND_LONG_FORM          ( sizeof(ALPHA_NUM_UTF) - 1           \
                               + sizeof(HALF_SIZE_UTF) - 1           \
                               + sizeof(PINYIN_UTF)    - 1           \
                               + sizeof(DB_BLANK) - 1 + 56)

/**************************************/
/*   Used by ZHIMProcess.c            */
/**************************************/

#define TEXT_ON         1
#define TEXT_OFF        0
#define ZHedProcError    IMError

/**************************************/
/*       ENV variable definition      */
/**************************************/

#define ENVVAR             "ZHIMPROFILE"
#define PYUSRDICT          "ZHIMPYUSRDICT"
#define TJUSRDICT          "ZHIMTJUSRDICT"
#define LEUSRDICT          "ZHIMLEUSRDICT"
#define ENUSRDICT          "ZHIMENUSRDICT"
#define ABCUSRREM          "ZHIMABCUSRREM"
#define ABCUSRDICT         "ZHIMABCUSRDICT"
#define PYSYSDICT          "ZHIMPYSYSDICT"
#define TJSYSDICT          "ZHIMTJSYSDICT"
#define LESYSDICT          "ZHIMLESYSDICT"
#define ENSYSDICT          "ZHIMENSYSDICT"
#define ABCSYSDICT         "ZHIMABCSYSDICT"
#define UDDICT             "ZHIMUDDICT"
#define USERDEFINEDIM      "ZHIMUDIM"

/***********************************************/
/*  dictionary name default value defintion    */
/***********************************************/

#define DEFAULTPYUSRDICT    "/usr/lpp/zhls/dict/pyusrdict"
#define DEFAULTTJUSRDICT    "/usr/lpp/zhls/dict/tjusrdict"
#define DEFAULTLEUSRDICT    "/usr/lpp/zhls/dict/leusrdict"
#define DEFAULTENUSRDICT    "/usr/lpp/zhls/dict/enusrdict"
#define DEFAULTPYSYSDICT1   "/usr/lpp/zhls/dict/pysysdict_comm"
#define DEFAULTPYSYSDICT2   "/usr/lpp/zhls/dict/pysysdict_gb"
#define DEFAULTPYSYSDICT3   "/usr/lpp/zhls/dict/pysysdict_cns"
#define DEFAULTPYSYSDICT4   "/usr/lpp/zhls/dict/pysysdict_jk"
#define DEFAULTTJSYSDICT    "/usr/lpp/zhls/dict/tjsysdict"
#define DEFAULTLESYSDICT    "/usr/lpp/zhls/dict/lesysdict"
#define DEFAULTENSYSDICT    "/usr/lpp/zhls/dict/ensysdict"
#define DEFAULTABCSYSDICT1  "/usr/lpp/zhls/dict/abcsysdict.cwd_s"
#define DEFAULTABCSYSDICT2  "/usr/lpp/zhls/dict/abcsysdict.cwd_t"
#define DEFAULTABCSYSDICT3  "/usr/lpp/zhls/dict/abcsysdict.ovl"
#define DEFAULTUDDICT       "/usr/lpp/zhls/dict/uddict"
#define DEFAULTPROFILE      "/usr/lpp/zhls/defaults/zhimrc"
#define FILENAME1           "zhimrc"
#define FILENAME2           "pyusrdict_utf"
#define FILENAME3           "leusrdict_utf"
#define FILENAME4           "enusrdict_utf"
#define FILENAME5           ".abcusrrem_utf"
#define FILENAME6           ".abcusrdict_utf"
#define FILENAME7           "pysysdict_comm"
#define FILENAME8           "pysysdict_gb"
#define FILENAME9           "pysysdict_cns"
#define FILENAME10          "pysysdict_jk"
#define FILENAME11          "lesysdict"
#define FILENAME12          "ensysdict"
#define FILENAME13          "abcsysdict.cwd_s"
#define FILENAME14          "abcsysdict.cwd_t"
#define FILENAME15          "abcsysdict.ovl"
#define FILENAME16          "uddict_utf"
#define FILENAME17          "tjusrdict_utf"
#define FILENAME18          "tjsysdict"

/***********************************************/
/*  User defined IM default value defintion    */
/***********************************************/

#define DEFAULTUDIM         "/usr/lpp/zhls/defaults/zhudim"
#define FILENAMEUDIM        "zhudim"

#define MAX_CHAR_NO        20
#define SPACE              0x20
#define PATHFIND           3

/**************************************/
/*     dictionary type defintion      */
/**************************************/

#define PY_USER_DICT       1
#define LE_USER_DICT       PY_USER_DICT + 1
#define EN_USER_DICT       LE_USER_DICT + 1
#define ABC_USER_REM       EN_USER_DICT + 1 
#define ABC_USER_DICT      ABC_USER_REM + 1 
#define PY_SYS_DICT_COMM   ABC_USER_DICT + 1
#define PY_SYS_DICT_GB     PY_SYS_DICT_COMM + 1
#define PY_SYS_DICT_CNS    PY_SYS_DICT_GB + 1
#define PY_SYS_DICT_JK     PY_SYS_DICT_CNS + 1
#define LE_SYS_DICT        PY_SYS_DICT_JK + 1
#define EN_SYS_DICT        LE_SYS_DICT + 1
#define ABC_SYS_DICT_CWD_S EN_SYS_DICT + 1
#define ABC_SYS_DICT_CWD_T ABC_SYS_DICT_CWD_S + 1
#define ABC_SYS_DICT_OVL   ABC_SYS_DICT_CWD_T + 1
#define TJ_SYS_DICT        ABC_SYS_DICT_OVL + 1
#define TJ_USER_DICT       TJ_SYS_DICT + 1
#define UD_DICT            TJ_USER_DICT + 1

/*****************************************/
/* Input Method List Box                 */
/*****************************************/
#define PYTITLE            "拼音选择      剩余"
#define LINE               "━━━━━━━━━━━━━━━━━━━━"
#define BOTTOM1            "取消(ESC)"
#define BOTTOM2            "上页(PgUp) 下页(PgDn)"
#define ENTITLE            "英中选择      剩余"
#define LETITLE            "拼音联想选择  剩余"
#define ABCTITLE           "智能ABC选择   剩余"
#define TJTITLE            "仓頡选择      剩余"
#define UDTITLE            "用户定义选择  剩余"

/*****************************************/
/*  profile structure string definition  */
/*****************************************/

#define INITCHAR           "initchar"
#define INITSIZE           "initsize"
#define INITBEEP           "initbeep"
#define ZHIM_INIT_AN       "alpha_numerical"
#define ZHIM_INIT_PY       "pinyin"
#define ZHIM_INIT_TJ       "tsang_jye"
#define ZHIM_INIT_PYL      "pinyin legend"
#define ZHIM_INIT_EN       "english_chinese"
#define ZHIM_INIT_ABC      "abc"
#define ZHIM_INIT_UD       "user defined"
#define ZHIM_INIT_HALF     "half"
#define ZHIM_INIT_FULL     "full"
#define ZHIM_INIT_BEEP_ON   "beep_on"
#define ZHIM_INIT_BEEP_OFF  "beep_off"

/*************************************/
/*     read/write only definition    */
/*************************************/

#define READ_ONLY          0x04
#define WRITE_ONLY         0x02

/******************************************/
/*   indicator buffer string max length   */
/******************************************/

#define ZHIM_INDSTRMAXLEN   110

/*******************************/
/*     aux buffer max size     */
/*******************************/

#define ZHIM_AUXCOLMAX      80
#define ZHIM_AUXROWMAX      14

/*******************************/
/*     aux buffer format       */
/*******************************/

#define SHORTAUX           0
#define LONGAUX            1

/*****************************/
/*     error definition      */
/*****************************/

#define ZHIMDictError       1
#define ZHIMEDError         2

/******************  Global Variables   *****************************/

char   *profile_ptr;     /* pointer to one allocated profile memory */
char   *profile_offset;  /* set initial offset for further used     */
int    profile_size;     /* the size of profile                     */

/******************************************************/
/*             Dictionary data structure              */
/******************************************************/
typedef struct{
        char    *Py_usr_dict;      /* path to PY_Usr_Dict       */
        char    *Le_usr_dict;      /* path to LE_Usr_Dict       */
        char    *En_usr_dict;      /* path to EN_Usr_Dict       */
        char    *Abc_usr_rem;      /* path to ABC_Usr_Rem       */
        char    *Abc_usr_dict;     /* path to ABC_Usr_Dict      */
        char    *Py_sys_dict_comm; /* path to PY_Sys_Dict_Comm  */
        char    *Py_sys_dict_gb;   /* path to PY_Sys_Dict_gb    */
        char    *Py_sys_dict_cns;  /* path to PY_Sys_Dict_cns   */
        char    *Py_sys_dict_jk;   /* path to PY_Sys_Dict_jk    */
        char    *Le_sys_dict;      /* path to LE_Sys_Dict       */
        char    *En_sys_dict;      /* path to EN_Sys_Dict       */
        char    *Abc_sys_dict_cwd_s; /* path to Simplified ABC_Sys_Dict_Cwd  */
        char    *Abc_sys_dict_cwd_t; /* path to Traditional ABC_Sys_Dict_Cwd */
        char    *Abc_sys_dict_ovl; /* path to ABC_Sys_Dict_Ovl  */
        char    *Tj_sys_dict;      /* path to Tj_Sys_Dict       */
        char    *Tj_usr_dict;      /* path to Tj_Usr_Dict       */
        char    *Ud_dict;          /* parh to UD_Dict           */
}DictName;

/*************************************************/
/*         ZHIM Profile data structure           */
/*************************************************/
typedef struct {
        int           initchar;      /* PY/LE/EN/ABC/UD/AN/TJ*/
        int           initsize;      /* half/full size       */
        int           initbeep;      /* Beep sound is ON/OFF */
        DictName      dictname;      /* PY/LE/En/ABC/ sys/usr*/
                                     /* dict and Ud dict     */
        char          *ud_im_name;   /* User_defined IM      */
                                     /* Module Name          */
        int           (*ud_im_func)();/* User Defined IM Function */
} ZHimProfile;


/************************************************/
/*        data structure for word tree          */
/************************************************/
typedef struct element {
        struct element  *next;
        char            *leftword;
        char            *rightword;
} list;

/************************************************/
/*           ZHIM ED data structure             */
/************************************************/
typedef struct {
        int     ZHedID;        /* ZHIM ED identification                */
        char    *echobufs;    /* echo buffer string for text area     */
        char    *echobufa;    /* echo buffer attribute for text area  */
        char    *fixbuf;      /* buffer for final result (UTF)        */
        int     echobufsize;  /* echo buffer size                     */
        char    **auxbufs;    /* pointer to aux buf string address    */
        char    **auxbufa;    /* pointer to aux buf attribute address */
        AuxSize auxsize;      /* auxiliary area size                  */
}ZHIMed,*ZHIMED;

/**********************************************/
/*          ZHIM-FEP data structure           */
/**********************************************/
typedef struct _ZHIMFEP{
        IMFepCommon     common;         /* IMFEP common portion */
        int             zhimver;        /* ZHIM version         */
        IMKeymap        *keymap;        /* ZHIM keymap          */
}ZHIMfep,*ZHIMFEP;

/************************************************/
/*           ZHIM Object data structure         */
/************************************************/
typedef struct _ZHIMobject{
        IMObjectCommon  imobject;       /* IM common information        */
        IMTextInfo      textinfo;       /* ZHIM text information        */
        IMAuxInfo       auxinfo;        /* ZHIM auxiliary information   */
        IMSTR           string;         /* ZHIM string for GetString    */
        IMSTR           indstr;         /* ZHIM indicator string        */
        IMIndicatorInfo indinfo;        /* ZHIM indicator string        */
        IMQueryState    querystate;     /* ZHIM query state             */
        caddr_t         auxID;          /* Auxiliary ID created by CB   */
        caddr_t         convert_str;    /* final UTF(Convert key press) */
        int             auxIDflag;      /* TRUE : Aux area is created   */
        int             auxstate;       /* Auxiliary area state         */
        int             textstate;      /* the state of text area       */
        int             state_aux_text; /* return value of IMProcess    */
        IMBuffer        output;         /* output buffer                */
        ZHIMed          zhedinfo;       /* ZHIM ed information          */
}ZHIMobj,*ZHIMOBJ;

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

#define ZHIM_VALID_BITS          (ShiftMask|LockMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask)

uint    erase_flag;        /* erase previous buffer   */
