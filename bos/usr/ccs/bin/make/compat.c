#ifndef lint
static char sccsid[] = "@(#)08  1.12 src/bos/usr/ccs/bin/make/compat.c, cmdmake, bos41J, 9524E 6/8/95 16:24:25";
#endif /* lint */
/*
 *   COMPONENT_NAME: CMDMAKE      System Build Facilities (make)
 *
 *   FUNCTIONS: CompatInterrupt
 *		CompatMake
 *		CompatRunCommand
 *		Compat_Run
 *		
 *
 *   ORIGINS: 27,85
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1995
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
static char rcsid[] = "@(#)$RCSfile: compat.c,v $ $Revision: 1.2.4.2 $ (OSF) $Date: 1992/09/24 14:42:06 $";
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

/*-
 * compat.c --
 *	The routines in this file implement the full-compatibility
 *	mode of PMake. Most of the special functionality of PMake
 *	is available in this mode. Things not supported:
 *	    - different shells.
 *	    - friendly variable substitution.
 *
 * Interface:
 *	Compat_Run	    Initialize things for this module and recreate
 *	    	  	    them as they need creating.
 */

#include    <stdio.h>
#include    <sys/types.h>
#include    <sys/stat.h>
#include    <sys/signal.h>
#include    <sys/wait.h>
#include    <sys/errno.h>
#include    <ctype.h>
#include    <stdlib.h>
#include    <unistd.h>
#include    "make.h"

/*
 * The following array is used to make a fast determination of which
 * characters are interpreted specially by the shell.  If a command
 * contains any of these characters, it is executed by the shell, not
 * directly by us.
 */

static char 	    meta[256];

static GNode	    *curTarg = NILGNODE;
static int  	    CompatRunCommand(char *, GNode *);
extern Boolean	    isFpathSet;

/*-
 *-----------------------------------------------------------------------
 * CompatInterrupt --
 *	Interrupt the creation of the current target and remove it if
 *	it isn't precious.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The target is removed and the process exits. If .INTERRUPT exists,
 *	its commands are run first WITH INTERRUPTS IGNORED.
 *
 *-----------------------------------------------------------------------
 */
static void
CompatInterrupt (
    int	    signo
    )
{
    GNode   *gn;
    
    if ((curTarg != NILGNODE) && !Targ_Precious (curTarg)) {
	struct stat st;
	char 	  *file = Var_Value (TARGET, curTarg);

	if (lstat (file, &st) == SUCCESS && (!S_ISDIR(st.st_mode)) &&
	    unlink (file) == SUCCESS) {
	    fprintf (stderr, MSGSTR(REMOVED, "*** %s removed\n"), file);
	}

	/*
	 * Run .INTERRUPT only if hit with interrupt signal
	 */
	if (signo == SIGINT) {
	    gn = Targ_FindNode(".INTERRUPT", TARG_NOCREATE);
	    if (gn != NILGNODE) {
		Lst_ForEach(gn->commands, CompatRunCommand, (ClientData)gn);
	    }
	}
    }
    if (signo != SIGQUIT) {
	(void) signal(signo, SIG_DFL);
	kill(getpid(), signo);
    }
    exit (2);
}

/*-
 *-----------------------------------------------------------------------
 * CompatRunCommand --
 *	Execute the next command for a target. If the command returns an
 *	error, the node's made field is set to ERROR and creation stops.
 *
 * Results:
 *	0 if the command succeeded, 1 if an error occurred.
 *
 * Side Effects:
 *	The node's 'made' field may be set to ERROR.
 *
 *-----------------------------------------------------------------------
 */
