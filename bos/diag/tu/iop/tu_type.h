/* @(#)87	1.1  src/bos/diag/tu/iop/tu_type.h, tu_iop, bos411, 9428A410j  12/15/93  13:46:56 */
/*
 * FUNCTIONS: Header File
 *
 * ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * tu_type.h
 *
 * Header file for interface to Micro Channel Test Units.
 */

#ifndef _H_TU_TYPE
#define _H_TU_TYPE

#include <diag/atu.h>

#define RULES_FILE_LEN          255

/*
 * Values for TUTYPE.header.tu
 */

typedef struct {
        long                   bad_others;
        long                   bad_reads;
        long                   bad_writes;
        long                   bytes_read;
        long                   bytes_written;
        long                   good_others;
        long                   good_reads;
        long                   good_writes;
} tu_stats_t;

/*
 * TUTYPE definition
 */

typedef struct iop_tucb {
        struct tucb_t           header;
        tu_stats_t tu_stats;
} TUTYPE, *TU_PTR_TYPE;





/***********************************************************************
   Error Codes.
***********************************************************************/

#define EXIT_GOOD                 0
#define EXIT_ARGS                 1
#define EXIT_HTXINIT              2
#define EXIT_OPENDEV              3
#define EXIT_OPENRULE             4
#define EXIT_READRULE             5
#define EXIT_ERRFILE		  6
#define EXIT_SIG                255

#define E_BASE                  0x0001

#define INT_HANDLER	50

#define START_TEST      101
#define SUSP_TEST       102
#define INTR_TEST       103
#define OSC_TEST        104
#define PERIODIC        105
#define TODNVRAM        106
#define SAVE_ENABLE     107
#define LEAP_YEAR       108
#define HOUR12          109
#define CLOCK_STOP      110
#define LOW_BATTERY     111
#define TOD_IRQ_STAT    112
#define PF_IRQ          113
#define ELAPSED         114
#define DEBUG_TOD	99
#define INTR_HANDLER    200 	/* Special exectu case added for */
				/* compatibility w/ DIAG app.    */

#define TOD_BUSY_ERR	0xfefe

#endif /* ndef H_TU_TYPE */
