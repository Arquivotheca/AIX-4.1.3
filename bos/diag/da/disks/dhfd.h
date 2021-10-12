/* @(#)47       1.4  src/bos/diag/da/disks/dhfd.h, dadisks, bos411, 9428A410j 12/10/92 15:52:00 */
/*
 *   COMPONENT_NAME: DADISKS
 *
 *   FUNCTIONS: Header
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1992
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _H_DHFD
#define _H_DHFD

#define CMD_TIMEOUT 30   /* Timeout(secs) to wait for cmd completion   */

/* TU number to execute */
#define BADATU_0  0
#define BADATU_1  1
#define BADATU_2  2
#define BADATU_3  3
#define BADATU_4  4
#define BADATU_5  5

/* TU error codes that could be returned */
#define BAD_DIAGS_GOOD  0
#define BAD_DIAGS_FAIL  1
#define BAD_ABORT_SENT  2
#define BAD_BUFF_FAIL   80
#define BAD_AIX_ERR     -1
#define BAD_INVALID_TU  255

/* Reason codes for inclusion in FruBucket */
#define BADATU_0_FAIL     0x100
#define BADATU_0_ABRT     0x101
#define BADATU_1_FAIL     0x102
#define BADATU_1_ABRT     0x103
#define BADATU_2_FAIL     0x104
#define BADATU_2_ABRT     0x105
#define BADATU_3_FAIL     0x106
#define BADATU_3_ABRT     0x107
#define BADATU_4_FAIL     0x108
#define BADATU_4_ABRT     0x109
#define BADATU_5_FAIL     0x110
#define BUF_FAILED        0x112
#define CMD_TIMEDOUT      0x114
#define ERRNO_FAILURE     0x116
#define DDCONFIG_ERR      0x118
#define ELA_100           0x120
#define ELA_NOT_100       0x122
#define IPLCB_ERR1        0x124
#define IPLCB_ERR2        0x126
#define IPLCB_ERR3        0x128

/* confidence levels to be put in FruBucket */
#define conf100         100
#define conf90          90
#define conf10          10
#define bad_da_error    -1
#define PASS_PARAM DKIOCMD                /* Pass-thru param needed by DD */

int		led_value;		  /* LED value */
int             fdes;                       /* file descriptor for device */
nl_catd catd;                              /* pointer to the catalog file */
int		menu_return;		   /* user response */
/* declaration for error log analysis */

#define num_errs 6
#define srchstr  "%s -N %s"
#define marg_hrd_errs   2    /* num of hre's that will cause menu goal */
#define min_data_rd	125  /* min data to be read */
#define se_min_data_rd  10000
#define max_se_errs	100

#endif /* _H_DHFD */
