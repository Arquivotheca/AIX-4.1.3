/* @(#)99	1.1  src/bos/usr/lbin/tty/mon-cxma/vpd.h, sysxtty, bos411, 9428A410j 6/23/94 15:28:36 */
/*
 * COMPONENT_NAME: (sysxtty) System Extension for tty support
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 */

#ifndef _VPD_H_
#define _VPD_H_

extern void print_vpd(ADAPTER_INFO *, int, int);
extern void get_concvpd(ADAPTER_INFO *, int, int);
extern void get_hostvpd(ADAPTER_INFO *);

#endif /* _VPD_H_ */
