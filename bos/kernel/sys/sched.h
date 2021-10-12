/* @(#)58	1.4  src/bos/kernel/sys/sched.h, sysproc, bos411, 9433B411a 8/11/94 08:46:56 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _H_SYS_SCHED
#define _H_SYS_SCHED

#include <sys/pri.h>   
#include <sys/m_param.h>   


#define PRIORITY_MIN PSWP        /* minumum priority */     
#define PRIORITY_MAX PIDLE       /* maximum priority */

#define SCHED_OTHER  0           /* default AIX scheduling policy */
#define SCHED_FIFO   1           /* first in-first out scheduling policy */
#define SCHED_RR     2           /* round robin scheduling policy */
#define SCHED_LOCAL  3           /* local thread scope scheduling policy */
#define SCHED_GLOBAL 4           /* global thread scope scheduling policy */

#define RR_INTERVAL HZ           /* time interval for round robin (SCHED_RR)
				    scheduling policy */

#endif /* _H_SYS_SCHED */
