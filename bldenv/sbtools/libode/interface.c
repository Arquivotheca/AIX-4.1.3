/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: arg_value
 *		built_ins
 *		entry_pattern
 *		get_dotsbx_options
 *		get_local_rcfile
 *		get_options
 *		hvalue
 *		in_arg_list
 *		init_htable
 *		init_one
 *		init_to_optlist
 *		load_entry
 *		load_htable
 *		load_max_args
 *		load_min_args
 *		load_options
 *		load_posix_entry1417
 *		load_using_posix1065
 *		match_arg_list
 *		match_entry
 *		match_pattern
 *		next_token
 *		print_htable
 *		print_internals
 *		rc_file_options
 *		set_duplicates
 *		set_ver
 *		skip_white_space
 *		special_case
 *		test_conflicts
 *		ui_arg_cnt
 *		ui_arg_value
 *		ui_args_to_argv
 *		ui_entries_to_argv2069
 *		ui_entry_cnt
 *		ui_entry_value
 *		ui_init
 *		ui_is_auto
 *		ui_is_info
 *		ui_is_set
 *		ui_load
 *		ui_print
 *		ui_print_revision2233
 *		ui_restore_progname
 *		ui_set_progname
 *		ui_unset
 *		ui_ver_level
 *		ui_ver_switch
 *		uiquit
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
 *  @OSF_FREE_COPYRIGHT@
 *  COPYRIGHT NOTICE
 *  Copyright (c) 1992, 1991, 1990  
 *  Open Software Foundation, Inc. 
 *   
 *  Permission is hereby granted to use, copy, modify and freely distribute 
 *  the software in this file and its documentation for any purpose without 
 *  fee, provided that the above copyright notice appears in all copies and 
 *  that both the copyright notice and this permission notice appear in 
 *  supporting documentation.  Further, provided that the name of Open 
 *  Software Foundation, Inc. ("OSF") not be used in advertising or 
 *  publicity pertaining to distribution of the software without prior 
 *  written permission from OSF.  OSF makes no representations about the 
 *  suitability of this software for any purpose.  It is provided "as is" 
 *  without express or implied warranty. 
 */
/*
 * HISTORY
 * $Log: interface.c,v $
 * Revision 1.3.13.8  1993/11/10  18:44:34  root
 * 	CR 463. Pedantic changes
 * 	[1993/11/10  18:43:05  root]
 *
 * Revision 1.3.13.7  1993/11/10  16:56:49  root
 * 	CR 463. Cast stdrup paramater to (char *)
 * 	[1993/11/10  16:55:54  root]
 * 
 * Revision 1.3.13.6  1993/11/08  22:23:23  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/08  22:22:32  damon]
 * 
 * Revision 1.3.13.5  1993/11/08  20:18:08  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/08  20:17:25  damon]
 * 
 * Revision 1.3.13.4  1993/11/05  20:34:20  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/05  20:33:35  damon]
 * 
 * Revision 1.3.13.3  1993/11/04  00:03:24  damon
 * 	CR 463. More pedantic
 * 	[1993/11/04  00:02:52  damon]
 * 
 * Revision 1.3.13.2  1993/11/03  20:40:25  damon
 * 	CR 463. More pedantic
 * 	[1993/11/03  20:38:04  damon]
 * 
 * Revision 1.3.13.1  1993/10/22  19:21:40  damon
 * 	CR 761. Removed SB_RC_OP
 * 	[1993/10/22  19:12:44  damon]
 * 
 * Revision 1.3.11.1  1993/08/19  18:35:18  damon
 * 	CR 622. Changed if STDC to ifdef STDC
 * 	[1993/08/19  18:33:07  damon]
 * 
 * Revision 1.3.6.7  1993/04/29  14:59:06  damon
 * 	CR 463. Pedantic changes
 * 	[1993/04/29  14:58:50  damon]
 * 
 * Revision 1.3.6.6  1993/04/09  17:22:39  damon
 * 	CR 446. Remove warnings with added includes
 * 	[1993/04/09  17:21:48  damon]
 * 
 * Revision 1.3.6.5  1993/04/08  21:16:40  damon
 * 	CR 446. Clean up include files
 * 	[1993/04/08  21:16:09  damon]
 * 
 * Revision 1.3.6.4  1993/01/21  18:47:54  damon
 * 	CR 401. Fixed redundant decls.
 * 	[1993/01/21  18:45:54  damon]
 * 
 * Revision 1.3.6.3  1993/01/21  18:39:13  damon
 * 	CR 401. Added stdarg
 * 	[1993/01/21  18:34:08  damon]
 * 
 * 	Taken from 2.1.1
 * 	[1992/06/15  16:31:24  damon]
 * 
 * Revision 1.3.2.10  1992/12/03  17:21:01  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:08:18  damon]
 * 
 * Revision 1.3.2.9  1992/11/12  18:27:53  damon
 * 	CR 329. Removed ANSI function decl.
 * 	[1992/11/12  18:11:29  damon]
 * 
 * Revision 1.3.2.8  1992/09/24  19:01:34  gm
 * 	CR282: Made more portable to non-BSD systems.
 * 	[1992/09/23  18:21:49  gm]
 * 
 * Revision 1.3.2.7  1992/09/15  19:27:11  damon
 * 	CR 265. Fixed initialization problem.
 * 	[1992/09/15  19:26:54  damon]
 * 
 * Revision 1.3.2.6  1992/09/15  19:20:57  damon
 * 	CR 265. Not all systems have getopt stuff defined
 * 	[1992/09/15  19:20:23  damon]
 * 
 * Revision 1.3.2.5  1992/08/07  15:31:29  damon
 * 	CR 266. Changed salloc to strdup
 * 	[1992/08/07  15:29:12  damon]
 * 
 * Revision 1.3.2.4  1992/08/06  21:10:20  damon
 * 	CR 265. Fixed initialization problem.
 * 	[1992/08/06  21:08:39  damon]
 * 
 * Revision 1.3.2.3  1992/08/06  15:58:38  damon
 * 	CR 265. Now optionally uses getopt
 * 	[1992/08/06  15:58:12  damon]
 * 
 * Revision 1.3.2.2  1992/06/15  18:10:36  damon
 * 	Synched with 2.1.1
 * 	[1992/06/15  18:04:34  damon]
 * 
 * Revision 1.3.2.2  1992/03/25  22:48:12  damon
 * 	Changes for ui_print_revision
 * 	[1992/03/25  21:52:29  damon]
 * 
 * Revision 1.3  1991/12/05  21:05:10  devrcs
 * 	changed *rcargc = alloc_arg to *rcargc >= alloc_arg
 * 	[91/06/25  15:00:10  mckeen]
 * 
 * 	added VERSION_OP as an alias for REV_OP
 * 	[91/06/03  13:41:56  ezf]
 * 
 * 	Changed reading .sandboxrc file so it removes any trailing spaces
 * 	  and does not count them as arguments.
 * 	Made a difference between loading arguments from the command line
 * 	  and from ui_load so that ui does not exit on failures from ui_load.
 * 	  It now returns ERROR.  Added uiquit () to do this and the global
 * 	  boolean from_ui_load.
 * 	[91/01/29  11:17:18  randyb]
 * 
 * 	Second submission
 * 	[91/01/22  12:48:12  randyb]
 * 
 * 	Added return_prog_name () before uquits
 * 	[91/01/22  12:31:21  randyb]
 * 
 * 	Wrote these routines to handle user interface in a standard way.
 * 	There are a series of public functions each starting with "ui_".
 * 	The other functions are static and are not meant to be used outside
 * 	the library.
 * 	[91/01/08  12:13:16  randyb]
 * 
 * 	Initial version.
 * 	[90/12/10  16:23:51  damon]
 * 
 * 	Wrote these routines to handle user interface in a standard way.
 * 	There are a series of public functions each starting with "ui_".
 * 	The other functions are static and are not meant to be used outside
 * 	the library.
 * 	[91/01/08  12:13:16  randyb]
 * 
 * 	Initial version.
 * 	[90/12/10  16:23:51  damon]
 * 
 * $EndLog$
 */
