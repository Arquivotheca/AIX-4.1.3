/* @(#)14	1.1  src/bos/usr/lib/nls/loc/CN.im/cnedinit.h, ils-zh_CN, bos41B, 9504A 12/19/94 14:33:41  */
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: none
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
/******************************************************************************/
/*                                                                            */
/* MODULE NAME:         CNEDINIT.H                                            */
/*                                                                            */
/* DESCRIPTIVE NAME:    Chinese Input Method private header file              */
/*                                                                            */
/* FUNCTION:                                                                  */
/*                                                                            */
/* MODULE TYPE:         C                                                     */
/*                                                                            */
/******************************************************************************/

/* -------------------  include file  ------------------------------ */
#include "stdio.h"

/**************** CNIM Profile data structure  ***********************/

typedef struct{

        char    *Py_usr_dict;       /* path to PY_Usr_Dict          */
        char    *Le_usr_dict;       /* path to LE_Usr_Dict          */
        char    *En_usr_dict;       /* path to EN_Usr_Dict          */
        char    *Fss_usr_dict;      /* path to FSS_Usr_Dict         */
        char    *Fs_usr_dict;       /* path to FS_Usr_Dict         */
        char    *Fsph_usr_dict;     /* path to FSPH_Usr_Dict         */
        char    *Abc_usr_rem;       /* path to ABC_Usr_Rem          */
        char    *Abc_usr_dict;      /* path to ABC_Usr_Dict         */
        char    *Py_sys_dict;       /* path to PY_Sys_Dict          */
        char    *Le_sys_dict;       /* path to LE_Sys_Dict          */
        char    *En_sys_dict;       /* path to EN_Sys_Dict          */
        char    *Fss_sys_dict;      /* path to FSS_Sys_Dict         */
        char    *Fssjm_sys_dict;    /* path to FSSJM_Sys_Dict       */
        char    *Fs_sys_dict;       /* path to FS_Sys_Dict         */
        char    *Fsph_sys_dict;     /* path to FSPH_Sys_Dict         */
        char    *Abc_sys_dict_cwd;  /* path to ABC_Sys_Dict_Cwd     */
        char    *Abc_sys_dict_ovl;  /* path to ABC_Sys_Dict_Ovl     */
        char    *Ud_dict;           /* path to UD_Dict              */

}DName;

typedef struct{

        int             initchar;       /* PY/LE/EN/ABC/AN      */
        int             initsize;       /* half/full size       */
        int             initbeep;       /* Beep sound is ON/OFF */
        DName           dictname;       /* PY/LE/EN/ABC sys/usr and UD dict */
        char            *ud_im_name;    /* User_defined IM Module Name     */
        int             (*ud_im_func)(); /* User Defined ED Function*/
}CNedProfile;

/*************************************/
/* Change Information In Echo Buffer */
/*************************************/
typedef struct {
        int             flag       ;   /* TRUE If Changed; Otherwise, FALSE   */
        int             chtoppos   ;   /* The First Position Of Change In Echo*/
        int             chlenbytes ;   /* Changed Len. From 1st Pos. Of Change*/
}   EchoBufChanged;

/*************************************/
/*        Auxiliary Area Size        */
/*************************************/
typedef struct {
        int             itemsize ;     /* Length (bytes) Of Each Aux Item     */
        int             itemnum  ;     /* Number Of Item (line)               */
}   AuxSize;

/*************************************/
/*  Auxiliary Area Cursor Positoin   */
/*************************************/
typedef struct {
        int             colpos ;       /* Column Position (byte) Within A Item*/
        int             rowpos ;       /* Row (item) Number Counter           */
}   AuxCurPos;

