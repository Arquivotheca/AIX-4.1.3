/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: ATOI
 *		BOOLEAN_to_str
 *		defined
 *		sci_check_out_client
 *		sci_client_latest_rev_list
 *		sci_server_bcs_opts
 *		sci_show_log_list_client
 *		sci_submit_client
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
 *  
 * Copyright (c) 1992 Carnegie Mellon University 
 * All Rights Reserved. 
 *  
 * Permission to use, copy, modify and distribute this software and its 
 * documentation is hereby granted, provided that both the copyright 
 * notice and this permission notice appear in all copies of the 
 * software, derivative works or modified versions, and any portions 
 * thereof, and that both notices appear in supporting documentation. 
 *  
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" 
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR 
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE. 
 *  
 * Carnegie Mellon requests users of this software to return to 
 *  
 * Software Distribution Coordinator  or  Software_Distribution@CS.CMU.EDU 
 * School of Computer Science 
 * Carnegie Mellon University 
 * Pittsburgh PA 15213-3890 
 *  
 * any improvements or extensions that they make and grant Carnegie Mellon 
 * the rights to redistribute these changes. 
 */
/*
 * HISTORY
 * $Log: sci_client.c,v $
 * Revision 1.1.6.4  1993/11/16  19:34:02  damon
 * 	CR 816. Fixed BCSTEMP declaration
 * 	[1993/11/16  19:33:43  damon]
 *
 * Revision 1.1.6.3  1993/11/10  16:57:05  root
 * 	CR 463. Cast stdrup paramater to (char *)
 * 	[1993/11/10  16:56:07  root]
 * 
 * Revision 1.1.6.2  1993/11/03  20:41:09  damon
 * 	CR 463. More pedantic
 * 	[1993/11/03  20:38:35  damon]
 * 
 * Revision 1.1.6.1  1993/10/29  13:04:11  damon
 * 	CR 766. Renamed #odexm to #clntmp and moved to sb tmp dir
 * 	[1993/10/29  13:04:05  damon]
 * 
 * Revision 1.1.4.4  1993/09/29  14:30:26  root
 * 	rios porting errors
 * 	[1993/09/29  14:22:55  root]
 * 
 * Revision 1.1.4.3  1993/09/22  16:04:30  damon
 * 	CR 675. Passing of -tag info between client/server restored
 * 	[1993/09/22  16:04:04  damon]
 * 
 * Revision 1.1.4.2  1993/09/07  16:34:33  damon
 * 	CR 625. Fixed bsubmit -info
 * 	[1993/09/07  16:33:37  damon]
 * 
 * Revision 1.1.4.1  1993/09/03  16:48:06  damon
 * 	CR 639. Fixed printing of Tlog path
 * 	[1993/09/03  16:47:19  damon]
 * 
 * Revision 1.1.2.30  1993/05/27  19:02:22  marty
 * 	CR # 558 - get bulding on rios_aix
 * 	[1993/05/27  19:02:11  marty]
 * 
 * Revision 1.1.2.29  1993/05/26  18:45:36  damon
 * 	CR 552. Pass ui_ver_level() to bsubmit_s
 * 	[1993/05/26  18:44:30  damon]
 * 
 * Revision 1.1.2.28  1993/05/25  17:58:39  damon
 * 	CR 539
 * 	Added in_submit_stage, resub_time, resub_date to sci_submit_client
 * 	[1993/05/25  17:57:21  damon]
 * 
 * Revision 1.1.2.27  1993/05/24  17:46:29  damon
 * 	CR 501. For resub, list of files returned to client for outdate
 * 	[1993/05/24  17:45:44  damon]
 * 
 * Revision 1.1.2.26  1993/05/24  16:13:33  damon
 * 	CR 484. Now gets log information back to client
 * 	[1993/05/24  16:06:12  damon]
 * 
 * Revision 1.1.2.25  1993/05/20  15:47:37  damon
 * 	CR 532. Covering bsubmit problem
 * 	[1993/05/20  15:47:18  damon]
 * 
 * 	CR 532. Made outdating optional for bsubmit
 * 	 *
 * 	Revision 1.1.2.24  1993/05/20  15:42:58  damon
 * 	CR 532. Made outdating optional for bsubmit
 * 	[1993/05/20  15:42:04  damon]
 * 	 *
 * 	Revision 1.1.2.23  1993/05/11  21:11:56  damon
 * 	CR 468. Made file locking customizeable
 * 	[1993/05/11  21:10:58  damon]
 * 	 *
 * 	Revision 1.1.2.22  1993/05/11  17:39:17  marty
 * 	CR # 480 - Add support for "-r" option.
 * 	[1993/05/11  17:39:06  marty]
 * 
 * Revision 1.1.2.21  1993/05/05  18:56:26  damon
 * 	CR 489. Pass on info about adding/removing from hold file
 * 	[1993/05/05  18:55:32  damon]
 * 
 * Revision 1.1.2.20  1993/05/05  12:59:23  damon
 * 	CR 478. Changed subscript for av from 1000 to 4
 * 	[1993/05/05  12:59:15  damon]
 * 
 * Revision 1.1.2.19  1993/05/04  19:46:21  damon
 * 	CR 486. Added size parameter to oxm_gets
 * 	[1993/05/04  19:45:57  damon]
 * 
 * Revision 1.1.2.18  1993/05/04  17:02:43  damon
 * 	CR 463. More pedantic changes
 * 	[1993/05/04  17:02:35  damon]
 * 
 * Revision 1.1.2.17  1993/05/03  20:11:06  damon
 * 	CR 473. outdating now removes co locks from hold file
 * 	[1993/05/03  20:11:00  damon]
 * 
 * Revision 1.1.2.16  1993/04/30  20:45:25  marty
 * 	sci_server_bcs_opts() routine was opeing more than one
 * 	temporary file (overwriting previous opens).
 * 	[1993/04/30  20:45:02  marty]
 * 
 * Revision 1.1.2.15  1993/04/30  19:47:58  damon
 * 	CR 471. Strict file lockingg only locks on initial bco
 * 	[1993/04/30  19:47:39  damon]
 * 
 * Revision 1.1.2.14  1993/04/29  14:52:04  damon
 * 	CR 463. Pedantic changes
 * 	[1993/04/29  14:51:54  damon]
 * 
 * Revision 1.1.2.13  1993/04/14  19:17:45  damon
 * 	CR 193. Put strict locking code into serer side
 * 	[1993/04/14  19:17:31  damon]
 * 
 * Revision 1.1.2.12  1993/04/13  17:22:58  damon
 * 	CR 436. Added handling of merges for bsubmit
 * 	[1993/04/13  17:19:37  damon]
 * 
 * Revision 1.1.2.11  1993/04/08  21:03:54  damon
 * 	CR 446. Clean up include files
 * 	[1993/04/08  21:03:18  damon]
 * 
 * Revision 1.1.2.10  1993/03/31  20:12:57  damon
 * 	CR 436. Now uses temp files
 * 	[1993/03/31  20:12:50  damon]
 * 
 * Revision 1.1.2.9  1993/03/29  18:03:21  marty
 * 	Changed arguments for sci_client_latest_rev_list ().
 * 	[1993/03/29  18:03:06  marty]
 * 
 * Revision 1.1.2.8  1993/03/24  20:49:50  damon
 * 	CR 436. Added remove and resub options
 * 	[1993/03/24  20:32:22  damon]
 * 
 * Revision 1.1.2.7  1993/03/19  16:33:04  marty
 * 	Added sci_server_bcs_opts() for bcs client/server.
 * 	[1993/03/19  16:32:27  marty]
 * 
 * Revision 1.1.2.6  1993/03/18  18:25:44  marty
 * 	Added sci_client_latest_rev_list() for bstat.
 * 	[1993/03/18  18:25:19  marty]
 * 
 * Revision 1.1.2.5  1993/03/17  16:13:33  damon
 * 	CR 436. Pass back return status
 * 	[1993/03/17  16:01:45  damon]
 * 
 * Revision 1.1.2.4  1993/03/15  21:07:41  damon
 * 	CR 436. Added more info for submit log
 * 	[1993/03/15  21:06:57  damon]
 * 
 * Revision 1.1.2.3  1993/03/15  20:25:04  damon
 * 	CR 436. Added sci_submit_client
 * 	[1993/03/15  20:24:55  damon]
 * 
 * Revision 1.1.2.2  1993/03/04  21:20:37  damon
 * 	CR 436. Initially just to send blog requests to blog_s
 * 	[1993/03/04  21:19:50  damon]
 * 
 * $EndLog$
 */
