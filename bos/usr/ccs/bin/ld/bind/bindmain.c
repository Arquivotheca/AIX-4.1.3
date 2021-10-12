#if lint == 0 && CENTERLINE == 0
#pragma comment (copyright, "@(#)86	1.17  src/bos/usr/ccs/bin/ld/bind/bindmain.c, cmdld, bos41J, 9516B_all 4/20/95 17:02:56")
#endif

/*
 *   COMPONENT_NAME: CMDLD
 *
 *   MAIN ENTRY POINT:
 *		main
 *
 *   FUNCTIONS: exec_loop
 *		cleanup
 *
 *   STATIC FUNCTIONS:
 *		display_prompt
 *		check_getc_errno
 *		get_line
 *		prompt_user
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <locale.h>

extern int getpagesize(void);		/* No prototype in standard hdr file */

#if lint || XLC_TYPCHK_BUG
#include "bind.h"
#endif

#include "global.h"
#include "error.h"
#include "strs.h"
#include "commands.h"
#include "util.h"

#define MAX_NESTING_LEVEL 64		/* Max recursion depth for exec cmd. */
/* Global variables */
#ifdef DEBUG
int	bind_debug;
#endif
int	Pagesize;
char	*Main_command_name;

/* Static variables */
static int	nesting_level = -1;	/* Nesting level of exec_loop */
static int	cmd_input_interrupted;	/* Reset upon successful command
					   input.  Used to detect two
					   consecutive ^C's without any
					   intervening input.  */

/* Input file names:
   NULL means input is from the outermost loop (always standard input).
   ""   means input is from /dev/tty (from exec - call)
   anything else is a real file name */
static char	*input_file_name;	/* Name of current input file name */
static char	*previous_file_name;	/* Name of previous input file name
					   if we're executing a recursive
					   exec_loop. Undefined if
					   nesting_level == 0. */

/************************************************************************
 * Name: main				Binder program (main entry point)
 *									*
 * PURPOSE: Combine and link object files into a relocatable,		*
 *	executable, and relinkable load module.				*
 *									*
 * RETURNS: Does not return directly.  Returns via call to cleanup().
 *	Possible exit values are:
 *		RC_OK (0):	No errors or warnings were detected.
 *		RC_WARNING (4):	Warning messages were printed.
 *		RC_ERROR (8):	Error messages were printed.
 *		RC_SEVERE (12):	A severe error occurred, probably the
 *				result of an error in another program (one
 *				that writes XCOFF files, for example) or the
 *				result of the user modifying an input file.
 *		RC_PGMERR (16):	An internal error occurred.  This is always
 *				a bug in the code, and should be reported.
 *	In addition, the low order bit is set if an output file was not
 *	successfully written.
 *
 ************************************************************************/
