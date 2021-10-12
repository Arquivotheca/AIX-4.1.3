#if lint == 0 && CENTERLINE == 0
#pragma comment (copyright, "@(#)89	1.26  src/bos/usr/ccs/bin/ld/bind/commands.c, cmdld, bos411, 9428A410j 4/4/94 18:04:25")
#endif

/*
 *   COMPONENT_NAME: CMDLD
 *
 *   FUNCTIONS (for binder commands):
 *		abort_bind
 *		align
 *		bindopt
 *		entry
 *		execute/do_execute
 *		halt
 *		help
 *		libpath/do_libpath
 *		maxdat
 *		maxstk
 *		max_retcode
 *		noentry
 *		nolibpath
 *		noop
 *		origin
 *		pad
 *		prompt
 *		quit
 *		rename_cmd
 *		shell
 *		stats
 *		stgcls
 *		tocload
 *		unsupported
 *
 *   FUNCTIONS:	do_command
 *		tokens
 *		moretokens
 *
 *   STATIC FUNCTIONS:
 *		output_line
 *		update_align
 *		update_smc
 *		lookup_smclass
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
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/wait.h>
#include <sys/param.h>

#ifdef STATS
/* Cause memory type names to be included from global.h */
#define MEM_NAMES
#endif

#include "global.h"
#include "bind.h"
#include "error.h"
#include "dump.h"
#include "match.h"
#include "strs.h"
#include "objects.h"
#include "commands.h"
#include "insert.h"
#include "util.h"

/* Define length of line for display setopt values */
#define LINE_LEN 80

/* Global variable */
char *Command_name;			/* Name of current command. */

/************************************************************************
 * Name: output_line
 *									*
 * Purpose: Output the current buffer if the next portion of the output
 *	line will go past 80 characters.  Otherwise, add a blank to
 *	separate options.  The caller will actually put the new string
 *	into the buffer.
 *									*
 * Returns:	New length of output buffer.
 *
 ************************************************************************/
static int
output_line(char *buf,			/* Current buffer */
	    int cur_len,		/* Used length of buffer */
	    int delta_len)		/* Length of string to be added. */
{
    if (cur_len + delta_len + 1 >= LINE_LEN - 1) {
	say(SAY_NO_NLS, "%s", buf);
	buf[0] = '\0';
	return 0;
    }
    buf[cur_len] = ' ';
    return cur_len + 1;
}
/************************************************************************
 * Name: bindopt		Set binder processing options		*
 *									*
 * Purpose: Allows the interactive user of the binder to modify		*
 *	some of the binder processing options.				*
 *									*
 * Command Format:							*
 *	SETOPT [option[:<argument>] ... ]
 *									*
 * Arguments:								*
 *	option	- Option to be set or reset.				*
 *
 * Notes: If setopt is invoked without any option, the list of current
 *	option settings is displayed.
 *	Valid options are found in cmd_list.c
 *	*** Most options have a corresponding NO option ***
 *	Some options have arguments.
 *
 * If arg[0] is NULL, bindopt is being called to process options specified
 *		on the command line.
 *
 * If Bind_state.state & STATE_PROCESSING_IMPEXP, arg[0] is the import or
 * export file being processed.
 *
 * Otherwise, arg[0] is the binder subcommand name (SETOPT).
 *
 * Returns: Always returns RC_OK.  If an invalid option name or argument
 *		is specified, a message is printed, setting the return
 *		value to RC_NI_ERROR.  If an option is set within an
 *		import or export file, the return value is set to
 *		RC_WARNING.
 *									*
 * Side Effects:							*
 *	Modifies the following EXTERNAL variables:			*
 *		Switches - Structure containing process control flags	*
 *		Bind_state- Load map filename stored in loadmap_fn	*
 *									*
 ************************************************************************/
RETCODE
bindopt(char *arg[])			/* argv-style arguments */
{
    static	char *id = "bindopt";
    int		len;
    int		cur_len;
    int		i, found,
		opt_no;
    int		count, opt_len;
    char	*cur_arg, *cur_arg1, *n1, *p;
    char	**strs;
    char	temp_buf[LINE_LEN+2];	/* Leave room for dummy at beginning
					   and terminating '\0'.*/
    char	*buf = &temp_buf[1];	/* One dummy byte exists before *buf */
    opts	*x;

    struct opt	*q;

    /* see if any arguments and not called from main() */
    if (arg[0] != NULL			/* If called from main(), arg[0]=NULL*/
	&& moretokens(&cur_arg, 1) == 0) {
	/* Display current setopt settings */

	buf[0] = '\0';
	cur_len = -1;

	/* Display options turned on and options with arguments. */
	for (q = &option[0]; q->opt_name; ++q) {
	    switch(q->opt_type) {
	      case NOARG_OPT:
		/* Ordinary on/off option */
		if (*(q->opt_value)) {
		    cur_len = output_line(buf, cur_len, strlen(q->opt_name));
		    strcpy(&buf[cur_len], q->opt_name);
		    cur_len += strlen(q->opt_name);
		}
		break;
	      case INT_OPT:
		cur_len = output_line(buf, cur_len,
				      strlen(q->opt_name)
				      + 11 /*Space for ":0x" and 8 hex digits*/
				      );
		cur_len += sprintf(&buf[cur_len], "%s:0x%X",
				   q->opt_name, *((int*)(q->opt_value)));
		break;
#ifdef IGNORE_OPT
	      case IGNORE_OPT:
		break;
#endif
	      case STRINGLIST_OPT:
		if (*((char ***)(q->opt_value)) != NULL) {
		    /* write current line */
		    (void) output_line(buf, cur_len, LINE_LEN);
		    strs = *(char ***)(q->opt_value);
		    while (*strs)
			say(SAY_NO_NLS, "%s:%s", q->opt_name, *strs++);
		    cur_len = -1;	/* Next option at beginning of line */
		}
		break;

	      case LOADMAP_OPT:
		if (Switches.loadmap == 0)
		    break;
		/* else fall through */

	      case STRING_OPT:
		if (*((char **)(q->opt_value)) != NULL) {
		    cur_len = output_line(buf, cur_len,
					  strlen(q->opt_name)
					  +strlen(*(char **)(q->opt_value)));
		    cur_len += sprintf(&buf[cur_len], "%s:%s",
				       q->opt_name, *(char **)(q->opt_value));
		}
		break;
	      case LIST_OPT:
		cur_arg = "??";
		for (x = &q->opt_args[0]; x->name; x++) {
		    if (x->value == *(q->opt_value)) {
			cur_arg = x->name;
			break;
		    }
		}
		cur_len = output_line(buf, cur_len,
				      strlen(q->opt_name)
				      + 1 + strlen(cur_arg));
		cur_len += sprintf(&buf[cur_len], "%s:%s",
				   q->opt_name, cur_arg);
		break;
	    } /* switch */
	} /* for (options turned on */

	/* Now print turned-off options */
	for (q = &option[0]; q->opt_name; ++q) {
	    switch(q->opt_type) {
	      case NOARG_OPT:
		if (*(q->opt_value) == 0) {
		    cur_len = output_line(buf, cur_len, strlen(q->opt_name)+2);
		    cur_len += sprintf(&buf[cur_len], "NO%s", q->opt_name);
		}
		break;
	      case STRINGLIST_OPT:
		if (*(char ***)(q->opt_value) == NULL) {
		    cur_len = output_line(buf, cur_len, strlen(q->opt_name)+2);
		    cur_len += sprintf(&buf[cur_len], "NO%s", q->opt_name);
		}
		break;
	      case LOADMAP_OPT:
		if (Switches.loadmap != 0)
		    break;
		/* else fall through */
	      case STRING_OPT:
		if (*(char **)(q->opt_value) == NULL) {
		    cur_len = output_line(buf, cur_len, strlen(q->opt_name)+2);
		    cur_len += sprintf(&buf[cur_len], "NO%s", q->opt_name);
		}
		break;
	    }
	}
	say(SAY_NO_NLS, "%s", buf);	/* Print final line. */
	return RC_OK;
    }

    /* Set some options */
    if (arg[0] == NULL) {		/* Processing command line options. */
	i = 1;
	cur_arg = arg[i];
    }
    do {
	/* Convert option argument to upper case, up to ':' delimiter. */
	p = cur_arg;
	n1 = cur_arg;
	while (*n1 && *n1 != ':') {
	    if (islower((int) *n1))
		*n1 = (char)toupper((int) *n1);
	    n1++;
	}
	if (*n1 == ':')			/* Option has argument? */
	    len = n1 - cur_arg;
	else
	    len = strlen(cur_arg);

	found = 0;

	if (strncmp(cur_arg, "NO", 2) == 0) {
	    opt_no = 2;
	    len -= 2;
	    cur_arg += 2;
	}
	else
	    opt_no = 0;

	for (q = &option[0]; found == 0 && q->opt_name; ++q) {
	    if (strncmp(cur_arg, q->opt_name, len) != 0
		|| q->opt_name[len] != '\0')
		continue;
	    found = 1;
	    cur_arg += len;		/* Point to argument (if any) */

	    if (!(q->opt_flags & OPT_OK_IN_IMPFILE))
		if (Bind_state.state & STATE_PROCESSING_IMPEXP) {
		    bind_err(SAY_NORMAL, RC_WARNING,
			     NLSMSG(SETOPT_BAD,
    "%1$s: 0711-153 WARNING: File %2$s\n"
    "\tOption %3$.*4$s may not be changed within an import or export file."),
			     Main_command_name, arg[0], cur_arg - len, len);
		    continue;
		}

	    switch(q->opt_type) {
	      case NOARG_OPT:
		*(q->opt_value) = opt_no ? 0 : 1;
		break;

	      case LOADMAP_OPT:
		if (opt_no) {
		    if (*cur_arg == ':')
			goto bad_opt;
		    (void) loadmap_control(LOADMAP_STOP);
		}
		else if (*cur_arg++ == ':') {
		    if (*cur_arg == '\0') { /* Missing argument */
			bind_err(SAY_NORMAL, RC_NI_SEVERE,
				 NLSMSG(CMD_LOADMAP_NOARG,
 "%1$s: 0711-150 SEVERE ERROR: Missing pathname for SETOPT loadmap:pathname"),
				 Main_command_name);
			break;
		    }
		    if (*cur_arg == '\\' && arg[0] != NULL) {
			/* Option is not from the command line.  We must undo
			   the escapes.  If the name is too long, cur_arg1
			   returns NULL.  cur_arg will still point to a
			   too-long string, which is passed to loadmap_control
			   which will print an error message. */
			cur_arg1 = unescape_pathname(cur_arg, PATH_MAX,
						     cur_arg);
			if (cur_arg1 != NULL)
			    cur_arg = cur_arg1;
		    }
		    (void) loadmap_control(LOADMAP_START, cur_arg);
		}
		else
		    (void) loadmap_control(LOADMAP_START, "");
		break;

	      case INT_OPT:
		if (opt_no || *cur_arg++ != ':')
		    goto bad_opt;
		*((int *)(q->opt_value)) = atoul(cur_arg);
		break;

#ifdef IGNORE_OPT
	      case IGNORE_OPT:
		bind_err(SAY_NORMAL, RC_WARNING,
			 NLSMSG(SETOPT_IGNORE,
		"%1$s: 0711-275 WARNING: %2$s option %3$s is ignored."),
			 Main_command_name, Command_name, p);
		break;
#endif

	      case LIST_OPT:
		if (opt_no)
		    goto bad_opt;
		else if (*cur_arg++ != ':' || *cur_arg == '\0')
		    goto missing_arg;
		lower(cur_arg);
		for (x = &q->opt_args[0]; x->name; x++) {
		    if (strcmp(cur_arg, x->name) == 0) {
			*(q->opt_value) = x->value;
			break;
		    }
		}
		if (x->name == NULL) {
		    bind_err(SAY_NORMAL, RC_NI_SEVERE,
			     NLSMSG(SETOPT_BADARG,
    "%1$s: 0711-155 SEVERE ERROR: %2$s option %3$s: Invalid argument: %4$s"),
			     Main_command_name, Command_name,
			     q->opt_name, cur_arg);
		}
		break;
	      case STRINGLIST_OPT:
		strs = *(char ***)(q->opt_value);
		if (opt_no) {
		    if (*cur_arg == ':')
			goto bad_opt;
		    while (*strs)
			efree(*strs++);
		    if (*(char ***)(q->opt_value) != NULL)
			efree(*(char ***)(q->opt_value));
		    *(char ***)(q->opt_value) = NULL;
		}
		else if (*cur_arg++ != ':' || *cur_arg == '\0')
		    goto missing_arg;
		else {
		    count = 0;
		    opt_len = strlen(cur_arg);
		    if (strs == NULL) {
			strs = emalloc(2 * sizeof(char **), id);
		    }
		    else {
			while (strs[count] != NULL)
			    ++count;
			strs = erealloc(strs, (count + 2) * sizeof(char **),
					id);
		    }
		    strs[count] = emalloc(opt_len + 1, id);
		    strcpy(strs[count], cur_arg);
		    strs[count+1] = NULL;
		    *((char ***)(q->opt_value)) = strs;
		}
		break;

	      case STRING_OPT:
		if (opt_no) {
		    if (*cur_arg == ':')
			goto bad_opt;
		    *(char **)(q->opt_value) = NULL;
		}
		else if (*cur_arg++ != ':' || *cur_arg == '\0') {
		  missing_arg:
		    bind_err(SAY_NORMAL, RC_NI_SEVERE,
			     NLSMSG(CMD_SETOPT_NOARG,
 "%1$s: 0711-278 SEVERE ERROR: %2$s: Argument missing for option %3$s"),
				 Main_command_name, Command_name, q->opt_name);
		}
		else
		    *((char **)(q->opt_value))
			= save_string(cur_arg, strlen(cur_arg) + 1);
		break;
	    } /* switch */
	}

	if (found == 0) {
	  bad_opt:
	    bind_err(SAY_NORMAL, RC_NI_SEVERE,
		     NLSMSG(CMD_BADARG,
	    "%1$s: 0711-151 SEVERE ERROR: %2$s: Invalid option name: %3$s"),
		     Main_command_name, Command_name, p);
	}
    } while ((cur_arg = (arg[0] == NULL
			 ? arg[++i]
			 : (moretokens(&cur_arg, 1),cur_arg))) != NULL);

    return RC_OK;
} /* bindopt */
/************************************************************************
 * Name: do_command		Binder command processor		*
 *									*
 * Purpose: This routine parses a control command, selects the		*
 *	appropriate processing procedure, and returns control to	*
 *	the main control procedure.  The processing procedures		*
 *	are invoked by way of function calls.				*
 *									*
 * Returns:								*
 *	RC_OK	- null command or comment				*
 *	RC_NI_SEVERE - invalid command given
 *	otherwise- Return code from command processed.			*
 *									*
 ************************************************************************/