/*************************************/
/*     Definition Of Input Mode      */
/*************************************/
typedef struct {
        unsigned        ind0 :4 ;     /* ALPH_NUM_MODE / ROW_COLUMN_MODE /    */
                                      /* PINYIN_MODE / ENGLISH_CHINESE_MODE   */
                                      /* ABC_MODE / USER_DEFINED_MODE /       */
                                      /* CAPS_LOCK_MODE /FIVESTROKE_STYLE_MODE*/
                                      /* FIVESTROKE_MODE                      */
        unsigned        ind1 :4 ;     /* FULL / HALF                          */
        unsigned        ind2 :4 ;     /* SELECT_OFF /ROW_COLUMN_SELECT_ON/    */
                                      /* PINYIN_SELECT_ON /                   */
                                      /* PINYIN_LEGEND_SELECT_ON /            */
                                      /* ENGLISH_CHINESE_SELECT_ON /          */
                                      /* ABC_SELECT_ON /USER_DEFINED_SELECT_ON*/
        unsigned        ind3 :4 ;     /* SUPPRESSED_MODE / NORMAL_MODE        */
        unsigned        ind4 :4 ;     /* INSERT_MODE / REPLACE_MODE           */
        unsigned        ind5 :4 ;     /* BLANK / ERROR1 / ERROR2 / RADICAL    */
        unsigned        ind6 :2 ;     /* INPUT_SELECTED / INPUT_NO_SELECTED   */
        unsigned        ind7 :2 ;     /* LEGEND_ON / LEGEND_OFF               */
        unsigned        dummy:4 ;
}   InputMode;

/*************************************/
/*Time Of Last Access Dictionary File*/
/*************************************/
typedef struct {
        char            pytime[30];    /* Time Of Last Access Pinyin Dic.File */
        char            entime[30];    /* Time Of Last Access ENGLISH_CHINESE */
                                       /* Dic. File                           */
        char            letime[30];    /* Time of Last Access PINYIN_LEGEND   */
                                       /* Dic. File                           */
        char            fsstime[30];   /* Time of Last Access FIVESTROKE      */
                                       /* _STYLE Dic. File                    */
        char            fstime[30];    /* Time of Last Access FIVESTROKE      */
                                       /* Dic. File                           */
        char            abctime[2][30];/* Time od Last Access ABC Dic. File   */
        char            udtime[30];    /* Time of Last Access User Defined    */
                                       /* Dic. File                           */
}   CTime;

        
/************************************/
/*        Master Index              */
/************************************/
typedef struct {
        unsigned char *index;
        int           length;
}   MI;


/*************************************/
/*   Master Index Of Dictionary File */
/*************************************/
typedef struct {
        MI     pysysmi[26] ;      /* Master Index Of Pinyin System Dictionary */
        MI     lesysmi[85] ;      /* Master Index Of Legend System Dictionary */
        MI     pyusrmi[26] ;      /* Master Index Of Pinyin User Dictionary   */
        MI     leusrmi[85] ;      /* Master Index Of Legend User Dictionary   */
        MI     ensysmi[26] ;      /* Master Index Of English System Dictionary*/
        MI     enusrmi[26] ;      /* Master Index Of English User Dictionary  */
        MI     fsssysmi[25] ;     /* Master Index Of Five Stroke Style System */
                                  /* Dictionary                               */
        MI     fssusrmi[25] ;     /* Master Index Of Five Stroke Style User   */
                                  /* Dictionary                               */
        MI     fssjmsysmi[25] ;   /* Master Index Of Five Stroke Simple Style */
                                  /* System Dictionary                        */
        MI     fssysmi[5] ;       /* Master Index Of Five Stroke System       */
                                  /* Dictionary                               */
        MI     fsphsysmi[5] ;     /* Master Index Of Five Stroke System       */
                                  /* Dictionary                               */
        MI     fsusrmi[5] ;       /* Master Index Of Five Stroke User         */
                                  /* Dictionary                               */
        MI     fsphusrmi[5] ;     /* Master Index Of Five Stroke User         */
                                  /* Dictionary                               */
        unsigned char *abcsysmi ; /* Pointer To ABC System Dictionary Master  */
                                  /* Index                                    */
        unsigned char *abcusrmi ; /* Pointer To ABC User Dictionary Master    */
                                  /* Index                                    */
        unsigned char   *udmi  ;  /* Pointer To User_Defined Master Index     */
}   MasterIndex;