int
main(int argc,				/* argument count */
     char *argv[])			/* arguments */
{
    int	cmd_ret;

    (void) setlocale(LC_ALL, "");
    (void) setlocale(LC_CTYPE, "C");

#ifndef NO_NLS
    catd = catopen("bind.cat", NL_CAT_LOCALE);
#endif

#ifdef DEBUG
    {
	/* Read debug variable from environment */
	char *s = getenv("BIND_DEBUG");
	if (s)
	    bind_debug = strtoul(s, NULL, 0);
	else
	    bind_debug = 0;
    }
#endif

    /* Set "stdin_is_tty" and "interactive" flags if stdin is a TTY. */
    if (isatty(STDIN_FILENO)) {
	Bind_state.tty_file = stdin;	/* File descriptor we can use to
					   prompt user after "exec -" and
					   ^C in the middle of an "exec"
					   command.  */
	Bind_state.stdin_is_tty = 1;
	Bind_state.interactive = 1;
	Bind_state.ever_interactive = 1;
	Main_command_name = "bind";
    }
    else
	Main_command_name = "ld";

    /* See if output is redirected. */
    if (!isatty(STDOUT_FILENO))
	Bind_state.stdout_is_tty = 0;

    /* Initialize. */
    Pagesize = getpagesize();
    init_ifiles();
    init_save();
    init_objects();
    init_signals();			/* Catch signals for cleanup. */

    /* Initialize default entry point in Bind_state. */
    Bind_state.entrypoint = putstring(DEFAULT_ENTRYPOINT);
    Bind_state.entrypoint->flags |= STR_ENTRY;

    /* If arguments exist, they are SETOPT operands, so process them. */
    if (argc > 1) {
	char *saved_argv0 = argv[0];
	argv[0] = NULL;			/* Clear argv[0] to tell bindopt() that
					   we're processing the options on the
					   command line.  */
	cmd_ret = bindopt(argv);
	argv[0] = saved_argv0;		/* Restore value,
					   or else ps gets confused.  */

	if (Bind_state.interactive == 0 && cmd_ret > Bind_state.err_exit_lev)
	    cleanup(cmd_ret);
	/*NOTREACHED*/
    }

    /* The return code from exec_loop will be saved in Bind_state.retcode. */
    (void) exec_loop(stdin, NULL);

    cleanup(Bind_state.retcode);
    /*NOTREACHED*/
} /* main */
/************************************************************************
 * Name: display_prompt
 *									*
 * Purpose: Write the prompt to the loadmap file, and to standard output
 *	if input if not from a tty.  The prompt is preceeded by "+"
 *	characters to indicate the nesting level.
 *
 * Arguments:
 *	j:	If 1, we must be running interactively, and the prompt is being
 *		displayed before a command has been read.  Therefore, the
 *		prompt is only displayed to stdout.
 *
 *		If 0, the prompt is being displayed after a command has
 *		been read.  If we are running interactively, the prompt is not
 *		displayed on stdout, because a previous call to this function
 *		displayed the prompt.
 *
 * Note:  The prompt is written before the next input line is read if we're
 *	currently reading from a tty.  Otherwise, the prompt is written
 *	after the next input line is read, and is not written at all if
 *	end-of-file is reached on stdin.
 ************************************************************************/
static void
display_prompt(int j)
{
    int i;
    int mask = SAY_NO_NLS | SAY_NO_NL;

    if (j != 0)
	mask |= SAY_STDOUT | SAY_NOLDMAP;
    else if (Bind_state.stdin_is_tty) {
	/* The prompt has already been displayed.  We must not write the
	   prompt to stdout, but only to the loadmap */
	mask |= SAY_NOSTDOUT;
    }

    for (i = 0; i < nesting_level; i++)
	say(mask, "+");
    say(mask, "%s", Bind_state.prompt);
}
/************************************************************************
 * Name: check_getc_errno
 *									*
 * Purpose: If getc() returns an error (-1), we check errno for the error.
 *	(We will have already checked to be sure we're not at the end of the
 *	file, so errno can be used.)  We return when EINTR is the error.
 *	Otherwise, a message is printed, and we call cleanup.
 *
 * Arguments:
 *	fname:	File being read--stdin, if fname == NULL
 *				 /dev/tty, if *fname == '\0'
*				an actual file name, otherwise
 ************************************************************************/
static void
check_getc_errno(char *fname)
{
	switch(errno) {
	  case EINTR:
	    return;			/* Interrupt (^C or ^Z) was received.
					   Return and input will be retried. */
	    /*NOTREACHED*/

	  case EIO:
	    cleanup(Bind_state.retcode); /* Parent was killed--exit silently.*/
	    /*NOTREACHED*/

	  default:
	    /* All other values of errno are unexpected and result in
	       cleanup() being called. */
	    bind_err(SAY_NL_ONLY, RC_SEVERE);
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(BAD_SYSCALL,
	    "%1$s: 0711-999 SEVERE ERROR: Unexpected system call error.\n"
	    "\t%1$s:%2$s() %3$s"),
		     Main_command_name, "getc", strerror(errno));
	    if (fname == NULL)
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(WHILE_READING_STDIN,
 "%s: 0711-989 Error occurred while reading binder commands from standard input."),
			 Main_command_name);
	    else if (*fname == '\0')
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(WHILE_READING_KEYBOARD,
 "%s: 0711-985 Error occurred while reading binder commands from the keyboard."),
			 Main_command_name);
	    else
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(WHILE_READING_FILE,
 "%1$s: 0711-988 Error occurred while reading binder commands from file: %2$s"),
			 Main_command_name, fname);
	    cleanup(RC_SEVERE);
	}
}
/************************************************************************
 * Name: get_line
 *									*
 * Purpose: Read the next line from the input file..
 *									*
 * Returns: -2: if EOF was reached (without any other input);
 *	    -1: if display_prompt() was called;
 *	    Length of returned line, otherwise.
 *
 * NOTE:  If -2 or -1 is returned, *result will be set to an error code,
 *	as defined by the RC_... macros.
 *									*
 ************************************************************************/
