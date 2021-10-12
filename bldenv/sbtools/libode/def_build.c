/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: build_base_dir
 *		default_build
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
 * $Log: def_build.c,v $
 * Revision 1.3.4.5  1993/04/28  14:35:49  damon
 * 	CR 463. More pedantic changes
 * 	[1993/04/28  14:34:40  damon]
 *
 * Revision 1.3.4.4  1993/04/27  20:10:07  damon
 * 	CR 463. Made pedantic changes
 * 	[1993/04/27  20:09:56  damon]
 * 
 * Revision 1.3.4.3  1993/04/08  16:30:02  damon
 * 	CR 446. Clean up include files
 * 	[1993/04/08  16:28:07  damon]
 * 
 * Revision 1.3.4.2  1993/01/28  23:38:32  damon
 * 	CR 417. Removed set parameter from default_build
 * 	[1993/01/28  23:38:17  damon]
 * 
 * Revision 1.3.2.6  1992/12/03  17:20:36  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:07:59  damon]
 * 
 * Revision 1.3.2.5  1992/09/24  19:01:26  gm
 * 	CR282: Made more portable to non-BSD systems.
 * 	[1992/09/23  18:21:32  gm]
 * 
 * Revision 1.3.2.4  1992/08/07  15:38:45  damon
 * 	CR 266. Changed salloc to strdup
 * 	[1992/08/07  15:37:07  damon]
 * 
 * Revision 1.3.2.3  1992/07/26  17:30:34  gm
 * 	Fixed to remove warnings when compiling under OSF/1 R1.1.
 * 	[1992/07/14  17:16:25  gm]
 * 
 * Revision 1.3.2.2  1992/02/18  22:04:23  damon
 * 	Changes for LBE removal
 * 	[1992/02/18  21:57:05  damon]
 * 
 * Revision 1.3  1991/12/05  21:04:31  devrcs
 * 	Moved get_rc_value from def_build to par_rc_file
 * 	[91/04/12  15:46:58  damon]
 * 
 * 	Changed sdm to ode; std_defs.h to  odedefs.h
 * 	[91/01/10  11:45:46  randyb]
 * 
 * 	Changed functions to use pointers instead of arrays.
 * 	Made rm_newline a public utility.
 * 	[91/01/08  12:09:49  randyb]
 * 
 * 	Added include of <sys/param.h>
 * 	[90/12/12  09:23:22  mckeen]
 * 
 * 	Changed the include of sys/access.h to be sys/file.h
 * 	[90/12/11  11:43:08  mckeen]
 * 
 * 	Took out an unnecessary variable from get_rc_value, the sb rc file
 * 	Can now call build_base_dir and get the default build_list; this
 * 	was done by making a new Boolean variable
 * 	[90/12/10  13:27:16  randyb]
 * 
 * 	Added ability to determine backing tree by using the rc variable
 * 	[90/12/07  13:56:32  randyb]
 * 
 * 	Added routine build_base_dir to determine the base directory of builds.
 * 	[90/12/06  11:49:58  randyb]
 * 
 * 	Fixed improper test in get_rc_value to prevent core dump
 * 	[90/12/06  09:40:47  damon]
 * 
 * 	Added argument to get_rc_value to conditionally supress
 * 	"line missing" error message.
 * 	[90/12/06  09:22:06  damon]
 * 
 * 	Pre-OSF1.0 changes
 * 	[90/11/13  17:02:35  randyb]
 * 
 * $EndLog$
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

#ifndef lint
static char sccsid[] = "@(#)76  1.1  src/bldenv/sbtools/libode/def_build.c, bldprocess, bos412, GOLDA411a 1/19/94 17:40:51";
#endif /* not lint */

#include <stdio.h>
#include <string.h>
#include <ode/interface.h>
#include <ode/odedefs.h>
#include <ode/parse_rc_file.h>
#include <ode/util.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>

int	default_build ( 
    char     ** base,                            /* base of project's builds */
    char     ** build,                    /* name of project's default build */
    char      * sb_rcfile )                      /* sandbox rc file to parse */

	/* This function retreives the values for each of the arguments:
	   base, build, and set, assuming they are not set to to NULL.
	   The values come from reading the sandbox rc file.  If all
	   goes well, OK is return, else ERROR. */


{
    struct      rcfile    rc_contents;          /* holds contents of rc file */
    int         value = OK;                                  /* return value */

  memset ( (char *)&rc_contents, 0, sizeof ( rc_contents ));

  if ( parse_rc_file ( sb_rcfile, &rc_contents )) {
    ui_print ( VWARN, "unable to parse sandbox rcfile %s.\n", sb_rcfile );
    value = ERROR;
  } /* if */

  if ( value == OK && build != NULL )
    value = get_rc_value ( DEF_BUILD, build, &rc_contents, TRUE );
  
  if ( value == OK && base != NULL )
    value = get_rc_value ( BUILD_BASE, base, &rc_contents, TRUE );
  
  return ( value );
}                                                           /* default build */

char *
build_base_dir ( 
    char      * keyword,       /* identifying build's record in "build_list" */
    struct      rcfile  * contents,             /* holds contents of rc file */
    BOOLEAN     errors,            /* true if error messages should be given */
    BOOLEAN     defaultx ) 			   /* use default build_list */

	/* This function looks for the base directory of the keyword;
	   assuming the keyword is a build.  It looks for it in the
	   file designated by the rc variable, BUILD_LIST, or in the
	   file, DEFAULT_BLIST.  It returns the base directory or NUL
	   if none is found. */


{
    FILE      * bptr;                              /* ptr to file build_list */
    struct      stat statb;                                /* stat structure */
    char        recbuf [ PATH_LEN ],      /* read of records from build_list */
		listbuf [ PATH_LEN ],   /* hold space for name of build_list */
              * blist,           	      /* path and name of build_list */
	      * rec_ptr,                           /* ptr to recbuf contents */
              * token;                         /* holds each token as parsed */
    const char * errormsg = "ERROR: build_base_dir:";   /* head for error msg */

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

      if (( rec_ptr = (char *) strdup ( rec_ptr )) == NULL ) {
	ui_print ( VWARN, "%s unable to strdup base dir.\n", errormsg );
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

