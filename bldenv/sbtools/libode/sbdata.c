/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: current_sb
 *		current_sb_rcfile
 *		current_set
 *		currentsb
 *		get_basedir
 *		get_default_sb
 *		get_default_sb_rcfile
 *		get_default_usr_rcfile
 *		in_sandbox
 *		is_existing_set
 *		is_project
 *		match_sb_basedir
 *		sb_conf_read
 *		sb_conf_read_chain
 *		sb_conf_resb
 *		sb_conf_std
 *		sb_conf_write
 *		sb_current_dir
 *		sb_full_path
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
 * $Log: sbdata.c,v $
 * Revision 1.2.10.5  1993/11/12  18:31:56  damon
 * 	CR 780. resb now handles subproject sb.conf files
 * 	[1993/11/12  18:31:43  damon]
 *
 * Revision 1.2.10.4  1993/11/10  16:57:01  root
 * 	CR 463. Cast stdrup paramater to (char *)
 * 	[1993/11/10  16:56:04  root]
 * 
 * Revision 1.2.10.3  1993/11/03  20:40:51  damon
 * 	CR 463. More pedantic
 * 	[1993/11/03  20:38:20  damon]
 * 
 * Revision 1.2.10.2  1993/10/19  20:53:21  damon
 * 	CR 708. Use only WORKON and SANDBOX env vars to determine in_sandbox()
 * 	[1993/10/19  20:53:11  damon]
 * 
 * Revision 1.2.10.1  1993/10/06  23:28:49  damon
 * 	CR 697. Do not complain if vars are not set in sb.conf
 * 	[1993/10/06  23:28:37  damon]
 * 
 * Revision 1.2.8.1  1993/09/21  16:48:44  damon
 * 	CR 673. Read all sb.conf files in backing chain
 * 	[1993/09/21  16:48:10  damon]
 * 
 * Revision 1.2.4.14  1993/06/02  13:52:02  damon
 * 	CR 517. Cleaned up subprojects wrt sb.conf and sc.conf
 * 	[1993/06/02  13:50:48  damon]
 * 
 * Revision 1.2.4.13  1993/05/27  16:34:51  marty
 * 	CR # 546 - is_sandbox() no longer frees the results gotten from getenv() call.
 * 	[1993/05/27  16:34:22  marty]
 * 
 * Revision 1.2.4.12  1993/05/27  15:15:22  damon
 * 	CR 548. Change sb_conf_read to use sb_path instead of basedir+sb
 * 	[1993/05/27  15:15:05  damon]
 * 
 * Revision 1.2.4.11  1993/05/14  16:50:20  damon
 * 	CR 518. Added sb_full_path
 * 	[1993/05/14  16:47:55  damon]
 * 
 * Revision 1.2.4.10  1993/05/13  14:53:03  damon
 * 	CR 521. Added sb_conf_resb for resbing
 * 	[1993/05/13  14:52:09  damon]
 * 
 * Revision 1.2.4.9  1993/05/10  19:11:37  marty
 * 	CR # 472 - Added routine in_sandbox() to determine (however loosely)
 * 	whether or not a user in in a sandbox (via 'workon').
 * 	[1993/05/10  19:11:19  marty]
 * 
 * Revision 1.2.4.8  1993/04/28  20:07:30  damon
 * 	CR Pedantic changes
 * 	[1993/04/28  20:07:12  damon]
 * 
 * Revision 1.2.4.7  1993/04/21  19:53:23  damon
 * 	CR 417. Removed link to backing build sb.conf file
 * 	[1993/04/21  19:53:10  damon]
 * 
 * Revision 1.2.4.6  1993/04/09  17:16:06  damon
 * 	CR 446. Remove warnings with added includes
 * 	[1993/04/09  17:15:16  damon]
 * 
 * Revision 1.2.4.5  1993/04/09  14:26:58  damon
 * 	CR 417. Made routines project aware
 * 	[1993/04/09  14:26:42  damon]
 * 
 * Revision 1.2.4.4  1993/02/01  21:55:08  damon
 * 	CR 417
 * 	Changed sb_config* to sb_conf.
 * 	Removed external_user and ode_links.
 * 	[1993/02/01  21:45:39  damon]
 * 
 * Revision 1.2.4.3  1993/01/28  23:33:28  damon
 * 	CR 417. Added sb_config_write
 * 	[1993/01/28  23:32:56  damon]
 * 
 * Revision 1.2.4.2  1993/01/27  18:19:11  damon
 * 	CR 417. Added sb_config_read and sb_config_std
 * 	[1993/01/27  18:17:10  damon]
 * 
 * Revision 1.2.2.6  1992/12/03  17:22:25  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:09:04  damon]
 * 
 * Revision 1.2.2.5  1992/11/06  18:34:47  damon
 * 	CR 329. Made more portable
 * 	[1992/11/06  18:32:50  damon]
 * 
 * Revision 1.2.2.4  1992/09/24  19:02:40  gm
 * 	CR282: Made more portable to non-BSD systems.
 * 	[1992/09/23  18:22:54  gm]
 * 
 * Revision 1.2.2.3  1992/08/07  15:31:35  damon
 * 	CR 266. Changed salloc to strdup
 * 	[1992/08/07  15:29:21  damon]
 * 
 * Revision 1.2.2.2  1992/07/08  14:45:26  gm
 * 	Bug #216: Add SANDBOXRC environment variable support.
 * 	[1992/07/08  14:43:54  gm]
 * 
 * Revision 1.2  1991/12/05  21:05:53  devrcs
 * 	Make get_default_usr_rcfile quieter
 * 	[1991/11/11  20:55:51  damon]
 * 
 * 	Hacked a fix to get_default_usr_rc so that the value
 * 	of rc would be set, even if $HOME is not set. Fixes
 * 	bug #99
 * 	[1991/10/30  21:38:58  damon]
 * 
 * 	Changed sdm to ode; std_defs.h to  odedefs.h
 * 	[91/01/10  11:46:57  randyb]
 * 
 * 	This routine replaces rc-files_ext.c.  It changed those routines
 * 	to take pointers instead of arrays and uses all the ui_
 * 	functionality.
 * 	[91/01/08  12:22:00  randyb]
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

