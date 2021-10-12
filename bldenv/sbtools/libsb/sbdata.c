static char sccsid[] = "@(#)56  1.1  src/bldenv/sbtools/libsb/sbdata.c, bldprocess, bos412, GOLDA411a 4/29/93 12:23:35";
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
**  Description:
**	These are functions for library libsb.a which get the current
**	sandbox, set, basedir, etc.  These are almost identical to the former
**	get_current routines but use pointers and create space.
**
**  Functions:
**    current_sb (** char, ** char, ** char, ** char) int
**      get_default_usr_rcfile (** char, int) BOOLEAN
**      get_default_sb (** char, * char) BOOLEAN
**      currentsb (* char, * char) BOOLEAN
**      get_basedir (* char, ** char, * char) BOOLEAN
**      match_sb_basedir (* char, * char, * char) BOOLEAN
**      current_sb_rcfile (* char, * char, ** char, * char) BOOLEAN
**        is_project (* char, * char, ** char) BOOLEAN
**        get_default_sb_rcfile (* char, * char, ** char, * char) BOOLEAN
**    current_set (** char, ** char, ** char, ** char) int
**      get_default_usr_rcfile
**      is_existing_set (* char, * char, ** char, ** char) BOOLEAN
**      current_sb
**
**  Functions Available for Use:
**    current_sb (** char, ** char, ** char, ** char) int
**    get_default_usr_rcfile (** char, int) BOOLEAN
**    current_set (** char, ** char, ** char, ** char) int
 */

#  include <sys/file.h>
#  include <ode/odedefs.h>

    static  BOOLEAN get_default_sb ();
    static  BOOLEAN currentsb ();
    static  BOOLEAN get_basedir ();
    static  BOOLEAN current_sb_rcfile ();
    static  BOOLEAN is_project ();
    static  BOOLEAN get_default_sb_rcfile ();
    static  BOOLEAN match_sb_basedir ();
    static  BOOLEAN is_existing_set ();


current_sb ( sb, basedir, sb_rcfile, usr_rcfile )

	/* This function checks to see what type of search is needed.
	   It then calls the appropriate functions to get the
	   information.  If the information is found, it returns
	   0, else -1. */

    char     ** sb,                                       /* name of sandbox */
             ** basedir,                           /* name of base directory */
             ** sb_rcfile,                       /* name of sandbox rc file. */
             ** usr_rcfile;                     /* name and path to rc file. */

{
    char        ab_path [ PATH_LEN ];                         /* misc string */

  ui_set_progname ( "current_sb" );

  if ( *usr_rcfile == NULL ) {
    if  ( ! get_default_usr_rcfile ( usr_rcfile, TRUE )) {
      ui_restore_progname ();
      return ( ERROR );
    } /* if */
  } /* if */

  else if ( **usr_rcfile != SLASH ) {
    if ( abspath ( *usr_rcfile, ab_path ) == ERROR ) {
      ui_print ( VFATAL, "ERROR: could not get cwd for rcfile %s\n",
			  *usr_rcfile );
      ui_restore_progname ();
      return ( ERROR );
    } /* if */

    *usr_rcfile = salloc ( ab_path );
  } /* else if */

  if ( *basedir == NULL ) {
    if ( *sb == NULL ) {                            /* get sb name if needed */
      if ( ! get_default_sb ( sb, *usr_rcfile )) {
	ui_restore_progname ();
	return ( ERROR );
      } /* if */
    } /* if */

    if ( ! currentsb ( *sb, *usr_rcfile )) {              /* is sb name okay */
      ui_restore_progname ();
      return ( ERROR );
    } /* if */

    if ( ! get_basedir ( *sb, basedir, *usr_rcfile )) {       /* now get dir */
      ui_restore_progname ();
      return ( ERROR );
    } /* if */
  } /* if */

  else if ( *sb != NULL ) {                            /* dir and sb entered */
    if ( ! match_sb_basedir ( *sb, *basedir, *usr_rcfile )) {
      ui_restore_progname ();
      return ( ERROR );
    } /* if */
  } /* else if */

  else {                                /* dir entered, no sb - not possible */
    ui_print ( VDIAG,
     "base dir: %s, entered but no sb name; illegal combination.\n", *basedir );
    ui_restore_progname ();
    return ( ERROR );
  } /* else */

  if ( ! current_sb_rcfile ( *sb, *basedir, sb_rcfile, *usr_rcfile )) {
    ui_restore_progname ();
    return ( ERROR );
  } /* if */

  ui_restore_progname ();
  return ( OK );
}                                                              /* current sb */