static int
get_line(FILE *fp,			/* IN:  File to read from */
	 char *fname,			/* IN:  Input file name (for errors) */
	 int max_len,			/* IN:  Length of array */
	 char *command[],		/* OUT: Pointer to buffer for saving
					   input line.  If line is longer than
					   max_len, a new buffer will be
					   allocated, which should be freed by
					   the caller. */
	 int *result)			/* OUT: Return code */
{
    int len = 0;
    int c;
    static char *id = "get_line";
    char *temp_buffer = NULL;

    while (1) {
	if (len == max_len) {
	    max_len *= 2;
	    if (temp_buffer)
		temp_buffer = erealloc(temp_buffer,  max_len, id);
	    else {
		temp_buffer = emalloc(max_len, id);
		memcpy(temp_buffer, *command, len);
		*command = temp_buffer;
	    }
	}
	(*command)[len] = c = getc(fp);	/* Read next character.  */
	if (c == '\n')			/* Complete line found. */
	    break;
	else if (c == EOF) {
	    if (feof(fp)) {
		clearerr(fp);		/* Reset bits in case input is stdin */
		/* End of file */
		if (len == 0) {
		    if (Bind_state.stdin_is_tty) {
			/* Write newline after last prompt. */
			say(SAY_NL_ONLY | SAY_STDOUT | SAY_NOLDMAP);
		    }
		    *result = RC_OK;
		    return -2;
		}
		else {
		    /* EOF reached after some characters input.  The last
		       line must be incomplete, but we'll still process it. */
		    break;
		}
	    }
	    /* Some signal must have been caught, or a real error occurred.
	       If ^C was caught and we're reading commands from a file,
	       the rest of the line is read anyway.  The command won't be
	       processed in this case.

	       If ^C was caught and we're reading from stdin, any partial
	       line is cleared, and we return, to allow the prompt to
	       be redisplayed.

	       If two ^C's are seen in a row without any complete line
	       being seen in the interim, a message is printed instructing
	       the user how to exit from the binder. */
	    check_getc_errno(fname);
	    stop_flag = 0;		/* Reset, in case it was set */
	    if (interrupt_flag) {
		interrupt_flag = 0;	/* Clear flag to prevent "interrupted"
					   message from being printed.  */
		if (!Bind_state.stdin_is_tty)
		    continue;	/* Keep reading */

		/* If we get 2 interrupts in a row, display message telling
		   how to exit the binder */
		if (cmd_input_interrupted) {
		    if (nesting_level == 0)
			say(SAY_STDOUT | SAY_NOLDMAP,
			    NLSMSG(MAIN_QUIT,
				   "Type\n"
				   "\t\tquit\n"
				   "\tto terminate the binder."));
		    else if (previous_file_name == NULL
			     || *previous_file_name == '\0') {
			say(SAY_STDOUT | SAY_NOLDMAP,
			    NLSMSG(MAIN_QUIT_ABORT2,
		   "Type\n"
		   "\t\tabort\n"
		   "\tto terminate the binder or\n"
		   "\t\tquit\n"
		   "\tto exit recursive EXEC binder command execution."));
		    }
		    else
			say(SAY_STDOUT | SAY_NOLDMAP,
			    NLSMSG(MAIN_QUIT_ABORT,
		   "Type\n"
		   "\t\tabort\n"
		   "\tto terminate the binder or\n"
		   "\t\tquit\n"
		   "\tto continue executing binder commands from file %s"),
			    previous_file_name);
		}
		else
		    cmd_input_interrupted = 1;
	    }

	    if (Bind_state.stdin_is_tty) {
		(*command)[0] = '\0';		/* Nullify command */
		return 0;		/* We'll need to reprompt */
	    }
	}
	else if (c == '\0') {
	    display_prompt(0);
	    if (Bind_state.interactive) {
		/* Null character entered from keyboard.  */
		say(SAY_STDOUT, NLSMSG(MAIN_NULLS_KEYBOARD,
 "%s: 0711-636 WARNING: Null characters are not allowed in a binder command."),
			 Main_command_name);
		*result = RC_OK;	/* We ignore bad line and continue */
		/* Read to a newline or the end-of-file. */
		while ((c = getc(fp)) != '\n')
		    if (c == EOF)
			if feof(fp) {
			    clearerr(fp); /* Reset in case input is stdin */
			    return -2;	/* EOF */
			}
			else
			    check_getc_errno(fname);
		return -1;
	    }
	    else {
		if (fname != NULL)
		    bind_err(SAY_NORMAL, RC_SEVERE,
			     /* This message has an embedded message number, so
				that the first line prints after the prompt and
				the error number prints on the next line. */
			     NLSMSG(MAIN_NULLS_FILE,
		    "Binder command contains null characters.\n"
		    "%1$s: 0711-638 SEVERE ERROR Check input file %2$s\n"
		    "\tThe remaining commands in the file are being ignored."),
			     Main_command_name, fname);
		else
		    bind_err(SAY_NORMAL, RC_SEVERE,
			     /* This message has an embedded message number, so
				that the first line prints after the prompt and
				the error number prints on the next line. */
			     NLSMSG(MAIN_NULLS_STDIN,
		    "Binder command contains null characters.\n"
		    "%1$s: 0711-637 SEVERE ERROR Check standard input.\n"
		    "\tThe remaining commands are being ignored."),
			     Main_command_name);
		*result = RC_SEVERE;
		return -2;		/* Simulate EOF to ignore file */
	    }
	}
	len++;
    }
    /* We got a good line.  Reset cmd_input_interrupted.  We will leave
       interrupt_flag set if ^C was seen while reading from a command file.  */
    cmd_input_interrupted = 0;
    (*command)[len] = '\0';		/* Terminate command */
    return len;
}
/************************************************************************
 * Name: prompt_user
 *									*
 * Purpose: Called in interactive mode only.
 *
 *	Prints message if a command was interrupted and queries user about
 *	continuing.  Displays the prompt if input is being read from the
 *	keyboard.
 *									*
 * ARGUMENTS:
 *	fname:	Name of file from which commands are being read.
 *
 * Returns: RC_OK if a prompt was displayed, or if a command was interrupted,
 *		but user answered 'y' (or language equivalent) to query.
 *	    RC_ABORT if user answers 'abort' to query.  (This return value will
 *		be propagated to all recursive command loops, and the binder
 *		will exit.)
 *	    RC_QUIT: if user answers 'n' (or language equivalent) or a
 *			blank line to query.
 *
 ************************************************************************/