/******************************************************************************
**                          Open Software Foundation                         **
**                              Cambridge, MA                                **
**                Randy Barbano, Damon Poole, Martha Hester                  **
**                              December 1990                                **
*******************************************************************************
**
**  Description:  These functions provide a common user interface for the
**	ODE tools.  They parse the command line, provide an interface to
**	the information, and provide a common way of printing out messages.
**
**  functions:
**    ui_init (int, ** char, int, * struct) int
**      ui_set_progname (* char) int
**      init_htable ( int, ** char) int
**        hvalue (* char) unsigned
**        init_one (* struct, * char, int) void
**      load_htable (int, * struct) void
**        test_conflicts (* struct) void
**        match_pattern (* char) * struct
**          hvalue 
**        hvalue 
**      rc_file_options (int, ** char, * char, * int, *** char) BOOLEAN
**        get_local_rcfile (* char, * char, * char) BOOLEAN
**        ui_print (int, * char, va_list) void
**        get_dotsbx_options (* char, * char, * int, *** char) BOOLEAN
**          ui_print 
**          get_options (* struct, * char) * char
**          next_token (** char, * char) BOOLEAN
**            skip_white_space (** char) void
**            ui_print 
**      load_options (int, ** char, int, * struct) int
**        built_ins (* char) * char
**        special_case (* char) BOOLEAN
**          ui_unset (* char) int
**            match_pattern 
**          ui_print 
**        entry_pattern (* char, * struct, int) * char
**        match_pattern 
**        load_entry (* struct, * struct, ** char, * char, int, * int) void
**          load_min_args (* struct, ** char, ** struct, int, * int, * int) void
**            match_arg_list (* char, * char) BOOLEAN
**            in_arg_list (* struct, * char) BOOLEAN
**          load_max_args (* struct, ** char, ** struct, int, * int, * int) void
**            match_arg_list 
**            in_arg_list 
**        ui_unset 
**        set_duplicates (* struct, ** struct, * char) void
**          match_entry (* struct, * char, ** struct) void
**      print_htable () void
**        ui_print 
**        print_internals (* struct, * struct, * struct, int) void
**          ui_print 
**      ui_restore_progname () int
**    ui_load (* char, int, va_list) int
**      built_ins 
**      special_case 
**      match_pattern 
**      ui_print 
**      load_entry 
**      ui_unset 
**      set_duplicates 
**    ui_is_set (* char) BOOLEAN
**      match_pattern 
**      ui_print 
**    ui_entry_cnt (* char) int
**      match_pattern 
**      ui_print 
**    ui_arg_cnt (* char, int) int
**      ui_print 
**      match_pattern 
**    ui_entry_value (* char, int) * char
**      ui_print 
**      match_pattern 
**    ui_arg_value (* char, int, int) * char
**      ui_print 
**      match_pattern 
**      arg_value (* struct, int, int) * char
**        ui_print 
**    ui_entries_to_argv (* char, *** char) int
**      match_pattern 
**      ui_print 
**    ui_args_to_argv (* char, int, *** char) int
**      ui_print 
**      match_pattern 
**    ui_ver_switch () * char
**      ui_print 
**    ui_is_auto () BOOLEAN
**    ui_is_info () BOOLEAN
**    ui_ver_level () int
**
*/

#ifndef lint
static char sccsid[] = "@(#)87  1.1  src/bldenv/sbtools/libode/interface.c, bldprocess, bos412, GOLDA411a 1/19/94 17:41:16";
#endif /* not lint */

#include  "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ode/sandboxes.h>
#include <ode/util.h>

extern char *optarg;
extern int optind;
extern int opterr;

static void
load_using_posix (
    int           usrct,                     /* number of arguments to parse */
    char       ** usr_list,                             /* list of arguments */
    int           initct,                    /* count of lines to initialize */
    UIINIT      * init);                       /* initialization information */
static int
load_posix_entry ( 
    HTABLE      * hptr,                       /* points to one table pattern */
    char	* entry,
    char	* arg);

void
#ifdef __STDC__
uiquit ( int , ... );
#else
uiquit ( va_alist );
#endif
int	ui_init ( argc, argv, initct, init, posix_ct, posix_ops )

	/*
     	 * Initialize ui with the argv passed in as well as any switches
	 * appearing in the rc file. This procedure must be called before
	 * any of the other library procedures are called. This replaces
	 * the old call to parse_cmd_line.
     	 * Returns ERROR if not successful, OK otherwise.
     	 */

    int         argc;                /* the number of command line arugments */
    char     ** argv;                          /* strings with each argument */
    int         initct;                      /* count of lines to initialize */
    UIINIT    * init;                          /* initialization information */
    int           posix_ct;                  /* count of lines to initialize */
    UIINIT      * posix_ops;                   /* initialization information */

{
    const char ** arglist;                        /* list of rc file arguments */
    int         argct;                                  /* rc file arg count */

  from_ui_load = FALSE;
  ui_set_progname ( "ui_init" );
  init_htable ( argc, argv );
  load_htable ( initct, init );
  load_htable ( posix_ct, posix_ops );

  if ( rc_file_options ( argc, argv, &argct, &arglist ))
    load_options ( argct, arglist, initct, init );

  if ( posix_ct > 0 ) {
    ui_print ( VDEBUG, "Using getopt to parse switches\n" );
    load_using_posix ( argc - 1 , argv + 1, initct, init );
    load_options ( argc - 1  - optind, (const char **) (argv + 1 + optind),
                   posix_ct, posix_ops );
  } else
    load_options ( argc - 1, (const char **) (argv + 1), initct, init );
  /* if */
  print_htable ();
  ui_restore_progname ();
  return ( OK );
}                                                                 /* ui init */


void
ui_set_progname ( const char * newname )

	/*
	 * Put the newname into progname after having saved the
	 * initial value.
	 */

{
  save_progname = progname;
  progname = newname;
}                                                         /* ui set_progname */


void
ui_restore_progname (void)

	/*
	 * Puts previous progname back into progname.
	 */

{
  progname = save_progname;
}                                                     /* ui restore progname */



static void
init_htable ( int argc, char ** argv )

	/*
     	 * Initializes the entire hash table with default values.
	 * Also loads the know options.
	 */
{
    HTABLE      * hptr;                               /* points to one entry */
    int           ct;                                        /* misc counter */

  auto_pos = hvalue ( AUTO_OP );
  info_pos = hvalue ( INFO_OP );

  if (( argc > 1 ) && streq ( argv[1], DEBUG_OP ))
    verbosity = VDEBUG;    	       /* a way to get inside interface info */
  else if (( argc > 1 ) && streq ( argv[1], VERBOSE_OP ))
    verbosity = VDETAIL;
  else 
    verbosity = VNORMAL;

  if (( htbl = (HTABLE *) malloc (sizeof ( HTABLE ) * HTABLESIZE)) == NULL )
    uquit ( ERROR, FALSE, "sys: cannot create space for user interface.\n" );

  for ( ct = 0; ct < HTABLESIZE; ct++ ) {
    hptr = htbl + ct;
    hptr->pattern = strdup ( EMPTY_STRING );
    hptr->entry = NULL;
    hptr->arg_list = NULL;
    hptr->entry_list = NULL;
    hptr->next_pattern = NULL;
    hptr->num_entries = hptr->num_args = 0;
  } /* for */

  init_one ( htbl + auto_pos, AUTO_OP, 0 );
  init_one ( htbl + info_pos, INFO_OP, 0 );
  init_one ( htbl + hvalue ( RC_OP), RC_OP, 1 );
}                                                             /* init htable */



static unsigned
hvalue ( const char * string )

	/*
	 * Calculates the hash value of the string.
	 */
{
    unsigned      hashval;                               /* cumulative value */

  for ( hashval = 0; *string != NUL; string++ )
    hashval = *string + 31 * hashval;

  return ( hashval % HTABLESIZE );
}                                                                  /* hvalue */



static
void init_one (
    HTABLE      * hptr,                               /* points to one entry */
    const char  * pattern,                               /* pattern to match */
    int           argct)                           /* args allowed or needed */

	/*
	 * Initializes a single hash table entry.
	 */


{
  hptr->pattern = strdup ( (char *)pattern );
  hptr->duplicates = OVERWRITE;
  hptr->max_entries = 1;
  hptr->min_args = hptr->max_args = argct;

  if ( argct == 0 )
    hptr->legal_args = strdup ( EMPTY_STRING );
  else
    hptr->legal_args = strdup ( ARGS_OP );
}                                                                /* init one */



static void
load_htable (
    int           initct,                    /* count of lines to initialize */
    UIINIT      * init)                        /* initialization information */

	/*
	 * loads the initial values into the hash table.  The
	 * built-in values are already loaded.
	 */


{
    HTABLE      * hptr;                               /* points to one entry */
    UIINIT      * iptr;              /* points to initialization information */
    int           ct;                                        /* misc counter */

  for ( ct = 0; ct < initct; ct++ ) {
    iptr = init + ct;

    test_conflicts ( iptr );

    if (( hptr = match_pattern ( iptr->pattern )) != NULL )
      uquit ( ERROR, FALSE, 
	      "internal error: program has duplicate patterns: %s.\n",
	       iptr->pattern );

    hptr = htbl + hvalue ( iptr->pattern );

    if ( ! streq ( hptr->pattern, EMPTY_STRING )) {
      while ( hptr->next_pattern != NULL )
	hptr = hptr->next_pattern;
      
      hptr->next_pattern = ( HTABLE * ) malloc ( sizeof ( HTABLE ));
      hptr = hptr->next_pattern;
      hptr->entry = NULL;
      hptr->entry_list = NULL;
      hptr->arg_list = NULL;
      hptr->next_pattern = NULL;
      hptr->num_entries = hptr->num_args = 0;
    } /* if */

    hptr->pattern = iptr->pattern;
    hptr->duplicates = iptr->duplicates;
    hptr->max_entries = iptr->max_entries;
    hptr->min_args = iptr->min_args;
    hptr->max_args = iptr->max_args;
    hptr->legal_args = iptr->legal_args;
  } /* for */
}                                                             /* load htable */



