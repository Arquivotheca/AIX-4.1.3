/* @(#)11  1.4  src/bos/usr/lib/nls/loc/imt/tfep/tedinit.h, libtw, bos41J, 9520A_all 5/10/95 13:52:56 */
/*
 *   COMPONENT_NAME: LIBTW
 *
 *   FUNCTIONS: none
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

/******************************************************************************/
/*                                                                            */
/* MODULE NAME:         TEDINIT.H                                             */
/*                                                                            */
/* DESCRIPTIVE NAME:    Chinese Input Method private header file              */
/*                                                                            */
/* FUNCTION:                                                                  */
/*                                                                            */
/* MODULE TYPE:         C                                                     */
/*                                                                            */
/* COMPILER:            AIX C                                                 */
/*                                                                            */
/* AUTHOR:              Mei Lin                                               */
/*                                                                            */
/* CHANGE ACTIVITY:     010/16/91 - Modified                                  */
/*                                                                            */
/* STATUS:              Chinese Input Method Version                          */
/*                                                                            */
/* CATEGORY:            Header                                                */
/*                                                                            */
/******************************************************************************/

/*********************  Define constants *****************************/
#define LEARN_NO    20    /* V410 Learning number */
#define LIST_NO     10    /* V410 List number     */
#define PH_NO       13    /* V410                 */
#define ASTARTB      1    /* V410 Simplified A*B mode */
#define ABSTART      2    /* V410 Simplified AB* mode */
#define ASTART       3    /* V410 Simplified A*  mode */
#define PHONETIC_L   4    /* V410 Phonetic mode       */

/* -------------------  include file  ------------------------------ */
#include "stdio.h"
#include "iconv.h"

/**************** TIM Profile data structure  ***********************/

typedef struct{

        char    *PH_usr_dict;   /* path to PH_Usr_Dict          */
        char    *TJ_usr_dict;   /* path to TJ_Usr_Dict          */
        char    *PH_sys_dict;   /* path to PH_Sys_Dict          */
        char    *TJ_sys_dict;   /* path to TJ_Sys_Dict          */

}DName;

typedef struct{

        int             initchar;       /* PH/TJ/AN             */
        int             initsize;       /* half/full size       */
        int             initbeep;       /* Beep sound is ON/OFF */
        int             initselection;  /* ListBox is On/Off  V410  */
        int             initlearn;      /* Learning is On/Off V410  */
        int             initcodeset;    /* CodeSet will be used  V410  */
        iconv_t         iconv_flag;     /* opening iconv flag    @big5 */
        int             initkeyboard;   /* IBM or non-IBM layout @big5 */
        DName           dictname;       /* Ph/Tj sys/usr dict   */
        char            *learnfile;       /* V410 path to learning file */

}TedProfile;

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
        int             itemlen  ;     /* maxlength (bytes) of Each Aux Item @big5 */
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
        unsigned        ind0 :4 ;      /* ALPH_NUM / PHONETIC / TSANG_JYE     */
                                       /* INTERNAL_CODE / CAPS_LOCK           */
        unsigned        ind5 :4 ;      /* BLANK / ERR1 / ERR2 / RADICAL       */
        unsigned        ind1 :4 ;      /* FULL / HALF                         */
        unsigned        ind2 :4 ;      /* SELECT_OFF / PHONETIC_SELECT_ON /   */
                                       /* STJ_SELECT_ON / STROKE_SELECT_ON    */
        unsigned        ind3 :4 ;      /* SUPPRESSED_MODE / NORMAL_MODE       */
        unsigned        ind4 :4 ;      /* INSERT_MODE / REPLACE_MODE          */
        unsigned        dummy:8 ;
}   InputMode;

/*************************************/
/*Time Of Last Access Dictionary File*/
/*************************************/
typedef struct {
        char            phtime[30];    /* Time Of Last Access Phonetic Dic.   */
        char            tjtime[30];    /* Time Of Last Access STJ Dic. File   */
}   CTime;

/*************************************/
/*   Tsang_Jye System Master Index   */
/*************************************/
typedef struct {
        long            index  ;       /* Index                               */
        unsigned short  count  ;       /* Counter                             */
        unsigned short  stroke ;       /* Stroke Number Of Word               */
}   TjSysMI;

/*************************************/
/*   Tsang_Jye User Master Index     */
/*************************************/
typedef struct {
        unsigned short  index  ;       /* Index                               */
        unsigned short  count  ;       /* Counter                             */
}   TjUsrMI;

/*************************************/
/*   Master Index Of Dictionary File */
/*************************************/
typedef struct {
        unsigned char   *phsysmi     ; /* Pointer To Master Index Of System's */
        unsigned char   *phusrmi     ; /* Pointer To Master Index Of User's   */
        TjSysMI        tjsysmi[27*27]; /* Master Index Of System Dictionary   */
        TjUsrMI         tjusrmi[28]  ; /* Master Index Of User Dictionary     */
}   MasterIndex;

/*************************************/
/*          STJ  Candidate           */
/*************************************/
typedef struct {
        unsigned short  stroke ;       /* Stroke                              */
        unsigned char   key[5] ;       /* Index Key                           */
        unsigned char   euc[4] ;       /* EUC Code                            */
}   StjCand;

