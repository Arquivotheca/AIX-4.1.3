/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: alloc_switch
 *		atomic_init
 *		begin_atomic
 *		canonicalize
 *		end_atomic
 *		enter
 *		exists
 *		get_date
 *		insert_line_in_sorted_file
 *		isdir
 *		leave
 *		report_current_function
 *		str_to_BOOLEAN
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
 * COPYRIGHT NOTICE
 * Copyright (c) 1993, 1992, 1991, 1990
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
 * $Log: misc.c,v $
 * Revision 1.1.6.1  1993/11/10  16:56:54  root
 * 	CR 463. Cast stdrup paramater to (char *)
 * 	[1993/11/10  16:55:58  root]
 *
 * Revision 1.1.4.1  1993/09/16  17:36:29  damon
 * 	Fixed COPYRIGHT NOTICE section
 * 	[1993/09/16  17:35:10  damon]
 * 
 * Revision 1.1.2.13  1993/06/18  19:12:46  marty
 * 	CR # 593 - leave will now complain (if VDEBUG is turned on) if it encounters
 * 	a leave() call when the call stack is empty. (i.e. when one of the routines
 * 	calls leave() but forgets to vall enter()).
 * 	[1993/06/18  19:12:35  marty]
 * 
 * Revision 1.1.2.12  1993/05/07  17:52:20  damon
 * 	CR 505. Tightened up initialization of enter/leave
 * 	[1993/05/07  17:48:31  damon]
 * 
 * Revision 1.1.2.11  1993/05/07  15:58:59  marty
 * 	Change enter() and leave() to build up call trace dynamically.
 * 	[1993/05/07  15:58:21  marty]
 * 
 * Revision 1.1.2.10  1993/04/28  14:35:58  damon
 * 	CR 463. More pedantic changes
 * 	[1993/04/28  14:34:45  damon]
 * 
 * Revision 1.1.2.9  1993/04/27  15:39:43  damon
 * 	CR 463. First round of -pedantic changes
 * 	[1993/04/27  15:39:20  damon]
 * 
 * Revision 1.1.2.8  1993/04/26  22:00:26  damon
 * 	CR 463. Pedantic changes
 * 	[1993/04/26  22:00:17  damon]
 * 
 * Revision 1.1.2.7  1993/04/09  17:15:58  damon
 * 	CR 446. Remove warnings with added includes
 * 	[1993/04/09  17:15:10  damon]
 * 
 * Revision 1.1.2.6  1993/04/08  19:28:54  damon
 * 	CR 446. Clean up include files
 * 	[1993/04/08  19:28:43  damon]
 * 
 * Revision 1.1.2.5  1993/03/04  21:29:54  damon
 * 	CR 436. Added str_to_BOOLEAN
 * 	[1993/03/04  20:02:42  damon]
 * 
 * Revision 1.1.2.4  1993/01/20  22:14:52  damon
 * 	CR 399. Added canonicalize
 * 	[1993/01/20  22:13:59  damon]
 * 
 * Revision 1.1.2.3  1993/01/15  16:18:26  damon
 * 	CR 399. Added more functions
 * 	[1993/01/15  16:18:12  damon]
 * 
 * Revision 1.1.2.2  1993/01/14  16:59:31  damon
 * 	CR 399. Added misc.c
 * 	[1993/01/14  16:51:47  damon]
 * 
 * $EndLog$
 */

#ifndef lint
static char sccsid[] = "@(#)92  1.1  src/bldenv/sbtools/libode/misc.c, bldprocess, bos412, GOLDA411a 1/19/94 17:41:27";
#endif /* not lint */

#include <signal.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ode/interface.h>
#include <ode/misc.h>
#include <ode/odedefs.h>
#include <ode/util.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>

# define MAX_FUNCTION_DEPTH 100

STATIC
int current_depth = 0;

struct function_list {
	char * function_name;
	struct function_list *next;
	struct function_list *prev;
};

/* The current function being executed. */
STATIC
struct function_list *current_function = NULL;
STATIC
struct function_list *functions = NULL;  /* Complete function backtrace. */

