/* @(#)96	1.1  src/bos/usr/lbin/tty/mon-cxma/term.h, sysxtty, bos411, 9428A410j 6/23/94 15:28:12 */
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

#ifndef _TERM_H_
#define _TERM_H_

extern void change_term(int, int);
extern void restore_term(void);

#endif /* _TERM_H_ */
