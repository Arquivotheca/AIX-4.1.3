static char sccsid[] = "@(#)72    1.19  src/bos/usr/ccs/lib/libdbx/dpi_command.c, libdbx, bos411, 9428A410j 9/14/93 11:50:19";
/*
 *   COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 *   FUNCTIONS: dpi_command
 *		process_input
 *		dpi_dealias
 *		dpi_aliasing_off
 *
 *   ORIGINS: 26, 27, 83
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1990,1991
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *
 * Copyright (c) 1982 Regents of the University of California
 *
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
*/


#ifdef KDBXRT
#include "rtnls.h"		/* MUST BE FIRST */
#endif
#include <setjmp.h>
#include <signal.h>
#include "defs.h"
#include "envdefs.h"
#include "process.h"
#include "scanner.h"
#include "commands.h"


#ifdef KDBX
extern boolean new_command;		/* To distinguish between a new */
					/* command and a repeat */
#endif /* KDBX */

public	Boolean	eofinput( );
extern	File	in;

public 	jmp_buf profile_env;		/* used for profiling DBX	*/
public	unsigned int	action_mask;	/* accumulates the actions that */
					/* dbx has done for a given cmd */
extern  boolean aliasingon;		/* true if scanner should alias */

extern	char	scanner_linebuf[];	/* input buffer for the scanner	*/
extern	char	repeat_linebuf[];	/* previous command		*/

#ifndef _NO_PROTO
extern	void catchintr(int);		/* interrupt handler to catch	*/
					/* catch SIGINTR and do a	*/
					/* longjmp() back to the current*/
					/* context			*/
#else
extern	void catchintr();
#endif
extern  int	*envptr;		/* setjmp/longjmp data		*/

extern boolean chkalias; /* set true to tell scanner we're just doing */
			 /* alias processing.                         */

/*
 * NAME: dpi_command
 *        
 * FUNCTION: Sends the command specified in the 'command' argument to
 *	the dbx parser and returns what type of commands were
 *	processed in the dpi_state bitmask.
 *        
 * EXECUTION ENVIRONMENT: Normal user process.
 *        
 * NOTES:  This function is called by the parser to dealias a command.
 *	Only the first word in the command line passed in will be
 *	dealiased.
 *
 * PARAMETERS:
 *	command	- the command to be executed.  The string will be in a 
 *		  format that dbx currently understands, as if the user
 *		  typed in a command to "vanilla" dbx.  The command should
 *		  be terminated by a newline.
 *	dpi_state - a mask which represents actions dbx performed during
 *		  the dpi_command.  The different mask values can be:
 *		  ASSIGNMENT - the dbx assign command was executed
 *		  BREAKPOINT - a breakpoint function was executed
 *		  CONFIGURATION - catch, ignore, set, unset, use, case,
 *			multiproc, command(s) executed
 *		  CONTEXT_CHANGE - a dbx command was executed which changed
 *			the context
 *		  DBX_TERMINATED - the dbx quit command has been executed
 *		  DETACHED - dbx detached from the debugged process
 *		  ELISTING - extended listing bit
 *		  EXECUTION - a command which caused execution was executed
 *			by dbx
 *		  EXECUTION_COMPLETED - the debugged program has completed
 *			execution
 *		  LISTING - a dbx command has changed the file or func
 *		  LOADCALL - program called load() or unload()
 *		  TRACE_ON - a trace point was set
 *
 * DATA STRUCTURES: 
 *	envptr is reset to longjmp back to this function while inside
 *		this function and is reset to its original value
 *		prior to returning.
 *
 * RETURNS:
 *	0 - if we don't get a fatal error.
 *     -1 - if we get a fatal error.
 */
