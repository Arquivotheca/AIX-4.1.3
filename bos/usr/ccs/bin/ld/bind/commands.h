/* @(#)13	1.5  src/bos/usr/ccs/bin/ld/bind/commands.h, cmdld, bos411, 9428A410j 4/4/94 16:28:08 */
#ifndef Binder_COMMANDS
#define Binder_COMMANDS
/*
 *   COMPONENT_NAME: CMDLD
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>

/* Type of a binder-command function */
typedef RETCODE bind_command_t(char *[]);

/*************************************************************************
 *	command		Structure to define mapping of linkage editor:
 *			Command name and function name to process command
 *************************************************************************/
struct command {
    char		*name;		/* command name */
    bind_command_t	*function;	/* function to call to run command */
    short		flags;
#define CMD_READ_FILES_FIRST 1

    short		minarg, maxarg;	/* min, max number arguments for cmd.
					 If max is -1, unlimited arguments */
#ifdef NO_NLS
    char		*usage_message;	/* If NULL, no usage message */
#else
    short		usage_message;	/* If 0, no usage message */
#endif
};

extern struct command	Commands[];
extern RETCODE		do_command(char *, int);
extern RETCODE		exec_loop(FILE *, char *);
extern RETCODE		bindopt(char *[]); /* argv-style arguments */

/* Structure for option arguments */
typedef struct options {
    char	*name;
    int		value;
} opts;

/* Structure for options (arguments to SETOPT command) */
typedef struct opt {
    char	*opt_name;
    char	*opt_value;
    short	opt_type;
#define NOARG_OPT 0
#define LOADMAP_OPT 1			/* Special opt. argument for LOADMAP */
#define INT_OPT 2			/* Option is integer */
#if 0
#define IGNORE_OPT 3			/* Option is ignored */
#endif
#define STRING_OPT 4			/* Option is any string */
#define STRINGLIST_OPT 5		/* Option is accumulating list of
					   strings. */
#define LIST_OPT 6			/* Option has list of choices */

    short	opt_flags;
#define OPT_OK_IN_IMPFILE 1		/* This option can appear in an
					   import or export file */

    opts	*opt_args;		/* if non-null, option takes arg. */
} option_t;

extern option_t option[];


#endif
