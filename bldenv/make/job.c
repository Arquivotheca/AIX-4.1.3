/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: JobCmpPid
 *		JobCondPassSig
 *		JobExec
 *		JobFinish
 *		JobInterrupt
 *		JobMakeArgv
 *		JobPassSig
 *		JobRestart
 *		JobStart
 *		Job_AbortAll
 *		Job_CatchChildren1291
 *		Job_CheckCommands
 *		Job_Empty
 *		Job_End
 *		Job_Full
 *		Job_Init
 *		Job_Make
 *		Job_Touch
 *		Job_Wait
 *		KILL
 *		WEXITSTATUS
 *		WIFEXITED
 *		WIFSIGNALED
 *		WIFSTOPPED
 *		WSTOPSIG
 *		WTERMSIG
 *		W_EXITCODE
 *		_WSTATUS
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
 * $Log: job.c,v $
 * Revision 1.2.8.2  1993/04/26  20:27:13  damon
 * 	CR 424. make now handles error returns better
 * 	[1993/04/26  20:26:53  damon]
 *
 * Revision 1.2.8.1  1993/04/26  20:27:09  damon
 * *** Initial Branch Revision ***
 *
 * Revision 1.2.3.9  1992/12/03  19:05:28  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:35:17  damon]
 * 
 * Revision 1.2.3.8  1992/11/13  15:19:45  root
 * 	Now includes string.h
 * 	[1992/11/13  14:54:32  root]
 * 
 * Revision 1.2.3.7  1992/11/09  21:50:22  damon
 * 	CR 296. Cleaned up to remove warnings
 * 	[1992/11/09  21:49:09  damon]
 * 
 * Revision 1.2.3.6  1992/09/24  19:24:14  gm
 * 	CR286: Major improvements to make internals.
 * 	[1992/09/24  17:54:21  gm]
 * 
 * Revision 1.2.3.5  1992/06/24  16:31:32  damon
 * 	CR 181. Changed vfork to fork
 * 	[1992/06/24  16:19:26  damon]
 * 
 * Revision 1.2.3.4  1992/06/16  21:24:32  damon
 * 	2.1.1 touch-up
 * 	[1992/06/16  21:19:12  damon]
 * 
 * Revision 1.2.3.3  1992/06/12  00:49:04  damon
 * 	Synched with 2.1.1
 * 	[1992/06/12  00:35:18  damon]
 * 
 * Revision 1.2.2.3  1992/03/25  22:45:56  damon
 * 	Removed comment after endif
 * 	[1992/03/25  21:49:44  damon]
 * 
 * Revision 1.2.2.2  1992/03/16  14:33:19  mckeen
 * 	Added code to JobStart to prevent abort if keepgoing is defined
 * 	and no rules are found for a target.  This fixes the -k switch.
 * 	[1992/03/16  14:30:58  mckeen]
 * 
 * Revision 1.2  1991/12/05  20:42:48  devrcs
 * 	Changes for parallel make.
 * 	[91/04/21  16:37:03  gm]
 * 
 * 	Added definition of FD_SETSIZE and INC_TYPES
 * 	[91/04/03  12:44:13  damon]
 * 
 * 	Changes for Reno make
 * 	[91/03/22  16:00:36  mckeen]
 * 
 * $EndLog$
 */
/*
 * Copyright (c) 1988, 1989, 1990 The Regents of the University of California.
 * Copyright (c) 1988, 1989 by Adam de Boor
 * Copyright (c) 1989 by Berkeley Softworks
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Adam de Boor.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
static char sccsid[] = "@(#)40  1.4  src/bldenv/make/job.c, bldprocess, bos412, GOLDA411a 1/19/94 16:27:36";
#endif /* not lint */

#ifndef lint
static char rcsid[] = "@(#)job.c	5.15 (Berkeley) 3/1/91";
#endif /* not lint */

/*-
 * job.c --
 *	handle the creation etc. of our child processes.
 *
 * Interface:
 *	Job_Make  	    	Start the creation of the given target.
 *
 *	Job_CatchChildren   	Check for and handle the termination of any
 *	    	  	    	children. This must be called reasonably
 *	    	  	    	frequently to keep the whole make going at
 *	    	  	    	a decent clip, since job table entries aren't
 *	    	  	    	removed until their process is caught this way.
 *	    	  	    	Its single argument is TRUE if the function
 *	    	  	    	should block waiting for a child to terminate.
 *
 *	Job_Init  	    	Called to intialize this module. in addition,
 *	    	  	    	any commands attached to the .BEGIN target
 *	    	  	    	are executed before this function returns.
 *	    	  	    	Hence, the makefile must have been parsed
 *	    	  	    	before this function is called.
 *
 *	Job_Full  	    	Return TRUE if the job table is filled.
 *
 *	Job_Empty 	    	Return TRUE if the job table is completely
 *	    	  	    	empty.
 *
 *	Job_End	  	    	Perform any final processing which needs doing.
 *	    	  	    	This includes the execution of any commands
 *	    	  	    	which have been/were attached to the .END
 *	    	  	    	target. It should only be called when the
 *	    	  	    	job table is empty.
 *
 *	Job_AbortAll	    	Abort all currently running jobs. It doesn't
 *	    	  	    	handle output or do anything for the jobs,
 *	    	  	    	just kills them. It should only be called in
 *	    	  	    	an emergency, as it were.
 *
 *	Job_CheckCommands   	Verify that the commands for a target are
 *	    	  	    	ok. Provide them if necessary and possible.
 *
 *	Job_Touch 	    	Update a target without really updating it.
 *
 *	Job_Wait  	    	Wait for all currently-running jobs to finish.
 */

#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <utime.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include "make.h"
#include "job.h"
#include "pathnames.h"

#define STATIC	/* static		debugging support */

extern char **environ;

#ifndef W_EXITCODE
#undef WIFSTOPPED
#undef WIFSIGNALED
#undef WIFEXITED
#define _WSTOPPED	0177
#define _WSTATUS(x)	((x)&_WSTOPPED)
#define WIFSTOPPED(x)	(_WSTATUS(x) == _WSTOPPED)
#undef WSTOPSIG
#define WSTOPSIG(x)	(((x)>>8)&0177)
#define WIFEXITED(x)	(_WSTATUS(x) == 0)
#undef WEXITSTATUS
#define WEXITSTATUS(x)	(((x)>>8)&0377)
#define WIFSIGNALED(x)	(_WSTATUS(x) != _WSTOPPED && _WSTATUS(x) != 0)
#undef WTERMSIG
#define WTERMSIG(x)	(_WSTATUS(x))
#define W_EXITCODE(r,s)	((r)<<8|(s))
#endif
#ifndef WAIT_ANY
#define WAIT_ANY	-1
#endif

#ifndef FD_SETSIZE
#define FD_SETSIZE      256
#endif

/*
 * error handling variables 
 */
