static char sccsid[] = "@(#)77  1.1  src/bldenv/sbtools/resb/resb.c, bldprocess, bos412, GOLDA411a 4/29/93 12:27:05";
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
**                                May 1990                                   **
*******************************************************************************
**
** Description:
**	This program re-targets a sandbox from one backing tree to another.
**	It also updates the rc files.
**
**  Functions:
**    main (int, ** char) int
**      cmdline_syntax (** char, ** char, ** char, ** char, ** char) int
**        legal_backing_tree (** char, * char, * char) int
**      move_rc_files (* char, * char, * char) int
**      check_makeconf (* char, * char) int
**      change_backing_link (* char, * char, * char, * char, * char) int
**        unmove_rc_files (* char, * char, * char) int
**        complete_recovery (* char, * char, * char, * char) int
**          unmove_rc_files
**      make_rc_files (* char, * char, * char, * char) int
**        create_sb_rc_files (* char, * char, * char, * char, * char,
**			      * char, * char, * char) int
**          complete_recovery
**    print_usage () int
 */

static char * rcsid =
 "$RCSfile: resb.c,v $ $Revision: 1.12.2.2 $ $Date: 92/03/25 22:47:06 $";

#  include <ode/odedefs.h>
#  include <sys/param.h>
#  include <sys/stat.h>
#  include <ode/parse_rc_file.h>

#  define  MAX_ARGS	3
#  define  BACKUP_TYPE 	".BAK"


    UIINIT init [] = {                      /* initialize the user interface */
      { SB_OP,     1, OVERWRITE, 1, 1, ARGS_OP },
      { RCONLY_OP, 1, OVERWRITE, 0, 0, "" },
      { ARGS_OP,   1, OVERWRITE, 0, 1, "" }
    };

    char 	* build_base_dir ();
    char        * progname = "resb";                      /* name of program */

main ( argc, argv )

	/* This function checks the command line arguments and makes
	   sure they are syntactically correct.  This is done using
           the library function ui_init.  If this is correct, the
	   dependencies are checked.  Errors lead to usage messages.
	   If all is okay, the primary procedure is called. */

    int           argc;              /* the number of command line arugments */
    char       ** argv;                        /* strings with each argument */

{
    char        * sb,                                        /* sandbox name */
                * sbbase = NULL,                          /* path to sandbox */
                * sb_rcfile,                         /* sandbox rc file name */
                * rc_file,                                   /* user rc file */
                * targdir,                       /* directory to retarget to */
                  linkdir [ PATH_LEN ],                 /* sandbox link file */
                  link [ PATH_LEN ];               /* holds old path of link */

  ui_init ( argc, argv, MAX_ARGS, init );
  cmdline_syntax ( &sb, &sbbase, &sb_rcfile, &rc_file, &targdir );
  move_rc_files ( sb, sbbase, LOCAL_RC );
  move_rc_files ( sb, sbbase, SHARED_RC );
  check_makeconf ( sb, sbbase );

  if ( ! ui_is_set ( RCONLY_OP ))
    change_backing_link ( sb, sbbase, targdir, linkdir, link );

  make_rc_files ( sb, sbbase, linkdir, link );
  return ( OK );
}                                                                    /* main */



cmdline_syntax ( sb, sbbase, sb_rcfile, rc_file, targdir )

	/* This procedure makes sure the options which are chosen
	   work together legally. */

    char       ** sb,                                        /* sandbox name */
               ** sbbase,                                 /* path to sandbox */
               ** sb_rcfile,                         /* sandbox rc file name */
               ** rc_file,                                   /* user rc file */
               ** targdir;                       /* directory to retarget to */

{
    char      * default_base;             /* base of default build directory */

  *sb = ui_arg_value ( SB_OP, 1, 1 );
  *sb_rcfile = ui_arg_value ( SB_RC_OP, 1, 1 );
  *rc_file = ui_arg_value ( RC_OP, 1, 1 );
  *targdir = ui_entry_value ( ARGS_OP, 1 );

  if ( current_sb ( sb, sbbase, sb_rcfile, rc_file ) == ERROR )
    uquit ( ERROR, TRUE, "\tcould not get current sandbox information.\n" );
  
  if ( default_build ( &default_base, NULL, NULL, *sb_rcfile ) == ERROR )
    uquit ( ERROR, FALSE,
	    "\tcannot continue without default directory info.\n" );

  ui_print ( VDETAIL, "Project's base directory is: %s.\n", default_base );

  if ( ui_is_set ( RCONLY_OP )) {
    if ( ui_is_set ( ARGS_OP ))
      uquit (  ERROR, TRUE,
	      "\tCannot set option %s and give backing tree.\n", RCONLY_OP );
  } /* if */

  else {
    if ( ! ui_is_set ( ARGS_OP ))
      uquit (  ERROR, TRUE,
              "\tMust include name of backing tree to retarget to.\n" );

    legal_backing_tree ( targdir, default_base, *sb_rcfile );
  } /* else */
}                                                          /* cmdline syntax */



