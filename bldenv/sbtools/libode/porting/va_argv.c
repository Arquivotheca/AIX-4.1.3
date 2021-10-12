/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: va_argv
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
 * $Log: va_argv.c,v $
 * Revision 1.1.8.1  1993/11/10  14:37:21  root
 * 	CR 463. Added include of string.h for strdup
 * 	[1993/11/10  14:37:02  root]
 *
 * Revision 1.1.6.2  1993/05/05  15:30:22  marty
 * 	Include header files interface.h and unistd.h
 * 	[1993/05/05  15:30:09  marty]
 * 
 * Revision 1.1.4.4  1992/12/03  17:22:00  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:08:47  damon]
 * 
 * Revision 1.1.4.3  1992/09/15  19:39:06  damon
 * 	CR 266. Changed salloc to strdup
 * 	[1992/09/15  19:38:47  damon]
 * 
 * Revision 1.1.4.2  1992/06/15  17:58:47  damon
 * 	Taken from 2.1.1
 * 	[1992/06/15  16:18:42  damon]
 * 
 * Revision 1.1.2.3  1992/03/24  14:55:41  damon
 * 	Removed gratuitous debug message
 * 	[1992/03/24  14:54:59  damon]
 * 
 * Revision 1.1.2.2  1992/03/24  00:45:48  damon
 * 	Initial Version. Taken from interface.c
 * 	[1992/03/24  00:45:19  damon]
 * 
 * $EndLog$
 */

#ifndef lint
static char sccsid[] = "@(#)07  1.1  src/bldenv/sbtools/libode/porting/va_argv.c, bldprocess, bos412, GOLDA411a 1/19/94 17:42:02";
#endif /* not lint */

#include <ode/interface.h>
#include <ode/odedefs.h>
#include <string.h>
#include <unistd.h>
#include <varargs.h>

char ** va_argv ( ap )
va_list ap;
{
    int           alloc_arg = 10,                     /* available arguments */
                  ct;                                        /* misc counter */
    char          * token;               /* holds each option */
  int argc;
  char ** argv;

  if (( argv = (char **) calloc ( 10, sizeof (char *))) == NULL ) {
    ui_print ( VFATAL,  "could not calloc space for rc file options.\n" );
    return ( FALSE );
  } /* if */

  argc = 0;            /* Allocate 10 args, realloc later if more needed. */

  while ( ( token = va_arg ( ap, char *) ) != (char *) 0 ) {

    rm_newline ( token );

    if ( argc >= alloc_arg ) {
      alloc_arg += 10;

      if (( argv = (char **) realloc
                      ( argv, sizeof (char *) * alloc_arg )) == NULL )
        return ( FALSE );
    } /* if */

    if (( argv[argc] = strdup ( token )) == NULL )
      return ( FALSE );

    argc++;
  }
  if ( argc >= alloc_arg ) {
    alloc_arg += 10;

    if (( argv = (char **) realloc
                    ( argv, sizeof (char *) * alloc_arg )) == NULL )
      return ( FALSE );
  } /* if */

  argv[argc] = NULL;
  return argv;
} /* va_argv */
