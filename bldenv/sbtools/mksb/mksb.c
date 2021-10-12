static char sccsid[] = "@(#)74  1.8  src/bldenv/sbtools/mksb/mksb.c, bldprocess, bos412, GOLDA411a 1/10/94 15:37:45";
/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: build_sb_structure1683
 *		check_colon_line
 *		cmdline_syntax
 *		create_default_set2076
 *		create_local_files2271
 *		create_machine_sub_dirs1767
 *		create_sandbox
 *		create_sb_rc_files1837
 *		create_tmp_file
 *		create_user_rc_file1906
 *		finish_fields
 *		init_rc_values
 *		legal_backing_tree
 *		legal_list_undo
 *		legal_machine_names
 *		legal_object_dirs
 *		legal_rc_file
 *		legal_sandbox
 *		legal_sandbox_dir
 *		legal_setname
 *		legal_source_dirs
 *		legal_tools_opt
 *		list_sandboxes
 *		load_base
 *		load_defaults
 *		load_sb
 *		main
 *		populate_sandbox2164
 *		print_out_rc_values1242
 *		print_usage
 *		read_in_rc_info
 *		remove_sb
 *		remove_sb_from_rc
 *		set_backing
 *		set_default_dir
 *		set_default_machine1490
 *		set_default_rc
 *		set_obj_population1565
 *		set_src_population1624
 *		set_tools_population1528
 *		tools_obj_run_through2217
 *		undo_sandbox
 *		unique_sb
 *		update_user_rc_file1958
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
 *
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
**  Description:
**	mksb.c creates a sandbox in accordance with the user's instructions.
**
**  Functions:
**    main (int, ** char) int
**      cmdline_syntax (** char, ** char, ** char, ** char, ** char) int
**        legal_sandbox (** char) int
**        legal_rc_file (** char) int
**          set_default_rc (** char) int
**        load_defaults (** char, ** char, ** char, ** char) int
**        legal_backing_tree (* char, * char) int
**        legal_sandbox_dir (* char) int
**        legal_setname (* char) int
**        legal_machine_names () int
**        legal_tools_opt () int
**        legal_object_dirs () int
**          check_colon_line (* char) int
**        legal_source_dirs () int
**          check_colon_line
**        legal_list_undo () int
**      undo_sandbox (* char, * char, * char) int
**        remove_sb (* char, * char, * char) int
**          remove_sb_from_rc (* char, * char) int
**            create_tmp_file (* char, * char) * struct
**      list_sandboxes (* char) int
**      create_sandbox (* char, * char, * char, * char, * char) int
**        init_rc_values (* struct) int
**        read_in_rc_info (* char, * struct) int
**          load_base (* struct, * char, * char) int
**          load_sb (* struct, * char) int
**          print_out_rc_values (* struct) int
**        finish_fields (* char, * char, * char, * char, * struct) int
**          unique_sb (* char, * struct) int
**          set_default_dir (* struct, * char) int
**            legal_sandbox_dir
**          set_backing (* char, * char, * char) int
**            legal_backing_tree
**          set_default_machine (* struct) int
**            legal_machine_names
**          set_tools_population (* struct) int
**          set_obj_population (* struct) int
**            check_colon_line
**          set_src_population (* struct) int
**            check_colon_line
**        build_sb_structure (* char, * char, * char) int
**          remove_sb
**          create_machine_sub_dirs (* char, * char, * char, * char) int
**            remove_sb
**          create_sb_rc_files (* char, * char, * char, * char, * char, * char)
**			        int
**            remove_sb
**        create_user_rc_file (* struct, * char, * char) int
**          print_out_rc_values
**        update_user_rc_file (* char, * char, * char) int
**          create_tmp_file
**        create_default_set (* char, * char, * char) int
**        populate_sandbox (* struct, * char, * char, * char) int
**          tools_obj_run_through (* char, * char, * char, * char, * char,
**				   * struct) int
**            create_local_files (* char, * char, * char, * char, int) int
**          create_local_files
**    print_usage () int
**    print_revision () int
**
**  Changes Made:
**    01/13/93 - Frank Li
**               Inserted code to create a user rc file if one doesn't exist
**               before build_sb_structure in create_sandbox.
**
**               Commented out code to create a user rc file if one doesn't
**               exist after build_sb_structure in create_sandbox.
**
**               Inserted code to update user rc file before create_default_set
**               in create_sandbox.
**
**               Inserted code to uquit when unable to create sandbox 
**               directories after remove_sb in build_sb_structure.
**
**               Inserted code to ui_print when unable to create sandbox
**               directories after uquit in build_sb_structure.
**
**               Inserted declaration for ipath in create_machine_sub_dirs.
**
**               Inserted code to build inst.images directory name after concat
**               of SHIP_DIR in create_machine_sub_dirs.
**
**               Commented out unnecessary fprintf statements in
**               create_user_rc_file.
**
**               Inserted code to assign value to sb_rcfile in
**               populate_sandbox.
**
**               Inserted code to assign value to sb_rcfile in
**               tools_obj_run_through.
**
**               Commented out unnecessary concat statement after nxtarg in
**               tools_obj_run_through.
**
**    04/10/93 - Michael Winestock
**               Commented out code to execute workon in create_default_set.
**
**               Added code to try and create the inst.images directory after
**               the mkdir of spath in create_machine_sub_dirs.
**
**    04/12/93 - Michael Winestock
**               Put back commented out fprintf statements in 
**               create_user_rc_file.  Reversed changes previously made by
**               Frank Li on 01/13/93.
**
**    05/04/93 - Alan Christensen
**               Removed hard coded pathname "/project/projects/build_list"
**               and added the environment variable ODE_BUILD_LIST to locate
**               the file.  If the file does not exist, it will require a
**               complete path to the backing tree.
**
**    05/13/93 - Michael Winestock
**               Fix file using /afs/austin/local/bin/prologs -v -C bldprocess
**               -O 27,71 -D 1990,1992 -P3 -T c_program -f mksb.c.
**
**    05/13/93 - Michael Winestock
**               Removed code to create export, ship, inst.images, host and
**               ode_tools directory.
**
 */

static char * rcsid =
 "$RCSfile: mksb.c,v $ $Revision: 1.11.2.2 $ $Date: 92/03/25 22:46:48 $";

extern unsigned int errno ;

#include "mksb.h"
#include <stdlib.h>

#ifdef NO_DIRENT
#  define dirent direct
#endif

    char        * progname = "mksb";                   /* the program's name */
    BOOLEAN 	  new_rc = FALSE;  		   /* is there a new rc file */

    UIINIT init [] = {                      /* initialize the user interface */
      { BACK_OP,  1, OVERWRITE,	 1, 1,	ARGS_OP },
      { DIR_OP,   1, OVERWRITE,  1, 1,  "/*" },
      { SET_OP,	  1, OVERWRITE,	 1, 1,	ARGS_OP },
      { M_OP,	  1, OVERWRITE,	 1, 1,	ARGS_OP },
      { DEF_OP,	  1, OVERWRITE,	 0, 0,	"" },
      { TOOLS_OP, 1, OVERWRITE,	 1, 1,	"b c l" },
      { OBJ_OP,   1, OVERWRITE,  2, 2,  "b c l /*" },
      { SRC_OP,   1, OVERWRITE,  2, 2,  "b c l /*" },
      { LIST_OP,  1, OVERWRITE,	 0, 0,	"" },
      { UNDO_OP,  1, OVERWRITE,	 0, 0,	"" },
      { ARGS_OP,  1, OVERWRITE,	 0, 0,	"" } 
    };


main ( argc, argv )

  	/* This function checks the command line arguments and makes
	   sure they are syntactically correct.  This is done using
	   the library function ui_init.  If this is correct,
	   the dependencies are checked.  Errors lead to usage messages.
	   If all is okay, the primary procedures are called. */

    int         argc;                /* the number of command line arugments */
    char     ** argv;                          /* strings with each argument */

{
    char      	* def_build,               /* default build for this project */
              	* proj_dir,           /* location of builds for this project */
	      	* sb,                                        /* sandbox name */
		* rc_file,                                   /* user rc file */
              	* sb_rc;                                /* sandbox's rc file */

  ui_init ( argc, argv, MAX_ARGS, init );
  cmdline_syntax ( &proj_dir, &def_build, &sb, &rc_file, &sb_rc );

  if ( ui_is_set ( UNDO_OP ))
    undo_sandbox ( sb, rc_file, sb_rc );
  else if ( ui_is_set ( LIST_OP ))
    list_sandboxes ( rc_file );
  else
    create_sandbox ( proj_dir, def_build, sb, rc_file, sb_rc );

  return ( OK );
}                                                                    /* main */



cmdline_syntax ( proj_dir, def_build, sb, rc_file, sb_rc )

	/* This procedure checks for relationships between the
	   command line arguments.  It assumes the syntax is
	   already correct. */

    char       ** proj_dir,           /* location of builds for this project */
               ** def_build,               /* default build for this project */
	       ** sb,                                        /* sandbox name */
	       ** rc_file,                                   /* user rc file */
               ** sb_rc;                                /* sandbox's rc file */

{
  if ( ! ui_is_set ( LIST_OP ))
    legal_sandbox ( sb );

  legal_rc_file ( rc_file );

  if ( ui_is_set ( RC_OP ))
    load_defaults ( proj_dir, def_build, sb_rc, rc_file );

  if ( ui_is_set ( BACK_OP ))
    legal_backing_tree ( *proj_dir, *sb_rc ); 
  
  if ( ui_is_set ( DIR_OP )) 
    if ( ! legal_sandbox_dir ( *sb ))
      ui_unset ( DIR_OP );
  
  if ( ui_is_set ( SET_OP ))
    legal_setname ( *sb );
  
  if ( ui_is_set ( M_OP )) 
    if ( ! legal_machine_names ())
      ui_unset (  M_OP );
  
  if ( ui_is_set ( TOOLS_OP ))
    legal_tools_opt ();

  if ( ui_is_set ( OBJ_OP ))
    legal_object_dirs () ;

  if ( ui_is_set ( SRC_OP ))
    legal_source_dirs () ;

  legal_list_undo ();
}                                                          /* cmdline syntax */



legal_sandbox ( sb )

	/* This procedure checks on the sandbox name to make sure
	   it is there when it should be and that it is legal. */

    char       ** sb;                                             /* sandbox */

{
    int           ct;                                        /* misc counter */

  if (( *sb = ui_entry_value ( ARGS_OP, 1 )) == NULL )
    uquit ( ERROR, TRUE, "name of sandbox required.\n" );

  if ( ui_is_set ( SET_OP )) {
    for ( ct = 0; ct < strlen ( *sb ); ct++ ) {

      if ( **sb + ct == SLASH )
        uquit ( ERROR, TRUE, "sandbox name, %s, has '%c' character in it.\n",
		*sb, SLASH );
    } /* for */
  } /* if */

  else if ( ! ui_is_set ( UNDO_OP )) {
    ui_load ( SET_OP, 1, *sb );
    ui_print ( VDETAIL, "Name of default set will match sandbox name: %s.\n",
	                 *sb );
  } /* else if */
  
  ui_print ( VDETAIL, "Name of sandbox will be: %s.\n", *sb );
}                                                           /* legal sandbox */



