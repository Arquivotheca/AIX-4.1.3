/* @(#)30       1.9.1.5  src/bos/diag/tu/async/atu_tty.h, tu_async, bos411, 9428A410j 6/8/94 17:21:09 */
/*
 * COMPONENT_NAME: tu_async
 *
 * FUNCTIONS:   This file contains global defines, variables, and structures
 *              for the Async adapter test units.
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_ATU_TTY
#define _H_ATU_TTY

#define MAXTIME 60
#define TIME_FACTOR 6
#define OFF     0
#define SET     0x01
#define CLEAR   0x00
#define CONFIG  0
#define TEST    1
#define DIAG    1

/* FRU levels */
#define LOCAL   0x00
#define DRIVER  0x01
#define CABLE   0x02
#define FANOUT  0x03
#define INTERP  0x04

/* Operation types */
#define ENABLE  0x01
#define DISABLE 0x02

/* Return codes */
#define PASSED  0
#define FAILED  -1
#define DD_HW_F 1
#define UNK_VPD 2
#define ADP_VPD 3
#define REG_VER 4
#define CON_VPD 5
#define LINE_ERR 6
#define ADP_ERR 7
#define CON_ERR 8
#define DWC_TST 2
#define SYN_TST 4
#define MCL_TST 3
#define PCC_TST 4
#define MEM_TST 4

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/devinfo.h>
#include <sys/tty.h>
#include <sys/termio.h>
#include <sys/signal.h>
#include <sys/sysconfig.h>
#include <sys/types.h>
#include <sys/device.h>
#include <sys/li.h>
#include <sys/cxma.h>
#include <sys/stropts.h>
#include <ctype.h>
#include <diag/atu.h>
#include <diag/ttycb.h>

#endif  /* _H_ATU_TTY */
