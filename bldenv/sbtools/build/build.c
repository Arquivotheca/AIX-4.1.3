static char sccsid[] = "@(#)17  1.1  src/bldenv/sbtools/build/build.c, bldprocess, bos412, GOLDA411a 4/29/93 12:18:26";
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
**      This program is a front end for make.  It sets all the necessary
**	environment for working in a sandbox before invoking make.
**
**  Functions:
**    main (int, ** char) int
**      cmdline_syntax (** char) int
**        resize_makeargs (* int) int
**      check_sandbox (* struct) int
**        exists (* char) int
**        print_envvar (* struct) int
**      set_paths () int
**        path_relative_to (* char) * char
**      check_targets () int
**        srcdir (int, * char) int
**          isdir (* char) int
**      build_targets () int
**        makedir (* char) int
**        runcmdv (* char, * char, int, int, ** char) int
**        endcmd (int) int
**    print_usage () int
**    print_revision () int
 */

static char * rcsid =
 "$RCSfile: build.c,v $ $Revision: 1.11.2.2 $ $Date: 92/03/25 22:45:35 $";

#  include <ode/odedefs.h>
#  include <sys/param.h>
#  include <sys/stat.h>
#  include <sys/time.h>
#  include <sys/resource.h>
#  include <sys/wait.h>
#  include <sys/errno.h>
#  include <varargs.h>
#  include <a.out.h>
#  include <pwd.h>
#  include <grp.h>
#  include <ode/parse_rc_file.h>

    extern char 	* errmsg ();
           char 	* path_relative_to ();
    extern int 		  errno;

#ifdef	lint
/*VARARGS1*//*ARGSUSED*/
int quit(status) {};
/*VARARGS2*//*ARGSUSED*/
char *concat(buf, len) char *buf; int len; { return buf; };
#endif	lint

#  define  MAX_ARGS     7
#  define  CLEAN_OP	"-clean"
#  define  HERE_OP	"-here"
#  define  LINT_OP	"-lint"
#  define  DASH_OP	"-*"
#  define  EQUAL_OP	"*=*"
#  define  ECHO         "/bin/echo"                          /* echo command */

    UIINIT init [] = {                      /* initialize the user interface */
      { CLEAN_OP, 1, OVERWRITE, 0, 0, "" },
      { HERE_OP,  1, OVERWRITE, 0, 0, "" },
      { LINT_OP,  1, OVERWRITE, 0, 0, "" },
      { SB_OP,    1, OVERWRITE, 1, 1, ARGS_OP }, 
      { DASH_OP,  UNLIMITED, OVERWRITE, 0, 0, "" }, 
      { EQUAL_OP, UNLIMITED, OVERWRITE, 0, 0, "" }, 
      { ARGS_OP,  UNLIMITED, OVERWRITE, 0, 0, "" }
    };

    char 	* rcfile_source_base,
         	* rcfile_object_base,
         	* rcfile_object_owner,
         	* rcfile_object_cover,
         	* rcfile_build_makeflags,
         	* progname = "build",
         	* user,
         	  makecmd [ MAXPATHLEN ],
         	* relative_path,
         	* sourcedirpath,
         	* deftargetdirs [ 2 ],
               ** makeargs,
               ** targets,
               ** targetdirs;
    
    int 	  needabsolute,
        	  ntargets,
        	  nmakeargs;

    char 	* deftargets [] = {
      		    "build_all",
      		    NULL
    		  };
    
    struct 	rcfile rcfile;
    
main ( argc, argv )

    int 	  argc;
    char       ** argv;

{
    char 	* sb_rcfile;

  ui_init ( argc, argv, MAX_ARGS, init );
  cmdline_syntax ( &sb_rcfile );

  if ( parse_rc_file ( sb_rcfile, &rcfile ) != 0 )
    uquit ( ERROR, FALSE, "unable to parse sandbox rc: %s.\n", sb_rcfile );

  check_sandbox(&rcfile);
  set_paths ();
  check_targets ();
  build_targets ();
  return ( OK );
}



cmdline_syntax ( sb_rcfile )

	/* This procedure checks for relationships between the
           command line arguments.  It assumes the syntax is
           already correct.  Most of the functions it calls
           will use uquit to exit if there is an error. */

    char       ** sb_rcfile;