static RETCODE
prompt_user(char *fname)
{
    char	cmd_buf[BUFSIZ];

    /* Print message if interrupted in interactive mode */
    if (interrupt_flag) {
	interrupt_flag = 0;
	if (nesting_level > 0
	    && Bind_state.tty_file /* We must be interactive at some level, */
	    && !Bind_state.stdin_is_tty /* but not at the current level. */) {
	    if (!Bind_state.stdin_is_tty)
		say(SAY_STDOUT,
		    NLSMSG(MAIN_INTERRUPT2,
		   "%1$s: 0711-639 WARNING: Binder commands interrupted."),
		    Main_command_name);
	    while (1) {
#ifdef DEBUG
		if (fname == NULL || *fname == '\0')
		    internal_error();
#endif
		say(SAY_NO_NL | SAY_NOLDMAP | SAY_STDOUT,
		    NLSMSG(MAIN_CONTINUE,
	   "Continue with remaining binder commands in file %s [NYA]? "),
		    fname);

		if (Bind_state.stdout_is_tty == 0) {
		    /* Flush standard output in case tee is being used. */
		    while (fflush(stdout) == EOF && errno == EINTR)
			 /* Skip */;	/* Ignore errors (retry for EINTR) */
		}
		if (fgets(cmd_buf, sizeof(cmd_buf), Bind_state.tty_file)
		    == NULL) {
		    if (stop_flag || interrupt_flag) {
			stop_flag = interrupt_flag = 0;
			continue;
		    }
		    /* EOF reached (^D).  Treat as if we say newline only */
		    clearerr(Bind_state.tty_file);
		    return RC_QUIT;
		}
		if (cmd_buf[0] == '\n') {
		    /* newline only--default response is no */
		    return RC_QUIT;
		}
		else switch(rpmatch(cmd_buf)) {
		  case 1:		/* Yes */
		    return RC_OK;
		  case 0:
		    return RC_QUIT;	/* No */
		  default:
		    upper(cmd_buf);
		    if (strncmp(cmd_buf,
				msg_get(NLSMSG(LIT_ABORT, "ABORT")),
				strlen(cmd_buf) - 1/* Don't compare newline */)
						      == 0)
			return RC_ABORT;
		    else
			say(SAY_NOLDMAP | SAY_STDOUT,
			    NLSMSG(MAIN_QUERY,
				   "Please answer NO or YES or ABORT."));
		}
	    }
	}
    }

    stop_flag = 0;		/* Clear flag (in case it was set) */

    /* If current input is from tty, always display prompt,
       regardless of setting of "quiet" flag. */
    if (Bind_state.stdin_is_tty) {
	display_prompt(1 /* Prompting before reading command */);
	if (Bind_state.stdout_is_tty == 0) {
	    /* Flush standard output in case tee is being used. */
	    while (fflush(stdout) == EOF && errno == EINTR)
		/* Skip */ ;		/* Ignore errors (retry for EINTR) */
	}
    }


    return RC_OK;
}
/************************************************************************
 * Name: exec_loop
 *									*
 * Purpose: Main command loop--process commands until:
 *	   1. EOF is reached in the current input file
 *	or 2. A command returns RC_QUIT or RC_ABORT.
 *	or 3. A command returns a return code > Bind_state.err_exit_lev
 *	or 4. We are executing commands from a file in interactive mode, ^C is
 *	   typed, and the uses answers "no" when asked to continue with the
 *	   commands in the file.
 *									*
 * Returns: RC_OK if a prompt was displayed, or if a command was interrupted,
 *		but user answered 'y' to query.
 *	    RC_ABORT if user answers 'a' to query.  (This value will be
 *		propagated to all recursive command loops.)
 *	    RC_QUIT: if user answers 'n' to query.
 *
 ************************************************************************/