BOOLEAN	legal_backing_tree ( dir, default_base, sb_rcfile )

	/* This function verifies the presence of a legal backing
	   tree.  If it finds one, after looking in both the default
	   base and in the build_list, it creates the space for the
	   name and points dir to it. */

    char       ** dir,                /* user entered backing tree directory */
                * default_base,                   /* default build directory */
                * sb_rcfile;                            /* sandbox's rc file */

{
    char        newtree [ PATH_LEN ],            /* path to new backing tree */
              * newbase;                      /* points to base of new build */
    struct      stat statbuf;                         /* buffer used by stat */
    struct      rcfile  rc_contents;           /* holds contents of rc files */
    BOOLEAN     justname = FALSE;                            /* misc boolean */

  if ( strchr ( *dir, SLASH ) == NULL ) {                /* no slash in name */
    concat ( newtree, PATH_LEN, default_base, "/", *dir, NULL );
    justname = TRUE;
  } /* if */
  
  else
    strcpy ( newtree, *dir );

  if (( stat ( newtree, &statbuf ) == ERROR ) ||
      (( statbuf.st_mode & S_IFDIR ) == 0 )) {
    if ( justname ) {
      bzero ( &rc_contents, sizeof ( rc_contents ));

      if ( parse_rc_file ( sb_rcfile, &rc_contents ))
        uquit (  ERROR, FALSE,
	 "\tno build %s in default base; no alternative base.\n", *dir );

      if (( newbase = build_base_dir ( *dir, &rc_contents,
			 ui_ver_level () >= VDIAG, FALSE )) == NULL )
        uquit (  ERROR, FALSE,
	 	 "\tno backing tree %s in known build locations.\n", *dir );
      else {
	concat ( newtree, PATH_LEN, newbase, "/", *dir, NULL );

	if (( stat ( newtree, &statbuf ) == ERROR ) ||
            (( statbuf.st_mode & S_IFDIR ) == 0 ))
        uquit ( ERROR, FALSE, "\t%s is not a directory.\n", newtree );
      } /* else */
    } /* if */

    else
      uquit ( ERROR, FALSE, "\t%s is not a directory.\n", newtree );
  } /* if */
 
  if (( *dir = salloc ( newtree )) == NULL )
    uquit ( ERROR, FALSE, "\tunable to salloc space for %s.\n", *dir );
}                                                      /* legal backing tree */



move_rc_files ( sb, sbbase, rcfile )

	/* This procedure checks the rc files for any changes that
	   should be saved */
       /* For now, this will merely save off the existing rc_files.  */

    char        * sb,                                        /* sandbox name */
                * sbbase,                                 /* path to sandbox */
                * rcfile;                        /* path and name to rc file */

{
    char        path1 [ PATH_LEN ];                     /* The existing file */
    char        path2 [ PATH_LEN ];                 /* The new, backup, file */

  concat ( path1, PATH_LEN, sbbase, "/", sb, "/", rcfile, NULL );
  concat ( path2, PATH_LEN, sbbase, "/", sb, "/", rcfile, BACKUP_TYPE, NULL);

  ui_print ( VDEBUG, "Renaming %s to %s\n", path1, path2 );

  if ( ! ui_is_info ()) {
    if ( rename ( path1, path2 ) != OK )
      uquit (  ERROR, FALSE, "\tUnable to rename '%s' to '%s'\n", path1, path2);

    ui_print ( VDETAIL, "%s has been renamed to:\n    %s.\n", path1, path2 );
  } /* if */

  else
    ui_print ( VALWAYS, "Would rename %s to:\n    %s.\n", path1, path2 );
}                                                           /* move rc files */



check_makeconf ( sb, sbbase )

	/* This procedure checks to see if the file Makeconf exists.
	   If it does not, it links it through the "link" directory. */

    char        * sb,                                        /* sandbox name */
                * sbbase;                                 /* path to sandbox */

