/* @(#)41       1.3  src/bldenv/make/job.h, bldprocess, bos412, GOLDA411a 1/19/94 16:27:41
 *
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: none
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
 * $Log: job.h,v $
 * Revision 1.2.2.3  1992/12/03  19:05:32  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:35:21  damon]
 *
 * Revision 1.2.2.2  1992/09/24  19:24:23  gm
 * 	CR286: Major improvements to make internals.
 * 	[1992/09/24  17:54:30  gm]
 * 
 * Revision 1.2  1991/12/05  20:42:52  devrcs
 * 	Changes for parallel make.
 * 	[91/04/21  16:37:13  gm]
 * 
 * 	Changes for Reno make
 * 	[91/03/22  16:00:45  mckeen]
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
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)job.h	5.3 (Berkeley) 6/1/90
 */

/*-
 * job.h --
 *	Definitions pertaining to the running of jobs in parallel mode.
 *	Exported from job.c for the use of remote-execution modules.
 */
#ifndef _JOB_H_
#define _JOB_H_

/*-
 * Job Table definitions. 
 *
 * Each job has several things associated with it:
 *	1) The process id of the child shell
 *	2) The graph node describing the target being made by this job
 *	3) A LstNode for the first command to be saved after the job
 *	   completes. This is NILLNODE if there was no "..." in the job's
 *	   commands.
 *	4) An FILE* for writing out the commands. This is only
 *	   used before the job is actually started.
 *	5) A union of things used for handling the shell's output. Different
 *	   parts of the union are used based on the value of the usePipes
 *	   flag. If it is true, the output is being caught via a pipe and
 *	   the descriptors of our pipe, an array in which output is line
 *	   buffered and the current position in that buffer are all
 *	   maintained for each job. If, on the other hand, usePipes is false,
 *	   the output is routed to a temporary file and all that is kept
 *	   is the name of the file and the descriptor open to the file.
 *	6) An identifier provided by and for the exclusive use of the
 *	   Rmt module.
 *	7) A word of flags which determine how the module handles errors,
 *	   echoing, etc. for the job 
 *
 * The job "table" is kept as a linked Lst in 'jobs', with the number of
 * active jobs maintained in the 'nJobs' variable. At no time will this
 * exceed the value of 'maxJobs', initialized by the Job_Init function. 
 *
 * When a job is finished, the Make_Update function is called on each of the
 * parents of the node which was just remade. This takes care of the upward
 * traversal of the dependency graph.
 */
#define JOB_BUFSIZE	1024
typedef struct Job {
    int       	pid;	    /* The child's process ID */
    GNode    	*node;      /* The target the child is making */
    void    	*rmtData;   /* Remote data for job */
    short      	flags;	    /* Flags to control treatment of job */
#define	JOB_IGNERR	0x001	/* Ignore non-zero exits */
#define	JOB_SILENT	0x002	/* no output */
#define JOB_SPECIAL	0x004	/* Target is a special one. i.e. run it locally
				 * if we can't export it and maxLocal is 0 */
#define JOB_REMOTE	0x010	/* Job is running remotely */  
#define JOB_FIRST	0x020	/* Job is first job for the node */
#define JOB_RESTART	0x080	/* Job needs to be completely restarted */
#define JOB_RESUME	0x100	/* Job needs to be resumed b/c it stopped,
				 * for some reason */
#define JOB_CONTINUING	0x200	/* We are in the process of resuming this job.
				 * Used to avoid infinite recursion between
				 * JobFinish and JobRestart */
} Job;

extern int  	nJobs;	    	/* Number of jobs running (local and remote) */
extern int  	nLocal;	    	/* Number of jobs running locally */
extern Lst  	jobs;	    	/* List of active job descriptors */
extern Lst  	stoppedJobs;	/* List of jobs that are stopped or didn't
				 * quite get started */
extern Boolean	jobFull;    	/* Non-zero if no more jobs should/will start*/

#endif /* _JOB_H_ */
