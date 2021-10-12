static char sccsid[] = "@(#)52  1.1  src/bldenv/sbtools/libsb/run.c, bldprocess, bos412, GOLDA411a 4/29/93 12:23:04";
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
 *  
 * Copyright (c) 1992 Carnegie Mellon University 
 * All Rights Reserved. 
 *  
 * Permission to use, copy, modify and distribute this software and its 
 * documentation is hereby granted, provided that both the copyright 
 * notice and this permission notice appear in all copies of the 
 * software, derivative works or modified versions, and any portions 
 * thereof, and that both notices appear in supporting documentation. 
 *  
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" 
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR 
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE. 
 *  
 * Carnegie Mellon requests users of this software to return to 
 *  
 *  Software Distribution Coordinator  or  Software_Distribution@CS.CMU.EDU 
 *  School of Computer Science 
 *  Carnegie Mellon University 
 *  Pittsburgh PA 15213-3890 
 *  
 * any improvements or extensions that they make and grant Carnegie Mellon 
 * the rights to redistribute these changes. 
 */
/*
 * ODE 2.1.1
 */
/*
 *  run, runv, runp, runvp --  execute process and wait for it to exit
 *
 *  Usage:
 *	i = run (file, arg1, arg2, ..., argn, 0);
 *	i = runv (file, arglist);
 *	i = runp (file, arg1, arg2, ..., argn, 0);
 *	i = runvp (file, arglist);
 *
 *  Run, runv, runp and runvp have argument lists exactly like the
 *  corresponding routines, execl, execv, execlp, execvp.  The run
 *  routines perform a fork, then:
 *  IN THE NEW PROCESS, an execl[p] or execv[p] is performed with the
 *  specified arguments.  The process returns with a -1 code if the
 *  exec was not successful.
 *  IN THE PARENT PROCESS, the signals SIGQUIT and SIGINT are disabled,
 *  the process waits until the newly forked process exits, the
 *  signals are restored to their original status, and the return
 *  status of the process is analyzed.
 *  All run routines return:  -1 if the exec failed or if the child was
 *  terminated abnormally; otherwise, the exit code of the child is
 *  returned.
 */

#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <ode/interface.h>

#if __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#if	VA_ARGV_IS_RECAST
#define va_argv(list) ((char **) list)
#else
#if	!VA_ARGV_IS_ROUTINE
error Please define va_argv() macro(?) for your machine!!
#endif
#endif

static int dorun();

#if __STDC__
int run (char *name, ...)
#else
int run (va_alist)
va_dcl
#endif
{
#if !__STDC__
	char *name;
#endif
	va_list ap;
	int val;

#if __STDC__
	va_start(ap, name);
#else
	va_start(ap);
	name = va_arg(ap, char *);
#endif
	val = runv (name, va_argv(ap));
	va_end(ap);
	return(val);
}

int runv (name,argv)
char *name,**argv;
{
	return (dorun (name, argv, 0));
}

#if __STDC__
int runp (char *name, ...)
#else
int runp (va_alist)
va_dcl
#endif
{
#if !__STDC__
	char *name;
#endif
	va_list ap;
	int val;

#if __STDC__
	va_start(ap, name);
#else
	va_start(ap);
	name = va_arg(ap, char *);
#endif
	val = runvp (name, va_argv(ap));
	va_end(ap);
	return (val);
}

int runvp (name,argv)
char *name,**argv;
{
	return (dorun (name, argv, 1));
}

static
int dorun (name,argv,usepath)
char *name,**argv;
int usepath;
{
	int wpid;
	register int pid;
	struct sigvec ignoresig,intsig,quitsig;
	union wait status;
	int execvp(), execv();
	int (*execrtn)() = usepath ? execvp : execv;

	if ((pid = vfork()) == -1)
		return(-1);	/* no more process's, so exit with error */

	if (pid == 0) {			/* child process */
		setgid (getgid());
		setuid (getuid());
		(*execrtn) (name,argv);
		ui_print ( VFATAL, "run: can't exec %s\n",name);
		_exit (0377);
	}

	ignoresig.sv_handler = SIG_IGN;	/* ignore INT and QUIT signals */
	ignoresig.sv_mask = 0;
	ignoresig.sv_onstack = 0;
	sigvec (SIGINT,&ignoresig,&intsig);
	sigvec (SIGQUIT,&ignoresig,&quitsig);
	do {
		wpid = wait3 ((union wait *)&status.w_status, WUNTRACED, 0);
		if (WIFSTOPPED (status)) {
		    kill (0,SIGTSTP);
		    wpid = 0;
		}
	} while (wpid != pid && wpid != -1);
	sigvec (SIGINT,&intsig,0);	/* restore signals */
	sigvec (SIGQUIT,&quitsig,0);

	if (WIFSIGNALED (status) || status.w_retcode == 0377)
		return (-1);

	return (status.w_retcode);
}
