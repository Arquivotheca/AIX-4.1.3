/* @(#)15	1.8  src/bos/kernext/lft/inc/kks.h, lftdd, bos411, 9428A410j 10/25/93 13:43:49 */
/*
 *   COMPONENT_NAME: LFTDD
 *
 * FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
  The composition table for diacritics (dead) keys and the start/stop
        search points for this table are in diacritics.c
  ------------*/


#include <sys/types.h>

#define MAX_CODE_SEQ_LENGTH 32
#define CHARSET_SIZE        256
#define _8_BIT_MASK         0xff

/* key positions for PC ALT-NUM emulation */
#define ALT_NUM_0 99
#define ALT_NUM_1 93
#define ALT_NUM_2 98
#define ALT_NUM_3 103
#define ALT_NUM_4 92
#define ALT_NUM_5 97
#define ALT_NUM_6 102
#define ALT_NUM_7 91
#define ALT_NUM_8 96
#define ALT_NUM_9 101


/* indices into lft_swkbd_t struc based on shift keys */
#define KMAP_INDEX_NORMAL    0x00
#define KMAP_INDEX_SHIFT     0x01
#define KMAP_INDEX_CNTL      0x02
#define KMAP_INDEX_ALT       0x03          /*alt state*/
#define KMAP_INDEX_LEFT_ALT  0x03          /*alt state*/
#define KMAP_INDEX_RIGHT_ALT 0x04          /*alt graphics state*/
#define KMAP_INDEX_KATA      0x05          /*katakana base state*/
#define KMAP_INDEX_KSHIFT    0x06          /*katakana shift state*/

/* interrupt type defines for calling lftintr */
#define INTR_ID_KSR_CODES       0x00
#define INTR_ID_COMMAND         0x01
#define INTR_ID_PROCEDURE       0x02
#define INTR_ID_FUNCTION        0x03
#define INTR_ID_KEYBOARD        0x04
#define INTR_ID_ASCII           0x05
#define INTR_ID_LOCATOR         0x11
#define INTR_ID_SCRN_GRANT      0x14
#define INTR_ID_SCRN_RELEASE    0x15
#define INTR_ID_ABORT           0x16
#define INTR_ID_SOUND_ACK       0x17
#define INTR_ID_MOM_BUFF_EMPTY  0x18
#define INTR_ID_KSR_BUFF_EMPTY  0x19
#define INTR_ID_SAK             0x1A    /* AIX V2.2.1 security enhs */


#ifndef LFNUMDIACRITIC
#define LFNUMDIACRITIC 1
#endif

struct diacritic {
        char     code_set[8];           /* code set this table used with */
        int      num_diacs;             /* number of entries in table    */
        struct   diacritic  *next;      /* points to next table in list  */
        struct   diac {
                 char  deadkey;
                 char  thiskey;
                 char  compose;
        } diacs[LFNUMDIACRITIC];
};


