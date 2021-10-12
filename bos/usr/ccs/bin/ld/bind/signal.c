#if lint == 0 && CENTERLINE == 0
#pragma comment (copyright, "@(#)04	1.8  src/bos/usr/ccs/bin/ld/bind/signal.c, cmdld, bos411, 9428A410j 1/28/94 13:06:17")
#endif

/*
 *   COMPONENT_NAME: CMDLD
 *
 *   FUNCTIONS: init_signals
 *		reset_signals
 *
 *   STATIC FUNCTIONS:
 *		xc_int
 *		xc_stop
 *		xc_bus_or_xfsz
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

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#include <sys/machine.h>

#include "global.h"
#include "bind.h"
#include "error.h"

#ifdef XLC_TYPCHK_BUG
#include "strs.h"
#endif

/* Forward declarations (for signal handlers) */
static void xc_bus_or_xfsz(int, int, struct sigcontext *);
static void xc_int(int sig);
static void xc_stop(int sig);

/* Global variables */
/* Define flags used in interactive mode */
volatile int stop_flag = 0;		/* Set if continuing after ^Z */
volatile int interrupt_flag = 0;	/* Set if ^C pressed */

/************************************************************************
 * Name: init_signals()
 *									*
 * Purpose: Set up signal handlers to allow the binder to clean up
 * (delete temporary files) before exiting.
 *									*
 ************************************************************************/
void
init_signals(void)
{
    struct sigaction	act;

    /* to catch interrupts (^C) */
    sigaction(SIGINT, NULL, &act);
    act.sa_handler = xc_int;
    act.sa_flags &= ~SA_OLDSTYLE;
    sigaction(SIGINT, &act, NULL);

    /* To catch continuing after ^Z */
    sigaction(SIGCONT, NULL, &act);
    act.sa_handler = xc_stop;
    act.sa_flags &= ~SA_OLDSTYLE;
    sigaction(SIGCONT, &act, NULL);

    /* To catch signals that may result from filling up a file system when
       writing to a mapped file.  */
    sigaction(SIGBUS, NULL, &act);
    act.sa_handler = (void (*)(int))xc_bus_or_xfsz;
    sigaddset(&act.sa_mask, SIGABRT);
    act.sa_flags &= ~SA_OLDSTYLE;
    sigaction(SIGBUS, &act, NULL);

    /* To catch filesize ulimit exceeded or file system quota exceeded */
    sigaction(SIGXFSZ, NULL, &act);
    act.sa_handler = (void (*)(int))xc_bus_or_xfsz;
    sigaddset(&act.sa_mask, SIGABRT);
    act.sa_flags &= ~SA_OLDSTYLE;
    sigaction(SIGXFSZ, &act, NULL);
}
/************************************************************************
 * Name: xc_int()
 *									*
 * Purpose: Process SIGINT signal (usually caused by typing ^C)
 *	If we're running interactively, set a flag to be checked for by
 *	the command loop or individual commands.  Otherwise, just call
 *	cleanup() and exit.
 ************************************************************************/
/*ARGSUSED*/
static void
xc_int(int sig)
{
    if (Bind_state.interactive) {
	(void) putchar('\n');	/* Start on clean line */
	interrupt_flag = 1;
	return;
    }

    if (Bind_state.loadmap_err)
	(void) putchar('\n');	/* Start on clean line */
    cleanup(RC_ERROR);
}
/************************************************************************
 * Name: xc_stop()
 *									*
 * Purpose: Process SIGCONT signal (usually caused when resuming after
 *	^Z was typed).
 *	If the current input file is from a TTY, set a flag, which
 *	causes the command loop to reprompt if the ^Z was typed while
 *	waiting for input.  Otherwise, no action is needed.
 ************************************************************************/
