static char sccsid[] = "@(#)19  1.5  src/bos/usr/ccs/lib/libc/fork.c, libcgen, bos411, 9428A410j 6/21/94 12:13:30";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: fork
 *
 * ORIGINS: 27,85
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * This is a front end to fork in kernel which is being renamed 
 * to kfork. This has been done to share a variable between
 * fork and the stdio library package for providing a different 
 * stdio behaviour in case an application is not following posix
 * streamio rules.
 */

/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */

#include <sys/types.h>
#include <errno.h>
#include<stdlib.h>
int __forkid;

/*
 * The prefork routine is called prior to invoking the fork
 * syscall. The postfork routine is called (in the original 
 * process) regardless of whether the fork succeeded or failed.
 * The child routine is called in the child, if the fork succeeded.
 * No arguments are provided, no return value used.
 */

#ifdef _THREAD_SAFE
/**********
  These are only needed in the thread safe case
**********/
void (*_atfork_prefork_routine)();
void (*_atfork_child_routine)();
void (*_atfork_postfork_routine)();
#endif /* _THREAD_SAFE */

pid_t
fork(void)
{
	__forkid++;
	return __fork();
}

pid_t
__fork(void)
{

#ifndef _THREAD_SAFE
	return(kfork());
#else /* _THREAD_SAFE */
	pid_t	pid;	/* return value from kfork */

	/**********
	  if the prefork routine is defined, call it
	**********/
	if (_atfork_prefork_routine) {
		(*_atfork_prefork_routine)();
	}

	/**********
	  if we are in the parent, or the kfork failed,
	  call postfork routine if defined
	**********/
	if (pid = kfork()) {	/* Caller (pid != 0) */
		if (_atfork_postfork_routine)
			(*_atfork_postfork_routine)();

	/**********
	  if we are in the child, call the child routine
	  if defined
	**********/
	} else {			/* Child (pid == 0) */
		_libc_locks_init();
		if (_atfork_child_routine)
			(*_atfork_child_routine)();
	}
	return (pid);
#endif /* _THREAD_SAFE */
}
