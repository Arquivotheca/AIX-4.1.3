#ifndef lint
static char sccsid[] = "@(#)14  1.7 src/bos/usr/ccs/bin/make/job.c, cmdmake, bos41J, 9512A_all 3/20/95 15:59:29";
#endif /* lint */
/*
 *   COMPONENT_NAME: CMDMAKE      System Build Facilities (make)
 *
 *   FUNCTIONS: Job_CheckCommands
 *		Job_Touch
 *		
 *
 *   ORIGINS: 27,85
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: job.c,v $ $Revision: 1.2.2.3 $ (OSF) $Date: 1992/03/17 22:14:30 $";
#endif
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
 */

/*-
 * job.c --
 *	handle the creation etc. of our child processes.
 *
 * Interface:
 *	Job_CheckCommands   	Verify that the commands for a target are
 *	    	  	    	ok. Provide them if necessary and possible.
 *
 *	Job_Touch 	    	Update a target without really updating it.
 */

#include "make.h"
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "job.h"

extern int  errno;
extern Boolean suff_is_empty;
extern Boolean noBuiltins;

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
    Boolean 	  silent   	/* TRUE if should not print messages */
    )
{
    int		  streamID;   	/* ID of stream opened to do the touch */
    struct timeval times[2];	/* Times for utimes() call */

    if (!silent) {
	printf(MSGSTR(TOUCH1, "Touch complete for file %s.\n"), gn->name);
    }

    if (noExecute) {
	return;
    }

    if (gn->type & OP_ARCHV) {
	Arch_Touch (gn);
    } else {
	char	*file = gn->path ? gn->path : gn->name;

	times[0].tv_sec = times[1].tv_sec = now;
	times[0].tv_usec = times[1].tv_usec = 0;
	if (utimes(file, times) < 0){
	    streamID = open (file, O_RDWR | O_CREAT, 0666);

	    if (streamID >= 0) {
		char	c;

		/*
		 * Read and write a byte to the file to change the
		 * modification time, then close the file.
		 */
		if (read(streamID, &c, 1) == 1) {
		    lseek(streamID, 0L, SEEK_SET);
		    write(streamID, &c, 1);
		}
		
		(void)close (streamID);
	    } else
	fprintf(stderr,MSGSTR(TOUCHERR2, "make: Cannot touch "
			"file %s.\n\t%s\n"), file, strerror(errno));
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
    GNode          *gn,	    	    /* The target whose commands need
				     * verifying */
    void    	  (*abortProc)(const char *, ...)
				    /* Function to abort with message */
    )
{
    if (OP_NOP(gn->type) && Lst_IsEmpty (gn->commands)) {
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
	    Make_HandleTransform(DEFAULT, gn);
	    Var_Set (IMPSRC, Var_Value (TARGET, gn), gn);
	} else if (Dir_MTime (gn) == 0) {
	    /*
	     * The node wasn't the target of an operator we have no .DEFAULT
	     * rule to go on and the target doesn't already exist. There's
	     * nothing more we can do for this branch. If the -k flag wasn't
	     * given, we stop in our tracks, otherwise we just don't update
	     * this node's parents so they never get examined. 
	     */
	    if (keepgoing) {
		fprintf(stdout,MSGSTR(NORULE1, "make: Cannot find "
			"a rule to create target %s from "
			"dependencies.\n(continuing)\n"), gn->name);
		keepgoing_error_count++;
		return (FALSE);
	    } else {
		fprintf(stdout, MSGSTR(NORULE2, "make: Cannot find "
			"a rule to create target %s from "
			"dependencies.\nStop.\n"), gn->name);
		if ((suff_is_empty) || (noBuiltins == TRUE))
			/* no rules for suffixes.  This is an error. */
			exit(2);
		else
			/* suffix rules exist, so must be empty inference rule.
			 * this is not an error.  See POSIX P2003.2, 108(A) */
			exit(0);
	    }
	}
    }
    return (TRUE);
}