legal_rc_file ( rc )

	/* This procedure checks to see if the named rc file exists
	   and is writable.  If it does not exist, it checks to 
	   see if it can create it.  */

    char       ** rc;  				       /* holds rc file name */

{
    FILE	* fptr;  /* misc file pointer */
    char          full_rc [ PATH_LEN ],                       /* misc string */
                  cwd [ MAXPATHLEN ];     /* holds current working directory */

  if (( *rc = ui_arg_value ( RC_OP, 1, 1 )) != NULL ) {
    if ( **rc != SLASH ) {
      if ( getwd ( cwd ) == NULL )
        uquit ( ERROR, FALSE, "call to getwd failed.\n" );

      concat ( full_rc, PATH_LEN, cwd, "/", *rc, NULL );
      ui_load ( RC_OP, 1, full_rc );
    } /* if */
  } /* if */
  
  else 
    set_default_rc ( rc );

  if ( access ( *rc, F_OK ) == ERROR ) {
    if (( fptr = fopen ( *rc, WRITE )) == NULL )
      uquit ( ERROR, FALSE, "Cannot create user's rc file: %s.\n", *rc );

    fclose ( fptr );
    unlink ( *rc );
    new_rc = TRUE;
    ui_print ( VNORMAL, "User rc file, %s, does not exist; will create it.\n",
                         *rc );
  } /* if */
  
  else if ( access ( *rc, W_OK ) == ERROR )
    uquit ( ERROR, FALSE, "Cannot write in user's rc file: %s.\n", *rc );
   
  else
    ui_print ( VDETAIL, "user's rc file is: %s.\n", *rc );
}                                                           /* legal rc file */



set_default_rc ( rc )

 	/* This procedure fills in the name of the default rc file. */

    char      ** rc;  					     /* user rc file */

{
  if ( ! get_default_usr_rcfile ( rc, FALSE ))
    new_rc = TRUE;

  ui_load ( RC_OP, 1, *rc );
}                                                          /* set default rc */



load_defaults ( proj_dir, def_build, sb_rc, rcfile ) 

	/* This procedure finds the project directory and default
	   build for the project and loads that information. */

    char       ** proj_dir,           /* location of builds for this project */
               ** def_build,               /* default build for this project */
               ** sb_rc,                                /* sandbox's rc file */
               ** rcfile;                                  /* user's rc file */

{
    char        * sb = NULL,                        /* holds default sandbox */
                * basedir = NULL;               /* base directory to sandbox */

  *sb_rc = ui_arg_value ( SB_RC_OP, 1, 1 );

  if ( *rcfile != NULL && access ( *rcfile, R_OK ) != ERROR ) {
    if (( current_sb ( &sb, &basedir, sb_rc, rcfile ) == ERROR ) ||
	( default_build ( proj_dir, def_build, NULL, *sb_rc ) == ERROR )) {
      *proj_dir = NULL;
      *def_build = NULL;
      ui_print ( VWARN, "could not get project directory or default build.\n" );
    } /* if */

    ui_print ( VDEBUG, "project dir: %s;\ndefault build: %s;\nsbrc: %s.\n",
			*proj_dir, *def_build, *sb_rc );
  } /* if */

  else {
    *proj_dir = NULL;
    *def_build = NULL;
  } /* else */
}                                                           /* load defaults */



legal_backing_tree ( build_dir, sbrc )

	/* This function checks to see if the backing tree is a
 	   legal entity.  If it is not, it unsets the flag so the
 	   program can prompt for this information later. */

    char      * build_dir,                      /* project's build directory */
              * sbrc;                                   /* sandbox's rc file */

{
    DIR       * dir_ptr;                            /* points to a directory */
    struct      rcfile    contents;             /* holds contents of rc file */
    char        local_file [ PATH_LEN ],           /* path to local template */
                shared_file [ PATH_LEN ],         /* path to shared template */
                tmp_file [ PATH_LEN ],                        /* misc string */
              * newbase,                           /* pts to build list base */
              * back_build;
    BOOLEAN     failed = FALSE;                              /* misc boolean */
    BOOLEAN     verbose;                                   /* verbose switch */

  verbose = ui_ver_level () >= VDETAIL;
  back_build = ui_arg_value ( BACK_OP, 1, 1 );

  if ( *back_build != SLASH ) {
    if ( build_dir != NULL )
      concat ( tmp_file, PATH_LEN, build_dir, "/", back_build, NULL);

    if ( build_dir != NULL &&
        (( dir_ptr = opendir ( tmp_file )) != NULL )) {
      ui_load ( BACK_OP, 1, tmp_file );
      closedir ( dir_ptr );
    } /* if */

    else {
      ui_print ( VDEBUG, "not in default build\n" );
      bzero ( &contents, sizeof ( contents ));

      if ( sbrc != NULL && parse_rc_file ( sbrc, &contents ) == OK )
	newbase = build_base_dir ( back_build, &contents, verbose, FALSE );
      else			  
	newbase = build_base_dir ( back_build, &contents, verbose, TRUE );

      if ( newbase != NULL ) {
	concat ( tmp_file, PATH_LEN, newbase, "/", back_build, NULL );

	if (( dir_ptr = opendir ( tmp_file )) != NULL ) {
	  ui_load ( BACK_OP, 1, tmp_file);
	  closedir ( dir_ptr );
	} /* if */

	else {
          ui_print ( VDEBUG, "not in build list build\n" );
	  failed = TRUE;
	} /* else */
      } /* if */

      else {
        ui_print ( VDEBUG, "parse rc failed\n" );
	failed = TRUE;
      } /* else */
    } /* else */
  } /* if */

  back_build = ui_arg_value ( BACK_OP, 1, 1 );

  if ( ! failed ) {  
    concat ( local_file, PATH_LEN, back_build, "/", LOCAL_T_RC, NULL );
    concat ( shared_file, PATH_LEN, back_build, "/", SHARED_T_RC, NULL );

    if (( access ( local_file, R_OK ) == ERROR ) ||
	( access ( shared_file, R_OK ) == ERROR )) {
      failed = TRUE;
      ui_print ( VDEBUG, "cannot access: %s, %s.\n", LOCAL_T_RC, SHARED_T_RC );
    } /* if */
  } /* if */

  if ( failed ) {
    ui_print ( VWARN, "%s is not a legal, accessible backing tree.\n", back_build );
    ui_print ( VCONT, "Will prompt for this information.\n" );
    ui_unset ( BACK_OP );
  } /* if */

  else
    ui_print ( VDETAIL, "Backing tree is build or sandbox: %s.\n", back_build );
}                                                      /* legal backing tree */



BOOLEAN	legal_sandbox_dir ( sandbox )

	/* This function checks to see if the directory entered
	   to create the sandbox in exists.  If it does, then it
	   checks to see if it can be written into. It returns
	   FALSE if it cannot access the directory for any reason. */

    char        * sandbox;                      /* name of sandbox to create */

{
    DIR         * dir_ptr;                          /* points to a directory */
    char          sb_string [ PATH_LEN ];           /* holds path to sandbox */
    char	* dir;

  dir = ui_arg_value ( DIR_OP, 1, 1 );

  if ( dir == NULL || ui_is_set ( UNDO_OP ) || ui_is_set ( LIST_OP ))
     return ( FALSE );	          /* cannot use this -dir with these options */

  if (( dir_ptr = opendir ( dir )) == NULL ) {
    ui_print ( VWARN, "the directory, %s,", dir );
    ui_print ( VCONT, "for the sandbox cannot be opened.\n" );
    return ( FALSE );
  } /* if */

  closedir ( dir_ptr );

  if ( access ( dir, W_OK ) == ERROR ) {
    ui_print ( VWARN, "cannot write in the directory, %s, for the sandbox.\n",
	      	       dir );
    return ( FALSE );
  } /* if */

  concat ( sb_string, PATH_LEN, dir, "/", sandbox, NULL );

  if ( access ( sb_string, F_OK ) == OK ) {               /* sb cannot exist */
    if ( ! ui_is_set ( LIST_OP )) {
      ui_print ( VWARN, "Found existing file at %s\n", sb_string );
      ui_print ( VCONT, "where sandbox was to be.\n" );
    } /* if */

    return ( FALSE );
  } /* else if */

  ui_print ( VDETAIL,"Sandbox directory and name will be: %s.\n", sb_string );
  return ( TRUE );
}                                                       /* legal sandbox dir */



legal_setname ( sandbox )

	/* This procedure checks to see if the setname has any
	   illegal characters in it. */

    char        * sandbox;                      /* name of sandbox to create */

{
    char	* set;  			   /* holds user entered set */
    int           ct;                                        /* misc counter */

  set = ui_arg_value ( SET_OP, 1, 1 );

  for ( ct = 0; ct < strlen ( set ); ct++ ) {
    if (( set[ct] == PERIOD ) ||
        ( set[ct] == SLASH ) ||
        ( set[ct] == DASH )) {
      
      if ( streq ( set, sandbox ))
        uquit ( ERROR, FALSE, "\tsetname, %s, %s %s: '%c' or '%c' or '%c'.\n",
		set, "which was taken from the sandbox name,\n",
		"\thas one of the following characters in it",
		PERIOD, DASH, SLASH );

      else 
        uquit ( ERROR, FALSE,  "\tsetname, %s, %s\t'%c' or '%c' or '%c'.\n",
		set, "has one of the following characters in it:\n",
		PERIOD, DASH, SLASH );
    } /* if */
  } /* for */

  ui_print ( VDETAIL, "Name of default set will be: %s.\n", set );
}                                                           /* legal setname */



BOOLEAN	legal_machine_names ()

	/* This function checks to see if it can parse the list
	   of machine names.  If the arguments is not legal, it
	   returns FALSE. */


{
    char      * machine_args;             /* the information on the machines */
    int         ct;                                          /* misc counter */

  machine_args = ui_arg_value ( M_OP, 1, 1 );

  for ( ct = 0; ct < strlen ( machine_args ); ct++ ) {
    if ( machine_args[ct] == SLASH ) {
      ui_print ( VWARN, "machine string, %s, has an imbedded slash.\n",
		         machine_args );
      return (  FALSE );
    } /* if */
  } /* for */

  ui_print ( VDETAIL, "Name(s) of machines will be: %s.\n", machine_args );
  return ( TRUE );
}                                                     /* legal machine names */



legal_tools_opt ()

	/* This function checks the argument given to the tools
	   populating option and verifies that it is legal. */

