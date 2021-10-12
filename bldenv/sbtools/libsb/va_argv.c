static char sccsid[] = "@(#)67  1.1  src/bldenv/sbtools/libsb/va_argv.c, bldprocess, bos412, GOLDA411a 4/29/93 12:25:29";
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

#include <ode/odedefs.h>
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

    if (( argv[argc] = salloc ( token )) == NULL )
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