static void
test_conflicts ( 
    UIINIT      * iptr)              /* points to initialization information */

	/*
	 * Tests user entered intialization array to make sure it is
	 * legal.
	 */


{
  if ( *(iptr->pattern) == NUL )
    uquit ( ERROR, FALSE, "internal error: UIINIT pattern set to NULL.\n" );

  if ( iptr->max_entries < 1 && iptr->max_entries != UNLIMITED )
    uquit ( ERROR, FALSE, 
            "internal error: UIINIT %s max_enties < 1.\n", iptr->pattern );

  if ( iptr->duplicates < TOGGLE || iptr->duplicates > ACCUM )
    uquit ( ERROR, FALSE, 
       "internal error: UIINIT %s duplicates out of range.\n", iptr->pattern );

  if ( iptr->min_args < 0 )
    uquit ( ERROR, FALSE, 
            "internal error: UIINIT %s min_args < 0.\n", iptr->pattern );

  if ( iptr->max_args < UNLIMITED )
    uquit ( ERROR, FALSE, 
            "internal error: UIINIT %s max_args < 0.\n", iptr->pattern );

  if ( iptr->min_args > iptr->max_args && iptr-> max_args != UNLIMITED )
    uquit ( ERROR, FALSE, 
           "internal error: UIINIT %s max_args < min_args.\n", iptr->pattern );

  if ( iptr->legal_args == NULL )
    uquit ( ERROR, FALSE,
        "internal error: UIINIT %s legal_args set to NULL.\n", iptr->pattern );

  if ( iptr->max_args == 0 && iptr->duplicates == ACCUM )
    uquit ( ERROR, FALSE, 
            "internal error: UIINIT %s max_args = 0 && duplicates = ACCUM.\n",
             iptr->pattern );

  if ( iptr->max_entries != 1 && iptr->duplicates == TOGGLE )
    uquit ( ERROR, FALSE, 
        "internal error: UIINIT %s max_entires != 1 && duplicates = TOGGLE.\n",
         iptr->pattern );

  if ( iptr->max_entries > 1 && *(iptr->pattern) != '!' &&
      ( strpbrk ( iptr->pattern, "*?[" ) == NULL ))
    uquit ( ERROR, FALSE, 
        "internal error: UIINIT unique pattern %s allows max_entires > 1.\n",
         iptr->pattern );
}                                                          /* test conflicts */



static HTABLE *
match_pattern ( const char * pattern )

	/*
	 * Finds the match to the pattern in the hash table.
	 * Returns a pointer to the table entry; NULL if none found.
	 */
{
    HTABLE      * hptr;                               /* points to one entry */

  hptr = htbl + hvalue ( pattern );

  while ( hptr != NULL ) {
    if ( streq ( hptr->pattern, pattern ))
      break;
    hptr = hptr->next_pattern;
  } /* while */

  return ( hptr );
}                                                           /* match pattern */



static BOOLEAN	rc_file_options (
    int           argc,              /* the number of command line arugments */
    char       ** argv,                            /* command line arguments */
    int         * rc_argc,                  /* the number of rc file options */
    const char *** rc_argv)                     /* strings with each argument */


	/*
 	 * Determine the rc file to parse for default options.
 	 */

{
    char       ** argv_ptr,                                 /* agrv traveler */
                * rc_file_name,                           /* name of rc file */
                * nameonly;                  /* in case more than name given */
    int           argn;                                      /* misc counter */
    BOOLEAN       alt_rc = FALSE;                            /* misc boolean */

  	/* Check to see if a different .sandboxrc file should be used.  */
  argv_ptr = argv + 1;

  if ( ( nameonly = strrchr ( *argv, SLASH )) != NULL )
    nameonly++;
  else
    nameonly = *argv;

  for ( argn = 1; argn < argc; argn++ ) {
    if ( streq ( *argv_ptr, RC_OP )) {
      alt_rc = TRUE;
      break;
    } /* if */

    argv_ptr++;
  } /* end for */

  if ( alt_rc ) {
    if ( access ( *(++argv_ptr), R_OK ) == OK )
      rc_file_name = *argv_ptr;

    else if ( ! get_local_rcfile ( *argv_ptr, &rc_file_name, EMPTY_STRING )) {
      ui_print ( VDEBUG, "Could not find rc file : %s\n", *argv_ptr );
      return ( FALSE );
    } /* else if */
  } /* if */

  else if ( ! get_local_rcfile ( nameonly, &rc_file_name, RCEXT )) {
    if ( ! get_default_usr_rcfile ( &rc_file_name, FALSE ))
      return ( FALSE );
  } /* else if */

  ui_print ( VDETAIL, "Reading rc file : %s\n", rc_file_name );
  return ( get_dotsbx_options ( rc_file_name, nameonly, rc_argc, rc_argv ));
}                                                         /* rc file options */



static BOOLEAN get_local_rcfile (
    char        * cname,                                     /* command name */
    char       ** rcfile,                     /* string to hold name of file */
    const char  * ext)                      /* extension to end of file name */

	/*
	 * Builds path and searches for existence of rc file with same
	 * name as program calling routine.
	 * Returns TRUE if local name is found; FALSE otherwise.
	 */

{
    char          trcfile [ PATH_LEN ],  /* temporary holder */
		* env_input;                     /* holds values from getenv */

  if (( env_input = getenv ( "HOME" )) == NULL )
    return ( FALSE );

  if ( streq ( ext, EMPTY_STRING ))
    concat ( trcfile, PATH_LEN, env_input, "/", cname, ext, NULL );
  else
    concat ( trcfile, PATH_LEN, env_input, "/.", cname, ext, NULL );

  if ( access ( trcfile, R_OK ) != OK ) {
    *rcfile = NULL;
    return ( FALSE );
  } /* if */

  *rcfile = strdup ( trcfile );
  return ( TRUE );
}                                                        /* get local rcfile */


#ifdef __STDC__
void    ui_print ( int msg_verbosity, ... )
#else
void    ui_print ( va_alist )
vadcl
#endif

	/*
	 * This procedure takes the in va_alist and parses out the
	 * pieces it needs then passes the rest to a regular print
	 * routine.  It provides a standard printing routine for
	 * the ODE commands.
	 */

{
#ifndef __STDC__
                int       msg_verbosity;         /* print on verbosity level */
#endif
                va_list   arglist;               /* variable length arg list */
                char    * msg_format;                   /* printf formatting */

    static const char   * outfmt[] = {                  /* leader for output */
      "",                                                         /* VALWAYS */
      ">> FATAL ERROR",                                            /* VFATAL */
      "",                                                          /* VQUIET */
      ">> WARNING",                                                 /* VWARN */
      "",                                                         /* VNORMAL */
      ">> DIAGNOSTIC",                                              /* VDIAG */
      ">  ",                                                      /* VDETAIL */
      ">> DEBUG INFO"                                              /* VDEBUG */
    };

    static      FILE    * outfile[] = {               /* placement of output */
      stdout,                                                     /* VALWAYS */
      stderr,                                                      /* VFATAL */
      stdout,                                                      /* VQUIET */
      stderr,                                                       /* VWARN */
      stdout,                                                     /* VNORMAL */
      stderr,                                                       /* VDIAG */
      stdout,                                                     /* VDETAIL */
      stderr                                                       /* VDEBUG */
    };

    static      int       last_ver = VNORMAL;      /* former verbosity level */

  fflush ( stdout );
#ifdef __STDC__
  va_start ( arglist, msg_verbosity );
#else
  va_start ( arglist );
  msg_verbosity = va_arg ( arglist, int );
#endif

  if (( msg_verbosity <= verbosity ) || 
      ( msg_verbosity == VCONT && last_ver <= verbosity )) {
    msg_format  = va_arg ( arglist, char * );

    if ( msg_verbosity == VFATAL  || msg_verbosity == VWARN ||
	 msg_verbosity == VDIAG || msg_verbosity == VDEBUG )
      fprintf ( outfile[msg_verbosity], "%s in %s:\n>> ",
		outfmt[msg_verbosity], progname );
   
    if ( msg_verbosity == VDETAIL )
      fprintf ( outfile[msg_verbosity], "%s", outfmt[msg_verbosity] );

    if ( msg_verbosity == VCONT ) {
      if ( last_ver == VFATAL  || last_ver == VWARN ||
	   last_ver == VDIAG || last_ver == VDEBUG )
	fprintf ( outfile[last_ver], ">> " );
      else if ( last_ver == VDETAIL )
	fprintf ( outfile[last_ver], ">  " );

      vfprintf ( outfile[last_ver], msg_format, arglist );
    } /* if */

    else
      vfprintf ( outfile[msg_verbosity], msg_format, arglist );
  } /* if */

  if ( msg_verbosity != VCONT )
    last_ver = msg_verbosity;

  va_end ( arglist );
}

