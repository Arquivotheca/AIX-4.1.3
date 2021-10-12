static char sccsid[] = "@(#)83  1.2  src/bldenv/sbtools/workon/workon.c, bldprocess, bos412, GOLDA411a 8/19/93 07:06:48";
/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: cd_to_src_path
 *		check_setdir
 *		cmdline_syntax
 *		create_sub_shell
 *		isdir
 *		legal_list_options
 *		legal_sandbox
 *		legal_setdir
 *		legal_setname
 *		legal_undo_options
 *		list_sets
 *		main
 *		print_usage
 *		remove_setname
 *		run_bco
 *		set_workon_var
 *		undo_set
 *		write_set
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1990,1992
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
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
 * ODE 2.1.3
 */
/******************************************************************************
**                          Open Software Foundation                         **
**                              Cambridge, MA                                **
**                              Randy Barbano                                **
**                               April 1990                                  **
*******************************************************************************
**
**  Description:
**	This program creates a "set" and/or puts the user into that set in
**	a sub shell.
**
**  Functions:
**    main (int, ** char) int
**      cmdline_syntax (** char, ** char, ** char, ** char, ** char,
**			** char, * int) int
**        legal_sandbox (** char, ** char, ** char, ** char) int
**        legal_list_options () int
**        legal_setdir (** char) int
**        legal_setname (** char, ** char, ** char, ** char, * char, * int) int
**        legal_undo_options () int
**      list_sets (* char, * char, * char) int
**      cd_to_src_path (struct) int
**        isdir (* char) int
**      undo_set (* char, * char, * char, * char) int
**        remove_setname (* char, * char, * char) int
**      write_set (* char, * char, * char, * char, ** char, struct) int
**        check_setdir (** char) int
**        run_bco (* char, * char, * char) int
**      check_setdir 84
**      set_workon_var () int
**      create_sub_shell (* char) int
**    print_usage () int
**
**  Changes Made:
**    04/10/93 - Michael Winestock
**               Added code to main to build the PATH for the new subshell
**               as PATH listed in rc_files/shared prepended to the PATH for
**               the current shell.
**    08/18/93 - Linda Croix
**               Added code to main to allow the default sandbox to no longer
**               exist and no set name is required.
 */

static char * rcsid =
 "$RCSfile: workon.c,v $ $Revision: 1.11.2.2 $ $Date: 92/03/25 22:47:38 $";

#  include <ode/odedefs.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <sys/file.h>
#  include <math.h>
#  include <ode/parse_rc_file.h>

#  define  MAX_ARGS     6
#  define  BCOARGS      20
#  define  WORKON       "WORKON"          /* env variable with nested levels */
#  define  NO_EDIT      "cat"                  /* just cat the file, no edit */
#  define  SRC_CONTROL  "src_control_init"       /* init src control pattern */
#  define  ECHO         "/bin/echo"                          /* echo command */

    UIINIT init [] = {                      /* initialize the user interface */
      		{ SB_OP,     1, OVERWRITE, 1, 1, ARGS_OP },
      		{ SETDIR_OP, 1, OVERWRITE, 1, 1, ARGS_OP },
      		{ NOSH_OP,   1, OVERWRITE, 0, 0, "" },
      		{ UNDO_OP,   1, OVERWRITE, 0, 0, "" },
      		{ LIST_OP,   1, OVERWRITE, 0, 0, "" },
      		{ ARGS_OP,   1, OVERWRITE, 0, 0, "" }
    };

    char * progname;


main ( argc, argv )

  	/* This function checks the command line arguments and makes
	   sure they are syntactically correct.  This is done using
	   the library function interface.  If this is correct, the
	   dependencies are checked.  Errors lead to usage messages.
	   If all is okay, the primary procedure is called. */

    int         argc;                /* the number of command line arugments */
    char     ** argv;                          /* strings with each argument */