#ifndef lint
static char sccsid[] = "@(#)19  1.1  src/bldenv/sbtools/libode/sbdata.c, bldprocess, bos412, GOLDA411a 1/19/94 17:42:26";
#endif /* not lint */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ode/errno.h>
#include <ode/interface.h>
#include <ode/odedefs.h>
#include <ode/parse_rc_file.h>
#include <ode/public/error.h>
#include <ode/sandboxes.h>
#include <ode/sets.h>
#include <ode/util.h>
#include <sys/param.h>

/*
 * PROTOTYPES
 */
static BOOLEAN get_default_sb ( char ** sb, char * rcfile );
static BOOLEAN currentsb ( char * sbname, char * rcfile );
static BOOLEAN get_basedir ( char * sb, char ** basedir, char * rcfile );
static BOOLEAN current_sb_rcfile ( char * sbname, char * basedir,
                                    char ** sb_rcfile, char * usr_rcfile );
static BOOLEAN is_project ( char * sbname, char * basedir, char ** sb_rcfile );
static BOOLEAN get_default_sb_rcfile ( char * sbname, char * basedir,
                                        char ** sb_rcfile, char * usr_rcfile );
static BOOLEAN match_sb_basedir ( char * sb, char * basedir, char * rcfile );
static	BOOLEAN	is_existing_set ( char * setname, char ** setdir,
                                  char ** sbname, char ** rc_file );
BOOLEAN	get_default_usr_rcfile ( char ** usr_rcfile, BOOLEAN report );

int
current_sb (
    char     ** sb,                                       /* name of sandbox */
    char     ** basedir,                           /* name of base directory */
    char     ** sb_rcfile,                       /* name of sandbox rc file. */
    char     ** usr_rcfile )                    /* name and path to rc file. */

	/* This function checks to see what type of search is needed.
	   It then calls the appropriate functions to get the
	   information.  If the information is found, it returns
	   0, else -1. */


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

    *usr_rcfile = strdup ( ab_path );
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