void enter ( const char * func_name )
{
  struct function_list *func_ptr;
  
  func_ptr = (struct function_list *) malloc(sizeof(struct function_list));
  if (functions == NULL) {
	functions = func_ptr;
	current_function = func_ptr;
	current_function->next = NULL;
	current_function->prev = NULL;
	current_function->function_name = strdup((char *)func_name);
  } else {
	func_ptr->next = NULL;
	func_ptr->prev = current_function;
	func_ptr->function_name = strdup((char *)func_name);
	current_function->next = func_ptr;
	current_function = func_ptr;
  }
  /* end if */
  ui_print ( VDEBUG, "Depth: %d. Entering '%s'.\n", current_depth, func_name );
  current_depth++;
} /* end enter */

void leave ( )
{
    struct function_list *func_ptr;

    if (current_depth <= 0) {
    	ui_print ( VDEBUG, "A call has been made to leave() while the \n");
	ui_print ( VDEBUG, "call stack is empty.\n");
	ui_print ( VDEBUG, "This is due to the absence of an enter() call\n");
	ui_print ( VDEBUG, "in one of the ODE library routines.\n");
	return;
    } 
    current_depth--;
    ui_print ( VDEBUG, "Depth: %d. Leaving '%s'.\n", current_depth,
                       current_function->function_name );
    if (current_function->function_name != NULL)
	free(current_function->function_name);
    if (current_function->prev != NULL) {
	func_ptr = current_function;
    	current_function = current_function->prev;
	current_function->next = NULL;
	free (func_ptr);
    } else {
	free(current_function);
	current_function = NULL;
	functions = NULL;
    }

    if ( current_depth > 0 )
      ui_print ( VCONT, "Returning to '%s'\n",
                         current_function->function_name );
    /* end if */
} /* end leave */

void report_current_function ( )
{
  if ( current_depth >= MAX_FUNCTION_DEPTH )
    ui_print ( VCONT, "Current function: unknown!\n" );
  else
    ui_print ( VCONT, "Current function: '%s'\n",
                      current_function [ current_depth ] );
  /* end if */
} /* end report_current_function */

char * alloc_switch(char swtch, const char * value)
{
    int len;
    char *buf;

    if (value == NULL)
        len = 0;
    else
        len = strlen(value);
    buf = (char *) malloc((size_t) (len + 3));
    if (buf == NULL) {
      ui_print ( VFATAL, "alloc failed\n" );
      return(NULL);
    }
    buf[0] = '-';
    buf[1] = swtch;
    if (len)
        memcpy(buf + 2, value, len);
    buf[len + 2] = '\0';
    return(buf);
}

STATIC
sigset_t sig_block_set_base;
STATIC
sigset_t *sig_block_set;

void atomic_init ( void )
{
  sig_block_set = &sig_block_set_base;
  sigemptyset ( sig_block_set );
  sigaddset ( sig_block_set, SIGINT );
}

void begin_atomic ( void )
{
  sigprocmask ( SIG_BLOCK, sig_block_set, NULL );
  ui_print ( VDEBUG, "begin_atomic\n" );
} /* end begin_atomic */

void end_atomic ( void )
{
  ui_print ( VDEBUG, "end_atomic\n" );
  sigprocmask ( SIG_UNBLOCK, sig_block_set, NULL );
} /* end end_atomic */

int     exists ( const char * epath )

        /* This function sees if a path exists and returns ERROR
           if it does not; OK if it does. */

{
    struct stat statb;                         /* structure from stat report */

  return ( stat ( epath, &statb ));
} /* exists */

BOOLEAN isdir ( const char * ipath )

        /* This function checks to see if the argument is a
           directory.  It returns the results. */

{
    struct      stat statb;

  if (( stat ( ipath, &statb ) == ERROR ) ||
      (( statb.st_mode & S_IFMT ) != S_IFDIR ))
    return ( ERROR );

  return ( OK );
} /* end is_dir */

void get_date ( char * date,                       /* date in mm/dd/yy */
                char * ltime )                        /* time in hh:mm */


        /* This function get the current date and time in hours
           and minutes and returns the values. */

