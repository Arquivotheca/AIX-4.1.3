#if lint == 0 && CENTERLINE == 0
#pragma comment (copyright, "@(#)92	1.12  src/bos/usr/ccs/bin/ld/bind/error.c, cmdld, bos411, 9428A410j 6/3/94 14:43:58")
#endif

/*
 *   COMPONENT_NAME: CMDLD
 *
 *   FUNCTIONS: bind_err
 *		internal_err
 *		loadmap_control
 *		msg_get
 *		say
 *		show_loadmap_message
 *		write_stuff
 *
 *   STATIC FUNCTIONS:
 *		show_error
 *		write_to_file
 *
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
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/stat.h>

#include "global.h"
#include "error.h"
#include "util.h"

#if lint || XLC_TYPCHK_BUG
#include "bind.h"
#include "strs.h"
#endif

#define DEFAULT_LOADMAP_NAME "load.map"

/* Static variables */
static FILE	*loadmap_fp = NULL;
static char	stdout_err = 0;

/* Global variables */
int		loadmap_message_seen = 0;
int		stdout_is_tty = 1;
#ifndef NO_NLS
nl_catd		catd;
#endif

#ifdef NO_NLS
#define def_message(id) id
#define catgets(c,s,m,d) d
#else
/*******************************************************
 * Declare an array to hold the default messages.  The text of the
 * messages is read from a file generated from the message catalog.
 ********************************************************/
static char *default_messages[] = {
    NULL				/* Placeholder--do not delete
					   --do not add a comma.  */
#    include "bind_msg.c"
};

/* Compute number of messages. */
static long max_message_number
    = (long)(sizeof(default_messages)/sizeof(char *) - 1);

/* Define a macro to get the default message from default_messages[].
   Validate the message number and return a pointer to the default message.
   If the message number if out of range, the macro returns
NLSMSG(BIND_MSG_ERROR,	"ld: 0711-100 INTERNAL MESSAGE ERROR.\n\
\tDepending on where this product was acquired, contact your service\n\
\trepresentative or the approved supplier.")
   */
#define def_message(id) \
    default_messages[(id<2||id>max_message_number)?BIND_MSG_ERROR:id]

/************************************************************************
 * Name: msg_get
 *
 * Purpose: Lookup message number "msg".
 *
 * Returns: Desired message.
 *
 * NOTE:  If NO_NLS is defined, this function is defined as a
 *	macro in error.h
 *
 ************************************************************************/
char *
msg_get(int msg)
{
    return catgets(catd, 1, msg, def_message(msg));
}
#endif /* NO_NLS */

/************************************************************************
 * Name: show_error
 *
 * Purpose: Print an error message to stderr after an error occurs
 *	while writing to a file.  The file will usually be the loadmap
 *	file, but may also be stdout or stderr, if either one is being
 *	redirected.
 *
 *	The only expected error is a file-system-full one (or a similar one).
 *
 ************************************************************************/