RETCODE
do_command(char *command,		/* binder command */
	   int echo)			/* Echo command to stdout? */
{
    int		i;
    int		rc, num_toks;
    char	*token[MAXTOKENS];
    char	*c, *p, *q;
    static char	cmd[BUFSIZ];		/* This buffer must be static, because
					   Command_name may point here after
					   we return. */

    /* Echo flag keeps interactively-entered commands from being echoed. */
    say(SAY_NO_NLS | (echo ? 0 : SAY_NOSTDOUT), "%s", command);

    /* null command or comment */
    if (*command == '\0' || *command == '*')
	return RC_OK;

    if (!Switches.asis)	/* parse command uppercase */
	upper(command);

    if (tokens(command, token, 1) > 0) {
	if (Switches.asis) {
	    if (strlen(token[0]) >= sizeof(cmd))
		goto command_invalid;
	    p = &cmd[0];
	    q = token[0];
	    while(*q) {
		if (islower((int) *q))
		    *p++ = (char)toupper((int) *q++);
		else
		    *p++ = *q++;
	    }
	    *p = '\0';
	    c = cmd;
	}
	else
	    c = token[0];

	Command_name = c;
	for (i = 0; Commands[i].name; ++i) {
	    if (strcmp(c, Commands[i].name) == 0) {
		if (Commands[i].maxarg == -1)
		    num_toks = Commands[i].minarg - 1;
		else
		    num_toks = Commands[i].maxarg;

		if (num_toks > MAXTOKENS - 2)
		    internal_error();

		if (num_toks >= 0) {
		    token[num_toks+1] = NULL;
		    num_toks = moretokens(&token[1], num_toks + 1);
		}
		else
		    num_toks = 0;

		if (num_toks < Commands[i].minarg ||
		    (Commands[i].maxarg != -1
		     && num_toks > Commands[i].maxarg)) {
		    if (Commands[i].maxarg == Commands[i].minarg)
			bind_err(SAY_NORMAL, RC_NI_SEVERE,
				 NLSMSG(CMD_TOOFEW,
 "%1$s: 0711-156 SEVERE ERROR: Binder command %2$s requires %3$d arguments."),
				 Main_command_name, token[0],
				 Commands[i].minarg);
		    else if (Commands[i].maxarg == -1)
			bind_err(SAY_NORMAL, RC_NI_SEVERE,
				 NLSMSG(CMD_TOOFEW2,
 "%1$s: 0711-157 SEVERE ERROR: Binder command %2$s requires %3$d or more arguments."),
				 Main_command_name, token[0],
				 Commands[i].minarg);
		    else
			bind_err(SAY_NORMAL, RC_NI_SEVERE,
				 NLSMSG(CMD_ARGCOUNT,
 "%1$s: 0711-158 SEVERE ERROR: Binder command %2$s requires %3$d to %4$d arguments."),
				 Main_command_name, token[0],
				 Commands[i].minarg, Commands[i].maxarg);
		    if (Commands[i].usage_message)
			bind_err(SAY_NORMAL, RC_NI_SEVERE,
				 Commands[i].usage_message,
				 Main_command_name);
		    rc = RC_NI_SEVERE;
		}
		else {
		    if (Commands[i].flags & CMD_READ_FILES_FIRST)
			insert_deferred_files();
		    Bind_state.cmd_err_lev = RC_OK;
		    rc = (*Commands[i].function)(token);
		    /* We also pick up return code from bind_error calls */
		    if (rc >= RC_OK)
			rc = max(Bind_state.cmd_err_lev, rc);
		}
		break;
	    }
	}

	if (Commands[i].name == NULL) {
	  command_invalid:
	    bind_err(SAY_NORMAL, RC_NI_SEVERE,
		     NLSMSG(CMD_INVALID,
	    "%1$s: 0711-152 SEVERE ERROR: Invalid binder command: %2$s"),
		     Main_command_name, token[0]);
	    Command_name = NULL;
	    rc = RC_NI_SEVERE;		/* Set return code explicitly */
	}
    }
    else
	rc = RC_NI_ERROR;

    return rc;
} /* do_command */
/************************************************************************
 * Name: execute		EXECUTE binder command processor	*
 *									*
 * Purpose: Allow binder commands to be read from an alternate source	*
 *									*
 * Command Format:							*
 *	EXEC -							*
 *	  or								*
 *	EXEC filename						*
 *									*
 * Arguments:								*
 *	-	- Read binder commands from the terminal		*
 *	filename- Read binder commands from file			*
 *									*
 * Returns: Returns a status completion code				*
 *	RC_OK	- No error detected					*
 *	RC_SEVERE- Unable to open command file				*
 *	RC_ABORT- ABORT binder command given				*
 *									*
 * Side Effects:							*
 *	Modifies the following EXTERNAL variables:			*
 *		Switches - retcode is set to highest return code	*
 *									*
 ************************************************************************/
