static char sccsid[] = "@(#)22	1.22.2.5  src/bos/usr/ccs/lib/libc/system.c, libcio, bos411, 9428A410j 6/3/94 15:20:45";
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: system 
 *
 * ORIGINS: 3, 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*LINTLIBRARY*/
#include	<signal.h>
#include	<stdio.h>
#include	<errno.h>
#include	<fcntl.h>	/* for O_RDWR   */
#include	<unistd.h>	/* for X_OK access */
#include	<sys/termio.h>	/* for TCQTRUST */
#include	<sys/types.h>
#include	<sys/priv.h>	/* for TRUSTED PATH */
#include	<sys/wait.h>


int 	
system(const char *string)
{
	int	status;
	pid_t	pid;
	struct sigaction ointact,oquitact,ignact,ochldact;
	sigset_t savemask;

	/**********
	  if the command processor is available, return 1, otherwise
	  return 0
	**********/
	if(string == NULL) {
		if (privcheck(TRUSTED_PATH) == 0)
			status = access ("/usr/bin/tsh", X_OK);
		else
			status = access ("/usr/bin/sh", X_OK);
		if (status) /* command processor not availabel */
			return (0);
		else
			return(1); 
	}

	ignact.sa_handler = SIG_IGN;
	sigemptyset(&(ignact.sa_mask));
	ignact.sa_flags = 0 ;
	sigaction(SIGINT,&ignact,&ointact);
	sigaction(SIGQUIT,&ignact,&oquitact);

	sigaddset(&ignact.sa_mask,SIGCHLD);
	sigprocmask(SIG_BLOCK,&ignact.sa_mask,&savemask);

	ignact.sa_handler = SIG_DFL;
	sigemptyset(&(ignact.sa_mask));
	ignact.sa_flags = 0;
	sigaction(SIGCHLD,&ignact,&ochldact);

	if((pid = __fork()) == 0) 
	{
		sigaction(SIGINT,&ointact,NULL);
		sigaction(SIGQUIT,&oquitact,NULL);
		sigaction(SIGCHLD,&ochldact,NULL);
		sigprocmask(SIG_SETMASK,&savemask,NULL);

		/* 
		 * if the terminal is in a trusted state then 
		 * maintain trustedness by running the trusted shell 
		 */
		if (privcheck(TRUSTED_PATH) == 0)
		{
			(void) execl("/usr/bin/tsh", "tsh", "-c", string, NULL);
			_exit(127);
		}
		(void) execl("/usr/bin/sh", "sh", "-c", string, NULL);
		_exit(127);
	}

	if(pid == -1)	/* fork failed */
		status = -1;
	else
		while (waitpid(pid,&status,0) == -1)
			if (errno != EINTR) {
				status = -1;
				break;
			}

	/* Restore original signal states */
	sigaction(SIGINT,&ointact,NULL);
	sigaction(SIGQUIT,&oquitact,NULL);
	sigaction(SIGCHLD,&ochldact,NULL);
	sigprocmask(SIG_SETMASK,&savemask,NULL);

	return(status);
}

int
_system(const char *string)
{
return(system(string));
}
