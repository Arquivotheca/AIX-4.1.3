/* @(#)94	1.1  src/bos/usr/lib/nls/loc/CN.im/chinese.h, ils-zh_CN, bos41B, 9504A 12/19/94 14:33:00  */
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
#include "cned.h"
#include "cnedinit.h"

/**********************************************/
/*      IMIndicatorInfo.unique declaration    */
/**********************************************/
#define IND_INITIALIZE        0x00
#define IND_ALPHA_NUM         0x00
#define IND_ROW_COLUMN        0x01
#define IND_PINYIN            0x02
#define IND_ENGLISH_CHINESE   0x03
#define IND_ABC               0x04
#define IND_USER_DEFINED      0x05
#define IND_FIVESTROKE        0x06
#define IND_FIVESTROKE_STYLE  0x07    /*         |<-- Msg -->|<-- IM --->| */ 
#define IND_BLANK             0x11    /* |---|---|---|---|---|---|---|---| */
#define IND_NO_WORD           0x10    /* | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | */
#define IND_ERROR_MSG         0x18    /* |---|---|---|---|---|---|---|---| */
#define IND_RADICAL           0x20
#define IM_MASK               0x07
#define MSG_MASK              0x38

#define IND_DEFAULT_LEN 80

/* ----------  Define The EUC  Temporarily define --*/

/* For IMShortForm Indicator                            */

#define S_ALPHA_NUM_EUC          " ASCII"
#define S_CHINESE_EUC            " 中文 "
#define S_HALF_SIZE_EUC          " 半角 "
#define S_FULL_SIZE_EUC          " 全角 "
#define S_ROW_COLUMN_EUC         " 区位 "
#define S_PINYIN_EUC             " 拼音 "
#define S_LEGEND_EUC             " 联想 "
#define S_ENGLISH_CHINESE_EUC    " 英中 "
#define S_ABC_EUC                " 智能ABC"
#define S_USER_DEFINED_EUC       " 用户 "
#define S_FIVESTROKE_STYLE_EUC   " 五笔字型 "
#define S_FIVESTROKE_EUC         " 五笔划 "
#define S_CAPS_LOCK_EUC          " 大写 "
#define S_BLANK_EUC              "       "
#define S_ERROR_MSG_EUC          " 按错键 "
#define S_NO_WORD_EUC            " 无字 "
#define SB_BLANK                 "     "
#define IND_SHORT_FORM           ( sizeof(S_ALPHA_NUM_EUC) - 1           \
                                 + sizeof(S_HALF_SIZE_EUC) - 1           \
                                 + sizeof(S_PINYIN_EUC)    - 1           \
                                 + sizeof(SB_BLANK) - 1 + 57             )


/* For IMLongForm indicator                             */

#define ALPHA_NUM_EUC          " 英文     "
#define CHINESE_EUC            " 中文     "
#define HALF_SIZE_EUC          " 半角     "
#define FULL_SIZE_EUC          " 全角     "
#define ROW_COLUMN_EUC         " 区位     "
#define PINYIN_EUC             " 拼音     "
#define LEGEND_EUC             " 联想     "
#define ENGLISH_CHINESE_EUC    " 英中     "
#define ABC_EUC                " 智能ABC  " 
#define USER_DEFINED_EUC       " 用户     " 
#define FIVESTROKE_STYLE_EUC   " 五笔字型 "
#define FIVESTROKE_EUC         " 五笔划   "
#define CAPS_LOCK_EUC          " 大写     "
#define BLANK_EUC              "         "
#define DB_BLANK               "         "
#define ERROR_MSG_EUC          " 按错键     "
#define NO_WORD_EUC            " 无字     "
#define IND_LONG_FORM          ( sizeof(ALPHA_NUM_EUC) - 1           \
                               + sizeof(HALF_SIZE_EUC) - 1           \
                               + sizeof(PINYIN_EUC)    - 1           \
                               + sizeof(DB_BLANK) - 1 + 56)

/**************************************/
/*   Used by CNIMProcess.c            */
/**************************************/

#define TEXT_ON         1
#define TEXT_OFF        0
#define CNedProcError    IMError

/**************************************/
/*       ENV variable definition      */
/**************************************/

#define ENVVAR             "CNIMPROFILE"
#define PYUSRDICT          "CNIMPYUSRDICT"
#define LEUSRDICT          "CNIMLEUSRDICT"
#define ENUSRDICT          "CNIMENUSRDICT"
#define ABCUSRREM          "CNIMABCUSRREM"
#define ABCUSRDICT         "CNIMABCUSRDICT"
#define FSSUSRDICT         "CNIMFSSUSRDICT"
#define FSUSRDICT          "CNIMFSUSRDICT"
#define PYSYSDICT          "CNIMPYSYSDICT"
#define LESYSDICT          "CNIMLESYSDICT"
#define ENSYSDICT          "CNIMENSYSDICT"
#define ABCSYSDICT         "CNIMABCSYSDICT"
#define FSSSYSDICT         "CNIMFSSSYSDICT"
#define FSSJMSYSDICT       "CNIMFSSJMSYSDICT"
#define FSSYSDICT          "CNIMFSSYSDICT"
#define FSPHSYSDICT        "CNIMFSPHSYSDICT"
#define FSPHUSRDICT        "CNIMFSPHUSRDICT"
#define UDDICT             "CNIMUDDICT"
#define USERDEFINEDIM      "CNIMUDIM"