int         	errors = 0;	    /* number of errors reported */
int  	    	aborting = 0;	    /* why is the make aborting? */
#define ABORT_ERROR	1   	    /* Because of an error */
#define ABORT_INTERRUPT	2   	    /* Because it was interrupted */
#define ABORT_WAIT	3   	    /* Waiting for jobs to finish */


/*
 * post-make command processing
 */
STATIC GNode	*endNode;	/* node containing commands to execute when
				 * everything else is done w/o errors */
STATIC GNode	*intrNode;	/* node describing the .INTERRUPT target */
STATIC GNode	*errNode;	/* node containing commands to execute when
				 * everything else is done w/ errors */
STATIC GNode	*exitNode;	/* node containing commands to execute when
				 * everything else is done always */

/*
 * Return values from JobStart.
 */
#define JOB_RUNNING	0   	/* Job is running */
#define JOB_ERROR 	1   	/* Error in starting the job */
#define JOB_FINISHED	2   	/* The job is already finished */
#define JOB_STOPPED	3   	/* The job is stopped */

STATIC int  	maxJobs;    	/* The most children we can run at once */
STATIC int  	maxLocal;    	/* The most local ones we can have */
int  	    	nJobs;	    	/* The number of children currently running */
int  	    	nLocal;    	/* The number of local children */
Lst  	    	jobs;		/* The structures that describe them */
Boolean	    	jobFull;    	/* Flag to tell when the job table is full. It
				 * is set TRUE when (1) the total number of
				 * running jobs equals the maximum allowed or
				 * (2) a job can only be run locally, but
				 * nLocal equals maxLocal */

/*
 * When JobStart attempts to run a job remotely but can't, and isn't allowed
 * to run the job locally, or when Job_CatchChildren detects a job that has
 * been migrated home, the job is placed on the stoppedJobs queue to be run
 * when the next job finishes. 
 */
Lst  	    stoppedJobs;	/* Lst of Job structures describing
				 * jobs that were stopped due to concurrency
				 * limits or migration home */


# if defined(USE_PGRP)
#define KILL(pid,sig)	killpg((pid),(sig))
# else
#define KILL(pid,sig)	kill((pid),(sig))
# endif

STATIC void JobRestart(Job *);
STATIC int  JobStart(GNode *, int, Job *);
STATIC void JobInterrupt(Boolean);
STATIC int  JobMakeArgv(Job *, char ***);

STATIC Lst  remoteHosts;	/* Remote hosts to use */
STATIC char *remoteBase;	/* Remote base directory */
STATIC char *remoteUser;	/* Remote directory owner */
STATIC char *remoteProg;	/* Remote program to execute */
STATIC char *remoteSubdir;	/* Subdirectory to run command in */
STATIC char *remoteEnviron;	/* File containing remote
				   environment variables */
STATIC char *remoteLastHost;	/* Last host to finish a remote job */

/*-
 *-----------------------------------------------------------------------
 * JobCondPassSig --
 *	Pass a signal to a job if the job is remote or if USE_PGRP
 *	is defined.
 *
 * Results:
 *	=== 0
 *
 * Side Effects:
 *	None, except the job may bite it.
 *
 *-----------------------------------------------------------------------
 */
STATIC int
JobCondPassSig(
    ClientData	jobCD,		/* Job to biff */
    ClientData	signo)		/* Signal to send it */
{
    Job	    	*job = (Job *)jobCD;

    if (DEBUG(JOB)) {
	printf("JobCondPassSig(job %s, signo %d)\n",
	       job->node ? job->node->name->data : "NONODE", (int)signo);
    }
    /*
     * Assume that sending the signal to job->pid will signal any remote
     * job as well.
     */
    KILL(job->pid, (int)signo);
    return(0);
}

/*-
 *-----------------------------------------------------------------------
 * JobPassSig --
 *	Pass a signal on to all remote jobs and to all local jobs if
 *	USE_PGRP is defined, then die ourselves.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	We die by the same signal.
 *	
 *-----------------------------------------------------------------------
 */
STATIC void
JobPassSig(int signo)		/* The signal number we've received */
{
    sigset_t	omask, nmask;
    
    if (DEBUG(JOB)) {
	printf("JobPassSig(signo %d)\n", signo);
    }
    Lst_ForEach(jobs, JobCondPassSig, (ClientData)signo);

    /*
     * Deal with proper cleanup based on the signal received. We only run
     * the .INTERRUPT target if the signal was in fact an interrupt. The other
     * three termination signals are more of a "get out *now*" command.
     */
    if (signo == SIGINT) {
	JobInterrupt(TRUE);
    } else if ((signo == SIGHUP) || (signo == SIGTERM) || (signo == SIGQUIT)) {
	JobInterrupt(FALSE);
    }
    
    /*
     * Leave gracefully if SIGQUIT, rather than core dumping.
     */
    if (signo == SIGQUIT) {
	Finish(errors);
    }
    
    /*
     * Send ourselves the signal now we've given the message to everyone else.
     * Note we block everything else possible while we're getting the signal.
     * This ensures that all our jobs get continued when we wake up before
     * we take any other signal.
     */
    sigfillset(&nmask);
    sigprocmask(SIG_SETMASK, &nmask, &omask);
    signal(signo, SIG_DFL);
    sigdelset(&nmask, signo);
    sigprocmask(SIG_SETMASK, &nmask, (sigset_t *)NULL);

    kill(getpid(), signo);

    Lst_ForEach(jobs, JobCondPassSig, (ClientData)SIGCONT);

    signal(signo, JobPassSig);
    sigprocmask(SIG_SETMASK, &omask, (sigset_t *)NULL);

}

/*-
 *-----------------------------------------------------------------------
 * JobCmpPid  --
 *	Compare the pid of the job with the given pid and return 0 if they
 *	are equal. This function is called from Job_CatchChildren via
 *	Lst_Find to find the job descriptor of the finished job.
 *
 * Results:
 *	0 if the pid's match
 *
 * Side Effects:
 *	None
 *-----------------------------------------------------------------------
 */
STATIC int
JobCmpPid (
    ClientData	jobCD,		/* job to examine */
    ClientData	pid)		/* process id desired */
{
    Job *job = (Job *)jobCD;

    if (DEBUG(JOB)) {
	printf("JobCmpPid(job %s, pid %d)\n",
	       job->node ? job->node->name->data : "NONODE", (int)pid);
    }
    return (job->pid != (int)pid);
}

/*-
 *-----------------------------------------------------------------------
 * JobFinish  --
 *	Do final processing for the given job including updating
 *	parents and starting new jobs as available/necessary. Note
 *	that we pay no attention to the JOB_IGNERR flag here.
 *	This is because when we're called because of a noexecute flag
 *	or something, status is 0 and when called from
 *	Job_CatchChildren, the status is zeroed if it s/b ignored.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Some nodes may be put on the toBeMade queue.
 *
 *	If we recognized an error (errors !=0), we set the aborting flag
 *	to ABORT_ERROR so no more jobs will be started.
 *-----------------------------------------------------------------------
 */
