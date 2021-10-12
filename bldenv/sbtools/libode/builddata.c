/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: bld_conf_read
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
 * $Log: builddata.c,v $
 * Revision 1.1.7.1  1993/11/03  20:40:19  damon
 * 	CR 463. More pedantic
 * 	[1993/11/03  20:37:59  damon]
 *
 * Revision 1.1.5.1  1993/09/03  17:43:47  damon
 * 	CR 612. Search for and use Buildconf and Builconf.exp separately
 * 	[1993/09/03  17:43:40  damon]
 * 
 * Revision 1.1.2.11  1993/06/02  14:08:20  damon
 * 	CR 517. Cleaned up subprojects wrt sb.conf and sc.conf
 * 	[1993/06/02  14:08:15  damon]
 * 
 * Revision 1.1.2.10  1993/06/02  14:06:46  damon
 * 	CR 517. Cleaned up subprojects wrt sb.conf and sc.conf
 * 	[1993/06/02  14:06:34  damon]
 * 
 * Revision 1.1.2.9  1993/05/27  15:14:20  damon
 * 	CR 548. Get backing_build from sb.conf files, not link
 * 	[1993/05/27  15:10:09  damon]
 * 
 * Revision 1.1.2.8  1993/05/27  14:45:36  damon
 * 	CR 541. Only use Buildconf.exp if there is a backing chain
 * 	[1993/05/27  14:45:25  damon]
 * 
 * Revision 1.1.2.7  1993/04/28  15:43:13  damon
 * 	CR 463. Pedantic changes
 * 	[1993/04/28  15:43:07  damon]
 * 
 * Revision 1.1.2.6  1993/04/09  17:22:57  damon
 * 	CR 446. Remove warnings with added includes
 * 	[1993/04/09  17:22:02  damon]
 * 
 * Revision 1.1.2.5  1993/04/09  14:26:55  damon
 * 	CR 417. Made routines project aware
 * 	[1993/04/09  14:26:41  damon]
 * 
 * Revision 1.1.2.4  1993/03/05  16:08:21  damon
 * 	CR 417. Removed interface.h
 * 	[1993/03/05  16:07:37  damon]
 * 
 * Revision 1.1.2.3  1993/03/05  16:03:15  damon
 * 	CR 417. Added ode/odedefs.h
 * 	[1993/03/05  16:02:48  damon]
 * 
 * Revision 1.1.2.2  1993/02/01  21:55:07  damon
 * 	CR 417. Created build_conf_read
 * 	[1993/02/01  21:50:14  damon]
 * 
 * $EndLog$
 */

#ifndef lint
static char sccsid[] = "@(#)73  1.1  src/bldenv/sbtools/libode/builddata.c, bldprocess, bos412, GOLDA411a 1/19/94 17:40:44";
#endif /* not lint */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ode/builddata.h>
#include <ode/odedefs.h>
#include <ode/parse_rc_file.h>
#include <ode/sandboxes.h>
#include <ode/util.h>
#include <sys/param.h>

void
bld_conf_read ( char * sandbox_base, char * project, struct rcfile * rc )
{
  char * bases [10];
  int num_bases=0;
  char * new_base;
  char rc_name [MAXPATHLEN];
  char rc_name2 [MAXPATHLEN];
  char * s;
  struct field *field_p;
  struct arg_list *args_p;
  int i;
  BOOLEAN has_backing_chain = FALSE;
  struct rcfile sbrc;

  bases [num_bases++] = strdup ( sandbox_base );
  for (;;) {
    sb_conf_read ( &sbrc, bases[num_bases-1], project, NULL );
    if ( get_rc_value ( "backing_build", &new_base, &sbrc, FALSE ) == OK ) {
      bases [num_bases++] = new_base;
      has_backing_chain = TRUE;
    } else
      break;
    /* if */
  } /* for */
  find_field ( rc, "sandbox_base", &field_p, 1 );
  create_arglist ( field_p, &args_p );
  append_arg ( bases[num_bases-1], args_p );
  get_rc_value ( "sandbox_base", &s, rc, TRUE );
  for ( i = 0; i < num_bases; i++ ) {
    concat ( rc_name, sizeof(rc_name), bases [i], "/src/", project,
                      "/Buildconf", NULL );
    if ( access ( rc_name, F_OK ) == 0 )
      break;
    /* if */
  } /* for */
  for ( i = 0; i < num_bases; i++ ) {
    concat ( rc_name2, sizeof(rc_name2), bases [i], "/src/", project,
                      "/Buildconf.exp", NULL );
    if ( access ( rc_name2, F_OK ) == 0 )
      break;
    /* if */
  } /* for */
  parse_rc_file ( rc_name, rc );
  if ( has_backing_chain ) {
    for ( i = num_bases-1; i > 0; i-- ) {
      find_field ( rc, "sandbox_base", &field_p, 1 );
      field_p -> args = NULL;
      create_arglist ( field_p, &args_p );
      append_arg ( bases[i-1], args_p );
      get_rc_value ( "sandbox_base", &s, rc, TRUE );
      parse_rc_file ( rc_name2, rc );
    } /* for */
  } /* if */
} /* bld_conf_read */