static void
show_error(FILE *f,			/* File with error */
	   int closing,			/* 1 if error occurred closing file */
	   char *syscall)		/* System call resulting in error */
{
    char *msg1, *msg2;
    char *f_name;
    char *e1, *e2;

    if (f == stderr)
	return;				/* We cannot report an error to stderr,
					   so we just return. */
    e1 = NULL;
    Bind_state.cmd_err_lev = max(Bind_state.cmd_err_lev, RC_SEVERE);
    switch(errno) {
      case ENOSPC:
	msg1 = msg_get(NLSMSG(FILE_SYSTEM_FULL,
	     "%s: 0711-750 SEVERE ERROR: The file system is full."));
	break;
      case EDQUOT:
	msg1 = msg_get(NLSMSG(FILE_QUOTA,
	     "%s: 0711-751 SEVERE ERROR: You have exceeded your disk quota."));
	break;
      case EFBIG:
	msg1 = msg_get(NLSMSG(FILE_ULIMIT,
     "%s: 0711-752 SEVERE ERROR: You have exceeded your filesize ulimit."));
	break;
      default:
	e1 = syscall;
	e2 = strerror(errno);
	msg1 = msg_get(NLSMSG(BAD_SYSCALL,
	     "%1$s: 0711-999 SEVERE ERROR: Unexpected system call error.\n"
			     "\t%1$s:%2$s() %3$s"));
    }

    if (f != stdout) {
	if (Bind_state.loadmap_err)
	    return;			/* We've already printed a message */
	Bind_state.loadmap_err = 1;
	f_name = Bind_state.loadmap_fn;

	if (closing)
	    msg2 = msg_get(NLSMSG(WHILE_CLOSING_LOADMAP,
	 "%1$s: 0711-990 Error occurred while closing loadmap file: %2$s"));
	else
	    msg2 = msg_get(NLSMSG(WHILE_WRITING_LOADMAP,
  "%1$s: 0711-991 Error occurred while writing to the loadmap file: %2$s"));

	/* Flush standard output before printing errors */
	while (fflush(stdout) == EOF && errno == EINTR)
	    /* Skip */ ;		/* Ignore errors (retry for EINTR) */
    }
    else {
	if (stdout_err)
	    return;			/* We've already printed a message */
	stdout_err = 1;
	f_name = NULL;
	msg2 = msg_get(NLSMSG(WHILE_WR_STDOUT_LOADMAP,
	     "%s: 0711-992 Error occurred while writing to standard output."));
    }

    /* In all the follow fprintf() and fflush() calls, we ignore errors
       except for EINTR, for which we retry the call. */
    while (fprintf(stderr, msg1, Main_command_name, e1, e2) == EOF
	   && errno == EINTR)
	/* Skip */ ;
    while (putc('\n', stderr) == EOF && errno == EINTR)
	/* Skip */ ;

    while (fprintf(stderr, msg2, Main_command_name, f_name) == EOF
	   && errno == EINTR)
	/* Skip */ ;
    while (putc('\n', stderr) == EOF && errno == EINTR)
	/* Skip */ ;

    /* Flush stderr */
    while (fflush(stderr) == EOF && errno == EINTR)
	/* Skip */ ;

    return;
}
/************************************************************************
 * FUNCTION:  loadmap_control
 *	Control the output of the map (or log) file.
 * Arguments:
 *	flag:  The follow values are defined:
 *		LOADMAP_STOP: Stop writing to map file and flush its buffer.
 *			The file won't be closed until loadmap_control is
 *			called again with flag==LOADMAP_END or with
 *			flag==LOADMAP_START and a different file name.
 *			Therefore, logging to a single loadmap file can be
 *			toggled on and off.
 *		LOADMAP_START: Begin writing to file "fname".  If a previous
 *			loadmap file is open, and "fname" is a new name, close
 *			the previous file.  Otherwise, continue writing to
 *			the previously opened file.
 *		LOADMAP_END:  Stop writing to the loadmap file and close it.
 *
 *	fname (optional second argument, if flag == LOADMAP_START):
 *		File name to write to.
 *
 * RETURN CODE:
 *	RC_ERROR:  for LOADMAP_START, if file cannot be opened for writing
 *		   for LOADMAP_END, if the fclose() call fails
 *	RC_OK: Otherwise
 ************************************************************************/