/*ARGSUSED*/
STATIC void
JobFinish (
    Job           *job,	      	  /* job to finish */
    int		  status)     	  /* sub-why job went away */
{
    Boolean 	  done;

    if (DEBUG(JOB)) {
	printf("JobFinish(job %s, status %x)\n",
	       job->node ? job->node->name->data : "NONODE", (unsigned)status);
    }
    if ((WIFEXITED(status) &&
	 WEXITSTATUS(status) != 0 && !(job->flags & JOB_IGNERR)) ||
	(WIFSIGNALED(status) && WTERMSIG(status) != SIGCONT))
    {
	/*
	 * If it exited non-zero and either we're doing things our
	 * way or we're not ignoring errors, the job is finished.
	 * Similarly, if the shell died because of a signal
	 * the job is also finished. In these
	 * cases, finish out the job's output before printing the exit
	 * status...
	 */
	fflush(stdout);

	done = TRUE;
    } else if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
	/*
	 * Deal with ignored errors in -B mode. We need to print a message
	 * telling of the ignored error as well as setting status
	 * to 0 so the next command gets run. To do this, we set done to be
	 * TRUE if in -B mode and the job exited non-zero. Note we don't
	 * want to close down any of the streams until we know we're at the
	 * end.
	 */
	done = TRUE;
    } else {
	/*
	 * No need to close things down or anything.
	 */
	done = FALSE;
    }
    
    if (done ||
	WIFSTOPPED(status) ||
	(WIFSIGNALED(status) && (WTERMSIG(status) == SIGCONT)) ||
	DEBUG(JOB))
    {
	if (WIFEXITED(status)) {
	    if (WEXITSTATUS(status) != 0) {
		printf ("*** Error code %d%s\n", WEXITSTATUS(status),
			 (job->flags & JOB_IGNERR) ? " (ignored)" : "");

		if (job->flags & JOB_IGNERR) {
		    status = W_EXITCODE(0, 0);
		}
	    } else if (DEBUG(JOB)) {
		printf ("*** Completed successfully\n");
	    }
	} else if (WIFSTOPPED(status)) {
	    printf ("*** Stopped -- signal %d\n", WSTOPSIG(status));
	    job->flags |= JOB_RESUME;
	    (void)Lst_AtEnd(stoppedJobs, (ClientData)job);
	    fflush(stdout);
	    return;
	} else if (WTERMSIG(status) == SIGCONT) {
	    /*
	     * If the beastie has continued, shift the Job from the stopped
	     * list to the running one (or re-stop it if concurrency is
	     * exceeded) and go and get another child.
	     */
	    if (job->flags & (JOB_RESUME|JOB_RESTART)) {
		printf ("*** Continued\n");
	    }
	    if (! (job->flags & JOB_CONTINUING)) {
		JobRestart(job);
	    } else {
		Lst_AtEnd(jobs, (ClientData)job);
		nJobs += 1;
		if (! (job->flags & JOB_REMOTE))
		    nLocal += 1;
		if (nJobs == maxJobs) {
		    jobFull = TRUE;
		    if (DEBUG(JOB)) {
			printf("Job queue is full.\n");
		    }
		}
	    }
	    fflush(stdout);
	    return;
	} else {
	    printf ("*** Signal %d\n", WTERMSIG(status));
	}

	fflush (stdout);
    }

    /*
     * Now handle the -B-mode stuff. If the beast still isn't finished,
     * try and restart the job on the next command. If JobStart says it's
     * ok, it's ok. If there's an error, this puppy is done.
     */
    if (status == 0 && !Lst_IsAtEnd (job->node->commands))
    {
	if (DEBUG(JOB)) {
	    printf("Starting next command\n");
	}
	switch (JobStart (job->node, 0, job)) {
	case JOB_RUNNING:
	    done = FALSE;
	    break;
	case JOB_ERROR:
	    done = TRUE;
	    status = W_EXITCODE(1, 0);
	    break;
	case JOB_FINISHED:
	    /*
	     * If we got back a JOB_FINISHED code, JobStart has already
	     * called Make_Update and freed the job descriptor. We set
	     * done to false here to avoid fake cycles and double frees.
	     * JobStart needs to do the update so we can proceed up the
	     * graph when given the -n flag..
	     */
	    done = FALSE;
	    break;
	}
    } else {
	done = TRUE;
    }
		

    if (done &&
	(aborting != ABORT_ERROR) &&
	(aborting != ABORT_INTERRUPT) &&
	(status == 0))
    {
	/*
	 * As long as we aren't aborting and the job didn't return a non-zero
	 * status that we shouldn't ignore, we call Make_Update to update
	 * the parents.
	 */
	job->node->made = MADE;
	Make_Update (job->node);
	free((Address)job);
    } else if (status) {
	job->node->made = aborting ? ABORTED : ERROR;
	errors += 1;
	free((Address)job);
    }

    while (!errors && !jobFull && !Lst_IsEmpty(stoppedJobs)) {
	JobRestart((Job *)Lst_DeQueue(stoppedJobs));
    }

    /*
     * Set aborting if any error.
     */
    if (errors && !keepgoing && (aborting != ABORT_INTERRUPT)) {
	/*
	 * If we found any errors in this batch of children and the -k flag
	 * wasn't given, we set the aborting flag so no more jobs get
	 * started.
	 */
	aborting = ABORT_ERROR;
    }
}

/*-
 *-----------------------------------------------------------------------
 * Job_Touch --
 *	Touch the given target. Called by JobStart when the -t flag was
 *	given
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	The data modification of the file is changed. In addition, if the
 *	file did not exist, it is created.
 *-----------------------------------------------------------------------
 */
void
Job_Touch (
    GNode         *gn,	      	/* the node of the file to touch */
    Boolean 	  silent)   	/* TRUE if should not print messages */
{
    int		  streamID;   	/* ID of stream opened to do the touch */

    if (DEBUG(JOB)) {
	printf("Job_Touch(gn %s, silent %d)\n", gn->name->data, silent);
    }
    if (gn->type & (OP_JOIN|OP_USE|OP_EXEC|OP_OPTIONAL)) {
	/*
	 * .JOIN, .USE, .ZEROTIME and .OPTIONAL targets are "virtual" targets
	 * and, as such, shouldn't really be created.
	 */
	return;
    }
    
    if (!silent) {
	printf ("touch %s\n", gn->name->data);
    }

    if (noExecute) {
	return;
    }

    if (gn->type & OP_ARCHV) {
	Arch_Touch (gn);
    } else if (gn->type & OP_LIB) {
	Arch_TouchLib (gn);
    } else {
	struct utimbuf times;	/* Times for utime() call */
	const char *file = gn->path ? gn->path->data : gn->name->data;

	times.actime = times.modtime = now;
	if (utime(file, &times) < 0) {
	    if (errno == ENOENT) {
		streamID = open (file, O_RDWR|O_EXCL|O_CREAT, 0666);
		if (streamID >= 0)
		    (void)close (streamID);
		else
		    printf("*** couldn't touch %s: %s", file, strerror(errno));
	    } else
		printf("*** couldn't touch %s: %s", file, strerror(errno));
	}
    }
}

