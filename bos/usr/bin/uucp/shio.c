static char sccsid[] = "@(#)15	1.4  src/bos/usr/bin/uucp/shio.c, cmduucp, bos411, 9428A410j 6/17/93 14:22:44";
/* 
 * COMPONENT_NAME: CMDUUCP shio.c
 * 
 * FUNCTIONS: shio 
 *
 * ORIGINS: 10  27  3 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	/sccs/src/cmd/uucp/s.shio.c
	shio.c	1.1	7/29/85 16:33:26
*/
#include "uucp.h"
/* VERSION( shio.c	5.2 -  -  ); */

extern void close_log();

/*
 * use shell to execute command with
 * fi, fo, and fe as standard input/output/error
 *	cmd 	-> command to execute
 *	fi 	-> standard input
 *	fo 	-> standard output
 *	fe 	-> standard error
 * return:
 *	0		-> success 
 *	non zero	-> failure  -  status from child
			(Note - -1 means the fork failed)
 */
shio(cmd, fi, fo, fe)
char *cmd, *fi, *fo, *fe;
{
	register int pid, ret;
	int status;

	if (fi == NULL)
		fi = "/dev/null";
	if (fo == NULL)
		fo = "/dev/null";
	if (fe == NULL)
		fe = "/dev/null";

	DEBUG(3, "shio - %s\n", cmd);
	if ((pid = fork()) == 0) {
		(void) signal(SIGINT, SIG_IGN);
		(void) signal(SIGHUP, SIG_IGN);
		(void) signal(SIGQUIT, SIG_IGN);
		close_log();
		(void) close(Ifn);	/* close connection fd's */
		(void) close(Ofn);
		(void) close(0);	/* get stdin from file fi */
		if (open(fi, 0) != 0)
			exit(errno);
		(void) close(1);	/* divert stdout to fo */
		if (creat(fo, 0666) != 1)
			exit(errno);
		(void) close(2);	/* divert stderr to fe */
		if (creat(fe, 0666) != 2)
			exit(errno);
		(void) execle(SHELL, "sh", "-c", cmd, 0, Env);
		exit(100);
	}

	/*
	 * the status returned from wait can never be -1
	 * see man page wait(2)
	 * So we use the -1 value to indicate fork failed
	 */
	if (pid == -1)
		return(-1);
	

	while ((ret = wait(&status)) != pid && ret != -1);
	DEBUG(3, "status %d\n", status);
	return(status);
}