#ifndef lint
static char sccsid[] = "@(#)21  1.1  src/bldenv/sbtools/libode/sci_client.c, bldprocess, bos412, GOLDA411a 1/19/94 17:42:31";
#endif /* not lint */

#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: sci_client.c,v $ $Revision: 1.1.6.4 $ (OSF) $Date: 1993/11/16 19:34:02 $";
#endif

#include <stdio.h>
#include <unistd.h>
#include <ode/errno.h>
#include <ode/odexm.h>
#include <ode/odedefs.h>
#include <ode/interface.h>
#include <ode/public/error.h>
#include <ode/public/odexm_client.h>
#include <ode/sci.h>
#include <ode/versions.h>
#include <ode/util.h>
#include <string.h>
#include <sys/param.h>

extern OXM_CNXTN rcs_monitor;
extern char * BCSTEMP;

#define RCS_MONITOR 0

#define ATOI(n, p, s) \
    (s) = 0; \
    (n) = 0; \
    if ('0' > *(p) || *(p) > '9') \
        (s) = -1; \
    while ('0' <= *(p) && *(p) <= '9') { \
        (n) = (n) * 10 + (*(p) - '0'); \
        (p)++; \
    }

static const char * one = "1";
static const char * zero = "0";

STATIC const char *
BOOLEAN_to_str ( BOOLEAN b )
{
  if ( b )
    return ( one );
  else
    return ( zero );
  /* if */
}