{
    char      * tool_arg;                /* the information on tools backing */

  tool_arg = ui_arg_value ( TOOLS_OP, 1, 1 );

  if ( *tool_arg != COPY_CH &&
       *tool_arg != LINK_CH  &&
       *tool_arg != BACK_CH ) 
    uquit ( ERROR, TRUE, "\targument to %s option must be %c, %c, or %c.\n",
	    TOOLS_OP, BACK_CH,COPY_CH, LINK_CH );

  ui_print ( VDETAIL, "Tools populating method: %c.\n", *tool_arg );
}                                                         /* legal tools opt */



legal_object_dirs ()

	/* This function checks the arguments given to the object
	   populating option and verifies that they are legal. */


{
    char      	* method,   		     /* method for populating object */
		* directories;      	      /* the directories to populate */

  method = ui_arg_value ( OBJ_OP, 1, 1 );
  directories = ui_arg_value ( OBJ_OP, 1, 2 );

  if ( *method != COPY_CH &&
       *method != LINK_CH  &&
       *method != BACK_CH ) {
    ui_print ( VWARN, "argument to %s option must be %c, %c, or %c.\n",
	               OBJ_OP, BACK_CH, COPY_CH, LINK_CH );
    ui_print ( VCONT, "will prompt for correct information.\n" );
    ui_unset ( OBJ_OP);
  } /* if */
  
  if ( ! check_colon_line ( directories )) {
    ui_unset ( OBJ_OP );
    ui_print ( VCONT, "will prompt for correct information.\n" );
  } /* if */

  if ( ui_is_set ( OBJ_OP )) {
    ui_print ( VDETAIL, "Object populating method: %c.\n", *method );
    ui_print ( VCONT, "directories to populate: %s.\n", directories );
  } /* if */
}                                                       /* legal object dirs */



BOOLEAN check_colon_line ( line )

	/* This function checks to see if each of the directories
	   in the colon separated lines starts with a SLASH.  It
	   returns TRUE if they all start with slash. */

    char      * line;                             /* line to parse and check */

{
    char        line_copy [ PATH_LEN ],    /* will hold first string of line */
              * line_ptr,                     /* needs to move along ns_line */
              * list;                         /* needs to move along ns_line */

  strcpy ( line_copy, line );
  line_ptr = line_copy;                         /* copy can't move, list can */

  do {
    list = nxtarg ( &line_ptr, ":" );

    if ( *list != SLASH ) {
      ui_print ( VWARN, "Each part of the colon separated list: %s,\n", line );
      ui_print ( VCONT, "must start with a '/'.\n" );
      return ( FALSE );
    } /* if */
  } while ( *line_ptr != NUL );

  return ( TRUE );
}                                                        /* check colon line */



legal_source_dirs ()

	/* This function checks the arguments given to the source
	   populating option and verifies that they are legal. */


{
    char      	* method,  	        /* the method for populating sources */
		* dirs;                       /* the directories to populate */

  method = ui_arg_value ( SRC_OP, 1, 1 );
  dirs = ui_arg_value ( SRC_OP, 1, 2 );

  if ( *method != COPY_CH &&
       *method != LINK_CH  &&
       *method != BACK_CH ) {
    ui_print ( VWARN, "argument to %s option must be %c, %c, or %c.\n",
	      	       SRC_OP, BACK_CH, COPY_CH, LINK_CH );
    ui_print ( VCONT, "will prompt for correct information.\n" );
    ui_unset ( SRC_OP );
  } /* if */
  
  if ( ! check_colon_line ( dirs )) {
    ui_print ( VCONT, "will prompt for correct information.\n" );
    ui_unset ( SRC_OP );
  } /* if */

  if ( ui_is_set ( SRC_OP )) {
    ui_print ( VDETAIL, "Object populating method: %c.\n", *method );
    ui_print ( VCONT, "directories to populate: %s.\n", dirs );
  } /* if */
}                                                       /* legal source dirs */



legal_list_undo ()

	/* This procedure checks to see if only legal options
	   were used with the list option.  */

{
  if ( ui_is_set ( UNDO_OP ) && ui_is_set ( LIST_OP ))
    uquit ( ERROR, TRUE, 
               "Both options %s and %se were set; this is not allowed.\n",
               LIST_OP, UNDO_OP );
}                                                    /* legal list undo */



undo_sandbox ( sb, rc_file, sbrc )

	/* This procedure undoes a sandbox if the sandbox has not
	   had files checked out into it.  It does this by opening
	   the directory and seeing if any BCS checkouts have
	   been done. */

    char        * sb,                         /* sandbox name */
                * rc_file,                    /* user rc file */
                * sbrc;                    /* sandbox rc file */

{
    DIR         * dir_ptr;                          /* points to a directory */
    struct	dirent * ds;               /* points to struct with dir list */
    char          ssb_path [ PATH_LEN ],          /* path and name to sb/src */
                * dir_args = NULL;               /* to hold sandbox base dir */
    BOOLEAN       safe = TRUE;                               /* misc boolean */

  if ( new_rc )
    uquit ( ERROR, FALSE, "\t%s user's rc file does not exist.\n",
                      	   ui_arg_value ( RC_OP, 1, 1 ));

  if ( current_sb ( &sb, &dir_args, &sbrc, &rc_file ) == ERROR )
    uquit ( ERROR, FALSE,  
    	    "sandbox, %s, not correctly setup\n\tin user's rc file %s\n",
	     sb, rc_file );

				   /* Determine if BCSpath has been created. */
  concat ( ssb_path, PATH_LEN, dir_args, "/", sb, "/", SRC_DIR, NULL );

  if (( dir_ptr = opendir ( ssb_path )) == NULL )
    uquit ( ERROR, FALSE, "Did not find directory at %s to remove.\n",
			   ssb_path );

  for ( ds = readdir ( dir_ptr ); ds != NULL; ds = readdir ( dir_ptr )) {
    if (( strncmp ( ds->d_name, BCS_SET, BCS_SET_L )) == OK ) {
      ui_print ( VWARN,
	"Found BCS file, %s: indicating files have been checked out\n",
	 ds->d_name );
      ui_print ( VCONT, "therefore not removing sandbox %s.\n", sb );
      safe = FALSE;
    } /* if */
  } /* for */

  closedir ( dir_ptr );

  if ( safe )
    remove_sb ( sb, rc_file, dir_args );
}                                                            /* undo sandbox */



remove_sb ( sb, rcfile, sbdir )

	/* This procedure removes the sandbox directory. */

    char        * sb,                         /* sandbox name */
                * rcfile,                    /* user rc file */
		* sbdir;	  /* sandbox base dir */

{
    char        path [ PATH_LEN ],                            /* misc string */
                reply [ NAME_LEN ];                           /* misc string */
    BOOLEAN     goahead = FALSE;                             /* misc boolean */

  if ( chdir ( "/" ) == ERROR )
    ui_print ( VFATAL, "could not change directories to '/'.\n" );

  concat ( path, PATH_LEN, sbdir, "/", sb, NULL );
 
  if ( ui_is_auto ())
    goahead = TRUE;
    
  else {
    ui_print ( VALWAYS, "Remove sandbox %s? [y|<n>] ", path );
    gets ( reply );

    if ( streq ( reply, YES ))
      goahead = TRUE;
  } /* else */

  if ( goahead ) {
    ui_print ( VDETAIL, "Removing sandbox %s.\n", path );

    switch ( fork ()) {
      case ERROR: ui_print ( VFATAL,
		  "fork failed, could not remove sandbox: %s.\n", path );
		  break;

      case CHILD: if ( ui_is_info ())
		    ui_print ( VALWAYS, "Would have done: 'rm -rf %s'\n", path);

		  else {
		    execlp ( RM, RM, "-r", "-f", path, NULL );
    		    uquit ( ERROR, FALSE, "\treturned from execlp of: %s.\n",
				     		RM );
		  }  /* else */

		  break;

      default :   wait (( int * ) 0 );
	          break;
    } /* switch */

    if ( remove_sb_from_rc ( sb, rcfile ) == FALSE )
      ui_print ( VWARN, "did not remove sandbox %s from rc file %s.\n",
		 	 sb, rcfile );
  } /* if */
}                                                               /* remove sb */



BOOLEAN	remove_sb_from_rc ( sbname, rcfile )

	/* This procedure removes the references to the sandbox
	   which has just been deleted from the rc file.  If for
	   any reason the file cannot be edited, it returns FALSE. */

    char      * sbname,                                   /* name of sandbox */
              * rcfile;                                   /* name of rc file */

{
    FILE      * ptr_file,                                  /* ptr to rc file */
              * tmp_file;                              /* ptr to tmp rc file */
    char        line [ PATH_LEN ],                            /* misc string */
                line_copy [ PATH_LEN ],                       /* misc string */
                tmp_rc [ PATH_LEN ];                          /* misc string */
    char      * line_ptr,
              * key,
              * target;

  if (( ptr_file = fopen ( rcfile, READ )) == NULL ) {
    ui_print ( VFATAL, "cannot read user's rc file: %s.\n", rcfile );
    return ( FALSE );
  } /* if */

  if ( ! ui_is_info ())
    if (( tmp_file = create_tmp_file ( rcfile, tmp_rc )) == NULL )
      return ( FALSE );

  while (( line_ptr = fgets ( line, PATH_LEN, ptr_file )) != NULL ) {
    strcpy ( line_copy, line );
    line_ptr = line_copy;
    key = nxtarg ( &line_ptr, WHITESPACE );
    target = nxtarg ( &line_ptr, WHITESPACE );

    if ( ! streq ( target, sbname )) {
      if ( ! ui_is_info ())
        fputs ( line, tmp_file );
      continue;
    } /* if */

    if ( streq ( key, DEFAULT )) {
      ui_print ( VWARN, "rc file lists the default sandbox, %s,\n", sbname );
      ui_print ( VCONT, "as the same sandbox that was just removed.\n" );

      if ( ! ui_is_info ())
        fputs ( line, tmp_file );
    } /* if */

    else if (( ! streq ( key, BASE )) && ( ! streq ( key, SB )))
      if ( ! ui_is_info ())
        fputs ( line, tmp_file );
  } /* while */

  fclose ( ptr_file );

  if ( ! ui_is_info ()) {
    fclose ( tmp_file );

    if ( rename ( tmp_rc, rcfile ) == ERROR )
      return ( FALSE );
  }  /* if */

  return ( TRUE );
}                                                       /* remove sb from rc */



FILE  *	create_tmp_file ( rc_path, tmp_rc )

	/* This function creates a tmp file with the same path as
	   the rc file.  It opens the file for writing and passes
	   back the pointer. */

    char      * rc_path,                     /* string with rc path and name */
              * tmp_rc;                                       /* misc string */