BOOLEAN	get_default_usr_rcfile ( usr_rcfile, report )

	/* This function gives the usr_rcfile the default value.
	   If it fails to find the file, it returns FALSE,
	   else TRUE. */

    char     ** usr_rcfile;                     /* name and path to rc file. */
    BOOLEAN     report;                            /* print messages if true */

{
    char      * env_input,                       /* holds values from getenv */
                trcfile [ PATH_LEN ];                  /* tmp rc file holder */

  if (( env_input = getenv ( "HOME" )) == NULL ) {
    if ( report )
      ui_print ( VWARN, "HOME not set in enviroment.\n" );
    /* if */
  } /* if */

  concat ( trcfile, PATH_LEN, env_input, "/", SANDBOXRC, NULL );
  *usr_rcfile = salloc ( trcfile );
  if ( env_input == NULL ) 
    return ( FALSE );

  if ( access ( *usr_rcfile, R_OK ) == ERROR ) {
    if ( report )
      ui_print ( VWARN, "could not access rc file, %s, for reading.\n",
			 *usr_rcfile );
    return ( FALSE );
  }/* if */

  return ( TRUE );
}                                                  /* get default usr rcfile */



static 	BOOLEAN get_default_sb ( sb, rcfile )

	/* This function looks for the default sb value, first
	   as an environment variable, next in the rcfile.  If
	   it is in neither, it returns FALSE. */

    char     ** sb,                                       /* name of sandbox */
              * rcfile;                         /* name and path to rc file. */

{
    FILE      * ptr_file;                                  /* ptr to rc file */
    char      * env_input,                       /* holds values from getenv */
                line [ PATH_LEN ];                            /* misc string */
    char      * line_ptr,
              * token;

  if (( env_input = getenv ( SANDBOX )) != NULL ) {
    *sb = salloc ( env_input );
    ui_print ( VDEBUG, "Found sb name in environment.  Name is: %s.\n", *sb );
    return ( TRUE );
  } /* if */

  if (( ptr_file = fopen ( rcfile, READ )) == NULL ) {
    ui_print ( VWARN, "ERROR: cannot read rc file %s.\n", rcfile );
    return ( FALSE );
  } /* if */

  while (( line_ptr = fgets ( line, PATH_LEN, ptr_file )) != NULL ) {
    token = nxtarg ( &line_ptr, WHITESPACE );
    if ( streq ( token, DEFAULT ))
      break;
  } /* while */

  fclose ( ptr_file );

  if ( line_ptr == NULL ) {
    ui_print ( VDEBUG, "Did not find default sb name in rc file: %s.\n",
		rcfile );
    return ( FALSE );
  } /* if */

  else {
    token = nxtarg ( &line_ptr, WHITESPACE );
    *sb = salloc ( token );
    ui_print ( VDEBUG, "Found default sb name in rc file. Name is: %s.\n", *sb);
    return ( TRUE );
  } /* else */
}                                                          /* get default sb */



static 	BOOLEAN currentsb ( sbname, rcfile )

	/* This function checks to be sure the entered name of
	   the sandbox actually exists.  If it doesn't, it
	   returns FALSE, TRUE otherwise. */

    char      * sbname,                                   /* name of sandbox */
              * rcfile;                         /* name and path to rc file. */