{
    char      * sb,                                          /* sandbox name */
              * env_var,     /* temporary environment variable pointer (MJW) */
              * shell_path,             /* holds path of current shell (MJW) */
              * new_path,               /* holds new path + shell_path (MJW) */
              * basedir = NULL,                     /* holds path to sandbox */
              * sb_rcfile,                           /* sandbox rc file name */
              * rc_file,                                     /* user rc file */
              * setname,                           /* holds full name of set */
              * setdir;                               /* holds set directory */
    struct      rcfile  rc_contents;    /* structure with contents of rcfile */
    BOOLEAN     insetsfile = FALSE;             /* is found in rc_files/sets */
    FILE      *lrcfile;
  if (argc > 0)
    progname = argv[0];
  else
    progname = "workon";

  ui_init ( argc, argv, MAX_ARGS, init );
  cmdline_syntax ( &sb, &basedir, &sb_rcfile, &rc_file, &setname,
		   &setdir, &insetsfile );

  if ( ui_is_set ( LIST_OP ))
    list_sets ( sb, basedir, rc_file );

  else {

/* --------------- Added code by Michael Winestock (MJW). ------------------ */

    env_var = getenv ( "PATH" );              /* Get path for current shell. */

    if ( env_var == NULL )
      env_var = "";             /* Need a default value if PATH not defined. */

    /* Allocate storage to store shell path.                                 */

    shell_path = malloc ( strlen ( env_var ) + 1 );

    if ( shell_path == NULL ) {
      uquit ( ERROR, FALSE,
              "No MEMORY available to store current PATH value!\n" );
    }

    /* Store a copy of shell path for later use.                             */

    strcpy ( shell_path, env_var );

/* ------------------------------------------------------------------------- */

    bzero ( &rc_contents, sizeof ( rc_contents ));

/* ------------Added code by Linda Croix ------------------------------------*/
/* This fixes the problem that workon dies when the default sandbox no       */
/* No longer exists.  It will now ignore the default sb rc file              */
/* Added if statement to check for the files existance before the library    */
/* call is made to parse the file.  If it no longer exists it does not call  */
/* parse_rc_file.                                                            */

    if ((lrcfile = fopen( sb_rcfile, "r")) == NULL )
      printf( "Unable to parse default sandbox rcfile %s.\n", sb_rcfile );
    else if ( parse_rc_file ( sb_rcfile, &rc_contents ))
      uquit ( ERROR, FALSE, "unable to parse sandbox rcfile %s.\n", sb_rcfile );
/* --------------- End of code added by Linda Croix (LMC). ------------------*/
/* --------------- Added code by Michael Winestock (MJW). ------------------ */

    env_var = getenv ( "PATH" );              /* Get path for new shell.     */

    if ( env_var == NULL )
      env_var = "";             /* Need a default value if PATH not defined. */

    /* Allocate storage to store shell path.                                 */

    new_path = malloc ( strlen ( env_var ) + strlen ( shell_path ) + 2 );

    if ( new_path == NULL ) {
      uquit ( ERROR, FALSE,
              "No MEMORY available to store new PATH value!\n" );
    }

    /* Store a copy of shell path for later use.                             */

    sprintf ( new_path, "%s:%s", env_var, shell_path );

    /* Store the new path back out to the environment for the new subshell.  */

    setenv ( "PATH", new_path, TRUE );

    free ( shell_path );
    free ( new_path );

/* ------------------------------------------------------------------------- */

/*  -------------the old code looked like this ---------------------

    cd_to_src_path ( rc_contents );

------------------------------------------------*/
/*  -------------my code ---------------------*/

    if ( ! ui_is_set ( NOSH_OP )) {
      cd_to_src_path ( rc_contents );
    } /* if */

/*  --------------------------------------------*/

    if ( ui_is_set ( UNDO_OP ))
      undo_set ( sb, basedir, rc_file, setname );

    else {                                               /* do actual workon */
      if ( ! insetsfile )
        write_set ( sb, basedir, rc_file, setname, &setdir, rc_contents );

      else
	check_setdir ( &setdir );

      if ( ! ui_is_set ( NOSH_OP )) {
	set_workon_var ();
	create_sub_shell ( setdir );
      } /* if */
    } /* else */
  } /* else */

  return ( OK );
}                                                                    /* main */