int
loadmap_control(LOADMAP_OPTION flag,	/* LOADMAP_STOP, LOADMAP_START, or
					   LOADMAP_END  */
		... )			/* File name, for LOADMAP_START only */
{
    va_list	ap;
    char	*fname;
    int		names_bits;
    int		len;

    switch(flag) {
      case LOADMAP_STOP:		/* "setopt noloadmap" */
	if (Switches.loadmap) {
	    Switches.loadmap = 0; /* Stop output to loadmap file */
	    if (loadmap_fp == NULL)
		return RC_OK;		/* File wasn't really open */
	    while (fflush(loadmap_fp) == EOF) {
		if (errno != EINTR) {
		    show_error(loadmap_fp, 0 /* Not closing file */, "fflush");
		    break;
		}
	    }
	}
	break;

      case LOADMAP_END:		/* Terminate loadmap, if necessary  */
	if (!Switches.loadmap || loadmap_fp == NULL)
	    return RC_OK;

	/* Close the file */
	while (fclose(loadmap_fp) == EOF) {
	    switch(errno) {
	      case EIO:
	      case EPIPE:
		return RC_OK;		/* Ignore */
	      case EINTR:
		break;
	      default:
		show_error(loadmap_fp, 1 /* Closing file */, "fclose");
		loadmap_fp = NULL;
		return RC_ERROR;
	    }
	}
	loadmap_fp = NULL;
	Switches.loadmap = 0;
	break;

      case LOADMAP_START:	/* "setopt loadmap" or "setopt loadmap:<fn>" */
	/* Start or restart LOADMAP output--if new filename specified,
	   save old loadmap output */

	/* Get file name */
	va_start(ap, flag);
	fname = va_arg(ap, char *);
	va_end(ap);

	len = strlen(fname);

	if (len > PATH_MAX) {
	  name_too_long:
	    bind_err(SAY_NORMAL, RC_NI_SEVERE,
		     NLSMSG(FILE_NAMETOOLONG,
		"%1$s: 0711-878 SEVERE ERROR: The pathname is too long.\n"
		"\tThe maximum length of a pathname is %2$d.\n"
		"\tBinder command %3$s cannot be executed."),
		     Main_command_name, PATH_MAX, "SETOPT LOADMAP");
	    return RC_NI_SEVERE;
	}

	/* Set a mask with the following bits:
	   1: set if we're processing an import or export file
	   2: set if we're currently writing to a loadmap file;
		NOTE:  Even if we are not currently writing to a loadmap file,
		a loadmap file may still be open.
	   4: set if an option argument was provided.
	   8: set if a loadmap file is currently open, which is always true
	   	once the first loadmap file has been opened.  That is,
		the "noloadmap" option doesn't close the file, but just
		stops writing to it.
	   16:  If an option argument is provided and a loadmap file is already
	   	open, the two names are compared.  If they are not the same,
		the 4 and 8 bits are cleared and the 16 bit is set.  Otherwise,
		the bits are unchanged.  Therefore, the values 12-15 can
		occur if the names are equal, and values 16-19 can occur
		if the names are not equal.  Value 19 is the highest that can
		occur.
	*/
	names_bits = ((Bind_state.state & STATE_PROCESSING_IMPEXP) ? 1 : 0)
		      | ((Switches.loadmap) ? 2 : 0)
		      | ((*fname != '\0') ? 4 : 0)
		      | ((Bind_state.loadmap_fn != NULL) ? 8 : 0);
	if ((names_bits & 12) == 12
	    && strcmp(Bind_state.loadmap_fn, fname) != 0)
	    names_bits += 4;		/* Equivalent to setting 16 bit and
					   clearing 4 and 8 bits. */

	switch(names_bits) {
	  case 0:	/* Not impexp; noloadmap; no fname; no loadmap name */
	    fname = DEFAULT_LOADMAP_NAME; /* Open file with default name.  */
	    len = strlen(DEFAULT_LOADMAP_NAME);
	    break;

	  case 4:	/* Not impexp; noloadmap;    fname; no loadmap name */
	    break;			/* Open new loadmap file */

	  case 16:	/* Not impexp; noloadmap; fname != loadmap name */
	  case 18:	/* Not impexp;   loadmap; fname != loadmap name */
	    /* Close existing loadmap file */
	    (void) loadmap_control(LOADMAP_END);
	    break;			/* And open new loadmap file */

	  case 1:	/*     impexp; noloadmap; no fname; no loadmap name */
	    return RC_OK;		/* Valid use.  Acts as no-op, since no
					   current loadmap file is defined. */

	  case 11:	/*     impexp;   loadmap; no fname; loadmap name */
	  case 10:	/* Not impexp;   loadmap; no fname; loadmap name */
	  case 15:	/*     impexp;   loadmap; fname == loadmap name */
	  case 14:	/* Not impexp;   loadmap; fname == loadmap name */
	    return RC_OK;		/* Already using loadmap file--keep
					   using it. */

	  case 8:	/* Not impexp; noloadmap; no fname; loadmap name */
	  case 9:	/*     impexp; noloadmap; no fname; loadmap name */
	  case 13:	/*     impexp; noloadmap; fname == loadmap name */
	  case 12:	/* Not impexp; noloadmap; fname == loadmap name */
	    Switches.loadmap = 1;	/* Start using loadmap file again. */
	    Bind_state.loadmap_err = 0;	/* Reset loadmap error flag */
	    return RC_OK;

#ifdef DEBUG
	    /* Impossible cases */
	  case 2:	/* Not impexp;   loadmap; no fname; no loadmap name */
	  case 3:	/*     impexp;   loadmap; no fname; no loadmap name */
	  case 6:	/* Not impexp;   loadmap;    fname; no loadmap name */
	  case 7:	/*     impexp;   loadmap;    fname; no loadmap name */
	    internal_error();
#endif

	  case 5:	/*     impexp; noloadmap;    fname; no loadmap name */
	  case 17:	/*     impexp; noloadmap; fname != loadmap name */
	  case 19:	/*     impexp;   loadmap; fname != loadmap name */
	    bind_err(SAY_NORMAL, RC_WARNING,
		     NLSMSG(IMPEXP_LOADMAP,
 "%1$s: 0711-756 WARNING: A filename must not be specified when the loadmap option\n"
 "\tis used in an import or export file. The option is being ignored."),
		     Main_command_name);
	    return RC_WARNING;
	}

	if (Bind_state.loadmap_fn)
	    efree(Bind_state.loadmap_fn);
	Bind_state.loadmap_fn = save_string(fname, len + 1);
	/* Reset loadmap error flag */
	Bind_state.loadmap_err = 0;

	Switches.loadmap = 1;
	while ((loadmap_fp = fopen(fname, "w")) == NULL) {
	    Switches.loadmap = 0;
	    if (errno != EINTR) {
		bind_err(SAY_NORMAL, RC_ERROR, NLSMSG(ERROR_NOOPEN_LOADMAP,
	     "%1$s: 0711-755 ERROR: Cannot open or create loadmap file: %2$s\n"
	     "\t%1$s:fopen() %3$s"),
			 Main_command_name, fname, strerror(errno));
		efree(Bind_state.loadmap_fn);
		Bind_state.loadmap_fn = NULL;
		return RC_ERROR;
	    }
	}
	break;
    }
    return RC_OK;
}
/************************************************************************
 * Name: write_to_file		Message output routine			*
 *									*
 * Purpose: Write the specified message to the appropriate file.  If
 *	EINTR occurs, retry printing the message.  Otherwise, show_error()
 *	is called to display a message on standard error (if possible).
 *
 ***********************************************************************/