{
    FILE      * ptr_file;                                  /* ptr to rc file */
    char        line [ PATH_LEN ];                            /* misc string */
    char      * line_ptr,
              * token;

  if (( ptr_file = fopen ( rcfile, READ )) == NULL ) {
    ui_print ( VWARN, "ERROR: cannot read rc file %s.\n", rcfile );
    return ( FALSE );
  } /* if */

  while (( line_ptr = fgets ( line, PATH_LEN, ptr_file )) != NULL ) {
    token = nxtarg ( &line_ptr, WHITESPACE );

    if ( streq ( token, SB )) {
      token = nxtarg ( &line_ptr, WHITESPACE );

      if ( streq ( token, sbname )) {
	ui_print ( VDEBUG, "sb name is in rc file.  Name is: %s.\n", sbname);
        fclose ( ptr_file );
        return ( TRUE );
      } /* if */
    } /* if */
  } /* while */

  ui_print ( VDEBUG, "Did not find sb name in rc file: %s.\n", rcfile );
  fclose ( ptr_file );
  return ( FALSE );
}                                                               /* currentsb */



static	BOOLEAN get_basedir ( sb, basedir, rcfile )

	/* This function looks for a directory to match the given
	   sb in the rcfile.  If it finds one, it returns TRUE. */

    char      * sb,                                       /* name of sandbox */
             ** basedir,                           /* name of base directory */
              * rcfile;                         /* name and path to rc file. */

{
    FILE      * ptr_file;                                  /* ptr to rc file */
    char        line [ PATH_LEN ];                            /* misc string */
    char      * line_ptr,
              * token;

  if (( ptr_file = fopen ( rcfile, READ )) == NULL ) {
    ui_print ( VWARN, "ERROR: cannot read rc file %s.\n", rcfile );
    return ( FALSE );
  } /* if */

  while (( line_ptr = fgets ( line, PATH_LEN, ptr_file )) != NULL ) {
    token = nxtarg ( &line_ptr, WHITESPACE );

    if ( streq ( token, BASE )) {
      token = nxtarg ( &line_ptr, WHITESPACE );

      if ( streq ( token, sb ))           /* sandbox name is matched exactly */
        break;

      if ( strchr ( token, STAR ) == NULL )      /* no wild card so no match */
        continue;

      if ( streq ( token, STAR_ST ))       /* "*" matches everything so okay */
        break;

      if ( gmatch ( sb, token ))        /* if wild card and match, then okay */
        break;
    } /* if */
  } /* while */

  fclose ( ptr_file );

  if ( line_ptr == NULL ) {
    ui_print ( VDEBUG, "Did not find base dir in rcfile: %s.\n", rcfile );
    return ( FALSE );
  } /* if */

  token = nxtarg ( &line_ptr, WHITESPACE );
  *basedir = salloc ( token );
  ui_print ( VDEBUG, "Found base dir in rcfile. Dir is: %s.\n", *basedir );
  return ( TRUE );
}                                                             /* get basedir */



static	BOOLEAN match_sb_basedir ( sb, basedir, rcfile )

	/* This function looks in the rcfile for a sb and basedir
	   pair that match the two entered.  If it finds a pair,
	   it returns TRUE. */

    char      * sb,                                       /* name of sandbox */
              * basedir,                           /* name of base directory */
              * rcfile;                         /* name and path to rc file. */

{
    FILE      * ptr_file;                                  /* ptr to rc file */
    char        line [ PATH_LEN ];                            /* misc string */
    char      * line_ptr,
              * token;

  if (( ptr_file = fopen ( rcfile, READ )) == NULL ) {
    ui_print ( VWARN, "ERROR: cannot read rc file %s.\n", rcfile );
    return ( FALSE );
  } /* if */

  while (( line_ptr = fgets ( line, PATH_LEN, ptr_file )) != NULL ) {
    token = nxtarg ( &line_ptr, WHITESPACE );

    if ( streq ( token, BASE )) {
      token = nxtarg ( &line_ptr, WHITESPACE );

      if ( ! streq ( token, sb )) {           /* if not sandbox name exactly */
        if ( strchr ( token, STAR ) == NULL )   /* no wild card, so no match */
	  continue;

        if ( ! streq ( token, STAR_ST ) &&         /* "*" matches everything */
             ! gmatch ( sb, token ))                 /* some wild card match */
	  continue;
      } /* if */

      token = nxtarg ( &line_ptr, WHITESPACE );

      if ( streq ( token, basedir )) {
        ui_print ( VDEBUG, "sb, %s, and base, %s, match.\n", sb, basedir );
        return ( TRUE );
      } /* if */
    } /* if */
  } /* while */

  ui_print ( VDEBUG, "sb, %s, and base, %s, don't match.\n", sb, basedir );
  fclose ( ptr_file );
  return ( FALSE );
}                                                        /* match sb basedir */



