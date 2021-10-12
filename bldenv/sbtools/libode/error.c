/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: err_append
 *		err_clear
 *		err_errno
 *		err_log
 *		err_occurred
 *		err_ode_errno
 *		err_prev
 *		err_report
 *		err_str
 *		err_total
 *		err_type
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
 * $Log: error.c,v $
 * Revision 1.1.4.6  1993/04/28  18:22:23  damon
 * 	CR 463. Pedantic changes
 * 	[1993/04/28  18:22:13  damon]
 *
 * Revision 1.1.4.5  1993/04/08  21:16:37  damon
 * 	CR 446. Clean up include files
 * 	[1993/04/08  21:16:06  damon]
 * 
 * Revision 1.1.4.4  1993/03/16  21:49:15  damon
 * 	CR 436. Added MERGEREQ
 * 	[1993/03/16  21:47:28  damon]
 * 
 * Revision 1.1.4.3  1993/03/15  18:17:18  damon
 * 	CR 396. Fixed pipe strings
 * 	[1993/03/15  18:17:10  damon]
 * 
 * Revision 1.1.4.2  1993/01/25  21:28:13  damon
 * 	CR 396. Converted history.c to err_log
 * 	[1993/01/25  21:26:45  damon]
 * 
 * Revision 1.1.2.7  1992/12/03  17:20:42  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:08:03  damon]
 * 
 * Revision 1.1.2.6  1992/11/12  18:27:50  damon
 * 	CR 329. Added conditional use of varargs.h
 * 	[1992/11/12  18:10:30  damon]
 * 
 * Revision 1.1.2.5  1992/09/24  19:01:30  gm
 * 	CR282: Made more portable to non-BSD systems.
 * 	[1992/09/23  18:21:37  gm]
 * 
 * Revision 1.1.2.4  1992/09/01  18:33:06  damon
 * 	Added error check to err_report
 * 	[1992/09/01  18:32:46  damon]
 * 
 * Revision 1.1.2.3  1992/08/07  19:39:11  damon
 * 	CR 236. Working version
 * 	[1992/08/07  19:37:28  damon]
 * 
 * Revision 1.1.2.2  1992/08/07  19:00:55  damon
 * 	Initial Version
 * 	[1992/08/07  19:00:03  damon]
 * 
 * $EndLog$
 */

#ifndef lint
static char sccsid[] = "@(#)78  1.1  src/bldenv/sbtools/libode/error.c, bldprocess, bos412, GOLDA411a 1/19/94 17:40:55";
#endif /* not lint */

#include <errno.h>
#if __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ode/errno.h>
#include <ode/interface.h>
#include <ode/odedefs.h>
#include <sys/param.h>

struct { const char * msg; int severity; } error_strings [] = {
  { "Internal error", FATAL },
  { "Could not read from file '%s'", FATAL },
  { "Could not write to file '%s'", FATAL },
  { "Could not rename file '%s' to '%s'", FATAL },
  { "Out of memory", FATAL },
  { "Could not run %", FATAL },
  { "Unable to create directory '%s'", FATAL },
  { "Process received signal %d", FATAL },
  { "Could not 'stat' file %s", FATAL },
  { "Could not 'lstat' file %s", FATAL },
  { "Unable to change directory to %s", FATAL },
  { "Fork failed", FATAL },
  { "Unable to get working directory (getcwd)", FATAL },
  { "Could not change the permissions on file or directory '%s'", FATAL },
  { "Could not close file '%s'", FATAL },
  { "Failed to open pipe", FATAL },
  { "Could not read from pipe", FATAL },
  { "Call to wait failed", FATAL },
  { "Could not set the environment variable %s", FATAL },
  { "%s is not a directory", FATAL },
  { "The file '%s' should be executable, but it isn't", FATAL },
  { "The environment variable %s is undefined", FATAL },
  { "Revision %s of file '%s' does not exist", FATAL },
  { "The revisions %s and %s for file '%s' have no common ancestor", FATAL },
  { "Unable to copy file '%s' to '%s'", FATAL },
  { "Missing Log marker", RECOVERABLE },
  { "Missing EndLog marker", RECOVERABLE },
  { "Missing HISTORY marker", RECOVERABLE },
  { "Improper leader line in line %d", RECOVERABLE },
  { "Unable to open the distribution marker file '%s'", FATAL },
  { "Invalid distribution marker '%s'", FATAL },
  { "The distribution marker file '%s' contains an empty marker", FATAL },
  { "File contains an empty log message", RECOVERABLE },
  { "The resource file variable %s is undefined", FATAL },
  { "Syntax error in resource file '%s'", FATAL },
  { "The current working directory is not in the current sandbox", FATAL },
  { "No valid distribution markers were found", FATAL },
  { "Bad distribution marker include syntax", FATAL },
  { "Could not open file '%s'", FATAL },
  { "Improperly formatted history log", RECOVERABLE },
  { "Improperly formatted revision string '%s'", FATAL },
  { "Empty comment leader", FATAL },
  { "File still contains the message '%s'", RECOVERABLE },
  { "Log contains a comment terminator", RECOVERABLE },
  { "Unexpected end of file", RECOVERABLE },
  { "Merges required", RECOVERABLE },
  { "An invalid error has occurred", FATAL },
};

