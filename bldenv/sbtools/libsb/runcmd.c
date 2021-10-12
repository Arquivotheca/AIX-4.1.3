static char sccsid[] = "@(#)54  1.1  src/bldenv/sbtools/libsb/runcmd.c, bldprocess, bos412, GOLDA411a 4/29/93 12:23:19";
/*
 * Copyright (c) 1990, 1991, 1992  
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
 * ODE 2.1.1
 */

#include <sys/types.h>
#include <sys/signal.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <stdio.h>
#include <ode/interface.h>

/*
 * FIXME
 * Should use stdarg.h if available
 */
#include <varargs.h>

#if     VA_ARGV_IS_RECAST
#define va_argv(list) ((char **) list)
#else
#if     !VA_ARGV_IS_ROUTINE
error Please define va_argv() macro(?) for your machine!!
#endif
#endif

fd_runcmdv(cmd, dir, withpath, closefd, outfd, av)
char *cmd, *dir;
int withpath;
int closefd, outfd;
char **av;
{
    int child;
    int fd;

    (void) fflush(stderr);
    if ((child = vfork()) == -1) {
	ui_print ( VFATAL, "%s vfork failed: %s\n", av[0], errmsg(-1));
	return(-1);
    }
    if (child == 0) {
	(void) setgid(getgid());
	(void) setuid(getuid());
	if (dir != NULL && chdir(dir) < 0) {
	    ui_print ( VFATAL, "%s chdir failed: %s\n", dir, errmsg(-1));
	    (void) fflush(stderr);
	    _exit(0377);
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
	_exit(0377);
    }
    return(child);
}

/* VARARGS6 */
fd_runcmd(cmd, dir, withpath, closefd, outfd, va_alist)
char *cmd, *dir;
int withpath;
int closefd, outfd;
va_dcl
{
    int status;
    va_list ap;

    va_start(ap);
    status = fd_runcmdv(cmd, dir, withpath, closefd, outfd, va_argv (ap) );
    va_end(ap);
    return(status);
}

runcmdv(cmd, dir, av)
char *cmd, *dir;
char **av;
{
    return(fd_runcmdv(cmd, dir, 0, -1, -1, av));
}

/* VARARGS3 */
runcmd(cmd, dir, va_alist)
char *cmd, *dir;
va_dcl
{
    int status;
    va_list ap;

    va_start(ap);
    status = fd_runcmdv(cmd, dir, 0, -1, -1, va_argv (ap) );
    va_end(ap);
    return(status);
}

endcmd(child)
int child;
{
    int pid, omask;
    union wait w;
    int status;

    omask = sigblock(sigmask(SIGINT)|sigmask(SIGQUIT)|sigmask(SIGHUP));
    do {
	pid = wait3(&w, WUNTRACED, (struct rusage *)NULL);
	status = w.w_status;
	if (WIFSTOPPED(w)) {
	    (void) kill(0, SIGTSTP);
	    pid = 0;
	}
    } while (pid != child && pid != -1);
    (void) sigsetmask(omask);
    if (pid == -1) {
	ui_print ( VFATAL, "wait error: %s\n", errmsg(-1));
	return(-1);
    }
    if (WIFSIGNALED(w) || w.w_retcode == 0377)
	return(-1);
    return(w.w_retcode);
}
