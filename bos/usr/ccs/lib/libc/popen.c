static char sccsid[] = "@(#)56	1.17.3.9  src/bos/usr/ccs/lib/libc/popen.c, libcio, bos411, 9428A410j 6/3/94 15:21:55";
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: popen, pclose 
 *
 * ORIGINS: 3,27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>	/* for fcntl( F_DUPFD ) */
#include <signal.h>
#include <unistd.h>
#include <sys/priv.h>	/* for TRUSTED PATH */
#include <errno.h>

#include <ts_supp.h>	/* for TS_LOCK and TS_UNLOCK */
#include "push_pop.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex _popen_rmutex;
#endif /* _THREAD_SAFE */

#define	tst(a,b) (*mode == 'r'? (b) : (a))
#define	RDR	0
#define	WTR	1

/**********
  this is ok in a thread safe environment because popen is the only
  one that modifies it
**********/
static int popen_pid[_NFILE];  /* _NFILE is # of open files */

/*                                                                    
 * FUNCTION: Initiates a pipe to/from a process.
 *                                                                    
 * RETURN VALUE DESCRIPTION: FILE * to opened stream, NULL on error. 
 */  

FILE *
popen(const char *cmd, const char *mode)
{
	int	p[2];
	register int myside, yourside;
	pid_t pid;

	if(pipe(p) < 0)
		return(NULL);
	myside = tst(p[WTR], p[RDR]);
	yourside = tst(p[RDR], p[WTR]);
	if((pid = __fork()) == 0) {
		/* myside and yourside reverse roles in child */
		int	stdio;
		int	i;			/** P44528 **/

		stdio = tst(0, 1);
		(void) close(myside);
		if ( stdio != yourside )
		{
			(void) close(stdio);
			(void) fcntl(yourside, F_DUPFD, stdio);
			(void) close(yourside);
		}
		/** Close previous popened files, if any ** P44528 **/

		for(i=0; i < _NFILE; i++)
		    if(popen_pid[i] != 0 && i != stdio)
			(void) close(i);

		/*
		 * if the terminal is in a trusted state then
		 * maintain trustedness by running the trusted shell
		 */
		if (privcheck(TRUSTED_PATH) == 0)
		{
			/*********
			  if the command language interpreter is not available
			  exit with a 127 (XPG4)
			**********/
			if (access("/usr/bin/tsh", X_OK))
				_exit(127);
			(void) execl("/usr/bin/tsh", "tsh", "-c", cmd, (char *)0);
			_exit(1);
		}
		/*********
		  if the command language interpreter is not available
		  exit with a 127 (XPG4)
		**********/
		if (access("/usr/bin/sh", X_OK))
			_exit(127);
		(void) execl("/usr/bin/sh", "sh", "-c", cmd, (char *)0);
		_exit(1);
	}
	if(pid == -1)
		return(NULL);

	TS_LOCK(&_popen_rmutex);
	popen_pid[myside] = pid;
	TS_UNLOCK(&_popen_rmutex);

	(void) close(yourside);
	return(fdopen(myside, mode));
}

int
pclose(FILE *ptr)
{
	register int f, r = 0 ;
	int status;
	struct sigaction ointact,oquitact,ohupact,ignact;

	ignact.sa_handler = SIG_IGN;
	sigemptyset(&(ignact.sa_mask));
	ignact.sa_flags = 0 ;


	f = fileno(ptr);
	(void) fclose(ptr);
	sigaction(SIGHUP,&ignact,&ohupact);
	sigaction(SIGINT,&ignact,&ointact);
	sigaction(SIGQUIT,&ignact,&oquitact);
	
	TS_LOCK(&_popen_rmutex);
	if (popen_pid[f] == 0)					/* p44528 */
		status = -1;					/* p44528 */
	else
	{
		TS_PUSH_CLNUP(&_popen_rmutex);
		while ( (r = waitpid(popen_pid[f],&status,(void *)0)) == -1 )
			if ( errno != EINTR )
				break ;
		TS_POP_CLNUP(0);
	}

	popen_pid[f] = 0;
	TS_UNLOCK(&_popen_rmutex);

	if(r == -1)
		status = -1;
	sigaction(SIGHUP,&ohupact,NULL);
	sigaction(SIGINT,&ointact,NULL);
	sigaction(SIGQUIT,&oquitact,NULL);

	return(status);
}