dpi_command (command, dpi_state)
char		*command;	/* the dbx command to be executed	*/
unsigned int	*dpi_state;	/* a mask which represents actions	*/
				/* that dbx performed in processing	*/
				/* the input command               	*/
{
    char	*line;		/* line read in from a file 		*/
    Boolean	repeatcmd;	/* true if $repeat set			*/
    extern	int dpi_update_count;

    action_mask = 0;
    dpi_update_count = 0;

    /*
     * if $repeat is set then we must repeat the last command if the
     * current command is "".
     */
    repeatcmd = varIsSet("$repeat");

    /*
     * If we're called from dbx or xde, aliasing will be true otherwise it will
     * have already been done by dpi_dealias() so it will not need to
     * be performed again.
     */
    chkalias = aliasingon;

    endshellmode( );

    if (command != nil) {
	/*
	 * We've been passed a real live command to execute.
	 * Set up the input to the scanner with the command.
	 */
	strcpy( scanner_linebuf, command );
#ifdef KDBX
	new_command = true;
#endif /* KDBX */
        if (repeatcmd) {
            if (*command == '\n') {
		/*
		 * $repeat is set and we've been given a nil command
		 * so substitute the previous command, if there is
		 * one into the current scanner input.
		 */
#ifdef KDBX
		new_command = false;
#endif  /* KDBX */
                if (repeat_linebuf[0] != '\0') {
		        strcpy(scanner_linebuf,repeat_linebuf);
                }
            } else {
		/*
		 * $repeat is set so we should save this command for
		 * a possible repeat next time.
		 */
	        strcpy(repeat_linebuf,scanner_linebuf);
            }
        }
	/*
	 * Issue the command, 0 returned from process_input() means success.
	 */
	if ( process_input( )) {
		*dpi_state = action_mask;
		signal(SIGINT, catchintr);
		return -1;
	}
    } 
		
    while (! isstdin( )) {
        chkalias = aliasingon;
        endshellmode( );
	if ((line = fgets( scanner_linebuf, MAXLINESIZE, in)) != nil) {
	    if (repeatcmd) {
	     	if (*line == '\n') {
	          	if (repeat_linebuf[0] != '\0') {
		        	 strcpy(line = scanner_linebuf,repeat_linebuf);
	            	}
	        } else {
		       	strcpy(repeat_linebuf,scanner_linebuf);
	        }
	    }
	    resetinput();
	    if ( process_input( )) {
		 *dpi_state = action_mask;
		 signal(SIGINT, catchintr);
		 return -1;
		/*
		 * Why does this return -1 for successful execution????
		 * Shouldn't this return 0 and the else return -1???
		 */
	    }
	} else if ( eofinput( ))
			action_mask |= DBX_TERMINATED;
    }

    if ((action_mask&DETACHED) && (process != nil))
	process->pid = 0;
    else if (action_mask & DBX_TERMINATED)
	pterm(process);

    *dpi_state = action_mask;
    signal(SIGINT, catchintr);
    return 0;
}

/*
 * NAME: process_input
 *        
 * FUNCTION: Save the current environment and start up a dbx parse
 *	and execution of a command.
 *        
 * EXECUTION ENVIRONMENT: Normal user process.
 *        
 * DATA STRUCTURES: 
 *	envptr is reset to longjmp back to this function while inside
 *		this function and is reset to its original value
 *		prior to returning.
 *
 * RETURNS:
 *	0 - if we don't get a fatal error.
 *     -1 - if we get a fatal error.
 */  
static int process_input( )
{
	jmp_buf	env;	/* longjmp buffer for saving context	*/
	int	*svenv; /* pointer to a longjmp buffer used for	*/
			/* saving and restoring the current	*/
			/* longjump environment (envptr) for	*/
			/* possible reuse by dbx		*/

	/*
	 * In order to maintain the correct setjmp/longjmp environment
	 * for dbx we must save off the current envptr at the beginning
	 * of this function and restore it prior to returning.  While
	 * in this function we must have the envptr be ours so a longjmp
	 * can return to our context.
	 */
    	svenv = envptr;
    	envptr = env;

	/*
	 * There appear to be 4 possible return codes from the longjmp.
	 * For these four cases, action_mask is set appropriately, the
	 * envptr is restored.  Return an error if we had a fatal
	 * error.
	 * The default case handles the return from the initial call
	 * to setjmp().
	 */
	switch (setjmp( env )) {
	case ENVCONT:
	    envptr = svenv;
	    return 0;
	    break;
	case ENVEXIT:
	    action_mask |= EXECUTION_COMPLETED;
	    envptr = svenv;
	    return 0;
	    break;
	case ENVQUIT:
	    action_mask |= DBX_TERMINATED;
	    envptr = svenv;
	    return 0;
	    break;
	case ENVFATAL:
	    action_mask |= DBX_TERMINATED;
	    envptr = svenv;
	    return -1;
	    break;
	default:
	    break;
	}

	/*
	 * Set up a signal handler for the interrupt signal so that we'll
	 * be sure to longjmp() back to the above switch.
	 */
	signal(SIGINT, catchintr);

#ifdef PROFILE
	if (setjmp(profile_env)) {
		*dpi_state = action_mask |= EXECUTION_COMPLETED;
		envptr = svenv;
		return 0;
	}
#endif
	/*
	 * execute the dbx command
	 */
	yyparse();

	/*
	 * Restore the original setjmp context.
	 */
	envptr = svenv;

	/*
	 * Success.
	 */
	return 0;
}