cmdline_syntax ( sb, sbbase, sb_rcfile, rc_file, setname, setdir, in_setsfile )

	/* This procedure checks for relationships between the
	   command line arguments.  It assumes the syntax is
	   already correct.  Most of the functions it calls
	   will use uquit to exit if there is an error. */

    char     ** sb,                                          /* sandbox name */
             ** sbbase,                             /* holds path to sandbox */
             ** sb_rcfile,                                /* sandbox rc file */
             ** rc_file,                                     /* user rc file */
             ** setname,                               /* holds full setname */
             ** setdir;                             /* name of set directory */
    BOOLEAN   * in_setsfile;                    /* is found in rc_files/sets */

{
    char      * env_ptr;                         /* holds environment values */

  legal_sandbox ( sb, sbbase, sb_rcfile, rc_file );
  env_ptr = getenv ( SANDBOX );

  if ( ui_is_set ( SB_OP ) || env_ptr != NULL ) {
    if ( setenv ( SANDBOX, *sb, TRUE ) == ERROR )
      uquit ( ERROR, FALSE, "\tSANDBOX setenv failure.\n" );

    ui_print ( VDETAIL, "setting environment variable %s to: %s.\n", SANDBOX, 
               *sb );
  } /* if */

  if ( ui_is_set ( LIST_OP ))
    legal_list_options ();

  else {
    env_ptr = getenv ( BCSSET );
    legal_setdir ( setdir );
    legal_setname ( setname, sb, rc_file, setdir, env_ptr, in_setsfile );

    if ( ui_is_set ( ARGS_OP ) || env_ptr != NULL ) {
      if ( setenv ( BCSSET, *setname, TRUE ) == ERROR )
	uquit ( ERROR, FALSE, "\tBSCSET setenv failure.\n" );
      
      ui_print ( VDETAIL, "setting environment variable %s to: %s.\n",
		 BCSSET, *setname );
    } /* if */

    if ( ui_is_set ( UNDO_OP ))
      legal_undo_options ();
  } /* else */
}                                                          /* cmdline syntax */




legal_sandbox ( sb, sbbase, sb_rcfile, rc_file )

	/* This procedure checks to see if the sandbox entered is
	   legal.  It gets the path to the sandbox, the alternate
	   sb rcfile, and the user rcfile if they aren't entered. */

    char     ** sb,                                          /* sandbox name */
             ** sbbase,                             /* holds path to sandbox */
             ** sb_rcfile,                                /* sandbox rc file */
             ** rc_file;                                     /* user rc file */

{
  *sb = ui_arg_value ( SB_OP, 1, 1 );
  *rc_file = ui_arg_value ( RC_OP, 1, 1 );
  *sb_rcfile = ui_arg_value ( SB_RC_OP, 1, 1 );

  if ( current_sb ( sb, sbbase, sb_rcfile, rc_file ) == ERROR ) {
    if ( *sb == NULL )
      uquit ( ERROR, FALSE, "\tUnable to find default sandbox.\n" );
    else
      uquit ( ERROR, FALSE, "\tUnable to find base for sandbox %s.\n", *sb );
  } /* if */

  if ( ui_ver_level () >= VDEBUG ) {
    ui_print ( VDEBUG, "sandbox is: %s\n", *sb );
    ui_print ( VCONT, "path to sandbox is: %s\n", *sbbase );
    ui_print ( VCONT, "sandbox's local, rc file is: %s\n", *sb_rcfile );
    ui_print ( VCONT, "user's rc file is: %s\n", *rc_file );
  } /* if */
}                                                           /* legal sandbox */



legal_list_options ( )

	/* This procedure checks that the options passed with
	   the list flag are legal. */

{
  if ( ui_is_set ( SETDIR_OP ))
    ui_print ( VWARN, "-setdir option ignored; cannot be used with -list.\n" );
	    
  if ( ui_is_set ( NOSH_OP ))
    ui_print ( VWARN, "-nosh option ignored; cannot be used with -list.\n" );
  
  if ( ui_is_set ( ARGS_OP ))
    ui_print ( VWARN, "set name ignored; cannot be used with -list.\n" );
  
  if ( ui_is_set ( UNDO_OP ))
    uquit ( ERROR, TRUE, "\t-list option cannot be used with -undo.\n" );
}                                                      /* legal list options */