ERR_LOG
sci_show_log_list_client ( SCI_LIST file_set, char * rev, BOOLEAN all_revs,
                           BOOLEAN lock_users, BOOLEAN header, BOOLEAN rcs_path,
                           BOOLEAN long_format )
{
  ERR_LOG log;
  int i;
  SCI_ELEM sci_ptr;

  const char * av [4];
  FILE * odexm_args;
  int status;
  char buf[MAXPATHLEN];
  char tempdir[MAXPATHLEN];
  char tempfile[MAXPATHLEN];
  char tmp_buf[MAXPATHLEN];

  concat ( tmp_buf, sizeof(tmp_buf), BCSTEMP, "/#clntmpXXXXXX", NULL );
  if ( ( log = oxm_open ( &rcs_monitor, RCS_MONITOR ) ) != OE_OK )
    return ( log );
  /* if */
  opentemp ( tmp_buf, tempdir );
  concat ( tempfile, sizeof(tempfile), tempdir, "/odexm_args", NULL );
  i = 0;
  av [i++] = "-t1";
  av [i++] = "1";
  av [i++] = "blog_s";
  av [i++] = tempfile;
  odexm_args = fopen ( tempfile, "w" );
  fprintf ( odexm_args, "%s\n", BUILD_VERSION );
  fprintf ( odexm_args, "%s\n", rev );
  fprintf ( odexm_args, "%s\n", BOOLEAN_to_str ( all_revs) );
  fprintf ( odexm_args, "%s\n", BOOLEAN_to_str ( lock_users) );
  fprintf ( odexm_args, "%s\n", BOOLEAN_to_str ( header) );
  fprintf ( odexm_args, "%s\n", BOOLEAN_to_str ( rcs_path) );
  fprintf ( odexm_args, "%s\n", BOOLEAN_to_str ( long_format) );
  for ( sci_ptr = sci_first ( file_set ); sci_ptr != NULL;
        sci_ptr = sci_next ( sci_ptr ) )
    fprintf ( odexm_args, "%s\n", sci_ptr -> name );
  /* for */
  fclose ( odexm_args );
  log = oxm_runcmd ( rcs_monitor, i, av, NULL );
  while ( oxm_gets( rcs_monitor, buf, sizeof(buf), &log ) != NULL) {
    rm_newline ( buf );
    ui_print ( VNORMAL, "%s\n", buf );
  } /* while */
  log = oxm_endcmd ( rcs_monitor, &status );
  if ( ( log = oxm_close ( rcs_monitor ) ) != OE_OK )
    return ( log );
  /* if */
  unlink ( tempfile );
  rmdir ( tempdir );
  return ( log );
} /* end sci_show_log_list_client */