{
    char        mkconf [ PATH_LEN ],                          /* misc string */
                sb_path [ PATH_LEN ];                 /* path and name to sb */

  concat ( sb_path, PATH_LEN, sbbase, "/", sb, "/", MKCONF_LINK, NULL );

  if ( access ( sb_path, F_OK ) == ERROR ) {
    concat ( mkconf, PATH_LEN, "../", LINK_DIR, "/", MKCONF_LINK, NULL );

    if ( ! ui_is_info ()) {
      if ( symlink ( mkconf, sb_path ) == ERROR )
	ui_print ( VFATAL,
		"could not create link, %s, to: %s.\n", sb_path, mkconf);
    } /* if */

    else
      ui_print ( VALWAYS, "Would link %s to backing tree.\n", MKCONF_LINK );

    ui_print ( VDETAIL, "Successfully linked %s to backing tree.\n",
			 MKCONF_LINK );
  } /* if */
}                                                          /* check makeconf */



change_backing_link ( sb, sbbase, targdir, linkdir, link )

	/* This procedure changes the backing link. */

    char        * sb,                                        /* sandbox name */
                * sbbase,                                 /* path to sandbox */
                * targdir,                       /* directory to retarget to */
                * linkdir,                  /* path to existing backing tree */
                * link;                            /* holds path to old link */

{
    int         ln_ct;             /* number of chars returned from readlink */

  concat ( linkdir, PATH_LEN, sbbase, "/", sb, "/", LINK_DIR, NULL );

  if (( ln_ct = readlink ( linkdir, link, PATH_LEN )) == ERROR ) {
    *link = NUL;
    ui_print ( VWARN, "old link, %s, could not be read.\n", linkdir );
  } /* if */

  else
    link [ ln_ct ] = NUL;

  ui_print ( VDEBUG, "Removing %s-> %s.\n", linkdir, link );

  if ( ! ui_is_info ()) {
    if ( unlink ( linkdir )) {
      unmove_rc_files ( sb, sbbase, LOCAL_RC );
      unmove_rc_files ( sb, sbbase, SHARED_RC );
      uquit (  ERROR, FALSE,
		   "\tunlink failed on current backing link: %s.\n", linkdir );
    } /* if */

    ui_print ( VDEBUG, "arg is %s\n  symlinking %s to %s.", targdir, targdir,
		  	linkdir );
  							    /* Make new link */
    if ( symlink ( targdir, linkdir ) == ERROR ) {
      complete_recovery ( sb, sbbase, link, linkdir );
      uquit (  ERROR, FALSE,
              "\tfailed to link to backing tree\n  %s->%s.\n%s\n",
	       targdir, linkdir,
	      "WARNING: you must create this link manually to continue." );
    } /* if */
  } /* if */

  else
    ui_print ( VALWAYS, "Would link %s to %s.\n", linkdir, targdir );

  ui_print ( VDETAIL, "Linked %s to %s.\n", linkdir, targdir );
}                                                     /* change backing link */



unmove_rc_files ( sb, sbbase, rcfile )

	/* This procedure returns an rc file to its original position.
	   It is used for recovery. */

    char        * sb,                                        /* sandbox name */
                * sbbase,                                 /* path to sandbox */
                * rcfile;                        /* path and name to rc file */

{
    char        path1 [ PATH_LEN ];                     /* The existing file */
    char        path2 [ PATH_LEN ];                 /* The new, backup, file */

  concat ( path1, PATH_LEN, sbbase, "/", sb, "/", rcfile, NULL );
  concat ( path2, PATH_LEN, sbbase, "/", sb, "/", rcfile, BACKUP_TYPE, NULL);
  ui_print ( VNORMAL, "Returning %s to its original name: \n  %s.\n",
		       path2, path1 );

  if ( rename ( path2, path1 ) != OK )
    ui_print ( VWARN, "Unable to rename %s to %s.\n", path2, path1 );
}                                                         /* unmove rc files */



complete_recovery ( sb, sbbase, link, linkdir )

	/* This procedure attempts to completely restore the sandbox
	   to its beginning state.  It is used when the resb has
	   failed. */

    char        * sb,                                        /* sandbox name */
                * sbbase,                                 /* path to sandbox */
                * link,                            /* original value of link */
                * linkdir;                                   /* sandbox link */

{
  ui_print ( VFATAL, "returning sandbox to its original state.\n" );

  if ( *link != NUL ) {
    if ( unlink ( linkdir ) == ERROR || symlink ( link, linkdir ) == ERROR )
      ui_print ( VWARN, "could not restore %s to %s.\n", linkdir, link );
  } /* if */

  unmove_rc_files ( sb, sbbase, LOCAL_RC );
  unmove_rc_files ( sb, sbbase, SHARED_RC );
}                                                       /* complete recovery */



make_rc_files ( sb, sbbase, linkdir, link )

     /* This function creates two new rc files, converting the
	template when necessary and copying the lines into the
	new template and local rc files.  */

    char        * sb,                                        /* sandbox name */
                * sbbase,                                 /* path to sandbox */
                * linkdir,                              /* sandbox link file */
                * link;                              /* contents of old link */