legal_setdir ( setdir )

	/* This procedure checks to see if there is a setdir.
	   If not, it creates a default "." setdir but does not
	   set the is_set field as the user did not set it. */

    char        ** setdir;               /* points to the path of the setdir */
{
  *setdir = ui_arg_value ( SETDIR_OP, 1, 1 );

  if ( *setdir == NULL )
    *setdir = salloc ( "." );

  ui_print ( VDEBUG, "set directory is: %s.\n", *setdir );
}                                                            /* legal setdir */




legal_setname ( setname, sb, rcfile, setdir, set_ptr, in_setfile )

	/* This procedure checks to see that the set name entered
	   is legal.  If it is, it puts the user's name in front of
	   it if it isn't already there and if the name doesn't
	   start with a capital letter.  A captial letter indicates
	   a shared set. */

    char     ** setname,                           /* will hold full setname */
             ** sb,                                   /* name of the sandbox */
             ** rcfile,                          /* name of rc file to check */
             ** setdir,                           /* holds setdir to move to */
              * set_ptr;                            /* holds value of BCSSET */
    BOOLEAN   * in_setfile;                     /* is found in rc_files/sets */

{
    char        tmp_name [ NAME_LEN ],                        /* misc string */
                tsetname [ PATH_LEN ],                  /* temporary setname */
              * tsetdir = NULL,                        /* temp ptr to setdir */
              * env_ptr,                              /* point to env string */
              * ptr;                                         /* misc pointer */
    int         ct;                                          /* misc counter */

  *setname = ui_entry_value ( ARGS_OP, 1 );
  
  if ( *setname == NULL ) {                      /* no user provided setname */
    if ( current_set ( setname, &tsetdir, sb, rcfile ) == ERROR ) {
      if ( set_ptr != NULL )
	uquit ( ERROR, FALSE, 
	      "\tset, %s, from env var, %s, not found in sandbox, %s.\n",
		set_ptr, BCSSET, sb );
      else
/*  Line added by Linda Croix to allow a default set name not to be specified */
        *setname = salloc ( "x" );
/*	uquit ( ERROR, FALSE, "\tno default set name; none entered.\n" );     */
    } /* if */

    if ( ! ui_is_set ( SETDIR_OP ))
      *setdir = tsetdir;

    *in_setfile = TRUE;
  } /* if */

  for ( ct = 0; ct < strlen ( *setname ); ct++ ) {
    if (( **setname + ct == PERIOD ) ||                  /* legal chars only */
        ( **setname + ct == SLASH ) ||
        ( **setname + ct == DASH ))
      uquit ( ERROR, FALSE, "\tsetname: %s, %s'%c' or '%c' or '%c'.\n",
              *setname, "has one of the following characters in it:\n      ",
	      PERIOD, DASH, SLASH );
  } /* for */

  if (( env_ptr = getenv ( "USER" )) == NULL )           /* insert user name */
    uquit ( ERROR, FALSE, "\tUSER not found in environment.\n" );

  ptr = concat ( tmp_name, NAME_LEN, env_ptr, "_", NULL );

		/* prepend user name if the setname does not start with
	   	   a capital letter and it isn't already there */

  if ((( **setname < 'A' ) || ( **setname > 'Z' )) &&
      ( strncmp ( tmp_name, *setname, ptr - tmp_name ))) {
    concat ( tsetname,  NAME_LEN, tmp_name, *setname, NULL );
    *setname = salloc ( tsetname );
  } /* if */

  ui_print ( VDEBUG, "name of set is: %s.\n", *setname );
}                                                           /* legal setname */




legal_undo_options ()

	/* This procedure checks that the options passed with
	   the undo flag are legal. */

{
  if ( ui_is_set ( SETDIR_OP ))
    ui_print ( VWARN, "-setdir option ignored; cannot be used with -undo.\n" );
	    
  if ( ui_is_set ( NOSH_OP ))
    ui_print ( VWARN, "-nosh option ignored; cannot be used with -undo.\n" );
}                                                      /* legal undo options */



