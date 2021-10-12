/* @(#)27	1.1  src/bos/usr/bin/bprt/prt-cfg.h, libbidi, bos411, 9428A410j 8/27/93 09:57:13 */
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************HEADER FILE HEADER*******************************/
/*                                                                            */
/*   HEADER FILE NAME: PRT-CFG.H  <Configuration file loader>                 */
/*                                                                            */
/*   FUNCTION: This unit (collection of routines) has the function of loading */
/*             printer specific information into the following global vars.   */
/*                                                                            */
/******************************************************************************/

#include "equate.h"

#define MAX_FONTS	15

typedef struct  _ESC_Elements {
        char            subcode;                   /*ESC_SUBCODE*/
        unsigned char   attr;                      /*ATTRIBUTE*/
        unsigned short  func;                      /*FUNCTION TO BE PERFORMED*/
        } ESC_Elements;

typedef struct  _NORMAL_Elements {
        unsigned char   attr;
        unsigned short  func;
        } NORMAL_Elements;

typedef struct  _WIDTH_Elements {
        unsigned char   pitch;                     /*PITCH BYTE*/
        unsigned char   attr;                      /*ATTRIBUTE*/
        } WIDTH_Elements;

typedef struct  _CODE_ON_TABLE_Elements {
        unsigned char   pitch;                   /*PITCH*/
        unsigned char   length;                  /*LENGTH OF SEQUENCE*/
        unsigned char   sequence[7];             /*ESC_VALUE(MAX 6)*/
        } CODE_ON_TABLE_Elements;

typedef struct  _CODE_OFF_TABLE_Elements {
        unsigned char   width;                   /*WIDTH*/
        unsigned char   length;                  /*LENGTH OF SEQUENCE*/
        unsigned char   sequence[7];             /*ESC_VALUE(MAX 6)*/
         } CODE_OFF_TABLE_Elements;

typedef struct _FONT_Element	{
	unsigned short	ID;
	unsigned short	pitch;
	unsigned char attr;
	} FONT_Element;

typedef struct __CFG {
          char            PrinterName[14]   ;
          unsigned short  PRT_CHAR2_ATTR1   ;
          unsigned short  PRT_LAST          ;
          unsigned short  PRT_DFLT_LENGTH   ;
          unsigned short  PRT_L_MARGIN      ;
          unsigned short  PRT_R_MARGIN      ;
          unsigned char   PRT_D_PITCH       ;
          unsigned short  PRT_OLD_CWIDTH    ;
          unsigned char   PRT_SPACE_FOR[11] ;
          unsigned short  PRT_SPEC_PITCH    ;
          unsigned short  RESERVED[2]       ;
          unsigned char   EXPANSION_CODE    ;
          unsigned char   TAB_FLAG          ;
          unsigned char   PRT_HOME_HEAD[8]  ;
          unsigned short  PRT_MIN_PNTER     ;
          unsigned short  PRT_MIN_CWIDTH    ;
          unsigned short  PRT_MIN_PITCH     ;
          unsigned short  PRT_HT_ATTR_1     ;
          unsigned short  PRT_HT_ATTR_2     ;
          unsigned short  PRT_END_ATTR_1    ;
          unsigned short  PRT_END_ATTR_2    ;
          unsigned char   TAIL_FLAG         ;
          unsigned short PRT_ATTR1          ;
          unsigned short PRT_ATTR2          ;
          unsigned short PRT_PSM_ATTR_1     ;
          unsigned short PRT_PSM_ATTR_2     ;
          unsigned char  DESELECT_n         ;
          unsigned char  PRT_420X           ;
          unsigned char   PRT_DEF_TABS[28]  ;
          NORMAL_Elements         NORMAL_CHARS_TAB[33];
          ESC_Elements            ESC_CHARS_TAB[45];
          WIDTH_Elements          WIDTH_TABLE[16];
          unsigned long           SPECIAL_TABLE[16];
          CODE_ON_TABLE_Elements  CODE_ON_TAB[32];
          CODE_OFF_TABLE_Elements CODE_OFF_TAB[32];
          unsigned short          PSM_TABLE_ORG_1046[256];
          unsigned short          PSM_TABLE_1046[256];
} _CFG,  *_PCFG;

extern _PCFG         CFG;
extern unsigned int  LoadCFG   ( int CFGSpec );