{

    char	backing [ PATH_LEN ];
    char        addition [ PATH_LEN ];           /* current sb path and name */
  
  concat ( backing, PATH_LEN, sbbase, "/", sb, "/", LINK_DIR, NULL );
  concat ( addition, PATH_LEN, sbbase, "/", sb, NULL );
  ui_print ( VDEBUG, "Using backing tree of %s\n", backing );
  
  if ( ! ui_is_info ()) {
    create_sb_rc_files ( backing, LOCAL_RC, LOCAL_T_RC, addition, linkdir,
			 link, sb, sbbase );
    create_sb_rc_files ( backing, SHARED_RC, SHARED_T_RC, addition, linkdir,
			 link, sb, sbbase );
  } /* if */

  else
    ui_print ( VALWAYS, "Would update rc files, %s and %s.\n",
			 LOCAL_RC, SHARED_RC );

  ui_print ( VDETAIL, "RC files, %s and %s, updated.\n", LOCAL_RC, SHARED_RC );
}



BOOLEAN create_sb_rc_files ( backing, rcfile, trcfile, addition, linkdir,
 			     link, sb, sbbase )

	/* This function creates two new rc files, converting the
	   template when necessary and copying the lines into the
	   new template and local rc files.  If it fails, it
	   returns FALSE. */

    char        * backing,                           /* path to backing tree */
                * rcfile,                        /* path and name to rc file */
                * trcfile,              /* path and name to template rc file */
                * addition,                          /* string add to string */
                * linkdir,                              /* sandbox link file */
                * link,                              /* contents of old link */
                * sb,                                        /* sandbox name */
                * sbbase;                                 /* path to sandbox */

{
    FILE      * org,                                        /* original file */
              * tcopy,                                      /* template file */
              * rccopy;                                           /* rc file */
    char        path [ PATH_LEN ],                            /* misc string */
                line [ PATH_LEN ],                            /* misc string */
                line_copy [ PATH_LEN ],                       /* misc string */
              * key,                                          /* misc string */
              * target;                                       /* misc string */
    char      * line_ptr;

    concat ( path, PATH_LEN, backing, "/", trcfile, NULL );
    
    if (( org = fopen ( path, READ )) == NULL ) {
      complete_recovery ( sb, sbbase, link, linkdir );
      uquit (  ERROR, FALSE,
              "\tcannot open backing tree rc file:\n\t%s.\n%s%s\n", path,
	      "Problem was most likely in ", backing );
    } /* if */

    concat ( path, PATH_LEN, sbbase, "/", sb, "/", trcfile, NULL );

    if (( tcopy = fopen ( path, WRITE )) == NULL ) {
      complete_recovery ( sb, sbbase, link, linkdir );
      uquit (  ERROR, FALSE,
              "\tcannot open local template rc file:\n\t%s.\n%s%s\n", path,
	      "Problem was most likely in ", backing );
    } /* if */

    concat ( path, PATH_LEN, sbbase, "/", sb, "/", rcfile, NULL );
    
    if (( rccopy = fopen ( path, WRITE )) == NULL ) {
      complete_recovery ( sb, sbbase, link, linkdir );
      uquit (  ERROR, FALSE,
      	      "\tcannot create local rc file for writing:\n\t%s.\n", path );
    } /* if */
    
    while ( ( line_ptr = fgets ( line, PATH_LEN, org )) != NULL ) {
      fputs ( line, tcopy );                  /* template gets an exact copy */
      strcpy ( line_copy, line );
      line_ptr = line_copy;
      key = nxtarg ( &line_ptr, WHITESPACE );

      if ( ! streq ( key, REPLACE )) {
	fputs ( line, rccopy );
	continue;
      } /* if */

      target = nxtarg ( &line_ptr, WHITESPACE );

      if ( streq ( target, SB_BASE ))     /* if sandbox_base put in addition */
	fprintf ( rccopy, "%s %s %s\n", key, target, addition );
      else
	fputs ( line, rccopy );
    } /* while */
    
    fclose ( org );
    fclose ( tcopy );
    fclose ( rccopy );
}                                                      /* create sb rc files */

print_usage ()

{
  printf ( "USAGE:\n" );
  printf ( "%s -rconly | new-backing-tree [sb_opts]\n", progname );
  printf ( "     -rconly: update current rc files; do not relink sandbox.\n" );
  printf ( "     new-backing-tree: the sandbox or build to be backed by.\n" );
  printf ( "     sb_opts:\n" );
  printf ( "       [-sb <sandbox>, -rc <rc_file>, -sb_rc <sb_rcfile>]\n" );
}                                                             /* print usage */