typedef const char * CCS;

static 	BOOLEAN  get_dotsbx_options ( 
    const char   * rc_file_name,                  /* name of rc file to parse */
    const char   * command,                    /* name of command to look for */
    int          * rcargc,                                    /* option count */
    const char *** rcargv)                                /* list of options */

	/*
 	 * Reads the options from the .sandboxrc file
	 * Returns TRUE if file was read.
 	 */


{
    FILE        * rcfile;                                  /* ptr to rc file */
    char        * options,               /* list of options found in sandbox */
                  token [ TOKEN_LENGTH ];               /* holds each option */
    int           alloc_arg = 10,                     /* available arguments */
                  ct;                                        /* misc counter */

  if (( rcfile = fopen ( rc_file_name, READ )) == NULL ) {
    ui_print ( VWARN,  "could not open %s.\n", rc_file_name );
    return ( FALSE );
  } /* if */
 
  options = get_options ( rcfile, command );

  if ( options == NULL || *options == NUL )
    return ( FALSE );              /* return immediately if options is empty */

  if (( *rcargv = (const char **) calloc ( 10, sizeof (char **))) == NULL ) {
    ui_print ( VWARN,  "could not calloc space for rc file options.\n" );
    return ( FALSE );
  } /* if */

  *rcargc = 0;            /* Allocate 10 args, realloc later if more needed. */

  for (;;) {
    if ( ! next_token ( &options, token ))
      return ( FALSE );

    if ( *token == NUL || *token == SPACE )
      break;

    rm_newline ( token );

    if ( *rcargc >= alloc_arg ) {
      alloc_arg += 10;

      if (( *rcargv = (const char **) realloc
		      ( *rcargv, sizeof (char **) * alloc_arg )) == NULL )
	return ( FALSE );
    } /* if */

    if (( (*rcargv)[*rcargc] = strdup ( token )) == NULL )
      return ( FALSE );

    (*rcargc)++;
  } /* for */

  ui_print ( VDEBUG, "Tokens are:\n" );
 
  for ( ct = 0; ct < *rcargc; ct++ )
    ui_print ( VCONT, "%s\n", (*rcargv)[ct] );

  fclose ( rcfile );
  return ( TRUE );
}                                                      /* get dotsbx options */



static char 	* get_options ( 
    FILE        * rc_file,                              /* file to read from */
    const char  * command)                          /* command to search for */

	/*
 	 * Generic procedure to find the options associated with a
	 * command in a given file.
 	 */


{
    char        * ptr,                          /* misc pointer to each line */
		* field,
                  buf [ PATH_LEN ],                           /* misc string */
		  options [ PATH_LEN ],
    		  options2 [ PATH_LEN ];

  *options = NUL;

  while ( fgets ( buf, PATH_LEN, rc_file ) != NULL ) {
    if (( ptr = strchr ( buf, '#' )) != NULL )          /* Check for comment. */
      *ptr = NUL;

    ptr = buf;
    field = nxtarg ( &ptr, WHITESPACE ); 

    if ( *field == NUL )
      continue;

   	/* For every line which matches the desired command,
    	 * add the list of options to the end of the options string.  */
    if ( gmatch ( command, field )) {
      concat ( options2, PATH_LEN, options, " ", ptr, NULL );
      strcpy ( options, options2 );
    } /* if */
  } /* while */

  ptr = options + strlen ( options ) - 1;
  return ( strdup ( options ));
}                                                             /* get options */



static BOOLEAN	next_token ( 
  char         ** str,                                    /* string to parse */
  char            token[])                                  /* token to fill */

	/* 
	 * This function extracts the next token from the string.
	 * Returns TRUE if there is not an illegal token.
	 */


{
  char 		* env;
  int             ct,                                        /* misc counter */
      		  expand,
		  braces;

  if ( *str != NULL )
    skip_white_space ( str );

  if ( *str == NULL || **str == NUL || **str == NEWLINE ) {
    *token = NUL;
    return ( TRUE );
  } /* if */

 	/* If a '$' is encountered, the remainder of the token is an
  	 *  environment variable and needs to be expanded. Strip off
  	 *  the '$' and record the fact that it needs to be expanded.  */
  expand = FALSE;
  braces = FALSE;

  if ( **str == '$' ) {
    expand = TRUE;
    (*str)++;

    if ( **str == '{' ) {
      braces = TRUE;
      (*str)++;
    } /* if */
  } /* if */

 	/* A token is either all of the characters within quotes or all of
  	 *  the characters between two areas of whitespace.  */
  ct = 0;

  if ( **str == '"' ) {
    (*str)++;

    while ( **str != NUL && **str != '"' ) {
      if ( ct >= TOKEN_LENGTH ) {
        ui_print (  VWARN, "token too long: %s.\n", *str );
        return ( FALSE );
      } /* if */

      token[ct++] = *(*str)++;
    } /* while */

    if ( **str == '"' )
      (*str)++;
  } /* if */
  
  else if ( braces ) {
    (*str)++;

    while ( **str != NUL && **str != '}' ) {
      if( ct >= TOKEN_LENGTH ) {
        ui_print (  VWARN, "token too long: %s.\n", *str );
        return ( FALSE );
      } /* if */

      token[ct++] = *(*str)++;
    } /* while */

    if(**str == '}')
      (*str)++;
  } /* else if */
  
  else {
    while ( ! WHITESPACEFN ( *str ) && **str != NUL ) {
      if( ct >= TOKEN_LENGTH ) {
        ui_print (  VWARN, "token too long: %s.\n", *str );
        return ( FALSE );
      } /* if */

      token[ct++] = *(*str)++;
    } /* while */
  } /* else */

  if ( ct < TOKEN_LENGTH )
    token[ct] = NUL;

 	/* If a '$' was encountered earlier, expand the token.  */
  if ( expand ) {
    env = getenv ( token );
    if ( strlen ( env ) > (unsigned) TOKEN_LENGTH ) {
      ui_print ( VWARN, "Environment variable too long: %s\n", env );
      return ( FALSE );
    } /* if */

    else
      strcpy ( token, env );

    if ( token == NULL )
      token = strdup ( EMPTY_STRING );
  } /* endif */

  return ( TRUE );
}                                                              /* next token */



static 	void skip_white_space ( 
    char       ** str)

	/* advance to none white space */


{
  while ( WHITESPACEFN (*str) && **str != NUL )
    (*str)++;
}                                                        /* skip white space */



static void
load_options ( 
    int           usrct,                     /* number of arguments to parse */
    const char ** usr_list,                             /* list of arguments */
    int           initct,                    /* count of lines to initialize */
    UIINIT      * init)                        /* initialization information */

	/*
 	 * Enters the arg_list into the ui information table.  First
	 * the built in's and special cases are checked, then the user
	 * entries.  Then load them according to type: first time,
	 * toggles, repeats of overwrite and accums.
	 * Returns ERROR if there is a failure; OK otherwise.
 	 */


{
    HTABLE      * hptr;                       /* points to one table pattern */
    ENTRY_SET   * eptr = NULL;                            /* points to entry */
    const char  * opt;                                   /* points to option */
    const char  * pattern;                        /* pattern to match option */
    int           ct = 0;                  /* count of arg list being parsed */
    int           usrct2;                     /* number of arguments to parse */
    char       ** usr_list2;                             /* list of arguments */

  usrct2 = usrct - 1;
  usr_list2 = usr_list2 + 1;

  while ( ct < usrct ) {
    opt = *(usr_list + ct);

    if (( pattern = built_ins ( opt )) == NULL ) {
      if ( special_case ( opt )) { /* try built in, special case before user */
	ct++;
	continue;
      } /* if */

      if  (( pattern = entry_pattern ( opt, init, initct )) == NULL )
	uiquit ( ERROR, TRUE, "No such option: %s.\n", opt );
    } /* if */
      
    if (( hptr = match_pattern ( pattern )) == NULL )
      uquit ( ERROR, FALSE, "Program error in ui function load_options.\n" );

    if ( hptr->num_entries == 0 )
      load_entry ( hptr, NULL, usr_list, opt, usrct, &ct );

    else if ( hptr->duplicates == TOGGLE )
      ui_unset ( pattern );

    else {
      set_duplicates ( hptr, &eptr, opt );
      load_entry ( hptr, eptr, usr_list, opt, usrct, &ct );
    } /* else */

    ct++;
  } /* while */
}                                                            /* load options */