RETCODE
exec_loop(FILE *fp,			/* IN: File to read commands from */
	  char *fname)			/* IN: Name of file (for messages) */
{
    int		cmd_ret;
    char	command_buf[BUFSIZ+1];
    char	*command = &command_buf[0];
    char	*outer_file_name;
    int		len;

    if (nesting_level++ > MAX_NESTING_LEVEL) {
	bind_err(SAY_NORMAL, RC_NI_SEVERE,
		 NLSMSG(NESTING_TOO_DEEP,
 "%1$s: 0711-634 SEVERE ERROR: EXEC binder commands nested too deeply.\n"
 "\tMaximum recursion level is %2$d. Returning to previous level."),
		 Main_command_name, MAX_NESTING_LEVEL);
	nesting_level--;
	return RC_NI_SEVERE;
    }

    outer_file_name = previous_file_name; /* 2nd previous fname in stack */
    previous_file_name = input_file_name; /* 1st previous fname in static */
    input_file_name = fname;		/* Save name for messages */
    cmd_input_interrupted = 0;

    do {
	if (Bind_state.interactive) {
	    if ((cmd_ret = prompt_user(fname)) != RC_OK)
		/* This call recovers from ^C and prompts the user if input is
		   from a tty.  If prompt_user doesn't return RC_OK, an exec
		   command was interrupted, and the user chose not to continue
		   executing commands from the file, so we can return. */
		break;
	}
	len = get_line(fp, fname, sizeof(command_buf), &command, &cmd_ret);
	if (len >= 0) {
	    /* Reset loadmap error flag so we see loadmap errors once per
	       command (if running interactively). */
	    if (Bind_state.interactive && len > 0)
		Bind_state.loadmap_err = 0;

	    /* Write prompt if not already written */
	    display_prompt(0);
	    if (interrupt_flag)
		cmd_ret = RC_OK;	/* We might print
					   "Command interrupted" */
	    else {
		cmd_ret = do_command(command,
				     Bind_state.stdout_is_tty == 0 ||
				     Bind_state.stdin_is_tty == 0 /* Echo? */);
	    }
	}
	/* Save maximum return code */
	if (cmd_ret > RC_OK) {
	    if (cmd_ret > RC_NI_BASE)
		if (Bind_state.stdin_is_tty)
		    cmd_ret = RC_OK;
		else
		    cmd_ret -= RC_NI_BASE;
	    if (cmd_ret > RC_OK)
		if (Command_name == NULL) {
		    say(SAY_NORMAL,
			NLSMSG(MAIN_RETCODE2, "The return code is %d."),
			cmd_ret);
		}
		else {
		    say(SAY_NORMAL,
			NLSMSG(MAIN_RETCODE, "%1$s: The return code is %2$d."),
			Command_name, cmd_ret);
		}
	    if (cmd_ret > Bind_state.retcode)
		Bind_state.retcode = cmd_ret;
	}

	/* Now we can free command, if necessary (Command_name might be
	   using its storage). */
	if (command != &command_buf[0]) {
	    efree(command);
	    command = &command_buf[0];
	}
    } while (len >= -1
	     && (Bind_state.stdin_is_tty || cmd_ret <= Bind_state.err_exit_lev)
	     && cmd_ret >= RC_OK);

    nesting_level--;
    /* Restore values of static variables. */
    input_file_name = previous_file_name;
    previous_file_name = outer_file_name;

    return cmd_ret;			/* RC_ABORT is only meaningful value.
					   Other return values can always be
					   obtained from Bind_state.retcode  */
}
/************************************************************************
 * Name: cleanup
 *									*
 * Purpose: Close files and delete temporary files before exiting.
 *		We don't have to check for the xref file, because it will
 *		be closed when we exit.
 *									*
 * ARGUMENTS:
 *	rc:	binder exit code--passed to exit()
 *
 * Returns:  Does not return.
 *
 * If interrupted, or if some exception is caught (e.g. bus error), cleanup
 * can be called again.  We use cleanup_state to ensure that progress is made
 * with each call, in case of a signal_handler-cleanup()-signal_handler() loop.
 * If cleanup is called enough failures occur so that _exit() is called
 * explicitly, files may not be closed properly.
 *
 ************************************************************************/
