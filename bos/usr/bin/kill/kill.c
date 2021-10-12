static char sccsid[] = "@(#)87  1.14.1.4  src/bos/usr/bin/kill/kill.c, cmdcntl, bos41B, 9504A 1/4/95 10:12:53";
/*
 * COMPONENT_NAME: (CMDCNTL) system control commands
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 26, 27, 18
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 */
/*
 * @OSF_COPYRIGHT@
static char rcsid[] = "RCSfile: kill.c,v Revision: 1.5 (OSF) Date: 90/10/07 16:38:49 ";
 */
/*
 *   The kill command sends a  signal to a running process, by default
 *   signal SIGTERM  (Software  Terminate #15).  This  default action
 *   normally kills  processes  that do  not catch  or ignore the
 *   signal.   You specify a process  by giving its process-ID (process
 *   identification  number, or PID).
 */                                                                   

#ifdef SEC_BASE
#include        <sys/secdefines.h>
#include        <sys/security.h>

extern priv_t   *privvec();
#endif

#include        <stdio.h>
#include        <signal.h>
#include        <limits.h>
#include        <sys/errno.h>
#include        <sys/wait.h>

#include        <nl_types.h>
#include        "kill_msg.h"

#define         MSGSTR(num,str) catgets(catd,MS_KILL,num,str)  /*MSG*/

static nl_catd         catd;
#include <locale.h>
extern int signum(char *signal);
extern void sigprt();

main(argc, argv)
char **argv;
{
        register signo, res;
        pid_t   pid;
        int     errlev = 0, neg = 0, zero = 0;
        extern  errno;
        char    *msg, *argp;


        (void) setlocale (LC_ALL,"");
        catd = catopen(MF_KILL, NL_CAT_LOCALE);
        if (argc <= 1)
                usage();
        argp = argv[1];
#if SEC_BASE
        set_auth_parameters(argc, argv);
#endif
        if (!strcmp(argp,"-l")) {
                if (argc > 2) {
                  int status = strtoul(argv[2], &argp,0);

		  if (status >= NSIG) {
		    signo = WSTOPSIG(status);
		    if (signo == -1) signo = WTERMSIG(status);
		  } else
		    signo = status;

                  if ( !argp || *argp || (signo == -1) || (signo >= NSIG)) {
                    fprintf(stderr, MSGSTR(BADEXIT,"bad exit status\n"));
                    exit(2);
                  }
                  sigprt(signo);
                } else
                  sigprt(-1);
                exit(0);
        }
        else if (*argp == '-') {
                if (!strcmp(argp,"-s")) {
			if (argc == 2) {
                		usage();
				exit(1);
			}
                        argp = argv[2];
                        argc--;
                        argv++;
                	signo = signum(argp);
                } else {
                        argp++;
			if (*argp == '-') signo = SIGTERM;
			else signo = signum(argp);
                }
                if (signo == -1) {
                        fprintf(stderr, MSGSTR(BADNO,"bad signal number\n"));
                        usage();
                }
                argc--;
                argv++;
        } else
                signo = SIGTERM;
	if (!strcmp(argv[1], "--")) {
                argc--;
                argv++;
	}
#if SEC_BASE
        initprivs();
        if (authorized_user("sysadmin") &&
            forceprivs(privvec(SEC_KILL,
#if SEC_MAC
                                SEC_ALLOWMACACCESS,
#endif
                                -1), (priv_t *) 0)) {
                fprintf(stderr, MSGSTR(PRIV, "%s: insufficient privileges\n"),
                        "kill");
                exit(1);
        }
#endif
        argv++;
	if (argc == 1) {
                usage();
		exit(1);
	}
        while (argc > 1) {
                if (**argv == '-') neg++;
                if (**argv == '0') zero++;
                pid = atoi(*argv);
                if (    ((pid == 0) && !zero)
                     || ((pid < 0) && !neg)
                     || (pid > PID_MAX)
                     || (pid < -PID_MAX)
                      ) usage();
#if SEC_BASE
                disablepriv(SEC_SUSPEND_AUDIT);
#endif
                res = kill(pid, signo);
#if SEC_BASE
                forcepriv(SEC_SUSPEND_AUDIT);
#endif
                if (res<0) {
                        if(pid <= 0) {
                                pid = abs(pid);
                                msg = MSGSTR(EPGROUP,
                                             "not a killable process group");
                        }
                        else if (errno == EPERM)
                                msg = MSGSTR(EPDENIED, "permission denied");
                        else if (errno == EINVAL)
                                msg = MSGSTR(ESIGNAL, "invalid signal");
                        else msg = MSGSTR(ENOPROC, "no such process");
                        fprintf(stderr,MSGSTR(EKILL, "kill: %d: %s\n"), pid, msg);
                        errlev = 2;
                }
                argc--;
                argv++;
                neg = zero = 0;
        }
        return(errlev);
}

/*
 * NAME: usage
 *                                                                    
 * FUNCTION: output usage messages and exit with abnormal status.
 *
 * RETURNS: exit with status 2.
 *
 */  
static usage()
{
        fprintf(stderr, MSGSTR(USAGE, "usage: kill [ -signal | -s signal] pid ...\n"));
        fprintf(stderr, MSGSTR(USAGE2, "usage: kill -l [exit_status]\n"));
        exit(2);
}
