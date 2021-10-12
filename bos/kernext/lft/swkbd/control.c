static char sccsid[] = "@(#)98  1.6  src/bos/kernext/lft/swkbd/control.c, lftdd, bos411, 9428A410j 10/25/93 15:27:15";
/*
 *   COMPONENT_NAME: LFTDD
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <lft_swkbd.h>
#include <lftcode.h>

/*-------------------------------------------------------------------
  Single control code table
  -------------------------------------------------------------------*/
uchar single_control_table[16] = {
        CODE_STAT_NONE << 4 |               /* NUL     0x0100  */
        CODE_STAT_NONE,                     /* SOH     0x0101  */
        CODE_STAT_NONE << 4 |               /* STX     0x0102  */
        CODE_STAT_NONE,                     /* ETX     0x03  */
        CODE_STAT_CURSOR << 4 |             /* EOT     0x04  */
        CODE_STAT_NONE,                     /* ENQ     0x05  */
        CODE_STAT_NONE << 4 |               /* ACK     0x06  */
        CODE_STAT_NONE,                     /* BEL     0x07  */
        CODE_STAT_CURSOR << 4 |             /* BS      0x08  */
        CODE_STAT_CURSOR,                   /* HT      0x09  */
        CODE_STAT_CURSOR << 4 |             /* LF      0x0A  */
        CODE_STAT_CURSOR,                   /* VT      0x0B  */
        CODE_STAT_CURSOR << 4 |             /* FF      0x0C  */
        CODE_STAT_CURSOR,                   /* CR      0x0D  */
        CODE_STAT_NONE << 4 |               /* SO      0x0E  */
        CODE_STAT_NONE,                     /* SI      0x0F  */
        CODE_STAT_NONE << 4 |               /* DLE     0x10  */
        CODE_STAT_NONE,                     /* DC1     0x11  */
        CODE_STAT_NONE << 4 |               /* DC2     0x12  */
        CODE_STAT_NONE,                     /* DC3     0x13  */
        CODE_STAT_NONE << 4 |               /* DC4     0x14  */
        CODE_STAT_NONE,                     /* NAK     0x15  */
        CODE_STAT_NONE << 4 |               /* SYN     0x16  */
        CODE_STAT_NONE,                     /* ETB     0x17  */
        CODE_STAT_NONE << 4 |               /* CAN     0x18  */
        CODE_STAT_NONE,                     /* EM      0x19  */
        CODE_STAT_NONE << 4 |               /* SUB     0x1a  */
        CODE_STAT_NONE,                     /* ESC     0x1b  */
        CODE_STAT_NONE << 4 |               /* FS      0x1c  */
        CODE_STAT_NONE,                     /* GS      0x1d  */
        CODE_STAT_NONE << 4 |               /* RS      0x1e  */
        CODE_STAT_NONE                      /* US      0x1f  */
};