list_sets ( sb, sbbase, rc_file )

	/* This function lists the sets, including the current
	   default one. */

    char      * sb,                                          /* sandbox name */
              * sbbase,                             /* holds path to sandbox */
              * rc_file;                                     /* user rc file */

{
    FILE      * ptr_file;                                  /* ptr to rc file */
    char        line [ STRING_LEN ],                    /* line from rc file */
                file [ PATH_LEN ],           /* holds set file path and name */
              * setname = NULL,                        /* holds full setname */
              * setdir = NULL,                        /* dummy set directory */
              * line_ptr,                                     /* misc string */
              * token;                                        /* misc string */

  if ( current_set ( &setname, &setdir, &sb, &rc_file ) == OK )
    ui_print ( VALWAYS, "Current set for sandbox %s %s is\n\tset: %s.\n",
               sb, "(environment variable or default)", setname );
  else
    ui_print ( VALWAYS, "No current default set for sandbox: %s.\n", sb );

  ui_print ( VCONT,
	"Existing sets and default set directories for sandbox %s:\n", sb );
  concat ( file, PATH_LEN, sbbase, "/", sb, "/", SET_RC, NULL );

  if (( ptr_file = fopen ( file, READ )) == NULL )
    uquit ( ERROR, FALSE, "\tcannot open set rcfile %s.\n", file );

  while (( line_ptr = fgets ( line, STRING_LEN, ptr_file )) != NULL ) {
    token = nxtarg ( &line_ptr, WHITESPACE );

    if ( streq ( token, SET_KEY )) {
      token = nxtarg ( &line_ptr, WHITESPACE );
      ui_print ( VCONT, "  set  %s;", token );
      token = nxtarg ( &line_ptr, WHITESPACE );
      ui_print ( VALWAYS, "\t setdir  %s\n", token );
    } /* if */
  } /* while */

  fclose ( ptr_file );
  return ( FALSE );
}                                                               /* list sets */



cd_to_src_path ( contents )

	/* This procedure determines the source directory for
	   the user and then does a cd to it. */

    struct      rcfile  contents;           /* to hold contents of sb rcfile */

{
    char      * src_dir,                 /* points to string with source dir */
                src_dir_file [ PATH_LEN ];   /* string to add phoney file to */

  if ( get_rc_value ( SOURCE_BASE, &src_dir, &contents, TRUE ) != OK )
    uquit ( ERROR, FALSE, "\tcannot continue without this information.\n" );

  if ( *src_dir != SLASH )
    uquit ( ERROR, FALSE, "\tvalue of %s field does not begin with a %c.\n",
			SOURCE_BASE, SLASH );

  if ( isdir ( src_dir ) == ERROR ) {
    concat ( src_dir_file, sizeof ( src_dir_file ), src_dir, "/.", NULL );

    if ( makepath ( src_dir_file, NULL, TRUE, TRUE ) == ERROR )
      uquit ( ERROR, FALSE, "\tcould not create path %s.\n", src_dir );
  } /* if */

  if ( chdir ( src_dir ) == ERROR ) 
    uquit ( ERROR, FALSE, "\tcould not change directory to %s.\n", src_dir );

  ui_print ( VNORMAL, "cd'ing to sandbox source directory: %s.\n", src_dir );
}                                                          /* cd to src path */



BOOLEAN isdir ( path )

	/* This function checks to see if the argument is a
	   directory.  It returns the results. */

    char      * path;                                       /* path to check */

{
    struct 	stat statb;

  if (( stat ( path, &statb ) == ERROR ) ||
      (( statb.st_mode & S_IFMT ) != S_IFDIR ))
    return ( ERROR );

  return ( OK );
}                                                                   /* isdir */




undo_set ( sb, sbbase, rc_file, setname )

	/* This procedure undoes a set after checking to see if
	   it is safe to do so.  It removes the set dir found
	   in .BCSpath- if there are no files in it, and then
	   removes the .BCS*setname files. */

    char      * sb,                                          /* sandbox name */
              * sbbase,                             /* holds path to sandbox */
              * rc_file,                                     /* user rc file */
              * setname;                               /* holds full setname */

