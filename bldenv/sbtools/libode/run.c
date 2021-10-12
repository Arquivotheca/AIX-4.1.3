/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: dorun
 *		run
 *		runp
 *		runv
 *		runvp
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
 * Software Distribution Coordinator  or  Software_Distribution@CS.CMU.EDU 
 * School of Computer Science 
 * Carnegie Mellon University 
 * Pittsburgh PA 15213-3890 
 *  
 * any improvements or extensions that they make and grant Carnegie Mellon 
 * the rights to redistribute these changes. 
 */
/*
 * HISTORY
 * $Log: run.c,v $
 * Revision 1.8.11.2  1993/11/08  20:18:17  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/08  20:17:34  damon]
 *
 * Revision 1.8.11.1  1993/11/03  20:40:43  damon
 * 	CR 463. More pedantic
 * 	[1993/11/03  20:38:15  damon]
 * 
 * Revision 1.8.9.1  1993/08/19  18:26:31  damon
 * 	CR 622. Changed if STDC to ifdef STDC
 * 	[1993/08/19  18:25:48  damon]
 * 
 * Revision 1.8.7.3  1993/04/28  14:35:46  damon
 * 	CR 463. More pedantic changes
 * 	[1993/04/28  14:34:38  damon]
 * 
 * Revision 1.8.7.2  1993/04/27  22:12:53  damon
 * 	CR 463. Made pedantic changes
 * 	[1993/04/27  22:12:41  damon]
 * 
 * Revision 1.8.2.6  1992/12/03  17:22:18  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:08:58  damon]
 * 
 * Revision 1.8.2.5  1992/12/02  20:26:37  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/02  20:23:30  damon]
 * 
 * Revision 1.8.2.4  1992/09/24  19:02:31  gm
 * 	CR282: Made more portable to non-BSD systems.
 * 	[1992/09/23  18:22:33  gm]
 * 
 * Revision 1.8.2.3  1992/07/26  17:37:58  gm
 * 	Removed dependencies on BSD wait interfaces.
 * 	[1992/07/14  17:17:48  gm]
 * 
 * Revision 1.8.2.2  1992/06/22  21:26:27  damon
 * 	CR 181. Changed vfork to fork
 * 	[1992/06/22  21:24:44  damon]
 * 
 * Revision 1.8  1991/12/05  21:05:41  devrcs
 * 	Changed sdm to ode; std_defs.h to  odedefs.h
 * 	[91/01/10  11:46:46  randyb]
 * 
 * 	Correct copyright; clean up lint input; added ui_print function.
 * 	[91/01/08  12:20:40  randyb]
 * 
 * 	Eliminate compiler warnings:
 * 	  Declare dorun() static.
 * 	  Cast first argument to wait3.
 * 	[90/11/26  18:27:36  tom]
 * 
 * Revision 1.6  90/10/07  20:04:26  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:10:39  gm]
 * 
 * Revision 1.5  90/08/09  14:23:47  devrcs
 * 	Moved here from usr/local/sdm/lib/libsb.
 * 	[90/08/05  12:48:16  gm]
 * 
 * Revision 1.4  90/06/29  14:39:17  devrcs
 * 	Moved here from defunct libcs library.
 * 	[90/06/23  14:21:42  gm]
 * 
 * Revision 1.3  90/05/24  21:54:35  devrcs
 * 	Minor cosmetic changes.
 * 	[90/05/20  11:14:01  gm]
 * 
 * Revision 1.2  90/01/02  19:27:11  gm
 * 	Fixes for first snapshot.
 * 
 * Revision 1.1  89/12/26  10:15:33  gm
 * 	Current version from CMU.
 * 	[89/12/23            gm]
 * 
 * 	Added va_argv().  The decision it embodies needs to be
 * 	made afresh for each new architecture.
 * 	[89/08/31  10:36:16  bww]
 * 
 * 	Updated handling of variable argument lists.
 * 	[89/08/30  18:09:03  bww]
 * 
 * Revision 1.2  89/08/03  14:36:46  mja
 * 	Update run() and runp() to use <varargs.h>.
 * 	[89/04/19            mja]
 * 
 * Revision 0.0  86/09/23            gm0w
 * 	Merged old runv and runvp modules.
 * 	[86/09/23            gm0w]
 * 
 * Revision 0.0  85/11/22            gm0w
 * 	Added check and kill if child process was stopped.
 * 	[85/11/22            gm0w]
 * 
 * Revision 0.0  85/04/30            sas
 * 	Adapted for 4.2 BSD UNIX:  Conforms to new signals and wait.
 * 
 * Revision 0.0  82/07/15            mja & naf
 * 	Added a return(-1) if vfork fails.  This should only happen
 * 	if there are no more processes available.
 * 	[85/04/30            sas]
 * 
 * Revision 0.0  80/01/28            sas
 * 	Added setuid and setgid for system programs' use.
 * 	[80/01/28            sas]
 * 
 * Revision 0.0  80/01/21            sas
 * 	Changed fork to vfork.
 * 	[80/01/21            sas]
 * 
 * Revision 0.0  79/11/20            sas
 * 	Created for VAX.  The proper way to fork-and-execute a system
 * 	program is now by "runvp" or "runp", with the program name
 * 	(rather than an absolute pathname) as the first argument;
 * 	that way, the "PATH" variable in the environment does the right
 * 	thing.  Too bad execvp and execlp (hence runvp and runp) don't
 * 	accept a pathlist as an explicit argument.
 * 	[79/11/20            sas]
 * 
 * $EndLog$
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

#ifndef lint
static char sccsid[] = "@(#)16  1.1  src/bldenv/sbtools/libode/run.c, bldprocess, bos412, GOLDA411a 1/19/94 17:42:19";
#endif /* not lint */

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ode/interface.h>
#include <ode/run.h>

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#if	VA_ARGV_IS_RECAST
#define va_argv(list) ((char * const *) list)
#else
#if	!VA_ARGV_IS_ROUTINE
error Please define va_argv() macro(?) for your machine!!
#endif
#endif

