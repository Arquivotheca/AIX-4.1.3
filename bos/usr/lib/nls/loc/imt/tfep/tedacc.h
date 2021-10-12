/* @(#)05  1.4  src/bos/usr/lib/nls/loc/imt/tfep/tedacc.h, libtw, bos411, 9428A410j 6/14/94 20:37:40 */
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

#define NOT_FOUND          0
#define FOUND_WORD         1
#define FOUND_CAND         2
#define NO_WORD            0
#define USR                1
#define SYS                2
#define TJ0                0
#define STJ1               1
#define STJ2               2
#define STJ3               3
#define TJ_SYS_MI_LEN      27*27*6
#define TJ_USR_MI_LEN      512
#define PH_SYS_MI_LEN      512*5
#define PH_USR_MI_LEN      512*5
#define PH_DR_MAX_LEN      512*4
#define TJ_SYS_TOT_MI      27*27
#define TJ_USR_TOT_MI      28    /*  27+26  */
#define TJ_USR_BLOCK_SIZE  512
#define PH_MI_END_MARK     0xFFFF
#define PH_DR_END_MARK     0xFFFF
#define PH_GP_END_MARK     0xFF
#define ZZ_INDEX           26*27-1
#define BASE               96
#define USR_FONT_EUC_BYTE1 0x8E
#define USR_FONT_EUC_BYTE2 0xAC
#define STROKE_TOT_NO      256
#define MAX_STROKE         255
#define EUC_BYTE1          0x8E
#define PHONETIC_LEN       30
#define RRN_LENGTH         4
#define MAX_PHONETIC_NUM   14
/* #define PLANE3_EUC_BYTE2   0xA3                     @big5  @V42 */
/* #define PLANE4_EUC_BYTE2   0xA4                     @big5  @V42 */
#define PLANE2_EUC_BYTE2   0xA2                            /* @V42 */
#define PLANEc_EUC_BYTE2   0xAC                            /* @V42 */
#define PLANEd_EUC_BYTE2   0xAD                            /* @V42 */