{
    struct tm * time_info;                        /* structure to hold ctime */
    long        t_clock;                                        /* misc long */

  t_clock = time (( long * ) 0 );
  time_info = localtime ( &t_clock );
  sprintf ( date, "%d/%02d/%d", time_info->tm_mon + 1, time_info->tm_mday,
                              time_info->tm_year );

  sprintf ( ltime, "%d:%02d", time_info->tm_hour, time_info->tm_min );
}                                                                /* get date */

int insert_line_in_sorted_file(filename, line, cmp_func, cmp_arg)
char *filename, *line;
int (*cmp_func)( char *, ... );
int cmp_arg;
{
    char buf[MAXPATHLEN];
    char buffer[MAXPATHLEN];
    FILE *inf, *outf;
    int didit, skipped;
    int cmp;

    if (ui_ver_level() >= VDEBUG) {
        if (access(filename, R_OK) < 0)
            ui_print ( VDETAIL, "creating %s\n", filename);
        else
            ui_print ( VDETAIL, "updating %s\n", filename);
    }
    (void) concat(buf, sizeof(buf), filename, ".tmp", NULL);
    (void) unlink(buf);
    if ((outf = fopen(buf, "w")) == NULL) {
      ui_print ( VFATAL, "unable to open %s for write", buf);
      return( ERROR );
    }
    didit = FALSE;
    skipped = FALSE;
    if ((inf = fopen(filename, "r")) != NULL) {
        while (fgets(buffer, sizeof(buffer), inf) != NULL) {
            cmp = (*cmp_func)(buffer, line, cmp_arg, &skipped);
            if (cmp > 0) {
                (void) fputs(line, outf);
                didit = TRUE;
                break;
            }
            if (cmp < 0)
                (void) fputs(buffer, outf);
        }
        if (didit)
            do {
                (void) fputs(buffer, outf);
            } while (fgets(buffer, sizeof(buffer), inf) != NULL);
        if (ferror(inf) || fclose(inf) == EOF) {
            ui_print ( VFATAL, "Error reading %s\n", filename);
            (void) fclose(outf);
            (void) unlink(buf);
            return( ERROR );
        }
    }
    if (!didit) {
        (void) fputs(line, outf);
    }
    if (ferror(outf) || fclose(outf) == EOF) {
        ui_print ( VFATAL, "error writing %s\n", buf);
        (void) unlink(buf);
        return( ERROR );
    }
    if (rename(buf, filename) < 0) {
        ui_print ( VFATAL, "rename %s to %s failed\n", buf, filename);
        (void) unlink(buf);
        return( ERROR );
    }
    return( OK );
}

/*
 * canonicalize path - similar to abspath
 */
int canonicalize( const char * base, const char * relpath, char * outbuf,
                  int outbuf_size)
{
    char *from;
    char *to;
    char *slash;
    char *peek;

    /*
     * concatenate parts of path into buffer
     */
    if (concat(outbuf, outbuf_size, base, "/", relpath, "/", NULL) == NULL) {
      ui_print ( VFATAL, "Path length exceeds buffer size\n" );
      return ( ERROR );
    } /* end if */

    /*
     * remember position of first slash
     */

    slash = strchr(outbuf, '/');
    from = to = slash + 1;

    /*
     * canonicalize the path
     */
    while (*from != '\0') {
        if ((*to++ = *from++) != '/')
            continue;
        peek = to-2;
        if (*peek == '/') {
            /*
             * found "//", back up one
             */
            to--;
            continue;
        }
        if (*peek != '.')
            continue;
        peek--;
        if (*peek == '/') {
            /*
             * found "/./", back up two
             */
            to -= 2;
            continue;
        }
        if (*peek != '.')
            continue;
        peek--;
        if (*peek != '/')
            continue;
        /*
         * found "/../", try to remove preceding token
         */
        if (peek == slash) {
            /*
             * hit the "first" slash, update to not remove any more tokens
             */
            slash = to-1;
            continue;
        }
        /*
         * backup one token
         */
        while (*--peek != '/')
            ;
        to = peek+1;
    }
    *to-- = '\0';
    if (to > outbuf && *to == '/')
        *to = '\0';
  return ( OK );
} /* end canonicalize */

BOOLEAN str_to_BOOLEAN ( const char * buf )
{
  return ( *buf != '0' );
} /* end str_to_BOOLEAN */
