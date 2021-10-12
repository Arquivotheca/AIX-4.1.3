/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: prj_read
 *		prj_write
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
 * $Log: project.c,v $
 * Revision 1.1.6.1  1993/11/05  20:34:24  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/05  20:33:39  damon]
 *
 * Revision 1.1.4.1  1993/09/03  21:47:56  damon
 * 	CR 643. mksb -u now more robust
 * 	[1993/09/03  21:47:34  damon]
 * 
 * Revision 1.1.2.6  1993/05/14  16:50:28  damon
 * 	CR 518. Changed prj_read and prj_write to take full sb path
 * 	[1993/05/14  16:49:02  damon]
 * 
 * Revision 1.1.2.5  1993/05/12  19:51:41  damon
 * 	CR 517. Added subprojects
 * 	[1993/05/12  19:48:45  damon]
 * 
 * Revision 1.1.2.4  1993/04/28  19:22:08  damon
 * 	CR 463. Pedantic changes
 * 	[1993/04/28  19:21:53  damon]
 * 
 * Revision 1.1.2.3  1993/04/09  17:16:02  damon
 * 	CR 446. Remove warnings with added includes
 * 	[1993/04/09  17:15:13  damon]
 * 
 * Revision 1.1.2.2  1993/04/09  14:02:49  damon
 * 	CR 417. Added project routines
 * 	[1993/04/09  14:02:17  damon]
 * 
 * $EndLog$
 */

#ifndef lint
static char sccsid[] = "@(#)12  1.1  src/bldenv/sbtools/libode/project.c, bldprocess, bos412, GOLDA411a 1/19/94 17:42:12";
#endif /* not lint */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ode/errno.h>
#include <ode/odedefs.h>
#include <ode/parse_rc_file.h>
#include <ode/project.h>
#include <ode/public/error.h>
#include <ode/util.h>

BOOLEAN
prj_read ( 

    const char * sb_path,                                 /* name of sandbox */
    const char * dir,                           /* name of base directory */
    char     ** project,
    char     ** sub_project )

	/* This function checks to see if there is a projects file
	   and if that file has the current working directory listed
	   in it.  If it does, it gets the name of the sb rcfile there.
	   It returns TRUE if all this works, FALSE if for any reason
	   it doesn't. */

{
    FILE      * ptr_project;                               /* ptr to rc file */
    char        line [ PATH_LEN ],                            /* misc string */
                srcdir [ PATH_LEN ],                       /* path to sb src */
                projects [ PATH_LEN ],              /* path to projects file */
              * line_ptr,                                   /* misc char ptr */
              * token;                                      /* misc char ptr */
  
  concat ( projects, PATH_LEN, sb_path, "/", PROJECTS, NULL );
  concat ( srcdir, PATH_LEN, sb_path, "/", SRC_DIR, NULL );
  
  if ( access ( projects, R_OK ) != OK )       /* is there projects file */
    return ( FALSE );

  if (( ptr_project = fopen ( projects, READ )) == NULL )
    return ( FALSE );

  while (( line_ptr = fgets ( line, PATH_LEN, ptr_project )) != NULL ) {
    token = nxtarg ( &line_ptr, WHITESPACE );                /* get dir path */

    if ( strncmp ( token, dir, strlen ( token )) == 0 ) {   /* matching dir */
      token = nxtarg ( &line_ptr, WHITESPACE );

      if ( *token == NUL )                                /* no second field */
        return ( FALSE );

      *project = strdup ( token );                              /* got it! */
      token = nxtarg ( &line_ptr, WHITESPACE );
      if ( *token == NUL ) {                              /* no third field */
        *sub_project = NULL;
      } else {
        *sub_project = strdup ( token );
      } /* if */ 
      fclose ( ptr_project );
      return ( TRUE );
    } /* if */
  } /* while */

  fclose ( ptr_project );
  return ( FALSE );
}                                                              /* is project */

ERR_LOG
prj_write ( const char * sb_path, const char * dir, const char * project )
{
  FILE      * ptr_project;                               /* ptr to rc file */
  char        projects [ PATH_LEN ];              /* path to projects file */
  
  concat ( projects, PATH_LEN, sb_path, "/", PROJECTS, NULL );
  if (( ptr_project = fopen ( projects, WRITE )) == NULL )
    return ( err_log ( OE_OPEN, projects ) );
  fputs ( dir, ptr_project );
  fputs ( "\t", ptr_project );
  fputs ( project, ptr_project );
  fputs ( "\n", ptr_project );
  fclose ( ptr_project );
  return ( OE_OK );
} /* end prj_write */
