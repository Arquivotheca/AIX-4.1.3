/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: endcmd
 *		fd_runcmd
 *		fd_runcmdv
 *		runcmd
 *		runcmdv
 *		va_argv
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * @OSF_FREE_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1992, 1991, 1990  
 * Open Software Foundation, Inc. 
 *  
 * Permission is hereby granted to use, copy, modify and freely distribute 
 * the software in this file and its documentation for any purpose without 
 * fee, provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation.  Further, provided that the name of Open 
 * Software Foundation, Inc. ("OSF") not be used in advertising or 
 * publicity pertaining to distribution of the software without prior 
 * written permission from OSF.  OSF makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is" 
 * without express or implied warranty. 
 */
/*
 * HISTORY
 * $Log: runcmd.c,v $
 * Revision 1.7.12.2  1993/11/05  22:43:18  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/05  22:41:22  damon]
 *
 * Revision 1.7.12.1  1993/11/03  20:40:48  damon
 * 	CR 463. More pedantic
 * 	[1993/11/03  20:38:18  damon]
 * 
 * Revision 1.7.10.1  1993/08/19  18:26:36  damon
 * 	CR 622. Changed if STDC to ifdef STDC
 * 	[1993/08/19  18:25:50  damon]
 * 
 * Revision 1.7.4.4  1993/04/27  22:12:58  damon
 * 	CR 463. Made pedantic changes
 * 	[1993/04/27  22:12:45  damon]
 * 
 * Revision 1.7.4.3  1993/04/08  22:51:35  damon
 * 	CR 446. Clean up include files
 * 	[1993/04/08  22:51:19  damon]
 * 
 * Revision 1.7.4.2  1993/01/21  19:12:30  damon
 * 	CR 401. Added stdarg
 * 	[1993/01/21  19:12:13  damon]
 * 
 * Revision 1.7.2.7  1992/12/03  17:22:23  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:09:02  damon]
 * 
 * Revision 1.7.2.6  1992/09/24  19:02:36  gm
 * 	CR282: Made more portable to non-BSD systems.
 * 	[1992/09/23  18:22:46  gm]
 * 
 * Revision 1.7.2.5  1992/07/26  17:38:02  gm
 * 	Removed dependencies on BSD wait interfaces.  Added consistant use
 * 	of NOEXEC definition.
 * 	[1992/07/14  17:18:46  gm]
 * 
 * Revision 1.7.2.4  1992/06/22  22:11:43  damon
 * 	CR 182. Added debugging code from bcs/subrs.c
 * 	[1992/06/22  22:09:38  damon]
 * 
 * Revision 1.7.2.3  1992/06/22  21:26:36  damon
 * 	CR 181. Changed vfork to fork
 * 	[1992/06/22  21:24:57  damon]
 * 
 * 	Added conditional va_argv recast
 * 	[1992/03/24  15:58:15  damon]
 * 
 * 	Added va_argv call in runcmd proc
 * 	[1992/03/24  01:32:10  damon]
 * 
 * 	Added va_argv call in fd_runcmd proc
 * 	[1992/03/24  01:18:09  damon]
 * 
 * Revision 1.7.2.2  1992/06/15  18:10:45  damon
 * 	Synched with 2.1.1
 * 	[1992/06/15  18:04:41  damon]
 * 
 * Revision 1.7.6.2  1992/06/15  16:32:40  damon
 * 	Taken from 2.1.1
 * 
 * Revision 1.7  1991/12/05  21:05:47  devrcs
 * 	Changed sdm to ode; std_defs.h to  odedefs.h
 * 	[91/01/10  11:46:52  randyb]
 * 
 * 	Correct copyright; clean up lint input; added ui_print function.
 * 	[91/01/08  12:20:57  randyb]
 * 
 * Revision 1.5  90/10/07  20:04:30  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:10:46  gm]
 * 
 * Revision 1.4  90/08/09  14:23:50  devrcs
 * 	Moved here from usr/local/sdm/lib/libsb.
 * 	[90/08/05  12:48:22  gm]
 * 
 * Revision 1.3  90/07/17  12:37:06  devrcs
 * 	More changes for gcc.
 * 	[90/07/04  22:31:10  gm]
 * 
 * Revision 1.2  90/05/24  23:12:47  devrcs
 * 	Added additional include files.
 * 	[90/05/09            gm]
 * 
 * 	Created.
 * 	[90/05/07            gm]
 * 
 * $EndLog$
 */