{
    char 	* sandbox,
         	* usr_rcfile,
         	* basedir = NULL;
    int   	  maxmakeargs = 0,
                  ct;                                        /* misc integer */

  sandbox = ui_arg_value ( SB_OP, 1, 1 );
  *sb_rcfile = ui_arg_value ( SB_RC_OP, 1, 1 );
  usr_rcfile = ui_arg_value ( RC_OP, 1, 1 );

  if ( current_sb ( &sandbox, &basedir, sb_rcfile, &usr_rcfile) == ERROR )
    uquit ( ERROR, FALSE, "unable to parse home sandbox rc: %s.\n", usr_rcfile);

  if (( user = getenv ( "USER" )) == NULL )
    uquit ( ERROR, FALSE, "USER not found in environment");
  
  ui_print ( VDETAIL, "environment variable USER: %s.\n", user );
  nmakeargs = 0;
  ui_print ( VDEBUG, "checking command line arguments\n" );

  for ( ct = 1; ct <= ui_entry_cnt ( DASH_OP ); ct++ ) {
    if ( nmakeargs == maxmakeargs )
      resize_makeargs ( &maxmakeargs );

    makeargs[nmakeargs++] = ui_entry_value ( DASH_OP, ct );
  } /* for */

  for ( ct = 1; ct <= ui_entry_cnt ( EQUAL_OP ); ct++ ) {
    if ( nmakeargs == maxmakeargs )
      resize_makeargs ( &maxmakeargs );

    makeargs[nmakeargs++] = ui_entry_value ( EQUAL_OP, ct );
  } /* for */

    /*
     * "no targets" implies "build_all" in the current directory when
     * not running as root.
     */

  if (( ntargets = ui_entry_cnt ( ARGS_OP )) == 0 ) {
    ntargets = 1;
    targets = deftargets;
    targetdirs = deftargetdirs;
  } /* if */

  else {
    if (( ntargets = ui_entries_to_argv ( ARGS_OP, &targets )) <= 0 )
      uquit ( ERROR, FALSE, "ui_entries_to_argv failed\n" );

    if (( targetdirs = (char **) malloc((unsigned) ntargets * sizeof(char *)))
		     == (char **) NULL )
      uquit ( ERROR, FALSE, "targetdirs malloc failed\n" );
  } /* else */
}                                                          /* cmdline syntax */



resize_makeargs  ( maxmakeargs )

    int   	* maxmakeargs;

{
  if ( *maxmakeargs == 0 ) {
    *maxmakeargs = 16;
    makeargs = (char **) malloc((unsigned) *maxmakeargs * sizeof(char *));
  }

  else {
    *maxmakeargs <<= 1;
    makeargs = (char **) realloc((char *) makeargs,
	                 (unsigned) *maxmakeargs * sizeof(char *));
  }

  if (makeargs == NULL)
    uquit ( ERROR, FALSE, "makeargs malloc failed");
}



check_sandbox (rcfile_p)

    struct 	rcfile   * rcfile_p;

{
    char 	* rcfile_source_cover;
    struct 	field    * field_p;
    struct 	arg_list * args_p;

  ui_print ( VDEBUG, "checking sandbox\n" );

  if (rc_file_field(rcfile_p, "source_base", &field_p) != 0)
    uquit ( ERROR, FALSE, "source_base not defined.\n" );

  args_p = field_p->args;

  if ( args_p->ntokens != 1 )
    uquit ( ERROR, FALSE, "improper source_base.\n" );

  rcfile_source_base = args_p->tokens[0];

  if ( *rcfile_source_base != '/' )
    uquit ( ERROR, FALSE, "source_base is not an absolute path.\n" );

  if ( exists(rcfile_source_base) < 0 )
    uquit ( ERROR, FALSE, "source_base %s not found.\n", rcfile_source_base );

  if ( rc_file_field ( rcfile_p, "source_owner", &field_p ) != 0 )
    rcfile_source_cover = NULL;
  
