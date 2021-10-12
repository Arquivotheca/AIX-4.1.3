/* @(#)70	1.5  src/bos/kernel/sys/POWER/rs.h, cmdtty, bos411, 9428A410j 2/21/94 12:47:15 */
#ifndef _H_RS
#define _H_RS

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

enum rs_ioctl {
    RS_GETA = 'r'<<14,
    RS_SETA };

struct rs_info {
    char rs_dma;			/* dma level -- 0 is off */
    char rs_rtrig;			/* receive trigger level */
    char rs_tbc;			/* transmit buffer count - up to 16 */
    char rs_mil;			/* mil inverter */
};

#define RS_TRIG_01 			/* trigger at 1 */
#define RS_TRIG_04 			/* trigger at 4 */
#define RS_TRIG_08 			/* trigger at 8 */
#define RS_TRIG_14			/* trigger at 14 */
#endif /* _H_RS */