#ifndef lint
static char sccsid[] = "@(#)18  1.1  src/bldenv/sbtools/libode/runcmd.c, bldprocess, bos412, GOLDA411a 1/19/94 17:42:23";
#endif /* not lint */

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ode/interface.h>
#include <ode/odedefs.h>
#include <ode/run.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#if     VA_ARGV_IS_RECAST
#define va_argv(list) ((char **) list)
#else
#if     !VA_ARGV_IS_ROUTINE
error Please define va_argv() macro(?) for your machine!!
#endif
#endif

#define NOEXEC          0377

int
fd_runcmdv( const char *cmd, const char *dir, int withpath, int closefd,
            int outfd, char **av )
{
    int child;
    int fd;

    (void) fflush(stderr);
    if ((child = fork()) == -1) {
	ui_print ( VFATAL, "%s fork failed: %s\n", av[0], strerror(errno));
	return(-1);
    }
    if (child == 0) {
	(void) setgid(getgid());
	(void) setuid(getuid());
	if (dir != NULL && chdir(dir) < 0) {
	    ui_print ( VFATAL, "%s chdir failed: %s\n", dir, strerror(errno));
	    (void) fflush(stderr);
	    _exit(NOEXEC);
	}
	if (closefd >= 0)
	    (void) close(closefd);
	if (outfd >= 0 && outfd != 1) {
	    (void) dup2(outfd, 1);
	    (void) close(outfd);
	}
	if ((fd = open("/dev/null", O_RDONLY, 0)) > 0) {
	    (void) dup2(fd, 0);
	    (void) close(fd);
	}
	if (withpath)
	    execvp(cmd, av);
	else
	    execv(cmd, av);
	_exit(NOEXEC);
    }
    return(child);
}

int
#ifdef __STDC__
fd_runcmd(const char *cmd, const char *dir, int withpath, int closefd,
          int outfd, ... )
#else
fd_runcmd(cmd, dir, withpath, closefd, outfd, va_alist)
va_dcl
#endif
{
    int status;
    va_list ap;

#ifdef __STDC__
    va_start(ap, outfd );
#else
    va_start(ap);
#endif
    status = fd_runcmdv(cmd, dir, withpath, closefd, outfd, va_argv (ap) );
    va_end(ap);
    return(status);
}

int
runcmdv( const char *cmd, const char *dir, char **av )
{
    return(fd_runcmdv(cmd, dir, 0, -1, -1, av));
}

int
#ifdef __STDC__
runcmd(const char *cmd, const char *dir, ... )
#else
runcmd(cmd, dir, va_alist)
char *cmd, *dir;
va_dcl
#endif
{
    int status;
    va_list ap;

#ifdef __STDC__
    va_start(ap, dir);
#else
    va_start(ap);
#endif
    status = fd_runcmdv(cmd, dir, 0, -1, -1, va_argv (ap) );
    va_end(ap);
    return(status);
}

int
endcmd( int child )
{
    int pid;
    sigset_t nmask, omask;
    int status;

    sigemptyset(&nmask);
    sigaddset(&nmask, SIGINT);
    sigaddset(&nmask, SIGQUIT);
    sigaddset(&nmask, SIGHUP);
    (void) sigprocmask(SIG_BLOCK, &nmask, &omask);
    do {
	pid = waitpid(child, &status, WUNTRACED);
	if (WIFSTOPPED(status)) {
	    (void) kill(0, SIGTSTP);
	    pid = 0;
	}
    } while (pid != child && pid != -1);
    (void) sigprocmask(SIG_SETMASK, &omask, (sigset_t *)0);
    if (pid == -1) {
	ui_print ( VFATAL, "wait error: %s\n", strerror(errno));
	return(-1);
    }
    if (WIFSIGNALED(status) || WEXITSTATUS(status) == NOEXEC) {
        if (WIFSIGNALED(status))
	    ui_print ( VFATAL, "process killed by signal %d", WTERMSIG(status));
	else
            ui_print ( VFATAL, "process exited with -1");
	return(-1);
    }
    ui_print ( VDEBUG, "process exited with %d", WEXITSTATUS(status));
    return(WEXITSTATUS(status));
}