static	BOOLEAN current_sb_rcfile ( sbname, basedir, sb_rcfile, usr_rcfile )

	/* This function determines the correct sb rc file by first
	   checking to see if one is set, then by checking the projects
	   file, and finally by getting the default sb rc file from
	   the rc file. */

    char      * sbname,                                   /* name of sandbox */
              * basedir,                           /* name of base directory */
             ** sb_rcfile,                      /* string to hold sb rc file */
              * usr_rcfile;                     /* name and path to rc file. */

{
    char        ab_path [ PATH_LEN ];                         /* misc string */

  if (( *sb_rcfile == NULL ) &&
      ( ! is_project ( sbname, basedir, sb_rcfile )) &&
      ( ! get_default_sb_rcfile ( sbname, basedir, sb_rcfile, usr_rcfile )))
    return ( FALSE );
  
  if ( **sb_rcfile != SLASH ) {
    concat ( ab_path, PATH_LEN,
    		     basedir, "/", sbname, "/", RC_DIR, "/", *sb_rcfile, NULL );
    *sb_rcfile = salloc ( ab_path );
  } /* if */

  return ( TRUE );
}                                                       /* current sb rcfile */



static	BOOLEAN is_project ( sbname, basedir, sb_rcfile )

	/* This function checks to see if there is a projects file
	   and if that file has the current working directory listed
	   in it.  If it does, it gets the name of the sb rcfile there.
	   It returns TRUE if all this works, FALSE if for any reason
	   it doesn't. */

    char      * sbname,                                   /* name of sandbox */
              * basedir,                           /* name of base directory */
             ** sb_rcfile;                      /* string to hold sb rc file */

{
    FILE      * ptr_project;                               /* ptr to rc file */
    char        line [ PATH_LEN ],                            /* misc string */
                currentdir [ PATH_LEN ],         /* current working diretory */
                srcdir [ PATH_LEN ],                       /* path to sb src */
                projects [ PATH_LEN ],              /* path to projects file */
              * line_ptr,                                   /* misc char ptr */
              * token,                                      /* misc char ptr */
              * cdir;                           /* current directory pointer */
  
  concat ( projects, PATH_LEN, basedir, "/", sbname, "/", PROJECTS, NULL );
  concat ( srcdir, PATH_LEN, basedir, "/", sbname, "/", SRC_DIR, NULL );
  
  if (( access ( projects, R_OK ) != OK ) ||       /* is there projects file */
      ( getwd ( currentdir ) == NULL ) ||  /* current wd is not under sb/src */
      ( strncmp ( currentdir, srcdir, strlen ( srcdir )) != 0 ))
    return ( FALSE );

  cdir = currentdir + strlen ( srcdir );                 /* dir below sb/src */

  if ( *cdir == NUL )
   cdir = "/";

  if ( *cdir != SLASH )                          /* this should never happen */
    return ( FALSE );

  if (( ptr_project = fopen ( projects, READ )) == NULL )
    return ( FALSE );

  while (( line_ptr = fgets ( line, PATH_LEN, ptr_project )) != NULL ) {
    token = nxtarg ( &line_ptr, WHITESPACE );                /* get dir path */

    if ( strncmp ( token, cdir, strlen ( token )) == 0 ) {   /* matching dir */
      token = nxtarg ( &line_ptr, WHITESPACE );

      if ( *token == NUL )                                /* no second field */
        return ( FALSE );

      *sb_rcfile = salloc ( token );                              /* got it! */
      fclose ( ptr_project );
      return ( TRUE );
    } /* if */
  } /* while */

  fclose ( ptr_project );
  return ( FALSE );
}                                                              /* is project */



static	BOOLEAN get_default_sb_rcfile ( sbname, basedir, sb_rcfile, usr_rcfile )

	/* This function reads through the rc file, usr_rcfile,
	   looking for a match for the sbname.  When it finds it
	   it checks to see if there is a third field.  If there
	   is, it returns that information, else it returns the
	   path to the default sandbox rc file.  If there is an
	   error it returns FALSE, else TRUE. */

    char      * sbname,                                   /* name of sandbox */
              * basedir,                           /* name of base directory */
             ** sb_rcfile,                      /* string to hold sb rc file */
              * usr_rcfile;                     /* name and path to rc file. */