{
    FILE      * tmp_ptr;                               /* ptr to new rc file */
    char        line [ PATH_LEN ],                            /* misc string */
              * line_ptr,                                     /* misc string */
              * path,                                         /* misc string */
              * ptr_slash;                        /* ptr to slash in rc path */

  strcpy ( line, rc_path );
  ptr_slash = strrchr ( line, SLASH );

  if ( ptr_slash == NULL )
    strcpy ( tmp_rc, TMP_NAME );

  else {
    *ptr_slash = NUL;
    line_ptr = line;
    path = nxtarg ( &line_ptr, WHITESPACE );
    concat ( tmp_rc, PATH_LEN, path, TMP_NAME, NULL);
  } /* else */

  if (( tmp_ptr = fopen ( tmp_rc, WRITE )) == NULL ) {
    ui_print ( VFATAL, "cannot open temporary rc file %s.\n", tmp_rc );
    tmp_ptr = NULL;
  } /* if */

  return ( tmp_ptr );
}                                                         /* create tmp file */



list_sandboxes ( rc )

	/* This procedure lists the currents sandboxes. */

    char      * rc;  				       /* holds rc file name */

{
    FILE      * ptr_file;                                  /* ptr to rc file */
    char        line [ PATH_LEN ],                            /* misc string */
              * key,                                          /* misc string */
              * targ,                                         /* misc string */
              * targ2,                                        /* misc string */
              * line_ptr;                            /* needed for traveling */

  if ( new_rc )
    ui_print ( VNORMAL, "User's rc file, %s, is new.  No sandboxes in it.\n",
	      		 rc );

  if (( ptr_file = fopen ( rc, READ )) == NULL )
    uquit ( ERROR, FALSE, "cannot read user's rc file: %s.\n", rc );

  ui_print ( VNORMAL, "Sandboxes in user's rc file: %s\n", rc );

  while (( line_ptr = fgets ( line, PATH_LEN, ptr_file )) != NULL ) {
    key = nxtarg ( &line_ptr, WHITESPACE );

    if ( streq ( key, DEFAULT )) {
      targ = nxtarg ( &line_ptr, WHITESPACE );
      ui_print ( VALWAYS, "Default sandbox : %s.\n", targ );
    } /* if */

    else if ( streq ( key, SB )) {
      targ = nxtarg ( &line_ptr, WHITESPACE );
      ui_print ( VALWAYS, "Sandbox name is : %s.\n", targ );
    } /* else if */

    else if ( streq ( key, BASE )) {
      targ = nxtarg ( &line_ptr, WHITESPACE );
      targ2 = nxtarg ( &line_ptr, WHITESPACE );
      ui_print ( VALWAYS, "Sandbox base for %s is : %s.\n", targ, targ2 );
    } /* else if */
  } /* while */

  fclose ( ptr_file );
}                                                          /* list sandboxes */



create_sandbox ( proj_dir, def_build, sb, rc_file, sb_rc )

	/* This procedure creates a sandbox.  It does the
	   primary function of this program. */

    char        * proj_dir,           /* location of builds for this project */
                * def_build,                      /* project's default build */
	      	* sb,                                        /* sandbox name */
		* rc_file,                                   /* user rc file */
                * sb_rc;                                /* sandbox's rc file */

{
    RC_CONT       rc_values;                /* hold values read from rc file */
 
  init_rc_values ( &rc_values );

  if ( ! new_rc )
    read_in_rc_info ( rc_file, &rc_values);

  finish_fields ( proj_dir, def_build, sb, sb_rc, &rc_values );

/* ------------------------------------------------------------------------- */
/* Inserted by Frank Li. */
  if ( new_rc ) {
    create_user_rc_file ( &rc_values, sb, rc_file );
  }
/* ------------------------------------------------------------------------- */

  build_sb_structure ( sb, rc_file, rc_values.def_sb_base );

/* ------------------------------------------------------------------------- */
/* Commented out by Frank Li.
  if ( new_rc )
      create_user_rc_file ( &rc_values, sb, rc_file );
*/
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* Inserted by Frank Li. */
  if ( ! new_rc )
    update_user_rc_file ( sb, rc_file, sb_rc );
/* ------------------------------------------------------------------------- */

  create_default_set ( sb, rc_file, sb_rc );
  populate_sandbox ( &rc_values, sb, rc_file, sb_rc );
}                                                          /* create sandbox */



init_rc_values ( rc_values )

	/* This procedure makes sure the rc_values start empty
	   or in the default mode. */

    RC_CONT   * rc_values;                  /* hold values read from rc file */

{
  char * tools, * objects, * sources;

  rc_values->default_sb[0] = NUL;
  rc_values->def_sb_base[0] = NUL;
  rc_values->def_machines[0] = NUL;
  rc_values->obj_dirs[0] = '/';
  rc_values->src_dirs[0] = '/';
  rc_values->obj_dirs[1] = NUL;
  rc_values->src_dirs[1] = NUL;
  rc_values->base = NULL;
  rc_values->sb = NULL;
  rc_values->tools = BACK_CH;
  rc_values->objects = BACK_CH;
  rc_values->sources = BACK_CH;

  if ( ui_is_set ( DIR_OP ))
    strcpy ( rc_values->def_sb_base, ui_arg_value ( DIR_OP, 1, 1 ));

  if ( ui_is_set ( M_OP ))
    strcpy ( rc_values->def_machines, ui_arg_value ( M_OP, 1, 1 ));

  if ( ui_is_set ( TOOLS_OP )) {
    tools = ui_arg_value ( TOOLS_OP, 1, 1 );
    rc_values->tools = *tools;
  }  /* if */

  if ( ui_is_set ( OBJ_OP )) {
    objects = ui_arg_value ( OBJ_OP, 1, 1 );
    rc_values->objects = *objects;
    strcpy ( rc_values->obj_dirs, ui_arg_value ( OBJ_OP, 1, 2 ));
  } /* if */

  if ( ui_is_set ( SRC_OP )) {
    sources = ui_arg_value ( SRC_OP, 1, 1 );
    rc_values->sources = *sources;
    strcpy ( rc_values->src_dirs, ui_arg_value ( SRC_OP, 1, 2 ));
  } /* if */
}                                                          /* init rc values */



read_in_rc_info ( rc_file, rc_values )

	/* This procedure reads in the values of the rc file
	   into the rc_value structure. */

    char      * rc_file;                 /* path and name of rc file to open */
    RC_CONT   * rc_values;                  /* hold values read from rc file */

{
    FILE      * ptr_file;                                  /* ptr to rc file */
    char        line [ PATH_LEN ],                      /* line from rc file */
              * key,                                        /* holds keyword */
              * targ,                                        /* holds target */
              * val;                                          /* holds value */
    char      * line_ptr;

  if (( ptr_file = fopen ( rc_file, READ )) == NULL )
    uquit ( ERROR, FALSE, "cannot open user's rc file %s.\n", rc_file );
  
  while (( line_ptr = fgets ( line, PATH_LEN, ptr_file )) != NULL ) {
    if ( streq ( line, CR_STRING ))                    /* ignore blank lines */
      continue;
    key = nxtarg ( &line_ptr, WHITESPACE );

    if ( streq ( key, DEFAULT )) {
      targ = nxtarg ( &line_ptr, WHITESPACE );
      strcpy ( rc_values->default_sb, targ );
    } /* if */

    else if ( streq ( key, BASE )) {
      targ = nxtarg ( &line_ptr, WHITESPACE );
      val = nxtarg ( &line_ptr, WHITESPACE );
      load_base ( rc_values, targ, val );
    } /* else if */

    else if ( streq ( key, SB )) {
      targ = nxtarg ( &line_ptr, WHITESPACE );
      load_sb ( rc_values, targ );
    } /* else if */
  } /* while */

  if ( ui_ver_level () >= VDEBUG )
    print_out_rc_values ( rc_values );
}                                                         /* read in rc info */



load_base ( rc_values, targ, val )

	/* This procedure creates the next entry structure and
	   puts the base target and value in it. */

    RC_CONT   * rc_values;                  /* hold values read from rc file */
    char        targ [],                                     /* holds target */
                val [];                                       /* holds value */

{
    ENTRY     * entry_ptr;                   /* points to last entry in list */

  if ( rc_values->base == NULL ) {                    /* first entry in base */
    rc_values->base = ( ENTRY * ) malloc ( sizeof ( ENTRY ));
    entry_ptr = rc_values->base;
  } /* if */

  else {                                               /* additional entries */
    entry_ptr = rc_values->base;

    while ( entry_ptr->nextentry != NULL )
      entry_ptr = entry_ptr->nextentry;

    entry_ptr->nextentry = ( ENTRY * ) malloc ( sizeof ( ENTRY ));
    entry_ptr = entry_ptr->nextentry;
  } /* else */

  strcpy ( entry_ptr->target, targ );
  strcpy ( entry_ptr->value, val );
  entry_ptr->nextentry = NULL;
}                                                               /* load base */



load_sb ( rc_values, targ )

	/* This procedure creates the next entry structure and
	   puts the sb target in it. */

    RC_CONT   * rc_values;                  /* hold values read from rc file */
    char        targ [];                                     /* holds target */

{
    ENTRY     * entry_ptr;                   /* points to last entry in list */


  if ( rc_values->sb == NULL )  {                       /* first entry in sb */
    rc_values->sb = ( ENTRY * ) malloc ( sizeof ( ENTRY ));
    entry_ptr = rc_values->sb;
  } /* if */

  else {                                               /* additional entries */
    entry_ptr = rc_values->sb;

    while ( entry_ptr->nextentry != NULL )
      entry_ptr = entry_ptr->nextentry;

    entry_ptr->nextentry = ( ENTRY * ) malloc ( sizeof ( ENTRY ));
    entry_ptr = entry_ptr->nextentry;
  } /* else */

  strcpy ( entry_ptr->value, targ );
  entry_ptr->nextentry = NULL;
}                                                                 /* load sb */



print_out_rc_values ( rc )

	/* This procedure prints out the values in rc_values. */

    RC_CONT   * rc;                         /* hold values read from rc file */

{
    ENTRY     * entry_ptr;                   /* points to last entry in list */

  ui_print ( VDETAIL, "Contents of rc file and entered values:\n" );
  ui_print ( VCONT, "  default sb         : %s\n", rc->default_sb );

  entry_ptr = rc->base;

  while ( entry_ptr != NULL ) {
    ui_print ( VCONT, "  sandbox and base   : %s, %s\n",
		      entry_ptr->target, entry_ptr->value );
    entry_ptr = entry_ptr->nextentry;
  } /* while */

  entry_ptr = rc->sb;

  while ( entry_ptr != NULL ) {
    ui_print ( VCONT, "  sandbox name       : %s\n", entry_ptr->value );
    entry_ptr = entry_ptr->nextentry;
  } /* while */

  ui_print ( VCONT, "  default sb base    : %s\n", rc->def_sb_base );
  ui_print ( VCONT, "  def machines       : %s\n", rc->def_machines );
  ui_print ( VCONT, "  tools population   : %c\n", rc->tools );
  ui_print ( VCONT, "  object population  : %c\n", rc->objects );
  ui_print ( VCONT, "  object directories : %s\n", rc->obj_dirs );
  ui_print ( VCONT, "  sources population : %c\n", rc->sources );
  ui_print ( VCONT, "  source directories : %s\n", rc->src_dirs );
}                                                     /* print out rc values */