static int
CompatRunCommand (
    char    	  *cmd,	    	/* Command to execute */
    GNode   	  *gn    	/* Node from which the command came */
    )
{
    char    	  *cmdStart;	/* Start of expanded command */
    register char *cp;
    Boolean 	  silent,   	/* Don't print command */
		  execute,	/* Execute the command */
		  errCheck, 	/* Check errors */
		  plusPrefix;	/* Command was prefixed with a plus */
    int 	  reason;   	/* Reason for child's death */
    int	    	  status;   	/* Description of child's death */
    int	    	  cpid;	    	/* Child actually found */
    ReturnStatus  stat;	    	/* Status of fork */
    LstNode 	  cmdNode;  	/* Node where current command is located */
    char    	  **av;	    	/* Argument vector for thing to exec */
    Boolean 	  local;    	/* TRUE if command should be executed
				 * locally */

    plusPrefix = FALSE;
    silent = gn->type & OP_SILENT;
    errCheck = !(gn->type & OP_IGNORE);
    execute = !(noExecute || touchFlag);

    cmdNode = Lst_Member (gn->commands, (ClientData)cmd);
    cmdStart = Var_Subst (cmd, gn, FALSE);

    /*
     * Str_Break will return an argv with a NULL in av[1], thus causing
     * execvp to choke and die horribly. Besides, how can we execute a null
     * command? In any case, we warn the user that the command expanded to
     * nothing (is this the right thing to do?).
     */
     
    if (*cmdStart == '\0') {
	Error(MSGSTR(MTSTRING, 
	    "make: Command \"%s\" expands to empty string.\n"), cmd);
	return(0);
    } else {
	cmd = cmdStart;
    }
    Lst_Replace (cmdNode, (ClientData)cmdStart);

    while ((*cmd == '@') || (*cmd == '-') || (*cmd == '+')) {
	if (*cmd == '@') {
	    silent = TRUE;
	} else if (*cmd == '+') {
	    plusPrefix = TRUE;
	    execute = TRUE;
	} else {
	    errCheck = FALSE;
	}
	cmd++;
    }
    
    /*
     * Search for meta characters in the command. If there are no meta
     * characters, there's no need to execute a shell to execute the
     * command.
     */
    for (cp = cmd; !meta[*cp]; cp++) {
	continue;
    }

    /*
     * Print the command before echoing if we're not supposed to be quiet for
     * this one.
     * 
     * We want to print the command if:
     *	- the command has a plus prefix and is not suppressed by the user
     *	- the command is not suppressed by the user, and this is not
     *	  simply a query or touch request
     *	- we are responding to the -n option (noExecute) and this is
     * 	  not a query or touch request as well
     *
     */
    if ( (plusPrefix && !silent) || 
	 (!silent && !queryFlag && !touchFlag) ||
	 (noExecute && !queryFlag && !touchFlag)) {
	printf ("\t%s\n", cmd);
	fflush(stdout);
    }

    /*
     * If we're not supposed to execute any commands, this is as far as
     * we go...
     */
    if (!execute) {
	return (0);
    }
    
    if ( (*cp != '\0') || (isFpathSet) ) {
	/*
	 * If *cp isn't the null character, we hit a "meta" character and
	 * need to pass the command off to the shell.  Also, if FPATH was
	 * set in the shell environment, then we have to use the shell because
	 * for all we know, one of the parameters might be a shell function
	 * located in the FPATH.  We give the shell the -e flag as well
	 * as -c if it's supposed to exit when it hits an error.
	 */
	static char	*shargv[4];

	/* Use the value of the SHELL macro as the shell. */
	shargv[0] = Var_Value("SHELL", VAR_GLOBAL);
	shargv[1] = (errCheck ? "-ec" : "-c");
	shargv[2] = cmd;
	shargv[3] = (char *)NULL;
	av = shargv;
    } else {
	/*
	 * No meta-characters, so no need to exec a shell. Break the command
	 * into words to form an argument vector we can execute.
	 */
	av = Str_Break((char *)NULL, cmd, (int *)NULL);
    }
    
    local = TRUE;

    /*
     * Fork and execute the single command. If the fork fails, we abort.
     */
    cpid = fork();
    if (cpid < 0) {
	Fatal(MSGSTR(CANTFORK, 
		"make: Cannot fork a new process for %s."), cmd);
    }
    if (cpid == 0) {
	if (local) {
	    execvp(av[0], av);
		 fputs(av[0],stderr);
	    Error(MSGSTR(NOTFOUND, ": not found\n"));
	} else {
	    (void)execv(av[0], av);
	}
	exit(1);
    }
    
    /*
     * The child is off and running. Now all we can do is wait...
     */
    while (1) {
	int 	  id;

	if (!local) {
	    id = 0;
	}

	while ((stat = wait(&reason)) != cpid) {
	    if (stat == -1 && errno != EINTR) {
		break;
	    }
	}
	
	if (stat > -1) {
	    if (WIFSTOPPED(reason)) {
		status = WSTOPSIG(reason);		/* stopped */
	    } else if (WIFEXITED(reason)) {
		status = WEXITSTATUS(reason);		/* exited */
		if (status != 0) {
		   fprintf(stderr,MSGSTR(ERRCODE, "make: The error "
			"code from the last command is %d.\n"), status);
		}
	    } else {
		status = WTERMSIG(reason);		/* signaled */
	fprintf(stderr,MSGSTR(SIGCODE, "make: The signal code "
			"from the last command is %d.\n"), status);
	    } 

	    
	    if (!WIFEXITED(reason) || (status != 0)) {
		if (errCheck) {
		    gn->made = ERROR;
		    if (keepgoing) {
			/*
			 * Abort the current target, but let others
			 * continue.
			 */
			fprintf(stderr,MSGSTR(CONTIN, " (continuing)\n"));
			keepgoing_error_count++;
		    }
		} else {
		    /*
		     * Continue executing commands for this target.
		     * If we return 0, this will happen...
		     */
		   fprintf(stderr,MSGSTR(ERRCODE1I, "make: Ignored "
			"error code %d from last command.\n"), status);
		    status = 0;
		}
	    }
	    break;
	} else {
	    Fatal (MSGSTR(BADWAIT, "make: The wait system call "
		"failed with status %d."), status);
	    /*NOTREACHED*/
	}
    }

    return (status);
}