void
sb_conf_read ( struct rcfile * rc, const char * sb_path, char * project,
               char * sub_project )
{
  char rcfile_name [MAXPATHLEN];

  if ( sub_project == NULL ) {
    concat ( rcfile_name, sizeof (rcfile_name ), sb_path, "/",
             RC_DIR, "/", project, "/", SBCONF_RC, NULL );
  } else {
    concat ( rcfile_name, sizeof (rcfile_name ), sb_path, "/",
             RC_DIR, "/", project, "/", sub_project, "/", SBCONF_RC, NULL );
  } /* if */
  init_rc_contents ( rc, rcfile_name );
} /* end sb_conf_read */

void
sb_conf_read_chain ( struct rcfile *rc, const char * sb_path,
                     char * project, const char * sub_project )
{
/* FIXME:
 * This routine stole code from bld_conf_read. There is a lot of
 * reduncancy when these two routines are both called. Probably should be
 * cleaned up at some point.
 */
  char * bases [10];
  int num_bases=0;
  struct rcfile sbrc;
  char * new_base;
  char rcfile_name [MAXPATHLEN];
  int i;

  bases [num_bases++] = strdup ( (char *)sb_path );
  for (;;) {
    sb_conf_read ( &sbrc, bases[num_bases-1], project, NULL );
    if ( get_rc_value ( "backing_build", &new_base, &sbrc, FALSE ) == OK ) {
      bases [num_bases++] = new_base;
    } else
      break;
    /* if */
  } /* for */
  memset ( (char *)rc, 0, sizeof ( *rc ));
  for ( i = num_bases-1; i >= 0; i-- ) {
    if ( sub_project == NULL ) {
      concat ( rcfile_name, sizeof (rcfile_name ), bases[i], "/",
               RC_DIR, "/", project, "/", SBCONF_RC, NULL );
    } else {
      concat ( rcfile_name, sizeof (rcfile_name ), bases[i], "/",
               RC_DIR, "/", project, "/", sub_project, "/", SBCONF_RC, NULL );
    } /* if */
    parse_rc_file ( rcfile_name, rc );
  } /* for */

} /* end sb_conf_read_chain */

ERR_LOG
sb_conf_write ( char * basedir, char * sb, char * backing_project,
                char * sub_project, char * backing_build, BOOLEAN ode_sc,
                BOOLEAN ode_build_env )
{
  FILE * f;
  char rcfile_name [MAXPATHLEN];
  char line [MAXPATHLEN];

  if ( sub_project == NULL ) {
    concat ( rcfile_name, sizeof (rcfile_name ), basedir, "/", sb, "/",
             RC_DIR, "/", backing_project, "/", SBCONF_RC, NULL );
  } else {
    concat ( rcfile_name, sizeof (rcfile_name ), basedir, "/", sb, "/",
             RC_DIR, "/", backing_project, "/", sub_project, "/", SBCONF_RC,
             NULL );
  } /* if */
  if ( (f = fopen ( rcfile_name, WRITE ) ) == NULL )
    return ( err_log ( OE_OPEN, rcfile_name ) );
  /* if */
  if ( backing_project != NULL ) {
    concat ( line, sizeof (line), "replace backing_project\t", backing_project,
             "\n", NULL );
    fputs ( line, f );
    concat ( line, sizeof (line), "replace backing_build\t", backing_build,
             "\n", NULL );
    fputs ( line, f );
  } /* if */
  if ( ode_sc )
    fputs ( "replace ode_sc\ttrue\n", f );
  else
    fputs ( "replace ode_sc\tfalse\n", f );
  /* if */
  if ( ode_build_env )
    fputs ( "replace ode_build_env\ttrue\n", f );
  else
    fputs ( "replace ode_build_env\tfalse\n", f );
  /* if */
  fclose ( f );
  return ( OE_OK );
} /* sb_conf_write */