static void
init_to_optlist (
  int		  initct,
  UIINIT	* init,
  char		* optbuf)
{
  int ct;
  int i=0;
  UIINIT	* initptr;

  initptr = init;
  for ( ct = 0; ct < initct; ct++, initptr++ ) {
    if ( *(initptr -> pattern) == '-' ) {
      optbuf [i++] = *(initptr -> pattern + 1);
      if ( initptr -> min_args == 1 )
        optbuf [i++] = ':';
      /* if */
    } /* if */
    
  } /* for */
  optbuf [ i ] = '\0';
  ui_print ( VDEBUG, "getopt option list is '%s'\n", optbuf );
} /* end init_to_optlist */

static void
load_using_posix (
    int           usrct,                     /* number of arguments to parse */
    char       ** usr_list,                             /* list of arguments */
    int           initct,                    /* count of lines to initialize */
    UIINIT      * init)                        /* initialization information */
{
  int errflg = 0;
  int c;
  char optbuf [100];
  char entry [3];
  HTABLE      * hptr;                       /* points to one table pattern */
  const char  * pattern;                        /* pattern to match option */

  entry [0] = '-';
  entry [2] = '\0';
  init_to_optlist ( initct, init, optbuf );

 /*
  * Tell getopt not to send diagnostic messages to stderr
  */
/*
  opterr = 0;
*/

 /*
  * Start at the first argument instead of skipping it.
  */
  optind = 0;

  while ( !errflg && ( c = getopt ( usrct, usr_list, optbuf ) ) != -1 ) {
    entry [1] = c;
    ui_print ( VDEBUG, "entry '%s'\n", entry );
    if  (( pattern = entry_pattern ( entry, init, initct )) == NULL )
      uiquit ( ERROR, TRUE, "No such option: %s.\n", entry );
    if (( hptr = match_pattern ( pattern )) == NULL )
      uquit ( ERROR, FALSE, "Program error in ui function load_options.\n" );

    load_posix_entry ( hptr, entry, optarg );
  } /* while */
} /* end load_using_posix */

static char * built_ins (
  const char * opt)                                    /* option to check */

	/*
	 * Checks to see if the opt matches one of the built-in
	 * patterns.
	 * Returns pointer to pattern if found; else NULL.
	 */


{
  if ( streq ( opt, AUTO_OP ))
    return ( strdup ( AUTO_OP ));

  if ( streq ( opt, INFO_OP ))
    return ( strdup ( INFO_OP ));

  if ( streq ( opt, RC_OP ))
    return ( strdup ( RC_OP ));

  return ( NULL );
}                                                               /* built ins */



static char *
entry_pattern ( 
    const char  * entry,                                   /* entry to match */
    UIINIT      * patlist,                 /* pattern list to search through */
    int           patct)                       /* number of patterns in list */

	/*
	 * Finds pattern to match entry in pattern list.  Purpose is to
	 * search pathlist in same order as program provided it.
	 * Returns pointer to pattern if found; else NULL.
	 */


{
    UIINIT      * pptr;                            /* points to pattern list */
    int           ct;                                        /* misc counter */

  for ( ct = 0; ct < patct; ct++ ) {
    pptr = patlist + ct;
    if ( gmatch ( entry, pptr->pattern ))
      return ( (char *)pptr->pattern );
  } /* for */

  return ( NULL );
}                                                           /* entry pattern */



static	BOOLEAN special_case (
    const char   * opt)                                    /* option to check */

	/*
	 * Checks option against internal, special values.  If there
	 * is a match, it carries out the appropriate action.
	 * Returns TRUE if opt matches a special case.
	 */


{
  if ( streq ( opt, QUIET_OP ))
    verbosity = VQUIET;
  else if ( streq ( opt, NORMAL_OP ))
    verbosity = VNORMAL;
  else if ( streq ( opt, VERBOSE_OP ))
    verbosity = VDETAIL;
  else if ( streq ( opt, DEBUG_OP ))
    verbosity = VDEBUG;

  else if ( streq ( opt, NOAUTO_OP )) {
    if ( ui_unset ( AUTO_OP ) != OK )
      ui_print ( VWARN, "Error in program; unable to unset %s.\n", AUTO_OP );
  } /* else if */

  else if ( streq ( opt, USAGE_OP )) {
    ui_restore_progname ();
    print_usage ();
    exit ( OK );
  } /* else if */

  else if ( streq ( opt, REV_OP ) || streq ( opt, VERSION_OP ) ) {
    ui_restore_progname ();
    ui_print_revision ();
    exit ( OK );
  } /* else if */

  else
    return ( FALSE );

  return ( TRUE );
}                                                            /* special case */



int set_ver ( 
    char        * opt)                                    /* option to check */

	/*
	 * Allows setting of verbosity.  This is a temporary function.
	 * Returns OK if verbosity level is legal, else ERROR.
	 */


{
  if ( streq ( opt, QUIET_OP ))
    verbosity = VQUIET;
  else if ( streq ( opt, NORMAL_OP ))
    verbosity = VNORMAL;
  else if ( streq ( opt, VERBOSE_OP ))
    verbosity = VDETAIL;
  else if ( streq ( opt, DEBUG_OP ))
    verbosity = VDEBUG;
  else
    return ( ERROR );

  return ( OK );
}  								  /* set ver */



int	ui_unset ( const char * pattern )

	/*
	 * Unsets the ui information for pattern.  Sets the pattern
	 * back to its initialized state.
	 * Returns ERROR if there is no such pattern; OK otherwise.
 	 */
{
    HTABLE      * hptr;                       /* points to one table pattern */
  
  if (( hptr = match_pattern ( pattern )) == NULL )
    return ( ERROR );

  hptr->entry = NULL;
  hptr->arg_list = NULL;
  hptr->entry_list = NULL;
  hptr->next_pattern = NULL;
  hptr->num_entries = hptr->num_args = 0;
  return ( OK );
}                                                                /* ui unset */



static 	int load_entry (
    HTABLE      * hptr,                   /* points to table pattern to fill */
    ENTRY_SET   * eptr,                           /* points to entry to load */
    const char ** usrlist,                /* list of user supplied arguments */
    const char  * entry,                    /* points to user supplied entry */
    int           usrct,                /* number of user supplied arguments */
    int         * pct)                      /* pointer to count of user list */

	/*
	 * Puts entry into place indicated by hptr and eptr.
	 */


{
  (hptr->num_entries)++;

  if ( hptr->num_entries > hptr->max_entries &&
       hptr->max_entries != UNLIMITED ) {
    uiquit ( ERROR, TRUE, "More than %d entr%s for option: %s.\n",
      hptr->max_entries, hptr->max_entries > 1 ? "ies" : "y", hptr->pattern );
    return ( ERROR );
  } /* if */

  if ( eptr == NULL ) {
    hptr->entry = entry;

    if ( load_min_args ( hptr, usrlist, &(hptr->arg_list), usrct,
		         &(hptr->num_args), pct ) == ERROR )
      return ( ERROR );

    load_max_args ( hptr, usrlist, &(hptr->arg_list), usrct,
		    &(hptr->num_args), pct );
  } /* if */

  else {
    eptr->entry = entry;

    if ( load_min_args ( hptr, usrlist, &(eptr->arg_list), usrct,
		         &(eptr->num_args), pct ) == ERROR )
      return ( ERROR );

    load_max_args ( hptr, usrlist, &(eptr->arg_list), usrct,
		    &(eptr->num_args), pct );
      return ( ERROR );
  } /* else */

  return ( OK );
}                                                              /* load entry */

void
#ifdef __STDC__
uiquit ( int status, ... )
#else
uiquit ( va_alist )
va_dcl
#endif

	/* This procedure takes a variable length argument list and
	   prints out the name of the function that failed, the error
	   message, usage if asked for, and then, if it is not a from
	   ui_load error, exits with the code entered.  This program
	   is actually uquit with a check for ui_load added. */

{
#ifndef __STDC__
    int         status;
#endif
    va_list     args;           /* see vprintf(3) and varargs(5) for details */
    int         usage;                                 /* do you print usage */
    char      * fmt;                                        /* format string */

  fflush ( stdout );
#ifdef __STDC__
  va_start ( args, status );
#else
  va_start ( args );
  status = va_arg ( args, int );         /* gets the first argument and type */
#endif
  usage = va_arg ( args, int );         /* gets the second argument and type */
  fmt = va_arg ( args, char * );

  if ( from_ui_load )
    fprintf ( stderr, "ERROR: " );

  else {
    ui_restore_progname ();
    fprintf ( stderr, ">> FATAL ERROR in %s:\n   ", progname );
  } /* if */

  vfprintf ( stderr, fmt, args );                 /* print out error message */

  if ( usage )
    print_usage ();

  va_end ( args );

  if ( ! from_ui_load )
    exit ( status );
}                                                                  /* uiquit */