RETCODE do_execute(char *);		/* Forward declaration */
RETCODE
execute(char *arg[])			/* argv-style arguments */
{
    char	execute_fn[PATH_MAX+1];

    /* Check for a valid pathname */
    if (*arg[1] == '\\') {
	/* The filename contains escapes */
	if (unescape_pathname(execute_fn, PATH_MAX, arg[1]) == NULL)
	    goto name_too_long;
	return do_execute(execute_fn);
    }
    else {
	if (strlen(arg[1]) > PATH_MAX) {
	  name_too_long:
	    bind_err(SAY_NORMAL, RC_NI_SEVERE,
		     NLSMSG(FILE_NAMETOOLONG,
		    "%1$s: 0711-878 SEVERE ERROR: The pathname is too long.\n"
		    "\tThe maximum length of a pathname is %2$d.\n"
		    "\tBinder command %3$s cannot be executed."),
		     Main_command_name, PATH_MAX, Command_name);
	    return RC_NI_SEVERE;
	}
	return do_execute(arg[1]);
    }
} /* execute */
/***********************************************************************/
RETCODE
do_execute(char *fname)
{
    int		rc;
    FILE	*fp;
    const int	save_stdin_is_tty = Bind_state.stdin_is_tty;
    const int	save_interactive = Bind_state.interactive;
    int		close_file = 1;

    if (strcmp(fname, "-") == 0) {
	fname = "";
	if (Bind_state.tty_file) {
	    close_file = 0;
	    fp = Bind_state.tty_file;
	}
	else {
	    if ((fp = fopen("/dev/tty", "r")) == NULL) {
		bind_err(Bind_state.interactive ? SAY_STDOUT : SAY_NORMAL,
			 RC_NI_SEVERE,
			 NLSMSG(CMD_TTY_CANNOT_OPEN,
			"%1$s: 0711-982 SEVERE ERROR: Cannot open /dev/tty."),
			 Main_command_name);
		return RC_NI_SEVERE;
	    }
	    Bind_state.tty_file = fp;
	}
	Bind_state.stdin_is_tty = 1;
	Bind_state.ever_interactive = 1;
	Bind_state.interactive = 1;
    }
    else {
	if ((fp = fopen(fname, "r")) == NULL) {
	    bind_err(Bind_state.interactive ? SAY_STDOUT : SAY_NORMAL,
		     RC_NI_SEVERE,
		     NLSMSG(FILE_CANNOT_OPEN,
		    "%1$s: 0711-160 SEVERE ERROR: Cannot open file: %2$s\n"
			    "\t%1$s:%3$s() %4$s"),
		     Main_command_name, fname, "fopen", strerror(errno));
	    return RC_NI_SEVERE;
	}
	Bind_state.stdin_is_tty = 0;
    }

    rc = exec_loop(fp, fname);
    Command_name = "EXEC";		/* Restore name for non-0 RC msg. */

    Bind_state.stdin_is_tty = save_stdin_is_tty;
    Bind_state.interactive = save_interactive;

    if (close_file) {
	if (fp == Bind_state.tty_file)
	    Bind_state.tty_file = NULL;
	(void) fclose(fp);
    }

    if (rc == RC_ABORT)
	return RC_ABORT;

    return RC_OK;
} /* do_execute */
/************************************************************************
 * Name: tokens								*
 *									*
 * Purpose: Split a character string into argv-style tokens		*
 *									*
 * Returns: Returns number of tokens found (up to maxtokens)
 *									*
 * Side Effects:							*
 *	Character string is modified inline, not copied.		*
 *									*
 ************************************************************************/
int
tokens(char	*argstring,	/* string to process into tokens */
       char	*token[],	/* array for tokens */
       int	maxtokens)	/* maximum number of tokens allowed */
{
    int	i;

    token[0] = strtok(argstring, " \t");
    if (token[0] == NULL)
	return 0;

    for (i = 1; i < maxtokens; ++i) {
	token[i] = strtok(NULL, " \t");
	if (token[i] == NULL)
	    return i;
    }

    return maxtokens;
} /* tokens */
/************************************************************************
 * Name: moretokens
 *									*
 * Purpose: Parse some additional tokens from command line.
 *									*
 * Returns: Number of additional tokens received.
 *
 ************************************************************************/
int
moretokens(char	*token[],	/* array for tokens */
	   int	maxtokens)	/* maximum number of additional allowed */
{
    int	i;

    for (i = 0; i < maxtokens; ++i) {
	token[i] = strtok(NULL, " \t");
	if (token[i] == NULL)
	    return i;
    }

    return maxtokens;
}
/************************************************************************
 * Name: abort_bind		ABORT binder command processor		*
 *									*
 * Binder Subcommand Format:ABORT					*
 *									*
 * Purpose: Stop execution of the binder immediately.			*
 *									*
 * Returns: DOES NOT RETURN						*
 *									*
 * Side Effects:							*
 *	Preserves existing LOADMAP and object file			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
RETCODE
abort_bind(char *arg[])	/* argv-style arguments */
{
    bind_err(SAY_STDOUT, RC_OK,
	     NLSMSG(CMD_ABORT, "%s: Binder stops."), Command_name);
    cleanup(Bind_state.retcode);
    /* NOTREACHED */
}
/************************************************************************
 * Name: halt			HALT binder command processor		*
 *									*
 * Purpose: Set or display error termination level of binder		*
 *									*
 * Binder Subcommand Format:						*
 *	HALT		- Display error termination level		*
 *	  or								*
 *	HALT <num>	- Set error termination level to <num>		*
 *									*
 * Arguments:								*
 *	num	- Value to be the error termination level.		*
 *									*
 * Returns: RC_OK							*
 *									*
 ************************************************************************/
RETCODE
halt(char *arg[])			/* argv-style arguments */
{
    if (arg[1]) {			/* set halt value */
	Bind_state.err_exit_lev = atoi(arg[1]);

	/* Maximum value of limit is RC_PGMERR */
	if (Bind_state.err_exit_lev > RC_PGMERR)
	    Bind_state.err_exit_lev = RC_PGMERR;
    }
    else				/* display setting */
	say(SAY_NORMAL, NLSMSG(HALT_QUERY,
      "%1$s: The binder will terminate on return codes greater than %2$d."),
	    Command_name, Bind_state.err_exit_lev);

    return RC_OK;
}
/************************************************************************
 * Name: help			HELP binder command processor		*
 *									*
 * Purpose: Provide the user with a list of valid commands and options	*
 *	giving a brief synopsis of each.				*
 *									*
 * Command Format:							*
 *	HELP								*
 *									*
 * Returns: RC_OK							*
 *									*
 ************************************************************************/
/*ARGSUSED*/
int
help(char *arg[])			/* argv-style arguments */
{
    say(SAY_NORMAL,
	NLSMSG(CMD_IGNORE, "%1$s: 0711-149 Binder command %2$s is ignored."),
	Main_command_name, Command_name);

    return RC_OK;
}
/************************************************************************
 * Name: quit			QUIT binder command processor		*
 *									*
 * Purpose: Stop reading binder commands from the current input source.	*
 *	If it is the original input source, stop the binder.		*
 *									*
 * Command Format:							*
 *	QUIT								*
 *									*
 * Returns: RC_QUIT							*
 *									*
 ************************************************************************/
/*ARGSUSED*/
RETCODE
quit(char *arg[])			/* argv-style arguments */
{
    return RC_QUIT;
}
/************************************************************************
 * Name: shell			SHELL binder command processor		*
 *									*
 * Purpose: Enter a subshell.						*
 *									*
 * Command Format:							*
 *	SH
 *									*
 * Arguments:								*
 *	None								*
 *									*
 * Returns: Returns a status completion code				*
 *	Value from system() call- Always returned.			*
 *									*
 * Side Effects:							*
 *	None.								*
 *									*
 ************************************************************************/
/*ARGSUSED*/
RETCODE
shell(char *arg[])			/* argv-style arguments */
{
    char *cmd;
    int rc;

    if ((cmd = getenv("SHELL")) == NULL)
	cmd = "sh";

    rc = system(cmd);
    if (WIFEXITED(rc))
	return RC_OK;
    else {
	bind_err(SAY_NORMAL, RC_NI_SEVERE,
		 NLSMSG(NOSHELL,
		"%1$s: 0711-135 SEVERE ERROR: Cannot execute subshell %2$s"),
		 Main_command_name, cmd);
	return RC_NI_SEVERE;
    }
}
/************************************************************************
 * Name: unsupported							*
 *									*
 * Purpose: Used as a stub for recognized command names with no		*
 *	corresponding implementation.					*
 *									*
 * Command Format:							*
 *	None								*
 *									*
 * Returns: Returns a status completion code				*
 *	RC_NI_WARNING - Always returned.				*
 *									*
 * Side Effects:							*
 *	None								*
 *									*
 ************************************************************************/
