/* @(#)20	1.1  src/bos/usr/lib/nls/loc/CN.im/cnedud.h, ils-zh_CN, bos41B, 9504A 12/19/94 14:33:56  */
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
/* MODULE NAME:         CNEDUD.H                                              */
/*                                                                            */
/* DESCRIPTIVE NAME:    Chinese Input Method contant file                     */
/*                                                                            */
/* FUNCTION:                                                                  */
/*                                                                            */
/******************************************************************************/

#include <stdio.h>

#define IS_RADICAL_FLAG       1
#define BACK_SPACE_FLAG      2
#define CLEAR_RADICAL_FLAG   3
#define LISTBOX_ON_FLAG      4
#define LEFT_ARROW_FLAG      5
#define RIGHT_ARROW_FLAG     6
#define INSERT_FLAG          7
#define DELETE_FLAG          8
#define ERROR_FLAG           9
#define PGUP_FLAG            10
#define PGDN_FLAG            11
#define ISRESULT_NUM_FLAG    12
#define ISRESULT_ALT_FLAG    13
#define LISTBOX_OFF_FLAG     14
#define CONVERT_FLAG         15
#define NON_CONVERT_FLAG     16

#define UD_FOUND             1
#define UD_NOT_FOUND         0

#define TRUE                 1
#define FALSE                0

#define FOUND_CANDIDATE      1
#define NOT_FOUND_CANDIDATE  0

typedef struct{
       unsigned char   *radicbuf;      /* User Defined IM Radical buffer,    */
                                       /* The user defined routines put      */
                                       /* radical into the buffer.           */
       unsigned char   *candbuf;       /* User Defined IM candidate buffer   */
                                       /* The user defined routines put the  */
                                       /* candidates into the buffer. IM API */
                                       /* get the candidates from this       */
                                       /* buffer, and display them in the    */
                                       /* candidates list box.               */
       int             allcandno;      /* Number of the all candidates.      */
       int            (*UserDefinedInitial)(); /* User Defined Entry Point   */
       int             (*UserDefinedFilter)(); /* Process User Defined IM    */
                                               /* Input key                  */
       int             (*UserDefinedSelect)(); /* Process User Defined IM    */
                                               /* select key                 */
       int             (*UserDefinedConvert)();  
       int             (*UserDefinedClose)();
       char            *FileName;              /* Pointer to User Defined    */
                                               /* Dictionary File            */
       int             ret;                    /* User Defined return code   */
       int             inputmaxlen;
}  UdimCommon;

UdimCommon *udimcomm;