  else {
    args_p = field_p->args;

    if (args_p->ntokens != 1)
      uquit ( ERROR, FALSE, "improper source_owner.\n" );

    if (rc_file_field(rcfile_p, "source_cover", &field_p) != 0)
      uquit ( ERROR, FALSE, "source_cover not defined.\n" );

    args_p = field_p->args;

    if (args_p->ntokens != 1)
      uquit ( ERROR, FALSE, "improper source_cover.\n" );

    rcfile_source_cover = args_p->tokens[0];

    if ( *rcfile_source_cover != '/' )
      uquit ( ERROR, FALSE, "source_cover is not an absolute path.\n" );

    if ( access ( rcfile_source_cover, X_OK ) < 0 )
      uquit ( ERROR, FALSE, "source_cover %s not executable.\n",
			     rcfile_source_cover );
  } /* else */

  if (rc_file_field(rcfile_p, "object_base", &field_p) != 0)
    uquit ( ERROR, FALSE, "object_base not defined.\n" );

  args_p = field_p->args;

  if (args_p->ntokens != 1)
    uquit ( ERROR, FALSE, "improper object_base.\n" );

  rcfile_object_base = args_p->tokens[0];

  if (*rcfile_object_base != '/')
    uquit ( ERROR, FALSE, "object_base is not an absolute path.\n" );

  if (exists(rcfile_object_base) < 0)
    uquit ( ERROR, FALSE, "object_base %s not found.\n", rcfile_object_base );

  if (rc_file_field(rcfile_p, "object_owner", &field_p) != 0) {
    rcfile_object_owner = NULL;
    rcfile_object_cover = NULL;
  } /* if */

  else {
    args_p = field_p->args;

    if (args_p->ntokens != 1)
      uquit ( ERROR, FALSE, "improper object_owner.\n" );

    rcfile_object_owner = args_p->tokens[0];

    if (rc_file_field(rcfile_p, "object_cover", &field_p) != 0)
      uquit ( ERROR, FALSE, "object_cover not defined.\n" );

    args_p = field_p->args;

    if (args_p->ntokens != 1)
      uquit ( ERROR, FALSE, "improper object_cover.\n" );

    rcfile_object_cover = args_p->tokens[0];

    if (*rcfile_object_cover != '/')
      uquit ( ERROR, FALSE, "object_cover is not an absolute path.\n" );

    if (access(rcfile_object_cover, X_OK) < 0)
      uquit ( ERROR, FALSE, "object_cover %s not executable.\n",
	rcfile_object_cover);
  } /* else */

  if (rc_file_field(rcfile_p, "build_makeflags", &field_p) != 0)
    rcfile_build_makeflags = NULL;

  else {
    args_p = field_p->args;

    if (args_p->ntokens != 1)
      uquit ( ERROR, FALSE, "improper build_makeflags.\n" );

    rcfile_build_makeflags = args_p->tokens[0];
  } /* else */

  if ( ui_ver_level () >= VDETAIL ) {
    if ( rc_file_field(rcfile_p, "setenv", &field_p ) == 0 ) {
      ui_print ( VDETAIL, "The following environment variables were set:\n" );

      for (args_p = field_p->args; args_p != NULL; args_p = args_p->next)
	print_envvar (args_p);
    } /* if */
  } /* if */
}                                                           /* check sandbox */



int exists ( path )

    char 	* path;

{
    struct 	stat statb;

    return ( stat ( path, &statb ));
}                                                                  /* exists */



print_envvar (var)

    struct 	arg_list *var;

{
    char 	* val;

  if (var->ntokens != 1)
    return;

  if ((val = getenv(var->tokens[0])) == NULL)
    return;

  ui_print ( VCONT, "  %s=%s\n", var->tokens[0], val);
}



set_paths ()

{
    char 	  buf [MAXPATHLEN];
    char 	  path [MAXPATHLEN];
    char 	* ptr;

  ui_print ( VDEBUG, "setting paths\n" );

  if (( ptr = getenv( "MAKE" )) != NULL ) {
    (void) strcpy ( makecmd, ptr );
    ui_print ( VDETAIL, "reading MAKE from environment as: %s.\n", makecmd );
  } /* if */
 
  else {
    if (( ptr = getenv ("PATH")) == NULL )
      uquit ( ERROR, FALSE, "PATH not defined in environment.\n" );
    else
      ui_print ( VDETAIL, "reading PATH from environment as:\n   %s.\n", ptr );

    if ( searchp ( ptr, "make", path, exists ) == 0 )
      (void) strcpy ( makecmd, path );
    else
      (void) strcpy ( makecmd, "make" );
  } /* else */

  if (( sourcedirpath = getenv ( "SOURCEDIR" )) == NULL )
    uquit ( ERROR, FALSE, "SOURCEDIR not defined in environment.\n" );

  if ( *sourcedirpath == NUL )
    sourcedirpath = rcfile_source_base;
  else {
    ui_print ( VDEBUG, "backing source: %s\n", sourcedirpath );
    concat( buf, MAXPATHLEN, rcfile_source_base, ":", sourcedirpath, NULL );
    
    if (( sourcedirpath = salloc (buf)) == NULL )
      uquit ( ERROR, FALSE, "salloc sourcedirpath failed.\n" );
  } /* else */

  ui_print ( VDEBUG, "usr source: %s\n", rcfile_source_base );
  relative_path = path_relative_to (rcfile_source_base);

  if ( relative_path != NULL )
    ui_print ( VNORMAL, "relative path: .%s.\n", relative_path );
}                                                               /* set paths */