/*ARGSUSED*/
RETCODE
unsupported(char *arg[])		/* argv-style arguments */
{
    bind_err(SAY_NORMAL, RC_NI_WARNING,
	     NLSMSG(CMD_NOSUP,
	    "%1$s: 0711-144 WARNING: Binder command %2$s is unsupported."),
	     Main_command_name, Command_name);
    return RC_NI_WARNING;
}
/************************************************************************
 * Name: ignored							*
 *									*
 * Purpose: Used as a stub for recognized command names with no		*
 *	corresponding implementation.					*
 *									*
 * Command Format:							*
 *	None								*
 *									*
 * Returns: Returns a status completion code				*
 *	OK	- Always returned.					*
 *									*
 * Side Effects:							*
 *	None								*
 *									*
 ************************************************************************/
/*ARGSUSED*/
RETCODE
ignored(char *arg[])			/* argv-style arguments */
{
    say(SAY_NORMAL,
	NLSMSG(CMD_IGNORE, "%1$s: 0711-149 Binder command %2$s is ignored."),
	Main_command_name, Command_name);
    return RC_OK;
}
/************************************************************************
 * Name: prompt			Processor for the PROMPT command
 * Purpose: This routine is used to change the binder prompt
 ***********************************************************************/
RETCODE
prompt(char *arg[])			/* argv-style arguments */
{
    int n = strlen(arg[1]);
    if (n != 0) {			/* Null prompt not allowed */
	/* Truncate the prompt if necessary */
	if (n >= sizeof(Bind_state.prompt) -1)
	    n = sizeof(Bind_state.prompt) - 2;

	(void) strncpy(Bind_state.prompt, arg[1], n);
	Bind_state.prompt[n] = ' ';	/* Add a trailing space. */
	Bind_state.prompt[n+1] = '\0';
    }
    return RC_OK;
}
/************************************************************************
 * Name: entry			Processor for the ENTRY command		*
 *									*
 * Purpose: This routine is used to display or change the entrypoint	*
 *									*
 *	The entrypoint is a special form of "export". The entrypoint
 *	descriptor esd index (loader section) is placed in the loader
 *	header section.
 *
 *	For user applications, the entrypoint should be a descriptor, but
 *	for special purposes (such as the kernel) a code symbol can be the
 *	entrypoint.
 *
 ***********************************************************************/
RETCODE
entry(char *arg[])			/* argv-style arguments */
{
    if (arg[1]) {	/* specify entry point */
	if (Bind_state.entrypoint)
	    Bind_state.entrypoint->flags &= ~STR_ENTRY; /* Reset */

	Bind_state.entrypoint = putstring(arg[1]);
	Bind_state.entrypoint->flags |= STR_ENTRY;
	Bind_state.state |= STATE_RESOLVE_NEEDED;
	say(SAY_NORMAL, NLSMSG(ENTRY_SET, "%1$s: Entry point set to %2$s"),
	    Command_name, Bind_state.entrypoint->name);
    }
    else if (Bind_state.entrypoint)
	say(SAY_NORMAL, NLSMSG(ENTRY_QUERY, "%1$s: The entry point is %2$s"),
	    Command_name, Bind_state.entrypoint->name);
    else
	say(SAY_NORMAL, NLSMSG(ENTRY_NONE, "%s: There is no entry point."),
	    Command_name);

    return RC_OK;
} /* entry */
/************************************************************************
 * Name: noentry		Processor for the NOENTRY command
 *									*
 * Purpose: Indicate that no entrypoint exists.
 *									*
 ***********************************************************************/
/*ARGSUSED*/
RETCODE
noentry(char *arg[])			/* argv-style arguments */
{
    if (Bind_state.entrypoint)
	Bind_state.entrypoint->flags &= ~STR_ENTRY; /* Reset */

    Bind_state.entrypoint = NULL;
    Bind_state.state |= STATE_RESOLVE_NEEDED;
    say(SAY_NORMAL, NLSMSG(ENTRY_NONE, "%s: There is no entry point."),
	Command_name);

    return RC_OK;
} /* noentry */
/************************************************************************
 * Name: maxdat			MAXDATA binder command processor	*
 *									*
 * Purpose: Set	maximum data size for object being generated.		*
 *		Sets value in the a.out optional header of object file  *
 *									*
 * Command Format:							*
 *	MAXDATA num	- Set max data size allowed by generated object *
 *									*
 * Arguments:								*
 *	num	- Value of the max data size.				*
 *									*
 * Returns: RC_OK							*
 *									*
 ************************************************************************/
RETCODE
maxdat(char *arg[])			/* argv-style arguments */
{
    if (arg[1])	/* set max_data variable */
	temp_aout_hdr.o_maxdata = strtoul(arg[1], NULL, 0);

    /* display value */
    if (temp_aout_hdr.o_maxdata == 0 ) {
	say(SAY_NORMAL,
	    NLSMSG(MAXDATA_ZERO,
 "%s: The maximum .data section size field in the auxiliary header of the output\n"
 "\tobject file will be set to zero. The system will use the default limit."),
	    Command_name);
    }
    else
	say(SAY_NORMAL,
	    NLSMSG(MAXDATA_SET,
  "%1$s: The maximum .data section size field in the auxiliary header of the\n"
  "\toutput object file will be set to: %2$d (0x%2$X) bytes"),
	    Command_name, temp_aout_hdr.o_maxdata);

    return RC_OK;
}
/************************************************************************
 * Name: maxstk			MAXSTACK binder command processor	*
 *									*
 * Purpose: Set	max stack size for object being generated.		*
 *		Sets value in the a.out optional header of object file  *
 *									*
 * Command Format:							*
 *	MAXSTACK num	- Set max stack size allowed by generated object*
 *									*
 * Arguments:								*
 *	num	- Value of the max stack size.				*
 *									*
 * Returns: Returns a status completion code				*
 *	RC_OK	- Always						*
 *									*
 ************************************************************************/
RETCODE
maxstk(char *arg[])			/* argv-style arguments */
{
    if (arg[1])	/* set max_stack variable */
	temp_aout_hdr.o_maxstack = strtoul(arg[1], NULL, 0);

    /* display value */
    if (temp_aout_hdr.o_maxstack == 0)
	say(SAY_NORMAL,
	    NLSMSG(MAXSTACK_ZERO,
  "%s: The stack size limit field in the auxiliary header of the output\n"
  "\tobject file will be set to zero. The system will use the default limit."),
	    Command_name);
    else
	say(SAY_NORMAL,
	    NLSMSG(MAXSTACK_SET,
	   "%1$s: The stack size limit field in the auxiliary header of the\n"
	   "\toutput object file will be set to: %2$d (0x%2$X) bytes"),
	    Command_name, temp_aout_hdr.o_maxstack);

    return RC_OK;
} /* maxstk */
/************************************************************************
 * Name: noop
 *									*
 * Purpose: Specify or list valid NOP instructions
 *									*
 * Command Format:	NOP [ [primary | no] ops ...]
 *
 *									*
 * Returns:
 *									*
 ************************************************************************/