/*ARGSUSED*/
static void
xc_stop (int sig)
{
    if (Bind_state.stdin_is_tty)
	stop_flag = 1;
    return;
}
/************************************************************************
 * Name: xc_bus_or_xfsz()
 *									*
 * Purpose: Process signals SIGBUS and SIGXFSZ, which could
 *	indicate quota or file system limits being reached while writing
 *	to the memory-mapped output file.  If so issue appropriate error
 *	diagnostic message.  In all cases, call cleanup() to delete
 *	temporary files or just reraise the signal so that core is dumped.
 *
 * Arguments:								*
 * sig		Signal that was caught
 * code:	Non-standard parameter to any AIX signal-handling function.
 *		Its value is always 0.
 * sigcontext:	Non-standard parameter to any AIX signal-handling function.
 *		Contains reason for error.
 *
 * NOTE:	No recovery is attempted, because the state of the data
 *		structures may be inconsistent.  It would be possible to call
 *		setjmp() at the beginning of save processing and have this
 *		command call longjmp().
 *
 ************************************************************************/
/*ARGSUSED*/
static void
xc_bus_or_xfsz(int sig,
	       int code,		/* Not used */
	       struct sigcontext *scp)
{
    int			except;
    int			dump_core = 0;
    struct sigaction	act;

    except = scp->sc_jmpbuf.jmp_context.excp_type;

    bind_err(SAY_NL_ONLY | SAY_NOLDMAP, RC_SEVERE); /* Print NL */
    if (Bind_state.out_writing) {
	switch(except) {
	  case ENOSPC:
	    bind_err(SAY_NORMAL, RC_SEVERE, NLSMSG(FILE_SYSTEM_FULL,
	  "%s: 0711-750 SEVERE ERROR: The file system is full."),
		     Main_command_name);
	    break;
	  case EDQUOT:
	    bind_err(SAY_NORMAL, RC_SEVERE, NLSMSG(FILE_QUOTA,
	  "%s: 0711-751 SEVERE ERROR: You have exceeded your disk quota."),
		     Main_command_name);
	    break;
	  case EFBIG:
	    bind_err(SAY_NORMAL, RC_SEVERE, NLSMSG(FILE_ULIMIT,
  "%s: 0711-752 SEVERE ERROR: You have exceeded your filesize ulimit."),
		     Main_command_name);
	    break;
	  default:
	    dump_core = 1;
	    bind_err(SAY_NORMAL, RC_SEVERE, NLSMSG(UNEXPECTED_EXCEPTION,
      "%1$s: 0711-997 SEVERE ERROR: Unexpected exception %2$d caught.\n"
					  "\t%3$s"),
		     Main_command_name, sig, strerror(except));
	}
	if (Bind_state.out_tmp_name)
	    bind_err(SAY_NORMAL, RC_SEVERE, NLSMSG(WHILE_WRITING_TEMP,
"%1$s: 0711-994 Error occurred while writing to the temporary output file: %2$s"),
		     Main_command_name, Bind_state.out_tmp_name);
	else
	    bind_err(SAY_NORMAL, RC_SEVERE, NLSMSG(WHILE_WRITING_OUTPUT,
   "%1$s: 0711-993 Error occurred while writing to the output file: %2$s"),
		     Main_command_name, Bind_state.out_name);

	if (dump_core == 0)
	    cleanup(RC_SEVERE);
    }
    else {
	bind_err(SAY_NORMAL, RC_SEVERE, NLSMSG(UNEXPECTED_EXCEPTION,
      "%1$s: 0711-997 SEVERE ERROR: Unexpected exception %2$d caught.\n"
				      "\t%3$s"),
		 Main_command_name, sig, strerror(except));
    }

    loadmap_control(LOADMAP_STOP);	/* Make sure buffer gets flushed. */

    /* Reset action to default.  When we return, the signal will be reraised
       and the default action will be to create a core file. */
    act.sa_handler = SIG_DFL;
    act.sa_flags = 0;
    sigaction(sig, &act, NULL);
}
