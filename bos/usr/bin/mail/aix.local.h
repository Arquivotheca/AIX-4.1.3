/* @(#)31	1.4  src/bos/usr/bin/mail/aix.local.h, cmdmailx, bos411, 9428A410j 9/21/93 13:24:07 */
/* 
 * COMPONENT_NAME: CMDMAILX aix.local.h
 * 
 * FUNCTIONS: dup2, gethostname, gtty, stty, vfork 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <sys/lockf.h>
#include <sys/utsname.h>

#define vfork() fork()

#define sgttyb termio
#define gtty(a,b) ioctl(a, TCGETA, b)
#define stty(a,b) ioctl(a, TCSETA, b)
#define sg_ospeed c_cflag & CBAUD
#define sg_erase c_cc[2]
#define sg_kill c_cc[3]