/*ARGSUSED*/
RETCODE
noop(char *arg[])			/* argv-style arguments */
{
    static char *id = "noop";

    char *c, *cur_arg;
    long op, op1;
    int rc = RC_OK;
    int i;
    long *new_nops;
    int primary = 0;
    int delete = 0;

    if (moretokens(&cur_arg, 1) == 1) {
	if (cur_arg[0] == 'p' || cur_arg[0] == 'P') {
	    lower(cur_arg);
	    if (strcmp(cur_arg, "primary") == 0) {
		primary = 1;
		if (moretokens(&cur_arg, 1) == 0) {
		    bind_err(SAY_NORMAL, RC_NI_SEVERE,
			     NLSMSG(INST_NOARG,
			    "%s: 0711-282 SEVERE ERROR: Missing argument.\n"
			    "\tSpecify a machine instruction after primary"),
			     Command_name);
		    return RC_NI_SEVERE;
		}
	    }
	}
	else if ((cur_arg[0] == 'n' || cur_arg[0] == 'N')
		 && (cur_arg[1] == 'o' || cur_arg[1] == 'O')) {
	    delete = 1;
	    if (moretokens(&cur_arg, 1) == 0) {
		bind_err(SAY_NORMAL, RC_NI_SEVERE,
			 NLSMSG(INST_NOARG2,
				"%s: 0711-283 SEVERE ERROR: Missing argument.\n"
				"\tSpecify a machine instruction after no"),
			 Command_name);
		return RC_NI_SEVERE;
	    }
	}

	do {
	    op = 0;
	    c = cur_arg;
	    while (*c) {
		if (isxdigit(*c))
		    if (isdigit(*c))
			op = op * 16 + *c - '0';
		    else
			op = op * 16 + (10 + *c - (isupper(*c) ? 'A' : 'a'));
		else {
		    bind_err(SAY_NORMAL, RC_NI_SEVERE,
			     NLSMSG(INST_BAD,
		    "%1$s: 0711-280 SEVERE ERROR: Invalid instruction %2$s\n"
		    "\tInstructions must consist of hex digits only."),
			     Command_name, cur_arg);
		    goto next_one;
		}
		c++;
	    }
	    if (primary) {
		op1 = Bind_state.nops[0];
		Bind_state.nops[0] = op;
		if (Bind_state.num_nops == Bind_state.max_nops) {
		    Bind_state.max_nops *= 2;
		    new_nops = emalloc(Bind_state.max_nops * sizeof(long), id);
		    for (i = 0; i < Bind_state.num_nops; i++)
			new_nops[i] = Bind_state.nops[i];
		    efree(Bind_state.nops);
		    Bind_state.nops = new_nops;
		}
		for (i = 1; i < Bind_state.num_nops; i++)
		    if (op == Bind_state.nops[i]) {
			Bind_state.nops[i] = op1;
			goto next_one;
		    }
		Bind_state.nops[Bind_state.num_nops++] = op1;
	    }
	    else if (delete) {
		if (op == Bind_state.nops[0]) {
		    bind_err(SAY_NORMAL, RC_NI_SEVERE,
			     NLSMSG(INST_NO_DELETE,
    "%s: 0711-281 SEVERE ERROR: The primary instruction cannot be deleted."),
			     Command_name);
		    rc = RC_NI_SEVERE;
		    goto next_one;
		}

		for (i = 1; i < Bind_state.num_nops; i++)
		    if (op == Bind_state.nops[i]) {
			Bind_state.nops[i]
			    = Bind_state.nops[--Bind_state.num_nops];
			goto next_one;
		    }
		bind_err(SAY_NORMAL, RC_NI_SEVERE,
			 NLSMSG(INST_NOP_NOT_FOUND,
	"%1$s: 0711-285 SEVERE ERROR: Instruction 0x%2$08x is not a current\n"
	"\tno-op instruction. It cannot be deleted."),
			 Command_name, op);
		rc = RC_NI_SEVERE;
	    }
	    else {
		if (Bind_state.num_nops == Bind_state.max_nops) {
		    Bind_state.max_nops *= 2;
		    new_nops = emalloc(Bind_state.max_nops * sizeof(long), id);
		    for (i = 0; i < Bind_state.num_nops; i++)
			new_nops[i] = Bind_state.nops[i];
		    efree(Bind_state.nops);
		    Bind_state.nops = new_nops;
		}
		Bind_state.nops[Bind_state.num_nops++] = op;
		for (i = 0; i < Bind_state.num_nops - 1; i++)
		    if (op == Bind_state.nops[i]) {
			--Bind_state.num_nops;
			break;
		    }
	    }
	  next_one:
	    primary = 0;
	} while (moretokens(&cur_arg, 1) == 1);
    }
    else {
	/* Display valid NOPs */
	say(SAY_NORMAL,
	    NLSMSG(INST_NOPS_VALID, "%s: The current no-op instructions are:"),
	    Command_name);
	for (i = 0; i < Bind_state.num_nops; i++)
	    say(SAY_NO_NLS, "\t0x%08x", Bind_state.nops[i]);
    }
    return rc;
}
/************************************************************************
 * Name: tocload
 *									*
 * Purpose: Specify or list valid TOCLOAD instructions
 *									*
 * Command Format:	TOCLOAD [ [primary | no] ops ...]
 *									*
 * Returns:
 *									*
 ************************************************************************/
/*ARGSUSED*/
RETCODE
tocload(char *arg[])			/* argv-style arguments */
{
    static char *id = "noop";

    char *c, *cur_arg;
    long op, op1;
    int rc = RC_OK;
    int i;
    long *new_loadtocs;
    int primary = 0;
    int delete = 0;

    if (moretokens(&cur_arg, 1) == 1) {
	if (cur_arg[0] == 'p' || cur_arg[0] == 'P') {
	    lower(cur_arg);
	    if (strcmp(cur_arg, "primary") == 0) {
		primary = 1;
		if (moretokens(&cur_arg, 1) == 0) {
		    bind_err(SAY_NORMAL, RC_NI_SEVERE,
			     NLSMSG(INST_NOARG,
			    "%s: 0711-282 SEVERE ERROR: Missing argument.\n"
			    "\tSpecify a machine instruction after primary"),
			     Command_name);
		    return RC_NI_SEVERE;
		}
	    }
	}
	else if ((cur_arg[0] == 'n' || cur_arg[0] == 'N')
		 && (cur_arg[1] == 'o' || cur_arg[1] == 'O')) {
	    delete = 1;
	    if (moretokens(&cur_arg, 1) == 0) {
		bind_err(SAY_NORMAL, RC_NI_SEVERE,
			 NLSMSG(INST_NOARG2,
				"%s: 0711-283 SEVERE ERROR: Missing argument.\n"
				"\tSpecify a machine instruction after no"),
			 Command_name);
		return RC_NI_SEVERE;
	    }
	}

	do {
	    op = 0;
	    c = cur_arg;
	    while (*c) {
		if (isxdigit(*c))
		    if (isdigit(*c))
			op = op * 16 + *c - '0';
		    else
			op = op * 16 + (10 + *c - (isupper(*c) ? 'A' : 'a'));
		else {
		    bind_err(SAY_NORMAL, RC_NI_SEVERE,
			     NLSMSG(INST_BAD,
		    "%1$s: 0711-280 SEVERE ERROR: Invalid instruction %2$s\n"
		    "\tInstructions must consist of hex digits only."),
			     Command_name, cur_arg);
		    goto next_one;
		}
		c++;
	    }
	    if (primary) {
		op1 = Bind_state.loadtocs[0];
		Bind_state.loadtocs[0] = op;
		if (Bind_state.num_loadtocs == Bind_state.max_loadtocs) {
		    Bind_state.max_loadtocs *= 2;
		    new_loadtocs
			= emalloc(Bind_state.max_loadtocs * sizeof(long), id);
		    for (i = 0; i < Bind_state.num_loadtocs; i++)
			new_loadtocs[i] = Bind_state.loadtocs[i];
		    efree(Bind_state.loadtocs);
		    Bind_state.loadtocs = new_loadtocs;
		}
		for (i = 1; i < Bind_state.num_loadtocs; i++)
		    if (op == Bind_state.loadtocs[i]) {
			Bind_state.loadtocs[i] = op1;
			break;
		    }
		Bind_state.loadtocs[Bind_state.num_loadtocs++] = op;
	    }
	    else if (delete) {
		if (op == Bind_state.loadtocs[0]) {
		    bind_err(SAY_NORMAL, RC_NI_SEVERE,
			     NLSMSG(INST_NO_DELETE,
    "%s: 0711-281 SEVERE ERROR: The primary instruction cannot be deleted."),
			     Command_name);
		    rc = RC_NI_SEVERE;
		    goto next_one;
		}

		for (i = 1; i < Bind_state.num_loadtocs; i++)
		    if (op == Bind_state.loadtocs[i]) {
			Bind_state.loadtocs[i]
			    = Bind_state.loadtocs[--Bind_state.num_loadtocs];
			goto next_one;
		    }
		bind_err(SAY_NORMAL, RC_NI_SEVERE,
			 NLSMSG(INST_TOCLOAD_NOT_FOUND,
	"%1$s: 0711-284 SEVERE ERROR: Instruction 0x%2$08x is not a current\n"
	"\tTOC-reload instruction. It cannot be deleted."),
			 Command_name, op);
		rc = RC_NI_SEVERE;
	    }
	    else {
		if (Bind_state.num_loadtocs == Bind_state.max_loadtocs) {
		    Bind_state.max_loadtocs *= 2;
		    new_loadtocs
			= emalloc(Bind_state.max_loadtocs * sizeof(long), id);
		    for (i = 0; i < Bind_state.num_loadtocs; i++)
			new_loadtocs[i] = Bind_state.loadtocs[i];
		    efree(Bind_state.loadtocs);
		    Bind_state.loadtocs = new_loadtocs;
		}
		Bind_state.loadtocs[Bind_state.num_loadtocs++] = op;
		for (i = primary; i < Bind_state.num_loadtocs - 1; i++)
		    if (op == Bind_state.loadtocs[i]) {
			--Bind_state.num_loadtocs;
			break;
		    }
	    }
	  next_one:
	    primary = 0;
	} while (moretokens(&cur_arg, 1) == 1);
    }
    else {
	/* Display valid TOCLOADs */
	say(SAY_NORMAL, NLSMSG(INST_TOCLOADS_VALID,
			       "%s: The current TOC-reload instructions are:"),
	    Command_name);
	for (i = 0; i < Bind_state.num_loadtocs; i++)
	    say(SAY_NO_NLS, "\t0x%08x", Bind_state.loadtocs[i]);
    }
    return rc;
}
/************************************************************************
 * Name: pad			PAD binder command processor		*
 *									*
 * Purpose:	Set or display the pad values.
 *									*
 * Command Format:	PAD .|<tpad> [.|<dpad> [<lpad>] ]
 *									*
 * Arguments:	. is a placeholder, keeping value from being modified.
 *									*
 * Returns: RC_OK							*
 *									*
 ************************************************************************/
RETCODE
pad(char *arg[])			/* argv-style arguments */
{
    if (arg[1]) {
	if (arg[1][0] != '.' || arg[1][1] != '\0')
	    Bind_state.tpad_align = strtoul(arg[1], NULL, 0);

	if (arg[2]) {
	    if (arg[2][0] != '.' || arg[2][1] != '\0')
		Bind_state.dpad_align = strtoul(arg[2], NULL, 0);

	    if (arg[3])
		Bind_state.lpad_align = strtoul(arg[3], NULL, 0);
	}
    }

    say(SAY_NORMAL, NLSMSG(PAD_SET,
			   "%1$s: The object file pad values are "
			   ".text: %2$d  .data: %3$d  .loader: %4$d"),
	Command_name,
	Bind_state.tpad_align,
	Bind_state.dpad_align,
	Bind_state.lpad_align);
    return RC_OK;
}
/************************************************************************
 * Name: update_align
 *									*
 * Purpose:	Static function used to update alignments for align
 *		command.  Only names for which there are non-duplicate,
 *		global symbols will be passed to this routine.
 *									*
 * Returns: RC_OK							*
 *									*
 ************************************************************************/