char * path_relative_to ( base_dir )

	/*
 	 * If we are within the directory subtree of base_dir, return the path
 	 * from there to the current directory.  Otherwise, return NULL.
 	 */

    char 	* base_dir;

{
    static char   curdir [MAXPATHLEN];
    static int    curdir_len = 0;
    char 	  basedir [MAXPATHLEN],
         	* path;
    int 	  len;

  if (curdir_len == 0) {
    if ( getwd ( curdir ) == NULL )
      uquit ( ERROR, FALSE, "getwd: %s.\n", curdir );

    curdir_len = strlen ( curdir );

    if ( curdir_len == 0 || curdir[0] != '/' )
      uquit ( ERROR, FALSE, "getwd returned bad current directory: %s.",
			     curdir );
  }  /* if */

  if ( chdir ( base_dir ) < 0 )
    uquit ( ERROR, FALSE, "chdir %s.\n", base_dir);

  if ( getwd ( basedir ) == NULL )
    uquit ( ERROR, FALSE, "getwd %s: %s.\n", base_dir, basedir);

  if ( chdir ( curdir ) < 0 )
    uquit ( ERROR, FALSE, "chdir %s.\n", curdir);

  len = strlen ( basedir );

  if ( len == 0 || basedir[0] != '/' )
    uquit ( ERROR, FALSE, "getwd returned bad base directory: %s.", basedir );

  if ( curdir_len < len )
    return( NULL );

  if ( bcmp ( basedir, curdir, len ) != 0 )
    return( NULL );

  if ( curdir[len] != NUL && curdir[len] != '/' )
    return( NULL );

  if (( path = salloc ( curdir + len )) == NULL )
    uquit ( ERROR, FALSE, "salloc relative path.\n" );

  return ( path );
}                                                        /* path relative to */



check_targets ()

{
    char 	  dirbuf [MAXPATHLEN],
		  filbuf [MAXPATHLEN],
         	  testtarget [MAXPATHLEN],
         	  targetdir [MAXPATHLEN],
         	* fulltarget,
         	* ptr,
		* p;
    int 	  absolute,
        	  i;

  for ( i = 0; i < ntargets; i++ ) {
    fulltarget = targets[i];
    ui_print ( VDETAIL, "target: %s\n", fulltarget );

	/*
	 * remember whether or not the target is absolute
	 */
    absolute = (*fulltarget == '/');

    if ( ! absolute ) {
      if (relative_path == NULL)
	uquit ( ERROR, FALSE, "not in sandbox and target %s is not absolute.\n",
		  	       fulltarget );
    }  /* if */

    if ( ui_is_set ( HERE_OP ) || strcmp(fulltarget, "build_all") == 0) {
      if ( streq ( fulltarget, "build_all" ))
	  targetdir[0] = NUL;
      
      else {
	(void) strcpy ( targetdir, fulltarget );
	if ( targetdir[0] == NUL ) {
	  targetdir[0] = '/';
	  targetdir[1] = NUL;
	}  /* if */
      }  /* else */

      filbuf[0] = NUL;
    }  /* if  */
    
    else {
      path ( fulltarget, dirbuf, filbuf );

      if ( filbuf[0] == NUL || ( filbuf[0] == '.' && filbuf[1] == NUL ))
	uquit ( ERROR, FALSE, "invalid null target.\n" );

      if ( srcdir ( absolute, fulltarget ))
	(void) strcpy ( targetdir, fulltarget );

      else {
	ptr = fulltarget;
	p = testtarget;

	while ( *p = *ptr++ )
	    p++;

	ptr = NULL;

	while ( p >= testtarget && *p != '/' ) {
	    if ( *p == '.' )
	      ptr = p;

	    p--;
	}  /* while */

	if (ptr != NULL && ptr != p + 1)
	  *ptr = NUL;
	else
	  ptr = NULL;

	if ( ptr != NULL && srcdir ( absolute, testtarget ))
	  (void) strcpy ( targetdir, testtarget );
	else
	  (void) strcpy ( targetdir, dirbuf );
      }  /* else */
    }  /* else */

    if (targetdir[0] == '.' && targetdir[1] == NUL)
      targetdir[0] = NUL;

    if ( ! srcdir ( absolute, targetdir ))
      uquit ( ERROR, FALSE, "targetdir %s is not a directory", targetdir);

    targets[i] = salloc(filbuf);

      	/*
       	 * If we are changing to some directory for the build (e.g. the
         * original target contained a slash), figure out how to
         * indicate this to the user.  If the target is relative to the
         * current directory, just use the relative directory name.  If
         * the target is absolute preceed this with the string "... ."
         * to suggest that the directory is relative to the absolute
         * base of the source tree.
         */
    if ( absolute || *targetdir != NUL )
      targetdirs[i] = salloc (targetdir);
    else
      targetdirs[i] = NULL;
  }  /* for */
}                                                           /* check targets */



