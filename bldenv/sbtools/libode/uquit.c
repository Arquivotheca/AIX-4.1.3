/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: uquit
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
 * $Log: uquit.c,v $
 * Revision 1.6.5.5  1993/04/28  14:35:43  damon
 * 	CR 463. More pedantic changes
 * 	[1993/04/28  14:34:36  damon]
 *
 * Revision 1.6.5.4  1993/04/08  22:51:33  damon
 * 	CR 446. Clean up include files
 * 	[1993/04/08  22:51:16  damon]
 * 
 * Revision 1.6.5.3  1993/01/21  18:39:17  damon
 * 	CR 401. Added stdarg
 * 	[1993/01/21  18:34:12  damon]
 * 
 * Revision 1.6.2.2  1992/12/03  17:23:04  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:09:30  damon]
 * 
 * Revision 1.6  1991/12/05  21:13:28  devrcs
 * 	Changed sdm to ode; std_defs.h to  odedefs.h
 * 	[91/01/10  11:47:15  randyb]
 * 
 * 	Correct copyright; clean up lint input; added ui_print function.
 * 	[91/01/08  12:24:24  randyb]
 * 
 * 	Style update; saberc lint
 * 	[90/11/08  09:57:43  randyb]
 * 
 * Revision 1.4  90/10/07  20:05:00  devrcs
 * 	Pre-OSF/1 changes
 * 
 * $EndLog$
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

#ifndef lint
static char sccsid[] = "@(#)30  1.1  src/bldenv/sbtools/libode/uquit.c, bldprocess, bos412, GOLDA411a 1/19/94 17:42:49";
#endif /* not lint */

#include <stdio.h>
#include <ode/odedefs.h>
#include <ode/util.h>
#if __STDC__
#  include <stdarg.h>
#else
#  include <varargs.h>
#endif
    extern char   * progname;                             /* program running */

void
#if __STDC__
uquit ( int status, ... )
#else
uquit ( va_alist )
va_dcl
#endif
	/* This procedure takes a variable length argument list and
	   prints out the name of the function that failed, the
	   error message, usage if asked for, and then exits with
	   the code entered. */

{
#if !__STDC__
    int         status;                               /* status to exit with */
#endif
    va_list     args;           /* see vprintf(3) and varargs(5) for details */
    int         usage;                                 /* do you print usage */
    char      * fmt;                                        /* format string */

  fflush ( stdout );
#if __STDC__
  va_start ( args, status );
#else
  va_start ( args );
  status = va_arg ( args, int );         /* gets the first argument and type */
#endif
  usage = va_arg ( args, int );         /* gets the second argument and type */
  fmt = va_arg ( args, char * );
  fprintf ( stderr, ">> FATAL ERROR in %s:\n   ", progname );
  vfprintf ( stderr, fmt, args );                 /* print out error message */

  if ( usage )
    print_usage ();

  va_end ( args );
  exit ( status );
}                                                                   /* uquit */