ERR_LOG
sb_conf_resb ( const char * basedir, const char * sb,
               const char * backing_project, const char * sub_project,
               const char * backing_build )
{
  FILE * f;
  FILE * tf;
  char rcfile_name [MAXPATHLEN];
  char tmp_rcfile [MAXPATHLEN];
  char buf [MAXPATHLEN];
  char * token1;
  char * token2;
  char * tmp_buf;
  char * buf_ptr;

  if ( sub_project == NULL ) {
    concat ( rcfile_name, sizeof (rcfile_name ), basedir, "/", sb, "/",
             RC_DIR, "/", backing_project, "/", SBCONF_RC, NULL );
  } else {
    concat ( rcfile_name, sizeof (rcfile_name ), basedir, "/", sb, "/",
             RC_DIR, "/", backing_project, "/", sub_project, "/", SBCONF_RC,
             NULL );
  } /* if */
  concat ( tmp_rcfile, sizeof (tmp_rcfile), rcfile_name, ".tmp", NULL );
  if ( (f = fopen ( rcfile_name, READ ) ) == NULL )
    return ( err_log ( OE_OPEN, rcfile_name ) );
  /* if */
  if ( (tf = fopen ( tmp_rcfile, WRITE ) ) == NULL )
    return ( err_log ( OE_OPEN, tmp_rcfile ) );
  /* if */
  while ( fgets ( buf, sizeof(buf), f ) != NULL ) {
    tmp_buf = strdup ( buf );
    buf_ptr = tmp_buf;
    token1 = nxtarg ( &buf_ptr, WHITESPACE );
    token2 = nxtarg ( &buf_ptr, WHITESPACE );
    if ( strcmp ( token1, "backing_build" ) == 0 ||
         ( strcmp ( token1, "replace" ) == 0 &&
           strcmp ( token2, "backing_build" ) == 0 ) ) {
      concat ( buf, sizeof (buf), "replace backing_build\t", backing_build,
               "\n", NULL );
      fputs ( buf, tf );
      free ( tmp_buf );
      break;
    } /* if */
    free ( tmp_buf );
    fputs ( buf, tf );
  } /* while */
  while ( fgets ( buf, sizeof(buf), f ) != NULL ) {
    fputs ( buf, tf );
  } /* while */
  fclose ( tf );
  fclose ( f );
  rename ( tmp_rcfile, rcfile_name );
  return ( OE_OK );
} /* sb_resb */

ERR_LOG
sb_conf_std ( struct rcfile * rc, char ** backing_project,
              char ** backing_build, BOOLEAN * ode_sc,
              BOOLEAN * ode_build_env )

{
  char * var;

  *backing_project = NULL;
  *backing_build = NULL;

  get_rc_value ( "backing_project", backing_project, rc, FALSE );
  get_rc_value ( "backing_build", backing_build, rc, FALSE );
  if ( get_rc_value ( "ode_sc", &var, rc, FALSE ) != OK ) {
    *ode_sc = TRUE;
  } /* if */
  *ode_sc = ( strcmp ( var, "true" ) == 0 );
  if ( get_rc_value ( "ode_build_env", &var, rc, FALSE ) != OK ) {
    *ode_build_env = TRUE;
  } /* if */
  *ode_build_env = ( strcmp ( var, "true" ) == 0 );
  return ( OE_OK );
} /* end sb_conf_std */



BOOLEAN	get_default_usr_rcfile ( char ** usr_rcfile, BOOLEAN report )

	/* This function gives the usr_rcfile the default value.
	   If it fails to find the file, it returns FALSE,
	   else TRUE. */

{
    char      * env_input,                       /* holds values from getenv */
                trcfile [ PATH_LEN ];                  /* tmp rc file holder */

  if (( env_input = getenv ( "SANDBOXRC" )) != NULL ) {
    *usr_rcfile = strdup ( env_input );
  }

  else {
    if (( env_input = getenv ( "HOME" )) == NULL ) {
      if ( report )
	ui_print ( VWARN, "HOME not set in enviroment.\n" );
      /* if */
      return ( FALSE );
    } /* if */

    concat ( trcfile, PATH_LEN, env_input, "/", SANDBOXRC, NULL );
    *usr_rcfile = strdup ( trcfile );

  } /* if */

  if ( access ( *usr_rcfile, R_OK ) == ERROR ) {
    if ( report )
      ui_print ( VWARN, "could not access rc file, %s, for reading.\n",
			 *usr_rcfile );
    return ( FALSE );
  }/* if */

  return ( TRUE );
}                                                  /* get default usr rcfile */