ERR_LOG
sci_submit_client ( SCI_LIST file_set, SCI_LIST * file_set2,
                    const char * user_set, const char * user_sandbox,
                    const char * submit_set, const char * expanded_config,
                    const char * submit_user_name, const char * defect_number,
                    BOOLEAN already_held, BOOLEAN info, const char * tag,
                    BOOLEAN resub, BOOLEAN rmove,
                    BOOLEAN outdate, const char * stime, const char * sdate,
                    BOOLEAN * in_submit_stage, char * rtime, char * rdate )
{
  ERR_LOG log;
  int i;
  SCI_ELEM sci_ptr;
  char tempdir[MAXPATHLEN];
  char tempfile[MAXPATHLEN];
  const char * av [16];
  FILE * odexm_args;
  int status;
  char buf[MAXPATHLEN];
  char log_file[MAXPATHLEN];
  int elems;
  char * buf_ptr;
  int tempslot;
  char tmp_buf[MAXPATHLEN];

  concat ( tmp_buf, sizeof(tmp_buf), BCSTEMP, "/#clntmpXXXXXX", NULL );

  if ( ( log = oxm_open ( &rcs_monitor, RCS_MONITOR ) ) != OE_OK )
    return ( log );
  /* if */
  i = 0;
  opentemp ( tmp_buf, tempdir );
  concat ( tempfile, sizeof(tempfile), tempdir, "/odexm_args", NULL );
  av [i++] = "-t2";
  tempslot = i++;
  av [i++] = "bsubmit_s";
  av [i++] = ui_ver_switch();
  av [tempslot] = "2";
  if ( info ) {
    av [tempslot] = "3";
    av [i++] = "-info";
  } /* if */
  if ( tag != NULL ) {
    av [tempslot] = "4";
    av [i++] = "-tag";
    av [i++] = strdup ( (char *)tag );
  } /* if */
  if ( tag != NULL && info ) {
    av [tempslot] = "5";
  } /* if */
  av [i++] = tempfile;
  av [i] = NULL;
  odexm_args = fopen ( tempfile, "w" );
  fprintf ( odexm_args, "%s\n", BUILD_VERSION );
  fprintf ( odexm_args, "%s\n", user_set );
  fprintf ( odexm_args, "%s\n", user_sandbox );
  fprintf ( odexm_args, "%s\n", submit_set );
  fprintf ( odexm_args, "%s\n", expanded_config );
  fprintf ( odexm_args, "%s\n", submit_user_name );
  fprintf ( odexm_args, "%s\n", defect_number );
  fprintf ( odexm_args, "%s\n", BOOLEAN_to_str ( already_held ) );
  fprintf ( odexm_args, "%s\n", BOOLEAN_to_str ( resub ) );
  fprintf ( odexm_args, "%s\n", BOOLEAN_to_str ( rmove ) );
  fprintf ( odexm_args, "%s\n", BOOLEAN_to_str ( outdate ) );
  if ( stime != NULL )
    fprintf ( odexm_args, "%s\n", stime );
  else
    fprintf ( odexm_args, "\n" );
  /* if */
  if ( sdate != NULL )
    fprintf ( odexm_args, "%s\n", sdate );
  else
    fprintf ( odexm_args, "\n" );
  /* if */
  if ( !resub && !rmove )
    for ( sci_ptr = sci_first ( file_set ); sci_ptr != NULL;
          sci_ptr = sci_next ( sci_ptr ) )
      fprintf ( odexm_args, "%s\n", sci_ptr -> name );
    /* for */
  /* if */
  fclose ( odexm_args );
  log = oxm_runcmd ( rcs_monitor, i, av, NULL );
  oxm_gets( rcs_monitor, buf, sizeof(buf), &log );
  rm_newline ( buf );
  strcpy ( rtime, buf );
  oxm_gets( rcs_monitor, buf, sizeof(buf), &log );
  rm_newline ( buf );
  strcpy ( rdate, buf );
  if ( resub ) {
    oxm_gets( rcs_monitor, buf, sizeof(buf), &log );
    rm_newline ( buf );
    if ( strcmp ( buf, "outdate_list" ) == 0 ) {
      oxm_gets( rcs_monitor, buf, sizeof(buf), &log );
      rm_newline ( buf );
      buf_ptr = buf;
      ATOI ( elems, buf_ptr, status );
      sci_new_list ( file_set2 );
      for ( i = 0; i < elems; i++ ) {
        oxm_gets( rcs_monitor, buf, sizeof(buf), &log );
        rm_newline ( buf );
        sci_add_to_list_as_is ( *file_set2, buf );
      } /* for */
    } else {
      ui_print ( VNORMAL, "%s\n", buf );
    } /* if */
  } /* if */
  while ( oxm_gets( rcs_monitor, buf, sizeof(buf), &log ) != NULL) {
    rm_newline ( buf );
    if ( strcmp ( buf, "Submitting files" ) == 0 ) {
      *in_submit_stage = TRUE;
    } /* if */
    ui_print ( VNORMAL, "%s\n", buf );
  } /* while */
  log = oxm_endcmd ( rcs_monitor, &status );
  if ( ( log = oxm_close ( rcs_monitor ) ) != OE_OK )
    return ( log );
  /* if */

  /* FIXME: Should have a way to transmit ode errors across net. */
  if ( status != 0 ) {
    if ( status == OE_MERGEREQ ) {
      sci_new_list ( file_set2 );
      odexm_args = fopen ( tempfile, "r" );
      while ( fgets ( buf, sizeof ( buf ), odexm_args ) != NULL ) {
        rm_newline ( buf );
        sci_add_to_list_as_is ( *file_set2, buf );
      } /* while */
      fclose ( odexm_args );
    } /* if */
    unlink ( tempfile );
    rmdir ( tempdir );
    return ( err_log ( status ) );
  } /* if */
  concat ( log_file, sizeof(log_file), rtime, ".Tlog", NULL );
  rename ( tempfile, log_file );
  rmdir ( tempdir );
  getcwd ( buf, sizeof(buf) );
  ui_print ( VNORMAL, "\n*** Please mail the log:\n    %s/%s\n",
                      buf, log_file );
  ui_print ( VNORMAL,
          "*** to the appropriate people in your project then remove.\n\n" );
  return ( log );
} /* end sci_submit_client */

