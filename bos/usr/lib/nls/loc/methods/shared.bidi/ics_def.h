/* @(#)45	1.1  src/bos/usr/lib/nls/loc/methods/shared.bidi/ics_def.h, cfgnls, bos411, 9428A410j 8/30/93 15:02:57 */
/*
 *   COMPONENT_NAME: LIBMETH
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
#define MAXINC 10
#define NUM_GRPS 11

/*  Constants for group names in ICS */

#define INIT    1
#define ZDEF    2
#define RDEF    3
#define LDEF    4
#define DIGIT   5
#define XPRE    6
#define PUNCT   7
#define NEUTR   8
#define SPACE   9
#define QRSP   10

/*  Constants for state names in ICS */

#define L0_INIT           0
#define L0_TEXT           1
#define L0_RSP            2
#define L1_TEXT           3
#define L1_TEXT_SA        4
#define L1_RSP            5
#define L1_RSP_SA         6 
#define NUMERIC           7
#define NUM_SUF           8
#define NUM_RSP           9
#define NUM_SA           10
#define NUM_SUF_SA       11
#define NUM_RSP_SA       12
#define NEUTR_SA         13
#define NEUTR_RSP_SA     14
#define L1_CONT          15
#define L1_SPACE         16
#define L1_SPC_SA        17
#define L1_SPC_CONT_SA   18
#define PREFIX           19
#define PREFIX_SA        20
#define NUM_CONT         21
#define NUM_CONT_SA      22
#define L1_SPC_PRE_SA    23

/* Constants for special cases */
#define  NOT_SET              -1
#define  CONT_SPACE           1
#define  CONT_PREFIX          2

        /* Constants for special attributes for swapping characters */
          /* Is true when (att & FONT_MASK) >> FONT_SHIFT is true */
#define FONT_MASK       0x00e0
#define FONT_SHIFT           5