finish_fields ( proj_dir, def_build, sb, sb_rc, rc_values )

	/* This procedure finishes off any un-filled fields.  After
	   it is done, the program is ready to do its work.  */

    char        * proj_dir,           /* location of builds for this project */
                * def_build,                      /* project's default build */
	      	* sb,                                        /* sandbox name */
                * sb_rc;                                /* sandbox's rc file */
    RC_CONT     * rc_values;                /* hold values read from rc file */

{
  unique_sb ( sb, rc_values->sb );

  if ( ! ui_is_set ( DIR_OP ))                         /* get directory name */
    set_default_dir ( rc_values, sb );

  if ( ! ui_is_set ( BACK_OP ))                     /* get backing tree name */
    set_backing ( proj_dir, def_build, sb_rc );

  if ( ! ui_is_set ( M_OP ))                            /* get machine names */
    set_default_machine ( rc_values );

  if ( ! ui_is_set ( TOOLS_OP ))         /* get tools population information */
    set_tools_population ( rc_values );

  if ( ! ui_is_set ( OBJ_OP ))      /* get object dir population information */
    set_obj_population ( rc_values );

  if ( ! ui_is_set ( SRC_OP ))      /* get source dir population information */
    set_src_population ( rc_values );
}                                                            /* finish field */



unique_sb ( newsb, currentsb )

	/* This procedure checks to see if the sandbox name is
	   already in use by this user.  It won't allow that
	   even with a different directory.  */

    char      * newsb;                                /* name of new sandbox */
    ENTRY     * currentsb;               /* ptr to list of existing sb names */

{
    ENTRY     * travel = currentsb;               /* ptr to existing sb name */

  while ( travel != NULL ) {
    if ( streq ( newsb, travel->value ))
      uquit ( ERROR, TRUE, "\tsandbox name, %s, already in use.\n", newsb );

    travel = travel->nextentry;
  } /* while */
}                                                               /* unique sb */



set_default_dir ( rc_values, sb )

	/* This procedure sets the directory to the default value
	   or, if that does not exist, it prompts the user for
	   an entry.  It then checks to see if this is a legal
	   directory.  It loops for a legal entry. */

    RC_CONT   * rc_values;                  /* hold values read from rc file */
    char	* sb;  /* sandbox name */

{
    char        line [ PATH_LEN ],                            /* misc string */
	      * line_ptr,
	      * ptr;
    BOOLEAN     legal_dir = FALSE;                           /* misc boolean */

  while ( ! legal_dir ) {
    if ( *rc_values->def_sb_base == NUL ) {
      ui_print ( VALWAYS,
		 "No default for sandbox directory and none entered.\n");
      ui_print ( VALWAYS, "Please enter the full path to the sandbox: " );
      gets ( line );
      line_ptr = line;
      ptr = nxtarg ( &line_ptr, WHITESPACE );
      strcpy ( rc_values->def_sb_base, ptr );
    } /* if */

    if ( *(rc_values->def_sb_base) != SLASH ) {
      ui_print ( VWARN, "Base directory must start with a '/'.\n" );
      rc_values->def_sb_base[0] = NUL;
    } /* if */

    else {
      ui_load ( DIR_OP, 1, rc_values->def_sb_base ); 

      if ( !( legal_dir = legal_sandbox_dir ( sb )))
	rc_values->def_sb_base[0] = NUL;
    } /* else */
  } /* while */
}                                                         /* set default dir */

/* --------- Modified set_backing to add ODE_BUILD_LIST -----------akc------ */

set_backing ( proj_dir, def_build, sb_rc )

	/* This procedure prompts the user for the backing tree after
	   first listing the available builds.  The available builds
	   are the directories in the proj_dir; the default build is
	   the value of def_build. */

    char      * proj_dir,                       /* default project directory */
              * def_build,                        /* project's default build */
              * sb_rc;                                  /* sandbox's rc file */

{
    DIR       * dir_ptr;                            /* points to a directory */
    struct      dirent * ds;               /* points to struct with dir list */
    char        line [ PATH_LEN ],                            /* misc string */
              * build,                                        /* misc string */
              * line_ptr;                             /* misc string pointer */
/*---------------------- another of my additions ----------------------------*/
    char      * name_ptr;                    /* name of entry in directory */
    int       i;                             /* counter */
    FILE      * f_ptr;                                  /* ptr to rc file */
    char      * key;
    char      * build_list ;

    build_list = getenv("ODE_BUILD_LIST") ;
    if (build_list == NULL ) {
       build_list= DEFAULT_BUILD_LIST ;
/*       ui_print( VALWAYS, "ODE_BUILD_LIST not set\n" ) ; */
    }
/*------------------- end of another of my additions ------------------------*/
    while ( ! ui_is_set ( BACK_OP )) {
      if ( proj_dir == NULL ) {
/*-------------my fix-------------------*/
        if (( f_ptr = fopen ( build_list, READ )) == NULL ) {
          ui_print ( VALWAYS,
          "Build list %s not found or not valid:\n", build_list);
          ui_print ( VCONT, "Enter complete path to backing tree.\n");
        } /* if */
        else {
          ui_print ( VALWAYS,
          "Enter name of backing tree: from the known list, or a complete path\n");
          ui_print ( VCONT, "or a name from the build list.\n" );
          ui_print ( VCONT, "Known current builds include:\n" );
          while (( line_ptr = fgets ( line, PATH_LEN, f_ptr )) != NULL ) {
            if ( streq ( line, CR_STRING )) continue;     /* ignore blank lines */
            key = nxtarg ( &line_ptr, WHITESPACE );
            ui_print ( VALWAYS, "   %s", key );
          } /* while */
/*-------------end of my fix-------------------*/
          ui_print ( VALWAYS, "\nBacking tree? <%s> ", def_build );
          /* closedir ( dir_ptr );   ***** NO opendir for this command */
        } /* else */
      } /* if */

      else {
        if (( dir_ptr = opendir ( proj_dir )) == NULL ) {
          ui_print ( VALWAYS, "No known current builds to pick from.\n" );
          ui_print ( VCONT, "Enter absolute path to backing tree or name from build list: " );
        } /* if */

        else {
/*-------------my fix-------------------*/
          if (( f_ptr = fopen ( build_list, READ )) == NULL ) {
            ui_print ( VALWAYS,
            "Build list %s not found or not valid:\n", build_list);
            ui_print ( VCONT, "Enter a complete path to backing tree.\n");

          } /* if */
          else {
            ui_print ( VALWAYS,
            "Enter name of backing tree: from the known list or a complete path\n");
            ui_print ( VCONT, "or a name from the build list.\n" );
            ui_print ( VCONT, "Known current builds include:\n" );
            while (( line_ptr = fgets ( line, PATH_LEN, f_ptr )) != NULL ) {
              if ( streq ( line, CR_STRING ))   continue;  /* ignore blank lines */
              key = nxtarg ( &line_ptr, WHITESPACE );
              ui_print ( VALWAYS, "   %s", key );
            } /* while */
            ui_print ( VALWAYS, "\nBacking tree? <%s> ", def_build );
          } /* else */
/*-------------end of my fix-------------------*/
          closedir ( dir_ptr );
        } /* else */
      } /* else */
      /* Warning -- stdin is at end-of-file ?why? */
      errno=0 ;
      gets ( line );

      if ( *line == NUL || *line == NEWLINE ) {
        if ( def_build == NULL ) 
          continue;
        else 
          ui_load ( BACK_OP, 1, def_build );
      } else {
        line_ptr = line;
        build = nxtarg ( &line_ptr, WHITESPACE );

        if ( *build == NUL )
          ui_load ( BACK_OP, 1, def_build );
        else
          ui_load ( BACK_OP, 1, build );
      } /* else */


      legal_backing_tree ( proj_dir, sb_rc );
    } /* while */
}                                                             /* set backing */
  


set_default_machine ( rc_values )

	/* This procedure fills the machine entry either by
	   copying it from the defaults or prompting the user.
	   If the answer is illegal, it continues to prompt
	   the user. */

    RC_CONT   * rc_values;                  /* hold values read from rc file */

{
    char        line [ PATH_LEN ],                            /* misc string */
	      * line_ptr,
	      * ptr;
    BOOLEAN     legal = FALSE;                               /* misc boolean */

  if ( *rc_values->def_machines == NUL ) {
    while ( ! legal ) {
      ui_print ( VALWAYS,
		 "Enter colon separated list of machines: <%s> ", DEF_MACH );
      gets ( line );
      line_ptr = line;
      ptr = nxtarg ( &line_ptr, WHITESPACE );

      if ( *ptr == NUL )
        ui_load ( M_OP, 1, DEF_MACH );
      else
        ui_load ( M_OP, 1, ptr );

      legal = legal_machine_names ();
    } /* while */
  } /* if */

  else
    ui_load ( M_OP, 1, rc_values->def_machines );
}                                                     /* set default machine */



set_tools_population ( rc_values )

	/* This procedure prompts the user for population of the
	   tools area. */

    RC_CONT   * rc_values;                  /* hold values read from rc file */

{
    char        line [ PATH_LEN ];                            /* misc string */
    BOOLEAN     chosen = FALSE;                              /* misc boolean */

  while ( ! chosen ) {
    ui_print ( VALWAYS,
      "Populate tools with links or copies or leave backed? [ l | c | <b> ] " );
    line[0] = NUL;
    gets ( line );

    if ( *line == NUL ) {
      rc_values->tools = BACK_CH;
      chosen = TRUE;
    } /* if */

    else {
      rc_values->tools = line[0];

      if (( rc_values->tools != LINK_CH ) &&
	  ( rc_values->tools != COPY_CH ) &&
	  ( rc_values->tools != BACK_CH ))
        ui_print ( VWARN, "Entry must be 'l', 'c', or 'b'.\n" );
      else
	chosen = TRUE;
    } /* else */
  } /* while */
}                                                    /* set tools population */



set_obj_population ( rc_values )

	/* This procedure prompts the user for population of the
	   object area.  It gets both the type of population and,
	   if it is not backed, the list of directories to populate. */

    RC_CONT   * rc_values;                  /* hold values read from rc file */

{
    char        line [ PATH_LEN ],                            /* misc string */
              * line_ptr,                                 /* misc string ptr */
              * arg;                                      /* misc string ptr */
    BOOLEAN     chosen = FALSE;                              /* misc boolean */

  while ( ! chosen ) {
    ui_print ( VALWAYS,
     "Populate objects with links or copies or leave backed? [ l | c | <b> ] ");
    line[0] = NUL;
    gets ( line );

    if ( *line == NUL ) {
      rc_values->objects = BACK_CH;
      chosen = TRUE;
    } /* if */

    else {
      rc_values->objects = line[0];

      if (( rc_values->objects != LINK_CH ) &&
	  ( rc_values->objects != COPY_CH ) &&
	  ( rc_values->objects != BACK_CH ))
        ui_print ( VWARN, "Entry must be 'l', 'c', or 'b'.\n" );
      else
	chosen = TRUE;
    } /* else */
  } /* while */

  if ( rc_values->objects != BACK_CH ) {
    *rc_values->obj_dirs = NUL;

    while ( *rc_values->obj_dirs == NUL ) {
      ui_print ( VALWAYS,
       "Enter a colon separated list of the object directories to populate\n" );
      ui_print ( VALWAYS, "each beginning with '/'.  '/' can be used " );
      ui_print ( VALWAYS, "to indicate all. <NO DEFAULT> " );
      gets ( line );
      line_ptr = line;
      arg = nxtarg ( &line_ptr, WHITESPACE );

      if ( check_colon_line ( arg ))
	strcpy ( rc_values->obj_dirs, arg );
      else
	*rc_values->obj_dirs = NUL;
    } /* while */
  } /* if */
}                                                      /* set obj population */