/***********************************************/
/*  dictionary name default value defintion    */
/***********************************************/

#define DEFAULTPYUSRDICT    "/usr/lpp/cnls/dict/pyusrdict"
#define DEFAULTLEUSRDICT    "/usr/lpp/cnls/dict/leusrdict"
#define DEFAULTENUSRDICT    "/usr/lpp/cnls/dict/enusrdict"
#define DEFAULTFSSUSRDICT   "/usr/lpp/cnls/dict/fssusrdict"
#define DEFAULTFSUSRDICT    "/usr/lpp/cnls/dict/fsusrdict"
#define DEFAULTPYSYSDICT    "/usr/lpp/cnls/dict/pysysdict"
#define DEFAULTPYLSYSDICT   "/usr/lpp/cnls/dict/lesysdict"
#define DEFAULTENSYSDICT    "/usr/lpp/cnls/dict/ensysdict"
#define DEFAULTABCSYSDICT1  "/usr/lpp/cnls/dict/abcsysdict.cwd"
#define DEFAULTABCSYSDICT2  "/usr/lpp/cnls/dict/abcsysdict.ovl"
#define DEFAULTFSSSYSDICT   "/usr/lpp/cnls/dict/fsssysdict"
#define DEFAULTFSSJMSYSDICT "/usr/lpp/cnls/dict/fssjmsysdict"
#define DEFAULTFSSYSDICT    "/usr/lpp/cnls/dict/fssysdict"
#define DEFAULTFSPHSYSDICT  "/usr/lpp/cnls/dict/fsphsysdict"
#define DEFAULTFSPHUSRDICT  "/usr/lpp/cnls/dict/fsphusrdict"
#define DEFAULTUDDICT       "/usr/lpp/cnls/dict/uddict"
#define DEFAULTPROFILE      "/usr/lpp/cnls/defaults/cnimrc"
#define FILENAME1           "cnimrc"
#define FILENAME2           "pyusrdict"
#define FILENAME3           "leusrdict"
#define FILENAME4           "enusrdict"
#define FILENAME5           ".abcusrrem"
#define FILENAME6           ".abcusrdict"
#define FILENAME7           "pysysdict"
#define FILENAME8           "lesysdict"
#define FILENAME9           "ensysdict"
#define FILENAME10          "abcsysdict.cwd"
#define FILENAME11          "abcsysdict.ovl"
#define FILENAME12          "uddict"
#define FILENAME13          "fsssysdict"
#define FILENAME14          "fssusrdict"
#define FILENAME15          "fssysdict"
#define FILENAME16          "fsusrdict"
#define FILENAME17          "fssjmsysdict"
#define FILENAME18          "fsphsysdict"
#define FILENAME19          "fsphusrdict"

/***********************************************/
/*  User defined IM default value defintion    */
/***********************************************/

#define DEFAULTUDIM         "/usr/lpp/cnls/defaults/cnudim"
#define FILENAMEUDIM        "cnudim"

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
#define FSS_USER_DICT      ABC_USER_DICT + 1 
#define PY_SYS_DICT        FSS_USER_DICT + 1
#define LE_SYS_DICT        PY_SYS_DICT + 1
#define EN_SYS_DICT        LE_SYS_DICT + 1
#define ABC_SYS_DICT_CWD   EN_SYS_DICT + 1
#define ABC_SYS_DICT_OVL   ABC_SYS_DICT_CWD + 1
#define FSS_SYS_DICT       ABC_SYS_DICT_OVL + 1
#define UD_DICT            FSS_SYS_DICT + 1
#define FS_SYS_DICT        UD_DICT + 1
#define FS_USER_DICT       FS_SYS_DICT + 1
#define FSSJM_SYS_DICT     FS_USER_DICT + 1
#define FSPH_SYS_DICT      FSSJM_SYS_DICT + 1
#define FSPH_USER_DICT     FSPH_SYS_DICT + 1

/*****************************************/
/*  profile structure string definition  */
/*****************************************/

#define INITCHAR           "initchar"
#define INITSIZE           "initsize"
#define INITBEEP           "initbeep"
#define CNIM_INIT_AN       "alpha_numerical"
#define CNIM_INIT_PY       "pinyin"
#define CNIM_INIT_PYL      "pinyin legend"
#define CNIM_INIT_EN       "english_chinese"
#define CNIM_INIT_ABC      "abc"
#define CNIM_INIT_FSS      "five stroke style"
#define CNIM_INIT_FS       "five stroke "
#define CNIM_INIT_UD       "user defined"
#define CNIM_INIT_HALF     "half"
#define CNIM_INIT_FULL     "full"
#define CNIM_INIT_BEEP_ON   "beep_on"
#define CNIM_INIT_BEEP_OFF  "beep_off"