static void
write_to_file(FILE *f,			/* File to write to */
	      char *fmt,		/* fprintf format string */
	      va_list ap,		/* Arguments for format string */
	      char *reset_error)	/* OUT: Pointer to flag which is
					   reset (set to 0) if no error
					   occurred while writing the
					   message.  This prevents
					   cascading errors from hard errors,
					   such as a file system filling up. */
{
    while (vfprintf(f, fmt, ap) < 0) {
	if (errno != EINTR) {
	    show_error(f, 0 /* Not closing file */, "vfprintf");
	    return;
	}
    }
    /* If we wrote successfully, we reset the error flag. */
    *reset_error = 0;
}
/************************************************************************
 * Name: write_stuff		Message output routine			*
 *									*
 * Purpose: Write a message to the loadmap file and to stdout,
 *	as appropriate.
 *
 * A newline is printed after the message unless SAY_NO_NL is set
 *  in msg_flags. If SAY_NL_ONLY is set in msg_flags, the message (fmt) is
 *  ignored. (If both SAY_NO_NL and SAY_NL_ONLY are set, nothing is printed.)
 *
 * Selection Criteria:
 *	Write to loadmap file if:
 *	1) Switches.loadmap is set AND
 *	2) SAY_NOLDMAP is not set in msg_flags
 *
 *	Write to stdout if:
 *	1a) Switches.quiet is not set OR
 *	1b) SAY_STDOUT is set in flags AND
 *	2) SAY_NOSTDOUT is not set in msg_flags.
 *
 *	Note that if SAY_STDOUT and SAY_NOSTDOUT are both set,
 *	SAY_NOSTDOUT takes precedence.
 *
 ***********************************************************************/