/*-
 *-----------------------------------------------------------------------
 * Job_CheckCommands --
 *	Make sure the given node has all the commands it needs. 
 *
 * Results:
 *	TRUE if the commands list is/was ok.
 *
 * Side Effects:
 *	The node will have commands from the .DEFAULT rule added to it
 *	if it needs them.
 *-----------------------------------------------------------------------
 */
Boolean
Job_CheckCommands (
    GNode          *gn,		/* The target whose commands need verifying */
    void    	  (*abortProc)(const char *, ...))
				/* Function to abort with msg */
{
    if (DEBUG(JOB)) {
	printf("Job_CheckCommands(gn %s)\n", gn->name->data);
    }
    if (OP_NOP(gn->type) && Lst_IsEmpty (gn->commands) &&
	(gn->type & OP_LIB) == 0) {
	/*
	 * No commands. Look for .DEFAULT rule from which we might infer
	 * commands 
	 */
	if ((DEFAULT != NILGNODE) && !Lst_IsEmpty(DEFAULT->commands)) {
	    /*
	     * Make only looks for a .DEFAULT if the node was never the
	     * target of an operator, so that's what we do too. If
	     * a .DEFAULT was given, we substitute its commands for gn's
	     * commands and set the IMPSRC variable to be the target's name
	     * The DEFAULT node acts like a transformation rule, in that
	     * gn also inherits any attributes or sources attached to
	     * .DEFAULT itself.
	     */
	    Make_HandleUse(DEFAULT, gn);
	    Var_Set(sIMPSRC, Var_StrValue(sTARGET, gn), gn);
	} else if (Dir_MTime (gn) == 0) {
	    /*
	     * The node wasn't the target of an operator we have no .DEFAULT
	     * rule to go on and the target doesn't already exist. There's
	     * nothing more we can do for this branch. If the -k flag wasn't
	     * given, we stop in our tracks, otherwise we just don't update
	     * this node's parents so they never get examined. 
	     */
	    if (gn->type & OP_OPTIONAL) {
		printf ("make: don't know how to make %s (ignored)\n",
			gn->name->data);
	    } else if (keepgoing) {
		printf ("make: don't know how to make %s (continuing)\n",
			gn->name->data);
		return (FALSE);
	    } else {
		(*abortProc) ("make: don't know how to make %s. Stop",
			     gn->name->data);
		return(FALSE);
	    }
	}
    }
    return (TRUE);
}

/*-
 *-----------------------------------------------------------------------
 * JobExec --
 *	Execute the shell for the given job. Called from JobStart and
 *	JobRestart.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	A shell is executed, outputs is altered and the Job structure added
 *	to the job table.
 *
 *-----------------------------------------------------------------------
 */
STATIC void
JobExec(Job *job)		/* Job to execute */
{
    int	    	  cpid;	    	/* ID of new child */
    char    	  **argv;
    char	  *emsg;
    
    if (DEBUG(JOB)) {
	printf("JobExec(job %s)\n",
	       job->node ? job->node->name->data : "NONODE");
	printf("Running %s %sly\n", job->node->name->data,
	       job->flags&JOB_REMOTE?"remote":"local");
    }
    if (!JobMakeArgv(job, &argv)) {
	JobFinish(job, 0);
	return;
    }
    if (DEBUG(JOB)) {
	int 	  i;
	
	printf("\tCommand:");
	for (i = 1; argv[i] != (char *)NULL; i++) {
	    printf(" %s", argv[i]);
	}
	printf("\n");
    }
    
    if (job->flags & JOB_REMOTE) {
	if (remoteLastHost != NULL) {
	    job->rmtData = (void *)remoteLastHost;
	    remoteLastHost = NULL;
	} else {
	    LstNode ln;

	    while ((ln = Lst_Next(remoteHosts)) == NILLNODE)
		Lst_Open(remoteHosts);
	    job->rmtData = (void *)Lst_Datum(ln);
	}
	if (DEBUG(JOB)) {
	    printf("Starting job on remote host '%s'\n", (char *)job->rmtData);
	}
    }

    if ((cpid =  fork()) == -1) {
	Punt ("Cannot fork");
    } else if (cpid == 0) {
	if (job->flags & JOB_REMOTE) {
	    setenv("AUTHCOVER_HOST", (char *)job->rmtData, TRUE);
	    setenv("AUTHCOVER_TESTDIR", remoteBase, TRUE);
	    setenv("AUTHCOVER_USER", remoteUser, TRUE);
	}
	(void) execvp (argv[0], &argv[1]);
	emsg = strerror (errno);
	(void) write (2, "make: ", 6);
	(void) write (2, argv[0], strlen(argv[0]));
	(void) write (2, ": ", 2);
	(void) write (2, emsg, strlen(emsg));
	(void) write (2, "\n", 1);
	_exit (1);
    }
    job->pid = cpid;
    if (DEBUG(JOB)) {
	printf("JobExec: job %s, pid %d\n",
	       job->node ? job->node->name->data : "NONODE", cpid);
    }

    /*
     * Now the job is actually running, add it to the table.
     */
    nJobs += 1;
    if (! (job->flags & JOB_REMOTE))
	nLocal += 1;
    (void)Lst_AtEnd (jobs, (ClientData)job);
    if (nJobs == maxJobs)
	jobFull = TRUE;
    if (job->node->type & OP_MAKE) {
	while (nJobs)
	    Job_CatchChildren();
    }
}

