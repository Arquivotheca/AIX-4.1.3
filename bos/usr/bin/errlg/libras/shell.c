static char sccsid[] = "@(#)97	1.1  src/bos/usr/bin/errlg/libras/shell.c, cmderrlg, bos411, 9428A410j 3/2/93 09:02:55";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: shell
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME:     shell
 * FUNCTION: Shell escape routine
 * INPUTS:   cmd  command line string to exec via sh -c
 * RETURNS:  return value from wait
 *           -1   cannot fork
 *
 * If 'cmd' is 0 or "", the shell is exec-ed directly with no -c option.
 */

/*
 * Execute the command string in 'cmd' as input to a shell.
 * For example, if 'cmd' = "echo hello world",
 *  what will be exec-ed here after the fork is
 * sh -c echo hello world
 * The shell will take care of parsing the command line.
 * If there is no command string in 'cmd', just exec the shell.
 * Signals are set to SIG_DFL in the child process.
 */

#include <signal.h>
#include <errno.h>
#include <libras.h>

extern char *getenv();

shell(cmd)
char *cmd;
{
	int i;
	int pid;
	int rv;
	int status;
	sigtype sigintsv;
	sigtype sigquitsv;

	Debug("shell(%s)\n",cmd ? cmd : "<shell_escape>");
	if((pid = fork()) < 0) {		/* fork */
		perror("fork");				/* this error is very rare */
		return(-1);
	}
	if(pid) {						/* parent */
		sigintsv  = (sigtype) signal(SIGINT,SIG_IGN);
		sigquitsv = (sigtype) signal(SIGQUIT,SIG_IGN);
		while((rv = wait(&status)) != pid) {
			if(rv == 0) {
				Debug("PID=0\n");
				break;
			}
			if(errno == EINTR)
				continue;
			Debug("wait: rv=%d|%x status=%x errno=%d pid=%x '%s'\n",
				rv,rv,status,errno,pid,errstr());
		}
		signal(SIGINT,(void(*)(int)) sigintsv);
		signal(SIGQUIT,(void(*)(int)) sigquitsv);
		if(status)
			Debug("error %d from shell\n",status);
		return(status);
	}
	if(cmd == 0 || cmd[0] == '\0')
		cmd = "sh";
	for(i = 3;i < 20;i++)			/* close files except for stdin,out,err */
		close(i);
	signal(SIGINT,SIG_DFL);
	signal(SIGQUIT,SIG_DFL);
	execl("/bin/sh","sh","-c",cmd,0);
	_exit(127);
}