void
write_stuff(int msg_flags,
	    char *fmt,			/* printf() format string */
	    va_list ap)			/* Arguments for format string */
{
    /* Write message to loadmap file, if required.  Even if loadmap file
       has an error, keep trying to write, because file space may be
       freed up by the user as the command executes.  */
    if (Switches.loadmap && !(msg_flags & SAY_NOLDMAP)) {
	if (!(msg_flags & SAY_NL_ONLY))
	    write_to_file(loadmap_fp, fmt, ap, &Bind_state.loadmap_err);
	if (!(msg_flags & SAY_NO_NL))
	    write_to_file(loadmap_fp, "\n", ap /* Ignored */,
			  &Bind_state.loadmap_err);
    }

    /* Write message to stdout, if required. */
    if (!(msg_flags & SAY_NOSTDOUT)
	&& (!Switches.quiet || (msg_flags & SAY_STDOUT))) {
	if (!(msg_flags & SAY_NL_ONLY))
	    write_to_file(stdout, fmt, ap, &stdout_err);
	if (!(msg_flags & SAY_NO_NL))
	    write_to_file(stdout, "\n", ap, &stdout_err);
    }
}
/************************************************************************
 * Name: say			Message output routine			*
 *									*
 * Purpose: This routine is used to print binder output messages	*
 *	to standard output, a file, or both.  See the prologue of
 *	write_stuff() for details.
 *									*
 ***********************************************************************/
void
say(int msg_flags,
    ...)				/* Format string and arguments,
					   if needed */
{
#ifndef NO_NLS
    int		msg;
#endif
    char	*fmt;
    va_list	ap;

    va_start(ap, msg_flags);

    if (!(msg_flags & SAY_NL_ONLY)) {
#ifdef NO_NLS
	fmt = va_arg(ap, char *);
#else
	if (msg_flags & SAY_NO_NLS)
	    fmt = va_arg(ap, char *);
	else {
	    msg = va_arg(ap, int);
	    fmt = catgets(catd, 1, msg, def_message(msg));
	}
#endif
    }

    (void) write_stuff(msg_flags, fmt, ap);
    va_end(ap);
} /* say */
/************************************************************************
 * Name: bind_err		Error Message output routine		*
 *									*
 * Purpose: This routine prints binder error messages.  It is identical
 *	to say() except for two differences:
 *
 *	1) If (a) Switches.quiet is set OR standard output is not a TTY,
 *	  AND (b) Switches.errmsg is set,
 *	  AND (c) the level of the error is greater than or equal to the
 *		current halt level,
 *	the message will be printed to stderr.
 *
 *	2) If SAY_STDERR_ONLY is set in msg_flags, the message will not
 *	be printed to the loadmap, but will only
 *	be printed to stderr under the conditions listed in (1).  This flag
 *	is used when a shorter version of an error message is sent to stderr,
 *	and the full message is written to the loadmap file.
 *									*
 ***********************************************************************/
