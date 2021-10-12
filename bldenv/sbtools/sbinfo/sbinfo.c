static char sccsid[] = "@(#)79  1.1  src/bldenv/sbtools/sbinfo/sbinfo.c, bldprocess, bos412, GOLDA411a 4/29/93 12:27:25";
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
**                               April 1990                                  **
*******************************************************************************
**
**  Description:
** 	This program prints out a description of the current sandbox
**	environment.
**
**  Functions:
**    main (int, * * char) int
**      print_sandbox (* char, * struct) int
**        print_envvar (* struct, int) int
**        print_field (* struct, * char, int) int
**    print_usage () int
 */

static char * rcsid =
 "$RCSfile: sbinfo.c,v $ $Revision: 1.9.2.2 $ $Date: 92/03/25 22:47:13 $";

#  include <sys/param.h>
#  include <ode/odedefs.h>
#  include <ode/parse_rc_file.h>

#  define  MAX_ARGS	2

    UIINIT init [] = {                      /* initialize the user interface */
   		{ SB_OP,   1, 	      OVERWRITE, 1, 1, ARGS_OP },
   		{ ARGS_OP, UNLIMITED, OVERWRITE, 0, 0, "" }
    };

    char        * progname = "sbinfo";                     /* program's name */
    struct      rcfile   rcfile;      /* structure to hold rc file internals */


main ( argc, argv )
	
	/*
	 * Establishes the environment and parses the arguments then
	 * calls the print routine.
	 */

    int         argc;                /* the number of command line arugments */
    char     ** argv;                          /* strings with each argument */

{
    char        * sandbox,                                /* name of sandbox */
                * basedir = NULL,                /* sandbox's base directory */
                * sb_rcfile,                              /* sandbox rc file */
                * usr_rcfile;                              /* user's rc file */

  ui_init ( argc, argv, MAX_ARGS, init );
  sandbox = ui_arg_value ( SB_OP, 1, 1 );
  sb_rcfile = ui_arg_value ( SB_RC_OP, 1, 1 );
  usr_rcfile = ui_arg_value ( RC_OP, 1, 1 );
  current_sb ( &sandbox, &basedir, &sb_rcfile, &usr_rcfile );

  if ( parse_rc_file ( sb_rcfile, &rcfile ) != OK )
						 /* read sandbox description */
    quit ( ERROR, "%s: unable to parse %s sandbox description\n",
		   progname, sandbox );

  print_sandbox ( sandbox, &rcfile );
  return ( OK );
}                                                                    /* main */



print_sandbox ( sandbox, rcfile_p )

	/*
	 * Goes through the list of variables to print and has them
	 * printed.  Checks to see if all should be printed.
	 */

    char 	* sandbox;
    struct 	rcfile   * rcfile_p;

{
    struct 	field    * field_p;
    struct 	arg_list * args_p;
    char 	* indent;
    char 	* spaces = "            ";
    int 	  ct;
    BOOLEAN 	  all = ! ui_is_set ( ARGS_OP );

  indent = spaces + strlen ( spaces );

  if ( all ) {
    ui_print ( VNORMAL, "Description of rc_file for %s sandbox:\n", sandbox );
    indent -= 4;

    if ( rc_file_field ( rcfile_p, "setenv", &field_p ) == 0 ) {
      ui_print ( VNORMAL, "\nThe following environment variables were set:\n");
      for  ( args_p = field_p->args; args_p != NULL; args_p = args_p->next )
	print_envvar ( args_p, all );
    } 

    if ( rc_file_field ( rcfile_p, "unsetenv", &field_p ) == 0 ) {
      ui_print ( VNORMAL,
		 "\nThe following environment variables were removed:\n" );
      for ( args_p = field_p->args; args_p != NULL; args_p = args_p->next )
	ui_print ( VNORMAL, "    ", args_p->tokens[0] );
    }
  } /* if */