{
    char        file [ NAME_LEN ],                      /* file to check for */
                reply [ NAME_LEN ],                           /* misc string */
              * setdir = NULL;                               /* place holder */
    BOOLEAN     goahead = FALSE;                             /* misc boolean */

  if ( current_set ( &setname, &setdir, &sb, &rc_file ) == ERROR )
    uquit ( ERROR, FALSE, "\tno set to undo.\n" );

  concat ( file, NAME_LEN, BCS_SET, setname, NULL );

  if ( access ( file, F_OK ) == OK )
    uquit ( ERROR, FALSE, "\tset %s has checked out files; cannot delete it.\n",
			setname );

  if ( ui_is_auto ())
    goahead = TRUE;

  else {
    ui_print ( VALWAYS, "Remove set %s? [y|<n>] ", setname );
    gets ( reply );

    if ( streq ( reply, YES ))
      goahead = TRUE;
  } /* else */

  if ( goahead ) {
    if ( ui_is_info ())
      ui_print ( VALWAYS, "Would remove set, %s, from sandbox: %s.\n",
			   setname, sb );

    else {
      remove_setname ( sb, sbbase, setname );
      ui_print ( VNORMAL, "NOTE:  you may still be in a sub shell.\n" );
    } /* else */
  } /* if */
}                                                                /* undo set */




remove_setname ( sb, sbbase, setname )

	/* This function removes the name of the set from the
	   sets rcfile. */

    char      * sb,                                          /* sandbox name */
              * sbbase,                             /* holds path to sandbox */
              * setname;                               /* holds full setname */

{
    FILE      * ptr_file,                                  /* ptr to rc file */
              * ptr_new;                               /* ptr to new rc file */
    char        line [ STRING_LEN ],                    /* line from rc file */
                line_copy [ STRING_LEN ],               /* line from rc file */
                newfile [ PATH_LEN ],                /* holds file to create */
                file [ PATH_LEN ],           /* holds set file path and name */
              * line_ptr,                          /* allows parsing of line */
              * token;                    /* holds each piece of parsed line */

  ui_print ( VNORMAL, "Removing set, %s, from sandbox: %s.\n", setname, sb );
  concat ( file, PATH_LEN, sbbase, "/", sb, "/", SET_RC, NULL );
  concat ( newfile, PATH_LEN, file, ".new", NULL );

  if (( ptr_file = fopen ( file, READ )) == NULL )
    uquit ( ERROR, FALSE, "\tcannot open set rcfile %s.\n", file );

  if (( ptr_new = fopen ( newfile, WRITE )) == NULL )
    uquit ( ERROR, FALSE, "\tcannot open tmp rcfile %s.\n", file);

  while (( fgets ( line, STRING_LEN, ptr_file )) != NULL ) {
    strcpy ( line_copy, line );
    line_ptr = line_copy;
    token = nxtarg ( &line_ptr, WHITESPACE );

    if ( streq ( token, SET_KEY )) {                   /* set field to check */
      token = nxtarg ( &line_ptr, WHITESPACE );

      if ( ! streq ( token, setname ))          /* no match so keep this set */
        fputs ( line, ptr_new );
    } /* if */

    else if ( *token != NUL )                    /* don't put in empty lines */
      fputs ( line, ptr_new );
  } /* while */

  fclose ( ptr_file );
  fclose ( ptr_new );
  rename ( newfile, file );
}                                                          /* remove setname */



write_set ( sb, sbbase, rc_file, setname, setdir, rc_contents )

	/* This procedure checks to see if the set is in the sets
	   file.  If it is, it handles setdir.  If it isn't, it
	   puts it in and calls a bco command to get the lock
	   file. */

    char      * sb,                                          /* sandbox name */
              * sbbase,                             /* holds path to sandbox */
              * rc_file,                                     /* user rc file */
              * setname,                               /* holds full setname */
             ** setdir;                             /* name of set directory */
    struct      rcfile  rc_contents;

