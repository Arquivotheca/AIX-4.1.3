static char sccsid[] = "@(#)51  1.1  src/bldenv/sbtools/libsb/rm_newline.c, bldprocess, bos412, GOLDA411a 4/29/93 12:22:57";
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
void    rm_newline ( string )

        /*
         * Removes the last new line in a string.
         */

    char        * string;                   /* string to remove newline from */

{
    char        * scratch;                  /* points to location of newline */

  if (( scratch = ( index ( string, NEWLINE ))) != NULL )
    *scratch = NUL;
}