set_src_population ( rc_values )

	/* This procedure prompts the user for population of the
	   source area.  It gets both the type of population and,
	   if it is not backed, the list of directories to populate. */

    RC_CONT   * rc_values;                  /* hold values read from rc file */

{
    char        line [ PATH_LEN ],                            /* misc string */
              * line_ptr,                                 /* misc string ptr */
              * arg;                                      /* misc string ptr */
    BOOLEAN     chosen = FALSE;                              /* misc boolean */

  while ( ! chosen ) {
    ui_print ( VALWAYS,
      "Populate source with links or copies or leave backed? [ l | c | <b> ] ");
    *line = NUL;
    gets ( line );

    if ( *line == NUL ) {
      rc_values->sources = BACK_CH;
      chosen = TRUE;
    } /* if */

    else {
      rc_values->sources = line[0];

      if (( rc_values->sources != LINK_CH ) &&
	  ( rc_values->sources != COPY_CH ) &&
	  ( rc_values->sources != BACK_CH ))
        ui_print ( VWARN, "Entry must be 'l', 'c', or 'b'.\n" );
      else
	chosen = TRUE;
    } /* else */
  } /* while */

  if ( rc_values->sources != BACK_CH ) {
    *rc_values->src_dirs = NUL;

    while ( *rc_values->src_dirs == NUL ) {
      ui_print ( VALWAYS,
	"Enter a colon separated list of the source directories to populate\n");
      ui_print ( VALWAYS, "each beginning with '/'.  '/' can be used " );
      ui_print ( VALWAYS, "to indicate all. <NO DEFAULT> " );
      gets ( line );
      line_ptr = line;
      arg = nxtarg ( &line_ptr, WHITESPACE );

      if ( check_colon_line ( arg ))
	strcpy ( rc_values->src_dirs, arg );
      else
	*rc_values->src_dirs = NUL;
    } /* while */
  } /* if */
}                                                      /* set src population */



build_sb_structure ( sb, rc_file, sbbase )

	/* This procedure makes the directories and files necessary
	   for a sandbox.  */

    char      	* sb,                                        /* sandbox name */
		* rc_file,   /* user rc file */
		* sbbase;  /* sandbox base dir */

{
    char          mkconf [ PATH_LEN ],                        /* misc string */
                  sb_path [ PATH_LEN ];               /* path and name to sb */

  concat ( sb_path, PATH_LEN, sbbase, "/", sb, NULL );

  if ( ui_is_info ()) {
    ui_print ( VALWAYS, "Would have created directory: %s.\n", sb_path );
    return;
  } /* if */

  if ( mkdir ( sb_path, MODE ) == ERROR ) {
    remove_sb ( sb, rc_file, sbbase );
    uquit ( ERROR, FALSE, "could not create sandbox %s.\n", sb_path );
  } /* if */

  if ( chdir ( sb_path ) == ERROR ) {
    remove_sb ( sb, rc_file, sbbase );
    uquit ( ERROR, FALSE, "\tcould not change directories to %s.\n", sb_path );
  } /* if */

  ui_print ( VDETAIL, "Successfully created new sandbox dir %s.\n", sb_path );

/* ------------------------------------------------------------------------- */
/* Changed by Michael Winestock
/*
/* From:
/*
  if (( mkdir ( SRC_DIR, MODE ) == ERROR ) ||
      ( mkdir ( OBJ_DIR, MODE ) == ERROR ) ||
      ( mkdir ( EXP_DIR, MODE ) == ERROR ) ||
      ( mkdir ( RC_DIR, MODE ) == ERROR ) ||
      ( mkdir ( SHIP_DIR, MODE ) == ERROR ) ||
      ( mkdir ( INST_DIR, MODE ) == ERROR ) ||
      ( mkdir ( HOST_DIR, MODE ) == ERROR ) ||
      ( mkdir ( TOOL_DIR, MODE ) == ERROR )) {
    remove_sb ( sb, rc_file, sbbase );     
/*
/* To:
/*                                                                           */
  if (( mkdir ( SRC_DIR, MODE ) == ERROR ) ||
      ( mkdir ( OBJ_DIR, MODE ) == ERROR ) ||
      ( mkdir ( RC_DIR, MODE ) == ERROR )) {
    remove_sb ( sb, rc_file, sbbase );     
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* Changed by Michael Winestock
/*
/* From:
/*
    uquit ( ERROR, FALSE,
	    "could not create sandbox directories:\n %s, %s, %s, %s, %s, %s, %s.\n",
	      SRC_DIR, OBJ_DIR, EXP_DIR, RC_DIR, SHIP_DIR, INST_DIR, HOST_DIR, TOOL_DIR );

  ui_print ( VALWAYS, "Successfully created new dirs:%s, %s, %s, %s, %s, %s, %s.\n",
             SRC_DIR, OBJ_DIR, EXP_DIR, RC_DIR, SHIP_DIR, INST_DIR, HOST_DIR, TOOL_DIR );
/*
/* To:
/*                                                                          */
    uquit ( ERROR, FALSE,
	    "could not create sandbox directories:\n %s, %s, %s\n",
	      SRC_DIR, OBJ_DIR, RC_DIR );

  ui_print ( VALWAYS, "Successfully created new dirs: %s, %s, %s\n",
             SRC_DIR, OBJ_DIR, RC_DIR );
/* ------------------------------------------------------------------------- */
  } /* if */

/* ------------------------------------------------------------------------- */
/* Changed by Michael Winestock
/*
/* From:
/*
  ui_print ( VALWAYS, "Successfully created new dirs:%s, %s, %s, %s, %s, %s, %s.\n",
             SRC_DIR, OBJ_DIR, EXP_DIR, RC_DIR, SHIP_DIR, INST_DIR, HOST_DIR, TOOL_DIR );
/*
/* To:
/*                                                                           */
  ui_print ( VALWAYS, "Successfully created new dirs: %s, %s, %s\n",
             SRC_DIR, OBJ_DIR, RC_DIR );
/* ------------------------------------------------------------------------- */

  if ( symlink ( ui_arg_value ( BACK_OP, 1, 1 ), LINK_DIR ) == ERROR ) {
    remove_sb ( sb, rc_file, sbbase );
    uquit ( ERROR, FALSE, "could not create backing tree link to: %s.\n",
		           ui_arg_value ( BACK_OP, 1, 1 ));
  } /* if */

  concat ( mkconf, PATH_LEN, "../", LINK_DIR, "/", MKCONF_LINK, NULL );

  if ( symlink ( mkconf, MKCONF_LINK ) == ERROR ) {                                 remove_sb ( sb, rc_file, sbbase );
    uquit ( ERROR, FALSE, "could not create link, %s, to: %s.\n",
		    	   MKCONF_LINK, mkconf );
  } /* if */

  ui_print ( VDETAIL, "Successfully created backing tree link to %s.\n",
		       ui_arg_value ( BACK_OP, 1, 1 ));
  create_machine_sub_dirs ( sb, rc_file, sbbase, ui_arg_value ( M_OP, 1, 1 ));
  ui_print ( VDETAIL, "Created new machine dirs %s.\n",
		       ui_arg_value ( M_OP, 1, 1 ));
  create_sb_rc_files ( ui_arg_value ( BACK_OP, 1, 1 ), sb, rc_file, sbbase,
		       LOCAL_RC, LOCAL_T_RC , sb_path );
  create_sb_rc_files ( ui_arg_value ( BACK_OP, 1, 1 ), sb, rc_file, sbbase,
		       SHARED_RC, SHARED_T_RC, sb_path );
}                                                      /* build sb structure */



create_machine_sub_dirs ( sb, rc_file, sbbase, machines )

	/* This procedure attempts to create all the machine dependent
	   directories under OBJ.  It expects a colon separated list
	   of machines.  */

    char      	* sb,                                        /* sandbox name */
		* rc_file,   /* user rc file */
		* sbbase,  /* sandbox base dir */
    		* machines;              /* colon separated list of machines */

{
    char          copy [ PATH_LEN ],        /* holds edited copy of machines */
                  opath [ PATH_LEN ],                     /* path to objects */
/* ------------------------------------------------------------------------- */
/* Removed by Michael Winestock
                  tpath [ PATH_LEN ],
                  epath [ PATH_LEN ],
                  spath [ PATH_LEN ],
                  ipath [ PATH_LEN ],
                  hpath [ PATH_LEN ],
   ------------------------------------------------------------------------- */
              	* list,                            /* points to each machine */
              	* line_ptr;                        /* points to each machine */

  strcpy ( copy, machines );
  line_ptr = copy;

  do {
    list = nxtarg ( &line_ptr, ":" );

    concat ( opath, PATH_LEN, OBJ_DIR, "/", list, NULL );

    if ( mkdir ( opath, MODE ) == ERROR ) {
      remove_sb ( sb, rc_file, sbbase );
      uquit ( ERROR, FALSE, "\tcould not make directory %s.\n", opath );
    } /* if */

/* ------------------------------------------------------------------------- */
/* Removed by Michael Winestock
    concat ( tpath, PATH_LEN, TOOL_DIR, "/", list, NULL );
    concat ( epath, PATH_LEN, EXP_DIR, "/", list, NULL );
    concat ( spath, PATH_LEN, SHIP_DIR, "/", list, NULL );
    concat ( ipath, PATH_LEN, INST_DIR, "/", list, NULL );
    concat ( hpath, PATH_LEN, HOST_DIR, "/", list, NULL );

    if ( mkdir ( tpath, MODE ) == ERROR ) {
      remove_sb ( sb, rc_file, sbbase );
      uquit ( ERROR, FALSE, "\tcould not make directory %s.\n", tpath );
    }

    if ( mkdir ( epath, MODE ) == ERROR ) {
      remove_sb ( sb, rc_file, sbbase );
      uquit ( ERROR, FALSE, "\tcould not make directory %s.\n", epath );
    }

    if ( mkdir ( spath, MODE ) == ERROR ) {
      remove_sb ( sb, rc_file, sbbase );
      uquit ( ERROR, FALSE, "\tcould not make directory %s.\n", spath );
    }

    if ( mkdir ( ipath, MODE ) == ERROR ) {
      remove_sb ( sb, rc_file, sbbase );
      uquit ( ERROR, FALSE, "\tcould not make directory %s.\n", ipath );
    }

    if ( mkdir ( hpath, MODE ) == ERROR ) {
      remove_sb ( sb, rc_file, sbbase );
      uquit ( ERROR, FALSE, "\tcould not make directory %s.\n", hpath );
    }
  -------------------------------------------------------------------------- */
  } while ( *line_ptr != NUL );
}                                                 /* create machine sub dirs */