ERR_LOG
sci_client_latest_rev_list ( SCI_LIST file_set, BOOLEAN rev_option,
                             const char * rev_label, BOOLEAN wantfilename ) 
{
  ERR_LOG log;
  int i;
  SCI_ELEM sci_ptr;

  const char * av [4];
  FILE * odexm_args;
  int status;
  char buf[MAXPATHLEN];
  char tempdir[MAXPATHLEN];
  char tempfile[MAXPATHLEN];
  char tmp_buf[MAXPATHLEN];

  concat ( tmp_buf, sizeof(tmp_buf), BCSTEMP, "/#clntmpXXXXXX", NULL );

  if ( ( log = oxm_open ( &rcs_monitor, RCS_MONITOR ) ) != OE_OK )
    return ( log );
  /* if */
  opentemp ( tmp_buf, tempdir );
  concat ( tempfile, sizeof(tempfile), tempdir, "/odexm_args", NULL );
  i = 0;
  av [i++] = "-t1";
  av [i++] = "1";
  av [i++] = "bstat_s";
  av [i++] = tempfile;
  odexm_args = fopen ( tempfile, "w" );
  fprintf ( odexm_args, "%s\n", BUILD_VERSION );
  fprintf ( odexm_args, "%s\n", BOOLEAN_to_str ( rev_option ) );
  if (rev_option)
  	fprintf ( odexm_args, "%s\n", rev_label );
  fprintf ( odexm_args, "%s\n", BOOLEAN_to_str ( wantfilename ) );
  for ( sci_ptr = sci_first ( file_set ); sci_ptr != NULL;
        sci_ptr = sci_next ( sci_ptr ) )
    fprintf ( odexm_args, "%s\n", sci_ptr -> name );
  /* for */
  fclose ( odexm_args );
  log = oxm_runcmd ( rcs_monitor, i, av, NULL );
  while ( oxm_gets( rcs_monitor, buf, sizeof(buf), &log ) != NULL) {
    rm_newline ( buf );
    ui_print ( VNORMAL, "%s\n", buf );
  } /* while */
  log = oxm_endcmd ( rcs_monitor, &status );
  if ( ( log = oxm_close ( rcs_monitor ) ) != OE_OK )
    return ( log );
  /* if */
  unlink ( tempfile );
  rmdir ( tempdir );
  return ( log );
} 