static 	BOOLEAN get_default_sb ( char ** sb, char * rcfile )

	/* This function looks for the default sb value, first
	   as an environment variable, next in the rcfile.  If
	   it is in neither, it returns FALSE. */

{
    FILE      * ptr_file;                                  /* ptr to rc file */
    char      * env_input,                       /* holds values from getenv */
                line [ PATH_LEN ];                            /* misc string */
    char      * line_ptr,
              * token;

  if (( env_input = getenv ( SANDBOX )) != NULL ) {
    *sb = strdup ( env_input );
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
    *sb = strdup ( token );
    ui_print ( VDEBUG, "Found default sb name in rc file. Name is: %s.\n", *sb);
    return ( TRUE );
  } /* else */
}                                                          /* get default sb */



static 	BOOLEAN currentsb ( char * sbname, char * rcfile )

	/* This function checks to be sure the entered name of
	   the sandbox actually exists.  If it doesn't, it
	   returns FALSE, TRUE otherwise. */

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



static	BOOLEAN get_basedir ( char * sb, char ** basedir, char * rcfile )

	/* This function looks for a directory to match the given
	   sb in the rcfile.  If it finds one, it returns TRUE. */

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
  *basedir = strdup ( token );
  ui_print ( VDEBUG, "Found base dir in rcfile. Dir is: %s.\n", *basedir );
  return ( TRUE );
}                                                             /* get basedir */



static	BOOLEAN match_sb_basedir ( char * sb, char * basedir, char * rcfile )

	/* This function looks in the rcfile for a sb and basedir
	   pair that match the two entered.  If it finds a pair,
	   it returns TRUE. */

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



static	BOOLEAN current_sb_rcfile ( char * sbname, char * basedir,
                                    char ** sb_rcfile, char * usr_rcfile )

	/* This function determines the correct sb rc file by first
	   checking to see if one is set, then by checking the projects
	   file, and finally by getting the default sb rc file from
	   the rc file. */

{
    char        ab_path [ PATH_LEN ];                         /* misc string */

  if (( *sb_rcfile == NULL ) &&
      ( ! is_project ( sbname, basedir, sb_rcfile )) &&
      ( ! get_default_sb_rcfile ( sbname, basedir, sb_rcfile, usr_rcfile )))
    return ( FALSE );
  
  if ( **sb_rcfile != SLASH ) {
    concat ( ab_path, PATH_LEN,
    		     basedir, "/", sbname, "/", RC_DIR, "/", *sb_rcfile, NULL );
    *sb_rcfile = strdup ( ab_path );
  } /* if */

  return ( TRUE );
}                                                       /* current sb rcfile */

char *
sb_full_path ( const char * basedir, const char * sb )
{
  char full_path[MAXPATHLEN];

  concat ( full_path, sizeof (full_path), basedir, "/", sb, NULL );
  return ( strdup ( full_path ) );
} /* end sb_full_path */

BOOLEAN
sb_current_dir ( const char * sbname, const char * basedir, char ** dir )
{
  char   currentdir [ PATH_LEN ];         /* current working diretory */
  char   srcdir [ PATH_LEN ];                       /* path to sb src */
  char * cdir;                           /* current directory pointer */

  /* current wd is not under sb/src */
  concat ( srcdir, PATH_LEN, basedir, "/", sbname, "/", SRC_DIR, NULL );
  if ( getcwd ( currentdir, sizeof(currentdir)) == NULL  ||
       strncmp ( currentdir, srcdir, strlen ( srcdir )) != 0 )
    return ( FALSE );

  cdir = currentdir + strlen ( srcdir );                 /* dir below sb/src */
  concat ( currentdir, sizeof (currentdir), ".", cdir, NULL );
  *dir = strdup ( currentdir );
  return ( TRUE );
} /* end sb_current_dir */