  else if ( rc_file_field ( rcfile_p, "setenv", &field_p ) == 0 ) {
    for ( args_p = field_p-> args; args_p != NULL; args_p = args_p-> next ) {
      for ( ct = 1; ct <= ui_entry_cnt ( ARGS_OP ); ct++ ) {
	if ( streq ( ui_entry_value ( ARGS_OP, ct ), args_p-> tokens [ 0 ] ))
	  break;
      } /* for */

      if ( ct > ui_entry_cnt ( ARGS_OP ))
	continue;

      print_envvar ( args_p, all );
    } /* for */
  } /* else if */

  if ( all )
    ui_print ( VNORMAL,
	       "\nThe following directives are defined in the rc file:\n" );

  for ( field_p = rcfile_p->list; field_p != NULL; field_p = field_p->next ) {
    if ( all ) {
      if ( streq ( field_p->name, "setenv" ))
	continue;
      if (streq ( field_p->name, "unsetenv" ))
	continue;
      print_field ( field_p, indent, all );
      continue;
    }

    for ( ct = 1; ct <= ui_entry_cnt ( ARGS_OP ); ct++ ) {
      if ( streq ( ui_entry_value ( ARGS_OP, ct ), field_p->name ))
	break;
    } /* for */

    if ( ct > ui_entry_cnt ( ARGS_OP ))
      continue;

    print_field ( field_p, indent, all );
  }
}                                                           /* print sandbox */



print_envvar ( var, all )

	/*
	 * Prints a piece of information if the environment variable
	 * is set.
	 */

    struct 	arg_list * var;
    int 	  all;

{
    char 	* val;

  if ( var->ntokens != 1 )
    return;

  if (( val = getenv ( var->tokens[0] )) == NULL )
    return;

  if ( all )
    ui_print ( VALWAYS, "    %s=%s\n", var->tokens[0], val );
  else
    ui_print  ( VALWAYS, "%s\n", val );
}                                                            /* print envvar */



print_field ( field_p, indent, all )

	/*
	 * Prints a piece of information
	 */

    struct 	field * field_p;
    char 	* indent;
    int 	  all;

{
    struct 	arg_list * args_p;
    int 	  i;

  if (( args_p = field_p->args)->next == NULL ) {
    if ( args_p->ntokens == 0 ) {
      ui_print ( VALWAYS, "%s%s\n", indent, field_p->name );
      return;
    } /* if */

    ui_print ( VALWAYS, "%s%s%s%s", indent, all ? field_p->name : "",
	       		 all ? ": " : "", args_p->tokens[0]);

    for ( i = 1; i < args_p->ntokens; i++ )
      ui_print ( VALWAYS, " %s", args_p->tokens[i] );

    (void) putchar ('\n');
    return;
  } /* if */

  if ( all ) {
    ui_print ( VALWAYS, "%s%s:\n", indent, field_p->name );
    indent -= 4;
  } /* if */

  do {
    if ( args_p->ntokens == 0 ) {
      ui_print ( VALWAYS, "%s%s\n", indent, field_p->name );
      continue;
    } /* if */

    ui_print ( VALWAYS, "%s%s", indent, args_p->tokens[0] );

    for ( i = 1; i < args_p->ntokens; i++ )
	ui_print ( VALWAYS, " %s", args_p->tokens[i] );

    (void) putchar( NEWLINE );
    args_p = args_p->next;
  } while ( args_p != NULL );
}                                                             /* print field */



print_usage ()
	
	/*
	 * This procedure prints the usages for workon.
	 */

{
    printf ( "USAGE:\n" );
    printf ( "%s [ sb_opts ] [field(s)]\n", progname );
    printf ( "       field(s): specific variable to give the value of\n" );
    printf ( "       sb_opts:\n" );
    printf ( "\t-sb <sandbox>, -rc <user rc file>, -sb_rc <sb rc file>\n" );
    printf ( "%s -usage | -rev\n", progname );
}                                                             /* print usage */
