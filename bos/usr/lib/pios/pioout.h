/* @(#)36       1.4  src/bos/usr/lib/pios/pioout.h, cmdpios, bos411, 9428A410j 5/28/91 12:48:40 */
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS:
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

/*** pioout.h ***/

/* Default values used by pioout if not overridden from command line */
#define DEFAULT_FF_STR  "\014"  /* default form feed string */
#define DEFAULT_FF_LEN  1       /* length of default form feed string */
#define DEF_NUM_FF_STRS 0       /* number of form feed strings to send */
#define DEFAULT_CAN_STR "\000"  /* default string to send to printer at cancel*/
#define DEFAULT_CAN_LEN 1       /* length of default cancel string */
#define DEF_NUM_CAN_STRS 3168   /* number of cancel strings to send */

/* Return Codes (must not match signal numbers) */
#define PIOO_GOOD     0         /* successful */
#define PIOO_BAD      101       /* bad flag, bad load, bad file open, etc. */
#define PIOO_FAIL     102       /* printer failed */
#define PIOO_USR1OK   103       /* filter terminated; printer OK */
#define PIOO_USR1QUIT 104       /* filter terminated; printer not printing */
#define PIOO_TERMOK   105       /* job cancelled; printer OK */
#define PIOO_TERMQUIT 106       /* job cancelled; printer not printing */