/*-
 *-----------------------------------------------------------------------
 * JobMakeArgv --
 *	Create the argv needed to execute the shell for a given job.
 *	
 *
 * Results:
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
STATIC int
JobMakeArgv(Job *job, char ***argvp)
{
    const char	**argv;
    const char	*cmd;
    LstNode	cmdNode;
    Boolean 	silent,   	/* Don't print command */
		errCheck; 	/* Check errors */
    string_t	scmd;
    int		argmax;
    int		argc;

    if (DEBUG(JOB)) {
	printf("JobMakeArgv(job %s)\n",
	       job->node ? job->node->name->data : "NONODE");
    }
    if (job->flags & JOB_FIRST)
	Lst_Open(job->node->commands);
    cmdNode = Lst_Next(job->node->commands);
    if (cmdNode == NILLNODE) {
	Lst_Close(job->node->commands);
	return(FALSE);
    }
    scmd = (string_t)Lst_Datum(cmdNode);
    if (string_hasvar(scmd))
	cmd = Var_Subst(scmd->data, job->node, FALSE);
    else
	cmd = scmd->data;
    if (*cmd == 0)
	return(FALSE);

    while (*cmd == '@' || *cmd == '-') {
	if (*cmd == '@')
	    job->flags |= JOB_SILENT;
	else
	    job->flags |= JOB_IGNERR;
	cmd++;
    }
    while (*cmd == ' ' || *cmd == '\t')
	cmd++;
    if (*cmd == 0)
	return(FALSE);

    silent = (job->flags & JOB_SILENT);
    errCheck = !(job->flags & JOB_IGNERR);

    /*
     * Print the command before echoing if we're not supposed to be quiet for
     * this one. We also print the command if -n given.
     */
    if (!silent || noExecute) {
	printf ("%s\n", cmd);
	fflush(stdout);
    }

    /*
     * If we're not supposed to execute any commands, this is as far as
     * we go...
     */
    if (noExecute)
	return(FALSE);
    
    argmax = 32;
    argv = (const char **)emalloc(argmax * sizeof(char *));
    argc = 1;

    if (job->flags & JOB_REMOTE) {
	argv[0] = remoteProg;
	argv[argc++] = "rmt-make";
	argv[argc++] = "-t1";
	argv[argc++] = "2";
	argv[argc++] = "exec";
	argv[argc++] = remoteSubdir;
	argv[argc++] = remoteEnviron;
    }

    if (Var_HasMeta(cmd)) {
	/*
	 * The command contains a shell "meta" character and we therefore
	 * need to pass the command off to the shell. We give the shell the
	 * -e flag as well as -c if it's supposed to exit when it hits an
	 * error.
	 */
	if (argc == 1) {
	    argv[0] = "/usr/bin/sh";
	    argv[argc++] = "sh";
	} else
	    argv[argc++] = "/usr/bin/sh";
	argv[argc++] = (errCheck ? "-ec" : "-c");
	argv[argc++] = cmd;
    } else {
	/*
	 * No meta-characters, so no need to exec a shell. Break the command
	 * into words to form an argument vector we can execute.
	 */
	char *buf;
	register int ch;
	register char inquote, *start, *t;
	const char *p;
	Boolean done;

	/* allocate room for a copy of the string */
	buf = strdup(cmd);

	/*
	 * copy the string; at the same time, parse backslashes,
	 * quotes and build the argument list.
	 */
	inquote = 0;
	done = FALSE;
	for (p = cmd, start = t = buf; !done; ++p) {
	    switch(ch = *p) {
	    case '"':
	    case '\'':
		if (inquote)
		    if (inquote == ch)
			inquote = 0;
		    else
			break;
		else
		    inquote = ch;
		continue;
	    case ' ':
	    case '\t':
		if (inquote)
		    break;
		if (!start)
		    continue;
		/* FALLTHROUGH */
	    case '\n':
	    case '\0':
		/*
		 * end of a token -- make sure there's enough argv
		 * space and save off a pointer.
		 */
		*t++ = '\0';
		if (argc == argmax) {
		    argmax <<= 1;		/* ramp up fast */
		    argv = (const char **)realloc(argv,
						  argmax * sizeof(char *));
		    if (argv == 0)
			enomem();
		}
		argv[argc++] = start;
		start = (char *)NULL;
		if (ch == '\n' || ch == '\0')
		    done = TRUE;
		continue;
	    case '\\':
		switch (ch = *++p) {
		case '\0':
		case '\n':
		    /* hmmm; fix it up as best we can */
		    ch = '\\';
		    --p;
		    break;
		}
		break;
	    }
	    if (!start)
		start = t;
	    *t++ = ch;
	}
	if (!(job->flags & JOB_REMOTE))
	    argv[0] = argv[1];
    }
    argv[argc] = (const char *)NULL;
    *argvp = (char **)argv;
    return(TRUE);
}

/*-
 *-----------------------------------------------------------------------
 * JobRestart --
 *	Restart a job that stopped for some reason. 
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	jobFull will be set if the job couldn't be run.
 *
 *-----------------------------------------------------------------------
 */
STATIC void
JobRestart(Job *job)    	/* Job to restart */
{
    Boolean error;
    int status;

    if (DEBUG(JOB)) {
	printf("JobRestart(job %s)\n",
	       job->node ? job->node->name->data : "NONODE");
    }
    if (job->flags & JOB_RESTART) {
	if (DEBUG(JOB)) {
	    printf("Restarting %s...", job->node->name->data);
	}
	if (((nLocal >= maxLocal) && ! (job->flags & JOB_SPECIAL))) {
	    if (nJobs >= maxJobs) {
		/*
		 * Can't be exported and not allowed to run locally -- put it
		 * back on the hold queue and mark the table full
		 */
		if (DEBUG(JOB)) {
		    printf("holding\n");
		}
		(void)Lst_AtFront(stoppedJobs, (ClientData)job);
		jobFull = TRUE;
		if (DEBUG(JOB)) {
		    printf("Job queue is full.\n");
		}
		return;
	    }
	    /*
	     * Job may be run remotely.
	     */
	    if (DEBUG(JOB)) {
		printf("running remotely\n");
	    }
	    job->flags |= JOB_REMOTE;
	} else {
	    /*
	     * Job may be run locally.
	     */
	    if (DEBUG(JOB)) {
		printf("running locally\n");
	    }
	    job->flags &= ~JOB_REMOTE;
	}
	JobExec(job);
	return;
    }
    /*
     * The job has stopped and needs to be restarted. Why it stopped,
     * we don't know...
     */
    if (DEBUG(JOB)) {
	printf("Resuming %s...", job->node->name->data);
    }
    if (nJobs >= maxJobs ||
	((job->flags & JOB_REMOTE) == 0 &&
	 nLocal >= maxLocal &&
	 ((job->flags & JOB_SPECIAL) == 0 || maxLocal != 0)))
    {
	/*
	 * Job cannot be restarted. Mark the table as full and
	 * place the job back on the list of stopped jobs.
	 */
	if (DEBUG(JOB)) {
	    printf("table full\n");
	}
	(void)Lst_AtFront(stoppedJobs, (ClientData)job);
	jobFull = TRUE;
	if (DEBUG(JOB)) {
	    printf("Job queue is full.\n");
	}
	return;
    }
    /*
     * If the job is remote, it's ok to resume it as long as the
     * maximum concurrency won't be exceeded. If it's local and
     * we haven't reached the local concurrency limit already (or the
     * job must be run locally and maxLocal is 0), it's also ok to
     * resume it.
     */
    error = (KILL(job->pid, SIGCONT) != 0);
    if (error) {
	Error("couldn't resume %s: %s",
	    job->node->name, strerror(errno));
	status = W_EXITCODE(1, 0);
	JobFinish(job, status);
    }
    /*
     * Make sure the user knows we've continued the beast and
     * actually put the thing in the job table.
     */
    job->flags |= JOB_CONTINUING;
    status = W_EXITCODE(0, SIGCONT);
    JobFinish(job, status);

    job->flags &= ~(JOB_RESUME|JOB_CONTINUING);
    if (DEBUG(JOB)) {
	printf("done\n");
    }
}

/*-
 *-----------------------------------------------------------------------
 * JobStart  --
 *	Start a target-creation process going for the target described
 *	by the graph node gn. 
 *
 * Results:
 *	JOB_ERROR if there was an error in the commands, JOB_FINISHED
 *	if there isn't actually anything left to do for the job and
 *	JOB_RUNNING if the job has been started.
 *
 * Side Effects:
 *	A new Job node is created and added to the list of running
 *	jobs. PMake is forked and a child shell created.
 *-----------------------------------------------------------------------
 */
