static char sccsid[] = "@(#)31  1.1  src/bldenv/sbtools/libsb/def_build.c, bldprocess, bos412, GOLDA411a 4/29/93 12:19:53";
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
**                          Open Software Foundation              	     **
**                              Cambridge, MA                                **
**                              Randy Barbano                                **
**                               April 1990                                  **
*******************************************************************************
**
**  Description:
**	These are functions for library libsb.a.
**
**  Lib Functions and Their Usage:
**	1) default_build ( char** base, char** build, char** set,
**			   char* sb_rcfile) int
**
**	   returns: ERROR if any type of failure else it returns OK.
**
**	   usage:
**	     The values of the first three pointers are filled with their
**	     values from the rc files, sb_rcfile.  If the pointer = NULL,
**	     there no attempt to fill it.
**
**	2) build_base_dir ( char* keyword, char* sbfile,
**			    struct rcfile contents, BOOLEAN errors,
**			    BOOLEAN default ) char *
**
**	   returns: pointer to string or NULL if none found
**
**	    usage:
**	      The value of the keyword is searched for in the build list
**	      file.  This file is found using the rc value of BUILD_LIST.
**	      keyword should be a build name and the return value will be
**	      the base directory to that build.
**
**    functions called by lib functions:
**	1) default_build: parse_rc_file, get_rc_value
**	3) build_base_dir: get_rc_value, nxtarg
**
 */

# include <ode/odedefs.h>
# include <sys/param.h>
# include <sys/stat.h>
# include <ode/parse_rc_file.h>

void 	rm_newline ();

int	default_build ( base, build, set, sb_rcfile )

	/* This function retreives the values for each of the arguments:
	   base, build, and set, assuming they are not set to to NULL.
	   The values come from reading the sandbox rc file.  If all
	   goes well, OK is return, else ERROR. */

    char     ** base,                            /* base of project's builds */
             ** build,                    /* name of project's default build */
             ** set,                /* name of default set for default build */
              * sb_rcfile;                       /* sandbox rc file to parse */

{
    struct      rcfile    rc_contents;          /* holds contents of rc file */
    int         value = OK;                                  /* return value */

  bzero ( &rc_contents, sizeof ( rc_contents ));

  if ( parse_rc_file ( sb_rcfile, &rc_contents )) {
    ui_print ( VWARN, "unable to parse sandbox rcfile %s.\n", sb_rcfile );
    value = ERROR;
  } /* if */

  if ( value == OK && set != NULL )
    value = get_rc_value ( DEF_SET, set, &rc_contents, TRUE );

  if ( value == OK && build != NULL )
    value = get_rc_value ( DEF_BUILD, build, &rc_contents, TRUE );
  
  if ( value == OK && base != NULL )
    value = get_rc_value ( BUILD_BASE, base, &rc_contents, TRUE );
  
  return ( value );
}                                                           /* default build */

char * build_base_dir ( keyword, contents, errors, defaultx )

	/* This function looks for the base directory of the keyword;
	   assuming the keyword is a build.  It looks for it in the
	   file designated by the rc variable, BUILD_LIST, or in the
	   file, DEFAULT_BLIST.  It returns the base directory or NUL
	   if none is found. */

    char      * keyword;       /* identifying build's record in "build_list" */
    struct      rcfile  * contents;             /* holds contents of rc file */
    BOOLEAN     errors,            /* true if error messages should be given */
		defaultx;  			   /* use default build_list */

{
    FILE      * bptr;                              /* ptr to file build_list */
    struct      stat statb;                                /* stat structure */
    char        recbuf [ PATH_LEN ],      /* read of records from build_list */
		listbuf [ PATH_LEN ],   /* hold space for name of build_list */
              * blist,           	      /* path and name of build_list */
	      * rec_ptr,                           /* ptr to recbuf contents */
              * token,                         /* holds each token as parsed */
              * errormsg = "ERROR: build_base_dir:";   /* head for error msg */

  if ( *keyword == NUL ) {
    if ( errors )
      ui_print( VWARN, "%s bad keyword argument.\n", errormsg );
    return ( NULL );
  } /* if */

  if ( defaultx ) {        				      /* use default */
    strcpy ( listbuf, DEFAULT_BLIST );
    blist = listbuf;
  } /* if */

  else if ( get_rc_value ( BUILD_LIST, &blist, contents, errors ) == ERROR ) {
    strcpy ( listbuf, DEFAULT_BLIST );  /* if build_list not given, try this */
    blist = listbuf;
  } /* else if */

  if (( access ( blist, R_OK ) != OK ) ||
      (( bptr = fopen ( blist, READ )) == NULL )) {
    if ( streq (  blist, DEFAULT_BLIST )) {
      if ( errors )  			   /* error if already using default */
	ui_print ( VWARN, "%s cannot read build list: %s.\n", errormsg, blist );
      return( NULL );
    } /* if */
 							      /* try default */
    else if (( access ( DEFAULT_BLIST, R_OK ) != OK ) ||
             (( bptr = fopen ( DEFAULT_BLIST, READ )) == NULL )) { 
      if ( errors )
	ui_print ( VWARN, "%s cannot read default build list: %s.\n",
			   errormsg, DEFAULT_BLIST );
      return( NULL );
    } /* else if */
  } /* if */

  while ( fgets ( recbuf, PATH_LEN, bptr) != NULL ) {
    rec_ptr = recbuf;
    token = nxtarg ( &rec_ptr, WHITESPACE );
	  
    if ( *token != NUL && streq ( token, keyword )) {
      token = nxtarg ( &rec_ptr, WHITESPACE );            /* get third token */

      if ( *token == NUL || *rec_ptr == NUL ) {
        if ( errors )
	  ui_print ( VWARN, "%s no base directory for build %s.\n",
			     errormsg, keyword );
        return( NULL );
      } /* if */

      rm_newline ( rec_ptr );

      if (( stat ( rec_ptr, &statb ) == ERROR ) ||
	  (( statb.st_mode & S_IFMT ) != S_IFDIR )) {
        if ( errors )
	  ui_print ( VWARN, "%s base for build %s is not a directory.\n",
			     errormsg, keyword );
        return( NULL );
      } /* if */

      if (( rec_ptr = (char *) salloc ( rec_ptr )) == NULL ) {
	ui_print ( VWARN, "%s unable to salloc base dir.\n", errormsg );
        return( NULL );
      } /* if */
	
      return( rec_ptr );
    } /* if */
  } /* while */

  if ( errors )
    ui_print ( VWARN, "%s build %s not found in build list: %s.\n", errormsg,
		      keyword, blist );

  return ( NULL );
}                                                          /* build base dir */