typedef struct err_type * ERR_LOG;

struct err_type  {
  int ode_errno;
  int errno;
  char * e_str;
  ERR_LOG next;
};

#if __STDC__
ERR_LOG err_log ( int ode_errno, ... )
#else
ERR_LOG err_log ( va_alist)
va_dcl
#endif
{
#if !__STDC__
  int ode_errno;
#endif
  va_list ap;
  ERR_LOG log;
  char buf [ MAXPATHLEN ];

  if ( ode_errno > OE_INVALID || ode_errno < -1 )
    ode_errno = OE_INVALID;
  /* if */
  log = ( ERR_LOG ) malloc (sizeof ( struct err_type ) );
  log -> ode_errno = ode_errno;
  if ( log -> ode_errno == OE_SYSERROR ) {
    log -> e_str =  strdup ( strerror ( errno ) );
  } else {
#if __STDC__
    va_start ( ap, ode_errno );
#else
    va_start ( ap );
    ode_errno = va_arg ( ap, int );
#endif
    vsprintf ( buf, error_strings [ log -> ode_errno - 1 ].msg, ap );
    log -> e_str = strdup ( buf );
    printf ( "%s\n", buf );
  } /* if */
  log -> next = NULL;
  return ( log );
} /* end err_log */

char *
err_str ( ERR_LOG log )
{ 
  return ( log -> e_str );
} /* end err_str */

void
err_append ( ERR_LOG log, ERR_LOG log_append )
{
  ERR_LOG log_ptr;

  for ( log_ptr = log; log_ptr -> next != NULL; log_ptr = log_ptr -> next );
  log_ptr -> next = log_append;
} /* end err_append */

/*
err_set_report ( (void) report_func ( ERR_LOG *) )
{
}
*/

int
err_errno ( ERR_LOG log )
{
  return ( log -> errno );
} /* end err_errno */

int
err_ode_errno ( ERR_LOG log )
{
  return ( log -> ode_errno );
} /* end err_ode_errno */

int
err_total ( ERR_LOG log )
{
  ERR_LOG log_ptr;
  int total;

  total = 0;
  for ( log_ptr = log; log_ptr -> next != NULL; log_ptr = log_ptr -> next )
    total ++;
  /* for */
  return ( total );
} /* end err_total */

void
err_report ( ERR_LOG log )
{
  ERR_LOG log_ptr;

  if ( log == NULL )
    return;
  /* if */
  ui_print ( VFATAL, "%s\n", log -> e_str );
  for ( log_ptr = log -> next; log_ptr != NULL; log_ptr = log_ptr -> next )
    ui_print ( VCONT, "%s\n", log_ptr -> e_str );
  /* for */
}

void
err_clear ( ERR_LOG log )
{
} /* end err_clear */

BOOLEAN
err_occurred ( ERR_LOG log, int ode_errno )
{
  ERR_LOG log_ptr;

  for ( log_ptr = log; log_ptr != NULL; log_ptr = log_ptr -> next )
    if ( log_ptr -> ode_errno == ode_errno )
      return ( TRUE );
    /* if */
  /* for */
  return ( FALSE );
} /* end err_occurred */

ERR_LOG
err_prev ( ERR_LOG log )
{
  return ( log -> next );
}

int
err_type ( ERR_LOG log )
{
  return ( error_strings [ log -> ode_errno -1 ].severity );
}