{
    FILE      * ptr_file;                                  /* ptr to rc file */
    char        line [ PATH_LEN ],                            /* misc string */
                tsbrcfile [ PATH_LEN ],              /* temp sb rc file name */
              * line_ptr,
              * token;

    
  if (( ptr_file = fopen ( usr_rcfile, READ )) == NULL ) {
    ui_print ( VWARN, "ERROR: cannot read rc file %s.\n", usr_rcfile );
    return ( FALSE );
  } /* if */

  while (( line_ptr = fgets ( line, PATH_LEN, ptr_file )) != NULL ) {
    token = nxtarg ( &line_ptr, WHITESPACE );

    if ( streq ( token, SB )) {                   /* key word to match is SB */
      token = nxtarg ( &line_ptr, WHITESPACE );

      if ( streq ( token, sbname )) {                /* matches sandbox name */
        token = nxtarg ( &line_ptr, WHITESPACE );

        if ( *token == NUL )                  /* no third field, use default */
	    break;

	ui_print ( VDEBUG,
	        "Found sandbox rc file in usr rcfile. File is: %s.\n", token );
        *sb_rcfile = salloc ( token );
        fclose ( ptr_file );
        return ( TRUE );
      } /* if */
    } /* if */
  } /* while */

  fclose ( ptr_file );
  concat ( tsbrcfile, PATH_LEN, basedir, "/", sbname, "/", LOCAL_RC, NULL );
  *sb_rcfile = salloc ( tsbrcfile );
  ui_print ( VDEBUG, "Using default sandbox rc file: %s.\n", *sb_rcfile );
  return ( TRUE );
}                                                   /* get default sb rcfile */



current_set ( setname, setdir, sbname, rc_file )

	/* This procedure gets the current set name, first from the
	   environment variable, BCSSET, and then, if it is not set
	   from the sandbox set rc file.  If the sandbox name is
	   empty, it gets the default sandbox.  It returns -1 if
	   it still can't determine the setname.  If the setname
	   entered is not empty, it checks to see if it is in the
	   sandbox sets rc file.  It returns -1 if it is not. */

    char     ** setname,                             /* the set name to fill */
             ** setdir,                              /* set directory t fill */
             ** sbname,                               /* the current sandbox */
             ** rc_file;                                   /* rc file to use */