{
    FILE      * ptr_file;                                  /* ptr to rc file */
    char        file [ PATH_LEN ],           /* holds set file path and name */
	      * tsetdir = NULL,				/* temporary set dir */
              * result;                 /* points to results of get_rc_value */

  if ( current_set ( &setname, &tsetdir, &sb, &rc_file ) == ERROR ) {
    check_setdir ( setdir );
    concat ( file, PATH_LEN, sbbase, "/", sb, "/", SET_RC, NULL );

    if ( ui_is_info ())
      ui_print ( VALWAYS, "would create new set: %s, setdir: %s\n",
			   setname, *setdir );

    else  {
      ui_print ( VNORMAL, "creating new set: %s, setdir: %s\n",
			   setname, *setdir );

      if (( ptr_file = fopen ( file, APPEND )) == NULL )
	uquit ( ERROR, FALSE, "\tcannot write to set rcfile %s.\n", file );

      fprintf ( ptr_file, "%s %s %s\n", SET_KEY, setname, *setdir );
      fclose ( ptr_file );
    } /* else */

    if ( access ( BCSLOCK, F_OK ) != OK ) {
/*--------------------------- old code -------------------------
      if ( get_rc_value ( SRC_CONTROL, &result, &rc_contents, TRUE ) == OK )
------------------------end of old code ----------------------*/

/*----------------------- my code ----------------------------------*/
      if ( get_rc_value ( SRC_CONTROL, &result, &rc_contents, FALSE ) == OK )
/*--------------------- end of my code ----------------------------------*/
	run_bco ( sb, rc_file, setname );   /* if source control, init it. */
    } /* if */
  } /* if */

  else {
    if ( ui_is_set ( SETDIR_OP ))
      check_setdir ( setdir );
    else
      *setdir = tsetdir;

    ui_print ( VNORMAL, "moving to existing set: %s, setdir: %s\n", setname,
	       		 *setdir );
  } /* else */
}                                                               /* write set */




check_setdir ( setdir )

	/* This procedure makes sure the first two characters in
	   the set directory name are "./".  This is so everything
	   is relative to the source directory. */

    char      ** setdir;                            /* name of set directory */

{
    char        tmp_setdir [ PATH_LEN ],           /* holds setdir with file */
                tmpline [ PATH_LEN ],          /* holds setdir before prefix */
              * prefix;                            /* holds prefix of setdir */
    BOOLEAN     fine = FALSE;                                /* misc boolean */

  if ( ! streq ( *setdir, "." )) {
    if ( **setdir == SLASH )
      prefix = ".";
    else if (( **setdir != '.' ) ||
	     ( **setdir + 1 != '/' ))
      prefix = "./";
    else
      fine = TRUE;

    if ( ! fine ) {
      concat ( tmpline, PATH_LEN, prefix, *setdir, NULL );
      *setdir = salloc ( tmpline );
    } /* if */

    concat ( tmp_setdir, PATH_LEN, *setdir, "/.", NULL );

    if ( isdir ( *setdir ) == ERROR ) {
      if ( ui_is_info ())
	ui_print ( VALWAYS, "would create directory: %s.\n", *setdir );
      else if ( makepath ( tmp_setdir, NULL, TRUE, TRUE ) == ERROR )
	uquit ( ERROR, FALSE, "\tcould not create directory %s.\n", *setdir );
    } /* if */
  } /* if */
}                                                            /* check setdir */



run_bco ( sb, rc_file, setname )

	/* This procedure runs the bco command for the new set
	   so a new lock will be created. */

    char      * sb,                                          /* sandbox name */
              * rc_file,                                     /* user rc file */
              * setname;                               /* holds full setname */