isdir ( buf )

    char 	* buf;

{
    struct stat statb;

  if ( stat ( buf, &statb ) == 0 && ( statb.st_mode & S_IFMT ) == S_IFDIR )
    return ( OK );

  return( ERROR );
}                                                                   /* isdir */




srcdir ( absolute, testtarget )
    int 	  absolute;
    char 	* testtarget;
{
    char 	  buf [MAXPATHLEN];

  if ( *testtarget == NUL )
    return ( TRUE );

  if ( absolute ) {
    if ( searchp ( sourcedirpath, testtarget + 1, buf, isdir ) == 0 )
      return ( TRUE );
  }  /* if */

  else if ( isdir ( testtarget ) == 0 )
    return ( TRUE );

  return ( FALSE );
}                                                                  /* srcdir */



build_targets ()

{
    char 	  prompt [MAXPATHLEN],
         	  dirbuf [MAXPATHLEN],
         	  build_target [MAXPATHLEN],
         	  clean_target [MAXPATHLEN],
         	  lint_target [MAXPATHLEN],
    		* av[64],
         	* cd,
		* target,
    		* dirptr,
    		* ptr,
    		* cmd;
    int 	  status,
    		  pid,
    		  i = 0,
		  vi, j,
    		  firsti;

  dirptr = concat ( dirbuf, sizeof ( dirbuf ), rcfile_source_base, NULL );

  if ( ui_ver_level () == VDEBUG )
    av[i++] = "echo";

  if ( rcfile_object_owner == NULL ) {
    if (( ptr = rindex ( makecmd, '/' )) != NULL )
      ptr++;
    else
      ptr = makecmd;

    av[i++] = ptr;
    cmd = makecmd;
  }  /* if */
  
  else {
    ui_print ( VDETAIL, "setenv AUTHCOVER_USER %s.\n", rcfile_object_owner );

    if ( ! ui_is_info ()) {
      if (setenv( "AUTHCOVER_USER", rcfile_object_owner, 1) < 0)
	uquit ( ERROR, FALSE, "AUTHCOVER_USER setenv failed");
    }  /* if */

    ui_print ( VDETAIL, "setenv AUTHCOVER_TESTDIR %s.\n", rcfile_object_base );

    if ( ! ui_is_info ()) {
      if ( setenv ( "AUTHCOVER_TESTDIR", rcfile_object_base, 1 ) < 0 )
	uquit ( ERROR, FALSE, "AUTHCOVER_TESTDIR setenv failed.\n" );
    }  /* if */

    av[i++] = "authcover";
    av[i++] = makecmd;
    cmd = rcfile_object_cover;
  }  /* else */

  if ( rcfile_build_makeflags != NULL )
    av[i++] = rcfile_build_makeflags;

  for ( j = 0; j < nmakeargs; j++ )
    av[i++] = makeargs[j];

  firsti = i;

    /*
     * For each target:
     *   Run a "clean" operation first, if requested
     *   Run a "build" operation
     *   Run a "lint" operation, if requested.
     */

  for ( j = 0; j < ntargets; j++ ) {
    i = firsti;
    cd = targetdirs[j];
    target = targets[j];

    if ( target == NULL )
      continue;

    ui_print ( VDETAIL, "building: %s.\n",
			 (*target != NUL) ? target : "build_all" );

    if (cd != NULL) {
      if (*cd != NUL && *cd != '/')
	ui_print ( VNORMAL, "cd %s\n", cd );

      else {
	ui_print ( VNORMAL, "cd ... .%s.\n", cd );
	(void) concat( dirptr, dirbuf + sizeof(dirbuf) - dirptr, cd, NULL );
	cd = dirbuf;
      }  /* else */

      if ( exists (cd) < 0 ) {
	(void) concat(prompt, sizeof(prompt),
		      "Directory ", cd, " not found - create", NULL );
	if ( ! getbool ( prompt, TRUE )) {
	  targets[j] = NULL;
	  continue;
	}  /* if */

	if ( ! ui_is_info ())
	  makedir(cd);
      }  /* if */
    }  /* if */

    if ( *target == NUL ) {
      if ( ui_is_set ( CLEAN_OP ))
	av[i++] = "clean_all";

      av[i++] = "build_all";

      if ( ui_is_set ( LINT_OP ))
	av[i++] = "lint_all";
    }   /* if */
    
    else {
      if ( ui_is_set ( CLEAN_OP )) {
	(void) concat ( clean_target, sizeof(clean_target),
		        target, NULL );
	av[i++] = clean_target;
      }  /* if */

      (void) concat ( build_target, sizeof(build_target),
		      target, NULL);
      av[i++] = build_target;

      if ( ui_is_set ( LINT_OP )) {
	(void) concat ( lint_target, sizeof(lint_target),
			target, NULL);
	av[i++] = lint_target;
      }  /* if */
    }  /* else */

    av[i++] = NULL;
    (void) fflush(stdout);
    (void) fflush(stderr);
		    
    if ( ui_ver_level () == VDEBUG ) {
      (void) runv( ECHO, av );
      continue;
    }  /* if */

    if ( ui_ver_level () >= VDETAIL ) {
      ui_print ( VDETAIL, "string passed to %s:\n   ", cmd );

      for ( vi = 0; vi < (i-1); vi++ )
	ui_print ( VALWAYS, "%s ", av[vi] );	
      
      ui_print ( VALWAYS, "\n" );
    }  /* if */

    if ( ! ui_is_info ()) {              /* make runcmd conditional on -info */
      if (( pid = runcmdv ( cmd, cd, av )) == ERROR )
	exit ( ERROR );
      if (( status = endcmd ( pid )) != 0 )
	exit ( status );
    }  /* ifi */
  }  /* for */
}                                                           /* build targets */