static int align_found;
static int alignment;
static RETCODE
update_align(STR *name)
{
    SYMBOL *sym;
    CSECT *cs;

    for (sym = name->first_ext_sym; sym; sym = sym->s_synonym) {
	if (sym->s_flags & S_DUPLICATE)	/* There must be non-duplicates, but
					   there could also be duplicate
					   symbols. */
	    continue;
	cs = sym->s_csect;

	/* Print a message if the verbose flag is set.
	   Do not list other labels in the csect. */
	if (Switches.verbose)
	    if (sym->s_flags & S_PRIMARY_LABEL)
		say(SAY_NORMAL, NLSMSG(ALIGN_UPDATE,
				       "%1$s: The alignment of %2$s was %3$d."),
		    Command_name, name->name, cs->c_align);
	    else
		say(SAY_NORMAL, NLSMSG(ALIGN_UPDATE2,
       "%1$s: The alignment of the csect containing the label %2$s was %3$d."),
		    Command_name, name->name, cs->c_align);

	cs->c_align = alignment;
	align_found = 1;
    }
    return RC_OK;
}
/************************************************************************
 * Name: align			Processor for the ALIGN control command	*
 *									*
 * Purpose: Set or display CSECT alignment.  The alignment factor is	*
 *	log2 value whith a valid range of 0 to 31.  This allows for	*
 *	alignment of 1 to 2,247,483,648.				*
 *									*
 *	If only an alignment factor is supplied, then this routine	*
 *	displays the name of all csects that have the specified	*
 *	alignment.							*
 *									*
 *	If both a pattern and alignment factor are supplied, then this	*
 *	routine sets the alignment for all csects that match the	*
 *	specified pattern. (Refer to the routine MATCH for a		*
 *	description of the pattern matching algorithm).			*
 *									*
 * Command Format:							*
 *	ALIGN pattern alignment						*
 *   or									*
 *	ALIGN alignment							*
 *									*
 * Parms/Returns:							*
 *	Input:	A - First or only parameter.				*
 *			CSECT pattern if B is not null.			*
 *			Alignment factor if B is null.			*
 *		B - Second parameter or null parameter.			*
 *			Alignment factor.				*
 *									*
 *	Returns: Returns a status completion code.			*
 *		0 - OK
 *		4 - WARNING No symbols match pattern
 *		12- SEVERE ERROR: Invalid argumments
 *									*
 * Side Effects:							*
 *	Causes messages to be displayed or placed on the LOAD MAP file	*
 *									*
 *	Modifies the c_align field of matching csects.
 *									*
 ************************************************************************/
int
align(char *arg[])			/* argv-style arguments */
{
    CSECT *cs;
    OBJECT *obj;
    SRCFILE *sf;

    if (arg[2]) {
	/* two arguments specified; operands: pattern alignment */
	if ((alignment = atoi(arg[2])) < 0 || alignment > 31) {
	    bind_err(SAY_NORMAL, RC_NI_SEVERE,
		     NLSMSG(ALIGN_BADARG,
	    "%1$s: 0711-611 SEVERE ERROR: %2$s: Invalid alignment.\n"
	    "\tValue must be in the range 0-31. Value specified is %3$s.\n"
	    "\tUSAGE: ALIGN [pattern] {0-31}"),
		     Main_command_name, Command_name, arg[2]);
	    return RC_NI_SEVERE;
	}

	align_found = 0;

	match(arg[1], MATCH_NO_NEWNAME, MATCH_EXT, update_align);

	if (align_found == 0) {
	    bind_err(SAY_NORMAL, RC_NI_WARNING,
		     NLSMSG(ALIGN_NOMATCH,
    "%1$s: 0711-613 WARNING: %2$s: No global symbols match pattern: %3$s"),
		     Main_command_name, Command_name, arg[1]);
	    return RC_NI_WARNING;
	}
	return RC_OK;
    }
    else {
	if ((alignment = atoi(arg[1])) < 0 || alignment > 31) {
	    bind_err(SAY_NORMAL, RC_NI_SEVERE,
		     NLSMSG(ALIGN_BADARG,
	    "%1$s: 0711-611 SEVERE ERROR: %2$s: Invalid alignment.\n"
	    "\tValue must be in the range 0-31. Value specified is %3$s.\n"
	    "\tUSAGE: ALIGN [pattern] {0-31}"),
		     Main_command_name, Command_name, arg[1]);
	    return RC_NI_SEVERE;
	}

	/* Check all csects for matches */
	for (obj = first_object(); obj; obj = obj->o_next) {
	    switch(obj->o_type) {
	      case O_T_OBJECT:
		for (sf = obj->oi_srcfiles; sf; sf = sf->sf_next) {
		    for (cs = sf->sf_csect; cs; cs = cs->c_next) {
			if (interrupt_flag) {
			    say(SAY_STDOUT,
				NLSMSG(MAIN_INTERRUPT,
		   "%1$s: 0711-635 WARNING: Binder command %2$s interrupted."),
				Main_command_name, Command_name);
			    return RC_WARNING;
			}
			if (cs->c_align == alignment) {
			    minidump_symbol(&cs->c_symbol,
					    /* Use shorter name than usual to
					       reduce chances of line wrap. */
					    MINIDUMP_NAME_LEN-5,
					    MINIDUMP_SYMNUM
					    | MINIDUMP_INPNDX
					    | MINIDUMP_TYPE
					    | MINIDUMP_SMCLASS
					    | MINIDUMP_CSECT_LEN_ALIGN
					    | MINIDUMP_SOURCE_INFO,
					    NULL);
			}
		    }
		}
	    }
	}

	return RC_OK;
    }
}
/************************************************************************
 * Name: lookup_smclass
 * 	Routine to transform a storage mapping class from a character
 *	string form (command input) to the binary values defined
 *	for the storage mapping class in "syms.h" of the xcoff.
 ***********************************************************************/
static struct {
    unsigned char	val;
    char		name[3];
} table[] = {
    {XMC_PR,	"PR"},
    {XMC_RO,	"RO"},
    {XMC_DB,	"DB"},
    {XMC_GL,	"GL"},
    {XMC_XO,	"XO"},
    {XMC_SV,	"SV"},
    {XMC_TI,	"TI"},
    {XMC_TB,	"TB"},
    {XMC_RW,	"RW"},
    {XMC_TC0,	"T0"},
    {XMC_TC,	"TC"},
    {XMC_TD,	"TD"},
    {XMC_DS,	"DS"},
    {XMC_UA,	"UA"},
    {XMC_BS,	"BS"},
    {XMC_UC,	"UC"},
    {0, ""}
};

static int
lookup_smclass(char *code)
{
    int i;

    upper(code);

    for (i = 0; table[i].name[0] != '\0'; i++)
	if (strcmp(code, table[i].name) == 0)
	    return table[i].val;

    bind_err(SAY_NO_NL, RC_NI_SEVERE,
	     NLSMSG(BAD_SMCLASS,
 "%1$s: 0711-614 SEVERE ERROR: Invalid storage-mapping class %2$s specified.\n"
 "\tValid storage-mapping classes are\n\t"),
	     Command_name, code);

    for (i = 0; table[i].name[0] != '\0'; i++)
	bind_err(SAY_NO_NL | SAY_NO_NLS, RC_NI_SEVERE, " %s", table[i].name);
    bind_err(SAY_NL_ONLY, RC_NI_SEVERE);

    return -1;
} /* lookup_smclass */
/************************************************************************
 * Name: update_smc
 *									*
 * Purpose:	Static function used to update storage-mapping classes
 *		for STG command.
 *									*
 * Returns: RC_OK							*
 *									*
 ************************************************************************/
static int smc_found;
static int smc;
static RETCODE
update_smc(STR *name)
{
    SYMBOL *sym, *sym2;

    for (sym = name->first_ext_sym; sym; sym = sym->s_synonym) {
	if (sym->s_flags & S_DUPLICATE)
	    continue;

	if (Switches.verbose)
	    say(SAY_NORMAL,
		NLSMSG(SMCLASS_UPDATE,
		       "%1$s: The storage-mapping class of %2$s was %3$s."),
		Command_name, name->name, get_smclass(sym->s_smclass));
	sym->s_smclass = smc;

	sym2 = &sym->s_csect->c_symbol;
	if (Switches.verbose && sym != sym2)
	    say(SAY_NORMAL,
		NLSMSG(SMCLASS2_UPDATE,
       "%1$s: The storage-mapping class of csect [%2$s]%3$s has been updated."),
		Command_name, show_sym(sym2, NULL), sym2->s_name->name);
	sym2->s_smclass = smc;

	for (sym2 = sym2->s_next_in_csect; sym2; sym2 = sym2->s_next_in_csect) {
	    if (Switches.verbose && sym != sym2)
		say(SAY_NORMAL,
		    NLSMSG(SMCLASS3_UPDATE,
   "%1$s: The storage-mapping class of label [%2$s]%3$s has been updated."),
		    Command_name, show_sym(sym2, NULL), sym2->s_name->name);
	    sym2->s_smclass = smc;
	}
    }
    smc_found = 1;
    return RC_OK;
}
/************************************************************************
 * Name: stgcls			Processor for the STG command		*
 *									*
 * Purpose: Display or set the storage-mapping class for symbols.
 *	When changing the storage-mapping class of an SD then all LDs
 *	within the csect must be the same type.  The storage-mapping
 *	of an LD must be the same as that of the containing SD.
 *									*
 * Command Format:							*
 *	STG		- Display symbols with no (UA) storage-mapping class.
 *   or									*
 *	STG smclass	- Display symbols with specified storage-mapping class.
 *   or									*
 *	STG pat smclass	- Set storage-mapping class of symbols matching
 *				the pattern to the specified class.
 *									*
 * Parms/Returns:							*
 *	Input:	PAT	- Null if request to display symbols with no	*
 *				storage code.				*
 *			- Display symbols with specified storage code	*
 *				if the second parm is null		*
 *			- Provides pattern for symbol names to change	*
 *				storage code.				*
 *		SMCLASS	- Provides storage code to change specified	*
 *				symbols.				*
 *									*
 *	Returns: Returns a status completion code.			*
 *		0 - OK							*
 *		8 - ERROR No symbols match pattern or invalid
 *			storage-mapping class specified
 *									*
 ************************************************************************/