STATIC int
JobStart (
    GNode         *gn,	      /* target to create */
    int		  flags,      /* flags for the job to override normal ones.
			       * e.g. JOB_SPECIAL */
    Job 	  *previous)  /* The previous Job structure for this node,
			       * if any. */
{
    register Job  *job;       /* new job descriptor */
    Boolean	  cmdsOK;     /* true if the nodes commands were all right */
    Boolean 	  noExec;     /* Set true if we decide not to run the job */

    if (DEBUG(JOB)) {
	printf("JobStart(gn %s, flags %x, previous %s)\n",
	       gn->name->data, (unsigned)flags,
	       previous ? previous->node->name->data : "no previous");
    }
    if (previous != (Job *)NULL) {
	previous->flags &= ~ (JOB_FIRST|JOB_IGNERR|JOB_SILENT|JOB_REMOTE);
	job = previous;
    } else {
	job = (Job *) emalloc (sizeof (Job));
	if (job == (Job *)NULL) {
	    Punt("JobStart out of memory");
	}
	flags |= JOB_FIRST;
    }

    job->node = gn;
    job->rmtData = NULL;

    /*
     * Set the initial value of the flags for this job based on the global
     * ones and the node's attributes... Any flags supplied by the caller
     * are also added to the field.
     */
    job->flags = 0;
    if (Targ_Ignore (gn)) {
	job->flags |= JOB_IGNERR;
    }
    if (Targ_Silent (gn)) {
	job->flags |= JOB_SILENT;
    }
    job->flags |= flags;

    /*
     * Check the commands now so any attributes from .DEFAULT have a chance
     * to migrate to the node
     */
    if (job->flags & JOB_FIRST) {
	cmdsOK = Job_CheckCommands(gn, Error);
    } else {
	cmdsOK = TRUE;
    }

    if (!cmdsOK && keepgoing) {
      free((Address)job);
      return(JOB_ERROR);
    }

    
    if (touchFlag && (gn->type & OP_MAKE) == 0) {
	/*
	 * Just touch the target and note that no shell should be executed.
	 * Check the commands, too, but don't die if they're no good -- it
	 * does no harm to keep working up the graph.
	 */
    	Job_Touch (gn, job->flags&JOB_SILENT);
	noExec = TRUE;
    } else {
	/*
	 * We're serious here, but if the commands were bogus, we're
	 * also dead...
	 */
	if (!cmdsOK && !noExecute) {
	    DieHorribly();
	}

	noExec = FALSE;
    }

    /*
     * If we're not supposed to execute a shell, don't. 
     */
    if (noExec) {
	/*
	 * Unlink and close the command file if we opened one
	 */
	fflush (stdout);

	/*
	 * We only want to work our way up the graph if we aren't here because
	 * the commands for the job were no good.
	 */
	if (cmdsOK) {
	    if (aborting == 0)
		Make_Update(job->node);
	    free((Address)job);
	    return(JOB_FINISHED);
	} else {
	    free((Address)job);
	    return(JOB_ERROR);
	}
    }

    /*
     * If we're using pipes to catch output, create the pipe by which we'll
     * get the shell's output. If we're using files, print out that we're
     * starting a job and then set up its temporary-file name. This is just
     * tfile with two extra digits tacked on -- jobno.
     */
    if (job->flags & JOB_FIRST) {
	if (noisy)
	    printf ("Building `%s'\n", gn->name->data);
	fflush (stdout);
    }

    if (nJobs >= maxJobs && (!(job->flags & JOB_SPECIAL) || maxLocal != 0))
    {
	/*
	 * The job can only be run locally, but we've hit the limit of
	 * local concurrency, so put the job on hold until some other job
	 * finishes. Note that the special jobs (.BEGIN, .INTERRUPT and .END)
	 * may be run locally even when the local limit has been reached
	 * (e.g. when maxLocal == 0), though they will be exported if at
	 * all possible. 
	 */
	jobFull = TRUE;
	
	if (DEBUG(JOB)) {
	    printf("Can only run job locally.\n");
	}
	job->flags |= JOB_RESTART;
	(void)Lst_AtEnd(stoppedJobs, (ClientData)job);
	return(JOB_RUNNING);
    }
    if (nJobs >= maxJobs) {
	/*
	 * If we're running this job locally as a special case (see above),
	 * at least say the table is full.
	 */
	jobFull = TRUE;
	if (DEBUG(JOB)) {
	    printf("Local job queue is full.\n");
	}
    }
    if (job->node->type & OP_MAKE) {
	job->flags |= JOB_SPECIAL;
	while (nJobs)
	    Job_CatchChildren();
    } else if (nJobs < maxJobs && nLocal >= maxLocal)
	job->flags |= JOB_REMOTE;
	
    JobExec(job);
    return(JOB_RUNNING);
}

/*-
 *-----------------------------------------------------------------------
 * Job_CatchChildren --
 *	Handle the exit of a child. Called from Make_Make.
 *
 * Results:
 *	none.
 *
 * Side Effects:
 *	The job descriptor is removed from the list of children.
 *
 * Notes:
 *	We do waits, blocking or not, according to the wisdom of our
 *	caller, until there are no more children to report. For each
 *	job, call JobFinish to finish things off. This will take care of
 *	putting jobs on the stoppedJobs queue.
 *
 *-----------------------------------------------------------------------
 */
void
Job_CatchChildren (void)
{
    int    	  pid;	    	/* pid of dead child */
    register Job  *job;	    	/* job descriptor for dead child */
    LstNode       jnode;    	/* list element for finding job */
    int		  status;   	/* Exit/termination status */

    if (DEBUG(JOB)) {
	printf("Job_CatchChildren() nLocal %d nJobs %d\n", nLocal, nJobs);
    }
    
    errno = 0;
    while ((pid = waitpid(WAIT_ANY, &status, WUNTRACED)) > 0)
    {
	if (DEBUG(JOB))
	    printf("Process %d exited or stopped.\n", pid);
	    

	jnode = Lst_Find (jobs, (ClientData)pid, JobCmpPid);

	if (jnode == NILLNODE) {
	    if (WIFSIGNALED(status) && (WTERMSIG(status) == SIGCONT)) {
		jnode = Lst_Find(stoppedJobs, (ClientData)pid, JobCmpPid);
		if (jnode == NILLNODE) {
		    Error("Resumed child (%d) not in table", pid);
		    errno = 0;
		    continue;
		}
		job = (Job *)Lst_Datum(jnode);
		Lst_Remove(stoppedJobs, jnode);
	    } else {
		Error ("Child (%d) not in table?", pid);
		errno = 0;
		continue;
	    }
	} else {
	    job = (Job *) Lst_Datum (jnode);
	    Lst_Remove (jobs, jnode);
	    remoteLastHost = (char *)job->rmtData;
	    nJobs -= 1;
	    if (! (job->flags & JOB_REMOTE))
		nLocal -= 1;
	    if (jobFull && DEBUG(JOB)) {
		printf("Job queue is no longer full.\n");
	    }
	    jobFull = FALSE;
	}

	JobFinish (job, status);
	errno = 0;
    }
    if (DEBUG(JOB)) {
	printf("Job_CatchChildren: pid %d, errno %d\n", pid, errno);
    }
}