/*
 * NAME: dpi_dealias
 *        
 * FUNCTION: Dealias the input command and pass back the result in
 *	aliased_command
 *        
 * EXECUTION ENVIRONMENT: Normal user process.
 *        
 * NOTES:  This function is called to dealias a command.  Only the first
 *	word in the command line passed in will be dealiased.
 *
 * DATA STRUCTURES: 
 *	chkalias is set to true to tell the scanner that we're just
 *		doing alias processing.
 *	scanner_linebuf is set to the command to be dealiased.
 *	envptr is reset to longjmp back to this function while inside
 *		this function and is reset to its original value
 *		prior to returning.
 *
 * RETURNS:
 *	0 - if the command is dealiased.  In this case aliased_command
 *	    will be malloced storage which will contain the new command line.
 *     -1 - if an error of any kind occurred.  In this case aliased_command
 *	    will not be changed in any way and no aliasing will be done.
 */  
int	dpi_dealias( command, aliased_command )
char	*command;	/* command string to be dealiased	*/
char	**aliased_command;/* pointer to returned dealiased string	*/
			/* upon successful completion of this	*/
			/* function				*/
{
	jmp_buf	env;	/* longjmp buffer for saving context	*/
	int	*svenv; /* pointer to a longjmp buffer used for	*/
			/* saving and restoring the current	*/
			/* longjump environment (envptr) for	*/
			/* possible reuse by dbx		*/

	Token	t;	/* token returned by yylex		*/
	extern	char	*nextchar(); /* returns the portion of	*/
			/* the input command line unprocessed	*/
			/* by yylex()				*/
	extern	char	*gettokenstring(); /* returns the 	*/
			/* real keyword associated with an alias*/
	char	buffer[2]; 	/* temp for holding a special	*/
			/* character token returned by yylex()	*/

	char	*dealiased_command;	/* pointer to the 	*/
			/* dealiased portion of the input	*/
			/* command string.			*/
	char	*rest_of_line;		/* the portion of the   */
			/* input command that was not aliased   */
	cases	savecase;

	/*
	 * In order to maintain the correct setjmp/longjmp environment
	 * for dbx we must save off the current envptr at the beginning
	 * of this function and restore it prior to returning.  While
	 * in this function we must have the envptr be ours so a longjmp
	 * can return to our context.
	 */
    	svenv = envptr;
    	envptr = env;
	savecase = dpi_setcase(mixed);

	/*
	 * Standard longjmp handling.
	 */
	if ( setjmp( env ) != 0 ) {
	    dpi_setcase(savecase);
	    envptr = svenv;
	    return -1;
	}

	/*
	 * Set up a signal handler for the interrupt signal so that we'll
	 * be sure to longjmp() back to the above switch.
	 */
	signal(SIGINT, catchintr);

	/*
	 * Tell the scanner that it should do alias processing.
	 */
        chkalias = true;

	endshellmode( );

	/*
	 * Use the command passed in as input to the scanner and set
	 * up the scanner to start processing the command from the
	 * beginning.
	 */
	strcpy( scanner_linebuf, command );
	resetinput();

	/*
	 * process the input command
	 */
	t=yylex();
	if (t == '\n')
	{
		/*
		 * special case for null line
		 */
		dealiased_command = "\\n";
		rest_of_line = nextchar();
	}
	else if ( t == NAME )
	{
		/*
		 * command was aliased
		 */
		dealiased_command = yylval.y_name->identifier;
		rest_of_line = nextchar();
	}
	else if ( ALIAS <= t && t <= XOR )
	{
		/*
		 * we have a dbx command
		 */
		dealiased_command = gettokenstring(t);
		rest_of_line = nextchar();
	}
	else
	{
		/*
		 * The command was not aliased.
		 */
		dealiased_command = "";
		rest_of_line = command;
	}

	/*
	 * nextchar() returns a pointer to the unprocessed (by yylex) portion
	 * of the input command string.  The dealiased portion of the original
	 * command is concatenated with the rest of the command to create the
	 * new dealiased command string to be returned.
	 */
	*aliased_command = malloc(strlen(rest_of_line) +
			   strlen(dealiased_command) + 1);
	strcpy(*aliased_command, dealiased_command);
	strcat(*aliased_command, rest_of_line);

	/*
	 * Restore the original setjmp context.
	 */
	envptr = svenv;
	dpi_setcase(savecase);

	/*
	 * We've successfully aliased the command.
	 */
	return 0;
}

/*
 * NAME: dpi_aliasing_off
 *        
 * FUNCTION: Turn off aliasing.
 *        
 * EXECUTION ENVIRONMENT: Normal user process.
 *        
 * NOTES:  This turns off aliasing that is normally done when
 *	dpi_command() is called.  Aliasing can still be done by
 *	calling dpi_dealias().
 *
 * DATA STRUCTURES: sets aliasingon to false
 *
 * RETURNS: NONE
 */  
dpi_aliasing_off()
{
	aliasingon = false;
}