create_sb_rc_files ( backing, sb, rc_file, sbbase, rcfile, trcfile, addition )

	/* This procedure creates two new rc files, converting the
	   template when necessary and copying the lines into the
	   new template and local rc files.  */

    char        * backing,                           /* path to backing tree */
              	* sb,                                      /* sandbox name */
              	* rc_file,                       /* user rc file */
		* sbbase,  /* sandbox base dir */
              	* rcfile,                        /* path and name to rc file */
              	* trcfile,              /* path and name to template rc file */
              	* addition;                 /* string to add to sandbox base */

{
    FILE      	* org,                                      /* original file */
              	* tcopy,                                    /* template file */
              	* rccopy;                                       /* rc file */
    char          path [ PATH_LEN ],                          /* misc string */
                  line [ PATH_LEN ],                          /* misc string */
                  line_copy [ PATH_LEN ],                     /* misc string */
              	* key,                                        /* misc string */
              	* target,                                     /* misc string */
    		* line_ptr;  /* misc char ptr */

  concat ( path, PATH_LEN, backing, "/", trcfile, NULL );

  if (( org = fopen ( path, READ )) == NULL ) {
    remove_sb ( sb, rc_file, sbbase );
    uquit ( ERROR, FALSE, "cannot open backing tree rc file: %s.\n", path );
  } /* if */

  if (( tcopy = fopen ( trcfile, WRITE )) == NULL ) {
    remove_sb ( sb, rc_file, sbbase );
    uquit ( ERROR, FALSE, "cannot open local rc file: %s.\n", trcfile );
  } /* if */

  if (( rccopy = fopen ( rcfile, WRITE )) == NULL ) {
    remove_sb ( sb, rc_file, sbbase );
    uquit ( ERROR, FALSE, "cannot open local rc file: %s.\n", rcfile );
  } /* if */

  while (( line_ptr = fgets ( line, PATH_LEN, org )) != NULL ) {
    fputs ( line, tcopy );                    /* template gets an exact copy */
    strcpy ( line_copy, line );
    line_ptr = line_copy;
    key = nxtarg ( &line_ptr, WHITESPACE );

    if ( ! streq ( key, REPLACE )) {
      fputs ( line, rccopy );
      continue;
    } /* if */

    target = nxtarg ( &line_ptr, WHITESPACE );

    if ( streq ( target, SB_BASE ))                       /* if sandbox_base */
      fprintf ( rccopy, "%s %s %s\n", key, target, addition );
							  /* put in addition */
    else
      fputs ( line, rccopy );
  } /* while */

  fclose ( org );
  fclose ( tcopy );
  fclose ( rccopy );
}                                                      /* create sb rc files */



create_user_rc_file ( rc, sb, rc_file )

	/* This procedure creates a new rc file from the information
	   which the user has put into this sandbox. */

    RC_CONT     * rc;                       /* hold values read from rc file */
    char      	* sb,                                        /* sandbox name */
		* rc_file;                                   /* user rc file */

{
    FILE      * ptr_file;                                  /* ptr to rc file */

  if ( ui_is_info ())
    ui_print ( VALWAYS, "Would have created new user rc file: %s\n", rc_file );
  else
    ui_print ( VDETAIL, "Creating new user rc file: %s.\n", rc_file );

  strcpy ( rc->default_sb, sb );
  strcpy ( rc->def_machines, ui_arg_value ( M_OP, 1, 1 ));
  strcpy ( rc->def_sb_base, ui_arg_value ( DIR_OP, 1, 1 ));

  if ( ui_ver_level () >= VDETAIL )
    print_out_rc_values ( rc );

  if ( ui_is_info ())
    return;

  if (( ptr_file = fopen ( rc_file, WRITE )) == NULL )
    uquit ( ERROR, FALSE, "\tcannot create new user rc file: %s.\n", rc_file );

  fprintf ( ptr_file, "\t# sandbox rc file created by %s\n", progname );
  fprintf ( ptr_file, "\n	# default sandbox\n" );
  fprintf ( ptr_file, "%s %s\n\n", DEFAULT, rc->default_sb );
  fprintf ( ptr_file, "\t# base directories to sandboxes\n" );
  fprintf ( ptr_file, "%s * %s\n\n", BASE, rc->def_sb_base );
  fprintf ( ptr_file, "\t# list of sandboxes\n" );
  fprintf ( ptr_file, "%s %s\n\n", SB, ui_entry_value ( ARGS_OP, 1 ));
  fprintf ( ptr_file, "\t# mksb config specific\n" );
  fprintf ( ptr_file, "%s %s %s\n", progname, DIR_OP,
		        ui_arg_value ( DIR_OP, 1, 1 ));
  fprintf ( ptr_file, "%s %s %s\n", progname, M_OP, rc->def_machines );
  fprintf ( ptr_file, "%s %s %c\n", progname, TOOLS_OP, rc->tools );
  fprintf ( ptr_file, "%s %s %c %s\n", progname, OBJ_OP, rc->objects,
				       rc->obj_dirs );
  fprintf ( ptr_file, "%s %s %c %s\n", progname, SRC_OP, rc->sources,
				       rc->src_dirs );

  fclose ( ptr_file );
}                                                     /* create user rc file */



update_user_rc_file ( sb, rc, sbrc )

	/* This procedure writes any new information into the
	   rc file or, if the rc file didn't exist, it creates
	   a new one. */

    char      	* sb,                                    /* sandbox name */
              	* rc,                                    /* user rc file */
              	* sbrc;                        /* sandbox rc file */

{
    FILE      	* ptr_file,                                /* ptr to rc file */
              	* tmp_file;                            /* ptr to tmp rc file */
    char      	* word,                                       /* misc string */
                  line [ PATH_LEN ],                          /* misc string */
                  line_copy [ PATH_LEN ],                     /* misc string */
                  tmp_rc [ PATH_LEN ],                        /* misc string */
                  new_base_dir [ PATH_LEN ],    /* to hold new base dir line */
              	* line_ptr,                           /* misc string pointer */
              	* dir;                             /* holds sandbox base dir */
    BOOLEAN       default_in = FALSE,                  /* default sb boolean */
                  base_dir_in,                       /* new base dir boolean */
                  sb_name_in = FALSE;                     /* sb name boolean */

  ui_print ( VDEBUG, "Updating rc file: %s.\n", rc );

  if (( ptr_file = fopen ( rc, READ )) == NULL )
    uquit ( ERROR, FALSE, "\tcannot read user's rc file %s.\n", rc );

  dir = ui_arg_value ( DIR_OP, 1, 1 );

  if ( ! ui_is_info ())
    tmp_file = create_tmp_file ( rc, tmp_rc );

  ui_print ( VDEBUG, "s: %s d: %s sbrc: %s rc: %s \n", sb, dir, sbrc, rc );
  base_dir_in = ( current_sb ( &sb, &dir, &sbrc, &rc ) == OK );

  if ( ! base_dir_in )
    concat ( new_base_dir, PATH_LEN, BASE, " ", sb, " ", dir, NULL );

  while (( line_ptr = fgets ( line, PATH_LEN, ptr_file )) != NULL ) {
    strcpy ( line_copy, line );
    line_ptr = line_copy;
    word = nxtarg ( &line_ptr, WHITESPACE );

    if ( streq ( word, DEFAULT )) {
      if ( ! ui_is_set ( DEF_OP )) {
        if ( ! ui_is_info ())
          fputs ( line, tmp_file );
	continue;
      } /* if */

      if ( default_in )
	continue;

      if ( ! ui_is_info ())
        fprintf ( tmp_file, "%s %s\n\n", DEFAULT, sb );

      default_in = TRUE;
      ui_print ( VDEBUG, "Making default sb, %s, in user's rc file.\n", sb );
      continue;
    } /* if */

    if (( streq ( word, BASE )) && ( ! base_dir_in )) {
      if ( ! ui_is_info ())
        fprintf ( tmp_file, "%s\n", new_base_dir );

      base_dir_in = TRUE;
      ui_print ( VDEBUG, "Adding base sandbox directory to user's rc file:\n" );
      ui_print ( VCONT, "%s.\n", new_base_dir );
    } /* if */

    else if (( streq ( word, SB )) && ( ! sb_name_in )) {
      if ( ! ui_is_info ())
        fprintf ( tmp_file, "%s %s\n", SB, sb );

      sb_name_in = TRUE;
      ui_print ( VDEBUG, "Adding new sandbox to user's rc file: %s.\n", sb );
    } /* else if */

    if ( ! ui_is_info ())
      fputs ( line, tmp_file );
  } /* while */

		  /* these last are for the case where there is no entry yet */
  if (( ui_is_set ( DEF_OP )) && ( ! default_in )) {
    if ( ! ui_is_info ())
      fprintf ( tmp_file, "%s %s\n\n", DEFAULT, sb );

    ui_print ( VDETAIL, "Making new, default sandbox, %s, in user's rc file.\n",
		         sb );
  } /* if */

  if ( !  base_dir_in ) {
    if ( !ui_is_info ())
      fprintf ( tmp_file, "%s\n", new_base_dir );

    ui_print ( VDEBUG, "Adding base sandbox directory to user's rc file:\n" );
    ui_print ( VCONT, "%s.\n", new_base_dir );
  } /* if */
      
  if ( ! sb_name_in ) {
     if ( ! ui_is_info ())
       fprintf ( tmp_file, "%s %s\n", SB, sb );

     ui_print ( VDETAIL, "Adding new sandbox to user's rc file: %s.\n", sb );
  } /* if */
      
  fclose ( ptr_file );

  if ( ! ui_is_info ()) {
    fclose ( tmp_file );
    rename ( tmp_rc, rc );
  } /* if */
}                                                     /* update user rc file */



create_default_set ( sb, rc_file, sb_rcfile )

	/* This procedure creates the default set for the sandbox.
	   It distinguishes between names which start with a capital
	   letter and those that don't by only preppending the user's
	   name to those that don't. */

    char	* sb,  /* sandbox name */
		* rc_file,  /* users rc file */
		* sb_rcfile;  /* sandbox rc file */