/*-
 *-----------------------------------------------------------------------
 * Job_Make --
 *	Start the creation of a target. Basically a front-end for
 *	JobStart used by the Make module.
 *
 * Results:
 *	TRUE if we could start a job, FALSE is we are not longer
 *	running jobs.
 *
 * Side Effects:
 *	Another job is started.
 *
 *-----------------------------------------------------------------------
 */
Boolean
Job_Make (GNode *gn)
{
    int status;

    if (DEBUG(JOB)) {
	printf("Job_Make(gn %s)\n", gn->name->data);
    }
    status = JobStart (gn, 0, (Job *)NULL);
    if (DEBUG(JOB)) {
	printf("Job_Make: JobStart returned %d\n", status);
    }
    return(status != JOB_ERROR && !aborting);
}

/*-
 *-----------------------------------------------------------------------
 * Job_Init --
 *	Initialize the process module
 *
 * Results:
 *	none
 *
 * Side Effects:
 *	lists and counters are initialized
 *-----------------------------------------------------------------------
 */
void
Job_Init (
    int           maxproc,  /* the greatest number of jobs which may be
			     * running at one time */
    int	    	  maxlocal) /* the greatest number of local jobs which may
			     * be running at once. */
{
    if (DEBUG(JOB)) {
	printf("Job_Init(maxproc %d, maxlocal %d)\n", maxproc, maxlocal);
    }
    jobs =  	  Lst_Init();
    stoppedJobs = Lst_Init();
    maxJobs = 	  maxproc;
    maxLocal = 	  maxlocal;
    nJobs = 	  0;
    nLocal = 	  0;
    jobFull = 	  FALSE;

    aborting = 	  0;
    errors = 	  0;

    /*
     * Catch the four signals that POSIX specifies if they aren't ignored.
     * JobPassSig will take care of calling JobInterrupt if appropriate.
     */
    if (signal (SIGINT, SIG_IGN) != SIG_IGN) {
	signal (SIGINT, JobPassSig);
    }
    if (signal (SIGHUP, SIG_IGN) != SIG_IGN) {
	signal (SIGHUP, JobPassSig);
    }
    if (signal (SIGQUIT, SIG_IGN) != SIG_IGN) {
	signal (SIGQUIT, JobPassSig);
    }
    if (signal (SIGTERM, SIG_IGN) != SIG_IGN) {
	signal (SIGTERM, JobPassSig);
    }
    /*
     * There are additional signals that need to be caught and passed if
     * either the export system wants to be told directly of signals or if
     * we're giving each job its own process group (since then it won't get
     * signals from the terminal driver as we own the terminal)
     */
#if defined(USE_PGRP)
    if (signal (SIGTSTP, SIG_IGN) != SIG_IGN) {
	signal (SIGTSTP, JobPassSig);
    }
    if (signal (SIGTTOU, SIG_IGN) != SIG_IGN) {
	signal (SIGTTOU, JobPassSig);
    }
    if (signal (SIGTTIN, SIG_IGN) != SIG_IGN) {
	signal (SIGTTIN, JobPassSig);
    }
    if (signal (SIGWINCH, SIG_IGN) != SIG_IGN) {
	signal (SIGWINCH, JobPassSig);
    }
#endif
    
    {
	GNode *begin = Targ_FindNode (s_BEGIN, TARG_NOCREATE);

	if (begin != NILGNODE) {
	    Suff_FindDeps (begin);
	    JobStart (begin, JOB_SPECIAL, (Job *)0);
	    while (nJobs) {
		Job_CatchChildren ();
	    }
	}
    }
    endNode = Targ_FindNode (s_END, TARG_NOCREATE);
    if (endNode != NILGNODE)
	Suff_FindDeps (endNode);
    intrNode = Targ_FindNode (s_INTERRUPT, TARG_NOCREATE);
    if (intrNode != NILGNODE)
	Suff_FindDeps (intrNode);
    errNode = Targ_FindNode (s_ERROR, TARG_NOCREATE);
    if (errNode != NILGNODE)
	Suff_FindDeps (errNode);
    exitNode = Targ_FindNode (s_EXIT, TARG_NOCREATE);
    if (exitNode != NILGNODE)
	Suff_FindDeps (exitNode);

    /*
     * See if we have remote setup to perform
     */
    if (maxJobs == maxLocal)
	return;

    {
	char *q;
	char *p = getenv("MAKERMTHOSTS");
	if (p == NULL)
	    Fatal("make: no MAKERMTHOSTS defined\n");
	remoteHosts = Lst_Init();
	p = strdup(p);
	for (;;) {
	    q = strchr(p, ':');
	    if (q == NULL) {
		if (*p)
		    Lst_AtEnd(remoteHosts, (ClientData)p);
		break;
	    }
	    *q++ = '\0';
	    if (*p)
		Lst_AtEnd(remoteHosts, (ClientData)p);
	    p = q;
	}
	Lst_Open(remoteHosts);
    }

    remoteBase = getenv("MAKERMTBASE");
    if (remoteBase == NULL)
	Fatal("make: no MAKERMTBASE defined\n");

    remoteUser = getenv("MAKERMTUSER");
    if (remoteUser == NULL)
	Fatal("make: no MAKERMTUSER defined\n");

    remoteProg = getenv("MAKERMTPROG");
    if (remoteProg == NULL)
	Fatal("make: no MAKERMTPROG defined\n");
    if (DEBUG(JOB)) {
	printf("MAKERMTPROG: %s\n", remoteProg);
    }
    {
	string_t str = Var_StrValue(sMAKESUB, VAR_GLOBAL);
	if (str == sNULL)
	    str = sDOT;
	else
	    str = string_head(str);
	string_ref(str);
	remoteSubdir = (char *)str->data;
	if (DEBUG(JOB)) {
	    printf("MAKERMTDIR: %s\n", remoteSubdir);
	}
    }
    {
	FILE *fp;
	char *p;

	remoteEnviron = (char *)tempnam(NULL, "makeenv");
	fp = fopen(remoteEnviron, "w");
	p = getenv("MAKERMTENV");
	if (p == NULL) {
	    char **pp;
	    for (pp = environ; *pp != NULL; pp++)
		fprintf(fp, "%s\n", *pp);
	} else {
	    char *cp, *q, *v;
	    cp = p = strdup(p);
	    for (;;) {
		q = strchr(p, ':');
		if (q == NULL) {
		    if (*p && (v = getenv(p)) != NULL)
			fprintf(fp, "%s=%s\n", p, v);
		    break;
		}
		*q++ = '\0';
		if (*p && (v = getenv(p)) != NULL)
		    fprintf(fp, "%s=%s\n", p, v);
		p = q;
	    }
	    free(cp);
	}
	fclose(fp);
    }
}