ERR_LOG
sci_server_bcs_opts ( SCI_LIST file_set, BOOLEAN revision_option,
		      BOOLEAN outdate_option,
                      const char * symbolic_name, const char * submit_set,
                      BOOLEAN leader_option,
                      const char * leader, BOOLEAN big_symname_option,
                      const char * big_symname, BOOLEAN small_symname_option,
                      const char * small_symname )
{
  ERR_LOG log;
  int i;
  SCI_ELEM sci_ptr;

  const char * av [4];
  FILE * odexm_args;
  int status;
  char buf[MAXPATHLEN];
  char tempdir[MAXPATHLEN];
  char tempfile[MAXPATHLEN];
  char tmp_buf[MAXPATHLEN];

  concat ( tmp_buf, sizeof(tmp_buf), BCSTEMP, "/#clntmpXXXXXX", NULL );

  if ( ( log = oxm_open ( &rcs_monitor, RCS_MONITOR ) ) != OE_OK )
    return ( log );
  /* if */
  opentemp ( tmp_buf, tempdir );
  concat ( tempfile, sizeof(tempfile), tempdir, "/odexm_args", NULL );
  i = 0;
  av [i++] = "-t1";
  av [i++] = "1";
  av [i++] = "bcs_s";
  av [i++] = tempfile;
  odexm_args = fopen ( tempfile, "w" );
  fprintf ( odexm_args, "%s\n", BUILD_VERSION );
  fprintf ( odexm_args, "%s\n", BOOLEAN_to_str ( revision_option ) );
  fprintf ( odexm_args, "%s\n", BOOLEAN_to_str ( outdate_option ) );
  if ((outdate_option) || (revision_option))
  	fprintf ( odexm_args, "%s\n", symbolic_name );
  if ((outdate_option) && (revision_option == FALSE)) {
  	fprintf ( odexm_args, "%s\n", submit_set );
  } /* if */
  fprintf ( odexm_args, "%s\n", BOOLEAN_to_str ( leader_option ) );
  if (leader_option)
  	fprintf ( odexm_args, "%s\n", leader );
  fprintf ( odexm_args, "%s\n", BOOLEAN_to_str ( big_symname_option ) );
  if (big_symname_option )
  	fprintf ( odexm_args, "%s\n", big_symname );
  fprintf ( odexm_args, "%s\n", BOOLEAN_to_str ( small_symname_option ) );
  if (small_symname_option)
  	fprintf ( odexm_args, "%s\n", small_symname );
  for ( sci_ptr = sci_first ( file_set ); sci_ptr != NULL;
        sci_ptr = sci_next ( sci_ptr ) )
    fprintf ( odexm_args, "%s\n", sci_ptr -> name );
  /* for */
  fclose ( odexm_args );
  log = oxm_runcmd ( rcs_monitor, i, av, NULL );
  while ( oxm_gets( rcs_monitor, buf, sizeof(buf), &log ) != NULL) {
    rm_newline ( buf );
    ui_print ( VNORMAL, "%s\n", buf );
  } /* while */
  log = oxm_endcmd ( rcs_monitor, &status );
  if ( ( log = oxm_close ( rcs_monitor ) ) != OE_OK )
    return ( log );
  /* if */
  unlink ( tempfile );
  rmdir ( tempdir );
  return ( log );
}