makedir ( dir )

    char 	* dir;

{
    char 	 dbuf [MAXPATHLEN];
    char 	* ptr;

  ptr = concat ( dbuf, sizeof ( dbuf ), dir, NULL );

  if (*(ptr-1) != '/')
    *ptr++ = '/';

  *ptr++ = '.';
  *ptr++ = NUL;

  if ( makepath ( dbuf, NULL, TRUE, TRUE ) != 0 )
    uquit ( ERROR, FALSE, "makepath %s failed.\n", dbuf );
}                                                                 /* makedir */



print_usage ()

        /* This procedure prints the usages for build. */

{
  printf ( "USAGE:\n" );
  printf ( "%s [-here -clean -lint] [-*] [*=*] [sb_opts] target(s)\n",
		    progname );
  printf ( "      -here : targets are directories to work from\n" );
  printf ( "      -clean : old format to remove and rebuild target\n" );
  printf ( "      -lint : old format to run lint\n" );
  printf ( "      -* : any additional dash arg is passed to make\n" );
  printf ( "      *=* : any equation of this form is passed to make\n" );
  printf ( "      sb_opts:\n" );
  printf ( "        -sb <sandbox>, -rc <user rc file>, -sb_rc <sb rc file>\n" );
  printf ( "      target(s) : list of items to build\n" );
  printf ( "%s -usage | -rev\n", progname );
}                                                             /* print usage */