/*************************************/
/*     read/write only definition    */
/*************************************/

#define READ_ONLY          0x04
#define WRITE_ONLY         0x02

/******************************************/
/*   indicator buffer string max length   */
/******************************************/

#define CNIM_INDSTRMAXLEN   110

/*******************************/
/*     aux buffer max size     */
/*******************************/

#define CNIM_AUXCOLMAX      80
#define CNIM_AUXROWMAX      14

/*******************************/
/*     aux buffer format       */
/*******************************/

#define SHORTAUX           0
#define LONGAUX            1

/*****************************/
/*     error definition      */
/*****************************/

#define CNIMDictError       1
#define CNIMEDError         2

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
        char    *Fss_usr_dict;     /* path to FSS_Usr_Dict      */
        char    *Fs_usr_dict;      /* path to FS_Usr_Dict      */
        char    *Fsph_usr_dict;    /* path to FSPH_Usr_Dict      */
        char    *Abc_usr_rem;      /* path to ABC_Usr_Rem       */
        char    *Abc_usr_dict;     /* path to ABC_Usr_Dict      */
        char    *Py_sys_dict;      /* path to PY_Sys_Dict       */
        char    *Le_sys_dict;      /* path to LE_Sys_Dict       */
        char    *En_sys_dict;      /* path to EN_Sys_Dict       */
        char    *Fss_sys_dict;     /* path to FSS_Sys_Dict     */
        char    *Fssjm_sys_dict;   /* path to FSSJM_Sys_Dict   */
        char    *Fs_sys_dict;      /* path to FS_Sys_Dict     */
        char    *Fsph_sys_dict;    /* path to FSPH_Sys_Dict     */
        char    *Abc_sys_dict_cwd; /* path to ABC_Sys_Dict_Cwd  */
        char    *Abc_sys_dict_ovl; /* path to ABC_Sys_Dict_Ovl  */
        char    *Ud_dict;          /* parh to UD_Dict           */
}DictName;

/*************************************************/
/*         CNIM Profile data structure           */
/*************************************************/
typedef struct {
        int           initchar;      /* PY/LE/EN/ABC/UD/AN   */
        int           initsize;      /* half/full size       */
        int           initbeep;      /* Beep sound is ON/OFF */
        DictName      dictname;      /* PY/LE/En/ABC/ sys/usr*/
                                     /* dict and Ud dict     */
        char          *ud_im_name;   /* User_defined IM      */
                                     /* Module Name          */
        int           (*ud_im_func)();/* User Defined IM Function */
} CNimProfile;


/************************************************/
/*        data structure for word tree          */
/************************************************/
typedef struct element {
        struct element  *next;
        char            *leftword;
        char            *rightword;
} list;

/************************************************/
/*           CNIM ED data structure             */
/************************************************/
typedef struct {
        int     CNedID;        /* CNIM ED identification                */
        char    *echobufs;    /* echo buffer string for text area     */
        char    *echobufa;    /* echo buffer attribute for text area  */
        char    *fixbuf;      /* buffer for final result (EUC)        */
        int     echobufsize;  /* echo buffer size                     */
        char    **auxbufs;    /* pointer to aux buf string address    */
        char    **auxbufa;    /* pointer to aux buf attribute address */
        AuxSize auxsize;      /* auxiliary area size                  */
}CNIMed,*CNIMED;

/**********************************************/
/*          CNIM-FEP data structure           */
/**********************************************/
typedef struct _CNIMFEP{
        IMFepCommon     common;         /* IMFEP common portion */
        int             cnimver;        /* CNIM version         */
        IMKeymap        *keymap;        /* CNIM keymap          */
}CNIMfep,*CNIMFEP;

/************************************************/
/*           CNIM Object data structure         */
/************************************************/
typedef struct _CNIMobject{
        IMObjectCommon  imobject;       /* IM common information        */
        IMTextInfo      textinfo;       /* CNIM text information        */
        IMAuxInfo       auxinfo;        /* CNIM auxiliary information   */
        IMSTR           string;         /* CNIM string for GetString    */
        IMSTR           indstr;         /* CNIM indicator string        */
        IMIndicatorInfo indinfo;        /* CNIM indicator string        */
        IMQueryState    querystate;     /* CNIM query state             */
        caddr_t         auxID;          /* Auxiliary ID created by CB   */
        caddr_t         convert_str;    /* final EUC(Convert key press) */
        int             auxIDflag;      /* TRUE : Aux area is created   */
        int             auxstate;       /* Auxiliary area state         */
        int             textstate;      /* the state of text area       */
        int             state_aux_text; /* return value of IMProcess    */
        IMBuffer        output;         /* output buffer                */
        CNIMed          cnedinfo;       /* CNIM ed information          */
}CNIMobj,*CNIMOBJ;

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

#define CNIM_VALID_BITS          (ShiftMask|LockMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask)

uint    erase_flag;        /* erase previous buffer   */