static int
dorun ( const char *name, char * const *argv, int usepath );
int
runv ( const char *name, char * const *argv );

#ifdef __STDC__
int run (char *name, ...)
#else
int run (va_alist)
va_dcl
#endif
{
#ifndef __STDC__
	char *name;
#endif
	va_list ap;
	int val;

#ifdef __STDC__
	va_start(ap, name);
#else
	va_start(ap);
	name = va_arg(ap, char *);
#endif
	val = runv (name, va_argv(ap));
	va_end(ap);
	return(val);
}

int runv ( const char *name, char * const *argv )
{
	return (dorun (name, argv, 0));
}

#ifdef __STDC__
int runp (const char *name, ...)
#else
int runp (va_alist)
va_dcl
#endif
{
#ifndef __STDC__
	char *name;
#endif
	va_list ap;
	int val;

#ifdef __STDC__
	va_start(ap, name);
#else
	va_start(ap);
	name = va_arg(ap, char *);
#endif
	val = runvp (name, va_argv(ap));
	va_end(ap);
	return (val);
}

int
runvp ( const char *name, char * const *argv )
{
	return (dorun (name, argv, 1));
}

static
int dorun ( const char *name, char * const *argv, int usepath )
{
	int wpid;
	register int pid;
	struct sigaction ignoresig,intsig,quitsig;
	int status;
	int (*execrtn)( const char *, char * const dargv[])
                      = usepath ? execvp : execv;

	if ((pid = fork()) == -1)
		return(-1);	/* no more process's, so exit with error */

	if (pid == 0) {			/* child process */
		setgid (getgid());
		setuid (getuid());
		(*execrtn) (name,argv);
		ui_print ( VFATAL, "run: can't exec %s\n",name);
		_exit (0377);
	}

	ignoresig.sa_handler = SIG_IGN;	/* ignore INT and QUIT signals */
	sigemptyset(&ignoresig.sa_mask);
	ignoresig.sa_flags = 0;
	sigaction (SIGINT,&ignoresig,&intsig);
	sigaction (SIGQUIT,&ignoresig,&quitsig);
	do {
		wpid = waitpid (pid, &status, WUNTRACED);
		if (WIFSTOPPED (status)) {
		    kill (0,SIGTSTP);
		    wpid = 0;
		}
	} while (wpid != pid && wpid != -1);
	sigaction (SIGINT,&intsig,0);	/* restore signals */
	sigaction (SIGQUIT,&quitsig,0);

	if (WIFSIGNALED (status) || WEXITSTATUS(status) == 0377)
		return (-1);

	return (WEXITSTATUS(status));
}
