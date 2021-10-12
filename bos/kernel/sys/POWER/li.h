/* @(#)65	1.3  src/bos/kernel/sys/POWER/li.h, sysxtty, bos411, 9428A410j 6/15/90 17:48:05 */

/*
 * COMPONENT_NAME: (sysxtty) System Extension for tty support
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* ioctl's for the 64 port adapter
*/

enum lion_ioctl {
    LI_GETVT = ('l'<<14),		/* get params for virtual terminal */
    LI_SETVT,				/* set params for virtual terminal */
    LI_GETXP,				/* get params for xparent printer */
    LI_SETXP,				/* set params for xparent printer */
    LI_SLP1,				/* turn on sync loop mode */
    LI_DSLP,				/* do sync loop test */
    LI_SLP0,				/* turn off sync loop mode */
    LI_PRES,				/* presence confidence check */
    LI_DRAM,				/* dram integrity check */
    LI_GETTBC,				/* get chunk count */
    LI_SETTBC,				/* set chunk count */
};

/* Structure for the LI_[GS]ETVT ioctls */
struct vter_parms {
    char goto1[5];			/* user sequence to go to screen 1 */
    char goto2[5];			/* user sequence to go to screen 2 */
    char screen1[11];			/* sequence to put term on screen 1 */
    char screen2[11];			/* sequence to put term on screen 2 */
};

/* Structure for the LI_[GS]ETXP ioctls */
struct xpar_parms {
    char in_xpar[11];			/* seq. to turn on transparent mode */
    char lv_xpar[11];			/* seq. to turn off transparent mode */
    char priority;			/* printer priority: 0(low)-63(high) */
};

/* Structure for the LI_DSLP ioctl */
struct slp_data {
    int result;				/* result code from test */
    int out_count;			/* number of chars to send (1-16) */
    char out_data[16];			/* the actual characters to send */
    int in_count;			/* return number of characters read */
    char in_data[16];			/* return the actual characters read */
};