/*
 * holdline == NULL indicates that the files should be removed from
 * the hold file.
 */
int 
sci_check_out_client ( SCI_LIST file_set, const char * holdline, BOOLEAN add,
                       const char * submit_set, ERR_LOG * log )
{
  int i;
  SCI_ELEM sci_ptr;

  const char * av [4];
  FILE * odexm_args;
  int status;
  char buf[MAXPATHLEN];
  char tempdir[MAXPATHLEN];
  char tempfile[MAXPATHLEN];
  char tmp_buf[MAXPATHLEN];

  concat ( tmp_buf, sizeof(tmp_buf), BCSTEMP, "/#clntmpXXXXXX", NULL );

  if ( ( *log = oxm_open ( &rcs_monitor, RCS_MONITOR ) ) != OE_OK )
    return ( FALSE );
  /* if */
  opentemp ( tmp_buf, tempdir );
  concat ( tempfile, sizeof(tempfile), tempdir, "/odexm_args", NULL );
  i = 0;
  av [i++] = "-t1";
  av [i++] = "1";
  av [i++] = "bco_s";
  av [i++] = tempfile;
  odexm_args = fopen ( tempfile, "w" );
  fprintf ( odexm_args, "%s\n", BUILD_VERSION );
  fprintf ( odexm_args, "%s\n", holdline );
  fprintf ( odexm_args, "%s\n", BOOLEAN_to_str ( add ) );
  fprintf ( odexm_args, "%s\n", submit_set );
  for ( sci_ptr = sci_first ( file_set ); sci_ptr != NULL;
        sci_ptr = sci_next ( sci_ptr ) )
   /*
    * Only need to put co locks on files that aren't already in
    *  the user's private branch
    */
    if ( sci_ptr -> ver_user == NULL || holdline == NULL )
      fprintf ( odexm_args, "%s\n", sci_ptr -> name );
    /* if */
  /* for */
  fclose ( odexm_args );
  *log = oxm_runcmd ( rcs_monitor, i, av, NULL );
  while ( oxm_gets( rcs_monitor, buf, sizeof(buf), log ) != NULL) {
    rm_newline ( buf );
    ui_print ( VNORMAL, "%s\n", buf );
  } /* while */
  *log = oxm_endcmd ( rcs_monitor, &status );
  if ( ( *log = oxm_close ( rcs_monitor ) ) != OE_OK )
    return ( FALSE );
  /* if */
  unlink ( tempfile );
  rmdir ( tempdir );
  return ( status );
} /* end sci_check_out_client */