static	BOOLEAN is_project ( char * sbname, char * basedir, char ** sb_rcfile )

	/* This function checks to see if there is a projects file
	   and if that file has the current working directory listed
	   in it.  If it does, it gets the name of the sb rcfile there.
	   It returns TRUE if all this works, FALSE if for any reason
	   it doesn't. */

{
    FILE      * ptr_project;                               /* ptr to rc file */
    char        line [ PATH_LEN ],                            /* misc string */
                projects [ PATH_LEN ],              /* path to projects file */
              * line_ptr,                                   /* misc char ptr */
              * token;                                      /* misc char ptr */
         char * cdir;                           /* current directory pointer */
  
  concat ( projects, PATH_LEN, basedir, "/", sbname, "/", PROJECTS, NULL );
  if ( access ( projects, R_OK ) != OK )
    return ( FALSE );
  if ( ! sb_current_dir ( sbname, basedir, &cdir ) )
    return ( FALSE );

  if ( *cdir == NUL )
   cdir = (char * )"/";

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

      *sb_rcfile = strdup ( token );                              /* got it! */
      fclose ( ptr_project );
      return ( TRUE );
    } /* if */
  } /* while */

  fclose ( ptr_project );
  return ( FALSE );
}                                                              /* is project */



static	BOOLEAN get_default_sb_rcfile ( char * sbname, char * basedir,
                                        char ** sb_rcfile, char * usr_rcfile )

	/* This function reads through the rc file, usr_rcfile,
	   looking for a match for the sbname.  When it finds it
	   it checks to see if there is a third field.  If there
	   is, it returns that information, else it returns the
	   path to the default sandbox rc file.  If there is an
	   error it returns FALSE, else TRUE. */

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
        *sb_rcfile = strdup ( token );
        fclose ( ptr_file );
        return ( TRUE );
      } /* if */
    } /* if */
  } /* while */

  fclose ( ptr_file );
  concat ( tsbrcfile, PATH_LEN, basedir, "/", sbname, "/", LOCAL_RC, NULL );
  *sb_rcfile = strdup ( tsbrcfile );
  ui_print ( VDEBUG, "Using default sandbox rc file: %s.\n", *sb_rcfile );
  return ( TRUE );
}                                                   /* get default sb rcfile */


int
current_set ( 
    char     ** setname,                             /* the set name to fill */
    char     ** setdir,                              /* set directory t fill */
    char     ** sbname,                               /* the current sandbox */
    char     ** rc_file )                                  /* rc file to use */

	/* This procedure gets the current set name, first from the
	   environment variable, BCSSET, and then, if it is not set
	   from the sandbox set rc file.  If the sandbox name is
	   empty, it gets the default sandbox.  It returns -1 if
	   it still can't determine the setname.  If the setname
	   entered is not empty, it checks to see if it is in the
	   sandbox sets rc file.  It returns -1 if it is not. */


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

    *rc_file = strdup ( ab_path );
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
    *setname = strdup ( env_input );         /* get it from the env variable */

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
      *setname = strdup ( token );
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



static	BOOLEAN	is_existing_set ( char * setname, char ** setdir,
                                  char ** sbname, char ** rc_file )

	/* This function checks to see if the set name is in the
	   current sandbox rc_files/set file.  It returns TRUE
	   if it is, FALSE if not. It also checks the setdir,
	   filling it in if it is empty and checking for consistency
	   with the setname if it is not. */

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
	  *setdir = strdup ( token );

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

BOOLEAN	in_sandbox (void)
	/* Check to see if a users has done a 'workon'.
	   Two environment variables will be used to 
	   verify sandbox environment (WORKON, SANDBOX).
	*/
{
    char      * env_input;                       /* holds values from getenv */

  env_input = getenv ( "SANDBOX" );
  if ( env_input == NULL ) {
	return (FALSE);
  } 

  env_input = getenv ( "WORKON" );
  if ( env_input == NULL ) {
	return (FALSE);
  } 

  return (TRUE);
}