void
cleanup(RETCODE rc)
{
    int		rc1;
    static int	cleanup_state = 0;

    switch(cleanup_state) {
      case 0:				/* Close temp. output file, if open */
	cleanup_state++;
	if (Bind_state.temp_out_Fd > 0)
	    while (close(Bind_state.temp_out_Fd) == EOF && errno == EINTR)
		/* skip */;

      case 1:				/* Unlink temp output file, if needed*/
	cleanup_state++;
	if (Bind_state.flags & FLAGS_UNLINK_TMP)
	    (void) unlink(Bind_state.out_tmp_name);

      case 2:				/* Close output file, if open */
	cleanup_state++;
	if (Bind_state.out_Fd > 0)
	    while(close(Bind_state.out_Fd) == EOF && errno == EINTR)
		/* skip */;

      case 3:				/* Unlink output file, if needed */
	cleanup_state++;
	if (Bind_state.flags & FLAGS_UNLINK_OUT)
	    (void) unlink(Bind_state.out_name);

      case 4:				/* Stop LOADMAP so it will be saved */
	cleanup_state++;
	rc1 = loadmap_control(LOADMAP_END);
	if (rc1 > rc)
	    rc = rc1;

      case 5:
	cleanup_state++;
	exit(rc | ((Bind_state.state & STATE_SAVE_COMPLETE) == 0)); /* Exit */
    }

    /* Last-ditch effort. */
    _exit(rc | ((Bind_state.state & STATE_SAVE_COMPLETE) == 0));
}
