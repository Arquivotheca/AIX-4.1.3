/* @(#)32	1.17  src/bos/kernel/sys/wait.h, sysproc, bos411, 9428A410j 12/7/93 18:52:31 */

/*
 * COMPONENT_NAME: SYSPROC 
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_WAIT
#define _H_WAIT

#ifndef _H_STANDARDS
#include <standards.h>
#endif

#ifndef _H_TYPES
#include <sys/types.h>
#endif

/*
 * This file holds definitions relevent to the waitpid(), wait(), and wait3()
 * system calls.  The rusage option is only available with the wait3() call.
 * The options field in wait3() and waitpid() determines the behavior of the
 * call, while the process ID field in the waitpid() call determines which
 * group of children to search.
 */

/*
 * POSIX requires that certain values be included in wait.h.  It also
 * requires that when _POSIX_SOURCE is defined only those standard
 * specific values are present.  This header includes all the POSIX
 * required entries.
 */
#ifdef _POSIX_SOURCE

#ifndef	_NONSTD_TYPES
#ifdef _NO_PROTO
extern pid_t wait();
extern pid_t waitpid();
#else
#ifdef _BSD
extern pid_t wait(union wait*);
#else
extern pid_t wait(int *);
#endif /* _BSD */
extern pid_t waitpid(pid_t, int *, int);
#endif /* _NO_PROTO */
#endif	/* _NONSTD_TYPES */

/*
 * The option field for wait3() and waitpid() is defined as follows:
 * WNOHANG causes the wait to not hang if there are no stopped or terminated
 * processes, rather returning an error indication in this case (pid==0).
 * WUNTRACED indicates that the caller should receive status about untraced
 * children which stop due to signals.  If children are stopped and a wait
 * without this option is done, it is as though they were still running...
 * nothing about them is returned.
 */
#define WNOHANG		0x1	/* dont hang in wait			 */
#define WUNTRACED	0x2	/* tell about stopped, untraced children */

/*
 * Stopped process status.  Returned only for traced children unless requested
 * with the WUNTRACED option bit.  Lower byte gives the reason, next byte is
 * the last signal received, i.e. p->p_cursig.
 */
#define	_W_STOPPED	0x00000040	/* bit set if stopped		*/
#define	_W_SLWTED	0x0000007c	/* value if stopped after load	*/
#define	_W_SEWTED	0x0000007d	/* value if stopped after exec	*/
#define	_W_SFWTED	0x0000007e	/* value if stopped after fork	*/
#define	_W_STRC		0x0000007f	/* value if stopped after trace	*/

/*
 * MACRO defines for application interfacing to waitpid(), wait(), and wait3()
 */
/* evaluates to a non-zero value if status returned for a stopped child	*/
#define	WIFSTOPPED(__x)	((__x) & _W_STOPPED )
/* evaluates to the number of the signal that caused the child to stop	*/
#define	WSTOPSIG(__x)	(int)(WIFSTOPPED(__x) ? (((__x) >> 8) & 0x7f) : -1)
/* evaluates to a non-zero value if status returned for normal termination */
#define	WIFEXITED(__x)	( !((__x) & 0xff) )
/* evaluates to the low-order 8 bits of the child exit status	*/
#define	WEXITSTATUS(__x)	(int)(WIFEXITED(__x) ? (((__x) >> 8) & 0xff) : -1)
/* evaluates to a non-zero value if status returned for abnormal termination */
#define	WIFSIGNALED(__x)	(  !WIFEXITED(__x) && !WIFSTOPPED(__x) )
/* evaluates to the number of the signal that caused the child to terminate */
#define	WTERMSIG(__x)	(int)(WIFSIGNALED(__x) ? ((__x) & 0x7f) : -1)

#endif /* _POSIX_SOURCE */

#ifdef _ALL_SOURCE
#include <sys/resource.h>  /* included for struct rusage */

#ifndef	_NONSTD_TYPES
#if !defined(_NO_PROTO) && !defined(_KERNEL)
extern pid_t wait3(int *, int, struct rusage *);
#else
extern pid_t wait3();
#endif  /* _NO_PROTO && _KERNEL */
#endif	/* _NONSTD_TYPES */

#define	W_STOPPED	_W_STOPPED
#define	W_SLWTED 	_W_SLWTED
#define	W_SEWTED 	_W_SEWTED
#define	W_SFWTED 	_W_SFWTED
#define	W_STRC    	_W_STRC

/* define for BSD compatibility					*/

#define	WSTOPPED	_W_STOPPED

#ifdef _BSD

#include <sys/m_wait.h>

#endif /* _BSD */

#endif /* _ALL_SOURCE */
#endif /* _H_WAIT */