RETCODE
stgcls(char *arg[])			/* argv-style arguments */
{
    int		i;
    STR		*nm;
    HASH_STR	*sroot, *shash;
    SYMBOL	*sym;
    OBJECT	*obj;
    SRCFILE	*sf;
    CSECT	*cs;

    if (arg[1] == NULL) {
	/* display UA symbols */
	smc = XMC_UA;
	goto display_smc;
    }

    if (arg[2] == NULL) {		/* display symbols with a given
					   storage-mapping class. */
	smc = lookup_smclass(arg[1]);
	if (smc == -1)
	    return RC_NI_ERROR;
      display_smc:
	/* Check all csects for matches */
	for (obj = first_object(); obj; obj = obj->o_next) {
	    switch(obj->o_type) {
	      case O_T_OBJECT:
		for (sf = obj->oi_srcfiles; sf; sf = sf->sf_next) {
		    for (cs = sf->sf_csect; cs; cs = cs->c_next) {
			if (interrupt_flag) {
			    say(SAY_STDOUT,
				NLSMSG(MAIN_INTERRUPT,
		   "%1$s: 0711-635 WARNING: Binder command %2$s interrupted."),
				Main_command_name, Command_name);
			    return RC_WARNING;
			}
			if (cs->c_symbol.s_smclass == smc) {
			    minidump_symbol(&cs->c_symbol,
					    /* Use shorter name than usual to
					       reduce chances of line wrap. */
					    MINIDUMP_NAME_LEN-5,
					    MINIDUMP_SYMNUM
					    | MINIDUMP_INPNDX
					    | MINIDUMP_TYPE
					    | MINIDUMP_SMCLASS
					    | MINIDUMP_CSECT_LEN_ALIGN
					    | MINIDUMP_SOURCE_INFO,
					    NULL);
			}
		    }
		}
	    }
	}
	return RC_OK;
    }

    /* Set storage-mapping class of symbols matching arg[1] to arg[2] */
    smc = lookup_smclass(arg[2]);
    if (smc == -1)
	return RC_NI_ERROR;

    match(arg[1], MATCH_NO_NEWNAME, MATCH_EXT, update_smc);

    if (smc_found == 0) {
	bind_err(SAY_NORMAL, RC_NI_WARNING,
		 NLSMSG(ALIGN_NOMATCH,
	"%1$s: 0711-613 WARNING: %2$s: No global symbols match pattern: %3$s"),
		 Main_command_name, Command_name, arg[1]);
    }
    return RC_OK;
} /* stgcls */
/************************************************************************
 * Name: origin			Processor for the ORIGIN command
 * Purpose: Display or change the text and data virtual origin values.
 * Returns: RC_OK
 *	    RC_SEVERE:  Bad or missing arguments
 ***********************************************************************/
RETCODE
origin(char *arg[])			/* argv-style arguments */
{
    int n;
    int page = 0;
    int org;

    Command_name = "ORIGIN";

    if (arg[1] != NULL) {
	if (arg[1][0] == 'p' || arg[1][0] == 'P') {
	    lower(arg[1]);
	    if (strcmp(arg[1], "page") == 0)
		page = 1;
	    if (arg[2] == NULL) {
		bind_err(SAY_NORMAL, RC_NI_SEVERE,
			 NLSMSG(CMD_NOARG,
	"%1$s: 0711-159 SEVERE ERROR: Binder command %2$s: Missing argument."),
			 Main_command_name, Command_name);
		return RC_NI_SEVERE;
	    }
	    else
		n = 2;
	}
	else
	    n = 1;

	if (arg[n]) {
	    if (arg[n][0] != '.' && arg[n][1] != '\0') {
		org = strtoul(arg[n], NULL, 0);
		if (page) {
		    if ((org & (Pagesize-1)) != 0) {
			bind_err(SAY_NORMAL, RC_NI_ERROR,
				 NLSMSG(ORIGIN_PAGE_BAD,
 "%1$s: 0711-276 ERROR: %2$s: A page origin must be a multiple of the\n"
	"\tpage size (%3$d)."),
				 Main_command_name, Command_name, Pagesize);
			return RC_NI_ERROR;
		    }
		    Bind_state.flags |= FLAGS_TEXT_PAGE_ADDR;
		}
		else
		    Bind_state.flags &= ~FLAGS_TEXT_PAGE_ADDR;
		temp_aout_hdr.o_text_start = org;
	    }
	    if (arg[n+1]) {
		org = strtoul(arg[n+1], NULL, 0);
		if (page) {
		    if ((org & (Pagesize-1)) != 0) {
			bind_err(SAY_NORMAL, RC_NI_ERROR,
				 NLSMSG(ORIGIN_PAGE_BAD,
	"%1$s: 0711-276 ERROR: %2$s: A page origin must be a multiple of the\n"
	"\tpage size (%3$d)."),
				 Main_command_name, Command_name, Pagesize);
			return RC_NI_ERROR;
		    }
		    Bind_state.flags |= FLAGS_DATA_PAGE_ADDR;
		}
		else
		    Bind_state.flags &= ~FLAGS_DATA_PAGE_ADDR;
		temp_aout_hdr.o_data_start = org;
	    }
	}
    }
    else {
	if (Bind_state.flags & FLAGS_TEXT_PAGE_ADDR)
	    say(SAY_NORMAL,
		NLSMSG(ORIGIN_PAGE_TEXT,
		       "%1$s:\t.text (page boundary) = %2$d (%2$8X hex)"),
		Command_name,
		temp_aout_hdr.o_text_start);
	else
	    say(SAY_NORMAL, NLSMSG(ORIGIN_TEXT,
				   "%1$s:\t.text = %2$d (%2$8X hex)"),
		Command_name, temp_aout_hdr.o_text_start);

	if (Bind_state.flags & FLAGS_DATA_PAGE_ADDR)
	    say(SAY_NORMAL,
		NLSMSG(ORIGIN_PAGE_DATA,
		       "\t.data (page boundary) = %1$d (%1$8X hex)"),
		temp_aout_hdr.o_data_start);
	else
	    say(SAY_NORMAL, NLSMSG(ORIGIN_DATA, "\t.data = %1$d (%1$8X hex)"),
		temp_aout_hdr.o_data_start);
    }
    return RC_OK;
}
/************************************************************************
 * Name: rc			Processor for the RC command
 * Purpose: Display the highest encountered return code from a binder command.
 * Returns RC_OK
 ***********************************************************************/
/*ARGSUSED*/
RETCODE
max_retcode(char *arg[])		/* argv-style arguments */
{
    say(SAY_NORMAL, NLSMSG(RC_QUERY, "%1$s: Highest return code was %2$d."),
	Command_name, Bind_state.retcode);
    return RC_OK;
}
/************************************************************************
 * Name: NOLIBPATH	Process for NOLIBPATH command
 * Purpose: Delete any existing value set by a previous libpath call
 *		The default value is restored.
 * Returns: RC_OK
 ***********************************************************************/
/*ARGSUSED*/
RETCODE
nolibpath(char *arg[])			/* argv-style arguments */
{
    if (Bind_state.libpath) {
	efree(Bind_state.libpath);
	Bind_state.libpath_len = 0;
	do_libpath(NULL, 0);		/* Display current value. */
    }

    return RC_OK;
} /* nolibpath */
/************************************************************************
 * Name: LIBPATH	Processor for LIBPATH command
 * Purpose: Set or display the libpath value to be written to loader section.
 * Returns: RC_OK
 *	NOTE:  The pathname is not checked for validity.
 *
 ***********************************************************************/
