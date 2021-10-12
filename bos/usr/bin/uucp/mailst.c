static char sccsid[] = "@(#)06	1.7  src/bos/usr/bin/uucp/mailst.c, cmduucp, bos411, 9428A410j 6/17/93 14:22:01";
/* 
 * COMPONENT_NAME: CMDUUCP mailst.c
 * 
 * FUNCTIONS: mailst, setuucp 
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

/*	/sccs/src/cmd/uucp/s.mailst.c
	mailst.c	1.1	7/29/85 16:33:14
*/
#include "uucp.h"
#include <sys/syslog.h>
/* VERSION( mailst.c	5.3 -  -  ); */

extern nl_catd catd;
/*
 * fork and execute a mail command sending 
 * string (str) to user (user).
 * If file is non-null, the file is also sent.
 * (this is used for mail returned to sender.)
 *	user	 -> user to send mail to
 *	str	 -> string mailed to user
 *	infile	 -> optional stdin mailed to user
 *	errfile	 -> optional stderr mailed to user
 */
mailst(user, str, infile, errfile)
char *user, *str, *infile, *errfile;
{
	register FILE *fp, *fi;
	char cmd[BUFSIZ];
	int rc;
	char *c;

        /* get rid of some stuff that could be dangerous */
        if ( (c = strpbrk(user, ";&|<>^`\\('\"{}\n")) != NULL) {
                *c = NULLCHAR;
        }

	openlog("uucp", LOG_PID | LOG_CONS, LOG_MAIL);
	(void) sprintf(cmd, "%s mail %s", PATH, user);
	if ((fp = popen(cmd, "w")) == NULL) {
		syslog(LOG_ERR,
		    MSGSTR(MSG_MAILST5, "error running mail command: %m"));
		return;
	}
	(void) fprintf(fp, "%s\n", str);

	/* copy back stderr */
	if (*errfile != '\0' && NOTEMPTY(errfile) && (fi = fopen(errfile, "r")) != NULL) {
		fprintf(fp, MSGSTR(MSG_MAILST2,
				"\n\t===== stderr was =====\n"));
		if (xfappend(fi, fp) != SUCCESS)
			fprintf(fp, MSGSTR(MSG_MAILST3, 
			   "\n\t===== well, i tried =====\n"));
		(void) fclose(fi);
		fprintf(fp, "\n");
	}

        /* copy back stdin */
	if ( *infile != '\0' ) {
		fprintf(fp, MSGSTR(MSG_MAILST4, 
			"\n\t===== stdin was =====\n"));
		if ( !NOTEMPTY(infile) )
			fprintf(fp, MSGSTR(MSG_MAILST7, 
				"empty =====\n"));
		else if ( chkpth(infile, CK_READ) == FAIL ) {
			fprintf(fp, MSGSTR(MSG_MAILST8, 
				"denied read permission =====\n"));
			sprintf(cmd, "user %s, stdin %s", user, infile);
			logent(cmd, "DENIED");
		}

		else if ( (fi = fopen(infile, "r")) == NULL ) {
			fprintf(fp, MSGSTR(MSG_MAILST9, 
				"unreadable =====\n"));
			sprintf(cmd, "user %s, stdin %s", user, infile);
			logent(cmd, "DENIED");
		}
		else {
			fputs("=====\n", fp);
			if (xfappend(fi, fp) != SUCCESS)
				fprintf(fp, MSGSTR(MSG_MAILST3, 
					"\n\t===== well, i tried =====\n"));
			(void) fclose(fi);
		}
                fputc('\n', fp);
        }

	if (rc = pclose(fp))
		syslog(LOG_ERR,
		    MSGSTR(MSG_MAILST6, "mail returned %#x status"), rc);
}
#ifndef	V7
static char un[2*NAMESIZE];
setuucp(p)
char *p;
{
   char **envp;

    envp = Env;
    for ( ; *envp; envp++) {
	if(PREFIX("LOGNAME", *envp)) {
	    (void) sprintf(un, "LOGNAME=%s",p);
	    envp[0] = &un[0];
	}
    }
}
#else
/*ARGSUSED*/
setuucp(p) char	*p; {}
#endif
