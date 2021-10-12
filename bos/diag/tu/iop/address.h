/* @(#)60	1.2  src/bos/diag/tu/iop/address.h, tu_iop, bos411, 9428A410j 4/14/94 10:35:18 */
/*
 *   COMPONENT_NAME: TU_IOP
 *
 *   FUNCTIONS: Header File  (address.h)
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1, 5 Years Bull Confidential Information
 *
 */

/* address header file for the Time of Day functions                */

#ifndef _H_ADDRESS
#define _H_ADDRESS

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/devinfo.h>
#include <curses.h>
#include <sys/mdio.h>
#include <sys/mode.h>
#include "tu_type.h"

/* #define print    */
/* screen print conditional directive */

           /* Base address of Time of Day chip */
#define MAIN                    0x04000C0       /* time of day base address */
#define SYSTAT                  0x04000E0       /* system reset status bit */
#define PFREG1                  0x04000E8       /* WO reg = power shutdown */
#define PFREG2                  0x04000E4
#define POW_STAT_KEY            PFREG2    /* RO reg = power status & key pos */
#define LED                     0x0A00300       /* led first byte addr */
#define ECLEVEL                 0x04000FC       /* I/O board EC level reg */
#define POWERSTAT               0x04000E4       /* 1st byte of PWR STAT REG */

#define TCR0                    MAIN + 1        /* c1 */
#define TCR1                    MAIN + 2        /* c2 */
#define PERFLAG                 MAIN + 3        /* c3 */
#define IRQROUTE                MAIN + 4        /* c4 */
#define REALTIME                MAIN + 1        /* c1 */
#define OUTPUTMODE              MAIN + 2        /* c2 */
#define IRQ0                    MAIN + 3        /* c3 */
#define IRQ1                    MAIN + 4        /* c4 */
#define CSECCOUNT               MAIN + 5        /* c5 */
#define SECCOUNT                MAIN + 6        /* c6 */
#define MINCOUNT                MAIN + 7        /* c7 */
#define HURCOUNT                MAIN + 8        /* c8 */
#define DAYCOUNT                MAIN + 9        /* c9 */
#define MONCOUNT                MAIN + 10       /* cA */
#define YERCOUNT                MAIN + 11       /* cB */
#define UJULCOUNT               MAIN + 12       /* cC */
#define CJULCOUNT               MAIN + 13       /* cD */
#define DYWKCOUNT               MAIN + 14       /* cE */
#define TIMER0L                 MAIN + 15       /* cF */
#define TIMER0M                 MAIN + 16       /* d0 */
#define TIMER1L                 MAIN + 17       /* d1 */
#define TIMER1M                 MAIN + 18       /* d2 */
#define SECCOMP                 MAIN + 19       /* d3 */
#define MINCOMP                 MAIN + 20       /* d4 */
#define HURCOMP                 MAIN + 21       /* d5 */
#define DAYCOMP                 MAIN + 22       /* d6 */
#define MONCOMP                 MAIN + 23       /* d7 */
#define DYWKCOMP                MAIN + 24       /* d8 */
#define SECSAVE                 MAIN + 25       /* d9 */
#define MINSAVE                 MAIN + 26       /* dA */
#define HURSAVE                 MAIN + 27       /* dB */
#define DAYSAVE                 MAIN + 28       /* dC */
#define MONSAVE                 MAIN + 29       /* dD */
#define RAMREG                  MAIN + 30       /* dE */
#define RAMTEAT                 MAIN + 31       /* dF */

#define PG0REG0                 0x00            /* PAGE 0 REG 0 */
#define PG0REG1                 0x40            /* PAGE 0 REG 1 */
#define PG1REG0                 0x80            /* PAGE 1 REG 0 */

#define NUM_TOD_REGS            67
#define NUM_TOD_FLAGS           38

#define F_MAIN                    0
#define F_TCR0                    1
#define F_TCR1                    2
#define F_PERFLAG                 3
#define F_IRQROUTE                4
#define F_REALTIME                5
#define F_OUTPUTMODE              6
#define F_IRQ0                    7
#define F_IRQ1                    8
#define F_CSECCOUNT               9
#define F_SECCOUNT                10
#define F_MINCOUNT                11
#define F_HURCOUNT                12
#define F_DAYCOUNT                13
#define F_MONCOUNT                14
#define F_YERCOUNT                15
#define F_UJULCOUNT               16
#define F_CJULCOUNT               17
#define F_DYWKCOUNT               18
#define F_TIMER0L                 19
#define F_TIMER0M                 20
#define F_TIMER1L                 21
#define F_TIMER1M                 22
#define F_SECCOMP                 23
#define F_MINCOMP                 24
#define F_HURCOMP                 25
#define F_DAYCOMP                 26
#define F_MONCOMP                 27
#define F_DYWKCOMP                28
#define F_SECSAVE                 29
#define F_MINSAVE                 30
#define F_HURSAVE                 31
#define F_DAYSAVE                 32
#define F_MONSAVE                 33
#define F_RAMREG                  34
#define F_RAMTEAT                 35
#define F_PG1RAM                  36
#define F_SAVE_COMPLETE           37

#define ONE                       1
#define TWO                       2
#define THREE                     3
#define FOUR                      4
#define FIVE                      5
#define SIX                       6
#define SEVEN                     7
#define EIGHT                     8
#define NINE                      9


#define LCD_STR_NEW               0xff6200c0       /*string to be displayed */
#define LCD_STR_OLD               (LCD_STR_NEW+32) /*string actually displayed*/


#endif /* ifndef _H_ADRESS */