static	int load_min_args (
    HTABLE      * hptr,                       /* points to one table pattern */
    const char ** usrlist,                /* list of user supplied arguments */
    ARG_SET    ** arglist,                     /* place to put argument list */
    int           usrct,                     /* the number of user arugments */
    int         * argct,             /* running total of arguments for entry */
    int         * pcount)                          /* counts current usrlist */

	/* 
	 * This function attempts to load the minimum number of required
	 * arguments into the entry.
	 */


{
    ARG_SET     * argptr=NULL;                   /* points to additonal args */

  if ( *arglist != NULL ) {
    argptr = *arglist;

    while ( argptr->next_arg != NULL )     /* bring argptr to end of arglist */
      argptr = argptr->next_arg;
  } /* if */

  while ( *argct < hptr->min_args ) {               /* get min number fields */
    if ( *pcount + 1 < usrct )
      (*pcount)++;

    else {
      uiquit ( ERROR, TRUE, "option %s requires %d argument%s.\n",
              hptr->pattern, hptr->min_args, hptr->min_args == 1 ? "" : "s" );
      return ( ERROR );
    } /* else */

    if ( ! match_arg_list ( *(usrlist + *pcount), hptr->legal_args )) {
      uiquit ( ERROR, TRUE,
	"argument: %s, not in %s's list of legal arguments.  List is:\n\t%s\n",
         *(usrlist + *pcount), hptr->pattern, hptr->legal_args );
      return ( ERROR );
    } /* if */

    if ( in_arg_list ( *arglist, *(usrlist + *pcount)))
      continue;

    if ( *arglist == NULL ) {
      *arglist = ( ARG_SET * ) malloc ( sizeof ( ARG_SET ));
      argptr = *arglist;
    } /* if */

    else {
      argptr->next_arg = ( ARG_SET * ) malloc ( sizeof ( ARG_SET ));
      argptr = argptr->next_arg;
    } /* else */

    argptr->arg = *(usrlist + *pcount);
    argptr->next_arg = NULL;
    (*argct)++;
  } /* while */

  return ( OK );
}                                                           /* load min args */

static	int load_posix_entry ( 
    HTABLE      * hptr,                       /* points to one table pattern */
    char	* entry,
    char	* arg)
	/* 
	 * This function attempts to load the minimum number of required
	 * arguments into the entry.
	 */
{
  ARG_SET     * argptr;                        /* points to additonal args */

  ui_print ( VDEBUG, "Loading entry '%s'.\n", entry );
  hptr -> num_entries = 1;
  hptr -> entry = strdup ( entry );

  if ( arg != NULL ) {
    if ( ! match_arg_list ( arg, hptr->legal_args )) {
      uiquit ( ERROR, TRUE,
  	"argument: %s, not in %s's list of legal arguments.  List is:\n\t%s\n",
           arg, hptr->pattern, hptr->legal_args );
      return ( ERROR );
    } /* if */
    hptr -> arg_list = ( ARG_SET * ) malloc ( sizeof ( ARG_SET ));
    argptr = hptr -> arg_list;

    argptr->arg = strdup (arg);
    argptr->next_arg = NULL;
    hptr -> num_args = 1;
  } /* if */
  return ( OK );
}                                                           /* load min args */



static	BOOLEAN match_arg_list (
    const char  * arg,                                  /* argument to check */
    const char  * list)             /* space separated list to check against */

	/*
	 * Tests the argument to see if it is in the legal list.
	 * Returns TRUE if it is; FALSE otherwise.
	 */


{
    char          copy [ PATH_LEN ],            /* don't touch original list */
                * listptr,                         /* points to copy of list */
                * token;                       /* holds each match from list */

  strcpy ( copy, list );
  listptr = copy;

  while ( (*(token = nxtarg ( &listptr, WHITESPACE ))) != NUL ) {
    if ( gmatch ( arg, token ))
      return ( TRUE );
  } /* while */

  return ( FALSE );
}                                                          /* match arg list */



static	BOOLEAN in_arg_list (
    ARG_SET     * list,                             /* list of args to check */
    const char  * arg)                              /* argument to check for */

	/*
	 * Checks to see if arg is already in list.
	 * Returns TRUE if arg is in list.
	 */


{
  while ( list != NULL ) {
    if ( streq ( list->arg, arg ))
      return ( TRUE );
    list = list->next_arg;
  } /* while */

  return ( FALSE );
}                                                             /* in arg list */



static	void load_max_args (
    HTABLE      * hptr,                       /* points to one table pattern */
    const char ** usrlist,                /* list of user supplied arguments */
    ARG_SET    ** arglist,                     /* place to put argument list */
    int           usrct,                     /* the number of user arugments */
    int         * argct,             /* running total of arguments for entry */
    int         * pcount)                          /* counts current usrlist */

	/* 
	 * This function attempts to load up to the maximum number of
	 * required arguments into the entry.
	 */


{
    ARG_SET     * argptr=NULL;                   /* points to additonal args */
    BOOLEAN       in = TRUE;                                 /* misc boolean */

  if ( *arglist != NULL ) {
    argptr = *arglist;

    while ( argptr->next_arg != NULL )     /* bring argptr to end of arglist */
      argptr = argptr->next_arg;
  } /* if */

  while ((( hptr->max_args == UNLIMITED ) || ( *argct < hptr->max_args )) &&
	    in ) {
    if ( *pcount + 1 < usrct ) {
      (*pcount)++;

      if (! (in = match_arg_list (*(usrlist + *pcount), hptr->legal_args )))
        (*pcount)--;

      else {
        if ( in_arg_list ( *arglist, *(usrlist + *pcount)))
          continue;

        if ( *arglist == NULL ) {
          *arglist = ( ARG_SET * ) malloc ( sizeof ( ARG_SET ));
          argptr = *arglist;
        } /* if */

        else {
          argptr->next_arg = ( ARG_SET * ) malloc ( sizeof ( ARG_SET ));
          argptr = argptr->next_arg;
        } /* else */

        argptr->arg = *(usrlist + *pcount);
        argptr->next_arg = NULL;
        (*argct)++;
      } /* else */
    } /* if */

    else
      in = FALSE;
  } /* while */
}                                                           /* load max args */



static	int set_duplicates (
    HTABLE      * hptr,                       /* points to one table pattern */
    ENTRY_SET  ** eeptr,                              /* points to entry ptr */
    const char  * entry)                                  /* points to entry */

	/*
	 * Sets counts and pointers appropriately for handling OVERWRITE
	 * and ACCUM duplicates.
	 */


{
  if ( hptr->max_entries == 1 ) {
    if ( ! streq ( hptr->entry, entry )) {
      if ( streq ( ARGS_OP, hptr->pattern ))
	uiquit ( ERROR, TRUE, "Only 1 argument allowed.\n" );
      else
	uiquit ( ERROR, TRUE, "More than 1 entry for option: %s.\n",
			       hptr->pattern );
      return ( ERROR );
    } /* if */

    hptr->num_entries = 0;

    if ( hptr->duplicates != ACCUM ) {           /* overwrite existing entry */
      hptr->num_args = 0;
      hptr->arg_list = NULL;
    } /* else */

    else if ( ! streq ( hptr->entry, entry )) {
      hptr->num_args = 0;                     /* can't accum different entry */
      hptr->arg_list = NULL;
    } /* else if */
  } /* if */

  else {
    match_entry ( hptr, entry, eeptr );
  
    if ( hptr->duplicates == OVERWRITE ) {
      if ( *eeptr == NULL ) {
	hptr->num_args = 0;
	hptr->arg_list = NULL;
      } /* if */

      else {
	(*eeptr)->num_args = 0;
	(*eeptr)->arg_list = NULL;
      } /* else */
    } /* if */
  } /* else */

  return ( OK );
}                                                          /* set duplicates */



static	void match_entry (
    HTABLE      * hptr,                       /* points to one table pattern */
    const char  * entry,                         /* option to actually match */
    ENTRY_SET  ** eeptr)                                  /* points to entry */

	/* 
	 * Finds matching entry and set the eptr to it.  If there is
	 * no entry yet or the new entry matches the first entry in
	 * the apattern, return eeptr as NULL.  If the new entry matches
	 * an existing entry, return a pointer to that entry.  Finally,
	 * if the entry does not exist, create space for it.
	 */


{
  *eeptr = NULL;

  if ( hptr->entry == NULL )
    return;
  
  if ( streq ( hptr->entry, entry )) {             /* entry already in place */
    (hptr->num_entries)--;       /* subtract entry as it will be added later */
    return;
  } /* if */

  if ( hptr->entry_list == NULL ) {
    hptr->entry_list = ( ENTRY_SET * ) malloc ( sizeof ( ENTRY_SET ));
    *eeptr = hptr->entry_list;
  } /* if */

  else {
    *eeptr = hptr->entry_list;

    if ( streq ( (*eeptr)->entry, entry )) {
      (hptr->num_entries)--;
      return;
    } /* if */

    while ( (*eeptr)->next_entry != NULL ) {
      *eeptr = (*eeptr)->next_entry;

      if ( streq ( (*eeptr)->entry, entry )) {
	(hptr->num_entries)--;
	return;
      } /* if */
    } /* while */

    (*eeptr)->next_entry = ( ENTRY_SET * ) malloc ( sizeof ( ENTRY_SET ));
    *eeptr = (*eeptr)->next_entry;
  } /* else */

  (*eeptr)->num_args = 0;
  (*eeptr)->arg_list = NULL;
  (*eeptr)->next_entry = NULL;
}                                                             /* match entry */