/*-
 *-----------------------------------------------------------------------
 * CompatMake --
 *	Make a target.
 *
 * Results:
 *	0
 *
 * Side Effects:
 *	If an error is detected and not being ignored, the process exits.
 *
 *-----------------------------------------------------------------------
 */
static int
CompatMake (
    GNode   	  *gn,	    /* The node to make */
    GNode   	  *pgn	    /* Parent to abort if necessary */
    )
{
    if (gn->made == UNMADE) {
	/*
	 * First mark ourselves to be made, then apply whatever transformations
	 * the suffix module thinks are necessary. Once that's done, we can
	 * descend and make all our children. If any of them has an error
	 * but the -k flag was given, our 'make' field will be set FALSE again.
	 * This is our signal to not attempt to do anything but abort our
	 * parent as well.
	 */
	gn->make = TRUE;
	gn->made = BEINGMADE;
	Suff_FindDeps (gn);
	Lst_ForEach (gn->children, CompatMake, (ClientData)gn);
	if (!gn->make) {
	    gn->made = ABORTED;
	    pgn->make = FALSE;
	    return (0);
	}

	if (Lst_Member (gn->iParents, pgn) != NILLNODE) {
	    Var_Set (IMPSRC, Var_Value(TARGET, gn), pgn);
	}
	
	/*
	 * All the children were made ok. Now cmtime contains the modification
	 * time of the newest child, we need to find out if we exist and when
	 * we were modified last. The criteria for datedness are defined by the
	 * Make_OODate function.
	 */
	if (DEBUG(MAKE)) {
	   fprintf(stderr,MSGSTR(EXAMINE, "Examining %s..."), gn->name);
	}
	if (! Make_OODate(gn)) {
	    gn->made = UPTODATE;
	    if (DEBUG(MAKE)) {
	fprintf(stderr,MSGSTR(UPTODATE2, "up-to-date.\n"));
	    }
	    return (0);
	} else if (DEBUG(MAKE)) {
	   fprintf(stderr,MSGSTR(OUTOFDATE1, "out-of-date.\n"));
	}

	/*
	 * If the user is just seeing if something is out-of-date, 
	 * set exit value now to tell him/her "yes".
	 */
	queryExit = 1;

	/*
	 * We need to be re-made. We also have to make sure we've got a $?
	 * variable. To be nice, we also define the $> variable using
	 * Make_DoAllVar().
	 */
	Make_DoAllVar(gn);
		    
	/*
	 * Alter our type to tell if errors should be ignored or things
	 * should not be printed so CompatRunCommand knows what to do.
	 */
	if (Targ_Ignore (gn)) {
	    gn->type |= OP_IGNORE;
	}
	if (Targ_Silent (gn)) {
	    gn->type |= OP_SILENT;
	}

	if (Job_CheckCommands (gn, Fatal)) {
	    /*
	     * Our commands are ok, but we still have to worry about the -t
	     * flag...
	     */
	    curTarg = gn;
	    Lst_ForEach (gn->commands, CompatRunCommand, (ClientData)gn);
	    if (touchFlag)
		Job_Touch(gn, gn->type & OP_SILENT);
	    curTarg = NILGNODE;
	} else {
	    gn->made = ERROR;
	}

	if (gn->made != ERROR) {
	    /*
	     * If the node was made successfully, mark it so, update
	     * its modification time and timestamp all its parents. Note
	     * that for .ZEROTIME targets, the timestamping isn't done.
	     * This is to keep its state from affecting that of its parent.
	     */
	    gn->made = MADE;
	    /*
	     * This is what Make does and it's actually a good thing, as it
	     * allows rules like
	     *
	     *	cmp -s y.tab.h parse.h || cp y.tab.h parse.h
	     *
	     * to function as intended. Unfortunately, thanks to the stateless
	     * nature of NFS (and the speed of this program), there are times
	     * when the modification time of a file created on a remote
	     * machine will not be modified before the stat() implied by
	     * the Dir_MTime occurs, thus leading us to believe that the file
	     * is unchanged, wreaking havoc with files that depend on this one.
	     */
	    if (noExecute || Dir_MTime(gn) == 0) {
		gn->mtime = now;
	    }
	    if (DEBUG(MAKE)) {
	fprintf(stderr,MSGSTR(UPTIME, 
			"update time: %s\n"), Targ_FmtTime(gn->mtime));
	    }
	    pgn->childMade = TRUE;
	    Make_TimeStamp(pgn, gn);
	} else if (keepgoing) {
	    pgn->make = FALSE;
	    keepgoing_error_count++;
	} else {
	   fprintf(stderr,MSGSTR(STOPMSG, "\n\nStop.\n"));
	    exit (2);
	}
    } else if (gn->made == ERROR) {
	/*
	 * Already had an error when making this beastie. Tell the parent
	 * to abort.
	 */
	pgn->make = FALSE;
    } else {
	if (Lst_Member (gn->iParents, pgn) != NILLNODE) {
	    Var_Set (IMPSRC, Var_Value(TARGET, gn), pgn);
	}
	switch(gn->made) {
	    case BEINGMADE:
		Error(MSGSTR(PREDCIRL, "make: Target \"%s\" is "
			"dependent on itself.\n"), gn->name);
		gn->made = ERROR;
		pgn->make = FALSE;
		break;
	    case MADE:
		pgn->childMade = TRUE;
		Make_TimeStamp(pgn, gn);
		break;
	    case UPTODATE:
		Make_TimeStamp(pgn, gn);
		break;
	    default:
		break;
	}
    }

    return (0);
}
	