void
bind_err(int msg_flags,
	 int error_level,
	 ...)				/* Format string and arguments,
					   if needed */
{
#ifndef NO_NLS
    int		msg;
#endif
    char	dummy;			/* Dummy parameter for write_to_file */
    char	*fmt;
    va_list	ap;

    va_start(ap, error_level);

    if (!(msg_flags & SAY_NL_ONLY)) {
#ifdef NO_NLS
	fmt = va_arg(ap, char *);
#else
	if (msg_flags & SAY_NO_NLS)
	    fmt = va_arg(ap, char *);
	else {
	    msg = va_arg(ap, int);
	    fmt = catgets(catd, 1, msg, def_message(msg));
	}
#endif
    }

    if (!(msg_flags & SAY_STDERR_ONLY))
	write_stuff(msg_flags, fmt, ap);

    /* Save command return code from parameter, but check for special
       case in interactive mode. */
    if (error_level > RC_NI_BASE)
	if (Bind_state.stdin_is_tty)
	    error_level = RC_OK;
	else
	    error_level -= RC_NI_BASE;
    Bind_state.cmd_err_lev = max(Bind_state.cmd_err_lev, error_level);

    /* Print message to standard error, if required. */
    if (error_level >= Bind_state.err_exit_lev
	&& (Switches.quiet || !Bind_state.stdout_is_tty)
	&& Switches.errmsg) {
	while (fflush(stdout) == EOF && errno == EINTR)
	    /* skip */;			/* Ignore errors (retry for EINTR) */
	if (!(msg_flags & SAY_NL_ONLY))
	    write_to_file(stderr, fmt, ap, &dummy);
	if (!(msg_flags & SAY_NO_NL))
	    write_to_file(stderr, "\n", NULL, &dummy);
	while (fflush(stderr) == EOF && errno == EINTR)
	    /* Skip */ ;		/* Ignore errors (retry for EINTR) */
    }
    va_end(ap);
}
/************************************************************************
 * Name: internal_err		Error Message output routine		*
 *									*
 * Purpose: Print a message for a fatal, internal error.  The message
 *	prints the file name and line number of the erroneous statement
 *	and exits.
 *
 * NOTE:  This call should be generated by the "internal_error()" macro,
 *	which adds the file and line number arguments.
 *									*
 ***********************************************************************/
void
internal_err(char *f,			/* Source file */
	     int l)			/* Source line number */
{
    bind_err(SAY_NORMAL, RC_PGMERR,
		 NLSMSG(ERROR_INTERNAL,
"%1$s: 0711-759 INTERNAL ERROR: Source file %2$s, line %3$d.\n"
"\tDepending on where this product was acquired, contact your service\n"
"\trepresentative or the approved supplier."),
		 Main_command_name, f, l);
	cleanup(RC_PGMERR);
	/*NOTREACHED*/
}
/***************************************************************************
 * Name:	show_loadmap_message
 *
 * Purpose:	Display a message once, depending on the error level.
 *
 * Results:
 * *************************************************************************/
void
show_loadmap_message(RETCODE rc)
{
    if (loadmap_message_seen == 0 && rc >= Bind_state.err_exit_lev) {
	loadmap_message_seen = 1;
	if (Switches.quiet)
	    if (Switches.loadmap)
		bind_err(SAY_NOLDMAP, rc,
			 NLSMSG(CMPCT_LOADMAP1,
	"%1$s: 0711-344 See the loadmap file %2$s for more information."),
			 Main_command_name, Bind_state.loadmap_fn);
	    else
		bind_err(SAY_NOLDMAP, rc,
			 NLSMSG(CMPCT_LOADMAP2,
"%1$s: 0711-345 Use the -bloadmap or -bnoquiet option to obtain more information."),
			 Main_command_name, Bind_state.loadmap_fn);
    }
}
