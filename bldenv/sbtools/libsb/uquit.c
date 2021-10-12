static char sccsid[] = "@(#)66  1.1  src/bldenv/sbtools/libsb/uquit.c, bldprocess, bos412, GOLDA411a 4/29/93 12:25:18";
/*
 * Copyright (c) 1990, 1991, 1992  
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
 * ODE 2.1.1
 */
/******************************************************************************
**                          Open Software Foundation                         **
**                              Cambridge, MA                                **
**                              Randy Barbano                                **
**                               April 1990                                  **
*******************************************************************************
**
**    Description:
**	This function is part of library libsb.a.  It purpose is to provide
**	a common method of exiting a program.  It provides a way of quitting
**	with or without calling print_usage.
**
**    lib functions and their usage:
**	1) uquit ( exit_value, usage, format, arg1, arg2... )
**	   args:
**	     int      exit_value;		-1, 0, 1, etc
**	     boolean  usage; 			true or false - 1 or 0
**	     char     format [],	 	"%s has value %d"
**	     	      args1 [], arg2 []...;	arguments to format
**
**	   returns:
**	     does not returns, calls exit with exit_value.
**
**	   usage:
**	     This procedure prints out an error message and exits the
**	     the program that called it.  It uses the form of ui_print to 
**	     output the message.  This program also depends on an external
**	     name, progname, to indicate which program was running when the
**	     failure occured.   It prints out the error message from format.
**	     Format is like a printf statement in that it can take a
**	     varying number of arguments.
**
**	     If usage is not set to 0, uquit will call the print_usage
**	     program before it exits with the value in exit_value.
**
**	     NOTE: print_usage is a routine called by uquit.  The
**	           user must create this procedure to use uquit.
*/

#  include <ode/odedefs.h>
#  include <varargs.h>

    extern char   * progname;                             /* program running */

uquit ( va_alist )

	/* This procedure takes a variable length argument list and
	   prints out the name of the function that failed, the
	   error message, usage if asked for, and then exits with
	   the code entered. */

va_dcl

{
    va_list     args;           /* see vprintf(3) and varargs(5) for details */
    int         status,                               /* status to exit with */
                usage;                                 /* do you print usage */
    char      * fmt;                                        /* format string */

  fflush ( stdout );
  va_start ( args );
  status = va_arg ( args, int );         /* gets the first argument and type */
  usage = va_arg ( args, int );         /* gets the second argument and type */
  fmt = va_arg ( args, char * );
  fprintf ( stderr, ">> FATAL ERROR in %s:\n   ", progname );
  vfprintf ( stderr, fmt, args );                 /* print out error message */

  if ( usage )
    print_usage ();

  va_end ( args );
  exit ( status );
}                                                                   /* uquit */