/*************************************/
/*          STJ  Candidate Struct    */
/*************************************/
typedef struct {
        StjCand *stjcand;              /* Pointer To Stj Candidates           */
        unsigned short allcandno;      /* Number Of All Candidates            */
        unsigned short headcandno;
        unsigned short curcandno;      /* Number Of Current Candidate         */
        unsigned short tailcandno;
}    StjStruct;

/*************************************/
/*          Stroke  Candidate        */
/*************************************/
typedef struct {
        unsigned short  stroke ;       /* Stroke Number                       */
        unsigned short  rrn    ;       /* Relative Record Number              */
}   StrokeCand;

/*************************************/
/*        Stroke Candidate Struct    */
/*************************************/
typedef struct {
        StrokeCand     *strokecand;   /* Pointer To Stroke Candidate         */
        unsigned short allcandno;     /* Number Of All Candidates            */
        unsigned short headcandno;
        unsigned short curcandno;     /* Number Of Current Candidate         */
        unsigned short tailcandno;
}   StrokeStruct;

/*************************************/
/*         Phonetic Candidate        */
/*************************************/
typedef struct {
        unsigned char   *phcand;       /* Pointer To Candidate                */
        unsigned char   *curptr;       /* Pointer To Current Candidates       */
        unsigned short  allcandno;     /* Number Of All Candidates            */
        unsigned short  more   ;       /* Number Of Remained Candidates       */
}   PhStruct;

/*************************************/
/*         Learning structure        */
/*************************************/
typedef struct {
        long            offset1,offset2;      /* V410 File offset             */
        long            offset3[PH_NO];       /* V410 File offset             */
        unsigned long   free;                 /* V410 Free area               */
        unsigned int    index[LEARN_NO];      /* V410 Learned elements        */
        unsigned char   *phlearndata;         /* V410 Phonetic master         */
        unsigned char   *learnadd;            /* V410 Address at *phlearndata */
        unsigned int    list_index[LIST_NO];  /* V410 Listed elements         */
        unsigned int    phfound;              /* V410 Change flag             */
        short           mode;                 /* Mode for A*B ... or phonetic */
}   LearnStruct;                              /* V410                         */

/*************************************/
/*        File  Descriptor           */
/*************************************/
typedef struct {
        FILE            *phsysfd ;     /* Phonetic System Dic. File Descriptor*/
        FILE            *phusrfd ;     /* Phonetic User Dic. File Descriptor  */
        FILE            *tjsysfd ;     /* TsangJye System Dic. File Descriptor*/
        FILE            *tjusrfd ;     /* TsangJye User Dic. File Descriptor  */
        FILE            *learnfd ;     /* V410 Learning File Descriptor       */
}   FileDescriptor;

/*************************************/
/*   Dictionary File Name            */
/*************************************/
typedef struct {
        char            *phsysfname;   /* Phonetic System Dictionary File Name*/
        char            *phusrfname;   /* Phonetic User Dictionary File Name  */
        char            *tjsysfname;   /* Tsang_Jye System Dict. File Name    */
        char            *tjusrfname;   /* Tsang_Jye User Dictionary File Name */
        char            *learnname;    /* V410 Learning File Name             */
}   FileName;

/*************************************/
/*  TIMED internal control block     */
/*************************************/
typedef struct {
        unsigned char   *echobufs  ;   /* Pointer To Echo Buffer for display  */
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
        unsigned char   *preinpbuf ;   /* Previous Input Buffer For Tsang-Jye */
        unsigned char   *curinpbuf ;   /* Current Input Buffer For Tsang-Jye  */
        unsigned char   *preedbuf  ;   /* Previous EUC Codes In Echo Buffer   */
        unsigned char   *ctrl_r_buf;   /* Previous Input Buffer For Phonetic  */
        unsigned char   *radeucbuf ;   /* Radicals Buffer For STJ             */
        int             inputlen   ;   /* Length Of Input Buffer              */
        CTime           ctime      ;   /* Time Of Last Access Dictionary File */
        MasterIndex     mi         ;   /* Master Index Of Dictionary File     */
        int             starpos    ;   /* Position Of Star Mark For STJ       */
        FileDescriptor  fd         ;   /* File Descriptor                     */
        int             ret        ;   /* Return Code                         */
        StjStruct       stjstruct  ; /* Simplify-Tsang-Jye Candidate Structure*/
        StrokeStruct   strokestruct;   /* Stroke Candidate Structure          */
        PhStruct        phstruct   ;   /* Phonetic Candidate Structure        */
        LearnStruct     learnstruct;   /* Learning structure                  */
        FileName        fname      ;   /* Dictionary File Name                */
        int             learning   ;   /* flag for learning on or off         */
        unsigned int    Lang_Type  ;   /* flag for support BIG5 code @big5    */
        iconv_t         iconv_flag ;    /* opening iconv flag                 */
        unsigned int    keylayout  ;   /* IBM or non-IBM layout @big5         */
	int		prev_mode  ;   /* Used by XK_ZH_Toggle	              */
}   FEPCB;