static	void print_htable (void)

	/*
	 * Prints out the initialized portion of the hash table.
	 */

{
    HTABLE      * hptr;                        /* points to each table entry */
    ARG_SET     * aptr;                                    /* points to args */
    ENTRY_SET   * eptr;                                 /* points to entries */
    int           ct;                                        /* misc counter */

  ui_print ( VDEBUG, "Contents of interface table:\n" );

  for ( ct = 0; ct < HTABLESIZE; ct++ ) {
    hptr = htbl + ct;
    aptr = hptr->arg_list;
    eptr = hptr->entry_list;

    if ( *(hptr->pattern) != NUL ) {
      print_internals ( hptr, aptr, eptr, ct );
      hptr = hptr->next_pattern;

      while ( hptr != NULL ) {
	aptr = hptr->arg_list;
        eptr = hptr->entry_list;
	print_internals ( hptr, aptr, eptr, -1 );
	hptr = hptr->next_pattern;
      } /* while */
    } /* if */
  } /* for */

  ui_print ( VCONT, "vl=%d; ap=%d; ip=%d.\n", verbosity, auto_pos, info_pos );
}                                                            /* print htable */



static	void print_internals (
    HTABLE      * hptr,                             /* points to table entry */
    ARG_SET     * aptr,                                    /* points to args */
    ENTRY_SET   * eptr,                                 /* points to entries */
    int           ct)                                         /* table entry */

	/*
	 * Prints out internals of a single hash table entry.
	 */


{
  if ( ct == -1 )
    ui_print ( VCONT, "    %s: maxe=%d; dup=%d; mina=%d; maxa=%d; a=%s.\n",
		       hptr->pattern, hptr->max_entries, hptr->duplicates,
		       hptr->min_args, hptr->max_args, hptr->legal_args  );
  else
    ui_print ( VCONT, "%-2d: %s: maxe=%d; dup=%d; mina=%d; maxa=%d; a=%s.\n",
		       ct, hptr->pattern, hptr->max_entries, hptr->duplicates,
		       hptr->min_args, hptr->max_args, hptr->legal_args  );

  if ( hptr->num_entries > 0 ) {
    ui_print ( VCONT, "    e=%s; numa=%d; nume=%d.\n",
	       hptr->entry, hptr->num_args, hptr->num_entries );
  
    while ( aptr != NULL ) {
      ui_print ( VCONT, "      arg: %s.\n", aptr->arg );
      aptr = aptr->next_arg;
    } /* while */

    while ( eptr != NULL ) {
      aptr = eptr->arg_list;
      ui_print ( VCONT, "    e=%s: numa=%d.\n", eptr->entry, eptr->num_args );
    
      while ( aptr != NULL ) {
	ui_print ( VCONT, "      arg: %s\n", aptr->arg );
	aptr = aptr->next_arg;
      } /* while */

      eptr = eptr->next_entry;
    } /* while */
  } /* if */
}                                                         /* print internals */


#ifdef __STDC__
int     ui_load ( const char * pattern, ... )
#else
int     ui_load ( va_alist )
    va_dcl
#endif
	/*
	 *  Looks for exact match to pattern and loads the table
	 *  entry.  It peals off the first two arguments to get the
	 *  pattern to match and the arg count.  It then creates a
	 *  **char list of the rest of the arguments and passes this
	 *  to the functions that load entries.
	 *  Returns OK if it is successful, else ERROR.
	 */

{
#ifndef __STDC__
    char        * pattern;                      /* pattern to match in table */
#endif
    HTABLE      * hptr;                             /* points to one pattern */
    va_list       arglist;                       /* variable length arg list */
    const char  * match;                                  /* misc string ptr */
    const char ** vlist=NULL;        /* holds list of arguments from arglist */
    int           argct,                     /* number of arguments to parse */
                  ct,                                       /* dummy counter */
		  rvalue = OK;	  			     /* return value */

  from_ui_load = TRUE;
  fflush ( stdout );
#ifdef __STDC__
  va_start ( arglist, pattern );             /* load first two fields */
#else
  va_start ( arglist );
  pattern = va_arg ( arglist, char * );             /* load first two fields */
#endif
  argct = va_arg ( arglist, int );

  if (( match = built_ins ( pattern )) == NULL ) {
    if ( special_case ( pattern )) {           /* try built in, special case */
      va_end ( arglist );
      from_ui_load = FALSE;
      return ( rvalue );
    } /* if */

    pattern = strdup ( (char *)pattern );
  } /* if */

  else
    pattern = match;

  if (( hptr = match_pattern ( pattern )) == NULL ) {
    ui_print ( VDEBUG, "pattern: %s, not in entry table.\n", pattern );
    rvalue = ERROR;
  } /* if */

  if ( rvalue != ERROR && hptr->max_entries != 1 ) {
    ui_print ( VDEBUG, "cannot ui_load pattern: %s, with max_entries: %d.\n",
			pattern, hptr->max_entries );
    rvalue = ERROR;
  } /* if */

  if ( rvalue != ERROR ) {
    if ( argct > 0 ) {
      argct++;                                        /* account for pattern */

      if (( vlist = ( const char ** ) calloc ( argct, sizeof ( char * )))
           == NULL )
	uquit ( ERROR, FALSE, "sys: could not calloc space for vlist.\n" );

      *vlist = pattern;

      for ( ct = 1; ct < argct; ct++ )
      *(vlist + ct) = strdup ( va_arg ( arglist, char * ));
    } /* if */

    ct = 0;

    if ( hptr->num_entries == 0 ) {
      if ( load_entry ( hptr, NULL, vlist, pattern, argct, &ct ) == ERROR )
	rvalue = ERROR;
    } /* if */

    else if ( hptr->duplicates == TOGGLE ) {
      if ( ui_unset ( pattern ) == ERROR )
	rvalue = ERROR;
    } /* else if */

    else {
      if ( set_duplicates ( hptr, NULL, pattern ) == ERROR )
	rvalue = ERROR;
      else if ( load_entry ( hptr, NULL, vlist, pattern, argct, &ct ) == ERROR )
	rvalue = ERROR;
    } /* else */
  } /* if */

  va_end ( arglist );
  from_ui_load = FALSE;
  return ( rvalue );
}                                                                 /* ui load */



BOOLEAN ui_is_set ( const char * pattern )

	/*
	 * Returns TRUE if pattern's entry is set; FALSE otherwise.
 	 */

{
    HTABLE      * hptr;                             /* points to one pattern */

  if (( hptr = match_pattern ( pattern )) == NULL ) {
    ui_print ( VDEBUG, "pattern: %s, not in entry table.\n", pattern );
    return ( FALSE );
  } /* if */

  return ( hptr->num_entries != 0 );
}                                                               /* ui is set */



int	ui_entry_cnt ( const char * pattern ) 
	/*
	 * Returns number of entries which match pattern; 0 if no entries
	 * matched the pattern; ERROR if no such pattern.
 	 */

{
    HTABLE      * hptr;                             /* points to one pattern */

  if (( hptr = match_pattern ( pattern )) == NULL ) {
    ui_print ( VDEBUG, "pattern: %s, not in entry table.\n", pattern );
    return ( ERROR );
  } /* if */

  return ( hptr->num_entries );
}                                                            /* ui entry cnt */



int	ui_arg_cnt ( const char * pattern, int entry_num )

	/*
	 * Returns number of arguments in entry; 0 if no entries matched
	 * the pattern; ERROR if no such pattern or entry_num.
 	 */

{
    HTABLE      * hptr;                             /* points to one pattern */
    ENTRY_SET   * eptr;                               /* points to one entry */
    int           ct;                                        /* misc counter */

  if ( entry_num < 1 ) {
    ui_print ( VDEBUG, "entry number: %d, is less than 1.\n", entry_num );
    return ( ERROR );
  } /* if */

  if (( hptr = match_pattern ( pattern )) == NULL ) {
    ui_print ( VDEBUG, "pattern: %s, not in entry table.\n", pattern );
    return ( ERROR );
  } /* if */

  if ( hptr->num_entries == 0 )
    return ( 0 );            /* can use this to check for existence of match */

  if ( entry_num == 1 )
    return ( hptr->num_args );

  if ( entry_num > hptr->num_entries ) {
    ui_print ( VDEBUG,
      "entry number: %d, greater than number of entries: %d.\n",
       entry_num, hptr->num_entries );
    return ( ERROR );
  } /* if */

  eptr = hptr->entry_list;

  for ( ct = 2; ct < entry_num; ct++ )
    eptr = eptr->next_entry;

  return ( eptr->num_args );
}                                                              /* ui arg cnt */



