/* @(#)38	1.8  src/bos/kernel/sys/pty.h, sysxpty, bos411, 9428A410j 6/16/90 00:34:41 */
#ifndef _H_PTY
#define _H_PTY

/*
 * COMPONENT_NAME: (sysxpty) PTY device driver
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 3, 9, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/ioctl.h>

/* ioctl defines */

#define PTYIOR          FIONREAD
#define PTYIOW          TIOCOUTQ

#endif /* _H_PTY */