/*************************************/
/*          Candidate                */
/*************************************/
typedef struct {
        unsigned char   *cand;       /* Pointer To Candidate                */
        unsigned char   *curptr;     /* Pointer To Current Candidates       */
        unsigned short  allcandno;   /* Number Of All Candidates            */
        unsigned short  more   ;     /* Number Of Remained Candidates       */
}   Cand_Struct;

/*************************************/
/*        File  Descriptor           */
/*************************************/
typedef struct {
        FILE            *pysysfd ;     /* Pinyin System Dic. File Descriptor  */
        FILE            *pyusrfd ;     /* Pinyin User Dic. File Descriptor    */
        FILE            *lesysfd ;     /* Legend System Dic. File Descriptor  */
        FILE            *leusrfd ;     /* Legend User Dic. File Descriptor    */
        FILE            *ensysfd ;     /* English_Chinese System Dic. File    */
                                       /* Descriptor                          */
        FILE            *enusrfd ;     /* English_Chinese User Dic. File      */
                                       /* Descriptor                          */
        FILE            *fsssysfd ;    /* Five Stroke Style System Dic. File  */
                                       /* Descriptor                          */
        FILE            *fssusrfd ;    /* Five Stroke Style User Dic. File    */
                                       /* Descriptor                          */
        FILE            *fssjmsysfd ;  /* Five Stroke Simple Style System Dic.*/
                                       /* File Descriptor                     */
        FILE            *fssysfd ;     /* Five Stroke System Dic. File        */
                                       /* Descriptor                          */
        FILE            *fsphsysfd ;   /* Five Stroke System Dic. File        */
                                       /* Descriptor                          */
        FILE            *fsusrfd ;     /* Five Stroke User Dic. File          */
                                       /* Descriptor                          */
        FILE            *fsphusrfd ;   /* Five Stroke User Dic. File          */
                                       /* Descriptor                          */
        FILE            *abcsysfd[2];  /* ABC System Dic. File Descriptor     */
        FILE            *abcusrfd[2];  /* ABC User Dic. File Descriptor       */
        FILE            *udfd;         /* User Defined Dic. File Descriptor   */
}   FileDescriptor;

/*************************************/
/*   Dictionary File Name            */
/*************************************/
typedef struct {
        char        *pysysfname;      /* Pinyin System Dictionary File Name  */
        char        *pyusrfname;      /* Pinyin User Dictionary File Name    */
        char        *lesysfname;      /* Legend System Dictionary File Name  */
        char        *leusrfname;      /* Legend User Dictionary File Name    */
        char        *ensysfname;      /* English_Chinese System Dict. File   */
                                      /* Name                                */
        char        *enusrfname;      /* English_Chinese User Dictionary File*/
                                      /* Name                                */
        char        *fsssysfname;     /* Five Stroke Style System Dict. File */
                                      /* Name                                */
        char        *fssusrfname;     /* Five Stroke Style User   Dict. File */
                                      /* Name                                */
        char        *fssjmsysfname;   /* Five Stroke Simple Style System Dict.*/
                                      /* File Name                           */
        char        *fssysfname;      /* Five Stroke System Dict. File Name  */
        char        *fsusrfname;      /* Five Stroke System Dict. File Name  */
        char        *fsphsysfname;    /* Five Stroke System Dict. File Name  */
        char        *fsphusrfname;    /* Five Stroke User   Dict. File Name  */
        char        *abcsysfname[2];  /* ABC System Dictionary File Name     */
        char        *abcusrfname[2];  /* ABC User Dictionary File Name       */
        char        *udfname;         /* User Defined Dictionary File Name   */
}   FileName;