char * ui_entry_value ( const char * pattern, int entry_num )

	/*
 	 * Value of a particular entry of a particular pattern.
	 * Returns pointer to string; NULL if no such pattern or entry_num
	 * or pattern not matched.
 	 */

{
    HTABLE      * hptr;                             /* points to one pattern */
    ENTRY_SET   * eptr;                               /* points to one entry */
    int           ct;                                        /* misc counter */

  if ( entry_num < 1 ) {
    ui_print ( VDEBUG, "entry number: %d, is less than 1.\n", entry_num );
    return ( NULL );
  } /* if */

  if (( hptr = match_pattern ( pattern )) == NULL ) {
    ui_print ( VDEBUG, "pattern: %s, not in entry table.\n", pattern );
    return ( NULL );
  } /* if */

  if ( hptr->num_entries == 0 )
    return ( NULL );         /* can use this to check for existence of match */

  if ( entry_num == 1 )
    return ( (char *)hptr->entry );

  if ( entry_num > hptr->num_entries ) {
    ui_print ( VDEBUG,
      "entry number: %d, greater than number of entries: %d.\n",
       entry_num, hptr->num_entries );
    return ( NULL );
  } /* if */

  eptr = hptr->entry_list;

  for ( ct = 2; ct < entry_num; ct++ )
    eptr = eptr->next_entry;

  return ( (char *)eptr->entry );
}                                                          /* ui entry value */



char * ui_arg_value ( const char * pattern, int entry_num, int arg_num )

	/*
 	 * Value of a particular argument of a particular entry.
	 * Returns pointer to string; NULL if no such pattern, entry_num,
	 * or arg_num, also NULL if pattern not matched.
 	 */

{
    HTABLE      * hptr;                             /* points to one pattern */
    ENTRY_SET   * eptr;                               /* points to one entry */
    int           ct;                                        /* misc counter */

  if ( entry_num < 1 ) {
    ui_print ( VDEBUG, "entry number: %d, is less than 1.\n", entry_num );
    return ( NULL );
  } /* if */

  if ( arg_num < 1 ) {
    ui_print ( VDEBUG, "argument number: %d, is less than 1.\n", arg_num );
    return ( NULL );
  } /* if */

  if (( hptr = match_pattern ( pattern )) == NULL ) {
    ui_print ( VDEBUG, "pattern: %s, not in entry table.\n", pattern );
    return ( NULL );
  } /* if */

  if ( hptr->num_entries == 0 )
    return ( NULL );

  if ( entry_num == 1 )
    return ( arg_value ( hptr->arg_list, arg_num, hptr->num_args ));

  if ( entry_num > hptr->num_entries ) {
    ui_print ( VDEBUG,
      "entry number: %d, greater than number of entries: %d.\n",
       entry_num, hptr->num_entries );
    return ( NULL );
  } /* if */

  eptr = hptr->entry_list;

  for ( ct = 2; ct < entry_num; ct++ )
    eptr = eptr->next_entry;

  return ( arg_value ( eptr->arg_list, arg_num, eptr->num_args ));
}                                                            /* ui arg value */



static	char * arg_value (
    ARG_SET     * arglist,                       /* list of args to run down */
    int           argnum,                        /* argument number to reach */
    int           maxargs)                    /* maximum arguments available */

	/*
	 * Finds the argument in the arglist specified by the argnum.
	 * Returns pointer to this string; NULL if if cannot find string
	 * for any reason.
	 */


{
    int           ct;                                        /* misc counter */

  if ( argnum > maxargs ) {
    ui_print ( VDEBUG,
      "argument number: %d, greater than number of argument: %d.\n",
       argnum, maxargs );
    return ( NULL );
  } /* if */

  for ( ct = 1; ct < argnum; ct++ )
    arglist = arglist->next_arg;

  return ( (char *)arglist->arg );
}                                                               /* arg value */



int	ui_entries_to_argv ( const char * pattern, char *** entries )

	/*
 	 * Place all entries for a pattern into a char ** list.
 	 * Returns number of entries put in "entries"; 0 if no
	 * entries; ERROR if no such pattern or entry.
 	 */
{
    HTABLE      * hptr;                             /* points to one pattern */
    ENTRY_SET   * eptr;                               /* points to one entry */
    int           ct;                                        /* misc counter */

  if (( hptr = match_pattern ( pattern )) == NULL ) {
    ui_print ( VDEBUG, "pattern: %s, not in entry table.\n", pattern );
    return ( ERROR );
  } /* if */

  if ( hptr->num_entries == 0 )
    return ( 0 );

  if (( *entries = ( char ** ) calloc ( hptr->num_entries,
                                              sizeof ( char * )))
       == NULL )
    uquit ( ERROR, FALSE, "sys: could not calloc space for entry argv list.\n");

  **entries = (char *)hptr->entry;
  eptr = hptr->entry_list;

  for ( ct = 1; ct < hptr->num_entries; ct++ ) {
    *(*entries + ct) = (char *)eptr->entry;
    eptr = eptr->next_entry;
  } /* for */

  return ( hptr->num_entries );
}                                                      /* ui entries to argv */



int	ui_args_to_argv (
    const char * pattern,                        /* pattern to match in table */
    int         entry_num,               /* entry number of matching pattern */
    char *** arguments)                          /* list to hold argument */

	/*
 	 * Place all arguments for a particular entry of a pattern into
	 * a char ** list.
 	 * Returns number of arguments put in "arguments"; 0 if no
	 * arguments; ERROR if no such pattern or entry_num.
 	 */

{
    HTABLE      * hptr;                             /* points to one pattern */
    ENTRY_SET   * eptr;                               /* points to one entry */
    ARG_SET     * aptr;                            /* points to one argument */
    int           total,                                     /* misc integer */
                  ct;                                        /* misc counter */

  if ( entry_num < 1 ) {
    ui_print ( VDEBUG, "entry number: %d, is less than 1.\n", entry_num );
    return ( ERROR );
  } /* if */

  if (( hptr = match_pattern ( pattern )) == NULL ) {
    ui_print ( VDEBUG, "pattern: %s, not in entry table.\n", pattern );
    return ( ERROR );
  } /* if */

  if ( hptr->num_entries == 0 )
    return ( 0 );

  if ( entry_num == 1 ) {
    aptr = hptr->arg_list;
    total = hptr->num_args;
  } /* if */

  else {
    if ( entry_num > hptr->num_entries ) {
      ui_print ( VDEBUG,
	"entry number: %d, greater than number of entries: %d.\n",
	 entry_num, hptr->num_entries );
      return ( ERROR );
    } /* if */

    eptr = hptr->entry_list;

    for ( ct = 2; ct < entry_num; ct++ )
      eptr = eptr->next_entry;

    aptr = eptr->arg_list;
    total = eptr->num_args;
  } /* else */

  if ( total == 0 )
    return ( 0 );

  if (( *arguments = ( char ** ) calloc ( total, sizeof ( char * )))
        == NULL )
    uquit ( ERROR, FALSE, "could not calloc space for argument argv list.\n" );

  for ( ct = 0; ct < total; ct++ ) {
    *(*arguments + ct) = (char *)aptr->arg;
    aptr = aptr->next_arg;
  } /* for */

  return ( total );
}                                                         /* ui args to argv */



char * 	ui_ver_switch (void)

	/*
 	 * Returns a command line switch, "string", corresponding to
	 * the current verbosity level.
 	 */

{
  switch ( verbosity ) {
    case VQUIET  : return ( strdup ( QUIET_OP ));
    case VNORMAL : return ( strdup ( NORMAL_OP ));
    case VDETAIL : return ( strdup ( VERBOSE_OP ));
    case VDEBUG  : return ( strdup ( DEBUG_OP ));
    default      : ui_print ( VFATAL, "illegal level of verbosity: %d.\n",
				       verbosity );
    		   return ( NULL );
  } /* switch */
}                                                       /* ui verbose switch */



BOOLEAN ui_is_auto (void)

	/*
 	 * Shortcut function.
	 * Returns is -auto set?
 	 */
{
  return ( (htbl+auto_pos)->num_entries != 0 );
}                                                              /* ui is auto */



BOOLEAN ui_is_info (void)

	/*
 	 * Shortcut function.
	 * Returns is -info set?
 	 */
{
  return ( (htbl+info_pos)->num_entries != 0 );
}                                                              /* ui is info */



int 	ui_ver_level (void)

	/*
 	 * Returns the current verbosity level as an integer level.
 	 */
{
  return ( verbosity );
}                                                            /* ui ver level */

void
ui_print_revision (void)
{
  ui_print ( VALWAYS, "program :  %s\nrelease :  %s\nlibode  :  %s\n",
                      progname, BUILD_VERSION, BUILD_DATE );
}