{
    FILE      	* setptr;                                   /* original file */
    char          path [ PATH_LEN ],                          /* misc string */
                  tmp_name [ NAME_LEN ],                      /* misc string */
		* sbbase = NULL,  /* sandbox base dir */
              	* env_user,                      /* holds values from getenv */
              	* ptr,                                       /* misc pointer */
              	* set;                                 /* points to set name */

  if ( sb == NULL || rc_file == NULL || sb_rcfile == NULL ) {
    if ( current_sb ( &sb, &sbbase, &sb_rcfile, &rc_file ) == ERROR )
      uquit ( ERROR, FALSE, "Last try at getting sandbox env failed." );
  } /* if */

  if (( env_user = getenv ( "USER" )) == NULL ) {
    ui_print ( VWARN, "env var USER not set.  No default set.\n" );
    return;
  } /* if */

  concat ( path, PATH_LEN, ui_arg_value ( DIR_OP, 1, 1 ), "/", sb, "/", SET_RC,
		 NULL );
  ui_print ( VDEBUG, "path for sets file is: %s.\n", path );
  set = ui_arg_value ( SET_OP, 1, 1 );

  if ( ! ui_is_info ()) {
    if (( setptr = fopen ( path, WRITE )) == NULL ) {
      ui_print ( VWARN, "cannot open sets rc file, %s,\n", path );
      ui_print ( VCONT, "for writing.  No default set.\n" );
      return;
    } /* if */

    ptr = concat ( tmp_name, PATH_LEN, env_user, "_", NULL );

    if ((( *set < 'A' ) || ( *set > 'Z' )) &&
	( strncmp ( tmp_name, set, ptr - tmp_name )))
      fprintf ( setptr, "%s %s_%s\n", DEFAULT, env_user, set );
    else
      fprintf ( setptr, "%s %s\n", DEFAULT, set );

    fclose ( setptr );
  } /* if */

  ui_print ( VDETAIL, "Building default set: %s.\n", set );

/* ------------------ Removed by Michael Winestock (MJW). ------------------ */
# ifdef MJW_REMOVED_WORKON_CODE
  switch ( fork ()) {
    case ERROR: ui_print ( VFATAL,
		           "fork failed, could not make set: %s.\n", set );
		break;

    case CHILD: if ( setenv ( "EDITOR", "/bin/cat", TRUE ) == ERROR )
		  ui_print ( VWARN, "setenv failure of EDITOR.\n" );

		  if ( ! ui_is_info ()) {
		    execlp ( WORKON, WORKON, ui_ver_switch (), NOSH_OP,
			     SB_OP, sb, RC_OP, rc_file, set, NULL );
                    uquit ( ERROR, FALSE,
			    "returned from execlp of: %s.\n", WORKON );
		  } /* if */

		  else
                    ui_print ( VALWAYS, "workon %s %s %s %s %s %s %s\n",
		          ui_ver_switch (), NOSH_OP, SB_OP, sb, RC_OP, rc_file,
			  set );

		break;

    default :   wait (( int * ) 0 );
	        break;
  } /* switch */
# endif
/* ------------------------------------------------------------------------- */
}                                                      /* create default set */



populate_sandbox ( rc_values, sb_name, rc_file, sb_rcfile )

	/* This procedure populates the tools, object, and
	   source areas per the user's request. */

    RC_CONT   	* rc_values;                /* hold values read from rc file */
    char        * sb_name,                  /* sandbox name */
		* rc_file,   /* user rc file */
		* sb_rcfile;  /* sandbox rc file */

{
    char          copy [ PATH_LEN ],        /* holds edited copy of dir list */
                  tsubdir [ PATH_LEN ],                  /* sub dir to tools */
                  osubdir [ PATH_LEN ],                /* sub dir to objects */
                  srcpath [ PATH_LEN ],               /* path of src to link */
              	* list,                          /* points to each directory */
              	* copy_ptr;                        /* points to each machine */
  
  strcpy ( copy, ui_arg_value ( M_OP, 1, 1 ));          /* do tools and objs */
  copy_ptr = copy;

  do {                                    /* do a pass for each machine type */
    list = nxtarg ( &copy_ptr, ":" );
    concat ( tsubdir, PATH_LEN, "/", TOOL_DIR, "/", list, NULL );
    concat ( osubdir, PATH_LEN, "/", OBJ_DIR, "/", list, NULL );
    tools_obj_run_through ( sb_name, rc_file, sb_rcfile, tsubdir, osubdir,
			    rc_values );
  } while ( *copy_ptr != NUL );

  if ( rc_values->sources == BACK_CH )                     /* do sources now */
    return;

  strcpy ( copy, rc_values->src_dirs );       /* don't want to ruin original */
  copy_ptr = copy;                              /* copy can't move, list can */

/* ------------------------------------------------------------------------- */
/* Inserted by Frank Li. */
  sb_rcfile = "sb_rcfile";
/* ------------------------------------------------------------------------- */

  do {                                /* do a pass for each source directory */
    list = nxtarg ( &copy_ptr, ":" );
    concat ( srcpath, PATH_LEN, "/", SRC_DIR, list, NULL );

    if ( rc_values->sources == COPY_CH )
      create_local_files ( sb_name, rc_file, sb_rcfile, srcpath, TRUE );
    else if ( rc_values->sources == LINK_CH )
      create_local_files ( sb_name, rc_file, sb_rcfile, srcpath, FALSE );
  } while ( *copy_ptr != NUL );
}                                                        /* populate sandbox */



tools_obj_run_through ( sb_name, rc_file, sb_rcfile, tsubdir, osubdir,
			rc_values )

	/* This procedure creates the various run throughs for
	   mklinks for tools and for each machine type. */

    char        * sb_name,                  /* sandbox name */
		* rc_file,   /* user rc file */
		* sb_rcfile,  /* sandbox rc file */
                  tsubdir [ PATH_LEN ],                  /* sub dir to tools */
                  osubdir [ PATH_LEN ];                /* sub dir to objects */
    RC_CONT   	* rc_values;                /* hold values read from rc file */

{
    char        copy [ PATH_LEN ],          /* holds edited copy of dir list */
              * list,                            /* points to each directory */
              * copy_ptr,                          /* points to each machine */
                objpath [ PATH_LEN ];                 /* path of obj to link */

/* ------------------------------------------------------------------------- */
/* Inserted by Frank Li. */
  sb_rcfile = "sb_rcfile";
/* ------------------------------------------------------------------------- */

  if ( rc_values->tools == COPY_CH )
    create_local_files ( sb_name, rc_file, sb_rcfile, tsubdir, TRUE );
  else if ( rc_values->tools == LINK_CH )
    create_local_files ( sb_name, rc_file, sb_rcfile, tsubdir, FALSE );
  if ( rc_values->objects == BACK_CH )
    return;

  strcpy ( copy, rc_values->obj_dirs );       /* don't want to ruin original */
  copy_ptr = copy;                              /* copy can't move, list can */

  do {
    list = nxtarg ( &copy_ptr, ":" );

/* ------------------------------------------------------------------------- */
/* Commented out by Frank Li.
    concat ( objpath, PATH_LEN, osubdir, "/", list, NULL );
*/
/* ------------------------------------------------------------------------- */

    concat ( objpath, PATH_LEN, osubdir, "", list, NULL );

    if ( rc_values->objects == COPY_CH )
      create_local_files ( sb_name, rc_file, sb_rcfile, objpath, TRUE );
    else if ( rc_values->objects == LINK_CH )
      create_local_files ( sb_name, rc_file, sb_rcfile, objpath, FALSE );
  } while ( *copy_ptr != NUL );
}                                                   /* tools obj run through */



create_local_files ( sb_name, rc_file, sb_rcfile, directory, copy )

	/* This procedure does the call to mklinks to generate
	   the local copies of the files.  It uses copy to
	   determine if the files should be copied or linked. */

    char      	* sb_name,                     /* the sandbox to put file in */
		* rc_file,                                   /* user rc file */
		* sb_rcfile,                              /* sandbox rc file */
              	* directory;               /* the directory to actually copy */
    BOOLEAN       copy;                                      /* copy or link */

{
  ui_print ( VDETAIL, "%s directory into sandbox:\n",
		       copy ? "Copying" : "Linking" );
  switch ( fork ()) {
    case ERROR: ui_print ( VFATAL, "fork failed at mklinks.\n" );
		break;

    case CHILD:
      if ( ! ui_is_info ()) {
	if ( copy ) {
          if ( ui_is_auto ())
	    execlp ( MKLINKS, MKLINKS, COPY_OP, SB_OP, sb_name, RC_OP,
		     rc_file, SB_RC_OP, sb_rcfile, ui_ver_switch (), AUTO_OP,
		     directory, NULL );
          else
	    execlp ( MKLINKS, MKLINKS, COPY_OP, SB_OP, sb_name, RC_OP, rc_file,
		     SB_RC_OP, sb_rcfile, ui_ver_switch (), directory, NULL );
	} /* if */

	else {
          if ( ui_is_auto ())
	    execlp ( MKLINKS, MKLINKS, SB_OP, sb_name, RC_OP, rc_file,
	             SB_RC_OP, sb_rcfile, ui_ver_switch (), AUTO_OP, directory,
		     NULL );
          else
	    execlp ( MKLINKS, MKLINKS, SB_OP, sb_name, RC_OP, rc_file,
		     SB_RC_OP, sb_rcfile, ui_ver_switch (), directory, NULL );
	} /* else */

        uquit ( ERROR, FALSE, "returned from execlp of: %s.\n", MKLINKS );
      } /* if */

      else
        ui_print ( VALWAYS, "mklinks %s %s %s %s %s %s %s %s %s %s\n",
		   copy ? COPY_OP : "", SB_OP, sb_name, RC_OP, rc_file,
		   SB_RC_OP, sb_rcfile, ui_ver_switch (),
		   ui_is_auto () ? AUTO_OP : "", directory );
      break;

    default :   wait (( int * ) 0 );
	        break;
  } /* switch */
}                                                      /* create local files */




print_usage ()

	/* This procedure prints the usages for mksb. */

{
  printf ( "USAGE:\n" );
  printf ( "%s [-back <backing-tree>] [-dir <sandbox-dir>] [-m <machine>[:<machine>]*]\n", progname );
  printf ( "     [-def] [sb_opts] [populate_opts] sandbox\n" );
  printf ( "       -back <backing-tree>: back sandbox with backing tree\n" );
  printf ( "       -dir <sandbox-dir>: directory to make sandbox in\n" );
  printf ( "       -m <machine>: colon separated list of machines to set up\n");
  printf ( "       -def: make new sandbox default sandbox\n" );
  printf ( "     sb_opts:\n" );
  printf ( "       -set <setname>, -rc <user rc file> -sb_rc <sb rc file>\n" );
  printf ( "     populate_opts:\n" );
  printf ( "       -tools <mode>: populate tools area\n" );
  printf ( "         mode: 'c' copy; 'l' link\n" );
  printf ( "       -obj <mode> <dirs>: populate object area\n" );
  printf ( "         dirs: colon separated list of directories to populate\n" );
  printf ( "       -src <mode> <dirs>: populate source area\n" );
  printf ( "     sandbox: name of sandbox to create\n\n" );
  printf ( "%s -list\n", progname );
  printf ( "%s -undo sandbox\n", progname );
  printf ( "%s -usage | -version\n", progname );
}                                                             /* print usage */