{
    char      * av [ BCOARGS ],                    /* holds args to bcs call */
              * env_ptr,                             /* points to editor env */
                sv_ed [ PATH_LEN ];                           /* misc string */
    int         ct = 0,                                      /* misc counter */
                status;                                      /* misc integer */

  env_ptr = getenv ( EDITOR );

  if ( env_ptr == NULL )
    *sv_ed = NUL;
  else
    strcpy ( sv_ed, env_ptr );

  setenv ( EDITOR, NO_EDIT, TRUE );
  ui_print ( VNORMAL, "creating lock file for new set.\n" );

  if ( ui_ver_level () == VDEBUG || ui_is_info ())
    av[ct++] = ECHO_OP;

  av[ct++] = BCO;
  av[ct++] = SB_OP;
  av[ct++] = sb;
  av[ct++] = RC_OP;
  av[ct++] = rc_file;
  av[ct++] = SET_OP;
  av[ct++] = setname;
  av[ct++] = ui_ver_switch ();
  av[ct] = NULL;
  ct = 0;

  if ( ui_ver_level () == VDEBUG || ui_is_info ()) {
    ( void ) runv ( ECHO, av );
    printf ( "\n" );
    ct++;
  } /* if */

  if ( ! ui_is_info ()) {
    if (( status = runvp ( BCO, &av[ct] )) != OK )
      ui_print ( VWARN, "failed to make lock file using bco. Status %d.\n",
			 status );
  } /* if */

  if ( *sv_ed == NUL )
    unsetenv ( EDITOR );
  else
    setenv ( EDITOR, sv_ed, TRUE );
}                                                                 /* run bco */




set_workon_var ()

	/* This procedure sets the workon environment variable
	   WORKON.  It tells how many levels of workon have been set. */

{
    char      * env_ptr,                         /* holds environment values */
                buf [ NAME_LEN ];                             /* misc string */
    int         count;                                       /* misc integer */

  env_ptr = getenv ( WORKON );

  if ( env_ptr == NULL ) {
    if ( setenv ( WORKON, "1", TRUE ) == ERROR )
      ui_print ( VWARN, "WORKON setenv failure.\n" );
  } /* if */

  else {
    count = atoi ( env_ptr );
    count++;
    sprintf ( buf, "%d", count );

    if ( setenv ( WORKON, buf, TRUE ) == ERROR )
      ui_print ( VWARN, "WORKON setenv failure.\n" );
  } /* else */
}                                                          /* set workon var */



create_sub_shell ( setdir )

	/* This procedure creates a new sub shell and puts the
	   user in it.  It first sets the environment, then
	   changes directory to the set directory.  Finally,
	   it execs a new shell. */

    char      * setdir;                             /* name of set directory */

{
    char      * ptr,                                     /* pointer for path */
              * shell_type;                              /* pointer for path */

  if ( ! streq ( setdir, "." )) {
    if ( ! ui_is_info ())
      if ( chdir ( setdir ) == ERROR )
	uquit ( ERROR, FALSE, "\tcould not change directory to %s for shell.\n",
			       setdir );

    ui_print ( VNORMAL, "cd'ing to set directory: %s.\n", setdir );
  } /* if */

  if (( shell_type = getenv ( "SHELL" )) == NULL )  /* get shell and exec it */
    ptr = shell_type = "csh";
  else if (( ptr = rindex ( shell_type, SLASH )) != NULL )
    ptr++;
  else
    ptr = shell_type;

  if ( ui_is_info ())
    ui_print ( VALWAYS, "would start new shell: %s.\n", shell_type );
  else {
    ui_print ( VNORMAL, "starting new shell: %s.\n", shell_type );
    execlp ( shell_type, ptr, 0 );
  } /* else */
}                                                        /* create sub shell */



print_usage ()

	/* This procedure prints the usages for workon. */

{
  printf ( "USAGE:\n" );
  printf ( "%s [-nosh] [-setdir <directory>] [sb_opts] setname\n", progname );
  printf ( "\t -nosh  : do not open a new shell\n" );
  printf ( "\t -setdir <directory> : directory the set cd's to\n" );
  printf ( "\t sb_opts:\n" );
  printf ( "\t   -sb <sandbox>, -rc <user rc file>, -sb_rc <sb rc file>\n" );
  printf ( "\t setname: the name of the set to create or move to\n\n" );
  printf ( "%s -undo [sb_opts] setname\n", progname );
  printf ( "%s -list [sb_opts]\n", progname );
  printf ( "%s -usage | -version\n", progname );
}                                                             /* print usage */