/*************************************/
/*  CNIMED internal control block     */
/*************************************/
typedef struct {
        unsigned char   *echobufs  ;   /* Pointer To Echo Buffer              */
        unsigned char   *echobufa  ;   /* Pointer To Echo Attribute Buffer    */
        unsigned char   *edendbuf  ;   /* Pointer To Edit-End Buffer          */
        unsigned char   **auxbufs  ;   /* Pointer To Aux-Area Buffer          */
        unsigned char   **auxbufa  ;   /* Pointer To Aux-Area Attribute Buffer*/
        EchoBufChanged  echochfg   ;   /* Changed Flag For Echo Buffer        */
        int             echocrps   ;   /* Cursor Position In Echo Buffer      */
        int             eccrpsch   ;   /* Changed Flag For Cursor Pos. (echo) */
        int             echoacsz   ;   /* Active Length In Echo Buffer        */
        int             echosize   ;   /* Echo Buffer Size                    */
        int             echoover   ;   /* Overflow Limit Within Echo Buffer   */
        int             edendacsz  ;   /* Active Size In Edit-End Buffer      */
        int             auxchfg    ;   /* Changed Flag For Aux-Area Buffer    */
        int             auxuse     ;   /* Aux-Area Use Flag                   */
        AuxCurPos       auxcrps    ;   /* Cursor Position In Aux-Area Buffer  */
        int             auxcrpsch  ;   /* Changed Flag For Cursor Pos. (aux)  */
        AuxSize         auxacsz    ;   /* Active Size In Aux-Area Buffer      */
        AuxSize         auxsize    ;   /* Aux-Area Buffer Size                */
        int             indchfg    ;   /* Changed Flag For Indicator Buffer   */
        int             indhide    ;   /* Hidden Flag For Indicator           */
        InputMode       imode      ;   /* Input Mode                          */
        int             isbeep     ;   /* Beep Sound Requested                */
        int             beepallowed;   /* Beep Sound Allowed                  */
        unsigned char   *rcinbuf   ;   /* Row/Column input buffers            */
        unsigned char   *pyinbuf   ;   /* Pinyin Input Buffers                */
        unsigned char   *eninbuf   ;   /* English_Chinese input buffers       */
        unsigned char   *fssinbuf  ;   /* Five Stroke Style input buffers     */
        unsigned char   *fsinbuf   ;   /* Five Stroke input buffers           */
        unsigned char   *abcinbuf  ;   /* ABC(Chinese Word) input buffers     */
        unsigned char   *udinbuf   ;   /* User defined input buffers          */
        int             inputlen   ;   /* Length Of Input Buffer              */
        CTime           ctime      ;   /* Time Of Last Access Dictionary File */
        MasterIndex     mi         ;   /* Master Index Of Dictionary File     */
        FileDescriptor  fd         ;   /* File Descriptor                     */
        int             ret        ;   /* Return Code                         */
        int             le_ret     ;   /* Legend Return Code                  */
        Cand_Struct     rcstruct   ;   /* Row_Column Candidate Structure      */
        Cand_Struct     pystruct   ;   /* Pinyin Candidate Structure          */
        Cand_Struct     lestruct   ;   /* Pinyin Legend Candidate Structure   */
        Cand_Struct     enstruct   ;   /* English_Chinese Candidate Structure */
        Cand_Struct     fssstruct  ;   /* Five Stroke Style Candidate Struct  */
        Cand_Struct     fsstruct   ;   /* Five Stroke Candidate Struct        */
        Cand_Struct     abcstruct  ;   /* Abc Candidate Structure             */
        Cand_Struct     udstruct   ;   /* User_Defined Candidate Structure    */
        FileName        fname      ;   /* Dictionary File Name                */
        int             flag       ;   /* TRUE If Changed; Otherwise, FALSE   */
/*        int             *udimfunc();    User_defined IM Entry Function      */
}   FEPCB;