/*-
 *-----------------------------------------------------------------------
 * Job_Full --
 *	See if the job table is full. It is considered full if it is OR
 *	if we are in the process of aborting OR if we have
 *	reached/exceeded our local quota. This prevents any more jobs
 *	from starting up.
 *
 * Results:
 *	TRUE if the job table is full, FALSE otherwise
 * Side Effects:
 *	None.
 *-----------------------------------------------------------------------
 */
Boolean
Job_Full (void)
{
    if (DEBUG(JOB)) {
	printf("Job_Full() aborting %d jobFull %d\n", aborting, jobFull);
    }
    return (aborting || jobFull);
}

/*-
 *-----------------------------------------------------------------------
 * Job_Empty --
 *	See if the job table is empty.  Because the local concurrency may
 *	be set to 0, it is possible for the job table to become empty,
 *	while the list of stoppedJobs remains non-empty. In such a case,
 *	we want to restart as many jobs as we can.
 *
 * Results:
 *	TRUE if it is. FALSE if it ain't.
 *
 * Side Effects:
 *	None.
 *
 * -----------------------------------------------------------------------
 */
Boolean
Job_Empty (void)
{
    if (DEBUG(JOB)) {
	printf("Job_Empty() nJobs %d aborting %d\n", nJobs, aborting);
    }
    if (nJobs == 0) {
	if (!Lst_IsEmpty(stoppedJobs) && !aborting) {
	    /*
	     * The job table is obviously not full if it has no jobs in
	     * it...Try and restart the stopped jobs.
	     */
	    jobFull = FALSE;
	    while (!jobFull && !Lst_IsEmpty(stoppedJobs)) {
		JobRestart((Job *)Lst_DeQueue(stoppedJobs));
	    }
	    return(FALSE);
	} else {
	    return(TRUE);
	}
    } else {
	return(FALSE);
    }
}

/*-
 *-----------------------------------------------------------------------
 * JobInterrupt --
 *	Handle the receipt of an interrupt.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	All children are killed. Another job will be started if the
 *	.INTERRUPT target was given.
 *-----------------------------------------------------------------------
 */
STATIC void
JobInterrupt (int runINTERRUPT)	/* Non-zero if commands for the .INTERRUPT
				 * target should be executed */
{
    LstNode 	  ln;		/* element in job table */
    Job           *job;	    	/* job descriptor in that element */
    
    if (DEBUG(JOB)) {
	printf("JobInterrupt(runINTERRUPT %d)\n", runINTERRUPT);
    }
    aborting = ABORT_INTERRUPT;

    Lst_Open (jobs);
    while ((ln = Lst_Next (jobs)) != NILLNODE) {
	job = (Job *) Lst_Datum (ln);

	if (!Targ_Precious (job->node)) {
	    struct stat st;
	    const char *file = (job->node->path == (string_t)NULL ?
				job->node->name->data :
				job->node->path->data);
		/* unlink any target that is not a directory */
	    if ((lstat(file, &st) == 0) && ((st.st_mode&S_IFMT) != S_IFDIR)
		&& (unlink (file) == 0)) {
		Error ("*** %s removed", file);
	    }
	}
	if (job->pid) {
	    KILL(job->pid, SIGINT);
	}
    }
    Lst_Close (jobs);

    if (runINTERRUPT && !touchFlag) {
	if (intrNode != NILGNODE) {
	    ignoreErrors = FALSE;
	    JobStart (intrNode, JOB_SPECIAL, (Job *)0);
	    while (nJobs) {
		Job_CatchChildren ();
	    }
	}
	if (exitNode != NILGNODE) {
	    ignoreErrors = FALSE;
	    JobStart (exitNode, JOB_SPECIAL, (Job *)0);
	    while (nJobs) {
		Job_CatchChildren ();
	    }
	}
    }
    exit (0);
}

/*
 *-----------------------------------------------------------------------
 * Job_End --
 *	Do final processing such as the running of the commands
 *	attached to the .END target. 
 *
 * Results:
 *	Number of errors reported.
 *
 * Side Effects:
 *	The process' temporary file (tfile) is removed if it still
 *	existed.
 *-----------------------------------------------------------------------
 */
int
Job_End (void)
{
    if (DEBUG(JOB)) {
	printf("Job_End()\n");
    }
    if (errors == 0) {
	if (endNode != NILGNODE) {
	    JobStart (endNode, JOB_SPECIAL, (Job *)0);
	    while (nJobs) {
		Job_CatchChildren ();
	    }
	}
    } else {
	if (errNode != NILGNODE) {
	    JobStart (errNode, JOB_SPECIAL, (Job *)0);
	    while (nJobs) {
		Job_CatchChildren ();
	    }
	}
    }
    if (exitNode != NILGNODE) {
	JobStart (exitNode, JOB_SPECIAL, (Job *)0);
	while (nJobs) {
	    Job_CatchChildren ();
	}
    }
    if (remoteEnviron != NULL) {
	(void) unlink(remoteEnviron);
	free(remoteEnviron);
	remoteEnviron = NULL;
    }
    return(errors);
}

/*-
 *-----------------------------------------------------------------------
 * Job_Wait --
 *	Waits for all running jobs to finish and returns. Sets 'aborting'
 *	to ABORT_WAIT to prevent other jobs from starting.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Currently running jobs finish.
 *
 *-----------------------------------------------------------------------
 */
void
Job_Wait(void)
{
    int old_aborting = aborting;

    if (DEBUG(JOB)) {
	printf("Job_Wait() (aborting %d)\n", aborting);
    }
    aborting = ABORT_WAIT;
    while (nJobs != 0) {
	Job_CatchChildren();
    }
    aborting = old_aborting;
}

/*-
 *-----------------------------------------------------------------------
 * Job_AbortAll --
 *	Abort all currently running jobs without handling output or anything.
 *	This function is to be called only in the event of a major
 *	error. Most definitely NOT to be called from JobInterrupt.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	All children are killed, not just the firstborn
 *-----------------------------------------------------------------------
 */
void
Job_AbortAll (void)
{
    LstNode           	ln;		/* element in job table */
    Job            	*job;	/* the job descriptor in that element */
    int     	  	foo;
    
    if (DEBUG(JOB)) {
	printf("Job_AbortAll()\n");
    }
    aborting = ABORT_ERROR;
    
    if (nJobs) {

	Lst_Open (jobs);
	while ((ln = Lst_Next (jobs)) != NILLNODE) {
	    job = (Job *) Lst_Datum (ln);

	    /*
	     * kill the child process with increasingly drastic signals to make
	     * darn sure it's dead. 
	     */
	    KILL(job->pid, SIGINT);
	    KILL(job->pid, SIGKILL);
	}
    }
    
    /*
     * Catch as many children as want to report in at first, then give up
     */
    while (waitpid(WAIT_ANY, &foo, WNOHANG) > 0) {
	;
    }
}
