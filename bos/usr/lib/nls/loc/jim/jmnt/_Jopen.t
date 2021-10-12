/* @(#)37	1.3  src/bos/usr/lib/nls/loc/jim/jmnt/_Jopen.t, libKJI, bos411, 9428A410j 6/6/91 13:50:53 */
/*
 * COMPONENT_NAME :	(LIBKJI) Japanese Input Method (JIM)
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *      _Jopen.t Profile Keyword Table.
 */

/*
 *      Profile Keyword List.
 */
#define KWD_NONE  (        -1)          /* Invalid Keyword Table Index. */
static  char *keyword[]={
#define KW_INS    (    0     )          /* Keyword Table No.0           */
        "insert"  ,
#define KW_KJIN   (KW_INS  +1)          /* Keyword Table No.1           */
        "kjin"    ,
#define KW_RESET  (KW_KJIN +1)          /* Keyword Table No.2           */
        "reset"   ,
#define KW_CONV   (KW_RESET+1)          /* Keyword Table No.3           */
        "conversion",
#define KW_KJNO   (KW_CONV +1)          /* Keyword Table No.4           */
        "kjno"    ,
#define KW_AUX1   (KW_KJNO +1)          /* Keyword Table No.5           */
        "aux1"    ,
#define KW_AUX2   (KW_AUX1 +1)          /* Keyword Table No.6           */
        "aux2"    ,
#define KW_AUX3   (KW_AUX2 +1)          /* Keyword Table No.7           */
        "aux3"    ,
#define KW_AUX4   (KW_AUX3 +1)          /* Keyword Table No.8           */
        "aux4"    ,
#define KW_1MAXC  (KW_AUX4 +1)          /* Keyword Table No.9           */
        "aux1maxc",
#define KW_2MAXC  (KW_1MAXC+1)          /* Keyword Table No.10          */
        "aux2maxc",
#define KW_3MAXC  (KW_2MAXC+1)          /* Keyword Table No.11          */
        "aux3maxc",
#define KW_4MAXC  (KW_3MAXC+1)          /* Keyword Table No.12          */
        "aux4maxc",
#define KW_1MAXR  (KW_4MAXC+1)          /* Keyword Table No.13          */
        "aux1maxr",
#define KW_2MAXR  (KW_1MAXR+1)          /* Keyword Table No.14          */
        "aux2maxr",
#define KW_3MAXR  (KW_2MAXR+1)          /* Keyword Table No.15          */
        "aux3maxr",
#define KW_4MAXR  (KW_3MAXR+1)          /* Keyword Table No.16          */
        "aux4maxr",
#define KW_BEEP   (KW_4MAXR+1)          /* Keyword Table No.17          */
        "beep"    ,
#define KW_REGIS  (KW_BEEP +1)          /* Keyword Table No.18          */
        "registration",
#define KW_MIX    (KW_REGIS+1)          /* Keyword Table No.19          */
        "mix"     ,
#define KW_INDIC  (KW_MIX  +1)          /* Keyword Table No.20          */
        "indicator",
#define KW_RKC    (KW_INDIC+1)          /* Keyword Table No.21          */
        "rkc"     ,
#define KW_KANA   (KW_RKC  +1)          /* Keyword Table No.22          */
        "kana"    ,
#define KW_KBLK   (KW_KANA +1)          /* Keyword Table No.23          */
        "kblock"  ,
#define KW_CONOT  (KW_KBLK +1)          /* Keyword Table No.24          */
        "cursorout",
#define KW_KATAK  (KW_CONOT+1)          /* Keyword Table No.25          */
        "katakana",
#define KW_ALPH   (KW_KATAK+1)          /* Keyword Table No.26          */
        "alphanum",
#define KW_LEARN  (KW_ALPH +1)          /* Keyword Table No.27          */
        "learning",
#define KW_ALLCA  (KW_LEARN+1)          /* Keyword Table No.28          */
        "allcand",
#define KW_KJNUM  (KW_ALLCA+1)          /* Keyword Table No.29          */
        "kanjinum",
#define KW_PFKRST (KW_KJNUM+1)          /* Keyword Table No.30          */
        "pfkreset",
#define KW_KUTEN  (KW_PFKRST+1)         /* Keyword Table No.31          */
        "kuten",
        NULL
};