/*-
 *-----------------------------------------------------------------------
 * Compat_Run --
 *	Initialize this mode and start making.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Guess what?
 *
 *-----------------------------------------------------------------------
 */
void
Compat_Run(
    Lst	    	  targs     /* List of target nodes to re-create */
    )
{
	char    	  *cp;	    /* Pointer to string of shell meta-characters */
	GNode   	  *gn;	    /* Current root target */
	int	    	  errors;   /* Number of targets not remade due to errors */

	if (signal(SIGINT, SIG_IGN) != SIG_IGN) {
		signal(SIGINT, CompatInterrupt);
	}
	if (signal(SIGTERM, SIG_IGN) != SIG_IGN) {
		signal(SIGTERM, CompatInterrupt);
	}
	if (signal(SIGHUP, SIG_IGN) != SIG_IGN) {
		signal(SIGHUP, CompatInterrupt);
	}
	if (signal(SIGQUIT, SIG_IGN) != SIG_IGN) {
		signal(SIGQUIT, CompatInterrupt);
	}

	for (cp = "\"#=|^(){};&<>*?[]:$'`\\\n"; *cp != '\0'; cp++) {
		meta[*cp] = 1;
	}
	/*
     * The null character serves as a sentinel in the string.
     */
	meta[0] = 1;

	/*
     * For each entry in the list of targets to create, call CompatMake on
     * it to create the thing. CompatMake will leave the 'made' field of gn
     * in one of several states:
     *	    UPTODATE	    gn was already up-to-date
     *	    MADE  	    gn was recreated successfully
     *	    ERROR 	    An error occurred while gn was being created
     *	    ABORTED	    gn was not remade because one of its inferiors
     *	    	  	    could not be made due to errors.
     */
	errors = 0;
	while (!Lst_IsEmpty (targs)) {
		gn = (GNode *) Lst_DeQueue (targs);
		CompatMake (gn, gn);

		if (gn->made == UPTODATE) {
			/* Non-diagnostic messages get written to stdout. */
			printf(MSGSTR(UP2DATE, "Target \"%s\" is up to "
			    "date.\n"), gn->name);
		} else if (gn->made == ABORTED) {
			fprintf(stderr,MSGSTR(NOTMADERR, "Target \"%s\" did not make because "
			    "of errors.\n"), gn->name);
			errors += 1;
		}
		/* Else if there are no commands listed for the target, then the
	   target is treated as up-to-date. */
		else if ((gn->made == MADE) && Lst_IsEmpty(gn->commands))
		{
			/* Non-diagnostic messages get written to stdout. */
			printf(MSGSTR(UP2DATE, "Target \"%s\" is up to date.\n"), gn->name);
			queryExit=0;
		}
	}
}