RETCODE do_libpath(char *, int);	/* Forward declaration */
RETCODE
libpath(char *arg[])			/* argv-style arguments */
{
    if (arg[1])				/* set libpath */
	return do_libpath(arg[1], arg[1][0] == '\\');
    else
	return do_libpath(NULL, 0);
} /* libpath */
/***********************************************************************/
RETCODE
do_libpath(char *libpath,
	   int has_escapes)
{
    char	*p;
    char	*id = "do_libpath";
    int		path_len;

    if (libpath) {			/* set libpath */
	path_len = strlen(libpath) + 1;
	if (path_len > Bind_state.libpath_len) { /* increase size */
	    if (Bind_state.libpath)
		efree(Bind_state.libpath);
	    Bind_state.libpath = emalloc(path_len, id);
	    Bind_state.libpath_len = path_len;
	}
	/* copy libpath */
	if (has_escapes)
	    (void) unescape_pathname(Bind_state.libpath, path_len, libpath);
	else
	    (void) strcpy(Bind_state.libpath, libpath);

	say(SAY_NORMAL, NLSMSG(LIBPATH_SET, "%1$s: Library path set to %2$s"),
	    Command_name, Bind_state.libpath);
    }
    else {				/* display current libpath */
	if ((p = Bind_state.libpath) == NULL
	    && (p = getenv("LIBPATH")) == NULL)
	    p = DEFLIBPATH;

	say(SAY_NORMAL,
	    NLSMSG(LIBPATH_QUERY, "%1$s: The library path is %2$s"),
	    Command_name, p);
    }

    return RC_OK;
} /* do_libpath */
/************************************************************************
 * Name: RENAME		Processor for the RENAME command		*
 *									*
 * Purpose: Rename a symbol.						*
 *									*
 * Command Format:							*
 *   RENAME oldname newname						*
 *
 * Arguments:								*
 *	oldname - Symbol to be renamed.					*
 *	newname - New symbol name.					*
 *									*
 *	Symbol renaming is effected by taking the three chains of symbols
 *	for oldname (for external symbols, hidden symbols, and external
 *	references) and merging them with the chains for newname.  The
 *	order of hidden symbols and external references doesn't matter,
 *	so one chain is appended to the other.  For external symbols, the
 *	two chains must be merged so that the final order is preserved
 *	based on the original order in which object files were read.  The
 *	"kept" or "exported" attributes of oldname are not affected by
 *	renaming.  If oldname was not exported, the, ISSYMBOL bit is reset
 *	for oldname.
 *
 *	If a symbol to be renamed is from an archive file, the archive
 *	member is read to be sure that all instances from the member are
 *	renamed.
 *
 * Returns: Returns a completion status code.				*
 *	OK	- no error detected.					*
 *	ERROR	- Couldn't find oldname
 *
 ************************************************************************/
RETCODE
rename_cmd(char *arg[])			/* argv-style arguments */
{
    STR		*oldname, *newname;
    SYMBOL	*oldsym, *newsym, head_sym, *prev_sym;
    OBJECT	*oldobj, *newobj;

    oldname = lookup_stringhash(arg[1]);
    if (oldname == NULL || !(oldname->flags & STR_ISSYMBOL)) {
	bind_err(SAY_NORMAL, RC_NI_ERROR,
		 NLSMSG(NAMES_NOTFOUND,
		"%1$s: 0711-673 ERROR: %2$s: Symbol %3$s was not found."),
		 Main_command_name, Command_name, arg[1]);
	return RC_NI_ERROR;
    }

    newname = putstring(arg[2]);
    newname->flags |= STR_ISSYMBOL;

    /* Read archive members for external symbols for oldname. */
    for (oldsym = oldname->first_ext_sym; oldsym; oldsym = oldsym->s_synonym) {
	if (oldsym->s_smtype == XTY_AR)
	    oldsym = read_archive_member_for_symbol(oldsym);
	oldsym->s_name = newname;	/* Rename symbol */
    }

    /* If newname has no external symbols, we just move the chain from
       oldname to newname. */
    if (newname->first_ext_sym == NULL)
	newname->first_ext_sym = oldname->first_ext_sym;
    else if (oldname->first_ext_sym != NULL) {
	/* Merge chains from both symbols by walking the chains. */

#define advance_sym(s,o) (o=((s=s->s_synonym)==NULL)?NULL\
  :(s->s_smtype==XTY_AR)?s->s_object:s->s_csect->c_srcfile->sf_object,s)

	oldsym = oldname->first_ext_sym;
	oldobj = oldsym->s_smtype == XTY_AR
	    ? oldsym->s_object : oldsym->s_csect->c_srcfile->sf_object;

	newsym = newname->first_ext_sym;
	newobj = newsym->s_smtype == XTY_AR
	    ? newsym->s_object : newsym->s_csect->c_srcfile->sf_object;

	head_sym.s_synonym = NULL;	/* Dummy symbol at head of new chain */
	prev_sym = &head_sym;		/* Current pointer to new chain. */

	while (oldobj && newobj) {
	    if (oldobj == newobj) {
		/* Symbols are from the same object file.
		   Use the input index to determine the order. */
		if (oldsym->s_inpndx < newsym->s_inpndx)
		    goto oldsym_first;
		else
		    goto newsym_first;
	    }
	    else {
		if (oldobj->o_ifile->i_ordinal == newobj->o_ifile->i_ordinal) {
		    /* Symbols are from same file, we can compare the objects
		       directly. */
		    if (oldobj < newobj)
			goto oldsym_first;
		    else
			goto newsym_first;
		}
		else if (oldobj->o_ifile->i_ordinal
			 < newobj->o_ifile->i_ordinal) {
		  oldsym_first:
		    prev_sym->s_synonym = oldsym;
		    prev_sym = oldsym;
		    oldsym = advance_sym(oldsym, oldobj);
		}
		else {
		  newsym_first:
		    prev_sym->s_synonym = newsym;
		    prev_sym = newsym;
		    newsym = advance_sym(newsym, newobj);
		}
	    }
	} /* while (oldobj && newobj) */

	if (oldobj)			/* Only oldname's symbols left. */
	    prev_sym->s_synonym = oldsym;
	else				/* Only newname's symbols left */
	    prev_sym->s_synonym = newsym;

	newname->first_ext_sym = head_sym.s_synonym;
    }
    oldname->first_ext_sym = NULL;

    /* Combine hidden symbols.  Order doesn't matter.
       First we rename all of oldname's hidden symbols to newname.
       Then we prepend oldname's chain of hidden symbols before
       newname's chain of hidden symbols. */
    prev_sym = NULL;
    for (oldsym = oldname->first_hid_sym; oldsym; oldsym = oldsym->s_synonym) {
	prev_sym = oldsym;
	oldsym->s_name = newname;		/* Rename symbol */
    }
    if (prev_sym)
	prev_sym->s_synonym = newname->first_hid_sym;
    newname->first_hid_sym = oldname->first_hid_sym;
    oldname->first_hid_sym = NULL;

    /* Combine ERs.  Order doesn't matter. */
    prev_sym = NULL;
    for (oldsym = oldname->refs; oldsym; oldsym = oldsym->s_synonym) {
	prev_sym = oldsym;
	oldsym->s_name = newname;		/* Rename symbol */
    }
    if (prev_sym)
	prev_sym->s_synonym = newname->refs;
    newname->refs = oldname->refs;
    oldname->refs = NULL;

    Bind_state.state |= STATE_RESOLVE_NEEDED;

    /* Unless oldname is exported, we reset the ISSYMBOL flag.  The STR_EXPORT
       and STR_KEEP bits can still be set for oldname, and are unaffected
       by the rename command. */
    if (!(oldname->flags & STR_EXPORT))
	oldname->flags &= ~STR_ISSYMBOL;

    return RC_OK;
} /* rename_cmd */
#ifdef STATS
/************************************************************************
 * Name: stats
 *									*
 * Purpose: Display memory usage statistics
 *									*
 * Command Format:							*
 *	STATS
 *									*
 * Arguments: none
 *									*
 * Returns:
 *	RC_OK	- no error detected.					*
 ************************************************************************/
/*ARGSUSED*/
RETCODE
stats(char *arg[])			/* argv-style arguments */
{
    int	i;

    extern void str_stats(void), stab_stats(void), TYPECHK_stats(void);
    extern size_t total_bytes_allocated;

#define f_mt_d "%-22s %9d %9d %9d %9d"

    say(SAY_NORMAL,
	NLSMSG(STATS_HEADER,
"\t\t     Allocations Allocated    In-Use   Maximum"));
/*   1  2         3         4         5         6
     7890123456789012345678901234567890123456789012 */
    for (i = 0; i < NUM_MEM_TYPES; i++) {
	if (Bind_state.memory[i].used > Bind_state.memory[i].maxused)
	    Bind_state.memory[i].maxused = Bind_state.memory[i].used;
	say(SAY_NO_NLS, f_mt_d,
	    mem_name[i],
	    Bind_state.memory[i].allocations,
	    Bind_state.memory[i].alloc,
	    Bind_state.memory[i].used,
	    Bind_state.memory[i].maxused);
    }
    say(SAY_NORMAL, NLSMSG(STATS_TOTAL1,
			   "\nSymbols: Generated %1$d\tERs %2$d\tTotal %3$d"),
	Bind_state.generated_symbols,
	total_ers_allocated(),
	total_symbols_allocated());

    say(SAY_NORMAL,
	NLSMSG(STATS_HEADER2,
"\n\t\t          In-Use   Maximum     Total    Undone"));
/*     1  2         3         4         5         6
       7890123456789012345678901234567890123456789012 */

    say(SAY_NO_NLS, f_mt_d,
	msg_get(NLSMSG(LIT_FILES, "Files")),
	Bind_state.open_cur, Bind_state.open_max,
	Bind_state.open_total, Bind_state.close_total);

    say(SAY_NO_NLS, f_mt_d,
	msg_get(NLSMSG(LIT_SHMAT_FILES, "Mapped with shmat()")),
	Bind_state.shmat_cur, Bind_state.shmat_max,
	Bind_state.shmat_total, Bind_state.shmdt_total);

    say(SAY_NO_NLS, f_mt_d,
	msg_get(NLSMSG(LIT_MMAP_FILES, "Mapped with mmap()")),
	Bind_state.mmap_cur, Bind_state.mmap_max,
	Bind_state.mmap_total, Bind_state.munmap_total);

    str_stats();
    TYPECHK_stats();
    stab_stats();
    return RC_OK;
}
#endif
