/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: sets_access
 *		sets_fullname
 *		sets_write
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
 */
/*
 * HISTORY
 * $Log: sets.c,v $
 * Revision 1.1.4.2  1993/11/08  17:58:57  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/08  16:08:42  damon]
 *
 * Revision 1.1.4.1  1993/11/05  20:34:43  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/05  20:33:51  damon]
 * 
 * Revision 1.1.2.4  1993/04/28  19:22:06  damon
 * 	CR 463. Pedantic changes
 * 	[1993/04/28  19:21:51  damon]
 * 
 * Revision 1.1.2.3  1993/04/09  17:16:00  damon
 * 	CR 446. Remove warnings with added includes
 * 	[1993/04/09  17:15:11  damon]
 * 
 * Revision 1.1.2.2  1993/04/09  14:03:43  damon
 * 	CR 417. Moving set routines to this file
 * 	[1993/04/09  14:03:35  damon]
 * 
 * $EndLog$
 */

#ifndef lint
static char sccsid[] = "@(#)23  1.1  src/bldenv/sbtools/libode/sets.c, bldprocess, bos412, GOLDA411a 1/19/94 17:42:35";
#endif /* not lint */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ode/interface.h>
#include <ode/odedefs.h>
#include <ode/parse_rc_file.h>
#include <ode/sets.h>
#include <ode/util.h>
#include <sys/param.h>

BOOLEAN
sets_access ( char * basedir, char * sb )
{
  char set_loc [MAXPATHLEN];

  concat ( set_loc, PATH_LEN, basedir, "/", sb, "/", SET_RC, NULL );
  return ( access ( set_loc, F_OK ) == 0 );
} /* end sets_access */

void
sets_write  ( 
    const char * sb,                                          /* sandbox name */
    const char * sbbase,                             /* holds path to sandbox */
    const char * setname,                               /* holds full setname */
    const char * setdir )                            /* name of set directory */

        /* This procedure checks to see if the set is in the sets
           file.  If it is, it handles setdir.  If it isn't, it
           puts it in and calls a bco command to get the lock
           file. */


{
    FILE      * ptr_file;                                  /* ptr to rc file */
    char        file [ PATH_LEN ],           /* holds set file path and name */
              * tsetdir = NULL;                         /* temporary set dir */
    char * rc_file = NULL;
    char * setname2;
    char * sb2;

    setname2 = (char *)setname;
    sb2 = (char *)sb;
  if ( current_set ( &setname2, &tsetdir, &sb2, &rc_file ) == ERROR ) {
#ifdef notdef
    check_setdir ( setdir );
#endif
    concat ( file, PATH_LEN, sbbase, "/", sb, "/", SET_RC, NULL );

    ui_print ( VNORMAL, "creating new set: %s, setdir: %s\n",
                         setname, setdir );

    if (( ptr_file = fopen ( file, APPEND )) == NULL )
      uquit ( ERROR, FALSE, "\tcannot write to set rcfile %s.\n", file );

    fprintf ( ptr_file, "%s %s %s\n", SET_KEY, setname, setdir );
    fclose ( ptr_file );
#ifdef notdef
  } else {
    if ( ui_is_set ( SETDIR_OP ))
      check_setdir ( setdir );
    else
      *setdir = tsetdir;
#endif
  } /* if */
}/* end sets_write */

void
sets_fullname ( char * setn, char * user, char ** symbolic_name )

        /* This procedure prepends the user name to the setname
           if the setname does not start with a capital letter
           and it isn't already there.  It puts the final setname
           in setinfo. */

{
    char        tmp_name [ NAME_LEN ],                        /* misc string */
                tmp_name2 [ NAME_LEN ],                        /* misc string */
              * ptr;                                  /* point to env string */

  if (( ptr = concat ( tmp_name, NAME_LEN , user, "_", NULL ))
            == NULL)
    uquit ( ERROR, FALSE, "\tno room in buffer for '%s_'\n", user );

  if ((( *setn < 'A' ) || ( *setn > 'Z' )) &&
      ( strncmp ( tmp_name, setn, ptr - tmp_name ))) {
    concat ( tmp_name2, NAME_LEN, tmp_name, setn, NULL );
    *symbolic_name = strdup ( tmp_name2 );
  } else {
    *symbolic_name = strdup ( setn );
  } /* if */
} /* end sets_fullname */