{
    FILE      * ptr_file;                                  /* ptr to rc file */
    char      * env_input,                       /* holds values from getenv */
              * tmp = NULL,                                   /* misc string */
              * base = NULL,                                  /* misc string */
                set_loc [ PATH_LEN ],                         /* misc string */
                line [ PATH_LEN ],                            /* misc string */
                ab_path [ PATH_LEN ];                         /* misc string */
    char      * line_ptr,
              * token;

  ui_set_progname ( "current_set" );

  if ( *rc_file == NULL ) {                       /* fix this no matter what */
    if  ( ! get_default_usr_rcfile ( rc_file, TRUE )) {
      ui_restore_progname ();
      return ( ERROR );
    } /* if */
  } /* if */

  else if ( **rc_file != SLASH ) {
    if ( abspath ( *rc_file, ab_path ) == ERROR ) {
      ui_print ( VFATAL, "ERROR: could not get cwd for rcfile %s\n", *rc_file );
      ui_restore_progname ();
      return ( ERROR );
    } /* if */

    *rc_file = salloc ( ab_path );
  } /* else if */

  if ( *setname != NULL ) {                                 /* already given */
    if ( is_existing_set ( *setname, setdir, sbname, rc_file )) {
      ui_restore_progname ();
      return ( OK );
    } /* if */

    else {
      ui_restore_progname ();
      return ( ERROR );
    } /* else */
  } /* if */

  if (( env_input = getenv ( BCSSET )) != NULL ) {
    *setname = salloc ( env_input );         /* get it from the env variable */

    if ( is_existing_set ( *setname, setdir, sbname, rc_file )) {
      ui_restore_progname ();
      return ( OK );
    } /* if */

    else {
      ui_restore_progname ();
      return ( ERROR );
    } /* else */
  } /* if */

  ui_restore_progname ();

  if ( current_sb ( sbname, &base, &tmp, rc_file ) == ERROR )
    return ( ERROR );

  ui_set_progname ( "current_set" );
  concat ( set_loc, PATH_LEN, base, "/", *sbname, "/", SET_RC, NULL );

  if (( ptr_file = fopen ( set_loc, READ )) == NULL ) {
    ui_print ( VWARN, "ERROR: cannot read from sets rc file\n  %s.\n",
	       set_loc );
    ui_restore_progname ();
    return ( ERROR );
  } /* if */

  while (( line_ptr = fgets ( line, PATH_LEN, ptr_file )) != NULL ) {
    token = nxtarg ( &line_ptr, WHITESPACE );

    if ( streq ( token, DEFAULT )) {
      token = nxtarg ( &line_ptr, WHITESPACE );
      *setname = salloc ( token );
      fclose ( ptr_file );
      ui_print ( VDEBUG, "found default set, %s, in sb rcfile.\n", *setname );

      if ( is_existing_set ( *setname, setdir, sbname, rc_file )) {
        ui_restore_progname ();
        return ( OK );
      } /* if */

      else {
        ui_restore_progname ();
        return ( ERROR );
      } /* else */
    } /* if */
  } /* while */

  fclose ( ptr_file );
  ui_print ( VDEBUG, "no default set in rcfile: %s.\n", set_loc );
  ui_restore_progname ();
  return ( ERROR );
}                                                             /* current set */



static	BOOLEAN	is_existing_set ( setname, setdir, sbname, rc_file )

	/* This function checks to see if the set name is in the
	   current sandbox rc_files/set file.  It returns TRUE
	   if it is, FALSE if not. It also checks the setdir,
	   filling it in if it is empty and checking for consistency
	   with the setname if it is not. */

    char      * setname,                             /* the set name to fill */
             ** setdir,                              /* set directory t fill */
             ** sbname,                               /* the current sandbox */
             ** rc_file;                                   /* rc file to use */

{
    FILE      * ptr_file;                                  /* ptr to rc file */
    char      * tmp = NULL,                                   /* misc string */
              * base = NULL,                                  /* misc string */
                set_loc [ PATH_LEN ],                         /* misc string */
                line [ PATH_LEN ];                            /* misc string */
    char      * line_ptr,
              * token;

  ui_restore_progname ();

  if ( current_sb ( sbname, &base, &tmp, rc_file ) == ERROR )
    return ( FALSE );

  ui_set_progname ( "current_set" );
  concat ( set_loc, PATH_LEN, base, "/", *sbname, "/", SET_RC, NULL );

  if (( ptr_file = fopen ( set_loc, READ )) == NULL ) {
    ui_print ( VWARN, "ERROR: cannot read from set rc file\n  %s.\n", set_loc);
    return ( FALSE );
  } /* if */

  while (( line_ptr = fgets ( line, PATH_LEN, ptr_file )) != NULL ) {
    token = nxtarg ( &line_ptr, WHITESPACE );

    if ( streq ( token, SET_KEY )) {
      token = nxtarg ( &line_ptr, WHITESPACE );

      if ( streq ( token, setname )) {
        token = nxtarg ( &line_ptr, WHITESPACE );
        fclose ( ptr_file );

	if ( *setdir == NULL )
	  *setdir = salloc ( token );

	else if ( ! streq ( *setdir, token )) {
          ui_print ( VDEBUG, "could not match set, %s, to setdir, %s.\n",
			      setname, *setdir );
	  return ( FALSE );
	} /* else if */

        ui_print ( VDEBUG, "matched set, %s, and setdir, %s, in sb rcfile.\n",
			setname, *setdir );
        return ( TRUE );
      } /* if */
    } /* if */
  } /* while */

  ui_print ( VDEBUG, "could not find set, %s, in sb rcfile %s.\n",
		       setname, set_loc );
  fclose ( ptr_file );
  return ( FALSE );
}                                                         /* is existing set */
